

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

void IRenderer::RenderShadows( SGRX_Scene* scene, uint8_t pass_id )
{
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

void IRenderer::RenderMeshes( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, uint8_t types, SGRX_MeshInstance** milist, size_t micount )
{
	_RS_LoadInstItems( scene->camera.mView, 1, milist, micount, SGRX_TY_Solid | SGRX_TY_Decal | SGRX_TY_Transparent );
	_RS_Compile_MeshLists( scene, milist, micount );
	DoRenderItems( scene, pass_id, 1, scene->camera, m_renderItemsAux.data(), m_renderItemsAux.data() + m_renderItemsAux.size() );
}

void IRenderer::RenderTypes( SGRX_Scene* scene, uint8_t pass_id, int maxrepeat, uint8_t types )
{
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
	if( DI.type & SGRX_TY_Transparent )
	{
		// bytes 33-54: depth
		out |= uint64_t( ( 1 - dist ) * 0x3fffff ) << 32;
	}
	else
	{
		// bytes 1-22: depth backwards
		out |= uint64_t( dist * 0x3fffff ) << 0;
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
		
		SGRX_DrawItemLight* mil = L->_dibuf_begin, *milend = L->_dibuf_end;
		while( mil < milend )
		{
			SGRX_MeshInstance* MI = mil->DI->MI;
			if( ( MI->layers & L->layers ) != 0 && MI->IsSkinned() == false )
			{
				SGRX_IMesh* M = MI->GetMesh();
				if( M )
				{
					size_t vertoff = m_projectorVertices.size();
					M->Clip( MI->matrix, L->viewProjMatrix, m_projectorVertices, true, invZNearToZFar, 0xffffffff, mil->DI->part, 1 );
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


//
// LIGHT TREE
//

#define LIGHTTREE_SAMPLE_EXTENTS 10.0f
#define LIGHTTREE_MIN_SPLIT_SIZE 4
#define LIGHTTREE_MAX_SPLIT_DEPTH 16

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

void _LightTree_TestNode( SGRX_LightTree* LT, int32_t node, const Vec3 qbb[3],
                          SGRX_LightTree::Colors* outaddcol, float* outaddwt )
{
	SGRX_LightTree::Node& N = LT->m_nodes[ node ];
	if( qbb[0].x > N.bbmax.x || qbb[1].x < N.bbmin.x ||
		qbb[0].y > N.bbmax.y || qbb[1].y < N.bbmin.y ||
		qbb[0].z > N.bbmax.z || qbb[1].z < N.bbmin.z )
		return;
	
	// samples
	if( N.sdo != -1 )
	{
		size_t chcount = LT->m_sampidx[ N.sdo ], off = N.sdo + 1;
		for( size_t i = 0; i < chcount; ++i )
		{
			size_t idx = LT->m_sampidx[ i + off ];
			 // distfac: max = 100 to avoid precision issues
			float distfac = powf( 2.0f / ( 1.0f + ( qbb[2] - LT->m_pos[ idx ] ).Length() ), 16.0f );
			for( int c = 0; c < 6; ++c )
				outaddcol->color[ c ] += LT->m_colors[ idx ].color[ c ] * distfac;
			*outaddwt += distfac;
		}
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
		_LightTree_TestNode( this, 0, qbb, &col, &total_weight );
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




//
// BATCH RENDERER
//

void GR2D_SetColor( float r, float g, float b, float a )
{
	GR2D_GetBatchRenderer().Col( r, g, b, a );
}

BatchRenderer::BatchRenderer( struct IRenderer* r ) : m_renderer( r ), m_diff( false )
{
	m_proto.x = 0;
	m_proto.y = 0;
	m_proto.z = 0;
	m_proto.u = 0;
	m_proto.v = 0;
	m_proto.color = 0xffffffff;
	worldMatrix = Mat4::Identity;
	viewMatrix = Mat4::Identity;
	
	m_vertexDecl = GR_GetVertexDecl( "pf3cb40f2" );
	m_defVShader = GR_GetVertexShader( "sys_batchvtx" );
	m_defPShader = GR_GetPixelShader( "sys_batchvtx" );
	m_whiteTex = GR_CreateTexture( 1, 1, TEXFORMAT_RGBA8, 0, 1 );
	uint32_t whiteCol = 0xffffffff;
	m_whiteTex.UploadRGBA8Part( &whiteCol );
	
	m_currState.vshader = m_nextState.vshader = m_defVShader;
	m_currState.pshader = m_nextState.pshader = m_defPShader;
	
	ResetState();
}

BatchRenderer& BatchRenderer::AddVertices( Vertex* verts, int count )
{
	m_verts.reserve( m_verts.size() + count );
	for( int i = 0; i < count; ++i )
		AddVertex( verts[ i ] );
	return *this;
}

BatchRenderer& BatchRenderer::AddVertex( const Vertex& vert )
{
	if( m_diff )
	{
		Flush();
		m_currState = m_nextState;
		m_diff = false;
	}
	m_verts.push_back( vert );
	return *this;
}

BatchRenderer& BatchRenderer::Colb( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	uint32_t col;
	col = COLOR_RGBA( r, g, b, a );
	m_proto.color = col;
	return *this;
}

BatchRenderer& BatchRenderer::Prev( int i )
{
	if( i < 0 || i >= (int) m_verts.size() )
		AddVertex( m_proto );
	else
	{
		Vertex v = m_verts[ m_verts.size() - 1 - i ];
		AddVertex( v );
	}
	return *this;
}

BatchRenderer& BatchRenderer::RawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0, z );
	Tex( 1, 0 ); Pos( x1, y1, z );
	Tex( 1, 1 ); Pos( x2, y2, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x3, y3, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Quad( float x0, float y0, float x1, float y1, float z )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x0, y0, z );
	Tex( 1, 0 ); Pos( x1, y0, z );
	Tex( 1, 1 ); Pos( x1, y1, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x0, y1, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::QuadFrame( float x0, float y0, float x1, float y1, float ix0, float iy0, float ix1, float iy1, float z )
{
	RawQuad( x0, y0, x1, y0, ix1, iy0, ix0, iy0, z );
	RawQuad( x1, y0, x1, y1, ix1, iy1, ix1, iy0, z );
	RawQuad( x1, y1, x0, y1, ix0, iy1, ix1, iy1, z );
	RawQuad( x0, y1, x0, y0, ix0, iy0, ix0, iy1, z );
	return *this;
}

BatchRenderer& BatchRenderer::VPQuad( SGRX_Viewport* vp, float z )
{
	SetPrimitiveType( PT_Triangles );
	
	float x0 = vp ? safe_fdiv( vp->x0, GR_GetWidth() ) : 0;
	float y0 = vp ? safe_fdiv( vp->y0, GR_GetHeight() ) : 0;
	float x1 = vp ? safe_fdiv( vp->x1, GR_GetWidth() ) : 1;
	float y1 = vp ? safe_fdiv( vp->y1, GR_GetHeight() ) : 1;
	
	Tex( x0, y0 ); Pos( x0, y0, z );
	Tex( x1, y0 ); Pos( x1, y0, z );
	Tex( x1, y1 ); Pos( x1, y1, z );
	Prev( 0 );
	Tex( x0, y1 ); Pos( x0, y1, z );
	Prev( 4 );
	
	return *this;
}

BatchRenderer& BatchRenderer::TurnedBox( float x, float y, float dx, float dy, float z )
{
	float tx = -dy;
	float ty = dx;
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( x - dx - tx, y - dy - ty, z );
	Tex( 1, 0 ); Pos( x - dx + tx, y - dy + ty, z );
	Tex( 1, 1 ); Pos( x + dx + tx, y + dy + ty, z );
	Prev( 0 );
	Tex( 0, 1 ); Pos( x + dx - tx, y + dy - ty, z );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Button( Vec4 rect, Vec4 bdr, Vec4 texbdr, float z )
{
	SetPrimitiveType( PT_Triangles );
	
	float pox[4] = { rect.x, rect.x + bdr.x, rect.z - bdr.z, rect.z };
	float poy[4] = { rect.y, rect.y + bdr.y, rect.w - bdr.w, rect.w };
	float txx[4] = { 0, texbdr.x, 1 - texbdr.z, 1 };
	float txy[4] = { 0, texbdr.y, 1 - texbdr.w, 1 };
	
	for( int y = 0; y < 3; ++y )
	{
		for( int x = 0; x < 3; ++x )
		{
			Tex( txx[x], txy[y] ); Pos( pox[x], poy[y], z );
			Tex( txx[x+1], txy[y] ); Pos( pox[x+1], poy[y], z );
			Tex( txx[x+1], txy[y+1] ); Pos( pox[x+1], poy[y+1], z );
			Prev(0);
			Tex( txx[x], txy[y+1] ); Pos( pox[x], poy[y+1], z );
			Prev(4);
		}
	}
	
	return *this;
}

BatchRenderer& BatchRenderer::Poly( const void* data, int count, float z, int stride )
{
	SGRX_CAST( const uint8_t*, bp, data );
	SetPrimitiveType( PT_TriangleStrip );
	for( int i = 0; i < count; ++i )
	{
		int v;
		if( i % 2 == 0 )
			v = i / 2;
		else
			v = count - 1 - i / 2;
		Pos( *(float*)(bp+v*stride), *(float*)(bp+v*stride+4), stride >= 12 ? *(float*)(bp+i*stride+8) : z );
	}
	return *this;
}

BatchRenderer& BatchRenderer::PolyOutline( const void* data, int count, float z, int stride )
{
	SGRX_CAST( const uint8_t*, bp, data );
	SetPrimitiveType( PT_LineStrip );
	for( int i = 0; i < count; ++i )
		Pos( *(float*)(bp+i*stride), *(float*)(bp+i*stride+4), stride >= 12 ? *(float*)(bp+i*stride+8) : z );
	Prev( count - 1 );
	return *this;
}

BatchRenderer& BatchRenderer::Sprite( const Vec3& pos, const Vec3& dx, const Vec3& dy )
{
	SetPrimitiveType( PT_Triangles );
	Tex( 0, 0 ); Pos( pos + dx + dy );
	Tex( 1, 0 ); Pos( pos - dx + dy );
	Tex( 1, 1 ); Pos( pos - dx - dy );
	Prev( 0 );
	Tex( 0, 1 ); Pos( pos + dx - dy );
	Prev( 4 );
	return *this;
}

BatchRenderer& BatchRenderer::Sprite( const Vec3& pos, float sx, float sy )
{
	Vec3 dx = invMatrix.TransformNormal( V3( 1, 0, 0 ) ).Normalized() * sx;
	Vec3 dy = invMatrix.TransformNormal( V3( 0, 1, 0 ) ).Normalized() * sy;
	return Sprite( pos, dx, dy );
}

BatchRenderer& BatchRenderer::TexLine( const Vec2& p0, const Vec2& p1, float rad )
{
	SetPrimitiveType( PT_Triangles );
	Vec2 D = ( p1 - p0 ).Normalized();
	Vec2 T = D.Perp();
	
	Tex( 0   , 0 ); Pos( p0 + ( -D +T ) * rad );
	Tex( 0.5f, 0 ); Pos( p0 + (    +T ) * rad );
	Tex( 0.5f, 1 ); Pos( p0 + (    -T ) * rad );
	Prev( 0 ); // #4 requesting #3
	Tex( 0   , 1 ); Pos( p0 + ( -D -T ) * rad );
	Prev( 4 ); // #6 requesting #1
	
	Prev( 4 ); // #7 requesting #2
	Tex( 0.5f, 0 ); Pos( p1 + (    +T ) * rad );
	Tex( 0.5f, 1 ); Pos( p1 + (    -T ) * rad );
	Prev( 0 ); // #10 requesting #9
	Prev( 7 ); // #11 requesting #3
	Prev( 9 ); // #12 requesting #2
	
	Prev( 4 ); // #13 requesting #8
	Tex( 1   , 0 ); Pos( p1 + ( +D +T ) * rad );
	Tex( 1   , 1 ); Pos( p1 + ( +D -T ) * rad );
	Prev( 0 ); // #16 requesting #15
	Prev( 7 ); // #17 requesting #9
	Prev( 9 ); // #18 requesting #8
	
	return *this;
}

BatchRenderer& BatchRenderer::CircleFill( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Triangles );
		float ad = M_PI * 2.0f / verts;
		float a = ad, ps = 0, pc = 1;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sin( a ), cc = cos( a );
			Pos( x + ps * r, y + pc * r, z );
			Pos( x + cs * r, y + cc * r, z );
			Pos( x, y, z );
			pc = cc;
			ps = cs;
			a += ad;
		}
	}
	return *this;
}

BatchRenderer& BatchRenderer::CircleOutline( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_LineStrip );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			Pos( x + sin( a ) * r, y + cos( a ) * r, z );
			a += ad;
		}
		Prev( verts - 1 );
	}
	return *this;
}

BatchRenderer& BatchRenderer::CircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts )
{
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			if( i > 1 )
				Prev( 0 );
			Pos( pos + sin( a ) * dx + cos( a ) * dy );
			a += ad;
		}
		Prev( 0 );
		Prev( verts * 2 - 2 );
	}
	return *this;
}

