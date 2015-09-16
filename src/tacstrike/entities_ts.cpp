

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
		ppos = static_cast<TSPlayer*>(m_level->m_player)->GetPosition();
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
	
	float f_turn_h = clamp( normalize_angle2( m_curDir.yaw ), -M_PI * 0.4f, M_PI * 0.4f ) / M_PI;
	float f_turn_v = clamp( normalize_angle2( m_curDir.pitch ), -M_PI * 0.25f, M_PI * 0.25f ) / M_PI;
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
	
	InfoEmissionSystem::Data D = {
		m_animChar.m_cachedMeshInst->matrix.TransformPos( V3(0) ), 0.5f, IEST_MapItem };
	m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
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
}

bool TSCamera::GetMapItemInfo( MapItemInfo* out )
{
	Mat4 mtx;
	m_animChar.GetAttachmentMatrix( 0, mtx );
	
	out->type = MI_Object_Camera;
	if( m_alertTimeout > 0 ) out->type |= MI_State_Alerted;
	else if( m_noticeTimeout > 0 ) out->type |= MI_State_Suspicious;
	else out->type |= MI_State_Normal;
	
	out->position = mtx.TransformPos( V3(0) );
	out->direction = mtx.TransformNormal( V3(1,0,0) );
	out->sizeFwd = 10;
	out->sizeRight = 6;
	return true;
}


TSCharacter::TSCharacter( GameLevel* lev, const Vec3& pos, const Vec3& dir ) :
	Entity( lev ),
	m_animChar( lev->GetScene(), lev->GetPhyWorld() ),
	m_footstepTime(0), m_isCrouching(false), m_isOnGround(false),
	m_ivPos( pos ), m_ivAimDir( dir ),
	m_position( pos ), m_moveDir( V2(0) ), m_turnAngle( atan2( dir.y, dir.x ) ),
	i_crouch( false ), i_move( V2(0) ), i_speed( 1 ), i_aim_at( false ), i_aim_target( V3(0) )
{
	m_meshInstInfo.typeOverride = "*human*";
	
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
	m_shadowInst->projectionShader = GR_GetPixelShader( "mtl:proj_default:base_proj" );
	m_shadowInst->projectionTextures[0] = GR_GetTexture( "textures/fx/blobshadow.png" );//GR_GetTexture( "textures/unit.png" );
	m_shadowInst->projectionTextures[1] = GR_GetTexture( "textures/fx/projfalloff2.png" );
	
	m_anLayers[0].anim = &m_anMainPlayer;
	m_anLayers[1].anim = &m_anTopPlayer;
	m_anLayers[2].anim = &m_animChar.m_layerAnimator;
	m_anLayers[2].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
	m_anLayers[3].anim = &m_animChar.m_anRagdoll;
	m_anLayers[3].tflags = AnimMixer::TF_Absolute_Pos | AnimMixer::TF_Absolute_Rot;
	m_anLayers[3].factor = 0;
	m_animChar.m_anMixer.layers = m_anLayers;
	m_animChar.m_anMixer.layerCount = 3;
}

