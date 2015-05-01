

#include <utils.hpp>


#define LM_MESHINST_SOLID 0x0001
#define LM_MESHINST_DYNLIT 0x0002
#define LM_MESHINST_CASTLMS 0x0004
struct LC_MeshInst
{
	String m_meshname;
	Mat4 m_mtx;
	uint32_t m_flags;
	
	// editor use only, not serialized
	float lmquality;
	
	template< class T > void Serialize( T& arch )
	{
		arch << m_meshname;
		arch << m_mtx;
		arch( m_flags, arch.version >= 1, LM_MESHINST_SOLID );
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
	
	template< class T > void Serialize( T& arch )
	{
		arch << type;
		arch << serialized_params;
	}
};

// LINE = Vec2 x2
// SAMPLE = Vec3