BatchRenderer& BatchRenderer::HalfCircleOutline( const Vec3& pos, const Vec3& dx, const Vec3& dy, int verts )
{
	if( verts >= 3 )
	{
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI / ( verts - 1 );
		for( int i = 0; i < verts; ++i )
		{
			if( i > 1 )
				Prev( 0 );
			Pos( pos + sin( a ) * dy + cos( a ) * dx );
			a += ad;
		}
	}
	return *this;
}

BatchRenderer& BatchRenderer::SphereOutline( const Vec3& pos, float radius, int verts )
{
	CircleOutline( pos, V3(radius,0,0), V3(0,radius,0), verts );
	CircleOutline( pos, V3(0,radius,0), V3(0,0,radius), verts );
	CircleOutline( pos, V3(0,0,radius), V3(radius,0,0), verts );
	return *this;
}

BatchRenderer& BatchRenderer::CapsuleOutline( const Vec3& pos, float radius, const Vec3& nrm, float ht, int verts )
{
	ht /= 2;
	Vec3 N = nrm.Normalized();
	if( nrm.LengthSq() < SMALL_FLOAT )
		return SphereOutline( pos, radius, verts );
	
	Vec3 refdir = V3( 0, 0, nrm.z >= 0 ? 1 : -1 );
	Vec3 rotaxis = Vec3Cross( N, refdir );
	float rotangle = acosf( clamp( Vec3Dot( N, refdir ), -1, 1 ) );
	
	Mat4 rot = Mat4::CreateRotationAxisAngle( rotaxis.Normalized(), -rotangle );
	Vec3 T = rot.TransformNormal( V3(1,0,0) ).Normalized();
	Vec3 B = rot.TransformNormal( V3(0,1,0) ).Normalized();
	Vec3 Tr = T * radius, Br = B * radius, Nr = N * radius;
	
	Vec3 p0 = pos - N * ht, p1 = pos + N * ht;
	
	HalfCircleOutline( p1, Tr, Nr, verts / 2 );
	HalfCircleOutline( p1, Br, Nr, verts / 2 );
	HalfCircleOutline( p0, Tr, -Nr, verts / 2 );
	HalfCircleOutline( p0, Br, -Nr, verts / 2 );
	CircleOutline( p0, Tr, Br, verts );
	if( ht )
	{
		CircleOutline( p1, Tr, Br, verts );
		SetPrimitiveType( PT_Lines );
		Pos( p0 - Tr ); Pos( p1 - Tr );
		Pos( p0 + Tr ); Pos( p1 + Tr );
		Pos( p0 - Br ); Pos( p1 - Br );
		Pos( p0 + Br ); Pos( p1 + Br );
	}
	
	return *this;
}

