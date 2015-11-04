

#include "entities_ts.hpp"


extern Vec2 CURSOR_POS;
extern Command MOVE_LEFT;
extern Command MOVE_RIGHT;
extern Command MOVE_UP;
extern Command MOVE_DOWN;
extern Command MOVE_X;
extern Command MOVE_Y;
extern Command AIM_X;
extern Command AIM_Y;
extern Command SHOOT;
extern Command LOCK_ON;
extern Command RELOAD;
extern Command CROUCH;
extern Command DO_ACTION;


#define MAGNUM_CAMERA_NOTICE_TIME 2
#define MAGNUM_CAMERA_ALERT_TIME 5
#define MAGNUM_CAMERA_VIEW_DIST 6.0f


TSCamera::TSCamera(
	GameLevel* lev,
	const StringView& name,
	const StringView& charname,
	const Vec3& pos,
	const Quat& rot,
	const Vec3& scl,
	const Vec3& dir0,
	const Vec3& dir1
) :
	Entity( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() ),
	m_playerVisible( false ), m_lastSeenPlayerDir( YP(0) ),
	m_curDir( YP( dir0 ) ), m_timeout( 0 ), m_state( 0 ),
	m_alertTimeout( 0 ), m_noticeTimeout( 0 ),
	m_dir0( YP( dir0 ) ), m_dir1( YP( dir1 ) ),
	m_moveTime( 3.0f ), m_pauseTime( 2.0f ),
	m_fov( 30.0f )
{
	m_typeName = "camera";
	m_name = name;
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "chars/%s.chr", StackString<200>( charname ).str );
	
	m_anLayers[0].anim = &m_animChar.m_layerAnimator;
	m_anLayers[0].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
	m_animChar.m_anMixer.layers = m_anLayers;
	m_animChar.m_anMixer.layerCount = 1;
	m_animChar.Load( bfr );
	StringView atchlist[] = { "view", "origin", "light" };
	m_animChar.SortEnsureAttachments( atchlist, 3 );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->layers = 0x2;
	MI->matrix = Mat4::CreateSRT( scl, rot, pos );
	m_level->LightMesh( MI );
	
	m_level->MapEntityByName( this );
}

void TSCamera::FixedTick( float deltaTime )
{
	m_playerVisible = false;
	if( m_level->m_player )
	{
		Vec3 ppos = V3(0);
#ifdef TSGAME
		ppos = static_cast<TSCharacter*>(m_level->m_player)->GetPosition();
#endif
		Mat4 viewmtx, originmtx, invviewmtx = Mat4::Identity, invoriginmtx = Mat4::Identity;
		m_animChar.GetAttachmentMatrix( 0, viewmtx );
		m_animChar.GetAttachmentMatrix( 1, originmtx );
		viewmtx.InvertTo( invviewmtx );
		originmtx.InvertTo( invoriginmtx );
		
		Vec3 viewpos = invviewmtx.TransformPos( ppos );
		Vec3 viewdir = viewpos.Normalized();
		float viewangle = acosf( clamp( Vec3Dot( viewdir, V3(1,0,0) ), -1, 1 ) );
		if( viewpos.Length() < MAGNUM_CAMERA_VIEW_DIST && viewangle < DEG2RAD( m_fov ) )
		{
			Vec3 viewpos = viewmtx.TransformPos( V3(0) );
			bool isect = m_level->GetPhyWorld()->Raycast( viewpos, ppos, 1, 1 );
			if( isect == false )
			{
				Vec3 origindir = invoriginmtx.TransformPos( ppos ).Normalized();
				m_playerVisible = true;
				m_lastSeenPlayerDir = YP( origindir );
			}
		}
	}
	
	if( m_alertTimeout > 0 )
	{
		if( m_playerVisible )
			m_alertTimeout = MAGNUM_CAMERA_ALERT_TIME;
		else
			m_alertTimeout -= deltaTime;
		m_curDir.TurnTo( m_lastSeenPlayerDir, YP(deltaTime * 2) );
	}
	else if( m_playerVisible )
	{
		m_curDir.TurnTo( m_lastSeenPlayerDir, YP(deltaTime) );
		m_noticeTimeout += deltaTime;
		if( m_noticeTimeout > MAGNUM_CAMERA_NOTICE_TIME )
		{
			m_noticeTimeout = 0;
			m_alertTimeout = MAGNUM_CAMERA_ALERT_TIME;
		}
	}
	else if( m_noticeTimeout > 0 )
	{
		m_curDir.TurnTo( m_lastSeenPlayerDir, YP(deltaTime) );
		m_noticeTimeout -= deltaTime;
	}
	else
	{
		switch( m_state )
		{
		case 0:
		case 2: // move
		{
			YawPitch tgt = m_state == 2 ? m_dir1 : m_dir0;
			YawPitch dst = YawPitchDist( m_dir0, m_dir1 ).Abs();
			float fdst = TMAX( dst.yaw, dst.pitch ) * safe_fdiv( deltaTime, m_moveTime );
			m_curDir.TurnTo( tgt, YP( fdst ) );
			if( YawPitchAlmostEqual( m_curDir, tgt ) )
			{
				m_state = ( m_state + 1 ) % 4;
				m_timeout = m_pauseTime;
			}
			break;
		}
		case 1:
		case 3: // wait
			m_timeout -= deltaTime;
			if( m_timeout <= 0 )
			{
				m_state = ( m_state + 1 ) % 4;
			}
			break;
		}
	}
	
	float f_turn_h = clamp( normalize_angle2( m_curDir.yaw ), -FLT_PI * 0.4f, FLT_PI * 0.4f ) / FLT_PI;
	float f_turn_v = clamp( normalize_angle2( m_curDir.pitch ), -FLT_PI * 0.25f, FLT_PI * 0.25f ) / FLT_PI;
	for( size_t i = 0; i < m_animChar.layers.size(); ++i )
	{
		AnimCharacter::Layer& L = m_animChar.layers[ i ];
		if( L.name == StringView("turn_h") )
			L.amount = f_turn_h;
		else if( L.name == StringView("turn_v") )
			L.amount = f_turn_v;
	}
	m_animChar.RecalcLayerState();
	m_animChar.FixedTick( deltaTime );
}

