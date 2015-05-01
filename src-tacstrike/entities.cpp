

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
	Update( g_GameLevel->m_infoEmitters.QueryAny( m_position, m_radius, IEST_Player ) );
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


TSCharacter::TSCharacter( const Vec3& pos, const Vec3& dir, const Vec4& color ) :
	m_footstepTime(0), m_isCrouching(0),
	m_ivPos( pos ), m_ivDir( Quat::CreateAxisAngle( V3(0,0,1), atan2( dir.y, dir.x ) ) ),
	m_position( pos.ToVec2() ), m_moveDir( V2(0) ), m_turnAngle( atan2( dir.y, dir.x ) )
{
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.friction = 0;
	rbinfo.restitution = 0;
	rbinfo.shape = g_PhyWorld->CreateCapsuleShape( 0.3f, 1.4f );
	rbinfo.mass = 70;
	rbinfo.inertia = V3(0);
	rbinfo.position = pos + V3(0,0,1);
	rbinfo.canSleep = false;
	rbinfo.group = 2;
	rbinfo.linearFactor = V3(1,1,0);
	m_bodyHandle = g_PhyWorld->CreateRigidBody( rbinfo );
	m_shapeHandle = g_PhyWorld->CreateCylinderShape( V3(0.29f) );
	
	m_meshInst = g_GameLevel->m_scene->CreateMeshInstance();
	m_meshInst->dynamic = 1;
	m_meshInst->layers = 0x2;
	m_meshInst->mesh = GR_GetMesh( "meshes/charmodel2.ssm" );
	m_meshInst->matrix = Mat4::CreateSRT( V3(1), m_ivDir.curr, pos );
	g_GameLevel->LightMesh( m_meshInst );
	m_meshInst->constants[ 0 ] = color;
	m_meshInst->skin_matrices.resize( m_meshInst->mesh->m_numBones );
	
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
	
	m_anEnd.animSource = &m_anMainPlayer;
	m_anEnd.PrepareForMesh( m_meshInst->mesh );
	m_anMainPlayer.Play( GR_GetAnim( "stand_anim" ) );
}

void TSCharacter::FixedTick( float deltaTime )
{
	Vec3 pos = m_bodyHandle->GetPosition() - V3(0,0,1);
	m_position = pos.ToVec2();
	
	bool ground = true;
	Vec3 lvel = m_bodyHandle->GetLinearVelocity();
	Vec2 lvel2 = lvel.ToVec2();
	
	float maxspeed = 2;
	float accel = ( m_moveDir.NearZero() && !m_isCrouching ) ? 38 : 30;
	if( m_isCrouching ){ accel = 5; maxspeed = 1.25f; }
	if( !ground ){ accel = 10; }
	
	float curspeed = Vec2Dot( lvel2, m_moveDir );
	float revmaxfactor = clamp( maxspeed - curspeed, 0, 1 );
	lvel2 += m_moveDir * accel * revmaxfactor * deltaTime;
	
	///// FRICTION /////
	curspeed = Vec2Dot( lvel2, m_moveDir );
	if( ground )
	{
		if( curspeed > maxspeed )
			curspeed = maxspeed;
	}
	lvel2 -= m_moveDir * curspeed;
	{
		Vec2 ldd = lvel2.Normalized();
		float llen = lvel2.Length();
		llen = TMAX( 0.0f, llen - deltaTime * ( ground ? 20 : ( m_isCrouching ? 0.5f : 3 ) ) );
		lvel2 = ldd * llen;
	}
	lvel2 += m_moveDir * curspeed;
	
	lvel.x = lvel2.x;
	lvel.y = lvel2.y;
	
	m_bodyHandle->SetLinearVelocity( lvel );
	
	m_ivPos.Advance( pos );
	m_ivDir.Advance( Quat::CreateAxisAngle( V3(0,0,1), m_turnAngle ) );
	m_anEnd.Advance( deltaTime );
}

void TSCharacter::Tick( float deltaTime, float blendFactor )
{
	Vec3 pos = m_ivPos.Get( blendFactor );
	Quat rdir = m_ivDir.Get( blendFactor ).Normalized();
	
	m_meshInst->matrix = Mat4::CreateSRT( V3(1), rdir, pos );
	m_shadowInst->position = pos + V3(0,0,1);
	
	g_GameLevel->LightMesh( m_meshInst );
	
	m_anEnd.Interpolate( blendFactor );
	GR_ApplyAnimator( &m_anEnd, m_meshInst );
}


