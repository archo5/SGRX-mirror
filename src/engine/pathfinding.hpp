

#pragma once
#include "utils.hpp"


inline Vec3 RC2SGRX( const Vec3& v ){ return V3( v.x, v.z, v.y ); }
inline Vec3 SGRX2RC( const Vec3& v ){ return V3( v.x, v.z, v.y ); }


struct SGRX_Pathfinder
{
	SGRX_Pathfinder();
	~SGRX_Pathfinder();
	
	bool Load( ByteView data );
	uint32_t FindPoly( const Vec3& pt, Vec3* outpt = NULL, const Vec3& ext = V3(2) );
	Vec3 GetPolyPos( uint32_t poly );
	int GetPolyNeighbors( uint32_t poly, uint32_t* out, int size );
	bool FindPath( const Vec3& from, uint32_t frompoly,
		const Vec3& to, uint32_t topoly, Array< Vec3 >& pts );
	bool FindPath( const Vec3& from, const Vec3& to, Array< Vec3 >& pts );
	void DebugDraw();
	
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
};

