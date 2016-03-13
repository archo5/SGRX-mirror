

#include <float.h>

#include <engine.hpp>
#include "compiler.hpp"

#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>

#include <lighter/lighter.h>



static LTRBOOL _LMRenderer_SizeFunc(
	ltr_Config* config,
	const char* mesh_ident,
	size_t mesh_ident_size,
	const char* inst_ident,
	size_t inst_ident_size,
	float computed_surface_area,
	float inst_importance,
	u32 out_size[2] )
{
	SGRX_CAST( LMRenderer*, LMR, config->userdata );
	uint32_t lmid = 0;
	ASSERT( inst_ident_size == sizeof(uint32_t) );
	memcpy( &lmid, inst_ident, sizeof(uint32_t) );
	Vec2 size = LMR->m_lmsizes.getcopy( lmid, V2(0) ) * config->global_size_factor;
	if( size == V2(0) )
	{
		out_size[0] = 0;
		out_size[1] = 0;
	}
	else
	{
		out_size[0] = TMAX( 1, TMIN( 1024, int(size.x) ) );
		out_size[1] = TMAX( 1, TMIN( 1024, int(size.y) ) );
	}
	return 1;
}

LMRenderer::LMRenderer()
{
	rendered_sample_count = 0;
	rendered_lightmap_count = 0;
	rendered_samples = NULL;
	completion = 0;
	
	m_scene = ltr_CreateScene();
	
	ltr_Config cfg;
	ltr_GetConfig( &cfg, m_scene );
	cfg.userdata = this;
	cfg.size_fn = _LMRenderer_SizeFunc;
	ltr_SetConfig( m_scene, &cfg );
}

LMRenderer::~LMRenderer()
{
	ltr_DestroyScene( m_scene );
}

void LMRenderer::Start()
{
	// easy to remember order: X,Y,Z / +,-
	static const ltr_VEC3 sdirs[6] = { {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1} };
	for( size_t i = 0; i < sample_positions.size(); ++i )
	{
		ltr_SampleInfo SI;
		LTR_VEC3_SET( SI.out_color, 0, 0, 0 );
		for( int s = 0; s < 6; ++ s )
		{
			SI.id = i * 6 + s;
			memcpy( &SI.position, &sample_positions[ i ], sizeof(Vec3) );
			memcpy( &SI.normal, &sdirs[ s ], sizeof(ltr_VEC3) );
			ltr_SampleAdd( m_scene, &SI );
		}
	}
	
	ltr_Config cfg;
	ltr_GetConfig( &cfg, m_scene );
	cfg.global_size_factor = config.lightmapDetail;
	memcpy( cfg.clear_color, &config.lightmapClearColor, sizeof(Vec3) );
	cfg.ambient_color[0] = powf( config.ambientColor.x, 2.2f );
	cfg.ambient_color[1] = powf( config.ambientColor.y, 2.2f );
	cfg.ambient_color[2] = powf( config.ambientColor.z, 2.2f );
	cfg.ao_distance = config.aoDistance;
	cfg.ao_multiplier = config.aoMultiplier;
	cfg.ao_falloff = config.aoFalloff;
	cfg.ao_effect = config.aoEffect;
	cfg.ao_num_samples = config.aoNumSamples;
	cfg.blur_size = config.lightmapBlurSize;
	cfg.generate_normalmap_data = 1;
	ltr_SetConfig( m_scene, &cfg );
	
	ltr_Start( m_scene );
}

bool LMRenderer::CheckStatus()
{
	ltr_WorkStatus wstatus;
	if( ltr_GetStatus( m_scene, &wstatus ) )
	{
		stage = wstatus.stage;
		completion = wstatus.completion;
		return false;
	}
	
	// completed
	ltr_WorkOutputInfo woutinfo;
	ltr_GetWorkOutputInfo( m_scene, &woutinfo );
	rendered_sample_count = woutinfo.sample_count / 6;
	rendered_lightmap_count = woutinfo.lightmap_count;
	rendered_samples = woutinfo.samples;
	
	return true;
}

bool LMRenderer::GetLightmap( uint32_t which, Array< Vec3 >& outcols,
	Array< Vec4 >& outxyzf, uint32_t outlmidsize[3] )
{
	if( which >= rendered_lightmap_count )
		return false;
	
	ltr_WorkOutput wout;
	if( ltr_GetWorkOutput( m_scene, which, &wout ) == 0 )
		return false;
	
	ASSERT( wout.inst_ident_size == 4 );
	memcpy( &outlmidsize[0], wout.inst_ident, sizeof(uint32_t) );
	outlmidsize[1] = wout.width;
	outlmidsize[2] = wout.height;
	outcols.resize( wout.width * wout.height );
	memcpy( outcols.data(), wout.lightmap_rgb, outcols.size_bytes() );
	outxyzf.resize( wout.width * wout.height );
	memcpy( outxyzf.data(), wout.normals_xyzf, outxyzf.size_bytes() );
	
	return true;
}

bool LMRenderer::GetSample( uint32_t which, Vec3 outcols[6] )
{
	if( which >= rendered_sample_count )
		return false;
	
	for( int i = 0; i < 6; ++i )
	{
		memcpy( &outcols[ i ], rendered_samples[ which * 6 + i ].out_color, sizeof(Vec3) );
	}
	return true;
}

