

#include "entities.hpp"
#include "resources.hpp"


ParticleFX::ParticleFX( GameLevel* lev ) : Entity( lev ), m_soundEventOneShot(false)
{
}

void ParticleFX::OnTransformUpdate()
{
	m_psys.SetTransform( GetWorldMatrix() );
	if( m_soundEventInst )
	{
		SGRX_Sound3DAttribs s3dattr = { GetWorldPosition(), V3(0), V3(0), V3(0) };
		m_soundEventInst->Set3DAttribs( s3dattr );
	}
}

void ParticleFX::EditorDrawWorld()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	br.Col( 0.9f, 0.5f, 0.1f, 0.8f );
	br.AABB( V3(-1), V3(1), GetWorldMatrix() );
}

void ParticleFX::Tick( float deltaTime, float blendFactor )
{
	bool needstrig = m_psys.Tick( deltaTime );
	if( needstrig && m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			SGRX_Sound3DAttribs s3dattr = { GetWorldPosition(), V3(0), V3(0), V3(0) };
			sndevinst->Set3DAttribs( s3dattr );
			sndevinst->Start();
		}
	}
}

void ParticleFX::PreRender()
{
	m_psys.PreRender();
}

void ParticleFX::sgsSetParticleSystem( StringView path )
{
	if( path == m_partSysPath )
		return;
	
	m_partSysPath = path;
	// reload
	m_psys.Load( path );
	m_psys.AddToScene( m_level->GetScene() );
	m_psys.SetTransform( GetWorldMatrix() );
	m_psys.OnRenderUpdate();
	
	if( m_enabled )
	{
		m_psys.Play();
	}
}

void ParticleFX::sgsSetSoundEvent( StringView name )
{
	if( name == m_soundEventName )
		return;
	// destroy previous effect
	m_soundEventInst = NULL;
	// set new one
	m_soundEventName = name;
	m_soundEventOneShot = m_level->GetSoundSys()->EventIsOneShot( name );
	// if particle system already running, start it
	// do not start one-shot events here because ...
	// ... this is generally called during resource creation
	if( m_enabled && !m_soundEventOneShot )
	{
		_StartSoundEvent();
	}
}

void ParticleFX::_StartSoundEvent()
{
	SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
	if( sndevinst )
	{
		SGRX_Sound3DAttribs s3dattr = { GetWorldPosition(), V3(0), V3(0), V3(0) };
		sndevinst->Set3DAttribs( s3dattr );
		sndevinst->Start();
		if( !sndevinst->isOneShot )
			m_soundEventInst = sndevinst;
	}
}

void ParticleFX::sgsSetPlaying( bool playing )
{
	if( playing == m_enabled )
		return;
	
	m_enabled = playing;
	if( playing )
	{
		m_psys.Play();
		_StartSoundEvent();
	}
	else
	{
		m_psys.Stop();
		m_soundEventInst = NULL;
	}
}

void ParticleFX::Trigger()
{
	m_psys.Trigger();
	if( m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			SGRX_Sound3DAttribs s3dattr = { GetWorldPosition(), V3(0), V3(0), V3(0) };
			sndevinst->Set3DAttribs( s3dattr );
			sndevinst->Start();
		}
	}
}


MeshEntity::MeshEntity( GameLevel* lev ) : Entity( lev ),
	m_isStatic( true ),
	m_isVisible( true ),
	m_isSolid( true ),
	m_lightingMode( SGRX_LM_Static ),
	m_lmQuality( 1 ),
	m_castLMS( true )
{
	m_meshInst = m_level->GetScene()->CreateMeshInstance();
	_UpdateBody();
}

MeshEntity::~MeshEntity()
{
}

void MeshEntity::OnTransformUpdate()
{
	m_meshInst->SetTransform( GetWorldMatrix() );
	_UpdateLighting();
	if( m_body )
	{
		m_body->SetPosition( GetWorldPosition() );
		m_body->SetRotation( GetWorldRotation() );
		m_body->GetShape()->SetScale( GetWorldScale() );
	}
	_UpEv();
}

void MeshEntity::EditorDrawWorld()
{
	if( m_mesh )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0.2f, 0.7f, 0.9f, 0.8f );
		br.AABB( m_mesh->m_boundsMin, m_mesh->m_boundsMax, GetWorldMatrix() );
	}
}

