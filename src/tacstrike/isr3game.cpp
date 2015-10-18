

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


struct ISR3Player : ISR3Drone
{
	Vec2 inCursorMove;
	Entity* m_targetII;
	bool m_targetTriggered;
	TSAimHelper m_aimHelper;
	
	ISR3Player( GameLevel* lev, Vec3 pos, Vec3 dir )
		: ISR3Drone( lev, pos, dir ), inCursorMove( V2(0) ),
		m_targetII( NULL ), m_targetTriggered( false ),
		m_aimHelper( lev )
	{
		m_animChar.Load( "chars/player.chr" );
		m_meshInstInfo.ownerType = GAT_Player;
		
		i_aim_at = true;
		
		m_shootPS.Load( "psys/gunflash.psy" );
		m_shootPS.AddToScene( m_level->GetScene() );
		m_shootPS.OnRenderUpdate();
		m_shootLT = m_level->GetScene()->CreateLight();
		m_shootLT->type = LIGHT_POINT;
		m_shootLT->enabled = false;
		m_shootLT->position = pos;
		m_shootLT->color = V3(0.9f,0.7f,0.5f)*1;
		m_shootLT->range = 4;
		m_shootLT->power = 4;
		m_shootLT->UpdateTransform();
		m_shootTimeout = 0;
	}
	
	void FixedTick( float deltaTime )
	{
		i_move = V2
		(
			-MOVE_X.value + MOVE_LEFT.value - MOVE_RIGHT.value,
			MOVE_Y.value + MOVE_DOWN.value - MOVE_UP.value
		);
		i_aim_target = m_aimHelper.GetAimPoint();
		if( i_move.Length() > 0.1f )
		{
			Vec2 md = i_move;
			if( Vec2Dot( ( i_aim_target - GetPosition() ).ToVec2(), md ) < 0 )
				md = -md;
		//	TurnTo( md, deltaTime * 8 );
		}
		
		if( DO_ACTION.value )
		{
			BeginClosestAction( 2 );
		}
		
		ISR3Drone::FixedTick( deltaTime );
	}
	
	void Tick( float deltaTime, float blendFactor )
	{
		ISR3Drone::Tick( deltaTime, blendFactor );
		
		Vec3 pos = m_ivPos.Get( blendFactor );
		
		float bmsz = ( GR_GetWidth() + GR_GetHeight() );// * 0.5f;
		Vec2 cursor_pos = CURSOR_POS;
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		Vec2 player_pos = m_level->GetScene()->camera.WorldToScreen( GetPosition() ).ToVec2() * screen_size;
		Vec2 diff = ( cursor_pos - player_pos ) / bmsz;
		m_aimHelper.Tick( deltaTime, GetPosition(), CURSOR_POS / screen_size, LOCK_ON.value > 0.5f );
		Vec3 camtgt = TLERP( pos, m_aimHelper.m_aimPoint, 0.1f * smoothstep( m_aimHelper.m_aimFactor ) );
		
		m_level->GetScene()->camera.znear = 0.1f;
		m_level->GetScene()->camera.angle = 90;
		m_level->GetScene()->camera.updir = V3(0,-1,0);
		m_level->GetScene()->camera.direction = V3(-diff.x,diff.y,-5);
		m_level->GetScene()->camera.position = camtgt + V3(-diff.x,diff.y,0) * 2 + V3(0,0,1) * 6;
		m_level->GetScene()->camera.UpdateMatrices();
		
		InfoEmissionSystem::Data D = { pos, 0.5f, IEST_HeatSource | IEST_Player };
		m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
		
		
		m_shootLT->enabled = false;
		if( m_shootTimeout > 0 )
		{
			m_shootTimeout -= deltaTime;
			m_shootLT->enabled = true;
		}
		if( SHOOT.value && m_shootTimeout <= 0 )
		{
			Mat4 mtx_l = m_animChar.GetAttachmentMatrix( m_animChar.FindAttachment( "gun_l" ) );
			Mat4 mtx_r = m_animChar.GetAttachmentMatrix( m_animChar.FindAttachment( "gun_r" ) );
			Vec3 origin_l = mtx_l.TransformPos( V3(0) );
			Vec3 origin_r = mtx_r.TransformPos( V3(0) );
			{
				Vec3 dir = ( i_aim_target - origin_l ).Normalized();
				dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
			//	printf("%f;%f;%f\n",dir.x,dir.y,dir.z);
				m_level->GetSystem<BulletSystem>()->Add( origin_l, dir * 100, 1, 1, m_meshInstInfo.ownerType );
				m_shootPS.SetTransform( mtx_l );
				m_shootPS.Trigger();
			}
			{
				Vec3 dir = ( i_aim_target - origin_r ).Normalized();
				dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
				m_level->GetSystem<BulletSystem>()->Add( origin_r, dir * 100, 1, 1, m_meshInstInfo.ownerType );
				m_shootPS.SetTransform( mtx_r );
				m_shootPS.Trigger();
			}
			m_shootLT->position = ( origin_l + origin_r ) * 0.5f;
			m_shootLT->UpdateTransform();
			m_shootLT->enabled = true;
			m_shootTimeout += 0.1f;
			m_level->GetSystem<AIDBSystem>()->AddSound( GetPosition(), 10, 0.2f, AIS_Shot );
		}
		m_shootLT->color = V3(0.9f,0.7f,0.5f) * smoothlerp_oneway( m_shootTimeout, 0, 0.1f );
		
		
		MapItemInfo mymapitem = { MI_Object_Player, GetPosition(), V3(0), 0, 0 };
		m_level->GetSystem<LevelMapSystem>()->UpdateItem( this, mymapitem );
		m_level->GetSystem<LevelMapSystem>()->m_viewPos = GetPosition().ToVec2();
		
		
		m_shootPS.Tick( deltaTime );
		m_shootPS.PreRender();
	}
	
