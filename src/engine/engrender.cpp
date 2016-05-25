

#include "renderer.hpp"



SGRX_DummyLightSampler g_DummyLightSampler;



//
// RENDERER
//


LightCount SGRX_Renderer_FindLights( const SGRX_Camera& CAM, SGRX_DrawItem* DI, int maxPL, int maxSL,
	SGRX_RPPointLightData* outPL, SGRX_RPSpotLightDataPS* outSL_PS, SGRX_RPSpotLightDataVS* outSL_VS, SGRX_Light** outSL_LT )
{
	LightCount out = { 0, 0 };
	SGRX_DrawItemLight* drlt = DI->_lightbuf_begin;
	while( drlt < DI->_lightbuf_end && ( out.numPL < maxPL || out.numSL < maxSL ) )
	{
		SGRX_Light* light = drlt->L;
		if( light->type == LIGHT_POINT && out.numPL < maxPL )
		{
			// copy data
			outPL->viewPos = CAM.mView.TransformPos( light->_tf_position );
			outPL->range = light->_tf_range;
			outPL->color = light->color;
			outPL->power = light->power;
			
			outPL++;
			out.numPL++;
			
			// remove light from array
			if( drlt > DI->_lightbuf_begin )
				*drlt = *DI->_lightbuf_begin;
			DI->_lightbuf_begin++;
		}
		if( light->type == LIGHT_SPOT && out.numSL < maxSL )
		{
			// copy data
			outSL_PS->viewPos = CAM.mView.TransformPos( light->_tf_position );
			outSL_PS->range = light->_tf_range;
			outSL_PS->color = light->color;
			outSL_PS->power = light->power;
			outSL_PS->viewDir = CAM.mView.TransformPos( light->_tf_direction ).Normalized();
			float tszx = 1, tszy = 1;
			if( light->shadowTexture )
			{
				const TextureInfo& texinfo = light->shadowTexture.GetInfo();
				tszx = texinfo.width;
				tszy = texinfo.height;
			}
			outSL_PS->angle = DEG2RAD( light->angle );
			outSL_PS->SMSize = V2( tszx, tszy );
			outSL_PS->invSMSize = V2( safe_fdiv( 1, tszx ), safe_fdiv( 1, tszy ) );
			
			outSL_VS->SMMatrix = light->viewProjMatrix;
			
			outSL_LT[0] = light;
			
			outSL_PS++;
			outSL_VS++;
			outSL_LT++;
			out.numSL++;
			
			// remove light from array
			if( drlt > DI->_lightbuf_begin )
				*drlt = *DI->_lightbuf_begin;
			DI->_lightbuf_begin++;
		}
		drlt++;
	}
	return out;
}


SGRX_IRenderControl::~SGRX_IRenderControl()
{
}

IRenderer::IRenderer() : m_inDebugDraw( false )
{
}

IRenderer::~IRenderer()
{
}

void IRenderer::SetRenderTargets( SGRX_IDepthStencilSurface* dss, const SGRX_RTClearInfo& info, TextureHandle rts[4] )
{
	SetRenderTargets( info, dss, rts );
}

void IRenderer::SortRenderItems( SGRX_Scene* scene )
{
	Game_FireEvent( EID_ScenePreRender, scene );
	
	_RS_UpdateProjectorMesh( scene );
	
	_RS_LoadInstItems( scene->camera.mView, 0, m_visible_meshes.data(),
		m_visible_meshes.size(), SGRX_TY_Solid | SGRX_TY_Decal | SGRX_TY_Transparent );
	
	m_riBaseStart = m_renderItemsBase.data();
	m_riBaseEnd = m_riBaseStart + m_renderItemsBase.size();
	
	m_riBaseSD = m_riBaseStart;
	while( m_riBaseSD < m_riBaseEnd && m_riBaseSD->IsSolid() )
		m_riBaseSD++;
	
	m_riBaseDT = m_riBaseSD;
	while( m_riBaseDT < m_riBaseEnd && m_riBaseDT->IsDecal() )
		m_riBaseDT++;
}

