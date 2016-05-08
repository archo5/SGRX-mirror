

#include "mapedit.hpp"



void LightmapF32ToRGBA( uint32_t* pxout, Vec3* pxin, int width, int height )
{
	size_t sz = width * height;
	
#if 1
	// Floyd-Steinberg dithering
	Array< Vec3 > dithered;
	dithered.resize( sz );
	for( size_t i = 0; i < sz; ++i )
		dithered[ i ] = Vec3::Min( pxin[ i ] * 0.5f, V3(1) ) * 255;
	for( int y = 0; y < height; ++y )
	{
		for( int x = 0; x < width; ++x )
		{
			Vec3& PX = dithered[ x + width * y ];
			Vec3 oldpx = PX;
			Vec3 newpx = V3( round( oldpx.x ), round( oldpx.y ), round( oldpx.z ) );
			PX = newpx;
			pxout[ x + width * y ] = COLOR_RGB( newpx.x, newpx.y, newpx.z );
			Vec3 quant_error = oldpx - newpx;
			
			if( x < width - 1 )
				dithered[ x + 1 + width * y ] += quant_error * (7.0f/16.0f);
			if( y < height - 1 )
			{
				dithered[ x + width * ( y + 1 ) ] += quant_error * (5.0f/16.0f);
				if( x > 0 )
					dithered[ x - 1 + width * ( y + 1 ) ] += quant_error * (3.0f/16.0f);
				if( x < width - 1 )
					dithered[ x + 1 + width * ( y + 1 ) ] += quant_error * (1.0f/16.0f);
			}
		}
	}
	pxin = dithered.data();
#else
	
	for( size_t i = 0; i < sz; ++i )
	{
		Vec3 incol = Vec3::Min( pxin[ i ] * 0.5f, V3(1) );
		pxout[ i ] = COLOR_RGB( incol.x * 255, incol.y * 255, incol.z * 255 );
	}
#endif
}

void LMNormalF32ToRGBA( uint32_t* pxout, Vec4* pxin, int width, int height )
{
	size_t sz = width * height;
	
	for( size_t i = 0; i < sz; ++i )
	{
		Vec4 indata = pxin[ i ];
		indata = V4( indata.ToVec3() * 0.5f + 0.5f, indata.w );
		pxout[ i ] = COLOR_RGBA(
			indata.x * 255,
			indata.y * 255,
			indata.z * 255,
			indata.w * 255
		);
	}
}



#define LGC_IS_VALID_ID( x ) ( (x) != 0 && (x) < uint32_t(0x80000000) )

void EdLevelGraphicsCont::Surface::RecalcTangents()
{
	Array<Vec3> tanstore;
	tanstore.resize_using( vertices.size() * 2, V3(0) );
	Vec3* tan1 = tanstore.data();
	Vec3* tan2 = tan1 + vertices.size();
	
	for( size_t trioff = 0; trioff + 2 < indices.size(); trioff += 3 )
	{
		uint16_t i1 = indices[ trioff ];
		uint16_t i2 = indices[ trioff + 1 ];
		uint16_t i3 = indices[ trioff + 2 ];
		
		const Vec3& v1 = vertices[ i1 ].pos;
		const Vec3& v2 = vertices[ i2 ].pos;
		const Vec3& v3 = vertices[ i3 ].pos;
		
		const Vec2& w1 = V2( vertices[ i1 ].tx0, vertices[ i1 ].ty0 );
		const Vec2& w2 = V2( vertices[ i2 ].tx0, vertices[ i2 ].ty0 );
		const Vec2& w3 = V2( vertices[ i3 ].tx0, vertices[ i3 ].ty0 );
		
		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;
		
		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
		
		float invR = s1 * t2 - s2 * t1;
		if( invR > 0 )
		{
			float r = 1.0f / invR;
			Vec3 sdir = V3((t2 * x1 - t1 * x2) * r,
				(t2 * y1 - t1 * y2) * r,
				(t2 * z1 - t1 * z2) * r);
			Vec3 tdir = V3((s1 * x2 - s2 * x1) * r,
				(s1 * y2 - s2 * y1) * r,
				(s1 * z2 - s2 * z1) * r);
			
			tan1[ i1 ] += sdir;
			tan1[ i2 ] += sdir;
			tan1[ i3 ] += sdir;
			
			tan2[ i1 ] += tdir;
			tan2[ i2 ] += tdir;
			tan2[ i3 ] += tdir;
		}
	}
	
	for( size_t i = 0; i < vertices.size(); ++i )
	{
		const Vec3& n = vertices[ i ].nrm;
		const Vec3& t = tan1[ i ];
		
		// Gram-Schmidt orthogonalize
		Vec3 outTng = ( t - n * Vec3Dot( n, t ) ).Normalized();
		
		// Calculate handedness
		float outDir = ( Vec3Dot( Vec3Cross( n, t ), tan2[ i ] ) < 0.0f ) ? -1.0f : 1.0f;
		
		vertices[ i ].tng = V4( outTng, outDir );
	}
}


void EdLevelGraphicsCont::LMap::ExportRGBA8( uint32_t* outcol, uint32_t* outnrm )
{
	size_t W = width;
	size_t H = height;
	if( W * H )
	{
		LightmapF32ToRGBA( outcol, lmdata.data(), W, H );
		if( nmdata.size() )
			LMNormalF32ToRGBA( outnrm, nmdata.data(), W, H );
		else
			TMEMSET( outnrm, W * H, COLOR_RGBA(127,127,127,0) );
	}
}

