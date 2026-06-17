[![Build and Test](https://github.com/bigladder/penumbra/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/bigladder/penumbra/actions/workflows/build-and-test.yml)
[![codecov](https://codecov.io/gh/bigladder/penumbra/branch/develop/graph/badge.svg)](https://codecov.io/gh/bigladder/penumbra)

# Penumbra

Penumbra is a free and open source library for GPU accelerated solar shading calculations using [pixel counting](http://www.ibpsa.org/proceedings/BS2011/P_1271.pdf). It is configured as a cross-platform CMake project. 

## Pre-requisites:

1. A C++ compiler (e.g., Clang, GCC, MSVC)
2. CMake

## Building penumbra from source

1. Clone the git repository.
2. Make a directory called `build` inside the top level of your source.
3. Open a console in the `build` directory.
4. Type `cmake ..`.
5. Type `cmake --build . --config Release`.
