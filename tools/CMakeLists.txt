
# Lightmap renderer
add_executable( lmrender
	lmrender
	../engine/utils ../engine/mesh ../engine/image ../engine/dds
	../ext/src/lighter/lighter ../ext/src/lighter/lighter_math
	../ext/src/libjpg1 ../ext/src/libjpg2 ../ext/src/libjpg3
	../ext/src/zlib1 ../ext/src/zlib2 ../ext/src/zlib3
	../ext/src/libpng1
)
set_target_properties( lmrender PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tools"
	INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ext/src/zlib"
	COMPILE_FLAGS "-m32 -DLTR_STATIC"
	LINK_FLAGS "-m32"
)

# Ragdoll generator
add_executable( rdgen
	rdgen
	../engine/utils ../engine/mesh
)
set_target_properties( rdgen PROPERTIES
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-exceptions -g"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tools"
	COMPILE_FLAGS "-m32"
	LINK_FLAGS "-m32"
)
