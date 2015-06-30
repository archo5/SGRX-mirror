

#define USE_HASHTABLE
#include "level.hpp"


Trigger::Trigger( const StringView& fn, const StringView& tgt, bool once, bool laststate ) :
	m_func( fn ), m_target( tgt ), m_once( once ), m_done( false ), m_lastState( laststate ), m_currState( false )
{
}

void Trigger::Invoke( bool newstate )
{
	const char* evname = newstate ? "trigger_enter" : "trigger_leave";
	if( m_func.size() )
	{
		SGS_CSCOPE( g_GameLevel->m_scriptCtx.C );
		g_GameLevel->m_scriptCtx.Push( newstate );
		g_GameLevel->m_scriptCtx.Push( g_GameLevel->m_scriptCtx.CreateString( evname ) );
		g_GameLevel->m_scriptCtx.GlobalCall( StackString<256>( m_func ), 2 );
	}
	if( m_target.size() )
		g_GameLevel->CallEntityByName( m_target, evname );
}

void Trigger::Update( bool newstate )
{
	if( m_currState != newstate && !m_done )
	{
		m_currState = newstate;
		Invoke( newstate );
		if( m_once && newstate == m_lastState ) // once on, once off
			m_done = true;
	}
}


BoxTrigger::BoxTrigger( const StringView& fn, const StringView& tgt, bool once, const Vec3& pos, const Quat& rot, const Vec3& scl ) :
	Trigger( fn, tgt, once ), m_matrix( Mat4::CreateSRT( scl, rot, pos ) )
{
	m_matrix.InvertTo( m_matrix );
}

void BoxTrigger::FixedTick( float deltaTime )
{
	Update( g_GameLevel->m_infoEmitters.QueryBB( m_matrix, IEST_Player ) );
}


ProximityTrigger::ProximityTrigger( const StringView& fn, const StringView& tgt, bool once, const Vec3& pos, float rad ) :
	Trigger( fn, tgt, once ), m_position( pos ), m_radius( rad )
{
}

void ProximityTrigger::FixedTick( float deltaTime )
{
	Update( g_GameLevel->m_infoEmitters.QuerySphereAny( m_position, m_radius, IEST_Player ) );
}


void SlidingDoor::_UpdatePhysics()
{
	if( !bodyHandle )
		return;
	Vec3 localpos = TLERP( pos_closed, pos_open, open_factor );
	Quat localrot = TLERP( rot_closed, rot_open, open_factor );
	Mat4 basemtx = Mat4::CreateSRT( scale, rotation, position );
	bodyHandle->SetPosition( basemtx.TransformPos( localpos ) );
	bodyHandle->SetRotation( localrot * rotation );
}

void SlidingDoor::_UpdateTransforms( float bf )
{
	Vec3 localpos = m_ivPos.Get( bf );
	Quat localrot = m_ivRot.Get( bf );
	Mat4 basemtx = Mat4::CreateSRT( scale, rotation, position );
	meshInst->matrix = Mat4::CreateSRT( V3(1), localrot, localpos ) * basemtx;
	g_GameLevel->LightMesh( meshInst );
}

