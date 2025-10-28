include(initialize-submodules)
initialize_submodules()

# GLAD
add_subdirectory(glad)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/vendor/glad/include)

# GLFW library
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if (APPLE)
  set(GLFW_USE_MENUBAR OFF CACHE BOOL "" FORCE)
endif()

mark_as_advanced(
  GLFW_BUILD_DOCS
  GLFW_BUILD_TESTS
  GLFW_BUILD_EXAMPLES
  GLFW_INSTALL
  GLFW_USE_MENUBAR
  GLFW_USE_OSMESA
  GLFW_VULKAN_STATIC
  BUILD_SHARED_LIBS
)

add_subdirectory(glfw)
target_include_directories(glfw PUBLIC
 ${PROJECT_SOURCE_DIR}/vendor/glfw/include
 ${PROJECT_SOURCE_DIR}/vendor/glfw/deps
)
if(NOT MSVC)
  target_compile_options(glfw PRIVATE -Wno-deprecated-declarations)
endif()

# tess2 library
file(GLOB libtess2_sources "${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/[a-zA-Z]*.c")

add_library(tess2 ${libtess2_sources})

target_include_directories(tess2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Include)
target_include_directories(tess2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source)

if (NOT TARGET courierr)
  add_subdirectory(courierr)
  mark_as_advanced(courierr_BUILD_TESTING courierr_COVERAGE courierr_STATIC_LIB)
endif ()

# googletest library
if (${PROJECT_NAME}_BUILD_TESTING AND NOT TARGET gtest)

  # Prevent GoogleTest from overriding our compiler/linker options
  # when building with Visual Studio
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(BUILD_GTEST ON CACHE BOOL "" FORCE MARK)
  set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BUILD_GTEST BUILD_GMOCK INSTALL_GTEST)

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest)

endif()
