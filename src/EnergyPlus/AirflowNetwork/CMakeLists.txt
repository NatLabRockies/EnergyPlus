cmake_minimum_required(VERSION 3.17...4.0.3)

project(AirflowNetwork)

set(HDRS include/AirflowNetwork/Solver.hpp include/AirflowNetwork/Elements.hpp include/AirflowNetwork/Properties.hpp)

set(SRCS src/Solver.cpp src/Elements.cpp src/Properties.cpp)

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
