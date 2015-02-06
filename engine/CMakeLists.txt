
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

# Direct3D9
add_library( d3dx9 SHARED IMPORTED )
set_target_properties( d3dx9 PROPERTIES
	IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ext/d3dx/d3dx9_42.dll"
	IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/ext/d3dx/libd3dx9.a"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${CMAKE_SOURCE_DIR}/ext/d3dx/d3dx9_42.dll" )

# SDL2
add_library( SDL2 SHARED IMPORTED )
set_target_properties( SDL2 PROPERTIES
	IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ext/bin-win32/SDL2.dll"
	IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/ext/lib-win32/libSDL2.dll.a"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${CMAKE_SOURCE_DIR}/ext/bin-win32/SDL2.dll" )

# Engine
add_library( sgrx-core SHARED
	engine image font mesh utils dds
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
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)

# Renderer - D3D9
add_library( sgrx-render-d3d9 SHARED renderer_d3d9 )
target_link_libraries( sgrx-render-d3d9 d3d9 d3dx9 sgrx-core )
set_target_properties( sgrx-render-d3d9 PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)

# Launchers
add_executable( sgrx-debug launcher )
add_executable( sgrx launcher )
target_link_libraries( sgrx-debug sgrx-core )
target_link_libraries( sgrx sgrx-core )
set_target_properties( sgrx-debug PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32 -s"
)
set_target_properties( sgrx PROPERTIES
	COMPILE_DEFINITIONS "SGRX_RELEASE=1"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32 -s -mwindows"
)

# Copy external libraries
message( STATUS "Copying shared libraries." )
file( COPY ${ExternalSharedLibraries} DESTINATION "${CMAKE_SOURCE_DIR}/bin" NO_SOURCE_PERMISSIONS )
