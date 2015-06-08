
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

add_library( detour STATIC DetourAlloc DetourCommon DetourNavMesh
	DetourNavMeshBuilder DetourNavMeshQuery DetourNode )
set_target_properties( detour PROPERTIES
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/recast/Detour/Include"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
