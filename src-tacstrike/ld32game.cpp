

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#include "game.hpp"


GameLevel* g_GameLevel = NULL;
bool g_Paused = false;
PhyWorldHandle g_PhyWorld;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command INTERACT( "interact" );
Command CROUCH( "crouch" );
// UNUSED
Command SHOW_OBJECTIVES( "show_objectives" );


static void resetcontrols()
{
	MOVE_LEFT.value = 0;
	MOVE_RIGHT.value = 0;
	MOVE_UP.value = 0;
	MOVE_DOWN.value = 0;
	INTERACT.value = 0;
	CROUCH.value = 0;
}


SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_SHADOWS, RPF_ENABLED, 1, 0, 0, "shadow" },
	{ RPT_SCREEN, RPF_ENABLED, 1, 0, 0, "ps_ss_fog" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 4, "ext_s4" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 4, "ext_s4" },
};



void Game_SetPaused( bool paused )
{
	g_Paused = paused;
}

bool Game_IsPaused()
{
	return g_Paused;
}



ActionInput g_i_move_left = ACTINPUT_MAKE_KEY( SDLK_a );
ActionInput g_i_move_right = ACTINPUT_MAKE_KEY( SDLK_d );
ActionInput g_i_move_up = ACTINPUT_MAKE_KEY( SDLK_w );
ActionInput g_i_move_down = ACTINPUT_MAKE_KEY( SDLK_s );
ActionInput g_i_interact = ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
ActionInput g_i_crouch = ACTINPUT_MAKE_KEY( SDLK_c );

float g_i_mouse_sensitivity = 1.0f;
bool g_i_mouse_invert_x = false;
bool g_i_mouse_invert_y = false;

float g_s_vol_master = 0.8f;
float g_s_vol_sfx = 0.8f;
float g_s_vol_music = 0.8f;



Player::Player( const Vec3& pos, const Vec3& dir ) :
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ),
	m_jumpTimeout(0), m_canJumpTimeout(0), m_footstepTime(0), m_isOnGround(false), m_isCrouching(0),
	m_ivPos( pos ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false )
{
}

void Player::FixedTick( float deltaTime )
{
}

void Player::Tick( float deltaTime, float blendFactor )
{
}

void Player::DrawUI()
{
}

bool Player::AddItem( const StringView& item, int count )
{
	String key = item;
	int* ic = m_items.getptr( key );
	if( count < 0 )
	{
		if( !ic || *ic < count )
			return false;
		*ic += count;
	}
	else
	{
		if( !ic )
			m_items.set( key, count );
		else
			*ic += count;
	}
	return true;
}

