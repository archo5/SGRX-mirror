

#include "levelcache.hpp"




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
		Array< int > m_polysizes;
		Array< float > m_polylmq;
		Array< size_t > m_solids;
		String m_texname;
		
		Array< Vec2 > m_lmverts; // size = # vertices
		Array< Vec2 > m_lmrects; // size = # polysizes
		Array< int > m_lmallocs; // size = # polysizes
	};
	
	struct Mesh
	{
		Array< Part > m_parts;
		Vec3 m_boundsMin;
		Vec3 m_boundsMax;
		Vec3 m_pos;
		int m_div;
	};
	
	struct Solid : Array< Vec4 > {};
	
	void _AddPoly( Mesh& M, Part& P, const Vec3& center, const Vertex* verts, int vcount, float lmquality, size_t fromsolid );
	void AddPoly( const Vertex* verts, int vcount, const String& texname_short, float lmquality, size_t fromsolid );
	size_t AddSolid( const Vec4* planes, int count );
	
	void AddMeshInst( const String& meshname, const Mat4& mtx, float lmquality = 1.0f, bool solid = true, bool dynlit = false, bool castlms = true )
	{
		LC_MeshInst MI = { meshname, mtx, ( solid * LM_MESHINST_SOLID ) | ( dynlit * LM_MESHINST_DYNLIT ) | ( castlms * LM_MESHINST_CASTLMS ), lmquality };
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
	
	void AddScriptedEntity( const StringView& tyname, const StringView& params )
	{
		LC_ScriptedEntity SE = { tyname, params };
		m_scriptents.push_back( SE );
	}
	
	static bool TexNull( const StringView& tex )
	{
		return tex == "textures/null.png" || tex == "textures/clip.png";
	}
	static bool TexNoLight( const StringView& tex )
	{
		return TexNull( tex ) || tex == "textures/black.png";
	}
	
	void _CutPoly( const PartPoly& PP, const Vec4& plane, Array< PartPoly >& outpolies );
	bool _PolyInside( const PartPoly& PP, const Solid& S );
	void RemoveHiddenSurfaces();
	void GenerateLines();
	void _GenerateLightmapPolys( Part& P );
	bool _PackLightmapPolys( Mesh& M, int curwidth );
	void GenerateLightmapCoords( Mesh& M );
	bool SaveMesh( int mid, Mesh& M, const StringView& path, bool remnull );
	bool SaveCache( const StringView& path );
	void GenerateLightmaps( const StringView& path );
	
	Array< Solid > m_solids;
	Array< Mesh > m_meshes;
	Array< LC_MeshInst > m_meshinst;
	Array< LC_Light > m_lights;
	Array< LC_ScriptedEntity > m_scriptents;
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



