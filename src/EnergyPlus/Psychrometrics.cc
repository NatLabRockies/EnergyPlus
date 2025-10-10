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

// C++ Headers
#include <cstdlib>

// ObjexxFCL Headers
// #include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CommandLineInterface.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

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

namespace Psychrometrics {
    // Module containing the Psychometric simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 1998
    //       MODIFIED       February 2010
    //       RE-ENGINEERED  Jan 2004: Rahul Chillar

    // PURPOSE OF THIS MODULE:
    // This module provides a repository for the psychrometric routines.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // OTHER NOTES:
    // Todo after 2.2 release:
    // remove restriction on MAX(W, 1d-5)
    // more research on hfg calc

    // Using/Aliasing
#ifdef EP_psych_errors
    using namespace DataEnvironment;
#endif

    // Use Statements for other routines
#ifdef EP_psych_errors

#endif

    void InitializePsychRoutines([[maybe_unused]] EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes some variables for PsychRoutines

#ifdef EP_cache_PsyTwbFnTdbWPb
        state.dataPsychCache->cached_Twb.fill(cached_twb_t());
#endif
#ifdef EP_cache_PsyPsatFnTemp
        state.dataPsychCache->cached_Psat.fill(cached_psat_t());
#endif
#ifdef EP_cache_PsyTsatFnPb
        state.dataPsychCache->cached_Tsat.fill(cached_tsat_h_pb());
#endif
#ifdef EP_cache_PsyTsatFnHPb
        state.dataPsychCache->cached_Tsat_HPb.fill(cached_tsat_h_pb());
#endif
    }

    void ShowPsychrometricSummary([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] InputOutputFile &auditFile)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Provides a Psychrometric summary report to the audit file.
        // Maybe later to the .eio file.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
#ifdef EP_psych_stats
        int Loop;
        Real64 AverageIterations;

        if (!auditFile.good()) {
            return;
        }
        for (int item : state.dataPsychCache->NumTimesCalled) {
            if (item) { // if item is greater than 0
                print(auditFile, "RoutineName,#times Called,Avg Iterations\n");
                for (Loop = 0; Loop < static_cast<int>(PsychrometricFunction::Num); ++Loop) {
                    if (!PsyReportIt[Loop]) {
                        continue;
                    }
                    const std::string istring = fmt::to_string(state.dataPsychCache->NumTimesCalled[Loop]);
                    if (state.dataPsychCache->NumIterations[Loop] > 0) {
                        AverageIterations = double(state.dataPsychCache->NumIterations[Loop]) / double(state.dataPsychCache->NumTimesCalled[Loop]);
                        print(auditFile, "{},{},{:.2R}\n", PsyRoutineNames[Loop], istring, AverageIterations);
                    } else {
                        print(auditFile, "{},{}\n", PsyRoutineNames[Loop], istring);
                    }
                }
            }
        }
#endif
    }

#ifdef EP_psych_errors
    void PsyRhoAirFnPbTdbW_error(EnergyPlusData &state,
                                 Real64 const pb,                  // barometric pressure (Pascals)
                                 Real64 const tdb,                 // dry bulb temperature (Celsius)
                                 Real64 const dw,                  // humidity ratio (kgWater/kgDryAir)
                                 Real64 const rhoair,              // density of air
                                 std::string_view const CalledFrom // routine this function was called from (error messages) !unused1208
    )
    {
        // Using/Aliasing

        if (rhoair < 0.0) {
            ShowSevereError(state, format("PsyRhoAirFnPbTdbW: RhoAir (Density of Air) is calculated <= 0 [{:.5R}].", rhoair));
            ShowContinueError(state, format("pb =[{:.2R}], tdb=[{:.2R}], w=[{:.7R}].", pb, tdb, dw));
            if (!CalledFrom.empty()) {
                ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
            } else {
                ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
            }
            ShowFatalError(state, "Program terminates due to preceding condition.");
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyRhFnTdbRhovLBnd0C_error(EnergyPlusData &state,
                                    Real64 const Tdb,                 // dry-bulb temperature {C}
                                    Real64 const Rhovapor,            // vapor density in air {kg/m3}
                                    Real64 const RHValue,             // relative humidity value (0.0-1.0)
                                    std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (RHValue > 1.01) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhovLBnd0C)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Rhovapor= {:.3T} Calculated Relative Humidity [%]= {:.2T}", Tdb, Rhovapor, RHValue * 100.0);
                    ShowWarningMessage(state, "Calculated Relative Humidity out of range (PsyRhFnTdbRhovLBnd0C) ");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    ShowContinueError(state, "Relative Humidity being reset to 100.0%");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Relative Humidity out of range (PsyRhFnTdbRhovLBnd0C)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhovLBnd0C)],
                                               RHValue * 100.0,
                                               RHValue * 100.0,
                                               _,
                                               "%",
                                               "%");
            }
        } else if (RHValue < -0.05) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhovLBnd0C)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Rhovapor= {:.3T} Calculated Relative Humidity [%]= {:.2T}", Tdb, Rhovapor, RHValue * 100.0);
                    ShowWarningMessage(state, "Calculated Relative Humidity out of range (PsyRhFnTdbRhovLBnd0C) ");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    ShowContinueError(state, "Relative Humidity being reset to 1%");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Relative Humidity out of range (PsyRhFnTdbRhovLBnd0C)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhovLBnd0C)],
                                               RHValue * 100.0,
                                               RHValue * 100.0,
                                               _,
                                               "%",
                                               "%");
            }
        }
    }
#endif

#ifdef EP_cache_PsyTwbFnTdbWPb

    Real64 PsyTwbFnTdbWPb(EnergyPlusData &state,
                          Real64 const Tdb,                 // dry-bulb temperature {C}
                          Real64 const W,                   // humidity ratio
                          Real64 const Pb,                  // barometric pressure {Pascals}
                          std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie/Amir Roth
        //       DATE WRITTEN   August 2011

        // PURPOSE OF THIS FUNCTION:
        // Provide a "cache" of results for the given arguments and wetbulb (twb) output result.

        // METHODOLOGY EMPLOYED:
        // Use grid shifting and masking to provide hash into the cache. Use Equivalence to
        // make Fortran ignore "types".

        // FUNCTION PARAMETER DEFINITIONS:
        std::uint64_t constexpr Grid_Shift = 64 - 12 - twbprecision_bits;

#    ifdef EP_psych_stats
        ++state.dataPsychCache->NumTimesCalled[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb_cache)];
