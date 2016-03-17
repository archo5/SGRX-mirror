

#include "entities_ts.hpp"


CVarBool gcv_notarget( "notarget", false );
CVarBool gcv_dbg_aifacts( "dbg_aifacts", false );
CVarBool gcv_dbg_aistack( "dbg_aistack", false );

void register_tsent_cvars()
{
	REGCOBJ( gcv_notarget );
	REGCOBJ( gcv_dbg_aifacts );
	REGCOBJ( gcv_dbg_aistack );
}



extern Vec2 CURSOR_POS;
extern InputState MOVE_LEFT;
extern InputState MOVE_RIGHT;
extern InputState MOVE_UP;
extern InputState MOVE_DOWN;
extern InputState MOVE_X;
extern InputState MOVE_Y;
extern InputState AIM_X;
extern InputState AIM_Y;
extern InputState SHOOT;
extern InputState LOCK_ON;
extern InputState RELOAD;
extern InputState CROUCH;
extern InputState DO_ACTION;


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
	SetID( name );
	
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
}

void TSCamera::FixedTick( float deltaTime )
{
	m_playerVisible = false;
#if 0
	if( m_level->m_player )
	{
		Vec3 ppos = V3(0);
#ifdef TSGAME
		ppos = static_cast<TSCharacter*>(m_level->m_player)->GetWorldPosition();
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
#endif
	
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
	SetInfoMask( IEST_Target );
	SetInfoTarget( tgtpos );
	
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



TSCharacter::TSCharacter( GameLevel* lev ) :
	Actor( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() ),
	m_health( 100 ), m_armor( 0 ),
	m_footstepTime(0), m_isCrouching(false), m_isOnGround(false),
	m_ivPos( V3(0) ), m_ivAimDir( V3(1,0,0) ),
	m_turnAngle( 0 ),
	m_aimDir( YP(V3(1,0,0)) ), m_aimDist( 1 ),
	m_infoFlags( IEST_HeatSource ), m_animTimeLeft( 0 ),
	m_skipTransformUpdate( false )
{
	typeOverride = "*human*";
	
	PhyShapeHandle baseShape = m_level->GetPhyWorld()->CreateCylinderShape( V3(0.3f,0.3f,0.5f) );
	PhyShapeHandle fullShape = m_level->GetPhyWorld()->CreateCompoundShape();
	fullShape->AddChildShape( baseShape );
	fullShape->AddChildShape( baseShape, V3( 0, -0.5f, 0 ) );
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = fullShape;
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = V3(0) + V3(0,0,1);
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
	m_shadowInst->enabled = true;
	
	m_anLayers[0].anim = &m_anMainPlayer;
	m_anLayers[1].anim = &m_anTopPlayer;
	m_anLayers[2].anim = &m_animChar.m_layerAnimator;
	m_anLayers[2].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
	m_anLayers[3].anim = &m_animChar.m_anRagdoll;
	m_anLayers[3].tflags = AnimMixer::TF_Absolute_Pos
		| AnimMixer::TF_Absolute_Rot
		| AnimMixer::TF_IgnoreMeshXF;
	m_anLayers[3].factor = 0;
	m_animChar.m_anMixer.layers = m_anLayers;
	m_animChar.m_anMixer.layerCount = 4;
	
	m_shootPS.Load( "psys/gunflash.psy" );
	m_shootPS.AddToScene( m_level->GetScene() );
	m_shootPS.OnRenderUpdate();
	m_shootLT = m_level->GetScene()->CreateLight();
	m_shootLT->type = LIGHT_POINT;
	m_shootLT->enabled = false;
	m_shootLT->position = V3(0);
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*1;
	m_shootLT->range = 4;
	m_shootLT->power = 4;
	m_shootLT->UpdateTransform();
	m_shootTimeout = 0;
	m_timeSinceLastHit = 9999;
}

TSCharacter::~TSCharacter()
{
}

void TSCharacter::OnTransformUpdate()
{
	if( m_skipTransformUpdate )
		return;
	Vec3 pos = GetWorldPosition();
	m_bodyHandle->SetPosition( pos );
	m_ivPos.Set( pos );
}

void TSCharacter::SetPlayerMode( bool isPlayer )
{
	if( isPlayer )
		m_infoFlags = ( m_infoFlags & ~IEST_Target ) | IEST_Player;
	else
		m_infoFlags = ( m_infoFlags & ~IEST_Player ) | IEST_Target;
	ownerType = isPlayer ? GAT_Player : GAT_Enemy;
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
	
	ProcessAnims( 0 );
	AnimInfo info = { m_animChar.m_cachedMeshInst->matrix };
	m_animChar.m_anEnd.Advance( 0, &info );
	m_animChar.m_anEnd.Transfer();
}

void TSCharacter::ProcessAnims( float deltaTime )
{
	m_animTimeLeft -= deltaTime;
	
	// turning
	Vec2 rundir = V2( cosf( m_turnAngle ), sinf( m_turnAngle ) );
	Vec3 aimdir = V3( rundir.x, rundir.y, 0 );
	float aimspeed = GetInputV2( ACT_Chr_AimAt ).y;
	if( GetInputB( ACT_Chr_AimAt ) )
	{
		aimdir = GetInputV3( ACT_Chr_AimTarget ) - GetWorldPosition();
	}
	m_aimDir.TurnTo( YP( aimdir ), YP( aimspeed * deltaTime ) );
	m_aimDist = aimdir.Length();
	aimdir = m_aimDir.ToVec3();
	m_ivAimDir.Advance( V3( aimdir.x, aimdir.y, 0 ) );
	
	Vec2 totalShape2Offset = rundir.Normalized() * 0.2f + aimdir.ToVec2().Normalized() * 0.4f;
	m_bodyHandle->GetShape()->UpdateChildShapeTransform( 1,
		V3( totalShape2Offset.x, totalShape2Offset.y, 0 ) );
	m_bodyHandle->FlushContacts();
	
	// committing to animator
	if( Vec2Dot( rundir, aimdir.ToVec2() ) < -0.1f )
		TurnTo( aimdir.ToVec2(), 8 * deltaTime );
	
	float f_turn_btm = ( atan2( rundir.y, rundir.x ) - M_PI / 2 ) / ( M_PI * 2 );
	float f_turn_top = ( atan2( aimdir.y, aimdir.x ) - M_PI / 2 ) / ( M_PI * 2 );
	if( m_animTimeLeft > 0 )
	{
		f_turn_top = f_turn_btm;
	}
	for( size_t i = 0; i < m_animChar.layers.size(); ++i )
	{
		AnimCharacter::Layer& L = m_animChar.layers[ i ];
		if( L.name == StringView("turn_bottom") )
			L.amount = f_turn_btm;
		else if( L.name == StringView("turn_top") )
			L.amount = f_turn_top;
	}
	m_animChar.RecalcLayerState();
}

void TSCharacter::FixedTick( float deltaTime )
{
	Actor::FixedTick( deltaTime );
	
	if( IsInAction() )
	{
//		i_move = V2(0);
		if( m_actState.timeoutMoveToStart > 0 )
		{
			if( !IsPlayingAnim() )
				m_anMainPlayer.Play( GR_GetAnim( "stand_with_gun_up" ), false, 0.2f );
			
			m_bodyHandle->SetLinearVelocity( V3(0) );
			if( ( m_actState.info.placePos - GetWorldPosition() ).ToVec2().Length() < 0.1f )
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
				if( !IsPlayingAnim() )
					m_anMainPlayer.Play( GR_GetAnim( "kneeling" ) );
				m_actState.target->GetScriptedObject().thiscall( C, "OnActionStart" );
			}
		//	if( pp < 0.5f && 0.5f <= cp )
		//	{
		//	}
			
			m_actState.progress = cp;
			if( m_actState.progress >= m_actState.info.timeActual )
			{
				// end of action
				m_actState.target->GetScriptedObject().thiscall( C, "OnActionEnd" );
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
		float i_speed = GetInputV3( ACT_Chr_Move ).z;
		
		// animate character
		const char* anim_stand = m_isCrouching ? "crouch" : "stand_with_gun_up";
		const char* anim_walk_fw = m_isCrouching ? "crouch_walk" : "walk";
		const char* anim_walk_bw = m_isCrouching ? "crouch_walk_bw" : "walk_bw";
		const char* anim_run_fw = m_isCrouching ? "crouch_walk" : "run";
		const char* anim_run_bw = m_isCrouching ? "crouch_walk_bw" : "run_bw";
		
		if( i_speed < 0.5f )
		{
			anim_run_fw = anim_walk_fw;
			anim_run_bw = anim_walk_bw;
		}
		
		const char* animname = anim_run_fw;
		Vec2 md = i_move.Normalized();
		if( Vec2Dot( md, GetAimDir_FT().ToVec2() ) < 0 )
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
		
		if( i_move.Length() < 0.3f )
			animname = anim_stand;
		
		if( !IsPlayingAnim() )
			m_anMainPlayer.Play( GR_GetAnim( animname ), false, 0.2f );
	}
	
	HandleMovementPhysics( deltaTime );
	
	ProcessAnims( deltaTime );
	
	if( m_anMainPlayer.CheckMarker( "step" ) )
	{
		Vec3 pos = m_bodyHandle->GetPosition();
		Vec3 lvel = m_bodyHandle->GetLinearVelocity();
		SoundEventInstanceHandle fsev = m_level->GetSoundSys()->CreateEventInstance( "/footsteps" );
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
	Actor::Tick( deltaTime, blendFactor );
	
	Vec3 turn = GetInputV3( ACT_Chr_Turn );
	if( turn.z > 0 )
		TurnTo( turn.ToVec2(), turn.z * deltaTime );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->matrix = Mat4::CreateTranslation( pos ); // Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	m_level->LightMesh( MI, V3(0,0,m_isCrouching ? 0.6f : 1) );
	
	m_animChar.PreRender( blendFactor );
	m_skipTransformUpdate = true;
	SetWorldPosition( pos );
	m_skipTransformUpdate = false;
	m_interpAimDir = m_ivAimDir.Get( blendFactor );
	
	
	if( m_health > 0 )
	{
		Vec3 tgtpos = m_animChar.GetLocalAttachmentPos( m_animChar.FindAttachment( "target" ) );
		SetInfoMask( m_infoFlags );
		SetInfoTarget( tgtpos );
	}
	
	
	AnimDeformer& AnD = m_animChar.m_anDeformer;
	for( size_t i = 0; i < AnD.forces.size(); ++i )
	{
		AnimDeformer::Force& F = AnD.forces[ i ];
		float t = F.lifetime;
		F.amount = sinf( t * FLT_PI ) * expf( -t * 8 ) * 7 * F.dir.Length();
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
	if( GetInputB( ACT_Chr_Shoot ) && m_health > 0 && m_shootTimeout <= 0 )
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
		m_level->GetSystem<AIDBSystem>()->AddSound( GetWorldPosition(), 10, 0.2f, AIS_Shot );
		
		Mat4 inv = m_animChar.m_cachedMeshInst->matrix.Inverted();
		Vec3 forcePos = inv.TransformPos( origin );
		Vec3 forceDir = inv.TransformNormal( mtx.TransformNormal(V3(0,0,-1)) ).Normalized();
		AnD.AddLocalForce( forcePos, forceDir * 0.2f, 1.0f );
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
		
		SoundEventInstanceHandle fsev = m_level->GetSoundSys()->CreateEventInstance( "/footsteps" );
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
	float angroot = m_aimDir.yaw + M_PI;
	float angend = normalize_angle( turnDir.Angle() - angroot ) + angroot;
	float angstart = normalize_angle( m_turnAngle - angroot ) + angroot;
	m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), speedDelta );
}

void TSCharacter::PushTo( const Vec3& pos, float speedDelta )
{
	Vec2 diff = pos.ToVec2() - GetWorldPosition().ToVec2();
	diff = diff.Normalized() * TMIN( speedDelta, diff.Length() );
	m_bodyHandle->SetPosition( GetWorldPosition() + V3( diff.x, diff.y, 0 ) );
}

void TSCharacter::BeginClosestAction( float maxdist )
{
	if( IsInAction() )
		return;
	
	Vec3 QP = GetQueryPosition_FT();
	EntityGather iact_gather;
	m_level->QuerySphere( &iact_gather, IEST_InteractiveItem, QP, 5 );
	if( iact_gather.items.size() )
	{
		iact_gather.DistanceSort( QP );
		if( ( iact_gather.items[ 0 ].E->GetWorldInfoTarget() - QP ).Length() < maxdist )
			BeginAction( iact_gather.items[ 0 ].E );
	}
}

bool TSCharacter::BeginAction( Entity* E )
{
	if( !E || IsInAction() )
		return false;
	
#if 0
	IInteractiveEntity* IE = E->GetInterface<IInteractiveEntity>();
	if( IE == NULL || IE->GetInteractionInfo( GetQueryPosition_FT(), &m_actState.info ) == false )
		return false;
	
	m_actState.timeoutMoveToStart = 1;
	m_actState.progress = 0;
	m_actState.target = E;
	return true;
#endif
	
	sgs_PushVar( C, GetScriptedObject() );
	E->GetScriptedObject().thiscall( C, "OnInteract", 1 );
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

void TSCharacter::PlayAnim( StringView name, bool loop )
{
	AnimHandle anim = GR_GetAnim( name );
	if( anim == NULL )
	{
		LOG_WARNING << "TSCharacter::PlayAnim - anim not found: " << name;
		return;
	}
	m_animTimeLeft = loop ? FLT_MAX : anim->GetAnimTime();
	m_anMainPlayer.Play( anim );
	m_anTopPlayer.Stop();
}

void TSCharacter::StopAnim()
{
	if( m_animTimeLeft > 0 )
	{
		m_animTimeLeft = 0;
		// animation will be changed on next tick
	}
}

void TSCharacter::Reset()
{
	m_health = 100;
	m_animChar.DisablePhysics();
	m_anLayers[3].factor = 0;
	m_bodyHandle->SetEnabled( true );
	m_animChar.m_cachedMeshInst->layers = 0xffffffff;
}

void TSCharacter::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		Mat4 inv = Mat4::Identity;
		m_animChar.m_cachedMeshInst->matrix.InvertTo( inv );
		Vec3 pos = inv.TransformPos( bhinfo->pos );
		Vec3 vel = inv.TransformNormal( bhinfo->vel ).Normalized();
		m_animChar.m_anDeformer.AddModelForce( pos, vel * 0.4f, 0.3f );
		Hit( bhinfo->vel.Length() * 0.15f );
	}
}

void TSCharacter::OnDeath()
{
	m_bodyHandle->SetEnabled( false );
	m_animChar.EnablePhysics();
	m_anLayers[3].factor = 1;
	SetInfoMask( 0 );
	m_animChar.m_cachedMeshInst->layers = 0;
	
	// event
	Game_FireEvent( TSEV_CharDied, this );
}

void TSCharacter::Hit( float pwr )
{
//	float m_health = 1000000;
	if( m_health > 0 )
	{
		m_timeSinceLastHit = 0;
//		RenewAction();
		m_health -= pwr;
		
		// event
		{
			TSEventData_CharHit data = { this, pwr };
			Game_FireEvent( TSEV_CharHit, &data );
		}
		
		if( m_health <= 0 )
		{
			OnDeath();
		}
	}
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

Vec3 TSCharacter::sgsGetAttachmentPos( StringView atch, Vec3 off )
{
	return m_animChar.GetAttachmentPos( m_animChar.FindAttachment( atch ), off );
}



TSScriptedController::TSScriptedController( GameLevel* lev ) : m_level( lev )
{
	_data = lev->GetScriptCtx().CreateDict();
}

void TSScriptedController::FixedTick( float deltaTime )
{
	sgsVariable fn_fixedupdate = GetScriptedObject().getprop( "FixedUpdate" );
	if( fn_fixedupdate.not_null() )
	{
		sgs_PushReal( C, deltaTime );
		GetScriptedObject().thiscall( C, fn_fixedupdate, 1 );
	}
}

void TSScriptedController::Tick( float deltaTime, float blendFactor )
{
	sgsVariable fn_update = GetScriptedObject().getprop( "Update" );
	if( fn_update.not_null() )
	{
		sgs_PushReal( C, deltaTime );
		sgs_PushReal( C, blendFactor );
		GetScriptedObject().thiscall( C, fn_update, 2 );
	}
}

Vec3 TSScriptedController::GetInput( uint32_t iid )
{
	sgsVariable fn_getinput = GetScriptedObject().getprop( "GetInput" );
	if( fn_getinput.not_null() )
	{
		SGS_SCOPE;
		sgs_PushInt( C, iid );
		GetScriptedObject().thiscall( C, fn_getinput, 1, 1 );
		return sgs_GetVar<Vec3>()( C, -1 );
	}
	return V3(0);
}

void TSScriptedController::Reset()
{
	sgsVariable fn_reset = GetScriptedObject().getprop( "Reset" );
	if( fn_reset.not_null() )
		GetScriptedObject().thiscall( C, fn_reset );
}

sgsVariable TSScriptedController::Create( SGS_CTX, GameLevelScrHandle lev )
{
	if( !lev )
	{
		sgs_Msg( C, SGS_WARNING, "argument 1 must be GameLevel" );
		return sgsVariable();
	}
	SGS_CREATECLASS( C, NULL, TSScriptedController, ( lev ) );
	return sgsVariable( C, sgsVariable::PickAndPop );
}



TSAimHelper::TSAimHelper( GameLevel* lev ) :
	m_level(lev), m_pos(V3(0)), m_cp(V2(0)), m_aimPtr(NULL),
	m_aimPoint(V3(0)), m_rcPoint(V3(0)), m_aimFactor(0), m_pDist(0),
	m_closestEnt(NULL), m_closestPoint(V3(0))
{
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
		lock = m_level->QuerySphere( this, IEST_Target, pos, 8.0f );
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

bool TSAimHelper::ProcessEntity( Entity* E )
{
	if( E->GetInfoMask() & IEST_Player )
		return true;
	
	Vec3 tgtPos = E->GetWorldInfoTarget();
	SceneRaycastCallback_Any srcb;
	m_level->GetScene()->RaycastAll( m_pos, tgtPos, &srcb, 0x1 );
	if( srcb.m_hit )
		return true;
	
	if( E == m_aimPtr )
	{
		m_closestEnt = m_aimPtr;
		m_closestPoint = tgtPos;
		return false;
	}
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	float bsz = TMIN( screen_size.x, screen_size.y );
	Vec2 curpos = m_cp * screen_size;
	Vec2 scrpos = m_level->GetScene()->camera.WorldToScreen( tgtPos ).ToVec2() * screen_size;
	float npdist = ( curpos - scrpos ).Length();
	if( npdist < m_pDist * bsz )
	{
		m_closestEnt = E;
		m_closestPoint = tgtPos;
		m_pDist = npdist / bsz;
	}
	return true;
}



TSPlayerController::TSPlayerController( GameLevel* lev ) :
	m_aimHelper( lev ), i_move( V2(0) ), i_aim_target( V3(0) ), i_turn( V3(0) )
{
}

void TSPlayerController::Tick( float deltaTime, float blendFactor )
{
	if( m_entity && ENTITY_IS_A( m_entity, TSCharacter ) )
	{
		SGRX_CAST( TSCharacter*, P, m_entity );
		Vec3 pos = P->GetQueryPosition();
		Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
		m_aimHelper.Tick( deltaTime, pos, CURSOR_POS / screen_size, LOCK_ON.value > 0.5f );
	}
	
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
	case ACT_Chr_AimAt: return V3( 1 /* yes */, 32 /* speed */, 0 );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3(SHOOT.value);
	case ACT_Chr_DoAction: return V3(DO_ACTION.value);
	}
	return V3(0);
}

void TSPlayerController::CalcUIAimInfo()
{
	m_aimHelper.m_pDist = 0.5f;
	m_aimHelper.m_closestEnt = NULL;
	m_aimHelper.m_level->QuerySphere( &m_aimHelper, IEST_Target, m_aimHelper.m_pos, 8.0f );
}

sgsVariable TSPlayerController::Create( SGS_CTX, GameLevelScrHandle lev )
{
	if( !lev )
	{
		sgs_Msg( C, SGS_WARNING, "argument 1 must be GameLevel" );
		return sgsVariable();
	}
	SGS_CREATECLASS( C, NULL, TSPlayerController, ( lev ) );
	return sgsVariable( C, sgsVariable::PickAndPop );
}



TPSPlayerController::TPSPlayerController( GameLevel* lev ) :
	m_level( lev ), m_angles( YP(0) ),
	i_move( V2(0) ), i_aim_target( V3(0) ), i_turn( V3(0) )
{
	m_castShape = lev->GetPhyWorld()->CreateSphereShape( 0.2f );
}

void TPSPlayerController::Tick( float deltaTime, float blendFactor )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	Vec2 joystick_aim = V2( AIM_X.value, AIM_Y.value );
	joystick_aim = -joystick_aim.Normalized() *
		TCLAMP( TREVLERP<float>( 0.2f, 1.0f, joystick_aim.Length() ), 0.0f, 1.0f );
	m_angles.yaw += joystick_aim.x * 10 * deltaTime;
	m_angles.pitch += joystick_aim.y * 10 * deltaTime;
	m_angles.pitch = TCLAMP( m_angles.pitch, -FLT_PI/3.0f, FLT_PI/3.0f );
	
	Vec2 move = V2
	(
		MOVE_X.value + MOVE_RIGHT.value - MOVE_LEFT.value,
		MOVE_Y.value + MOVE_DOWN.value - MOVE_UP.value
	);
	SGRX_Scene* scene = m_level->GetScene();
	SGRX_Camera& CAM = scene->camera;
	
	// raycast for direction
	i_aim_target = CAM.position + CAM.direction.Normalized() * 1000;
	{
		Vec3 pos, dir;
		if( scene->camera.GetCursorRay( 0.5f, 0.5f, pos, dir ) )
		{
			Vec3 start = pos;
			Vec3 end = pos + dir * scene->camera.zfar;
			
			// center plane
			Vec3 ppos = chr->GetWorldPosition();
			Vec2 planedir = ( ppos - scene->camera.position ).ToVec2().Normalized();
			Vec3 planedir3 = V3( planedir.x, planedir.y, 0 );
			Vec4 plane = V4( planedir3, Vec3Dot( planedir3, ppos ) + 0.1f ); // safety offset
			
			// offset ray up to center plane
			float dsts[2];
			if( RayPlaneIntersect( start, dir, plane, dsts ) )
			{
				start += dir * dsts[0];
			}
			
			SceneRaycastInfo hitinfo;
			if( scene->RaycastOne( start, end, &hitinfo, 0xffffffff ) )
			{
				i_aim_target = TLERP( start, end, hitinfo.factor );
			}
		}
	}
	
	Vec2 fwd = CAM.direction.ToVec2().Normalized();
	Vec2 rgt = CAM.GetRight().ToVec2().Normalized();
	i_move = fwd * -move.y + rgt * move.x;
	i_turn = V3(0);
	if( i_move.Length() > 0.1f )
	{
		Vec2 md = i_move;
		if( Vec2Dot( ( i_aim_target - chr->GetWorldPosition() ).ToVec2(), md ) < 0 )
			md = -md;
		i_turn = V3( md.x, md.y, 8 );
	}
}

Vec3 TPSPlayerController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, 1 );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3(CROUCH.value);
	case ACT_Chr_AimAt: return V3( 1 /* yes */, 32 /* speed */, 0 );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3(SHOOT.value);
	case ACT_Chr_DoAction: return V3(DO_ACTION.value);
	}
	return V3(0);
}

