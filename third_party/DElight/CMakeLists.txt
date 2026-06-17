CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12...4.0.3 )  # bumped max support to hush a cmake deprecation warning

PROJECT(DElight)

LIST( APPEND CMAKE_CXX_SOURCE_FILE_EXTENSIONS "CPP" )

INCLUDE_DIRECTORIES( "${CMAKE_CURRENT_SOURCE_DIR}" )

TRY_COMPILE( HAS_ISNAN "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/has_isnan.c" )

IF (UNIX)
  ADD_DEFINITIONS("-fPIC")
ENDIF()

IF( HAS_ISNAN )
  ADD_DEFINITIONS(-DHAS_ISNAN)
ENDIF()

SET( delight_src 
  btdf.cpp
  dlmath.h
  CFSSurface.cpp
  CFSSystem.cpp
  coordsys3.cpp
  DElight2.cpp
  DElightManagerC.cpp
  DFcalcs.cpp
  DOE2dl.cpp
  ECM.cpp
  EPlus_DElight.cpp
  EPlus_ECM.cpp
  EPlus_Geom.cpp
  EPlus_Loaddata.cpp
  Geom.cpp
  GeomMesh.cpp
  helpers.cpp
  hemisphiral.cpp
  line2.cpp
  line3.cpp
  Loaddata.cpp
  matrix3.cpp
  NodeMesh2.cpp
  plane3.cpp
  point2.cpp
  point3.cpp
  poly2.cpp
  Radiosity.cpp
  Savedata.cpp
  SOL.cpp
  Struct.cpp
  TOOLS.cpp
  vector2.cpp
  vector3.cpp
  W4Lib.cpp
  WLCSurface.cpp
  WxTMY2.cpp
  BGL.h
  btdf.h
  CFSSurface.h
  CFSSystem.h
  CONST.H
  coordsys3.h
  DBCONST.H
  DEF.H
  DElight2.h
  DElightAPI.h
  DElightManagerC.h
  DFcalcs.h
  DOE2DL.H
  ECM.H
  EPlus_ECM.H
  EPlus_Geom.h
  EPlus_Loaddata.h
  geom.h
  GeomMesh.h
  helpers.h
  hemisphiral.h
  line2.h
  line3.h
  Loaddata.h
  matrix3.h
  NodeMesh2.h
  plane3.h
  point2.h
  point3.h
  poly2.h
  Radiosity.h
  rand.h
  savedata.h
  SOL.H
  struct.h
  TOOLS.H
  vector2.h
  vector3.h
  W4Lib.h
  WLCSurface.h
  WxTMY2.h
)

ADD_LIBRARY( DElight STATIC ${delight_src} )