#    endif

        DISABLE_WARNING_PUSH
        DISABLE_WARNING_STRICT_ALIASING
        // cppcheck-suppress invalidPointerCast
        std::uint64_t Tdb_tag = *reinterpret_cast<std::uint64_t const *>(&Tdb) >> Grid_Shift;
        // cppcheck-suppress invalidPointerCast
        std::uint64_t W_tag = *reinterpret_cast<std::uint64_t const *>(&W) >> Grid_Shift;
        // cppcheck-suppress invalidPointerCast
        std::uint64_t Pb_tag = *reinterpret_cast<std::uint64_t const *>(&Pb) >> Grid_Shift;
        DISABLE_WARNING_POP

        std::uint64_t hash = (Tdb_tag ^ (W_tag ^ Pb_tag)) & std::uint64_t(twbcache_size - 1);

        auto &cached_Twb = state.dataPsychCache->cached_Twb;

        if (cached_Twb[hash].iTdb != Tdb_tag || cached_Twb[hash].iW != W_tag || cached_Twb[hash].iPb != Pb_tag) {
            cached_Twb[hash].iTdb = Tdb_tag;
            cached_Twb[hash].iW = W_tag;
            cached_Twb[hash].iPb = Pb_tag;

            DISABLE_WARNING_PUSH
            DISABLE_WARNING_STRICT_ALIASING
            Tdb_tag <<= Grid_Shift;
            // cppcheck-suppress invalidPointerCast
            Real64 Tdb_tag_r = *reinterpret_cast<Real64 const *>(&Tdb_tag);

            W_tag <<= Grid_Shift;
            // cppcheck-suppress invalidPointerCast
            Real64 W_tag_r = *reinterpret_cast<Real64 const *>(&W_tag);

            Pb_tag <<= Grid_Shift;
            // cppcheck-suppress invalidPointerCast
            Real64 Pb_tag_r = *reinterpret_cast<Real64 const *>(&Pb_tag);
            DISABLE_WARNING_POP

            cached_Twb[hash].Twb = PsyTwbFnTdbWPb_raw(state, Tdb_tag_r, W_tag_r, Pb_tag_r, CalledFrom);
        }

        return cached_Twb[hash].Twb;
    }

    Real64 PsyTwbFnTdbWPb_raw(EnergyPlusData &state,
                              Real64 const TDB,                 // dry-bulb temperature {C}
                              Real64 const dW,                  // humidity ratio
                              Real64 const Patm,                // barometric pressure {Pascals}
                              std::string_view const CalledFrom // routine this function was called from (error messages)
    )

#else

    Real64 PsyTwbFnTdbWPb(EnergyPlusData &state,
                          Real64 const TDB,                 // dry-bulb temperature {C}
                          Real64 const dW,                  // humidity ratio
                          Real64 const Patm,                // barometric pressure {Pascals}
                          std::string_view const CalledFrom // routine this function was called from (error messages)
    )
#endif
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         George Shih
        //       DATE WRITTEN   May 1976
        //       MODIFIED       na
        //       RE-ENGINEERED  Dec 2003; Rahul Chillar
        //                      2011; as time saving measure, cache some values.

        // PURPOSE OF THIS FUNCTION:
        // This function provides the wet-bulb temperature from dry-bulb temperature,
        // humidity ratio and barometric pressure.

        // METHODOLOGY EMPLOYED:
        // Uses an Iterative procedure to calculate WetBulbTemperature

        // Using/Aliasing
        using General::Iterate;

        // Return value
        Real64 TWB; // result=> Temperature Wet-Bulb {C}

        // FUNCTION PARAMETER DEFINITIONS:
        int constexpr itmax(100); // Maximum No of Iterations

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 tBoil;    // Boiling temperature of water at given pressure
        Real64 newW;     // Humidity ratio calculated with wet bulb guess
        Real64 W;        // Humidity ratio entered and corrected as necessary
        Real64 ResultX;  // ResultX is the final Iteration result passed back to the calling routine
        Real64 WBT;      // Current Value of WetBulbTemperature
        Real64 error;    // Deviation of dependent variable in iteration
        Real64 X1;       // Independent variable in ITERATE
        Real64 Y1;       // Dependent variable in ITERATE
        Real64 Wstar;    // Humidity  ratio as a function of Sat Press of Wet Bulb
        Real64 PSatstar; // Saturation pressure at wet bulb temperature
        int iter;        // Iteration counter
        int icvg;        // Iteration convergence flag

#ifdef EP_psych_stats
        ++state.dataPsychCache->NumTimesCalled[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb)];
#endif

        // CHECK TDB IN RANGE.
        bool FlagError = false;
#ifdef EP_psych_errors
        if (TDB <= -100.0 || TDB >= 200.0) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb)] == 0) {
                    ShowWarningMessage(state, "Temperature out of range [-100. to 200.] (PsyTwbFnTdbWPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, format(" Input Temperature={:.2T}", TDB));
                    FlagError = true;
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Temperature out of range [-100. to 200.] (PsyTwbFnTdbWPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb)],
                                               TDB,
                                               TDB,
                                               _,
                                               "C",
                                               "C");
            }
        }
#endif

        W = dW;
        if (W < 0.0) {
#ifdef EP_psych_errors
            if (W <= -0.0001) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb2)] == 0) {
                        state.dataPsychrometrics->String = format(" Dry-Bulb= {:.2T} Humidity Ratio= {:.3T} Pressure= {:.2T}", TDB, W, Patm);
                        ShowWarningMessage(state, "Entered Humidity Ratio invalid (PsyTwbFnTdbWPb)");
                        if (!CalledFrom.empty()) {
                            ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                        } else {
                            ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                        }
                        ShowContinueError(state, state.dataPsychrometrics->String);
                        state.dataPsychrometrics->String = format("Humidity Ratio= {:.4T}", W);
                        ShowContinueError(state, format("{} ... Humidity Ratio set to .00001", state.dataPsychrometrics->String));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Entered Humidity Ratio invalid (PsyTwbFnTdbWPb)",
                                                   state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb2)],
                                                   W,
                                                   W,
                                                   _,
                                                   "[]",
                                                   "[]");
                }
            }