void TSCamera::Tick( float deltaTime, float blendFactor )
{
	m_animChar.PreRender( blendFactor );
	Mat4 mtx;
	m_animChar.GetAttachmentMatrix( 2, mtx );
	
	Vec3 color = V3(0,1,0);
	if( m_alertTimeout > 0 )
	{
		color = V3(1,0,0);
		if( m_playerVisible == false && fmodf( m_alertTimeout, 0.4f ) < 0.2f )
			color = V3(0);
	}
	else if( m_noticeTimeout > 0 )
		color = V3(1,1,0);
	FSFlare FD = { mtx.TransformPos( V3(0) ), color, 1, true };
	
	m_level->GetSystem<FlareSystem>()->UpdateFlare( this, FD );
	
	Vec3 tgtpos = m_animChar.GetAttachmentPos( m_animChar.FindAttachment( "origin" ) );
	InfoEmissionSystem::Data D = { tgtpos, 0.5f, IEST_Target };
	m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
	
	LevelMapSystem* lms = m_level->GetSystem<LevelMapSystem>();
	if( lms )
	{
		MapItemInfo mii = {0};
		
		Mat4 mtx;
		m_animChar.GetAttachmentMatrix( 0, mtx );
		
		mii.type = MI_Object_Camera;
		if( m_alertTimeout > 0 ) mii.type |= MI_State_Alerted;
		else if( m_noticeTimeout > 0 ) mii.type |= MI_State_Suspicious;
		else mii.type |= MI_State_Normal;
		
		mii.position = mtx.TransformPos( V3(0) );
		mii.direction = mtx.TransformNormal( V3(1,0,0) );
		mii.sizeFwd = 10;
		mii.sizeRight = 6;
		
		lms->UpdateItem( this, mii );
	}
}



TSCharacter::TSCharacter( GameLevel* lev, const Vec3& pos, const Vec3& dir ) :
	SGRX_Actor( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() ),
	m_footstepTime(0), m_isCrouching(false), m_isOnGround(false),
	m_ivPos( pos ), m_ivAimDir( dir ),
	m_position( pos ), m_moveDir( V2(0) ), m_turnAngle( atan2( dir.y, dir.x ) ),
	m_infoFlags( IEST_HeatSource )
{
	m_typeName = "character";
	typeOverride = "*human*";
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.3f,0.3f,0.5f) );
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	m_bodyHandle = m_level->GetPhyWorld()->CreateRigidBody( rbinfo );
	m_shapeHandle = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.25f) );
	
	m_shadowInst = m_level->GetScene()->CreateLight();
	m_shadowInst->type = LIGHT_PROJ;
	m_shadowInst->direction = V3(0,0,-1);
	m_shadowInst->updir = V3(0,1,0);
	m_shadowInst->angle = 60;
	m_shadowInst->range = 1.5f;
	m_shadowInst->UpdateTransform();
	m_shadowInst->projectionMaterial.textures[0] = GR_GetTexture( "textures/fx/blobshadow.png" );//GR_GetTexture( "textures/unit.png" );
	m_shadowInst->projectionMaterial.textures[1] = GR_GetTexture( "textures/fx/projfalloff2.png" );
	
	m_anLayers[0].anim = &m_anMainPlayer;
	m_anLayers[1].anim = &m_anTopPlayer;
	m_anLayers[2].anim = &m_animChar.m_layerAnimator;
	m_anLayers[2].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
	m_anLayers[3].anim = &m_animChar.m_anRagdoll;
	m_anLayers[3].tflags = AnimMixer::TF_Absolute_Pos | AnimMixer::TF_Absolute_Rot;
	m_anLayers[3].factor = 0;
	m_animChar.m_anMixer.layers = m_anLayers;
	m_animChar.m_anMixer.layerCount = 3;
	
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
	m_timeSinceLastHit = 9999;
}

void TSCharacter::InitializeMesh( const StringView& path )
{
	m_animChar.Load( path );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->userData = (SGRX_MeshInstUserData*) this;
	MI->layers = 0x2;
	MI->matrix = Mat4::CreateSRT( V3(1), Quat::Identity, m_ivPos.curr );
	m_level->LightMesh( MI, V3(1) );
	
	m_anTopPlayer.ClearBlendFactors( 0.0f );
	m_animChar.ApplyMask( "top", &m_anTopPlayer );
	
	m_anMainPlayer.Play( GR_GetAnim( "standing_idle" ) );
//	m_anTopPlayer.Play( GR_GetAnim( "run" ) );
}

