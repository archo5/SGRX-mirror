

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
#define LGC_MESH_LMID( x ) (x)
#define LGC_SURF_LMID( x ) ((x)|0x80000000)
#define LGC_LMID_GET_ID( x ) ((x)&0x7fffffff)
#define LGC_IS_MESH_LMID( x ) (((x)&0x80000000)==0)

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

EdLevelGraphicsCont::EdLevelGraphicsCont( GameLevel* lev )
	: IGameLevelSystem( lev, -1 ),
	m_nextSolidID(1), m_nextSurfID(1), m_nextMeshEntID(1), m_nextLightEntID(1),
	m_invalidSamples(false), m_alrInvalidSamples(false), m_lmRenderer(NULL)
{
	g_Level->m_lightTree = &m_sampleTree;
	
	Game_RegisterEventHandler( this, EID_MeshUpdate );
	Game_RegisterEventHandler( this, EID_LightUpdate );
}

EdLevelGraphicsCont::~EdLevelGraphicsCont()
{
	if( m_lmRenderer )
		delete m_lmRenderer;
}

void EdLevelGraphicsCont::OnDestroy()
{
	Game_UnregisterEventHandler( this, EID_MeshUpdate );
	Game_UnregisterEventHandler( this, EID_LightUpdate );
}

void EdLevelGraphicsCont::Reset()
{
	m_nextSolidID = 1;
	m_nextSurfID = 1;
	m_nextMeshEntID = 1;
	m_nextLightEntID = 1;
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
	br.marker( "SGRXLMC2" );
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
		StringView meshid;
		uint32_t lmid = 0;
		br << lmid;
		if( lmid == 0 )
		{
			br.stringView( meshid );
		//	printf("LOAD LIGHTMAP MESH: %s\n", StackString<256>(meshid).str);
		}
		if( meshid.size() )
		{
			// find the mesh by ID
			for( size_t i = 0; i < m_meshes.size(); ++i )
			{
				if( m_meshes.item( i ).value.ent->GetID() == meshid )
				{
					lmid = LGC_MESH_LMID( m_meshes.item( i ).key );
				}
			}
		}
		br << LM;
		if( m_lightmaps.isset( lmid ) )
		{
			*m_lightmaps[ lmid ] = LM;
			m_lightmaps[ lmid ]->ReloadTex();
//			printf( "%d x %d\n", int(LM.width), int(LM.height) );
			ApplyLightmap( lmid );
		}
	}
	
	m_invalidLightmaps.clear();
	for( size_t i = 0; i < m_lightmaps.size(); ++i )
	{
		if( m_lightmaps.item( i ).value->invalid )
		{
			uint32_t lmid = m_lightmaps.item( i ).key;
			m_invalidLightmaps.set( lmid, lmid );
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
	bw.marker( "SGRXLMC2" );
	
	bw << m_invalidSamples;
	bw << m_sampleTree;
	
	for( size_t i = 0; i < m_lightmaps.size(); ++i )
	{
		bw.marker( "LM" );
		uint32_t lmid = m_lightmaps.item( i ).key;
		StringView meshid;
		if( LGC_IS_MESH_LMID( lmid ) )
		{
			uint32_t mid = LGC_LMID_GET_ID( lmid );
			Mesh* M = m_meshes.getptr( mid );
			if( M )
			{
				meshid = M->ent->GetID();
			}
		}
		if( meshid.size() )
		{
			bw.write<uint32_t>(0);
			bw.stringView( meshid );
		//	printf("SAVE LIGHTMAP MESH: %s\n", StackString<256>(meshid).str);
		}
		else
		{
			bw << lmid;
		}
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
		uint32_t id = m_lightmaps.item( i ).key;
		uint32_t subid = LGC_LMID_GET_ID( id );
		LMapHandle lmh = m_lightmaps.item( i ).value;
		printf( "LM id=%u type=%s width=%d height=%d inv:%s|%s\n",
			(unsigned) subid, LGC_IS_MESH_LMID(id) ? "MESH" : "SURF", lmh->width, lmh->height,
			lmh->invalid ? "Y" : "n", lmh->alr_invalid ? "Y" : "n" );
		if( LGC_IS_MESH_LMID( id ) )
		{
			Mesh* M = m_meshes.getptr( subid );
			if( M )
				printf( "- path=%s\n", StackString<256>(M->info.path).str );
			else
				puts( "- MESH NOT FOUND -" );
		}
		else
		{
			Surface* S = m_surfaces.getptr( subid );
			if( S )
				printf( "- mtl=%s\n", StackString<256>(S->mtlname).str );
			else
				puts( "- SURFACE NOT FOUND -" );
		}
	}
	printf( "samples inv:%s|%s\n", m_invalidSamples ? "Y" : "n", m_alrInvalidSamples ? "Y" : "n" );
}

void EdLevelGraphicsCont::LightMesh( SGRX_MeshInstance* MI, uint32_t lmid )
{
	// static lighting
	ASSERT( m_lightmaps.isset( lmid ) );
	MI->SetMITexture( 0, m_lightmaps[ lmid ]->texture );
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
		LightMesh( m_meshes.item( i ).value.ent->m_meshInst, LGC_MESH_LMID( m_meshes.item( i ).key ) );
	}
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		LightMesh( m_surfaces.item( i ).value.meshInst, LGC_SURF_LMID( m_surfaces.item( i ).key ) );
	}
}

