

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#define USE_SERIALIZATION
#include "game.hpp"


Entity::~Entity()
{
	g_GameLevel->UnmapEntityByName( this );
}


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


void InfoEmissionSystem::UpdateEmitter( Entity* e, const Data& data )
{
	m_emissionData[ e ] = data;
}

void InfoEmissionSystem::RemoveEmitter( Entity* e )
{
	m_emissionData.unset( e );
}

bool InfoEmissionSystem::QueryAny( const Vec3& pos, float rad, uint32_t types )
{
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		float dst = rad + D.radius;
		if( ( D.pos - pos ).LengthSq() < dst * dst )
			return true;
	}
	return false;
}

bool InfoEmissionSystem::QueryBB( const Mat4& mtx, uint32_t types )
{
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		Vec3 tp = mtx.TransformPos( D.pos );
		if( tp.x >= -1 && tp.x <= 1 &&
			tp.y >= -1 && tp.y <= 1 &&
			tp.z >= -1 && tp.z <= 1 )
			return true;
	}
	return false;
}

Entity* InfoEmissionSystem::QueryOneRay( const Vec3& from, const Vec3& to, uint32_t types )
{
	Vec3 dir = to - from;
	float maxdist = dir.Length();
	dir /= maxdist;
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		
		float outdst[1];
		if( RaySphereIntersect( from, dir, D.pos, D.radius, outdst ) && outdst[0] < maxdist )
			return m_emissionData.item( i ).key;
	}
	return NULL;
}


MessagingSystem::MessagingSystem()
{
	m_tx_icon_info = GR_GetTexture( "ui/icon_info.png" );
	m_tx_icon_warning = GR_GetTexture( "ui/icon_warning.png" );
	m_tx_icon_cont = GR_GetTexture( "ui/icon_cont.png" );
}

void MessagingSystem::AddMessage( MSMessage::Type type, const StringView& sv, float tmlength )
{
	m_messages.push_back( MSMessage() );
	MSMessage& msg = m_messages.last();
	msg.type = type;
	msg.text = sv;
	msg.tmlength = tmlength;
	msg.position = 0;
}

void MessagingSystem::Tick( float dt )
{
	for( size_t i = m_messages.size(); i > 0; )
	{
		i--;
		MSMessage& msg = m_messages[ i ];
		msg.position += dt;
		if( msg.position > msg.tmlength + 2 )
		{
			m_messages.erase( i );
		}
	}
}

void MessagingSystem::DrawUI()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetFont( "fonts/lato-regular.ttf", 16 );
	
	float y = 0;
	
	for( size_t i = 0; i < m_messages.size(); ++i )
	{
		MSMessage& msg = m_messages[ i ];
		float q = smoothlerp_range( msg.position, 0, 1, 1 + msg.tmlength, 2 + msg.tmlength );
		y += 10 - ( 1 - q ) * 50;
		
		br.Reset();
		br.Col( 0, 0.5f * q );
		br.Quad( 100, y, 450, y + 40 );
		br.Col( 1 );
		switch( msg.type )
		{
		case MSMessage::Info: br.SetTexture( m_tx_icon_info ); break;
		case MSMessage::Warning: br.SetTexture( m_tx_icon_warning ); break;
		case MSMessage::Continued:
		default:
			br.SetTexture( m_tx_icon_cont );
			break;
		}
		br.Quad( 110, y+10, 130, y+30 );
		br.UnsetTexture();
		br.Col( 1, 0.9f * q );
		GR2D_DrawTextLine( 150, y + 20, msg.text, HALIGN_LEFT, VALIGN_CENTER );
		
		y += 40;
	}
}


ObjectiveSystem::ObjectiveSystem() :
	m_alpha(0)
{
	m_tx_icon_open = GR_GetTexture( "ui/obj_open.png" );
	m_tx_icon_done = GR_GetTexture( "ui/obj_done.png" );
	m_tx_icon_failed = GR_GetTexture( "ui/obj_failed.png" );
}

int ObjectiveSystem::AddObjective( const StringView& sv, OSObjective::State state )
{
	int out = m_objectives.size();
	m_objectives.push_back( OSObjective() );
	OSObjective& obj = m_objectives.last();
	obj.text = sv;
	obj.state = state;
	return out;
}