BatchRenderer& BatchRenderer::ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, float angle, int verts )
{
	return ConeOutline( pos, dir, up, radius, V2(angle), verts );
}

BatchRenderer& BatchRenderer::ConeOutline( const Vec3& pos, const Vec3& dir, const Vec3& up, float radius, Vec2 angles, int verts )
{
	if( verts >= 3 )
	{
		Vec2 hra = DEG2RAD( angles ) / 2.0f;
		Vec3 dx = Vec3Cross(dir,up).Normalized();
		Vec3 dy = Vec3Cross(dir,dx).Normalized();
		dx *= radius * sinf( hra.x );
		dy *= radius * sinf( hra.y );
		
		SetPrimitiveType( PT_Lines );
		float a = 0;
		float ad = M_PI * 2.0f / verts;
		for( int i = 0; i < verts; ++i )
		{
			float ca = cosf( a ), sa = sinf( a );
			float aca = fabsf( ca ), asa = fabsf( sa );
			float distangle = ( aca * hra.x + asa * hra.y ) / ( aca + asa );
			float rc = radius * cosf( distangle );
			
			Pos( pos );
			Pos( pos + dir * rc + ca * dx + sa * dy );
			
			if( i > 0 )
			{
				Prev( 2 ); // previous line position
				Prev( 1 ); // current line position (-1 from last insertion)
			}
			
			a += ad;
		}
		
		Prev( 2 ); // previous line position
		Prev( verts * 2 + ( verts - 1 ) * 2 - 1 ); // 1st line position
	}
	return *this;
}

