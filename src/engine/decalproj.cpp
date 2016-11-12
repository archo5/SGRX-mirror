

#include "engext.hpp"
#include "renderer.hpp"



template< typename T, typename T2 > void sa2_insert( T* arr, T2* arr2, int& count, int& at, const T& val, const T2& val2 )
{
	for( int i = count; i > at; )
	{
		i--;
		arr[ i + 1 ] = arr[ i ];
		arr2[ i + 1 ] = arr2[ i ];
	}
	arr2[ at ] = val2;
	arr[ at++ ] = val;
	count++;
}
template< typename T, typename T2 > void sa2_remove( T* arr, T2* arr2, int& count, int& at )
{
	count--;
	for( int i = at; i < count; ++i )
	{
		arr[ i ] = arr[ i + 1 ];
		arr2[ i ] = arr2[ i + 1 ];
	}
	at--;
}

void SGRX_IMesh_Clip_Core_ClipTriangle( const Mat4& mtx,
	                                    const Mat4& vpmtx,
	                                    ByteArray& outverts,
	                                    SGRX_IVertexDecl* vdecl,
	                                    bool decal,
	                                    float inv_zn2zf,
	                                    uint32_t color,
	                                    const void* v1,
	                                    const void* v2,
	                                    const void* v3 )
{
	const void* verts[3] = { v1, v2, v3 };
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( vdecl->m_info, 3, verts, pos );
	
	pos[0] = mtx.TransformPos( pos[0] );
	pos[1] = mtx.TransformPos( pos[1] );
	pos[2] = mtx.TransformPos( pos[2] );
	
	Vec4 tpos[3] =
	{
		vpmtx.Transform( V4( pos[0], 1.0f ) ),
		vpmtx.Transform( V4( pos[1], 1.0f ) ),
		vpmtx.Transform( V4( pos[2], 1.0f ) ),
	};
	Vec4 pts[9] =
	{
		tpos[0], tpos[1], tpos[2],
		V4(0), V4(0), V4(0),
		V4(0), V4(0), V4(0)
	};
	Vec3 fcs[9] =
	{
		V3(1,0,0), V3(0,1,0), V3(0,0,1),
		V3(0), V3(0), V3(0),
		V3(0), V3(0), V3(0)
	};
	int pcount = 3;
	
#define IMCCCT_CLIP_Pred( initsd, loopsd )           \
	{                                                \
		Vec4 prevpt = pts[ pcount - 1 ];             \
		Vec3 prevfc = fcs[ pcount - 1 ];             \
		float prevsd = /* = 1 */ initsd /* */;       \
		for( int i = 0; i < pcount; ++i )            \
		{                                            \
			Vec4 currpt = pts[ i ];                  \
			Vec3 currfc = fcs[ i ];                  \
			float currsd = /* = 2 */ loopsd /* */;   \
			if( prevsd * currsd < 0 )                \
			{                                        \
				/* insert intersection point */      \
				float f = safe_fdiv( -prevsd,        \
					( currsd - prevsd ) );           \
				sa2_insert( pts, fcs, pcount, i,     \
					TLERP( prevpt, currpt, f ),      \
					TLERP( prevfc, currfc, f ) );    \
			}                                        \
			if( currsd >= 0 )                        \
			{                                        \
				sa2_remove( pts, fcs, pcount, i );   \
			}                                        \
			prevpt = currpt;                         \
			prevfc = currfc;                         \
			prevsd = currsd;                         \
		}                                            \
	}
	IMCCCT_CLIP_Pred( SMALL_FLOAT - prevpt.w, SMALL_FLOAT - currpt.w ); // clip W <= 0
	IMCCCT_CLIP_Pred( prevpt.x - prevpt.w, currpt.x - currpt.w ); // clip X > W
	IMCCCT_CLIP_Pred( -prevpt.x - prevpt.w, -currpt.x - currpt.w ); // clip X < -W
	IMCCCT_CLIP_Pred( prevpt.y - prevpt.w, currpt.y - currpt.w ); // clip Y > W
	IMCCCT_CLIP_Pred( -prevpt.y - prevpt.w, -currpt.y - currpt.w ); // clip Y < -W
	IMCCCT_CLIP_Pred( prevpt.z - prevpt.w, currpt.z - currpt.w ); // clip Z > W
	IMCCCT_CLIP_Pred( -prevpt.z - prevpt.w, -currpt.z - currpt.w ); // clip Z < -W
	// LOG << "VCOUNT: " << pcount;
	// for(int i = 0; i < pcount;++i) LOG << pts[i] << fcs[i];
	if( pcount < 3 )
		return;
	
	// interpolate vertices
	uint8_t vbuf[ 256 * 9 ];
	if( decal )
	{
		SGRX_CAST( SGRX_Vertex_Decal*, dvs, vbuf );
		
		// fixed function interpolation -- faster than VD_LerpTri
		Vec3 nrm[3] = {0};
		VD_ExtractFloat3P( vdecl->m_info, 3, verts, nrm, VDECLUSAGE_NORMAL );
		
		nrm[0] = mtx.TransformNormal( nrm[0] );
		nrm[1] = mtx.TransformNormal( nrm[1] );
		nrm[2] = mtx.TransformNormal( nrm[2] );
		
		for( int i = 0; i < pcount; ++i )
		{
			const Vec3& f = fcs[ i ];
			dvs[ i ].position = pos[0] * f.x + pos[1] * f.y + pos[2] * f.z;
			dvs[ i ].normal = nrm[0] * f.x + nrm[1] * f.y + nrm[2] * f.z;
			// ignore texcoords, they will be regenerated
			dvs[ i ].tangent = 0x007f7f7f; // TODO FIX
			dvs[ i ].color = color; // always constant for decals
			dvs[ i ].padding0 = 0;
		}
		
		if( inv_zn2zf /* perspective distance correction */ )
		{
			for( int i = 0; i < pcount; ++i )
			{
				Vec4 vtp = vpmtx.Transform( V4( dvs[ i ].position, 1.0f ) );
				if( vtp.w )
				{
					float rcp_vtp_w = 1.0f / vtp.w;
					dvs[ i ].texcoord = V3
					(
						vtp.x * rcp_vtp_w * 0.5f + 0.5f,
						vtp.y * rcp_vtp_w * 0.5f + 0.5f,
						vtp.z * inv_zn2zf
					);
				}
				else
				{
					dvs[ i ].texcoord = V3(0);
				}
			}
		}
		else
		{
			for( int i = 0; i < pcount; ++i )
			{
				Vec4 vtp = vpmtx.Transform( V4( dvs[ i ].position, 1.0f ) );
				if( vtp.w )
				{
					float rcp_vtp_w = 1.0f / vtp.w;
					dvs[ i ].texcoord = V3
					(
						vtp.x * rcp_vtp_w * 0.5f + 0.5f,
						vtp.y * rcp_vtp_w * 0.5f + 0.5f,
						vtp.z * rcp_vtp_w * 0.5f + 0.5f
					);
				}
				else
				{
					dvs[ i ].texcoord = V3(0);
				}
			}
		}
	}
	else
	{
		memset( vbuf, 0, sizeof(vbuf) );
		VD_LerpTri( vdecl->m_info, pcount, vbuf, fcs, v1, v2, v3 );
	}
	const int stride = decal ? sizeof(SGRX_Vertex_Decal) : vdecl->m_info.size;
	for( int i = 1; i < pcount - 1; ++i )
	{
		outverts.append( vbuf, stride );
		outverts.append( vbuf + i * stride, stride * 2 );
	}
}

