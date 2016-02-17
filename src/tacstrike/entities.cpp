

#include "entities.hpp"


extern Vec2 CURSOR_POS;


Trigger::Trigger( GameLevel* lev, bool laststate ) :
	Entity( lev ), m_once( true ), m_done( false ), m_lastState( laststate ), m_currState( false )
{
}

void Trigger::Invoke( bool newstate )
{
	sgsVariable& curfn = newstate ? m_func : m_funcOut;
	if( curfn.not_null() )
	{
		SGS_CSCOPE( m_level->m_scriptCtx.C );
		m_level->m_scriptCtx.Push( newstate );
		m_level->m_scriptCtx.Call( curfn, 1 );
	}
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

void Trigger::sgsSetupTrigger( bool once, sgsVariable fn, sgsVariable fnout )
{
	m_done = false;
	m_once = once;
	m_func = fn;
	if( sgs_StackSize( C ) < 3 )
		m_funcOut = fn;
	else if( sgs_ItemType( C, 2 ) == SGS_VT_BOOL )
		m_funcOut = sgs_GetBool( C, 2 ) ? fn : sgsVariable();
	else
		m_funcOut = fnout;
}


BoxTrigger::BoxTrigger( GameLevel* lev, StringView name, const Vec3& pos, const Quat& rot, const Vec3& scl ) :
	Trigger( lev ), m_matrix( Mat4::CreateSRT( scl, rot, pos ) )
{
	m_name = name;
	m_level->MapEntityByName( this );
}

void BoxTrigger::FixedTick( float deltaTime )
{
	Update( m_level->QueryOBB( NULL, IEST_Player, m_matrix ) );
}


ProximityTrigger::ProximityTrigger( GameLevel* lev, StringView name, const Vec3& pos, float rad ) :
	Trigger( lev ), m_position( pos ), m_radius( rad )
{
	m_name = name;
	m_level->MapEntityByName( this );
}

void ProximityTrigger::FixedTick( float deltaTime )
{
	Update( m_level->QuerySphere( NULL, IEST_Player, m_position, m_radius ) );
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
	m_level->LightMesh( meshInst );
}

SlidingDoor::SlidingDoor
(
	GameLevel* lev,
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
	bool isswitch
):
	Trigger( lev, true ),
	open_factor( istate ), open_target( istate ), open_time( TMAX( otime, SMALL_FLOAT ) ),
	pos_open( oopen ), pos_closed( oclos ), rot_open( ropen ), rot_closed( rclos ),
	target_state( istate ), m_isSwitch( isswitch ),
	position( pos ), rotation( rot ), scale( scl ),
	m_bbMin( V3(-1) ), m_bbMax( V3(1) ),
	m_ivPos( V3(0) ), m_ivRot( Quat::Identity )
{
	m_name = name;
	meshInst = m_level->GetScene()->CreateMeshInstance();
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	meshInst->SetMesh( bfr );
	
	if( meshInst->GetMesh() )
	{
		m_bbMin = meshInst->GetMesh()->m_boundsMin;
		m_bbMax = meshInst->GetMesh()->m_boundsMax;
	}
	
	if( m_isSwitch == false )
	{
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.shape = m_level->GetPhyWorld()->CreateAABBShape( m_bbMin, m_bbMax );
		rbinfo.shape->SetScale( scale );
		rbinfo.kinematic = true;
		rbinfo.position = position;
		rbinfo.rotation = rotation;
		rbinfo.mass = 0;
		rbinfo.inertia = V3(0);
		bodyHandle = m_level->GetPhyWorld()->CreateRigidBody( rbinfo );
		soundEvent = m_level->GetSoundSys()->CreateEventInstance( "/gate_open" );
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
	
	m_level->MapEntityByName( this );
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
			SetInfoMask( IEST_InteractiveItem );
		}
		else
			SetInfoMask( 0 );
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
			SGS_CSCOPE( m_level->m_scriptCtx.C );
			m_level->m_scriptCtx.Push( newstate );
			m_level->m_scriptCtx.Call( m_switchPred, 1, 1 );
			bool val = sgs_GetVar<bool>()( m_level->m_scriptCtx.C, -1 );
			if( !val )
				return;
		}
		
		target_state = newstate;
		
		if( m_isSwitch )
		{
			Update( target_state );
		}
	}
}


