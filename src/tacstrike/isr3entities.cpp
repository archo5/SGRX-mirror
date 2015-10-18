

#include "isr3entities.hpp"


extern Command MOVE_LEFT;
extern Command MOVE_RIGHT;
extern Command MOVE_UP;
extern Command MOVE_DOWN;
extern Command MOVE_X;
extern Command MOVE_Y;
extern Command AIM_X;
extern Command AIM_Y;
extern Command DO_ACTION;
extern Command SHOOT;
extern Command LOCK_ON;
extern Vec2 CURSOR_POS;


ISR3Drone::ISR3Drone( GameLevel* lev, Vec3 pos, Vec3 dir ) :
	Entity( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() )
{
	i_move = V2(0);
	i_speed = 10;
	i_aim_at = false;
	i_aim_target = V3(0);
	
	m_health = 100;
	m_hitTimeout = 0;
	m_turnAngle = atan2( dir.y, dir.x );
	m_ivPos = pos;
	m_ivRot = Quat::Identity;
	m_interpPos = V3(0);
	m_interpAimDir = V3(0);
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->userData = (SGRX_MeshInstUserData*) this;
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
			m_level->GetSystem<FlareSystem>()->RemoveFlare( this );
			m_level->GetSystem<InfoEmissionSystem>()->RemoveEmitter( this );
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
				m_actState.timeoutEnding = 0;//IA_NEEDS_LONG_END( m_actState.info.type ) ? 1 : 0;
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
	
	Vec2 md = i_move.Normalized() * clamp( i_move.Length(), 0, 1 ) * i_speed;
//	float pwr = TMAX( 0.0f, Vec2Dot( md.Normalized(), m_body->GetLinearVelocity().ToVec2() ) );
//	Vec3 force = V3( md.x, md.y, 0 ) / ( 0.1f + pwr * 0.5f );
	Vec3 force = ( V3( md.x, md.y, 0 ) - m_body->GetLinearVelocity() ) * 5.0f;
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
	MI->constants[ 0 ] = col;
	m_shadowInst->position = pos + V3(0,0,1);
	
	m_level->LightMesh( MI );
	
	m_animChar.PreRender( blendFactor );
	m_interpPos = m_ivPos.Get( blendFactor );
	m_interpAimDir = Mat4::CreateRotationFromQuat( m_ivRot.Get( blendFactor ) ).TransformNormal( V3(0,1,0) );
	if( m_health > 0 )
	{
		Vec3 vpos = m_animChar.GetAttachmentPos( m_animChar.FindAttachment( "camera" ) );
		FSFlare statusFlare = { vpos, V3(2.0f,0.05f,0.01f), 0.1f, true };
		FS->UpdateFlare( this, statusFlare );
	}
}