BatchRenderer& BatchRenderer::AABB( const Vec3& bbmin, const Vec3& bbmax, const Mat4& transform )
{
	Vec3 pp[8] =
	{
		transform.TransformPos( V3(bbmin.x,bbmin.y,bbmin.z) ), transform.TransformPos( V3(bbmax.x,bbmin.y,bbmin.z) ),
		transform.TransformPos( V3(bbmin.x,bbmax.y,bbmin.z) ), transform.TransformPos( V3(bbmax.x,bbmax.y,bbmin.z) ),
		transform.TransformPos( V3(bbmin.x,bbmin.y,bbmax.z) ), transform.TransformPos( V3(bbmax.x,bbmin.y,bbmax.z) ),
		transform.TransformPos( V3(bbmin.x,bbmax.y,bbmax.z) ), transform.TransformPos( V3(bbmax.x,bbmax.y,bbmax.z) ),
	};
	SetPrimitiveType( PT_Lines );
	// X
	Pos( pp[0] ); Pos( pp[1] ); Pos( pp[2] ); Pos( pp[3] ); Pos( pp[4] ); Pos( pp[5] ); Pos( pp[6] ); Pos( pp[7] );
	// Y
	Pos( pp[0] ); Pos( pp[2] ); Pos( pp[1] ); Pos( pp[3] ); Pos( pp[4] ); Pos( pp[6] ); Pos( pp[5] ); Pos( pp[7] );
	// Z
	Pos( pp[0] ); Pos( pp[4] ); Pos( pp[1] ); Pos( pp[5] ); Pos( pp[2] ); Pos( pp[6] ); Pos( pp[3] ); Pos( pp[7] );
	return *this;
}