void TPSPlayerController::SafePosPush( Vec3& pos, Vec3 dir )
{
	SGRX_PhyRaycastInfo rcinfo;
	if( m_level->GetPhyWorld()->ConvexCast( m_castShape,
			pos, pos + dir, 0x0001, 0x0001, &rcinfo ) )
	{
		pos += dir * rcinfo.factor;
	}
	else
	{
		pos += dir;
	}
}

Vec3 TPSPlayerController::GetCameraPos()
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return V3(0);
	Vec3 campos = chr->GetWorldPosition();
	campos += V3(0,0,1);
	SafePosPush( campos, V3(0,0,1) );
	SafePosPush( campos, -m_angles.ToVec3() );
	return campos;
}

sgsVariable TPSPlayerController::Create( SGS_CTX, GameLevelScrHandle lev )
{
	if( !lev )
	{
		sgs_Msg( C, SGS_WARNING, "argument 1 must be GameLevel" );
		return sgsVariable();
	}
	SGS_CREATECLASS( C, NULL, TPSPlayerController, ( lev ) );
	return sgsVariable( C, sgsVariable::PickAndPop );
}



TSEnemyController::TSEnemyController( GameLevel* lev ) :
	m_level( lev ),
	i_crouch( false ), i_move( V2(0) ), i_speed( 1 ), i_turn( V3(0) ),
	i_aim_at( false ), i_aim_target( V3(0) ), i_shoot( false ), i_act( false ),
	m_inPlayerTeam( false ),
	m_aidb( m_level->GetSystem<AIDBSystem>() ),
	m_coverSys( m_level->GetSystem<CoverSystem>() )
{
	_data = m_level->GetScriptCtx().CreateDict();
	GetScriptedObject().thiscall( C, "Init" );
}

