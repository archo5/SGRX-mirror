

#include "engine_int.hpp"

extern IRenderer* g_Renderer;
extern MeshHashTable* g_Meshes;


//
// MESH PARSING

/* FORMAT
	BUFFER: (min size = 4)
	- uint32 size
	- uint8 data[size]
	
	SMALLBUF: (min size = 1)
	- uint8 size
	- uint8 data[size]
	
	PART: (min size = 19)
	- uint32 voff
	- uint32 vcount
	- uint32 ioff
	- uint32 icount
	- uint8 texcount
	- smallbuf shader
	- smallbuf textures[texcount]
	
	MESH:
	- magic "SS3DMESH"
	- uint32 flags
	
	- float boundsmin[3]
	- float boundsmax[3]
	
	- buffer vdata
	- buffer idata
	- smallbuf format
	- uint8 numparts
	- part parts[numparts]
	
	minimum size = 12+24+10 = 46
*/

static int md_parse_buffer( char* buf, size_t size, char** outptr, uint32_t* outsize )
{
	if( size < 4 )
		return 0;
	memcpy( outsize, buf, 4 );
	if( size < *outsize + 4 )
		return 0;
	*outptr = buf + 4;
	return 1;
}

static int md_parse_smallbuf( char* buf, size_t size, char** outptr, uint8_t* outsize )
{
	if( size < 1 )
		return 0;
	memcpy( outsize, buf, 1 );
	if( size < (size_t) *outsize + 1 )
		return 0;
	*outptr = buf + 1;
	return 1;
}

const char* MeshData_Parse( char* buf, size_t size, MeshFileData* out )
{
	uint8_t p, t, b;
	size_t off = 0;
	if( size < 46 || memcmp( buf, "SS3DMESH", 8 ) != 0 )
		return "file too small or not SS3DMESH";
	memcpy( &out->dataFlags, buf + 8, 4 );
	memcpy( &out->boundsMin, buf + 12, 12 );
	memcpy( &out->boundsMax, buf + 24, 12 );
	off = 36;
	if( !md_parse_buffer( buf + off, size - off, &out->vertexData, &out->vertexDataSize ) )
		return "failed to parse VDATA buffer";
	off += 4 + out->vertexDataSize;
	if( !md_parse_buffer( buf + off, size - off, &out->indexData, &out->indexDataSize ) )
		return "failed to parse IDATA buffer";
	off += 4 + out->indexDataSize;
	if( !md_parse_smallbuf( buf + off, size - off, &out->formatData, &out->formatSize ) )
		return "failed to parse FORMAT buffer";
	off += 1 + out->formatSize;
	if( off >= size )
		return "mesh incomplete (missing part data)";
	out->numParts = (uint8_t) buf[ off++ ];
	if( out->numParts > MAX_MESH_FILE_PARTS )
		return "invalid part count";
	for( p = 0; p < out->numParts; ++p )
	{
		MeshFilePartData* pout = out->parts + p;
		memset( pout, 0, sizeof(*pout) );
		if( out->dataFlags & MDF_MTLINFO )
		{
			if( off + 20 > size )
				return "mesh incomplete (corrupted part data)";
			pout->flags = buf[ off++ ];
			pout->blendMode = buf[ off++ ];
		}
		if( out->dataFlags & MDF_PARTNAMES )
		{
			if( off + 19 + 16 * 4 > size )
				return "mesh incomplete (corrupted part data)";
			if( !md_parse_smallbuf( buf + off, size - off, &pout->nameStr, &pout->nameStrSize ) )
				return "failed to parse part name";
			off += 1 + pout->nameStrSize;
			// node transform matrix
			memcpy( &pout->nodeTransform, buf + off, 16 * 4 );
			off += 16 * 4;
		}
		if( off + 18 > size )
			return "mesh incomplete (corrupted part data)";
		memcpy( &pout->vertexOffset, buf + off, 4 ); off += 4;
		memcpy( &pout->vertexCount, buf + off, 4 ); off += 4;
		memcpy( &pout->indexOffset, buf + off, 4 ); off += 4;
		memcpy( &pout->indexCount, buf + off, 4 ); off += 4;
		memcpy( &pout->materialTextureCount, buf + off, 1 ); off += 1;
		for( t = 0; t < pout->materialTextureCount + 1; ++t )
		{
			if( !md_parse_smallbuf( buf + off, size - off, &pout->materialStrings[t], &pout->materialStringSizes[t] ) )
				return "failed to parse material string buffer";
			off += 1 + pout->materialStringSizes[t];
		}
	}
	if( out->dataFlags & MDF_SKINNED )
	{
		if( off >= size )
			return "mesh incomplete (missing bone data)";
		out->numBones = (uint8_t) buf[ off++ ];
		if( out->numBones > SGRX_MAX_MESH_BONES )
			return "invalid bone count";
		for( b = 0; b < out->numBones; ++b )
		{
			MeshFileBoneData* bout = out->bones + b;
			memset( bout, 0, sizeof(*bout) );
			if( !md_parse_smallbuf( buf + off, size - off, &bout->boneName, &bout->boneNameSize ) )
				return "failed to parse bone name string buffer";
			off += 1 + bout->boneNameSize;
			if( off >= size )
				return "mesh bone data incomplete [parent id]";
			bout->parent_id = buf[ off++ ];
			// printf( "pid = %d, namesize = %d, name = %.*s\n", (int)bout->parent_id, (int)bout->boneNameSize, (int)bout->boneNameSize, bout->boneName );
			if( off + 64 > size )
				return "mesh bone data incomplete [matrix]";
			memcpy( bout->boneOffset.a, buf + off, sizeof(Mat4) /* 64 */ );
			off += 64;
		}
	}
	else
		out->numBones = 0;
	return NULL;
}