void IRenderer::RenderShadows( SGRX_Scene* scene, int pass_id )
{
	if( pass_id < 0 )
		return;
	
	for( size_t vsl_id = 0; vsl_id < m_visible_spot_lights.size(); ++vsl_id )
	{
		SGRX_Light* L = m_visible_spot_lights[ vsl_id ];
		if( !L->enabled ||
			!L->hasShadows ||
			!L->shadowTexture ||
			!L->shadowTexture->m_isRenderTexture )
			continue;
		
		const TextureInfo& TI = L->shadowTexture.GetInfo();
		DepthStencilSurfHandle dssh = GR_GetDepthStencilSurface( TI.width, TI.height, TI.format );
		GR_PreserveResource( dssh );
		
		SGRX_RTClearInfo info = { SGRX_RT_ClearAll, 0, 0, 1 };
		TextureHandle thlist[4] = { L->shadowTexture, NULL, NULL, NULL };
		SetRenderTargets( info, dssh, thlist );
		
		_RS_Cull_SpotLight_MeshList( scene, L );
		
		_RS_LoadInstItems( L->viewMatrix, 1, m_visible_spot_meshes.data(), m_visible_spot_meshes.size(), SGRX_TY_Solid );
		
		SGRX_Camera cam;
		L->GenerateCamera( cam );
		DoRenderItems( scene, pass_id, 1, cam, m_renderItemsAux.data(), m_renderItemsAux.data() + m_renderItemsAux.size() );
	}
}

void IRenderer::RenderMeshes( SGRX_Scene* scene, int pass_id, int maxrepeat, uint8_t types, SGRX_MeshInstance** milist, size_t micount )
{
	if( pass_id < 0 )
		return;
	
	_RS_LoadInstItems( scene->camera.mView, 1, milist, micount, SGRX_TY_Solid | SGRX_TY_Decal | SGRX_TY_Transparent );
	_RS_Compile_MeshLists( scene, milist, micount );
	DoRenderItems( scene, pass_id, 1, scene->camera, m_renderItemsAux.data(), m_renderItemsAux.data() + m_renderItemsAux.size() );
}

void IRenderer::RenderTypes( SGRX_Scene* scene, int pass_id, int maxrepeat, uint8_t types )
{
	if( pass_id < 0 )
		return;
	
	if( ( types & SGRX_TY_Solid ) != 0 && m_riBaseSD > m_riBaseStart )
	{
		DoRenderItems( scene, pass_id, maxrepeat, scene->camera, m_riBaseStart, m_riBaseSD );
	}
	if( ( types & SGRX_TY_Decal ) != 0 && m_riBaseDT > m_riBaseSD )
	{
		DoRenderItems( scene, pass_id, maxrepeat, scene->camera, m_riBaseSD, m_riBaseDT );
	}
	if( ( types & SGRX_TY_Transparent ) != 0 && m_riBaseEnd > m_riBaseDT )
	{
		DoRenderItems( scene, pass_id, maxrepeat, scene->camera, m_riBaseDT, m_riBaseEnd );
	}
}


uint64_t IRenderer::_RS_GenSortKey( const Mat4& view, SGRX_MeshInstance* MI, uint32_t part_id )
{
	static const int mtlflags_to_sortb2[8] = { 0, 0, 1, 1, 2, 2, 2, 2 };
	SGRX_DrawItem& DI = MI->m_drawItems[ part_id ];
//	SGRX_Material& MTL = MI->GetMaterial( part_id );
	uint64_t out = 0;
	
	// bytes 63-64: solid/decal/transparent
	out |= uint64_t( mtlflags_to_sortb2[ DI.type & 7 ] ) << 62;
	// bytes 55-62: sort index
	out |= uint64_t( MI->sortidx & 0xff ) << 54;
	float dist = 1 / ( 1 + fabsf( view.TransformPos( MI->matrix.GetTranslation() ).z ) );
	if( DI.type & (SGRX_TY_Decal|SGRX_TY_Transparent) )
	{
		// bytes 33-54: depth
		out |= uint64_t( dist * 0x3fffff ) << 32;
	}
	else
	{
		// bytes 1-22: depth backwards
		out |= uint64_t( ( 1 - dist ) * 0x3fffff ) << 0;
	}
	
	return out;
}