void ObjectiveSystem::Tick( float dt )
{
	float tgt = SHOW_OBJECTIVES.value ? 1 : 0;
	float diff = tgt - m_alpha;
	m_alpha += sign( diff ) * TMIN( diff > 0 ? dt * 3 : dt, fabsf( diff ) );
}

void ObjectiveSystem::DrawUI()
{
	int W = GR_GetWidth();
	int H = GR_GetHeight();
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetFont( "fonts/lato-regular.ttf", 16 );
	
	br.Reset();
	br.Col( 0, 0.5f * m_alpha );
	br.Quad( 0, 0, W, H );
	
	br.Col( 1, m_alpha );
	GR2D_DrawTextLine( 200, 10, "Objectives:" );
	
	float y = 40;
	
	for( size_t i = 0; i < m_objectives.size(); ++i )
	{
		OSObjective& obj = m_objectives[ i ];
		if( obj.state == OSObjective::Hidden )
			continue;
		
		switch( obj.state )
		{
		default:
		case OSObjective::Open: br.SetTexture( m_tx_icon_open ); break;
		case OSObjective::Done: br.SetTexture( m_tx_icon_done ); break;
		case OSObjective::Failed: br.SetTexture( m_tx_icon_failed ); break;
		}
		if( obj.state != OSObjective::Cancelled )
		{
			br.Quad( 210, y, 240, y + 30 );
		}
		
		GR2D_DrawTextLine( 250, y+15, obj.text, HALIGN_LEFT, VALIGN_CENTER );
		if( obj.state == OSObjective::Cancelled )
		{
			br.Reset();
			br.Col( 1, m_alpha );
			br.Quad( 250, y + 17, 250 + GR2D_GetTextLength( obj.text ), y + 19 );
		}
		y += 30;
	}
}


BulletSystem::BulletSystem( DamageSystem* dmgsys ) :
	m_damageSystem( dmgsys )
{
	m_tx_bullet = GR_GetTexture( "textures/particles/bullet.png" );
}

void BulletSystem::Tick( float deltaTime )
{
	for( size_t i = 0; i < m_bullets.size(); ++i )
	{
		Bullet& B = m_bullets[i];
		
		B.timeleft -= deltaTime;
		if( B.timeleft <= 0 )
		{
			m_bullets.erase( i-- );
			break;
		}
		Vec2 p1 = B.position;
		Vec2 p2 = p1 + B.velocity * deltaTime;
		bool rcr = false; // g_ColWorld.raycast( p1.x, p1.y, p2.x, p2.y );
		if( rcr )
		{
			m_bullets.erase( i-- );
	//		if( rcr.eid >= 0 )                   TODO RAYCAST
	//		{
	//			g_World.send_message( rcr.eid, "bullet_hit", [B.dmg * B.velocity, rcr.pos, rcr.normal] );
	//		}
	//		else
	//			g_World.add_entity( EntityBuilder.build( "fx_wall_hit", { x = rcr.pos.x, y = rcr.pos.y, nx = rcr.normal.x, ny = rcr.normal.y } ) );
		}
		else
			B.position = p2;
	}
}

void BulletSystem::Draw2D()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.SetTexture( m_tx_bullet );
	br.Col( 1 );
	for( size_t i = 0; i < m_bullets.size(); ++i )
	{
		const Bullet& B = m_bullets[i];
		br.TurnedBox( B.position.x, B.position.y, B.dir.x * 2, B.dir.y * 2 );
	}
}

void BulletSystem::Add( const Vec2& pos, const Vec2& vel, float timeleft, float dmg )
{
	Bullet B = { pos, vel, vel.Normalized(), timeleft, dmg };
	m_bullets.push_back( B );
}

void BulletSystem::Clear()
{
	m_bullets.clear();
}