template< class IdxType >
struct IMesh_ClipQuery
{
	void operator () ( int32_t* tris, int32_t count )
	{
		for( int32_t i = 0; i < count; ++i )
		{
			int32_t tri = tris[ i ];
			if( usedTris[ tri >> 5 ] & ( 1 << ( tri & 31 ) ) )
				continue;
			usedTris[ tri >> 5 ] |= 1 << ( tri & 31 );
			tri *= 3;
			tri += MP.indexOffset;
			SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf, color
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
			);
		}
	}
	
	Mat4 mtx;
	Mat4 vpmtx;
	ByteArray& outverts;
	bool decal;
	float inv_zn2zf;
	uint32_t color;
	SGRX_IMesh* mesh;
	SGRX_MeshPart& MP;
	size_t stride;
	IdxType* indices;
	
	Array< uint8_t > usedTris;
};

void FrustumAABB( const Mat4& mvp, Vec3& outmin, Vec3& outmax )
{
	Mat4 inv = mvp.Inverted();
	Vec3 pts[8] =
	{
		inv.TransformPos( V3(-1,-1,-1) ),
		inv.TransformPos( V3(+1,-1,-1) ),
		inv.TransformPos( V3(-1,+1,-1) ),
		inv.TransformPos( V3(+1,+1,-1) ),
		inv.TransformPos( V3(-1,-1,+1) ),
		inv.TransformPos( V3(+1,-1,+1) ),
		inv.TransformPos( V3(-1,+1,+1) ),
		inv.TransformPos( V3(+1,+1,+1) ),
	};
	outmin = outmax = pts[0];
	for( int i = 1; i < 8; ++i )
	{
		outmin = Vec3::Min( outmin, pts[ i ] );
		outmax = Vec3::Max( outmax, pts[ i ] );
	}
}