static bool renderitem_sort( const void* a, const void* b, void* )
{
	SGRX_CAST( RenderItem*, RIa, a );
	SGRX_CAST( RenderItem*, RIb, b );
	return RIa->key < RIb->key;
}

void IRenderer::_RS_LoadInstItems( const Mat4& view, int slot, SGRX_MeshInstance** milist, size_t micount, uint8_t flags )
{
	Array< RenderItem >* RIA = slot ? &m_renderItemsAux : &m_renderItemsBase;
	
	RIA->clear();
	for( size_t miid = 0; miid < micount; ++miid )
	{
		SGRX_MeshInstance* MI = milist[ miid ];
		for( size_t part_id = 0; part_id < MI->m_drawItems.size(); ++part_id )
		{
			if( MI->m_drawItems.size() <= part_id ||
				MI->GetMaterialCount() <= part_id ||
				( MI->m_drawItems[ part_id ].type & flags ) == 0 ||
				( MI->GetMaterial( part_id ).flags & SGRX_MtlFlag_Disable ) )
				continue;
			
			RenderItem RI = { _RS_GenSortKey( view, MI, part_id ), MI, part_id };
			RIA->push_back( RI );
		}
	}
	
	sgrx_combsort( RIA->data(), RIA->size(), sizeof(RenderItem), renderitem_sort, NULL );
}


void IRenderer::_RS_PreProcess( SGRX_Scene* scene )
{
	m_stats.Reset();
	
	_RS_Cull_Camera_Prepare( scene );
	m_stats.numVisMeshes = _RS_Cull_Camera_MeshList( scene );
	m_stats.numVisPLights = _RS_Cull_Camera_PointLightList( scene );
	m_stats.numVisSLights = _RS_Cull_Camera_SpotLightList( scene );
	
	// MESH INST/LIGHT RELATIONS & DrawItems
	_RS_Compile_MeshLists( scene, m_visible_meshes.data(), m_visible_meshes.size() );
}

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

static int sort_drawitemlight_by_light( const void* p1, const void* p2 )
{
	SGRX_DrawItemLight* mil1 = (SGRX_DrawItemLight*) p1;
	SGRX_DrawItemLight* mil2 = (SGRX_DrawItemLight*) p2;
	return mil1->L == mil2->L ? 0 : ( mil1->L < mil2->L ? -1 : 1 );
}