#endif
            W = 1.0e-5;
        }

        // Initial temperature guess at atmospheric pressure
        if (Patm != state.dataPsychrometrics->last_Patm) {
            tBoil =
                PsyTsatFnPb(state, Patm, (CalledFrom.empty() ? PsyRoutineNames[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb)] : CalledFrom));
            state.dataPsychrometrics->last_Patm = Patm;
            state.dataPsychrometrics->last_tBoil = tBoil;
        } else {
            tBoil = state.dataPsychrometrics->last_tBoil;
        }

        // Set initial guess of WetBulbTemp=Entering Dry Bulb Temperature
        WBT = TDB;

        // Initializing  value for iter
        iter = 0;

        // Begin iteration loop
        for (iter = 1; iter <= itmax; ++iter) {

            // Assigning a value to WBT
            if (WBT >= (tBoil - 0.09)) {
                WBT = tBoil - 0.1;
            }

            // Determine the saturation pressure for wet bulb temperature
            PSatstar =
                PsyPsatFnTemp(state, WBT, (CalledFrom.empty() ? PsyRoutineNames[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb)] : CalledFrom));

            // Determine humidity ratio for given saturation pressure
            Wstar = 0.62198 * PSatstar / (Patm - PSatstar);

            // Calculate new humidity ratio and determine difference from known
            // humidity ratio which is wStar calculated earlier
            if (WBT >= 0.0) {
                newW = ((2501.0 - 2.326 * WBT) * Wstar - 1.006 * (TDB - WBT)) / (2501.0 + 1.86 * TDB - 4.186 * WBT);
            } else {
                newW = ((2830.0 - 0.24 * WBT) * Wstar - 1.006 * (TDB - WBT)) / (2830.0 + 1.86 * TDB - 2.1 * WBT);
            }

            // Check error, if not satisfied, calculate new guess and iterate
            error = W - newW;

            // Using Iterative Procedure to Calculate WetBulb
            Iterate(ResultX, state.dataPsychrometrics->iconvTol, WBT, error, X1, Y1, iter, icvg);
            WBT = ResultX;

            // If converged, leave iteration loop.
            if (icvg == 1) {
                break;
            }

        } // End of Iteration Loop

#ifdef EP_psych_stats
        state.dataPsychCache->NumIterations[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb)] += iter;
#endif

        // Wet bulb temperature has not converged after maximum specified
        // iterations. Print error message, set return error flag, and RETURN
#ifdef EP_psych_errors
        if (iter > itmax) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb3)] == 0) {
                    ShowWarningMessage(state, format("WetBulb not converged after {} iterations(PsyTwbFnTdbWPb)", iter));
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={},", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, format(" Input Temperature = {:.2T}", TDB));
                    ShowContinueError(state, format(" Input Humidity Ratio= {:.6T}", W));
                    ShowContinueError(state, format(" Input Pressure = {:.2T}", Patm));
                    FlagError = true;
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "WetBulb not converged after max iterations(PsyTwbFnTdbWPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TwbFnTdbWPb3)]);
            }
        }
#endif

        // Result is Temperature Wet Bulb
        TWB = WBT;

#ifdef EP_psych_errors
        if (FlagError) {
            ShowContinueError(state, format(" Resultant Temperature= {:.2T}", WBT));
        }
#endif

        // If (TempWetBulb)>(Dry Bulb Temp) , Setting (TempWetBulb)=(DryBulbTemp).
        if (TWB > TDB) {
            TWB = TDB;
        }

#ifdef generatetestdata
        print(IOFiles::getSingleton().debug, "{}{}{}{}", TDB, dW, Patm, Twb);
#endif

        return TWB;
    }

#ifdef EP_psych_errors
    void PsyVFnTdbWPb_error(EnergyPlusData &state,
                            Real64 const TDB,                 // dry-bulb temperature {C}
                            Real64 const w,                   // humidity ratio
                            Real64 const PB,                  // barometric pressure {Pascals}
                            Real64 const V,                   // specific volume {m3/kg}
                            std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (V <= -0.01) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::VFnTdbWPb)] == 0) {
                    state.dataPsychrometrics->String = format(" Dry-Bulb= {:.2T} Humidity Ratio= {:.3T} Pressure= {:.2T}", TDB, w, PB);
                    ShowWarningMessage(state, "Calculated Specific Volume out of range (PsyVFnTdbWPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    state.dataPsychrometrics->String = format("Calculated Volume= {:.3T}", V);
                    ShowContinueError(state, format("{} ... Since Calculated Volume < 0.0, it is set to .83", state.dataPsychrometrics->String));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Specific Volume out of range (PsyVFnTdbWPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::VFnTdbWPb)],
                                               V,
                                               V,
                                               _,
                                               "m3/kg",
                                               "m3/kg");
            }
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyWFnTdbH_error(EnergyPlusData &state,
                          Real64 const TDB,                 // dry-bulb temperature {C}
                          Real64 const H,                   // enthalpy {J/kg}
                          Real64 const W,                   // humidity ratio
                          std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (W < -0.0001) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbH)] == 0) {
                    state.dataPsychrometrics->String = format(" Dry-Bulb= {:.2T} Enthalpy= {:.3T}", TDB, H);
                    ShowWarningMessage(state, "Calculated Humidity Ratio invalid (PsyWFnTdbH)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    state.dataPsychrometrics->String = format("Calculated Humidity Ratio= {:.4T}", W);
                    ShowContinueError(state, format("{} ... Humidity Ratio set to .00001", state.dataPsychrometrics->String));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Humidity Ratio invalid (PsyWFnTdbH)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbH)],
                                               W,
                                               W,
                                               _,
                                               "[]",
                                               "[]");
            }
        }
    }