PickupItem::PickupItem( GameLevel* lev, const StringView& name, const StringView& type,
	int count, const StringView& mesh, const Vec3& pos, const Quat& rot, const Vec3& scl ) :
	Entity( lev ), m_type( type ), m_count( count ), m_pos( pos )
{
	m_name = name;
	m_meshInst = m_level->GetScene()->CreateMeshInstance();
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	m_meshInst->SetMesh( bfr );
	m_meshInst->matrix = Mat4::CreateSRT( scl, rot, pos );
	m_level->LightMesh( m_meshInst );
	
	SetInfoMask( IEST_InteractiveItem );
}

void PickupItem::OnEvent( const StringView& type )
{
	if( ( type == "trigger_switch" || type == "action_end" ) )
	{
		SGS_SCOPE;
		
		sgsVariable scrobj = GetScriptedObject();
		scrobj.push( C );
		scrobj.getprop( "level" ).thiscall( C, "onPickupItem", 1, 1 );
		bool keep = sgs_GetVar<bool>()( C, -1 );
		if( keep == false )
		{
			SetInfoMask( 0 );
			m_meshInst->enabled = false;
		}
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


Actionable::Actionable( GameLevel* lev, const StringView& name, const StringView& mesh,
	const Vec3& pos, const Quat& rot, const Vec3& scl, const Vec3& placeoff, const Vec3& placedir ) :
	Entity( lev )
{
	Mat4 mtx = Mat4::CreateSRT( scl, rot, pos );
	
	m_info.type = IT_Investigate;
	m_info.placePos = mtx.TransformPos( placeoff );
	m_info.placeDir = mtx.TransformNormal( placedir ).Normalized();
	m_info.timeEstimate = 0.5f;
	m_info.timeActual = 0.5f;
	
	m_name = name;
//	m_viewName = name;
	m_meshInst = m_level->GetScene()->CreateMeshInstance();
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "meshes/%.*s.ssm", TMIN( 240, (int) mesh.size() ), mesh.data() );
	m_meshInst->SetMesh( bfr );
	m_meshInst->matrix = mtx;
	m_level->LightMesh( m_meshInst );
	
	SetEnabled( true );
	
	m_level->MapEntityByName( this );
}

void Actionable::OnEvent( const StringView& type )
{
	if( m_enabled )
	{
		if( type == "action_start" )
		{
			// start animation?
		}
		else if( type == "action_end" )
		{
			// end animation?
			SGS_SCOPE;
			GetScriptedObject().push( C );
			m_onSuccess.call( C, 1, 1 );
			bool keep = sgs_GetVar<bool>()( C, -1 );
			if( keep == false )
			{
				SetInfoMask( 0 );
			}
		}
	}
}

bool Actionable::GetInteractionInfo( Vec3 pos, InteractInfo* out )
{
	if( m_enabled == false )
		return false;
	*out = m_info;
	return true;
}

void Actionable::SetEnabled( bool v )
{
	m_enabled = v;
	if( v )
	{
		SetInfoMask( IEST_InteractiveItem );
	}
	else
	{
		SetInfoMask( 0 );
	}
}


ParticleFX::ParticleFX( GameLevel* lev, const StringView& name, const StringView& psys, const StringView& sndev, const Vec3& pos, const Quat& rot, const Vec3& scl, bool start ) :
	Entity( lev ), m_soundEventName( sndev ), m_position( pos )
{
	m_name = name;
	m_soundEventOneShot = m_level->GetSoundSys()->EventIsOneShot( sndev );
	
	char bfr[ 256 ] = {0};
	sgrx_snprintf( bfr, sizeof(bfr), "psys/%.*s.psy", TMIN( 240, (int) psys.size() ), psys.data() );
	m_psys.Load( bfr );
	m_psys.AddToScene( m_level->GetScene() );
	m_psys.SetTransform( Mat4::CreateSRT( scl, rot, pos ) );
	m_psys.OnRenderUpdate();
	
	if( start )
		OnEvent( "trigger_enter" );
	
	m_level->MapEntityByName( this );
}

void ParticleFX::Tick( float deltaTime, float blendFactor )
{
	bool needstrig = m_psys.Tick( deltaTime );
	if( needstrig && m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
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
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
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
			SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
			if( sndevinst )
			{
				SGRX_Sound3DAttribs s3dattr = { m_position, V3(0), V3(0), V3(0) };
				sndevinst->Set3DAttribs( s3dattr );
				sndevinst->Start();
			}
		}
	}
}


