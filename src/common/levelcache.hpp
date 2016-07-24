

#pragma once
#include <utils.hpp>


#define SGRX_LEVEL_DIR_SFX ".cache"
#define SGRX_LEVELDIR_COMPILED_SFX "/compiled"
#define SGRX_LEVELDIR_LMCACHE_SFX "/editor_lightmaps"
#define SGRX_LEVELDIR_MDCACHE_SFX "/metadata"
#define SGRX_LEVEL_COMPILED_SFX SGRX_LEVEL_DIR_SFX SGRX_LEVELDIR_COMPILED_SFX
#define SGRX_LEVEL_LMCACHE_SFX SGRX_LEVEL_DIR_SFX SGRX_LEVELDIR_LMCACHE_SFX
#define SGRX_LEVEL_MDCACHE_SFX SGRX_LEVEL_DIR_SFX SGRX_LEVELDIR_MDCACHE_SFX


// 0: version restart
#define LC_FILE_VERSION 0


struct LC_Chunk
{
	char sys_id[ 4 ];
	uint32_t size;
	uint8_t* ptr;
	
	template< class T > void Serialize( T& arch )
	{
		arch.charbuf( sys_id, 4 );
		arch << size;
		if( T::IsReader )
		{
			ptr = (uint8_t*) arch.at();
			arch.padding( size );
		}
		else
		{
			arch.memory( ptr, size );
		}
	}
};

struct LC_Level
{
	Array< LC_Chunk > chunks;
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "SGRXCLEV" );
		SerializeVersionHelper<T> svh( arch, LC_FILE_VERSION );
		svh << chunks;
	}
};


struct LC_Lightmap
{
	uint16_t width;
	uint16_t height;
	Array< uint32_t > data;
	Array< uint32_t > nmdata;
	
	template< class T > void Serialize( T& arch )
	{
		arch << width << height;
		if( T::IsReader )
		{
			data.resize( size_t(width) * size_t(height) );
			nmdata.resize( size_t(width) * size_t(height) );
		}
		arch.memory( data.data(), data.size_bytes() );
		arch.memory( nmdata.data(), nmdata.size_bytes() );
	}
};

#define LM_MESHINST_SOLID 0x0001
#define LM_MESHINST_DYNLIT 0x0002
#define LM_MESHINST_UNLIT 0x0020
#define LM_MESHINST_CASTLMS 0x0004
#define LM_MESHINST_DECAL 0x0008
#define LM_MESHINST_EDITOR_ONLY 0x0010 // editor-only
#define LM_MESHINST_TRANSPARENT 0x0040
#define LM_MESHINST_VCOL 0x0080
struct LC_MeshInst
{
	String m_meshname;
	Mat4 m_mtx;
	uint32_t m_flags;
	uint8_t m_decalLayer;
	
	LC_Lightmap m_lmap;
	
	template< class T > void Serialize( T& arch )
	{
		arch << m_meshname;
		arch << m_mtx;
		arch << m_flags;
		arch << m_decalLayer;
		arch << m_lmap;
	}
};

#define LM_LIGHT_POINT 0
#define LM_LIGHT_SPOT 1
#define LM_LIGHT_DIRECT 2
#define LM_LIGHT_DYN_POINT 10
#define LM_LIGHT_DYN_SPOT 11
struct LC_Light
{
	int type;
	Vec3 pos; // point / spot
	Vec3 dir; // dir / spot
	Vec3 up; // spot
	float range; // point / spot
	float power; // point / spot
	float light_radius; // point / spot / direct (as divergence)
	Vec3 color; // -
	int num_shadow_samples; // -
	float flaresize; // point / spot
	Vec3 flareoffset; // point / spot
	float innerangle;
	float outerangle;
	float spotcurve;
	
	template< class T > void Serialize( T& arch )
	{
		arch << type;
		arch << pos;
		arch << dir;
		arch << up;
		arch << range;
		arch << power;
		arch << light_radius;
		arch << color;
		arch << num_shadow_samples;
		arch << flaresize;
		arch << flareoffset;
		arch << innerangle;
		arch << outerangle;
		arch << spotcurve;
	}
};
typedef Array< LC_Light > LCLightArray;

struct LC_PhysicsMesh
{
	Array< String > materials;
	Array< Vec3 > positions;
	Array< uint32_t > indices; // array of 4 indices (v0,v1,v2,mtl)
	
	template< class T > void Serialize( T& arch )
	{
		arch << materials;
		arch << positions;
		arch << indices;
	}
};