void MeshEntity::_UpdateLighting()
{
	if( m_lightingMode == SGRX_LM_Dynamic )
	{
		m_level->LightMesh( m_meshInst );
	}
	_UpEv();
}

void MeshEntity::_UpdateBody()
{
	bool need = m_isSolid && m_mesh;
	if( !need && m_body )
	{
		m_body = NULL;
		m_phyShape = NULL;
	}
	else if( need && !m_body )
	{
		if( !m_phyShape )
		{
			m_phyShape = m_level->GetPhyWorld()->CreateShapeFromMesh( m_mesh );
			m_phyShape->SetScale( GetWorldScale() );
		}
		
		SGRX_PhyRigidBodyInfo rbi;
		rbi.shape = m_phyShape;
		rbi.position = GetWorldPosition();
		rbi.rotation = GetWorldRotation();
		rbi.kinematic = !m_isStatic;
		m_body = m_level->GetPhyWorld()->CreateRigidBody( rbi );
	}
	_UpEv();
}

void MeshEntity::SetMeshData( MeshHandle mesh )
{
	if( m_mesh == mesh )
		return;
	m_mesh = mesh;
	m_phyShape = NULL;
	m_body = NULL;
	m_meshInst->SetMesh( mesh );
	_UpdateBody();
	// _UpEv already called
}

void MeshEntity::SetShaderConst( int v, Vec4 var )
{
	if( v < 0 || v >= MAX_MI_CONSTANTS )
	{
		sgs_Msg( C, SGS_WARNING, "shader constant %d outside range [0;%d)", v, MAX_MI_CONSTANTS );
		return;
	}
	m_meshInst->constants[ v ] = var;
}


LightEntity::LightEntity( GameLevel* lev ) : Entity( lev ),
	m_isStatic( false ),
	m_type( LIGHT_POINT ),
	m_isEnabled( true ),
	m_color( V3(1) ),
	m_intensity( 1 ),
	m_range( 1 ),
	m_power( 2 ),
	m_angle( 65 ),
	m_aspect( 1 ),
	m_hasShadows( true ),
	m_flareSize( 0 ),
	m_flareOffset( V3(0) ),
	m_innerAngle( 0 ),
	m_spotCurve( 1 ),
	m_lightRadius( 0.1f )
{
	_UpdateLight();
	_UpdateShadows();
	_UpdateFlare();
}

LightEntity::~LightEntity()
{
	FlareSystem* FS = m_level->GetSystem<FlareSystem>();
	if( FS )
		FS->RemoveFlare( this );
}

void LightEntity::_UpdateLight()
{
	bool need = !m_isStatic;
	if( !need && m_light )
		m_light = NULL;
	else if( need && !m_light )
	{
		m_light = m_level->GetScene()->CreateLight();
		m_light->type = m_type;
		m_light->position = V3(0);
		m_light->direction = V3(0,0,-1);
		m_light->updir = V3(0,-1,0);
		m_light->color = m_color * m_intensity;
		m_light->range = m_range;
		m_light->power = m_power;
		m_light->angle = m_angle;
		m_light->aspect = m_aspect;
		m_light->hasShadows = m_hasShadows;
		m_light->SetTransform( GetWorldMatrix() );
		m_light->UpdateTransform();
		_UpdateShadows();
	}
}

void LightEntity::_UpdateShadows()
{
	bool need = m_hasShadows && m_type == LIGHT_SPOT;
	if( !need && m_light->shadowTexture )
		m_light->shadowTexture = NULL;
	else if( need && !m_light->shadowTexture )
		m_light->shadowTexture = GR_CreateRenderTexture( 512, 512, RT_FORMAT_DEPTH );
}

void LightEntity::_UpdateFlare()
{
	FlareSystem* FS = m_level->GetSystem<FlareSystem>();
	if( FS )
	{
		bool need = m_flareSize > 0;
		if( !need )
			FS->RemoveFlare( this );
		else
		{
			FSFlare F =
			{
				LocalToWorld( m_flareOffset ),
				m_color * m_intensity,
				m_flareSize,
				m_isEnabled
			};
			FS->UpdateFlare( this, F );
		}
	}
}


