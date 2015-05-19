

#include <float.h>

#define USE_VEC4
#define USE_ARRAY
#define USE_MAT4
#include <engine.hpp>
#include "compiler.hpp"



int RectPacker::_NodeAlloc( int startnode, int w, int h )
{
	Node* me = &m_tree[ startnode ];
	
	// if not leaf..
	if( me->child0 )
	{
		int ret = _NodeAlloc( me->child0, w, h );
		if( ret > 0 )
			return ret;
		// -- no need to refresh pointer since node allocations only precede successful rect allocations
		return _NodeAlloc( me->child1, w, h );
	}
	
	// occupied
	if( me->occupied )
		return -1;
	
	// too small
	if( w > me->x1 - me->x0 || h > me->y1 - me->y0 )
		return -1;
	
	if( w == me->x1 - me->x0 && h == me->y1 - me->y0 )
	{
		me->occupied = true;
		return me - &m_tree[0];
	}
	
	// split
	me->child0 = m_tree.size();
	me->child1 = me->child0 + 1;
	Node tmpl = { 0, 0, 0, 0, 0, 0, false };
	m_tree.push_back( tmpl );
	m_tree.push_back( tmpl );
	me = &m_tree[ startnode ]; // -- refresh my pointer (in case of reallocation)
	Node* ch0 = &m_tree[ me->child0 ];
	Node* ch1 = &m_tree[ me->child1 ];
	
	int dw = me->x1 - me->x0 - w;
	int dh = me->y1 - me->y0 - h;
	ch0->x0 = me->x0;
	ch0->y0 = me->y0;
	ch1->x1 = me->x1;
	ch1->y1 = me->y1;
	if( dw > dh )
	{
		ch0->x1 = me->x0 + w;
		ch0->y1 = me->y1;
		ch1->x0 = me->x0 + w;
		ch1->y0 = me->y0;
	}
	else
	{
		ch0->x1 = me->x1;
		ch0->y1 = me->y0 + h;
		ch1->x0 = me->x0;
		ch1->y0 = me->y0 + h;
	}
	
	return _NodeAlloc( me->child0, w, h );
}



#define LINE_HEIGHT 0.45f

void LevelCache::_AddPoly( Mesh& M, Part& P, const Vec3& center, const Vertex* verts, int vcount, float lmquality, size_t fromsolid )
{
	M.m_pos += center;
	M.m_div++;
	
	for( int i = 0; i < vcount; ++i )
	{
		M.m_boundsMin = Vec3::Min( M.m_boundsMin, verts[ i ].pos );
		M.m_boundsMax = Vec3::Max( M.m_boundsMax, verts[ i ].pos );
	}
	
	P.m_vertices.append( verts, vcount );
	P.m_polysizes.push_back( vcount );
	P.m_polylmq.push_back( lmquality );
	P.m_solids.push_back( fromsolid );
}

void LevelCache::AddPoly( const Vertex* verts, int vcount, const String& texname_short, float lmquality, size_t fromsolid )
{
	if( vcount < 3 )
		return;
	
	char texbfr[ 256 ];
	sgrx_snprintf( texbfr, sizeof(texbfr), "textures/%.*s.png", TMIN( (int) texname_short.size(), 200 ), texname_short.data() );
	StringView texname = texbfr;
	
	Vec3 center = V3(0);
	for( int i = 0; i < vcount; ++i )
		center += verts[ i ].pos;
	center /= vcount;
	
	for( size_t mid = 0; mid < m_meshes.size(); ++mid )
	{
		Mesh& M = m_meshes[ mid ];
		
		Vec3 curpos = M.m_pos / M.m_div;
		if( ( curpos - center ).Length() > 100 )
			continue;
		
		for( size_t pid = 0; pid < M.m_parts.size(); ++pid )
		{
			Part& P = M.m_parts[ pid ];
			if( P.m_texname == texname )
			{
				_AddPoly( M, P, center, verts, vcount, lmquality, fromsolid );
				return;
			}
		}
		
		if( M.m_parts.size() < 16 )
		{
			M.m_parts.push_back( Part() );
			Part& P = M.m_parts.last();
			P.m_texname = texname;
			
			_AddPoly( M, P, center, verts, vcount, lmquality, fromsolid );
			return;
		}
	}
	
	// add new mesh
	m_meshes.push_back( Mesh() );
	Mesh& M = m_meshes.last();
	M.m_pos = V3(0);
	M.m_boundsMin = V3(FLT_MAX);
	M.m_boundsMax = V3(-FLT_MAX);
	M.m_div = 0;
	M.m_parts.push_back( Part() );
	Part& P = M.m_parts.last();
	P.m_texname = texname;
	_AddPoly( M, P, center, verts, vcount, lmquality, fromsolid );
	
	// add instance for new mesh
	char bfr[ 32 ];
	sprintf( bfr, "~/%d.ssm", (int) m_meshes.size() - 1 );
	AddMeshInst( bfr, Mat4::Identity );
}