#endif

#ifdef EP_cache_PsyPsatFnTemp

    Real64 PsyPsatFnTemp_raw([[maybe_unused]] EnergyPlusData &state,
                             Real64 const T,                                    // dry-bulb temperature {C}
                             [[maybe_unused]] std::string_view const CalledFrom // routine this function was called from (error messages)
    )

#else

    Real64 PsyPsatFnTemp(EnergyPlusData &state,
                         Real64 const T,                                    // dry-bulb temperature {C}
                         [[maybe_unused]] std::string_view const CalledFrom // routine this function was called from (error messages)
    )
#endif
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         George Shih
        //       DATE WRITTEN   May 1976
        //       MODIFIED       NA
        //       RE-ENGINEERED  Nov 2003; Rahul Chillar

        // PURPOSE OF THIS FUNCTION:
        // This function provides the saturation pressure as a function of temperature.

        // METHODOLOGY EMPLOYED:
        // Hyland & Wexler Formulation, range -100C to 200C

        // REFERENCES:
        // ASHRAE HANDBOOK OF FUNDAMENTALS, 2005, Chap 6 (Psychrometrics), Eqn 5 & 6.
        // Compared to Table 3 values (August 2007) with average error of 0.00%, max .30%,
        // min -.39%.  (Spreadsheet available on request - Lawrie).

        // Note: the ASHRAE Handbook of Fundamentals  is being slightly inaccurate in its wording,
        // and referring to Eq 5 applying to -100°C to 0°C and Eq 6 applying to 0°C to 200°C
        // In fact, it is **not** 0°C, but the triple-point of water, which is 0.01°C. Evaluating the Eq 5 and 6 up to and from the triple-point
        // is removing the discontinuity altogether.

        // USE STATEMENTS:

        // Return value
        Real64 Pascal; // result=> saturation pressure {Pascals}

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:

#ifdef EP_psych_stats
        ++state.dataPsychCache->NumTimesCalled[static_cast<int>(PsychrometricFunction::PsatFnTemp)];
#endif

        // CHECK T IN RANGE.
#ifdef EP_psych_errors
        if (!state.dataGlobal->WarmupFlag) {
            if (T <= -100.0 || T >= 200.0) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::PsatFnTemp)] == 0) {
                    ShowWarningMessage(state, "Temperature out of range [-100. to 200.] (PsyPsatFnTemp)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={},", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, format(" Input Temperature={:.2T}", T));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Temperature out of range [-100. to 200.] (PsyPsatFnTemp)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::PsatFnTemp)],
                                               T,
                                               T,
                                               _,
                                               "C",
                                               "C");
            }
        }
#endif

        // Convert temperature from Centigrade to Kelvin.
        Real64 const Tkel(T + Constant::Kelvin); // Dry-bulb in REAL(r64) for function passing

        // If below -100C,set value of Pressure corresponding to Saturation Temperature of -100C.
        if (Tkel < 173.15) {
            Pascal = 0.001405102123874164;

            // If below freezing, calculate saturation pressure over ice.
        } else if (Tkel < Constant::TriplePointOfWaterTempKelvin) { // Tkel >= 173.15, Tkel < 273.16 (0.01°C)
            Real64 constexpr C1(-5674.5359);
            Real64 constexpr C2(6.3925247);
            Real64 constexpr C3(-0.9677843e-2);
            Real64 constexpr C4(0.62215701e-6);
            Real64 constexpr C5(0.20747825e-8);
            Real64 constexpr C6(-0.9484024e-12);
            Real64 constexpr C7(4.1635019);
            Pascal = std::exp(C1 / Tkel + C2 + Tkel * (C3 + Tkel * (C4 + Tkel * (C5 + C6 * Tkel))) + C7 * std::log(Tkel));

            // If above freezing, calculate saturation pressure over liquid water.
        } else if (Tkel <= 473.15) { // Tkel >= 173.15 // Tkel >= TriplePointOfWaterTempKelvin
#ifndef EP_IF97
            Real64 constexpr C8(-5800.2206);
            Real64 constexpr C9(1.3914993);
            Real64 constexpr C10(-0.048640239);
            Real64 constexpr C11(0.41764768e-4);
            Real64 constexpr C12(-0.14452093e-7);
            Real64 constexpr C13(6.5459673);
            Pascal = std::exp(C8 / Tkel + C9 + Tkel * (C10 + Tkel * (C11 + Tkel * C12)) + C13 * std::log(Tkel));

            // If above 200C, set value of Pressure corresponding to Saturation Temperature of 200C.
        } else { // Tkel >= 173.15 // Tkel >= TriplePointOfWaterTempKelvin // Tkel > 473.15
            Pascal = 1555073.745636215;
        }
#else
            // Table 34 in IF97
            Real64 constexpr N1(0.11670521452767e04);
            Real64 constexpr N2(-0.72421316703206e06);
            Real64 constexpr N3(-0.17073846940092e02);
            Real64 constexpr N4(0.12020824702470e05);
            Real64 constexpr N5(-0.32325550322333e07);
            Real64 constexpr N6(0.14915108613530e02);
            Real64 constexpr N7(-0.48232657361591e04);
            Real64 constexpr N8(0.40511340542057e06);
            Real64 constexpr N9(-0.23855557567849);
            Real64 constexpr N10(0.65017534844798e03);
            //         !IF97 equations
            Real64 const phi = Tkel + N9 / (Tkel - N10); // IF97 equation 29b
            Real64 const phi2 = phi * phi;               // phi squared
            Real64 const A = phi2 + N1 * phi + N2;
            Real64 const B = N3 * phi2 + N4 * phi + N5;
            Real64 const C = N6 * phi2 + N7 * phi + N8;
            Pascal = 1000000.0 * pow_4((2.0 * C) / (-B + std::sqrt((B * B) - 4.0 * A * C)));

            // If above 200C, set value of Pressure corresponding to Saturation Temperature of 200C.
        } else { // Tkel >= 173.15 // Tkel >= KelvinConv // Tkel > 473.15
            Pascal = 1554671.8682698254;
        }
