
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )
link_directories( ${EXTDIR}/d3dx )
if(MSVC)
	link_directories( D:/DXSDK/Lib/x86 )
endif()

# Engine
add_library( sgrx-core SHARED
	engine.cpp engine.hpp engrender.cpp engbatchr.cpp engrsrc.cpp
	enganim.cpp enganim.hpp
	engext.cpp engext.hpp
	edgui.cpp edgui.hpp
	image.cpp font.cpp mesh.cpp culling.cpp
	utils.cpp utils.hpp
	dds.c dds.h
	${EXTDIR}/src/zlib1 ${EXTDIR}/src/zlib2 ${EXTDIR}/src/zlib3
	${EXTDIR}/src/libpng1
	${EXTDIR}/src/libjpg1 ${EXTDIR}/src/libjpg2 ${EXTDIR}/src/libjpg3
	${EXTDIR}/src/freetype1
	${EXTDIR}/src/sgscript/sgs_regex.c
)
target_link_libraries( sgrx-core SDL2 )
set_target_properties( sgrx-core PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}/include;${EXTDIR}/include/freetype;${EXTDIR}/src;${EXTDIR}/src/zlib"
	COMPILE_FLAGS "${CORE_FLAGS} -DENGINE_BUILDING"
	LINK_FLAGS "${CORE_FLAGS}"
)

if(NOT (CMAKE_SYSTEM_NAME STREQUAL WindowsPhone OR CMAKE_SYSTEM_NAME STREQUAL WindowsStore))
# Renderer - D3D9
add_library( sgrx-render-d3d9 SHARED renderer_d3d9 )
target_link_libraries( sgrx-render-d3d9 d3d9 d3dx9 sgrx-core )
set_target_properties( sgrx-render-d3d9 PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}"
	COMPILE_FLAGS "${CORE_FLAGS} ${MINGW_D3D_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
endif()

# Renderer - D3D11
if(MSVC)
	set(D3DCOMPILER_LIB d3dcompiler)
else()
	set(D3DCOMPILER_LIB d3dcompiler_42)
endif()
add_library( sgrx-render-d3d11 SHARED renderer_d3d11 )
target_link_libraries( sgrx-render-d3d11 d3d11 ${D3DCOMPILER_LIB} sgrx-core )
set_target_properties( sgrx-render-d3d11 PROPERTIES
	INCLUDE_DIRECTORIES "D:/DXSDK/Include"
	COMPILE_FLAGS "${CORE_FLAGS} ${MINGW_D3D_FLAGS} -DENABLE_SHADER_COMPILING"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Sound - FMOD
add_library( sgrx-sound-fmod SHARED sound_fmod )
target_link_libraries( sgrx-sound-fmod fmod fmodstudio sgrx-core )
set_target_properties( sgrx-sound-fmod PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}/include/fmod"
	COMPILE_FLAGS "${CORE_FLAGS} -DSOUND_BUILDING"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Pathfinding - Detour
add_library( sgrx-pathfinding STATIC pathfinding )
target_link_libraries( sgrx-pathfinding detour rcd_debug )
set_target_properties( sgrx-pathfinding PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}/src/recast/Detour/Include;${EXTDIR}/src/recast/DetourTileCache/Include"
	COMPILE_FLAGS "${PF_CORE_FLAGS} -DPATHFINDING_BUILDING"
	LINK_FLAGS "${PF_CORE_FLAGS}"
)

# Physics - Bullet
add_library( sgrx-physics-bullet SHARED physics_bullet
	${EXTDIR}/src/bullet_BulletCollision.cpp
	${EXTDIR}/src/bullet_BulletDynamics.cpp
	${EXTDIR}/src/bullet_LinearMath.cpp
)
target_link_libraries( sgrx-physics-bullet sgrx-core )
set_target_properties( sgrx-physics-bullet PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}/src;${EXTDIR}/src/bullet/src"
	COMPILE_FLAGS "${PF_CORE_FLAGS} -DPHYSICS_BUILDING"
	LINK_FLAGS "${PF_CORE_FLAGS}"
)

# Scripting - SGScript
add_library( sgrx-script SHARED script
	${EXTDIR}/src/sgscript.c
)
target_link_libraries( sgrx-script sgrx-core )
set_target_properties( sgrx-script PROPERTIES
	INCLUDE_DIRECTORIES "${EXTDIR}/src/sgscript"
	COMPILE_FLAGS "${CORE_FLAGS} -DSCRIPT_BUILDING -DSGS_COMPILE_MODULE -DSGS_DLL=1"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Launchers
add_executable( sgrx-debug launcher )
add_executable( sgrx launcher )
target_link_libraries( sgrx-debug sgrx-core )
target_link_libraries( sgrx sgrx-core )
set_target_properties( sgrx-debug PROPERTIES
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS} ${LINK_STRIP_FLAG}"
)
set_target_properties( sgrx PROPERTIES
	COMPILE_DEFINITIONS "SGRX_RELEASE=1"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS} ${LINK_STRIP_FLAG} ${LINK_WINDOWS_FLAG}"
)
