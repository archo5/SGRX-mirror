

#pragma once
#include <utils.hpp>


#define SGRX_LEVELS_DIR "levels/"
#define SGRX_LEVEL_DIR_SFX ".cache"
#define SGRX_LEVELDIR_COMPILED_SFX "/compiled"
#define SGRX_LEVELDIR_LMCACHE_SFX "/editor_lightmaps"
#define SGRX_LEVEL_COMPILED_SFX SGRX_LEVEL_DIR_SFX SGRX_LEVELDIR_COMPILED_SFX
#define SGRX_LEVEL_LMCACHE_SFX SGRX_LEVEL_DIR_SFX SGRX_LEVELDIR_LMCACHE_SFX


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
#define LC_FILE_GEOM_VERSION 3
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
	SGRX_LightTree* lightTree;
	LC_PhysicsMesh* physics;
	Array< LC_SolidBox >* solidBoxes;
	StringView skyTexture;
	StringView clutTexture;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_GEOM_VERSION );
		svh << *meshinsts;
		svh << *lights;
		svh << *lightTree;
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

// Level entity definitions
#define LC_FILE_ENTS_NAME "ENTS"
#define LC_FILE_ENTS_VERSION 0
struct LC_ScriptedEntity
{
	String type;
	String serialized_params;
	Array< LC_ScriptedEntity > subentities;
	
	template< class T > void Serialize( T& arch )
	{
		arch << type;
		arch << serialized_params;
		arch << subentities;
	}
};
struct LC_Chunk_Ents
{
	Array< LC_ScriptedEntity > entities;
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_ENTS_VERSION );
		svh << entities;
	}
};

// Level game object definitions
#define LC_FILE_GOBJ_NAME "GOBJ"
#define LC_FILE_GOBJ_VERSION 0
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
	
	template< class T > void Serialize( T& arch )
	{
		SerializeVersionHelper<T> svh( arch, LC_FILE_GOBJ_VERSION );
		svh << gameObjects;
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