bool LMRenderer::AddMeshInst( SGRX_MeshInstance* MI, const Vec2& lmsize, uint32_t lmid, bool solid )
{
	if( MI->GetMesh() == NULL )
		return false;
	
	SGRX_IMesh* M = MI->GetMesh();
	Mesh*& mesh = m_meshes[ M ];
	if( mesh == NULL )
	{
		mesh = new Mesh;
		mesh->ltrMesh = ltr_CreateMesh( m_scene, M->m_key.data(), M->m_key.size() );
		
		VDeclInfo vertex_decl = M->m_vertexDecl.GetInfo();
		int p_off = -1, n_off = -1, t0_off = -1, t1_off = -1;
		for( int i = 0; i < (int) vertex_decl.count; ++i )
		{
			if( vertex_decl.usages[ i ] == VDECLUSAGE_POSITION )
			{
				if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT3 )
				{
					LOG_ERROR << "MESH position data not FLOAT[3] (file: " << M->m_key << ")";
					return false;
				}
				p_off = vertex_decl.offsets[ i ];
			}
			else if( vertex_decl.usages[ i ] == VDECLUSAGE_NORMAL )
			{
				if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT3 )
				{
					LOG_ERROR << "MESH normal data not FLOAT[3] (file: " << M->m_key << ")";
					return false;
				}
				n_off = vertex_decl.offsets[ i ];
			}
			else if( vertex_decl.usages[ i ] == VDECLUSAGE_TEXTURE0 )
			{
				if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT2 )
				{
					LOG_ERROR << "MESH texcoord_0 data not FLOAT[2] (file: " << M->m_key << ")";
					return false;
				}
				t0_off = vertex_decl.offsets[ i ];
			}
			else if( vertex_decl.usages[ i ] == VDECLUSAGE_TEXTURE1 )
			{
				if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT2 )
				{
					LOG_ERROR << "MESH texcoord_1 data not FLOAT[2] (file: " << M->m_key << ")";
					return false;
				}
				t1_off = vertex_decl.offsets[ i ];
			}
		}
		if( p_off < 0 )
		{
			LOG_ERROR << "MESH has no position data (file: " << M->m_key << ")";
			return false;
		}
		if( n_off < 0 )
		{
			LOG_ERROR << "MESH has no normal data (file: " << M->m_key << ")";
			return false;
		}
		if( t0_off < 0 && t1_off < 0 )
		{
			LOG_ERROR << "MESH has no texture coordinate data (file: " << M->m_key << ")";
			return false;
		}
		if( t0_off < 0 ) t0_off = t1_off;
		else if( t1_off < 0 ) t1_off = t0_off;
		
		// TODO COPY PNTT data
		uint32_t vcount = M->m_vdata.size() / vertex_decl.size;
		mesh->positions.resize( vcount );
		mesh->normals.resize( vcount );
		mesh->texcoords0.resize( vcount );
		mesh->texcoords1.resize( vcount );
		for( uint32_t i = 0; i < vcount; ++i )
		{
			memcpy( &mesh->positions[ i ], &M->m_vdata[ vertex_decl.size * i + p_off ], sizeof(Vec3) );
			memcpy( &mesh->normals[ i ], &M->m_vdata[ vertex_decl.size * i + n_off ], sizeof(Vec3) );
			memcpy( &mesh->texcoords0[ i ], &M->m_vdata[ vertex_decl.size * i + t0_off ], sizeof(Vec2) );
			memcpy( &mesh->texcoords1[ i ], &M->m_vdata[ vertex_decl.size * i + t1_off ], sizeof(Vec2) );
		}
		
		if( M->m_dataFlags & MDF_INDEX_32 )
		{
			mesh->indices.resize( M->m_idata.size() / sizeof(uint32_t) );
			memcpy( mesh->indices.data(), M->m_idata.data(), mesh->indices.size_bytes() );
		}
		else
		{
			mesh->indices.resize( M->m_idata.size() / sizeof(uint16_t) );
			ByteReader br( M->m_idata );
			for( size_t i = 0; i < mesh->indices.size(); ++i )
			{
				uint16_t idx = 0;
				br << idx;
				mesh->indices[ i ] = idx;
			}
		}
		for( size_t i = 0; i + 2 < mesh->indices.size(); i += 3 )
			TSWAP( mesh->indices[ i + 1 ], mesh->indices[ i + 2 ] );
		
		for( size_t mpid = 0; mpid < M->m_meshParts.size(); ++mpid )
		{
			SGRX_MeshPart& MP = M->m_meshParts[ mpid ];
			ltr_MeshPartInfo mpinfo =
			{
				&mesh->positions[ MP.vertexOffset ],
				&mesh->normals[ MP.vertexOffset ],
				&mesh->texcoords0[ MP.vertexOffset ],
				&mesh->texcoords1[ MP.vertexOffset ],
				sizeof(Vec3), sizeof(Vec3), sizeof(Vec2), sizeof(Vec2),
				&mesh->indices[ MP.indexOffset ], MP.vertexCount, MP.indexCount,
				1
			};
			ltr_MeshAddPart( mesh->ltrMesh, &mpinfo );
		}
	}
	
	ltr_MeshInstanceInfo mi_info;
	memcpy( mi_info.matrix, &MI->matrix, sizeof(float)*16 );
	mi_info.importance = 1;
	mi_info.shadow = solid;
	mi_info.ident = (char*) &lmid;
	mi_info.ident_size = sizeof(lmid);
	ltr_MeshAddInstance( mesh->ltrMesh, &mi_info );
	
	m_lmsizes.set( lmid, lmsize );
	
	return true;
}

bool LMRenderer::AddLight( const LC_Light& light )
{
	ltr_LightInfo light_info;
	
	if( light.type == LM_LIGHT_POINT )
		light_info.type = LTR_LT_POINT;
	else if( light.type == LM_LIGHT_SPOT )
		light_info.type = LTR_LT_SPOT;
	else if( light.type == LM_LIGHT_DIRECT )
		light_info.type = LTR_LT_DIRECT;
	else return false;
	
	memcpy( light_info.position, &light.pos, sizeof(Vec3) );
	memcpy( light_info.direction, &light.dir, sizeof(Vec3) );
	memcpy( light_info.up_direction, &light.up, sizeof(Vec3) );
	memcpy( light_info.color_rgb, &light.color, sizeof(Vec3) );
	light_info.range = light.range;
	light_info.power = light.power;
	light_info.light_radius = light.light_radius;
	light_info.shadow_sample_count = light.num_shadow_samples;
	light_info.spot_angle_out = light.outerangle * 0.5f;
	light_info.spot_angle_in = light.innerangle * 0.5f;
	light_info.spot_curve = light.spotcurve;
	
	ltr_LightAdd( m_scene, &light_info );
	
	return true;
}



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


static const int g_lmsizes[][2] =
{
	{ 64, 64 },
	{ 128, 64 },
	{ 128, 128 },
	{ 256, 128 },
	{ 256, 256 },
	{ 512, 256 },
	{ 512, 512 },
	{ 1024, 512 },
	{ 1024, 1024 },
	{ 2048, 1024 },
	{ 2048, 2048 },
	{0,0},
};

LMRectPacker::LMRectPacker() : m_sizeid(0)
{
	Init( g_lmsizes[ m_sizeid ][0], g_lmsizes[ m_sizeid ][1] );
}

int LMRectPacker::LMAlloc( int w, int h )
{
	int id = Alloc( w, h );
	int szid = m_sizeid + 1;
	while( id == -1 && g_lmsizes[ szid ][0] != 0 )
	{
		// prepare for creating new packing
		m_tmpAllocs.resize( m_allocs.size() );
		RectPacker RP( g_lmsizes[ szid ][0], g_lmsizes[ szid ][1] );
		
		// try to copy all existing nodes
		bool copied = true;
		for( size_t i = 0; i < m_allocs.size(); ++i )
		{
			Node& N = m_tree[ m_allocs[ i ] ];
			int nid = RP.Alloc( N.x1 - N.x0, N.y1 - N.y0 );
			if( nid == -1 )
			{
				copied = false;
				break;
			}
			else
			{
				m_tmpAllocs[ i ] = nid;
			}
		}
		
		// try to add new node
		if( copied )
		{
			id = RP.Alloc( w, h );
			if( id != -1 )
			{
				m_allocs = m_tmpAllocs;
				m_tree = RP.m_tree;
				m_sizeid = szid;
				break;
			}
		}
		szid++;
	}
	
	if( id != -1 )
	{
		m_allocs.push_back( id );
		return m_allocs.size() - 1;
	}
	return -1;
}


