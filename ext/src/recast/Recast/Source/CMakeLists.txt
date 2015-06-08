
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

add_library( recast STATIC Recast RecastAlloc RecastArea
	RecastContour RecastFilter RecastLayers RecastMesh
	RecastMeshDetail RecastRasterization RecastRegion )
set_target_properties( recast PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/recast/Recast/Include"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
