

#include "engine.hpp"
#include "pathfinding.hpp"

#include <DetourNavMesh.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <../../DebugUtils/Include/DebugDraw.h>
#include <../../DebugUtils/Include/DetourDebugDraw.h>


#define MAX_PATH_POLYS 128


inline Vec3 RC2SGRX( const Vec3& v ){ return V3( v.x, v.z, v.y ); }
inline Vec3 SGRX2RC( const Vec3& v ){ return V3( v.x, v.z, v.y ); }


struct DetourDebugDraw : duDebugDraw
{
	DetourDebugDraw() : br( GR2D_GetBatchRenderer().Reset() ){}
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

bool SGRX_Pathfinder::Load( const uint8_t* ptr, size_t sz )
{
	uint8_t* navData = (uint8_t*) dtAlloc( sz, DT_ALLOC_PERM );
	memcpy( navData, ptr, sz );
	
	dtStatus status;
	
	status = m_navMesh->init( navData, sz, DT_TILE_FREE_DATA );
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

uint64_t SGRX_Pathfinder::FindPoly( const Vec3& pt, Vec3* outpt, const Vec3& ext )
{
	dtQueryFilter qfilter;
	
	Vec3 rc_pt = SGRX2RC( pt );
	Vec3 rc_ext = SGRX2RC( ext );
	
	dtPolyRef out = 0;
#if 1
	dtPolyRef polylist[ 128 ];
	int numpoly = 0, nearest = -1;
	float ndst = FLT_MAX;
	m_navQuery->queryPolygons( &rc_pt.x, &rc_ext.x, &qfilter, polylist, &numpoly, 128 );
	for( int i = 0; i < numpoly; ++i )
	{
		Vec3 closest = V3(0);
		m_navQuery->closestPointOnPoly( polylist[ i ], &rc_pt.x, &closest.x, NULL );
		
		float dst = ( closest - rc_pt ).Length();
		if( closest.y > rc_pt.y )
			dst *= 10; // if point is not above polygon, weigh distance appropriately (?)
		if( dst < ndst )
		{
			ndst = dst;
			nearest = i;
			if( outpt )
				*outpt = closest;
		}
	}
	if( nearest == -1 )
		return 0;
	out = polylist[ nearest ];
#else
	m_navQuery->findNearestPoly( &rc_pt.x, &rc_ext.x, &qfilter, &out, outpt ? &outpt->x : NULL );
#endif
	if( outpt )
		*outpt = RC2SGRX( *outpt );
	return out;
}

bool SGRX_Pathfinder::FindPath( const Vec3& from, uint64_t frompoly,
	const Vec3& to, uint64_t topoly, Array< Vec3 >& pts )
{
	dtQueryFilter qfilter;
	
	int numpoly = 0;
	dtPolyRef polylist[ MAX_PATH_POLYS ];
	int numpoints = 0;
	
	pts.clear();
	pts.resize( MAX_PATH_POLYS );
	
	Vec3 rc_from = SGRX2RC( from );
	Vec3 rc_to = SGRX2RC( to );
	
	dtStatus status = m_navQuery->findPath( frompoly, topoly,
		&rc_from.x, &rc_to.x, &qfilter, polylist, &numpoly, MAX_PATH_POLYS );
	if( numpoly && !( status & DT_PARTIAL_RESULT ) )
	{
		// In case of partial path, make sure the end point is clamped to the last polygon.
		Vec3 epos = rc_to;
		if( polylist[ numpoly - 1 ] != topoly )
			m_navQuery->closestPointOnPoly( polylist[ numpoly - 1 ], &rc_to.x, &epos.x, NULL );
		
		if( dtStatusSucceed( m_navQuery->findStraightPath( &rc_from.x, &epos.x,
			polylist, numpoly, &pts[0].x, NULL, NULL, &numpoints, MAX_PATH_POLYS, 0 ) ) )
		{
			pts.resize( numpoints );
			for( size_t i = 0; i < pts.size(); ++i )
				pts[ i ] = RC2SGRX( pts[ i ] );
			return true;
		}
	}
	pts.clear();
	return false;
}

bool SGRX_Pathfinder::FindPath( const Vec3& from, const Vec3& to, Array< Vec3 >& pts )
{
	Vec3 fxfrom = from, fxto = to;
	uint64_t pfrom = FindPoly( from, &fxfrom );
	if( !pfrom )
		return false;
	uint64_t pto = FindPoly( to, &fxto );
	if( !pto )
		return false;
	return FindPath( fxfrom, pfrom, fxto, pto, pts );
}

void SGRX_Pathfinder::DebugDraw()
{
	GR2D_SetWorldMatrix( Mat4::Basis( V3(1,0,0), V3(0,0,1), V3(0,1,0) ) );
	DetourDebugDraw ddd;
	duDebugDrawNavMesh( &ddd, *m_navMesh, DU_DRAWNAVMESH_OFFMESHCONS
		| DU_DRAWNAVMESH_CLOSEDLIST | DU_DRAWNAVMESH_COLOR_TILES );
	duDebugDrawNavMeshPortals( &ddd, *m_navMesh );
	GR2D_SetWorldMatrix( Mat4::Identity );
}

