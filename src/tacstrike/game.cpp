

#include "tsgame.hpp"


GameLevel* g_GameLevel = NULL;
SoundSystemHandle g_SoundSys;
SGRX_LineSet g_DebugLines;

InputState MOVE_LEFT( "move_left" );
InputState MOVE_RIGHT( "move_right" );
InputState MOVE_UP( "move_up" );
InputState MOVE_DOWN( "move_down" );
InputState MOVE_X( "move_x" );
InputState MOVE_Y( "move_y" );
InputState AIM_X( "aim_x", 0 );
InputState AIM_Y( "aim_y", 0 );
InputState SHOOT( "shoot" );
InputState LOCK_ON( "lock_on" );
InputState RELOAD( "reload" );
InputState SLOW_WALK( "slow_walk" );
InputState SPRINT( "sprint" );
InputState CROUCH( "crouch" );
InputState SHOW_OBJECTIVES( "show_objectives" );
InputState DO_ACTION( "do_action" );
InputState SLOWDOWN_TEST( "slowdown_test" );
Vec2 CURSOR_POS = V2(0);



//static sgs_Prof prof;
TSFightGameMode::TSFightGameMode( GameLevel* lev ) :
	IGameLevelSystem( lev, e_system_uid ), m_state( GS_TEST2 ),
	m_timeout( 3 ), m_points_ply( 0 ), m_points_enm( 0 ), m_points_target( 10 ),
	m_respawnTimeout_ply( 0 ), m_respawnTimeout_enm( 0 )
{
	RegisterHandler( TSEV_CharDied );
	
	m_player = NULL;
	m_enemy = NULL;
	
	//sgs_ProfInit( m_level->GetSGSC(), &prof, SGS_PROF_FUNCTIME );
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
	switch( m_state )
	{
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
}

void TSFightGameMode::DrawUI()
{
	if( m_state == GS_Intro )
	{
		GR2D_SetFont( "core", 32 );
		GR2D_SetColor( 0, 0, 1 );
		char bfr[ 128 ];
		sgrx_snprintf( bfr, sizeof(bfr), "%d", (int) ceil( m_timeout ) );
		GR2D_DrawTextLine( GR_GetWidth() / 2, GR_GetHeight() / 2, bfr, HALIGN_CENTER, VALIGN_CENTER );
	}
	else if( m_state == GS_Playing )
	{
		GR2D_SetFont( "core", 32 );
		GR2D_SetColor( 1, 0, 0 );
		char bfr[ 128 ];
		sgrx_snprintf( bfr, sizeof(bfr), "[P] %d : [E] %d", m_points_ply, m_points_enm );
		GR2D_DrawTextLine( GR_GetWidth() / 2, GR_GetHeight() / 16, bfr, HALIGN_CENTER, VALIGN_CENTER );
	}
}

void TSFightGameMode::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	switch( eid )
	{
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



#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct TACStrikeGame : IGame, SGRX_DebugDraw
{
	TACStrikeGame() : m_accum( 0.0f )
	{
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
		return true;
	}
	
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_LoadFont( "fancy", "fonts/gratis.ttf" );
		GR2D_LoadFont( "mono", "fonts/dejavu-sans-mono-regular.ttf:nohint" );
		GR2D_LoadSVGIconFont( "tsicons", "ui/tsicons.svf" );
		GR2D_SetFont( "core", 12 );
		
		g_SoundSys = SND_CreateSystem();
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &SHOOT );
		Game_RegisterAction( &LOCK_ON );
		Game_RegisterAction( &RELOAD );
		Game_RegisterAction( &SLOW_WALK );
		Game_RegisterAction( &SPRINT );
		Game_RegisterAction( &CROUCH );
		Game_RegisterAction( &DO_ACTION );
		
		Game_BindKeyToAction( SDLK_a, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_d, &MOVE_RIGHT );
		Game_BindKeyToAction( SDLK_w, &MOVE_UP );
		Game_BindKeyToAction( SDLK_s, &MOVE_DOWN );
	//	Game_BindKeyToAction( SDLK_g, &SHOOT );
		Game_BindMouseButtonToAction( SGRX_MB_LEFT, &SHOOT );
		Game_BindMouseButtonToAction( SGRX_MB_RIGHT, &LOCK_ON );
		Game_BindKeyToAction( SDLK_r, &RELOAD );
	//	Game_BindKeyToAction( SDLK_LCTRL, &SLOW_WALK );
		Game_BindKeyToAction( SDLK_LSHIFT, &SPRINT );
		Game_BindKeyToAction( SDLK_LCTRL, &CROUCH );
		Game_BindKeyToAction( SDLK_SPACE, &DO_ACTION );
		Game_BindKeyToAction( SDLK_TAB, &SLOWDOWN_TEST );
		
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_LEFTX, &MOVE_X );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_LEFTY, &MOVE_Y );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_RIGHTX, &AIM_X );
		Game_BindGamepadAxisToAction( SDL_CONTROLLER_AXIS_RIGHTY, &AIM_Y );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, &SHOOT );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_LEFTSHOULDER, &LOCK_ON );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_Y, &RELOAD );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_A, &CROUCH );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_X, &DO_ACTION );
		
		g_GameLevel = new GameLevel( PHY_CreateWorld() );
		g_GameLevel->SetGlobalToSelf();
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
		AddSystemToLevel<TSGameSystem>( g_GameLevel );
		AddSystemToLevel<InfoEmissionSystem>( g_GameLevel );
		AddSystemToLevel<LevelMapSystem>( g_GameLevel );
		AddSystemToLevel<MessagingSystem>( g_GameLevel );
		AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
		AddSystemToLevel<FlareSystem>( g_GameLevel );
		AddSystemToLevel<LevelCoreSystem>( g_GameLevel );
		AddSystemToLevel<ScriptedSequenceSystem>( g_GameLevel );
		AddSystemToLevel<MusicSystem>( g_GameLevel );
		AddSystemToLevel<DamageSystem>( g_GameLevel );
		AddSystemToLevel<BulletSystem>( g_GameLevel );
		AddSystemToLevel<AIDBSystem>( g_GameLevel );
		AddSystemToLevel<CoverSystem>( g_GameLevel );
		AddSystemToLevel<StockEntityCreationSystem>( g_GameLevel );
		AddSystemToLevel<TSFightGameMode>( g_GameLevel );
		
	//	Game_AddOverlayScreen( &g_SplashScreen );
		
	//	GR_LoadAnims( "meshes/animtest.ssm.anm", "my_" );
		GR_LoadAnims( "meshes/tstest.ssm.anm" );
