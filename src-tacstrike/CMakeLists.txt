
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Game
add_library( game SHARED game level actors )
target_link_libraries( game sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet )
set_target_properties( game PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/engine;${CMAKE_SOURCE_DIR}/src-common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# BRSD 4 Game
add_library( jmpgame SHARED brsd4game level )
target_link_libraries( jmpgame sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet )
set_target_properties( jmpgame PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/engine;${CMAKE_SOURCE_DIR}/src-common"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS} -DBRSD4GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