TSEnemyController::~TSEnemyController()
{
}

struct TSEC_FindChar : AIFactDistance
{
	float GetDistance( const AIFact& fact )
	{
		if( fact.type != type )
			return FLT_MAX;
		
		float dist = PointLineDistance( pos,
			fact.position - V3(0,0,0.5f), fact.position + V3(0,0,0.5f) ) - 0.5f;
		float maxdist = ( curTime - fact.created ) * 0.001f * speed;
		if( dist > maxdist )
			return FLT_MAX;
		return dist;
	}
	Vec3 GetPosition(){ return pos; }
	
	uint32_t type;
	Vec3 pos;
	float speed;
	uint32_t curTime;
};

struct EPEnemyViewProc : EntityProcessor
{
	EPEnemyViewProc() : sawEnemy(false){}
	bool ProcessEntity( Entity* ent )
	{
		Vec3 enemypos = ent->GetWorldInfoTarget();
		
		// verify the find
		if( enemy->CanSeePoint( enemypos ) == false )
		{
			if( ENTITY_IS_A( ent, TSCharacter ) == false )
				return true;
			if( enemy->CanSeePoint( ((TSCharacter*)ent)->sgsGetAttachmentPos( "head", V3(0) ) ) == false )
				return true;
		}
		
		// TODO friendlies
		AIFactStorage& FS = enemy->m_factStorage;
		
		if( ent->GetInfoMask() & IEST_AIAlert )
		{
			FS.InsertOrUpdate( FT_Sight_Alarming,
				enemypos, 0, curtime, curtime + 5*1000, 0 );
		}
		else if( gcv_notarget.value == false )
		{
			sawEnemy = true;
			
			TSEC_FindChar fchr;
			{
				fchr.type = FT_Sight_Foe,
				fchr.pos = enemypos;
				fchr.speed = 10;
				fchr.curTime = curtime;
			}
			
			// fact of seeing
			FS.CustomInsertOrUpdate( fchr, FT_Sight_Foe,
				curtime, curtime + 5*1000 );
			
			// fact of position
			fchr.type = FT_Position_Foe;
			FS.CustomInsertOrUpdate( fchr, FT_Position_Foe,
				curtime, curtime + 30*1000, FS.last_mod_id );
		}
		
		return true;
	}
	
