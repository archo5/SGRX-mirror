

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

void IRenderer::SetRenderTargets( SGRX_IDepthStencilSurface* dss, const SGRX_RTClearInfo& info, SGRX_RTSpec rts[4] )
{
	SetRenderTargets( info, dss, rts );
}

void IRenderer::SortRenderItems( SGRX_Scene* scene )
{
	Game_FireEvent( EID_ScenePreRender, scene );
	
	_RS_Cull_Camera_Prepare( scene );
	m_stats.numVisMeshes = _RS_Cull_Camera_MeshList( scene );
	m_stats.numVisPLights = _RS_Cull_Camera_PointLightList( scene );
	m_stats.numVisSLights = _RS_Cull_Camera_SpotLightList( scene );
	
	// MESH INST/LIGHT RELATIONS & DrawItems
	_RS_Compile_MeshLists( scene, m_visible_meshes.data(), m_visible_meshes.size() );
	
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

void IRenderer::RenderShadows( SGRX_Scene* scene, SGRX_PassType passtype )
{
	for( size_t light_id = 0; light_id < scene->m_lights.size(); ++light_id )
	{
		SGRX_Light* L = scene->m_lights.item( light_id ).key;
		if( L->type != LIGHT_SPOT )
			continue;
		if( !L->enabled ||
			!L->hasShadows ||
			!L->shadowTexture ||
			!L->shadowTexture->m_isRenderTexture )
			continue;
		
		const TextureInfo& TI = L->shadowTexture.GetInfo();
		DepthStencilSurfHandle dssh = GR_GetDepthStencilSurface( TI.width, TI.height, (SGRX_TextureFormat) TI.format );
		GR_PreserveResource( dssh );
		
		SGRX_RTClearInfo info = { SGRX_RT_ClearAll, 0, 0, 1 };
		SGRX_RTSpec thlist[4] = { L->shadowTexture, SGRX_RTSpec(), SGRX_RTSpec(), SGRX_RTSpec() };
		SetRenderTargets( info, dssh, thlist );
		
		_RS_Cull_SpotLight_MeshList( scene, L );
		
		_RS_LoadInstItems( L->viewMatrix, 1, m_visible_spot_meshes.data(), m_visible_spot_meshes.size(), SGRX_TY_Solid );
		
		SGRX_Camera cam;
		L->GenerateCamera( cam );
		DoRenderItems( scene, passtype, 1, cam, m_renderItemsAux.data(), m_renderItemsAux.data() + m_renderItemsAux.size() );
	}
}

void IRenderer::RenderMeshes( SGRX_Scene* scene, SGRX_PassType passtype, int maxrepeat, uint8_t types, SGRX_MeshInstance** milist, size_t micount )
{
	_RS_LoadInstItems( scene->camera.mView, 1, milist, micount, SGRX_TY_Solid | SGRX_TY_Decal | SGRX_TY_Transparent );
	_RS_Compile_MeshLists( scene, milist, micount );
	DoRenderItems( scene, passtype, 1, scene->camera, m_renderItemsAux.data(), m_renderItemsAux.data() + m_renderItemsAux.size() );
}

void IRenderer::RenderTypes( SGRX_Scene* scene, SGRX_PassType passtype, int maxrepeat, uint8_t types )
{
	if( ( types & SGRX_TY_Solid ) != 0 && m_riBaseSD > m_riBaseStart )
	{
		DoRenderItems( scene, passtype, maxrepeat, scene->camera, m_riBaseStart, m_riBaseSD );
	}
	if( ( types & SGRX_TY_Decal ) != 0 && m_riBaseDT > m_riBaseSD )
	{
		DoRenderItems( scene, passtype, maxrepeat, scene->camera, m_riBaseSD, m_riBaseDT );
	}
	if( ( types & SGRX_TY_Transparent ) != 0 && m_riBaseEnd > m_riBaseDT )
	{
		DoRenderItems( scene, passtype, maxrepeat, scene->camera, m_riBaseDT, m_riBaseEnd );
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
	
	// generate render items from mesh instances and their parts
	RIA->clear();
	for( size_t miid = 0; miid < micount; ++miid )
	{
		SGRX_MeshInstance* MI = milist[ miid ];
		for( size_t part_id = 0; part_id < MI->m_drawItems.size(); ++part_id )
		{
			if( uint16_t(MI->m_drawItems.size()) <= part_id ||
				MI->GetMaterialCount() <= part_id ||
				( MI->m_drawItems[ part_id ].type & flags ) == 0 ||
				( MI->GetMaterial( (uint16_t) part_id ).flags & SGRX_MtlFlag_Disable ) )
				continue;
			
			RenderItem RI = { _RS_GenSortKey( view, MI, part_id ), MI, part_id };
			RIA->push_back( RI );
		}
	}
	
	// sort render items according to key (multiple influences)
	sgrx_combsort( RIA->data(), RIA->size(), sizeof(RenderItem), renderitem_sort, NULL );
}


void IRenderer::_RS_PreProcess( SGRX_Scene* scene )
{
	m_stats.Reset();
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


//
// LIGHT ENVIRONMENT
//

void SGRX_LightEnv::SetSamplePositions( Vec3* samples, size_t count, const Vec3& col )
{
	Colors defcol = { { col, col, col, col, col, col } };
	m_samples.resize( count );
	for( size_t i = 0; i < count; ++i )
	{
		m_samples[ i ].pos = samples[ i ];
		m_samples[ i ].cols = defcol;
	}
}

void SGRX_LightEnv::GetColors( Vec3 pos, Colors* out )
{
	if( m_samples.size() )
	{
		size_t closest = 0;
		float ndst = ( m_samples[ 0 ].pos - pos ).LengthSq();
		for( size_t i = 1; i < m_samples.size(); ++i )
		{
			float dst = ( m_samples[ i ].pos - pos ).LengthSq();
			if( dst < ndst )
			{
				ndst = dst;
				closest = i;
			}
		}
		*out = m_samples[ closest ].cols;
	}
	else
		out->Clear();
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

void SGRX_LightEnvSampler::SampleLight( const Vec3& pos, Vec3 outcolors[6] )
{
	SGRX_LightEnv::Colors cols;
	if( m_lightEnv )
	{
		m_lightEnv->GetColors( pos, &cols );
		for( int i = 0; i < 6; ++i )
			outcolors[ i ] = cols.color[ i ];
	}
	else
	{
		for( int i = 0; i < 6; ++i )
			outcolors[ i ] = V3(1);
	}
}



