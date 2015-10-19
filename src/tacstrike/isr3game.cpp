

#include "isr3entities.hpp"


GameLevel* g_GameLevel = NULL;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command MOVE_X( "move_x" );
Command MOVE_Y( "move_y" );
Command AIM_X( "aim_x", 0 );
Command AIM_Y( "aim_y", 0 );
Command DO_ACTION( "do_action" );
Command SHOOT( "shoot" );
Command LOCK_ON( "lock_on" );
Command SHOW_OBJECTIVES( "show_objectives" );

Command CROUCH( "crouch" );

Vec2 CURSOR_POS = V2(0);


ActionInput g_i_move_left = ACTINPUT_MAKE_KEY( SDLK_a );
ActionInput g_i_move_right = ACTINPUT_MAKE_KEY( SDLK_d );
ActionInput g_i_move_up = ACTINPUT_MAKE_KEY( SDLK_w );
ActionInput g_i_move_down = ACTINPUT_MAKE_KEY( SDLK_s );
ActionInput g_i_do_action = ACTINPUT_MAKE_KEY( SDLK_SPACE );
ActionInput g_i_shoot = ACTINPUT_MAKE_MOUSE( SGRX_MB_LEFT );
ActionInput g_i_lock_on = ACTINPUT_MAKE_MOUSE( SGRX_MB_RIGHT );
ActionInput g_i_show_objectives = ACTINPUT_MAKE_KEY( SDLK_TAB );

float g_s_vol_master = 0.8f;
float g_s_vol_sfx = 0.8f;
float g_s_vol_music = 0.8f;


struct StartScreen : IScreen
{
	float m_timer;
	TextureHandle m_tx_logo;
	
	StartScreen() : m_timer(0)
	{
	}
	
	void OnStart()
	{
		m_timer = 0;
		m_tx_logo = GR_GetTexture( "ui/scr_title.png" );
	}
	void OnEnd()
	{
		m_tx_logo = NULL;
	}
	
	bool OnEvent( const Event& e )
	{
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE )
		{
			Game_ShowCursor( false );
			g_GameLevel->StartLevel();
			m_timer = 5;
		}
		return true;
	}
	bool Draw( float delta )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		// logo
		const TextureInfo& texinfo = m_tx_logo.GetInfo();
		float scale = GR_GetWidth() / 1024.0f;
		br.Reset().SetTexture( m_tx_logo ).Box( GR_GetWidth() / 2.0f, GR_GetHeight() / 2.1f, texinfo.width * scale, texinfo.height * scale );
		
		br.Reset();
		GR2D_SetFont( "core", GR_GetHeight()/20 );
		GR2D_DrawTextLine( GR_GetWidth()/2,GR_GetHeight()*3/4, "Press SPACE to start", HALIGN_CENTER, VALIGN_CENTER );
		
		return m_timer >= 5;
	}
}
g_StartScreen;


struct EndScreen : IScreen
{
	float m_timer;
	bool m_restart;
	bool success;
	
	EndScreen() : m_timer(0), m_restart(false), success(false)
	{
	}
	
	void OnStart()
	{
		m_timer = 0;
		m_restart = false;
	}
	void OnEnd()
	{
	}
	