#endif
        return Pascal;
    }

#ifdef EP_psych_errors
    void PsyWFnTdbTwbPb_temperature_error(EnergyPlusData &state,
                                          Real64 const TDB,                 // dry-bulb temperature {C}
                                          Real64 const TWB,                 // wet-bulb temperature {C}
                                          Real64 const PB,                  // barometric pressure {Pascals}
                                          std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (TWB > (TDB + 0.01)) {
            if (state.dataPsychrometrics->ReportErrors && !state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbTwbPb)] == 0) {
                    state.dataPsychrometrics->String = format(" Dry-Bulb= {:.2T} Pressure= {:.2T}", TDB, PB);
                    ShowWarningMessage(state, "Given Wet Bulb Temperature invalid (PsyWFnTdbTwbPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    state.dataPsychrometrics->String = format("Calculated Wet-Bulb= {:.2T}", TWB);
                    ShowContinueError(state,
                                      format("{} ... Since Dry Bulb < Wet Bulb, Wet Bulb set = to Dry Bulb", state.dataPsychrometrics->String));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Given Wet Bulb Temperature invalid (PsyWFnTdbTwbPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbTwbPb)],
                                               TWB,
                                               TWB,
                                               _,
                                               "C",
                                               "C");
            }
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyWFnTdbTwbPb_humidity_error(EnergyPlusData &state,
                                       Real64 const TDB,                 // dry-bulb temperature {C}
                                       Real64 const TWB,                 // wet-bulb temperature {C}
                                       Real64 const PB,                  // barometric pressure {Pascals}
                                       Real64 const W,                   // humidity ratio
                                       std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {

        if (W < 0.0) {
            if (state.dataPsychrometrics->ReportErrors && !state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbTwbPb2) == 0]) {
                    state.dataPsychrometrics->String = format(" Dry-Bulb= {:.2T} Wet-Bulb= {:.2T} Pressure= {:.2T}", TDB, TWB, PB);
                    ShowWarningMessage(state, "Calculated Humidity Ratio Invalid (PsyWFnTdbTwbPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    state.dataPsychrometrics->String = format("Calculated Humidity Ratio= {:.4T}, will recalculate Humidity Ratio", W);
                    ShowContinueError(
                        state, format("{} using Relative Humidity .01% (and Dry-Bulb and Pressure as shown)", state.dataPsychrometrics->String));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Humidity Ratio Invalid (PsyWFnTdbTwbPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbTwbPb2)],
                                               W,
                                               W,
                                               _,
                                               "[]",
                                               "[]");
            }
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyTdpFnTdbTwbPb_error(EnergyPlusData &state,
                                Real64 const TDB,                 // dry-bulb temperature {C}
                                Real64 const TWB,                 // wet-bulb temperature {C}
                                Real64 const PB,                  // barometric pressure (N/M**2) {Pascals}
                                Real64 const W,                   // humidity ratio
                                Real64 const TDP,                 // dew-point temperature {C}
                                std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (TDP > TWB + 0.1) {
            if (!state.dataGlobal->WarmupFlag) { // Display error message
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TdpFnTdbTwbPb)] == 0) {
                    ShowWarningMessage(state, "Calculated Dew Point Temperature being reset (PsyTdpFnTdbTwbPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    state.dataPsychrometrics->String =
                        format(" Dry-bulb={:.2T} Wet-Bulb (WB)= {:.2T} Pressure= {:.2T} Humidity Ratio={:.3T}", TDB, TWB, PB, W);
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    state.dataPsychrometrics->String =
                        format(" Calculated Dew Point Temperature (DPT)= {:.2T}; Since DPT > WB, DPT will be set to WB", TDP);
                    ShowContinueError(state, state.dataPsychrometrics->String);
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Dew Point Temperature being reset (PsyTdpFnTdbTwbPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TdpFnTdbTwbPb)],
                                               TDP,
                                               TDP,
                                               _,
                                               "C",
                                               "C");
            }
        }
    }
#endif

#ifdef EP_cache_PsyTsatFnHPb
    Real64 PsyTsatFnHPb_raw(EnergyPlusData &state,
                            Real64 const H,                                    // enthalpy {J/kg}
                            Real64 const PB,                                   // barometric pressure {Pascals}
                            [[maybe_unused]] std::string_view const CalledFrom // routine this function was called from (error messages)
    )
#else
    Real64 PsyTsatFnHPb(EnergyPlusData &state,
                        Real64 const H,                   // enthalpy {J/kg}
                        Real64 const PB,                  // barometric pressure {Pascals}
                        std::string_view const CalledFrom // routine this function was called from (error messages)
    )
#endif
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         George Shih
        //       DATE WRITTEN   May 1976
        //       MODIFIED       July 2003; LKL -- peg min/max values (outside range of functions)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function provides the saturation temperature from the enthalpy
        // and barometric pressure.

        // REFERENCES:
        // ASHRAE HANDBOOK OF FUNDAMENTALS, 1972, P99, EQN 22

        // Return value
        Real64 T; // result=> saturation temperature {C}

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 T1; // APPROXIMATE SATURATION TEMPERATURE (C)
        Real64 T2; // APPROXIMATE SATURATION TEMPERATURE (C)
        Real64 TN; // NEW ASSUMED SATURATION TEMPERATURE (C)
        Real64 H1; // APPROXIMATE ENTHALPY (J/KG)
        Real64 H2; // APPROXIMATE ENTHALPY (J/KG)
        Real64 Y1; // ERROR IN ENTHALPY
        Real64 Y2; // ERROR IN ENTHALPY
        int IterCount;
        Real64 HH;      // temporary enthalpy (calculation) value
        bool FlagError; // Set when errors should be flagged
        Real64 Hloc;    // local value of H

        HH = H + 1.78637e4;

        if (H >= 0.0) {
            Hloc = max(0.00001, H);
        } else {
            Hloc = min(-0.00001, H);
        }