SGRX_IMesh::SGRX_IMesh() :
	m_dataFlags( 0 ),
	m_vertexDataSize( 0 ),
	m_indexDataSize( 0 ),
	m_numBones( 0 ),
	m_boundsMin( Vec3::Create( 0 ) ),
	m_boundsMax( Vec3::Create( 0 ) )
{
}

SGRX_IMesh::~SGRX_IMesh()
{
	if( m_key.size() )
		g_Meshes->unset( m_key );
}

bool SGRX_IMesh::ToMeshData( ByteArray& out )
{
	if( m_meshParts.size() > MAX_MESH_FILE_PARTS )
		return false;
	ByteWriter bw( &out );
	bw.marker( "SS3DMESH" );
	bw.write( uint32_t(m_dataFlags & MDF__PUBFLAGMASK) );
	bw.write( m_boundsMin );
	bw.write( m_boundsMax );
	bw << m_vdata;
	bw << m_idata;
	bw.write( uint8_t(m_vertexDecl->m_key.size()) );
	bw.memory( m_vertexDecl->m_key.data(), m_vertexDecl->m_key.size() );
	bw.write( uint8_t(m_meshParts.size()) );
	for( size_t pid = 0; pid < m_meshParts.size(); ++pid )
	{
		SGRX_MeshPart& MP = m_meshParts[ pid ];
		if( m_dataFlags & MDF_MTLINFO )
		{
			bw.write( uint8_t(MP.mtlFlags) );
			bw.write( uint8_t(MP.mtlBlendMode) );
		}
		bw.write( uint32_t(MP.vertexOffset) );
		bw.write( uint32_t(MP.vertexCount) );
		bw.write( uint32_t(MP.indexOffset) );
		bw.write( uint32_t(MP.indexCount) );
		bw.write( uint8_t(SGRX_MAX_MESH_TEXTURES) ); // materialTextureCount
		bw.write( uint8_t(MP.shader.size()) );
		bw.memory( MP.shader.data(), MP.shader.size() );
		for( int tid = 0; tid < SGRX_MAX_MESH_TEXTURES; ++tid )
		{
			const String& tex = MP.textures[ tid ];
			bw.write( uint8_t(tex.size()) );
			bw.memory( tex.data(), tex.size() );
		}
	}
	if( m_dataFlags & MDF_SKINNED )
	{
		bw.write( uint8_t(m_numBones) );
		for( int bid = 0; bid < m_numBones; ++bid )
		{
			const SGRX_MeshBone& B = m_bones[ bid ];
			bw.write( uint8_t(B.name.size()) );
			bw.memory( B.name.data(), B.name.size() );
			bw.write( uint8_t(B.parent_id) );
			bw.write( B.boneOffset );
		}
	}
	return true;
}

