

#pragma once
#include "utils.hpp"


struct SGRX_Pathfinder
{
	SGRX_Pathfinder();
	~SGRX_Pathfinder();
	
	bool Load( const ByteArray& data );
	uint64_t FindPoly( const Vec3& pt, Vec3* outpt = NULL, const Vec3& ext = V3(2) );
	bool FindPath( const Vec3& from, uint64_t frompoly,
		const Vec3& to, uint64_t topoly, Array< Vec3 >& pts );
	bool FindPath( const Vec3& from, const Vec3& to, Array< Vec3 >& pts );
	void DebugDraw();
	
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
};

