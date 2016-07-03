

#include "resources.hpp"

#include "imgui.hpp"


MeshResource::MeshResource( GameObject* obj ) : GOResource( obj ),
	m_isStatic( true ),
	m_isVisible( true ),
	m_lightingMode( SGRX_LM_Dynamic ),
	m_lmQuality( 1 ),
	m_castLMS( true )
{
	m_meshInst = m_level->GetScene()->CreateMeshInstance();
	m_meshInst->SetLightingMode( (SGRX_LightingMode) m_lightingMode );
	_UpdateMatrix();
}

MeshResource::~MeshResource()
{
}

void MeshResource::OnTransformUpdate()
{
	_UpdateMatrix();
}

void MeshResource::_UpdateMatrix()
{
	m_meshInst->SetTransform( GetWorldMatrix() );
	_UpdateLighting();
}

void MeshResource::EditorDrawWorld()
{
	if( m_mesh )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Reset();
		br.Col( 0.2f, 0.7f, 0.9f, 0.8f );
		br.AABB( m_mesh->m_boundsMin, m_mesh->m_boundsMax, m_meshInst->matrix );
	}
}

void MeshResource::_UpdateLighting()
{
	if( m_lightingMode == SGRX_LM_Dynamic )
	{
		m_level->LightMesh( m_meshInst );
	}
	_UpEv();
}

void MeshResource::SetMeshData( MeshHandle mesh )
{
	if( m_mesh == mesh )
		return;
	m_mesh = mesh;
	m_meshInst->SetMesh( mesh );
	// _UpEv already called
}

void MeshResource::SetShaderConst( int v, Vec4 var )
{
	if( v < 0 || v >= MAX_MI_CONSTANTS )
	{
		sgs_Msg( C, SGS_WARNING, "shader constant %d outside range [0;%d)", v, MAX_MI_CONSTANTS );
		return;
	}
	m_meshInst->constants[ v ] = var;
}


LightResource::LightResource( GameObject* obj ) : GOResource( obj ),
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
	m_innerAngle( 0 ),
	m_spotCurve( 1 ),
	m_lightRadius( 0.1f )
{
	_UpdateLight();
	_UpdateShadows();
}

LightResource::~LightResource()
{
}

void LightResource::OnTransformUpdate()
{
	_UpdateMatrix();
}

void LightResource::_UpdateMatrix()
{
	if( m_light )
	{
		m_light->SetTransform( GetWorldMatrix() );
		m_light->UpdateTransform();
	}
	_UpEv();
}

void LightResource::_UpdateLight()
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
		_UpdateMatrix();
		_UpdateShadows();
	}
}

void LightResource::_UpdateShadows()
{
	bool need = m_hasShadows && m_type == LIGHT_SPOT;
	if( !need && m_light->shadowTexture )
		m_light->shadowTexture = NULL;
	else if( need && !m_light->shadowTexture )
		m_light->shadowTexture = GR_CreateRenderTexture( 512, 512, RT_FORMAT_DEPTH );
}


ParticleSystemResource::ParticleSystemResource( GameObject* obj ) :
	GOResource( obj ),
	m_enabled( false ),
	m_soundEventOneShot( false )
{
}

void ParticleSystemResource::OnTransformUpdate()
{
	Mat4 mtx = GetWorldMatrix();
	m_psys.SetTransform( mtx );
	if( m_soundEventInst )
	{
		m_soundEventInst->Set3DAttribs( _Get3DAttribs() );
	}
}

void ParticleSystemResource::EditorDrawWorld()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	br.Col( 0.9f, 0.5f, 0.1f, 0.8f );
	br.AABB( V3(-1), V3(1), GetWorldMatrix() );
}

void ParticleSystemResource::Update()
{
	bool needstrig = m_psys.Tick( m_level->GetDeltaTime() );
	if( needstrig && m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			sndevinst->Set3DAttribs( _Get3DAttribs() );
			sndevinst->Start();
		}
	}
}