void TSCharacter::FixedTick( float deltaTime )
{
	SGRX_Actor::FixedTick( deltaTime );
	
	if( IsInAction() )
	{
//		i_move = V2(0);
		if( m_actState.timeoutMoveToStart > 0 )
		{
			m_anMainPlayer.Play( GR_GetAnim( "stand_with_gun_up" ), false, 0.2f );
			
			m_bodyHandle->SetLinearVelocity( V3(0) );
			if( ( m_actState.info.placePos - GetPosition() ).ToVec2().Length() < 0.1f )
			{
				m_actState.timeoutMoveToStart = 0;
			}
			else
			{
				PushTo( m_actState.info.placePos, deltaTime );
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
				m_anMainPlayer.Play( GR_GetAnim( "kneeling" ) );
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
	else
	{
		Vec2 i_move = GetInputV2( ACT_Chr_Move );
		
		// animate character
		const char* anim_stand = m_isCrouching ? "crouch" : "stand_with_gun_up";
		const char* anim_run_fw = m_isCrouching ? "crouch_walk" : "run";
		const char* anim_run_bw = m_isCrouching ? "crouch_walk_bw" : "run_bw";
		
		const char* animname = anim_run_fw;
		Vec2 md = i_move.Normalized();
		if( Vec2Dot( md, GetAimDir().ToVec2() ) < 0 )
		{
			md = -md;
			animname = anim_run_bw;
		}
		// TODO TURN ISSUES
	//	if( i_move.Length() > 0.1f )
	//	{
	//		TurnTo( md, deltaTime * 8 );
	//	}
#ifdef LD33GAME
		anim_stand = "idle";
		animname = "walk";
#endif
		
		m_anMainPlayer.Play( GR_GetAnim( i_move.Length() > 0.5f ? animname : anim_stand ), false, 0.2f );
	}
	
	HandleMovementPhysics( deltaTime );
	
	//
	Vec2 rundir = V2( cosf( m_turnAngle ), sinf( m_turnAngle ) );
	Vec2 aimdir = rundir;
	if( GetInputB( ACT_Chr_AimAt ) )
	{
		aimdir = ( GetInputV3( ACT_Chr_AimTarget ) - GetPosition() ).ToVec2();
	}
	m_ivAimDir.Advance( V3( aimdir.x, aimdir.y, 0 ) );
	//
	
	float f_turn_btm = ( atan2( rundir.y, rundir.x ) - M_PI / 2 ) / ( M_PI * 2 );
	float f_turn_top = ( atan2( aimdir.y, aimdir.x ) - M_PI / 2 ) / ( M_PI * 2 );
	for( size_t i = 0; i < m_animChar.layers.size(); ++i )
	{
		AnimCharacter::Layer& L = m_animChar.layers[ i ];
		if( L.name == StringView("turn_bottom") )
			L.amount = f_turn_btm;
		else if( L.name == StringView("turn_top") )
			L.amount = f_turn_top;
	}
	m_animChar.RecalcLayerState();
	
	if( m_anMainPlayer.CheckMarker( "step" ) )
	{
		Vec3 pos = m_bodyHandle->GetPosition();
		Vec3 lvel = m_bodyHandle->GetLinearVelocity();
		SoundEventInstanceHandle fsev = g_SoundSys->CreateEventInstance( "/footsteps" );
		SGRX_Sound3DAttribs s3dattr = { pos, lvel, V3(0), V3(0) };
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
	}
	
	m_animChar.FixedTick( deltaTime );
	m_timeSinceLastHit += deltaTime;
	
	if( GetInputB( ACT_Chr_DoAction ) )
	{
		BeginClosestAction( 2 );
	}
}

void TSCharacter::Tick( float deltaTime, float blendFactor )
{
	SGRX_Actor::Tick( deltaTime, blendFactor );
	
	Vec3 turn = GetInputV3( ACT_Chr_Turn );
	if( turn.z > 0 )
		TurnTo( turn.ToVec2(), turn.z * deltaTime );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->matrix = Mat4::CreateTranslation( pos ); // Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	m_level->LightMesh( MI, V3(0,0,m_isCrouching ? 0.6f : 1) );
	
	m_animChar.PreRender( blendFactor );
	m_interpPos = m_ivPos.Get( blendFactor );
	m_interpAimDir = m_ivAimDir.Get( blendFactor );
	
	
	Vec3 tgtpos = m_animChar.GetAttachmentPos( m_animChar.FindAttachment( "target" ) );
	InfoEmissionSystem::Data D = { tgtpos, 0.5f, m_infoFlags };
	m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
	
	
	AnimDeformer& AnD = m_animChar.m_anDeformer;
	for( size_t i = 0; i < AnD.forces.size(); ++i )
	{
		AnimDeformer::Force& F = AnD.forces[ i ];
		float t = F.lifetime;
		F.amount = sinf( t * FLT_PI ) * expf( -t * 8 ) * 7 * 0.2f;
		if( F.lifetime > 3 )
		{
			AnD.forces.erase( i-- );
		}
	}
	
	
	m_shootLT->enabled = false;
	if( m_shootTimeout > 0 )
	{
		m_shootTimeout -= deltaTime;
		m_shootLT->enabled = true;
	}
	if( GetInputB( ACT_Chr_Shoot ) && m_shootTimeout <= 0 )
	{
		Mat4 mtx = GetBulletOutputMatrix();
		Vec3 origin = mtx.TransformPos( V3(0) );
		Vec3 dir = ( GetInputV3( ACT_Chr_AimTarget ) - origin ).Normalized();
		dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
		m_level->GetSystem<BulletSystem>()->Add( origin, dir * 100, 1, 1, ownerType );
		m_shootPS.SetTransform( mtx );
		m_shootPS.Trigger();
		m_shootLT->position = origin;
		m_shootLT->UpdateTransform();
		m_shootLT->enabled = true;
		m_shootTimeout += 0.1f;
		m_level->GetSystem<AIDBSystem>()->AddSound( GetInterpPos(), 10, 0.2f, AIS_Shot );
	}
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*0.5f * smoothlerp_oneway( m_shootTimeout, 0, 0.1f );
	
	m_shootPS.Tick( deltaTime );
	m_shootPS.PreRender();
}

void TSCharacter::HandleMovementPhysics( float deltaTime )
{
	// disabled features
	bool jump = false;
	float m_jumpTimeout = 1, m_canJumpTimeout = 1;
	
	SGRX_PhyRaycastInfo rcinfo;
	SGRX_PhyRaycastInfo rcinfo2;
	
	m_jumpTimeout = TMAX( 0.0f, m_jumpTimeout - deltaTime );
	m_canJumpTimeout = TMAX( 0.0f, m_canJumpTimeout - deltaTime );
	
	Vec3 pos = m_bodyHandle->GetPosition();
	
	bool prevCrouch = m_isCrouching;
	m_isCrouching = GetInputB( ACT_Chr_Crouch );
	if( m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
		fabsf( rcinfo.point.z - rcinfo2.point.z ) < 1.8f )
	{
		m_isCrouching = true;
	}
	
	if( prevCrouch != m_isCrouching )
	{
		float diff = prevCrouch - m_isCrouching;
		Vec3 chg = V3( 0, 0, 0.7f * diff );
		pos += chg;
		m_bodyHandle->SetPosition( pos );
	}
	
	float cheight = m_isCrouching ? 0.6f : 1.3f;
	float rcxdist = 1.0f;
	Vec3 lvel = m_bodyHandle->GetLinearVelocity();
	float ht = cheight - 0.25f;
	if( lvel.z >= -SMALL_FLOAT && !m_jumpTimeout )
		ht += rcxdist;
	
	m_ivPos.Advance( pos + V3(0,0,-cheight) );
	
	bool ground = false;
	if( m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-ht), 1, 1, &rcinfo )
		&& fabsf( rcinfo.point.z - pos.z ) < cheight + SMALL_FLOAT )
	{
		Vec3 v = m_bodyHandle->GetPosition();
		float tgt = rcinfo.point.z + cheight;
		v.z += TMIN( deltaTime * 5, fabsf( tgt - v.z ) ) * sign( tgt - v.z );
		m_bodyHandle->SetPosition( v );
		lvel.z = 0;
		ground = true;
		m_canJumpTimeout = 0.5f;
	}
	if( !m_jumpTimeout && m_canJumpTimeout && jump )
	{
		lvel.z = 4;
		m_jumpTimeout = 0.5f;
		
		SoundEventInstanceHandle fsev = g_SoundSys->CreateEventInstance( "/footsteps" );
		SGRX_Sound3DAttribs s3dattr = { pos, lvel, V3(0), V3(0) };
		fsev->Set3DAttribs( s3dattr );
		fsev->Start();
	}
	
	if( !m_isOnGround && ground )
	{
		m_footstepTime = 1.0f;
	}
	m_isOnGround = ground;
	
	Vec2 md = GetInputV2( ACT_Chr_Move );
	if( md.LengthSq() > 1 )
		md.Normalize();
	
	Vec2 lvel2 = lvel.ToVec2();
	
	float maxspeed = 5 * GetInputV3( ACT_Chr_Move ).z;
	float accel = ( md.NearZero() && !m_isCrouching ) ? 38 : 30;
	if( m_isCrouching ){ accel = 5; maxspeed = 2.5f; }
	if( !ground ){ accel = 10; }
	
	float curspeed = Vec2Dot( lvel2, md );
	float revmaxfactor = clamp( maxspeed - curspeed, 0, 1 );
	lvel2 += md * accel * revmaxfactor * deltaTime;
	
	///// FRICTION /////
	curspeed = Vec2Dot( lvel2, md );
	if( ground )
	{
		if( curspeed > maxspeed )
			curspeed = maxspeed;
	}
	lvel2 -= md * curspeed;
	{
		Vec2 ldd = lvel2.Normalized();
		float llen = lvel2.Length();
		llen = TMAX( 0.0f, llen - deltaTime * ( ground ? 20 : ( m_isCrouching ? 0.5f : 3 ) ) );
		lvel2 = ldd * llen;
	}
	lvel2 += md * curspeed;
	
	lvel.x = lvel2.x;
	lvel.y = lvel2.y;
	
	m_bodyHandle->SetLinearVelocity( lvel );
}

void TSCharacter::TurnTo( const Vec2& turnDir, float speedDelta )
{
	float angend = normalize_angle( turnDir.Angle() );
	float angstart = normalize_angle( m_turnAngle );
	if( fabs( angend - angstart ) > M_PI )
		angstart += angend > angstart ? M_PI * 2 : -M_PI * 2;
//	printf( "cur: %1.2f, target: %1.2f\n", angstart, angend);
	m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), speedDelta );
}