void EdLevelGraphicsCont::LMap::ReloadTex()
{
	if( lmdata.size() )
	{
		Array< uint32_t > convdata;
		convdata.resize( width * height * 2 );
		ExportRGBA8( convdata.data(), convdata.data() + ( width * height ) );
		
		texture = GR_CreateTexture( width, height, TEXFORMAT_RGBA8,
			TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, convdata.data() );
		
		nmtexture = GR_CreateTexture( width, height, TEXFORMAT_RGBA8,
			TEXFLAGS_LERP | TEXFLAGS_CLAMP_X | TEXFLAGS_CLAMP_Y, 1, &convdata[width*height] );
	}
	else
	{
		texture = GR_GetTexture( "textures/deflm.png" );
		nmtexture = GR_GetTexture( "textures/defnm.png" );
	}
}

EdLevelGraphicsCont::EdLevelGraphicsCont()
	: m_invalidSamples(false), m_alrInvalidSamples(false), m_lmRenderer(NULL)
{
	g_Level->m_lightTree = &m_sampleTree;
	
	Game_RegisterEventHandler( this, EID_GOResourceAdd );
	Game_RegisterEventHandler( this, EID_GOResourceRemove );
	Game_RegisterEventHandler( this, EID_GOResourceUpdate );
}

EdLevelGraphicsCont::~EdLevelGraphicsCont()
{
	Game_UnregisterEventHandler( this, EID_GOResourceAdd );
	Game_UnregisterEventHandler( this, EID_GOResourceRemove );
	Game_UnregisterEventHandler( this, EID_GOResourceUpdate );
	
	if( m_lmRenderer )
		delete m_lmRenderer;
}

void EdLevelGraphicsCont::Reset()
{
	m_solids.clear();
	m_meshes.clear();
	m_surfaces.clear();
	m_lights.clear();
	m_lightmaps.clear();
	m_invalidSamples = false;
	m_alrInvalidSamples = false;
	m_invalidLightmaps.clear();
	m_alrInvalidLightmaps.clear();
}

void EdLevelGraphicsCont::LoadLightmaps( const StringView& levname )
{
	char fname[ 256 ];
	sgrx_snprintf( fname, sizeof(fname), SGRX_LEVELS_DIR "%s" SGRX_LEVEL_LMCACHE_SFX, StackString<200>(levname).str );
	
	ByteArray ba;
	if( FS_LoadBinaryFile( fname, ba ) == false )
	{
		LOG_WARNING << "Could not load lightmap cache";
		return;
	}
	
	ByteReader br( ba );
	br.marker( "SGRXLMC3" );
	if( br.error )
	{
		LOG_WARNING << "LMCACHE: File format error";
		return;
	}
	
	br << m_invalidSamples;
	br << m_sampleTree;
	
	LMap LM;
	while( br.pos < ba.size() )
	{
		br.marker( "LM" );
		if( br.error )
		{
			LOG_WARNING << "LMCACHE: missing 'LM', load error at " << br.pos;
			break;
		}
		SGRX_GUID lmguid;
		br << lmguid;
		br << LM;
		if( m_lightmaps.isset( lmguid ) )
		{
			*m_lightmaps[ lmguid ] = LM;
			m_lightmaps[ lmguid ]->ReloadTex();
//			printf( "%d x %d\n", int(LM.width), int(LM.height) );
			ApplyLightmap( lmguid );
		}
	}
	
	m_invalidLightmaps.clear();
	for( size_t i = 0; i < m_lightmaps.size(); ++i )
	{
		if( m_lightmaps.item( i ).value->invalid )
		{
			SGRX_GUID lmguid = m_lightmaps.item( i ).key;
			m_invalidLightmaps.set( lmguid, NoValue() );
		}
	}
	
	RelightAllMeshes();
	
	// - cancel all edits
	m_movedMeshes.clear();
	m_movedSurfs.clear();
	m_movedLights.clear();
}

void EdLevelGraphicsCont::SaveLightmaps( const StringView& levname )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	bw.marker( "SGRXLMC3" );
	
	bw << m_invalidSamples;
	bw << m_sampleTree;
	
	for( size_t i = 0; i < m_lightmaps.size(); ++i )
	{
		bw.marker( "LM" );
		SGRX_GUID lmguid = m_lightmaps.item( i ).key;
		bw << lmguid;
		bw << *m_lightmaps.item( i ).value;
	}
	
	char path[ 256 ], fname[ 256 ];
	sgrx_snprintf( path, sizeof(path),
		SGRX_LEVELS_DIR "%s" SGRX_LEVEL_DIR_SFX,
		StackString<200>(levname).str );
	sgrx_snprintf( fname, sizeof(fname),
		SGRX_LEVELS_DIR "%s" SGRX_LEVEL_LMCACHE_SFX,
		StackString<200>(levname).str );
	
	FS_DirCreate( path );
	FS_SaveBinaryFile( fname, ba.data(), ba.size() );
}

void EdLevelGraphicsCont::DumpLightmapInfo()
{
	puts( "--- LIGHTMAP INFO ---" );
	for( size_t i = 0; i < m_lightmaps.size(); ++i )
	{
		SGRX_GUID lmguid = m_lightmaps.item( i ).key;
		
		char lmguid_str[37];
		lmguid.ToCharArray( lmguid_str );
		
		bool hasmesh = m_meshes.isset( lmguid );
		bool hassurf = m_surfaces.isset( lmguid );
		const char* type = hasmesh ? ( hassurf ? "mesh/surf (ERROR!)" : "mesh" )
			: ( hassurf ? "surf" : "LEAK (WARNING!)" );
		
		LMapHandle lmh = m_lightmaps.item( i ).value;
		printf( "LM guid=%s type=%s width=%d height=%d inv:%s|%s\n",
			lmguid_str, type, lmh->width, lmh->height,
			lmh->invalid ? "Y" : "n", lmh->alr_invalid ? "Y" : "n" );
		if( hasmesh )
		{
			Mesh* M = m_meshes.getptr( lmguid );
			if( M )
				printf( "- mesh path=%s\n", StackString<256>(M->info.path).str );
			else
				puts( "- MESH NOT FOUND -" );
		}
		if( hassurf )
		{
			Surface* S = m_surfaces.getptr( lmguid );
			if( S )
				printf( "- surf mtl=%s\n", StackString<256>(S->mtlname).str );
			else
				puts( "- SURFACE NOT FOUND -" );
		}
	}
	printf( "samples inv:%s|%s\n", m_invalidSamples ? "Y" : "n", m_alrInvalidSamples ? "Y" : "n" );
}

