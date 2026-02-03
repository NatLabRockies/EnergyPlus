project(BOOST)

if (KIVA_EXE_BUILD)
    add_subdirectory(libs/program_options)
    target_compile_features(boost_program_options PRIVATE cxx_std_17)
endif ()