	bool OnEvent( const Event& e )
	{
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r )
		{
			Game_ShowCursor( false );
			GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
			g_GameLevel->Load( "level1" );
			g_GameLevel->Tick( 0, 0 );
			g_GameLevel->StartLevel();
			m_restart = true;
		}
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE )
		{
			Game_End();
		}
		return true;
	}
	bool Draw( float delta )
	{
		m_timer += delta;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		if( success )
		{
			br.Reset()
				.Col( 0, clamp( m_timer / 3, 0, 0.4f ) )
				.Quad( 0, 0, GR_GetWidth(), GR_GetHeight() );
		}
		else
		{
			br.Reset()
				.Col( 0.5f, 0, 0, clamp( m_timer / 3, 0, 0.4f ) )
				.Quad( 0, 0, GR_GetWidth(), GR_GetHeight() );
		}
		
		br.Reset().Col( 1 );
		GR2D_SetFont( "core", GR_GetHeight()/20 );
		GR2D_DrawTextLine( GR_GetWidth()/2,GR_GetHeight()*2/4, 
			success ? "Information obtained! Good job!" : "Mission failed!", HALIGN_CENTER, VALIGN_CENTER );
		GR2D_DrawTextLine( GR_GetWidth()/2,GR_GetHeight()*3/4,
			"Press R to restart level", HALIGN_CENTER, VALIGN_CENTER );
		GR2D_DrawTextLine( GR_GetWidth()/2,GR_GetHeight()*3.5f/4,
			"Press Esc to quit", HALIGN_CENTER, VALIGN_CENTER );
		
		return m_restart;
	}
}
g_EndScreen;


void MissionFailed()
{
	if( Game_HasOverlayScreen( &g_EndScreen ) == false )
	{
		g_EndScreen.success = false;
		Game_AddOverlayScreen( &g_EndScreen );
	}
}

int EndGame( SGS_CTX )
{
	if( Game_HasOverlayScreen( &g_EndScreen ) == false )
	{
		g_EndScreen.success = true;
		Game_AddOverlayScreen( &g_EndScreen );
	}
	return 0;
};

float action = 0;
void RenewAction()
{
	action = 1;
}


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct DroneTheftGame : IGame
{
	DroneTheftGame() : cursor_dt(V2(0)), m_accum( 0.0f ), m_lastFrameReset( false )
	{
	}
	
	bool OnConfigure( int argc, char** argv )
	{
		return true;
	}
	
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_LoadFont( "mono", "fonts/dejavu-sans-mono-regular.ttf:nohint" );
		
		g_SoundSys = SND_CreateSystem();
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &DO_ACTION );
		Game_RegisterAction( &SHOOT );
		Game_RegisterAction( &LOCK_ON );
		Game_RegisterAction( &SHOW_OBJECTIVES );
		
		Game_BindInputToAction( g_i_move_left, &MOVE_LEFT );
		Game_BindInputToAction( g_i_move_right, &MOVE_RIGHT );
		Game_BindInputToAction( g_i_move_up, &MOVE_UP );
		Game_BindInputToAction( g_i_move_down, &MOVE_DOWN );
		Game_BindInputToAction( g_i_do_action, &DO_ACTION );
		Game_BindInputToAction( g_i_shoot, &SHOOT );
		Game_BindInputToAction( g_i_lock_on, &LOCK_ON );
		Game_BindInputToAction( g_i_show_objectives, &SHOW_OBJECTIVES );
		
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_LEFTX, &MOVE_X );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_LEFTY, &MOVE_Y );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_RIGHTX, &AIM_X );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_RIGHTY, &AIM_Y );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, &SHOOT );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_LEFTSHOULDER, &LOCK_ON );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_X, &DO_ACTION );
		
		g_SoundSys->Load( "sound/master.bank" );
		g_SoundSys->Load( "sound/master.strings.bank" );
		
		g_SoundSys->SetVolume( "bus:/", g_s_vol_master );
		g_SoundSys->SetVolume( "bus:/music", g_s_vol_music );
		g_SoundSys->SetVolume( "bus:/sfx", g_s_vol_sfx );
		
		g_GameLevel = new GameLevel( PHY_CreateWorld() );
		sgs_PushCFunction( g_GameLevel->GetSGSC(), EndGame );
		sgs_StoreGlobal( g_GameLevel->GetSGSC(), "EndGame" );
		g_GameLevel->SetGlobalToSelf();
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
		AddSystemToLevel<InfoEmissionSystem>( g_GameLevel );
		AddSystemToLevel<MessagingSystem>( g_GameLevel );
		AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
		AddSystemToLevel<HelpTextSystem>( g_GameLevel );
		AddSystemToLevel<FlareSystem>( g_GameLevel );
		AddSystemToLevel<LevelCoreSystem>( g_GameLevel );
		AddSystemToLevel<LevelMapSystem>( g_GameLevel );
	//	AddSystemToLevel<ScriptedSequenceSystem>( g_GameLevel );
		AddSystemToLevel<MusicSystem>( g_GameLevel );
		AddSystemToLevel<DamageSystem>( g_GameLevel );
		AddSystemToLevel<BulletSystem>( g_GameLevel );
		AddSystemToLevel<AIDBSystem>( g_GameLevel );
		AddSystemToLevel<StockEntityCreationSystem>( g_GameLevel );
		AddSystemToLevel<ISR3EntityCreationSystem>( g_GameLevel );
		
		HelpTextSystem* HTS = g_GameLevel->GetSystem<HelpTextSystem>();
		HTS->renderer = &htr;
		htr.lineHeightFactor = 1.4f;
		htr.buttonTex = GR_GetTexture( "ui/key.png" );
		htr.centerPos = V2( GR_GetWidth() / 2, GR_GetHeight() * 3 / 4 );
		htr.fontSize = GR_GetHeight() / 24;
		htr.SetNamedFont( "", "core" );
		
		GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
		g_GameLevel->Load( "level1" );
		g_GameLevel->Tick( 0, 0 );
		
		Game_ShowCursor( true );
		
		Game_AddOverlayScreen( &g_StartScreen );
		g_GameLevel->GetSystem<MusicSystem>()->sgsSetTrack( "/game_music" );
		
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
		if( e.type == SDL_MOUSEMOTION )
		{
			CURSOR_POS = Game_GetCursorPos();
		}
		else if( e.type == SDL_CONTROLLERAXISMOTION )
		{
#if 1
			float rad = TMIN( GR_GetWidth(), GR_GetHeight() ) * 0.05f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.35f )
			{
				cursor_dt = off.Normalized() *
					powf( TREVLERP<float>( 0.35f, 0.75f, off.Length() ), 1.5f ) * rad;
			}
			else
				cursor_dt = V2(0);