template< class IdxType >
void SGRX_IMesh_Clip_Core( SGRX_IMesh* mesh,
	                       const Mat4& mtx,
	                       const Mat4& vpmtx,
	                       bool decal,
	                       float inv_zn2zf,
	                       uint32_t color,
	                       ByteArray& outverts,
	                       size_t fp,
	                       size_t ep )
{
	size_t stride = mesh->m_vertexDecl.GetInfo().size;
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	
	for( size_t part_id = fp; part_id < ep; ++part_id )
	{
		SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
		if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
			MP.mtlBlendMode != SGRX_MtlBlend_Basic )
			continue;
		
		if( MP.m_triTree.m_bbTree.m_nodes.size() )
		{
			// we have a tree!
			Vec3 bbmin, bbmax;
			FrustumAABB( mtx * vpmtx, bbmin, bbmax );
			IMesh_ClipQuery<IdxType> query = {
				mtx, vpmtx, outverts, decal, inv_zn2zf, color,
				mesh, MP, stride, indices
			};
			query.usedTris.resize_using( ( ( MP.indexCount / 3 ) >> 5 ) + 1, 0 );
			MP.m_triTree.m_bbTree.Query( bbmin, bbmax, query );
		}
		else
		{
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				SGRX_IMesh_Clip_Core_ClipTriangle( mtx, vpmtx, outverts, mesh->m_vertexDecl, decal, inv_zn2zf, color
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::Clip( const Mat4& mtx,
	                   const Mat4& vpmtx,
	                   ByteArray& outverts,
	                   bool decal,
	                   float inv_zn2zf,
	                   uint32_t color,
	                   size_t firstPart,
	                   size_t numParts )
{
	if( m_vdata.size() == 0 || m_idata.size() == 0 )
		return;
	
	size_t MPC = m_meshParts.size();
	firstPart = TMIN( firstPart, MPC - 1 );
	size_t oneOverLastPart = TMIN( firstPart + TMIN( numParts, MPC ), MPC );
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_Clip_Core< uint32_t >( this, mtx, vpmtx, decal, inv_zn2zf, color, outverts, firstPart, oneOverLastPart );
	}
	else
	{
		SGRX_IMesh_Clip_Core< uint16_t >( this, mtx, vpmtx, decal, inv_zn2zf, color, outverts, firstPart, oneOverLastPart );
	}
}




static uint32_t SGRX_FindOrAddVertex( ByteArray& vertbuf, size_t searchoffset, size_t& writeoffset, const uint8_t* vertex, size_t vertsize )
{
	const size_t idxoffset = 0;
	for( size_t i = searchoffset; i < writeoffset; i += vertsize )
	{
		if( 0 == memcmp( &vertbuf[ i ], vertex, vertsize ) )
			return ( i - idxoffset ) / vertsize;
	}
	uint32_t out = ( writeoffset - idxoffset ) / vertsize;
	memcpy( &vertbuf[ writeoffset ], vertex, vertsize );
	writeoffset += vertsize;
	return out;
}

static void SGRX_DoIndexTriangleMeshVertices( UInt32Array& indices, ByteArray& vertices, size_t offset, size_t stride )
{
#if 1
	while( offset < vertices.size() )
	{
		indices.push_back( offset / stride );
		offset += stride;
	}
	return;
#endif
	// <= 1 tri
	if( vertices.size() <= offset + stride * 3 )
		return;
	
	uint8_t trivertdata[ 256 * 3 ];
	size_t end = ( ( vertices.size() - offset ) / (stride*3) ) * stride * 3 + offset;
	size_t writeoffset = offset;
	size_t readoffset = offset;
	while( readoffset < end )
	{
		// extract a triangle
		memcpy( trivertdata, &vertices[ readoffset ], stride * 3 );
		readoffset += stride * 3;
		
		// insert each vertex/index
		uint32_t idcs[3] =
		{
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata, stride ),
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata + stride, stride ),
			SGRX_FindOrAddVertex( vertices, offset, writeoffset, trivertdata + stride * 2, stride ),
		};
		indices.append( idcs, 3 );
	}
	// remove unused data
	vertices.resize( writeoffset );
}