void TSCharacter::InitializeMesh( const StringView& path )
{
	m_animChar.Load( path );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->userData = &m_meshInstInfo;
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
	if( IsInAction() )
	{
		i_move = V2(0);
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
		// animate character
		const char* anim_stand = i_crouch ? "crouch" : "stand_with_gun_up";
		const char* anim_run_fw = i_crouch ? "crouch_walk" : "run";
		const char* anim_run_bw = i_crouch ? "crouch_walk_bw" : "run_bw";
		
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
	if( i_aim_at )
	{
		aimdir = ( i_aim_target - GetPosition() ).ToVec2();
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
}

void TSCharacter::Tick( float deltaTime, float blendFactor )
{
	Vec3 pos = m_ivPos.Get( blendFactor );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->matrix = Mat4::CreateTranslation( pos ); // Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	m_level->LightMesh( MI, V3(0,0,i_crouch ? 0.6f : 1) );
	
	m_animChar.PreRender( blendFactor );
	m_interpPos = m_ivPos.Get( blendFactor );
	m_interpAimDir = m_ivAimDir.Get( blendFactor );
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
	m_isCrouching = i_crouch;
	if( m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		m_level->GetPhyWorld()->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
		fabsf( rcinfo.point.z - rcinfo2.point.z ) < 1.8f )
	{
		m_isCrouching = 1;
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
	
	Vec2 md = i_move;
	if( md.LengthSq() > 1 )
		md.Normalize();
	
	Vec2 lvel2 = lvel.ToVec2();
	
	float maxspeed = 5 * i_speed;
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
	return m_actState.target;
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
	if( i_aim_at )
	{
		aimdir = ( i_aim_target - GetPosition() );
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



#ifndef TSGAME_NO_PLAYER

TSPlayer::TSPlayer( GameLevel* lev, const Vec3& pos, const Vec3& dir ) :
	TSCharacter( lev, pos-V3(0,0,1), dir ),
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false ),
	m_crouchIconShowTimeout( 0 ), m_standIconShowTimeout( 1 )
{
	InitializeMesh( "chars/tstest.chr" );
	
	m_meshInstInfo.ownerType = GAT_Player;
	
	m_tex_cursor = GR_GetTexture( "ui/crosshair.png" );
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

void TSPlayer::FixedTick( float deltaTime )
{
	i_move = V2
	(
		-MOVE_X.value + MOVE_LEFT.value - MOVE_RIGHT.value,
		MOVE_Y.value + MOVE_DOWN.value - MOVE_UP.value
	);
	i_aim_target = FindTargetPosition();
	if( i_move.Length() > 0.1f )
	{
		Vec2 md = i_move;
		if( Vec2Dot( ( i_aim_target - GetPosition() ).ToVec2(), md ) < 0 )
			md = -md;
		TurnTo( md, deltaTime * 8 );
	}
//	i_crouch = CROUCH.value;
	
	if( DO_ACTION.value )
	{
		BeginClosestAction( 2 );
	}
	
	TSCharacter::FixedTick( deltaTime );
}

void TSPlayer::Tick( float deltaTime, float blendFactor )
{
	m_crouchIconShowTimeout -= deltaTime;
	m_standIconShowTimeout -= deltaTime;
	if( CROUCH.IsPressed() )
	{
		i_crouch = !i_crouch;
#if 1
		m_crouchIconShowTimeout = i_crouch ? 2 : 0;
		m_standIconShowTimeout = i_crouch ? 0 : 2;
#else
		if( i_crouch )
			m_crouchIconShowTimeout = 2;
		else
			m_standIconShowTimeout = 2;
#endif
	}
	m_crouchIconShowTimeout = TMAX( m_crouchIconShowTimeout, i_crouch ? 1.0f : 0.0f );
	m_standIconShowTimeout = TMAX( m_standIconShowTimeout, i_crouch ? 0.0f : 1.0f );
	
	TSCharacter::Tick( deltaTime, blendFactor );
	
	m_angles += inCursorMove * V2(-0.01f);
	m_angles.y = clamp( m_angles.y, (float) -M_PI/2 + SMALL_FLOAT, (float) M_PI/2 - SMALL_FLOAT );
	
//	float ch = cosf( m_angles.x ), sh = sinf( m_angles.x );
//	float cv = cosf( m_angles.y ), sv = sinf( m_angles.y );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
//	Vec3 dir = V3( ch * cv, sh * cv, sv );
	m_position = pos;
	
	float bmsz = ( GR_GetWidth() + GR_GetHeight() );// * 0.5f;
	Vec2 cursor_pos = CURSOR_POS;
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	Vec2 player_pos = m_level->GetScene()->camera.WorldToScreen( m_position ).ToVec2() * screen_size;
	Vec2 diff = ( cursor_pos - player_pos ) / bmsz;
	
	m_level->GetScene()->camera.znear = 0.1f;
	m_level->GetScene()->camera.angle = 90;
	m_level->GetScene()->camera.updir = V3(0,-1,0);
	m_level->GetScene()->camera.direction = V3(-diff.x,diff.y,-5);
	m_level->GetScene()->camera.position = pos + V3(-diff.x,diff.y,0) * 2 + V3(0,0,1) * 6;
	m_level->GetScene()->camera.UpdateMatrices();
	
	InfoEmissionSystem::Data D = { pos, 0.5f, IEST_HeatSource | IEST_Player };
	m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
	
	
	m_shootLT->enabled = false;
	if( m_shootTimeout > 0 )
		m_shootTimeout -= deltaTime;
	if( SHOOT.value && m_shootTimeout <= 0 )
	{
		Mat4 mtx = GetBulletOutputMatrix();
		Vec3 origin = mtx.TransformPos( V3(0) );
		Vec3 dir = ( i_aim_target - origin ).Normalized();
		dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
		m_level->GetSystem<BulletSystem>()->Add( origin, dir * 100, 1, 1, m_meshInstInfo.ownerType );
		m_shootPS.SetTransform( mtx );
		m_shootPS.Trigger();
		m_shootLT->position = origin;
		m_shootLT->UpdateTransform();
		m_shootLT->enabled = true;
		m_shootTimeout += 0.1f;
		m_level->GetSystem<AIDBSystem>()->AddSound( GetPosition(), 10, 0.2f, AIS_Shot );
	}
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*0.5f * smoothlerp_oneway( m_shootTimeout, 0, 0.1f );
	
	
	MapItemInfo mymapitem = { MI_Object_Player, GetPosition(), V3(0), 0, 0 };
	m_level->GetSystem<LevelMapSystem>()->UpdateItem( this, mymapitem );
	m_level->GetSystem<LevelMapSystem>()->m_viewPos = GetPosition().ToVec2();
	
	
	m_shootPS.Tick( deltaTime );
	m_shootPS.PreRender();
}

void TSPlayer::DrawUI()
{
	SGRX_FontSettings fs;
	GR2D_GetFontSettings( &fs );
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
	Vec2 cursor_pos = CURSOR_POS;
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	Vec2 player_pos = m_level->GetScene()->camera.WorldToScreen( m_position ).ToVec2() * screen_size;
	
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
	
	if( m_targetII )
	{
		float x = GR_GetWidth() / 2.0f;
		float y = GR_GetHeight() / 2.0f;
		br.Reset().SetTexture( m_tex_interact_icon ).QuadWH( x, y, bsz / 10, bsz / 10 );
	}
	
	float cursor_size = bsz / 20;
	float cursor_angle = ( cursor_pos - player_pos ).Angle() + M_PI;
	br.Reset().SetTexture( m_tex_cursor ).TurnedBox(
		cursor_pos.x, cursor_pos.y, cosf( cursor_angle ) * cursor_size, sinf( cursor_angle ) * cursor_size );
	
	GR2D_SetFont( "tsicons", bsz * 0.2f );
	br.Col( 1, 0.25f * m_crouchIconShowTimeout );
	GR2D_DrawTextLine( round( bsz * 0.1f ), round( screen_size.y - bsz * 0.1f ), "\x0a", HALIGN_LEFT, VALIGN_BOTTOM );
	br.Col( 1, 0.25f * m_standIconShowTimeout );
	GR2D_DrawTextLine( round( bsz * 0.1f ), round( screen_size.y - bsz * 0.1f ), "\x0b", HALIGN_LEFT, VALIGN_BOTTOM );
	
	GR2D_SetFontSettings( &fs );
}

Vec3 TSPlayer::FindTargetPosition()
{
	Vec3 crpos, crdir;
	Vec2 crsp = CURSOR_POS / Game_GetScreenSize();
	m_level->GetScene()->camera.GetCursorRay( crsp.x, crsp.y, crpos, crdir );
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
	if( RayPlaneIntersect( crpos, crdir, V4(0,0,1.1f,GetPosition().z), dsts ) )
	{
		return crpos + crdir * dsts[0];
	}
	
	return V3(0);
}

#endif



TSFactStorage::TSFactStorage() : m_lastTime(0), last_mod_id(0), m_next_fact_id(1)
{
}

static bool sort_facts_created_desc( const void* pa, const void* pb, void* )
{
	SGRX_CAST( TSFactStorage::Fact*, fa, pa );
	SGRX_CAST( TSFactStorage::Fact*, fb, pb );
	return fa->created > fb->created;
}

void TSFactStorage::SortCreatedDesc()
{
	sgrx_combsort( facts.data(), facts.size(), sizeof(facts[0]), sort_facts_created_desc, NULL );
}

static bool sort_facts_expires_desc( const void* pa, const void* pb, void* )
{
	SGRX_CAST( TSFactStorage::Fact*, fa, pa );
	SGRX_CAST( TSFactStorage::Fact*, fb, pb );
	return fa->expires > fb->expires;
}

void TSFactStorage::Process( TimeVal curTime )
{
	m_lastTime = curTime;
	
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].expires < curTime )
		{
			facts.uerase( i-- );
			break;
		}
	}
	
	if( facts.size() > 256 )
	{
		sgrx_combsort( facts.data(), facts.size(), sizeof(facts[0]), sort_facts_expires_desc, NULL );
		facts.resize( 256 );
	}
}

bool TSFactStorage::HasFact( int typemask )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( (1<<facts[ i ].type) & typemask )
			return true;
	}
	return false;
}

