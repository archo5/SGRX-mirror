

#pragma once
#include <utils.hpp>


// 0: version restart
#define LC_FILE_VERSION 0


struct LC_Lightmap
{
	uint16_t width;
	uint16_t height;
	Array< uint32_t > data;
	
	template< class T > void Serialize( T& arch )
	{
		arch << width << height;
		if( T::IsReader )
			data.resize( width * height );
		arch.memory( data.data(), data.size_bytes() );
	}
};

#define LM_MESHINST_SOLID 0x0001
#define LM_MESHINST_DYNLIT 0x0002
#define LM_MESHINST_CASTLMS 0x0004
#define LM_MESHINST_DECAL 0x0008
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
		arch( m_flags, true, LM_MESHINST_SOLID );
		arch( m_decalLayer, true, 0 );
		arch( m_lmap, true );
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
		arch( flareoffset, true, V3(0) );
		arch << innerangle;
		arch << outerangle;
		arch << spotcurve;
	}
};
typedef Array< LC_Light > LCLightArray;

struct LC_ScriptedEntity
{
	String type;
	String serialized_params;
	Array< LC_ScriptedEntity > subentities;
	
	template< class T > void Serialize( T& arch )
	{
		arch << type;
		arch << serialized_params;
		arch( subentities, true );
	}
};

// LINE = Vec2 x2
// SAMPLES = SGRX_LightTree

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