SlidingDoor::SlidingDoor
(
	const StringView& name,
	const StringView& mesh,
	const Vec3& pos,
	const Quat& rot,
	const Vec3& scl,
	const Vec3& oopen,
	const Quat& ropen,
	const Vec3& oclos,
	const Quat& rclos,
	float otime,
	bool istate,
	bool isswitch,
	const StringView& pred,
	const StringView& fn,
	const StringView& tgt,
	bool once
):
	Trigger( fn, tgt, once, true ),
	open_factor( istate ), open_target( istate ), open_time( TMAX( otime, SMALL_FLOAT ) ),
	pos_open( oopen ), pos_closed( oclos ), rot_open( ropen ), rot_closed( rclos ),
	target_state( istate ), m_isSwitch( isswitch ), m_switchPred( pred ),
	position( pos ), rotation( rot ), scale( scl ),
	m_bbMin( V3(-1) ), m_bbMax( V3(1) ),
	m_ivPos( V3(0) ), m_ivRot( Quat::Identity )
{
	m_name = name;
	meshInst = g_GameLevel->m_scene->CreateMeshInstance();
	meshInst->dynamic = 1;
	
	char bfr[ 256 ] = {0};
	snprintf( bfr, 255, "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	meshInst->mesh = GR_GetMesh( bfr );
	
	if( meshInst->mesh )
	{
		m_bbMin = meshInst->mesh->m_boundsMin;
		m_bbMax = meshInst->mesh->m_boundsMax;
	}
	
	if( m_isSwitch == false )
	{
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.shape = g_PhyWorld->CreateAABBShape( m_bbMin, m_bbMax );
		rbinfo.shape->SetScale( scale );
		rbinfo.kinematic = true;
		rbinfo.position = position;
		rbinfo.rotation = rotation;
		rbinfo.mass = 0;
		rbinfo.inertia = V3(0);
		bodyHandle = g_PhyWorld->CreateRigidBody( rbinfo );
		soundEvent = g_SoundSys->CreateEventInstance( "/gate_open" );
		if( soundEvent )
		{
			SGRX_Sound3DAttribs s3dattr = { position, V3(0), V3(0), V3(0) };
			soundEvent->Set3DAttribs( s3dattr );
		}
	}
	
	_UpdatePhysics();
	m_ivPos.prev = m_ivPos.curr = TLERP( pos_closed, pos_open, open_factor );
	m_ivRot.prev = m_ivRot.curr = TLERP( rot_closed, rot_open, open_factor );
	_UpdateTransforms( 1 );
	
	g_GameLevel->MapEntityByName( this );
}

void SlidingDoor::FixedTick( float deltaTime )
{
	if( soundEvent )
		soundEvent->SetParameter( "position", (target_state?0:1) + (target_state?1:-1) * open_factor / 1 );
	if( open_factor != open_target )
	{
		float df = open_target - open_factor;
		df /= open_time;
		float df_len = fabsf( df );
		float df_dir = sign( df );
		if( df_len > deltaTime )
			df_len = deltaTime;
		open_factor += df_len * df_dir;
		
		_UpdatePhysics();
	}
	else if( open_target != (target_state ? 1.0f : 0.0f) )
	{
		open_target = target_state;
		if( soundEvent )
			soundEvent->Start();
	}
	m_ivPos.Advance( TLERP( pos_closed, pos_open, open_factor ) );
	m_ivRot.Advance( TLERP( rot_closed, rot_open, open_factor ) );
	
	if( m_isSwitch )
	{
		if( !m_done )
		{
			InfoEmissionSystem::Data D = { position, 0.5f, IEST_InteractiveItem };
			g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
		}
		else
			g_GameLevel->m_infoEmitters.RemoveEmitter( this );
	}
}

void SlidingDoor::Tick( float deltaTime, float blendFactor )
{
	_UpdateTransforms( blendFactor );
}

void SlidingDoor::OnEvent( const StringView& type )
{
	if( type == "trigger_enter" || type == "trigger_leave" || type == "trigger_switch" )
	{
		bool newstate = type == "trigger_enter" || ( type == "trigger_switch" && !target_state );
		if( m_isSwitch )
		{
			if( open_factor != open_target )
			{
				return;
			}
			SGS_CSCOPE( g_GameLevel->m_scriptCtx.C );
			g_GameLevel->m_scriptCtx.Push( newstate );
			if( g_GameLevel->m_scriptCtx.GlobalCall( StackString<256>( m_switchPred ), 1, 1 ) )
			{
				bool val = sgs_GetVar<bool>()( g_GameLevel->m_scriptCtx.C, -1 );
				if( !val )
					return;
			}
		}
		
		target_state = newstate;
		
		if( m_isSwitch )
		{
			Update( target_state );
		}
	}
}


PickupItem::PickupItem( const StringView& id, const StringView& name, int count, const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl ) :
	m_count( count )
{
	m_name = id;
	m_viewName = name;
	m_meshInst = g_GameLevel->m_scene->CreateMeshInstance();
	m_meshInst->dynamic = 1;
	
	char bfr[ 256 ] = {0};
	snprintf( bfr, 255, "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	m_meshInst->mesh = GR_GetMesh( bfr );
	m_meshInst->matrix = Mat4::CreateSRT( scl, rot, pos );
	g_GameLevel->LightMesh( m_meshInst );
	
	InfoEmissionSystem::Data D = { pos, 0.5f, IEST_InteractiveItem };
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
}

void PickupItem::OnEvent( const StringView& type )
{
	if( type == "trigger_switch" && g_GameLevel->m_player )
	{
		g_GameLevel->m_player->AddItem( m_name, m_count );
		g_GameLevel->m_infoEmitters.RemoveEmitter( this );
		m_meshInst->enabled = false;
		
		char bfr[ 256 ];
		sprintf( bfr, "Picked up %.*s", TMIN( 240, (int) m_viewName.size() ), m_viewName.data() );
		g_GameLevel->m_messageSystem.AddMessage( MSMessage::Info, bfr );
	}
}


ParticleFX::ParticleFX( const StringView& name, const StringView& psys, const StringView& sndev, const Vec3& pos, const Quat& rot, const Vec3& scl, bool start ) :
	m_soundEventName( sndev ), m_position( pos )
{
	m_name = name;
	m_soundEventOneShot = g_SoundSys->EventIsOneShot( sndev );
	
	char bfr[ 256 ] = {0};
	snprintf( bfr, 255, "psys/%.*s.psy", TMIN( 240, (int) psys.size() ), psys.data() );
	m_psys.Load( bfr );
	m_psys.AddToScene( g_GameLevel->m_scene );
	m_psys.SetTransform( Mat4::CreateSRT( scl, rot, pos ) );
	m_psys.OnRenderUpdate();
	
	if( start )
		OnEvent( "trigger_enter" );
	
	g_GameLevel->MapEntityByName( this );
}

void ParticleFX::Tick( float deltaTime, float blendFactor )
{
	bool needstrig = m_psys.Tick( deltaTime );
	if( needstrig && m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = g_SoundSys->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			SGRX_Sound3DAttribs s3dattr = { m_position, V3(0), V3(0), V3(0) };
			sndevinst->Set3DAttribs( s3dattr );
			sndevinst->Start();
		}
	}
	m_psys.PreRender();
}

void ParticleFX::OnEvent( const StringView& _type )
{
	StringView type = _type;
	if( type == "trigger_switch" )
		type = m_psys.m_isPlaying ? "trigger_leave" : "trigger_enter";
	if( type == "trigger_enter" )
	{
		m_psys.Play();
		SoundEventInstanceHandle sndevinst = g_SoundSys->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			SGRX_Sound3DAttribs s3dattr = { m_position, V3(0), V3(0), V3(0) };
			sndevinst->Set3DAttribs( s3dattr );
			sndevinst->Start();
			if( !sndevinst->isOneShot )
				m_soundEventInst = sndevinst;
		}
	}
	else if( type == "trigger_leave" )
	{
		m_psys.Stop();
		m_soundEventInst = NULL;
	}
	else if( type == "trigger_once" )
	{
		m_psys.Trigger();
		if( m_soundEventOneShot )
		{
			SoundEventInstanceHandle sndevinst = g_SoundSys->CreateEventInstance( m_soundEventName );
			if( sndevinst )
			{
				SGRX_Sound3DAttribs s3dattr = { m_position, V3(0), V3(0), V3(0) };
				sndevinst->Set3DAttribs( s3dattr );
				sndevinst->Start();
			}
		}
	}
}






