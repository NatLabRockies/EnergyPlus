# Main library

set(library_sources
        regular-grid-interpolator-implementation.h
        regular-grid-interpolator-implementation.cpp
        regular-grid-interpolator.cpp
        grid-axis.cpp
        )

# CMake versions before 4.0 place MSVC's /Od, /Ob0, and /RTC1 defaults directly in
# CMAKE_CXX_FLAGS_DEBUG. Remove them in this directory only so Btwxt can use /O1 in Debug builds.
if (MSVC)
    string(REGEX REPLACE "([/-]Ob0|[/-]Od|[/-]RTC[1csu]*)" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif ()

option(${PROJECT_NAME}_STATIC_LIB "Make ${PROJECT_NAME} a static library" ON)

if (${PROJECT_NAME}_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${library_sources})
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D${PROJECT_NAME}_STATIC_DEFINE")
else ()
    set(CMAKE_MACOSX_RPATH 1)
    add_library(${PROJECT_NAME} SHARED ${library_sources})
endif ()

if (MSVC)
    # CMake 4.0+ models /RTC with this property. The custom property also prevents EnergyPlus'
    # project_options interface from adding /Ob0 and /RTCsu back to this target.
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_CHECKS ""
            ENERGYPLUS_OPTIMIZED_DEBUG ON
            )
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_interface courierr)

target_compile_options(btwxt PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Wpedantic>
        )

# High-dimensional interpolation is prohibitively slow without optimization. Keep a
# debugger-friendly optimization level for Btwxt in the Debug configuration.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:/O1>
            $<$<CONFIG:Debug>:/Zo>
            )
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>>:-O1>
            )
endif ()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

if (${PROJECT_NAME}_COVERAGE)
    add_coverage(${PROJECT_NAME})
endif ()