bool TSFactStorage::HasRecentFact( int typemask, TimeVal maxtime )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( ( (1<<facts[ i ].type) & typemask ) != 0 && facts[ i ].created + maxtime > m_lastTime )
			return true;
	}
	return false;
}

TSFactStorage::Fact* TSFactStorage::GetRecentFact( int typemask, TimeVal maxtime )
{
	Fact* F = NULL;
//	puts("GetRecentFact");
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( ( (1<<facts[ i ].type) & typemask ) != 0 && facts[ i ].created + maxtime > m_lastTime )
		{
			F = &facts[ i ];
		//	printf("fact %p created at %d within %d\n", F, F->created, maxtime );
			maxtime = m_lastTime - facts[ i ].created;
		}
	}
	return F;
}

void TSFactStorage::Insert( FactType type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	Fact F = { m_next_fact_id++, ref, type, pos, created, expires };
//	printf( "INSERT FACT: type %d, pos: %g;%g;%g, created: %d, expires: %d\n",
//		(int)type, pos.x,pos.y,pos.z, (int)created, (int)expires );
	facts.push_back( F );
	last_mod_id = F.id;
}

bool TSFactStorage::Update( FactType type, Vec3 pos, float rad,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	float rad2 = rad * rad;
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].type == type &&
			( facts[ i ].position - pos ).LengthSq() < rad2 )
		{
			facts[ i ].position = pos;
			if( reset )
			{
				facts[ i ].created = created;
				facts[ i ].expires = expires;
			}
			facts[ i ].ref = ref;
			last_mod_id = facts[ i ].id;
			return true;
		}
	}
	
	return false;
}