#ifdef TSGAME


TSCharacter::TSCharacter( const Vec3& pos, const Vec3& dir ) :
	m_footstepTime(0), m_isCrouching(false), m_isOnGround(false),
	m_ivPos( pos ), m_ivDir( Quat::CreateAxisAngle( V3(0,0,1), atan2( dir.y, dir.x ) ) ),
	m_position( pos ), m_moveDir( V2(0) ), m_turnAngle( atan2( dir.y, dir.x ) ),
	i_crouch( false ), i_move( V2(0) ), i_aim_at( false ), i_aim_target( V3(0) )
{
	m_meshInstInfo.typeOverride = "*human*";
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = g_PhyWorld->CreateCylinderShape( V3(0.3f,0.3f,0.5f) );
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	m_bodyHandle = g_PhyWorld->CreateRigidBody( rbinfo );
	m_shapeHandle = g_PhyWorld->CreateCylinderShape( V3(0.25f) );
	
	m_shadowInst = g_GameLevel->m_scene->CreateLight();
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
//	m_anLayers[3].anim = &m_anRagdoll;
//	m_anLayers[3].tflags = AnimMixer::TF_Absolute_Pos | AnimMixer::TF_Absolute_Rot;
//	m_anLayers[3].factor = 0;
	m_animChar.m_anMixer.layers = m_anLayers;
	m_animChar.m_anMixer.layerCount = 3;
	
	InitializeMesh( "chars/tstest.chr" );
}