bool Player::HasItem( const StringView& item, int count )
{
	int* ic = m_items.getptr( item );
	return ic && *ic >= count;
}


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct LD32Game : IGame
{
	LD32Game() : m_accum( 0.0f ), m_lastFrameReset( false )
	{
	}
	
	void OnConfigure( int argc, char** argv )
	{
		RenderSettings rs = { 0, 1024, 576, 60, FULLSCREEN_NONE, true, ANTIALIAS_NONE, 4 };
		
		InLocalStorage ils( "CrageGames/LD32Game" );
		
		String configdata;
		if( LoadTextFile( "config.cfg", configdata ) )
		{
			StringView key, value;
			ConfigReader cr( configdata );
			while( cr.Read( key, value ) )
			{
				if(0);
				// GRAPHICS
				else if( key == "r_display" ) rs.display = TMAX( 0, TMIN( GR_GetDisplayCount() - 1, (int) String_ParseInt( value ) ) );
				else if( key == "r_width" ) rs.width = TMAX( 1, TMIN( 16384, (int) String_ParseInt( value ) ) );
				else if( key == "r_height" ) rs.height = TMAX( 1, TMIN( 16384, (int) String_ParseInt( value ) ) );
				else if( key == "r_refresh_rate" ) rs.refresh_rate = TMAX( 1, TMIN( 1200, (int) String_ParseInt( value ) ) );
				else if( key == "r_fullscreen" ) rs.fullscreen = TMAX( 0, TMIN( 2, (int) String_ParseInt( value ) ) );
				else if( key == "r_vsync" ) rs.vsync = String_ParseBool( value );
				else if( key == "r_antialias" )
				{
					if( value == "none" );
					else if( value == "msaa2x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 2;
					}
					else if( value == "msaa4x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 4;
					}
					else if( value == "msaa8x" )
					{
						rs.aa_mode = ANTIALIAS_MULTISAMPLE;
						rs.aa_quality = 8;
					}
				}
				// SOUND
				else if( key == "s_vol_master" ) g_s_vol_master = clamp( String_ParseFloat( value ), 0, 1 );
				else if( key == "s_vol_music" ) g_s_vol_music = clamp( String_ParseFloat( value ), 0, 1 );
				else if( key == "s_vol_sfx" ) g_s_vol_sfx = clamp( String_ParseFloat( value ), 0, 1 );
				// INPUT
				else if( key == "i_move_left"       ) g_i_move_left       = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_right"      ) g_i_move_right      = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_up"         ) g_i_move_up         = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_move_down"       ) g_i_move_down       = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_interact"        ) g_i_interact        = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_crouch"          ) g_i_crouch          = ACTINPUT_MAKE( String_ParseInt( value.until(":") ), String_ParseInt( value.after(":") ) );
				else if( key == "i_mouse_sensitivity" ) g_i_mouse_sensitivity = String_ParseFloat( value );
				else if( key == "i_mouse_invert_x" ) g_i_mouse_invert_x = String_ParseBool( value );
				else if( key == "i_mouse_invert_y" ) g_i_mouse_invert_y = String_ParseBool( value );
			}
		}
		
		// TODO override with command line params
		
		GR_SetVideoMode( rs );
	}
	
	void OnInitialize()
	{
	//	g_SoundSys = SND_CreateSystem();
		
		g_PhyWorld = PHY_CreateWorld();
		g_PhyWorld->SetGravity( V3( 0, 0, -9.81f ) );
		
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &INTERACT );
		Game_RegisterAction( &CROUCH );
		
		Game_BindInputToAction( g_i_move_left, &MOVE_LEFT );
		Game_BindInputToAction( g_i_move_right, &MOVE_RIGHT );
		Game_BindInputToAction( g_i_move_up, &MOVE_UP );
		Game_BindInputToAction( g_i_move_down, &MOVE_DOWN );
		Game_BindInputToAction( g_i_interact, &INTERACT );
		Game_BindInputToAction( g_i_crouch, &CROUCH );
		
	//	g_SoundSys->Load( "sound/master.bank" );
	//	g_SoundSys->Load( "sound/master.strings.bank" );
	//	
	//	g_SoundSys->SetVolume( "bus:/", g_s_vol_master );
	//	g_SoundSys->SetVolume( "bus:/music", g_s_vol_music );
	//	g_SoundSys->SetVolume( "bus:/sfx", g_s_vol_sfx );
	//	LOG << g_SoundSys->GetVolume( "bus:/music" );
	//	
	//	m_music = g_SoundSys->CreateEventInstance( "/lev1_music" );
	//	m_music->Start();
		
		g_GameLevel = new GameLevel();
		g_GameLevel->m_scene->skyTexture = GR_GetTexture( "textures/env/back.dds" );
		
	//	Game_AddOverlayScreen( &g_MainMenu );
	//	Game_AddOverlayScreen( &g_SplashScreen );
		
		g_GameLevel->Load( "test1" );
		g_GameLevel->Tick( 0, 0 );
	}
	void OnDestroy()
	{
		Game_RemoveAllOverlayScreens();
		
		delete g_GameLevel;
		g_GameLevel = NULL;
		
	//	m_music->Stop();
	//	m_music = NULL;
		
		g_PhyWorld = NULL;
	//	g_SoundSys = NULL;
	}
	
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST )
		{
			m_lastFrameReset = false;
			Game_ShowCursor( true );
		}
		if( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED )
		{
			if( Game_HasOverlayScreens() == false )
				Game_ShowCursor( false );
		}
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			if( Game_HasOverlayScreens() == false )
	;//			Game_AddOverlayScreen( &g_PauseMenu );
		}
	}
	
	void Game_FixedTick( float dt )
	{
		int ITERS = 10;
		if( !g_GameLevel->m_paused )
		{
			for( int i = 0; i < ITERS; ++i )
				g_PhyWorld->Step( dt / ITERS );
		}
		g_GameLevel->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
		if( g_GameLevel->m_endFactor >= 0 && !Game_HasOverlayScreens() )
		{
	;//		Game_AddOverlayScreen( &g_EndMenu );
			Game_ShowCursor( true );
		}
	}
	void Game_Render()
	{
		g_GameLevel->Draw();
		g_GameLevel->Draw2D();
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
#if 0
			TODO
		g_SoundSys->Update();
		
		if( g_GameLevel->m_player )
		{
			g_GameLevel->m_player->inCursorMove = V2(0);
			if( Game_HasOverlayScreens() == false )
			{
				Vec2 cpos = Game_GetCursorPos();
				Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 curmove = cpos - opos;
				if( m_lastFrameReset )
					g_GameLevel->m_player->inCursorMove = curmove * V2( g_i_mouse_invert_x ? -1 : 1, g_i_mouse_invert_y ? -1 : 1 ) * g_i_mouse_sensitivity;
				m_lastFrameReset = true;
			}
			else
				m_lastFrameReset = false;
		}
#endif
		
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
	bool m_lastFrameReset;
	
	SoundEventInstanceHandle m_music;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

