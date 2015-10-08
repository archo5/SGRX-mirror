
cmake_minimum_required( VERSION 2.8 )
cmake_policy( SET CMP0048 NEW )
project( SGRX )


set(SRCDIR "${CMAKE_SOURCE_DIR}/src")
set(EXTDIR "${CMAKE_SOURCE_DIR}/ext")
if(NOT MSVC)
	set(PF_CORE_FLAGS "-m32")
	set(CORE_FLAGS "${PF_CORE_FLAGS} -Wall -Wno-narrowing -Wno-missing-braces -g -fno-rtti -fno-exceptions -fno-unwind-tables -ffunction-sections")
	if(NOT WIN32)
		set(CORE_FLAGS "${CORE_FLAGS} -fPIC")
		set(MINGW_D3D_FLAGS "")
	else()
		set(MINGW_D3D_FLAGS "-Wno-unknown-pragmas -Wno-comment")
	endif(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CORE_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CORE_FLAGS}")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${CORE_FLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CORE_FLAGS}")
	set(LINK_STRIP_FLAG "-s")
	set(LINK_WINDOWS_FLAG "-mwindows")
	set(LIB_FMOD_NAME "libfmod.a")
	set(LIB_FMOD_STUDIO_NAME "libfmodstudio.a")
else()
	set(CORE_FLAGS "")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /W3 /Zi /wd4244 /wd4996")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /DEBUG")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
	set(LINK_STRIP_FLAG "") # symbols are separate, MSVC doesn't need this
	set(LINK_WINDOWS_FLAG "/SUBSYSTEM:WINDOWS")
	set(LIB_FMOD_NAME "fmod_vc.lib")
	set(LIB_FMOD_STUDIO_NAME "fmodstudio_vc.lib")
	set(MINGW_D3D_FLAGS "")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL WindowsPhone OR CMAKE_SYSTEM_NAME STREQUAL WindowsStore)
	set(RESTRICTED TRUE)
else()
	set(RESTRICTED FALSE)
endif()


# Direct3D9
#add_library( d3dx9 SHARED IMPORTED )
#set_target_properties( d3dx9 PROPERTIES
#	IMPORTED_LOCATION "${EXTDIR}/d3dx/d3dx9_42.dll"
#	IMPORTED_IMPLIB "${EXTDIR}/d3dx/libd3dx9.a"
#	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
#)
list( APPEND ExternalSharedLibraries "${EXTDIR}/d3dx/d3dx9_42.dll" )

# SDL2
add_library( SDL2 SHARED IMPORTED )
set_target_properties( SDL2 PROPERTIES
	IMPORTED_LOCATION "${EXTDIR}/bin-win32/SDL2.dll"
	IMPORTED_IMPLIB "${EXTDIR}/lib-win32/libSDL2.dll.a"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${EXTDIR}/bin-win32/SDL2.dll" )

# FMOD
add_library( fmod SHARED IMPORTED )
set_target_properties( fmod PROPERTIES
	IMPORTED_LOCATION "${EXTDIR}/bin-win32/fmod.dll"
	IMPORTED_IMPLIB "${EXTDIR}/lib-win32/${LIB_FMOD_NAME}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${EXTDIR}/bin-win32/fmod.dll" )
add_library( fmodstudio SHARED IMPORTED )
set_target_properties( fmodstudio PROPERTIES
	IMPORTED_LOCATION "${EXTDIR}/bin-win32/fmodstudio.dll"
	IMPORTED_IMPLIB "${EXTDIR}/lib-win32/${LIB_FMOD_STUDIO_NAME}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
list( APPEND ExternalSharedLibraries "${EXTDIR}/bin-win32/fmodstudio.dll" )

# Assimp
add_subdirectory( ext/src/assimp )

# Recast / Detour
add_subdirectory( ext/src/recast/Recast/Source )
add_subdirectory( ext/src/recast/Detour/Source )
add_subdirectory( ext/src/recast/DebugUtils/Source )

add_subdirectory( src/engine )
add_subdirectory( src/common )
add_subdirectory( src/tacstrike )
add_subdirectory( src/smallgames )

if(NOT RESTRICTED)
add_subdirectory( src/tools )
endif()

# Copy external libraries
message( STATUS "Copying shared libraries." )
file( COPY ${ExternalSharedLibraries} DESTINATION "${CMAKE_SOURCE_DIR}/bin" NO_SOURCE_PERMISSIONS )
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	file( COPY ${ExternalSharedLibraries} DESTINATION "${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG}" NO_SOURCE_PERMISSIONS )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
