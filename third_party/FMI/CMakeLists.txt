include_directories("${PROJECT_SOURCE_DIR}/third_party/")
include_directories(${PROJECT_SOURCE_DIR}/third_party/zlib)

set(SRC
  FMI/main.h
  FMI/main.c
  FMI/fmiFunctions.h
  FMI/fmiModelTypes.h
  FMI/fmiPlatformTypes.h
  FMI/fmumini.c
  FMI/fmumini.h
  FMI/stack.c
  FMI/stack.h
  FMI/util.c
  FMI/util.h
  FMI/xml_parser_cosim.c
  FMI/xml_parser_cosim.h
  FMI/eplusModelFunctions.h
  FMI/fmiModelFunctions.h
  FMI/fmiModelTypes.h
)

add_library(epfmiimport STATIC ${SRC})
target_include_directories(epfmiimport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  set_property(TARGET epfmiimport PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(epfmiimport PRIVATE epexpat miniziplib)

if(UNIX)
  set_target_properties(epfmiimport PROPERTIES INSTALL_NAME_DIR "@executable_path")
endif()