void TSCharacter::InitializeMesh( const StringView& path )
{
	m_animChar.Load( path );
	m_animChar.AddToScene( g_GameLevel->m_scene );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->userData = &m_meshInstInfo;
	MI->dynamic = 1;
	MI->layers = 0x2;
	MI->matrix = Mat4::CreateSRT( V3(1), m_ivDir.curr, m_ivPos.curr );
	g_GameLevel->LightMesh( MI );
	
	m_anTopPlayer.ClearBlendFactors( 0.0f );
//	m_animChar.ApplyMask( "top", &m_anTopPlayer );
	
	m_anMainPlayer.Play( GR_GetAnim( "standing_idle" ) );
	m_anTopPlayer.Play( GR_GetAnim( "run" ) );
}

void TSCharacter::FixedTick( float deltaTime )
{
	HandleMovementPhysics( deltaTime );
	
	m_ivDir.Advance( Quat::CreateAxisAngle( V3(0,0,1), m_turnAngle ) );
	
	//
	Vec2 rundir = V2( cosf( m_turnAngle ), sinf( m_turnAngle ) );
	Vec2 aimdir = rundir;
	if( i_aim_at )
	{
		aimdir = ( i_aim_target - GetPosition() ).ToVec2();
	}
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
	
	m_animChar.FixedTick( deltaTime );
}

void TSCharacter::Tick( float deltaTime, float blendFactor )
{
	Vec3 pos = m_ivPos.Get( blendFactor );
//	Quat rdir = m_ivDir.Get( blendFactor ).Normalized();
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->matrix = Mat4::CreateTranslation( pos ); // Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	g_GameLevel->LightMesh( MI );
	
	m_animChar.PreRender( blendFactor );
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
	
	m_isCrouching = i_crouch;
	if( g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
		fabsf( rcinfo.point.z - rcinfo2.point.z ) < 1.8f )
	{
		m_isCrouching = 1;
	}
	
	float cheight = m_isCrouching ? 0.6f : 1.3f;
	float rcxdist = 1.0f;
	Vec3 lvel = m_bodyHandle->GetLinearVelocity();
	float ht = cheight - 0.25f;
	if( lvel.z >= -SMALL_FLOAT && !m_jumpTimeout )
		ht += rcxdist;
	
	m_ivPos.Advance( pos + V3(0,0,-cheight) );
	
	bool ground = false;
	if( g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-ht), 1, 1, &rcinfo )
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
	
	float maxspeed = 5;
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
	m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), speedDelta );
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
	return m_ivPos.Get( 1 );
}

Vec3 TSCharacter::GetInterpAimDir()
{
	return GetAimDir();
}


