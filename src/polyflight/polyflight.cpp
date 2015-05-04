

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#include <engine.hpp>
#include <sound.hpp>


bool g_Paused = false;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command SHOOT( "shoot" );
Command DASH( "dash" );


SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_SHADOWS, RPF_ENABLED, 1, 0, 0, "shadow" },
	{ RPT_SCREEN, RPF_ENABLED, 1, 0, 0, "ps_ss_fog" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	// { RPT_OBJECT, RPF_MTL_SOLID | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 4, "ext_s4" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	// { RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 4, "ext_s4" },
};



void Game_SetPaused( bool paused )
{
	g_Paused = paused;
}

bool Game_IsPaused()
{
	return g_Paused;
}


struct SplashScreen : IScreen
{
	float m_timer;
	TextureHandle m_tx_crage;
	TextureHandle m_tx_back;
	
	SplashScreen() : m_timer(0)
	{
	}
	
	void OnStart()
	{
		m_timer = 0;
		m_tx_crage = GR_GetTexture( "ui/crage_logo.png" );
		m_tx_back = GR_GetTexture( "ui/flare_bg.png" );
	}
	void OnEnd()
	{
		m_tx_crage = NULL;
		m_tx_back = NULL;
	}
	
	bool OnEvent( const Event& e )
	{
		if( m_timer > 0.5 && ( e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN ) )
		{
			m_timer = 5;
		}
		return true;
	}
	bool Draw( float delta )
	{
		m_timer += delta;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.UnsetTexture()
			.Col( 0, 1 )
			.Quad( 0, 0, GR_GetWidth(), GR_GetHeight() );
		
		float vis_crage = smoothlerp_range( m_timer, 0, 1, 3, 5 );
		if( vis_crage )
		{
			// background
			float maxw = TMAX( GR_GetWidth(), GR_GetHeight() ) + 50;
			br.SetTexture( m_tx_back ).Col( 1, vis_crage ).QuadWH( 0, -m_timer * 10, maxw, maxw );
			
			// logo
			const TextureInfo& texinfo = m_tx_crage.GetInfo();
			float scale = GR_GetWidth() / 1024.0f;
			br.SetTexture( m_tx_crage ).Box( GR_GetWidth() / 2.0f, GR_GetHeight() / 2.1f, texinfo.width * scale, texinfo.height * scale );
		}
		
		return m_timer > 5;
	}
}
g_SplashScreen;


struct GameLevel
{
	struct Rock
	{
		Vec3 m_pos;
		float m_radius;
		Vec3 m_rot_axis;
		float m_rot_angle_unit;
		MeshInstHandle m_inst;
		
		void Tick( float dt, float gtime )
		{
			m_inst->matrix = Mat4::CreateScale( V3( m_radius ) ) * Mat4::CreateRotationAxisAngle( m_rot_axis, m_rot_angle_unit * gtime ) * Mat4::CreateTranslation( m_pos );
		}
	};
	
	GameLevel() : m_createdPos(20)
	{
		m_mesh_rock = GR_GetMesh( "models/rock1.ssm" );
		m_tx_specmask = GR_GetTexture( "textures/specmask1.png" );
		m_tx_specenv = GR_GetTexture( "textures/specenv1.dds" );
		
		m_scene = GR_CreateScene();
		m_scene->skyTexture = GR_GetTexture( "textures/env1.dds" );
		m_scene->ambientLightColor = V3( 0.32f, 0.36f, 0.42f ) * 0.3f;
		m_scene->dirLightColor = V3( 0.32f, 0.36f, 0.42f ) * 0.5f;
		m_scene->dirLightDir = V3( -1, 0.2f, -1 ).Normalized();
	}
	~GameLevel()
	{
		m_rocks.clear();
	}
	
	void Tick( float dt )
	{
		m_levelTime += dt;
		
		m_shipPos.x += ( MOVE_RIGHT.value - MOVE_LEFT.value ) * dt * 100;
		m_shipPos.z += ( MOVE_UP.value - MOVE_DOWN.value ) * dt * 100;
		m_shipPos.y += dt * 50;
		
		for( size_t i = 0; i < m_rocks.size(); ++i )
		{
			if( m_rocks[ i ].m_pos.y + 1 < m_shipPos.y )
				m_rocks.uerase( i-- );
		}
		
		for( ; m_createdPos < m_shipPos.y + 100; m_createdPos += 3 )
			AddRock();
		
		for( size_t i = 0; i < m_rocks.size(); ++i )
			m_rocks[ i ].Tick( dt, m_levelTime );
		
		m_scene->camera.position = m_shipPos;
		m_scene->camera.direction = ( V3(0,m_shipPos.y + 100,0) - m_shipPos ).Normalized();
		m_scene->camera.updir = V3(0,0,1);
		m_scene->camera.aspect = GR_GetWidth() / (float) GR_GetHeight();
		m_scene->camera.UpdateMatrices();
	}
	
	void Render()
	{
		SGRX_RenderScene rsinfo( V4( m_levelTime ), m_scene );
		GR_RenderScene( rsinfo );
	}
	
	void AddRock()
	{
		Vec3 pos = V3( randf11() * 30, m_createdPos + randf() * 20, randf11() * 30 );
		m_rocks.push_back( Rock() );
		Rock& R = m_rocks.last();
		R.m_pos = pos;
		R.m_radius = 2 + randf() * 2;
		R.m_rot_axis = V3( randf11(), randf11(), randf11() ).Normalized();
		R.m_rot_angle_unit = 1.0f + randf11() * 0.1f;
		R.m_inst = m_scene->CreateMeshInstance();
		R.m_inst->mesh = m_mesh_rock;
		R.m_inst->textures[0] = m_tx_specmask;
		R.m_inst->textures[1] = m_tx_specenv;
	}
	
	float m_levelTime;
	
	Array< Rock > m_rocks;
	SceneHandle m_scene;
	Vec3 m_shipPos;
	float m_createdPos;
	
	MeshHandle m_mesh_rock;
	TextureHandle m_tx_specmask;
	TextureHandle m_tx_specenv;
}
*g_GameLevel;


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct PolyFlightGame : IGame
{
	PolyFlightGame() : m_accum( 0.0f )
	{
	}
	
	void OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &SHOOT );
		Game_RegisterAction( &DASH );
		
		Game_BindKeyToAction( SDLK_LEFT, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_RIGHT, &MOVE_RIGHT );
		Game_BindKeyToAction( SDLK_UP, &MOVE_UP );
		Game_BindKeyToAction( SDLK_DOWN, &MOVE_DOWN );
		Game_BindKeyToAction( SDLK_z, &SHOOT );
		Game_BindKeyToAction( SDLK_x, &DASH );
		
	//	Game_AddOverlayScreen( &g_SplashScreen );
		g_GameLevel = new GameLevel;
	}
	void OnDestroy()
	{
		delete g_GameLevel;
		g_GameLevel = NULL;
	}
	
	void Game_FixedTick( float dt )
	{
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt );
	}
	void Game_Render()
	{
		g_GameLevel->Render();
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
		if( dt > MAX_TICK_SIZE )
			dt = MAX_TICK_SIZE;
		
		m_accum += dt;
		while( m_accum >= 0 )
		{
			Game_FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		Game_Tick( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
		
		Game_Render();
	}
	
	float m_accum;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