ISR3Player::ISR3Player( GameLevel* lev, Vec3 pos, Vec3 dir )
	: ISR3Drone( lev, pos, dir ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false ),
	m_aimHelper( lev )
{
	m_animChar.Load( "chars/player.chr" );
	ownerType = GAT_Player;
	
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

void ISR3Player::FixedTick( float deltaTime )
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

void ISR3Player::Tick( float deltaTime, float blendFactor )
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
			m_level->GetSystem<BulletSystem>()->Add( origin_l, dir * 100, 1, 1, ownerType );
			m_shootPS.SetTransform( mtx_l );
			m_shootPS.Trigger();
		}
		{
			Vec3 dir = ( i_aim_target - origin_r ).Normalized();
			dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
			m_level->GetSystem<BulletSystem>()->Add( origin_r, dir * 100, 1, 1, ownerType );
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

void ISR3Player::DrawUI()
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




extern sgs_ObjInterface ISR3Enemy_iface[1];

#define ISR3ENEMY_VALID if( E == NULL ){ return sgs_Msg( C, SGS_WARNING, "enemy no longer exists" ); }

static int ISR3Enemy_HasFact( SGS_CTX )
{
	ISR3Enemy* E;
	SGS_PARSE_METHOD( C, ISR3Enemy_iface, E, ISR3Enemy, HasFact );
	ISR3ENEMY_VALID;
	sgs_PushBool( C, E->HasFact( sgs_GetVar<int>()( C, 0 ) ) );
	return 1;
}

static int ISR3Enemy_HasRecentFact( SGS_CTX )
{
	ISR3Enemy* E;
	SGS_PARSE_METHOD( C, ISR3Enemy_iface, E, ISR3Enemy, HasRecentFact );
	ISR3ENEMY_VALID;
	sgs_PushBool( C, E->HasRecentFact( sgs_GetVar<int>()( C, 0 ), sgs_GetVar<int>()( C, 1 ) ) );
	return 1;
}

static int ISR3Enemy_GetRecentFact( SGS_CTX )
{
	ISR3Enemy* E;
	SGS_PARSE_METHOD( C, ISR3Enemy_iface, E, ISR3Enemy, GetRecentFact );
	ISR3ENEMY_VALID;
	TSFactStorage::Fact* F = E->GetRecentFact( sgs_GetVar<int>()( C, 0 ), sgs_GetVar<int>()( C, 1 ) );
	if( F )
	{
		sgs_PushString( C, "id" ); sgs_PushInt( C, F->id );
		sgs_PushString( C, "ref" ); sgs_PushInt( C, F->ref );
		sgs_PushString( C, "type" ); sgs_PushInt( C, F->type );
		sgs_PushString( C, "position" ); sgs_PushVec3p( C, &F->position.x );
		sgs_PushString( C, "created" ); sgs_PushInt( C, F->created );
		sgs_PushString( C, "expires" ); sgs_PushInt( C, F->expires );
		sgs_PushDict( C, 12 );
		return 1;
	}
	return 0;
}

static int ISR3Enemy_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "HasFact" ) return sgs_PushCFunction( C, ISR3Enemy_HasFact );
		SGS_CASE( "HasRecentFact" ) return sgs_PushCFunction( C, ISR3Enemy_HasRecentFact );
		SGS_CASE( "GetRecentFact" ) return sgs_PushCFunction( C, ISR3Enemy_GetRecentFact );
	SGS_END_INDEXFUNC
}

sgs_ObjInterface ISR3Enemy_iface[1] =
{{
	"ISR3Enemy",
	NULL, NULL,
	ISR3Enemy_getindex, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL
}};


ISR3Enemy::ISR3Enemy( GameLevel* lev, const StringView& name, const Vec3& pos, const Vec3& dir, sgsVariable args ) :
	ISR3Drone( lev, pos, dir ),
	i_turn( V2(0) )
{
	m_typeName = "enemy";
	m_name = name;
	
	m_aidb = m_level->GetSystem<AIDBSystem>();
	ownerType = GAT_Enemy;
	
	// create self
	{
		sgs_Variable var;
		sgs_InitObject( m_level->m_scriptCtx.C, &var, this, ISR3Enemy_iface );
		m_scrObj = sgs_GetObjectStructP( &var );
	}
	
	// create ESO (enemy scripted object)
	{
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		sgs_PushObjectPtr( m_level->m_scriptCtx.C, m_scrObj );
		m_level->m_scriptCtx.Push( args );
		m_level->m_scriptCtx.Push( GetPosition() );
		m_level->m_scriptCtx.Push( GetAimDir() );
		if( m_level->m_scriptCtx.GlobalCall( "ISR3Enemy_Create", 4, 1 ) == false )
		{
			LOG_ERROR << "FAILED to create enemy state";
		}
		m_enemyState = sgsVariable( m_level->m_scriptCtx.C, -1 );
	}
	
	StringView charpath = m_enemyState.getprop("charpath").get<StringView>();
	m_animChar.Load( charpath ? charpath : "chars/enemy.chr" );
	
	m_level->MapEntityByName( this );
}

ISR3Enemy::~ISR3Enemy()
{
	// destroy ESO
	{
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		m_enemyState.thiscall( "destroy" );
	}
	
	// destroy self
	m_scrObj->data = NULL;
	sgs_ObjRelease( m_level->m_scriptCtx.C, m_scrObj );
}

