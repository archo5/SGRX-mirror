

#include "level.hpp"
#include "tsui.hpp"
#include "entities.hpp"
#include "entities_ts.hpp"


GameLevel* g_GameLevel = NULL;
SoundSystemHandle g_SoundSys;
SGRX_LineSet g_DebugLines;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command MOVE_X( "move_x" );
Command MOVE_Y( "move_y" );
Command AIM_X( "aim_x", 0 );
Command AIM_Y( "aim_y", 0 );
Command SHOOT( "shoot" );
Command RELOAD( "reload" );
Command SLOW_WALK( "slow_walk" );
Command SPRINT( "sprint" );
Command CROUCH( "crouch" );
Command SHOW_OBJECTIVES( "show_objectives" );
Command DO_ACTION( "do_action" );
Command SLOWDOWN_TEST( "slowdown_test" );
Vec2 CURSOR_POS = V2(0);



#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct TACStrikeGame : IGame, SGRX_DebugDraw
{
	TACStrikeGame() : m_accum( 0.0f )
	{
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
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_Y, &RELOAD );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_A, &CROUCH );
		Game_BindGamepadButtonToAction( SDL_CONTROLLER_BUTTON_X, &DO_ACTION );
		
		g_GameLevel = new GameLevel( PHY_CreateWorld() );
		g_GameLevel->SetGlobalToSelf();
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
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
		AddSystemToLevel<TSEntityCreationSystem>( g_GameLevel );
		
	//	Game_AddOverlayScreen( &g_SplashScreen );
		
		g_GameLevel->Load( "ai-test" );
	//	g_GameLevel->Load( "v3decotest" );
		
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
		
	//	GR_LoadAnims( "meshes/animtest.ssm.anm", "my_" );
		GR_LoadAnims( "meshes/tstest.ssm.anm" );
//		GR_LoadAnims( "meshes/charmodel2.ssm.anm" );
		
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
		
		g_GameLevel->StartLevel();
		
		Game_AddOverlayScreen( &g_PauseMenu );
		return true;
	}
	void OnDestroy()
	{
		myscritem->Release();
		myscritem = NULL;
		mylight = NULL;
		
		delete g_GameLevel;
		g_GameLevel = NULL;
		
		g_SoundSys = NULL;
	}
	
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_MOUSEMOTION )
		{
			CURSOR_POS = Game_GetCursorPos();
		}
		else if( e.type == SDL_CONTROLLERAXISMOTION )
		{
			float rad = TMAX( GR_GetWidth(), GR_GetHeight() ) * 0.5f;
			Vec2 off = V2( AIM_X.value, AIM_Y.value );
			if( off.Length() > 0.1f )
				CURSOR_POS = Game_GetScreenSize() * 0.5f + off * rad;
		}
	}
	
	void Game_FixedTick( float dt )
	{
		g_GameLevel->FixedTick( dt );
		
		myscritem->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
		
		myscritem->Tick( dt, bf );
		myscritem->PreRender();
		
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
		g_GameLevel->DebugDraw();
	//	g_GameLevel->GetScene()->DebugDraw_MeshRaycast();
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		CSCoverInfo cinfo;
		TSPlayer* PLY = (TSPlayer*) g_GameLevel->m_player;
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

