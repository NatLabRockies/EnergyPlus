// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef PsychCacheData_hh_INCLUDED
#define PsychCacheData_hh_INCLUDED

#include <EnergyPlus/Data/BaseData.hh>

namespace EnergyPlus {

enum class PsychrometricFunction : int
{
    Invalid = -1,
    TdpFnTdbTwbPb,
    RhFnTdbWPb,
    TwbFnTdbWPb,
    VFnTdbWPb,
    WFnTdpPb,
    WFnTdbH,
    WFnTdbTwbPb,
    WFnTdbRhPb,
    PsatFnTemp,
    TsatFnHPb,
    TsatFnPb,
    RhFnTdbRhov,
    RhFnTdbRhovLBnd0C,
    TwbFnTdbWPb2,
    TwbFnTdbWPb3, // convergence,
    WFnTdbTwbPb2,
    TsatFnPb2, // iteration,
    TwbFnTdbWPb_cache,
    PsatFnTemp_cache,
    Num // The number of enums in this enum class
};

#ifdef EP_nocache_Psychrometrics
#    undef EP_cache_PsyTwbFnTdbWPb
#    undef EP_cache_PsyPsatFnTemp
#    undef EP_cache_PsyTsatFnPb
#    undef EP_cache_PsyTsatFnHPb
#else
#    define EP_cache_PsyTwbFnTdbWPb
#    define EP_cache_PsyPsatFnTemp
#    define EP_cache_PsyTsatFnPb
#    define EP_cache_PsyTsatFnHPb
#endif

#ifdef EP_cache_PsyTwbFnTdbWPb
constexpr int twbcache_size = 1024 * 1024;
constexpr int twbprecision_bits = 20;
#endif
#ifdef EP_cache_PsyPsatFnTemp
constexpr int psatcache_size = 1024 * 1024;
constexpr int psatprecision_bits = 24; // 28  //24  //32
constexpr Int64 psatcache_mask = psatcache_size - 1;
#endif
#ifdef EP_cache_PsyTsatFnPb
constexpr int tsatcache_size = 1024 * 1024;
constexpr Int64 tsatcache_mask = tsatcache_size - 1;
#endif
#ifdef EP_cache_PsyTsatFnHPb
constexpr int tsat_hbp_cache_size = 1024 * 1024;
constexpr int tsat_hbp_precision_bits = 28;
#endif

#ifdef EP_cache_PsyTwbFnTdbWPb
struct cached_twb_t
{
    // Members
    std::uint64_t iTdb{0};
    std::uint64_t iW{0};
    std::uint64_t iPb{0};
    Real64 Twb{0};
};
#endif
#ifdef EP_cache_PsyTsatFnHPb
struct cached_tsat_h_pb
{
    // Members
    Int64 iH;
    Int64 iPb;
    Real64 Tsat;

    // Default Constructor
    cached_tsat_h_pb() : iH(0), iPb(0), Tsat(0.0)
    {
    }
};
#endif
#ifdef EP_cache_PsyPsatFnTemp
struct cached_psat_t
{
    // Members
    Int64 iTdb;
    Real64 Psat;

    // Default Constructor
    cached_psat_t() : iTdb(-1000), Psat(0.0)
    {
    }
};
#endif
#ifdef EP_cache_PsyTsatFnPb
struct cached_tsat_pb
{
    // Members
    Int64 iPb;
    Real64 Tsat;

    // Default Constructor
    cached_tsat_pb() : iPb(-1000), Tsat(0.0)
    {
    }
};

#endif

struct PsychrometricCacheData : BaseGlobalStruct
{

#ifdef EP_cache_PsyTwbFnTdbWPb
    std::array<cached_twb_t, twbcache_size> cached_Twb;
#endif
#ifdef EP_cache_PsyPsatFnTemp
    std::array<cached_psat_t, psatcache_size> cached_Psat;
#endif
#ifdef EP_cache_PsyTsatFnPb
    std::array<cached_tsat_h_pb, tsatcache_size> cached_Tsat;
    int tsatprecision_bits = 24;
#endif
#ifdef EP_cache_PsyTsatFnHPb
    std::array<cached_tsat_h_pb, tsat_hbp_cache_size> cached_Tsat_HPb;
#endif

#ifdef EP_psych_stats
    std::array<std::int64_t, static_cast<int>(PsychrometricFunction::Num)> NumTimesCalled;
    std::array<int, static_cast<int>(PsychrometricFunction::Num)> NumIterations;
#endif

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
#ifdef EP_psych_stats
        NumTimesCalled.fill(0);
        NumIterations.fill(0);
#endif
#ifdef EP_cache_PsyTsatFnPb
        tsatprecision_bits = 24;
#endif
    }
};

} // namespace EnergyPlus

#endif // PsychCacheData_hh_INCLUDED