ScriptedItem::ScriptedItem( GameLevel* lev, const StringView& name, sgsVariable args ) : Entity( lev ), m_scrItem(NULL)
{
	char bfr[ 256 ];
	sgrx_snprintf( bfr, 256, "SCRITEM_CREATE_%s", StackString<200>(name).str );
	sgsVariable func = m_level->m_scriptCtx.GetGlobal( bfr );
	if( func.not_null() )
	{
		sgs_Variable pvar = sgs_MakePtr( this );
		args.setprop( "__entity", sgsVariable( m_level->m_scriptCtx.C, &pvar ) );
		
		m_scrItem = SGRX_ScriptedItem::Create(
			m_level->GetScene(), m_level->GetPhyWorld(), m_level->m_scriptCtx.C,
			func, args );
		m_scrItem->SetLightSampler( m_level );
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

void ScriptedItem::OnEvent( const StringView& type )
{
	if( m_scrItem )
	{
		m_scrItem->EntityEvent( type );
	}
}



#define SCRENT_OFSCHK( i, ret ) if( (i) < 0 || (i) >= 4 ){ \
	sgs_Msg( C, SGS_WARNING, "wrong offset: %d outside " SCRENT_RANGE_STR, (int)(i) ); ret; }
#define SCRENT_MESHCHK( i, ret ) if( m_meshes[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no mesh at offset %d", (int)(i) ); ret; }
#define SCRENT_PSYSCHK( i, ret ) if( m_partSys[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no part.sys at offset %d", (int)(i) ); ret; }
#define SCRENT_BODYCHK( i, ret ) if( m_bodies[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no body at offset %d", (int)(i) ); ret; }
#define SCRENT_JOINTCHK( i, ret ) if( m_joints[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no joint at offset %d", (int)(i) ); ret; }
#define SCRENT_DSYSCHK( ret ) if( m_dmgDecalSys == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no decal sys" ); ret; }


MultiEntity::MultiEntity( GameLevel* lev ) : Entity( lev )
{
	for( int i = 0; i < SCRITEM_NUM_SLOTS; ++i )
	{
		m_meshMatrices[ i ] = Mat4::Identity;
		m_partSysMatrices[ i ] = Mat4::Identity;
	}
}

MultiEntity::~MultiEntity()
{
}

void MultiEntity::FixedTick( float deltaTime )
{
	for( int i = 0; i < SCRENT_NUM_SLOTS; ++i )
	{
		if( m_bodies[ i ] )
		{
			m_bodyPos[ i ].Advance( m_bodies[ i ]->GetPosition() );
			m_bodyRot[ i ].Advance( m_bodies[ i ]->GetRotation() );
		}
	}
	
	Entity::FixedTick( deltaTime );
}

void MultiEntity::Tick( float deltaTime, float blendFactor )
{
	for( int i = 0; i < SCRENT_NUM_SLOTS; ++i )
	{
		if( m_bodies[ i ] )
		{
			m_bodyPosLerp[ i ] = m_bodyPos[ i ].Get( blendFactor );
			m_bodyRotLerp[ i ] = m_bodyRot[ i ].Get( blendFactor );
		}
	}
	
	for( int i = 0; i < SCRENT_NUM_SLOTS; ++i )
	{
		if( m_partSys[ i ] )
			m_partSys[ i ]->Tick( deltaTime );
	}
	
	Entity::Tick( deltaTime, blendFactor );
	
	PreRender();
}

void MultiEntity::PreRender()
{
	for( int i = 0; i < SCRENT_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_level->LightMesh( m_meshes[ i ] );
		if( m_partSys[ i ] )
			m_partSys[ i ]->PreRender();
	}
	if( m_dmgDecalSys )
		m_dmgDecalSys->Upload();
	if( m_ovrDecalSys )
		m_ovrDecalSys->Upload();
	if( m_dmgDecalSys )
		m_level->LightMesh( m_dmgDecalSys->m_meshInst );
	if( m_ovrDecalSys )
		m_level->LightMesh( m_ovrDecalSys->m_meshInst );
}

void MultiEntity::OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data )
{
	if( evid == MIEVT_BulletHit && GetScriptedObject().getprop( "OnHit" ).not_null() )
	{
		SGRX_CAST( MI_BulletHit_Data*, bhinfo, data );
		SGS_SCOPE;
		sgs_CreateVec3p( C, NULL, &bhinfo->pos.x );
		sgs_CreateVec3p( C, NULL, &bhinfo->vel.x );
		int i = 0;
		for( ; i < SCRENT_NUM_SLOTS; ++i )
		{
			if( MI == m_meshes[ i ] )
			{
				sgs_PushInt( C, i );
				break;
			}
		}
		if( i == SCRENT_NUM_SLOTS )
			sgs_PushInt( C, -1 ); // wat
		GetScriptedObject().thiscall( C, "OnHit", 3 );
	}
}

