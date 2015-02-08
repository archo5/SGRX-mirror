
add_executable( lmrender
	lmrender
	../engine/utils ../engine/mesh
	../ext/src/lighter/lighter ../ext/src/lighter/lighter_math
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
