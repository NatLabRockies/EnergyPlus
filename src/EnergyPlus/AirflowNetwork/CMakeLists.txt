cmake_minimum_required(VERSION 3.17)

project(AirflowNetwork)

function(print_target_properties target)
    if(NOT TARGET ${target})
      message(STATUS "There is no target named '${target}'")
      return()
    endif()

    foreach(property ${CMAKE_PROPERTY_LIST})
        message("${target} ${property}")
        string(REPLACE "<CONFIG>" "DEBUG" property ${property})

        get_property(was_set TARGET ${target} PROPERTY ${property} SET)
        if(was_set)
            get_target_property(value ${target} ${property})
            message("${target} ${property} = ${value}")
        endif()
    endforeach()
endfunction()

if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER "3.0")
  cmake_policy(SET CMP0054 NEW) # CMake 3.1 added this policy
endif()

set(HDRS include/AirflowNetwork/Solver.hpp include/AirflowNetwork/Elements.hpp include/AirflowNetwork/Properties.hpp)

set(SRCS src/Solver.cpp src/Elements.cpp src/Properties.cpp)

# Create a static library
# this will be linked statically to create the DLL and also the unit tests
add_library(airflownetworklib STATIC ${SRCS} ${HDRS})

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") # g++/Clang
  target_compile_options(airflownetworklib PRIVATE -Wno-attributes) # Don't warn on attributes that g++ and Clang don't know
endif()

message(STATUS "XXXXXXXXXXXXX")
#message(STATUS "${CMAKE_BINARY_DIR}")
#message(STATUS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
#message(STATUS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
message(STATUS "XXXXXXXXXXXXX")
#target_include_directories(airflownetworklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(airflownetworklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(airflownetworklib PUBLIC fmt::fmt)
message(STATUS "XXXXXXXXXXXXX")
#message(STATUS "${CMAKE_BINARY_DIR}")
#message(STATUS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
#message(STATUS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
get_target_property(dirs airflownetworklib INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

print_target_properties(airflownetworklib)
message(STATUS "XXXXXXXXXXXXX")