size_t LevelCache::AddSolid( const Vec4* planes, int count )
{
	m_solids.push_back( Solid() );
	Solid& S = m_solids.last();
	S.append( planes, count );
	return m_solids.size() - 1;
}


FINLINE float safediv( float a, float b )
{
	if( !b )
		return 0;
	return a / b;
}

void LevelCache::_CutPoly( const PartPoly& PP, const Vec4& plane, Array< PartPoly >& outpolies )
{
	int numfront = 0;
	int numback = 0;
	
	Vec3 PN = plane.ToVec3();
	float PD = plane.w;
	
	PartPoly PPB; PPB.m_solid = PP.m_solid;
	PartPoly PPF; PPF.m_solid = PP.m_solid;
	
	for( size_t p = 0; p < PP.size(); ++p )
	{
		const Vertex& PPV = PP[ p ];
		
		size_t p1 = ( p + 1 ) % PP.size();
		
		float sigdist = Vec3Dot( PN, PP[ p ].pos ) - PD;
		float sigdist2 = Vec3Dot( PN, PP[ p1 ].pos ) - PD;
		
		if( sigdist > SMALL_FLOAT ) numfront++;
		if( sigdist < -SMALL_FLOAT ) numback++;
		
		if( sigdist < -SMALL_FLOAT )
		{
			PPB.push_back( PPV );
			Vertex IPV = PPV.Interpolate( PP[ p1 ], safediv( 0 - sigdist, sigdist2 - sigdist ) );
			if( sigdist2 > SMALL_FLOAT )
			{
				PPB.push_back( IPV );
				PPF.push_back( IPV );
			}
		}
		else if( sigdist > SMALL_FLOAT )
		{
			PPF.push_back( PPV );
			Vertex IPV = PPV.Interpolate( PP[ p1 ], safediv( 0 - sigdist, sigdist2 - sigdist ) );
			if( sigdist2 < -SMALL_FLOAT )
			{
				PPB.push_back( IPV );
				PPF.push_back( IPV );
			}
		}
		else
		{
			PPB.push_back( PPV );
			PPF.push_back( PPV );
		}
	}
	
	if( numfront == 0 || numback == 0 )
	{
		outpolies.push_back( PP );
		return;
	}
	
//		LOG << "PLANE: " << plane;
//	if( PPB.size() >= 3 )
//	{
//		LOG << "POLY BACK [" << PPB.size() << "]";
//		for( size_t i = 0; i < PPB.size(); ++i )
//			LOG << "- " << i << ": " << PPB[ i ].pos;
//	}
//	if( PPF.size() >= 3 )
//	{
//		LOG << "POLY FRONT [" << PPF.size() << "]";
//		for( size_t i = 0; i < PPF.size(); ++i )
//			LOG << "- " << i << ": " << PPF[ i ].pos;
//	}
	
	if( PPB.size() >= 3 ) outpolies.push_back( PPB );
	if( PPF.size() >= 3 ) outpolies.push_back( PPF );
}

// int SOLID = 0;
bool LevelCache::_PolyInside( const PartPoly& PP, const Solid& S )
{
	Vec3 center = {0,0,0};
	for( size_t i = 0; i < PP.size(); ++i )
		center += PP[ i ].pos;
	center /= PP.size();
	
	center += Vec3Cross( PP[2].pos - PP[0].pos, PP[1].pos - PP[0].pos ).Normalized() * SMALL_FLOAT;
	
	for( size_t i = 0; i < S.size(); ++i )
	{
		Vec4 plane = S[ i ];
		if( Vec3Dot( plane.ToVec3(), center ) > plane.w )
			return false;
	}
//	if( fabsf(center.x- 2) < SMALL_FLOAT && fabsf(center.y- 1) < SMALL_FLOAT )
//	{
//		LOG << "ACHTUNG " << SOLID << "<|>" << PP.m_solid << " | " << center;
//		LOG << S;
//		if( PP.size() >= 3 )
//		{
//			LOG << "POLY [" << PP.size() << "]";
//			for( size_t i = 0; i < PP.size(); ++i )
//				LOG << "- " << i << ": " << PP[ i ].pos;
//		}
//	}
	return true;
}