RigidBodyEntity::RigidBodyEntity( GameLevel* lev ) :
	Entity( lev ),
	shapeType( ShapeType_AABB ),
	shapeRadius( 0.5f ),
	shapeHeight( 1.0f ),
	shapeExtents( V3(0.5f) ),
	shapeMinExtents( V3(-0.5f) )
{
	_UpdateShape();
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.shape = m_shape;
	m_body = m_level->GetPhyWorld()->CreateRigidBody( rbinfo );
}

void RigidBodyEntity::FixedTick( float deltaTime )
{
	Entity::FixedTick( deltaTime );
}

void RigidBodyEntity::Tick( float deltaTime, float blendFactor )
{
	Entity::Tick( deltaTime, blendFactor );
}

void RigidBodyEntity::_UpdateShape()
{
	PhyWorldHandle pw = m_level->GetPhyWorld();
	if( shapeType == ShapeType_Box )
		m_shape = pw->CreateBoxShape( shapeExtents );
	else if( shapeType == ShapeType_Sphere )
		m_shape = pw->CreateSphereShape( shapeRadius );
	else if( shapeType == ShapeType_Cylinder )
		m_shape = pw->CreateCylinderShape( shapeExtents );
	else if( shapeType == ShapeType_Capsule )
		m_shape = pw->CreateCapsuleShape( shapeRadius, shapeHeight );
	else if( shapeType == ShapeType_Mesh )
		m_shape = pw->CreateShapeFromMesh( shapeMesh );
	else // AABB / other
		m_shape = pw->CreateAABBShape( shapeMinExtents, shapeExtents );
	
	if( m_body )
		m_body->SetShape( m_shape );
}


ReflectionPlaneEntity::ReflectionPlaneEntity( GameLevel* lev ) : Entity( lev )
{
}




static int RigidBodyInfo( SGS_CTX )
{
	SGS_CREATECLASS( C, NULL, SGRX_RigidBodyInfo, () );
	return 1;
}

static int HingeJointInfo( SGS_CTX )
{
	SGS_CREATECLASS( C, NULL, SGRX_HingeJointInfo, () );
	return 1;
}

static int ConeTwistJointInfo( SGS_CTX )
{
	SGS_CREATECLASS( C, NULL, SGRX_ConeTwistJointInfo, () );
	return 1;
}

static sgs_RegIntConst multient_intconsts[] =
{
	{ "ForceType_Velocity", ForceType_Velocity },
	{ "ForceType_Impulse", ForceType_Impulse },
	{ "ForceType_Acceleration", ForceType_Acceleration },
	{ "ForceType_Force", ForceType_Force },
	{ NULL, 0 },
};

static sgs_RegFuncConst multient_funcconsts[] =
{
	{ "RigidBodyInfo", RigidBodyInfo },
	{ "HingeJointInfo", HingeJointInfo },
	{ "ConeTwistJointInfo", ConeTwistJointInfo },
	{ NULL, NULL },
};

void MultiEnt_InstallAPI( SGS_CTX )
{
	sgs_RegIntConsts( C, multient_intconsts, -1 );
	sgs_RegFuncConsts( C, multient_funcconsts, -1 );
}


#define MULTIENT_OFSCHK( i, ret ) if( (i) < 0 || (i) >= 4 ){ \
	sgs_Msg( C, SGS_WARNING, "wrong offset: %d outside " MULTIENT_RANGE_STR, (int)(i) ); ret; }
#define MULTIENT_MESHCHK( i, ret ) if( m_meshes[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no mesh at offset %d", (int)(i) ); ret; }
#define MULTIENT_PSYSCHK( i, ret ) if( m_partSys[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no part.sys at offset %d", (int)(i) ); ret; }
#define MULTIENT_BODYCHK( i, ret ) if( m_bodies[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no body at offset %d", (int)(i) ); ret; }
#define MULTIENT_JOINTCHK( i, ret ) if( m_joints[ i ] == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no joint at offset %d", (int)(i) ); ret; }
#define MULTIENT_DSYSCHK( ret ) if( m_dmgDecalSys == NULL ){ \
	sgs_Msg( C, SGS_WARNING, "no decal sys" ); ret; }


