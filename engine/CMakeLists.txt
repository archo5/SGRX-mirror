
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )
link_directories( D:/DXSDK/Lib/x86 ${CMAKE_SOURCE_DIR}/ext/d3dx )

# Engine
add_library( sgrx-core SHARED
	engine.cpp engine.hpp engrender.cpp enganim.cpp enganim.hpp edgui.cpp edgui.hpp
	image.cpp font.cpp mesh.cpp culling.cpp renderer.hpp utils.cpp utils.hpp dds.c dds.h
	../ext/src/zlib1 ../ext/src/zlib2 ../ext/src/zlib3
	../ext/src/libpng1
	../ext/src/libjpg1 ../ext/src/libjpg2 ../ext/src/libjpg3
	../ext/src/freetype1
)
target_link_libraries( sgrx-core SDL2 )
set_target_properties( sgrx-core PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/include;${CMAKE_SOURCE_DIR}/ext/include/freetype;${CMAKE_SOURCE_DIR}/ext/src/zlib"
	COMPILE_FLAGS "${CORE_FLAGS} -DENGINE_BUILDING"
	LINK_FLAGS "${CORE_FLAGS}"
)

if(NOT (CMAKE_SYSTEM_NAME STREQUAL WindowsPhone OR CMAKE_SYSTEM_NAME STREQUAL WindowsStore))
# Renderer - D3D9
add_library( sgrx-render-d3d9 SHARED renderer_d3d9 )
target_link_libraries( sgrx-render-d3d9 d3d9 d3dx9 sgrx-core )
set_target_properties( sgrx-render-d3d9 PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
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
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "D:/DXSDK/Include"
	COMPILE_FLAGS "${CORE_FLAGS} ${MINGW_D3D_FLAGS} -DENABLE_SHADER_COMPILING"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Sound - FMOD
add_library( sgrx-sound-fmod SHARED sound_fmod )
target_link_libraries( sgrx-sound-fmod fmod fmodstudio sgrx-core )
set_target_properties( sgrx-sound-fmod PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/include/fmod"
	COMPILE_FLAGS "${CORE_FLAGS} -DSOUND_BUILDING"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Physics - Bullet
add_library( sgrx-physics-bullet SHARED physics_bullet
	../ext/src/bullet_BulletCollision.cpp
	../ext/src/bullet_BulletDynamics.cpp
	../ext/src/bullet_LinearMath.cpp
)
target_link_libraries( sgrx-physics-bullet sgrx-core )
set_target_properties( sgrx-physics-bullet PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/bullet/src"
	COMPILE_FLAGS "${PF_CORE_FLAGS} -DPHYSICS_BUILDING"
	LINK_FLAGS "${PF_CORE_FLAGS}"
)

# Scripting - SGScript
add_library( sgrx-script SHARED script
	../ext/src/sgscript.c
)
target_link_libraries( sgrx-script sgrx-core )
set_target_properties( sgrx-script PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/sgscript"
	COMPILE_FLAGS "${CORE_FLAGS} -DSCRIPT_BUILDING -DSGS_COMPILE_MODULE -DSGS_DLL=1"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Launchers
add_executable( sgrx-debug launcher )
add_executable( sgrx launcher )
target_link_libraries( sgrx-debug sgrx-core )
target_link_libraries( sgrx sgrx-core )
set_target_properties( sgrx-debug PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS} ${LINK_STRIP_FLAG}"
)
set_target_properties( sgrx PROPERTIES
	COMPILE_DEFINITIONS "SGRX_RELEASE=1"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS} ${LINK_STRIP_FLAG} ${LINK_WINDOWS_FLAG}"
)
