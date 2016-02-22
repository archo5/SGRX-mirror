
set( SGSVM_PATH "sgsvm" CACHE FILEPATH
	"Path to SGSVM (SGScript virtual machine)" )
set( SGSCPPBC_PATH "../ext/cppbc.sgs" CACHE FILEPATH
	"Path to cppbc.sgs (SGS/CPP-BC binding code generator)" )

if( MSVC )
	set( CPPBC_INC_PFX /I )
	set( CPPBC_PP_ARGS /P /DSGS_CPPBC_PP=1 /Fi )
else()
	set( CPPBC_INC_PFX -I )
	set( CPPBC_PP_ARGS -E -DSGS_CPPBC_PP=1 -o )
endif()

macro( cppbc_header TARGET CPPFILE HFILE INCNAME )
	get_target_property( LIST ${TARGET} INCLUDE_DIRECTORIES )
	if( NOT LIST )
		set( LIST "" )
	endif()
	separate_arguments( LIST )
	set( CFLAGS "" )
	foreach( F ${LIST} )
		list( APPEND CFLAGS "${CPPBC_INC_PFX}${F}" )
	endforeach( F )
	add_custom_command(
		OUTPUT "${CMAKE_SOURCE_DIR}/${CPPFILE}"
		DEPENDS "${CMAKE_SOURCE_DIR}/${HFILE}"
		COMMAND ${SGSVM_PATH} ARGS -p ${SGSCPPBC_PATH} -cmake0 cppbc_tmp.cpp "${CMAKE_SOURCE_DIR}/${HFILE}"
		COMMAND ${CMAKE_CXX_COMPILER} ARGS ${CPPBC_PP_ARGS} cppbc_tmp.i cppbc_tmp.cpp ${CFLAGS}
		COMMAND ${SGSVM_PATH} ARGS -p ${SGSCPPBC_PATH} -o "${CMAKE_SOURCE_DIR}/${CPPFILE}" -iname ${INCNAME} cppbc_tmp.i
	)
endmacro( cppbc_header )

macro( cppbc_headers TARGET )
	foreach( FILE ${ARGN} )
		get_filename_component( FILE ${FILE} ABSOLUTE )
		file( RELATIVE_PATH FILE ${CMAKE_SOURCE_DIR} ${FILE} )
		get_filename_component( FDIR ${FILE} PATH )
		get_filename_component( FNAME ${FILE} NAME_WE )
		get_filename_component( FXNAME ${FILE} NAME )
		set( OUTFILE "${FDIR}/cppbc_${FNAME}.cpp" )
		cppbc_header( ${TARGET} ${OUTFILE} ${FILE} ${FXNAME} )
	endforeach( FILE )
endmacro( cppbc_headers )
