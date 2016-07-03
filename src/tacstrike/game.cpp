

#include "tsgame.hpp"


InputState ESCAPE( "escape" );
InputState MOVE_LEFT( "move_left" );
InputState MOVE_RIGHT( "move_right" );
InputState MOVE_UP( "move_up" );
InputState MOVE_DOWN( "move_down" );
InputState MOVE_X( "move_x" );
InputState MOVE_Y( "move_y" );
InputState AIM_X( "aim_x", 0 );
InputState AIM_Y( "aim_y", 0 );
InputState WP_SHOOT( "wp_shoot" );
InputState WP_LOCK_ON( "wp_lock_on" );
InputState WP_RELOAD( "wp_reload" );
InputState WP_DROP( "wp_drop" );
InputState WP_HOLSTER( "wp_holster" );
InputState SLOW_WALK( "slow_walk" );
InputState SPRINT( "sprint" );
InputState CROUCH( "crouch" );
InputState JUMP( "jump" );
InputState SHOW_OBJECTIVES( "show_objectives" );
InputState DO_ACTION( "do_action" );
InputState SLOWDOWN_TEST( "slowdown_test" );
Vec2 CURSOR_POS = V2(0);



CVarFloat gcv_ts_time_since_last_hit( "ts_time_since_last_hit" );
CVarInt gcv_ts_fight_state( "ts_fight_state" );
CVarFloat gcv_ts_fight_timeleft( "ts_fight_timeleft" );
CVarInt gcv_ts_fight_p1_points( "ts_fight_p1_points" );
CVarInt gcv_ts_fight_p2_points( "ts_fight_p2_points" );

#if 0
//static sgs_Prof prof;
TSFightGameMode::TSFightGameMode( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ), m_state( GS_Intro ),
	m_timeout( 3 ), m_points_ply( 0 ), m_points_enm( 0 ), m_points_target( 10 ),
	m_respawnTimeout_ply( 0 ), m_respawnTimeout_enm( 0 ), m_timeSinceLastHit( 9999.0f )
{
	REGCOBJ( gcv_ts_time_since_last_hit );
	REGCOBJ( gcv_ts_fight_state );
	REGCOBJ( gcv_ts_fight_timeleft );
	REGCOBJ( gcv_ts_fight_p1_points );
	REGCOBJ( gcv_ts_fight_p2_points );
	
	RegisterHandler( TSEV_CharHit );
	RegisterHandler( TSEV_CharDied );
	
	m_player = NULL;
	m_enemy = NULL;
	
	//sgs_ProfInit( m_level->GetSGSC(), &prof, SGS_PROF_FUNCTIME );
	
	m_level->GetGUI()->Load( "ui/fight.sgs" );
}

TSFightGameMode::~TSFightGameMode()
{
	//sgs_ProfDump( m_level->GetSGSC(), &prof );
	//sgs_ProfClose( m_level->GetSGSC(), &prof );
}

void TSFightGameMode::OnPostLevelLoad()
{
	// pick spawnpoints
	Vec3 playerPos = m_spawnPoints[ rand() % m_spawnPoints.size() ];
	Vec3 enemyPos = PickFurthestSpawnPoint( playerPos );
	
	// create the player
	m_player = new TSCharacter
	(
		m_level,
		playerPos,
		( enemyPos - playerPos ).Normalized()
	);
	m_player->m_infoFlags |= IEST_Player;
	m_player->InitializeMesh( "chars/tstest.chr" );
	m_player->ownerType = GAT_Player;
	m_level->AddEntity( m_player );
	m_level->SetPlayer( m_player );
	m_actorCtrl_ply = &m_level->GetSystem<TSGameSystem>()->m_playerCtrl;
	
	// create the enemy
	m_enemy = new TSCharacter
	(
		m_level,
		enemyPos,
		( playerPos - enemyPos ).Normalized()
	);
	m_enemy->m_infoFlags |= IEST_Target;
	m_enemy->InitializeMesh( "chars/tstest.chr" );
	m_enemy->ownerType = GAT_Enemy;
	m_enemy->m_name = "enemy";
	m_level->MapEntityByName( m_enemy );
	m_level->AddEntity( m_enemy );
	m_actorCtrl_enm = new TSEnemyController( m_level, m_enemy, sgsVariable() );
	
	if( m_state == GS_TEST || m_state == GS_TEST2 )
	{
		m_player->ctrl = m_actorCtrl_ply;
	}
	
	m_state = GS_Intro;
	m_timeout = 3;
	m_points_ply = 0;
	m_points_enm = 0;
	m_points_target = 10;
	m_respawnTimeout_ply = 0;
	m_respawnTimeout_enm = 0;
}

