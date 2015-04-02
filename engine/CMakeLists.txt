
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Engine
add_library( sgrx-core SHARED
	engine enganim edgui image font mesh culling utils dds
	../ext/src/zlib1 ../ext/src/zlib2 ../ext/src/zlib3
	../ext/src/libpng1
	../ext/src/libjpg1 ../ext/src/libjpg2 ../ext/src/libjpg3
	../ext/src/freetype1
)
target_link_libraries( sgrx-core SDL2 )
set_target_properties( sgrx-core PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/include;${CMAKE_SOURCE_DIR}/ext/include/freetype;${CMAKE_SOURCE_DIR}/ext/src/zlib"
	LINKER_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/lib-win32"
	COMPILE_FLAGS "-m32 -DENGINE_BUILDING -Wall -Wno-narrowing"
	LINK_FLAGS "-m32"
)

# Renderer - D3D9
add_library( sgrx-render-d3d9 SHARED renderer_d3d9 )
target_link_libraries( sgrx-render-d3d9 d3d9 d3dx9 sgrx-core )
set_target_properties( sgrx-render-d3d9 PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32 -Wall -Wno-narrowing -Wno-unknown-pragmas -Wno-comment"
	LINK_FLAGS "-m32"
)

# Renderer - D3D11
add_library( sgrx-render-d3d11 SHARED renderer_d3d11 )
target_link_libraries( sgrx-render-d3d11 d3d11 sgrx-core )
set_target_properties( sgrx-render-d3d11 PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32 -Wall -Wno-narrowing -Wno-unknown-pragmas -Wno-comment"
	LINK_FLAGS "-m32 -L${CMAKE_SOURCE_DIR}/ext/d3dx"
)

# Sound - FMOD
add_library( sgrx-sound-fmod SHARED sound_fmod )
target_link_libraries( sgrx-sound-fmod fmod fmodstudio sgrx-core )
set_target_properties( sgrx-sound-fmod PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/include/fmod"
	COMPILE_FLAGS "-m32 -DSOUND_BUILDING"
	LINK_FLAGS "-m32"
)

# Physics - Bullet
add_library( sgrx-physics-bullet SHARED physics_bullet
	../ext/src/bullet_BulletCollision.cpp
	../ext/src/bullet_BulletDynamics.cpp
	../ext/src/bullet_LinearMath.cpp
)
target_link_libraries( sgrx-physics-bullet sgrx-core )
set_target_properties( sgrx-physics-bullet PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/bullet/src"
	COMPILE_FLAGS "-m32 -DPHYSICS_BUILDING"
	LINK_FLAGS "-m32"
)

# Scripting - SGScript
add_library( sgrx-script SHARED script
	../ext/src/sgscript.c
)
target_link_libraries( sgrx-script sgrx-core )
set_target_properties( sgrx-script PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/sgscript"
	COMPILE_FLAGS "-m32 -DSCRIPT_BUILDING -DSGS_COMPILE_MODULE -DSGS_DLL=1 -Wall -Wno-narrowing"
	LINK_FLAGS "-m32"
)

# Launchers
add_executable( sgrx-debug launcher )
add_executable( sgrx launcher )
target_link_libraries( sgrx-debug sgrx-core )
target_link_libraries( sgrx sgrx-core )
set_target_properties( sgrx-debug PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32 -Wall -Wno-narrowing"
	LINK_FLAGS "-m32 -s"
)
set_target_properties( sgrx PROPERTIES
	COMPILE_DEFINITIONS "SGRX_RELEASE=1"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32 -Wall -Wno-narrowing"
	LINK_FLAGS "-m32 -s -mwindows"
)
