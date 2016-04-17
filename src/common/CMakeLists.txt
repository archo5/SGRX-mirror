
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Core game library
add_library( sgrx-gfw SHARED
	level.hpp level.cpp
	entities.hpp entities.cpp
	systems.hpp systems.cpp
	gfwcore.hpp gfwcore.cpp
	gamegui.hpp gamegui.cpp
	cppbc_gfw.cpp )
target_link_libraries( sgrx-gfw sgrx-pathfinding sgrx-script sgrx-core )
set_target_properties( sgrx-gfw PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	COMPILE_FLAGS "${CORE_FLAGS} -DGFW_BUILDING"
	LINK_FLAGS "${CORE_FLAGS}"
)
cppbc_headers( sgrx-gfw gfw.hpp )

if(NOT RESTRICTED)
# Lightmap renderer
add_library( lighter SHARED
	${EXTDIR}/src/lighter/lighter ${EXTDIR}/src/lighter/lighter_math )
set_target_properties( lighter PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}/src"
	COMPILE_FLAGS "${CORE_FLAGS} -DLTRBUILD -O2"
	LINK_FLAGS "${CORE_FLAGS} -O2"
)

# Editor
mpd_spec( mapedit.mpd.hpp mapedit.mpd.sgs )
add_library( editor SHARED
	mapedit.hpp mapedit.cpp
	mapedit_graphics.hpp mapedit_graphics.cpp
	mapedit_edit.cpp
	mapedit_ent.cpp mapedit_block.cpp mapedit_mpath.cpp mapedit_patch.cpp
	compiler.hpp compiler.cpp
	mapedit.mpd.hpp )
target_link_libraries( editor
	sgrx-gfw sgrx-core sgrx-script sgrx-physics-bullet sgrx-sound-fmod
	recast detour lighter )
set_target_properties( editor PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${EXTDIR}/src;${EXTDIR}/src/recast/Recast/Include;${EXTDIR}/src/recast/Detour/Include"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Asset editor
mpd_spec( aedit.mpd.hpp aedit.mpd.sgs )
add_library( aedit SHARED aedit.cpp assetcomp.hpp assetcomp.cpp aedit.mpd.hpp
	${EXTDIR}/src/zlib1 ${EXTDIR}/src/zlib2 ${EXTDIR}/src/zlib3
	${EXTDIR}/src/libpng1 )
target_link_libraries( aedit sgrx-core assimp )
set_target_properties( aedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine;${EXTDIR}/src;${EXTDIR}/src/assimp/include;${EXTDIR}/src/zlib"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Particle system editor
add_library( psedit SHARED psedit )
target_link_libraries( psedit sgrx-core )
set_target_properties( psedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Character editor
add_library( charedit SHARED charedit )
target_link_libraries( charedit sgrx-core sgrx-physics-bullet )
set_target_properties( charedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Cutscene editor
add_library( csedit SHARED csedit )
target_link_libraries( csedit sgrx-core )
set_target_properties( csedit PROPERTIES
	INCLUDE_DIRECTORIES "${SRCDIR}/engine"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
endif()