void EdLevelGraphicsCont::LightMesh( SGRX_MeshInstance* MI, SGRX_GUID lmguid )
{
	// static lighting
	ASSERT( m_lightmaps.isset( lmguid ) );
	MI->SetMITexture( 0, m_lightmaps[ lmguid ]->texture );
	// dynamic lighting
	if( m_sampleTree.m_pos.size() )
	{
		SGRX_LightTreeSampler lts;
		lts.m_lightTree = &m_sampleTree;
		Vec3 colors[6];
		lts.SampleLight( MI->matrix.TransformPos( V3(0) ), colors );
		for( int i = 0; i < 6; ++i )
			MI->constants[ i + 10 ] = V4( colors[ i ], 1.0f );
	}
	else
	{
		for( int i = 10; i < 16; ++i )
			MI->constants[ i ] = V4(0.15f);
	}
}

void EdLevelGraphicsCont::RelightAllMeshes()
{
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		LightMesh( m_meshes.item( i ).value.ent->m_meshInst, m_meshes.item( i ).key );
	}
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		LightMesh( m_surfaces.item( i ).value.meshInst, m_surfaces.item( i ).key );
	}
}

void EdLevelGraphicsCont::CreateLightmap( SGRX_GUID lmguid )
{
	LMap* LM = new LMap;
	LM->width = 0;
	LM->height = 0;
	LM->texture = GR_GetTexture( "textures/deflm.png" );
	LM->nmtexture = GR_GetTexture( "textures/defnm.png" );
	LM->invalid = true;
	m_lightmaps[ lmguid ] = LM;
	m_invalidLightmaps[ lmguid ] = NoValue();
}

void EdLevelGraphicsCont::ClearLightmap( SGRX_GUID lmguid )
{
	LMap* LM = m_lightmaps[ lmguid ];
	LM->width = 0;
	LM->height = 0;
	LM->lmdata.clear();
	LM->texture = GR_GetTexture( "textures/deflm.png" );
	LM->nmtexture = GR_GetTexture( "textures/defnm.png" );
	ApplyLightmap( lmguid );
}

void EdLevelGraphicsCont::ApplyLightmap( SGRX_GUID lmguid )
{
	if( m_meshes.isset( lmguid ) )
	{
		m_meshes[ lmguid ].ent->m_meshInst->SetMITexture( 0, m_lightmaps[ lmguid ]->texture );
		m_meshes[ lmguid ].ent->m_meshInst->SetMITexture( 1, m_lightmaps[ lmguid ]->nmtexture );
	}
	if( m_surfaces.isset( lmguid ) )
	{
		m_surfaces[ lmguid ].meshInst->SetMITexture( 0, m_lightmaps[ lmguid ]->texture );
		m_surfaces[ lmguid ].meshInst->SetMITexture( 1, m_lightmaps[ lmguid ]->nmtexture );
	}
}

void EdLevelGraphicsCont::InvalidateLightmap( SGRX_GUID lmguid )
{
//	printf( "invalidated %u\n", unsigned(lmguid) );
	if( m_lmRenderer )
	{
		m_lightmaps[ lmguid ]->alr_invalid = true;
		m_alrInvalidLightmaps[ lmguid ] = NoValue();
	}
	
	m_lightmaps[ lmguid ]->invalid = true;
	m_invalidLightmaps[ lmguid ] = NoValue();
}

void EdLevelGraphicsCont::ValidateLightmap( SGRX_GUID lmguid )
{
	m_lightmaps[ lmguid ]->invalid = m_lightmaps[ lmguid ]->alr_invalid;
	if( m_alrInvalidLightmaps.isset( lmguid ) )
		m_invalidLightmaps.set( lmguid, NoValue() );
	else
		m_invalidLightmaps.unset( lmguid );
}

void EdLevelGraphicsCont::ApplyInvalidation()
{
	// convert invalidated meshes/surfaces into lights
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		SGRX_GUID guid = m_lights.item( i ).key;
		if( m_movedLights.isset( guid ) )
			continue;
		Light& L = m_lights.item( i ).value;
		
		// - check if any of invalidated meshes hit the light
		for( size_t j = 0; j < m_movedMeshes.size(); ++j )
		{
			Mesh* pM = m_meshes.getptr( m_movedMeshes.item( j ).key );
			if( pM == NULL )
				continue;
			SGRX_IMesh* XM = pM->ent->m_mesh;
			if( XM == NULL )
				continue;
			PrevMeshData& pmd = m_movedMeshes.item( j ).value;
			if( L.info.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, pM->ent->m_meshInst->matrix ) ||
				L.info.IntersectsAABB( pmd.bbmin, pmd.bbmax, pmd.transform ) )
			{
				m_movedLights.set( guid, L.info );
				goto lightdone;
			}
		}
		
		// - check if any of invalidated surfaces hit the light
		for( size_t j = 0; j < m_movedSurfs.size(); ++j )
		{
			Surface* pS = m_surfaces.getptr( m_movedSurfs.item( j ).key );
			if( pS == NULL )
				continue;
			Surface& S = *pS;
			SGRX_IMesh* XM = S.meshInst->GetMesh();
			if( XM == NULL )
				continue;
			PrevMeshData& pmd = m_movedSurfs.item( j ).value;
			if( L.info.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, S.meshInst->matrix ) ||
				L.info.IntersectsAABB( pmd.bbmin, pmd.bbmax, pmd.transform ) )
			{
				m_movedLights.set( guid, L.info );
				goto lightdone;
			}
		}
