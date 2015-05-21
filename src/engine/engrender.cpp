

#define USE_HASHTABLE
#include "renderer.hpp"



void IRenderer::_RS_Cull_Camera_Prepare( SGRX_Scene* scene )
{
	SGRX_Cull_Camera_Prepare( scene );
}

uint32_t IRenderer::_RS_Cull_Camera_MeshList( SGRX_Scene* scene )
{
	m_visible_meshes.clear();
	return SGRX_Cull_Camera_MeshList( m_visible_meshes, m_scratchMem, scene );
}

uint32_t IRenderer::_RS_Cull_Camera_PointLightList( SGRX_Scene* scene )
{
	m_visible_point_lights.clear();
	return SGRX_Cull_Camera_PointLightList( m_visible_point_lights, m_scratchMem, scene );
}

uint32_t IRenderer::_RS_Cull_Camera_SpotLightList( SGRX_Scene* scene )
{
	m_visible_spot_lights.clear();
	return SGRX_Cull_Camera_SpotLightList( m_visible_spot_lights, m_scratchMem, scene );
}

uint32_t IRenderer::_RS_Cull_SpotLight_MeshList( SGRX_Scene* scene, SGRX_Light* L )
{
	SGRX_Cull_SpotLight_Prepare( scene, L );
	m_visible_spot_meshes.clear();
	return SGRX_Cull_SpotLight_MeshList( m_visible_spot_meshes, m_scratchMem, scene, L );
}

static int sort_meshinstlight_by_light( const void* p1, const void* p2 )
{
	SGRX_MeshInstLight* mil1 = (SGRX_MeshInstLight*) p1;
	SGRX_MeshInstLight* mil2 = (SGRX_MeshInstLight*) p2;
	return mil1->L == mil2->L ? 0 : ( mil1->L < mil2->L ? -1 : 1 );
}

void IRenderer::_RS_Compile_MeshLists( SGRX_Scene* scene )
{
	m_inst_light_buf.clear();
	for( size_t inst_id = 0; inst_id < scene->m_meshInstances.size(); ++inst_id )
	{
		SGRX_MeshInstance* MI = scene->m_meshInstances.item( inst_id ).key;
		
		MI->_lightbuf_begin = NULL;
		MI->_lightbuf_end = NULL;
		
		if( !MI->mesh || !MI->enabled || MI->unlit )
			continue;
		MI->_lightbuf_begin = (SGRX_MeshInstLight*) m_inst_light_buf.size_bytes();
		// POINT LIGHTS
		for( size_t light_id = 0; light_id < m_visible_point_lights.size(); ++light_id )
		{
			SGRX_Light* L = m_visible_point_lights[ light_id ];
			SGRX_MeshInstLight mil = { MI, L };
			m_inst_light_buf.push_back( mil );
		}
		// SPOTLIGHTS
		for( size_t light_id = 0; light_id < m_visible_spot_lights.size(); ++light_id )
		{
			SGRX_Light* L = m_visible_spot_lights[ light_id ];
			SGRX_MeshInstLight mil = { MI, L };
			m_inst_light_buf.push_back( mil );
		}
		MI->_lightbuf_end = (SGRX_MeshInstLight*) m_inst_light_buf.size_bytes();
	}
	for( size_t inst_id = 0; inst_id < scene->m_meshInstances.size(); ++inst_id )
	{
		SGRX_MeshInstance* MI = scene->m_meshInstances.item( inst_id ).key;
		if( !MI->mesh || !MI->enabled || MI->unlit )
			continue;
		MI->_lightbuf_begin = (SGRX_MeshInstLight*)( (uintptr_t) MI->_lightbuf_begin + (uintptr_t) m_inst_light_buf.data() );
		MI->_lightbuf_end = (SGRX_MeshInstLight*)( (uintptr_t) MI->_lightbuf_end + (uintptr_t) m_inst_light_buf.data() );
	}
	
	/*  insts -> lights  TO  lights -> insts  */
	m_light_inst_buf = m_inst_light_buf;
	qsort( m_light_inst_buf.data(), m_light_inst_buf.size(), sizeof( SGRX_MeshInstLight ), sort_meshinstlight_by_light );
	
	for( size_t light_id = 0; light_id < scene->m_lights.size(); ++light_id )
	{
		SGRX_Light* L = scene->m_lights.item( light_id ).key;
		if( !L->enabled )
			continue;
		L->_mibuf_begin = NULL;
		L->_mibuf_end = NULL;
	}
	
	SGRX_MeshInstLight* pmil = m_light_inst_buf.data();
	SGRX_MeshInstLight* pmilend = pmil + m_light_inst_buf.size();
	
	while( pmil < pmilend )
	{
		if( !pmil->L->_mibuf_begin )
			pmil->L->_mibuf_begin = pmil;
		pmil->L->_mibuf_end = pmil + 1;
		pmil++;
	}
}


