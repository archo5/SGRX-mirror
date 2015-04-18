
cmake_minimum_required( VERSION 2.8 )
set( CMAKE_SHARED_LIBRARY_PREFIX "" )

if(NOT RESTRICTED)
# Editor
add_library( editor SHARED editor compiler )
target_link_libraries( editor sgrx-core sgrx-script sgrx-sound-fmod )
set_target_properties( editor PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Particle system editor
add_library( psedit SHARED psedit )
target_link_libraries( psedit sgrx-core )
set_target_properties( psedit PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/engine"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
endif()

