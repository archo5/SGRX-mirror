

#pragma once

#include <engine.hpp>
#include "levelcache.hpp"




struct EdMetaDataCont
{
	EdMetaDataCont();
	~EdMetaDataCont();
	void Reset();
	void LoadCache( const StringView& levname );
	void SaveCache( const StringView& levname );
	
	void RebuildNavMesh();
	void RebuildCovers();
	void RebuildMap();
	
	void DebugDrawCovers();
	
	// navmesh data
	ByteArray m_navMeshData;
	// cover data
	Array< LC_CoverPart > m_coverData;
	// map data
	Array< Vec2 > m_mapLines;
	Array< LC_Map_Layer > m_mapLayers;
};