TSPlayer::TSPlayer( const Vec3& pos, const Vec3& dir ) :
	TSCharacter( pos-V3(0,0,1), dir, V4( 0.5f, 0.7f, 0.9f, 1 ) ),
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false )
{
}

void TSPlayer::FixedTick( float deltaTime )
{
	Vec2 realdir = { cos( m_angles.x ), sin( m_angles.x ) };
	Vec2 perpdir = realdir.Perp();
	
	Vec2 md = { MOVE_LEFT.value - MOVE_RIGHT.value, MOVE_DOWN.value - MOVE_UP.value };
	md.Normalize();
	md = -realdir * md.y - perpdir * md.x;
	
	m_moveDir = md * 1.1f;
	
	bool moving = m_moveDir.Length() > 0.1f;
	const char* animname =
		m_isCrouching
		? ( moving ? "crouch_walk" : "crouch" )
		: ( moving ? "sneak" : "stand_anim" )
	;
	m_anMainPlayer.Play( GR_GetAnim( animname ) );
	
	if( md.Length() > 0.1f )
	{
		float angend = normalize_angle( m_moveDir.Angle() );
		float angstart = normalize_angle( m_turnAngle );
		if( fabs( angend - angstart ) > M_PI )
			angstart += angend > angstart ? M_PI * 2 : -M_PI * 2;
		m_turnAngle = angstart + sign( angend - angstart ) * TMIN( fabsf( angend - angstart ), deltaTime * 8 );
	}
	
	TSCharacter::FixedTick( deltaTime );
}

void TSPlayer::Tick( float deltaTime, float blendFactor )
{
	if( CROUCH.IsPressed() )
	{
		m_isCrouching = !m_isCrouching;
	}
	
	TSCharacter::Tick( deltaTime, blendFactor );
	
	m_angles += inCursorMove * V2(-0.01f);
	m_angles.y = clamp( m_angles.y, (float) -M_PI/2 + SMALL_FLOAT, (float) M_PI/2 - SMALL_FLOAT );
	
	float ch = cos( m_angles.x ), sh = sin( m_angles.x );
	float cv = cos( m_angles.y ), sv = sin( m_angles.y );
	
	Vec3 pos = m_ivPos.Get( blendFactor );
	Vec3 dir = V3( ch * cv, sh * cv, sv );
	
	g_GameLevel->m_scene->camera.znear = 0.1f;
	g_GameLevel->m_scene->camera.angle = 90;
	g_GameLevel->m_scene->camera.direction = dir;
	g_GameLevel->m_scene->camera.position = pos - dir * 3 + V3(0,0,1.5f);
	g_GameLevel->m_scene->camera.UpdateMatrices();
}

void TSPlayer::DrawUI()
{
	if( m_targetII )
	{
		float bsz = TMIN( GR_GetWidth(), GR_GetHeight() );
		float x = GR_GetWidth() / 2.0f;
		float y = GR_GetHeight() / 2.0f;
		GR2D_GetBatchRenderer().Reset().Col(1).SetTexture( m_tex_interact_icon ).QuadWH( x, y, bsz / 10, bsz / 10 );
	}
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
	TSCharacter( pos, dir, V4( 0.8f, 0.1f, 0.05f, 1 ) ),
	m_taskTimeout( 0 ), m_curTaskID( 0 ), m_curTaskMode( false ), m_turnAngleStart(0), m_turnAngleEnd(0)
{
	m_typeName = "enemy";
	m_name = name;
	
	UpdateTask();
	
	g_GameLevel->MapEntityByName( this );
}

void TSEnemy::FixedTick( float deltaTime )
{
	TSCharacter::FixedTick( deltaTime );
	
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
			m_moveDir = ( T.target - m_position ).Normalized();
			m_anMainPlayer.Play( GR_GetAnim( "march" ) );
			break;
		}
	//	LOG << "TASK " << T.type << "|" << T.timeout << "|" << T.target;
		
		if( m_taskTimeout <= 0 || ( T.target - m_position ).Length() < 0.5f )
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
			Vec2 td = ( T.target - m_position ).Normalized();
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