void LevelCache::RemoveHiddenSurfaces()
{
	PartPoly PPstage;
	Array< PartPoly > m_polies;
	Array< PartPoly > m_cutpolies;
	Array< PartPoly > m_inprog;
	Array< PartPoly > m_cutinprog;
	
	for( size_t m = 0; m < m_meshes.size(); ++m )
	{
		Mesh& M = m_meshes[ m ];
		for( size_t p = 0; p < M.m_parts.size(); ++p )
		{
			Part& P = M.m_parts[ p ];
			
			// load the polygons
			m_polies.clear();
			int ofs = 0;
			for( size_t q = 0; q < P.m_polysizes.size(); ++q )
			{
				PPstage.clear();
				PPstage.m_solid = P.m_solids[ q ];
				for( int i = 0; i < P.m_polysizes[ q ]; ++i )
					PPstage.push_back( P.m_vertices[ ofs++ ] );
				m_polies.push_back( PPstage );
			}
			
		//	LOG << "CSG mesh " << m << " plane " << p << " in: " << m_polies.size();
			
			// cut polygons
			for( size_t s = 0; s < m_solids.size() && m_polies.size(); ++s )
			{
				m_cutpolies.clear();
				const Solid& S = m_solids[ s ];
				
			//	size_t ops = m_polies.size();
				while( m_polies.size() )
				{
					PartPoly LPP = m_polies.last();
					if( LPP.m_solid == s )
					{
						// skip this polygon / solid combo (same source)
						m_cutpolies.push_back( LPP );
						m_polies.pop_back();
						continue;
					}
					
					// take one source polygon
					m_cutinprog.clear();
					m_inprog.clear();
					m_inprog.push_back( LPP );
					m_polies.pop_back();
					
					// cut by all planes of a solid
					for( size_t sp = 0; sp < S.size(); ++sp )
					{
						const Vec4 plane = S[ sp ];
						
						for( size_t pip = 0; pip < m_inprog.size(); ++pip )
						{
							_CutPoly( m_inprog[ pip ], plane, m_cutinprog );
						}
						m_inprog.clear();
						TSWAP( m_inprog, m_cutinprog );
					}
					
					// remove inside polygons
				//	SOLID = s;
					size_t oipsize = m_inprog.size();
					for( size_t pip = 0; pip < m_inprog.size(); ++pip )
						if( _PolyInside( m_inprog[ pip ], S ) )
							m_inprog.uerase( pip-- );
					
					if( oipsize == m_inprog.size() )
						m_cutpolies.push_back( LPP );
					else
						m_cutpolies.append( m_inprog.data(), m_inprog.size() );
				}
		//		LOG << "csg trimmed by solid " << s << " from " << ops << " to " << m_cutpolies.size();
				
				TSWAP( m_polies, m_cutpolies );
			}
			
			// put polygons back
			P.m_vertices.clear();
			P.m_solids.clear();
			P.m_polysizes.clear();
			
			P.m_solids.reserve( m_polies.size() );
			P.m_polysizes.reserve( m_polies.size() );
			
			for( size_t cp = 0; cp < m_polies.size(); ++cp )
			{
				PartPoly& PP = m_polies[ cp ];
				P.m_polysizes.push_back( PP.size() );
				P.m_solids.push_back( PP.m_solid );
				for( size_t cpv = 0; cpv < PP.size(); ++cpv )
					P.m_vertices.push_back( PP[ cpv ] );
			}
		}
	}
}