#ifdef EP_psych_stats
        ++state.dataPsychCache->NumTimesCalled[static_cast<int>(PsychrometricFunction::TsatFnHPb)];
#endif

        FlagError = false;
#ifdef EP_psych_errors
        if (HH <= -4.24E4 || HH >= 4.5866E7) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TsatFnHPb)] == 0) {
                    ShowWarningMessage(state, "Enthalpy out of range (PsyTsatFnHPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={},", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    state.dataPsychrometrics->String = format(" Enthalpy={:.5T} Pressure= {:.2T}", HH, PB);
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    FlagError = true;
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Enthalpy out of range (PsyTsatFnHPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TsatFnHPb)],
                                               HH,
                                               HH,
                                               _,
                                               "J/kg",
                                               "J/kg");
            }
        }
#endif
        std::array<double, 10> CaseRange = {-4.24e4, -2.2138e4, -6.7012e2, 2.7297e4, 7.5222e4, 1.8379e5, 4.7577e5, 1.5445e6, 3.8353e6, 4.5866e7};
        int CaseIndex = 0;
        int beg(0), mid, end(9); // 1-based indexing

        while (beg + 1 < end) {
            mid = ((beg + end) >> 1);
            (HH > CaseRange[mid] ? beg : end) = mid;
        }

        CaseIndex = beg + 1;

        switch (CaseIndex) {
        case 1: // -2.2138e4 > HH > -4.24e4
            if (HH < -4.24e4) {
                HH = -4.24e4;
            }
            T = F6(HH, -19.44, 8.53675e-4, -5.12637e-9, -9.85546e-14, -1.00102e-18, -4.2705e-24);
            break;
        case 2: // -6.7012e2 > HH > -2.2138e4
            T = F6(HH, -1.94224e1, 8.5892e-4, -4.50709e-9, -6.19492e-14, 8.71734e-20, 8.73051e-24);
            break;
        case 3: // 2.7297e4 > HH > -6.7012e2
            T = F6(HH, -1.94224e1, 8.59061e-4, -4.4875e-9, -5.76696e-14, 7.72217e-19, 3.97894e-24);
            break;
        case 4: // 7.5222e4 > HH > 2.7297e4
            T = F6(HH, -2.01147e1, 9.04936e-4, -6.83305e-9, 2.3261e-14, 7.27237e-20, -6.31939e-25);
            break;
        case 5: // 7.5222e4 > HH > 2.7297e4
            T = F6(HH, -1.82124e1, 8.31683e-4, -6.16461e-9, 3.06411e-14, -8.60964e-20, 1.03003e-25);
            break;
        case 6:
            T = F6(HH, -1.29419, 3.88538e-4, -1.30237e-9, 2.78254e-15, -3.27225e-21, 1.60969e-27);
            break;
        case 7:
            T = F6(HH, 2.39214e1, 1.27519e-4, -1.52089e-10, 1.1043e-16, -4.33919e-23, 7.05296e-30);
            break;
        case 8:
            T = F6(HH, 4.88446e1, 3.85534e-5, -1.78805e-11, 4.87224e-18, -7.15283e-25, 4.36246e-32);
            break;
        case 9:
            if (HH > 4.5866e7) {
                HH = 4.5866e7;
            }
            T = F7(HH, 7.60565e11, 5.80534e4, -7.36433e-3, 5.11531e-10, -1.93619e-17, 3.70511e-25, -2.77313e-33);
            break;
        }

#ifdef EP_psych_errors
        if (FlagError) {
            ShowContinueError(state, format(" Initial Resultant Temperature= {:.2T}", T));
        }
#endif
        if (std::abs(PB - 1.0133e5) / 1.0133e5 > 0.01) {
            IterCount = 0;
            T1 = T;
            H1 = PsyHFnTdbW(T1, PsyWFnTdbTwbPb(state, T1, T1, PB, CalledFrom));
            Y1 = H1 - Hloc;
            if (std::abs(Y1 / Hloc) <= 0.1e-4) {
                T = T1;
            } else {
                T2 = T1 * 0.9;
                while (IterCount <= 30) {
                    ++IterCount;
                    H2 = PsyHFnTdbW(T2, PsyWFnTdbTwbPb(state, T2, T2, PB, CalledFrom));
                    Y2 = H2 - Hloc;
                    if (std::abs(Y2 / Hloc) <= 0.1e-4 || Y2 == Y1) {
                        T = T2;
                        break;
                    }

                    TN = T2 - Y2 / (Y2 - Y1) * (T2 - T1);
                    T1 = T2;
                    T2 = TN;
                    Y1 = Y2;
                }
#ifdef EP_psych_errors
                if (FlagError && IterCount > 30) {
                    ShowSevereError(state, "Temperature did not converge (PsyTsatFnHPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    state.dataPsychrometrics->String = format(" Enthalpy={:.5T} Pressure= {:.2T}", HH, PB);
                    ShowContinueError(state, format("{} Last T={:.2T}", state.dataPsychrometrics->String, T));
                }
#endif
            }
        }

        return T;
    }