MultiEntity::MultiEntity( GameLevel* lev ) : Entity( lev ),
	mi0sampleOffset( V3(0) )
{
	for( int i = 0; i < MULTIENT_NUM_SLOTS; ++i )
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
	for( int i = 0; i < MULTIENT_NUM_SLOTS; ++i )
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
	for( int i = 0; i < MULTIENT_NUM_SLOTS; ++i )
	{
		if( m_bodies[ i ] )
		{
			m_bodyPosLerp[ i ] = m_bodyPos[ i ].Get( blendFactor );
			m_bodyRotLerp[ i ] = m_bodyRot[ i ].Get( blendFactor );
		}
	}
	
	for( int i = 0; i < MULTIENT_NUM_SLOTS; ++i )
	{
		if( m_partSys[ i ] )
			m_partSys[ i ]->Tick( deltaTime );
	}
	
	Entity::Tick( deltaTime, blendFactor );
	
	PreRender();
}

void MultiEntity::PreRender()
{
	for( int i = 0; i < MULTIENT_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_level->LightMesh( m_meshes[ i ], i == 0 ? mi0sampleOffset : V3(0) );
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
		for( ; i < MULTIENT_NUM_SLOTS; ++i )
		{
			if( MI == m_meshes[ i ] )
			{
				sgs_PushInt( C, i );
				break;
			}
		}
		if( i == MULTIENT_NUM_SLOTS )
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
	for( int i = 0; i < MULTIENT_NUM_SLOTS; ++i )
	{
		if( m_meshes[ i ] )
			m_meshes[ i ]->matrix = m_meshMatrices[ i ] * mtx;
		if( m_partSys[ i ] )
			m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * mtx );
	}
}

void MultiEntity::MICreate( int i, StringView path )
{
	MULTIENT_OFSCHK( i, return );
	m_meshes[ i ] = m_level->GetScene()->CreateMeshInstance();
	m_meshes[ i ]->matrix = m_meshMatrices[ i ] * GetWorldMatrix();
	m_meshes[ i ]->userData = (SGRX_MeshInstUserData*) this;
	if( path )
		m_meshes[ i ]->SetMesh( path );
}

void MultiEntity::MIDestroy( int i )
{
	MULTIENT_OFSCHK( i, return );
	m_meshes[ i ] = NULL;
}

bool MultiEntity::MIExists( int i )
{
	MULTIENT_OFSCHK( i, return false );
	return m_meshes[ i ] != NULL;
}

void MultiEntity::MISetMesh( int i, StringView path )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_MESHCHK( i, return );
	m_meshes[ i ]->SetMesh( path );
}

void MultiEntity::MISetEnabled( int i, bool enabled )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_MESHCHK( i, return );
	m_meshes[ i ]->enabled = enabled;
}

void MultiEntity::MISetMatrix( int i, Mat4 mtx )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_MESHCHK( i, return );
	m_meshMatrices[ i ] = mtx;
	m_meshes[ i ]->matrix = mtx * GetWorldMatrix();
}

void MultiEntity::MISetShaderConst( int i, int v, Vec4 var )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_MESHCHK( i, return );
	if( v < 0 || v >= MAX_MI_CONSTANTS )
	{
		sgs_Msg( C, SGS_WARNING, "shader constant %d outside range [0;%d)", v, MAX_MI_CONSTANTS );
		return;
	}
	m_meshes[ i ]->constants[ v ] = var;
}

void MultiEntity::MISetLayers( int i, uint32_t layers )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_MESHCHK( i, return );
	m_meshes[ i ]->layers = layers;
}

MeshHandle MultiEntity::sgsGetMI0Mesh()
{
	MULTIENT_OFSCHK( 0, return NULL );
	MULTIENT_MESHCHK( 0, return NULL );
	return m_meshes[ 0 ]->GetMesh();
}

void MultiEntity::sgsSetMI0Mesh( MeshHandle m )
{
	MULTIENT_OFSCHK( 0, return );
	MULTIENT_MESHCHK( 0, return );
	m_meshes[ 0 ]->SetMesh( m );
}

void MultiEntity::PSCreate( int i, StringView path )
{
	MULTIENT_OFSCHK( i, return );
	m_partSys[ i ] = new ParticleSystem;
	m_partSys[ i ]->AddToScene( m_level->GetScene() );
	if( path )
		m_partSys[ i ]->Load( path );
	m_partSys[ i ]->SetTransform( m_partSysMatrices[ i ] * GetWorldMatrix() );
}

void MultiEntity::PSDestroy( int i )
{
	MULTIENT_OFSCHK( i, return );
	m_partSys[ i ] = NULL;
}

