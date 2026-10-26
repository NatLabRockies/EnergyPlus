#####################################################################################################################
#
# Project Settings
#
#####################################################################################################################

include_directories(. ../shared ../ssc)

set(SSC_SRC
		cmod_battery.h
#		cmod_csp_tower_eqns.h
#		cmod_financial_eqns.h
#		cmod_merchantplant_eqns.h
#		cmod_mhk_eqns.h
#		cmod_pvsamv1_eqns.h
		cmod_pvwattsv5.cpp
#		cmod_utilityrate5.h
#		cmod_utilityrate5_eqns.h
#		cmod_windpower_eqns.h
		common.cpp
		common.h
		common_financial.cpp
		common_financial.h
		core.cpp
		core.h
		ssc_equations.h
        sscapi.cpp
		sscapi.h
		vartab.cpp
		vartab.h
		)

if(MSVC)
	set	(SSC_SRC
		${SSC_SRC}
		vbapi.cpp
		vbapi.h
		)
endif(MSVC)


#####################################################################################################################
#
# CMake Targets
#
#####################################################################################################################

add_library(ssc STATIC ${SSC_SRC})
set_target_properties(ssc
    PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX "d"
	)

set_disabled_warnings(ssc)
set_default_compile_options(ssc)

if(MSVC)
    set_additional_compile_options(ssc "-D__DLL__ -D__VISUALC__")
endif()

# libssc for PySAM
if(UNIX)
	if (SAMAPI_EXPORT)
		set(CMAKE_MACOSX_RPATH 1)
		set_property(TARGET ssc PROPERTY PREFIX "lib")
		set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
		if (APPLE)
			add_custom_command(
				TARGET ssc
				POST_BUILD
				COMMAND install_name_tool -id @rpath/libssc$<$<CONFIG:Debug>:d>.so $<TARGET_FILE:ssc>
			)
		endif()
	endif()
endif()


#####################################################################################################################
#
# Link Libraries and Options
#
#####################################################################################################################

set( DEPENDENCIES
	shared
	)
foreach( name ${DEPENDENCIES} )
	target_link_libraries(ssc PUBLIC ${name})
endforeach()

if (UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(ssc PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
  find_library(MATH_LIBRARY m)
  if(MATH_LIBRARY)
    target_link_libraries(ssc PUBLIC ${MATH_LIBRARY})
  endif()
endif()


#####################################################################################################################
#
# Target Installation
#
#####################################################################################################################

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	set(SSC_LIB $<TARGET_FILE_DIR:ssc>/$<TARGET_FILE_BASE_NAME:ssc>.lib)
	set(SSC_H ${CMAKE_CURRENT_LIST_DIR}/sscapi.h)

	set(TESTDIR ${CMAKE_CURRENT_BINARY_DIR}/../test/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)

	if(DEFINED ENV{SAMNTDIR})
		add_custom_command(
			TARGET ssc
			POST_BUILD
			COMMAND ${CMAKE_COMMAND}
			ARGS -E make_directory $ENV{SAMNTDIR}/deploy/x64
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} ${SSC_H} $ENV{SAMNTDIR}/deploy/x64
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} ${TESTDIR}
			)
	endif()

	set (SDKTOOLDIR ${CMAKE_CURRENT_BINARY_DIR}/../sdktool/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
	if (EXISTS ${SDKTOOLDIR})
		add_custom_command(
			TARGET ssc
			POST_BUILD
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} ${SDKTOOLDIR}
			)
	endif()

	set (TCSCONSOLDEDIR ${CMAKE_CURRENT_BINARY_DIR}/../tcsconsole/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
	if (EXISTS ${TCSCONSOLDEDIR})
		add_custom_command(
			TARGET ssc
			POST_BUILD
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} ${TCSCONSOLDEDIR}>
			)
	endif()

	if (SAMAPI_EXPORT)
		if (${CMAKE_PROJECT_NAME} STREQUAL system_advisor_model)
			set(APIGENDIR ${CMAKE_CURRENT_BINARY_DIR}/../../sam/api/api_autogen/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
			add_custom_command(
				TARGET ssc
				POST_BUILD
				COMMAND ${CMAKE_COMMAND}
				ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} ${APIGENDIR}
				)
		else()
			set(APIGENDIR $ENV{SAMNTDIR}/build/api/api_autogen/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
            if (EXISTS ${APIGENDIR})
			    add_custom_command(
				    TARGET ssc
				    POST_BUILD
				    COMMAND ${CMAKE_COMMAND}
				    ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} ${APIGENDIR}
				    )
               endif()
		endif()
	endif()

	add_custom_target(build-time-make-directory ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTDIR} ${APIGENDIR})
	add_dependencies(ssc build-time-make-directory)

endif()

if(DEFINED ENV{PYSAMDIR})
	if (SAMAPI_EXPORT OR MSVC)
		if (MSVC)
			add_custom_command(
					TARGET ssc
					POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E remove
					$ENV{PYSAMDIR}/files/$<TARGET_FILE_BASE_NAME:ssc>.lib
					$ENV{PYSAMDIR}/files/$<TARGET_FILE_BASE_NAME:ssc>.dll
			)
			set(SSC_LIB $<TARGET_FILE_DIR:ssc>/$<TARGET_FILE_BASE_NAME:ssc>.lib)
		endif()
		add_custom_command(
				TARGET ssc
				POST_BUILD
				COMMAND ${CMAKE_COMMAND}
				ARGS -E copy $<TARGET_FILE:ssc> ${SSC_LIB} $ENV{PYSAMDIR}/files
		)
	endif()
endif()