// Level geometry system
#define LC_FILE_GEOM_NAME "GEOM"
// v1: added solid boxes
// v2: added sky texture
// v3: added cLUT texture
// v4: changed lighting env. (incompatible)
#define LC_FILE_GEOM_VERSION 4
struct LC_SolidBox
{
	Vec3 position;
	Quat rotation;
	Vec3 scale;
	
	template< class T > void Serialize( T& arch )
	{
		arch << position;
		arch << rotation;
		arch << scale;
	}
};
struct LC_Chunk_Geom
{
	Array< LC_MeshInst >* meshinsts;
	Array< LC_Light >* lights;
	SGRX_LightEnv* lightEnv;
	LC_PhysicsMesh* physics;
	Array< LC_SolidBox >* solidBoxes;
	StringView skyTexture;
	StringView clutTexture;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_GEOM_VERSION );
		svh << *meshinsts;
		svh << *lights;
		svh << *lightEnv;
		svh << *physics;
		svh( *solidBoxes, svh.version >= 1 );
		if( svh.version >= 2 )
			svh.stringView( skyTexture );
		else if( T::IsReader )
			skyTexture = SV();
		if( svh.version >= 3 )
			svh.stringView( clutTexture );
		else if( T::IsReader )
			clutTexture = SV();
	}
};

// Level game object definitions
#define LC_FILE_GOBJ_NAME "GOBJ"
#define LC_FILE_GOBJ_VERSION 0
struct LC_GOLink
{
	SGRX_GUID obj_guid;
	SGRX_GUID dst_guid;
	String prop;
	
	template< class T > void Serialize( T& arch )
	{
		arch << obj_guid;
		arch << dst_guid;
		arch << prop;
	}
};
struct LC_GOLightmap
{
	SGRX_GUID rsrc_guid;
	LC_Lightmap lmap;
	
	template< class T > void Serialize( T& arch )
	{
		arch << rsrc_guid;
		arch << lmap;
	}
};
struct LC_GameObject
{
	String name;
	String id;
	Mat4 transform;
	SGRX_GUID guid;
	SGRX_GUID parent_guid;
	// srlz_* contains [__name, __type, __guid] special vars
	Array< String > srlz_resources;
	Array< String > srlz_behaviors;
	
	template< class T > void Serialize( T& arch )
	{
		arch << name;
		arch << id;
		arch << transform;
		arch << guid;
		arch << parent_guid;
		arch << srlz_resources;
		arch << srlz_behaviors;
	}
};
struct LC_Chunk_Gobj
{
	Array< LC_GameObject > gameObjects;
	Array< LC_GOLink > links;
	Array< LC_GOLightmap > lightmaps;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_GOBJ_VERSION );
		svh << gameObjects;
		svh << links;
		svh << lightmaps;
	}
};

// Level marker definitions
#define LC_FILE_MRKR_NAME "MRKR"
#define LC_FILE_MRKR_VERSION 0
struct LC_Marker
{
	StringView name;
	Vec3 pos;
	
	template< class T > void Serialize( T& arch )
	{
		arch.stringView( name );
		arch << pos;
	}
};
struct LC_Chunk_Mrkr
{
	Array< LC_Marker > markers;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_MRKR_VERSION );
		svh << markers;
	}
};

// Level map system
#define LC_FILE_MAPL_NAME "MAPL"
#define LC_FILE_MAPL_VERSION 1
struct LC_Map_Layer
{
	uint32_t from;
	uint32_t to;
	float height;
	
	template< class T > void Serialize( T& arch )
	{
		arch << from;
		arch << to;
		arch << height;
	}
};
struct LC_Chunk_Mapl
{
	Array< Vec2 >* lines;
	Array< LC_Map_Layer >* layers;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_MAPL_VERSION );
		svh << *lines;
		svh( *layers, svh.version >= 1 );
	}
};

// Pathfinding data (just a byte buffer)
#define LC_FILE_PFND_NAME "PFND"

// Cover data (method 2)
#define LC_FILE_COV2_NAME "COV2"
#define LC_FILE_COV2_VERSION 0
#define COVER_LOW 0x01
#define COVER_OPENLEFT 0x02
#define COVER_OPENRIGHT 0x04
#define COVER_OPENTYPE 0x07 // mask for previous flags
struct LC_CoverPart
{
	Vec3 p0, p1, n;
	uint32_t polyRef;
	uint32_t flags; // COVER_*
	
	template< class T > void Serialize( T& arch )
	{
		arch << p0;
		arch << p1;
		arch << n;
		arch << polyRef;
		arch << flags;
	}
};
struct LC_Chunk_COV2
{
	Array< LC_CoverPart >* covers;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_COV2_VERSION );
		svh << *covers;
	}
};


