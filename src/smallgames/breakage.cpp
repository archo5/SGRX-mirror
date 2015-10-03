

#include <engine.hpp>
#include <sound.hpp>


SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command SPEED( "speed" );


struct GameLevel
{
	GameLevel()
	{
		m_scene = GR_CreateScene();
		m_scene->SetDefines( ":MOD_BLENDCOLOR 0:LIGHT_MULTIPLIER 1" );
		m_scene->ambientLightColor = V3( 0.32f, 0.36f, 0.42f ) * 0.3f;
		m_scene->dirLightColor = V3( 0.32f, 0.36f, 0.42f ) * 0.5f;
		m_scene->dirLightDir = V3( -1, 0.2f, -1 ).Normalized();
		
		m_mi_area = m_scene->CreateMeshInstance();
		m_mi_area->SetMesh( "models/area.ssm" );
		ASSERT( m_mi_area->GetMaterialCount() == 1 );
		m_mi_area->SetLightingMode( SGRX_LM_Decal );
		m_mi_area->GetMaterial(0).shader = "default";
		m_mi_area->GetMaterial(0).textures[0] = GR_GetTexture( "textures/wood.png" );
		m_mi_area->SetMITexture( 0, GR_GetTexture( "textures/area_ao.png" ) );
		GR_GetDummyLightSampler().LightMesh( m_mi_area );
		
		m_mi_sphere = m_scene->CreateMeshInstance();
		m_mi_sphere->SetMesh( "models/sphere.ssm" );
		ASSERT( m_mi_sphere->GetMaterialCount() == 1 );
		m_mi_sphere->GetMaterial(0).shader = "glass";
		m_mi_sphere->GetMaterial(0).blendMode = SGRX_MtlBlend_Basic;
		GR_GetDummyLightSampler().LightMesh( m_mi_sphere );
	}
	~GameLevel()
	{
	}
	
	void Tick( float dt )
	{
	//	dt *= m_speed;
		m_levelTime += dt;
		
		m_scene->camera.position = V3(0,0,20*0.8f);
		m_scene->camera.direction = V3(0,0,-1);
		m_scene->camera.updir = V3(0,-1,0);
		m_scene->camera.angle = 90;
		m_scene->camera.aspect = GR_GetWidth() / (float) GR_GetHeight();
		m_scene->camera.UpdateMatrices();
	}
	
	void Render()
	{
		SGRX_RenderScene rsinfo( V4( m_levelTime ), m_scene );
		GR_RenderScene( rsinfo );
	}
	
	float m_levelTime;
	
	SceneHandle m_scene;
	MeshInstHandle m_mi_area;
	MeshInstHandle m_mi_sphere;
}
*g_GameLevel;


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct BreakageGame : IGame
{
	BreakageGame() : m_accum( 0.0f )
	{
	}
	
	bool OnInitialize()
	{
		GR_GetDummyLightSampler().defval = 1;
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &SPEED );
		
		Game_BindKeyToAction( SDLK_a, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_d, &MOVE_RIGHT );
		Game_BindKeyToAction( SDLK_w, &MOVE_UP );
		Game_BindKeyToAction( SDLK_s, &MOVE_DOWN );
	//	Game_BindKeyToAction( SDLK_z, &SPEED );
		
	//	Game_AddOverlayScreen( &g_SplashScreen );
		g_GameLevel = new GameLevel;
		
		return true;
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

