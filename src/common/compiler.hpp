

#pragma once

#include <engine.hpp>
#include "levelcache.hpp"




struct LMRenderer
{
	struct Config
	{
		Config()
		{
			ambientColor = V3(0.1f);
			lightmapClearColor = V3(0.0f);
			lightmapDetail = 1;
			lightmapBlurSize = 1;
			aoDistance = 2;
			aoMultiplier = 1;
			aoFalloff = 2;
			aoEffect = 0;
			aoColor = V3(0);
			aoNumSamples = 15;
		}
		
		Vec3 ambientColor;
		Vec3 lightmapClearColor;
		float lightmapDetail;
		float lightmapBlurSize;
		float aoDistance;
		float aoMultiplier;
		float aoFalloff;
		float aoEffect;
		Vec3 aoColor;
		int aoNumSamples;
	};
	struct Mesh
	{
		Array< Vec3 > positions;
		Array< Vec3 > normals;
		Array< Vec2 > texcoords0;
		Array< Vec2 > texcoords1;
		Array< uint32_t > indices;
		struct ltr_Mesh* ltrMesh;
	};
	struct OutLMInfo
	{
		SGRX_GUID guid;
		uint32_t w, h;
	};
	
	LMRenderer();
	~LMRenderer();
	
	void Start();
	bool CheckStatus();
	bool GetLightmap( uint32_t which, Array< Vec3 >& outcols,
		Array< Vec4 >& outxyzf, OutLMInfo& outlminfo );
	bool GetSample( uint32_t which, Vec3 outcols[6] );
	bool AddMeshInst( SGRX_MeshInstance* MI, const Vec2& lmsize, SGRX_GUID lmguid, bool solid );
	bool AddLight( const LC_Light& light );
	
	struct ltr_Scene* m_scene;
	HashTable< MeshHandle, Mesh* > m_meshes;
	HashTable< SGRX_GUID, Vec2 > m_lmsizes;
	
	Config config;
	Array< Vec3 > sample_positions;
	
	uint32_t rendered_sample_count;
	uint32_t rendered_lightmap_count;
	struct ltr_SampleInfo* rendered_samples;
	
	String stage;
	float completion;
};



struct RectPacker
{
	struct Node
	{
		int child0, child1;
		int x0, y0, x1, y1;
		bool occupied;
	};
	
	int _NodeAlloc( int startnode, int w, int h );
	
	RectPacker(){ Init( 1, 1 ); }
	RectPacker( int w, int h ){ Init( w, h ); }
	void Init( int w, int h )
	{
		Node N = { 0, 0, 0, 0, w, h, false };
		m_tree.assign( &N, 1 );
	}
	
	int Alloc( int w, int h )
	{
		return _NodeAlloc( 0, w, h );
	}
	
	bool GetOffset( int pos, int outoff[2] )
	{
		if( pos < 0 || pos >= (int) m_tree.size() )
			return false;
		outoff[0] = m_tree[ pos ].x0;
		outoff[1] = m_tree[ pos ].y0;
		return true;
	}
	
	Array< Node > m_tree;
};

struct LMRectPacker : RectPacker
{
	LMRectPacker();
	int LMAlloc( int w, int h );
	
	bool GetOffset( int pos, int outoff[2] )
	{
		if( pos < 0 || pos >= (int) m_allocs.size() )
			return false;
		Node& N = m_tree[ m_allocs[ pos ] ];
		outoff[0] = N.x0;
		outoff[1] = N.y0;
		return true;
	}
	
	Array< int > m_allocs;
	Array< int > m_tmpAllocs;
	int m_sizeid;
};

struct VoxelBlock
{
	// blocks of 4x4x4, stored into uint64 array
	VoxelBlock( Vec3 bbmin, Vec3 bbmax, float stepsize );
	void RasterizeTriangle( Vec3 p1, Vec3 p2, Vec3 p3 );
	int32_t RasterizeSolid( Vec4* planes, size_t count );
	Vec3 GetPosition( int32_t x, int32_t y, int32_t z );
	
	bool Get( int32_t x, int32_t y, int32_t z );
	void Set1( int32_t x, int32_t y, int32_t z );
	void Set0( int32_t x, int32_t y, int32_t z );
	bool _FindBlock( int32_t x, int32_t y, int32_t z, uint64_t** outblk, int* outbit );
	bool _FindAABB( Vec3 bbmin, Vec3 bbmax, int32_t outbb[6] ); // X,Y,Z | min,max
	void _PosToCoord( Vec3 p, int32_t outco[3] );
	
	int32_t m_xsize, m_ysize, m_zsize;
	int32_t m_xbcnt, m_ybcnt, m_zbcnt;
	Vec3 m_bbmin;
	Vec3 m_bbmax;
	float m_stepsize;
	uint64_t* m_data;
};



#define MAX_MATERIAL_TEXTURES 8
struct MapMaterial : SGRX_RefCounted
{
	String name;
	String shader;
	String texture[ MAX_MATERIAL_TEXTURES ];
	int texcount;
	int blendmode;
	int flags;
};
typedef Handle< MapMaterial > MapMaterialHandle;
typedef HashTable< StringView, MapMaterialHandle > MapMaterialMap;


#define LM_MESHINST_ED__FLAGS 0xffff0000
#define LM_MESHINST_ED_SKIPCUT 0x80000000


