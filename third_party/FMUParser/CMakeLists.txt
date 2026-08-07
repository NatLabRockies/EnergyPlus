
include_directories("${PROJECT_SOURCE_DIR}/third_party/Expat/lib")
include_directories("${PROJECT_SOURCE_DIR}/third_party/")
include_directories("${PROJECT_SOURCE_DIR}/third_party/zlib")

set(SRC
  parser.h
  parser.c
  fmumini.c
  fmumini.h
  stack.c
  stack.h
  util.c
  util.h
  xml_parser_cosim.c
  xml_parser_cosim.h
  fmiFunctions.h
  fmiModelTypes.h
)

#add_library(parser SHARED ${SRC})

if(UNIX)
  add_definitions("-fPIC")
endif()

add_executable(parser ${SRC})

target_link_libraries(parser PRIVATE epexpat miniziplib)

install(TARGETS parser
  RUNTIME DESTINATION PreProcess/FMUParser
  LIBRARY DESTINATION ./
  ARCHIVE DESTINATION ./
)

if(UNIX)
  set_target_properties(
    parser
    PROPERTIES
    INSTALL_NAME_DIR "@executable_path"
  )
endif()

if(BUILD_TESTING)
  # Avoid creating some artifacts (ShadingController/modelDescription.xml and ShadingController/binaries inside "${PROJECT_SOURCE_DIR}/datasets/FMUs/"
  configure_file("${PROJECT_SOURCE_DIR}/datasets/FMUs/ShadingController.fmu" "${CMAKE_CURRENT_BINARY_DIR}/ShadingController.fmu" COPYONLY)
  add_test(NAME "FMUParser"
    COMMAND parser "--printidf" "${CMAKE_CURRENT_BINARY_DIR}/ShadingController.fmu"
  )
endif()
