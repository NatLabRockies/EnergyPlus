SET(SRC
defines.h
utilSocket.c
utilSocket.h
utilXml.c
utilXml.h
)

IF( WIN32 )
  LIST( APPEND SRC bcvtb.def )
ENDIF()

INCLUDE_DIRECTORIES( "${PROJECT_SOURCE_DIR}/third_party/Expat/lib")

IF (UNIX)
  ADD_DEFINITIONS("-fPIC")
ENDIF()

ADD_LIBRARY( bcvtb STATIC ${SRC} )

IF( WIN32 )
  TARGET_LINK_LIBRARIES( bcvtb wsock32 )
ENDIF()

set_target_properties(
  bcvtb
  PROPERTIES
  INSTALL_NAME_DIR "@executable_path"
)