//		GR_LoadAnims( "meshes/charmodel2.ssm.anm" );
		
		g_GameLevel->Load( "ai-test" );
	//	g_GameLevel->Load( "v3decotest" );
		
#if 0
		mylight = g_GameLevel->GetScene()->CreateLight();
		mylight->type = LIGHT_SPOT;
		mylight->enabled = true;
		mylight->position = V3(1,-1,1);
		mylight->direction = V3(0.5f,0.5f,-0.5f);
		mylight->color = V3(0.9f,0.8f,0.7f) * 10;
		mylight->angle = 90;
		mylight->range = 10;
		mylight->cookieTexture = GR_GetTexture( "textures/cookies/default.png" );
		mylight->shadowTexture = GR_CreateRenderTexture( 128, 128, RT_FORMAT_DEPTH );
		mylight->hasShadows = true;
		mylight->UpdateTransform();
		
		sgsVariable args = g_GameLevel->m_scriptCtx.CreateDict();
		args.setprop( "position", g_GameLevel->m_scriptCtx.CreateVec3( V3(2,-2,1) ) );
		args.setprop( "rot_angles", g_GameLevel->m_scriptCtx.CreateVec3( V3(0,0,0) ) );
		myscritem = SGRX_ScriptedItem::Create(
			g_GameLevel->m_scene,
			g_GameLevel->GetPhyWorld(),
			g_GameLevel->m_scriptCtx.C,
			g_GameLevel->m_scriptCtx.GetGlobal( "SCRITEM_CREATE_window1" ),
			args
		);
		myscritem->SetLightSampler( g_GameLevel );
		myscritem->SetPSRaycast( g_GameLevel->GetSystem<DamageSystem>() );
