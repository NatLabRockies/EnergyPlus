# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.16)

project(boost_program_options LANGUAGES CXX)

add_library(boost_program_options
  src/cmdline.cpp
  src/config_file.cpp
  src/convert.cpp
  src/options_description.cpp
  src/parsers.cpp
  src/positional_options.cpp
  src/split.cpp
  src/utf8_codecvt_facet.cpp
  src/value_semantic.cpp
  src/variables_map.cpp
  src/winmain.cpp
)

add_library(Boost::program_options ALIAS boost_program_options)

target_include_directories(boost_program_options PUBLIC ${BOOST_SOURCE_DIR})

target_compile_definitions(boost_program_options
  PUBLIC BOOST_PROGRAM_OPTIONS_NO_LIB
  # Source files already define BOOST_PROGRAM_OPTIONS_SOURCE
  # PRIVATE BOOST_PROGRAM_OPTIONS_SOURCE
)

target_compile_definitions(boost_program_options PUBLIC BOOST_PROGRAM_OPTIONS_STATIC_LINK)