void MultiEntity::OnTransformUpdate()
{
	Mat4 mtx = GetWorldMatrix();
	if( m_dmgDecalSys )
		m_dmgDecalSys->m_meshInst->matrix = mtx;
	if( m_ovrDecalSys )
		m_ovrDecalSys->m_meshInst->matrix = mtx;
	for( int i = 0; i < SCRENT_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_meshes[ i ]->matrix = m_meshMatrices[ i ] * mtx;
		if( m_partSys[ i ] )
			m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * mtx );
	}
}

void MultiEntity::MICreate( int i, StringView path )
{
	SCRENT_OFSCHK( i, return );
	m_meshes[ i ] = m_level->GetScene()->CreateMeshInstance();
	m_meshes[ i ]->matrix = m_meshMatrices[ i ] * GetWorldMatrix();
	m_meshes[ i ]->userData = this;
	if( path )
		m_meshes[ i ]->SetMesh( path );
}

void MultiEntity::MIDestroy( int i )
{
	SCRENT_OFSCHK( i, return );
	m_meshes[ i ] = NULL;
}

bool MultiEntity::MIExists( int i )
{
	SCRENT_OFSCHK( i, return false );
	return m_meshes[ i ] != NULL;
}

void MultiEntity::MISetMesh( int i, StringView path )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_MESHCHK( i, return );
	m_meshes[ i ]->SetMesh( path );
}

void MultiEntity::MISetEnabled( int i, bool enabled )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_MESHCHK( i, return );
	m_meshes[ i ]->enabled = enabled;
}

void MultiEntity::MISetMatrix( int i, Mat4 mtx )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_MESHCHK( i, return );
	m_meshMatrices[ i ] = mtx;
	m_meshes[ i ]->matrix = mtx * GetWorldMatrix();
}

void MultiEntity::MISetShaderConst( int i, int v, Vec4 var )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_MESHCHK( i, return );
	if( v < 0 || v >= MAX_MI_CONSTANTS )
	{
		sgs_Msg( C, SGS_WARNING, "shader constant %d outside range [0;%d)", v, MAX_MI_CONSTANTS );
		return;
	}
	m_meshes[ i ]->constants[ v ] = var;
}

void MultiEntity::MISetLayers( int i, uint32_t layers )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_MESHCHK( i, return );
	m_meshes[ i ]->layers = layers;
}

void MultiEntity::PSCreate( int i, StringView path )
{
	SCRENT_OFSCHK( i, return );
	m_partSys[ i ] = new ParticleSystem;
	m_partSys[ i ]->AddToScene( m_level->GetScene() );
	if( path )
		m_partSys[ i ]->Load( path );
	m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * GetWorldMatrix() );
}

void MultiEntity::PSDestroy( int i )
{
	SCRENT_OFSCHK( i, return );
	m_partSys[ i ] = NULL;
}