void ParticleSystemResource::sgsSetParticleSystem( StringView path )
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

void ParticleSystemResource::sgsSetSoundEvent( StringView name )
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

void ParticleSystemResource::_StartSoundEvent()
{
	SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
	if( sndevinst )
	{
		sndevinst->Set3DAttribs( _Get3DAttribs() );
		sndevinst->Start();
		if( !sndevinst->isOneShot )
			m_soundEventInst = sndevinst;
	}
}

void ParticleSystemResource::sgsSetPlaying( bool playing )
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

void ParticleSystemResource::Trigger()
{
	m_psys.Trigger();
	if( m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			sndevinst->Set3DAttribs( _Get3DAttribs() );
			sndevinst->Start();
		}
	}
}

SGRX_Sound3DAttribs ParticleSystemResource::_Get3DAttribs()
{
	SGRX_Sound3DAttribs s3dattr = { GetWorldMatrix().GetTranslation(), V3(0), V3(0), V3(0) };
	return s3dattr;
}


SoundSourceResource::SoundSourceResource( GameObject* obj ) :
	GOResource( obj ),
	m_enabled( false ),
	m_soundEventOneShot( false )
{
}

void SoundSourceResource::OnTransformUpdate()
{
	if( m_soundEventInst )
	{
		m_soundEventInst->Set3DAttribs( _Get3DAttribs() );
	}
}

void SoundSourceResource::EditorDrawWorld()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset();
	br.Col( 0.5f, 0.5f, 0.1f, 0.8f );
	br.AABB( V3(-1), V3(1), GetWorldMatrix() );
}

void SoundSourceResource::sgsSetSoundEvent( StringView name )
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

void SoundSourceResource::_StartSoundEvent()
{
	SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
	if( sndevinst )
	{
		sndevinst->Set3DAttribs( _Get3DAttribs() );
		sndevinst->Start();
		if( !sndevinst->isOneShot )
			m_soundEventInst = sndevinst;
	}
}

void SoundSourceResource::sgsSetPlaying( bool playing )
{
	if( playing == m_enabled )
		return;
	
	m_enabled = playing;
	if( playing )
	{
		_StartSoundEvent();
	}
	else
	{
		m_soundEventInst = NULL;
	}
}

void SoundSourceResource::Trigger()
{
	if( m_soundEventOneShot )
	{
		SoundEventInstanceHandle sndevinst = m_level->GetSoundSys()->CreateEventInstance( m_soundEventName );
		if( sndevinst )
		{
			sndevinst->Set3DAttribs( _Get3DAttribs() );
			sndevinst->Start();
		}
	}
}

SGRX_Sound3DAttribs SoundSourceResource::_Get3DAttribs()
{
	SGRX_Sound3DAttribs s3dattr = { GetWorldMatrix().GetTranslation(), V3(0), V3(0), V3(0) };
	return s3dattr;
}


RigidBodyResource::RigidBodyResource( GameObject* obj ) :
	GOResource( obj ),
	shapeType( ShapeType_AABB ),
	shapeRadius( 0.5f ),
	shapeHeight( 1.0f ),
	shapeExtents( V3(0.5f) ),
	shapeMinExtents( V3(-0.5f) )
{
	Mat4 xf = GOResource::GetWorldMatrix();
	_UpdateShape();
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.shape = m_shape;
	rbinfo.position = m_prevPos = xf.GetTranslation();
	rbinfo.rotation = m_prevRot = xf.GetRotationQuaternion();
	m_body = m_level->GetPhyWorld()->CreateRigidBody( rbinfo );
}

void RigidBodyResource::OnTransformUpdate()
{
	Mat4 xf = GOResource::GetWorldMatrix();
	Vec3 pos = xf.GetTranslation();
	Quat rot = xf.GetRotationQuaternion();
	if( m_level->GetEventType() == LEV_FixedUpdate )
	{
		m_prevPos = pos;
		m_prevRot = rot;
	}
	m_body->SetPosition( pos );
	m_body->SetRotation( rot );
	m_shape->SetScale( xf.GetScale() );
}