struct IESEnemyViewProc : InfoEmissionSystem::IESProcessor
{
	bool Process( Entity* ent, const InfoEmissionSystem::Data& data )
	{
		Vec3 enemypos = data.pos + V3(0,0,1); // FIXME MAYBE?
		
		// verify the find
		Vec3 vieworigin = enemy->GetPosition();
		Vec3 viewdir = enemy->GetAimDir();
		Vec3 view2pos = enemypos - vieworigin;
		float vpdot = Vec3Dot( viewdir.Normalized(), view2pos.Normalized() );
		if( vpdot < cosf(DEG2RAD(40.0f)) )
			return true; // outside view cone
		
		if( ent->m_level->GetPhyWorld()->Raycast( vieworigin, enemypos, 1, 1 ) )
			return true; // behind wall
		
		// TODO friendlies
		TSFactStorage& FS = enemy->m_factStorage;
		
		if( data.types & IEST_AIAlert )
		{
			FS.InsertOrUpdate( TSFactStorage::FT_Sight_Alarming,
				enemypos, 0, curtime, curtime + 5*1000, 0 );
		}
		else
		{
			// fact of seeing
			FS.MovingInsertOrUpdate( TSFactStorage::FT_Sight_Foe,
				enemypos, 10, curtime, curtime + 5*1000 );
			
			// fact of position
			FS.MovingInsertOrUpdate( TSFactStorage::FT_Position_Foe,
				enemypos, 10, curtime, curtime + 30*1000, FS.last_mod_id );
		}
		
		return true;
	}
	
	TimeVal curtime;
	ISR3Enemy* enemy;
};

void ISR3Enemy::FixedTick( float deltaTime )
{
	TimeVal curTime = m_level->GetPhyTime();
	
	// process facts
	m_factStorage.Process( curTime );
	// - vision
	IESEnemyViewProc evp;
	evp.curtime = curTime;
	evp.enemy = this;
	m_level->GetSystem<InfoEmissionSystem>()->QuerySphereAll( &evp, GetPosition(), 10.0f, IEST_Player | IEST_AIAlert );
	// - sounds
	for( int i = 0; i < m_aidb->GetNumSounds(); ++i )
	{
		if( m_aidb->CanHearSound( GetPosition(), i ) == false )
			continue;
		AISound S = m_aidb->GetSoundInfo( i );
		
		if( S.type == AIS_Footstep || S.type == AIS_Shot )
		{
			TSFactStorage::FactType sndtype = TSFactStorage::FT_Sound_Footstep;
			if( S.type == AIS_Shot )
				sndtype = TSFactStorage::FT_Sound_Shot;
			
			m_factStorage.InsertOrUpdate( sndtype,
				S.position, SMALL_FLOAT, curTime, curTime + 1*1000, 0, false );
			
			int lastid = m_factStorage.last_mod_id;
			bool found_friend = m_factStorage.MovingUpdate( TSFactStorage::FT_Position_Friend,
				S.position, 10, curTime, curTime + 30*1000, lastid );
			if( found_friend == false )
			{
				m_factStorage.MovingUpdate( TSFactStorage::FT_Position_Foe,
					S.position, 10, curTime, curTime + 30*1000, lastid );
			}
		}
		else
		{
			m_factStorage.InsertOrUpdate( TSFactStorage::FT_Sound_Noise,
				S.position, 1, curTime, curTime + 1*1000 );
		}
	}
	
	// tick ESO
	{
		m_level->m_scriptCtx.Push( GetPosition() );
		m_enemyState.setprop( "position", sgsVariable( m_level->m_scriptCtx.C, sgsVariable::PickAndPop ) );
		m_level->m_scriptCtx.Push( GetAimDir() );
		m_enemyState.setprop( "viewdir", sgsVariable( m_level->m_scriptCtx.C, sgsVariable::PickAndPop ) );
		
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		m_level->m_scriptCtx.Push( deltaTime );
		m_enemyState.thiscall( "tick", 1 );
		
		i_move = m_enemyState[ "i_move" ].get<Vec2>();
		i_speed = m_enemyState[ "i_speed" ].get<float>();
		i_turn = m_enemyState[ "i_turn" ].get<Vec2>();
	}
	
	if( i_turn.Length() > 0.1f )
	{
		TurnTo( i_turn, deltaTime * 8 );
	}
	
	ISR3Drone::FixedTick( deltaTime );
	
	if( m_health > 0 )
	{
		Vec3 tgtpos = m_animChar.GetAttachmentPos( m_animChar.FindAttachment( "target" ) );
		InfoEmissionSystem::Data D = { tgtpos, 0.5f, IEST_MapItem | IEST_HeatSource | IEST_Target };
		m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
	}
}