inline int32_t divideup( int32_t x, int d ){ return ( x + d - 1 ) / d; }

VoxelBlock::VoxelBlock( Vec3 bbmin, Vec3 bbmax, float stepsize )
{
	m_stepsize = stepsize;
	Vec3 bbsize = bbmax - bbmin;
	m_xsize = ceilf( bbsize.x / stepsize );
	m_ysize = ceilf( bbsize.y / stepsize );
	m_zsize = ceilf( bbsize.z / stepsize );
	m_xbcnt = divideup( m_xsize, 4 );
	m_ybcnt = divideup( m_ysize, 4 );
	m_zbcnt = divideup( m_zsize, 4 );
	int32_t total = m_xbcnt * m_ybcnt * m_zbcnt;
	m_data = new uint64_t[ total ];
	TMEMSET( m_data, total, uint64_t(0) );
	m_bbmin = bbmin;
	// avoid conversion issues:
	m_bbmax = bbmin + V3( m_xsize, m_ysize, m_zsize ) * stepsize;
}

void VoxelBlock::RasterizeTriangle( Vec3 p1, Vec3 p2, Vec3 p3 )
{
	int32_t cobb[6];
	Vec3 bbmin = Vec3::Min( p1, Vec3::Min( p2, p3 ) );
	Vec3 bbmax = Vec3::Max( p1, Vec3::Max( p2, p3 ) );
	if( !_FindAABB( bbmin, bbmax, cobb ) )
		return;
	
	Vec3 vbext = V3( m_stepsize * 0.5f );
	for( int32_t z = cobb[2]; z <= cobb[5]; ++z )
	{
		for( int32_t y = cobb[1]; y <= cobb[4]; ++y )
		{
			for( int32_t x = cobb[0]; x <= cobb[3]; ++x )
			{
				Vec3 vbpos = GetPosition( x, y, z );
				if( TriangleAABBIntersect( p1, p2, p3, vbpos - vbext, vbpos + vbext ) )
				{
					Set1( x, y, z );
				}
			}
		}
	}
}

int32_t VoxelBlock::RasterizeSolid( Vec4* planes, size_t count )
{
	int32_t numhit = 0;
	// TOOD MAYBE optimize
	for( int32_t z = 0; z <= m_zsize; ++z )
	{
		for( int32_t y = 0; y <= m_ysize; ++y )
		{
			for( int32_t x = 0; x <= m_xsize; ++x )
			{
				Vec3 vbpos = GetPosition( x, y, z );
				size_t i;
				for( i = 0; i < count; ++i )
					if( Vec3Dot( -planes[ i ].ToVec3(), vbpos ) > -planes[ i ].w + SMALL_FLOAT )
						break;
				if( i == count )
				{
					Set1( x, y, z );
					numhit++;
				}
			}
		}
	}
	return numhit;
}

Vec3 VoxelBlock::GetPosition( int32_t x, int32_t y, int32_t z )
{
	Vec3 q =
	{
		safe_fdiv( x + 0.5f, m_xsize ),
		safe_fdiv( y + 0.5f, m_ysize ),
		safe_fdiv( z + 0.5f, m_zsize )
	};
	return TLERP( m_bbmin, m_bbmax, q );
}

bool VoxelBlock::Get( int32_t x, int32_t y, int32_t z )
{
	uint64_t* blk;
	int bit;
	if( !_FindBlock( x, y, z, &blk, &bit ) )
		return false;
	return ( *blk & ( uint64_t(1) << bit ) ) != 0;
}

void VoxelBlock::Set1( int32_t x, int32_t y, int32_t z )
{
	uint64_t* blk;
	int bit;
	if( !_FindBlock( x, y, z, &blk, &bit ) )
		return;
	*blk |= uint64_t(1) << bit;
}

void VoxelBlock::Set0( int32_t x, int32_t y, int32_t z )
{
	uint64_t* blk;
	int bit;
	if( !_FindBlock( x, y, z, &blk, &bit ) )
		return;
	*blk &= ~( uint64_t(1) << bit );
}

bool VoxelBlock::_FindBlock( int32_t x, int32_t y, int32_t z, uint64_t** outblk, int* outbit )
{
	if( x < 0 || y < 0 || z < 0 ||
		x >= m_xsize || y >= m_ysize || z >= m_zsize )
		return false;
	int32_t bx = x / 4, by = y / 4, bz = z / 4;
	x -= bx * 4;
	y -= by * 4;
	z -= bz * 4;
	*outblk = m_data + ( bx + by * m_xbcnt + bz * m_xbcnt * m_ybcnt );
	*outbit = x + y * 4 + z * 4 * 4;
	return true;
}

bool VoxelBlock::_FindAABB( Vec3 bbmin, Vec3 bbmax, int32_t outbb[6] )
{
	if( bbmin.x > m_bbmax.x || bbmin.y > m_bbmax.y || bbmin.z > m_bbmax.z ||
		bbmax.x < m_bbmin.x || bbmax.y < m_bbmin.y || bbmax.z < m_bbmin.z )
		return false;
	_PosToCoord( bbmin, outbb + 0 );
	_PosToCoord( bbmax, outbb + 3 );
	outbb[0] = TMAX( 0, TMIN( m_xsize - 1, outbb[0] ) );
	outbb[1] = TMAX( 0, TMIN( m_ysize - 1, outbb[1] ) );
	outbb[2] = TMAX( 0, TMIN( m_zsize - 1, outbb[2] ) );
	outbb[3] = TMAX( 0, TMIN( m_xsize - 1, outbb[3] ) );
	outbb[4] = TMAX( 0, TMIN( m_ysize - 1, outbb[4] ) );
	outbb[5] = TMAX( 0, TMIN( m_zsize - 1, outbb[5] ) );
	return true;
}

void VoxelBlock::_PosToCoord( Vec3 p, int32_t outco[3] )
{
	outco[0] = TREVLERP<float>( m_bbmin.x, m_bbmax.x, p.x ) * m_xsize;
	outco[1] = TREVLERP<float>( m_bbmin.y, m_bbmax.y, p.y ) * m_ysize;
	outco[2] = TREVLERP<float>( m_bbmin.z, m_bbmax.z, p.z ) * m_zsize;
}


LevelCache::LevelCache( SGRX_LightTree* sampleTree ) : m_sampleTree( sampleTree )
{
}


#define LINE_HEIGHT 0.45f