bool MultiEntity::PSExists( int i )
{
	MULTIENT_OFSCHK( i, return false );
	return m_partSys[ i ] != NULL;
}

void MultiEntity::PSLoad( int i, StringView path )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_PSYSCHK( i, return );
	m_partSys[ i ]->Load( path );
}

void MultiEntity::PSSetMatrix( int i, Mat4 mtx )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_PSYSCHK( i, return );
	m_partSysMatrices[ i ] = mtx;
	m_partSys[ i ]->SetTransform( mtx * GetWorldMatrix() );
}

void MultiEntity::PSSetMatrixFromMeshAABB( int i, int mi )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_PSYSCHK( i, return );
	MULTIENT_OFSCHK( mi, return );
	MULTIENT_MESHCHK( mi, return );
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
	MULTIENT_OFSCHK( i, return );
	MULTIENT_PSYSCHK( i, return );
	m_partSys[ i ]->Play();
}

void MultiEntity::PSStop( int i )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_PSYSCHK( i, return );
	m_partSys[ i ]->Stop();
}

void MultiEntity::PSTrigger( int i )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_PSYSCHK( i, return );
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
	MULTIENT_DSYSCHK( return );
	m_dmgDecalSys->SetSize( size );
	m_ovrDecalSys->SetSize( size );
}

void MultiEntity::DSClear()
{
	MULTIENT_DSYSCHK( return );
	m_dmgDecalSys->ClearAllDecals();
	m_ovrDecalSys->ClearAllDecals();
}

void MultiEntity::RBCreateFromMesh( int i, int mi, SGRX_RigidBodyInfo* spec )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_OFSCHK( mi, return );
	MULTIENT_MESHCHK( mi, return );
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

void MultiEntity::RBCreateFromConvexPointSet( int i, StringView cpset, SGRX_RigidBodyInfo* spec )
{
	MULTIENT_OFSCHK( i, return );
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
	MULTIENT_OFSCHK( i, return );
	m_bodies[ i ] = NULL;
}

bool MultiEntity::RBExists( int i )
{
	MULTIENT_OFSCHK( i, return false );
	return m_bodies[ i ] != NULL;
}

void MultiEntity::RBSetEnabled( int i, bool enabled )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_BODYCHK( i, return );
	m_bodies[ i ]->SetEnabled( enabled );
}

Vec3 MultiEntity::RBGetPosition( int i )
{
	MULTIENT_OFSCHK( i, return V3(0) );
	MULTIENT_BODYCHK( i, return V3(0) );
	return m_bodyPosLerp[ i ]; // m_bodies[ i ]->GetPosition();
}

void MultiEntity::RBSetPosition( int i, Vec3 v )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_BODYCHK( i, return );
	m_bodyPos[ i ] = IVState<Vec3>( m_bodyPosLerp[ i ] = v );
	m_bodies[ i ]->SetPosition( v );
}

Quat MultiEntity::RBGetRotation( int i )
{
	MULTIENT_OFSCHK( i, return Quat::Identity );
	MULTIENT_BODYCHK( i, return Quat::Identity );
	return m_bodyRotLerp[ i ]; // m_bodies[ i ]->GetRotation();
}

void MultiEntity::RBSetRotation( int i, Quat v )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_BODYCHK( i, return );
	m_bodyRot[ i ] = IVState<Quat>( m_bodyRotLerp[ i ] = v );
	m_bodies[ i ]->SetRotation( v );
}

Mat4 MultiEntity::RBGetMatrix( int i )
{
	MULTIENT_OFSCHK( i, return Mat4::Identity );
	MULTIENT_BODYCHK( i, return Mat4::Identity );
	return Mat4::CreateRotationFromQuat( m_bodyRotLerp[ i ] ) // m_bodies[ i ]->GetRotation() )
		* Mat4::CreateTranslation( m_bodyPosLerp[ i ] ); // m_bodies[ i ]->GetPosition() );
}

void MultiEntity::RBApplyForce( int i, int type, Vec3 v, /*opt*/ Vec3 p )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_BODYCHK( i, return );
	if( sgs_StackSize( C ) >= 4 )
		m_bodies[ i ]->ApplyForce( (EPhyForceType) type, v, p );
	else
		m_bodies[ i ]->ApplyCentralForce( (EPhyForceType) type, v );
}