	TimeVal curtime;
	TSEnemyController* enemy;
	bool sawEnemy;
};

struct TSEC_FindSoundSource : AIFactDistance
{
	float GetDistance( const AIFact& fact )
	{
		if( fact.type != FT_Position_Friend && fact.type != FT_Position_Foe )
			return FLT_MAX;
		
		float dist = PointLineDistance( pos,
			fact.position - V3(0,0,0.5f), fact.position + V3(0,0,0.5f) ) - 0.5f;
		float maxdist = ( curTime - fact.created ) * 0.001f * speed;
		if( dist > maxdist )
			return FLT_MAX;
		return dist;
	}
	Vec3 GetPosition(){ return pos; }
	
	Vec3 pos;
	float speed;
	uint32_t curTime;
};

void TSEnemyController::FixedTick( float deltaTime )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	TimeVal curTime = m_level->GetPhyTime();
	
	// process facts
	m_factStorage.Process( curTime );
	// - self
	TSEC_FindChar fchr;
	{
		fchr.type = FT_Position_Friend;
		fchr.pos = chr->GetPosition_FT();
		fchr.speed = 10;
		fchr.curTime = curTime;
	}
	m_factStorage.CustomInsertOrUpdate( fchr, FT_Position_Friend,
		curTime, curTime + 1*1000, 0 );
	// - sounds
	for( int i = 0; i < m_aidb->GetNumSounds(); ++i )
	{
		if( m_aidb->CanHearSound( chr->GetPosition_FT(), i ) == false )
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
			TSEC_FindSoundSource fss;
			{
				fss.pos = S.position;
				fss.speed = 10;
				fss.curTime = curTime;
			}
			m_factStorage.CustomUpdate( fss, curTime, curTime + 30*1000, lastid );
		}
		else
		{
			m_factStorage.InsertOrUpdate( FT_Sound_Noise,
				S.position, 1, curTime, curTime + 1*1000 );
		}
	}
	// - vision
	EPEnemyViewProc evp;
	evp.curtime = curTime;
	evp.enemy = this;
	uint32_t qmask = ( m_inPlayerTeam ? IEST_Target : IEST_Player ) | IEST_AIAlert;
	m_level->QuerySphere( &evp, qmask, chr->GetPosition_FT(), 10.0f );
	
	// tick ESO
	m_level->m_scriptCtx.Push( deltaTime );
	GetScriptedObject().thiscall( C, "tick", 1 );
	
	i_crouch = _data[ "i_crouch" ].get<bool>();
	i_move = _data[ "i_move" ].get<Vec2>();
	i_speed = _data[ "i_speed" ].get<float>();
	i_turn = _data[ "i_turn" ].get<Vec3>();
	sgsVariable aim_target = _data[ "i_aim_target" ];
	i_aim_at = aim_target.not_null();
	i_aim_target = aim_target.get<Vec3>();
	i_shoot = _data[ "i_shoot" ].get<bool>();
	i_act = _data[ "i_act" ].get<bool>();
}