TSPlayer::TSPlayer( const Vec3& pos, const Vec3& dir ) :
	TSCharacter( pos-V3(0,0,1), dir ),
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false )
{
	m_meshInstInfo.ownerType = GAT_Player;
	
	m_tex_cursor = GR_GetTexture( "ui/crosshair.png" );
	i_aim_at = true;
	
	m_shootPS.Load( "psys/fastspark.psy" );
	m_shootPS.AddToScene( g_GameLevel->m_scene );
	m_shootPS.OnRenderUpdate();
	m_shootTimeout = 0;
}

void TSPlayer::FixedTick( float deltaTime )
{
	i_move = V2( MOVE_LEFT.value - MOVE_RIGHT.value, MOVE_DOWN.value - MOVE_UP.value );
	i_aim_target = FindTargetPosition();
	
	//	bool moving = m_moveDir.Length() > 0.1f;
//	const char* animname =
//		m_isCrouching
//		? ( moving ? "crouch_walk" : "crouch" )
//		: ( moving ? "sneak" : "stand_anim" )
//	;
//	m_anMainPlayer.Play( GR_GetAnim( animname ) );
	
	const char* animname = "run";
	Vec2 md = i_move.Normalized();
	if( Vec2Dot( md, GetAimDir().ToVec2() ) < 0 )
	{
		md = -md;
		animname = "run_bw";
	}
	if( i_move.Length() > 0.1f )
	{
		TurnTo( md, deltaTime * 8 );
	}
	
//	i_crouch = CROUCH.value;
	
	m_anMainPlayer.Play( GR_GetAnim( i_move.Length() ? animname : "stand_with_gun_up" ), false, 0.2f );
	
	TSCharacter::FixedTick( deltaTime );
	
	while( m_shootTimeout > 0 )
		m_shootTimeout -= deltaTime;
	if( SHOOT.value && m_shootTimeout <= 0 )
	{
		Mat4 mtx = GetBulletOutputMatrix();
		Vec3 origin = mtx.TransformPos( V3(0) );
		Vec3 dir = ( i_aim_target - origin ).Normalized();
		dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
		LOG << dir;
		g_GameLevel->m_bulletSystem.Add( origin, dir * 100, 1, 1, m_meshInstInfo.ownerType );
		m_shootPS.SetTransform( Mat4::CreateScale( V3(0.1f) ) * mtx );
		m_shootPS.Trigger();
		m_shootTimeout += 0.1f;
	}
}

void TSPlayer::Tick( float deltaTime, float blendFactor )
{
	if( CROUCH.IsPressed() )
	{
		i_crouch = !i_crouch;
	}
	
	TSCharacter::Tick( deltaTime, blendFactor );
	
	m_angles += inCursorMove * V2(-0.01f);
	m_angles.y = clamp( m_angles.y, (float) -M_PI/2 + SMALL_FLOAT, (float) M_PI/2 - SMALL_FLOAT );
	
	float ch = cosf( m_angles.x ), sh = sinf( m_angles.x );
	float cv = cosf( m_angles.y ), sv = sinf( m_angles.y );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	Vec3 dir = V3( ch * cv, sh * cv, sv );
	m_position = pos;
	
	float bmsz = ( GR_GetWidth() + GR_GetHeight() );// * 0.5f;
	Vec2 cursor_pos = Game_GetCursorPos();
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	Vec2 player_pos = g_GameLevel->m_scene->camera.WorldToScreen( m_position ).ToVec2() * screen_size;
	Vec2 diff = ( cursor_pos - player_pos ) / bmsz;
	
	g_GameLevel->m_scene->camera.znear = 0.1f;
	g_GameLevel->m_scene->camera.angle = 90;
	g_GameLevel->m_scene->camera.updir = V3(0,-1,0);
	g_GameLevel->m_scene->camera.direction = V3(-diff.x,diff.y,-5);
	g_GameLevel->m_scene->camera.position = pos + V3(-diff.x,diff.y,0) * 2 + V3(0,0,1) * 6;
	g_GameLevel->m_scene->camera.UpdateMatrices();
	
	InfoEmissionSystem::Data D = { pos, 0.5f, IEST_HeatSource | IEST_Player };
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
	
	m_shootPS.Tick( deltaTime );
	m_shootPS.PreRender();
}