void EdLevelGraphicsCont::CreateLightmap( uint32_t lmid )
{
	LMap* LM = new LMap;
	LM->width = 0;
	LM->height = 0;
	LM->texture = GR_GetTexture( "textures/deflm.png" );
	LM->nmtexture = GR_GetTexture( "textures/defnm.png" );
	LM->invalid = true;
	m_lightmaps[ lmid ] = LM;
	m_invalidLightmaps[ lmid ] = lmid;
}

void EdLevelGraphicsCont::ClearLightmap( uint32_t lmid )
{
	LMap* LM = m_lightmaps[ lmid ];
	LM->width = 0;
	LM->height = 0;
	LM->lmdata.clear();
	LM->texture = GR_GetTexture( "textures/deflm.png" );
	LM->nmtexture = GR_GetTexture( "textures/defnm.png" );
	ApplyLightmap( lmid );
}

void EdLevelGraphicsCont::ApplyLightmap( uint32_t lmid )
{
	uint32_t id = LGC_LMID_GET_ID( lmid );
	if( LGC_IS_MESH_LMID( lmid ) )
	{
		m_meshes[ id ].ent->m_meshInst->SetMITexture( 0, m_lightmaps[ lmid ]->texture );
		m_meshes[ id ].ent->m_meshInst->SetMITexture( 1, m_lightmaps[ lmid ]->nmtexture );
	}
	else
	{
		m_surfaces[ id ].meshInst->SetMITexture( 0, m_lightmaps[ lmid ]->texture );
		m_surfaces[ id ].meshInst->SetMITexture( 1, m_lightmaps[ lmid ]->nmtexture );
	}
}

void EdLevelGraphicsCont::InvalidateLightmap( uint32_t lmid )
{
//	printf( "invalidated %u\n", unsigned(lmid) );
	if( m_lmRenderer )
	{
		m_lightmaps[ lmid ]->alr_invalid = true;
		m_alrInvalidLightmaps[ lmid ] = lmid;
	}
	
	m_lightmaps[ lmid ]->invalid = true;
	m_invalidLightmaps[ lmid ] = lmid;
}

void EdLevelGraphicsCont::ValidateLightmap( uint32_t lmid )
{
	m_lightmaps[ lmid ]->invalid = m_lightmaps[ lmid ]->alr_invalid;
	if( m_alrInvalidLightmaps.isset( lmid ) )
		m_invalidLightmaps.set( lmid, lmid );
	else
		m_invalidLightmaps.unset( lmid );
}