void TSCharacter::PushTo( const Vec3& pos, float speedDelta )
{
	Vec2 diff = pos.ToVec2() - GetPosition().ToVec2();
	diff = diff.Normalized() * TMIN( speedDelta, diff.Length() );
	m_bodyHandle->SetPosition( GetPosition() + V3( diff.x, diff.y, 0 ) );
}

void TSCharacter::BeginClosestAction( float maxdist )
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

bool TSCharacter::BeginAction( Entity* E )
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

bool TSCharacter::IsInAction()
{
	return m_actState.target != NULL;
}

bool TSCharacter::CanInterruptAction()
{
	if( IsInAction() == false )
		return false;
	
	IInteractiveEntity* IE = m_actState.target->GetInterface<IInteractiveEntity>();
	return IE && IE->CanInterruptAction( m_actState.progress );
}

void TSCharacter::InterruptAction( bool force )
{
	if( force == false && CanInterruptAction() == false )
		return;
	
	m_actState.progress = 0;
	m_actState.target = NULL;
}

void TSCharacter::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		Mat4 inv = Mat4::Identity;
		m_animChar.m_cachedMeshInst->matrix.InvertTo( inv );
		Vec3 pos = inv.TransformPos( bhinfo->pos );
		Vec3 vel = inv.TransformNormal( bhinfo->vel );
		m_animChar.m_anDeformer.AddModelForce( pos, vel * 0.1f, 0.3f );
		Hit( bhinfo->vel.Length() * 0.1f );
	}
}

