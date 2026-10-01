set(SRC
  BCVTB/defines.h
  BCVTB/utilSocket.c
  BCVTB/utilSocket.h
  BCVTB/utilXml.c
  BCVTB/utilXml.h
)

if(WIN32)
  list(APPEND SRC BCVTB/bcvtb.def)
endif()

add_library( bcvtb STATIC ${SRC} )
target_include_directories(bcvtb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bcvtb PRIVATE epexpat)
set_property(TARGET bcvtb PROPERTY POSITION_INDEPENDENT_CODE ON) # Enable fPIC where supported
target_compile_options(bcvtb PRIVATE $<$<COMPILE_LANG_AND_ID:C,AppleClang>:-Wno-shorten-64-to-32>)

if(WIN32)
  target_link_libraries(bcvtb PRIVATE wsock32)
endif()

set_target_properties(
  bcvtb
  PROPERTIES
  INSTALL_NAME_DIR "@executable_path"
)

