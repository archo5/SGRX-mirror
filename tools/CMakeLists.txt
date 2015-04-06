
# Lightmap renderer
add_executable( lmrender
	lmrender
	../ext/src/lighter/lighter ../ext/src/lighter/lighter_math
	../ext/src/zlib1 ../ext/src/zlib2 ../ext/src/zlib3
	../ext/src/libpng1
)
target_link_libraries( lmrender sgrx-core )
target_link_libraries( lmrender sgrx-script )
set_target_properties( lmrender PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/zlib"
	COMPILE_FLAGS "${CORE_FLAGS} -DLTR_STATIC -O2"
	LINK_FLAGS "${CORE_FLAGS} -O2"
)

# Blender cubemap converter
add_executable( blendcubedds blendcubedds )
target_link_libraries( blendcubedds sgrx-core )
set_target_properties( blendcubedds PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Ragdoll generator
add_executable( rdgen
	rdgen
)
target_link_libraries( rdgen sgrx-core )
set_target_properties( rdgen PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)