bool TSFightGameMode::AddEntity( const StringView& type, sgsVariable data )
{
	if( type == "dm_spawn" )
	{
		m_spawnPoints.push_back( data.getprop("position").get<Vec3>() );
		return true;
	}
	
	return false;
}

void TSFightGameMode::Tick( float deltaTime, float blendFactor )
{
	m_timeout = TMAX( 0.0f, m_timeout - deltaTime );
	m_timeSinceLastHit += deltaTime;
	
	switch( m_state )
	{
	case GS_TEST2: break;
	case GS_TEST:
		if( m_timeout <= 0 )
		{
			Vec3 enemyPos = m_spawnPoints[ rand() % m_spawnPoints.size() ];
			TSCharacter* E = new TSCharacter
			(
				m_level,
				enemyPos,
				V3(1,0,0)
			);
			E->m_infoFlags |= IEST_Target;
			E->InitializeMesh( "chars/tstest.chr" );
			E->ownerType = GAT_Enemy;
			E->m_name = "enemy";
			m_level->MapEntityByName( E );
			m_level->AddEntity( E );
		//	E->ctrl = new TSEnemyController( m_level, E, sgsVariable() );
			m_timeout += 3;
		}
		break;
	case GS_Intro:
		// timeout to intro end
		if( m_timeout <= 0 )
		{
			m_level->GetGUI()->CallFunc( "ev_fight_start" );
			
			// start the game
			m_state = GS_Playing;
			
			// set controllers
			m_player->ctrl = m_actorCtrl_ply;
			m_enemy->ctrl = m_actorCtrl_enm;
		}
		break;
	case GS_Playing:
		// timeout to respawn
		m_respawnTimeout_ply = TMAX( 0.0f, m_respawnTimeout_ply - deltaTime );
		m_respawnTimeout_enm = TMAX( 0.0f, m_respawnTimeout_enm - deltaTime );
		if( m_player->IsAlive() == false && m_respawnTimeout_ply <= 0 )
		{
			m_actorCtrl_ply->Reset();
			m_player->Reset();
			m_player->SetPosition( PickFurthestSpawnPoint( m_enemy->GetPosition() ) );
		}
		if( m_enemy->IsAlive() == false && m_respawnTimeout_enm <= 0 )
		{
			m_actorCtrl_enm->Reset();
			m_enemy->Reset();
			m_enemy->SetPosition( PickFurthestSpawnPoint( m_player->GetPosition() ) );
		}
		break;
	case GS_Ending:
		break;
	}
	
	gcv_ts_time_since_last_hit.value = m_timeSinceLastHit;
	gcv_ts_fight_timeleft.value = m_timeout;
	gcv_ts_fight_state.value = m_state;
	gcv_ts_fight_p1_points.value = m_points_ply;
	gcv_ts_fight_p2_points.value = m_points_enm;
}

