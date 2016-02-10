
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Core game library
add_library( coregamelib STATIC
	level.hpp level.cpp cppbc_level.cpp
	entities.hpp entities.cpp cppbc_entities.cpp
	systems.hpp systems.cpp cppbc_systems.cpp
	../common/gamegui.hpp ../common/gamegui.cpp ../common/cppbc_gamegui.cpp
	../common/scritem.hpp ../common/scritem.cpp ../common/cppbc_scritem.cpp )
set_target_properties( coregamelib PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Test Suite
add_library( testsuite SHARED testsuite.cpp
	../common/gamegui.hpp ../common/gamegui.cpp ../common/cppbc_gamegui.cpp )
target_link_libraries( testsuite sgrx-core sgrx-script )
set_target_properties( testsuite PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS} -DTESTSUITE"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Game
add_library( game SHARED game entities_ts cppbc_entities_ts ) # gameui tsui
target_link_libraries( game coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( game PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS} -DTSGAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

if(0)
# BRSD 4 Game
add_library( jmpgame SHARED brsd4game entities_brsd4 )
target_link_libraries( jmpgame coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( jmpgame PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS} -DBRSD4GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# LD33 Game
add_library( ld33game SHARED ld33game entities_ld33 cppbc_entities_ts )
target_link_libraries( ld33game coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( ld33game PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS} -DLD33GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# MLD62 Game
add_library( mld62game SHARED mld62game )
target_link_libraries( mld62game coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( mld62game PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS} -DMLD62GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)

# ISR3 Game
add_library( isr3game SHARED isr3game isr3entities cppbc_isr3entities entities_ts cppbc_entities_ts )
target_link_libraries( isr3game coregamelib sgrx-core sgrx-script sgrx-sound-fmod sgrx-physics-bullet sgrx-pathfinding )
set_target_properties( isr3game PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${SRCDIR}/common"
	COMPILE_FLAGS "${CORE_FLAGS} -DISR3GAME"
	LINK_FLAGS "${CORE_FLAGS}"
)
endif()