BatchRenderer& BatchRenderer::Tick( const Vec3& pos, float radius, const Mat4& transform )
{
	SetPrimitiveType( PT_Lines );
	Pos( transform.TransformPos( pos + V3(-radius,0,0) ) ); Pos( transform.TransformPos( pos + V3(radius,0,0) ) );
	Pos( transform.TransformPos( pos + V3(0,-radius,0) ) ); Pos( transform.TransformPos( pos + V3(0,radius,0) ) );
	Pos( transform.TransformPos( pos + V3(0,0,-radius) ) ); Pos( transform.TransformPos( pos + V3(0,0,radius) ) );
	return *this;
}

static const float cos135deg = cosf( FLT_PI * 0.75f );
BatchRenderer& BatchRenderer::AAPoly( const Vec2* polydata, size_t polysize, float z )
{
	if( polysize < 3 )
		return *this;
	
	Vec2 center = V2(0);
	for( size_t i = 0; i < polysize; ++i )
		center += polydata[ i ];
	center /= polysize;
	
	uint32_t colorf = m_proto.color;
	uint32_t colora = colorf & 0x00ffffff;
	Vec2 prev_n;
	SetPrimitiveType( PT_Triangles );
	for( size_t i = 0; i <= polysize; ++i )
	{
		size_t i0 = ( i + polysize - 1 ) % polysize;
		size_t i1 = i % polysize;
		size_t i2 = ( i + 1 ) % polysize;
		Vec2 p0 = polydata[ i0 ];
		Vec2 p1 = polydata[ i1 ];
		Vec2 p2 = polydata[ i2 ];
		Vec2 d01 = ( p1 - p0 ).Normalized();
		Vec2 d12 = ( p2 - p1 ).Normalized();
		float dot = clamp( Vec2Dot( d01, d12 ), -1, 1 );
		Vec2 n;
		if( dot < cos135deg )
		{
			// TODO MAYBE FIX
			n = V2(0);
		}
		else
		{
			n = ( d12 + d01 ).Normalized().Perp2() / cosf( 0.5f * acosf( dot ) );
		}
		
		n *= 0.5f; // half in each direction for a full pixel in both
		if( i != 0 )
		{
			Vec2 curr_n = n;
			Vec2 i0in = p0 - prev_n, i0out = p0 + prev_n;
			Vec2 i1in = p1 - curr_n, i1out = p1 + curr_n;
			
			// 3 triangles: (center, in = opaque, out = transparent)
			// - center, i0in, i1in
			// - i0out, i1out, i0in(3)
			// - i1out(1), i1in(4), i0in(2)
			
			m_proto.color = colorf; Pos( center, z ); Pos( i0in, z ); Pos( i1in, z );
			m_proto.color = colora; Pos( i0out, z ); Pos( i1out, z ); Prev( 3 );
			Prev( 1 ); Prev( 4 ); Prev( 2 );
		}
		prev_n = n;
	}
	m_proto.color = colorf;
	return *this;
}