void TSFactStorage::InsertOrUpdate( FactType type, Vec3 pos, float rad,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( Update( type, pos, rad, created, expires, ref, reset ) == false )
		Insert( type, pos, created, expires, ref );
}

bool TSFactStorage::MovingUpdate( FactType type, Vec3 pos, float movespeed,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].type != type )
			continue;
		
		float rad = ( created - facts[ i ].created ) * 0.001f * movespeed;
		if( ( facts[ i ].position - pos ).LengthSq() <= rad * rad + SMALL_FLOAT )
		{
			facts[ i ].position = pos;
			if( reset )
			{
				facts[ i ].created = created;
				facts[ i ].expires = expires;
			}
			facts[ i ].ref = ref;
			last_mod_id = facts[ i ].id;
			return true;
		}
	}
	
	return false;
}

void TSFactStorage::MovingInsertOrUpdate( FactType type, Vec3 pos, float movespeed,
	TimeVal created, TimeVal expires, uint32_t ref, bool reset )
{
	if( MovingUpdate( type, pos, movespeed, created, expires, ref, reset ) == false )
		Insert( type, pos, created, expires, ref );
}




extern sgs_ObjInterface TSEnemy_iface[1];

#define TSENEMY_VALID if( E == NULL ){ return sgs_Msg( C, SGS_WARNING, "enemy no longer exists" ); }

static int TSEnemy_HasFact( SGS_CTX )
{
	TSEnemy* E;
	SGS_PARSE_METHOD( C, TSEnemy_iface, E, TSEnemy, HasFact );
	TSENEMY_VALID;
	sgs_PushBool( C, E->HasFact( sgs_GetVar<int>()( C, 0 ) ) );
	return 1;
}

static int TSEnemy_HasRecentFact( SGS_CTX )
{
	TSEnemy* E;
	SGS_PARSE_METHOD( C, TSEnemy_iface, E, TSEnemy, HasRecentFact );
	TSENEMY_VALID;
	sgs_PushBool( C, E->HasRecentFact( sgs_GetVar<int>()( C, 0 ), sgs_GetVar<int>()( C, 1 ) ) );
	return 1;
}