void TSEnemyController::Tick( float deltaTime, float blendFactor )
{
	UNUSED( deltaTime );
	UNUSED( blendFactor );
}

Vec3 TSEnemyController::GetInput( uint32_t iid )
{
	switch( iid )
	{
	case ACT_Chr_Move: return V3( i_move.x, i_move.y, i_speed );
	case ACT_Chr_Turn: return i_turn;
	case ACT_Chr_Crouch: return V3( i_crouch );
	case ACT_Chr_AimAt: return V3( i_aim_at /* y/n */, 8 /* speed */, 0 );
	case ACT_Chr_AimTarget: return i_aim_target;
	case ACT_Chr_Shoot: return V3( i_shoot );
	case ACT_Chr_DoAction: return V3( i_act );
	}
	return V3(0);
}

void TSEnemyController::Reset()
{
	m_factStorage.Clear();
	GetScriptedObject().thiscall( C, "Reset" );
}

void TSEnemyController::DebugDrawWorld()
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	if( gcv_dbg_aifacts.value )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset().Col( 0.9f, 0.2f, 0.1f );
		Vec3 pos = chr->GetWorldPosition();
		
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
}

void TSEnemyController::DebugDrawUI()
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	Vec3 pos = chr->GetWorldPosition();
	bool infront;
	Vec3 screenpos = m_level->GetScene()->camera.WorldToScreen( pos, &infront );
	if( !infront )
		return;
	int x = screenpos.x * GR_GetWidth();
	int y = screenpos.y * GR_GetHeight();
	
	GR2D_SetFont( "core", 12 );
	GR2D_SetFont( "mono", 12 );
	
	if( gcv_dbg_aifacts.value )
	{
		char bfr[ 256 ];
		
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
	if( gcv_dbg_aistack.value )
	{
		char bfr[ 512 ];
		
		sgs_Context* T = _data.getprop("coro_ai_main").var.data.T;
		sgs_StackFrame* sf = sgs_GetFramePtr( T, NULL, 0 );
		while( sf )
		{
			int line;
			const char *name, *file;
			sgs_StackFrameInfo( T, sf, &name, &file, &line );
			sgrx_snprintf( bfr, 512, "%.240s at %.240s:%d", name, file, line );
			sf = sgs_GetFramePtr( T, sf, 0 );
			
			int len = GR2D_GetTextLength( bfr );
			br.Reset().Col( 0.0f, 0.5f ).Quad( x, y, x + len, y + 12 );
			br.Col( 1.0f );
			GR2D_DrawTextLine( x, y, bfr );
			
			y += 13;
		}
	}
}

