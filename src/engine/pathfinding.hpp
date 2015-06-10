

#include "utils.hpp"


struct SGRX_Pathfinder
{
	SGRX_Pathfinder();
	~SGRX_Pathfinder();
	
	bool Load( const ByteArray& data );
	void DebugDraw();
	
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
};