	void DrawUI()
	{
		SGRX_FontSettings fs;
		GR2D_GetFontSettings( &fs );
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
		
		Vec3 QP = GetQueryPosition();
		IESItemGather ies_gather;
		m_level->GetSystem<InfoEmissionSystem>()->QuerySphereAll( &ies_gather, QP, 5, IEST_InteractiveItem );
		if( ies_gather.items.size() )
		{
			ies_gather.DistanceSort( QP );
			for( size_t i = ies_gather.items.size(); i > 0; )
			{
				i--;
				Entity* E = ies_gather.items[ i ].E;
				Vec3 pos = ies_gather.items[ i ].D.pos;
				bool infront;
				Vec2 screenpos = m_level->GetScene()->camera.WorldToScreen( pos, &infront ).ToVec2() * screen_size;
				if( infront )
				{
					float dst = ( QP - pos ).Length();
					bool activate = i == 0 && dst < 2;
					Vec2 dir = V2( 2, -1 ).Normalized();
					Vec2 clp0 = screenpos + dir * 12;
					Vec2 clp1 = screenpos + dir * 64;
					Vec2 cline[2] = { clp0, clp1 };
					Vec2 addX = V2( 0, -48 ), addY = V2( 120, 0 );
					Vec2 irect[4] = { clp1, clp1 + addY, clp1 + addX + addY, clp1 + addX };
					float a = smoothlerp_oneway( dst, 5.0f, 4.0f );
					
					br.Reset();
					if( activate )
					{
						br.Col( 0.9f, 0.1f, 0, 0.5f * a ).CircleFill( screenpos.x, screenpos.y, 12 );
					}
					br.Col( 0, 0.5f * a ).QuadWH( clp1.x, clp1.y, 120, -48 );
					br.Col( 0.905f, 1 * a ).AACircleOutline( screenpos.x, screenpos.y, 12, 2 );
					br.AAStroke( cline, 2, 2, false );
					br.AAStroke( irect, 4, 2, true );
					
					GR2D_SetFont( "mono", 15 );
					GR2D_DrawTextRect(
						round( clp1.x + 4 ), round( clp1.y - 48 + 4 ),
						round( clp1.x + 120 - 4 ), round( clp1.y - 4 ),
						E->m_viewName, HALIGN_LEFT, VALIGN_TOP );
				}
			}
		}
		
	//	if( m_targetII )
	//	{
	//		float x = GR_GetWidth() / 2.0f;
	//		float y = GR_GetHeight() / 2.0f;
	//		br.Reset().SetTexture( m_tex_interact_icon ).QuadWH( x, y, bsz / 10, bsz / 10 );
	//	}
		
		m_aimHelper.DrawUI();
		
		GR2D_SetFontSettings( &fs );
	}
};