void TSFightGameMode::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	switch( eid )
	{
	case TSEV_CharHit:
		{
			SGRX_CAST( TSEventData_CharHit*, hitdata, edata.GetUserData() );
			
			if( m_player == hitdata->ch )
			{
				m_timeSinceLastHit = TMIN( m_timeSinceLastHit, 1.0f );
				m_timeSinceLastHit = clamp( m_timeSinceLastHit - hitdata->power * 0.1f, 0, 1 );
			}
		}
		break;
	case TSEV_CharDied:
		{
			SGRX_CAST( TSCharacter*, ch, edata.GetUserData() );
			
			if( m_enemy == ch )
			{
				m_points_ply++;
				m_respawnTimeout_enm = 3;
			}
			if( m_player == ch )
			{
				m_points_enm++;
				m_respawnTimeout_ply = 3;
			}
			
			if( m_points_ply >= m_points_target || m_points_enm >= m_points_target )
			{
				m_state = GS_Ending;
				
				// unlink controllers
				m_player->ctrl = NULL;
				m_enemy->ctrl = NULL;
			}
		}
		break;
	}
}

Vec3 TSFightGameMode::PickFurthestSpawnPoint( Vec3 from )
{
	float dist = 0;
	size_t fp = NOT_FOUND;
	for( size_t i = 0; i < m_spawnPoints.size(); ++i )
	{
		float ndst = ( m_spawnPoints[ i ] - from ).Length();
		if( ndst > dist )
		{
			dist = ndst;
			fp = i;
		}
	}
	return fp != NOT_FOUND ? m_spawnPoints[ fp ] : from;
}
#endif


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct TACStrikeGame : BaseGame, SGRX_DebugDraw
{
	TACStrikeGame()
	{
		RegisterCommonGameCVars();
		m_mapName = "b5-mainmenu";
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
		AddSystemToLevel<HelpTextSystem>( level );
		AddSystemToLevel<FlareSystem>( level );
		AddSystemToLevel<LevelCoreSystem>( level );
		AddSystemToLevel<GFXSystem>( level );
		AddSystemToLevel<ScriptedSequenceSystem>( level );
		AddSystemToLevel<MusicSystem>( level );
		AddSystemToLevel<DamageSystem>( level );
		AddSystemToLevel<BulletSystem>( level );
		AddSystemToLevel<AIDBSystem>( level );
		AddSystemToLevel<CoverSystem>( level );
		AddSystemToLevel<DevelopSystem>( level );
		
		HelpTextSystem* HTS = level->GetSystem<HelpTextSystem>();
		HTS->renderer = &htr;
		htr.lineHeightFactor = 1.4f;
		htr.buttonTex = GR_GetTexture( "ui/key.png" );
		htr.SetNamedFont( "", "core" );
		
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
		
		Game_RegisterAction( &ESCAPE );
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &WP_SHOOT );
		Game_RegisterAction( &WP_LOCK_ON );
		Game_RegisterAction( &WP_RELOAD );
		Game_RegisterAction( &WP_DROP );
		Game_RegisterAction( &WP_HOLSTER );
		Game_RegisterAction( &SLOW_WALK );
		Game_RegisterAction( &SPRINT );
		Game_RegisterAction( &CROUCH );
		Game_RegisterAction( &JUMP );
		Game_RegisterAction( &DO_ACTION );
		
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
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_Y, &WP_RELOAD );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_A, &CROUCH );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_X, &DO_ACTION );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_B, &JUMP );
		
	//	Game_AddOverlayScreen( &g_SplashScreen );
		
	//	GR_LoadAnims( "meshes/animtest.ssm.anm", "my_" );
///////		GR_LoadAnims( "meshes/chars/tstest.anb" );
//		GR_LoadAnims( "meshes/charmodel2.ssm.anm" );
		
		if( !BaseGame::OnInitialize() )
			return false;
		
		m_level->m_enableLoadingScreen = false;
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
	
	void OnDestroy()
	{
		BaseGame::OnDestroy();
		htr.buttonTex = NULL;
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
		else if( e.type == SDL_KEYDOWN )
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
	
	SGRX_HelpTextRenderer htr;
};


extern "C" EXPORT IGame* CreateGame()
{
	return new TACStrikeGame;
}