void LevelCache::AddPart( const Vertex* verts, int vcount, LC_Lightmap& lm,
	const StringView& mtlname, size_t fromsolid, uint32_t flags, int decalLayer )
{
	ASSERT( vcount >= 3 && vcount % 3 == 0 );
	
	m_meshParts.push_back( Part() );
	Part& P = m_meshParts.last();
	
	for( int i = 0; i + 2 < vcount; i += 3 )
	{
		if( TriangleArea( verts[ i+0 ].pos, verts[ i+1 ].pos, verts[ i+2 ].pos ) < 0.0000001f )
			continue;
		P.m_vertices.append( &verts[ i ], 3 );
	}
	if( P.m_vertices.size() == 0 )
	{
		m_meshParts.pop_back();
		return;
	}
	
	P.m_solid = fromsolid;
	P.m_mtlname = mtlname;
	P.m_lightmap = lm;
	P.m_lmalloc = -1;
	P.m_flags = flags;
	P.m_decalLayer = decalLayer;
}

size_t LevelCache::AddSolid( const Vec4* planes, int count )
{
	m_solids.push_back( Solid() );
	Solid& S = m_solids.last();
	S.append( planes, count );
	return m_solids.size() - 1;
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
			Vertex IPV = PPV.Interpolate( PP[ p1 ], safe_fdiv( 0 - sigdist, sigdist2 - sigdist ) );
			if( sigdist2 > SMALL_FLOAT )
			{
				PPB.push_back( IPV );
				PPF.push_back( IPV );
			}
		}
		else if( sigdist > SMALL_FLOAT )
		{
			PPF.push_back( PPV );
			Vertex IPV = PPV.Interpolate( PP[ p1 ], safe_fdiv( 0 - sigdist, sigdist2 - sigdist ) );
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
	
	for( size_t p = 0; p < m_meshParts.size(); ++p )
	{
		Part& P = m_meshParts[ p ];
		
		// load the polygons
		m_polies.clear();
		for( size_t v = 0; v + 2 < P.m_vertices.size(); v += 3 )
		{
			PPstage.assign( &P.m_vertices[ v ], 3 );
			PPstage.m_solid = P.m_solid;
			m_polies.push_back( PPstage );
		}
		
	//	LOG << "CSG mesh " << m << " plane " << p << " in: " << m_polies.size();
		
		// cut polygons
		for( size_t s = 0; s < m_solids.size() && m_polies.size(); ++s )
		{
			m_cutpolies.clear();
			const Solid& S = m_solids[ s ];
			if( S.size() == 0 )
				continue;
			
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
		
		for( size_t cp = 0; cp < m_polies.size(); ++cp )
		{
			PartPoly& PP = m_polies[ cp ];
			for( size_t cpv = 1; cpv + 1 < PP.size(); ++cpv )
			{
				P.m_vertices.push_back( PP[ 0 ] );
				P.m_vertices.push_back( PP[ cpv ] );
				P.m_vertices.push_back( PP[ cpv + 1 ] );
			}
		}
	}
}

static int map_layer_height_sort( const void* a, const void* b )
{
	SGRX_CAST( LC_Map_Layer*, A, a );
	SGRX_CAST( LC_Map_Layer*, B, b );
	return A->height == B->height ? 0 : ( A->height < B->height ? -1 : 1 );
}

void LevelCache::GenerateLines()
{
	qsort( m_mapLayers.data(), m_mapLayers.size(), sizeof(LC_Map_Layer), map_layer_height_sort );
	for( size_t layer_id = 0; layer_id < m_mapLayers.size(); ++layer_id )
	{
		m_mapLayers[ layer_id ].from = m_mapLines.size();
		float height = m_mapLayers[ layer_id ].height;
		for( size_t p = 0; p < m_meshParts.size(); ++p )
		{
			Part& P = m_meshParts[ p ];
			
			for( size_t mpv = 0; mpv + 2 < P.m_vertices.size(); mpv += 3 )
			{
				Vertex* verts = &P.m_vertices[ mpv ];
				int vcount = 3;
				
				Vec3 nrm = Vec3Cross( verts[1].pos - verts[0].pos, verts[2].pos - verts[0].pos ).Normalized();
				if( fabsf( Vec3Dot( nrm, V3(0,0,1) ) ) > cosf(FLT_PI/4.0f) )
				{
					// surface is too flat
					continue;
				}
				
				// ADD LINES
				Vec3 isps[ 2 ];
				int ispc = 0;
				for( int i = 0; i < vcount && ispc < 2; ++i )
				{
					Vec3 v0 = verts[ i ].pos;
					Vec3 v1 = verts[ ( i + 1 ) % vcount ].pos;
					if( ( v0.z <= height && v1.z >= height ) ||
						( v0.z >= height && v1.z <= height ) )
					{
						if( v1.z == v0.z )
						{
							break; // going to be easier (less code dup) to insert these separately (polygons are closed)
						}
						else
						{
							Vec3 isp = TLERP( v0, v1, ( height - v0.z ) / ( v1.z - v0.z ) );
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
					m_mapLines.append( line, 2 );
				}
			}
		}
		m_mapLayers[ layer_id ].to = m_mapLines.size();
	}
}


static int sort_part_by_texture( const void* a, const void* b )
{
	SGRX_CAST( LevelCache::Part*, pa, a );
	SGRX_CAST( LevelCache::Part*, pb, b );
	return pa->m_mtlname.compare_to( pb->m_mtlname );
}

void LevelCache::GatherMeshes()
{
	qsort( m_meshParts.data(), m_meshParts.size(), sizeof(Part), sort_part_by_texture );
	
	for( size_t pid = 0; pid < m_meshParts.size(); ++pid )
	{
		Part& P = m_meshParts[ pid ];
		if( P.m_mtlname != SV("clip") )
		{
			// mesh combination
			Vec3 pcenter = V3(0), pmin = V3(FLT_MAX), pmax = V3(-FLT_MAX);
			for( size_t v = 0; v < P.m_vertices.size(); ++v )
			{
				Vec3 p = P.m_vertices[ v ].pos;
				pcenter += p;
				pmin = Vec3::Min( pmin, p );
				pmax = Vec3::Max( pmax, p );
			}
			pcenter /= P.m_vertices.size();
			
			Mesh* TM = NULL;
			int lmalloc = -1;
			
			for( size_t mid = 0; mid < m_meshes.size(); ++mid )
			{
				Mesh& M = m_meshes[ mid ];
				Part& OP = m_meshParts[ M.m_partIDs[ 0 ] ];
				
				// do not group dynlit meshes
				if( ( OP.m_flags | P.m_flags ) & LM_MESHINST_DYNLIT )
					continue;
				
				// wrong decal layer ID
				if( OP.m_decalLayer != P.m_decalLayer )
					continue;
				
				// not the same lightmap status (exists or not)
				if( ( OP.m_lightmap.width != 0 && OP.m_lightmap.height != 0 ) !=
					( P.m_lightmap.width != 0 && P.m_lightmap.height != 0 ) )
					continue;
				
				// no more space
				if( M.m_mtlnames.size() >= 8 && M.m_mtlnames.find_first_at( P.m_mtlname ) == NOT_FOUND )
					continue;
				
				// too far
				Vec3 curpos = M.m_pos / M.m_div;
				if( ( curpos - pcenter ).Length() > 20 )
					continue;
				
				if( P.m_lightmap.width && P.m_lightmap.height )
				{
					lmalloc = M.m_packer.LMAlloc( P.m_lightmap.width + 2, P.m_lightmap.height + 2 );
				//	printf( "lmalloc %d\n", lmalloc );
					if( lmalloc == -1 )
						continue;
				}
				
				TM = &m_meshes[ mid ];
				P.m_lmalloc = lmalloc;
				break;
			}
			
			if( !TM )
			{
				m_meshes.push_back( Mesh() );
				TM = &m_meshes.last();
				TM->m_pos = V3(0);
				TM->m_div = 0;
				TM->m_boundsMin = V3(FLT_MAX);
				TM->m_boundsMax = V3(-FLT_MAX);
				lmalloc = -1;
				if( P.m_lightmap.width && P.m_lightmap.height )
				{
					lmalloc = TM->m_packer.LMAlloc( P.m_lightmap.width + 2, P.m_lightmap.height + 2 );
					if( lmalloc == -1 )
						LOG_WARNING << "Could not allocate lightmap!!!";
					ASSERT( lmalloc != -1 );
				}
				P.m_lmalloc = lmalloc;
			}
			
			TM->m_pos += pcenter;
			TM->m_div++;
			TM->m_mtlnames.find_or_add( P.m_mtlname );
			TM->m_partIDs.push_back( pid );
			TM->m_boundsMin = Vec3::Min( TM->m_boundsMin, pmin );
			TM->m_boundsMax = Vec3::Max( TM->m_boundsMax, pmax );
		}
		
		// physics mesh generation
		if( P.m_flags & LM_MESHINST_SOLID )
		{
			uint32_t phy_mtl_id = m_phyMesh.materials.find_or_add( P.m_mtlname );
			
			for( size_t v = 0; v + 2 < P.m_vertices.size(); v += 3 )
			{
				m_phyMesh.indices.push_back( m_phyMesh.positions.find_or_add( P.m_vertices[ v + 0 ].pos ) );
				m_phyMesh.indices.push_back( m_phyMesh.positions.find_or_add( P.m_vertices[ v + 2 ].pos ) );
				m_phyMesh.indices.push_back( m_phyMesh.positions.find_or_add( P.m_vertices[ v + 1 ].pos ) );
				m_phyMesh.indices.push_back( phy_mtl_id );
			}
		}
		
		// navmesh generation
		if( ( P.m_flags & LM_MESHINST_SOLID ) && P.m_mtlname != SV("black") )
		{
			uint32_t nav_mtl_id = m_navMesh.materials.find_or_add( P.m_mtlname );
			
			for( size_t v = 0; v + 2 < P.m_vertices.size(); v += 3 )
			{
				m_navMesh.indices.push_back( m_navMesh.positions.find_or_add( P.m_vertices[ v + 0 ].pos ) );
				m_navMesh.indices.push_back( m_navMesh.positions.find_or_add( P.m_vertices[ v + 2 ].pos ) );
				m_navMesh.indices.push_back( m_navMesh.positions.find_or_add( P.m_vertices[ v + 1 ].pos ) );
				m_navMesh.indices.push_back( nav_mtl_id );
			}
		}
	}
}

struct PartRangeData
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	int part_id;
};

static void LMCOPY( LC_Lightmap& dst, size_t dstoff, const LC_Lightmap& src, size_t srcoff, size_t pxcount )
{
	memcpy( &dst.data[ dstoff ], &src.data[ srcoff ], pxcount * sizeof(uint32_t) );
	memcpy( &dst.nmdata[ dstoff ], &src.nmdata[ srcoff ], pxcount * sizeof(uint32_t) );
}

bool LevelCache::SaveMesh( MapMaterialMap& mtls, int mid, Mesh& M, const StringView& path )
{
	Array< Vertex > verts;
	Array< uint16_t > indices;
	Array< PartRangeData > parts;
	
	bool needlm = false;
	for( size_t pid = 0; pid < M.m_partIDs.size(); ++pid )
		if( ( needlm = ( m_meshParts[ M.m_partIDs[ pid ] ].m_lmalloc != -1 ) ) )
			break;
	
	LC_Lightmap lightmap = { 0, 0 };
	if( needlm ) // all parts should have the same LM status
	{
		lightmap.width = M.m_packer.m_tree[0].x1;
		lightmap.height = M.m_packer.m_tree[0].y1;
		lightmap.data.resize_using( lightmap.width * lightmap.height, 0 );
		lightmap.nmdata.resize_using( lightmap.width * lightmap.height, COLOR_RGBA(128,128,128,0) );
	}
	
	for( size_t pid = 0; pid < M.m_partIDs.size(); )
	{
		const Part& P = m_meshParts[ M.m_partIDs[ pid ] ];
		
		PartRangeData PRD = { verts.size(), 0, indices.size(), 0, M.m_partIDs[ pid ] };
		// the sorting of paths allows us to concatenate them
		StringView mtlName = P.m_mtlname;
		while( pid < M.m_partIDs.size() && m_meshParts[ M.m_partIDs[ pid ] ].m_mtlname == mtlName )
		{
			const Part& SOP = m_meshParts[ M.m_partIDs[ pid ] ];
			
			// copy lightmap
			int lmoff[2] = {0,0};
			bool haslm = M.m_packer.GetOffset( SOP.m_lmalloc, lmoff );
		//	printf( "mid %d pid %d alloc %d haslm %d lmoff %d %d size %d %d\n",
		//		mid, pid, SOP.m_lmalloc, int(haslm), lmoff[0], lmoff[1],
		//		int(SOP.m_lightmap.width), int(SOP.m_lightmap.height) );
			Vec2 coords_min = V2(0);
			Vec2 coords_max = V2(0);
			if( haslm )
			{
				ASSERT( lightmap.width != 0 && lightmap.height != 0 );
				int dstw = lightmap.width;
				int srcw = SOP.m_lightmap.width;
				int srch = SOP.m_lightmap.height;
				for( uint16_t y = 0; y < SOP.m_lightmap.height; ++y )
				{
					// row padding - left side
					LMCOPY( lightmap, ( y + lmoff[1] + 1 ) * dstw + lmoff[0],
						SOP.m_lightmap, y * srcw, 1 );
					// main row data
					LMCOPY(
						lightmap, ( y + lmoff[1] + 1 ) * dstw + lmoff[0] + 1,
						SOP.m_lightmap, y * srcw,
						srcw
					);
					// row padding - right side
					LMCOPY( lightmap, ( y + lmoff[1] + 1 ) * dstw + lmoff[0] + 1 + srcw,
						SOP.m_lightmap, y * srcw + srcw - 1, 1 );
				}
				// row padding - copy top back
				LMCOPY(
					lightmap, ( lmoff[1] ) * dstw + lmoff[0],
					lightmap, ( lmoff[1] + 1 ) * dstw + lmoff[0],
					srcw + 2
				);
				// row padding - copy bottom forward
				LMCOPY(
					lightmap, ( lmoff[1] + 1 + srch ) * dstw + lmoff[0],
					lightmap, ( lmoff[1] + srch ) * dstw + lmoff[0],
					srcw + 2
				);
				// generate coords
				coords_min = V2(
					( lmoff[0] + 1.0f ) / float(lightmap.width),
					( lmoff[1] + 1.0f ) / float(lightmap.height)
				);
				coords_max = V2(
					( lmoff[0] + srcw + 1.0f ) / float(lightmap.width),
					( lmoff[1] + srch + 1.0f ) / float(lightmap.height)
				);
			}
			
			// copy vertex/index data
			for( size_t v = 0; v + 2 < SOP.m_vertices.size(); v += 3 )
			{
				Vertex V0 = SOP.m_vertices[ v + 0 ].InterpLMCoords( coords_min, coords_max );
				Vertex V1 = SOP.m_vertices[ v + 1 ].InterpLMCoords( coords_min, coords_max );
				Vertex V2 = SOP.m_vertices[ v + 2 ].InterpLMCoords( coords_min, coords_max );
				V0.pos -= M.m_pos;
				V1.pos -= M.m_pos;
				V2.pos -= M.m_pos;
				indices.push_back( verts.find_or_add( V0, PRD.vertexOffset ) - PRD.vertexOffset );
				indices.push_back( verts.find_or_add( V1, PRD.vertexOffset ) - PRD.vertexOffset );
				indices.push_back( verts.find_or_add( V2, PRD.vertexOffset ) - PRD.vertexOffset );
			}
			pid++;
		}
		PRD.vertexCount = verts.size() - PRD.vertexOffset;
		PRD.indexCount = indices.size() - PRD.indexOffset;
		parts.push_back( PRD );
	}
	
	ByteArray ba;
	ByteWriter bw( &ba );
	
	bw.marker( "SS3DMESH" );
	uint32_t vu32 = MDF_MTLINFO; // mesh data flags
	bw << vu32;
	Vec3 v3 = M.m_boundsMin - M.m_pos;
	bw << v3;
	v3 = M.m_boundsMax - M.m_pos;
	bw << v3;
	vu32 = verts.size_bytes(); // vertex buffer size
	bw << vu32;
	for( size_t i = 0; i < verts.size(); ++i )
		bw << verts[ i ];
	vu32 = indices.size_bytes(); // index buffer size
	bw << vu32;
	for( size_t i = 0; i < indices.size(); ++i )
		bw << indices[ i ];
#define EDMESH_VTX_FORMAT LCVertex_DECL
#define EDMESH_SHADER "default"
	uint8_t vu8 = sizeof(EDMESH_VTX_FORMAT) - 1; // vertex format length
	bw << vu8;
	bw.marker( EDMESH_VTX_FORMAT );
	vu8 = parts.size(); // mesh part count
	bw << vu8;
	for( size_t i = 0; i < parts.size(); ++i )
	{
		LevelCache::Part& MP = m_meshParts[ parts[ i ].part_id ];
		
		StringView tn = MP.m_mtlname;
		MapMaterial* mmtl = mtls.getcopy( tn );
		if( mmtl )
		{
			vu8 = mmtl->flags; bw << vu8;
			vu8 = mmtl->blendmode; bw << vu8;
			bw << parts[ i ].vertexOffset;
			bw << parts[ i ].vertexCount;
			bw << parts[ i ].indexOffset;
			bw << parts[ i ].indexCount;
			vu8 = mmtl->texcount;
			bw << vu8;
			vu8 = TMIN( mmtl->shader.size(), size_t(255) );
			bw << vu8;
			bw.memory( mmtl->shader.data(), vu8 );
			for( int i = 0; i < mmtl->texcount; ++i )
			{
				vu8 = TMIN( mmtl->texture[ i ].size(), size_t(255) );
				bw << vu8;
				bw.memory( mmtl->texture[ i ].data(), vu8 );
			}
		}
		else
		{
			vu8 = 0;
			bw << vu8 << vu8; // material flags, blend mode
			bw << parts[ i ].vertexOffset;
			bw << parts[ i ].vertexCount;
			bw << parts[ i ].indexOffset;
			bw << parts[ i ].indexCount;
			vu8 = 1; // texture count
			bw << vu8;
			vu8 = sizeof(EDMESH_SHADER) - 1; // shader name length
			bw << vu8;
			bw.marker( EDMESH_SHADER );
			vu8 = MP.m_mtlname.size() + 13; // texture name length
			bw << vu8;
			char pfx[] = "textures/";
			char sfx[] = ".png";
			bw.memory( pfx, 9 );
			bw.memory( MP.m_mtlname.data(), MP.m_mtlname.size() );
			bw.memory( sfx, 4 );
		}
	}
	
	// add instance for new mesh
	{
		const Part& P = m_meshParts[ M.m_partIDs[ 0 ] ];
		char bfr[ 32 ];
		sgrx_snprintf( bfr, sizeof(bfr), "~/%d.ssm", mid );
		AddMeshInst( bfr, Mat4::CreateTranslation( M.m_pos ),
			P.m_flags & ~LM_MESHINST_SOLID, P.m_decalLayer, lightmap );
	}
	
	char bfr[ 256 ];
	sgrx_snprintf( bfr, sizeof(bfr), "%.*s/%d.ssm", TMIN( (int) path.size(), 200 ), path.data(), mid );
	return FS_SaveBinaryFile( bfr, ba.data(), ba.size() );
}

bool LevelCache::SaveCache( MapMaterialMap& mtls, const StringView& path )
{
	FS_DirCreate( path );
	
	GatherMeshes();
	
	RemoveHiddenSurfaces();
	GenerateLines();
	
	for( size_t i = 0; i < m_meshes.size(); ++i )
	{
		SaveMesh( mtls, i, m_meshes[ i ], path );
	}
	
	LC_Level level;
	LC_Chunk chunk;
	
	// core data
	ByteArray ba_geom;
	LC_Chunk_Geom ch_geom =
	{
		&m_meshinst,
		&m_lights,
		m_sampleTree,
		&m_phyMesh,
		&m_solidBoxes,
		m_skyTexture,
	};
	ByteWriter( &ba_geom ) << ch_geom;
	{
		memcpy( chunk.sys_id, LC_FILE_GEOM_NAME, sizeof(chunk.sys_id) );
		chunk.ptr = ba_geom.data();
		chunk.size = ba_geom.size();
		level.chunks.push_back( chunk );
	}
	
	// entity data
	ByteArray ba_ents;
	LC_Chunk_Ents ch_ents = { m_scriptents };
	ByteWriter( &ba_ents ) << ch_ents;
	{
		memcpy( chunk.sys_id, LC_FILE_ENTS_NAME, sizeof(chunk.sys_id) );
		chunk.ptr = ba_ents.data();
		chunk.size = ba_ents.size();
		level.chunks.push_back( chunk );
	}
	
	// map system
	ByteArray ba_mapl;
	LC_Chunk_Mapl ch_mapl = { &m_mapLines, &m_mapLayers };
	ByteWriter( &ba_mapl ) << ch_mapl;
	{
		memcpy( chunk.sys_id, LC_FILE_MAPL_NAME, sizeof(chunk.sys_id) );
		chunk.ptr = ba_mapl.data();
		chunk.size = ba_mapl.size();
		level.chunks.push_back( chunk );
	}
	
	// AI system (pathfinding)
	ByteArray ba_pfnd;
	GenerateNavmesh( path, ba_pfnd );
	{
		memcpy( chunk.sys_id, LC_FILE_PFND_NAME, sizeof(chunk.sys_id) );
		chunk.ptr = ba_pfnd.data();
		chunk.size = ba_pfnd.size();
		level.chunks.push_back( chunk );
	}
	
	// misc. chunks
	Array< LC_Chunk > misc_chunks;
	ByteReader mcbr( m_chunkData );
	while( !mcbr.atEnd() )
	{
		LC_Chunk chk;
		mcbr << chk;
		misc_chunks.push_back( chk );
	}
	level.chunks.append( misc_chunks );
	
	ByteArray ba;
	ByteWriter bw( &ba );
	bw << level;
	
	return FS_SaveBinaryFile( String_Concat( path, SGRX_LEVELDIR_COMPILED_SFX ), ba.data(), ba.size() );
}


enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

bool LevelCache::GenerateNavmesh( const StringView& path, ByteArray& outData )
{
	if( m_navMesh.positions.size() == 0 ||
		m_navMesh.indices.size() == 0 )
	{
		LOG_ERROR << "No physics data, cannot generate navmesh!";
		return false;
	}
	
	Vec3 bmin = V3(FLT_MAX), bmax = V3(-FLT_MAX);
	Array< Vec3 > recastVerts;
	recastVerts.resize( m_navMesh.positions.size() );
	for( size_t i = 0; i < m_navMesh.positions.size(); ++i )
	{
		Vec3 p = m_navMesh.positions[ i ];
		Vec3 pconv = V3( p.x, p.z, p.y );
		recastVerts[ i ] = pconv;
		bmin = Vec3::Min( bmin, pconv );
		bmax = Vec3::Max( bmax, pconv );
	}
	
	Array< int > indices;
	for( size_t i = 0; i < m_navMesh.indices.size(); i += 4 )
	{
		int idcs[3] = { m_navMesh.indices[i], m_navMesh.indices[i+2], m_navMesh.indices[i+1] };
		indices.append( idcs, 3 );
	}
	
	const float* verts = &recastVerts[0].x;
	const int nverts = recastVerts.size();
	const int* tris = indices.data();
	const int ntris = m_navMesh.indices.size() / 4;
	
	bool retval = true;
	unsigned char* triareas = NULL;
	rcHeightfield* rchf = NULL;
	dtNavMesh* dtnavmsh = NULL;
	rcCompactHeightfield* cchf = NULL;
	rcContourSet* cset = NULL;
	rcPolyMesh* pmesh = NULL;
	rcPolyMeshDetail* dmesh = NULL;
	
	rcContext ctx;
	rcContext* prcctx = &ctx;
	
	//
	// Step 1. Initialize build config.
	//
	float cellSize = 0.3f;
	float cellHeight = 0.2f;
	float agentHeight = 1.5f;
	float agentRadius = 0.3f;
	float agentMaxClimb = 0.2f;
	float agentMaxSlope = 45.0f;
	int regionMinSize = 2;
	int regionMergeSize = 10;
	float edgeMaxLen = 12.0f;
	float edgeMaxError = 0.3f;
	float vertsPerPoly = 6.0f;
	float detailSampleDist = 3.0f;
	float detailSampleMaxError = 0.5f;
	SamplePartitionType partitionType = SAMPLE_PARTITION_WATERSHED;
	
	// Init build configuration from GUI
	rcConfig cfg;
	memset( &cfg, 0, sizeof(cfg) );
	cfg.cs = cellSize;
	cfg.ch = cellHeight;
	cfg.walkableSlopeAngle = agentMaxSlope;
	cfg.walkableHeight = (int)ceilf(agentHeight / cfg.ch);
	cfg.walkableClimb = (int)floorf(agentMaxClimb / cfg.ch);
	cfg.walkableRadius = (int)ceilf(agentRadius / cfg.cs);
	cfg.maxEdgeLen = (int)(edgeMaxLen / cellSize);
	cfg.maxSimplificationError = edgeMaxError;
	cfg.minRegionArea = (int)rcSqr(regionMinSize);		// Note: area = size*size
	cfg.mergeRegionArea = (int)rcSqr(regionMergeSize);	// Note: area = size*size
	cfg.maxVertsPerPoly = (int)vertsPerPoly;
	cfg.detailSampleDist = detailSampleDist < 0.9f ? 0 : cellSize * detailSampleDist;
	cfg.detailSampleMaxError = cellHeight * detailSampleMaxError;
	
	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy( cfg.bmin, &bmin.x );
	rcVcopy( cfg.bmax, &bmax.x );
	rcCalcGridSize( cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height );
	
	// Reset build times gathering.
	double time_total_begin = sgrx_hqtime(), time_total_end;
	
	LOG << "Building navigation:";
	LOG << " - " << cfg.width << " x " << cfg.height << " cells";
	LOG << " - " << nverts/1000.0f << "K verts, " << ntris/1000.0f << "K tris";
	
	//
	// Step 2. Rasterize input polygon soup.
	//
	
	// Allocate voxel heightfield where we rasterize our input data to.
	rchf = rcAllocHeightfield();
	if( !rchf )
	{
		LOG_ERROR << "buildNavigation: Out of memory 'rchf'.";
		goto fail;
	}
	if( !rcCreateHeightfield( prcctx, *rchf, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch ) )
	{
		LOG_ERROR << "buildNavigation: Could not create solid heightfield.";
		goto fail;
	}
	
	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	triareas = new unsigned char[ ntris ];
	if( !triareas )
	{
		LOG_ERROR << "buildNavigation: Out of memory 'triareas' (" << ntris << ").";
		goto fail;
	}
	
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset( triareas, 0, ntris*sizeof(unsigned char) );
	rcMarkWalkableTriangles( prcctx, cfg.walkableSlopeAngle, verts, nverts, tris, ntris, triareas );
	rcRasterizeTriangles( prcctx, verts, nverts, tris, triareas, ntris, *rchf, cfg.walkableClimb );
	
	delete [] triareas;
	triareas = 0;
	
	//
	// Step 3. Filter walkables surfaces.
	//
	
	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles( prcctx, cfg.walkableClimb, *rchf );
	rcFilterLedgeSpans( prcctx, cfg.walkableHeight, cfg.walkableClimb, *rchf );
	rcFilterWalkableLowHeightSpans( prcctx, cfg.walkableHeight, *rchf );
	
	
	//
	// Step 4. Partition walkable surface to simple regions.
	//
	
	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	cchf = rcAllocCompactHeightfield();
	if( !cchf )
	{
		LOG_ERROR << "buildNavigation: Out of memory 'cchf'.";
		goto fail;
	}
	if( !rcBuildCompactHeightfield( prcctx, cfg.walkableHeight, cfg.walkableClimb, *rchf, *cchf ) )
	{
		LOG_ERROR << "buildNavigation: Could not build compact data.";
		goto fail;
	}
	
	rcFreeHeightField( rchf );
	rchf = NULL;
		
	// Erode the walkable area by agent radius.
	if( !rcErodeWalkableArea( prcctx, cfg.walkableRadius, *cchf ) )
	{
		LOG_ERROR << "buildNavigation: Could not erode.";
		goto fail;
	}
	
#if 0
	TODO
	
	// (Optional) Mark areas.
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
		rcMarkConvexPolyArea(prcctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *cchf);
#endif
	
	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles
	
	if( partitionType == SAMPLE_PARTITION_WATERSHED )
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if( !rcBuildDistanceField( prcctx, *cchf ) )
		{
			LOG_ERROR << "buildNavigation: Could not build distance field.";
			goto fail;
		}
		
		// Partition the walkable surface into simple regions without holes.
		if( !rcBuildRegions( prcctx, *cchf, 0, cfg.minRegionArea, cfg.mergeRegionArea ) )
		{
			LOG_ERROR << "buildNavigation: Could not build watershed regions.";
			goto fail;
		}
	}
	else if( partitionType == SAMPLE_PARTITION_MONOTONE )
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if( !rcBuildRegionsMonotone( prcctx, *cchf, 0, cfg.minRegionArea, cfg.mergeRegionArea ) )
		{
			LOG_ERROR << "buildNavigation: Could not build monotone regions.";
			goto fail;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if( !rcBuildLayerRegions( prcctx, *cchf, 0, cfg.minRegionArea ) )
		{
			LOG_ERROR << "buildNavigation: Could not build layer regions.";
			goto fail;
		}
	}
	
	//
	// Step 5. Trace and simplify region contours.
	//
	
	// Create contours.
	cset = rcAllocContourSet();
	if( !cset )
	{
		LOG_ERROR << "buildNavigation: Out of memory 'cset'.";
		goto fail;
	}
	if( !rcBuildContours( prcctx, *cchf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset ) )
	{
		LOG_ERROR << "buildNavigation: Could not create contours.";
		goto fail;
	}
	
	//
	// Step 6. Build polygons mesh from contours.
	//
	
	// Build polygon navmesh from the contours.
	pmesh = rcAllocPolyMesh();
	if( !pmesh )
	{
		LOG_ERROR << "buildNavigation: Out of memory 'pmesh'.";
		goto fail;
	}
	if( !rcBuildPolyMesh( prcctx, *cset, cfg.maxVertsPerPoly, *pmesh ) )
	{
		LOG_ERROR << "buildNavigation: Could not triangulate contours.";
		goto fail;
	}
	
	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	
	dmesh = rcAllocPolyMeshDetail();
	if( !dmesh )
	{
		LOG_ERROR << "buildNavigation: Out of memory 'pmdtl'.";
		goto fail;
	}
	
	if( !rcBuildPolyMeshDetail( prcctx, *pmesh, *cchf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh ) )
	{
		LOG_ERROR << "buildNavigation: Could not build detail mesh.";
		goto fail;
	}
	
	rcFreeCompactHeightfield( cchf );
	cchf = NULL;
	rcFreeContourSet( cset );
	cset = NULL;
	
	// At this point the navigation mesh data is ready, you can access it from pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.
	
	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//
	
	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;
		
		// Update poly flags from areas.
		for (int i = 0; i < pmesh->npolys; ++i)
		{
			pmesh->flags[i] = 1;
		}
		
		dtNavMeshCreateParams params;
		memset( &params, 0, sizeof(params) );
		params.verts = pmesh->verts;
		params.vertCount = pmesh->nverts;
		params.polys = pmesh->polys;
		params.polyAreas = pmesh->areas;
		params.polyFlags = pmesh->flags;
		params.polyCount = pmesh->npolys;
		params.nvp = pmesh->nvp;
		params.detailMeshes = dmesh->meshes;
		params.detailVerts = dmesh->verts;
		params.detailVertsCount = dmesh->nverts;
		params.detailTris = dmesh->tris;
		params.detailTriCount = dmesh->ntris;
#if 0
		TODO
		
		params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		params.offMeshConCount = m_geom->getOffMeshConnectionCount();
#endif
		params.walkableHeight = agentHeight;
		params.walkableRadius = agentRadius;
		params.walkableClimb = agentMaxClimb;
		rcVcopy( params.bmin, pmesh->bmin );
		rcVcopy( params.bmax, pmesh->bmax );
		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = true;
		
		if( !dtCreateNavMeshData( &params, &navData, &navDataSize ) )
		{
			LOG_ERROR << "Could not build Detour navmesh.";
			if (params.nvp > DT_VERTS_PER_POLYGON)
				LOG << "Reason: reached DT_VERTS_PER_POLYGON limit!";
			if (params.vertCount >= 0xffff)
				LOG << "Reason: Vertex data size exceeds 65535!";
			if (!params.vertCount || !params.verts)
				LOG << "Reason: No vertex data!";
			if (!params.polyCount || !params.polys)
				LOG << "Reason: No polygon data!";
			goto fail;
		}
		
		outData.assign( navData, navDataSize );
		dtFree( navData );
		
#if 0
		TODO
		
		dtnavmsh = dtAllocNavMesh();
		if (!dtnavmsh)
		{
			dtFree(navData);
			LOG_ERROR << "Could not create Detour navmesh";
			goto fail;
		}
		
		dtStatus status;
		
		status = dtnavmsh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			LOG_ERROR << "Could not init Detour navmesh";
			goto fail;
		}
		
		status = m_navQuery->init(dtnavmsh, 2048);
		if (dtStatusFailed(status))
		{
			LOG_ERROR << "Could not init Detour navmesh query";
			goto fail;
		}
#endif
	}
	
	time_total_end = sgrx_hqtime();
	
	LOG << ">> Polymesh: " << pmesh->nverts << " vertices  " << pmesh->npolys << " polygons";
	LOG << "--- TIME: " << ( time_total_end - time_total_begin );
	
ending:
	if( rchf ) rcFreeHeightField( rchf );
	if( dtnavmsh ) dtFreeNavMesh( dtnavmsh );
	if( cchf ) rcFreeCompactHeightfield( cchf );
	if( cset ) rcFreeContourSet( cset );
	if( pmesh ) rcFreePolyMesh( pmesh );
	if( dmesh ) rcFreePolyMeshDetail( dmesh );
	return retval;
	
fail:
	retval = false;
	goto ending;
}