bool SGRX_IMesh::SetPartData( const SGRX_MeshPart* parts, int count )
{
	if( count < 0 )
		return false;
	m_meshParts.assign( parts, count );
	return true;
}

bool SGRX_IMesh::SetBoneData( SGRX_MeshBone* bones, int count )
{
	if( count < 0 || count > SGRX_MAX_MESH_BONES )
		return false;
	int i;
	for( i = 0; i < count; ++i )
		m_bones[ i ] = bones[ i ];
	for( ; i < count; ++i )
		m_bones[ i ] = SGRX_MeshBone();
	m_numBones = count;
	return RecalcBoneMatrices();
}

bool SGRX_IMesh::RecalcBoneMatrices()
{
	if( !m_numBones )
	{
		return true;
	}
	
	for( int b = 0; b < m_numBones; ++b )
	{
		if( m_bones[ b ].parent_id < -1 || m_bones[ b ].parent_id >= b )
		{
			LOG_WARNING << "RecalcBoneMatrices: each parent_id must point to a previous bone or no bone (-1) [error in bone "
				<< b << ": " << m_bones[ b ].parent_id << "]";
			return false;
		}
	}
	
	Mat4 skinOffsets[ SGRX_MAX_MESH_BONES ];
	for( int b = 0; b < m_numBones; ++b )
	{
		if( m_bones[ b ].parent_id >= 0 )
			skinOffsets[ b ].Multiply( m_bones[ b ].boneOffset, skinOffsets[ m_bones[ b ].parent_id ] );
		else
			skinOffsets[ b ] = m_bones[ b ].boneOffset;
		m_bones[ b ].skinOffset = skinOffsets[ b ];
	}
	for( int b = 0; b < m_numBones; ++b )
	{
		if( !skinOffsets[ b ].InvertTo( m_bones[ b ].invSkinOffset ) )
		{
			LOG_WARNING << "RecalcBoneMatrices: failed to invert skin offset matrix #" << b;
			m_bones[ b ].invSkinOffset = Mat4::Identity;
		}
	}
	return true;
}

bool SGRX_IMesh::SetAABBFromVertexData( const void* data, size_t size, VertexDeclHandle vd )
{
	return GetAABBFromVertexData( vd.GetInfo(), (const char*) data, size, m_boundsMin, m_boundsMax );
}

static void _Tris_Add( Array< Triangle >& tris, VDeclInfo* vdinfo, const void* v1, const void* v2, const void* v3 )
{
	const void* verts[3] = { v1, v2, v3 };
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( *vdinfo, 3, verts, pos );
	
	Triangle T = { pos[0], pos[1], pos[2] };
	tris.push_back( T );
}

template< class IdxType > void SGRX_IMesh_GenTriTree_Core( SGRX_IMesh* mesh )
{
	Array< Triangle > tris;
	
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	VDeclInfo* vdinfo = &mesh->m_vertexDecl->m_info;
	size_t stride = vdinfo->size;
	
	for( size_t part_id = 0; part_id < mesh->m_meshParts.size(); ++part_id )
	{
		SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
		tris.clear();
		for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
		{
			_Tris_Add( tris, vdinfo
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
				, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
			);
		}
		MP.m_triTree.SetTris( tris.data(), tris.size() );
	}
}

void SGRX_IMesh::GenerateTriangleTree()
{
	LOG_FUNCTION;
	
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_GenTriTree_Core< uint32_t >( this );
	}
	else
	{
		SGRX_IMesh_GenTriTree_Core< uint16_t >( this );
	}
}

void SGRX_IMesh_RaycastAll_Core_TestTriangle( const Vec3& rpos, const Vec3& rdir, float rlen,
	SceneRaycastCallback* cb, SceneRaycastInfo* srci, VDeclInfo* vdinfo, const void* v1, const void* v2, const void* v3 )
{
	const void* verts[3] = { v1, v3, v2 }; // order swapped for RayPolyIntersect
	Vec3 pos[3] = {0};
	VD_ExtractFloat3P( *vdinfo, 3, verts, pos );
	
	float dist[1];
	if( RayPolyIntersect( rpos, rdir, pos, 3, dist ) && dist[0] >= 0 && dist[0] < rlen )
	{
		srci->factor = dist[0] / rlen;
		srci->normal = Vec3Cross( pos[1] - pos[0], pos[2] - pos[0] ).Normalized();
		if( srci->meshinst )
			srci->normal = srci->meshinst->matrix.TransformNormal( srci->normal );
		
		// TODO u/v
		cb->AddResult( srci );
	}
}

