

#define USE_HASHTABLE
#include "level.hpp"


extern Vec2 CURSOR_POS;


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
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
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
	m_count( count ), m_pos( pos )
{
	m_name = id;
	m_viewName = name;
	m_meshInst = g_GameLevel->m_scene->CreateMeshInstance();
	m_meshInst->dynamic = 1;
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	m_meshInst->mesh = GR_GetMesh( bfr );
	m_meshInst->matrix = Mat4::CreateSRT( scl, rot, pos );
	g_GameLevel->LightMesh( m_meshInst );
	
	InfoEmissionSystem::Data D = { pos, 0.5f, IEST_InteractiveItem };
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
}

void PickupItem::OnEvent( const StringView& type )
{
	if( ( type == "trigger_switch" || type == "action_end" ) && g_GameLevel->m_player )
	{
		g_GameLevel->m_player->AddItem( m_name, m_count );
		g_GameLevel->m_infoEmitters.RemoveEmitter( this );
		m_meshInst->enabled = false;
		
		char bfr[ 256 ];
		sprintf( bfr, "Picked up %.*s", TMIN( 240, (int) m_viewName.size() ), m_viewName.data() );
		g_GameLevel->m_messageSystem.AddMessage( MSMessage::Info, bfr );
	}
}

bool PickupItem::GetInteractionInfo( Vec3 pos, InteractInfo* out )
{
	out->type = IT_Pickup;
	out->placePos = m_pos;
	out->placeDir = V3(0);
	out->timeEstimate = 0.5f;
	out->timeActual = 0.5f;
	return true;
}


Actionable::Actionable( const StringView& name, const StringView& mesh,
	const Vec3& pos, const Quat& rot, const Vec3& scl, const Vec3& placeoff, const Vec3& placedir )
{
	Mat4 mtx = Mat4::CreateSRT( scl, rot, pos );
	
	m_info.type = IT_Investigate;
	m_info.placePos = mtx.TransformPos( placeoff );
	m_info.placeDir = mtx.TransformNormal( placedir ).Normalized();
	m_info.timeEstimate = 0.5f;
	m_info.timeActual = 0.5f;
	
	m_name = name;
	m_viewName = name;
	m_meshInst = g_GameLevel->m_scene->CreateMeshInstance();
	m_meshInst->dynamic = 1;
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	m_meshInst->mesh = GR_GetMesh( bfr );
	m_meshInst->matrix = mtx;
	g_GameLevel->LightMesh( m_meshInst );
	
	InfoEmissionSystem::Data D = { pos, 0.5f, IEST_InteractiveItem };
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
	
	g_GameLevel->MapEntityByName( this );
}

void Actionable::OnEvent( const StringView& type )
{
	if( type == "action_start" )
	{
		// start animation?
	}
	else if( type == "action_end" )
	{
		// end animation?
		SGS_CSCOPE( g_GameLevel->m_scriptCtx.C );
		g_GameLevel->m_scriptCtx.Push( m_name );
		if( m_onSuccess.call( 1, 1 ) )
		{
			bool keep = sgsVariable( g_GameLevel->m_scriptCtx.C, -1 ).get<bool>();
			if( keep == false )
			{
				g_GameLevel->m_infoEmitters.RemoveEmitter( this );
			}
		}
	}
}

bool Actionable::GetInteractionInfo( Vec3 pos, InteractInfo* out )
{
	*out = m_info;
	return true;
}

void Actionable::SetProperty( const StringView& name, sgsVariable value )
{
	if( name == "viewName" ) m_viewName = value.get<String>();
	else if( name == "timeEstimate" ) m_info.timeEstimate = value.get<float>();
	else if( name == "timeActual" ) m_info.timeActual = value.get<float>();
	else if( name == "callback" ) m_onSuccess = value;
}


ParticleFX::ParticleFX( const StringView& name, const StringView& psys, const StringView& sndev, const Vec3& pos, const Quat& rot, const Vec3& scl, bool start ) :
	m_soundEventName( sndev ), m_position( pos )
{
	m_name = name;
	m_soundEventOneShot = g_SoundSys->EventIsOneShot( sndev );
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "psys/%.*s.psy", TMIN( 240, (int) psys.size() ), psys.data() );
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