BatchRenderer& BatchRenderer::AARect( float x0, float y0, float x1, float y1, float z )
{
	Vec2 verts[] = { V2(x0,y0), V2(x1,y0), V2(x1,y1), V2(x0,y1) };
	AAPoly( verts, 4, z );
	return *this;
}

BatchRenderer& BatchRenderer::AACircle( float x, float y, float r, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		m_polyCache.clear();
		m_polyCache.resize( verts );
		float ad = M_PI * 2.0f / verts;
		float a = ad;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sinf( a ), cc = cosf( a );
			m_polyCache[ i ] = V2( x + cs * r, y + cc * r );
			a += ad;
		}
		AAPoly( m_polyCache.data(), m_polyCache.size(), z );
	}
	return *this;
}

BatchRenderer& BatchRenderer::AAStroke( const Vec2* linedata, size_t linesize, float width, bool closed, float z )
{
	if( linesize < 2 )
		return *this;
	
	float width0 = width - 1;
	float width1 = width + 1;
	uint32_t colorf = m_proto.color;
	uint32_t colora = colorf & 0x00ffffff;
	Vec2 prev_n;
	SetPrimitiveType( PT_Triangles );
	for( size_t i = 0; i < linesize + (closed?1:0); ++i )
	{
		size_t i0 = ( i + linesize - 1 ) % linesize;
		size_t i1 = i % linesize;
		size_t i2 = ( i + 1 ) % linesize;
		if( closed == false )
		{
			if( i1 == 0 ) i0 = i1;
			if( i1 == linesize - 1 ) i2 = i1;
		}
		Vec2 p0 = linedata[ i0 ];
		Vec2 p1 = linedata[ i1 ];
		Vec2 p2 = linedata[ i2 ];
		Vec2 d01 = ( p1 - p0 ).Normalized();
		Vec2 d12 = ( p2 - p1 ).Normalized();
		float dot = clamp( Vec2Dot( d01, d12 ), -1, 1 );
		Vec2 n;
		if( dot < cos135deg )
		{
			// TODO MAYBE FIX
			n = V2(0);
		}
		else
		{
			n = ( d12 + d01 ).Normalized().Perp2() / cosf( 0.5f * acosf( dot ) );
		}
		
		n *= 0.5f; // half in each direction for a full pixel in both
		if( i != 0 )
		{
			Vec2 curr_n = n;
			Vec2 pin0 = p0 + prev_n * width0, pout0 = p0 + prev_n * width1;
			Vec2 pin1 = p1 + curr_n * width0, pout1 = p1 + curr_n * width1;
			Vec2 nin0 = p0 - prev_n * width0, nout0 = p0 - prev_n * width1;
			Vec2 nin1 = p1 - curr_n * width0, nout1 = p1 - curr_n * width1;
			
			// 6 triangles: (in = opaque, out = transparent)
			// - pout0, pout1, pin1
			// - pin1(0), pin0, pout0(4)
			// - nout0, nout1, nin1
			// - nin1(0), nin0, nout0(4)
			// - pin0(7), pin1(9), nin1(4)
			// - nin1(0), nin0(5), pin0(4)
			
			m_proto.color = colora; Pos( pout0 ); Pos( pout1 );
			m_proto.color = colorf; Pos( pin1 ); Prev( 0 ); Pos( pin0 ); Prev( 4 );
			m_proto.color = colora; Pos( nout0 ); Pos( nout1 );
			m_proto.color = colorf; Pos( nin1 ); Prev( 0 ); Pos( nin0 ); Prev( 4 );
			Prev( 7 ); Prev( 9 ); Prev( 4 );
			Prev( 0 ); Prev( 5 ); Prev( 4 );
		}
		prev_n = n;
	}
	m_proto.color = colorf;
	return *this;
}

