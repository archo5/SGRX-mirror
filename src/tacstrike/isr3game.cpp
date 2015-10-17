

#include "level.hpp"
#include "systems.hpp"
#include "entities.hpp"


GameLevel* g_GameLevel = NULL;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command INTERACT( "interact" );
Command SHOOT( "shoot" );
Command LOCK_ON( "lock_on" );
Command SHOW_OBJECTIVES( "show_objectives" );


ActionInput g_i_move_left = ACTINPUT_MAKE_KEY( SDLK_a );
ActionInput g_i_move_right = ACTINPUT_MAKE_KEY( SDLK_d );
ActionInput g_i_move_up = ACTINPUT_MAKE_KEY( SDLK_w );
ActionInput g_i_move_down = ACTINPUT_MAKE_KEY( SDLK_s );
ActionInput g_i_interact = ACTINPUT_MAKE_KEY( SDLK_e );
ActionInput g_i_shoot = ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
ActionInput g_i_lock_on = ACTINPUT_MAKE_MOUSE( SGRX_MB_RIGHT );
ActionInput g_i_show_objectives = ACTINPUT_MAKE_KEY( SDLK_TAB );

float g_s_vol_master = 0.8f;
float g_s_vol_sfx = 0.8f;
float g_s_vol_music = 0.8f;


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct DroneTheftGame : IGame
{
	DroneTheftGame() : m_accum( 0.0f ), m_lastFrameReset( false )
	{
	}
	
	bool OnConfigure( int argc, char** argv )
	{
		return true;
	}
	
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		
		g_SoundSys = SND_CreateSystem();
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &INTERACT );
		Game_RegisterAction( &SHOOT );
		Game_RegisterAction( &LOCK_ON );
		Game_RegisterAction( &SHOW_OBJECTIVES );
		
		Game_BindInputToAction( g_i_move_left, &MOVE_LEFT );
		Game_BindInputToAction( g_i_move_right, &MOVE_RIGHT );
		Game_BindInputToAction( g_i_move_up, &MOVE_UP );
		Game_BindInputToAction( g_i_move_down, &MOVE_DOWN );
		Game_BindInputToAction( g_i_interact, &INTERACT );
		Game_BindInputToAction( g_i_shoot, &SHOOT );
		Game_BindInputToAction( g_i_lock_on, &LOCK_ON );
		Game_BindInputToAction( g_i_show_objectives, &SHOW_OBJECTIVES );
		
		g_SoundSys->Load( "sound/master.bank" );
		g_SoundSys->Load( "sound/master.strings.bank" );
		
		g_SoundSys->SetVolume( "bus:/", g_s_vol_master );
		g_SoundSys->SetVolume( "bus:/music", g_s_vol_music );
		g_SoundSys->SetVolume( "bus:/sfx", g_s_vol_sfx );
		
		g_GameLevel = new GameLevel( PHY_CreateWorld() );
		g_GameLevel->SetGlobalToSelf();
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
		AddSystemToLevel<InfoEmissionSystem>( g_GameLevel );
	//	AddSystemToLevel<MessagingSystem>( g_GameLevel );
	//	AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
		AddSystemToLevel<HelpTextSystem>( g_GameLevel );
		AddSystemToLevel<FlareSystem>( g_GameLevel );
		AddSystemToLevel<LevelCoreSystem>( g_GameLevel );
		AddSystemToLevel<ScriptedSequenceSystem>( g_GameLevel );
		AddSystemToLevel<MusicSystem>( g_GameLevel );
		AddSystemToLevel<DamageSystem>( g_GameLevel );
		AddSystemToLevel<BulletSystem>( g_GameLevel );
		AddSystemToLevel<AIDBSystem>( g_GameLevel );
		AddSystemToLevel<StockEntityCreationSystem>( g_GameLevel );
	//	AddSystemToLevel<MLD62EntityCreationSystem>( g_GameLevel );
		
		HelpTextSystem* HTS = g_GameLevel->GetSystem<HelpTextSystem>();
		HTS->renderer = &htr;
		htr.lineHeightFactor = 1.4f;
		htr.buttonTex = GR_GetTexture( "ui/key.png" );
		htr.centerPos = V2( GR_GetWidth() / 2, GR_GetHeight() * 3 / 4 );
		htr.fontSize = GR_GetHeight() / 20;
		htr.SetNamedFont( "", "core" );
		
		GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
		g_GameLevel->Load( "level1" );
		g_GameLevel->Tick( 0, 0 );
		
		Game_ShowCursor( false );
		
		return true;
	}
	void OnDestroy()
	{
		delete g_GameLevel;
		g_GameLevel = NULL;
		
		htr.buttonTex = NULL;
		
		g_SoundSys = NULL;
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
				;//Game_AddOverlayScreen( &g_PauseMenu );
		}
	}
	void Game_FixedTick( float dt )
	{
		g_GameLevel->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
	}
	void Game_Render()
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		int W = GR_GetWidth();
		int H = GR_GetHeight();
		int minw = TMIN( W, H );
		
		htr.centerPos = V2( GR_GetWidth() / 2, GR_GetHeight() * 3 / 4 );
		htr.fontSize = GR_GetHeight() / 20;
		htr.buttonBorder = GR_GetHeight() / 80;
		
		g_GameLevel->Draw();
		g_GameLevel->Draw2D();
		
		#if 0
		if( g_GameLevel->m_player )
		{
			SGRX_CAST( MLD62Player*, player, g_GameLevel->m_player );
			if( player->Alive() == false )
			{
				float a = clamp( player->m_deadTimeout, 0, 1 );
				float s = 2 - smoothstep( a );
				br.Reset().Col( 1, a ).SetTexture( player->m_tex_dead_img )
				  .Box( W/2, H/2, minw*s/1, minw*s/2 ).Flush();
			}
		}
		#endif
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
		g_SoundSys->Update();
		
		#if 0
		if( g_GameLevel->m_player )
		{
			static_cast<MLD62Player*>(g_GameLevel->m_player)->inCursorMove = V2(0);
			if( Game_HasOverlayScreens() == false )
			{
				Vec2 cpos = Game_GetCursorPos();
				Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 curmove = cpos - opos;
				if( m_lastFrameReset )
					static_cast<MLD62Player*>(g_GameLevel->m_player)->inCursorMove = curmove * V2( g_i_mouse_invert_x ? -1 : 1, g_i_mouse_invert_y ? -1 : 1 ) * g_i_mouse_sensitivity;
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
	
	SGRX_HelpTextRenderer htr;
	float m_accum;
	bool m_lastFrameReset;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