bool MultiEntity::PSExists( int i )
{
	SCRENT_OFSCHK( i, return false );
	return m_partSys[ i ] != NULL;
}

void MultiEntity::PSLoad( int i, StringView path )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_PSYSCHK( i, return );
	m_partSys[ i ]->Load( path );
}

void MultiEntity::PSSetMatrix( int i, Mat4 mtx )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_PSYSCHK( i, return );
	m_partSysMatrices[ i ] = mtx;
	m_partSys[ i ]->SetTransform( mtx * GetWorldMatrix() );
}

void MultiEntity::PSSetMatrixFromMeshAABB( int i, int mi )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_PSYSCHK( i, return );
	SCRENT_OFSCHK( mi, return );
	SCRENT_MESHCHK( mi, return );
	SGRX_IMesh* M = m_meshes[ mi ]->GetMesh();
	if( M == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "mesh is not loaded" );
		return;
	}
	Vec3 diff = M->m_boundsMax - M->m_boundsMin;
	Vec3 off = ( M->m_boundsMax + M->m_boundsMin ) * 0.5f;
	Mat4 mtx = Mat4::CreateScale( diff * 0.5f ) * Mat4::CreateTranslation( off );
	m_partSysMatrices[ i ] = mtx;
	m_partSys[ i ]->SetTransform( mtx * GetWorldMatrix() );
}

void MultiEntity::PSPlay( int i )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_PSYSCHK( i, return );
	m_partSys[ i ]->Play();
}

void MultiEntity::PSStop( int i )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_PSYSCHK( i, return );
	m_partSys[ i ]->Stop();
}

void MultiEntity::PSTrigger( int i )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_PSYSCHK( i, return );
	m_partSys[ i ]->Trigger();
}

void MultiEntity::DSCreate( StringView texDmgDecalPath,
	StringView texOvrDecalPath, StringView texFalloffPath, uint32_t size )
{
	if( sgs_StackSize( C ) < 4 )
		size = 64*1024;
	
	m_dmgDecalSys = new SGRX_DecalSystem;
	m_dmgDecalSys->Init( m_level->GetScene(), GR_GetTexture( texDmgDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_dmgDecalSys->SetSize( size );
	m_dmgDecalSys->SetDynamic( true );
	
	dmgDecalSysOverride = m_dmgDecalSys;
	
	m_ovrDecalSys = new SGRX_DecalSystem;
	m_ovrDecalSys->Init( m_level->GetScene(), GR_GetTexture( texOvrDecalPath ), GR_GetTexture( texFalloffPath ) );
	m_ovrDecalSys->SetSize( size );
	m_ovrDecalSys->SetDynamic( true );
	
	ovrDecalSysOverride = m_ovrDecalSys;
}

void MultiEntity::DSDestroy()
{
	m_dmgDecalSys = NULL;
	m_ovrDecalSys = NULL;
	dmgDecalSysOverride = NULL;
	ovrDecalSysOverride = NULL;
}

void MultiEntity::DSResize( uint32_t size )
{
	SCRENT_DSYSCHK( return );
	m_dmgDecalSys->SetSize( size );
	m_ovrDecalSys->SetSize( size );
}

void MultiEntity::DSClear()
{
	SCRENT_DSYSCHK( return );
	m_dmgDecalSys->ClearAllDecals();
	m_ovrDecalSys->ClearAllDecals();
}

void MultiEntity::RBCreateFromMesh( int i, int mi, SGRX_SIRigidBodyInfo* spec )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_OFSCHK( mi, return );
	SCRENT_MESHCHK( mi, return );
	SGRX_IMesh* M = m_meshes[ mi ]->GetMesh();
	if( M == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "mesh is not loaded" );
		return;
	}
	SGRX_PhyRigidBodyInfo rbi;
	if( spec )
		rbi = *spec;
	rbi.shape = m_level->GetPhyWorld()->CreateShapeFromMesh( M );
	m_bodies[ i ] = m_level->GetPhyWorld()->CreateRigidBody( rbi );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = rbi.position );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = rbi.rotation );
}

