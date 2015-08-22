
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Game
add_library( game SHARED game gameui tsui level entities entities_ts systems
	../common/scritem ../common/cppbc_scritem )
target_link_libraries( game sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( game PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DTSGAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# BRSD 4 Game
add_library( jmpgame SHARED brsd4game level entities entities_brsd4 systems
	../common/scritem ../common/cppbc_scritem )
target_link_libraries( jmpgame sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( jmpgame PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DBRSD4GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# LD33 Game
add_library( ld33game SHARED ld33game level entities entities_ts entities_ld33 systems
	../common/scritem ../common/cppbc_scritem )
target_link_libraries( ld33game sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( ld33game PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DLD33GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