BatchRenderer& BatchRenderer::AACircleOutline( float x, float y, float r, float width, float z, int verts )
{
	if( verts < 0 )
		verts = r * M_PI * 2;
	if( verts >= 3 )
	{
		m_polyCache.clear();
		m_polyCache.resize( verts );
		float ad = M_PI * 2.0f / verts;
		float a = ad;
		for( int i = 0; i < verts; ++i )
		{
			float cs = sinf( a ), cc = cosf( a );
			m_polyCache[ i ] = V2( x + cs * r, y + cc * r );
			a += ad;
		}
		AAStroke( m_polyCache.data(), m_polyCache.size(), width, true, z );
	}
	return *this;
}

bool BatchRenderer::CheckSetTexture( int i, const TextureHandle& tex )
{
	ASSERT( i >= 0 && i < SGRX_MAX_TEXTURES );
	if( tex != m_nextState.textures[ i ] )
	{
		m_nextState.textures[ i ] = tex;
		_UpdateDiff();
		return true;
	}
	_UpdateDiff();
	return false;
}

BatchRenderer& BatchRenderer::SetVertexShader( const VertexShaderHandle& shd )
{
	m_nextState.vshader = shd ? shd : m_defVShader;
	_UpdateDiff();
	return *this;
}

BatchRenderer& BatchRenderer::SetPixelShader( const PixelShaderHandle& shd )
{
	m_nextState.pshader = shd ? shd : m_defPShader;
	_UpdateDiff();
	return *this;
}