void LevelCache::GenerateLines()
{
	for( size_t m = 0; m < m_meshes.size(); ++m )
	{
		Mesh& M = m_meshes[ m ];
		for( size_t p = 0; p < M.m_parts.size(); ++p )
		{
			Part& P = M.m_parts[ p ];
			
			int ofs = 0;
			for( size_t pp = 0; pp < P.m_polysizes.size(); ++pp )
			{
				Vertex* verts = &P.m_vertices[ ofs ];
				int vcount = P.m_polysizes[ pp ];
				ofs += vcount;
				
				// ADD LINES
				Vec3 isps[ 2 ];
				int ispc = 0;
				for( int i = 0; i < vcount && ispc < 2; ++i )
				{
					Vec3 v0 = verts[ i ].pos;
					Vec3 v1 = verts[ ( i + 1 ) % vcount ].pos;
					if( ( v0.z <= LINE_HEIGHT && v1.z >= LINE_HEIGHT ) ||
						( v0.z >= LINE_HEIGHT && v1.z <= LINE_HEIGHT ) )
					{
						if( v1.z == v0.z )
						{
							break; // going to be easier (less code dup) to insert these separately (polygons are closed)
						}
						else
						{
							Vec3 isp = TLERP( v0, v1, ( LINE_HEIGHT - v0.z ) / ( v1.z - v0.z ) );
							int at = 0;
							for( ; at < ispc; ++at )
								if( ( isps[ at ] - isp ).LengthSq() < SMALL_FLOAT )
									break;
							if( at == ispc )
							{
								isps[ ispc++ ] = isp;
							}
						}
					}
				}
				if( ispc == 2 )
				{
					Vec2 line[] = { isps[0].ToVec2(), isps[1].ToVec2() };
					m_lines.append( line, 2 );
				}
			}
		}
	}
}


void LevelCache::_GenerateLightmapPolys( Part& P )
{
	if( TexNoLight( P.m_texname ) )
	{
		for( size_t i = 0; i < P.m_vertices.size(); ++i )
		{
			P.m_vertices[ i ].nrm = V3(0);
			P.m_lmverts.push_back( V2(0) );
		}
		for( size_t i = 0; i < P.m_polysizes.size(); ++i )
		{
			P.m_lmrects.push_back( V2(0) );
			P.m_lmallocs.push_back( -1 );
		}
		return;
	}
	// longest edge is placed on X
	size_t at = 0;
	for( size_t i = 0; i < P.m_polysizes.size(); ++i )
	{
		Vec3 longedge = V3(0);
		Vec3 normal = V3(0);
		float lel = 0;
		
		int polysize = P.m_polysizes[ i ];
		Vec3 prevedge = P.m_vertices[ at ].pos - P.m_vertices[ at + polysize - 1 ].pos;
		for( int v = 0; v < polysize; ++v )
		{
			Vec3 newedge = P.m_vertices[ at + ( v + 1 ) % polysize ].pos - P.m_vertices[ at + v ].pos;
			float nel = newedge.Length();
			normal += Vec3Cross( newedge, prevedge );
			if( nel > lel )
			{
				lel = nel;
				longedge = newedge;
			}
			prevedge = newedge;
		}
		
		longedge.Normalize();
		normal.Normalize();
		Vec3 edge_x = longedge;
		Vec3 edge_y = Vec3Cross( edge_x, normal ).Normalized();
		
		Vec2 tmin = V2(FLT_MAX), tmax = V2(-FLT_MAX);
		for( int v = 0; v < polysize; ++v )
		{
			// write back normal
			P.m_vertices[ at + v ].nrm = normal;
			
			// generate lightmap poly
			Vec3 p = P.m_vertices[ at + v ].pos;
			Vec2 t = V2( Vec3Dot( edge_x, p ), Vec3Dot( edge_y, p ) ) * P.m_polylmq[ i ]   *   2; // magic constant for normal gap/polygon ratio
			tmin = Vec2::Min( tmin, t );
			tmax = Vec2::Max( tmax, t );
			P.m_lmverts.push_back( t );
		}
		for( int v = 0; v < polysize; ++v )
			P.m_lmverts[ at + v ] -= tmin;
		P.m_lmrects.push_back( tmax - tmin );
		P.m_lmallocs.push_back( -1 );
		
		at += polysize;
	}
}

