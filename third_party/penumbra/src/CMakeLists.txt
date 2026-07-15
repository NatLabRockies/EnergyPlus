# Main library
file(GLOB_RECURSE sources "${PROJECT_SOURCE_DIR}/src/[a-zA-Z]*.cpp")
file(GLOB_RECURSE public_headers "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/[a-zA-Z]*.h")
file(GLOB_RECURSE private_headers "${PROJECT_SOURCE_DIR}/src/[a-zA-Z]*.h")

set(library_sources
  ${sources}
  ${public_headers}
  ${private_headers}
)

option( ${PROJECT_NAME}_STATIC_LIB "Make ${PROJECT_NAME} a static library" ON )

if (${PROJECT_NAME}_STATIC_LIB)
  add_library(${PROJECT_NAME} STATIC ${library_sources})
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D${PROJECT_NAME}_STATIC_DEFINE")
else()
  set(CMAKE_MACOSX_RPATH 1)
  add_library(${PROJECT_NAME} SHARED ${library_sources})
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  PRIVATE
  ${PROJECT_SOURCE_DIR}/src
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw tess2 penumbra_common_interface PUBLIC courierr)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# If MSVC_RUNTIME_LIBRARY is not by a parent project use the default.
# It's not clear why this is needed, since documentation indicates it
#   should be happening with the CMP0091 policy set to NEW.
get_target_property(RTL ${PROJECT_NAME} MSVC_RUNTIME_LIBRARY)
if ("${RTL}" STREQUAL "RTL-NOTFOUND")
  set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if (${PROJECT_NAME}_COVERAGE)
  add_coverage(${PROJECT_NAME})
endif()
