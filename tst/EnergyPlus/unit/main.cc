// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// EnergyPlus Unit Test Driver

// Google Test Headers
#include <gtest/gtest.h>

#ifdef DEBUG_ARITHM_GCC_OR_CLANG
#    include <EnergyPlus/fenv_missing.h>
#endif

#ifdef DEBUG_ARITHM_MSVC
#    include <cfloat>
#endif

// Google Test main
int main(int argc, char **argv)
{
#ifdef ENABLE_GTEST_DEBUG_MODE
    ::testing::GTEST_FLAG(break_on_failure) = true;
    ::testing::GTEST_FLAG(catch_exceptions) = false;
#endif
#ifdef ENABLE_GTEST_SHUFFLE
    ::testing::GTEST_FLAG(shuffle) = true;
#endif
    ::testing::InitGoogleTest(&argc, argv);
#ifdef DEBUG_ARITHM_GCC_OR_CLANG
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

#ifdef DEBUG_ARITHM_MSVC
    // Note: what you need to pass to the _controlfp_s is actually the opposite
    // By default all bits are 1, and the exceptions are turned off, so you need to turn off the bits for the exceptions you want to enable
    // > For the _MCW_EM mask, clearing it sets the exception, which allows the hardware exception; setting it hides the exception.
    unsigned int fpcntrl = 0;
    _controlfp_s(&fpcntrl, 0, 0);
    unsigned int new_exceptions = _EM_ZERODIVIDE | _EM_INVALID | _EM_OVERFLOW;
    unsigned int new_control = fpcntrl & ~new_exceptions;
    _controlfp_s(&fpcntrl, new_control, _MCW_EM);
#endif

    return RUN_ALL_TESTS();
}
