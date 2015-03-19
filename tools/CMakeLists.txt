
# Lightmap renderer
add_executable( lmrender
	lmrender
	../ext/src/lighter/lighter ../ext/src/lighter/lighter_math
	../ext/src/zlib1 ../ext/src/zlib2 ../ext/src/zlib3
	../ext/src/libpng1
)
target_link_libraries( lmrender sgrx-core )
set_target_properties( lmrender PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/zlib"
	COMPILE_FLAGS "-m32 -DLTR_STATIC -O2"
	LINK_FLAGS "-m32 -O2"
)

# Blender cubemap converter
add_executable( blendcubedds blendcubedds )
target_link_libraries( blendcubedds sgrx-core )
set_target_properties( blendcubedds PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)

# Ragdoll generator
add_executable( rdgen
	rdgen
)
target_link_libraries( rdgen sgrx-core )
set_target_properties( rdgen PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)
