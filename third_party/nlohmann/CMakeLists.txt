#### Create interface for JSON for Modern C++ so we can include it in targets

add_library( nlohmann_json INTERFACE )
target_compile_definitions(
        nlohmann_json
        INTERFACE
        JSON_USE_IMPLICIT_CONVERSIONS=1
)
if(NOT MSVC)  # This is really only needed with newer Clang, but gcc is fine with it
  target_compile_options(nlohmann_json INTERFACE -Wno-deprecated-declarations)
endif()
target_compile_options(nlohmann_json INTERFACE $<$<COMPILE_LANG_AND_ID:CXX,AppleClang>:-Wno-shorten-64-to-32>)
target_include_directories(
        nlohmann_json
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
set_source_files_properties(
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/nlohmann/json.hpp
  TARGET_DIRECTORY nlohmann_json
  PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations
)