bool TSEnemyController::CanSeePoint( Vec3 pt )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return false;
	
	Vec3 vieworigin = chr->GetPosition_FT();
	Vec3 viewdir = chr->GetAimDir_FT();
	Vec3 view2pos = pt - vieworigin;
	
	// check immediate proximity
	if( view2pos.ToVec2().Length() < 0.5f && fabsf( view2pos.z ) < 1 )
		return true;
	
	// increase vertical FOV
	viewdir.z *= 0.5f;
	view2pos.z *= 0.5f;
	
	float vpdot = Vec3Dot( viewdir.Normalized(), view2pos.Normalized() );
	if( vpdot < cosf(DEG2RAD(40.0f)) )
		return false; // outside view cone
	
	if( m_level->GetPhyWorld()->Raycast( vieworigin, pt, 1, 1 ) )
		return false; // behind wall
	
	return true;
}

bool TSEnemyController::LookingAtPoint( Vec3 pt )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return false;
	
	Vec3 vieworigin = chr->GetPosition_FT();
	Vec3 viewdir = chr->GetAimDir_FT();
	Vec3 view2pos = pt - vieworigin;
	
	// increase vertical FOV
	viewdir.z *= 0.5f;
	view2pos.z *= 0.5f;
	
	float vpdot = Vec3Dot( viewdir.Normalized(), view2pos.Normalized() );
	return vpdot >= cosf(DEG2RAD(40.0f));
}