void MultiEntity::RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_SIRigidBodyInfo* spec )
{
	SCRENT_OFSCHK( i, return );
	ConvexPointSetHandle cpsh = GP_GetConvexPointSet( cpset );
	if( cpsh == NULL )
	{
		sgs_Msg( C, SGS_WARNING, "failed to load convex point set" );
		return;
	}
	SGRX_PhyRigidBodyInfo rbi;
	if( spec )
		rbi = *spec;
	rbi.shape = m_level->GetPhyWorld()->CreateConvexHullShape( cpsh->data.points.data(), cpsh->data.points.size() );
	m_bodies[ i ] = m_level->GetPhyWorld()->CreateRigidBody( rbi );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = rbi.position );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = rbi.rotation );
}

void MultiEntity::RBDestroy( int i )
{
	SCRENT_OFSCHK( i, return );
	m_bodies[ i ] = NULL;
}

bool MultiEntity::RBExists( int i )
{
	SCRENT_OFSCHK( i, return false );
	return m_bodies[ i ] != NULL;
}

void MultiEntity::RBSetEnabled( int i, bool enabled )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_BODYCHK( i, return );
	m_bodies[ i ]->SetEnabled( enabled );
}

Vec3 MultiEntity::RBGetPosition( int i )
{
	SCRENT_OFSCHK( i, return V3(0) );
	SCRENT_BODYCHK( i, return V3(0) );
	return m_bodyPosLerp[ i ]; // m_bodies[ i ]->GetPosition();
}

void MultiEntity::RBSetPosition( int i, Vec3 v )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_BODYCHK( i, return );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = v );
	m_bodies[ i ]->SetPosition( v );
}

Quat MultiEntity::RBGetRotation( int i )
{
	SCRENT_OFSCHK( i, return Quat::Identity );
	SCRENT_BODYCHK( i, return Quat::Identity );
	return m_bodyRotLerp[ i ]; // m_bodies[ i ]->GetRotation();
}

void MultiEntity::RBSetRotation( int i, Quat v )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_BODYCHK( i, return );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = v );
	m_bodies[ i ]->SetRotation( v );
}

Mat4 MultiEntity::RBGetMatrix( int i )
{
	SCRENT_OFSCHK( i, return Mat4::Identity );
	SCRENT_BODYCHK( i, return Mat4::Identity );
	return Mat4::CreateRotationFromQuat( m_bodyRotLerp[ i ] ) // m_bodies[ i ]->GetRotation() )
		* Mat4::CreateTranslation( m_bodyPosLerp[ i ] ); // m_bodies[ i ]->GetPosition() );
}

void MultiEntity::RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_BODYCHK( i, return );
	if( sgs_StackSize( C ) >= 4 )
		m_bodies[ i ]->ApplyForce( (EPhyForceType) type, v, p );
	else
		m_bodies[ i ]->ApplyCentralForce( (EPhyForceType) type, v );
}

void MultiEntity::JTCreateHingeB2W( int i, int bi, SGRX_SIHingeJointInfo* spec )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_OFSCHK( bi, return );
	SCRENT_BODYCHK( bi, return );
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateHingeJoint( hjinfo );
}

void MultiEntity::JTCreateHingeB2B( int i, int biA, int biB, SGRX_SIHingeJointInfo* spec )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_OFSCHK( biA, return );
	SCRENT_BODYCHK( biA, return );
	SCRENT_OFSCHK( biB, return );
	SCRENT_BODYCHK( biB, return );
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ biA ];
	hjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateHingeJoint( hjinfo );
}

void MultiEntity::JTCreateConeTwistB2W( int i, int bi, SGRX_SIConeTwistJointInfo* spec )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_OFSCHK( bi, return );
	SCRENT_BODYCHK( bi, return );
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateConeTwistJoint( ctjinfo );
}

void MultiEntity::JTCreateConeTwistB2B( int i, int biA, int biB, SGRX_SIConeTwistJointInfo* spec )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_OFSCHK( biA, return );
	SCRENT_BODYCHK( biA, return );
	SCRENT_OFSCHK( biB, return );
	SCRENT_BODYCHK( biB, return );
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ biA ];
	ctjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateConeTwistJoint( ctjinfo );
}

