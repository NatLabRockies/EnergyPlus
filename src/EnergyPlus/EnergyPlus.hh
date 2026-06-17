// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

#ifndef EnergyPlus_hh_INCLUDED
#define EnergyPlus_hh_INCLUDED

// EnergyPlus Project-Wide Header File
//
// Language: C++

// C++ Headers
#include <cassert>
#include <cstdint> // C++11
#include <stdexcept>

#include <EnergyPlus/api/TypeDefs.h>

namespace EnergyPlus {
class FatalError : public std::runtime_error
{
public:
    FatalError(std::string const &msg) : runtime_error(msg)
    {
    }
};
} // namespace EnergyPlus

// macro to guarantee array sizing in debug builds
#define EP_SIZE_CHECK(array, min_size)                                                                                                               \
    assert(min_size >= 0);                                                                                                                           \
    assert(array.size() >= (size_t)min_size)

typedef std::int32_t Int32;
typedef std::int64_t Int64;

// ObjexxFCL
#include <ObjexxFCL/Array1.fwd.hh>
#include <ObjexxFCL/Array1A.fwd.hh>
#include <ObjexxFCL/Array1D.fwd.hh>
#include <ObjexxFCL/Array1S.fwd.hh>
#include <ObjexxFCL/Array2.fwd.hh>
#include <ObjexxFCL/Array2A.fwd.hh>
#include <ObjexxFCL/Array2D.fwd.hh>
#include <ObjexxFCL/Array2S.fwd.hh>
#include <ObjexxFCL/Array3D.fwd.hh>
#include <ObjexxFCL/Array4D.fwd.hh>
#include <ObjexxFCL/MArray1.fwd.hh>
#include <ObjexxFCL/Omit.hh>
#include <ObjexxFCL/Reference.fwd.hh>
#include <ObjexxFCL/Vector2.fwd.hh>
#include <ObjexxFCL/Vector3.fwd.hh>
#include <ObjexxFCL/Vector4.fwd.hh>

using ObjexxFCL::_;

using ObjexxFCL::Array1;
using ObjexxFCL::Array1_int;
using ObjexxFCL::Array1_string;
using ObjexxFCL::Array1A;
using ObjexxFCL::Array1A_int;
using ObjexxFCL::Array1D;
using ObjexxFCL::Array1D_bool;
using ObjexxFCL::Array1D_double;
using ObjexxFCL::Array1D_int;
using ObjexxFCL::Array1D_string;
using ObjexxFCL::Array1S;
using ObjexxFCL::Array1S_bool;
using ObjexxFCL::Array1S_int;
using ObjexxFCL::Array1S_string;
using ObjexxFCL::Array2;
using ObjexxFCL::Array2_int;
using ObjexxFCL::Array2A;
using ObjexxFCL::Array2A_bool;
using ObjexxFCL::Array2A_int;
using ObjexxFCL::Array2D;
using ObjexxFCL::Array2D_bool;
using ObjexxFCL::Array2D_int;
using ObjexxFCL::Array2D_string;
using ObjexxFCL::Array2S;
using ObjexxFCL::Array2S_bool;
using ObjexxFCL::Array2S_int;
using ObjexxFCL::Array2S_string;
using ObjexxFCL::Array3D;
using ObjexxFCL::Array3D_bool;
using ObjexxFCL::Array3D_int;
using ObjexxFCL::Array4D;
using ObjexxFCL::Array4D_int;
using ObjexxFCL::MArray1;
using ObjexxFCL::Reference;
using ObjexxFCL::Vector2;
using ObjexxFCL::Vector3;
using ObjexxFCL::Vector4;

// ObjexxFCL Functions
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/environment.hh>
#include <ObjexxFCL/floops.hh>
#include <ObjexxFCL/numeric.hh>
#include <ObjexxFCL/random.hh>
#include <ObjexxFCL/string.functions.hh>
#include <ObjexxFCL/time.hh>

using ObjexxFCL::CEILING;
using ObjexxFCL::equali;
using ObjexxFCL::floop_end;
using ObjexxFCL::get_environment_variable;
using ObjexxFCL::has;
using ObjexxFCL::has_any_of;
using ObjexxFCL::has_prefix;
using ObjexxFCL::has_prefixi;
using ObjexxFCL::hasi;
using ObjexxFCL::HUGE_;
using ObjexxFCL::index;
using ObjexxFCL::is_any_of;
using ObjexxFCL::is_blank;
using ObjexxFCL::len;
using ObjexxFCL::len_trim;
using ObjexxFCL::lessthani;
using ObjexxFCL::ljustified;
using ObjexxFCL::max;
using ObjexxFCL::min;
using ObjexxFCL::mod;
using ObjexxFCL::nint;
using ObjexxFCL::nint64;
using ObjexxFCL::not_blank;
using ObjexxFCL::pare;
using ObjexxFCL::pow_2;
using ObjexxFCL::pow_3;
using ObjexxFCL::pow_4;
using ObjexxFCL::pow_5;
using ObjexxFCL::pow_6;
using ObjexxFCL::pow_7;
using ObjexxFCL::RANDOM_NUMBER;
using ObjexxFCL::RANDOM_SEED;
using ObjexxFCL::rjustified;
using ObjexxFCL::root_4;
using ObjexxFCL::rstrip;
using ObjexxFCL::scan;
using ObjexxFCL::sign;
using ObjexxFCL::sized;
using ObjexxFCL::square;
using ObjexxFCL::strip;
using ObjexxFCL::stripped;
using ObjexxFCL::SYSTEM_CLOCK;
using ObjexxFCL::TINY;
using ObjexxFCL::trim;
using ObjexxFCL::trimmed;
using ObjexxFCL::uppercase;
using ObjexxFCL::uppercased;

#endif