bool TSEnemyController::sgsHasFact( uint32_t typemask )
{
	return m_factStorage.HasFact( typemask );
}

bool TSEnemyController::sgsHasRecentFact( uint32_t typemask, TimeVal maxtime )
{
	return m_factStorage.HasRecentFact( typemask, maxtime );
}

SGS_MULTRET TSEnemyController::sgsGetRecentFact( uint32_t typemask, TimeVal maxtime )
{
	AIFact* F = m_factStorage.GetRecentFact( typemask, maxtime );
	if( F )
	{
		sgs_CreateLiteClassFrom( C, NULL, F );
		return 1;
	}
	return 0;
}

void TSEnemyController::sgsInsertFact( uint32_t type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	m_factStorage.Insert( type, pos, created, expires, ref );
}

bool TSEnemyController::sgsUpdateFact( uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( C ) < 7 )
		reset = true;
	return m_factStorage.Update( type, pos, rad, created, expires, ref, reset );
}

void TSEnemyController::sgsInsertOrUpdateFact( uint32_t type, Vec3 pos,
	float rad, TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( sgs_StackSize( C ) < 7 )
		reset = true;
	m_factStorage.InsertOrUpdate( type, pos, rad, created, expires, ref, reset );
}

void TSEnemyController::sgsQueryCoverLines( Vec3 bbmin,
	Vec3 bbmax, float dist, float height, Vec3 viewer, bool visible )
{
	if( m_coverSys == NULL )
		return;
	m_coverSys->QueryLines( bbmin, bbmax, dist, height, viewer, visible, m_coverInfo );
	// TODO clip by facts
}