void EdLevelGraphicsCont::ApplyInvalidation()
{
	// convert invalidated meshes/surfaces into lights
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		uint32_t id = m_lights.item( i ).key;
		if( m_movedLights.isset( id ) )
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
				m_movedLights.set( id, L.info );
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
				m_movedLights.set( id, L.info );
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
			uint32_t id = m_movedLights.item( j ).key;
			Light* pL = m_lights.getptr( id );
			if( pL == NULL )
				continue;
			EdLGCLightInfo& PrevL = m_movedLights.item( j ).value;
			if( pL->info.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, M.ent->m_meshInst->matrix ) ||
				PrevL.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, M.ent->m_meshInst->matrix ) )
			{
				InvalidateLightmap( LGC_MESH_LMID( m_meshes.item( i ).key ) );
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
			uint32_t id = m_movedLights.item( j ).key;
			Light* pL = m_lights.getptr( id );
			if( pL == NULL )
				continue;
			EdLGCLightInfo& PrevL = m_movedLights.item( j ).value;
			if( pL->info.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, S.meshInst->matrix ) ||
				PrevL.IntersectsAABB( XM->m_boundsMin, XM->m_boundsMax, S.meshInst->matrix ) )
			{
				InvalidateLightmap( LGC_SURF_LMID( m_surfaces.item( i ).key ) );
				break;
			}
		}
	}
	
	// clean up
	m_movedMeshes.clear();
	m_movedSurfs.clear();
	m_movedLights.clear();
}

void EdLevelGraphicsCont::InvalidateMesh( uint32_t id )
{
	if( m_movedMeshes.isset( id ) )
		return;
	Mesh* pM = m_meshes.getptr( id );
	if( pM == NULL )
		return;
	MeshHandle XM = GR_GetMesh( pM->info.path );
	if( XM )
	{
		PrevMeshData pmd = { XM->m_boundsMin, XM->m_boundsMax, pM->info.xform };
		m_movedMeshes.set( id, pmd );
	}
}

void EdLevelGraphicsCont::InvalidateSurface( uint32_t id )
{
	if( m_movedSurfs.isset( id ) )
		return;
	Surface* pS = m_surfaces.getptr( id );
	if( pS == NULL )
		return;
	SGRX_IMesh* XM = pS->meshInst->GetMesh();
	if( XM )
	{
		PrevMeshData pmd = { XM->m_boundsMin, XM->m_boundsMax, pS->meshInst->matrix };
		m_movedSurfs.set( id, pmd );
	}
}