static int TSEnemy_GetRecentFact( SGS_CTX )
{
	TSEnemy* E;
	SGS_PARSE_METHOD( C, TSEnemy_iface, E, TSEnemy, GetRecentFact );
	TSENEMY_VALID;
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

static int TSEnemy_getindex( SGS_ARGS_GETINDEXFUNC )
{
	SGS_BEGIN_INDEXFUNC
		SGS_CASE( "HasFact" ) return sgs_PushCFunction( C, TSEnemy_HasFact );
		SGS_CASE( "HasRecentFact" ) return sgs_PushCFunction( C, TSEnemy_HasRecentFact );
		SGS_CASE( "GetRecentFact" ) return sgs_PushCFunction( C, TSEnemy_GetRecentFact );
	SGS_END_INDEXFUNC
}

sgs_ObjInterface TSEnemy_iface[1] =
{{
	"TSEnemy",
	NULL, NULL,
	TSEnemy_getindex, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL
}};


TSEnemy::TSEnemy( GameLevel* lev, const StringView& name, const Vec3& pos, const Vec3& dir, sgsVariable args ) :
	TSCharacter( lev, pos, dir ),
	i_turn( V2(0) )
{
	m_typeName = "enemy";
	m_name = name;
	
	m_aidb = m_level->GetSystem<AIDBSystem>();
	m_meshInstInfo.ownerType = GAT_Enemy;
	
	// create self
	{
		sgs_Variable var;
		sgs_InitObject( m_level->m_scriptCtx.C, &var, this, TSEnemy_iface );
		m_scrObj = sgs_GetObjectStructP( &var );
	}
	
	// create ESO (enemy scripted object)
	{
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		sgs_PushObjectPtr( m_level->m_scriptCtx.C, m_scrObj );
		m_level->m_scriptCtx.Push( args );
		m_level->m_scriptCtx.Push( m_position );
		m_level->m_scriptCtx.Push( GetViewDir() );
		if( m_level->m_scriptCtx.GlobalCall( "TSEnemy_Create", 4, 1 ) == false )
		{
			LOG_ERROR << "FAILED to create enemy state";
		}
		m_enemyState = sgsVariable( m_level->m_scriptCtx.C, -1 );
	}
	
	StringView charpath = m_enemyState.getprop("charpath").get<StringView>();
	InitializeMesh( charpath ? charpath : "chars/tstest.chr" );
	
	m_level->MapEntityByName( this );
}

TSEnemy::~TSEnemy()
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
		Vec3 viewdir = enemy->GetViewDir();
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
	TSEnemy* enemy;
};

void TSEnemy::FixedTick( float deltaTime )
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
		m_level->m_scriptCtx.Push( GetViewDir() );
		m_enemyState.setprop( "viewdir", sgsVariable( m_level->m_scriptCtx.C, sgsVariable::PickAndPop ) );
		
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		m_level->m_scriptCtx.Push( deltaTime );
		m_enemyState.thiscall( "tick", 1 );
		
		i_crouch = m_enemyState[ "i_crouch" ].get<bool>();
		i_move = m_enemyState[ "i_move" ].get<Vec2>();
		i_speed = m_enemyState[ "i_speed" ].get<float>();
		i_turn = m_enemyState[ "i_turn" ].get<Vec2>();
	}
	
	if( i_turn.Length() > 0.1f )
	{
		TurnTo( i_turn, deltaTime * 8 );
	}
	
	TSCharacter::FixedTick( deltaTime );
	
	InfoEmissionSystem::Data D = { GetPosition(), 0.5f, IEST_MapItem };
	m_level->GetSystem<InfoEmissionSystem>()->UpdateEmitter( this, D );
}

void TSEnemy::Tick( float deltaTime, float blendFactor )
{
	TSCharacter::Tick( deltaTime, blendFactor );
}

bool TSEnemy::GetMapItemInfo( MapItemInfo* out )
{
	out->type = MI_Object_Enemy | MI_State_Normal;
	out->position = GetInterpPos();
	out->direction = GetInterpAimDir();
	out->sizeFwd = 10;
	out->sizeRight = 8;
	return true;
}

void TSEnemy::DebugDrawWorld()
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

void TSEnemy::DebugDrawUI()
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


TSEntityCreationSystem::TSEntityCreationSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	m_level->GetScriptCtx().Include( "data/enemy" );
}

bool TSEntityCreationSystem::AddEntity( const StringView& type, sgsVariable data )
{
#ifndef TSGAME_NO_PLAYER
	///////////////////////////
	if( type == "player" )
	{
		TSPlayer* P = new TSPlayer
		(
			m_level,
			data.getprop("position").get<Vec3>(),
			data.getprop("viewdir").get<Vec3>()
		);
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
		TSEnemy* E = new TSEnemy
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

void TSEntityCreationSystem::DrawUI()
{
#ifndef TSGAME_NO_PLAYER
	SGRX_CAST( TSPlayer*, P, m_level->m_player );
	if( P )
		P->DrawUI();
#endif
}