bool LevelCache::_PackLightmapPolys( Mesh& M, int curwidth )
{
	RectPacker rp( curwidth, curwidth );
	for( size_t i = 0; i < M.m_parts.size(); ++i )
	{
		Part& P = M.m_parts[ i ];
		if( TexNoLight( P.m_texname ) )
			continue;
		for( size_t i = 0; i < P.m_lmrects.size(); ++i )
		{
			int szx = ceil( P.m_lmrects[ i ].x ) + 4; // padding
			int szy = ceil( P.m_lmrects[ i ].y ) + 4;
			int pos = rp.Alloc( szx, szy );
		//	LOG << "x = " << szx << " , y = " << szy << " , pos = " << pos;
			if( pos < 0 )
				return false;
			P.m_lmallocs[ i ] = pos;
		}
	}
	
	// if not returned here by now, fix coords
	float scale = 1.0f / curwidth;
	for( size_t i = 0; i < M.m_parts.size(); ++i )
	{
		size_t at = 0;
		Part& P = M.m_parts[ i ];
		for( size_t i = 0; i < P.m_polysizes.size(); ++i )
		{
			int polysize = P.m_polysizes[ i ];
			
			int off[2] = {0,0};
			rp.GetOffset( P.m_lmallocs[ i ], off );
			off[0] += 1; // use half the padding
			off[1] += 1;
			
			for( int v = 0; v < polysize; ++v )
			{
				Vertex& V = P.m_vertices[ at + v ];
				V.tx1 = ( P.m_lmverts[ at + v ].x + off[0] ) * scale;
				V.ty1 = ( P.m_lmverts[ at + v ].y + off[1] ) * scale;
			}
			
			at += polysize;
		}
	}
	
	return true;
}

void LevelCache::GenerateLightmapCoords( Mesh& M )
{
	// generate 2D polygons
	for( size_t i = 0; i < M.m_parts.size(); ++i )
		_GenerateLightmapPolys( M.m_parts[ i ] );
	
	// try to pack all polygons
	static const int widths[] = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
	for( size_t i = 0; i < sizeof(widths)/sizeof(int); ++i )
		if( _PackLightmapPolys( M, widths[ i ] ) )
			break;
}

struct PartRangeData
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	int part_id;
};

bool LevelCache::SaveMesh( int mid, Mesh& M, const StringView& path, bool remnull )
{
	Array< Vertex > verts;
	Array< uint16_t > indices;
	Array< PartRangeData > parts;
	
	for( size_t pid = 0; pid < M.m_parts.size(); ++pid )
	{
		const Part& P = M.m_parts[ pid ];
		if( remnull && TexNull( P.m_texname ) )
			continue;
		
		PartRangeData PRD = { verts.size(), P.m_vertices.size(), indices.size(), 0, pid };
		verts.append( P.m_vertices.data(), P.m_vertices.size() );
		
		size_t at = 0;
		for( size_t p = 0; p < P.m_polysizes.size(); ++p )
		{
			int psz = P.m_polysizes[ p ];
			PRD.indexCount += ( psz - 2 ) * 3;
			
			for( int i = 2; i < psz; ++i )
			{
				indices.push_back( at );
				indices.push_back( at + i - 1 );
				indices.push_back( at + i );
			}
			at += psz;
		}
		parts.push_back( PRD );
	}
	
	ByteArray ba;
	ByteWriter bw( &ba );
	
	bw.marker( "SS3DMESH" );
	uint32_t vu32 = 0; // mesh data flags
	bw << vu32;
	bw << M.m_boundsMin;
	bw << M.m_boundsMax;
	vu32 = verts.size_bytes(); // vertex buffer size
	bw << vu32;
	for( size_t i = 0; i < verts.size(); ++i )
		bw << verts[ i ];
	vu32 = indices.size_bytes(); // index buffer size
	bw << vu32;
	for( size_t i = 0; i < indices.size(); ++i )
		bw << indices[ i ];
#define EDMESH_VTX_FORMAT "pf3nf3cb40f21f2"
#define EDMESH_SHADER "default"
	uint8_t vu8 = sizeof(EDMESH_VTX_FORMAT) - 1; // vertex format length
	bw << vu8;
	bw.marker( EDMESH_VTX_FORMAT );
	vu8 = parts.size(); // mesh part count
	bw << vu8;
	for( size_t i = 0; i < parts.size(); ++i )
	{
		bw << parts[ i ].vertexOffset;
		bw << parts[ i ].vertexCount;
		bw << parts[ i ].indexOffset;
		bw << parts[ i ].indexCount;
		vu8 = 1; // texture count
		bw << vu8;
		vu8 = sizeof(EDMESH_SHADER) - 1; // shader name length
		bw << vu8;
		bw.marker( EDMESH_SHADER );
		LevelCache::Part& MP = M.m_parts[ parts[ i ].part_id ];
		vu8 = MP.m_texname.size(); // texture name length
		bw << vu8;
		bw.memory( MP.m_texname.data(), MP.m_texname.size() );
	}
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, sizeof(bfr), "%.*s/%d.ssm", TMIN( (int) path.size(), 200 ), path.data(), mid );
	return FS_SaveBinaryFile( bfr, ba.data(), ba.size() );
}

