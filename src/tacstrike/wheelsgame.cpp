

#include "level.hpp"
#include "entities.hpp"


InputState ESCAPE( "escape" );
InputState MOVE_LEFT( "move_left" );
InputState MOVE_RIGHT( "move_right" );
InputState MOVE_UP( "move_up" );
InputState MOVE_DOWN( "move_down" );
InputState MOVE_X( "move_x" );
InputState MOVE_Y( "move_y" );
InputState AIM_X( "aim_x", 0 );
InputState AIM_Y( "aim_y", 0 );
InputState SET_SAVEPOINT( "set_savepoint" );
InputState GOTO_SAVEPOINT( "goto_savepoint" );
InputState JUMP( "jump" );
InputState SHOW_OBJECTIVES( "show_objectives" );
InputState DO_ACTION( "do_action" );
InputState SLOWDOWN_TEST( "slowdown_test" );
Vec2 CURSOR_POS = V2(0);


struct WheelsGame : BaseGame
{
	WheelsGame()
	{
		RegisterCommonGameCVars();
		m_mapName = "mainmenu";
	}
	
	bool OnConfigure( int argc, char* argv[] )
	{
#if 0
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.width = 1920;
		rs.height = 1080;
		rs.fullscreen = FULLSCREEN_WINDOWED;
		GR_SetVideoMode( rs );
#endif
#if 0
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.vsync = false;
		GR_SetVideoMode( rs );
#endif
		return BaseGame::OnConfigure( argc, argv );
	}
	
	PhyWorldHandle CreatePhyWorld(){ return PHY_CreateWorld(); }
	
	GameLevel* CreateLevel()
	{
		if( !m_soundSys )
		{
			m_soundSys = SND_CreateSystem();
		}
		GameLevel* level = BaseGame::CreateLevel();
		AddSystemToLevel<HelpTextSystem>( level );
		AddSystemToLevel<FlareSystem>( level );
		AddSystemToLevel<LevelCoreSystem>( level );
		AddSystemToLevel<GFXSystem>( level );
		AddSystemToLevel<ScriptedSequenceSystem>( level );
		AddSystemToLevel<MusicSystem>( level );
		AddSystemToLevel<DamageSystem>( level );
		AddSystemToLevel<StockEntityCreationSystem>( level );
		AddSystemToLevel<DevelopSystem>( level );
		
		HelpTextSystem* HTS = level->GetSystem<HelpTextSystem>();
		HTS->renderer = &htr;
		htr.lineHeightFactor = 1.4f;
	//	htr.buttonTex = GR_GetTexture( "ui/key.png" );
		htr.SetNamedFont( "", "core" );
		
		return level;
	}
	
	bool OnInitialize()
	{
		if( !m_soundSys )
		{
			m_soundSys = SND_CreateSystem();
		}
		m_soundSys->Load( "sound/master.bank" );
		m_soundSys->Load( "sound/master.strings.bank" );
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_LoadFont( "fancy", "fonts/gratis.ttf" );
		GR2D_LoadSVGIconFont( "tsicons", "ui/wheels-icons.svf" );
		GR2D_SetFont( "core", 12 );
		
		Game_RegisterAction( &ESCAPE );
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &MOVE_X );
		Game_RegisterAction( &MOVE_Y );
		Game_RegisterAction( &AIM_X );
		Game_RegisterAction( &AIM_Y );
		Game_RegisterAction( &SET_SAVEPOINT );
		Game_RegisterAction( &GOTO_SAVEPOINT );
		Game_RegisterAction( &JUMP );
		Game_RegisterAction( &SHOW_OBJECTIVES );
		Game_RegisterAction( &DO_ACTION );
		Game_RegisterAction( &SLOWDOWN_TEST );
		
		Game_BindKeyToAction( SDLK_ESCAPE, &ESCAPE );
		Game_BindKeyToAction( SDLK_a, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_d, &MOVE_RIGHT );
		Game_BindKeyToAction( SDLK_w, &MOVE_UP );
		Game_BindKeyToAction( SDLK_s, &MOVE_DOWN );
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
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_X, &DO_ACTION );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_B, &JUMP );
		
		if( !BaseGame::OnInitialize() )
			return false;
		
		m_level->m_enableLoadingScreen = false;
		m_level->Load( m_mapName );
		
		return true;
	}
	
	void OnDestroy()
	{
		BaseGame::OnDestroy();
		htr.buttonTex = NULL;
	}
	
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_KEYDOWN )
		{
			if( e.key.keysym.sym == SDLK_r &&
				e.key.repeat == 0 &&
				( e.key.keysym.mod & KMOD_CTRL ) &&
				( e.key.keysym.mod & KMOD_SHIFT ) )
			{
				String levname = m_level->GetLevelName();
				m_level->Load( levname );
			}
		}
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
		m_timeMultiplier = SLOWDOWN_TEST.state ? 0.25f : 1.0f;
		
		BaseGame::OnTick( dt, gametime );
	}
	
	SGRX_HelpTextRenderer htr;
};


extern "C" EXPORT IGame* CreateGame()
{
	return new WheelsGame;
}