static int SendMessage( SGS_CTX )
{
	SGSFN( "SendMessage" );
	int ssz = sgs_StackSize( C );
	g_GameLevel->m_messageSystem.AddMessage( (MSMessage::Type) sgs_GetVar<int>()( C, 0 ), sgs_GetVar<String>()( C, 1 ), ssz < 3 ? 3.0f : sgs_GetVar<float>()( C, 2 ) );
	return 0;
}
static int EndLevel( SGS_CTX )
{
	SGSFN( "EndLevel" );
	g_GameLevel->m_endFactor = 0;
	return 0;
}
static int CallEntity( SGS_CTX )
{
	SGSFN( "CallEntity" );
	g_GameLevel->CallEntityByName( sgs_GetVar<String>()( C, 0 ), sgs_GetVar<String>()( C, 1 ) );
	return 0;
}
static int PlayerHasItem( SGS_CTX )
{
	SGSFN( "PlayerHasItem" );
	sgs_PushVar<bool>( C, g_GameLevel->m_player && g_GameLevel->m_player->HasItem( sgs_GetVar<String>()( C, 0 ), sgs_GetVar<int>()( C, 1 ) ) );
	return 1;
}
static int ObjectiveAdd( SGS_CTX )
{
	SGSFN( "ObjectiveAdd" );
	sgs_PushVar<int>( C, g_GameLevel->m_objectiveSystem.AddObjective( sgs_GetVar<String>()( C, 0 ), (OSObjective::State) sgs_GetVar<int>()( C, 1 ) ) );
	return 1;
}
static int ObjectiveGetState( SGS_CTX )
{
	SGSFN( "ObjectiveGetState" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	sgs_PushVar<int>( C, g_GameLevel->m_objectiveSystem.m_objectives[ which ].state );
	return 1;
}
static int ObjectiveGetText( SGS_CTX )
{
	SGSFN( "ObjectiveGetText" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	sgs_PushVar( C, g_GameLevel->m_objectiveSystem.m_objectives[ which ].text );
	return 1;
}
static int ObjectiveSetState( SGS_CTX )
{
	SGSFN( "ObjectiveSetState" );
	int which = sgs_GetVar<int>()( C, 0 );
	if( which < 0 || which >= (int) g_GameLevel->m_objectiveSystem.m_objectives.size() )
		return sgs_Msg( C, SGS_WARNING, "index out of bounds: %d", which );
	int state = sgs_GetVar<int>()( C, 1 );
	g_GameLevel->m_objectiveSystem.m_objectives[ which ].state = (OSObjective::State) state;
	return 0;
}

static sgs_RegFuncConst g_gameapi_rfc[] =
{
	{ "SendMessage", SendMessage },
	{ "EndLevel", EndLevel },
	{ "CallEntity", CallEntity },
	{ "PlayerHasItem", PlayerHasItem },
	{ "ObjectiveAdd", ObjectiveAdd },
	{ "ObjectiveGetText", ObjectiveGetText },
	{ "ObjectiveGetState", ObjectiveGetState },
	{ "ObjectiveSetState", ObjectiveSetState },
	SGS_RC_END(),
};

static sgs_RegIntConst g_gameapi_ric[] =
{
	{ "MT_Continued", MSMessage::Continued },
	{ "MT_Info", MSMessage::Info },
	{ "MT_Warning", MSMessage::Warning },
	{ "OS_Hidden", OSObjective::Hidden },
	{ "OS_Open", OSObjective::Open },
	{ "OS_Done", OSObjective::Done },
	{ "OS_Failed", OSObjective::Failed },
	{ "OS_Cancelled", OSObjective::Cancelled },
	{ NULL, 0 },
};

static int InitGameAPI( SGS_CTX )
{
	sgs_RegFuncConsts( C, g_gameapi_rfc, -1 );
	sgs_RegIntConsts( C, g_gameapi_ric, -1 );
	return 0;
}


GameLevel::GameLevel() :
	m_nameIDGen( 0 ),
	m_bulletSystem( &m_damageSystem ),
	m_paused( false ),
	m_endFactor( -1 ),
	m_levelTime( 0 ),
	m_player( NULL )
{
	m_playerSpawnInfo[0] = V3(0);
	m_levelCameraInfo[0] = V3(0);
	m_playerSpawnInfo[1] = V3(1,0,0);
	m_levelCameraInfo[1] = V3(1,0,0);
	
	m_scene = GR_CreateScene();
	m_scene->camera.position = Vec3::Create( 4, 4, 4 );
	m_scene->camera.direction = Vec3::Create( -1, -1, -1 ).Normalized();
	m_scene->camera.aspect = 1024.0f / 576.0f;
	m_scene->camera.UpdateMatrices();
	
	m_ps_flare = GR_GetPixelShader( "flare" );
	m_tex_flare = GR_GetTexture( "textures/fx/flare.png" );
	m_tex_mapline = GR_GetTexture( "ui/mapline.png" );
	m_tex_mapframe = GR_GetTexture( "ui/mapframe.png" );
	
	InitGameAPI( m_scriptCtx.C );
}

GameLevel::~GameLevel()
{
	ClearLevel();
}


bool GameLevel::Load( const StringView& levelname )
{
	char bfr[ 256 ];
	snprintf( bfr, sizeof(bfr), "levels/%.*s/cache", TMIN( (int) levelname.size(), 200 ), levelname.data() );
	ByteArray ba;
	if( !FS_LoadBinaryFile( bfr, ba ) )
		return false;
	
	ClearLevel();
	
	snprintf( bfr, sizeof(bfr), "levels/%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data() );
	m_scriptCtx.Include( bfr );
	
	ByteReader br( &ba );
	
	br.marker( "COMPILED" );
	SerializeVersionHelper<ByteReader> svh( br, 1 );
	
	svh.marker( "SCRENTS" );
	Array< LC_ScriptedEntity > screntdefs;
	svh << screntdefs;
	
	svh.marker( "INST" );
	Array< LC_MeshInst > mesh_inst_defs;
	svh << mesh_inst_defs;
	
	svh.marker( "LINES" );
	svh << m_lines;
	
	svh.marker( "LIGHTS" );
	svh << m_lights;
	
	svh.marker( "SAMPLES" );
	Array< LightTree::Sample > lt_samples;
	svh << lt_samples;
	
	// LOAD LIGHT SAMPLES
//	for( size_t i = 0; i < lt_samples.size(); ++i )
//	{
//		LOG << lt_samples[i].pos << "|" << lt_samples[i].color[0] << lt_samples[i].color[1] << lt_samples[i].color[2] << lt_samples[i].color[3] << lt_samples[i].color[4] << lt_samples[i].color[5];
//	}
//	m_ltSamples.InsertSample( lt_samples[ 10 ] );
	m_ltSamples.InsertSamples( lt_samples.data(), lt_samples.size() );
	
	
	// CREATE STATIC GEOMETRY
	SGRX_PhyRigidBodyInfo rbinfo;
	
	for( size_t i = 0; i < mesh_inst_defs.size(); ++i )
	{
		char subbfr[ 512 ];
		MeshInstHandle MI = m_scene->CreateMeshInstance();
		StringView src = mesh_inst_defs[ i ].m_meshname;
		if( src.ch() == '~' )
		{
			snprintf( subbfr, sizeof(subbfr), "levels/%.*s%.*s", TMIN( (int) levelname.size(), 200 ), levelname.data(), TMIN( (int) src.size() - 1, 200 ), src.data() + 1 );
			MI->mesh = GR_GetMesh( subbfr );
		}
		else
			MI->mesh = GR_GetMesh( src );
		
		snprintf( subbfr, sizeof(subbfr), "levels/%.*s/%d.png", TMIN( (int) levelname.size(), 200 ), levelname.data(), (int) i );
		MI->textures[0] = GR_GetTexture( subbfr );
		
		MI->matrix = mesh_inst_defs[ i ].m_mtx;
		
		if( mesh_inst_defs[ i ].m_flags & LM_MESHINST_DYNLIT )
		{
			MI->dynamic = true;
			LightMesh( MI );
		}
		
		m_meshInsts.push_back( MI );
		
		if( mesh_inst_defs[ i ].m_flags & LM_MESHINST_SOLID )
		{
			rbinfo.shape = g_PhyWorld->CreateShapeFromMesh( MI->mesh );
			rbinfo.shape->SetScale( MI->matrix.GetScale() );
			rbinfo.position = MI->matrix.GetTranslation();
			rbinfo.rotation = MI->matrix.GetRotationQuaternion();
			m_levelBodies.push_back( g_PhyWorld->CreateRigidBody( rbinfo ) );
		}
	}
	
	
	// CREATE ENTITIES
	for( size_t i = 0; i < screntdefs.size(); ++i )
	{
		CreateEntity( screntdefs[ i ].type, screntdefs[ i ].serialized_params );
	}
	
	
	return true;
}

void GameLevel::ClearLevel()
{
	EndLevel();
	m_ltSamples.Clear();
	m_damageSystem.Clear();
	m_bulletSystem.Clear();
	m_lights.clear();
	m_meshInsts.clear();
	m_levelBodies.clear();
}

void GameLevel::CreateEntity( const StringView& type, const StringView& sgsparams )
{
	sgsVariable data = m_scriptCtx.Unserialize( sgsparams );
	if( data.not_null() == false )
	{
		LOG << "BAD PARAMS FOR ENTITY " << type;
		return;
	}
	
	///////////////////////////
	if( type == "player_start" )
	{
		m_playerSpawnInfo[0] = data.getprop("position").get<Vec3>();
		m_playerSpawnInfo[1] = data.getprop("viewdir").get<Vec3>().Normalized();
		return;
	}
	
	///////////////////////////
	if( type == "camera_start" )
	{
		m_levelCameraInfo[0] = data.getprop("position").get<Vec3>();
		m_levelCameraInfo[1] = data.getprop("viewdir").get<Vec3>().Normalized();
		return;
	}
	
	///////////////////////////
	if( type == "solidbox" )
	{
		Vec3 scale = data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>();
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.shape = g_PhyWorld->CreateAABBShape( -scale, scale );
		rbinfo.mass = 0;
		rbinfo.inertia = V3(0);
		rbinfo.position = data.getprop("position").get<Vec3>();
		rbinfo.rotation = Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion();
		m_levelBodies.push_back( g_PhyWorld->CreateRigidBody( rbinfo ) );
		return;
	}
	
	///////////////////////////
	if( type == "trigger" )
	{
		BoxTrigger* BT = new BoxTrigger
		(
			data.getprop("func").get<String>(),
			data.getprop("target").get<String>(),
			data.getprop("once").get<bool>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>()
		);
		m_entities.push_back( BT );
		return;
	}
	
	///////////////////////////
	if( type == "trigger_prox" )
	{
		ProximityTrigger* PT = new ProximityTrigger
		(
			data.getprop("func").get<String>(),
			data.getprop("target").get<String>(),
			data.getprop("once").get<bool>(),
			data.getprop("position").get<Vec3>(),
			data.getprop("distance").get<float>()
		);
		m_entities.push_back( PT );
		return;
	}
	
	///////////////////////////
	if( type == "door_slide" )
	{
		SlidingDoor* SD = new SlidingDoor
		(
			data.getprop("name").get<String>(),
			data.getprop("mesh").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("open_offset").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("open_rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			V3(0),
			Quat::Identity,
			data.getprop("open_time").get<float>(),
			false,
			data.getprop("is_switch").get<bool>(),
			data.getprop("pred").get<String>(),
			data.getprop("func").get<String>(),
			data.getprop("target").get<String>(),
			data.getprop("once").get<bool>()
		);
		m_entities.push_back( SD );
		return;
	}
	
	///////////////////////////
	if( type == "door_slide_prox" )
	{
		StackShortName name = GenerateName();
		
		SlidingDoor* SD = new SlidingDoor
		(
			name.str,
			data.getprop("mesh").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("open_offset").get<Vec3>(),
			Quat::Identity,
			V3(0),
			Quat::Identity,
			data.getprop("open_time").get<float>(),
			false
		);
		m_entities.push_back( SD );
		
		ProximityTrigger* PT = new ProximityTrigger
		(
			"", name.str, false,
			SD->meshInst->matrix.TransformPos( data.getprop("scan_offset").get<Vec3>() ),
			data.getprop("scan_distance").get<float>()
		);
		m_entities.push_back( PT );
		return;
	}
	
	///////////////////////////
	if( type == "pickup" )
	{
		PickupItem* PI = new PickupItem
		(
			data.getprop("id").get<String>(),
			data.getprop("name").get<String>(),
			data.getprop("count").get<int>(),
			data.getprop("mesh").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>()
		);
		m_entities.push_back( PI );
		return;
	}
	
	///////////////////////////
	if( type == "particle_fx" )
	{
		ParticleFX* PF = new ParticleFX
		(
			data.getprop("name").get<String>(),
			data.getprop("partsys").get<String>(),
			data.getprop("soundevent").get<String>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("start").get<bool>()
		);
		m_entities.push_back( PF );
		return;
	}
	
	///////////////////////////
	if( type == "mesharray" ||
		type == "m3sh" )
	{
		return;
	}
	
	LOG << "ENTITY TYPE NOT FOUND: " << type;
}

StackShortName GameLevel::GenerateName()
{
	m_nameIDGen++;
	StackShortName tmp("");
	snprintf( tmp.str, 15, "_name%u", (unsigned int) m_nameIDGen );
	return tmp;
}

void GameLevel::StartLevel()
{
	m_endFactor = -1;
	m_cameraInfoCached = false;
	m_levelTime = 0;
	if( !m_player )
	{
		Player* P = new Player
		(
			m_playerSpawnInfo[0]
#if defined(BRSD4GAME) || defined(LD32GAME)
			,m_playerSpawnInfo[1]
#endif
		);
		m_player = P;
	}
	m_scriptCtx.GlobalCall( "onLevelStart" );
}

void GameLevel::EndLevel()
{
	for( size_t i = 0; i < m_entities.size(); ++i )
		delete m_entities[ i ];
	m_entities.clear();
	
	m_endFactor = -1;
	m_cameraInfoCached = false;
	if( m_player )
	{
		delete m_player;
		m_player = NULL;
	}
}

void GameLevel::FixedTick( float deltaTime )
{
	if( !m_paused )
	{
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->FixedTick( deltaTime );
		if( m_player )
			m_player->FixedTick( deltaTime );
	}
}

void GameLevel::Tick( float deltaTime, float blendFactor )
{
	if( m_endFactor >= 0 )
	{
		m_endFactor = clamp( m_endFactor + deltaTime / 3, 0, 1 );
	}
	
	m_levelTime += deltaTime;
	
	if( !m_player )
	{
		m_scene->camera.position = m_levelCameraInfo[0];
		m_scene->camera.direction = m_levelCameraInfo[1];
		m_scene->camera.up = V3(0,0,1);
		m_scene->camera.aspect = GR_GetWidth() / (float) GR_GetHeight();
		m_scene->camera.UpdateMatrices();
	}
	
	if( !m_paused )
	{
		m_damageSystem.Tick( deltaTime );
		m_bulletSystem.Tick( deltaTime );
		if( m_player )
			m_player->Tick( deltaTime, blendFactor );
		for( size_t i = 0; i < m_entities.size(); ++i )
			m_entities[ i ]->Tick( deltaTime, blendFactor );
	}
	
	m_messageSystem.Tick( deltaTime );
	m_objectiveSystem.Tick( deltaTime );
	
	if( m_endFactor > 0 )
	{
		if( !m_cameraInfoCached )
		{
			m_cachedCameraInfo[0] = m_scene->camera.position;
			m_cachedCameraInfo[1] = m_scene->camera.direction;
			m_cameraInfoCached = true;
		}
		Vec3 p0 = m_cachedCameraInfo[0];
		Vec3 p1 = m_levelCameraInfo[0];
		Vec3 d0 = m_cachedCameraInfo[1];
		Vec3 d1 = m_levelCameraInfo[1];
		float q = smoothstep( m_endFactor );
		m_scene->camera.position = TLERP( p0, p1, q );
		m_scene->camera.direction = TLERP( d0, d1, q ).Normalized();
		m_scene->camera.UpdateMatrices();
	}
}

void GameLevel::Draw2D()
{
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	m_bulletSystem.Draw2D();
	m_damageSystem.Draw2D();
	m_messageSystem.DrawUI();
	m_objectiveSystem.DrawUI();
//	if( m_player )
//		m_player->Draw2D();
	
	// LIGHTS
//	BatchRenderer& br = GR2D_GetBatchRenderer();
//	for( size_t i = 0; i < m_lights.size(); ++i )
//	{
//		Light& L = m_lights[ i ];
//		br.SetTexture( L.tex_flare ).Col( L.color.x, L.color.y, L.color.z, 0.2 );
//		br.Box( L.pos.x, L.pos.y, L.radius, L.radius );
//	}
	
	//
	// UI
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	
	int size_x = GR_GetWidth();
	int size_y = GR_GetHeight();
//	float aspect = size_x / (float) size_y;
	int sqr = TMIN( size_x, size_y );
//	int margin_x = ( size_x - sqr ) / 2;
//	int margin_y = ( size_y - sqr ) / 2;
	int safe_margin = sqr * 1 / 16;
	
#ifndef BRSD4GAME
	// MAP
	{
		int mapsize_x = sqr * 4 / 10;
		int mapsize_y = sqr * 3 / 10;
		int msm = 0; // sqr / 100;
		float map_aspect = mapsize_x / (float) mapsize_y;
		int x1 = size_x - safe_margin;
		int x0 = x1 - mapsize_x;
		int y0 = safe_margin;
		int y1 = y0 + mapsize_y;
		
		br.Reset().Col( 0, 0.5f );
		br.Quad( x0, y0, x1, y1 );
		br.Flush();
		
		br.Reset().SetTexture( NULL ).Col( 0.2f, 0.4f, 0.8f );
		Vec2 pos = m_scene->camera.position.ToVec2();
		Mat4 lookat = Mat4::CreateLookAt( V3( pos.x, pos.y, -0.5f ), V3(0,0,1), V3(0,-1,0) );
		GR2D_SetViewMatrix( lookat * Mat4::CreateScale( 1.0f / ( 8 * map_aspect ), 1.0f / 8, 1 ) );
		
		GR2D_SetScissorRect( x0, y0, x1, y1 );
		GR2D_SetViewport( x0, y0, x1, y1 );
		
		for( size_t i = 0; i < m_lines.size(); i += 2 )
		{
			Vec2 l0 = m_lines[ i ];
			Vec2 l1 = m_lines[ i + 1 ];
			
			br.TexLine( l0, l1, 0.1f );
		}
		
		br.Reset().SetTexture( m_tex_mapline ).Col( 0.2f, 0.9f, 0.1f ).Box( pos.x, pos.y, 1, 1 );
		
		br.Flush();
		GR2D_UnsetViewport();
		GR2D_UnsetScissorRect();
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		
		br.Reset().SetTexture( m_tex_mapframe ).Quad( x0 - msm, y0 - msm, x1 + msm, y1 + msm ).Flush();
	}
#endif
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	if( m_player )
		m_player->DrawUI();
}

void GameLevel::PostDraw()
{
//	return;
	
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	float W = GR_GetWidth();
	float H = GR_GetHeight();
	float sz = TMIN( W, H ) * 0.2f;
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().SetShader( m_ps_flare ).SetTexture( m_tex_flare );
	br.ShaderData.push_back( V4( W, H, 1.0f / W, 1.0f / H ) );
	br.ShaderData.push_back( V4(1) );
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		Vec3 flarepos = m_lights[ i ].pos + m_lights[ i ].flareoffset;
		float flaresize = m_lights[ i ].flaresize;
		if( flaresize <= 0 )
			continue;
		br.ShaderData[1] = V4( m_lights[ i ].color, 0.1f / ( ( flarepos - m_scene->camera.position ).Length() + 1 ) );
		Vec3 screenpos = m_scene->camera.WorldToScreen( flarepos );
		if( Vec3Dot( flarepos, m_scene->camera.direction ) < Vec3Dot( m_scene->camera.position, m_scene->camera.direction ) )
			continue;
		if( g_PhyWorld->Raycast( flarepos, m_scene->camera.position, 1, 1 ) )
			continue;
	//	LOG << screenpos.z;
		float dx = cos(0.1f)*0.5f*sz * flaresize;
		float dy = sin(0.1f)*0.5f*sz * flaresize;
		br.TurnedBox( screenpos.x * W, screenpos.y * H, dx, dy );
		br.Flush();
	}
}

void GameLevel::Draw()
{
	SGRX_RenderScene rsinfo( V4( m_levelTime ), m_scene );
//	rsinfo.debugdraw = this;
	rsinfo.postdraw = this;
	GR_RenderScene( rsinfo );
}

void GameLevel::MapEntityByName( Entity* e )
{
	m_entNameMap[ e->m_name ] = e;
}

void GameLevel::UnmapEntityByName( Entity* e )
{
	m_entNameMap.unset( e->m_name );
}

void GameLevel::CallEntityByName( const StringView& name, const StringView& action )
{
	Entity* e = m_entNameMap.getcopy( name );
	if( e )
		e->OnEvent( action );
}


void GameLevel::LightMesh( MeshInstHandle mih, int32_t* outlastfound )
{
	LightTree::Sample SMP = { mih->matrix.TransformPos( V3(0) ) };
	m_ltSamples.Interpolate( SMP, outlastfound );
	mih->constants[10] = V4( SMP.color[0], 1 );
	mih->constants[11] = V4( SMP.color[1], 1 );
	mih->constants[12] = V4( SMP.color[2], 1 );
	mih->constants[13] = V4( SMP.color[3], 1 );
	mih->constants[14] = V4( SMP.color[4], 1 );
	mih->constants[15] = V4( SMP.color[5], 1 );
}

