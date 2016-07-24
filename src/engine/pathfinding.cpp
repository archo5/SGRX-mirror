

#include "engine.hpp"
#include "pathfinding.hpp"

#include <DetourNavMesh.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <../../DebugUtils/Include/DebugDraw.h>
#include <../../DebugUtils/Include/DetourDebugDraw.h>


#define MAX_PATH_POLYS 128


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

bool SGRX_Pathfinder::Load( ByteView data )
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

uint32_t SGRX_Pathfinder::FindPoly( const Vec3& pt, Vec3* outpt, const Vec3& ext )
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

Vec3 SGRX_Pathfinder::GetPolyPos( uint32_t poly )
{
	Vec3 out = V3(0);
	const dtMeshTile* tt;
	const dtPoly* pp;
	if( dtStatusFailed( m_navMesh->getTileAndPolyByRef( poly, &tt, &pp ) )
		|| pp->vertCount == 0 )
		return out;
	for( uint8_t i = 0; i < pp->vertCount; ++i )
	{
		out += V3P( &tt->verts[ pp->verts[ i ] * 3 ] );
	}
	return RC2SGRX( out / pp->vertCount );
}

int SGRX_Pathfinder::GetPolyNeighbors( uint32_t poly, uint32_t* out, int size )
{
	const dtMeshTile* tt;
	const dtPoly* pp;
	if( dtStatusFailed( m_navMesh->getTileAndPolyByRef( poly, &tt, &pp ) )
		|| pp->vertCount == 0 )
		return 0;
	int count = 0;
	for( int i = 0; i < pp->vertCount && count < size; ++i )
	{
		if( pp->neis[ i ] == 0 )
			continue;
		if( pp->neis[ i ] & DT_EXT_LINK )
		{
			for( unsigned int k = pp->firstLink; k != DT_NULL_LINK; k = tt->links[ k ].next )
			{
				if( tt->links[ k ].edge == i )
				{
					out[ count++ ] = tt->links[ k ].ref;
					goto foundextlink;
				}
			}
foundextlink:;
		}
		else
		{
			out[ count++ ] = m_navMesh->getPolyRefBase( tt ) | dtPolyRef( pp->neis[ i ] );
		}
	}
	return count;
}

bool SGRX_Pathfinder::FindPath( const Vec3& from, uint32_t frompoly,
	const Vec3& to, uint32_t topoly, Array< Vec3 >& pts )
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
	uint32_t pfrom = FindPoly( from, &fxfrom );
	if( !pfrom )
		return false;
	uint32_t pto = FindPoly( to, &fxto );
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

