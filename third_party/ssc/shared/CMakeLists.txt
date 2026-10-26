#####################################################################################################################
#
# Project Settings
#
#####################################################################################################################
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(.)

set(SHARED_SRC
#        6par_jacobian.h
#        6par_lu.h
#        6par_newton.h
#        6par_search.h
#        CMakeLists.txt
        lib_battery.cpp
        lib_battery.h
        lib_battery_capacity.cpp
        lib_battery_capacity.h
		lib_battery_dispatch.cpp
		lib_battery_dispatch.h
        lib_battery_lifetime.cpp
        lib_battery_lifetime.h
        lib_battery_lifetime_calendar_cycle.cpp
        lib_battery_lifetime_calendar_cycle.h
        lib_battery_lifetime_nmc.cpp
        lib_battery_lifetime_nmc.h
        lib_battery_powerflow.cpp
        lib_battery_powerflow.h
        lib_battery_voltage.cpp
        lib_battery_voltage.h
        lib_irradproc.cpp
        lib_irradproc.h
#        lib_power_electronics.h
        lib_pv_incidence_modifier.cpp
        lib_pv_incidence_modifier.h
        lib_pvinv.cpp
        lib_pvinv.h
#        lib_pvmodel.h
        lib_pvshade.cpp
        lib_pvshade.h
        lib_pvwatts.cpp
        lib_pvwatts.h
        lib_resilience.cpp
        lib_resilience.h
        lib_sandia.cpp
        lib_sandia.h
        lib_shared_inverter.cpp
        lib_shared_inverter.h
        lib_time.cpp
        lib_time.h
        lib_util.cpp
        lib_util.h
        lib_utility_rate.cpp
        lib_utility_rate.h
		lib_utility_rate_equations.cpp
        lib_utility_rate_equations.h
        lib_weatherfile.cpp
        lib_weatherfile.h
        )


#####################################################################################################################
#
# CMake Targets
#
#####################################################################################################################

add_library(shared STATIC ${SHARED_SRC})
set_target_properties(shared
        PROPERTIES
        DEBUG_POSTFIX "d"
        PREFIX ""
        )

set_default_compile_options(shared)
set_disabled_warnings(shared)

#####################################################################################################################
#
# Link Libraries and Options
#
#####################################################################################################################

if (UNIX)
    set(CMAKE_SHARED_LINKER_FLAGS "-lm")
endif ()
if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif ()