void TSPlayer::DrawUI()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
	Vec2 cursor_pos = Game_GetCursorPos();
	Vec2 screen_size = V2( GR_GetWidth(), GR_GetHeight() );
	Vec2 player_pos = g_GameLevel->m_scene->camera.WorldToScreen( m_position ).ToVec2() * screen_size;
	
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
}

Vec3 TSPlayer::FindTargetPosition()
{
	Vec3 crpos, crdir;
	Vec2 crsp = Game_GetCursorPosNormalized();
	g_GameLevel->m_scene->camera.GetCursorRay( crsp.x, crsp.y, crpos, crdir );
	Vec3 crtgt = crpos + crdir * 100;
	
	SGRX_PhyRaycastInfo rcinfo;
	if( g_PhyWorld->Raycast( crpos, crtgt, 0x1, 0xffff, &rcinfo ) )
	{
		bool atwall = 0.707f > Vec3Dot( rcinfo.normal, V3(0,0,1) ); // > ~45deg to up vector
		bool frontface = Vec3Dot( rcinfo.normal, crdir ) < 0; 
		if( atwall && frontface )
			return rcinfo.point;
		
		// must adjust target height above ground
		if( frontface )
			return rcinfo.point + V3(0,0,1.5f);
	}
	
	// backup same level plane test if aiming into nothing
	float dsts[2];
	if( RayPlaneIntersect( crpos, crdir, V4(0,0,1,GetPosition().z), dsts ) )
	{
		return crpos + crdir * dsts[0];
	}
	
	return V3(0);
}

bool TSPlayer::AddItem( const StringView& item, int count )
{
	String key = item;
	int* ic = m_items.getptr( key );
	if( count < 0 )
	{
		if( !ic || *ic < count )
			return false;
		*ic += count;
	}
	else
	{
		if( !ic )
			m_items.set( key, count );
		else
			*ic += count;
	}
	return true;
}

bool TSPlayer::HasItem( const StringView& item, int count )
{
	int* ic = m_items.getptr( item );
	return ic && *ic >= count;
}


TSEnemy::TSEnemy( const StringView& name, const Vec3& pos, const Vec3& dir ) :
	TSCharacter( pos, dir ),
	m_taskTimeout( 0 ), m_curTaskID( 0 ), m_curTaskMode( false ), m_turnAngleStart(0), m_turnAngleEnd(0)
{
	m_typeName = "enemy";
	m_name = name;
	
	m_meshInstInfo.ownerType = GAT_Enemy;
	
	UpdateTask();
	
	// create ESO (enemy scripted object)
	{
		SGS_CSCOPE( g_GameLevel->m_scriptCtx.C );
		g_GameLevel->m_scriptCtx.Push( (void*) this );
		g_GameLevel->m_scriptCtx.Push( m_position );
		g_GameLevel->m_scriptCtx.Push( GetViewDir() );
		if( g_GameLevel->m_scriptCtx.GlobalCall( "TSEnemy_Create", 3, 1 ) == false )
		{
			LOG_ERROR << "FAILED to create enemy state";
		}
		m_enemyState = sgsVariable( g_GameLevel->m_scriptCtx.C, -1 );
	}
	
	g_GameLevel->MapEntityByName( this );
}

TSEnemy::~TSEnemy()
{
	// destroy ESO
	{
		SGS_CSCOPE( g_GameLevel->m_scriptCtx.C );
		m_enemyState.thiscall( "destroy" );
	}
}