void TSCharacter::Hit( float pwr )
{
	float m_health = 1000000;
	if( m_health > 0 )
	{
		m_timeSinceLastHit = 0;
//		RenewAction();
		m_health -= pwr;
		if( m_health <= 0 )
		{
			OnDeath();
			m_level->GetSystem<InfoEmissionSystem>()->RemoveEmitter( this );
		}
	}
}

Vec3 TSCharacter::GetQueryPosition()
{
	return GetPosition() + V3(0,0,0.5f);
}

Vec3 TSCharacter::GetPosition()
{
	return m_bodyHandle->GetPosition();
}

Vec3 TSCharacter::GetViewDir()
{
	return V3( cosf( m_turnAngle ), sinf( m_turnAngle ), 0 );
}

Vec3 TSCharacter::GetAimDir()
{
	Vec3 aimdir = V3( cosf( m_turnAngle ), sinf( m_turnAngle ), 0 );
	if( GetInputB( ACT_Chr_AimAt ) )
	{
		aimdir = ( GetInputV3( ACT_Chr_AimTarget ) - GetPosition() );
	}
	return aimdir;
}

Mat4 TSCharacter::GetBulletOutputMatrix()
{
	Mat4 out = m_animChar.m_cachedMeshInst->matrix;
	for( size_t i = 0; i < m_animChar.attachments.size(); ++i )
	{
		if( m_animChar.attachments[ i ].name == StringView("gun_barrel") )
		{
			m_animChar.GetAttachmentMatrix( i, out );
			break;
		}
	}
	return out;
}

Vec3 TSCharacter::GetInterpPos()
{
	return m_interpPos;
}

Vec3 TSCharacter::GetInterpAimDir()
{
	return m_interpAimDir;
}



TSAimHelper::TSAimHelper( GameLevel* lev ) :
	m_level(lev), m_pos(V3(0)), m_cp(V2(0)), m_aimPtr(NULL),
	m_aimPoint(V3(0)), m_rcPoint(V3(0)), m_aimFactor(0), m_pDist(0),
	m_closestEnt(NULL), m_closestPoint(V3(0))
{
	m_tex_cursor = GR_GetTexture( "ui/crosshair.png" );
}

void TSAimHelper::Tick( float deltaTime, Vec3 pos, Vec2 cp, bool lock )
{
	m_pos = pos;
	if( m_aimFactor == 0 || m_aimPtr == NULL )
		m_cp = cp;
	m_rcPoint = _CalcRCPos( pos );
	
	if( lock )
	{
		m_pDist = 0.5f;
		m_closestEnt = NULL;
		lock = m_level->GetSystem<InfoEmissionSystem>()
			->QuerySphereAll( this, pos, 8.0f, IEST_Target );
		if( m_aimPtr == NULL || m_aimPtr == m_closestEnt )
		{
			m_aimPoint = m_closestPoint;
			if( m_aimFactor < SMALL_FLOAT )
			{
				m_aimPtr = m_closestEnt;
			}
		}
		else
		{
			m_aimPtr = NULL;
		}
	}
	else m_aimPtr = NULL;
	
	float tgt = m_aimPtr ? 1 : 0;
	float diff = tgt - m_aimFactor;
	m_aimFactor += TMIN( fabsf( diff ), deltaTime * 5 ) * sign( diff );
	
	if( m_aimFactor > 0 && diff >= 0 )
	{
		Vec3 tgtpos = TLERP( m_rcPoint, m_aimPoint, m_aimFactor );
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		Vec2 target_pos = m_level->GetScene()->camera.WorldToScreen( tgtpos ).ToVec2();
		Vec2 tgt = V2( clamp( target_pos.x, 0, 1 ), clamp( target_pos.y, 0, 1 ) ) * screen_size;
		Game_SetCursorPos( tgt.x, tgt.y );
	}
}

void TSAimHelper::DrawUI()
{
	m_pDist = 0.5f;
	m_closestEnt = NULL;
	m_level->GetSystem<InfoEmissionSystem>()
		->QuerySphereAll( this, m_pos, 8.0f, IEST_Target );
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	Vec2 cursor_pos = m_cp * screen_size;
	Vec2 player_pos = m_level->GetScene()->camera.WorldToScreen( m_pos ).ToVec2() * screen_size;
	
	// target cursor
	if( m_aimPtr == NULL && m_closestEnt )
	{
		Vec2 cp = m_level->GetScene()->camera.WorldToScreen( m_closestPoint ).ToVec2() * screen_size;
		float cursor_size = bsz / 20 * powf( 1 - m_pDist * 1.9f, 0.5f );
		float cursor_angle = ( cp - player_pos ).Angle() + M_PI;
		br.Reset().Col( 1, 0.5f ).SetTexture( m_tex_cursor ).TurnedBox( cp.x, cp.y,
			cosf( cursor_angle ) * cursor_size, sinf( cursor_angle ) * cursor_size );
	}
	
	// main cursor
	{
		Vec2 target_pos = m_level->GetScene()->camera.WorldToScreen( m_aimPoint ).ToVec2() * screen_size;
		Vec2 cp = TLERP( cursor_pos, target_pos, m_aimFactor );
		float cursor_size = bsz / 20;
		float cursor_angle = ( cp - player_pos ).Angle() + M_PI;
		br.Reset().SetTexture( m_tex_cursor ).TurnedBox( cp.x, cp.y,
			cosf( cursor_angle ) * cursor_size, sinf( cursor_angle ) * cursor_size );
	}
}

Vec3 TSAimHelper::GetAimPoint()
{
	return TLERP( m_rcPoint, m_aimPoint, m_aimFactor );
}