#else
			float rad = TMAX( GR_GetWidth(), GR_GetHeight() ) * 0.5f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.1f )
				CURSOR_POS = Game_GetScreenSize() * 0.5f + off * rad;
#endif
		}
	}
	void Game_FixedTick( float dt )
	{
		g_GameLevel->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		action = TMAX( action - dt * 0.5f, 0.0f );
		g_GameLevel->GetSystem<MusicSystem>()->sgsSetVar( "action", action > 0 );
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
			SGRX_CAST( ISR3Player*, player, g_GameLevel->m_player );
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
		CURSOR_POS += cursor_dt;
		CURSOR_POS.x = clamp( CURSOR_POS.x, 0, GR_GetWidth() );
		CURSOR_POS.y = clamp( CURSOR_POS.y, 0, GR_GetHeight() );
		
		g_SoundSys->Update();
		
		#if 0
		if( g_GameLevel->m_player )
		{
			static_cast<ISR3Player*>(g_GameLevel->m_player)->inCursorMove = V2(0);
			if( Game_HasOverlayScreens() == false )
			{
				Vec2 cpos = Game_GetCursorPos();
				Game_SetCursorPos( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 opos = V2( GR_GetWidth() / 2, GR_GetHeight() / 2 );
				Vec2 curmove = cpos - opos;
				if( m_lastFrameReset )
					static_cast<ISR3Player*>(g_GameLevel->m_player)->inCursorMove = curmove * V2( g_i_mouse_invert_x ? -1 : 1, g_i_mouse_invert_y ? -1 : 1 ) * g_i_mouse_sensitivity;
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
	Vec2 cursor_dt;
	float m_accum;
	bool m_lastFrameReset;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