#ifdef EP_psych_errors
    void PsyRhFnTdbRhov_error(EnergyPlusData &state,
                              Real64 const Tdb,                 // dry-bulb temperature {C}
                              Real64 const Rhovapor,            // vapor density in air {kg/m3}
                              Real64 const RHValue,             // relative humidity value (0.0-1.0)
                              std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (RHValue > 1.01) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhov)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Rhovapor= {:.3T} Calculated Relative Humidity [%]= {:.2T}", Tdb, Rhovapor, RHValue * 100.0);
                    ShowWarningMessage(state, "Calculated Relative Humidity out of range (PsyRhFnTdbRhov) ");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    ShowContinueError(state, "Relative Humidity being reset to 100.0 %");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Relative Humidity out of range (PsyRhFnTdbRhov)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhov)],
                                               RHValue * 100.0,
                                               RHValue * 100.0,
                                               _,
                                               "%",
                                               "%");
            }
        } else if (RHValue < -0.05) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhov)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Rhovapor= {:.3T} Calculated Relative Humidity [%]= {:.2T}", Tdb, Rhovapor, RHValue * 100.0);
                    ShowWarningMessage(state, "Calculated Relative Humidity out of range (PsyRhFnTdbRhov) ");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    ShowContinueError(state, "Relative Humidity being reset to 1%");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Relative Humidity out of range (PsyRhFnTdbRhov)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbRhov)],
                                               RHValue * 100.0,
                                               RHValue * 100.0,
                                               _,
                                               "%",
                                               "%");
            }
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyRhFnTdbWPb_error(EnergyPlusData &state,
                             Real64 const TDB,                 // dry-bulb temperature {C}
                             Real64 const W,                   // humidity ratio
                             Real64 const RHValue,             // relative humidity (0.0-1.0)
                             std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (RHValue > 1.01) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbWPb)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Humidity Ratio= {:.3T} Calculated Relative Humidity [%]= {:.2T}", TDB, W, RHValue * 100.0);
                    ShowWarningMessage(state, "Calculated Relative Humidity out of range (PsyRhFnTdbWPb) ");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    ShowContinueError(state, "Relative Humidity being reset to 100.0%");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Relative Humidity out of range (PsyRhFnTdbWPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbWPb)],
                                               RHValue * 100.0,
                                               RHValue * 100.0,
                                               _,
                                               "%",
                                               "%");
            }
        } else if (RHValue < -0.05) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbWPb)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Humidity Ratio= {:.3T} Calculated Relative Humidity [%]= {:.2T}", TDB, W, RHValue * 100.0);
                    ShowWarningMessage(state, "Calculated Relative Humidity out of range (PsyRhFnTdbWPb) ");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    ShowContinueError(state, "Relative Humidity being reset to 1%");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Relative Humidity out of range (PsyRhFnTdbWPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::RhFnTdbWPb)],
                                               RHValue * 100.0,
                                               RHValue * 100.0,
                                               _,
                                               "%",
                                               "%");
            }
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyWFnTdpPb_error(EnergyPlusData &state,
                           Real64 const TDP,                 // dew-point temperature {C}
                           Real64 const PB,                  // barometric pressure {Pascals}
                           Real64 const W,                   // humidity ratio
                           Real64 const DeltaT,              // Reduced temperature difference of dew point
                           std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (!state.dataGlobal->WarmupFlag) {
            if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdpPb)] == 0) {
                state.dataPsychrometrics->String = format(" Dew-Point= {:.2T} Barometric Pressure= {:.2T}", TDP, PB);
                ShowWarningMessage(state,
                                   "Calculated partial vapor pressure is greater than the barometric pressure, so that calculated humidity ratio is "
                                   "invalid (PsyWFnTdpPb).");
                if (!CalledFrom.empty()) {
                    ShowContinueErrorTimeStamp(state, format(" Routine={},", CalledFrom));
                } else {
                    ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                }
                ShowContinueError(state, state.dataPsychrometrics->String);
                state.dataPsychrometrics->String =
                    format("Instead, calculated Humidity Ratio at {:.1T} ({} degree less) = {:.4T}", TDP - DeltaT, static_cast<int>(DeltaT), W);
                ShowContinueError(state, format("{} will be used. Simulation continues.", state.dataPsychrometrics->String));
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "Entered Humidity Ratio invalid (PsyWFnTdpPb)",
                                           state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdpPb)],
                                           W,
                                           W,
                                           _,
                                           "[]",
                                           "[]");
        }
    }
#endif

#ifdef EP_psych_errors
    void PsyWFnTdbRhPb_error(EnergyPlusData &state,
                             Real64 const TDB,                 // dry-bulb temperature {C}
                             Real64 const RH,                  // relative humidity value (0.0-1.0)
                             Real64 const PB,                  // barometric pressure {Pascals}
                             Real64 const W,                   // humidity ratio
                             std::string_view const CalledFrom // routine this function was called from (error messages)
    )
    {
        if (W <= -0.0001) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbRhPb)] == 0) {
                    state.dataPsychrometrics->String =
                        format(" Dry-Bulb= {:.2T} Relative Humidity [%]= {:.2T} Pressure= {:.2T}", TDB, RH * 100.0, PB);
                    ShowWarningMessage(state, "Calculated Humidity Ratio is invalid (PsyWFnTdbRhPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, state.dataPsychrometrics->String);
                    state.dataPsychrometrics->String = format("Calculated Humidity Ratio= {:.4T}", W);
                    ShowContinueError(state, format("{} ... Humidity Ratio set to .00001", state.dataPsychrometrics->String));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Calculated Humidity Ratio Invalid (PsyWFnTdbTwbPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::WFnTdbRhPb)],
                                               W,
                                               W,
                                               _,
                                               "[]",
                                               "[]");
            }
        }
    }
#endif

#ifdef EP_cache_PsyTsatFnPb

    Real64 PsyTsatFnPb_raw(EnergyPlusData &state,
                           Real64 const Press,               // barometric pressure {Pascals}
                           std::string_view const CalledFrom // routine this function was called from (error messages)
    )

#else
    Real64 PsyTsatFnPb(EnergyPlusData &state,
                       Real64 const Press,               // barometric pressure {Pascals}
                       std::string_view const CalledFrom // routine this function was called from (error messages)
    )
#endif
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         George Shih
        //       DATE WRITTEN   May 1976
        //       RE-ENGINEERED  Dec 2003; Rahul Chillar

        // PURPOSE OF THIS FUNCTION:
        // This function provides the saturation temperature from barometric pressure.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // 1989 ASHRAE Handbook - Fundamentals
        // Checked against 2005 HOF, Chap 6, Table 3 (using pressure in, temperature out) with
        // good correlation from -60C to 160C

        // Using/Aliasing
        using General::Iterate;

        // Return value

        // Locals
        // FUNCTION ARGUMENT DEFINITIONS:

        // FUNCTION PARAMETER DEFINITIONS:
        int constexpr itmax(50); // Maximum number of iterations
        Real64 constexpr convTol(0.0001);

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 tSat; // Water temperature guess
        int iter;    // Iteration counter

