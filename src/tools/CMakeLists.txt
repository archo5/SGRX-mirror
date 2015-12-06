
# Mesh processor
add_executable( meshproc
	meshproc
	${EXTDIR}/src/bullet_LinearMath.cpp
)
target_link_libraries( meshproc sgrx-core )
set_target_properties( meshproc PROPERTIES
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
	INCLUDE_DIRECTORIES "${EXTDIR}/src"
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

# Blender cubemap converter
add_executable( blendcubedds blendcubedds )
target_link_libraries( blendcubedds sgrx-core )
set_target_properties( blendcubedds PROPERTIES
	COMPILE_FLAGS "${CORE_FLAGS}"
	LINK_FLAGS "${CORE_FLAGS}"
)