lightdone:;
	}
	
	// invalidate mesh lightmaps based on lights
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		Mesh& M = m_meshes.item( i ).value;
		SGRX_IMesh* XM = M.ent->m_meshInst->GetMesh();
		if( XM == NULL )
			continue;
		for( size_t j = 0; j < m_movedLights.size(); ++j )
		{
			SGRX_GUID guid = m_movedLights.item( j ).key;
			Light* pL = m_lights.getptr( guid );
			if( pL == NULL )
				continue;
			EdLGCLightInfo& PrevL = m_movedLights.item( j ).value;
			if( pL->info.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, M.ent->m_meshInst->matrix ) ||
				PrevL.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, M.ent->m_meshInst->matrix ) )
			{
				InvalidateLightmap( m_meshes.item( i ).key );
				break;
			}
		}
	}
	
	// invalidate surface lightmaps based on lights
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		SGRX_IMesh* XM = S.meshInst->GetMesh();
		if( XM == NULL )
			continue;
		for( size_t j = 0; j < m_movedLights.size(); ++j )
		{
			SGRX_GUID guid = m_movedLights.item( j ).key;
			Light* pL = m_lights.getptr( guid );
			if( pL == NULL )
				continue;
			EdLGCLightInfo& PrevL = m_movedLights.item( j ).value;
			if( pL->info.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, S.meshInst->matrix ) ||
				PrevL.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, S.meshInst->matrix ) )
			{
				InvalidateLightmap( m_surfaces.item( i ).key );
				break;
			}
		}
	}
	
	// clean up
	m_movedMeshes.clear();
	m_movedSurfs.clear();
	m_movedLights.clear();
}

void EdLevelGraphicsCont::InvalidateMesh( SGRX_GUID guid )
{
	if( m_movedMeshes.isset( guid ) )
		return;
	Mesh* pM = m_meshes.getptr( guid );
	if( pM == NULL )
		return;
	MeshHandle XM = GR_GetMesh( pM->info.path );
	if( XM )
	{
		PrevMeshData pmd = { XM->m_boundsMin, XM->m_boundsMax, pM->info.xform };
		m_movedMeshes.set( guid, pmd );
	}
}

void EdLevelGraphicsCont::InvalidateSurface( SGRX_GUID guid )
{
	if( m_movedSurfs.isset( guid ) )
		return;
	Surface* pS = m_surfaces.getptr( guid );
	if( pS == NULL )
		return;
	SGRX_IMesh* XM = pS->meshInst->GetMesh();
	if( XM )
	{
		PrevMeshData pmd = { XM->m_boundsMin, XM->m_boundsMax, pS->meshInst->matrix };
		m_movedSurfs.set( guid, pmd );
	}
}

void EdLevelGraphicsCont::InvalidateLight( SGRX_GUID guid )
{
	if( m_movedLights.isset( guid ) )
		return;
	Light* pL = m_lights.getptr( guid );
	if( pL == NULL )
		return;
	m_movedLights.set( guid, pL->info );
}

void EdLevelGraphicsCont::InvalidateSamples()
{
	if( m_lmRenderer )
	{
		m_alrInvalidSamples = true;
	}
	m_invalidSamples = true;
}

void EdLevelGraphicsCont::InvalidateAll()
{
	InvalidateSamples();
	for( size_t i = 0; i < m_lightmaps.size(); ++i )
		InvalidateLightmap( m_lightmaps.item( i ).key );
}

bool EdLevelGraphicsCont::IsInvalidated( SGRX_GUID lmguid )
{
	return m_lightmaps.getcopy( lmguid )->invalid;
}

static bool MtlNeedsLM( const StringView& name )
{
	return name != "" && name != "null" && name != "clip" && name != "black";
}

static bool MtlIsSolid( const StringView& name )
{
	if( name == "black" ) return true;
	if( MtlNeedsLM( name ) == false ) return false;
	return true;
}

static bool RenderInfoNeedsLM( const EdLGCRenderInfo& rinfo )
{
	return ( rinfo.rflags & (LM_MESHINST_DYNLIT|LM_MESHINST_UNLIT) ) == 0;
}

static bool RenderInfoIsSolid( const EdLGCRenderInfo& rinfo )
{
	return ( rinfo.rflags & (LM_MESHINST_DECAL|LM_MESHINST_TRANSPARENT) ) == 0 &&
		( rinfo.rflags & LM_MESHINST_CASTLMS ) != 0;
}