inline bool _is_noncont_primtype( EPrimitiveType pt )
{
	return pt == PT_LineStrip || pt == PT_TriangleStrip;
}

BatchRenderer& BatchRenderer::SetPrimitiveType( EPrimitiveType pt )
{
	if( _is_noncont_primtype( pt ) || _is_noncont_primtype( m_nextState.primType ) )
		Flush();
	m_nextState.primType = pt;
	_UpdateDiff();
	return *this;
}

BatchRenderer& BatchRenderer::QuadsToTris()
{
	if( m_nextState.primType == PT_Triangles && m_verts.size() % 4 == 0 )
	{
		size_t oldsize = m_verts.size();
		size_t newsize = oldsize / 4 * 6;
		m_verts.resize( newsize );
		for( size_t i = oldsize, j = newsize; i > 0; )
		{
			i -= 4;
			j -= 6;
			Vertex vts[4] = { m_verts[ i ], m_verts[ i + 1 ], m_verts[ i + 2 ], m_verts[ i + 3 ] };
			m_verts[ j+0 ] = vts[0];
			m_verts[ j+1 ] = vts[1];
			m_verts[ j+2 ] = vts[2];
			m_verts[ j+3 ] = vts[2];
			m_verts[ j+4 ] = vts[4];
			m_verts[ j+5 ] = vts[0];
		}
	}
	else
		LOG_ERROR << __FUNCTION__ << " - incorrect vertex count: " << m_verts.size();
	return *this;
}

BatchRenderer& BatchRenderer::Flush()
{
	if( m_verts.size() )
	{
		RenderStateHandle rsh = GR_GetRenderState( RenderState );
		VtxInputMapHandle vimh = GR_GetVertexInputMapping( m_currState.vshader, m_vertexDecl );
		
		GR_PreserveResource( rsh );
		GR_PreserveResource( vimh );
		
		SGRX_ImmDrawData immdd =
		{
			m_verts.data(), m_verts.size(),
			m_currState.primType,
			m_vertexDecl,
			vimh,
			m_currState.vshader,
			m_currState.pshader,
			rsh,
			ShaderData.data(), ShaderData.size()
		};
		for( int i = 0; i < SGRX_MAX_TEXTURES; ++i )
			immdd.textures[ i ] = m_currState.textures[ i ];
		if( immdd.textures[ 0 ] == NULL )
			immdd.textures[ 0 ] = m_whiteTex;
		
		m_renderer->DrawImmediate( immdd );
		
		m_verts.clear();
	}
	return *this;
}

BatchRenderer& BatchRenderer::Reset()
{
	ShaderData.clear();
	for( size_t i = 0; i < SGRX_MAX_TEXTURES; ++i )
		CheckSetTexture( i, NULL );
	SetVertexShader( NULL );
	SetPixelShader( NULL );
	SetPrimitiveType( PT_None );
	m_proto.color = 0xffffffff;
	m_proto.u = 0;
	m_proto.v = 0;
	return *this;
}

void BatchRenderer::_UpdateDiff()
{
	m_diff = m_currState.IsDiff( m_nextState );
}

void BatchRenderer::_RecalcMatrices()
{
	invMatrix = Mat4::Identity;
	( worldMatrix * viewMatrix ).InvertTo( invMatrix );
}


void SGRX_LineSet::DrawLine( const Vec3& p1, const Vec3& p2, uint32_t col )
{
	Point pt1 = { p1, col };
	Point pt2 = { p2, col };
	m_lines.push_back( pt1 );
	m_lines.push_back( pt2 );
}

void SGRX_LineSet::Flush()
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().SetPrimitiveType( PT_Lines );
	for( size_t i = 0; i < m_lines.size(); ++i )
	{
		br.Colu( m_lines[ i ].color ).Pos( m_lines[ i ].pos );
	}
	br.Flush();
	m_lines.clear();
}