void EdLevelGraphicsCont::InvalidateLight( uint32_t id )
{
	if( m_movedLights.isset( id ) )
		return;
	Light* pL = m_lights.getptr( id );
	if( pL == NULL )
		return;
	m_movedLights.set( id, pL->info );
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

bool EdLevelGraphicsCont::IsInvalidated( uint32_t lmid )
{
	return m_lightmaps.getcopy( lmid )->invalid;
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
	return ( rinfo.rflags & LM_MESHINST_DECAL ) == 0 &&
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
		uint32_t lmid = LGC_MESH_LMID( m_meshes.item( i ).key );
		m_lightmaps[ lmid ]->alr_invalid = false;
		bool solid = RenderInfoIsSolid( M.info );
		bool needslm = RenderInfoNeedsLM( M.info );
		if( needslm && IsInvalidated( lmid ) )
		{
			m_lmRenderer->AddMeshInst( M.ent->m_meshInst, V2(32 * M.info.lmdetail), lmid, solid );
		}
		else
		{
			if( needslm == false )
				ClearLightmap( lmid );
			ValidateLightmap( lmid );
			m_lmRenderer->AddMeshInst( M.ent->m_meshInst, V2(0), 0, solid );
		}
	}
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		uint32_t lmid = LGC_SURF_LMID( m_surfaces.item( i ).key );
		m_lightmaps[ lmid ]->alr_invalid = false;
		bool solid = MtlIsSolid( S.mtlname ) && RenderInfoIsSolid( S.info );
		bool needslm = RenderInfoNeedsLM( S.info ) && MtlNeedsLM( S.mtlname );
		if( needslm && IsInvalidated( lmid ) )
		{
			m_lmRenderer->AddMeshInst( S.meshInst, S.lmsize * S.info.lmdetail, lmid, solid );
		}
		else
		{
			if( needslm == false )
				ClearLightmap( lmid );
			ValidateLightmap( lmid );
			if( solid )
				m_lmRenderer->AddMeshInst( S.meshInst, V2(0), 0, true );
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
			uint32_t lmidsize[3];
			if( m_lmRenderer->GetLightmap( i, colors, normals, lmidsize ) &&
				m_lightmaps.isset( lmidsize[0] ) )
			{
				uint32_t lmid = lmidsize[0];
				LMap& L = *m_lightmaps[ lmid ];
				L.width = lmidsize[1];
				L.height = lmidsize[2];
				L.lmdata = colors;
				L.nmdata = normals;
				L.ReloadTex();
				ApplyLightmap( lmid );
				ValidateLightmap( lmid );
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
		if( M.ent->IsStatic() == false )
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


void EdLevelGraphicsCont::ExportLightmap( uint32_t lmid, LC_Lightmap& outlm )
{
	LMap& LM = *m_lightmaps[ lmid ];
	outlm.width = LM.width;
	outlm.height = LM.height;
	outlm.data.resize( LM.width * LM.height );
	outlm.nmdata.resize( LM.width * LM.height );
	LM.ExportRGBA8( outlm.data.data(), outlm.nmdata.data() );
}

void EdLevelGraphicsCont::UpdateCache( LevelCache& LC )
{
	for( size_t i = 0; i < m_solids.size(); ++i )
	{
		Solid& S = m_solids.item( i ).value;
		LC.AddSolid( S.planes.data(), S.planes.size() );
	}
	
	for( size_t i = 0; i < m_surfaces.size(); ++i )
	{
		Surface& S = m_surfaces.item( i ).value;
		if( S.mtlname == SV("") || S.mtlname == SV("null") )
			continue;
		
		LC_Lightmap lm;
		ExportLightmap( LGC_SURF_LMID( m_surfaces.item( i ).key ), lm );
		
		Array< LCVertex > verts;
		verts.reserve( S.indices.size() );
		for( size_t v = 0; v < S.indices.size(); ++v )
			verts.push_back( S.vertices[ S.indices[ v ] ] );
		
		size_t solid = S.solid_id > 0 ? S.solid_id - 1 : NOT_FOUND;
		int decalLayer = ( S.info.rflags & LM_MESHINST_DECAL ) != 0 ? S.info.decalLayer : -1;
		LC.AddPart( verts.data(), verts.size(), lm, S.mtlname, solid, S.info.rflags, decalLayer );
	}
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		Mesh& M = m_meshes.item( i ).value;
		if( M.ent->m_isStatic == false || M.ent->m_isVisible == false )
			continue;
		
		LC_Lightmap lm;
		ExportLightmap( LGC_MESH_LMID( m_meshes.item( i ).key ), lm );
		
		LC.AddMeshInst( M.info.path, M.ent->m_meshInst->matrix, M.info.rflags, M.info.decalLayer, lm );
	}
	
	LC.AddLight( g_EdWorld->GetDirLightInfo() );
	
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		if( m_lights.item( i ).value.ent->m_isStatic )
			LC.AddLight( m_lights.item( i ).value.info );
	}
}


uint32_t EdLevelGraphicsCont::CreateSolid( EdLGCSolidInfo* info )
{
	uint32_t id = m_nextSolidID;
	ASSERT( LGC_IS_VALID_ID( id ) );
	RequestSolid( id, info );
	return id;
}

void EdLevelGraphicsCont::RequestSolid( uint32_t id, EdLGCSolidInfo* info )
{
	ASSERT( m_solids.isset( id ) == false );
	Solid S;
	m_solids.set( id, S );
	UpdateSolid( id, info );
	if( id >= m_nextSolidID )
		m_nextSolidID = id + 1;
}

void EdLevelGraphicsCont::DeleteSolid( uint32_t id )
{
	ASSERT( m_solids.isset( id ) );
	m_solids.unset( id );
	if( id == m_nextSolidID - 1 )
		m_nextSolidID--;
}

static EdLGCSolidInfo g_defSolidInfo;
void EdLevelGraphicsCont::UpdateSolid( uint32_t id, EdLGCSolidInfo* info )
{
	if( info == NULL )
		info = &g_defSolidInfo;
	ASSERT( m_solids.isset( id ) );
	Solid& S = m_solids[ id ];
	S.planes.assign( info->planes, info->pcount );
}

uint32_t EdLevelGraphicsCont::CreateSurface( EdLGCSurfaceInfo* info )
{
	uint32_t id = m_nextSurfID;
	ASSERT( LGC_IS_VALID_ID( id ) );
	RequestSurface( id, info );
	return id;
}

void EdLevelGraphicsCont::RequestSurface( uint32_t id, EdLGCSurfaceInfo* info )
{
	ASSERT( m_surfaces.isset( id ) == false );
	Surface S;
	S.lmsize = V2(0);
	S.meshInst = g_EdScene->CreateMeshInstance();
	S.meshInst->SetMesh( GR_CreateMesh() );
	SGRX_Material mtl;
	mtl.shader = "default";
	S.meshInst->materials.assign( &mtl, 1 );
	m_surfaces.set( id, S );
	CreateLightmap( LGC_SURF_LMID( id ) );
	UpdateSurface( id, LGC_CHANGE_ALL, info );
	if( id >= m_nextSurfID )
		m_nextSurfID = id + 1;
}

void EdLevelGraphicsCont::DeleteSurface( uint32_t id )
{
	ASSERT( m_surfaces.isset( id ) );
	m_lightmaps.unset( LGC_SURF_LMID( id ) );
	m_surfaces.unset( id );
	if( id == m_nextSurfID - 1 )
		m_nextSurfID--;
}

static EdLGCSurfaceInfo g_defSurfInfo;
void EdLevelGraphicsCont::UpdateSurface( uint32_t id, uint32_t changes, EdLGCSurfaceInfo* info )
{
	if( info == NULL )
		info = &g_defSurfInfo;
	ASSERT( m_surfaces.isset( id ) );
	Surface& S = m_surfaces[ id ];
	
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
		InvalidateSurface( id );
	
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
			InvalidateLightmap( LGC_SURF_LMID( id ) );
			S.lmsize = info->lmsize;
			InvalidateLightmap( LGC_SURF_LMID( id ) );
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
		S.solid_id = info->solid_id;
	}
	if( changes & LGC_SURF_CHANGE_LMPARENT )
	{
		S.lmparent_id = info->lmparent_id;
		InvalidateLightmap( LGC_SURF_LMID( id ) );
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
				InvalidateLightmap( LGC_SURF_LMID( id ) ); // necessary?
			
			S.info = *info;
			
			if( diff == 1 )
				InvalidateLightmap( LGC_SURF_LMID( id ) );
		}
		SGRX_LightingMode lmode = SGRX_LM_Static;
		if( info->rflags & LM_MESHINST_DYNLIT )
			lmode = SGRX_LM_Dynamic;
		if( info->rflags & LM_MESHINST_UNLIT )
			lmode = SGRX_LM_Unlit;
		S.meshInst->SetLightingMode( lmode );
		SGRX_Material& mtl = S.meshInst->GetMaterial( 0 );
		bool decal = ( info->rflags & LM_MESHINST_DECAL ) != 0;
		
		mtl.blendMode = SGRX_MtlBlend_None;
		mtl.flags = 0;
		MapMaterial* mapmtl = g_NUISurfMtlPicker->m_materials.getcopy( S.mtlname );
		if( mapmtl )
		{
			mtl.blendMode = mapmtl->blendmode;
			mtl.flags = mapmtl->flags;
		}
		if( decal )
		{
			mtl.flags |= SGRX_MtlFlag_Decal;
			mtl.blendMode = SGRX_MtlBlend_Basic;
		}
	//	mtl.flags = decal ? SGRX_MtlFlag_Decal : 0;
	//	mtl.blendMode = decal ? SGRX_MtlBlend_Basic : SGRX_MtlBlend_None;
		S.meshInst->OnUpdate();
		LightMesh( S.meshInst, LGC_SURF_LMID( id ) );
	}
	S.meshInst->enabled = S.mtlname.size() != 0 && S.vertices.size() != 0 && S.indices.size() != 0;
	
	if( edited )
		InvalidateSurface( id );
}