bool EdLevelGraphicsCont::ILMBeginRender()
{
	if( m_lmRenderer )
		return false;
	if( GetInvalidItemCount() == 0 )
		return false;
	
	m_alrInvalidLightmaps.clear();
	m_alrInvalidSamples = false;
	
	m_lmRenderer = new LMRenderer;
	
	LMRenderer::Config& cfg = m_lmRenderer->config;
	cfg.ambientColor = HSV( g_EdWorld->m_lighting.ambientColor );
	cfg.lightmapClearColor = HSV( g_EdWorld->m_lighting.lightmapClearColor );
	cfg.lightmapDetail = g_EdWorld->m_lighting.lightmapDetail;
	cfg.lightmapBlurSize = g_EdWorld->m_lighting.lightmapBlurSize;
	cfg.aoDistance = g_EdWorld->m_lighting.aoDist;
	cfg.aoMultiplier = g_EdWorld->m_lighting.aoMult;
	cfg.aoFalloff = g_EdWorld->m_lighting.aoFalloff;
	cfg.aoEffect = g_EdWorld->m_lighting.aoEffect;
	cfg.aoColor = HSV( g_EdWorld->m_lighting.aoColor );
	cfg.aoNumSamples = g_EdWorld->m_lighting.aoNumSamples;
	
	if( m_invalidSamples )
	{
		for( size_t i = 0; i < m_sampleTree.m_pos.size(); ++i )
		{
			m_lmRenderer->sample_positions.push_back( m_sampleTree.m_pos[ i ] );
		}
	}
	
	if( g_EdWorld->m_lighting.dirLightColor.z > 0 )
	{
		m_lmRenderer->AddLight( g_EdWorld->GetDirLightInfo() );
	}
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		Mesh& M = m_meshes.item( i ).value;
		SGRX_GUID lmguid = m_meshes.item( i ).key;
		m_lightmaps[ lmguid ]->alr_invalid = false;
		bool solid = RenderInfoIsSolid( M.info );
		bool needslm = RenderInfoNeedsLM( M.info );
		if( needslm && IsInvalidated( lmguid ) )
		{
			m_lmRenderer->AddMeshInst( M.ent->m_meshInst, V2(32 * M.info.lmdetail), lmguid, solid );
		}
		else
		{
			if( needslm == false )
				ClearLightmap( lmguid );
			ValidateLightmap( lmguid );
			m_lmRenderer->AddMeshInst( M.ent->m_meshInst, V2(0), SGRX_GUID::Null, solid );
		}
	}
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		SGRX_GUID lmguid = m_surfaces.item( i ).key;
		m_lightmaps[ lmguid ]->alr_invalid = false;
		bool solid = MtlIsSolid( S.mtlname ) && RenderInfoIsSolid( S.info );
		bool needslm = RenderInfoNeedsLM( S.info ) && MtlNeedsLM( S.mtlname );
		if( needslm && IsInvalidated( lmguid ) )
		{
			m_lmRenderer->AddMeshInst( S.meshInst, S.lmsize * S.info.lmdetail, lmguid, solid );
		}
		else
		{
			if( needslm == false )
				ClearLightmap( lmguid );
			ValidateLightmap( lmguid );
			if( solid )
				m_lmRenderer->AddMeshInst( S.meshInst, V2(0), SGRX_GUID::Null, true );
		}
	}
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		Light& L = m_lights.item( i ).value;
		if( L.ent->m_isStatic )
			m_lmRenderer->AddLight( L.info );
	}
	m_lmRenderer->Start();
	
	return true;
}

void EdLevelGraphicsCont::ILMAbort()
{
	if( m_lmRenderer )
	{
		delete m_lmRenderer;
		m_lmRenderer = NULL;
	}
}

void EdLevelGraphicsCont::ILMCheck()
{
	if( m_lmRenderer == NULL )
		return;
	
	if( m_lmRenderer->CheckStatus() )
	{
		for( uint32_t i = 0; i < m_lmRenderer->rendered_lightmap_count; ++i )
		{
			Array< Vec3 > colors;
			Array< Vec4 > normals;
			LMRenderer::OutLMInfo lminfo;
			if( m_lmRenderer->GetLightmap( i, colors, normals, lminfo ) &&
				m_lightmaps.isset( lminfo.guid ) )
			{
				LMap& L = *m_lightmaps[ lminfo.guid ];
				L.width = lminfo.w;
				L.height = lminfo.h;
				L.lmdata = colors;
				L.nmdata = normals;
				L.ReloadTex();
				ApplyLightmap( lminfo.guid );
				ValidateLightmap( lminfo.guid );
			}
		}
		
		if( m_lmRenderer->rendered_sample_count == m_sampleTree.m_colors.size() )
		{
			for( size_t i = 0; i < m_sampleTree.m_colors.size(); ++i )
			{
				m_lmRenderer->GetSample( i, m_sampleTree.m_colors[ i ].color );
			}
			if( m_sampleTree.m_colors.size() )
				RelightAllMeshes();
			m_invalidSamples = m_alrInvalidSamples;
		}
		
		delete m_lmRenderer;
		m_lmRenderer = NULL;
	}
}