SGRX_DecalSystem::SGRX_DecalSystem() : m_lightSampler(NULL), m_vbSize(0)
{
}

SGRX_DecalSystem::~SGRX_DecalSystem()
{
	Free();
}

void SGRX_DecalSystem::Init( SceneHandle scene, TextureHandle texDecal, TextureHandle texFalloff )
{
	m_vertexDecl = GR_GetVertexDecl( SGRX_VDECL_DECAL );
	m_mesh = GR_CreateMesh();
	
	m_meshInst = scene->CreateMeshInstance();
	m_meshInst->SetMesh( m_mesh );
	SGRX_Material mtl;
	mtl.flags = SGRX_MtlFlag_Decal | SGRX_MtlFlag_VCol;
	mtl.blendMode = SGRX_MtlBlend_Basic;
	mtl.shader = "decal";
	mtl.textures[0] = texDecal;
	mtl.textures[1] = texFalloff;
	m_meshInst->materials.assign( &mtl, 1 );
	m_meshInst->SetMITexture( 0, GR_GetTexture( "textures/white.png" ) );
	m_meshInst->SetLightingMode( SGRX_LM_Decal );
}

void SGRX_DecalSystem::Free()
{
	ClearAllDecals();
	m_vertexDecl = NULL;
	m_mesh = NULL;
	m_meshInst = NULL;
	m_vbSize = 0;
}

void SGRX_DecalSystem::SetSize( uint32_t vbSize )
{
	m_vbSize = vbSize;
}

void SGRX_DecalSystem::SetDynamic( bool dynamic )
{
	m_meshInst->SetLightingMode( dynamic ? SGRX_LM_Dynamic : SGRX_LM_Decal );
}

void SGRX_DecalSystem::Upload()
{
	if( m_mesh == NULL )
		return;
	
	// cut excess decals
	size_t vbcutsize = 0, vbsize = m_vertexData.size(), ibcutsize = 0, cutcount = 0;
	while( vbsize > m_vbSize + vbcutsize )
	{
		vbcutsize += m_decals[ cutcount++ ];
		ibcutsize += m_decals[ cutcount++ ];
	}
	if( cutcount )
	{
		m_vertexData.erase( 0, vbcutsize );
		m_indexData.erase( 0, ibcutsize );
		m_decals.erase( 0, cutcount );
		uint32_t iboff = vbcutsize / sizeof(SGRX_Vertex_Decal);
		for( size_t i = 0; i < m_indexData.size(); ++i )
		{
			m_indexData[ i ] -= iboff;
		}
	}
	
	// apply data
	if( m_vertexData.size() )
	{
		m_mesh->SetVertexData( m_vertexData.data(), m_vertexData.size_bytes(), m_vertexDecl );
		m_mesh->SetIndexData( m_indexData.data(), m_indexData.size_bytes(), true );
	}
	SGRX_MeshPart mp = { 0, m_vertexData.size() / sizeof(SGRX_Vertex_Decal), 0, m_indexData.size() };
	m_mesh->SetPartData( &mp, 1 );
}