ScriptedItem::ScriptedItem( const StringView& name, sgsVariable args ) : m_scrItem(NULL)
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "SCRITEM_CREATE_%s", StackString<200>(name).str );
	sgsVariable func = g_GameLevel->m_scriptCtx.GetGlobal( bfr );
	if( func.not_null() )
	{
		m_scrItem = SGRX_ScriptedItem::Create(
			g_GameLevel->m_scene, g_PhyWorld, g_GameLevel->m_scriptCtx.C,
			func, args );
		m_scrItem->SetLightSampler( g_GameLevel );
		m_scrItem->PreRender();
	}
}

ScriptedItem::~ScriptedItem()
{
	if( m_scrItem )
		m_scrItem->Release();
}

void ScriptedItem::FixedTick( float deltaTime )
{
	if( m_scrItem )
	{
		m_scrItem->FixedTick( deltaTime );
	}
}

void ScriptedItem::Tick( float deltaTime, float blendFactor )
{
	if( m_scrItem )
	{
		m_scrItem->Tick( deltaTime, blendFactor );
		m_scrItem->PreRender();
	}
}






#ifdef TSGAME


#define MAGNUM_CAMERA_NOTICE_TIME 2
#define MAGNUM_CAMERA_ALERT_TIME 5
#define MAGNUM_CAMERA_VIEW_DIST 6.0f


TSCamera::TSCamera(
	const StringView& name,
	const StringView& charname,
	const Vec3& pos,
	const Quat& rot,
	const Vec3& scl,
	const Vec3& dir0,
	const Vec3& dir1
) :
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
	m_animChar.AddToScene( g_GameLevel->m_scene );
	StringView atchlist[] = { "view", "origin", "light" };
	m_animChar.SortEnsureAttachments( atchlist, 3 );
	
	SGRX_MeshInstance* MI = m_animChar.m_cachedMeshInst;
	MI->dynamic = 1;
	MI->layers = 0x2;
	MI->matrix = Mat4::CreateSRT( scl, rot, pos );
	g_GameLevel->LightMesh( MI );
	
	g_GameLevel->MapEntityByName( this );
}