void EdLevelGraphicsCont::STRegenerate()
{
	if( m_lmRenderer )
		return;
	
	float density = g_EdWorld->m_lighting.sampleDensity;
	float stepsize = density ? 1.0f / density : 1.0f;
	
	Vec3 bbmin = V3( FLT_MAX ), bbmax = V3( -FLT_MAX );
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		Mesh& M = m_meshes.item( i ).value;
		if( M.ent->m_isStatic == false )
			continue;
		
		SGRX_IMesh* mesh = M.ent->m_meshInst->GetMesh();
		if( mesh == NULL )
			continue;
		
		Vec3 tfbbmin = mesh->m_boundsMin;
		Vec3 tfbbmax = mesh->m_boundsMax;
		TransformAABB( tfbbmin, tfbbmax, M.ent->m_meshInst->matrix );
		bbmin = Vec3::Min( bbmin, tfbbmin );
		bbmax = Vec3::Max( bbmax, tfbbmax );
	}
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		SGRX_IMesh* mesh = S.meshInst->GetMesh();
		if( mesh == NULL )
			continue;
		
		Vec3 tfbbmin = mesh->m_boundsMin;
		Vec3 tfbbmax = mesh->m_boundsMax;
		TransformAABB( tfbbmin, tfbbmax, S.meshInst->matrix );
		bbmin = Vec3::Min( bbmin, tfbbmin );
		bbmax = Vec3::Max( bbmax, tfbbmax );
	}
	
	if( bbmin.x > bbmax.x )
		return; // no data to generate samples for!
	
	LOG << "Generating samples for " << bbmin << " - " << bbmax << " area with step size: " << stepsize;
	
	VoxelBlock VB( bbmin, bbmax, stepsize );
	LOG << "- voxel count: " << ( VB.m_xsize * VB.m_ysize * VB.m_zsize );
	LOG << "- rasterizing blocks...";
	// rasterize surfaces
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		for( size_t j = 0; j + 2 < S.indices.size(); j += 3 )
		{
			VB.RasterizeTriangle(
				S.vertices[ S.indices[ j ] ].pos,
				S.vertices[ S.indices[ j + 1 ] ].pos,
				S.vertices[ S.indices[ j + 2 ] ].pos
			);
		}
	}
	LOG << "- rasterizing solids...";
	// rasterize solids
	for( size_t i = 0; i < m_solids.size(); ++i )
	{
		Solid& S = m_solids.item( i ).value;
		if( S.planes.size() )
			VB.RasterizeSolid( S.planes.data(), S.planes.size() );
	}
	LOG << "- rasterizing meshes...";
	// rasterize meshes
	// TODO
	
	LOG << "- generating samples...";
	Array< Vec3 > samples;
	size_t osc = samples.size();
	// generate samples (1 on every side, 0.125 per voxel otherwise - 1 in each 2x2 block)
	for( int32_t z = 0; z < VB.m_zsize; ++z )
	{
		for( int32_t y = 0; y < VB.m_ysize; ++y )
		{
			for( int32_t x = 0; x < VB.m_xsize; ++x )
			{
				if( VB.Get( x, y, z ) )
					continue; // cannot put samples into geometry
				
				bool putsample = ( x % 2 == 0 ) && ( y % 2 == 0 ) && ( z % 2 == 0 );
				if( !putsample )
				{
					// check for nearby blocks
					if( VB.Get( x - 1, y, z ) || VB.Get( x + 1, y, z ) ||
						VB.Get( x, y - 1, z ) || VB.Get( x, y + 1, z ) ||
						VB.Get( x, y, z - 1 ) || VB.Get( x, y, z + 1 ) )
						putsample = true;
				}
				if( putsample )
					samples.push_back( VB.GetPosition( x, y, z ) );
			}
		}
	}
	LOG << "- done, generated sample count: " << ( samples.size() - osc );
	
	m_sampleTree.SetSamplesUncolored( samples.data(), samples.size() );
	m_invalidSamples = true;
}


void EdLevelGraphicsCont::ExportLightmap( SGRX_GUID lmguid, LC_Lightmap& outlm )
{
	LMap& LM = *m_lightmaps[ lmguid ];
	outlm.width = LM.width;
	outlm.height = LM.height;
	outlm.data.resize( LM.width * LM.height );
	outlm.nmdata.resize( LM.width * LM.height );
	LM.ExportRGBA8( outlm.data.data(), outlm.nmdata.data() );
}

void EdLevelGraphicsCont::UpdateCache( LevelCache& LC )
{
	HashTable< SGRX_GUID, size_t > solids_guid2id;
	for( size_t i = 0; i < m_solids.size(); ++i )
	{
		Solid& S = m_solids.item( i ).value;
		size_t id = LC.AddSolid( S.planes.data(), S.planes.size() );
		solids_guid2id.set( m_solids.item( i ).key, id );
	}
	
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		if( S.mtlname == SV("") || S.mtlname == SV("null") )
			continue;
		
		LC_Lightmap lm;
		ExportLightmap( m_surfaces.item( i ).key, lm );
		
		Array< LCVertex > verts;
		verts.reserve( S.indices.size() );
		for( size_t v = 0; v < S.indices.size(); ++v )
			verts.push_back( S.vertices[ S.indices[ v ] ] );
		
		size_t solid = solids_guid2id.getcopy( S.solid_guid, NOT_FOUND );
		int decalLayer = ( S.info.rflags & LM_MESHINST_DECAL ) != 0 ? S.info.decalLayer : -1;
		LC.AddPart( verts.data(), verts.size(), lm, S.mtlname, solid, S.info.rflags, decalLayer );
	}
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		Mesh& M = m_meshes.item( i ).value;
		if( M.ent->m_isStatic == false )//|| M.ent->m_isVisible == false )
			continue;
		
		LC_GOLightmap lm;
		lm.rsrc_guid = m_meshes.item( i ).key;
		ExportLightmap( m_meshes.item( i ).key, lm.lmap );
		
		LC.m_gobj.lightmaps.push_back( lm );
		
	//	LC.AddMeshInst( M.info.path, M.ent->m_meshInst->matrix, M.info.rflags, M.info.decalLayer, lm );
	}
	
	LC.AddLight( g_EdWorld->GetDirLightInfo() );
	
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		if( m_lights.item( i ).value.ent->m_isStatic )
			LC.AddLight( m_lights.item( i ).value.info );
	}
}


void EdLevelGraphicsCont::RequestSolid( SGRX_GUID guid, EdLGCSolidInfo* info )
{
	ASSERT( m_solids.isset( guid ) == false );
	Solid S;
	m_solids.set( guid, S );
	UpdateSolid( guid, info );
}

void EdLevelGraphicsCont::DeleteSolid( SGRX_GUID guid )
{
	ASSERT( m_solids.isset( guid ) );
	m_solids.unset( guid );
}

static EdLGCSolidInfo g_defSolidInfo;
void EdLevelGraphicsCont::UpdateSolid( SGRX_GUID guid, EdLGCSolidInfo* info )
{
	if( info == NULL )
		info = &g_defSolidInfo;
	ASSERT( m_solids.isset( guid ) );
	Solid& S = m_solids[ guid ];
	S.planes.assign( info->planes, info->pcount );
}