Mat4 RigidBodyResource::GetWorldMatrix() const
{
	Vec3 currPos = m_body->GetPosition();
	Quat currRot = m_body->GetRotation();
	if( m_level->GetEventType() == LEV_Update )
	{
		currPos = TLERP( m_prevPos, currPos, m_level->GetBlendFactor() );
		currRot = TLERP( m_prevRot, currRot, m_level->GetBlendFactor() );
	}
	return Mat4::CreateSRT( m_shape->GetScale(), currRot, currPos );
}

void RigidBodyResource::PrePhysicsFixedUpdate()
{
	m_prevPos = m_body->GetPosition();
	m_prevRot = m_body->GetRotation();
}

void RigidBodyResource::_UpdateShape()
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


ReflectionPlaneResource::ReflectionPlaneResource( GameObject* obj ) :
	GOResource( obj )
{
}


CameraResource::CameraResource( GameObject* obj ) :
	GOResource( obj ),
	depth( 1000 ),
	fieldOfView( 90 ),
	aspectMix( 0.5f ),
	nearPlane( 0.1f ),
	farPlane( 10000 ),
	enabled( true )
{
}

void CameraResource::GetCamera( SGRX_Camera& out )
{
	Mat4 mtx = GetWorldMatrix();
	
	out.position = mtx.GetTranslation();
	out.direction = mtx.TransformNormal( V3(0,0,1) ).Normalized();
	out.updir = mtx.TransformNormal( V3(0,1,0) ).Normalized();
	
	out.angle = fieldOfView;
	out.aspect = safe_fdiv( GR_GetWidth(), GR_GetHeight() );
	out.aamix = aspectMix;
	out.znear = nearPlane;
	out.zfar = farPlane;
	
	out.UpdateMatrices();
}

SGS_MULTRET CameraResource::sgsWorldToScreen( Vec3 pos )
{
	SGRX_Camera cam;
	GetCamera( cam );
	bool infront = false;
	sgs_PushVar( C, cam.WorldToScreen( pos, &infront ) );
	sgs_PushVar( C, infront );
	return 2;
}

SGS_MULTRET CameraResource::sgsWorldToScreenPx( Vec3 pos )
{
	SGRX_Camera cam;
	GetCamera( cam );
	bool infront = false;
	sgs_PushVar( C, cam.WorldToScreen( pos, &infront )
		* V3( GR_GetWidth(), GR_GetHeight(), 1 ) );
	sgs_PushVar( C, infront );
	return 2;
}

bool CameraResource::GetCursorWorldPoint( Vec3* isp, uint32_t layers, Vec2 cpn )
{
	SGRX_Camera cam;
	GetCamera( cam );
	Vec3 pos, dir, end;
	if( !cam.GetCursorRay( cpn.x, cpn.y, pos, dir ) )
		return false;
	SceneRaycastInfo hitinfo;
	end = pos + dir * cam.zfar;
	if( !m_level->GetScene()->RaycastOne( pos, end, &hitinfo, layers ) )
		return false;
	*isp = TLERP( pos, end, hitinfo.factor );
	return true;
}

SGS_MULTRET CameraResource::sgsGetCursorWorldPoint( uint32_t layers )
{
	Vec3 p;
	if( !GetCursorWorldPoint( &p, sgs_StackSize( C ) >= 1 ? layers : 0xffffffff ) )
		return 0;
	sgs_PushVar( C, p );
	return 1;
}