void TSCamera::FixedTick( float deltaTime )
{
	m_playerVisible = false;
	if( g_GameLevel->m_player )
	{
		Vec3 ppos = g_GameLevel->m_player->GetPosition();
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
			bool isect = g_PhyWorld->Raycast( viewpos, ppos, 1, 1 );
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
	g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
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
	
	g_GameLevel->m_flareSystem.UpdateFlare( this, FD );
}

void TSCamera::SetProperty( const StringView& name, sgsVariable value )
{
	if( name == "moveTime" ) m_moveTime = value.get<float>();
	else if( name == "pauseTime" ) m_pauseTime = value.get<float>();
	else if( name == "fov" ) m_fov = value.get<float>();
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


TSCharacter::TSCharacter( const Vec3& pos, const Vec3& dir ) :
	m_footstepTime(0), m_isCrouching(false), m_isOnGround(false),
	m_ivPos( pos ), m_ivAimDir( dir ),
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
	MI->matrix = Mat4::CreateSRT( V3(1), Quat::Identity, m_ivPos.curr );
	g_GameLevel->LightMesh( MI, V3(1) );
	
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
		if( i_move.Length() > 0.1f )
		{
			TurnTo( md, deltaTime * 8 );
		}
		
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
	
	g_GameLevel->LightMesh( MI, V3(1) );
	
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
	if( g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,3), 1, 1, &rcinfo ) &&
		g_PhyWorld->ConvexCast( m_shapeHandle, pos + V3(0,0,0), pos + V3(0,0,-3), 1, 1, &rcinfo2 ) &&
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
	g_GameLevel->m_infoEmitters.QuerySphereAll( &ies_gather, QP, 5, IEST_InteractiveItem );
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
	
	if( E->GetInteractionInfo( GetQueryPosition(), &m_actState.info ) == false )
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
	return IsInAction() && m_actState.target->CanInterruptAction( m_actState.progress );
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


TSPlayer::TSPlayer( const Vec3& pos, const Vec3& dir ) :
	TSCharacter( pos-V3(0,0,1), dir ),
	m_angles( V2( atan2( dir.y, dir.x ), atan2( dir.z, dir.ToVec2().Length() ) ) ), inCursorMove( V2(0) ),
	m_targetII( NULL ), m_targetTriggered( false ),
	m_crouchIconShowTimeout( 0 ), m_standIconShowTimeout( 1 )
{
	m_meshInstInfo.ownerType = GAT_Player;
	
	m_tex_cursor = GR_GetTexture( "ui/crosshair.png" );
	i_aim_at = true;
	
	m_shootPS.Load( "psys/gunflash.psy" );
	m_shootPS.AddToScene( g_GameLevel->m_scene );
	m_shootPS.OnRenderUpdate();
	m_shootLT = g_GameLevel->m_scene->CreateLight();
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
	
	
	m_shootLT->enabled = false;
	if( m_shootTimeout > 0 )
		m_shootTimeout -= deltaTime;
	if( SHOOT.value && m_shootTimeout <= 0 )
	{
		Mat4 mtx = GetBulletOutputMatrix();
		Vec3 origin = mtx.TransformPos( V3(0) );
		Vec3 dir = ( i_aim_target - origin ).Normalized();
		dir = ( dir + V3( randf11(), randf11(), randf11() ) * 0.02f ).Normalized();
		g_GameLevel->m_bulletSystem.Add( origin, dir * 100, 1, 1, m_meshInstInfo.ownerType );
		m_shootPS.SetTransform( mtx );
		m_shootPS.Trigger();
		m_shootLT->position = origin;
		m_shootLT->UpdateTransform();
		m_shootLT->enabled = true;
		m_shootTimeout += 0.1f;
		g_GameLevel->m_aidbSystem.AddSound( GetPosition(), 10, 0.2f, AIS_Shot );
	}
	m_shootLT->color = V3(0.9f,0.7f,0.5f)*0.5f * smoothlerp_oneway( m_shootTimeout, 0, 0.1f );
	
	
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
	Vec2 player_pos = g_GameLevel->m_scene->camera.WorldToScreen( m_position ).ToVec2() * screen_size;
	
	Vec3 QP = GetQueryPosition();
	IESItemGather ies_gather;
	g_GameLevel->m_infoEmitters.QuerySphereAll( &ies_gather, QP, 5, IEST_InteractiveItem );
	if( ies_gather.items.size() )
	{
		ies_gather.DistanceSort( QP );
		for( size_t i = ies_gather.items.size(); i > 0; )
		{
			i--;
			Entity* E = ies_gather.items[ i ].E;
			Vec3 pos = ies_gather.items[ i ].D.pos;
			bool infront;
			Vec2 screenpos = g_GameLevel->m_scene->camera.WorldToScreen( pos, &infront ).ToVec2() * screen_size;
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
	g_GameLevel->m_scene->camera.GetCursorRay( crsp.x, crsp.y, crpos, crdir );
	Vec3 crtgt = crpos + crdir * 100;
	
	SGRX_PhyRaycastInfo rcinfo;
	if( g_PhyWorld->Raycast( crpos, crtgt, 0x1, 0x1, &rcinfo ) )
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


TSFactStorage::TSFactStorage() : last_mod_id(0), m_next_fact_id(1)
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

void TSFactStorage::Insert( FactType type, Vec3 pos, TimeVal created, TimeVal expires, uint32_t ref )
{
	Fact F = { m_next_fact_id++, ref, type, pos, created, expires };
//	printf( "INSERT FACT: type %d, pos: %g;%g;%g, created: %d, expires: %d\n",
//		(int)type, pos.x,pos.y,pos.z, (int)created, (int)expires );
	facts.push_back( F );
	last_mod_id = F.id;
}

bool TSFactStorage::Update( FactType type, Vec3 pos, float rad,
	TimeVal created, TimeVal expires, uint32_t ref )
{
	float rad2 = rad * rad;
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].type == type &&
			( facts[ i ].position - pos ).LengthSq() < rad2 )
		{
			facts[ i ].position = pos;
			facts[ i ].created = created;
			facts[ i ].expires = expires;
			facts[ i ].ref = ref;
			last_mod_id = facts[ i ].id;
			return true;
		}
	}
	
	return false;
}

void TSFactStorage::InsertOrUpdate( FactType type, Vec3 pos, float rad,
	TimeVal created, TimeVal expires, uint32_t ref )
{
	if( Update( type, pos, rad, created, expires, ref ) == false )
		Insert( type, pos, created, expires, ref );
}

bool TSFactStorage::MovingUpdate( FactType type, Vec3 pos, float movespeed,
	TimeVal created, TimeVal expires, uint32_t ref )
{
	for( size_t i = 0; i < facts.size(); ++i )
	{
		if( facts[ i ].type != type )
			continue;
		
		float rad = ( created - facts[ i ].created ) * 0.001f * movespeed;
		if( ( facts[ i ].position - pos ).LengthSq() <= rad * rad + SMALL_FLOAT )
		{
			facts[ i ].position = pos;
			facts[ i ].created = created;
			facts[ i ].expires = expires;
			facts[ i ].ref = ref;
			last_mod_id = facts[ i ].id;
			return true;
		}
	}
	
	return false;
}

void TSFactStorage::MovingInsertOrUpdate( FactType type, Vec3 pos, float movespeed,
	TimeVal created, TimeVal expires, uint32_t ref )
{
	if( MovingUpdate( type, pos, movespeed, created, expires, ref ) == false )
		Insert( type, pos, created, expires, ref );
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
		
		if( g_PhyWorld->Raycast( vieworigin, enemypos, 1, 1 ) )
			return true; // behind wall
		
		// TODO friendlies
		TSFactStorage& FS = enemy->m_factStorage;
		
		// fact of seeing
		FS.MovingInsertOrUpdate( TSFactStorage::FT_Sight_Foe,
			enemypos, 10, curtime, curtime + 5*1000 );
		
		// fact of position
		FS.MovingInsertOrUpdate( TSFactStorage::FT_Position_Foe,
			enemypos, 10, curtime, curtime + 30*1000, FS.last_mod_id );
		
		return true;
	}
	
	TimeVal curtime;
	TSEnemy* enemy;
};

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
	
	TimeVal curTime = g_GameLevel->GetPhyTime();
	
	// process facts
	m_factStorage.Process( curTime );
	// - vision
	IESEnemyViewProc evp;
	evp.curtime = curTime;
	evp.enemy = this;
	g_GameLevel->m_infoEmitters.QuerySphereAll( &evp, GetPosition(), 10.0f, IEST_Player );
	// - sounds
	for( int i = 0; i < g_GameLevel->m_aidbSystem.GetNumSounds(); ++i )
	{
		if( g_GameLevel->m_aidbSystem.CanHearSound( GetPosition(), i ) == false )
			continue;
		AISound S = g_GameLevel->m_aidbSystem.GetSoundInfo( i );
		
		if( S.type == AIS_Footstep || S.type == AIS_Shot )
		{
			TSFactStorage::FactType sndtype = TSFactStorage::FT_Sound_Footstep;
			if( S.type == AIS_Shot )
				sndtype = TSFactStorage::FT_Sound_Shot;
			
			m_factStorage.MovingInsertOrUpdate( sndtype,
				S.position, 10, curTime, curTime + 1*1000 );
			
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
	
	TSCharacter::FixedTick( deltaTime );
	
	InfoEmissionSystem::Data D = { GetPosition(), 0.5f, IEST_MapItem };
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

bool TSEnemy::GetMapItemInfo( MapItemInfo* out )
{
	out->type = MI_Object_Enemy | MI_State_Normal;
	out->position = GetInterpPos();
	out->direction = GetInterpAimDir();
	out->sizeFwd = 10;
	out->sizeRight = 8;
	return true;
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
	Vec3 screenpos = g_GameLevel->m_scene->camera.WorldToScreen( pos, &infront );
	if( !infront )
		return;
	int x = screenpos.x * GR_GetWidth();
	int y = screenpos.y * GR_GetHeight();
	
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


#endif