void EdLevelGraphicsCont::RequestSurface( SGRX_GUID guid, EdLGCSurfaceInfo* info )
{
	ASSERT( m_surfaces.isset( guid ) == false );
	Surface S;
	S.lmsize = V2(0);
	S.meshInst = g_EdScene->CreateMeshInstance();
	S.meshInst->SetMesh( GR_CreateMesh() );
	SGRX_Material mtl;
	mtl.shader = "default";
	S.meshInst->materials.assign( &mtl, 1 );
	m_surfaces.set( guid, S );
	CreateLightmap( guid );
	UpdateSurface( guid, LGC_CHANGE_ALL, info );
}

void EdLevelGraphicsCont::DeleteSurface( SGRX_GUID guid )
{
	ASSERT( m_surfaces.isset( guid ) );
	m_lightmaps.unset( guid );
	m_surfaces.unset( guid );
}

static EdLGCSurfaceInfo g_defSurfInfo;
void EdLevelGraphicsCont::UpdateSurface( SGRX_GUID guid, uint32_t changes, EdLGCSurfaceInfo* info )
{
	if( info == NULL )
		info = &g_defSurfInfo;
	ASSERT( m_surfaces.isset( guid ) );
	Surface& S = m_surfaces[ guid ];
	
	bool surfedit = false;
	if( changes & LGC_SURF_CHANGE_VIDATA )
	{
		surfedit = S.vertices.size() != info->vcount ||
			S.indices.size() != info->icount ||
			memcmp( S.vertices.data(), info->vdata, S.vertices.size_bytes() ) != 0 ||
			memcmp( S.indices.data(), info->idata, S.indices.size_bytes() ) != 0;
	}
	bool edited =
		( ( changes & LGC_SURF_CHANGE_VIDATA ) && surfedit ) ||
		( ( changes & LGC_SURF_CHANGE_MTLDATA ) && S.mtlname != info->mtlname ) ||
		( ( changes & LGC_CHANGE_XFORM ) && S.meshInst->matrix != info->xform ) ||
		( ( changes & LGC_CHANGE_RSPEC ) && S.info.RIDiff( *info ) == 2 );
	if( edited )
		InvalidateSurface( guid );
	
	if( changes & LGC_SURF_CHANGE_VIDATA )
	{
		if( surfedit )
		{
			S.vertices.assign( info->vdata, info->vcount );
			S.indices.assign( info->idata, info->icount );
			S.RecalcTangents();
			SGRX_IMesh* mesh = S.meshInst->GetMesh();
			mesh->m_vdata.assign( S.vertices.data(), S.vertices.size_bytes() );
			mesh->m_idata.assign( S.indices.data(), S.indices.size_bytes() );
			
			VertexDeclHandle vd = GR_GetVertexDecl( LCVertex_DECL );
			if( S.vertices.size() )
			{
				mesh->SetVertexData( S.vertices.data(), S.vertices.size_bytes(), vd );
				mesh->SetAABBFromVertexData( S.vertices.data(), S.vertices.size_bytes(), vd );
			}
			if( S.indices.size() )
				mesh->SetIndexData( S.indices.data(), S.indices.size_bytes(), false );
			SGRX_MeshPart mp = { 0, S.vertices.size(), 0, S.indices.size() };
			mesh->SetPartData( &mp, 1 );
		}
		
		if( S.lmsize != info->lmsize )
		{
			InvalidateLightmap( guid );
			S.lmsize = info->lmsize;
			InvalidateLightmap( guid );
		}
	}
	if( changes & LGC_SURF_CHANGE_MTLDATA )
	{
		if( S.mtlname != info->mtlname )
		{
			S.mtlname = info->mtlname;
			
			if( S.mtlname.size() )
			{
				SGRX_Material& M = S.meshInst->GetMaterial( 0 );
				MapMaterial* mtl = g_NUISurfMtlPicker->m_materials.getcopy( S.mtlname );
				if( mtl )
				{
					M.shader = mtl->shader;
					for( int i = 0; i < MAX_MATERIAL_TEXTURES; ++i )
					{
						M.textures[ i ] = mtl->texture[ i ].size() ? GR_GetTexture( mtl->texture[ i ] ) : NULL;
					}
					M.blendMode = mtl->blendmode;
					M.flags = mtl->flags;
				}
				else
				{
					char bfr[ 256 ];
					sgrx_snprintf( bfr, sizeof(bfr), "textures/%s.png", StackString<200>(S.mtlname).str );
					M.shader = "default";
					M.textures[ 0 ] = GR_GetTexture( bfr );
					M.blendMode = SGRX_MtlBlend_None;
					M.flags = 0;
				}
				S.meshInst->OnUpdate();
			}
		}
	}
	if( changes & LGC_SURF_CHANGE_SOLID )
	{
		S.solid_guid = info->solid_guid;
	}
	if( changes & LGC_SURF_CHANGE_LMPARENT )
	{
		S.lmparent_guid = info->lmparent_guid;
		InvalidateLightmap( guid );
	}
	if( changes & LGC_CHANGE_XFORM )
	{
		if( S.meshInst->matrix != info->xform )
		{
			S.meshInst->matrix = info->xform;
		}
	}
	if( changes & LGC_CHANGE_RSPEC )
	{
		int diff = S.info.RIDiff( *info );
		if( diff != 0 )
		{
			if( diff == 1 )
				InvalidateLightmap( guid ); // necessary?
			
			S.info = *info;
			
			if( diff == 1 )
				InvalidateLightmap( guid );
		}
		SGRX_LightingMode lmode = SGRX_LM_Static;
		if( info->rflags & LM_MESHINST_DYNLIT )
			lmode = SGRX_LM_Dynamic;
		if( info->rflags & LM_MESHINST_UNLIT )
			lmode = SGRX_LM_Unlit;
		S.meshInst->SetLightingMode( lmode );
		SGRX_Material& mtl = S.meshInst->GetMaterial( 0 );
		
		mtl.blendMode = SGRX_MtlBlend_None;
		mtl.flags = 0;
		MapMaterial* mapmtl = g_NUISurfMtlPicker->m_materials.getcopy( S.mtlname );
		if( mapmtl )
		{
			mtl.blendMode = mapmtl->blendmode;
			mtl.flags = mapmtl->flags;
		}
		
		if( info->rflags & LM_MESHINST_VCOL )
			mtl.flags |= SGRX_MtlFlag_VCol;
		if( info->rflags & LM_MESHINST_TRANSPARENT )
			mtl.blendMode = SGRX_MtlBlend_Basic;
		if( info->rflags & LM_MESHINST_DECAL )
			mtl.flags |= SGRX_MtlFlag_Decal;
		
		S.meshInst->OnUpdate();
		LightMesh( S.meshInst, guid );
	}
	S.meshInst->enabled = S.mtlname.size() != 0 && S.vertices.size() != 0 && S.indices.size() != 0;
	
	if( edited )
		InvalidateSurface( guid );
}