struct MPTRayQuery : BaseRayQuery
{
	MPTRayQuery( SceneRaycastCallback* cb, SceneRaycastInfo* srciptr, Triangle* ta,
		const Vec3& r0, const Vec3& r1 )
	: srcb( cb ), srci( srciptr ), tris( ta ), ray_end( r1 )
	{
		SetRay( r0, r1 );
	}
	bool operator () ( int32_t* ids, int32_t count )
	{
		for( int32_t i = 0; i < count; ++i )
		{
			Triangle& T = tris[ ids[ i ] ];
			float dist = IntersectLineSegmentTriangle( ray_origin, ray_end, T.P1, T.P2, T.P3 );
			if( dist < 1.0f )
			{
				srci->factor = dist;
				srci->normal = T.GetNormal();
				if( srci->meshinst )
					srci->normal = srci->meshinst->matrix.TransformNormal( srci->normal );
				
				// TODO u/v
				srcb->AddResult( srci );
			}
		}
		return true;
	}
	
	SceneRaycastCallback* srcb;
	SceneRaycastInfo* srci;
	Triangle* tris;
	Vec3 ray_end;
};

template< class IdxType > void SGRX_IMesh_RaycastAll_Core( SGRX_IMesh* mesh, const Vec3& from, const Vec3& to,
	SceneRaycastCallback* cb, SceneRaycastInfo* srci, size_t fp, size_t ep )
{
	SGRX_CAST( IdxType*, indices, mesh->m_idata.data() );
	VDeclInfo* vdinfo = &mesh->m_vertexDecl->m_info;
	size_t stride = vdinfo->size;
	
	Vec3 dtdir = to - from;
	float rlen = dtdir.Length();
	dtdir /= rlen;
	
	for( size_t part_id = fp; part_id < ep; ++part_id )
	{
		srci->partID = part_id;
		SGRX_MeshPart& MP = mesh->m_meshParts[ part_id ];
		if( MP.mtlBlendMode != SGRX_MtlBlend_None &&
			MP.mtlBlendMode != SGRX_MtlBlend_Basic )
			continue;
		
		if( MP.m_triTree.m_bbTree.m_nodes.size() )
		{
			// we have a tree!
			MPTRayQuery query( cb, srci, MP.m_triTree.m_tris.data(), from, to );
			MP.m_triTree.m_bbTree.RayQuery( query );
		}
		else
		{
			// no tree, iterate all triangles
			for( uint32_t tri = MP.indexOffset, triend = MP.indexOffset + MP.indexCount; tri < triend; tri += 3 )
			{
				srci->triID = tri / 3;
				SGRX_IMesh_RaycastAll_Core_TestTriangle( from, dtdir, rlen, cb, srci, vdinfo
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 1 ] ) * stride ]
					, &mesh->m_vdata[ ( MP.vertexOffset + indices[ tri + 2 ] ) * stride ]
				);
			}
		}
	}
}

void SGRX_IMesh::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi )
{
	if( !m_vdata.size() || !m_idata.size() || !m_meshParts.size() )
		return;
	
	SceneRaycastInfo srci = { 0, V3(0), 0, 0, -1, -1, -1, cbmi };
	if( ( m_dataFlags & MDF_INDEX_32 ) != 0 )
	{
		SGRX_IMesh_RaycastAll_Core< uint32_t >( this, from, to, cb, &srci, 0, m_meshParts.size() );
	}
	else
	{
		SGRX_IMesh_RaycastAll_Core< uint16_t >( this, from, to, cb, &srci, 0, m_meshParts.size() );
	}
}

void SGRX_IMesh::MRC_DebugDraw( SGRX_MeshInstance* mi )
{
	// TODO
}


int SGRX_IMesh::FindBone( const StringView& name )
{
	for( int bid = 0; bid < m_numBones; ++bid )
	{
		if( m_bones[ bid ].name == name )
			return bid;
	}
	return -1;
}

bool SGRX_IMesh::IsBoneUnder( int bone, int parent )
{
	while( bone != parent && bone != -1 )
	{
		bone = m_bones[ bone ].parent_id;
	}
	return bone == parent;
}

