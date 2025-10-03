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

#ifndef fenv_missing_h_INCLUDED
#define fenv_missing_h_INCLUDED

#include <cfenv>

// clang-format off
#ifndef HAVE_FEENABLEEXCEPT
#  if defined(__APPLE__) && defined(__MACH__)
#    if defined __i386__ || defined __x86_64__

    // Public domain polyfill for feenableexcept on OS X
    // http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c

    inline int fegetexcept()
    {
      static fenv_t fenv;

      return std::fegetenv(&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
    }

    inline int feenableexcept(unsigned int excepts)
    {
      static fenv_t fenv;
      unsigned int new_excepts = excepts & FE_ALL_EXCEPT;
      // previous masks
      unsigned int old_excepts;

      if (std::fegetenv(&fenv)) {
        return -1;
      }
      old_excepts = fenv.__control & FE_ALL_EXCEPT;

      // unmask
      fenv.__control &= ~new_excepts;
      fenv.__mxcsr &= ~(new_excepts << 7);

      return std::fesetenv(&fenv) ? -1 : old_excepts;
    }

    inline int fedisableexcept(unsigned int excepts)
    {
      static fenv_t fenv;
      unsigned int new_excepts = excepts & FE_ALL_EXCEPT;
      // all previous masks
      unsigned int old_excepts;

      if (std::fegetenv(&fenv)) {
        return -1;
      }
      old_excepts = fenv.__control & FE_ALL_EXCEPT;

      // mask
      fenv.__control |= new_excepts;
      fenv.__mxcsr |= new_excepts << 7;

      return std::fesetenv(&fenv) ? -1 : old_excepts;
    }

#    elif defined __arm64__

    // On arm**64**, this is different. the fenv_t has two members that are unsigned long long: __fpcr and __fpsr (for status)
    // Meaning there is a bit shift of 8 between the FE_XXX macros and the fpcr
    // Also, you **SET** the bits, not mask them (the opposite of intel)
    //
    // There are also enums that contain the bit shifting, and we can assert this:
    // assert((FE_INEXACT << 8) == __fpcr_trap_inexact);
    // assert((FE_UNDERFLOW << 8) == __fpcr_trap_underflow);
    // assert((FE_OVERFLOW << 8) == __fpcr_trap_overflow);
    // assert((FE_DIVBYZERO << 8) == __fpcr_trap_divbyzero);
    // assert((FE_INVALID << 8) == __fpcr_trap_invalid);
    // assert((FE_FLUSHTOZERO << 8) != __fpcr_flush_to_zero);
    // assert((FE_FLUSHTOZERO << 8) == __fpcr_trap_denormal);
    static constexpr unsigned int FPCR_EXCEPT_SHIFT = 8;

    // These will be the exception flags we use for exception values normalized
    // from both status word and control word.
    // We add EX_ prefix to the names since macOS <math.h> defines OVERFLOW and
    // UNDERFLOW macros.
    static constexpr uint32_t EX_INVALID = 0x1;
    static constexpr uint32_t EX_DIVBYZERO = 0x2;
    static constexpr uint32_t EX_OVERFLOW = 0x4;
    static constexpr uint32_t EX_UNDERFLOW = 0x8;
    static constexpr uint32_t EX_INEXACT = 0x10;
    // __APPLE__ ARM64 has an extra flag that is raised when a denormal is flushed
    // to zero.
    static constexpr uint32_t EX_FLUSHTOZERO = 0x20;

    inline int fegetexcept()
    {
      static fenv_t fenv;
      if (std::fegetenv(&fenv) != 0) {
        return -1;
      }
      const unsigned long long old_fpcr = fenv.__fpcr;
      const unsigned int old_excepts = (old_fpcr >> FPCR_EXCEPT_SHIFT) & unsigned(FE_ALL_EXCEPT);
      return old_excepts;
    }

    inline int feenableexcept(unsigned int excepts)
    {
      static fenv_t fenv;
      if (std::fegetenv(&fenv) != 0) {
        return -1;
      }
      const unsigned long long old_fpcr = fenv.__fpcr;
      const unsigned int old_excepts = (old_fpcr >> FPCR_EXCEPT_SHIFT) & unsigned(FE_ALL_EXCEPT);

      // Check the bits passed are valid, and bit shift them
      const unsigned int new_excepts = excepts & unsigned(FE_ALL_EXCEPT);
      const unsigned long long  new_fpcr = new_excepts << FPCR_EXCEPT_SHIFT;

      // Set the new bits
      fenv.__fpcr = fenv.__fpcr | new_fpcr;

      return (std::fesetenv(&fenv) != 0) ? -1 : static_cast<int>(old_excepts);
    }

    inline int fedisableexcept(unsigned int excepts)
    {
      static fenv_t fenv;
      if (std::fegetenv(&fenv) != 0) {
        return -1;
      }

      const unsigned long long old_fpcr = fenv.__fpcr;
      const unsigned int old_excepts = ((old_fpcr >> FPCR_EXCEPT_SHIFT) &  unsigned(FE_ALL_EXCEPT));

      // Check the bits passed are valid, and bit shift them
      const unsigned int new_excepts = excepts &  unsigned(FE_ALL_EXCEPT);
      const unsigned long long  new_fpcr = new_excepts << FPCR_EXCEPT_SHIFT;

      // Clear the bits
      fenv.__fpcr &= ~new_fpcr;

      return std::fesetenv(&fenv) ? -1 : static_cast<int>(old_excepts);
    }

#    endif

#  else
  inline int feenableexcept(unsigned int excepts)
  {
  #pragma STDC FENV_ACCESS ON
    fexcept_t flags;
    /* Save current exception flags. */
    fegetexceptflag(&flags, FE_ALL_EXCEPT);

    feclearexcept(FE_ALL_EXCEPT); /* clear all fp exception conditions */
    return fesetexceptflag(&flags, excepts) != 0 ? -1 : flags; /* set new flags */
  }

  inline int fedisableexcept(unsigned int excepts)
  {
  #pragma STDC FENV_ACCESS ON
    fexcept_t flags;
    /* Save current exception flags. */
    fegetexceptflag(&flags, FE_ALL_EXCEPT);

    feclearexcept(FE_ALL_EXCEPT); /* clear all fp exception conditions */
    return fesetexceptflag(&flags, ~excepts) != 0 ? -1 : flags; /* set new flags */
  }

#  endif
#endif

// clang-format on

#endif