static void ReadLightInfo( EdLGCLightInfo* out, LightResource* lr )
{
	out->type = lr->m_type == LIGHT_SPOT ? LM_LIGHT_SPOT : LM_LIGHT_POINT;
	if( lr->m_isStatic == false || lr->m_isEnabled == false )
		out->type = -1;
	Mat4 mtx = lr->GetWorldMatrix();
	out->pos = mtx.GetTranslation();
	out->dir = mtx.TransformNormal( V3(0,0,-1) ).Normalized();
	out->up = mtx.TransformNormal( V3(0,-1,0) ).Normalized();
	out->range = lr->m_range;
	out->power = lr->m_power;
	out->light_radius = lr->m_lightRadius;
	out->color = lr->m_color * lr->m_intensity;
	out->num_shadow_samples = 0;
	out->flaresize = 0; // lr->m_flareSize;
	out->flareoffset = V3(0); // lr->m_flareOffset;
	out->innerangle = lr->m_innerAngle;
	out->outerangle = lr->m_angle;
	out->spotcurve = lr->m_spotCurve;
}

static void ReadMeshInfo( EdLGCMeshInfo* out, MeshResource* mr )
{
	out->path = mr->m_mesh ? mr->m_mesh->m_key : "";
	out->xform = mr->m_meshInst->matrix;
	out->rflags = 0
//		| ( mr->m_isSolid ? LM_MESHINST_SOLID : 0 )
		| ( !mr->m_isStatic || mr->m_lightingMode == SGRX_LM_Dynamic ? LM_MESHINST_DYNLIT : 0 )
		| ( mr->m_lightingMode == SGRX_LM_Unlit ? LM_MESHINST_UNLIT : 0 )
		| ( mr->m_castLMS ? LM_MESHINST_CASTLMS : 0 )
		| ( mr->m_lightingMode == SGRX_LM_Decal ? LM_MESHINST_DECAL : 0 );
	out->lmdetail = mr->m_lmQuality;
	out->decalLayer = 0;
}

void EdLevelGraphicsCont::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_GOResourceAdd )
	{
		SGRX_CAST( GOResource*, rsrc, edata.GetUserData() );
		if( rsrc->m_src_guid.IsNull() )
			rsrc->m_src_guid.SetGenerated();
		
		if( ENTITY_IS_A( rsrc, MeshResource ) )
		{
			SGRX_CAST( MeshResource*, MR, rsrc );
			Mesh M;
			{
				ReadMeshInfo( &M.info, MR );
				M.ent = MR;
			}
			m_meshes.set( MR->m_src_guid, M );
			CreateLightmap( MR->m_src_guid );
		}
		else if( ENTITY_IS_A( rsrc, LightResource ) )
		{
			SGRX_CAST( LightResource*, LR, rsrc );
			Light L;
			{
				ReadLightInfo( &L.info, LR );
				L.ent = LR;
			}
			m_lights.set( LR->m_src_guid, L );
		}
	}
	else if( eid == EID_GOResourceRemove )
	{
		SGRX_CAST( GOResource*, rsrc, edata.GetUserData() );
		if( ENTITY_IS_A( rsrc, MeshResource ) )
		{
			SGRX_CAST( MeshResource*, MR, rsrc );
			InvalidateMesh( MR->m_src_guid );
			ApplyInvalidation();
			m_movedMeshes.unset( MR->m_src_guid );
			m_lightmaps.unset( MR->m_src_guid );
			m_meshes.unset( MR->m_src_guid );
		}
		else if( ENTITY_IS_A( rsrc, LightResource ) )
		{
			SGRX_CAST( LightResource*, LR, rsrc );
			InvalidateLight( LR->m_src_guid );
			ApplyInvalidation();
			m_movedLights.unset( LR->m_src_guid );
			m_lights.unset( LR->m_src_guid );
		}
	}
	else if( eid == EID_GOResourceUpdate )
	{
		SGRX_CAST( GOResource*, rsrc, edata.GetUserData() );
		if( ENTITY_IS_A( rsrc, MeshResource ) )
		{
			SGRX_CAST( MeshResource*, MR, rsrc );
			if( MR->m_src_guid.NotNull() )
			{
				EdLGCMeshInfo& info = m_meshes[ MR->m_src_guid ].info;
				InvalidateMesh( MR->m_src_guid );
				InvalidateLightmap( MR->m_src_guid );
				ReadMeshInfo( &info, MR );
				InvalidateMesh( MR->m_src_guid );
			}
		}
		else if( ENTITY_IS_A( rsrc, LightResource ) )
		{
			SGRX_CAST( LightResource*, LR, rsrc );
			if( LR->m_src_guid.NotNull() )
			{
				InvalidateLight( LR->m_src_guid );
				ReadLightInfo( &m_lights[ LR->m_src_guid ].info, LR );
				InvalidateLight( LR->m_src_guid );
			}
		}
	}
}


