

#include "resources.hpp"

#include "imgui.hpp"


MeshResource::MeshResource( GameObject* obj ) : GOResource( obj ),
	m_isStatic( true ),
	m_isVisible( true ),
	m_localMatrix( Mat4::Identity ),
	m_matrixMode( MM_Relative ),
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
	if( m_matrixMode == MM_Relative )
		m_meshInst->SetTransform( m_localMatrix * m_obj->GetWorldMatrix() );
	else if( m_matrixMode == MM_Absolute )
		m_meshInst->SetTransform( m_localMatrix );
	else // MM_None / other
		m_meshInst->SetTransform( m_obj->GetWorldMatrix() );
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
	m_localMatrix( Mat4::Identity ),
	m_matrixMode( MM_Relative ),
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
		m_light->SetTransform( _GetFullMatrix() );
		m_light->UpdateTransform();
	}
	_UpEv();
}

Mat4 LightResource::_GetFullMatrix()
{
	if( m_matrixMode == MM_Relative )
		return m_localMatrix * m_obj->GetWorldMatrix();
	else if( m_matrixMode == MM_Absolute )
		return m_localMatrix;
	else // MM_None / other
		return m_obj->GetWorldMatrix();
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