SGS_MULTRET CameraResource::sgsGetCursorMeshInst( uint32_t layers /* = 0xffffffff */ )
{
	SGRX_Camera cam;
	GetCamera( cam );
	Vec2 cpn = Game_GetCursorPosNormalized();
	Vec3 pos, dir, end;
	if( !cam.GetCursorRay( cpn.x, cpn.y, pos, dir ) )
		return 0;
	SceneRaycastInfo hitinfo;
	end = pos + dir * cam.zfar;
	if( !m_level->GetScene()->RaycastOne( pos, end, &hitinfo, sgs_StackSize( C ) >= 1 ? layers : 0xffffffff ) )
		return 0;
	sgs_PushPtr( C, hitinfo.meshinst.item );
	return 1;
}



BhResourceMoveObject::BhResourceMoveObject( GameObject* obj ) :
	GOBehavior( obj ), mask( MoveMask_Position | MoveMask_Rotation )
{
}

void BhResourceMoveObject::FixedUpdate()
{
	GOResource* rsrc = resource;
	if( rsrc && mask )
	{
		TempSwapper<const void*> ts( m_obj->_xfChangeInvoker, rsrc );
		if( mask == MoveMask_ALL )
		{
			m_obj->SetWorldMatrix( rsrc->GetWorldMatrix() );
		}
		else
		{
			Mat4 A = m_obj->GetWorldMatrix();
			Mat4 B = rsrc->GetWorldMatrix();
			Vec3 pos = ( mask & MoveMask_Position ? B : A ).GetTranslation();
			Quat rot = ( mask & MoveMask_Rotation ? B : A ).GetRotationQuaternion();
			Vec3 scl = ( mask & MoveMask_Scale ? B : A ).GetScale();
			m_obj->SetWorldMatrix( Mat4::CreateSRT( scl, rot, pos ) );
		}
	}
}

void BhResourceMoveObject::Update()
{
	FixedUpdate();
}


BhResourceMoveResource::BhResourceMoveResource( GameObject* obj ) :
	GOBehavior( obj ), mask( MoveMask_Position | MoveMask_Rotation )
{
}

void BhResourceMoveResource::FixedUpdate()
{
	GOResource* rsrc = resource;
	GOResource* tgt = follow;
	if( rsrc && tgt && mask )
	{
		rsrc->SetMatrixMode( MM_Absolute );
		if( mask == MoveMask_ALL )
		{
			rsrc->SetLocalMatrix( tgt->GetWorldMatrix() );
		}
		else
		{
			Mat4 A = rsrc->GetWorldMatrix();
			Mat4 B = tgt->GetWorldMatrix();
			Vec3 pos = ( mask & MoveMask_Position ? B : A ).GetTranslation();
			Quat rot = ( mask & MoveMask_Rotation ? B : A ).GetRotationQuaternion();
			Vec3 scl = ( mask & MoveMask_Scale ? B : A ).GetScale();
			rsrc->SetLocalMatrix( Mat4::CreateSRT( scl, rot, pos ) );
		}
	}
}

void BhResourceMoveResource::Update()
{
	FixedUpdate();
}


BhControllerBase::BhControllerBase( GameObject* obj ) :
	GOBehavior( obj ), enabled( true )
{
	_data = m_level->GetScriptCtx().CreateDict();
}

Vec3 BhControllerBase::GetInput( uint32_t iid )
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

void BhControllerBase::Reset()
{
	sgsVariable fn_reset = GetScriptedObject().getprop( "Reset" );
	if( fn_reset.not_null() )
		GetScriptedObject().thiscall( C, fn_reset );
}


static IController g_DummyController;
IController* GetObjectController( GameObject* obj, bool def )
{
	for( size_t i = 0; i < obj->m_bhvr_order.size(); ++i )
	{
		GOBehavior* bhvr = obj->m_bhvr_order[ i ];
		BhControllerBase* bcb = sgsHandle<BhControllerBase>(
			obj->m_level->GetSGSC(), bhvr->m_sgsObject );
		if( bcb && bcb->enabled )
			return bcb;
	}
	
	return def ? &g_DummyController : NULL;
}


