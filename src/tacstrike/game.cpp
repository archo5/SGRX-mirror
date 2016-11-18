

#include "tsgame.hpp"


Vec2 CURSOR_POS = V2(0);


struct TACStrikeGame : BaseGame
{
	TACStrikeGame()
	{
		RegisterCommonGameCVars();
	}
	
	SoundSystemHandle CreateSoundSystem(){ return SND_CreateSystem(); }
	PhyWorldHandle CreatePhyWorld(){ return PHY_CreateWorld(); }
	
	GameLevel* CreateLevel()
	{
		GameLevel* level = BaseGame::CreateLevel();
		AddSystemToLevel<TSGameSystem>( level );
		AddSystemToLevel<LevelMapSystem>( level );
		AddSystemToLevel<LevelCoreSystem>( level );
		AddSystemToLevel<GFXSystem>( level );
		AddSystemToLevel<ScriptedSequenceSystem>( level );
		AddSystemToLevel<MusicSystem>( level );
		AddSystemToLevel<DamageSystem>( level );
		AddSystemToLevel<BulletSystem>( level );
		AddSystemToLevel<AIDBSystem>( level );
		AddSystemToLevel<DevelopSystem>( level );
		
		return level;
	}
	
	bool OnInitialize()
	{
		Game_BindKeyToAction( SDLK_ESCAPE, &ESCAPE );
		Game_BindKeyToAction( SDLK_a, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_d, &MOVE_RIGHT );
		Game_BindKeyToAction( SDLK_w, &MOVE_UP );
		Game_BindKeyToAction( SDLK_s, &MOVE_DOWN );
		Game_BindMouseButtonToAction( SGRX_MB_LEFT, &WP_SHOOT );
		Game_BindMouseButtonToAction( SGRX_MB_RIGHT, &WP_LOCK_ON );
		Game_BindKeyToAction( SDLK_r, &WP_RELOAD );
		Game_BindKeyToAction( SDLK_g, &WP_DROP );
		Game_BindKeyToAction( SDLK_q, &WP_HOLSTER );
	//	Game_BindKeyToAction( SDLK_LCTRL, &SLOW_WALK );
		Game_BindKeyToAction( SDLK_LSHIFT, &SPRINT );
		Game_BindKeyToAction( SDLK_LCTRL, &CROUCH );
#if 0
		Game_BindKeyToAction( SDLK_SPACE, &DO_ACTION );
#else
		Game_BindKeyToAction( SDLK_f, &DO_ACTION );
		Game_BindKeyToAction( SDLK_SPACE, &JUMP );
#endif
		Game_BindKeyToAction( SDLK_TAB, &SLOWDOWN_TEST );
		
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_START, &ESCAPE );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_LEFTX, &MOVE_X );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_LEFTY, &MOVE_Y );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_RIGHTX, &AIM_X );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_RIGHTY, &AIM_Y );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, &WP_SHOOT );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_LEFTSHOULDER, &WP_LOCK_ON );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_RIGHTSTICK, &WP_REMOVE_LOCK_ON );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_Y, &WP_RELOAD );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_B, &CROUCH );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_A, &DO_ACTION );
	//	Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_X, &JUMP );
		
		if( !BaseGame::OnInitialize() )
			return false;
		
	//	m_level->m_enableLoadingScreen = false;
		m_level->Load( m_mapName );
		
		cursor_dt = V2(0);
		return true;
	}
	
	Vec2 cursor_dt;
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_MOUSEMOTION )
		{
			CURSOR_POS = Game_GetCursorPos();
		}
		else if( e.type == SDL_CONTROLLERAXISMOTION )
		{
#if AAA
			float rad = TMIN( GR_GetWidth(), GR_GetHeight() ) * 0.05f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.35f )
			{
				cursor_dt = off.Normalized() *
					powf( TREVLERP<float>( 0.35f, 0.75f, off.Length() ), 1.5f ) * rad;
			}
			else
				cursor_dt = V2(0);
#elif BBB
			float rad = TMAX( GR_GetWidth(), GR_GetHeight() ) * 0.5f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.1f )
				CURSOR_POS = Game_GetScreenSize() * 0.5f + off * rad;
#endif
		}
		else if( e.type == SDL_KEYDOWN )
		{
			if( e.key.keysym.sym == SDLK_r &&
				e.key.repeat == 0 &&
				( e.key.keysym.mod & KMOD_CTRL ) &&
				( e.key.keysym.mod & KMOD_SHIFT ) )
			{
				String levname = m_level->GetLevelName().str();
				m_level->Load( levname );
			}
		}
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
	//	CURSOR_POS += V2( AIM_X.value, AIM_Y.value ) * TMIN( GR_GetWidth(), GR_GetHeight() ) * 0.03f;
		CURSOR_POS += cursor_dt;
		CURSOR_POS.x = clamp( CURSOR_POS.x, 0, GR_GetWidth() );
		CURSOR_POS.y = clamp( CURSOR_POS.y, 0, GR_GetHeight() );
		m_level->GetScriptCtx().SetGlobal( "CURSOR_POS",
			m_level->GetScriptCtx().CreateVec2( CURSOR_POS ) );
		m_timeMultiplier = Game_GetActionState( "slowdown_test" ).state ? 0.25f : 1.0f;
		
		BaseGame::OnTick( dt, gametime );
	}
};


extern "C" EXPORT IGame* CreateGame()
{
	return new TACStrikeGame;
}