#define LCVertex_DECL "pf3nf3tf4cb40f21f2"

struct LevelCache
{
	struct Vertex
	{
		Vec3 pos;
		Vec3 nrm;
		Vec4 tng;
		uint32_t color;
		float tx0, ty0;
		float tx1, ty1;
		
		Vertex Interpolate( const Vertex& o, float q ) const
		{
			Vertex out =
			{
				TLERP( pos, o.pos, q ),
				TLERP( nrm, o.nrm, q ),
				TLERP( tng, o.tng, q ),
				Vec4ToCol32( TLERP( Col32ToVec4( color ), Col32ToVec4( o.color ), q ) ),
				TLERP( tx0, o.tx0, q ),
				TLERP( ty0, o.ty0, q ),
				TLERP( tx1, o.tx1, q ),
				TLERP( ty1, o.ty1, q ),
			};
			return out;
		}
		Vertex InterpLMCoords( const Vec2& lmin, const Vec2& lmax ) const
		{
			Vertex O = *this;
			O.tx1 = TLERP( lmin.x, lmax.x, O.tx1 );
			O.ty1 = TLERP( lmin.y, lmax.y, O.ty1 );
			return O;
		}
		void SetDefault()
		{
			pos = V3(0);
			nrm = V3(0,0,1);
			tng = V4(1,0,0,1);
			color = 0xffffffff;
			tx0 = 0;
			ty0 = 0;
			tx1 = 0;
			ty1 = 0;
		}
		
		template< class T > void Serialize( T& arch )
		{
			arch << pos;
			arch << nrm;
			arch << tng;
			arch << color;
			arch << tx0 << ty0;
			arch << tx1 << ty1;
		}
		
		bool operator == ( const Vertex& o ) const
		{
			return pos == o.pos
				&& nrm == o.nrm
				&& tng == o.tng
				&& color == o.color
				&& tx0 == o.tx0
				&& ty0 == o.ty0
				&& tx1 == o.tx1
				&& ty1 == o.ty1;
		}
	};
	
	struct PartPoly : Array< Vertex >
	{
		size_t m_solid;
	};
	
	struct Part
	{
		Array< Vertex > m_vertices;
		size_t m_solid;
		String m_mtlname;
		LC_Lightmap m_lightmap;
		int m_lmalloc;
		uint32_t m_flags;
		int m_decalLayer; // -1 if none
		bool m_skipCut;
	};
	
	struct Mesh
	{
		LMRectPacker m_packer;
		Array< size_t > m_partIDs;
		Array< String > m_mtlnames;
		Vec3 m_boundsMin;
		Vec3 m_boundsMax;
		Vec3 m_pos;
		int m_div;
	};
	
	struct PathArea : Array< Vec2 >
	{
		float z0, z1;
	};
	
	struct Solid : Array< Vec4 > {};
	
	LevelCache( SGRX_LightEnv* lightEnv );
	
	void AddPart( const Vertex* verts, int vcount, LC_Lightmap& lm,
		const StringView& mtlname, size_t fromsolid, uint32_t flags, int decalLayer );
	size_t AddSolid( const Vec4* planes, int count );
	
	void AddMeshInst( const String& meshname, const Mat4& mtx,
		uint32_t flags, int decalLayer, LC_Lightmap& lm )
	{
		flags &= (LM_MESHINST_SOLID|LM_MESHINST_DYNLIT
			|LM_MESHINST_CASTLMS|LM_MESHINST_DECAL
			|LM_MESHINST_TRANSPARENT|LM_MESHINST_VCOL);
		LC_MeshInst MI = { meshname, mtx, flags, decalLayer, lm };
		m_meshinst.push_back( MI );
	}
	
	void AddLight( const LC_Light& L )
	{
		m_lights.push_back( L );
	}
	
	void _CutPoly( const PartPoly& PP, const Vec4& plane, Array< PartPoly >& outpolies );
	bool _PolyInside( const PartPoly& PP, const Solid& S );
	void RemoveHiddenSurfaces();
	void GenerateLines();
	void CombineParts();
	
	enum GatherMeshesFlags
	{
		GM_Render = 0x01,
		GM_Physics = 0x02,
		GM_Navigation = 0x04,
		GM_ALL = 0xff,
	};
	void GatherMeshes( unsigned which );
	bool SaveMesh( MapMaterialMap& mtls, int mid, Mesh& M, const StringView& path );
	bool SaveCache( MapMaterialMap& mtls, const StringView& path, ArrayView<LC_Chunk> xchunks );
	bool GenerateNavmesh( ByteArray& outData );
	bool GenerateCoverData( const ByteView& navData, Array<LC_CoverPart>& out );
	
	Array< Solid > m_solids;
	Array< Mesh > m_meshes;
	Array< Part > m_meshParts;
	Array< LC_MeshInst > m_meshinst;
	Array< LC_Light > m_lights;
	LC_Chunk_Gobj m_gobj;
	LC_PhysicsMesh m_phyMesh;
	Array< LC_SolidBox > m_solidBoxes;
	StringView m_skyTexture;
	StringView m_clutTexture;
	LC_PhysicsMesh m_navMesh;
	SGRX_LightEnv* m_lightEnv;
	
	Array< Vec2 > m_mapLines;
	Array< LC_Map_Layer > m_mapLayers;
	
	Array< PathArea > m_pathAreas;
	ByteArray m_chunkData;
};



