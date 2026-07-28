configure_file("Energy+.idd.in" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.idd")


add_custom_command(
  OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.schema.epJSON"
  COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/schema/generate_epJSON_schema.py" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
  MAIN_DEPENDENCY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.idd"
  DEPENDS
    schema/generate_epJSON_schema.py
    schema/idd_parser.py
    schema/modify_schema.py
  VERBATIM
)

add_executable(generate_embeddable_epJSON_schema embedded/generate_embeddable_epJSON_schema.cpp)
target_link_libraries(generate_embeddable_epJSON_schema PRIVATE nlohmann_json project_options project_fp_options project_warnings)
target_compile_options(generate_embeddable_epJSON_schema PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-shorten-64-to-32>)
set_target_properties(generate_embeddable_epJSON_schema PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/scripts")
# See https://en.cppreference.com/w/cpp/filesystem#Notes
if ((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1) OR
    ( ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)))
  target_link_libraries(generate_embeddable_epJSON_schema PRIVATE stdc++fs)
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/embedded/EmbeddedEpJSONSchema.cc"
  COMMAND
    generate_embeddable_epJSON_schema "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.schema.epJSON" "${CMAKE_CURRENT_BINARY_DIR}/embedded/EmbeddedEpJSONSchema.cc"
  DEPENDS
    generate_embeddable_epJSON_schema
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Energy+.schema.epJSON" # File-level dependency
  VERBATIM)

add_library(embedded_epjson_source STATIC
  embedded/EmbeddedEpJSONSchema.hh
  "${CMAKE_CURRENT_BINARY_DIR}/embedded/EmbeddedEpJSONSchema.cc"
)
target_include_directories(embedded_epjson_source PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(embedded_epjson_source PRIVATE project_options project_fp_options project_warnings)

#install(FILES "versions/V7-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-0-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-1-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-3-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-4-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-5-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-6-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-7-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-8-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
#install(FILES "versions/V8-9-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-0-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-1-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-3-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-4-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-5-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V9-6-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V22-1-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V22-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V23-1-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V23-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V24-1-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V24-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V25-1-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")
install(FILES "versions/V25-2-0-Energy+.idd" DESTINATION "PreProcess/IDFVersionUpdater")

# For fortran...
set(PREVIOUS_IDD "${CMAKE_CURRENT_SOURCE_DIR}/versions/V${PREV_RELEASE_MAJOR}-${PREV_RELEASE_MINOR}-${PREV_RELEASE_PATCH}-Energy+.idd")
set(PREVIOUS_IDD ${PREVIOUS_IDD} PARENT_SCOPE)

install(FILES "${PREVIOUS_IDD}" DESTINATION "PreProcess/IDFVersionUpdater")

if (BUILD_TESTING)

  if (Pytest_AVAILABLE)
    add_test(NAME idd_schema.test_idd_parser
      COMMAND ${Python_EXECUTABLE} -m pytest --verbose "${CMAKE_CURRENT_SOURCE_DIR}/schema/test_idd_parser.py"
    )
    set_tests_properties(idd_schema.test_idd_parser PROPERTIES TIMEOUT 30)
  endif()

endif()
