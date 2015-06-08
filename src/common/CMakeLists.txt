
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

if(NOT RESTRICTED)
# Editor
add_library( editor SHARED mapedit mapedit_ent mapedit_block mapedit_patch mapedit_edit compiler )
target_link_libraries( editor sgrx-core sgrx-script sgrx-sound-fmod recast detour )
set_target_properties( editor PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${EXTDIR}/src/recast/Recast/Include;${EXTDIR}/src/recast/Detour/Include"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Particle system editor
add_library( psedit SHARED psedit )
target_link_libraries( psedit sgrx-core )
set_target_properties( psedit PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Cutscene editor
add_library( csedit SHARED csedit )
target_link_libraries( csedit sgrx-core )
set_target_properties( csedit PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
endif()

