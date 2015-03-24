
cmake_minimum_required( VERSION 2.8 )

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

# FMOD
add_library( fmod SHARED IMPORTED )
set_target_properties( fmod PROPERTIES
	IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ext/bin-win32/fmod.dll"
	IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/ext/lib-win32/libfmod.a"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${CMAKE_SOURCE_DIR}/ext/bin-win32/fmod.dll" )
add_library( fmodstudio SHARED IMPORTED )
set_target_properties( fmodstudio PROPERTIES
	IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/ext/bin-win32/fmodstudio.dll"
	IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/ext/lib-win32/libfmodstudio.a"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${CMAKE_SOURCE_DIR}/ext/bin-win32/fmodstudio.dll" )

add_subdirectory( engine )
add_subdirectory( game-tacstrike )
add_subdirectory( tools )

# Copy external libraries
message( STATUS "Copying shared libraries." )
file( COPY ${ExternalSharedLibraries} DESTINATION "${CMAKE_SOURCE_DIR}/bin" NO_SOURCE_PERMISSIONS )