Vec3 TSAimHelper::_CalcRCPos( Vec3 pos )
{
	Vec3 crpos, crdir;
	m_level->GetScene()->camera.GetCursorRay( m_cp.x, m_cp.y, crpos, crdir );
	Vec3 crtgt = crpos + crdir * 100;
	
	SGRX_PhyRaycastInfo rcinfo;
	if( m_level->GetPhyWorld()->Raycast( crpos, crtgt, 0x1, 0x1, &rcinfo ) )
	{
		bool atwall = 0.707f > fabsf(Vec3Dot( rcinfo.normal, V3(0,0,1) )); // > ~45deg to up vector
		bool frontface = Vec3Dot( rcinfo.normal, crdir ) < 0; 
		if( atwall && frontface )
			return rcinfo.point;
		
		// must adjust target height above ground
		if( frontface )
			return rcinfo.point + V3(0,0,1.1f);
	}
	
	// backup same level plane test if aiming into nothing
	float dsts[2];
	if( RayPlaneIntersect( crpos, crdir, V4(0,0,1.1f,pos.z), dsts ) )
	{
		return crpos + crdir * dsts[0];
	}
	
	return V3(0);
}

bool TSAimHelper::Process( Entity* E, const InfoEmissionSystem::Data& D )
{
	if( D.types & IEST_Player )
		return true;
	
	SceneRaycastCallback_Any srcb;
	m_level->GetScene()->RaycastAll( m_pos, D.pos, &srcb, 0x1 );
	if( srcb.m_hit )
		return true;
	
	if( E == m_aimPtr )
	{
		m_closestEnt = m_aimPtr;
		m_closestPoint = D.pos;
		return false;
	}
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	float bsz = TMIN( screen_size.x, screen_size.y );
	Vec2 curpos = m_cp * screen_size;
	Vec2 scrpos = m_level->GetScene()->camera.WorldToScreen( D.pos ).ToVec2() * screen_size;
	float npdist = ( curpos - scrpos ).Length();
	if( npdist < m_pDist * bsz )
	{
		m_closestEnt = E;
		m_closestPoint = D.pos;
		m_pDist = npdist / bsz;
	}
	return true;
}



#ifndef TSGAME_NO_PLAYER


TSPlayerController::TSPlayerController( GameLevel* lev ) : m_aimHelper( lev ),
	i_move( V2(0) ), i_aim_target( V3(0) ), i_turn( V3(0) )
{
}

void TSPlayerController::Tick( float deltaTime, float blendFactor )
{
	i_move = V2
	(
		-MOVE_X.value + MOVE_LEFT.value - MOVE_RIGHT.value,
		MOVE_Y.value + MOVE_DOWN.value - MOVE_UP.value
	);
	i_aim_target = m_aimHelper.GetAimPoint();
	i_turn = V3(0);
	if( i_move.Length() > 0.1f )
	{
		Vec2 md = i_move;
		if( Vec2Dot( ( i_aim_target - m_aimHelper.m_pos ).ToVec2(), md ) < 0 )
			md = -md;
		i_turn = V3( md.x, md.y, 8 );
	}
}

Vec3 TSPlayerController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, 1 );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3(CROUCH.value);
	case ACT_Chr_AimAt: return V3(1);
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3(SHOOT.value);
	case ACT_Chr_DoAction: return V3(DO_ACTION.value);
	}
	return V3(0);
}

#endif



TSEnemyController::TSEnemyController( GameLevel* lev, TSCharacter* chr, sgsVariable args ) :
	i_crouch( false ), i_move( V2(0) ), i_speed( 1 ), i_turn( V3(0) ),
	i_aim_at( false ), i_aim_target( V3(0) ), i_shoot( false ), i_act( false ),
	m_level( lev ), m_aidb( m_level->GetSystem<AIDBSystem>() ), m_char( chr )
{
	// create controller scripted object
	{
		_sgs_interface->destruct = NULL;
		SGS_CSCOPE( m_level->GetSGSC() );
		sgs_PushClass( m_level->GetSGSC(), this );
		C = m_level->GetSGSC();
		m_sgsObject = sgs_GetObjectStruct( C, -1 );
		sgs_ObjAcquire( C, m_sgsObject );
	}
	
	// create ESO (enemy scripted object)
	{
		chr->InitScriptInterface();
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		sgs_PushObjectPtr( m_level->m_scriptCtx.C, m_sgsObject );
		sgs_PushObjectPtr( m_level->m_scriptCtx.C, chr->m_sgsObject );
		m_level->m_scriptCtx.Push( args );
		if( m_level->m_scriptCtx.GlobalCall( "TSEnemy_Create", 3, 1 ) == false )
		{
			LOG_ERROR << "FAILED to create enemy state";
		}
		m_enemyState = sgsVariable( m_level->m_scriptCtx.C, -1 );
	}
}

TSEnemyController::~TSEnemyController()
{
	// destroy ESO
	{
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		m_enemyState.thiscall( "destroy" );
	}
	
	sgs_ObjRelease( C, m_sgsObject );
	m_sgsObject = NULL;
}

struct IESEnemyViewProc : InfoEmissionSystem::IESProcessor
{
	bool Process( Entity* ent, const InfoEmissionSystem::Data& data )
	{
		Vec3 enemypos = data.pos + V3(0,0,1); // FIXME MAYBE?
		
		// verify the find
		Vec3 vieworigin = enemy->m_char->GetPosition();
		Vec3 viewdir = enemy->m_char->GetViewDir();
		Vec3 view2pos = enemypos - vieworigin;
		float vpdot = Vec3Dot( viewdir.Normalized(), view2pos.Normalized() );
		if( vpdot < cosf(DEG2RAD(40.0f)) )
			return true; // outside view cone
		
		if( ent->m_level->GetPhyWorld()->Raycast( vieworigin, enemypos, 1, 1 ) )
			return true; // behind wall
		
		// TODO friendlies
		AIFactStorage& FS = enemy->m_factStorage;
		
		if( data.types & IEST_AIAlert )
		{
			FS.InsertOrUpdate( FT_Sight_Alarming,
				enemypos, 0, curtime, curtime + 5*1000, 0 );
		}
		else
		{
			// fact of seeing
			FS.MovingInsertOrUpdate( FT_Sight_Foe,
				enemypos, 10, curtime, curtime + 5*1000 );
			
			// fact of position
			FS.MovingInsertOrUpdate( FT_Position_Foe,
				enemypos, 10, curtime, curtime + 30*1000, FS.last_mod_id );
		}
		
		return true;
	}
	
