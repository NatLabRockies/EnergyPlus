include(initialize-submodules)
initialize_submodules()

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

endif ()

if (NOT TARGET courierr)
    add_subdirectory(courierr)
    mark_as_advanced(courierr_BUILD_TESTING courierr_COVERAGE courierr_STATIC_LIB)
endif ()