void SGRX_DecalSystem::AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, const Mat4& worldMatrix )
{
	float inv_zn2zf;
	Mat4 vpmtx;
	_GenDecalMatrix( projInfo, &vpmtx, &inv_zn2zf );
	uint32_t color = Vec3ToCol32( m_lightSampler ?
		m_lightSampler->SampleLight( projInfo.pos ) * 0.125f : V3(0.125f) );
	
	size_t origvbsize = m_vertexData.size(), origibsize = m_indexData.size();
	targetMesh->Clip( worldMatrix, vpmtx, m_vertexData, true, inv_zn2zf, color );
	if( m_vertexData.size() > origvbsize )
	{
		_ScaleDecalTexcoords( projInfo, origvbsize );
		_InvTransformDecals( origvbsize );
		SGRX_DoIndexTriangleMeshVertices( m_indexData, m_vertexData, origvbsize, sizeof(SGRX_Vertex_Decal) );
		m_decals.push_back( m_vertexData.size() - origvbsize );
		m_decals.push_back( m_indexData.size() - origibsize );
	}
}

void SGRX_DecalSystem::AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, int partID, const Mat4& worldMatrix )
{
	float inv_zn2zf;
	Mat4 vpmtx;
	_GenDecalMatrix( projInfo, &vpmtx, &inv_zn2zf );
	uint32_t color = Vec3ToCol32( m_lightSampler ?
		m_lightSampler->SampleLight( projInfo.pos ) * 0.125f : V3(0.125f) );
	
	size_t origvbsize = m_vertexData.size(), origibsize = m_indexData.size();
	targetMesh->Clip( worldMatrix, vpmtx, m_vertexData, true, inv_zn2zf, color, partID, 1 );
	if( m_vertexData.size() > origvbsize )
	{
		_ScaleDecalTexcoords( projInfo, origvbsize );
		_InvTransformDecals( origvbsize );
		SGRX_DoIndexTriangleMeshVertices( m_indexData, m_vertexData, origvbsize, sizeof(SGRX_Vertex_Decal) );
		m_decals.push_back( m_vertexData.size() - origvbsize );
		m_decals.push_back( m_indexData.size() - origibsize );
	}
}

void SGRX_DecalSystem::ClearAllDecals()
{
	m_vertexData.clear();
	m_indexData.clear();
	m_decals.clear();
}

void SGRX_DecalSystem::GenerateCamera( const DecalProjectionInfo& projInfo, SGRX_Camera* out )
{
	// TODO for now...
	ASSERT( projInfo.perspective );
	
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	float dist = DMPI.size.z * projInfo.distanceScale;
	
	out->position = projInfo.pos - projInfo.dir * projInfo.pushBack * dist;
	out->direction = projInfo.dir;
	out->updir = projInfo.up;
	out->angle = projInfo.fovAngleDeg;
	out->aspect = projInfo.aspectMult;
	out->aamix = projInfo.aamix;
	out->znear = dist * 0.001f;
	out->zfar = dist;
	
	out->UpdateMatrices();
}

void SGRX_DecalSystem::_ScaleDecalTexcoords( const DecalProjectionInfo& projInfo, size_t vbfrom )
{
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	
	SGRX_CAST( SGRX_Vertex_Decal*, vdata, m_vertexData.data() );
	SGRX_Vertex_Decal* vdend = vdata + m_vertexData.size() / sizeof(SGRX_Vertex_Decal);
	vdata += vbfrom / sizeof(SGRX_Vertex_Decal);
	while( vdata < vdend )
	{
		vdata->texcoord.x = TLERP( DMPI.bbox.x, DMPI.bbox.z, vdata->texcoord.x );
		vdata->texcoord.y = TLERP( DMPI.bbox.y, DMPI.bbox.w, vdata->texcoord.y );
		vdata++;
	}
}

