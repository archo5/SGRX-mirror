

#include "level.hpp"
#include "systems.hpp"
#include "entities.hpp"
#include "entities_ts.hpp"


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



struct ISR3Drone : Entity, SGRX_MeshInstUserData
{
	struct ActionState
	{
		ActionState() : timeoutMoveToStart(0), timeoutEnding(0),
			progress(0), target(NULL){}
		
		float timeoutMoveToStart;
		float timeoutEnding;
		float progress;
		Entity* target;
		InteractInfo info;
	};
	
	ISR3Drone( GameLevel* lev, Vec3 pos, Vec3 dir );
	
	void TurnTo( const Vec2& turnDir, float speedDelta );
	Vec3 GetAimDir();
	void BeginClosestAction( float maxdist );
	bool BeginAction( Entity* E );
	bool IsInAction();
	bool CanInterruptAction();
	void InterruptAction( bool force );
	
	void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	void Hit( float pwr );
	
	void FixedTick( float deltaTime );
	void Tick( float deltaTime, float blendFactor );
	void DrawUI();
	
	Vec3 GetPosition(){ return m_body->GetPosition(); }
	Vec3 GetQueryPosition(){ return GetPosition() + V3(0,0,0.5f); }
	
	PhyRigidBodyHandle m_body;
	PhyShapeHandle m_shapeHandle;
	LightHandle m_shadowInst;
	
	AnimCharacter m_animChar;
	SGRX_MeshInstUserData m_meshInstInfo;
	ActionState m_actState;
	
	IVState< Vec3 > m_ivPos;
	IVState< Quat > m_ivRot;
	float m_turnAngle;
	
	HashTable< String, int > m_items;

	ParticleSystem m_shootPS;
	LightHandle m_shootLT;
	float m_shootTimeout;
	
	float m_health;
	float m_hitTimeout;
	
	Vec2 i_move;
	float i_speed;
	bool i_aim_at;
	Vec3 i_aim_target;
};

ISR3Drone::ISR3Drone( GameLevel* lev, Vec3 pos, Vec3 dir ) :
	Entity( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() )
{
	i_move = V2(0);
	i_speed = 10;
	i_aim_at = false;
	i_aim_target = V3(0);
	
	m_health = 20;
	m_hitTimeout = 0;
	m_turnAngle = atan2( dir.y, dir.x );
	m_ivPos = pos;
	m_ivRot = Quat::Identity;
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->userData = &m_meshInstInfo;
	MI->layers = 0x2;
	
	m_shadowInst = m_level->GetScene()->CreateLight();
	m_shadowInst->type = LIGHT_PROJ;
	m_shadowInst->direction = V3(0,0,-1);
	m_shadowInst->updir = V3(0,1,0);
	m_shadowInst->angle = 45;
	m_shadowInst->range = 3.5f;
	m_shadowInst->UpdateTransform();
	m_shadowInst->projectionMaterial.textures[0] = GR_GetTexture( "textures/fx/blobshadow.png" );//GR_GetTexture( "textures/unit.png" );
	m_shadowInst->projectionMaterial.textures[1] = GR_GetTexture( "textures/fx/projfalloff2.png" );
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 1.0f;
	rbinfo.restitution = 0.1f;
	rbinfo.shape = lev->GetPhyWorld()->CreateSphereShape( 0.5f );
	rbinfo.shape->SetScale( V3(0.7f,0.7f,0.32f) );
	rbinfo.mass = 5;
	rbinfo.angularDamping = 0.5f;
	rbinfo.inertia = rbinfo.shape->CalcInertia( rbinfo.mass ) * 0.1f;
	rbinfo.position = pos;
	rbinfo.rotation = Quat::CreateAxisAngle( V3(0,0,1), atan2(dir.y,dir.x) );
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	m_body = lev->GetPhyWorld()->CreateRigidBody( rbinfo );
}

void ISR3Drone::TurnTo( const Vec2& turnDir, float speedDelta )
{
	float angend = normalize_angle( turnDir.Angle() );
	float angstart = normalize_angle( m_turnAngle );
	if( fabs( angend - angstart ) > M_PI )
		angstart += angend > angstart ? M_PI * 2 : -M_PI * 2;
//	printf( "cur: %1.2f, target: %1.2f\n", angstart, angend);
	m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), speedDelta );
}

Vec3 ISR3Drone::GetAimDir()
{
	Vec3 aimdir = V3( cosf( m_turnAngle ), sinf( m_turnAngle ), 0 );
	if( i_aim_at )
	{
		aimdir = ( i_aim_target - GetPosition() );
	}
	return aimdir;
}

void ISR3Drone::BeginClosestAction( float maxdist )
{
	if( IsInAction() )
		return;
	
	Vec3 QP = GetQueryPosition();
	IESItemGather ies_gather;
	m_level->GetSystem<InfoEmissionSystem>()->QuerySphereAll( &ies_gather, QP, 5, IEST_InteractiveItem );
	if( ies_gather.items.size() )
	{
		ies_gather.DistanceSort( QP );
		if( ( ies_gather.items[ 0 ].D.pos - QP ).Length() < maxdist )
			BeginAction( ies_gather.items[ 0 ].E );
	}
}

bool ISR3Drone::BeginAction( Entity* E )
{
	if( !E || IsInAction() )
		return false;
	
	IInteractiveEntity* IE = E->GetInterface<IInteractiveEntity>();
	if( IE == NULL || IE->GetInteractionInfo( GetQueryPosition(), &m_actState.info ) == false )
		return false;
	
	m_actState.timeoutMoveToStart = 1;
	m_actState.progress = 0;
	m_actState.target = E;
	return true;
}

