
set( SGSVM_PATH "sgsvm" CACHE FILEPATH
	"Path to SGSVM (SGScript virtual machine)" )
set( MPDGEN_PATH "mpdgen.sgs" CACHE FILEPATH
	"Path to mpdgen.sgs (MetaProp generator script)" )

macro( mpd_spec HFILE SPECFILE )
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${HFILE}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SPECFILE}"
		COMMAND ${SGSVM_PATH} ARGS -p ${MPDGEN_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/${SPECFILE}" > "${CMAKE_CURRENT_SOURCE_DIR}/${HFILE}"
	)
endmacro( mpd_spec )
