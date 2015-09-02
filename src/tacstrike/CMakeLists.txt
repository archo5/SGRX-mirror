
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Core game library
add_library( coregamelib STATIC level cppbc_level entities systems cppbc_systems ../common/scritem ../common/cppbc_scritem )
set_target_properties( coregamelib PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Game
add_library( game SHARED game gameui tsui entities_ts cppbc_entities_ts )
target_link_libraries( game coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( game PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DTSGAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# BRSD 4 Game
add_library( jmpgame SHARED brsd4game entities_brsd4 )
target_link_libraries( jmpgame coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( jmpgame PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DBRSD4GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# LD33 Game
add_library( ld33game SHARED ld33game entities_ld33 cppbc_entities_ts )
target_link_libraries( ld33game coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( ld33game PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DLD33GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