void SGRX_DecalSystem::_InvTransformDecals( size_t vbfrom )
{
	if( m_meshInst->GetLightingMode() == SGRX_LM_Decal )
		return;
	
	Mat4 inv = Mat4::Identity;
	m_meshInst->matrix.InvertTo( inv );
	
	SGRX_CAST( SGRX_Vertex_Decal*, vdata, m_vertexData.data() );
	SGRX_Vertex_Decal* vdend = vdata + m_vertexData.size() / sizeof(SGRX_Vertex_Decal);
	vdata += vbfrom / sizeof(SGRX_Vertex_Decal);
	while( vdata < vdend )
	{
		vdata->position = inv.TransformPos( vdata->position );
		vdata->normal = inv.TransformNormal( vdata->normal );
		vdata++;
	}
}

void SGRX_DecalSystem::_GenDecalMatrix( const DecalProjectionInfo& projInfo, Mat4* outVPM, float* out_invzn2zf )
{
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	
	float znear = 0, dist = DMPI.size.z * projInfo.distanceScale;
	Mat4 projMtx, viewMtx = Mat4::CreateLookAt(
		projInfo.pos - projInfo.dir * projInfo.pushBack * dist,
		projInfo.dir, projInfo.up );
	if( projInfo.perspective )
	{
		float aspect = DMPI.size.x / DMPI.size.y * projInfo.aspectMult;
		znear = dist * 0.001f;
		projMtx = Mat4::CreatePerspective( projInfo.fovAngleDeg, aspect, projInfo.aamix, znear, dist );
		*out_invzn2zf = safe_fdiv( 1, dist - znear );
	}
	else
	{
		Vec2 psz = DMPI.size.ToVec2() * 0.5f * projInfo.orthoScale;
		projMtx = Mat4::CreateOrtho( V3( -psz.x, -psz.y, 0 ), V3( psz.x, psz.y, DMPI.size.z * projInfo.distanceScale ) );
		*out_invzn2zf = 0;
	}
	*outVPM = viewMtx * projMtx;
}



struct IF_GCC(ENGINE_EXPORT) SGRX_ProjectionMeshProcessor : IProcessor
{
	ByteArray* outVertices;
	UInt32Array* outIndices;
	Mat4 viewProjMatrix;
	float invZNearToZFar;
	
	SGRX_ProjectionMeshProcessor( ByteArray* verts, UInt32Array* indices, const Mat4& mtx, float zn2zf );
	ENGINE_EXPORT virtual void Process( void* data );
};

SGRX_ProjectionMeshProcessor::SGRX_ProjectionMeshProcessor( ByteArray* verts, UInt32Array* indices, const Mat4& mtx, float zn2zf ) :
	outVertices( verts ), outIndices( indices ), viewProjMatrix( mtx ), invZNearToZFar( safe_fdiv( 1.0f, zn2zf ) )
{
}

void SGRX_ProjectionMeshProcessor::Process( void* data )
{
	LOG_FUNCTION;
	
	SGRX_CAST( SGRX_MeshInstance*, MI, data );
	
	SGRX_IMesh* M = MI->GetMesh();
	if( M )
	{
		size_t vertoff = outVertices->size();
		M->Clip( MI->matrix, viewProjMatrix, *outVertices, true, invZNearToZFar );
		SGRX_DoIndexTriangleMeshVertices( *outIndices, *outVertices, vertoff, sizeof(SGRX_Vertex_Decal) );
	}
}

void SGRX_Scene::GenerateProjectionMesh( const SGRX_Camera& cam, ByteArray& outverts, UInt32Array& outindices, uint32_t layers )
{
	SGRX_ProjectionMeshProcessor pmp( &outverts, &outindices, cam.mView * cam.mProj, cam.zfar - cam.znear );
	GatherMeshes( cam, &pmp, layers );
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
						SGRX_DoIndexTriangleMeshVertices( m_projectorIndices, m_projectorVertices, vertoff, sizeof(SGRX_Vertex_Decal) );
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
		scene->m_projMeshInst->SetMaterialCount( (uint16_t) m_projectorMaterials.size() );
		for( size_t i = 0; i < m_projectorMaterials.size(); ++i )
		{
			scene->m_projMeshInst->GetMaterial( i ) = *m_projectorMaterials[ i ];
		}
	}
	
	return apply;
}


