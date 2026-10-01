file(GLOB lib_headers "${PROJECT_SOURCE_DIR}/include/libtk205/*.h")
file(GLOB lib_src "${PROJECT_SOURCE_DIR}/src/*.cpp")

set (sources "${lib_headers}"
             "${lib_src}")

option(${PROJECT_NAME}_STATIC_LIB "Make ${PROJECT_NAME} a static library" ON)

if (${PROJECT_NAME}_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${sources})
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D${PROJECT_NAME}_STATIC_DEFINE")
else ()
    set(CMAKE_MACOSX_RPATH 1)
    add_library(${PROJECT_NAME} SHARED ${sources})
endif ()

target_link_libraries(libtk205 PUBLIC courierr nlohmann_json btwxt)
target_include_directories(libtk205 PUBLIC ${PROJECT_SOURCE_DIR}/include/libtk205)
target_include_directories(libtk205 PRIVATE ${PROJECT_SOURCE_DIR}/vendor/valijson/include)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Wpedantic>
        )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

if (${PROJECT_NAME}_COVERAGE)
    add_coverage(${PROJECT_NAME})
endif ()