void ISR3Enemy::Tick( float deltaTime, float blendFactor )
{
	ISR3Drone::Tick( deltaTime, blendFactor );
}

bool ISR3Enemy::GetMapItemInfo( MapItemInfo* out )
{
	out->type = MI_Object_Enemy | MI_State_Normal;
	out->position = GetInterpPos();
	out->direction = GetInterpAimDir();
	out->sizeFwd = 10;
	out->sizeRight = 8;
	return true;
}

void ISR3Enemy::DebugDrawWorld()
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().Col( 0.9f, 0.2f, 0.1f );
	Vec3 pos = GetPosition();
	
	m_factStorage.SortCreatedDesc();
	
	size_t count = TMIN( size_t(10), m_factStorage.facts.size() );
	for( size_t i = 0; i < count; ++i )
	{
		TSFactStorage::Fact& F = m_factStorage.facts[ i ];
		br.SetPrimitiveType( PT_Lines );
		br.Pos( pos ).Pos( F.position );
		br.Tick( F.position, 0.1f );
	}
}

void ISR3Enemy::DebugDrawUI()
{
	char bfr[ 256 ];
	BatchRenderer& br = GR2D_GetBatchRenderer();
	Vec3 pos = GetPosition();
	bool infront;
	Vec3 screenpos = m_level->GetScene()->camera.WorldToScreen( pos, &infront );
	if( !infront )
		return;
	int x = screenpos.x * GR_GetWidth();
	int y = screenpos.y * GR_GetHeight();
	
	GR2D_SetFont( "core", 12 );
	GR2D_SetFont( "mono", 12 );
	
	size_t count = TMIN( size_t(10), m_factStorage.facts.size() );
	sgrx_snprintf( bfr, 256, "count: %d, mod id: %d, next: %d",
		int(m_factStorage.facts.size()), int(m_factStorage.last_mod_id),
		int(m_factStorage.m_next_fact_id) );
	
	int len = GR2D_GetTextLength( bfr );
	br.Reset().Col( 0.0f, 0.5f ).Quad( x, y, x + len, y + 12 );
	br.Col( 1.0f );
	GR2D_DrawTextLine( x, y, bfr );
	
	y += 13;
	
	for( size_t i = 0; i < count; ++i )
	{
		TSFactStorage::Fact& F = m_factStorage.facts[ i ];
		const char* type = "type?";
		switch( F.type )
		{
		case TSFactStorage::FT_Unknown: type = "unknown"; break;
		case TSFactStorage::FT_Sound_Noise: type = "snd-noise"; break;
		case TSFactStorage::FT_Sound_Footstep: type = "snd-step"; break;
		case TSFactStorage::FT_Sound_Shot: type = "snd-shot"; break;
		case TSFactStorage::FT_Sight_ObjectState: type = "sight-state"; break;
		case TSFactStorage::FT_Sight_Alarming: type = "sight-alarm"; break;
		case TSFactStorage::FT_Sight_Friend: type = "sight-friend"; break;
		case TSFactStorage::FT_Sight_Foe: type = "sight-foe"; break;
		case TSFactStorage::FT_Position_Friend: type = "pos-friend"; break;
		case TSFactStorage::FT_Position_Foe: type = "pos-foe"; break;
		}
		
		sgrx_snprintf( bfr, 256, "Fact #%d (ref=%d) <%s> @ %.4g;%.4g;%.4g cr: %d, exp: %d",
			int(F.id), int(F.ref), type, F.position.x, F.position.y, F.position.z,
			int(F.created), int(F.expires) );
		
		int len = GR2D_GetTextLength( bfr );
		br.Reset().Col( 0.0f, 0.5f ).Quad( x, y, x + len, y + 12 );
		br.Col( 1.0f );
		GR2D_DrawTextLine( x, y, bfr );
		
		y += 13;
	}
}



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
	
	///////////////////////////
	if( type == "enemy_start" )
	{
		ISR3Enemy* E = new ISR3Enemy
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>(),
			data
		);
		m_level->AddEntity( E );
		return true;
	}
	return false;
}

void ISR3EntityCreationSystem::DrawUI()
{
	SGRX_CAST( ISR3Player*, P, m_level->m_player );
	if( P )
		P->DrawUI();
}