void MultiEntity::JTCreateHingeB2W( int i, int bi, SGRX_HingeJointInfo* spec )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_OFSCHK( bi, return );
	MULTIENT_BODYCHK( bi, return );
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateHingeJoint( hjinfo );
}

void MultiEntity::JTCreateHingeB2B( int i, int biA, int biB, SGRX_HingeJointInfo* spec )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_OFSCHK( biA, return );
	MULTIENT_BODYCHK( biA, return );
	MULTIENT_OFSCHK( biB, return );
	MULTIENT_BODYCHK( biB, return );
	SGRX_PhyHingeJointInfo hjinfo = *spec;
	hjinfo.bodyA = m_bodies[ biA ];
	hjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateHingeJoint( hjinfo );
}

void MultiEntity::JTCreateConeTwistB2W( int i, int bi, SGRX_ConeTwistJointInfo* spec )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_OFSCHK( bi, return );
	MULTIENT_BODYCHK( bi, return );
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ bi ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateConeTwistJoint( ctjinfo );
}

void MultiEntity::JTCreateConeTwistB2B( int i, int biA, int biB, SGRX_ConeTwistJointInfo* spec )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_OFSCHK( biA, return );
	MULTIENT_BODYCHK( biA, return );
	MULTIENT_OFSCHK( biB, return );
	MULTIENT_BODYCHK( biB, return );
	SGRX_PhyConeTwistJointInfo ctjinfo = *spec;
	ctjinfo.bodyA = m_bodies[ biA ];
	ctjinfo.bodyB = m_bodies[ biB ];
	m_joints[ i ] = m_level->GetPhyWorld()->CreateConeTwistJoint( ctjinfo );
}

void MultiEntity::JTDestroy( int i )
{
	MULTIENT_OFSCHK( i, return );
	m_joints[ i ] = NULL;
}

bool MultiEntity::JTExists( int i )
{
	MULTIENT_OFSCHK( i, return false );
	return m_joints[ i ] != NULL;
}

void MultiEntity::JTSetEnabled( int i, bool enabled )
{
	MULTIENT_OFSCHK( i, return );
	MULTIENT_JOINTCHK( i, return );
	m_joints[ i ]->SetEnabled( enabled );
}



StockEntityCreationSystem::StockEntityCreationSystem( GameLevel* lev ) : IGameLevelSystem( lev, e_system_uid )
{
	MultiEnt_InstallAPI( lev->GetSGSC() );
	sgs_RegIntConst ric[] =
	{
		{ "ShapeType_AABB", ShapeType_AABB },
		{ "ShapeType_Box", ShapeType_Box },
		{ "ShapeType_Sphere", ShapeType_Sphere },
		{ "ShapeType_Cylinder", ShapeType_Cylinder },
		{ "ShapeType_Capsule", ShapeType_Capsule },
		{ "ShapeType_Mesh", ShapeType_Mesh },
		{ NULL, 0 },
	};
	sgs_RegIntConsts( lev->GetSGSC(), ric, -1 );
	lev->RegisterNativeEntity<Entity>( "Entity" );
	lev->RegisterNativeEntity<ParticleFX>( "ParticleFX" );
	lev->RegisterNativeEntity<MeshEntity>( "Mesh" );
	lev->RegisterNativeEntity<LightEntity>( "Light" );
	lev->RegisterNativeEntity<RigidBodyEntity>( "RigidBody" );
	lev->RegisterNativeEntity<ReflectionPlaneEntity>( "ReflectionPlane" );
	lev->RegisterNativeEntity<MultiEntity>( "MultiEntity" );
	
	lev->RegisterNativeClass<MeshResource>( "MeshResource" );
	lev->RegisterNativeClass<LightResource>( "LightResource" );
}

Entity* StockEntityCreationSystem::AddEntity( StringView type )
{
	if( type == "Entity" ) return new Entity( m_level );
	if( type == "ParticleFX" ) return new ParticleFX( m_level );
	if( type == "Mesh" ) return new MeshEntity( m_level );
	if( type == "Light" ) return new LightEntity( m_level );
	if( type == "RigidBody" ) return new RigidBodyEntity( m_level );
	if( type == "ReflectionPlane" ) return new ReflectionPlaneEntity( m_level );
	if( type == "MultiEntity" ) return new MultiEntity( m_level );
	
	return NULL;
}