bool IRenderer::_RS_ProjectorInit()
{
	m_projectorMesh = GR_CreateMesh();
	m_projectorVertexDecl = GR_GetVertexDecl( SGRX_VDECL_DECAL );
	
	return m_projectorMesh && m_projectorVertexDecl;
}

void IRenderer::_RS_ProjectorFree()
{
	m_projectorMesh = NULL;
	m_projectorVertexDecl = NULL;
	m_projectorMeshParts.clear();
}

bool IRenderer::_RS_UpdateProjectorMesh( SGRX_Scene* scene )
{
	// clear all
	m_projectorVertices.clear();
	m_projectorIndices.clear();
	m_projectorMeshParts.clear();
	m_projectorList.clear();
	size_t stride = m_projectorVertexDecl.GetInfo().size;
	
	// generate vertex data
	for( size_t i = 0; i < m_visible_spot_lights.size(); ++i )
	{
		SGRX_Light* L = m_visible_spot_lights[ i ];
		if( L->type != LIGHT_PROJ || !L->projectionShader )
			continue;
		
		L->UpdateTransform();
		
		SGRX_MeshPart mp = { m_projectorVertices.size() / stride, 0, m_projectorIndices.size(), 0 };
		
	//	SGRX_Camera lightcam;
	//	L->GenerateCamera( lightcam );
	//	scene->GenerateProjectionMesh( lightcam, m_projectorVertices, m_projectorIndices, L->layers );
		
		float invZNearToZFar = safe_fdiv( 1.0f, L->range * 0.999f );
		
		SGRX_MeshInstLight* mil = L->_mibuf_begin, *milend = L->_mibuf_end;
		while( mil < milend )
		{
			SGRX_MeshInstance* MI = mil->MI;
			if( ( MI->layers & L->layers ) != 0 && MI->skin_matrices.size() == 0 )
			{
				SGRX_IMesh* M = MI->mesh;
				if( MI->transparent == false && M )
				{
					size_t vertoff = m_projectorVertices.size();
					M->Clip( MI->matrix, L->viewProjMatrix, m_projectorVertices, true, invZNearToZFar );
					SGRX_DoIndexTriangleMeshVertices( m_projectorIndices, m_projectorVertices, vertoff, 48 );
				}
			}
			mil++;
		}
		
		mp.vertexCount = m_projectorVertices.size() / stride - mp.vertexOffset;
		mp.indexCount = m_projectorIndices.size() - mp.indexOffset;
		for( size_t ipos = mp.indexOffset; ipos < m_projectorIndices.size(); ++ipos )
			m_projectorIndices[ ipos ] -= mp.vertexOffset;
		
		m_projectorMeshParts.push_back( mp );
		m_projectorList.push_back( L );
	}
	
	// apply new data
	bool apply = m_projectorVertices.size() && m_projectorIndices.size() && m_projectorMeshParts.size();
	if( apply )
	{
		m_projectorMesh->SetVertexData( m_projectorVertices.data(), m_projectorVertices.size_bytes(), m_projectorVertexDecl, false );
		m_projectorMesh->SetIndexData( m_projectorIndices.data(), m_projectorIndices.size_bytes(), true );
		m_projectorMesh->SetPartData( m_projectorMeshParts.data(), m_projectorMeshParts.size() );
	}
	
	return apply;
}