#ifdef EP_psych_stats
        ++state.dataPsychCache->NumTimesCalled[static_cast<int>(PsychrometricFunction::TsatFnPb)];
#endif

        // Check press in range.
        bool FlagError = false;
#ifdef EP_psych_errors
        if (!state.dataGlobal->WarmupFlag) {
            if (Press <= 0.0017 || Press >= 1555000.0) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TsatFnPb)] == 0) {
                    ShowWarningMessage(state, "Pressure out of range (PsyTsatFnPb)");
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, format(" Input Pressure= {:.2T}", Press));
                    FlagError = true;
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Pressure out of range (PsyTsatFnPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TsatFnPb)],
                                               Press,
                                               Press,
                                               _,
                                               "Pa",
                                               "Pa");
            }
        }
#endif
        if (Press == state.dataPsychrometrics->Press_Save) {
            return state.dataPsychrometrics->tSat_Save;
        }
        state.dataPsychrometrics->Press_Save = Press;
        iter = 0;
        if (state.dataPsychrometrics->useInterpolationPsychTsatFnPb) {
            int n_sample = 1651; // sample bin size = 64 Pa; continous sample size = 1651
            // CSpline interpolation
            tSat = CSplineint(n_sample, Press); // Cubic spline interpolation
        } else {
            // Uses an iterative process to determine the saturation temperature at a given
            // pressure by correlating saturated water vapor as a function of temperature.

            // Initial guess of boiling temperature
            tSat = 100.0;

            // If above 1555000,set value of Temp corresponding to Saturation Pressure of 1555000 Pascal.
            if (Press >= 1555000.0) {
                tSat = 200.0;
                // If below 0.0017,set value of Temp corresponding to Saturation Pressure of 0.0017 Pascal.
            } else if (Press <= 0.0017) {
                tSat = -100.0;

                // Setting Value of PsyTsatFnPb= 0C, due to non-continuous function for Saturation Pressure at 0C.
            } else if ((Press > 611.000) && (Press < 611.25)) {
                tSat = 0.0;

            } else {
                // Iterate to find the saturation temperature
                // of water given the total pressure

                // Set iteration loop parameters
                // make sure these are initialized
                Real64 pSat;    // Pressure corresponding to temp. guess
                Real64 error;   // Deviation of dependent variable in iteration
                Real64 X1;      // Previous value of independent variable in ITERATE
                Real64 Y1;      // Previous value of dependent variable in ITERATE
                Real64 ResultX; // ResultX is the final Iteration result passed back to the calling routine
                bool const CalledFrom_empty(CalledFrom.empty());
                int icvg; // Iteration convergence flag
                for (iter = 1; iter <= itmax; ++iter) {

                    // Calculate saturation pressure for estimated boiling temperature
                    pSat = PsyPsatFnTemp(
                        state, tSat, (CalledFrom_empty ? PsyRoutineNames[static_cast<int>(PsychrometricFunction::TsatFnPb)] : CalledFrom));

                    // Compare with specified pressure and update estimate of temperature
                    error = Press - pSat;
                    Iterate(ResultX, convTol, tSat, error, X1, Y1, iter, icvg);
                    tSat = ResultX;
                    // If converged leave loop iteration
                    if (icvg == 1) {
                        break;
                    }

                    // Water temperature not converged, repeat calculations with new
                    // estimate of water temperature
                }

                // Saturation temperature has not converged after maximum specified
                // iterations. Print error message, set return error flag, and RETURN
            }
        } // End If for the Pressure Range Checking

#ifdef EP_psych_stats
        state.dataPsychCache->NumIterations[static_cast<int>(PsychrometricFunction::TsatFnPb)] += iter;
#endif

#ifdef EP_psych_errors
        if (iter > itmax) {
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TsatFnPb2)] == 0) {
                    ShowWarningMessage(state, format("Saturation Temperature not converged after {} iterations (PsyTsatFnPb)", iter));
                    if (!CalledFrom.empty()) {
                        ShowContinueErrorTimeStamp(state, format(" Routine={}", CalledFrom));
                    } else {
                        ShowContinueErrorTimeStamp(state, " Routine=Unknown,");
                    }
                    ShowContinueError(state, format(" Input Pressure= {:.2T}", Press));
                    FlagError = true;
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "Saturation Temperature not converged after max iterations (PsyTsatFnPb)",
                                               state.dataPsychrometrics->iPsyErrIndex[static_cast<int>(PsychrometricFunction::TsatFnPb2)],
                                               tSat,
                                               tSat,
                                               _,
                                               "C",
                                               "C");
            }
        }
#endif

        // Result is SatTemperature
        Real64 const Temp = state.dataPsychrometrics->tSat_Save = tSat; // result=> saturation temperature {C}

#ifdef EP_psych_errors
        if (FlagError) {
            ShowContinueError(state, format(" Resultant Temperature= {:.2T}", Temp));
        }
#endif

        return Temp;
    }
    Real64 CSplineint(int const n, // sample data size
                      Real64 x)    // given value of x
    {                              // Cubic Spline interpolation
        // Reference: Numerical Recipies in C (pp.97)
        Real64 A, B, y;
        // find location of x in arrays without searching since array bins are equally sized
        int x_int = static_cast<int>(x);
        //********continous sample start
        int j = (x_int >> 6) - 1; // sample bin 64, sample size=1651
        if (j < 0) {
            j = 0;
        }
        if (j > (n - 2)) {
            j = n - 2;
        }
        static constexpr Real64 h(64); // sample bin 64, sample size=1651
        //********continous sample end
        int tsat_fn_pb_x_j1 = 64 * (j + 1); // sample data for pressure
        A = (tsat_fn_pb_x_j1 - x) / h;
        B = 1 - A;
        y = A * tsat_fn_pb_y[j] + B * tsat_fn_pb_y[j + 1] +
            ((A * A * A - A) * (tsat_fn_pb_d2y[j]) + (B * B * B - B) * (tsat_fn_pb_d2y[j + 1])) * (h * h) * 0.1666666667;
        return y;
    }

} // namespace Psychrometrics

} // namespace EnergyPlus
