

#include "tsgame.hpp"


Vec2 CURSOR_POS = V2(0);



#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct TACStrikeGame : BaseGame, SGRX_DebugDraw
{
	TACStrikeGame()
	{
		RegisterCommonGameCVars();
		m_mapName = "ts-mainmenu";
	}
	
	PhyWorldHandle CreatePhyWorld(){ return PHY_CreateWorld(); }
	
	void InitSoundSystem()
	{
		if( !m_soundSys )
		{
			m_soundSys = SND_CreateSystem();
			m_soundSys->Load( "master.bank" );
			m_soundSys->Load( "master.strings.bank" );
		}
	}
	
	GameLevel* CreateLevel()
	{
		InitSoundSystem();
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
		InitSoundSystem();
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_LoadFont( "fancy", "fonts/gratis.ttf" );
		GR2D_LoadFont( "mono", "fonts/dejavu-sans-mono-regular.ttf:nohint" );
		GR2D_LoadSVGIconFont( "tsicons", "ui/tsicons.svf" );
		GR2D_SetFont( "core", 12 );
		
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
		
	//	Game_AddOverlayScreen( &g_SplashScreen );
		
	//	GR_LoadAnims( "meshes/animtest.ssm.anm", "my_" );
///////		GR_LoadAnims( "meshes/chars/tstest.anb" );
//		GR_LoadAnims( "meshes/charmodel2.ssm.anm" );
		
		if( !BaseGame::OnInitialize() )
			return false;
		
	//	m_level->m_enableLoadingScreen = false;
		m_level->Load( m_mapName );
	//	m_level->Load( "tpstest" );
	//	m_level->Load( "b5-1" );
	//	m_level->Load( "ai-test-suite" );
	//	m_level->Load( "gp-test-suite" );
	//	m_level->Load( "v3decotest" );
		
	//	Game_AddOverlayScreen( &g_PauseMenu );
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
		m_timeMultiplier = SLOWDOWN_TEST.state ? 0.25f : 1.0f;
		
		BaseGame::OnTick( dt, gametime );
	}
	void DebugDraw()
	{
		m_level->DebugDraw();
		
	//	BatchRenderer& br = GR2D_GetBatchRenderer();
		
#if TEST_PATHFINDER
		br.Reset().Col( 1, 0, 0 );
		
		Vec3 otg = V3(0,0,0);
		Array< Vec3 > path;
		AIDBSystem* aidbSys = m_level->GetSystem<AIDBSystem>();
		Vec3 rpos, rdir;
		Vec2 cpn = Game_GetCursorPosNormalized();
		SceneRaycastInfo srci;
		if( m_level->GetScene()->camera.GetCursorRay( cpn.x, cpn.y, rpos, rdir ) &&
			m_level->GetScene()->RaycastOne( rpos, rdir * 100, &srci ) &&
			aidbSys->m_pathfinder.FindPath(
				((TSCharacter*) m_level->m_player)->GetPosition() + otg,
				rpos + rdir * 100 * srci.factor + otg, path ) &&
			path.size() >= 2 )
		{
			br.SetPrimitiveType( PT_LineStrip );
			for( size_t i = 0; i < path.size(); ++i )
				br.Pos( path[ i ] );
		}
#endif
		
#if TEST_COVERSYSTEM
		CSCoverInfo cinfo;
		CoverSystem* CS = m_level->GetSystem<CoverSystem>();
		Vec3 viewer_pos = ((TSCharacter*) m_level->m_player)->GetPosition(); // V3(-8,10,1);
		CS->QueryLines( V3(-100), V3(100), 0.5f, 0.5f, viewer_pos, true, cinfo );
		Vec4 sphere = V4( viewer_pos, 1 );
		cinfo.ClipWithSpheres( &sphere, 1 );
		for( size_t i = 0; i < cinfo.covers.size(); ++i )
		{
			CSCoverLine& CL = cinfo.covers[ i ];
//			g_DebugLines.DrawLine( CL.p0, CL.p1 );
		}
		
		static Array<Vec3> positions;
#define MAX_POSS 100
		Vec3 genpos;
		if( cinfo.GetPosition( viewer_pos, Game_GetCursorPosNormalized().x, genpos ) )
		{
			positions.insert( 0, genpos );
			positions.resize( MAX_POSS );
		}
		for( size_t i = 0; i < positions.size(); ++i )
		{
			br.Reset().Col( 0, 1, 0, 1-float(i)/MAX_POSS ).Tick( positions[i], 0.1f );
		}
		br.Flush();
#endif
		
//		g_DebugLines.Flush();
		
//		g_PhyWorld->DebugDraw();
	}
};


extern "C" EXPORT IGame* CreateGame()
{
	return new TACStrikeGame;
}