void TSEnemy::FixedTick( float deltaTime )
{
	TSTaskArray* ta = m_curTaskMode ? &m_disturbTasks : &m_patrolTasks;
	if( ta->size() )
	{
		m_taskTimeout -= deltaTime;
		
		TSTask& T = (*ta)[ m_curTaskID ];
		switch( T.type )
		{
		case TT_Wait:
			m_moveDir = V2(0);
			m_anMainPlayer.Play( GR_GetAnim( "stand_anim" ) );
			break;
		case TT_Turn:
			m_moveDir = V2(0);
			m_turnAngle = TLERP( m_turnAngleStart, m_turnAngleEnd, 1 - m_taskTimeout / T.timeout );
			m_anMainPlayer.Play( GR_GetAnim( "turn" ) );
			break;
		case TT_Walk:
			m_moveDir = ( T.target - m_position.ToVec2() ).Normalized();
			m_anMainPlayer.Play( GR_GetAnim( "march" ) );
			break;
		}
	//	LOG << "TASK " << T.type << "|" << T.timeout << "|" << T.target;
		
		if( m_taskTimeout <= 0 || ( T.target - m_position.ToVec2() ).Length() < 0.5f )
		{
			m_curTaskID++;
			if( m_curTaskID >= (int) ta->size() )
			{
				m_curTaskID = 0;
				m_curTaskMode = false;
			}
			UpdateTask();
		}
	}
	
	// tick ESO
	{
		g_GameLevel->m_scriptCtx.Push( GetPosition() );
		m_enemyState.setprop( "position", sgsVariable( g_GameLevel->m_scriptCtx.C, sgsVariable::PickAndPop ) );
		g_GameLevel->m_scriptCtx.Push( GetViewDir() );
		m_enemyState.setprop( "viewdir", sgsVariable( g_GameLevel->m_scriptCtx.C, sgsVariable::PickAndPop ) );
		
		SGS_CSCOPE( g_GameLevel->m_scriptCtx.C );
		g_GameLevel->m_scriptCtx.Push( deltaTime );
		m_enemyState.thiscall( "tick", 1 );
		
		i_crouch = m_enemyState[ "i_crouch" ].get<bool>();
		i_move = m_enemyState[ "i_move" ].get<Vec2>();
	}
	
	if( i_move.Length() > 0.5f )
	{
		TurnTo( i_move, deltaTime * 8 );
	}
	
	const char* animname = "run";
	m_anMainPlayer.Play( GR_GetAnim( i_move.Length() > 0.5f ? animname : "standing_idle" ), false, 0.2f );
	
	TSCharacter::FixedTick( deltaTime );
	
	InfoEmissionSystem::Data D = { GetPosition(), 0.5f, IEST_Enemy };
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
}

void TSEnemy::Tick( float deltaTime, float blendFactor )
{
	TSCharacter::Tick( deltaTime, blendFactor );
}

void TSEnemy::UpdateTask()
{
	TSTaskArray* ta = m_curTaskMode ? &m_disturbTasks : &m_patrolTasks;
	if( ta->size() )
	{
		TSTask& T = (*ta)[ m_curTaskID ];
		m_taskTimeout = T.timeout;
		if( T.type == TT_Turn )
		{
			Vec2 td = ( T.target - m_position.ToVec2() ).Normalized();
			m_turnAngleEnd = normalize_angle( atan2( td.y, td.x ) );
			m_turnAngleStart = normalize_angle( m_turnAngle );
			if( fabs( m_turnAngleEnd - m_turnAngleStart ) > M_PI )
				m_turnAngleStart += m_turnAngleEnd > m_turnAngleStart ? M_PI * 2 : -M_PI * 2;
		}
	}
}

void TSParseTaskArray( TSTaskArray& out, sgsVariable var )
{
	ScriptVarIterator it( var );
	while( it.Advance() )
	{
		TSTask ntask = { TT_Wait, 100.0f, V2(0) };
		
		sgsVariable item = it.GetValue();
		{
			sgsVariable p_type = item.getprop("type");
			if( p_type.not_null() )
				ntask.type = (TSTaskType) p_type.get<int>();
		}
		{
			sgsVariable p_tgt = item.getprop("target");
			if( p_tgt.not_null() )
				ntask.target = p_tgt.get<Vec2>();
		}
		{
			sgsVariable p_time = item.getprop("timeout");
			if( p_time.not_null() )
				ntask.timeout = p_time.get<float>();
		}
		out.push_back( ntask );
	}
}


#endif