struct ISR3Enemy : ISR3Drone
{
	SGS_OBJECT_INHERIT( ISR3Drone );
	ENT_SGS_IMPLEMENT;
	
	Vec2 i_turn;
	
	SGS_PROPERTY_FUNC( READ VARNAME state ) sgsVariable m_enemyState;
	TSFactStorage m_factStorage;
	AIDBSystem* m_aidb;
	sgs_VarObj* m_scrObj;
	
	ISR3Enemy( GameLevel* lev, const StringView& name, const Vec3& pos, const Vec3& dir, sgsVariable args );
	~ISR3Enemy();
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	bool GetMapItemInfo( MapItemInfo* out );
	void DebugDrawWorld();
	void DebugDrawUI();
	
	bool HasFact( int typemask ){ return m_factStorage.HasFact( typemask ); }
	bool HasRecentFact( int typemask, TimeVal maxtime ){ return m_factStorage.HasRecentFact( typemask, maxtime ); }
	TSFactStorage::Fact* GetRecentFact( int typemask, TimeVal maxtime ){ return m_factStorage.GetRecentFact( typemask, maxtime ); }
};



struct ISR3EntityCreationSystem : IGameLevelSystem
{
	enum { e_system_uid = 1000 };
	ISR3EntityCreationSystem( GameLevel* lev );
	virtual bool AddEntity( const StringView& type, sgsVariable data );
	virtual void DrawUI();
};


ISR3EntityCreationSystem::ISR3EntityCreationSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
}

bool ISR3EntityCreationSystem::AddEntity( const StringView& type, sgsVariable data )
{
	///////////////////////////
	if( type == "player" )
	{
		ISR3Player* P = new ISR3Player
		(
			m_level,
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>()
		);
		m_level->AddEntity( P );
		m_level->SetPlayer( P );
		return true;
	}
	
	#if 0
	///////////////////////////
	if( type == "enemy_start" )
	{
		StringView type = data.getprop("type").get<StringView>();
		if( type == "eye" )
		{
			ISR3_BossEye* E = new ISR3_BossEye
			(
				m_level,
				data.getprop("name").get<StringView>(),
				data.getprop("position").get<Vec3>(),
				data.getprop("viewdir").get<Vec3>()
			);
			m_level->AddEntity( E );
			return true;
		}
		if( type == "robosaw" )
		{
			ISR3_RoboSaw* E = new ISR3_RoboSaw
			(
				m_level,
				data.getprop("name").get<StringView>(),
				data.getprop("position").get<Vec3>(),
				data.getprop("viewdir").get<Vec3>()
			);
			m_level->AddEntity( E );
			return true;
		}
	}
	#endif
	return false;
}

void ISR3EntityCreationSystem::DrawUI()
{
	SGRX_CAST( ISR3Player*, P, m_level->m_player );
	if( P )
		P->DrawUI();
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
		g_GameLevel->SetGlobalToSelf();
		g_GameLevel->GetPhyWorld()->SetGravity( V3( 0, 0, -9.81f ) );
		AddSystemToLevel<InfoEmissionSystem>( g_GameLevel );
		AddSystemToLevel<MessagingSystem>( g_GameLevel );
		AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
		AddSystemToLevel<HelpTextSystem>( g_GameLevel );
		AddSystemToLevel<FlareSystem>( g_GameLevel );
		AddSystemToLevel<LevelCoreSystem>( g_GameLevel );
		AddSystemToLevel<LevelMapSystem>( g_GameLevel );
		AddSystemToLevel<ScriptedSequenceSystem>( g_GameLevel );
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
		htr.fontSize = GR_GetHeight() / 20;
		htr.SetNamedFont( "", "core" );
		
		GR2D_SetFont( "core", TMIN(GR_GetWidth(),GR_GetHeight())/20 );
		g_GameLevel->Load( "level1" );
		g_GameLevel->Tick( 0, 0 );
		
		Game_ShowCursor( false );
		
		g_GameLevel->StartLevel();
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