bool LevelCache::SaveCache( const StringView& path )
{
	FS_DirCreate( path );
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		GenerateLightmapCoords( m_meshes[ i ] );
	}
	
	RemoveHiddenSurfaces();
	GenerateLines();
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		SaveMesh( i, m_meshes[ i ], path, false );
	}
	
	GenerateLightmaps( path );
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		SaveMesh( i, m_meshes[ i ], path, true );
	}
	
	ByteArray ba;
	ByteWriter bw( &ba );
	
	bw.marker( "COMPILED" );
	SerializeVersionHelper<ByteWriter> svh( bw, LC_FILE_VERSION );
	
	svh.marker( "SCRENTS" );
	svh << m_scriptents;
	
	svh.marker( "INST" );
	svh << m_meshinst;
	
	svh.marker( "LINES" );
	svh << m_lines;
	
	svh.marker( "LIGHTS" );
	svh << m_lights;
	
	svh.marker( "SAMPLES" );
	
	ByteArray sample_data;
	if( !FS_LoadBinaryFile( String_Concat( path, "/samples" ), sample_data ) )
	{
		LOG_ERROR << "FAILED to read from /samples";
		return false;
	}
	svh.memory( sample_data.data(), sample_data.size() );
	
	return FS_SaveBinaryFile( String_Concat( path, "/cache" ), ba.data(), ba.size() );
}

static LC_MeshInst* lm_sort_res = NULL;
static int lm_sort_fn( const void* a, const void* b )
{
	LC_MeshInst& MA = lm_sort_res[ *(int*)a ];
	LC_MeshInst& MB = lm_sort_res[ *(int*)b ];
	return MA.m_meshname.compare_to( MB.m_meshname );
}