bool ISR3Drone::IsInAction()
{
	return m_actState.target != NULL;
}

bool ISR3Drone::CanInterruptAction()
{
	if( IsInAction() == false )
		return false;
	
	IInteractiveEntity* IE = m_actState.target->GetInterface<IInteractiveEntity>();
	return IE && IE->CanInterruptAction( m_actState.progress );
}

void ISR3Drone::InterruptAction( bool force )
{
	if( force == false && CanInterruptAction() == false )
		return;
	
	m_actState.progress = 0;
	m_actState.target = NULL;
}

void ISR3Drone::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		m_body->ApplyForce( PFT_Impulse, bhinfo->vel * 0.1f, bhinfo->pos );
		Hit( bhinfo->vel.Length() * 0.1f );
	}
}

void ISR3Drone::Hit( float pwr )
{
	if( m_health > 0 )
	{
		m_hitTimeout = 0.1f;
		m_health -= pwr;
		if( m_health <= 0 )
		{
			FlareSystem* FS = m_level->GetSystem<FlareSystem>();
			FS->RemoveFlare( this );
		}
	}
}

void ISR3Drone::FixedTick( float deltaTime )
{
	if( IsInAction() )
	{
		i_move = V2(0);
		if( m_actState.timeoutMoveToStart > 0 )
		{
			if( ( m_actState.info.placePos - GetPosition() ).ToVec2().Length() < 0.1f )
			{
				m_actState.timeoutMoveToStart = 0;
			}
			else
			{
				i_move = ( m_actState.info.placePos - GetPosition() ).ToVec2();
				m_actState.timeoutMoveToStart -= deltaTime;
			}
		}
		else if( m_actState.progress < m_actState.info.timeActual )
		{
			float pp = m_actState.progress;
			float cp = pp + deltaTime;
			
			// <<< TODO EVENTS >>>
			if( pp < 0.01f && 0.01f <= cp )
			{
				m_actState.target->OnEvent( "action_start" );
			}
		//	if( pp < 0.5f && 0.5f <= cp )
		//	{
		//	}
			
			m_actState.progress = cp;
			if( m_actState.progress >= m_actState.info.timeActual )
			{
				// end of action
				m_actState.target->OnEvent( "action_end" );
				m_actState.timeoutEnding = IA_NEEDS_LONG_END( m_actState.info.type ) ? 1 : 0;
			}
		}
		else
		{
			m_actState.timeoutEnding -= deltaTime;
			if( m_actState.timeoutEnding <= 0 )
			{
				// end of action ending
				InterruptAction( true );
			}
		}
	}
	
	Vec3 pos = m_body->GetPosition();
	
	Vec2 md = i_move * i_speed;
	Vec3 force = V3( md.x, md.y, 0 );
	{
		Vec3 gnd = pos + V3(0,0,-100);
		SGRX_PhyRaycastInfo info;
		if( m_level->GetPhyWorld()->Raycast( pos, gnd, 0x0001, 0xffff, &info ) )
		{
			gnd = TLERP( pos, gnd, info.factor );
		}
		Vec3 tgt = gnd + V3(0,0,1.5f);
		float len = ( tgt - pos ).Length();
		if( tgt.z > pos.z )
			force += ( tgt - pos ).Normalized() * TMIN( len, 1.0f ) * 50 - m_body->GetLinearVelocity();
	}
	
	if( m_health > 0 )
	{
		Vec2 aimdir = GetAimDir().ToVec2().Perp();
		Quat qdst = Quat::CreateAxisAngle( V3(0,0,1), atan2( aimdir.y, aimdir.x ) );
		Vec3 angvel = m_body->GetAngularVelocity() * 0.5f
			+ 100 * deltaTime * PHY_QuaternionToEulerXYZ( qdst * m_body->GetRotation().Inverted() );
		
		m_body->ApplyCentralForce( PFT_Velocity, force * deltaTime );
		m_body->SetAngularVelocity( angvel );
	}
	
	m_ivPos.Advance( pos );
	m_ivRot.Advance( m_body->GetRotation() );
}

void ISR3Drone::Tick( float deltaTime, float blendFactor )
{
	m_hitTimeout = TMAX( 0.0f, m_hitTimeout - deltaTime );
	
	Vec4 col = V4(0);
	if( m_hitTimeout > 0 )
		col = V4( 1, 1, 1, 0.5f );
	
	FlareSystem* FS = m_level->GetSystem<FlareSystem>();
	Mat4 mtx =
		Mat4::CreateScale( V3(0.5f) ) *
		Mat4::CreateRotationFromQuat( m_ivRot.Get( blendFactor ) ) *
		Mat4::CreateTranslation( m_ivPos.Get( blendFactor ) );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->matrix = mtx;
	m_shadowInst->position = pos + V3(0,0,1);
	
	m_level->LightMesh( MI );
	
	m_animChar.PreRender( blendFactor );
	if( m_health > 0 )
	{
		Vec3 vpos = m_animChar.GetAttachmentPos( m_animChar.FindAttachment( "camera" ) );
		FSFlare statusFlare = { vpos, V3(2.0f,0.05f,0.01f), 0.1f, true };
		FS->UpdateFlare( this, statusFlare );
	}
}


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
					GR2D_DrawTextLine( round( clp1.x + 4 ), round( clp1.y - 48 + 4 ), E->m_viewName );
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
	//	AddSystemToLevel<MessagingSystem>( g_GameLevel );
	//	AddSystemToLevel<ObjectiveSystem>( g_GameLevel );
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

