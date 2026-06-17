include_directories("${PROJECT_SOURCE_DIR}/third_party/Expat/lib")
include_directories("${PROJECT_SOURCE_DIR}/third_party/")
include_directories(${PROJECT_SOURCE_DIR}/third_party/zlib)

set(SRC
  main.h
  main.c
  fmiFunctions.h
  fmiModelTypes.h
  fmiPlatformTypes.h
  fmumini.c
  fmumini.h
  stack.c
  stack.h
  util.c
  util.h
  xml_parser_cosim.c
  xml_parser_cosim.h
  eplusModelFunctions.h
  fmiModelFunctions.h
  fmiModelTypes.h
)

add_library(epfmiimport STATIC ${SRC})

if(UNIX)
  set_property(TARGET epfmiimport PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(epfmiimport PRIVATE epexpat miniziplib)

if(UNIX)
  set_target_properties(epfmiimport PROPERTIES INSTALL_NAME_DIR "@executable_path")
endif()