void MultiEntity::JTDestroy( int i )
{
	SCRENT_OFSCHK( i, return );
	m_joints[ i ] = NULL;
}

bool MultiEntity::JTExists( int i )
{
	SCRENT_OFSCHK( i, return false );
	return m_joints[ i ] != NULL;
}

void MultiEntity::JTSetEnabled( int i, bool enabled )
{
	SCRENT_OFSCHK( i, return );
	SCRENT_JOINTCHK( i, return );
	m_joints[ i ]->SetEnabled( enabled );
}



StockEntityCreationSystem::StockEntityCreationSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
//	ScrItem_InstallAPI( lev->GetSGSC() );
//	lev->GetScriptCtx().Include( "data/scritems" );
	lev->RegisterNativeEntity<Entity>( "Entity" );
	lev->RegisterNativeEntity<MultiEntity>( "MultiEntity" );
}

Entity* StockEntityCreationSystem::AddEntity( StringView type )
{
#if 0
	///////////////////////////
	if( type == "trigger" )
	{
		return new BoxTrigger
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>()
		);
	}
	
	///////////////////////////
	if( type == "trigger_prox" )
	{
		return new ProximityTrigger
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			data.getprop("distance").get<float>()
		);
	}
	
	///////////////////////////
	if( type == "door_slide" )
	{
		return new SlidingDoor
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("mesh").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("open_offset").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("open_rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			V3(0),
			Quat::Identity,
			data.getprop("open_time").get<float>(),
			false,
			data.getprop("is_switch").get<bool>()
		);
	}
	
	///////////////////////////
	if( type == "pickup" )
	{
		return new PickupItem
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("type").get<StringView>(),
			data.getprop("count").get<int>(),
			data.getprop("mesh").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>()
		);
	}
	
	///////////////////////////
	if( type == "actionable" )
	{
		return new Actionable
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("mesh").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("place_offset").get<Vec3>(),
			data.getprop("place_dir").get<Vec3>()
		);
	}
	
	///////////////////////////
	CoverSystem* coverSys = m_level->GetSystem<CoverSystem>();
	if( type == "cover" && coverSys )
	{
		if( !m_level->GetEditorMode() )
		{
			Mat4 mtx = Mat4::CreateSXT(
				data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
				Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ),
				data.getprop("position").get<Vec3>() );
			coverSys->AddAABB( data.getprop("name").get<StringView>(), V3(-1), V3(1), mtx );
		}
		return true;
	}
	
	///////////////////////////
	AIDBSystem* aidbSys = m_level->GetSystem<AIDBSystem>();
	if( type == "room" && aidbSys )
	{
		if( !m_level->GetEditorMode() )
		{
			Mat4 mtx = Mat4::CreateSXT(
				data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
				Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ),
				data.getprop("position").get<Vec3>() );
			aidbSys->AddRoomPart(
				data.getprop("name").get<StringView>(), mtx,
				data.getprop("negative").get<bool>(), data.getprop("cell_size").get<float>() );
		}
	}
	
	///////////////////////////
	if( type == "particle_fx" )
	{
		return new ParticleFX
		(
			m_level,
			data.getprop("name").get<StringView>(),
			data.getprop("partsys").get<StringView>(),
			data.getprop("soundevent").get<StringView>(),
			data.getprop("position").get<Vec3>(),
			Mat4::CreateRotationXYZ( DEG2RAD( data.getprop("rot_angles").get<Vec3>() ) ).GetRotationQuaternion(),
			data.getprop("scale_sep").get<Vec3>() * data.getprop("scale_uni").get<float>(),
			data.getprop("start").get<bool>()
		);
	}
	
	///////////////////////////
	if( type == "scritem" )
	{
		sgsVariable scritem = data.getprop("scritem");
		return new ScriptedItem
		(
			m_level,
			scritem.getprop("__type").get<StringView>(),
			scritem
		);
	}
#endif
	
	if( type == "Entity" ) return new Entity( m_level );
	if( type == "MultiEntity" ) return new MultiEntity( m_level );
	
	return NULL;
}