#endif
		
		Game_AddOverlayScreen( &g_PauseMenu );
		cursor_dt = V2(0);
		return true;
	}
	void OnDestroy()
	{
		if( myscritem )
		{
			myscritem->Release();
			myscritem = NULL;
		}
		mylight = NULL;
		
		delete g_GameLevel;
		g_GameLevel = NULL;
		
		g_SoundSys = NULL;
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
	}
	
	void Game_FixedTick( float dt )
	{
		g_GameLevel->FixedTick( dt );
		
		if( myscritem ) myscritem->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
		
		if( myscritem )
		{
			myscritem->Tick( dt, bf );
			myscritem->PreRender();
		}
		
#ifdef TESTSHOOT
		Vec3 CP, CD;
		if( SHOOT.value && g_GameLevel->m_scene->camera.GetCursorRay(
			Game_GetCursorPos().x / GR_GetWidth(), Game_GetCursorPos().y / GR_GetHeight(), CP, CD ) )
		{
			g_GameLevel->m_bulletSystem.Add( CP, CD * 10, 1, 1 );
		}
#endif
	}
	void Game_Render()
	{
	//	g_GameLevel->Draw();
		SGRX_RenderScene rsinfo( V4( g_GameLevel->m_levelTime ), g_GameLevel->m_scene );
		rsinfo.debugdraw = this;
		rsinfo.postdraw = g_GameLevel;
		GR_RenderScene( rsinfo );
		
		g_GameLevel->Draw2D();
		
		//
		// TEST
		//
#if 0
		GR2D_SetColor( 1 );
		char bfr[ 1204 ];
		sprintf( bfr, "meshes: %d, draw calls: %d", (int) GR_GetRenderStats().numVisMeshes, (int) GR_GetRenderStats().numMDrawCalls );
	//	GR2D_SetFont( "core", 24 );
	//	GR2D_DrawTextLine( bfr );
		
		GR2D_SetFont( "tsicons", 24 );
		GR2D_DrawTextLine( 64, 64, "i" );
		GR2D_SetFont( "tsicons", 48 );
		GR2D_DrawTextLine( "#", HALIGN_LEFT, VALIGN_CENTER );
		GR2D_SetFont( "core", 24 );
		GR2D_DrawTextLine( " test: press " );
		GR2D_SetFont( "tsicons", 24 );
		GR2D_DrawTextLine( "t" );
		GR2D_SetFont( "core", 24 );
		GR2D_DrawTextLine( " to act " );
		GR2D_SetFont( "tsicons", 24 );
		GR2D_DrawTextLine( "!" );
#endif
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
	//	CURSOR_POS += V2( AIM_X.value, AIM_Y.value ) * TMIN( GR_GetWidth(), GR_GetHeight() ) * 0.03f;
		CURSOR_POS += cursor_dt;
		CURSOR_POS.x = clamp( CURSOR_POS.x, 0, GR_GetWidth() );
		CURSOR_POS.y = clamp( CURSOR_POS.y, 0, GR_GetHeight() );
		
		g_SoundSys->Update();
		g_GameLevel->ProcessEvents();
		
		if( dt > MAX_TICK_SIZE )
			dt = MAX_TICK_SIZE;
		if( SLOWDOWN_TEST.value )
			dt *= 0.25f;
		
		m_accum += dt;
		while( m_accum >= 0 )
		{
			Game_FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		Game_Tick( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
		
		Game_Render();
	}
	void DebugDraw()
	{
//		g_GameLevel->DebugDraw();
	//	g_GameLevel->GetScene()->DebugDraw_MeshRaycast();
		
		TSCharacter* PLY = (TSCharacter*) g_GameLevel->m_player;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
#if TEST_PATHFINDER
		br.Reset().Col( 1, 0, 0 );
		
		Vec3 otg = V3(0,0,0);
		Array< Vec3 > path;
		AIDBSystem* aidbSys = g_GameLevel->GetSystem<AIDBSystem>();
		Vec3 rpos, rdir;
		Vec2 cpn = Game_GetCursorPosNormalized();
		SceneRaycastInfo srci;
		if( g_GameLevel->GetScene()->camera.GetCursorRay( cpn.x, cpn.y, rpos, rdir ) &&
			g_GameLevel->GetScene()->RaycastOne( rpos, rdir * 100, &srci ) &&
			aidbSys->m_pathfinder.FindPath(
				PLY->GetPosition() + otg, rpos + rdir * 100 * srci.factor + otg, path ) &&
			path.size() >= 2 )
		{
			br.SetPrimitiveType( PT_LineStrip );
			for( size_t i = 0; i < path.size(); ++i )
				br.Pos( path[ i ] );
		}
#endif
		
#if TEST_COVERSYSTEM
		CSCoverInfo cinfo;
		CoverSystem* CS = g_GameLevel->GetSystem<CoverSystem>();
		Vec3 viewer_pos = PLY->GetPosition(); // V3(-8,10,1);
		CS->QueryLines( V3(-100), V3(100), 0.5f, 0.5f, viewer_pos, true, cinfo );
		Vec4 sphere = V4( viewer_pos, 1 );
		cinfo.ClipWithSpheres( &sphere, 1 );
		for( size_t i = 0; i < cinfo.covers.size(); ++i )
		{
			CSCoverLine& CL = cinfo.covers[ i ];
			g_DebugLines.DrawLine( CL.p0, CL.p1 );
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
		
		g_DebugLines.Flush();
		
//		g_PhyWorld->DebugDraw();
	}
	
	float m_accum;
	
	SGRX_ScriptedItem* myscritem;
	LightHandle mylight;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}

