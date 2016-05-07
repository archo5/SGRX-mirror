

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


ParticleSystemResource::ParticleSystemResource( GameObject* obj ) : GOResource( obj ), m_soundEventOneShot(false)
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

void ParticleSystemResource::Tick()
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

void ParticleSystemResource::PreRender()
{
	m_psys.PreRender();
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
	printf("%s %s\n",playing?"true":"false",m_enabled?"true":"false");
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


