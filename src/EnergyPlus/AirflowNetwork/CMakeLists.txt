cmake_minimum_required(VERSION 3.17)

project(AirflowNetwork)

if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER "3.0")
  cmake_policy(SET CMP0054 NEW) # CMake 3.1 added this policy
endif()

set(HDRS include/AirflowNetwork/Types.hpp include/AirflowNetwork/Solver.hpp include/AirflowNetwork/Elements.hpp include/AirflowNetwork/Linkages.hpp include/AirflowNetwork/Properties.hpp include/AirflowNetwork/Nodes.hpp)

set(SRCS src/Solver.cpp src/Elements.cpp src/Linkages.cpp src/Properties.cpp src/Nodes.cpp)

# Create a static library
# this will be linked statically to create the DLL and also the unit tests
add_library(airflownetworklib STATIC ${SRCS} ${HDRS})

if(ENABLE_UNITY)
  set_target_properties(airflownetworklib PROPERTIES UNITY_BUILD ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") # g++/Clang
  target_compile_options(airflownetworklib PRIVATE -Wno-attributes) # Don't warn on attributes that g++ and Clang don't know
endif()

target_include_directories(airflownetworklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(airflownetworklib PUBLIC btwxt fmt::fmt)
