
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

add_library( rcd_debug STATIC DebugDraw DetourDebugDraw RecastDebugDraw RecastDump )
set_target_properties( rcd_debug PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/recast/DebugUtils/Include;${CMAKE_SOURCE_DIR}/ext/src/recast/Detour/Include;${CMAKE_SOURCE_DIR}/ext/src/recast/DetourTileCache/Include;${CMAKE_SOURCE_DIR}/ext/src/recast/Recast/Include"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