	TimeVal curtime;
	TSEnemyController* enemy;
};

void TSEnemyController::FixedTick( float deltaTime )
{
	TimeVal curTime = m_level->GetPhyTime();
	
	// process facts
	m_factStorage.Process( curTime );
	// - vision
	IESEnemyViewProc evp;
	evp.curtime = curTime;
	evp.enemy = this;
	m_level->GetSystem<InfoEmissionSystem>()->QuerySphereAll( &evp,
		m_char->GetPosition(), 10.0f, IEST_Player | IEST_AIAlert );
	// - sounds
	for( int i = 0; i < m_aidb->GetNumSounds(); ++i )
	{
		if( m_aidb->CanHearSound( m_char->GetPosition(), i ) == false )
			continue;
		AISound S = m_aidb->GetSoundInfo( i );
		
		if( S.type == AIS_Footstep || S.type == AIS_Shot )
		{
			AIFactType sndtype = FT_Sound_Footstep;
			if( S.type == AIS_Shot )
				sndtype = FT_Sound_Shot;
			
			m_factStorage.InsertOrUpdate( sndtype,
				S.position, SMALL_FLOAT, curTime, curTime + 1*1000, 0, false );
			
			int lastid = m_factStorage.last_mod_id;
			bool found_friend = m_factStorage.MovingUpdate( FT_Position_Friend,
				S.position, 10, curTime, curTime + 30*1000, lastid );
			if( found_friend == false )
			{
				m_factStorage.MovingUpdate( FT_Position_Foe,
					S.position, 10, curTime, curTime + 30*1000, lastid );
			}
		}
		else
		{
			m_factStorage.InsertOrUpdate( FT_Sound_Noise,
				S.position, 1, curTime, curTime + 1*1000 );
		}
	}
	
	// tick ESO
	{
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		m_level->m_scriptCtx.Push( deltaTime );
		m_enemyState.thiscall( "tick", 1 );
		
		i_crouch = m_enemyState[ "i_crouch" ].get<bool>();
		i_move = m_enemyState[ "i_move" ].get<Vec2>();
		i_speed = m_enemyState[ "i_speed" ].get<float>();
		i_turn = m_enemyState[ "i_turn" ].get<Vec3>();
		sgsVariable aim_target = m_enemyState[ "i_aim_target" ];
		i_aim_at = aim_target.not_null();
		i_aim_target = aim_target.get<Vec3>();
		i_shoot = m_enemyState[ "i_shoot" ].get<bool>();
		i_act = m_enemyState[ "i_act" ].get<bool>();
	}
}

void TSEnemyController::Tick( float deltaTime, float blendFactor )
{
	LevelMapSystem* lms = m_level->GetSystem<LevelMapSystem>();
	if( lms )
	{
		MapItemInfo mii = {0};
		
		mii.type = MI_Object_Enemy | MI_State_Normal;
		mii.position = m_char->GetInterpPos();
		mii.direction = m_char->GetInterpAimDir();
		mii.sizeFwd = 10;
		mii.sizeRight = 8;
		
		lms->UpdateItem( m_char, mii );
	}
}

Vec3 TSEnemyController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, 1 );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3( i_crouch );
	case ACT_Chr_AimAt: return V3( i_aim_at );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3( i_shoot );
	case ACT_Chr_DoAction: return V3( i_act );
	}
	return V3(0);
}

#if 0
bool TSEnemyController::GetMapItemInfo( MapItemInfo* out )
{
	return true;
}
#endif

void TSEnemyController::DebugDrawWorld()
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().Col( 0.9f, 0.2f, 0.1f );
	Vec3 pos = m_char->GetPosition();
	
	m_factStorage.SortCreatedDesc();
	
	size_t count = TMIN( size_t(10), m_factStorage.facts.size() );
	for( size_t i = 0; i < count; ++i )
	{
		AIFact& F = m_factStorage.facts[ i ];
		br.SetPrimitiveType( PT_Lines );
		br.Pos( pos ).Pos( F.position );
		br.Tick( F.position, 0.1f );
	}
}

