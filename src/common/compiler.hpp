

#include "levelcache.hpp"




#define MAX_MATERIAL_TEXTURES 8
struct MapMaterial : SGRX_RefCounted
{
	String name;
	String shader;
	String texture[ MAX_MATERIAL_TEXTURES ];
	int texcount;
};
typedef Handle< MapMaterial > MapMaterialHandle;
typedef HashTable< StringView, MapMaterialHandle > MapMaterialMap;



struct RectPacker
{
	struct Node
	{
		int child0, child1;
		int x0, y0, x1, y1;
		bool occupied;
	};
	
	int _NodeAlloc( int startnode, int w, int h );
	
	RectPacker( int w, int h )
	{
		Node N = { 0, 0, 0, 0, w, h, false };
		m_tree.push_back( N );
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

struct VoxelBlock
{
	// blocks of 4x4x4, stored into uint64 array
	VoxelBlock( Vec3 bbmin, Vec3 bbmax, float stepsize );
	void RasterizeTriangle( Vec3 p1, Vec3 p2, Vec3 p3 );
	void RasterizeSolid( Vec4* planes, size_t count );
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


#define LCVertex_DECL "pf3nf3cb40f21f2"

struct LevelCache
{
	struct Vertex
	{
		Vec3 pos;
		Vec3 nrm;
		uint32_t color;
		float tx0, ty0;
		float tx1, ty1;
		
		Vertex Interpolate( const Vertex& o, float q ) const
		{
			Vertex out =
			{
				TLERP( pos, o.pos, q ),
				TLERP( nrm, o.nrm, q ),
				Vec4ToCol32( TLERP( Col32ToVec4( color ), Col32ToVec4( o.color ), q ) ),
				TLERP( tx0, o.tx0, q ),
				TLERP( ty0, o.ty0, q ),
				TLERP( tx1, o.tx1, q ),
				TLERP( ty1, o.ty1, q ),
			};
			return out;
		}
		
		template< class T > void Serialize( T& arch )
		{
			arch << pos;
			arch << nrm;
			arch << color;
			arch << tx0 << ty0;
			arch << tx1 << ty1;
		}
		
		bool operator == ( const Vertex& o ) const
		{
			return pos == o.pos
				&& nrm == o.nrm
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
		String m_texname;
		Vec2 m_lmrect;
		Vec2 m_lmmin;
		int m_lmalloc;
		bool m_isSolid;
		int m_decalLayer; // -1 if none
	};
	
	struct Mesh
	{
		Array< size_t > m_partIDs;
		Array< String > m_texnames;
		Vec3 m_boundsMin;
		Vec3 m_boundsMax;
		Vec3 m_pos;
		int m_div;
	};
	
	struct Solid : Array< Vec4 > {};
	
	LevelCache();
	
	void AddPart( const Vertex* verts, int vcount, const StringView& texname_short, size_t fromsolid, bool solid, int decalLayer );
	size_t AddSolid( const Vec4* planes, int count );
	void GenerateSamples( float stepsize );
	
	void AddMeshInst( const String& meshname, const Mat4& mtx, float lmquality, bool solid, bool dynlit, bool castlms, int decalLayer )
	{
		uint32_t flags = 0;
		if( solid ) flags |= LM_MESHINST_SOLID;
		if( dynlit ) flags |= LM_MESHINST_DYNLIT;
		if( castlms ) flags |= LM_MESHINST_CASTLMS;
		LC_MeshInst MI = { meshname, mtx, flags, 0, lmquality };
		if( decalLayer != -1 )
		{
			MI.m_flags |= LM_MESHINST_DECAL;
			MI.m_decalLayer = decalLayer;
		}
		m_meshinst.push_back( MI );
	}
	
	void AddLight( const LC_Light& L )
	{
		m_lights.push_back( L );
	}
	
	void AddSample( const Vec3& p )
	{
		m_samples.push_back( p );
	}
	
	static bool TexNull( const StringView& tex )
	{
		return tex == "textures/null.png" || tex == "textures/clip.png";
	}
	static bool TexNoLight( const StringView& tex )
	{
		return TexNull( tex ) || tex == "textures/black.png";
	}
	static bool TexNoSolid( const StringView& tex )
	{
		return TexNoLight( tex );
	}
	
	void _CutPoly( const PartPoly& PP, const Vec4& plane, Array< PartPoly >& outpolies );
	bool _PolyInside( const PartPoly& PP, const Solid& S );
	void RemoveHiddenSurfaces();
	void GenerateLines();
	void CombineParts();
//	void _GenerateLightmapPolys( Part& P );
	bool _PackLightmapPolys( Mesh& M, int curwidth );
	void GatherMeshes();
	void GenerateLightmapCoords( Mesh& M );
	bool SaveMesh( int mid, Mesh& M, const StringView& path, bool remnull );
	bool SaveCache( const StringView& path );
	void GenerateLightmaps( const StringView& path );
	bool GenerateNavmesh( const StringView& path, ByteArray& outData );
	
	MapMaterialMap m_mapMtls;
	Array< Solid > m_solids;
	Array< Mesh > m_meshes;
	Array< Part > m_meshParts;
	Array< LC_MeshInst > m_meshinst;
	Array< LC_Light > m_lights;
	Array< LC_ScriptedEntity > m_scriptents;
	LC_PhysicsMesh m_phyMesh;
	Array< Vec3 > m_samples;
	Array< Vec2 > m_lines;
	
	Vec3 AmbientColor;
	Vec3 LightmapClearColor;
	int RADNumBounces;
	float LightmapDetail;
	float LightmapBlurSize;
	float AODistance;
	float AOMultiplier;
	float AOFalloff;
	float AOEffect;
//	float AODivergence;
	Vec3 AOColor;
	int AONumSamples;
};