sgsMaybe<Vec3> TSEnemyController::sgsGetCoverPosition(
	Vec3 position, float distpow, float interval /* = 0.1 */ )
{
	if( sgs_StackSize( C ) < 3 )
		interval = 0.1f;
	Vec3 out = V3(0);
	if( m_coverInfo.GetPosition( position, distpow, out, interval ) )
		return out;
	return sgsMaybe<Vec3>();
}

bool TSEnemyController::sgsIsWalkable( Vec3 pos, Vec3 ext )
{
	return m_aidb->m_pathfinder.FindPoly( pos, NULL, ext ) != 0;
}

bool TSEnemyController::sgsFindPath( const Vec3& to )
{
	TSCharacter* chr = GetChar();
	if( !chr )
		return false;
	
	m_aidb->m_pathfinder.FindPath( chr->GetPosition_FT(), to, m_path );
	return m_path.size() > 0;
}

bool TSEnemyController::sgsHasPath()
{
	return m_path.size() != 0;
}

int TSEnemyController::sgsGetPathPointCount()
{
	return m_path.size();
}

bool TSEnemyController::sgsAdvancePath( float dist )
{
	while( dist > 0 && m_path.size() >= 2 )
	{
		float len = ( m_path[0] - m_path[1] ).Length();
		if( dist > len )
		{
			dist -= len;
			m_path.erase(0);
		}
		else
		{
			m_path[0] = TLERP( m_path[0], m_path[1], dist / len );
			break;
		}
	}
	if( m_path.size() == 1 )
		m_path.clear();
	return m_path.size() != 0;
}

sgsMaybe<Vec3> TSEnemyController::sgsGetNextPathPoint()
{
	if( m_path.size() )
		return m_path[ 0 ];
	return sgsMaybeNot;
}

bool TSEnemyController::sgsRemoveNextPathPoint()
{
	if( m_path.size() )
	{
		m_path.erase( 0 );
		return true;
	}
	return false;
}

sgsVariable TSEnemyController::Create( SGS_CTX, GameLevelScrHandle lev )
{
	if( !lev )
	{
		sgs_Msg( C, SGS_WARNING, "argument 1 must be GameLevel" );
		return sgsVariable();
	}
	SGS_CREATECLASS( C, NULL, TSEnemyController, ( lev ) );
	return sgsVariable( C, sgsVariable::PickAndPop );
}



static sgs_RegIntConst g_ts_ints[] =
{
	{ "ACT_Chr_Move", ACT_Chr_Move },
	{ "ACT_Chr_Turn", ACT_Chr_Turn },
	{ "ACT_Chr_Crouch", ACT_Chr_Crouch },
	{ "ACT_Chr_AimAt", ACT_Chr_AimAt },
	{ "ACT_Chr_AimTarget", ACT_Chr_AimTarget },
	{ "ACT_Chr_Shoot", ACT_Chr_Shoot },
	{ "ACT_Chr_DoAction", ACT_Chr_DoAction },
	{ NULL, 0 },
};

TSGameSystem::TSGameSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	register_tsent_cvars(); // TODO global init?
	m_level->GetScriptCtx().SetGlobal( "TSScriptedController",
		sgs_GetClassInterface<TSScriptedController>( m_level->GetSGSC() ) );
	m_level->GetScriptCtx().SetGlobal( "TSPlayerController",
		sgs_GetClassInterface<TSPlayerController>( m_level->GetSGSC() ) );
	m_level->GetScriptCtx().SetGlobal( "TPSPlayerController",
		sgs_GetClassInterface<TPSPlayerController>( m_level->GetSGSC() ) );
	m_level->GetScriptCtx().SetGlobal( "TSEnemyController",
		sgs_GetClassInterface<TSEnemyController>( m_level->GetSGSC() ) );
	m_level->GetScriptCtx().Include( "data/enemy" );
	
	sgs_RegIntConsts( m_level->GetSGSC(), g_ts_ints, -1 );
	
	lev->RegisterNativeEntity<TSCharacter>( "TSCharacter" );
}

Entity* TSGameSystem::AddEntity( StringView type )
{
#if 0
	///////////////////////////
	if( type == "camera" )
	{
		return new TSCamera( m_level );
#if 0
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
#endif
	}
#endif
	if( type == "TSCharacter" ) return new TSCharacter( m_level );
	return NULL;
}