int SGRX_IMesh::BoneDistance( int bone, int parent )
{
	int count = 1;
	while( bone != parent && bone != -1 )
	{
		bone = m_bones[ bone ].parent_id;
		count++;
	}
	return bone == parent ? count : -1;
}


static MeshHandle OnCreateSysMesh( const StringView& key )
{
	if( key == "sys:plane" )
	{
		static const SGRX_Vertex_Decal verts[ 4 ] =
		{
			// +Z
			{ V3(+1,-1,0), V3(0,0,+1), V3(0,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,-1,0), V3(0,0,+1), V3(1,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,0), V3(0,0,+1), V3(1,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,0), V3(0,0,+1), V3(0,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
		};
		static const uint16_t indices[ 6 ] =
		{
			0, 1, 2, 2, 3, 0,
		};
		static const SGRX_MeshPart part =
		{
			0, 4, 0, 6,
			"default",
			{ "textures/unit.png", "", "", "", "", "", "", "" },
			0, SGRX_MtlBlend_None,
			"", Mat4::Identity
		};
		
		MeshHandle mesh = GR_CreateMesh();
		mesh->SetVertexData( verts, sizeof(verts), GR_GetVertexDecl( SGRX_VDECL_DECAL ) );
		mesh->SetIndexData( indices, sizeof(indices), false );
		mesh->SetPartData( &part, 1 );
		mesh->m_boundsMin = V3(-1,-1,0);
		mesh->m_boundsMax = V3(1,1,0);
		mesh->m_vdata.append( verts, sizeof(verts) );
		mesh->m_idata.append( indices, sizeof(indices) );
		return mesh;
	}
	if( key == "sys:cube" )
	{
		static const SGRX_Vertex_Decal verts[ 4 * 6 ] =
		{
			// +X
			{ V3(+1,-1,+1), V3(+1,0,0), V3(0,0,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			{ V3(+1,+1,+1), V3(+1,0,0), V3(1,0,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			{ V3(+1,+1,-1), V3(+1,0,0), V3(1,1,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			{ V3(+1,-1,-1), V3(+1,0,0), V3(0,1,0), COLOR_RGB(127,255,127), 0xffffffff, 0 },
			// -X
			{ V3(-1,+1,+1), V3(-1,0,0), V3(0,0,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			{ V3(-1,-1,+1), V3(-1,0,0), V3(1,0,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			{ V3(-1,-1,-1), V3(-1,0,0), V3(1,1,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			{ V3(-1,+1,-1), V3(-1,0,0), V3(0,1,0), COLOR_RGB(127,000,127), 0xffffffff, 0 },
			// +Y
			{ V3(+1,+1,+1), V3(0,+1,0), V3(0,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,+1), V3(0,+1,0), V3(1,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,-1), V3(0,+1,0), V3(1,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,-1), V3(0,+1,0), V3(0,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			// -Y
			{ V3(-1,-1,+1), V3(0,-1,0), V3(0,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,-1,+1), V3(0,-1,0), V3(1,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,-1,-1), V3(0,-1,0), V3(1,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(-1,-1,-1), V3(0,-1,0), V3(0,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			// +Z
			{ V3(+1,-1,+1), V3(0,0,+1), V3(0,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,-1,+1), V3(0,0,+1), V3(1,0,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,+1), V3(0,0,+1), V3(1,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,+1), V3(0,0,+1), V3(0,1,0), COLOR_RGB(000,127,127), 0xffffffff, 0 },
			// -Z
			{ V3(-1,-1,-1), V3(0,0,-1), V3(0,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,-1,-1), V3(0,0,-1), V3(1,0,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(+1,+1,-1), V3(0,0,-1), V3(1,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
			{ V3(-1,+1,-1), V3(0,0,-1), V3(0,1,0), COLOR_RGB(255,127,127), 0xffffffff, 0 },
		};
		static const uint16_t indices[ 6 * 6 ] =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20,
		};
		static const SGRX_MeshPart part =
		{
			0, 4 * 6, 0, 6 * 6,
			"default",
			{ "textures/unit.png", "", "", "", "", "", "", "" },
			0, SGRX_MtlBlend_None,
			"", Mat4::Identity
		};
		
		MeshHandle mesh = GR_CreateMesh();
		mesh->SetVertexData( verts, sizeof(verts), GR_GetVertexDecl( SGRX_VDECL_DECAL ) );
		mesh->SetIndexData( indices, sizeof(indices), false );
		mesh->SetPartData( &part, 1 );
		mesh->m_boundsMin = V3(-1);
		mesh->m_boundsMax = V3(1);
		mesh->m_vdata.append( verts, sizeof(verts) );
		mesh->m_idata.append( indices, sizeof(indices) );
		return mesh;
	}
	if( key == "sys:sphere" )
	{
		int hparts = 32;
		int vparts = 16;
		
		Array< SGRX_Vertex_Decal > verts;
		Array< uint16_t > indices;
		
		for( int h = 0; h <= hparts; ++h )
		{
			float hq1 = float(h) / hparts;
			Vec2 hdir1 = V2( cosf( hq1 * M_PI * 2 ), sinf( hq1 * M_PI * 2 ) );
			for( int v = 0; v <= vparts; ++v )
			{
				float vq1 = float(v) / vparts;
				float cv1 = cosf( ( vq1 * 2 - 1 ) * M_PI * 0.5f );
				float sv1 = sinf( ( vq1 * 2 - 1 ) * M_PI * 0.5f );
				Vec3 dir1 = V3( hdir1.x * cv1, hdir1.y * cv1, sv1 );
				Vec3 tangent = Vec3Cross( V3(hdir1.x,hdir1.y,0), V3(0,0,1) ).Normalized();
				SGRX_Vertex_Decal vtx =
				{
					dir1, dir1, V3( hq1, 1 - vq1, 0 ),
					Vec4ToCol32( V4( tangent * 0.5f + 0.5f, 1 ) ),
					0xffffffff, 0,
				};
				verts.push_back( vtx );
			}
		}
		for( int h = 0; h < hparts; ++h )
		{
			int h1 = h + 1;
			for( int v = 0; v < vparts; ++v )
			{
				int v1 = v + 1;
				uint16_t i1 = v + h * ( vparts + 1 );
				uint16_t i2 = v + h1 * ( vparts + 1 );
				uint16_t i4 = v1 + h * ( vparts + 1 );
				uint16_t i3 = v1 + h1 * ( vparts + 1 );
				uint16_t idcs[6] = { i1, i4, i3, i3, i2, i1 };
				indices.append( idcs, 6 );
			}
		}
		
		static const SGRX_MeshPart part =
		{
			0, verts.size(), 0, indices.size(),
			"default",
			{ "textures/unit.png", "", "", "", "", "", "", "" },
			0, SGRX_MtlBlend_None,
			"", Mat4::Identity
		};
		
		MeshHandle mesh = GR_CreateMesh();
		mesh->SetVertexData( verts.data(), verts.size_bytes(), GR_GetVertexDecl( SGRX_VDECL_DECAL ) );
		mesh->SetIndexData( indices.data(), indices.size_bytes(), false );
		mesh->SetPartData( &part, 1 );
		mesh->m_boundsMin = V3(-1);
		mesh->m_boundsMax = V3(1);
		mesh->m_vdata.append( verts.data(), verts.size_bytes() );
		mesh->m_idata.append( indices.data(), indices.size_bytes() );
		return mesh;
	}
	return NULL;
}


MeshHandle GR_CreateMesh()
{
	LOG_FUNCTION;
	
	SGRX_IMesh* mesh = g_Renderer->CreateMesh();
	return mesh;
}

MeshHandle GR_GetMesh( const StringView& path, bool dataonly )
{
	LOG_FUNCTION_ARG( path );
	
	if( !path )
	{
		if( VERBOSE ) LOG_ERROR << LOG_DATE << "  Failed to open mesh file - empty path";
		return NULL;
	}
	
	MeshHandle mesh = g_Meshes->getcopy( path );
	if( mesh )
		return mesh;
	
	double t0 = sgrx_hqtime();
	mesh = OnCreateSysMesh( path );
	if( mesh )
	{
		if( dataonly == false )
		{
			mesh->GenerateTriangleTree();
		}
		
		mesh->m_key = path;
		g_Meshes->set( mesh->m_key, mesh );
		if( VERBOSE ) LOG << "Created sys. mesh: " << path << " (time=" << ( sgrx_hqtime() - t0 ) << ")";
		return mesh;
	}
	
	ByteArray meshdata;
	if( !FS_LoadBinaryFile( path, meshdata ) )
	{
		LOG_ERROR << LOG_DATE << "  Failed to access mesh data file - " << path;
		return NULL;
	}
	
	MeshFileData mfd;
	const char* err = MeshData_Parse( (char*) meshdata.data(), meshdata.size(), &mfd );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse mesh file - " << err;
		return NULL;
	}
	
	SGRX_MeshBone bones[ SGRX_MAX_MESH_BONES ];
	for( int i = 0; i < mfd.numBones; ++i )
	{
		MeshFileBoneData* mfdb = &mfd.bones[ i ];
		bones[ i ].name.append( mfdb->boneName, mfdb->boneNameSize );
		bones[ i ].boneOffset = mfdb->boneOffset;
		bones[ i ].parent_id = mfdb->parent_id == 255 ? -1 : mfdb->parent_id;
	}
	
	VertexDeclHandle vdh;
	mesh = g_Renderer->CreateMesh();
	if( !mesh ||
		!( vdh = GR_GetVertexDecl( StringView( mfd.formatData, mfd.formatSize ) ) ) ||
		( dataonly == false && !mesh->SetVertexData( mfd.vertexData, mfd.vertexDataSize, vdh ) ) ||
		( dataonly == false && !mesh->SetIndexData( mfd.indexData, mfd.indexDataSize, ( mfd.dataFlags & MDF_INDEX_32 ) != 0 ) ) ||
		!mesh->SetBoneData( bones, mfd.numBones ) )
	{
		LOG << "...while trying to create mesh: " << path;
		delete mesh;
		return NULL;
	}
	mesh->m_vertexDecl = vdh; // in case dataonly = false
	
	mesh->m_dataFlags = mfd.dataFlags;
	mesh->m_boundsMin = mfd.boundsMin;
	mesh->m_boundsMax = mfd.boundsMax;
	
	mesh->m_meshParts.clear();
	mesh->m_meshParts.resize( mfd.numParts );
	SGRX_MeshPart* parts = mesh->m_meshParts.data();
	for( int i = 0; i < mfd.numParts; ++i )
	{
		SGRX_MeshPart& P = parts[ i ];
		P.vertexOffset = mfd.parts[ i ].vertexOffset;
		P.vertexCount = mfd.parts[ i ].vertexCount;
		P.indexOffset = mfd.parts[ i ].indexOffset;
		P.indexCount = mfd.parts[ i ].indexCount;
		
		StringView mtltext( mfd.parts[ i ].materialStrings[0], mfd.parts[ i ].materialStringSizes[0] );
		
		// LOAD MATERIAL
		//
		P.mtlFlags = mfd.parts[ i ].flags;
		P.mtlBlendMode = mfd.parts[ i ].blendMode;
		if( mfd.parts[ i ].materialStringSizes[0] >= SHADER_NAME_LENGTH )
		{
			LOG_WARNING << "Shader name for part " << i << " is too long";
		}
		else
		{
			P.shader = mtltext;
		}
		for( int tid = 0; tid < mfd.parts[ i ].materialTextureCount; ++tid )
		{
			P.textures[ tid ] = StringView( mfd.parts[ i ].materialStrings[ tid + 1 ], mfd.parts[ i ].materialStringSizes[ tid + 1 ] );
		}
		
		if( mfd.dataFlags & MDF_PARTNAMES )
		{
			P.name.assign( mfd.parts[ i ].nameStr, mfd.parts[ i ].nameStrSize );
			P.nodeTransform = mfd.parts[ i ].nodeTransform;
		}
	}
	
	mesh->m_vdata.append( (const uint8_t*) mfd.vertexData, mfd.vertexDataSize );
	mesh->m_idata.append( (const uint8_t*) mfd.indexData, mfd.indexDataSize );
	
	if( dataonly == false )
	{
		mesh->GenerateTriangleTree();
	}
	
	mesh->m_key = path;
	g_Meshes->set( mesh->m_key, mesh );
	if( VERBOSE ) LOG << "Loaded mesh: " << path << " (time=" << ( sgrx_hqtime() - t0 ) << ")";;
	return mesh;
}


