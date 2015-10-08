
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

if(NOT RESTRICTED)
# Lightmap renderer
add_library( lighter SHARED
	${EXTDIR}/src/lighter/lighter ${EXTDIR}/src/lighter/lighter_math )
set_target_properties( lighter PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${EXTDIR}/src"
	COMPILE_FLAGS "${CORE_FLAGS} -DLTRBUILD -O2"
	LINK_FLAGS "${CORE_FLAGS} -O2"
)

# Editor
add_library( editor SHARED
	mapedit mapedit_ent mapedit_block mapedit_patch mapedit_edit
	compiler scritem cppbc_scritem )
target_link_libraries( editor
	sgrx-core sgrx-script sgrx-physics-bullet sgrx-sound-fmod
	recast detour lighter )
set_target_properties( editor PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${EXTDIR}/src;${EXTDIR}/src/recast/Recast/Include;${EXTDIR}/src/recast/Detour/Include"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Asset editor
add_library( aedit SHARED aedit.cpp assetcomp.hpp assetcomp.cpp
	${EXTDIR}/src/zlib1 ${EXTDIR}/src/zlib2 ${EXTDIR}/src/zlib3
	${EXTDIR}/src/libpng1 )
target_link_libraries( aedit sgrx-core assimp )
set_target_properties( aedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${EXTDIR}/src;${EXTDIR}/src/assimp/include;${EXTDIR}/src/zlib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Particle system editor
add_library( psedit SHARED psedit )
target_link_libraries( psedit sgrx-core )
set_target_properties( psedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Character editor
add_library( charedit SHARED charedit )
target_link_libraries( charedit sgrx-core sgrx-physics-bullet )
set_target_properties( charedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Cutscene editor
add_library( csedit SHARED csedit )
target_link_libraries( csedit sgrx-core )
set_target_properties( csedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
endif()