void TSEnemyController::DebugDrawUI()
{
	char bfr[ 256 ];
	BatchRenderer& br = GR2D_GetBatchRenderer();
	Vec3 pos = m_char->GetPosition();
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
		AIFact& F = m_factStorage.facts[ i ];
		const char* type = "type?";
		switch( F.type )
		{
		case FT_Unknown: type = "unknown"; break;
		case FT_Sound_Noise: type = "snd-noise"; break;
		case FT_Sound_Footstep: type = "snd-step"; break;
		case FT_Sound_Shot: type = "snd-shot"; break;
		case FT_Sight_ObjectState: type = "sight-state"; break;
		case FT_Sight_Alarming: type = "sight-alarm"; break;
		case FT_Sight_Friend: type = "sight-friend"; break;
		case FT_Sight_Foe: type = "sight-foe"; break;
		case FT_Position_Friend: type = "pos-friend"; break;
		case FT_Position_Foe: type = "pos-foe"; break;
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

bool TSEnemyController::sgsHasFact( int typemask )
{
	return HasFact( typemask );
}

bool TSEnemyController::sgsHasRecentFact( int typemask, TimeVal maxtime )
{
	return HasRecentFact( typemask, maxtime );
}

SGS_MULTRET TSEnemyController::sgsGetRecentFact( int typemask, TimeVal maxtime )
{
	AIFact* F = GetRecentFact( typemask, maxtime );
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


TSGameSystem::TSGameSystem( GameLevel* lev )
	: IGameLevelSystem( lev, e_system_uid )
#ifndef TSGAME_NO_PLAYER
	, m_playerCtrl( lev )
	, m_crouchIconShowTimeout( 0 ), m_standIconShowTimeout( 1 ), m_prevCrouchValue( 0 )
#endif
{
#ifndef TSGAME_NO_PLAYER
	m_playerCtrl.Acquire();
#endif
	m_level->GetScriptCtx().Include( "data/enemy" );
}

bool TSGameSystem::AddEntity( const StringView& type, sgsVariable data )
{
#ifndef TSGAME_NO_PLAYER
	///////////////////////////
	if( type == "player" )
	{
		TSCharacter* P = new TSCharacter
		(
			m_level,
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>()
		);
		P->m_infoFlags |= IEST_Player;
		P->InitializeMesh( "chars/tstest.chr" );
		P->ownerType = GAT_Player;
		P->ctrl = &m_playerCtrl;
		m_level->AddEntity( P );
		m_level->SetPlayer( P );
		return true;
	}
#endif
	
	///////////////////////////
	if( type == "camera" )
	{
		TSCamera* CAM = new TSCamera
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("char").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("dir0").get<Vec3>(),
			data.getprop("dir1").get<Vec3>()
		);
		m_level->AddEntity( CAM );
		return true;
	}
	
	///////////////////////////
	if( type == "enemy_start" )
	{
		TSCharacter* E = new TSCharacter
		(
			m_level,
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>()
		);
		E->m_infoFlags |= IEST_Target;
		E->InitializeMesh( "chars/tstest.chr" );
		E->ownerType = GAT_Enemy;
		E->m_name = data.getprop("name").get<StringView>();
		m_level->MapEntityByName( E );
		E->ctrl = new TSEnemyController( m_level, E, data );
		m_level->AddEntity( E );
		return true;
	}
	
	return false;
}

void TSGameSystem::Tick( float deltaTime, float blendFactor )
{
#ifndef TSGAME_NO_PLAYER
	SGRX_CAST( TSCharacter*, P, m_level->m_player );
	if( P )
	{
		// crouch UI
		float crouchVal = m_playerCtrl.GetInput( ACT_Chr_Crouch ).x;
		bool crouch = crouchVal > 0.5f;
		if( crouchVal != m_prevCrouchValue )
		{
			m_crouchIconShowTimeout = crouch ? 2 : 0;
			m_standIconShowTimeout = crouch ? 0 : 2;
			m_prevCrouchValue = crouchVal;
		}
		m_crouchIconShowTimeout = TMAX( m_crouchIconShowTimeout - deltaTime, crouch ? 1.0f : 0.0f );
		m_standIconShowTimeout = TMAX( m_standIconShowTimeout - deltaTime, crouch ? 0.0f : 1.0f );
		
		// camera
		TSAimHelper& AH = m_playerCtrl.m_aimHelper;
		Vec3 pos = P->GetInterpPos();
		float bmsz = ( GR_GetWidth() + GR_GetHeight() );// * 0.5f;
		Vec2 cursor_pos = CURSOR_POS;
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		Vec2 player_pos = m_level->GetScene()->camera.WorldToScreen( pos ).ToVec2() * screen_size;
		Vec2 diff = ( cursor_pos - player_pos ) / bmsz;
		AH.Tick( deltaTime, pos, CURSOR_POS / screen_size, LOCK_ON.value > 0.5f );
		Vec3 camtgt = TLERP( pos, AH.m_aimPoint, 0.1f * smoothstep( AH.m_aimFactor ) );
		
		SGRX_Camera& CAM = m_level->GetScene()->camera;
		CAM.znear = 0.1f;
		CAM.angle = 90;
		CAM.updir = V3(0,-1,0);
		CAM.direction = V3(-diff.x,diff.y,-5);
		CAM.position = camtgt + V3(-diff.x,diff.y,0) * 2 + V3(0,0,1) * 6;
		CAM.UpdateMatrices();
		
		// map
		MapItemInfo mymapitem = { MI_Object_Player, pos, V3(0), 0, 0 };
		m_level->GetSystem<LevelMapSystem>()->UpdateItem( P, mymapitem );
		m_level->GetSystem<LevelMapSystem>()->m_viewPos = pos.ToVec2();
	}
#endif
}

void TSGameSystem::DrawUI()
{
#ifndef TSGAME_NO_PLAYER
	
	// UI
	SGRX_CAST( TSCharacter*, P, m_level->m_player );
	if( P )
	{
		SGRX_FontSettings fs;
		GR2D_GetFontSettings( &fs );
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
		
		Vec3 QP = P->GetQueryPosition();
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
		
		m_playerCtrl.m_aimHelper.DrawUI();
		
		GR2D_SetFont( "tsicons", bsz * 0.2f );
		br.Col( 1, 0.25f * m_crouchIconShowTimeout );
		GR2D_DrawTextLine( round( bsz * 0.1f ), round( screen_size.y - bsz * 0.1f ), "\x0a", HALIGN_LEFT, VALIGN_BOTTOM );
		br.Col( 1, 0.25f * m_standIconShowTimeout );
		GR2D_DrawTextLine( round( bsz * 0.1f ), round( screen_size.y - bsz * 0.1f ), "\x0b", HALIGN_LEFT, VALIGN_BOTTOM );
		
		GR2D_SetFontSettings( &fs );
	}
#endif
}


