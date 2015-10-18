

#include "isr3entities.hpp"



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