static void ReadLightInfo( EdLGCLightInfo* out, LightEntity* le )
{
	out->type = le->m_type == LIGHT_SPOT ? LM_LIGHT_SPOT : LM_LIGHT_POINT;
	if( le->m_isStatic == false || le->m_isEnabled == false )
		out->type = -1;
	out->pos = le->GetWorldPosition();
	out->dir = le->LocalToWorldDir( V3(0,0,-1) );
	out->up = le->LocalToWorldDir( V3(0,-1,0) );
	out->range = le->m_range;
	out->power = le->m_power;
	out->light_radius = le->m_lightRadius;
	out->color = le->m_color * le->m_intensity;
	out->num_shadow_samples = 0;
	out->flaresize = le->m_flareSize;
	out->flareoffset = le->m_flareOffset;
	out->innerangle = le->m_innerAngle;
	out->outerangle = le->m_angle;
	out->spotcurve = le->m_spotCurve;
}

static void ReadMeshInfo( EdLGCMeshInfo* out, MeshEntity* me )
{
	out->path = me->m_mesh ? me->m_mesh->m_key : "";
	out->xform = me->GetWorldMatrix();
	out->rflags = 0
		| ( me->m_isSolid ? LM_MESHINST_SOLID : 0 )
		| ( !me->m_isStatic || me->m_lightingMode == SGRX_LM_Dynamic ? LM_MESHINST_DYNLIT : 0 )
		| ( me->m_lightingMode == SGRX_LM_Unlit ? LM_MESHINST_UNLIT : 0 )
		| ( me->m_castLMS ? LM_MESHINST_CASTLMS : 0 )
		| ( me->m_lightingMode == SGRX_LM_Decal ? LM_MESHINST_DECAL : 0 );
	out->lmdetail = me->m_lmQuality;
	out->decalLayer = 0;
}

