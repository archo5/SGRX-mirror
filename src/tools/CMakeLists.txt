
# Lightmap renderer
add_executable( lmrender
	lmrender
	${EXTDIR}/src/lighter/lighter ${EXTDIR}/src/lighter/lighter_math
	${EXTDIR}/src/zlib1 ${EXTDIR}/src/zlib2 ${EXTDIR}/src/zlib3
	${EXTDIR}/src/libpng1
)
target_link_libraries( lmrender sgrx-core )
target_link_libraries( lmrender sgrx-script )
set_target_properties( lmrender PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${EXTDIR}/src;${EXTDIR}/src/zlib"
	COMPILE_FLAGS "${CORE_FLAGS} -DLTR_STATIC -O2"
	LINK_FLAGS "${CORE_FLAGS} -O2"
)

# Mesh processor
add_executable( meshproc
	meshproc
	${EXTDIR}/src/bullet_LinearMath.cpp
)
target_link_libraries( meshproc sgrx-core )
set_target_properties( meshproc PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${EXTDIR}/src"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Texture processor
add_executable( texproc
	texproc
)
target_link_libraries( texproc sgrx-core )
set_target_properties( texproc PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	INCLUDE_DIRECTORIES "${EXTDIR}/src"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Blender cubemap converter
add_executable( blendcubedds blendcubedds )
target_link_libraries( blendcubedds sgrx-core )
set_target_properties( blendcubedds PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

