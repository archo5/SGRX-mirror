

#include "mapedit.hpp"


EdMetaDataCont::EdMetaDataCont()
{
}

EdMetaDataCont::~EdMetaDataCont()
{
}

void EdMetaDataCont::Reset()
{
	m_navMeshData.clear();
	m_coverData.clear();
}

void EdMetaDataCont::LoadCache( const StringView& levname )
{
	char fname[ 256 ];
	sgrx_snprintf( fname, sizeof(fname), SGRXPATH_CACHE_LEVELS "/%s" SGRX_LEVEL_LMCACHE_SFX, StackString<200>(levname).str );
	
	ByteArray ba;
	if( FS_LoadBinaryFile( fname, ba ) == false )
	{
		LOG_WARNING << "Could not load metadata cache";
		return;
	}
	
	ByteReader br( ba );
	br.marker( "SGRXMDC0" );
	if( br.error )
	{
		LOG_WARNING << "AICACHE: File format error";
		return;
	}
	
	br << m_navMeshData;
	br << m_coverData;
	br << m_mapLines;
	br << m_mapLayers;
	
}

void EdMetaDataCont::SaveCache( const StringView& levname )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	bw.marker( "SGRXMDC0" );
	
	bw << m_navMeshData;
	bw << m_coverData;
	bw << m_mapLines;
	bw << m_mapLayers;
	
	char path[ 256 ], fname[ 256 ];
	sgrx_snprintf( path, sizeof(path),
		SGRXPATH_CACHE_LEVELS "/%s" SGRX_LEVEL_DIR_SFX,
		StackString<200>(levname).str );
	sgrx_snprintf( fname, sizeof(fname),
		SGRXPATH_CACHE_LEVELS "/%s" SGRX_LEVEL_LMCACHE_SFX,
		StackString<200>(levname).str );
	
	FS_DirCreate( SGRXPATH_CACHE_LEVELS );
	FS_DirCreate( path );
	FS_SaveBinaryFile( fname, ba.data(), ba.size() );
}

void EdMetaDataCont::RebuildNavMesh()
{
	LevelCache* LC = g_UIFrame->CreateCache();
	LC->GatherMeshes( LevelCache::GM_Navigation );
	LC->GenerateNavmesh( m_navMeshData );
	delete LC;
}

void EdMetaDataCont::RebuildCovers()
{
	LevelCache* LC = g_UIFrame->CreateCache();
	LC->GatherMeshes( LevelCache::GM_Navigation );
	LC->GenerateCoverData( m_navMeshData, m_coverData );
	delete LC;
}

void EdMetaDataCont::RebuildMap()
{
	LevelCache* LC = g_UIFrame->CreateCache();
	LC->GatherMeshes( LevelCache::GM_Render );
	LC->RemoveHiddenSurfaces();
	LC->GenerateLines();
	m_mapLines = LC->m_mapLines;
	m_mapLayers = LC->m_mapLayers;
	delete LC;
}