void LevelCache::GenerateLightmaps( const StringView& path )
{
	Array< int > instidcs;
	for( size_t i = 0; i < m_meshinst.size(); ++i )
	{
		if( ( m_meshinst[ i ].m_flags & LM_MESHINST_DYNLIT ) != 0 &&
			( m_meshinst[ i ].m_flags & LM_MESHINST_CASTLMS ) == 0 )
			continue;
		instidcs.push_back( i );
	}
	
	lm_sort_res = m_meshinst.data();
	qsort( instidcs.data(), instidcs.size(), sizeof(int), lm_sort_fn ); // sort by mesh name
	lm_sort_res = NULL;
	
	char bfr[ 65536 ];
	sgrx_snprintf( bfr, sizeof(bfr),
		"CONFIG ambient_color %g %g %g\n"
		"CONFIG clear_color %g %g %g\n"
		"CONFIG bounce_count %d\n"
		"CONFIG global_size_factor %g\n"
		"CONFIG blur_size %g\n"
		"CONFIG ao_distance %g\n"
		"CONFIG ao_multiplier %g\n"
		"CONFIG ao_falloff %g\n"
		"CONFIG ao_effect %g\n"
	//	"CONFIG ao_divergence %g\n"
		"CONFIG ao_color %g %g %g\n"
		"CONFIG ao_num_samples %d\n"
		"CONFIG samples_out %.*s/samples\n"
		"CONFIG texture_spec editor/textures.sgs\n"
		, AmbientColor.x, AmbientColor.y, AmbientColor.z
		, LightmapClearColor.x, LightmapClearColor.y, LightmapClearColor.z
		, RADNumBounces
		, LightmapDetail
		, LightmapBlurSize
		, AODistance
		, AOMultiplier
		, AOFalloff
		, AOEffect
	//	, AODivergence
		, AOColor.x, AOColor.y, AOColor.z
		, AONumSamples
		, TMIN( 260, (int) path.size() ), path.data()
	);
	
	String lm_cmds = bfr;
	StringView lastmesh;
	for( size_t i = 0; i < instidcs.size(); ++i )
	{
		const LC_MeshInst& MI = m_meshinst[ instidcs[ i ] ];
		if( lastmesh != MI.m_meshname )
		{
			lastmesh = MI.m_meshname;
			lm_cmds.append( "MESH " );
			if( lastmesh.ch() == '~' )
			{
				lm_cmds.append( path.data(), path.size() );
				lm_cmds.append( lastmesh.data() + 1, lastmesh.size() - 1 );
			}
			else
				lm_cmds.append( lastmesh.data(), lastmesh.size() );
			lm_cmds.append( "\n" );
		}
		sgrx_snprintf( bfr, sizeof(bfr), "INST lightmap %.*s/%d.png importance %g matrix  %g %g %g %g  %g %g %g %g  %g %g %g %g  END\n"
			, TMIN( 260, (int) path.size() ), path.data(), instidcs[ i ]
			, MI.lmquality * ( ( MI.m_flags & LM_MESHINST_DYNLIT ) ? 0 : 1 )  *  2.0f // magic factor
			, MI.m_mtx.m[0][0], MI.m_mtx.m[1][0], MI.m_mtx.m[2][0], MI.m_mtx.m[3][0]
			, MI.m_mtx.m[0][1], MI.m_mtx.m[1][1], MI.m_mtx.m[2][1], MI.m_mtx.m[3][1]
			, MI.m_mtx.m[0][2], MI.m_mtx.m[1][2], MI.m_mtx.m[2][2], MI.m_mtx.m[3][2] );
		lm_cmds.append( bfr );
	}
//	lm_cmds.append( "LIGHT POINT position 0.1 2.1 3 color 0.7 0.65 0.6 range 25.0 power 2.0 shadow_sample_count 4 END\n" );
//	lm_cmds.append( "LIGHT POINT position -0.1 -6 2.5 color 0.227 0.438 1 range 25.0 power 2.0 shadow_sample_count 4 END\n" );
	for( size_t i = 0; i < m_lights.size(); ++i )
	{
		char appbuf[ 10000 ]; // enough for 6 floats and more
		
		const LC_Light& L = m_lights[ i ];
		lm_cmds.append( "LIGHT " );
		if( L.type == LM_LIGHT_POINT )
			lm_cmds.append( "POINT" );
		else if( L.type == LM_LIGHT_SPOT )
			lm_cmds.append( "SPOT" );
		else if( L.type == LM_LIGHT_DIRECT )
			lm_cmds.append( "DIRECT" );
		else
			lm_cmds.append( "UNKNOWN" );
		
		if( L.type == LM_LIGHT_POINT || L.type == LM_LIGHT_SPOT )
		{
			sprintf( appbuf, " position %g %g %g power %g", L.pos.x, L.pos.y, L.pos.z, L.power );
			lm_cmds.append( appbuf );
		}
		if( L.type == LM_LIGHT_SPOT || L.type == LM_LIGHT_DIRECT )
		{
			sprintf( appbuf, " direction %g %g %g", L.dir.x, L.dir.y, L.dir.z );
			lm_cmds.append( appbuf );
		}
		if( L.type == LM_LIGHT_SPOT )
		{
			sprintf( appbuf, " up_direction %g %g %g spot_angle_out %g spot_angle_in %g spot_curve %g", L.up.x, L.up.y, L.up.z, L.outerangle, L.innerangle, L.spotcurve );
			lm_cmds.append( appbuf );
		}
		sprintf( appbuf, " range %g light_radius %g color %g %g %g shadow_sample_count %d END\n",
			L.range, L.light_radius, L.color.x, L.color.y, L.color.z, L.num_shadow_samples );
		lm_cmds.append( appbuf );
	}
	
	for( size_t i = 0; i < m_samples.size(); ++i )
	{
		char appbuf[ 10000 ];
		
		const Vec3& p = m_samples[ i ];
		sprintf( appbuf, "SAMPLE %g %g %g\n", p.x, p.y, p.z );
		lm_cmds.append( appbuf );
	}
	
	FS_SaveTextFile( "lmjob", lm_cmds );
	
	LOG << "Rendering lightmaps...";
	sgrx_snprintf( bfr, sizeof(bfr), "lmrender -i %.*s/lmjob", (int) Game_GetDir().size(), Game_GetDir().data() );
	LOG << "Command line: " << bfr;
	system( bfr );
	
	LOG << "\nFinished!";
}


