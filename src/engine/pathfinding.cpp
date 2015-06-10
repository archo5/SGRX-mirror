

#include "engine.hpp"
#include "pathfinding.hpp"

#include <DetourNavMesh.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <../../DebugUtils/Include/DebugDraw.h>
#include <../../DebugUtils/Include/DetourDebugDraw.h>


struct DetourDebugDraw : duDebugDraw
{
	DetourDebugDraw() : br( GR2D_GetBatchRenderer() ){}
	virtual void depthMask( bool state )
	{
		// TODO ?
	}
	virtual void texture( bool state )
	{
		// ?
	}
	virtual void begin( duDebugDrawPrimitives prim, float size = 1.0f )
	{
		br.Flush();
		switch( prim )
		{
		case DU_DRAW_POINTS: br.SetPrimitiveType( PT_Points ); break;
		case DU_DRAW_LINES: br.SetPrimitiveType( PT_Lines ); break;
		case DU_DRAW_TRIS: br.SetPrimitiveType( PT_Triangles ); break;
		case DU_DRAW_QUADS: br.SetPrimitiveType( PT_Triangles ); break;
		}
		quad = prim == DU_DRAW_QUADS;
	}
	virtual void vertex(const float* pos, unsigned int color)
	{
		br.Colu( color ).Pos( pos[0], pos[1], pos[2] );
	}
	virtual void vertex(const float x, const float y, const float z, unsigned int color)
	{
		br.Colu( color ).Pos( x, y, z );
	}
	virtual void vertex(const float* pos, unsigned int color, const float* uv)
	{
		br.Tex( uv[0], uv[1] ).Colu( color ).Pos( pos[0], pos[1], pos[2] );
	}
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
	{
		br.Tex( u, v ).Colu( color ).Pos( x, y, z );
	}
	virtual void end()
	{
		if( quad )
		{
		}
		br.Flush();
	}
	BatchRenderer& br;
	bool quad;
};


SGRX_Pathfinder::SGRX_Pathfinder()
{
	m_navMesh = dtAllocNavMesh();
	if( !m_navMesh )
	{
		LOG_ERROR << "Could not create Detour navmesh";
		return;
	}
	m_navQuery = dtAllocNavMeshQuery();
}

SGRX_Pathfinder::~SGRX_Pathfinder()
{
	dtFreeNavMesh( m_navMesh );
	dtFreeNavMeshQuery( m_navQuery );
}

bool SGRX_Pathfinder::Load( const ByteArray& data )
{
	uint8_t* navData = (uint8_t*) dtAlloc( data.size(), DT_ALLOC_PERM );
	memcpy( navData, data.data(), data.size() );
	
	dtStatus status;
	
	status = m_navMesh->init( navData, data.size(), DT_TILE_FREE_DATA );
	if( dtStatusFailed( status ) )
	{
		dtFree( navData );
		LOG_ERROR << "Could not init Detour navmesh";
		return false;
	}
	
	status = m_navQuery->init( m_navMesh, 2048 );
	if( dtStatusFailed( status ) )
	{
		LOG_ERROR << "Could not init Detour navmesh query";
		return false;
	}
	
	return true;
}

void SGRX_Pathfinder::DebugDraw()
{
	GR2D_SetWorldMatrix( Mat4::Basis( V3(1,0,0), V3(0,0,1), V3(0,1,0) ) );
	DetourDebugDraw ddd;
	duDebugDrawNavMesh( &ddd, *m_navMesh, 0 );
	GR2D_SetWorldMatrix( Mat4::Identity );
}