void EdLevelGraphicsCont::OnAddEntity( Entity* ent )
{
	ASSERT( ent );
	if( ENTITY_IS_A( ent, LightEntity ) )
	{
		SGRX_CAST( LightEntity*, LE, ent );
		uint32_t id = m_nextLightEntID++;
		ASSERT( LGC_IS_VALID_ID( id ) );
		Light L;
		{
			ReadLightInfo( &L.info, LE );
			L.ent = LE;
		}
		m_lights.set( id, L );
		LE->m_edLGCID = id;
	}
	if( ENTITY_IS_A( ent, MeshEntity ) )
	{
		SGRX_CAST( MeshEntity*, ME, ent );
		uint32_t id = m_nextMeshEntID++;
		ASSERT( LGC_IS_VALID_ID( id ) );
		Mesh M;
		{
			ReadMeshInfo( &M.info, ME );
			M.ent = ME;
		}
		ASSERT( M.ent );
		m_meshes.set( id, M );
		CreateLightmap( LGC_MESH_LMID( id ) );
		ME->m_edLGCID = id;
	}
}

void EdLevelGraphicsCont::OnRemoveEntity( Entity* ent )
{
	ASSERT( ent );
	if( ENTITY_IS_A( ent, LightEntity ) )
	{
		SGRX_CAST( LightEntity*, LE, ent );
		uint32_t id = LE->m_edLGCID;
		
		InvalidateLight( id );
		ApplyInvalidation();
		m_movedLights.unset( id );
		
		ASSERT( m_lights.isset( id ) );
		m_lights.unset( id );
		if( id && id == m_nextLightEntID - 1 )
			m_nextLightEntID--;
		LE->m_edLGCID = 0;
	}
	if( ENTITY_IS_A( ent, MeshEntity ) )
	{
		SGRX_CAST( MeshEntity*, ME, ent );
		uint32_t id = ME->m_edLGCID;
		
		InvalidateMesh( id );
		ApplyInvalidation();
		m_movedMeshes.unset( id );
		
		ASSERT( m_meshes.isset( id ) );
		m_lightmaps.unset( LGC_MESH_LMID( id ) );
		m_meshes.unset( id );
		if( id && id == m_nextMeshEntID - 1 )
			m_nextMeshEntID--;
		ME->m_edLGCID = 0;
	}
}

void EdLevelGraphicsCont::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_MeshUpdate )
	{
		SGRX_CAST( MeshEntity*, ME, edata.GetUserData() );
		if( ME->m_edLGCID )
		{
			EdLGCMeshInfo& info = m_meshes[ ME->m_edLGCID ].info;
			InvalidateMesh( ME->m_edLGCID );
			InvalidateLightmap( LGC_MESH_LMID( ME->m_edLGCID ) );
			ReadMeshInfo( &info, ME );
			InvalidateMesh( ME->m_edLGCID );
		}
	}
	else if( eid == EID_LightUpdate )
	{
		SGRX_CAST( LightEntity*, LE, edata.GetUserData() );
		if( LE->m_edLGCID )
		{
			InvalidateLight( LE->m_edLGCID );
			ReadLightInfo( &m_lights[ LE->m_edLGCID ].info, LE );
			InvalidateLight( LE->m_edLGCID );
		}
	}
}


