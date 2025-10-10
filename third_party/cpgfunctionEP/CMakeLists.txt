cmake_minimum_required(VERSION 3.6...4.0.3)  # bumped max support to hush a cmake deprecation warning
project(cpgfunctionEP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if (MSVC)
    add_compile_definitions(_USE_MATH_DEFINES)
endif ()

# Everything will need to access this
include_directories(include)

# Create a DLL with an API
add_library(cpgfunctionEP
        src/blas.cpp
        src/boreholes.cpp
        src/coordinates.cpp
        src/gfunction.cpp
        src/heat_transfer.cpp
        src/interpolation.cpp
        src/lu.cpp
        src/segments.cpp
        src/statistics.cpp
        src/utilities.cpp
        )

# It depends on threads
find_package(Threads REQUIRED)
target_link_libraries(cpgfunctionEP INTERFACE Threads::Threads)

option(USE_OpenMP "Use OpenMP" ON)
if (USE_OpenMP)
    find_package(OpenMP)
    if (OpenMP_CXX_FOUND)
        target_link_libraries(cpgfunctionEP PRIVATE OpenMP::OpenMP_CXX)
    endif ()
endif ()

# if we are only building cpgfunctionEP
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # local eigen if not in EnergyPlus third_party/
    include_directories(third_party/eigen-3.3.9)  # Eigen for LU decomposition
    include_directories(third_party/nlohmann-3.9.1)
    # Turn on testing
    enable_testing()

    # Create a test binary that depends on the DLL
    add_executable(gFunction_minimal test/gFunction_minimal.cpp)
    add_executable(interpolation test/interpolation.cpp)
    # add_executable(test3 test/configurations.cpp)
    add_executable(borefield_definition test/borefield_definition.cpp)
    add_executable(time_definition test/time_definition.cpp)
    add_executable(gFunction_tests test/gFunction_tests.cpp)
    add_executable(packed test/packed.cpp)
    add_executable(gFunction_adaptive test/gFunction_adaptive.cpp)
    add_executable(gFunctionTiming test/gFunctionTimings.cpp)
    add_executable(ComputeFLS test/ComputeFLS.cpp)

    target_link_libraries(gFunction_minimal PRIVATE cpgfunctionEP)
    target_link_libraries(interpolation PRIVATE cpgfunctionEP)
    # target_link_libraries(test3 PRIVATE cpgfunctionEP)
    target_link_libraries(borefield_definition PRIVATE cpgfunctionEP)
    target_link_libraries(time_definition PRIVATE cpgfunctionEP)
    target_link_libraries(gFunction_tests PRIVATE cpgfunctionEP)
    target_link_libraries(packed PRIVATE cpgfunctionEP)
    target_link_libraries(gFunction_adaptive PRIVATE cpgfunctionEP)
    target_link_libraries(gFunctionTiming PRIVATE cpgfunctionEP)
    target_link_libraries(ComputeFLS PRIVATE cpgfunctionEP)

    # Copy validation files to build directory so tests can open
    file(GLOB JsonValidation test/validation/*.json)
    file(GLOB CustomLayout test/custom_configurations/*.json)

    file(COPY ${JsonValidation} DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CustomLayout} DESTINATION ${CMAKE_BINARY_DIR})
    message(STATUS "The binary directory: ${CMAKE_BINARY_DIR}")

    # Test the test binary itself
    add_test(NAME RunTest1 COMMAND "${CMAKE_BINARY_DIR}/gFunction_minimal")
    add_test(NAME RunTest2 COMMAND "${CMAKE_BINARY_DIR}/interpolation")
    # add_test(NAME RunTest3 COMMAND "${CMAKE_BINARY_DIR}/test3")
    add_test(NAME RunTest4 COMMAND "${CMAKE_BINARY_DIR}/borefield_definition")
    add_test(NAME RunTest5 COMMAND "${CMAKE_BINARY_DIR}/time_definition")
    # Pass variable path into test 6 for json files
    add_test(NAME RunTest6 COMMAND ${CMAKE_BINARY_DIR}/gFunction_tests)
    add_test(NAME RunTest7 COMMAND ${CMAKE_BINARY_DIR}/packed)
    add_test(NAME RunTest8 COMMAND ${CMAKE_BINARY_DIR}/gFunction_adaptive)
    add_test(NAME RunTest9 COMMAND ${CMAKE_BINARY_DIR}/gFunctionTiming)
    add_test(NAME RunTest10 COMMAND ${CMAKE_BINARY_DIR}/ComputeFLS)
endif ()