void IRenderer::_RS_Compile_MeshLists( SGRX_Scene* scene, SGRX_MeshInstance** milist, size_t micount )
{
	m_inst_light_buf.clear();
	for( size_t inst_id = 0; inst_id < micount; ++inst_id )
	{
		SGRX_MeshInstance* MI = milist[ inst_id ];
		MI->_Precache();
		
		for( size_t i = 0; i < MI->m_drawItems.size(); ++i )
		{
			SGRX_DrawItem& DI = MI->m_drawItems[ i ];
			
			DI._lightbuf_begin = (SGRX_DrawItemLight*) m_inst_light_buf.size_bytes();
			// POINT LIGHTS
			for( size_t light_id = 0; light_id < m_visible_point_lights.size(); ++light_id )
			{
				SGRX_Light* L = m_visible_point_lights[ light_id ];
				SGRX_DrawItemLight mil = { &DI, L };
				m_inst_light_buf.push_back( mil );
			}
			// SPOTLIGHTS
			for( size_t light_id = 0; light_id < m_visible_spot_lights.size(); ++light_id )
			{
				SGRX_Light* L = m_visible_spot_lights[ light_id ];
				SGRX_DrawItemLight mil = { &DI, L };
				m_inst_light_buf.push_back( mil );
			}
			DI._lightbuf_end = (SGRX_DrawItemLight*) m_inst_light_buf.size_bytes();
		}
	}
	
	// covert offsets to pointers
	for( size_t inst_id = 0; inst_id < micount; ++inst_id )
	{
		SGRX_MeshInstance* MI = milist[ inst_id ];
		
		for( size_t i = 0; i < MI->m_drawItems.size(); ++i )
		{
			SGRX_DrawItem& DI = MI->m_drawItems[ i ];
			
			DI._lightbuf_begin = (SGRX_DrawItemLight*)( (uintptr_t) DI._lightbuf_begin + (uintptr_t) m_inst_light_buf.data() );
			DI._lightbuf_end = (SGRX_DrawItemLight*)( (uintptr_t) DI._lightbuf_end + (uintptr_t) m_inst_light_buf.data() );
		}
	}
	
	/*  insts -> lights  TO  lights -> insts  */
	m_light_inst_buf = m_inst_light_buf;
	qsort( m_light_inst_buf.data(), m_light_inst_buf.size(), sizeof( SGRX_DrawItemLight ), sort_drawitemlight_by_light );
	
	for( size_t light_id = 0; light_id < scene->m_lights.size(); ++light_id )
	{
		SGRX_Light* L = scene->m_lights.item( light_id ).key;
		if( !L->enabled )
			continue;
		L->_dibuf_begin = NULL;
		L->_dibuf_end = NULL;
	}
	
	SGRX_DrawItemLight* pmil = m_light_inst_buf.data();
	SGRX_DrawItemLight* pmilend = pmil + m_light_inst_buf.size();
	
	while( pmil < pmilend )
	{
		if( !pmil->L->_dibuf_begin )
			pmil->L->_dibuf_begin = pmil;
		pmil->L->_dibuf_end = pmil + 1;
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
	m_projectorMaterials.clear();
	size_t stride = m_projectorVertexDecl.GetInfo().size;
	
	// generate vertex data
	for( size_t i = 0; i < m_visible_spot_lights.size(); ++i )
	{
		SGRX_Light* L = m_visible_spot_lights[ i ];
		if( L->type != LIGHT_PROJ )
			continue;
		
		L->UpdateTransform();
		
		SGRX_MeshPart mp = { m_projectorVertices.size() / stride, 0, m_projectorIndices.size(), 0 };
		
	//	SGRX_Camera lightcam;
	//	L->GenerateCamera( lightcam );
	//	scene->GenerateProjectionMesh( lightcam, m_projectorVertices, m_projectorIndices, L->layers );
		
		float invZNearToZFar = safe_fdiv( 1.0f, L->range * 0.999f );
		
		SGRX_DrawItemLight* mil = L->_dibuf_begin, *milend = L->_dibuf_end;
		while( mil < milend )
		{
			SGRX_MeshInstance* MI = mil->DI->MI;
			if( ( MI->layers & L->layers ) != 0 && MI->IsSkinned() == false )
			{
				SGRX_IMesh* M = MI->GetMesh();
				if( M && MI->GetMaterialCount() > mil->DI->part )
				{
					SGRX_Material& MTL = MI->GetMaterial( mil->DI->part );
					if( ( MTL.flags & SGRX_MtlFlag_Disable ) == 0 && MTL.blendMode == SGRX_MtlBlend_None )
					{
						size_t vertoff = m_projectorVertices.size();
						M->Clip( MI->matrix, L->viewProjMatrix, m_projectorVertices, true, invZNearToZFar, 0xffffffff, mil->DI->part, 1 );
						SGRX_DoIndexTriangleMeshVertices( m_projectorIndices, m_projectorVertices, vertoff, 48 );
					}
				}
			}
			mil++;
		}
		
		mp.vertexCount = m_projectorVertices.size() / stride - mp.vertexOffset;
		mp.indexCount = m_projectorIndices.size() - mp.indexOffset;
		for( size_t ipos = mp.indexOffset; ipos < m_projectorIndices.size(); ++ipos )
			m_projectorIndices[ ipos ] -= mp.vertexOffset;
		
		m_projectorMeshParts.push_back( mp );
		m_projectorMaterials.push_back( &L->projectionMaterial );
	}
	
	// apply new data
	bool apply = m_projectorVertices.size() && m_projectorIndices.size() && m_projectorMeshParts.size();
	scene->m_projMeshInst->enabled = apply;
	if( apply )
	{
		m_projectorMesh->SetVertexData( m_projectorVertices.data(), m_projectorVertices.size_bytes(), m_projectorVertexDecl );
		m_projectorMesh->SetIndexData( m_projectorIndices.data(), m_projectorIndices.size_bytes(), true );
		m_projectorMesh->SetPartData( m_projectorMeshParts.data(), m_projectorMeshParts.size() );
		
		scene->m_projMeshInst->SetMesh( m_projectorMesh, false );
		scene->m_projMeshInst->SetMaterialCount( m_projectorMaterials.size() );
		for( size_t i = 0; i < m_projectorMaterials.size(); ++i )
		{
			scene->m_projMeshInst->GetMaterial( i ) = *m_projectorMaterials[ i ];
		}
	}
	
	return apply;
}


//
// LIGHT TREE
//

#define LIGHTTREE_SAMPLE_EXTENTS 10.0f
#define LIGHTTREE_MIN_SPLIT_SIZE 32
#define LIGHTTREE_MAX_SPLIT_DEPTH 8

struct _LightTree_SortIndices_Data
{
	SGRX_LightTree* LT;
	Vec3 splitnrm;
};

int _LightTree_SortIndices( const void* A, const void* B, void* UD )
{
	SGRX_CAST( int32_t*, idx_a, A );
	SGRX_CAST( int32_t*, idx_b, B );
	SGRX_CAST( _LightTree_SortIndices_Data*, data, UD );
	Array< Vec3 >& pos = data->LT->m_pos;
	float dot_a = Vec3Dot( data->splitnrm, pos[ *idx_a ] );
	float dot_b = Vec3Dot( data->splitnrm, pos[ *idx_b ] );
	return dot_a == dot_b ? 0 : ( dot_a < dot_b ? -1 : 1 );
}

void _LightTree_MakeNode( SGRX_LightTree* LT, int32_t node,
                          int32_t* sampidx_data, size_t sampidx_count, int depth )
{
	SGRX_LightTree::Node& N = LT->m_nodes[ node ];
	
	Vec3 bbmin = V3( FLT_MAX ), bbmax = V3( -FLT_MAX );
	for( size_t i = 0; i < sampidx_count; ++i )
	{
		Vec3 pos = LT->m_pos[ sampidx_data[ i ] ];
		bbmin = Vec3::Min( bbmin, pos );
		bbmax = Vec3::Max( bbmax, pos );
	}
	N.bbmin = bbmin;
	N.bbmax = bbmax;
	
	if( sampidx_count > LIGHTTREE_MIN_SPLIT_SIZE &&
		depth < LIGHTTREE_MAX_SPLIT_DEPTH )
	{
		// split
		int32_t ch = LT->m_nodes.size();
		N.sdo = -1;
		N.ch = ch;
		
		// find split direction
		Vec3 bbsize = bbmax - bbmin;
		Vec3 splitnrm = V3(0,0,1);
		if( bbsize.x > bbsize.y && bbsize.x > bbsize.z ) splitnrm = V3(1,0,0);
		else if( bbsize.y > bbsize.x && bbsize.y > bbsize.z ) splitnrm = V3(0,1,0);
		
		// sort and find middle
		Array< int32_t > subsampidx( sampidx_data, sampidx_count );
		_LightTree_SortIndices_Data LTSID = { LT, splitnrm };
		sgrx_quicksort( subsampidx.data(), subsampidx.size(), sizeof(int32_t),
			_LightTree_SortIndices, &LTSID );
		size_t mid = sampidx_count / 2;
		
		// -- DO NOT TOUCH <N> ANYMORE --
		LT->m_nodes.push_back( SGRX_LightTree::Node() );
		LT->m_nodes.push_back( SGRX_LightTree::Node() );
		_LightTree_MakeNode( LT, ch + 0, subsampidx.data(), mid, depth + 1 );
		_LightTree_MakeNode( LT, ch + 1, &subsampidx[ mid ], sampidx_count - mid, depth + 1 );
	}
	else
	{
		// make leaf
		N.sdo = LT->m_sampidx.size();
		N.ch = -1;
		LT->m_sampidx.push_back( sampidx_count );
		LT->m_sampidx.append( sampidx_data, sampidx_count );
	}
}


#ifdef DEBUG_LIGHT_TREE
int lt_samples = 0;
int lt_nodes = 0;
int lt_nodes_all = 0;
#endif
void _LightTree_TestNode( SGRX_LightTree* LT, int32_t node, const Vec3 qbb[3],
                          SGRX_LightTree::Colors* outaddcol, float* outaddwt )
{
#ifdef DEBUG_LIGHT_TREE
	lt_nodes_all++;
#endif
	SGRX_LightTree::Node& N = LT->m_nodes[ node ];
	if( qbb[0].x > N.bbmax.x || qbb[1].x < N.bbmin.x ||
		qbb[0].y > N.bbmax.y || qbb[1].y < N.bbmin.y ||
		qbb[0].z > N.bbmax.z || qbb[1].z < N.bbmin.z )
		return;
	
#ifdef DEBUG_LIGHT_TREE
	lt_nodes++;
#endif
	// samples
	if( N.sdo != -1 )
	{
		size_t chcount = LT->m_sampidx[ N.sdo ], off = N.sdo + 1;
		
#ifdef DEBUG_LIGHT_TREE
		lt_samples += chcount;
#endif
		
		SGRX_LightTree::Colors tmpout;
		tmpout.Clear();
		float tmpwt = 0;
		
		// gather
		for( size_t i = 0; i < chcount; ++i )
		{
			size_t idx = LT->m_sampidx[ i + off ];
			// distfac: max = 100 to avoid precision issues
			float distfac = powf( 2.0f / ( 1.0f + ( qbb[2] - LT->m_pos[ idx ] ).Length() ), 16.0f );
			for( int c = 0; c < 6; ++c )
				tmpout.color[ c ] += LT->m_colors[ idx ].color[ c ] * distfac;
			tmpwt += distfac;
		}
		
		// commit
		for( int c = 0; c < 6; ++c )
			outaddcol->color[ c ] += tmpout.color[ c ];
		*outaddwt += tmpwt;
	}
	
	// child nodes
	if( N.ch != -1 )
	{
		_LightTree_TestNode( LT, N.ch + 0, qbb, outaddcol, outaddwt );
		_LightTree_TestNode( LT, N.ch + 1, qbb, outaddcol, outaddwt );
	}
}

void SGRX_LightTree::SetSamples( Sample* samples, size_t count )
{
	m_pos.clear();
	m_colors.clear();
	m_pos.reserve( count );
	m_colors.reserve( count );
	for( size_t i = 0; i < count; ++i )
	{
		if( m_pos.find_first_at( samples[ i ].pos ) == NOT_FOUND )
		{
			m_pos.push_back( samples[ i ].pos );
			m_colors.push_back( samples[ i ] );
		}
	}
	
	_RegenBVH();
}

void SGRX_LightTree::SetSamplesUncolored( Vec3* samples, size_t count, const Vec3& col )
{
	Colors defcol = { { col, col, col, col, col, col } };
	m_pos.clear();
	m_colors.clear();
	m_pos.reserve( count );
	m_colors.reserve( count );
	for( size_t i = 0; i < count; ++i )
	{
		if( m_pos.find_first_at( samples[ i ] ) == NOT_FOUND )
		{
			m_pos.push_back( samples[ i ] );
			m_colors.push_back( defcol );
		}
	}
	
	_RegenBVH();
}

void SGRX_LightTree::GetColors( Vec3 pos, Colors* out )
{
	Vec3 qbb[3] =
	{
		pos - V3(LIGHTTREE_SAMPLE_EXTENTS),
		pos + V3(LIGHTTREE_SAMPLE_EXTENTS),
		pos
	};
	
	Colors col;
	col.Clear();
	float total_weight = 0;
	
	if( m_nodes.size() )
	{
#ifdef DEBUG_LIGHT_TREE
		int s = lt_samples;
		int n = lt_nodes;
		int a = lt_nodes_all;
#endif
		
		_LightTree_TestNode( this, 0, qbb, &col, &total_weight );
		
#ifdef DEBUG_LIGHT_TREE
		LOG << "LIGHT TREE SAMPLING SESSION AT " << pos << ":"
			<< "\n- samples found: " << ( lt_samples - s )
			<< "\n- nodes processed: " << ( lt_nodes - n )
			<< "\n- nodes traversed: " << ( lt_nodes_all - a );
#endif
		
		if( total_weight > SMALL_FLOAT )
		{
			for( int c = 0; c < 6; ++c )
				col.color[ c ] /= total_weight;
		}
	}
	
	*out = col;
}

void SGRX_LightTree::_RegenBVH()
{
	m_nodes.clear();
	m_sampidx.clear();
	if( m_pos.size() == 0 )
		return;
	
	// BVH generation...
	m_nodes.push_back( Node() );
	Array< int32_t > sampidx;
	for( size_t i = 0; i < m_pos.size(); ++i )
		sampidx.push_back( i );
	_LightTree_MakeNode( this, 0, sampidx.data(), sampidx.size(), 0 );
}


void SGRX_LightSampler::SampleLight( const Vec3& pos, Vec3& outcolor )
{
	Vec3 outcolors[6];
	SampleLight( pos, outcolors );
	Vec3 oc = V3(0);
	for( int i = 0; i < 6; ++i )
		oc += outcolors[ i ];
	outcolor = oc; // ??? / 6;
}

static const Vec3 LightSampler_Dir[6] =
{
	V3(1,0,0), V3(-1,0,0),
	V3(0,1,0), V3(0,-1,0),
	V3(0,0,1), V3(0,0,-1),
};

void SGRX_LightSampler::SampleLight( const Vec3& pos, const Vec3& dir, Vec3& outcolor )
{
	if( dir.NearZero() )
	{
		SampleLight( pos, outcolor );
		return;
	}
	Vec3 outcolors[6];
	SampleLight( pos, outcolors );
	Vec3 oc = V3(0);
	float sum = 0;
	for( int i = 0; i < 6; ++i )
	{
		float factor = Vec3Dot( dir, LightSampler_Dir[ i ] );
		oc += outcolors[ i ] * factor;
		sum += factor;
	}
	outcolor = oc / sum;
}

void SGRX_LightSampler::LightMeshAt( SGRX_MeshInstance* MI, Vec3 pos, int constoff )
{
	Vec3 colors[6];
	SampleLight( pos, colors );
	MI->constants[ constoff + 0 ] = V4( colors[0], 1 );
	MI->constants[ constoff + 1 ] = V4( colors[1], 1 );
	MI->constants[ constoff + 2 ] = V4( colors[2], 1 );
	MI->constants[ constoff + 3 ] = V4( colors[3], 1 );
	MI->constants[ constoff + 4 ] = V4( colors[4], 1 );
	MI->constants[ constoff + 5 ] = V4( colors[5], 1 );
}

void SGRX_LightSampler::LightMesh( SGRX_MeshInstance* MI, Vec3 off, int constoff )
{
	LightMeshAt( MI, MI->matrix.TransformPos( off ), constoff );
}

SGRX_DummyLightSampler::SGRX_DummyLightSampler() : defval(0.25f)
{
}

void SGRX_DummyLightSampler::SampleLight( const Vec3& pos, Vec3& outcolor )
{
	outcolor = V3(defval);
}

void SGRX_DummyLightSampler::SampleLight( const Vec3& pos, Vec3 outcolors[6] )
{
	for( int i = 0; i < 6; ++i )
		outcolors[ i ] = V3(defval);
}

void SGRX_DummyLightSampler::SampleLight( const Vec3& pos, const Vec3& dir, Vec3& outcolor )
{
	outcolor = V3(defval);
}

SGRX_DummyLightSampler& GR_GetDummyLightSampler()
{
	return g_DummyLightSampler;
}

void SGRX_LightTreeSampler::SampleLight( const Vec3& pos, Vec3 outcolors[6] )
{
	SGRX_LightTree::Colors cols;
	if( m_lightTree )
	{
		m_lightTree->GetColors( pos, &cols );
		for( int i = 0; i < 6; ++i )
			outcolors[ i ] = cols.color[ i ];
	}
	else
	{
		for( int i = 0; i < 6; ++i )
			outcolors[ i ] = V3(1);
	}
}



