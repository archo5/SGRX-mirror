

#include <utils.hpp>


// 1: added mesh instance flags
// 2: added flare offset
// 3: added decal layer
// 4: added subentities
// 5: added physics mesh
// 6: added navmesh
#define LC_FILE_VERSION 6


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
	
	// editor use only, not serialized
	float lmquality;
	
	template< class T > void Serialize( T& arch )
	{
		arch << m_meshname;
		arch << m_mtx;
		arch( m_flags, arch.version >= 1, LM_MESHINST_SOLID );
		arch( m_decalLayer, arch.version >= 3, 0 );
	}
};

#define LM_LIGHT_POINT 0
#define LM_LIGHT_SPOT 1
#define LM_LIGHT_DIRECT 2
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
		arch( flareoffset, arch.version >= 2, V3(0) );
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
		arch( subentities, arch.version >= 4 );
	}
};

// LINE = Vec2 x2
// SAMPLE = Vec3

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


