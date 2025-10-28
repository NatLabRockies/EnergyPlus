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
#include <algorithm>
#include <boost/math/tools/roots.hpp>
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace ThermalComfort {

    // Module containing the routines dealing with the CalcThermalComfortFanger,
    // CalcThermalComfortPierce, and CalcThermalComfortKSU

    // MODULE INFORMATION:
    //       AUTHOR         Jaewook Lee
    //       DATE WRITTEN   January 2000
    //       MODIFIED       Rick Strand (for E+ implementation February 2000)

    // PURPOSE OF THIS MODULE:
    // To calculate thermal comfort indices based on the
    // three thermal comfort prediction models (Fanger, Pierce, KSU)

    // METHODOLOGY EMPLOYED:
    // For each thermal comfort model type, the subroutines will loop through
    // the people statements and perform the requested thermal comfort evaluations

    // Using/Aliasing
    using DataHeatBalance::PeopleData;
    using Psychrometrics::PsyRhFnTdbWPb;

    void ManageThermalComfort(EnergyPlusData &state, bool const InitializeOnly) // when called from ZTPC and calculations aren't needed
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Rick Strand
        //     DATE WRITTEN   February 2000

        if (state.dataThermalComforts->FirstTimeFlag) {
            InitThermalComfort(state); // Mainly sets up output stuff
            state.dataThermalComforts->FirstTimeFlag = false;
        }

        if (state.dataGlobal->DayOfSim == 1) {
            if (state.dataGlobal->HourOfDay < 7) {
                state.dataThermalComforts->TemporarySixAMTemperature = 1.868132;
            } else if (state.dataGlobal->HourOfDay == 7) {
                if (state.dataGlobal->TimeStep == 1) {
                    state.dataThermalComforts->TemporarySixAMTemperature = state.dataEnvrn->OutDryBulbTemp;
                }
            }
        } else {
            if (state.dataGlobal->HourOfDay == 7) {
                if (state.dataGlobal->TimeStep == 1) {
                    state.dataThermalComforts->TemporarySixAMTemperature = state.dataEnvrn->OutDryBulbTemp;
                }
            }
        }

        if (InitializeOnly) {
            return;
        }

        if (state.dataGlobal->BeginEnvrnFlag) {
            state.dataThermalComforts->ZoneOccHrs = 0.0;
        }

        if (!state.dataGlobal->DoingSizing && !state.dataGlobal->WarmupFlag) {
            CalcThermalComfortFanger(state);
            if (state.dataHeatBal->AnyThermalComfortPierceModel) {
                CalcThermalComfortPierceASHRAE(state);
            }
            if (state.dataHeatBal->AnyThermalComfortKSUModel) {
                CalcThermalComfortKSU(state);
            }
            if (state.dataHeatBal->AnyThermalComfortCoolingEffectModel) {
                CalcThermalComfortCoolingEffectASH(state);
            }
            if (state.dataHeatBal->AnyThermalComfortAnkleDraftModel) {
                CalcThermalComfortAnkleDraftASH(state);
            }
            CalcThermalComfortSimpleASH55(state);
            CalcIfSetPointMet(state);
            if (state.dataHeatBal->AdaptiveComfortRequested_ASH55) {
                CalcThermalComfortAdaptiveASH55(state, false);
            }
            if (state.dataHeatBal->AdaptiveComfortRequested_CEN15251) {
                CalcThermalComfortAdaptiveCEN15251(state, false);
            }
        }
    }

    void InitThermalComfort(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Rick Strand
        //     DATE WRITTEN   February 2000

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop; // DO loop counter
        std::string CurrentGroupName;

        state.dataThermalComforts->ThermalComfortData.allocate(state.dataHeatBal->TotPeople);

        for (Loop = 1; Loop <= state.dataHeatBal->TotPeople; ++Loop) {

            CurrentGroupName = state.dataHeatBal->People(Loop).Name;

            // CurrentModuleObject='People'
            // MJW MRT ToDo: Rename most Zone Thermal Comfort output variables to People Thermal Comfort ('cause they're keyed by People name)
            if (state.dataHeatBal->People(Loop).Fanger) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Fanger Model PMV",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).FangerPMV,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Fanger Model PPD",
                                    Constant::Units::Perc,
                                    state.dataThermalComforts->ThermalComfortData(Loop).FangerPPD,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Clothing Surface Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).CloSurfTemp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }

            if (state.dataHeatBal->People(Loop).Pierce) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Pierce Model Effective Temperature PMV",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).PiercePMVET,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Pierce Model Standard Effective Temperature PMV",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).PiercePMVSET,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Pierce Model Discomfort Index",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).PierceDISC,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Pierce Model Thermal Sensation Index",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).PierceTSENS,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Pierce Model Standard Effective Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).PierceSET,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }

            if (state.dataHeatBal->People(Loop).KSU) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort KSU Model Thermal Sensation Vote",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).KsuTSV,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }

            if ((state.dataHeatBal->People(Loop).Fanger) || (state.dataHeatBal->People(Loop).Pierce) || (state.dataHeatBal->People(Loop).KSU)) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Mean Radiant Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortMRT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Operative Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortOpTemp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Clothing Value",
                                    Constant::Units::clo,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ClothingValue,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }

            if (state.dataHeatBal->People(Loop).AdaptiveASH55) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Adaptive Model 90% Acceptability Status",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortAdaptiveASH5590,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Adaptive Model 80% Acceptability Status",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortAdaptiveASH5580,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Adaptive Model Running Average Outdoor Air Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ASHRAE55RunningMeanOutdoorTemp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Adaptive Model Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).TComfASH55,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }

            if (state.dataHeatBal->People(Loop).AdaptiveCEN15251) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort CEN 15251 Adaptive Model Category I Status",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortAdaptiveCEN15251CatI,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort CEN 15251 Adaptive Model Category II Status",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortAdaptiveCEN15251CatII,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort CEN 15251 Adaptive Model Category III Status",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).ThermalComfortAdaptiveCEN15251CatIII,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort CEN 15251 Adaptive Model Running Average Outdoor Air Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).CEN15251RunningMeanOutdoorTemp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort CEN 15251 Adaptive Model Temperature",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).TComfCEN15251,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }
            if (state.dataHeatBal->People(Loop).CoolingEffectASH55) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Elevated Air Speed Cooling Effect",
                                    Constant::Units::C,
                                    state.dataThermalComforts->ThermalComfortData(Loop).CoolingEffectASH55,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Elevated Air Speed Cooling Effect Adjusted PMV",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).CoolingEffectAdjustedPMVASH55,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Elevated Air Speed Cooling Effect Adjusted PPD",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).CoolingEffectAdjustedPPDASH55,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }
            if (state.dataHeatBal->People(Loop).AnkleDraftASH55) {
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort ASHRAE 55 Ankle Draft PPD",
                                    Constant::Units::None,
                                    state.dataThermalComforts->ThermalComfortData(Loop).AnkleDraftPPDASH55,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->People(Loop).Name);
            }
        }
        state.dataThermalComforts->ThermalComfortInASH55.allocate(state.dataGlobal->NumOfZones);

        // ASHRAE 55 Warning. If any people statement for a zone is true, set that zone to true
        for (Loop = 1; Loop <= state.dataHeatBal->TotPeople; ++Loop) {
            if (state.dataHeatBal->People(Loop).Show55Warning) {
                state.dataThermalComforts->ThermalComfortInASH55(state.dataHeatBal->People(Loop).ZonePtr).Enable55Warning = true;
            }
        }

        // CurrentModuleObject='Zone'
        for (Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            SetupOutputVariable(state,
                                "Zone Thermal Comfort ASHRAE 55 Simple Model Summer Clothes Not Comfortable Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortInASH55(Loop).timeNotSummer,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Thermal Comfort ASHRAE 55 Simple Model Winter Clothes Not Comfortable Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortInASH55(Loop).timeNotWinter,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Thermal Comfort ASHRAE 55 Simple Model Summer or Winter Clothes Not Comfortable Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortInASH55(Loop).timeNotEither,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
        }
        SetupOutputVariable(state,
                            "Facility Thermal Comfort ASHRAE 55 Simple Model Summer Clothes Not Comfortable Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Summer,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Thermal Comfort ASHRAE 55 Simple Model Winter Clothes Not Comfortable Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Winter,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Thermal Comfort ASHRAE 55 Simple Model Summer or Winter Clothes Not Comfortable Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Either,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");

        state.dataThermalComforts->ThermalComfortSetPoint.allocate(state.dataGlobal->NumOfZones);
        for (Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            SetupOutputVariable(state,
                                "Zone Heating Setpoint Not Met Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortSetPoint(Loop).notMetHeating,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Heating Setpoint Not Met While Occupied Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortSetPoint(Loop).notMetHeatingOccupied,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Cooling Setpoint Not Met Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortSetPoint(Loop).notMetCooling,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Cooling Setpoint Not Met While Occupied Time",
                                Constant::Units::hr,
                                state.dataThermalComforts->ThermalComfortSetPoint(Loop).notMetCoolingOccupied,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(Loop).Name);
        }

        SetupOutputVariable(state,
                            "Facility Heating Setpoint Not Met Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneNotMetHeating,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Cooling Setpoint Not Met Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneNotMetCooling,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Heating Setpoint Not Met While Occupied Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneNotMetHeatingOccupied,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Cooling Setpoint Not Met While Occupied Time",
                            Constant::Units::hr,
                            state.dataThermalComforts->AnyZoneNotMetCoolingOccupied,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "Facility");

        GetAngleFactorList(state);

        state.dataThermalComforts->ZoneOccHrs.dimension(state.dataGlobal->NumOfZones, 0.0);
    }

    void CalcThermalComfortFanger(EnergyPlusData &state,
                                  ObjexxFCL::Optional_int_const PNum,     // People number for thermal comfort control
                                  ObjexxFCL::Optional<Real64 const> Tset, // Temperature setpoint for thermal comfort control
                                  ObjexxFCL::Optional<Real64> PMVResult   // PMV value for thermal comfort control
    )
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   January 2000
        //     MODIFIED       Rick Strand (for E+ implementation February 2000)
        //                    Brent Griffith modifications for CR 5641 (October 2005)
        //                    L. Gu, Added optional arguments for thermal comfort control (May 2006)
        //                    T. Hong, added Fanger PPD (April 2009)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates PMV(Predicted Mean Vote) using the Fanger thermal
        // comfort model. This subroutine is also used for thermal comfort control by determining
        // the temperature at which the PMV is equal to a PMV setpoint specified by the user.

        // METHODOLOGY EMPLOYED:
        // This subroutine is based heavily upon the work performed by Dan Maloney for
        // the BLAST program.  Many of the equations are based on the original Fanger
        // development.  See documentation for further details and references.

        // REFERENCES:
        // Maloney, Dan, M.S. Thesis, University of Illinois at Urbana-Champaign
        // BG note (10/21/2005),  This formulation is based on the the BASIC program
        // that is included in ASHRAE Standard 55 Normative Appendix D.

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) { // Is there a reason why this is a state variable and not a local variable?

            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

            // Optional argument is used to access people object when thermal comfort control is used
            if (present(PNum)) {
                if (state.dataThermalComforts->PeopleNum != PNum) {
                    continue;
                }
            }

            // If optional argument is used do not cycle regardless of thermal comfort reporting type
            if ((!people.Fanger) && (!present(PNum))) {
                continue;
            }

            state.dataThermalComforts->ZoneNum = people.ZonePtr;
            auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataThermalComforts->ZoneNum);
            if (present(PNum)) {
                state.dataThermalComforts->AirTemp = Tset;
            } else {
                state.dataThermalComforts->AirTemp = thisZoneHB.ZTAVComf;
            }
            if (state.dataRoomAir->anyNonMixingRoomAirModel) {
                state.dataThermalComforts->ZoneNum = people.ZonePtr;
                if (state.dataRoomAir->IsZoneDispVent3Node(state.dataThermalComforts->ZoneNum) ||
                    state.dataRoomAir->IsZoneUFAD(state.dataThermalComforts->ZoneNum)) {
                    state.dataThermalComforts->AirTemp = state.dataRoomAir->TCMF(state.dataThermalComforts->ZoneNum); // PH 3/7/04
                    // UCSD-CV
                } else if (state.dataRoomAir->IsZoneCrossVent(state.dataThermalComforts->ZoneNum)) {
                    if (state.dataRoomAir->ZoneCrossVent(state.dataThermalComforts->ZoneNum).VforComfort == RoomAir::Comfort::Jet) {
                        state.dataThermalComforts->AirTemp = state.dataRoomAir->ZTJET(state.dataThermalComforts->ZoneNum);
                    } else if (state.dataRoomAir->ZoneCrossVent(state.dataThermalComforts->ZoneNum).VforComfort == RoomAir::Comfort::Recirculation) {
                        state.dataThermalComforts->AirTemp = state.dataRoomAir->ZTJET(state.dataThermalComforts->ZoneNum);
                    }
                }
            }
            state.dataThermalComforts->RadTemp = CalcRadTemp(state, state.dataThermalComforts->PeopleNum);
            // Use mean air temp for calculating RH when thermal comfort control is used
            if (present(PNum)) {
                state.dataThermalComforts->RelHum =
                    PsyRhFnTdbWPb(state,
                                  state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataThermalComforts->ZoneNum).MAT,
                                  thisZoneHB.airHumRatAvgComf,
                                  state.dataEnvrn->OutBaroPress);
            } else {
                state.dataThermalComforts->RelHum =
                    PsyRhFnTdbWPb(state, state.dataThermalComforts->AirTemp, thisZoneHB.airHumRatAvgComf, state.dataEnvrn->OutBaroPress);
            }
            people.TemperatureInZone = state.dataThermalComforts->AirTemp;
            people.RelativeHumidityInZone = state.dataThermalComforts->RelHum * 100.0;

            // Metabolic rate of body (W/m2)
            state.dataThermalComforts->ActLevel = people.activityLevelSched->getCurrentVal() / BodySurfArea;
            // Energy consumption by external work (W/m2)
            state.dataThermalComforts->WorkEff = people.workEffSched->getCurrentVal() * state.dataThermalComforts->ActLevel;
            // Clothing unit
            Real64 IntermediateClothing;
            switch (people.clothingType) {
            case DataHeatBalance::ClothingType::InsulationSchedule:
                state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                break;
            case DataHeatBalance::ClothingType::DynamicAshrae55:
                comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
                comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                DynamicClothingModel(state);
                state.dataThermalComforts->CloUnit = comfort.ClothingValue;
                break;
            case DataHeatBalance::ClothingType::CalculationSchedule:
                IntermediateClothing = people.clothingMethodSched->getCurrentVal();
                if (IntermediateClothing == 1.0) {
                    state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                    comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                } else if (IntermediateClothing == 2.0) {
                    comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
                    comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                    DynamicClothingModel(state);
                    state.dataThermalComforts->CloUnit = comfort.ClothingValue;
                } else {
                    state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                    ShowWarningError(
                        state, format("PEOPLE=\"{}\", Scheduled clothing value will be used rather than clothing calculation method.", people.Name));
                }
                break;
            default:
                ShowSevereError(state, format("PEOPLE=\"{}\", Incorrect Clothing Type", people.Name));
            }

            if (state.dataRoomAir->anyNonMixingRoomAirModel && state.dataRoomAir->IsZoneCrossVent(state.dataThermalComforts->ZoneNum)) {
                if (state.dataRoomAir->ZoneCrossVent(state.dataThermalComforts->ZoneNum).VforComfort == RoomAir::Comfort::Jet) {
                    state.dataThermalComforts->AirVel = state.dataRoomAir->Ujet(state.dataThermalComforts->ZoneNum);
                } else if (state.dataRoomAir->ZoneCrossVent(state.dataThermalComforts->ZoneNum).VforComfort == RoomAir::Comfort::Recirculation) {
                    state.dataThermalComforts->AirVel = state.dataRoomAir->Urec(state.dataThermalComforts->ZoneNum);
                } else {
                    state.dataThermalComforts->AirVel = 0.2;
                }
            } else {
                state.dataThermalComforts->AirVel = people.airVelocitySched->getCurrentVal();
                // Ensure air velocity within the reasonable range. Otherwise reccusive warnings is provided
                if (present(PNum) && (state.dataThermalComforts->AirVel < 0.1 || state.dataThermalComforts->AirVel > 0.5)) {
                    if (people.AirVelErrIndex == 0) {
                        ShowWarningMessage(
                            state,
                            format("PEOPLE=\"{}\", Air velocity is beyond the reasonable range (0.1,0.5) for thermal comfort control.", people.Name));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "PEOPLE=\"" + people.Name + "\",Air velocity is still beyond the reasonable range (0.1,0.5)",
                                                   people.AirVelErrIndex,
                                                   state.dataThermalComforts->AirVel,
                                                   state.dataThermalComforts->AirVel,
                                                   _,
                                                   "[m/s]",
                                                   "[m/s]");
                }
            }

            Real64 PMV = CalcFangerPMV(state,
                                       state.dataThermalComforts->AirTemp,
                                       state.dataThermalComforts->RadTemp,
                                       state.dataThermalComforts->RelHum,
                                       state.dataThermalComforts->AirVel,
                                       state.dataThermalComforts->ActLevel,
                                       state.dataThermalComforts->CloUnit,
                                       state.dataThermalComforts->WorkEff);

            comfort.FangerPMV = PMV;
            // Pass resulting PMV based on temperature setpoint (Tset) when using thermal comfort control
            if (present(PNum)) {
                PMVResult = PMV;
            }
            comfort.ThermalComfortMRT = state.dataThermalComforts->RadTemp;
            comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
            comfort.CloSurfTemp = state.dataThermalComforts->CloSurfTemp;

            // Calculate the Fanger PPD (Predicted Percentage of Dissatisfied), as a %
            Real64 PPD = CalcFangerPPD(PMV);
            comfort.FangerPPD = PPD;
        }
    }

    Real64 CalcFangerPMV(
        EnergyPlusData &state, Real64 AirTemp, Real64 RadTemp, Real64 RelHum, Real64 AirVel, Real64 ActLevel, Real64 CloUnit, Real64 WorkEff)
    {

        // Using/Aliasing
        using Psychrometrics::PsyPsatFnTemp;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(150);             // Limit of iteration
        Real64 constexpr StopIterCrit(0.00015); // Stop criteria for iteration

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 P1;  // Intermediate variables to calculate clothed body ratio and clothing temperature
        Real64 P2;  // Intermediate variables to calculate clothed body ratio and clothing temperature
        Real64 P3;  // Intermediate variables to calculate clothed body ratio and clothing temperature
        Real64 P4;  // Intermediate variables to calculate clothed body ratio and clothing temperature
        Real64 XF;  // Intermediate variables to calculate clothed body ratio and clothing temperature
        Real64 XN;  // Intermediate variables to calculate clothed body ratio and clothing temperature
        Real64 PMV; // temporary variable to store calculated Fanger PMV value
        // VapPress    = CalcSatVapPressFromTemp(AirTemp)  !original
        // VapPress    = RelHum*VapPress                   !original might be in torrs

        // Reference: this subroutine is based on ANSI/ASHRAE Standard 55-2020 and ISO 7730:2005

        state.dataThermalComforts->VapPress = PsyPsatFnTemp(state, AirTemp); // use psych routines inside E+ , returns Pa

        state.dataThermalComforts->VapPress *= RelHum; // in units of [Pa]

        state.dataThermalComforts->IntHeatProd = ActLevel - WorkEff;

        Real64 stdICL = 0.155 * CloUnit;

        // Compute the Corresponding Clothed Body Ratio
        // The ratio of the surface area of the clothed body to the surface area of nude body
        if (stdICL < 0.078) {
            state.dataThermalComforts->CloBodyRat = 1.0 + 1.29 * stdICL;
        } else {
            state.dataThermalComforts->CloBodyRat = 1.05 + 0.645 * stdICL;
        }

        state.dataThermalComforts->AbsRadTemp = RadTemp + TAbsConv;
        state.dataThermalComforts->AbsAirTemp = AirTemp + TAbsConv;

        state.dataThermalComforts->CloInsul = stdICL * state.dataThermalComforts->CloBodyRat; // Thermal resistance of the clothing // icl

        P2 = state.dataThermalComforts->CloInsul * 3.96;
        P3 = state.dataThermalComforts->CloInsul * 100.0;
        P1 = state.dataThermalComforts->CloInsul * state.dataThermalComforts->AbsAirTemp;                                        // p4
        P4 = 308.7 - 0.028 * state.dataThermalComforts->IntHeatProd + P2 * pow_4(state.dataThermalComforts->AbsRadTemp / 100.0); // p5

        // First guess for clothed surface temperature
        state.dataThermalComforts->AbsCloSurfTemp = state.dataThermalComforts->AbsAirTemp + (35.5 - AirTemp) / (3.5 * stdICL + 0.1);
        XN = state.dataThermalComforts->AbsCloSurfTemp / 100.0;
        state.dataThermalComforts->HcFor = 12.1 * std::sqrt(AirVel); // Heat transfer coefficient by forced convection
        state.dataThermalComforts->IterNum = 0;
        XF = XN;

        // COMPUTE SURFACE TEMPERATURE OF CLOTHING BY ITERATIONS
        while (((std::abs(XN - XF) > StopIterCrit) || (state.dataThermalComforts->IterNum == 0)) && (state.dataThermalComforts->IterNum < MaxIter)) {
            XF = (XF + XN) / 2.0;
            state.dataThermalComforts->HcNat =
                2.38 * root_4(std::abs(100.0 * XF - state.dataThermalComforts->AbsAirTemp)); // Heat transfer coefficient by natural convection
            state.dataThermalComforts->Hc =
                max(state.dataThermalComforts->HcFor, state.dataThermalComforts->HcNat); // Determination of convective heat transfer coefficient
            XN = (P4 + P1 * state.dataThermalComforts->Hc - P2 * pow_4(XF)) / (100.0 + P3 * state.dataThermalComforts->Hc);
            ++state.dataThermalComforts->IterNum;
            if (state.dataThermalComforts->IterNum > MaxIter) {
                ShowWarningError(state, "Max iteration exceeded in CalcThermalFanger");
            }
        }
        state.dataThermalComforts->AbsCloSurfTemp = 100.0 * XN;
        state.dataThermalComforts->CloSurfTemp = state.dataThermalComforts->AbsCloSurfTemp - TAbsConv;

        // COMPUTE PREDICTED MEAN VOTE
        // Sensible heat loss
        // RadHeatLoss = RadSurfEff*CloBodyRat*SkinEmiss*StefanBoltz* &   !original
        //                            (AbsCloSurfTemp**4 - AbsRadTemp**4) ! Heat loss by radiation

        // following line is ln 480 in ASHRAE 55 append. D
        state.dataThermalComforts->RadHeatLoss =
            3.96 * state.dataThermalComforts->CloBodyRat *
            (pow_4(state.dataThermalComforts->AbsCloSurfTemp * 0.01) - pow_4(state.dataThermalComforts->AbsRadTemp * 0.01));

        state.dataThermalComforts->ConvHeatLoss = state.dataThermalComforts->CloBodyRat * state.dataThermalComforts->Hc *
                                                  (state.dataThermalComforts->CloSurfTemp - AirTemp); // Heat loss by convection

        state.dataThermalComforts->DryHeatLoss = state.dataThermalComforts->RadHeatLoss + state.dataThermalComforts->ConvHeatLoss;

        // Evaporative heat loss
        // Heat loss by regulatory sweating
        state.dataThermalComforts->EvapHeatLossRegComf = 0.0;
        if (state.dataThermalComforts->IntHeatProd > 58.2) {
            state.dataThermalComforts->EvapHeatLossRegComf = 0.42 * (state.dataThermalComforts->IntHeatProd - ActLevelConv);
        }
        // SkinTempComf = 35.7 - 0.028*IntHeatProd ! Skin temperature required to achieve thermal comfort
        // SatSkinVapPress = 1.92*SkinTempComf - 25.3 ! Water vapor pressure at required skin temperature
        // Heat loss by diffusion
        // EvapHeatLossDiff = 0.4148*(SatSkinVapPress - VapPress) !original
        state.dataThermalComforts->EvapHeatLossDiff =
            3.05 * 0.001 *
            (5733.0 - 6.99 * state.dataThermalComforts->IntHeatProd - state.dataThermalComforts->VapPress); // ln 440 in ASHRAE 55 Append. D

        state.dataThermalComforts->EvapHeatLoss = state.dataThermalComforts->EvapHeatLossRegComf + state.dataThermalComforts->EvapHeatLossDiff;
        // Heat loss by respiration
        // original: LatRespHeatLoss = 0.0023*ActLevel*(44. - VapPress) ! Heat loss by latent respiration
        state.dataThermalComforts->LatRespHeatLoss =
            1.7 * 0.00001 * ActLevel * (5867.0 - state.dataThermalComforts->VapPress); // ln 460 in ASHRAE 55 Append. D

        // LatRespHeatLoss = 0.017251*ActLevel*(5.8662 - VapPress)
        // V-1.2.2 'fix' BG 3/2005 5th term in LHS Eq (58)  in 2001 HOF Ch. 8
        // this was wrong because VapPress needed to be kPa

        state.dataThermalComforts->DryRespHeatLoss = 0.0014 * ActLevel * (34.0 - AirTemp); // Heat loss by dry respiration.

        state.dataThermalComforts->RespHeatLoss = state.dataThermalComforts->LatRespHeatLoss + state.dataThermalComforts->DryRespHeatLoss;

        state.dataThermalComforts->ThermSensTransCoef = 0.303 * std::exp(-0.036 * ActLevel) + 0.028; // Thermal transfer coefficient to calculate PMV

        PMV = state.dataThermalComforts->ThermSensTransCoef * (state.dataThermalComforts->IntHeatProd - state.dataThermalComforts->EvapHeatLoss -
                                                               state.dataThermalComforts->RespHeatLoss - state.dataThermalComforts->DryHeatLoss);

        return PMV;
    }

    Real64 CalcFangerPPD(Real64 PMV)
    {
        Real64 PPD;
        Real64 expTest1 = -0.03353 * pow_4(PMV) - 0.2179 * pow_2(PMV);
        if (expTest1 > DataPrecisionGlobals::EXP_LowerLimit) {
            PPD = 100.0 - 95.0 * std::exp(expTest1);
        } else {
            PPD = 100.0;
        }

        if (PPD < 0.0) {
            PPD = 0.0;
        } else if (PPD > 100.0) {
            PPD = 100.0;
        }
        return PPD;
    }

    Real64 CalcRelativeAirVelocity(Real64 AirVel, Real64 ActMet)
    {
        if (ActMet > 1) {
            return AirVel + 0.3 * (ActMet - 1);
        } else {
            return AirVel;
        }
    }

    void GetThermalComfortInputsASHRAE(EnergyPlusData &state)
    {
        auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
        auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

        state.dataThermalComforts->ZoneNum = people.ZonePtr;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataThermalComforts->ZoneNum);
        // (var TA)
        state.dataThermalComforts->AirTemp = thisZoneHB.ZTAVComf;
        if (state.dataRoomAir->anyNonMixingRoomAirModel) {
            if (state.dataRoomAir->IsZoneDispVent3Node(state.dataThermalComforts->ZoneNum) ||
                state.dataRoomAir->IsZoneUFAD(state.dataThermalComforts->ZoneNum)) {
                state.dataThermalComforts->AirTemp = state.dataRoomAir->TCMF(state.dataThermalComforts->ZoneNum); // PH 3/7/04
            }
        }
        // (var TR)
        state.dataThermalComforts->RadTemp = CalcRadTemp(state, state.dataThermalComforts->PeopleNum);
        // (var RH)
        state.dataThermalComforts->RelHum =
            PsyRhFnTdbWPb(state, state.dataThermalComforts->AirTemp, thisZoneHB.airHumRatAvgComf, state.dataEnvrn->OutBaroPress);
        // Metabolic rate of body (W/m2) (var RM, M)
        state.dataThermalComforts->ActLevel = people.activityLevelSched->getCurrentVal() / BodySurfAreaPierce;
        // Energy consumption by external work (W/m2) (var WME)
        state.dataThermalComforts->WorkEff = people.workEffSched->getCurrentVal() * state.dataThermalComforts->ActLevel;

        // Clothing unit  (var CLO)
        Real64 IntermediateClothing;
        switch (people.clothingType) {
        case DataHeatBalance::ClothingType::InsulationSchedule:
            state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
            comfort.ClothingValue = state.dataThermalComforts->CloUnit;
            break;
        case DataHeatBalance::ClothingType::DynamicAshrae55:
            comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
            comfort.ClothingValue = state.dataThermalComforts->CloUnit;
            DynamicClothingModel(state);
            state.dataThermalComforts->CloUnit = comfort.ClothingValue;
            break;
        case DataHeatBalance::ClothingType::CalculationSchedule:
            IntermediateClothing = people.clothingMethodSched->getCurrentVal();
            if (IntermediateClothing == 1.0) {
                state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                comfort.ClothingValue = state.dataThermalComforts->CloUnit;
            } else if (IntermediateClothing == 2.0) {
                comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
                comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                DynamicClothingModel(state);
                state.dataThermalComforts->CloUnit = comfort.ClothingValue;
            } else {
                state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                ShowWarningError(state, "Scheduled clothing value will be used rather than clothing calculation method.");
            }
            break;
        default:
            ShowSevereError(state, "Incorrect Clothing Type");
        }
        // (var VEL)
        state.dataThermalComforts->AirVel = people.airVelocitySched->getCurrentVal();
        // (var MET)
        state.dataThermalComforts->ActMet = state.dataThermalComforts->ActLevel / ActLevelConv;
    }

    Real64 CalcStandardEffectiveTemp(
        EnergyPlusData &state, Real64 AirTemp, Real64 RadTemp, Real64 RelHum, Real64 AirVel, Real64 ActMet, Real64 CloUnit, Real64 WorkEff)
    {

        // Thermal const
        constexpr Real64 CloFac(0.25);                  // Clothing factor determined experimentally (var KCLO)
        constexpr Real64 BodyWeight(69.9);              // (var BODYWEIGHT)
        constexpr Real64 SweatContConst(170.0);         // Proportionality constant for sweat control; g/m2.hr (var CSW)
        constexpr Real64 DriCoeffVasodilation(120);     // driving coefficient for vasodilation (var CDIL)
        constexpr Real64 DriCoeffVasoconstriction(0.5); // (var CSTR)
        constexpr Real64 MaxSkinBloodFlow(90.0);        // Max. value of skin blood flow
        constexpr Real64 MinSkinBloodFlow(0.5);         // Min. value of skin blood flow
        constexpr Real64 RegSweatMax(500);              // Max. value of regulatory sweating; w/m2

        // Standard condition const
        // Definition of vascular control signals CoreTempSet, SkinTempSet, and AvgBodyTempSet are the setpoints for core, skin and
        // average body temperatures corresponding to physiol.  neutrality SkinMassRatSet is the ratio of skin mass to total body mass (skin+core)
        // Typical values for CoreTempSet, SkinTempSet and SkinMassRatSet are 36.8, 33.7 and 0.10 SkinMassRat is the actual skin to total body mass
        // ratio
        constexpr Real64 SkinTempSet(33.7);     // (var TempSkinNeutral)
        constexpr Real64 CoreTempSet(36.8);     // (var TempCoreNeutral)
        constexpr Real64 SkinBloodFlowSet(6.3); // (var SkinBloodFlowNeutral)
        constexpr Real64 SkinMassRatSet(0.1);   // (var ALFA)

        if (AirVel < 0.1) {
            AirVel = 0.1;
        }

        // (var VaporPressure)
        state.dataThermalComforts->VapPress = RelHum * CalcSatVapPressFromTempTorr(AirTemp);
        Real64 ActLevel = ActLevelConv * ActMet;
        state.dataThermalComforts->IntHeatProd = ActLevel - WorkEff;

        // Step 1: CALCULATE VARIABLESS THAT REMAIN CONSTANT FOR AN HOUR
        Real64 PInAtmospheres = state.dataEnvrn->OutBaroPress / 101325;
        Real64 RClo = CloUnit * 0.155; // (var RCL)
        Real64 TotCloFac = 1.0 + 0.15 * CloUnit;
        Real64 LewisRatio = 2.2 / PInAtmospheres; // Lewis Relation is 2.2 at sea level, 25C (var LR)
        Real64 EvapEff;                           // evaporative efficiency

        // APPROXIMATE THE FOLLOWING VALUES TO START
        state.dataThermalComforts->SkinTemp = SkinTempSet;
        state.dataThermalComforts->CoreTemp = CoreTempSet;
        Real64 SkinBloodFlow = SkinBloodFlowSet;
        Real64 SkinMassRat = SkinMassRatSet;

        // Mass transfer equation between skin and environment
        // CloInsul is efficiency of mass transfer for CloUnit.
        if (CloUnit <= 0) {
            EvapEff = 0.38 * std::pow(AirVel, -0.29); // (var WCRIT)
            state.dataThermalComforts->CloInsul = 1.0;
        } else {
            EvapEff = 0.59 * std::pow(AirVel, -0.08); // (var ICL)
            state.dataThermalComforts->CloInsul = 0.45;
        }

        Real64 CorrectedHC = 3.0 * std::pow(PInAtmospheres, 0.53);              // corrected convective heat transfer coefficient
        Real64 ForcedHC = 8.600001 * std::pow((AirVel * PInAtmospheres), 0.53); // forced convective heat transfer coefficient, W/(m2 °C) (CHCV)
        state.dataThermalComforts->Hc = std::max(CorrectedHC, ForcedHC);        // (CHC)
        state.dataThermalComforts->Hr = 4.7;                                    // (CHR)
        state.dataThermalComforts->EvapHeatLoss = 0.1 * ActMet;
        Real64 RAir = 1.0 / (TotCloFac * (state.dataThermalComforts->Hc + state.dataThermalComforts->Hr)); // resistance of air layer to dry heat (RA)
        state.dataThermalComforts->OpTemp = (state.dataThermalComforts->Hr * RadTemp + state.dataThermalComforts->Hc * AirTemp) /
                                            (state.dataThermalComforts->Hc + state.dataThermalComforts->Hr); // operative temperature (TOP)
        Real64 ActLevelStart = ActLevel; // ActLevel gets increased by shivering in the following
        Real64 AvgBodyTempSet = SkinMassRatSet * SkinTempSet + (1.0 - SkinMassRatSet) * CoreTempSet; // (var TempBodyNeutral)

        // Step 2: BEGIN MINUTE BY MINUTE CALCULATIONS FOR ONE HOUR SIMULATION OF TEMPERATURE REGULATION.
        // This section simulates the temperature regulation over 1 minute.
        // Inputs are the physiological data from the previous time step and the current environmental conditions. Loop and must be increased from the
        // start level, not perpetually increased
        for (int IterMin = 1; IterMin <= 60; ++IterMin) {
            // Dry heat balance: solve for CloSurfTemp and Hr, GUESS CloSurfTemp TO START
            state.dataThermalComforts->CloSurfTemp =
                (RAir * state.dataThermalComforts->SkinTemp + RClo * state.dataThermalComforts->OpTemp) / (RAir + RClo);
            bool converged = false;
            while (!converged) {
                state.dataThermalComforts->Hr =
                    4.0 * StefanBoltz * std::pow((state.dataThermalComforts->CloSurfTemp + RadTemp) / 2.0 + 273.15, 3) * 0.72;
                RAir = 1.0 / (TotCloFac * (state.dataThermalComforts->Hc + state.dataThermalComforts->Hr));
                state.dataThermalComforts->OpTemp = (state.dataThermalComforts->Hr * RadTemp + state.dataThermalComforts->Hc * AirTemp) /
                                                    (state.dataThermalComforts->Hc + state.dataThermalComforts->Hr);
                Real64 CloSurfTempNew = (RAir * state.dataThermalComforts->SkinTemp + RClo * state.dataThermalComforts->OpTemp) / (RAir + RClo);
                if (std::abs(CloSurfTempNew - state.dataThermalComforts->CloSurfTemp) <= 0.01) {
                    converged = true;
                }
                state.dataThermalComforts->CloSurfTemp = CloSurfTempNew;
            }

            // CALCULATE THE COMBINED HEAT TRANSFER COEFF. (H)
            state.dataThermalComforts->H = state.dataThermalComforts->Hr + state.dataThermalComforts->Hc;
            // Heat flow from Clothing surface to environment
            state.dataThermalComforts->DryHeatLoss = (state.dataThermalComforts->SkinTemp - state.dataThermalComforts->OpTemp) / (RAir + RClo);

            // dry and latent respiratory heat losses
            state.dataThermalComforts->LatRespHeatLoss =
                0.0023 * ActLevel * (44.0 - state.dataThermalComforts->VapPress); // latent heat loss due to respiration
            state.dataThermalComforts->DryRespHeatLoss = 0.0014 * ActLevel * (34.0 - AirTemp);

            state.dataThermalComforts->RespHeatLoss = state.dataThermalComforts->LatRespHeatLoss + state.dataThermalComforts->DryRespHeatLoss;

            // Heat flows to skin and core: 5.28 is skin conductance in the absence of skin blood flow
            state.dataThermalComforts->HeatFlow =
                (state.dataThermalComforts->CoreTemp - state.dataThermalComforts->SkinTemp) * (5.28 + 1.163 * SkinBloodFlow);

            Real64 CoreHeatStorage = ActLevel - state.dataThermalComforts->HeatFlow - state.dataThermalComforts->RespHeatLoss -
                                     WorkEff; // rate of energy storage in the core
            Real64 SkinHeatStorage = state.dataThermalComforts->HeatFlow - state.dataThermalComforts->DryHeatLoss -
                                     state.dataThermalComforts->EvapHeatLoss; // rate of energy storage in the skin

            // Thermal capacities
            state.dataThermalComforts->CoreThermCap = 0.97 * (1 - SkinMassRat) * BodyWeight;
            state.dataThermalComforts->SkinThermCap = 0.97 * SkinMassRat * BodyWeight;

            // Temperature changes in 1 minute
            state.dataThermalComforts->CoreTempChange = (CoreHeatStorage * BodySurfAreaPierce / (state.dataThermalComforts->CoreThermCap * 60.0));
            state.dataThermalComforts->SkinTempChange = (SkinHeatStorage * BodySurfAreaPierce) / (state.dataThermalComforts->SkinThermCap * 60.0);

            state.dataThermalComforts->CoreTemp += state.dataThermalComforts->CoreTempChange;
            state.dataThermalComforts->SkinTemp += state.dataThermalComforts->SkinTempChange;
            state.dataThermalComforts->AvgBodyTemp =
                SkinMassRat * state.dataThermalComforts->SkinTemp + (1.0 - SkinMassRat) * state.dataThermalComforts->CoreTemp;

            Real64 SkinThermSigWarm;                                               // vasodilation signal (WARMS)
            Real64 SkinThermSigCold;                                               // vasoconstriction signal
            Real64 SkinSignal = state.dataThermalComforts->SkinTemp - SkinTempSet; // thermoregulatory control signal from the skin
            if (SkinSignal > 0) {
                SkinThermSigWarm = SkinSignal;
                SkinThermSigCold = 0.0;
            } else {
                SkinThermSigCold = -SkinSignal;
                SkinThermSigWarm = 0.0;
            }

            Real64 CoreThermSigWarm;                                               // vasodialtion signal (WARMC)
            Real64 CoreThermSigCold;                                               // vasoconstriction signal
            Real64 CoreSignal = state.dataThermalComforts->CoreTemp - CoreTempSet; // thermoregulatory control signal from the skin, °C
            if (CoreSignal > 0) {
                CoreThermSigWarm = CoreSignal;
                CoreThermSigCold = 0.0;
            } else {
                CoreThermSigCold = -CoreSignal;
                CoreThermSigWarm = 0.0;
            }

            Real64 BodyThermSigWarm; // WARMB
            Real64 BodySignal = state.dataThermalComforts->AvgBodyTemp - AvgBodyTempSet;

            if (BodySignal > 0) {
                BodyThermSigWarm = BodySignal;
            } else {
                BodyThermSigWarm = 0.0;
            }

            state.dataThermalComforts->VasodilationFac = DriCoeffVasodilation * CoreThermSigWarm;
            state.dataThermalComforts->VasoconstrictFac = DriCoeffVasoconstriction * SkinThermSigCold;
            SkinBloodFlow = (SkinBloodFlowSet + state.dataThermalComforts->VasodilationFac) / (1.0 + state.dataThermalComforts->VasoconstrictFac);
            // SkinBloodFlow is never below 0.5 liter/(m2.hr) nor above 90 liter/(m2.hr)
            if (SkinBloodFlow < MinSkinBloodFlow) {
                SkinBloodFlow = MinSkinBloodFlow;
            }
            if (SkinBloodFlow > MaxSkinBloodFlow) {
                SkinBloodFlow = MaxSkinBloodFlow;
            }
            SkinMassRat = 0.0417737 + 0.7451832 / (SkinBloodFlow + 0.585417); // ratio of skin-core masses change with SkinBloodFlow

            Real64 RegSweat = SweatContConst * BodyThermSigWarm * std::exp(SkinThermSigWarm / 10.7); // control of regulatory sweating
            if (RegSweat > RegSweatMax) {
                RegSweat = RegSweatMax;
            }
            state.dataThermalComforts->EvapHeatLossRegSweat = 0.68 * RegSweat; // heat lost by vaporization sweat

            // adjustment of metabolic heat due to shivering (Stolwijk, Hardy)
            state.dataThermalComforts->ShivResponse = 19.4 * SkinThermSigCold * CoreThermSigCold;
            ActLevel = ActLevelStart + state.dataThermalComforts->ShivResponse;

            // Evaluation of heat transfer by evaporation at skin surface
            Real64 AirEvapHeatResist = 1.0 / (LewisRatio * TotCloFac * state.dataThermalComforts->Hc); // evaporative resistance air layer
            Real64 CloEvapHeatResist = RClo / (LewisRatio * state.dataThermalComforts->CloInsul);
            Real64 TotEvapHeatResist = AirEvapHeatResist + CloEvapHeatResist;
            state.dataThermalComforts->SatSkinVapPress = CalcSatVapPressFromTempTorr(state.dataThermalComforts->SkinTemp); // PSSK
            state.dataThermalComforts->EvapHeatLossMax =
                (state.dataThermalComforts->SatSkinVapPress - state.dataThermalComforts->VapPress) / TotEvapHeatResist; // TotEvapHeatResist;
            state.dataThermalComforts->SkinWetSweat =
                state.dataThermalComforts->EvapHeatLossRegSweat /
                state.dataThermalComforts->EvapHeatLossMax; // ratio heat loss sweating to max heat loss sweating

            state.dataThermalComforts->SkinWetDiff =
                (1.0 - state.dataThermalComforts->SkinWetSweat) * 0.06; // 0.06 if SkinWetDiff for nonsweating skin --- Kerslake
            state.dataThermalComforts->EvapHeatLossDiff = state.dataThermalComforts->SkinWetDiff * state.dataThermalComforts->EvapHeatLossMax;
            state.dataThermalComforts->EvapHeatLoss = state.dataThermalComforts->EvapHeatLossRegSweat + state.dataThermalComforts->EvapHeatLossDiff;
            state.dataThermalComforts->SkinWetTot = state.dataThermalComforts->EvapHeatLoss / state.dataThermalComforts->EvapHeatLossMax;

            // Beginning of dripping (Sweat not evaporated on skin surface)
            if (state.dataThermalComforts->SkinWetTot >= EvapEff) {
                state.dataThermalComforts->SkinWetTot = EvapEff;
                state.dataThermalComforts->SkinWetSweat = EvapEff / 0.94;
                state.dataThermalComforts->EvapHeatLossRegSweat =
                    state.dataThermalComforts->SkinWetSweat * state.dataThermalComforts->EvapHeatLossMax;
                state.dataThermalComforts->SkinWetDiff = (1.0 - state.dataThermalComforts->SkinWetSweat) * 0.06;
                state.dataThermalComforts->EvapHeatLossDiff = state.dataThermalComforts->SkinWetDiff * state.dataThermalComforts->EvapHeatLossMax;
                state.dataThermalComforts->EvapHeatLoss =
                    state.dataThermalComforts->EvapHeatLossRegSweat + state.dataThermalComforts->EvapHeatLossDiff;
            }

            // When EvapHeatLossMax<0. condensation on skin occurs.
            if (state.dataThermalComforts->EvapHeatLossMax < 0.0) {
                state.dataThermalComforts->SkinWetDiff = 0.0;
                state.dataThermalComforts->EvapHeatLossDiff = 0.0;
                state.dataThermalComforts->EvapHeatLoss = 0.0;
                state.dataThermalComforts->SkinWetTot = EvapEff;
                state.dataThermalComforts->SkinWetSweat = EvapEff;
                state.dataThermalComforts->EvapHeatLossRegSweat = 0.0;
            }
            // Vapor pressure at skin (as measured by dewpoint sensors)
            state.dataThermalComforts->SkinVapPress = state.dataThermalComforts->SkinWetTot * state.dataThermalComforts->SatSkinVapPress +
                                                      (1.0 - state.dataThermalComforts->SkinWetTot) * state.dataThermalComforts->VapPress;
        } // END OF MINUTE BY MINUTE TEMPERATURE REGULATION LOOP

        // EvapHeatLossMax is readjusted for EvapEff
        state.dataThermalComforts->EvapHeatLossMax *= EvapEff;

        // Step 3: Heat transfer indices in real environment. Computation of comfort indices.
        // Inputs to this SECTION are the physiological data from the simulation of temperature regulation loop.
        Real64 EffectSkinHeatLoss = state.dataThermalComforts->DryHeatLoss + state.dataThermalComforts->EvapHeatLoss;
        // ET*(standardization humidity/REAL(r64) CloUnit, StdAtm and Hc)
        state.dataThermalComforts->CloBodyRat = 1.0 + CloFac * CloUnit;
        Real64 EffectCloUnit =
            CloUnit - (state.dataThermalComforts->CloBodyRat - 1.0) / (0.155 * state.dataThermalComforts->CloBodyRat * state.dataThermalComforts->H);
        Real64 EffectCloThermEff = 1.0 / (1.0 + 0.155 * state.dataThermalComforts->Hc * EffectCloUnit);
        state.dataThermalComforts->CloPermeatEff =
            1.0 / (1.0 + (0.155 / state.dataThermalComforts->CloInsul) * state.dataThermalComforts->Hc * EffectCloUnit);
        // Get a low approximation for ET* and solve balance equation by iteration
        Real64 ET = state.dataThermalComforts->SkinTemp - EffectSkinHeatLoss / (state.dataThermalComforts->H * EffectCloThermEff);
        Real64 EnergyBalErrET;
        while (true) {
            Real64 StdVapPressET = CalcSatVapPressFromTempTorr(ET); // THE STANDARD VAPOR PRESSURE AT THE EFFECTIVE TEMP : StdVapPressET
            EnergyBalErrET = EffectSkinHeatLoss - state.dataThermalComforts->H * EffectCloThermEff * (state.dataThermalComforts->SkinTemp - ET) -
                             state.dataThermalComforts->SkinWetTot * LewisRatio * state.dataThermalComforts->Hc *
                                 state.dataThermalComforts->CloPermeatEff * (state.dataThermalComforts->SatSkinVapPress - StdVapPressET / 2.0);
            if (EnergyBalErrET >= 0.0) {
                break;
            }
            ET += 0.1;
        }
        state.dataThermalComforts->EffTemp = ET;

        // Standard effective temperature SET* standardized humidity.  Hc, CloUnit, StdAtm normalized for given ActLel AirVel
        // Standard environment
        Real64 StdHr = state.dataThermalComforts->Hr;
        Real64 StdHc; // standard conv. heat tr. coeff. (level walking/still air)
        if (ActMet <= 0.85) {
            StdHc = 3.0; // minimum value of Hc at sea leAirVel = 3.0 (AirVel = .137 m/s)
        } else {
            StdHc = 5.66 * std::pow(ActMet - 0.85, 0.39);
        }
        if (StdHc <= 3.0) {
            StdHc = 3.0;
        }
        Real64 StdH = StdHc + StdHr; // StdH Standard combined heat transfer coefficient
        // standard MET - StdCloUnit relation gives SET* = 24 C when PMV = 0
        Real64 StdCloUnit = 1.52 / (ActMet - WorkEff / ActLevelConv + 0.6944) - 0.1835;        // RCLOS
        Real64 StdRClo = 0.155 * StdCloUnit;                                                   // RCLS
        Real64 StdCloBodyRat = 1.0 + CloFac * StdCloUnit;                                      // FACLS
        Real64 StdEffectCloThermEff = 1.0 / (1.0 + 0.155 * StdCloBodyRat * StdH * StdCloUnit); // FCLS
        Real64 StdCloInsul = state.dataThermalComforts->CloInsul * StdHc / StdH * (1 - StdEffectCloThermEff) /
                             (StdHc / StdH - state.dataThermalComforts->CloInsul * StdEffectCloThermEff);
        Real64 StdREvap = 1.0 / (LewisRatio * StdCloBodyRat * StdHc);
        Real64 StdREvapClo = StdRClo / (LewisRatio * StdCloInsul);
        Real64 StdHEvap = 1.0 / (StdREvap + StdREvapClo);
        Real64 StdRAir = 1.0 / (StdCloBodyRat * StdH);
        Real64 StdHDry = 1.0 / (StdRAir + StdRClo);

        // Get a low approximation for SET* and solve balance equ. by iteration
        Real64 StdEffectSkinHeatLoss = state.dataThermalComforts->DryHeatLoss + state.dataThermalComforts->EvapHeatLoss;
        Real64 OldSET = round((state.dataThermalComforts->SkinTemp - StdEffectSkinHeatLoss / StdHDry) * 100) / 100;
        Real64 delta = 0.0001;
        Real64 err = 100.0;
        while (std::abs(err) > 0.01) {
            Real64 StdVapPressSET_1 = CalcSatVapPressFromTempTorr(OldSET); // StdVapPressSET *= VapPressConv;
            Real64 EnergyBalErrSET_1 =
                StdEffectSkinHeatLoss - StdHDry * (state.dataThermalComforts->SkinTemp - OldSET) -
                state.dataThermalComforts->SkinWetTot * StdHEvap * (state.dataThermalComforts->SatSkinVapPress - StdVapPressSET_1 / 2.0);
            Real64 StdVapPressSET_2 = CalcSatVapPressFromTempTorr(OldSET + delta);
            Real64 EnergyBalErrSET_2 =
                StdEffectSkinHeatLoss - StdHDry * (state.dataThermalComforts->SkinTemp - (OldSET + delta)) -
                state.dataThermalComforts->SkinWetTot * StdHEvap * (state.dataThermalComforts->SatSkinVapPress - StdVapPressSET_2 / 2.0);
            Real64 NewSET = OldSET - delta * EnergyBalErrSET_1 / (EnergyBalErrSET_2 - EnergyBalErrSET_1);
            err = NewSET - OldSET;
            OldSET = NewSET;
        }
        Real64 SET = OldSET;
        // PMV*(PMVET in prgm) uses ET instead of OpTemp
        state.dataThermalComforts->DryHeatLossET = StdH * StdEffectCloThermEff * (state.dataThermalComforts->SkinTemp - ET);
        // SPMV*(PMVSET in prgm) uses SET instead of OpTemp
        state.dataThermalComforts->DryHeatLossSET = StdH * StdEffectCloThermEff * (state.dataThermalComforts->SkinTemp - SET);
        return SET;
    }

    void CalcThermalComfortPierceASHRAE(EnergyPlusData &state)
    {
        // This subroutine calculates ET, SET, SETPMV, SETPPD using Pierce two-node model.
        // Reference: ANSI/ASHRAE Standard 55-2017 Appendix D.

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) {
            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            if (!people.Pierce) {
                continue;
            }

            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

            // STEP 1: Get input (TA, TR, RH, VEL, CLO, MET, WME)
            GetThermalComfortInputsASHRAE(state);

            // STEP 2: Calculate SET.
            Real64 SET = CalcStandardEffectiveTemp(state,
                                                   state.dataThermalComforts->AirTemp,
                                                   state.dataThermalComforts->RadTemp,
                                                   state.dataThermalComforts->RelHum,
                                                   state.dataThermalComforts->AirVel,
                                                   state.dataThermalComforts->ActMet,
                                                   state.dataThermalComforts->CloUnit,
                                                   state.dataThermalComforts->WorkEff);

            // STEP 3: Report SET related variables.
            // Fanger's comfort equation. Thermal transfer coefficient to calculate PMV
            state.dataThermalComforts->ThermSensTransCoef = 0.303 * std::exp(-0.036 * state.dataThermalComforts->ActLevel) + 0.028;
            // Fanger's reg. sweating at comfort threshold (PMV=0) is:
            state.dataThermalComforts->EvapHeatLossRegComf = (state.dataThermalComforts->IntHeatProd - ActLevelConv) * 0.42;
            comfort.PiercePMVET =
                state.dataThermalComforts->ThermSensTransCoef *
                (state.dataThermalComforts->IntHeatProd - state.dataThermalComforts->RespHeatLoss - state.dataThermalComforts->DryHeatLossET -
                 state.dataThermalComforts->EvapHeatLossDiff - state.dataThermalComforts->EvapHeatLossRegComf);
            comfort.PiercePMVSET =
                state.dataThermalComforts->ThermSensTransCoef *
                (state.dataThermalComforts->IntHeatProd - state.dataThermalComforts->RespHeatLoss - state.dataThermalComforts->DryHeatLossSET -
                 state.dataThermalComforts->EvapHeatLossDiff - state.dataThermalComforts->EvapHeatLossRegComf);

            // PHeat stress and heat strain indices derived from EvapHeatLoss, DISC (discomfort) varies with relative thermoregulatory strain
            comfort.PierceDISC = 5.0 * (state.dataThermalComforts->EvapHeatLossRegSweat - state.dataThermalComforts->EvapHeatLossRegComf) /
                                 (state.dataThermalComforts->EvapHeatLossMax - state.dataThermalComforts->EvapHeatLossRegComf -
                                  state.dataThermalComforts->EvapHeatLossDiff);

            // Thermal sensation TSENS as function of mean body temp.-
            // AvgBodyTempLow is AvgBodyTemp when DISC is 0. (lower limit of zone of evap. regul.)
            Real64 AvgBodyTempLow = (0.185 / ActLevelConv) * (state.dataThermalComforts->ActLevel - state.dataThermalComforts->WorkEff) + 36.313;
            // AvgBodyTempHigh is AvgBodyTemp when HSI=100 (upper limit of zone of evap. regul.)
            Real64 AvgBodyTempHigh = (0.359 / ActLevelConv) * (state.dataThermalComforts->ActLevel - state.dataThermalComforts->WorkEff) + 36.664;

            // TSENS=DISC=4.7 when HSI =1 00 (HSI is Belding's classic heat stress index)
            // In cold, DISC &TSENS are the same and neg. fct of AvgBodyTemp
            if (state.dataThermalComforts->AvgBodyTemp > AvgBodyTempLow) {
                comfort.PierceTSENS = 4.7 * (state.dataThermalComforts->AvgBodyTemp - AvgBodyTempLow) / (AvgBodyTempHigh - AvgBodyTempLow);

            } else {
                comfort.PierceTSENS = 0.68175 * (state.dataThermalComforts->AvgBodyTemp - AvgBodyTempLow);
                comfort.PierceDISC = comfort.PierceTSENS;
            }

            comfort.ThermalComfortMRT = state.dataThermalComforts->RadTemp;
            comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
            comfort.ClothingValue = state.dataThermalComforts->CloUnit;
            comfort.PierceSET = SET;
        }
    }

    void CalcThermalComfortCoolingEffectASH(EnergyPlusData &state)
    {
        // This subroutine calculates ASHRAE Cooling effect adjusted PMV and PPD
        // Reference: ANSI/ASHRAE Standard 55-2017 Appendix D.

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) {
            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            if (!people.CoolingEffectASH55) {
                continue;
            }

            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

            // Get input (TA, TR, RH, VEL, CLO, MET, WME)
            GetThermalComfortInputsASHRAE(state);

            // Calculate elevated air cooling effect using the SET function.
            Real64 CoolingEffect = 0;
            Real64 CoolingEffectAdjustedPMV;
            CalcCoolingEffectAdjustedPMV(state, CoolingEffect, CoolingEffectAdjustedPMV);

            // Report.
            comfort.CoolingEffectASH55 = CoolingEffect;
            comfort.CoolingEffectAdjustedPMVASH55 = CoolingEffectAdjustedPMV;
            comfort.CoolingEffectAdjustedPPDASH55 = CalcFangerPPD(CoolingEffectAdjustedPMV);
        }
    }

    void CalcCoolingEffectAdjustedPMV(EnergyPlusData &state, Real64 &CoolingEffect, Real64 &CoolingEffectAdjustedPMV)
    {
        auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);

        // Calculate SET without cooling effect.
        Real64 RelAirVel = CalcRelativeAirVelocity(state.dataThermalComforts->AirVel, state.dataThermalComforts->ActMet);
        Real64 SET = CalcStandardEffectiveTemp(state,
                                               state.dataThermalComforts->AirTemp,
                                               state.dataThermalComforts->RadTemp,
                                               state.dataThermalComforts->RelHum,
                                               RelAirVel,
                                               state.dataThermalComforts->ActMet,
                                               state.dataThermalComforts->CloUnit,
                                               state.dataThermalComforts->WorkEff);

        Real64 ASHRAE55PMV = CalcFangerPMV(state,
                                           state.dataThermalComforts->AirTemp,
                                           state.dataThermalComforts->RadTemp,
                                           state.dataThermalComforts->RelHum,
                                           RelAirVel,
                                           state.dataThermalComforts->ActLevel,
                                           state.dataThermalComforts->CloUnit,
                                           state.dataThermalComforts->WorkEff);

        Real64 StillAirVel = 0.1;
        auto ce_root_function = [&state, &StillAirVel, &SET](Real64 x) {
            return CalcStandardEffectiveTemp(state,
                                             state.dataThermalComforts->AirTemp - x,
                                             state.dataThermalComforts->RadTemp - x,
                                             state.dataThermalComforts->RelHum,
                                             StillAirVel,
                                             state.dataThermalComforts->ActMet,
                                             state.dataThermalComforts->CloUnit,
                                             state.dataThermalComforts->WorkEff) -
                   SET;
        };

        auto ce_root_termination = [](Real64 min, Real64 max) { return abs(max - min) <= 0.01; };
        Real64 lowerBound = 0.0;
        Real64 upperBound = 50.0;

        // We have yet another solver?
        try {
            std::pair<Real64, Real64> solverResult = boost::math::tools::bisect(ce_root_function, lowerBound, upperBound, ce_root_termination);
            CoolingEffect = (solverResult.first + solverResult.second) / 2;
        } catch (const std::exception &e) {
            ShowRecurringWarningErrorAtEnd(state,
                                           "The cooling effect could not be solved for People=\"" + people.Name + "\"" +
                                               "As a result, no cooling effect will be applied to adjust the PMV and PPD results.",
                                           state.dataThermalComforts->CoolingEffectWarningInd);
            CoolingEffect = 0;
        }

        if (CoolingEffect > 0) {
            CoolingEffectAdjustedPMV = CalcFangerPMV(state,
                                                     state.dataThermalComforts->AirTemp - CoolingEffect,
                                                     state.dataThermalComforts->RadTemp - CoolingEffect,
                                                     state.dataThermalComforts->RelHum,
                                                     StillAirVel,
                                                     state.dataThermalComforts->ActLevel,
                                                     state.dataThermalComforts->CloUnit,
                                                     state.dataThermalComforts->WorkEff);
        } else {
            CoolingEffectAdjustedPMV = ASHRAE55PMV;
        }
    }

    void CalcThermalComfortAnkleDraftASH(EnergyPlusData &state)
    {
        // This subroutine calculates ASHRAE Ankle draft PPD
        // Reference: ANSI/ASHRAE Standard 55-2017 Appendix I.

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) {

            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            if (!people.AnkleDraftASH55) {
                continue;
            }

            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

            GetThermalComfortInputsASHRAE(state);
            Real64 RelAirVel = CalcRelativeAirVelocity(state.dataThermalComforts->AirVel, state.dataThermalComforts->ActMet);
            Real64 PPD_AD = -1.0;
            if (state.dataThermalComforts->ActMet < 1.3 && state.dataThermalComforts->CloUnit < 0.7 && RelAirVel < 0.2) {
                Real64 AnkleAirVel = people.ankleAirVelocitySched->getCurrentVal();
                Real64 PMV = CalcFangerPMV(state,
                                           state.dataThermalComforts->AirTemp,
                                           state.dataThermalComforts->RadTemp,
                                           state.dataThermalComforts->RelHum,
                                           RelAirVel,
                                           state.dataThermalComforts->ActLevel,
                                           state.dataThermalComforts->CloUnit,
                                           state.dataThermalComforts->WorkEff);
                PPD_AD = (std::exp(-2.58 + 3.05 * AnkleAirVel - 1.06 * PMV) / (1 + std::exp(-2.58 + 3.05 * AnkleAirVel - 1.06 * PMV))) * 100.0;

            } else {
                if (state.dataGlobal->DisplayExtraWarnings) {
                    if (RelAirVel >= 0.2) {
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            "Relative air velocity is above 0.2 m/s in Ankle draft PPD calculations. PPD at ankle draft will be set to -1.0.",
                            state.dataThermalComforts->AnkleDraftAirVelWarningInd,
                            RelAirVel,
                            RelAirVel,
                            _,
                            "[m/s]",
                            "[m/s]");
                    }
                    if (state.dataThermalComforts->ActMet >= 1.3) {
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            "Metabolic rate is above 1.3 met in Ankle draft PPD calculations. PPD at ankle draft will be set to -1.0.",
                            state.dataThermalComforts->AnkleDraftActMetWarningInd,
                            state.dataThermalComforts->ActMet,
                            state.dataThermalComforts->ActMet,
                            _,
                            "[m/s]",
                            "[m/s]");
                    }
                    if (state.dataThermalComforts->CloUnit >= 0.7) {
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            "Clothing unit is above 0.7 in Ankle draft PPD calculations. PPD at ankle draft will be set to -1.0.",
                            state.dataThermalComforts->AnkleDraftCloUnitWarningInd,
                            state.dataThermalComforts->CloUnit,
                            state.dataThermalComforts->CloUnit,
                            _,
                            "[m/s]",
                            "[m/s]");
                    }
                }
            }
            comfort.AnkleDraftPPDASH55 = PPD_AD;
        }
    }

    void CalcThermalComfortKSU(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   January 2000
        //     MODIFIED       Rick Strand (for E+ implementation February 2000)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates TSV using the KSU 2 Node model.

        // METHODOLOGY EMPLOYED:
        // This subroutine is based heavily upon the work performed by Dan Maloney for
        // the BLAST program.  Many of the equations are based on the original Pierce
        // development.  See documentation for further details and references.

        // REFERENCES:
        // Maloney, Dan, M.S. Thesis, University of Illinois at Urbana-Champaign

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr CloEmiss(0.8); // Clothing Emissivity

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 BodyWt;            // Weight of body, kg
        Real64 DayNum;            // Number of days of acclimation
        int NumDay;               // Loop counter for DayNum
        Real64 EmissAvg;          // Average emissivity
        int IncreDayNum;          // Number of days of increment in the outputs as desired
        Real64 IntHeatProdMet;    // Internal heat production in MET
        Real64 IntHeatProdMetMax; // Maximum value of internal heat production in MET
        int LastDayNum;           // Number of days for the last print out
        Real64 SkinWetFac;        // Skin wettedness factor
        Real64 SkinWetNeut;       // Skin wettedness at neutral state
        int StartDayNum;          // Number of days for the first print out
        // Unacclimated man = 1, Acclimated man = 14
        Real64 SweatSuppFac; // Sweat suppression factor due to skin wettedness
        Real64 TempDiffer;   // Temperature difference between the rectal and esophageal temperatures
        // If not measured, set it to be 0.5 Deg. C.
        int TempIndiceNum;     // Number of temperature indices
        Real64 ThermCndctMin;  // Minimum value of thermal conductance
        Real64 ThermCndctNeut; // Thermal conductance at neutral state
        Real64 TimeExpos;      // Time period in the exposure, hr
        Real64 TimeInterval;   // Time interval of outputs desired, hr
        Real64 TSVMax;         // Maximum value of thermal sensation vote
        Real64 IntermediateClothing;

        TempIndiceNum = 2;

        // NEXT GROUP OF VARIABLE ARE FIXED FOR BLAST PROGRAM - UNACCLIMATED MAN
        // THE TSV MODEL CAN BE APPLIED TO UNACCLIMATED MAN ONLY.
        TimeInterval = 1.0;
        TSVMax = 4.0;
        StartDayNum = 1;
        LastDayNum = 1;
        IncreDayNum = 1;
        TimeExpos = 1.0;
        TempDiffer = 0.5;

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) {
            // THE NEXT SIX VARIABLES WILL BE READ IN FROM INPUT DECK
            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            if (!people.KSU) {
                continue;
            }

            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

            state.dataThermalComforts->ZoneNum = people.ZonePtr;
            auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataThermalComforts->ZoneNum);

            state.dataThermalComforts->AirTemp = thisZoneHB.ZTAVComf;
            if (state.dataRoomAir->anyNonMixingRoomAirModel) {
                if (state.dataRoomAir->IsZoneDispVent3Node(state.dataThermalComforts->ZoneNum) ||
                    state.dataRoomAir->IsZoneUFAD(state.dataThermalComforts->ZoneNum)) {
                    state.dataThermalComforts->AirTemp = state.dataRoomAir->TCMF(state.dataThermalComforts->ZoneNum); // PH 3/7/04
                }
            }
            state.dataThermalComforts->RadTemp = CalcRadTemp(state, state.dataThermalComforts->PeopleNum);
            state.dataThermalComforts->RelHum =
                PsyRhFnTdbWPb(state, state.dataThermalComforts->AirTemp, thisZoneHB.airHumRatAvgComf, state.dataEnvrn->OutBaroPress);
            state.dataThermalComforts->ActLevel = people.activityLevelSched->getCurrentVal() / BodySurfArea;
            state.dataThermalComforts->WorkEff = people.workEffSched->getCurrentVal() * state.dataThermalComforts->ActLevel;

            switch (people.clothingType) {
            case DataHeatBalance::ClothingType::InsulationSchedule: {
                state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                comfort.ClothingValue = state.dataThermalComforts->CloUnit;
            } break;
            case DataHeatBalance::ClothingType::DynamicAshrae55: {
                comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
                comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                DynamicClothingModel(state);
                state.dataThermalComforts->CloUnit = comfort.ClothingValue;
            } break;
            case DataHeatBalance::ClothingType::CalculationSchedule: {
                IntermediateClothing = people.clothingMethodSched->getCurrentVal();
                if (IntermediateClothing == 1.0) {
                    state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                    comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                } else if (IntermediateClothing == 2.0) {
                    comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;
                    comfort.ClothingValue = state.dataThermalComforts->CloUnit;
                    DynamicClothingModel(state);
                    state.dataThermalComforts->CloUnit = comfort.ClothingValue;
                } else {
                    state.dataThermalComforts->CloUnit = people.clothingSched->getCurrentVal();
                    ShowWarningError(
                        state, format("PEOPLE=\"{}\", Scheduled clothing value will be used rather than clothing calculation method.", people.Name));
                }
            } break;
            default:
                ShowSevereError(state, format("PEOPLE=\"{}\", Incorrect Clothing Type", people.Name));
            }

            state.dataThermalComforts->AirVel = people.airVelocitySched->getCurrentVal();
            state.dataThermalComforts->IntHeatProd = state.dataThermalComforts->ActLevel - state.dataThermalComforts->WorkEff;
            // THE FOLLOWING ARE TYPICAL VALUES SET FOR BLAST RUNS
            // STANDARD MAN: 70. KG WEIGHT, 1.8 M2 SURFACE AREA
            BodyWt = 70.0;
            state.dataThermalComforts->CoreTemp = 37.0;
            state.dataThermalComforts->SkinTemp = 31.0;

            //   CALCULATIONS NEEDED FOR THE PASSIVE STATE EQUATIONS
            state.dataThermalComforts->CoreThermCap = 0.9 * BodyWt * 0.97 / BodySurfArea;
            state.dataThermalComforts->SkinThermCap = 0.1 * BodyWt * 0.97 / BodySurfArea;
            //   KERSLAKE'S FORMULA (0.05<AirVel<5. M/S)
            if (state.dataThermalComforts->AirVel < 0.137) {
                state.dataThermalComforts->AirVel = 0.137;
            }
            state.dataThermalComforts->Hc = 8.3 * std::sqrt(state.dataThermalComforts->AirVel);
            EmissAvg = RadSurfEff * CloEmiss + (1.0 - RadSurfEff) * 1.0;
            //   IBERALL EQUATION
            state.dataThermalComforts->Hr = EmissAvg * (3.87 + 0.031 * state.dataThermalComforts->RadTemp);
            state.dataThermalComforts->H = state.dataThermalComforts->Hr + state.dataThermalComforts->Hc;
            state.dataThermalComforts->OpTemp = (state.dataThermalComforts->Hc * state.dataThermalComforts->AirTemp +
                                                 state.dataThermalComforts->Hr * state.dataThermalComforts->RadTemp) /
                                                state.dataThermalComforts->H;
            state.dataThermalComforts->VapPress = CalcSatVapPressFromTemp(state.dataThermalComforts->AirTemp);
            state.dataThermalComforts->VapPress *= state.dataThermalComforts->RelHum;
            state.dataThermalComforts->CloBodyRat = 1.0 + 0.2 * state.dataThermalComforts->CloUnit;
            state.dataThermalComforts->CloThermEff =
                1.0 / (1.0 + 0.155 * state.dataThermalComforts->H * state.dataThermalComforts->CloBodyRat * state.dataThermalComforts->CloUnit);
            state.dataThermalComforts->CloPermeatEff = 1.0 / (1.0 + 0.143 * state.dataThermalComforts->Hc * state.dataThermalComforts->CloUnit);
            //  BASIC INFORMATION FOR THERMAL SENSATION.
            IntHeatProdMet = state.dataThermalComforts->IntHeatProd / ActLevelConv;
            IntHeatProdMetMax = max(1.0, IntHeatProdMet);
            ThermCndctNeut = 12.05 * std::exp(0.2266 * (IntHeatProdMetMax - 1.0));
            SkinWetNeut = 0.02 + 0.4 * (1.0 - std::exp(-0.6 * (IntHeatProdMetMax - 1.0)));
            ThermCndctMin = (ThermCndctNeut - 5.3) * 0.26074074 + 5.3;
            Real64 const ThemCndct_75_fac(1.0 / (75.0 - ThermCndctNeut));
            Real64 const ThemCndct_fac(1.0 / (ThermCndctNeut - ThermCndctMin));
            //  CALCULATE THE PHYSIOLOGICAL REACTIONS OF AN UNACCLIMATED
            //  MAN (LastDayNum = 1), OR AN ACCLIMATED MAN (LastDayNum = 14, IncreDayNum = 13),
            assert(IncreDayNum > 0); // Autodesk:F2C++ Loop setup assumption
            for (NumDay = StartDayNum; NumDay <= LastDayNum; NumDay += IncreDayNum) {
                //  INITIAL CONDITIONS IN AN EXPOSURE
                DayNum = double(NumDay);
                state.dataThermalComforts->Time = 0.0;
                state.dataThermalComforts->TimeChange = 0.01;
                SweatSuppFac = 1.0;
                state.dataThermalComforts->Temp(1) = state.dataThermalComforts->CoreTemp;
                state.dataThermalComforts->Temp(2) = state.dataThermalComforts->SkinTemp;
                state.dataThermalComforts->Coeff(1) = state.dataThermalComforts->Coeff(2) = 0.0;
                //  PHYSIOLOGICAL ADJUSTMENTS IN HEAT ACCLIMATION.
                state.dataThermalComforts->AcclPattern = 1.0 - std::exp(-0.12 * (DayNum - 1.0));
                state.dataThermalComforts->CoreTempNeut = 36.9 - 0.6 * state.dataThermalComforts->AcclPattern;
                state.dataThermalComforts->SkinTempNeut = 33.8 - 1.6 * state.dataThermalComforts->AcclPattern;
                state.dataThermalComforts->ActLevel -= 0.07 * state.dataThermalComforts->ActLevel * state.dataThermalComforts->AcclPattern;
                Real64 const SkinTempNeut_fac(1.0 / (1.0 - SkinWetNeut));
                //  CALCULATION OF CoreTempChange/TempChange & SkinTempChange/TempChange
                DERIV(state, TempIndiceNum, state.dataThermalComforts->Temp, state.dataThermalComforts->TempChange);
                while (true) {
                    //  CALCULATION OF THERMAL SENSATION VOTE (TSV).
                    //  THE TSV MODEL CAN BE APPLIED TO UNACCLIMATED MAN ONLY.
                    SkinWetFac = (state.dataThermalComforts->SkinWetSweat - SkinWetNeut) * SkinTempNeut_fac;
                    state.dataThermalComforts->VasodilationFac = (state.dataThermalComforts->ThermCndct - ThermCndctNeut) * ThemCndct_75_fac;
                    state.dataThermalComforts->VasoconstrictFac = (ThermCndctNeut - state.dataThermalComforts->ThermCndct) * ThemCndct_fac;
                    //  IF VasodilationFac < 0.0, VASOCONSTRICTION OCCURS AND RESULTS IN COLD SENSATION.
                    //  OTHERWISE NORMAL BLOOD FLOW OR VASODILATION OCCURS AND RESULTS IN
                    //  THERMAL NEUTRALITY OR WARM SENSATION.
                    if (state.dataThermalComforts->VasodilationFac < 0) {
                        comfort.KsuTSV = -1.46153 * state.dataThermalComforts->VasoconstrictFac +
                                         3.74721 * pow_2(state.dataThermalComforts->VasoconstrictFac) -
                                         6.168856 * pow_3(state.dataThermalComforts->VasoconstrictFac);
                    } else {
                        comfort.KsuTSV = (5.0 - 6.56 * (state.dataThermalComforts->RelHum - 0.50)) * SkinWetFac;
                        if (comfort.KsuTSV > TSVMax) {
                            comfort.KsuTSV = TSVMax;
                        }
                    }

                    comfort.ThermalComfortMRT = state.dataThermalComforts->RadTemp;
                    comfort.ThermalComfortOpTemp = (state.dataThermalComforts->RadTemp + state.dataThermalComforts->AirTemp) / 2.0;

                    state.dataThermalComforts->CoreTemp = state.dataThermalComforts->Temp(1);
                    state.dataThermalComforts->SkinTemp = state.dataThermalComforts->Temp(2);
                    state.dataThermalComforts->EvapHeatLossSweatPrev = state.dataThermalComforts->EvapHeatLossSweat;

                    RKG(state,
                        TempIndiceNum,
                        state.dataThermalComforts->TimeChange,
                        state.dataThermalComforts->Time,
                        state.dataThermalComforts->Temp,
                        state.dataThermalComforts->TempChange,
                        state.dataThermalComforts->Coeff);

                    if (state.dataThermalComforts->Time > TimeExpos) {
                        break;
                    }
                }
            }
        }
    }

    void DERIV(EnergyPlusData &state,
               [[maybe_unused]] int &TempIndiceNum,    // Number of temperature indices  unused1208
               [[maybe_unused]] Array1D<Real64> &Temp, // Temperature unused1208
               Array1D<Real64> &TempChange             // Change of temperature
    )
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   January 2000
        //     MODIFIED       Rick Strand (for E+ implementation February 2000)

        // PURPOSE OF THIS SUBROUTINE:
        // THIS SUBROUTINE CALCULATES HEAT TRANSFER TERMS INVOLVED IN THE
        // THERMOREGULATORY SYSTEM TO OBTAIN THE RATES OF CHANGE OF CoreTemp & SkinTemp
        // VIZ., CoreTempChange/TempChange & SkinTempChange/TempChange RESPECTIVELY.

        // METHODOLOGY EMPLOYED:
        // This subroutine is based heavily upon the work performed by Dan Maloney for
        // the BLAST program.  Many of the equations are based on the original Pierce
        // development.  See documentation for further details and references.

        // REFERENCES:
        // Maloney, Dan, M.S. Thesis, University of Illinois at Urbana-Champaign

        // Argument array dimensioning
        // EP_SIZE_CHECK(Temp, 2);
        EP_SIZE_CHECK(TempChange, 2);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ActLevelTot;             // Total activity level
        Real64 CoreSignalShiv;          // Core signal when shivering occurs
        Real64 CoreSignalShivMax;       // Maximum value of core signal when shivering occurs
        Real64 CoreSignalSkinSens;      // The sensitivity of the skin signal increases
        Real64 CoreSignalSweatMax;      // Maximum value of core signal when sweating occurs
        Real64 CoreSignalSweatWarm;     // Core signal when sweating occurs
        Real64 CoreTempSweat;           // Core temperature when sweating occurs
        Real64 CoreSignalWarm;          // Warm core signal
        Real64 CoreSignalWarmMax;       // Maximum value of warm core signal
        Real64 EvapHeatLossDrySweat;    // Evaporative heat loss by sweating when total skin wettedness < 0.4
        Real64 Err;                     // Stop criteria for iteration
        Real64 ErrPrev;                 // Previous value of stop criteria for iteration
        Real64 EvapHeatLossSweatEst;    // Estimated evaporative heat loss by sweating
        Real64 EvapHeatLossSweatEstNew; // New value of estimated evaporative heat loss by sweating
        Real64 IntHeatProdTot;          // Total internal heat production
        Real64 SkinCndctMax;            // Maximum value of skin conductance
        Real64 SkinSignalCold;          // Cold skin signal
        Real64 SkinSignalColdMax;       // Maximum value of cold skin signal
        Real64 SkinSignalSweatCold;     // Cold skin signal for sweat inhibition
        Real64 SkinSignalSweatColdMax;  // Maximum value of cold skin signal for sweat inhibition
        Real64 SkinCndctDilation;       // Overall skin conductance due to vasodilation
        Real64 SkinCndctConstriction;   // Overall skin conductance due to vasoconstriction
        Real64 SkinSignalShiv;          // Skin signal when shivering occurs
        Real64 SkinSignalShivMax;       // Maximum value of skin signal when shivering occurs
        Real64 SkinSignalSweatMax;      // Skin signal when sweating occurs
        Real64 SkinSignalSweatWarm;     // Maximum value of skin signal when sweating occurs
        Real64 SkinSignalWarm;          // Warm skin signal
        Real64 SkinSignalWarmMax;       // Maximum value of warm skin signal
        Real64 SkinTempSweat;           // Skin temperature when sweating occurs
        Real64 SkinWetSignal;           // Skin wettedness signal
        Real64 SweatCtrlFac;            // Sweat control factor
        Real64 SweatSuppFac;            // Sweat suppression factor due to skin wettedness
        Real64 WeighFac;                // Weighting factor of core signal

        // THE CONTROLLING SYSTEM.
        // THE CONTROLLING SIGNALS :
        // SIGNALS FOR KS.
        CoreSignalWarm = state.dataThermalComforts->CoreTemp - 36.98;
        SkinSignalWarm = state.dataThermalComforts->SkinTemp - 33.8;
        SkinSignalCold = 32.1 - state.dataThermalComforts->SkinTemp;
        CoreSignalSkinSens = state.dataThermalComforts->CoreTemp - 35.15;
        CoreSignalWarmMax = max(0.0, CoreSignalWarm);
        SkinSignalWarmMax = max(0.0, SkinSignalWarm);
        SkinSignalColdMax = max(0.0, SkinSignalCold);

        // SIGNALS FOR EvapHeatLossSweat.
        CoreTempSweat = state.dataThermalComforts->CoreTemp;
        if (CoreTempSweat > 38.29) {
            CoreTempSweat = 38.29;
        }
        CoreSignalSweatWarm = CoreTempSweat - state.dataThermalComforts->CoreTempNeut;
        SkinTempSweat = state.dataThermalComforts->SkinTemp;
        if (SkinTempSweat > 36.1) {
            SkinTempSweat = 36.1;
        }
        SkinSignalSweatWarm = SkinTempSweat - state.dataThermalComforts->SkinTempNeut;
        CoreSignalSweatMax = max(0.0, CoreSignalSweatWarm);
        SkinSignalSweatMax = max(0.0, SkinSignalSweatWarm);
        SkinSignalSweatCold = 33.37 - state.dataThermalComforts->SkinTemp;
        if (state.dataThermalComforts->SkinTempNeut < 33.37) {
            SkinSignalSweatCold = state.dataThermalComforts->SkinTempNeut - state.dataThermalComforts->SkinTemp;
        }
        SkinSignalSweatColdMax = max(0.0, SkinSignalSweatCold);

        // SIGNALS FOR SHIVERING.
        CoreSignalShiv = 36.9 - state.dataThermalComforts->CoreTemp;
        SkinSignalShiv = 32.5 - state.dataThermalComforts->SkinTemp;
        CoreSignalShivMax = max(0.0, CoreSignalShiv);
        SkinSignalShivMax = max(0.0, SkinSignalShiv);

        // CONTROLLING FUNCTIONS :
        // SHIVERING RESPONSE IN W/M**2.
        state.dataThermalComforts->ShivResponse = 20.0 * CoreSignalShivMax * SkinSignalShivMax + 5.0 * SkinSignalShivMax;
        if (state.dataThermalComforts->CoreTemp >= 37.1) {
            state.dataThermalComforts->ShivResponse = 0.0;
        }

        // SWEAT FUNCTION IN W/M**2.
        WeighFac = 260.0 + 70.0 * state.dataThermalComforts->AcclPattern;
        SweatCtrlFac = 1.0 + 0.05 * std::pow(SkinSignalSweatColdMax, 2.4);

        // EvapHeatLossDrySweat = SWEAT WHEN SkinWetTot < 0.4.
        EvapHeatLossDrySweat =
            ((WeighFac * CoreSignalSweatMax + 0.1 * WeighFac * SkinSignalSweatMax) * std::exp(SkinSignalSweatMax / 8.5)) / SweatCtrlFac;

        // MAXIMUM EVAPORATIVE POWER, EvapHeatLossMax, IN W/M**2.
        state.dataThermalComforts->SkinVapPress = CalcSatVapPressFromTemp(state.dataThermalComforts->SkinTemp);
        state.dataThermalComforts->EvapHeatLossMax = 2.2 * state.dataThermalComforts->Hc *
                                                     (state.dataThermalComforts->SkinVapPress - state.dataThermalComforts->VapPress) *
                                                     state.dataThermalComforts->CloPermeatEff;
        if (state.dataThermalComforts->EvapHeatLossMax > 0.0) {
            state.dataThermalComforts->SkinWetSweat = EvapHeatLossDrySweat / state.dataThermalComforts->EvapHeatLossMax;
            state.dataThermalComforts->EvapHeatLossDiff = 0.408 * (state.dataThermalComforts->SkinVapPress - state.dataThermalComforts->VapPress);
            state.dataThermalComforts->EvapHeatLoss = state.dataThermalComforts->SkinWetSweat * state.dataThermalComforts->EvapHeatLossMax +
                                                      (1.0 - state.dataThermalComforts->SkinWetSweat) * state.dataThermalComforts->EvapHeatLossDiff;
            state.dataThermalComforts->SkinWetTot = state.dataThermalComforts->EvapHeatLoss / state.dataThermalComforts->EvapHeatLossMax;
            if (state.dataThermalComforts->Time == 0.0) {
                state.dataThermalComforts->EvapHeatLossSweat = EvapHeatLossDrySweat;
                state.dataThermalComforts->EvapHeatLossSweatPrev = EvapHeatLossDrySweat;
            }
            if (state.dataThermalComforts->SkinWetTot > 0.4) {

                // ITERATION  FOR SWEAT WHEN SkinWetTot IS GREATER THAT 0.4.
                state.dataThermalComforts->IterNum = 0;
                if (state.dataThermalComforts->SkinWetSweat > 1.0) {
                    state.dataThermalComforts->SkinWetSweat = 1.0;
                }
                while (true) {
                    EvapHeatLossSweatEst = state.dataThermalComforts->EvapHeatLossSweatPrev;
                    state.dataThermalComforts->SkinWetSweat = EvapHeatLossSweatEst / state.dataThermalComforts->EvapHeatLossMax;

                    if (state.dataThermalComforts->SkinWetSweat > 1.0) {
                        state.dataThermalComforts->SkinWetSweat = 1.0;
                    }

                    state.dataThermalComforts->EvapHeatLossDiff =
                        0.408 * (state.dataThermalComforts->SkinVapPress - state.dataThermalComforts->VapPress);
                    state.dataThermalComforts->EvapHeatLoss =
                        (1.0 - state.dataThermalComforts->SkinWetTot) * state.dataThermalComforts->EvapHeatLossDiff +
                        state.dataThermalComforts->EvapHeatLossSweat;
                    state.dataThermalComforts->SkinWetTot = state.dataThermalComforts->EvapHeatLoss / state.dataThermalComforts->EvapHeatLossMax;

                    if (state.dataThermalComforts->SkinWetTot > 1.0) {
                        state.dataThermalComforts->SkinWetTot = 1.0;
                    }

                    SkinWetSignal = max(0.0, state.dataThermalComforts->SkinWetTot - 0.4);
                    SweatSuppFac = 0.5 + 0.5 * std::exp(-5.6 * SkinWetSignal);
                    EvapHeatLossSweatEstNew = SweatSuppFac * EvapHeatLossDrySweat;

                    if (state.dataThermalComforts->IterNum == 0) {
                        state.dataThermalComforts->EvapHeatLossSweat = EvapHeatLossSweatEstNew;
                    }

                    Err = EvapHeatLossSweatEst - EvapHeatLossSweatEstNew;

                    if (state.dataThermalComforts->IterNum != 0) {
                        if ((ErrPrev * Err) < 0.0) {
                            state.dataThermalComforts->EvapHeatLossSweat = (EvapHeatLossSweatEst + EvapHeatLossSweatEstNew) / 2.0;
                        }
                        if ((ErrPrev * Err) >= 0.0) {
                            state.dataThermalComforts->EvapHeatLossSweat = EvapHeatLossSweatEstNew;
                        }
                    }

                    // STOP CRITERION FOR THE ITERATION.
                    if ((std::abs(Err) <= 0.5) || (state.dataThermalComforts->IterNum >= 10)) {
                        break;
                    }
                    ++state.dataThermalComforts->IterNum;
                    state.dataThermalComforts->EvapHeatLossSweatPrev = state.dataThermalComforts->EvapHeatLossSweat;
                    ErrPrev = Err;
                }

            } else {
                state.dataThermalComforts->EvapHeatLossSweat = EvapHeatLossDrySweat;
            }

        } else {
            state.dataThermalComforts->SkinWetSweat = 1.0;
            state.dataThermalComforts->SkinWetTot = 1.0;
            state.dataThermalComforts->EvapHeatLossSweat = 0.5 * EvapHeatLossDrySweat;
            state.dataThermalComforts->EvapHeatLoss = state.dataThermalComforts->EvapHeatLossSweat;
        }

        // OVERALL SKIN CONDUCTANCE, KS, IN W/M**2/C.
        // SkinCndctDilation = EFFECT DUE TO VASODILATION.
        // SkinCndctConstriction = EFFECT DUE TO VASOCONSTRICTION.
        SkinCndctDilation = 42.45 * CoreSignalWarmMax + 8.15 * std::pow(CoreSignalSkinSens, 0.8) * SkinSignalWarmMax;
        SkinCndctConstriction = 1.0 + 0.4 * SkinSignalColdMax;
        // ThermCndct IS EQUIVALENT TO KS
        state.dataThermalComforts->ThermCndct = 5.3 + (6.75 + SkinCndctDilation) / SkinCndctConstriction;
        SkinCndctMax = 75.0 + 10.0 * state.dataThermalComforts->AcclPattern;
        if (state.dataThermalComforts->ThermCndct > SkinCndctMax) {
            state.dataThermalComforts->ThermCndct = SkinCndctMax;
        }

        // PASSIVE ENERGY BALANCE EQUATIONS.
        // TOTAL METABOLIC HEAT PRODUCTION RATE, ActLevel, IN W/M**2.
        ActLevelTot = state.dataThermalComforts->ActLevel + state.dataThermalComforts->ShivResponse;
        IntHeatProdTot = ActLevelTot - state.dataThermalComforts->WorkEff;
        // RESPIRATION HEAT LOSS, RespHeatLoss, IN W/M**0.
        state.dataThermalComforts->LatRespHeatLoss = 0.0023 * ActLevelTot * (44.0 - state.dataThermalComforts->VapPress);
        state.dataThermalComforts->DryRespHeatLoss = 0.0014 * ActLevelTot * (34.0 - state.dataThermalComforts->AirTemp);
        state.dataThermalComforts->RespHeatLoss = state.dataThermalComforts->LatRespHeatLoss + state.dataThermalComforts->DryRespHeatLoss;
        // HEAT FLOW FROM CORE TO SKIN, HeatFlow, IN W/M**2.
        state.dataThermalComforts->HeatFlow =
            state.dataThermalComforts->ThermCndct * (state.dataThermalComforts->CoreTemp - state.dataThermalComforts->SkinTemp);
        // TempChange(1) = CoreTempChange/TempChange, IN C/HR.
        TempChange(1) = (IntHeatProdTot - state.dataThermalComforts->RespHeatLoss - state.dataThermalComforts->HeatFlow) /
                        state.dataThermalComforts->CoreThermCap;
        if (state.dataThermalComforts->EvapHeatLoss > state.dataThermalComforts->EvapHeatLossMax) {
            state.dataThermalComforts->EvapHeatLoss = state.dataThermalComforts->EvapHeatLossMax;
        }

        // DRY HEAT EXCHANGE BY RADIATION & CONVECTION, R+C, IN W/M**2.
        state.dataThermalComforts->DryHeatLoss = state.dataThermalComforts->H * state.dataThermalComforts->CloBodyRat *
                                                 state.dataThermalComforts->CloThermEff *
                                                 (state.dataThermalComforts->SkinTemp - state.dataThermalComforts->OpTemp);
        // TempChange(2) = SkinTempChange/TempChange, IN C/HR.
        TempChange(2) = (state.dataThermalComforts->HeatFlow - state.dataThermalComforts->EvapHeatLoss - state.dataThermalComforts->DryHeatLoss) /
                        state.dataThermalComforts->SkinThermCap;
    }

    void RKG(EnergyPlusData &state, int &NEQ, Real64 const H, Real64 &X, Array1D<Real64> &Y, Array1D<Real64> &DY, Array1D<Real64> &C)
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   January 2000
        //     MODIFIED       Rick Strand (for E+ implementation February 2000)

        // PURPOSE OF THIS SUBROUTINE:
        // This is a subroutine for integration by Runga-Kutta's method.

        // METHODOLOGY EMPLOYED:
        // This subroutine is based heavily upon the work performed by Dan Maloney for
        // the BLAST program.  Many of the equations are based on the original Pierce
        // development.  See documentation for further details and references.

        // REFERENCES:
        // Maloney, Dan, M.S. Thesis, University of Illinois at Urbana-Champaign

        // Argument array dimensioning
        EP_SIZE_CHECK(Y, NEQ);
        EP_SIZE_CHECK(DY, NEQ);
        EP_SIZE_CHECK(C, NEQ);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int I;
        int J;
        Real64 B;
        Real64 H2;
        static constexpr std::array<Real64, 2> A = {0.29289321881345, 1.70710678118654};

        H2 = 0.5 * H;

        DERIV(state, NEQ, Y, DY);
        for (I = 1; I <= NEQ; ++I) {
            B = H2 * DY(I) - C(I);
            Y(I) += B;
            C(I) += 3.0 * B - H2 * DY(I);
        }

        X += H2;

        for (J = 0; J < 2; ++J) {
            DERIV(state, NEQ, Y, DY);
            for (I = 1; I <= NEQ; ++I) {
                B = A[J] * (H * DY(I) - C(I));
                Y(I) += B;
                C(I) += 3.0 * B - A[J] * H * DY(I);
            }
        }

        X += H2;
        DERIV(state, NEQ, Y, DY);

        for (I = 1; I <= NEQ; ++I) {
            B = (H * DY(I) - 2.0 * C(I)) / 6.0;
            Y(I) += B;
            C(I) += 3.0 * B - H2 * DY(I);
        }

        DERIV(state, NEQ, Y, DY);
    }

    void GetAngleFactorList(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   July 2001

        static constexpr std::string_view routineName("GetAngleFactorList: "); // include trailing blank space
        Real64 constexpr AngleFacLimit(0.01);                                  // To set the limit of sum of angle factors

        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int IOStatus;
        int NumAlphas;  // Number of Alphas from InputProcessor
        int NumNumbers; // Number of Numbers from Input Processor
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

        cCurrentModuleObject = "ComfortViewFactorAngles";
        int NumOfAngleFactorLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        state.dataThermalComforts->AngleFactorList.allocate(NumOfAngleFactorLists);

        for (int Item = 1; Item <= NumOfAngleFactorLists; ++Item) {

            Real64 AllAngleFacSummed = 0.0; // Sum of angle factors in each zone
            auto &thisAngFacList(state.dataThermalComforts->AngleFactorList(Item));

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            thisAngFacList.Name = state.dataIPShortCut->cAlphaArgs(1); // no need for verification/uniqueness.

            thisAngFacList.TotAngleFacSurfaces = NumNumbers;
            thisAngFacList.SurfaceName.allocate(thisAngFacList.TotAngleFacSurfaces);
            thisAngFacList.SurfacePtr.allocate(thisAngFacList.TotAngleFacSurfaces);
            thisAngFacList.AngleFactor.allocate(thisAngFacList.TotAngleFacSurfaces);

            for (int SurfNum = 1; SurfNum <= thisAngFacList.TotAngleFacSurfaces; ++SurfNum) {
                thisAngFacList.SurfaceName(SurfNum) = state.dataIPShortCut->cAlphaArgs(SurfNum + 1);
                thisAngFacList.SurfacePtr(SurfNum) = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(SurfNum + 1), state.dataSurface->Surface);
                thisAngFacList.AngleFactor(SurfNum) = state.dataIPShortCut->rNumericArgs(SurfNum);
                // Error trap for surfaces that do not exist or surfaces not in the zone
                if (thisAngFacList.SurfacePtr(SurfNum) == 0) {
                    ShowSevereError(state,
                                    format("{}: invalid {}, entered value={}",
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaFieldNames(SurfNum + 1),
                                           state.dataIPShortCut->cAlphaArgs(SurfNum + 1)));
                    ShowContinueError(state,
                                      format("ref {}={} not found in {}={}",
                                             state.dataIPShortCut->cAlphaFieldNames(1),
                                             state.dataIPShortCut->cAlphaArgs(1),
                                             state.dataIPShortCut->cAlphaFieldNames(2),
                                             state.dataIPShortCut->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    // Found Surface, is it in same enclosure?
                    auto &thisSurf = state.dataSurface->Surface(thisAngFacList.SurfacePtr(SurfNum));
                    if (SurfNum == 1) {
                        thisAngFacList.EnclosurePtr = thisSurf.RadEnclIndex; // Save enclosure num of first surface
                    }
                    if (thisAngFacList.EnclosurePtr != thisSurf.RadEnclIndex) {
                        ShowWarningError(state,
                                         format("{}: For {}=\"{}\", surfaces are not all in the same radiant enclosure.",
                                                routineName,
                                                cCurrentModuleObject,
                                                thisAngFacList.Name));
                        ShowContinueError(state,
                                          format("... Surface=\"{}\" is in enclosure=\"{}\"",
                                                 state.dataSurface->Surface(thisAngFacList.SurfacePtr(1)).Name,
                                                 state.dataViewFactor->EnclRadInfo(thisAngFacList.EnclosurePtr).Name));
                        ShowContinueError(state,
                                          format("... Surface=\"{}\" is in enclosure=\"{}\"",
                                                 thisSurf.Name,
                                                 state.dataViewFactor->EnclRadInfo(thisSurf.RadEnclIndex).Name));
                    }
                }

                AllAngleFacSummed += thisAngFacList.AngleFactor(SurfNum);
            }

            if (std::abs(AllAngleFacSummed - 1.0) > AngleFacLimit) {
                ShowSevereError(state, format("{}=\"{}\", invalid - Sum[AngleFactors]", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("...Sum of Angle Factors [{:.3R}] should not deviate from expected sum [1.0] by more than limit [{:.3R}].",
                                         AllAngleFacSummed,
                                         AngleFacLimit));
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "GetAngleFactorList: Program terminated due to preceding errors.");
        }

        for (int Item = 1; Item <= state.dataHeatBal->TotPeople; ++Item) {
            auto &thisPeople = state.dataHeatBal->People(Item);
            if (thisPeople.MRTCalcType != DataHeatBalance::CalcMRT::AngleFactor) {
                continue;
            }
            thisPeople.AngleFactorListPtr = Util::FindItemInList(thisPeople.AngleFactorListName, state.dataThermalComforts->AngleFactorList);
            int WhichAFList = thisPeople.AngleFactorListPtr;
            if (WhichAFList == 0 && (thisPeople.Fanger || thisPeople.Pierce || thisPeople.KSU)) {
                ShowSevereError(state, format("{}{}=\"{}\", invalid", routineName, cCurrentModuleObject, thisPeople.AngleFactorListName));
                ShowContinueError(state, format("... Angle Factor List Name not found for PEOPLE=\"{}\"", thisPeople.Name));
                ErrorsFound = true;
            } else {
                auto &thisAngFacList = state.dataThermalComforts->AngleFactorList(WhichAFList);
                if (state.dataHeatBal->space(thisPeople.spaceIndex).radiantEnclosureNum != thisAngFacList.EnclosurePtr &&
                    (thisPeople.Fanger || thisPeople.Pierce || thisPeople.KSU)) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", radiant enclosure mismatch.", routineName, cCurrentModuleObject, thisAngFacList.Name));
                    ShowContinueError(
                        state,
                        format("...Enclosure=\"{}\" doe not match enclosure=\"{}\" for PEOPLE=\"{}\"",
                               state.dataViewFactor->EnclRadInfo(thisAngFacList.EnclosurePtr).Name,
                               state.dataViewFactor->EnclRadInfo(state.dataHeatBal->space(thisPeople.spaceIndex).radiantEnclosureNum).Name,
                               thisPeople.Name));
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "GetAngleFactorList: Program terminated due to preceding errors.");
        }
    }

    Real64 CalcAngleFactorMRT(EnergyPlusData &state, int const AngleFacNum)
    {

        // SUBROUTINE INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   July 2001
        //     MODIFIED       November 2017 (R Strand): Added fourth power and emissivity to calculation

        // Return value
        Real64 CalcAngleFactorMRT;

        Real64 SurfTempEmissAngleFacSummed = 0.0;
        Real64 SumSurfaceEmissAngleFactor = 0.0;

        auto &thisAngFacList(state.dataThermalComforts->AngleFactorList(AngleFacNum));

        for (int SurfNum = 1; SurfNum <= thisAngFacList.TotAngleFacSurfaces; ++SurfNum) {
            Real64 SurfaceTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(thisAngFacList.SurfacePtr(SurfNum)) + Constant::Kelvin;
            Real64 SurfEAF =
                state.dataConstruction->Construct(state.dataSurface->Surface(thisAngFacList.SurfacePtr(SurfNum)).Construction).InsideAbsorpThermal *
                thisAngFacList.AngleFactor(SurfNum);
            SurfTempEmissAngleFacSummed += SurfEAF * pow_4(SurfaceTemp);
            SumSurfaceEmissAngleFactor += SurfEAF;
        }

        CalcAngleFactorMRT = root_4(SurfTempEmissAngleFacSummed / SumSurfaceEmissAngleFactor) - Constant::Kelvin;

        return CalcAngleFactorMRT;
    }

    Real64 CalcSurfaceWeightedMRT(EnergyPlusData &state, int const SurfNum, bool AverageWithSurface)
    {

        // Purpose: Calculate a modified zone MRT that excludes the Surface( SurfNum ).
        //          This is necessary for the surface weighted option to not in essence
        //          double count SurfNum in the MRT calculation when averaged with the Surface( SurfNum ).
        //          Other than that, the method here is the same as CalculateZoneMRT.  Once a modified zone
        //          MRT is calculated, the subroutine then calculates and returns the
        //          RadTemp (radiant temperature) for use by the thermal comfort routines
        //          that is the average of the surface temperature to be weighted and
        //          the modified zone MRT.

        // Return value
        Real64 CalcSurfaceWeightedMRT = 0.0;

        // Initialize ZoneAESum for all zones and SurfaceAE for all surfaces at the start of the simulation
        if (state.dataThermalComforts->FirstTimeSurfaceWeightedFlag) {
            state.dataThermalComforts->FirstTimeError = true;
            state.dataThermalComforts->FirstTimeSurfaceWeightedFlag = false;
            for (auto const &thisRadEnclosure : state.dataViewFactor->EnclRadInfo) {
                for (int const SurfNum2 : thisRadEnclosure.SurfacePtr) {
                    auto &thisSurface2 = state.dataSurface->Surface(SurfNum2);
                    thisSurface2.AE = thisSurface2.Area * state.dataConstruction->Construct(thisSurface2.Construction).InsideAbsorpThermal;
                }
                // Do NOT include the contribution of the Surface that is being surface weighted in this calculation since it will already be
                // accounted for
                for (int const SurfNum1 : thisRadEnclosure.SurfacePtr) {
                    auto &thisSurface1 = state.dataSurface->Surface(SurfNum1);
                    thisSurface1.enclAESum = 0.0;
                    for (int const SurfNum2 : thisRadEnclosure.SurfacePtr) {
                        if (SurfNum2 == SurfNum1) {
                            continue;
                        }
                        auto &thisSurface2 = state.dataSurface->Surface(SurfNum2);
                        thisSurface1.enclAESum += thisSurface2.AE;
                    }
                }
            }
        }

        // Calculate the sum of area*emissivity and area*emissivity*temperature for all surfaces in the zone EXCEPT the surface being weighted
        Real64 sumAET = 0.0; // Intermediate calculational variable (area*emissivity*T) sum

        auto &thisSurface = state.dataSurface->Surface(SurfNum);
        auto &thisRadEnclosure = state.dataViewFactor->EnclRadInfo(thisSurface.RadEnclIndex);
        // Recalc SurfaceEnclAESum only if needed due to window shades or EMS
        if (thisRadEnclosure.radReCalc) {
            thisSurface.enclAESum = 0.0;
            for (int const SurfNum2 : thisRadEnclosure.SurfacePtr) {
                if (SurfNum2 == SurfNum) {
                    continue;
                }
                auto &thisSurface2 = state.dataSurface->Surface(SurfNum2);
                thisSurface2.AE = thisSurface2.Area * state.dataConstruction->Construct(thisSurface2.Construction).InsideAbsorpThermal;
                thisSurface.enclAESum += thisSurface2.AE;
            }
        }
        for (int const SurfNum2 : thisRadEnclosure.SurfacePtr) {
            if (SurfNum2 == SurfNum) {
                continue;
            }
            sumAET += state.dataSurface->Surface(SurfNum2).AE * state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum2);
        }

        // Now weight the MRT
        auto &thisSurfaceTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum);
        if (thisSurface.enclAESum > 0.01) {
            CalcSurfaceWeightedMRT = sumAET / thisSurface.enclAESum;
            // if averaged with surface--half comes from the surface used for weighting (SurfNum) and the rest from the calculated MRT that excludes
            // this surface
            if (AverageWithSurface) {
                CalcSurfaceWeightedMRT = 0.5 * (thisSurfaceTemp + CalcSurfaceWeightedMRT);
            }
        } else {
            if (state.dataThermalComforts->FirstTimeError) {
                int spaceNum = thisSurface.spaceNum;
                ShowWarningError(state,
                                 format("CalcSurfaceWeightedMRT: Areas*Inside surface emissivities are summing to zero for Enclosure=\"{}\"",
                                        thisRadEnclosure.Name));
                ShowContinueError(state,
                                  format("As a result, the MAT for Space={} will be used for MRT when calculating the surface weighted MRT.",
                                         state.dataHeatBal->space(spaceNum).Name));
                ShowContinueError(state, format("for Surface={}", thisSurface.Name));
                state.dataThermalComforts->FirstTimeError = false;
                CalcSurfaceWeightedMRT = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT;
                if (AverageWithSurface) {
                    CalcSurfaceWeightedMRT = 0.5 * (thisSurfaceTemp + CalcSurfaceWeightedMRT);
                }
            }
        }

        return CalcSurfaceWeightedMRT;
    }

    Real64 CalcSatVapPressFromTemp(Real64 const Temp)
    {

        // FUNCTION INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   January 2000
        //     MODIFIED       Rick Strand (for E+ implementation February 2000)

        // PURPOSE OF THIS FUNCTION:
        // THIS IS A FUNCTION TO CALCULATE THE SATURATED VAPOR PRESSURE
        // FROM AIR TEMPERATURE

        // METHODOLOGY EMPLOYED:
        // This function is based upon the work performed by Dan Maloney for
        // the BLAST program.
        // REFERENCES:
        // Maloney, Dan, M.S. Thesis, University of Illinois at Urbana-Champaign

        Real64 const XT(Temp / 100.0);
        return 6.16796 + 358.1855 * pow_2(XT) - 550.3543 * pow_3(XT) + 1048.8115 * pow_4(XT);

        // Helper function for pierceSET calculates Saturated Vapor Pressure (Torr) at Temperature T (°C)
        //        return Math.exp(18.6686 - 4030.183/(T + 235.0));
    }

    Real64 CalcSatVapPressFromTempTorr(Real64 const Temp)
    {
        // Helper function for pierceSET calculates Saturated Vapor Pressure (Torr) at Temperature T (°C)
        return std::exp(18.6686 - 4030.183 / (Temp + 235.0));
    }

    Real64 CalcRadTemp(EnergyPlusData &state, int const PeopleListNum)
    {

        // FUNCTION INFORMATION:
        //     AUTHOR         Jaewook Lee
        //     DATE WRITTEN   November 2000
        //     MODIFIED       Rick Strand (for E+ implementation November 2000)
        //                    Rick Strand (for high temperature radiant heaters March 2001)

        // PURPOSE OF THIS FUNCTION:
        // THIS IS A FUNCTION TO CALCULATE EITHER ZONE AVERAGED MRT OR
        // SURFACE WEIGHTED MRT

        // METHODOLOGY EMPLOYED:
        // The method here is fairly straight-forward.  If the user has selected
        // a zone average MRT calculation, then there is nothing to do other than
        // to assign the function value because the zone MRT has already been
        // calculated.  Note that this value is an "area-emissivity" weighted value.
        // If the user wants to place the occupant "near" a particular surface,
        // then at the limit half of the radiant field will be from this surface.
        // As a result, an average of the zone MRT and the surface temperature
        // is taken to arrive at an approximate radiant temperature.
        // If a high temperature radiant heater is present, then this must also be
        // taken into account.  The equation used to account for this factor is
        // based on equation 49 on page 150 of Fanger's text (see reference below).
        // The additional assumptions for EnergyPlus are that the radiant energy
        // from the heater must be spread over the average area of a human being
        // (see parameter below) and that the emissivity and absorptivity of the
        // occupant are equivalent for the dominant wavelength of radiant energy
        // from the heater.  These assumptions might be off slightly, but it does
        // allow for an approximation of the effects of surfaces and heaters
        // within a space.  Future additions might include the effect of direct
        // solar energy on occupants.

        Real64 CalcRadTemp = 0.0;
        Real64 constexpr AreaEff = 1.8;                    // Effective area of a "standard" person in meters squared
        Real64 constexpr StefanBoltzmannConst = 5.6697e-8; // Stefan-Boltzmann constant in W/(m2*K4)

        auto &thisPeople = state.dataHeatBal->People(PeopleListNum);
        switch (thisPeople.MRTCalcType) {
        case DataHeatBalance::CalcMRT::EnclosureAveraged: {
            int enclNum = state.dataHeatBal->space(thisPeople.spaceIndex).radiantEnclosureNum;
            state.dataThermalComforts->RadTemp = state.dataViewFactor->EnclRadInfo(enclNum).MRT;
        } break;
        case DataHeatBalance::CalcMRT::SurfaceWeighted: {
            state.dataThermalComforts->RadTemp = CalcSurfaceWeightedMRT(state, thisPeople.SurfacePtr);
        } break;
        case DataHeatBalance::CalcMRT::AngleFactor: {
            state.dataThermalComforts->RadTemp = CalcAngleFactorMRT(state, thisPeople.AngleFactorListPtr);
        } break;
        default:
            break;
        }

        // If high temperature radiant heater present and on, then must account for this in MRT calculation
        // MJW MRT ToDo: Think about what happens here - at a minimum, set a flag to skip this if there isn't any radiant HVAC
        state.dataHeatBalFanSys->ZoneQdotRadHVACToPerson(state.dataThermalComforts->ZoneNum) =
            state.dataHeatBalFanSys->ZoneQHTRadSysToPerson(state.dataThermalComforts->ZoneNum) +
            state.dataHeatBalFanSys->ZoneQCoolingPanelToPerson(state.dataThermalComforts->ZoneNum) +
            state.dataHeatBalFanSys->ZoneQHWBaseboardToPerson(state.dataThermalComforts->ZoneNum) +
            state.dataHeatBalFanSys->ZoneQSteamBaseboardToPerson(state.dataThermalComforts->ZoneNum) +
            state.dataHeatBalFanSys->ZoneQElecBaseboardToPerson(state.dataThermalComforts->ZoneNum);
        if (state.dataHeatBalFanSys->ZoneQdotRadHVACToPerson(state.dataThermalComforts->ZoneNum) > 0.0) {
            state.dataThermalComforts->RadTemp += Constant::Kelvin; // Convert to Kelvin
            state.dataThermalComforts->RadTemp =
                root_4(pow_4(state.dataThermalComforts->RadTemp) +
                       (state.dataHeatBalFanSys->ZoneQdotRadHVACToPerson(state.dataThermalComforts->ZoneNum) / AreaEff / StefanBoltzmannConst));
            state.dataThermalComforts->RadTemp -= Constant::Kelvin; // Convert back to Celsius
        }

        CalcRadTemp = state.dataThermalComforts->RadTemp;

        return CalcRadTemp;
    }

    void CalcThermalComfortSimpleASH55(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   June 2005

        // PURPOSE OF THIS SUBROUTINE:
        //   Determines if the space is within the ASHRAE 55-2004 comfort region
        //   based on operative temperature and humidity ratio

        // Using/Aliasing
        using OutputReportTabular::isInQuadrilateral;
        using namespace OutputReportPredefined;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OperTemp;
        Real64 NumberOccupants;
        bool isComfortableWithSummerClothes;
        bool isComfortableWithWinterClothes;
        Real64 allowedHours;
        bool showWarning;

        state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Summer = 0.0;
        state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Winter = 0.0;
        state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Either = 0.0;

        // assume the zone is unoccupied
        for (auto &e : state.dataThermalComforts->ThermalComfortInASH55) {
            e.ZoneIsOccupied = false;
        }
        // loop through the people objects and determine if the zone is currently occupied
        for (auto const &people : state.dataHeatBal->People) {
            state.dataThermalComforts->ZoneNum = people.ZonePtr;
            NumberOccupants = people.NumberOfPeople * people.sched->getCurrentVal();
            if (NumberOccupants > 0) {
                state.dataThermalComforts->ThermalComfortInASH55(state.dataThermalComforts->ZoneNum).ZoneIsOccupied = true;
            }
        }
        // loop through the zones and determine if in simple ashrae 55 comfort regions
        // MJW MRT ToDo: Extend ASHRAE 55 to spaces?
        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            if (state.dataThermalComforts->ThermalComfortInASH55(iZone).ZoneIsOccupied) {
                auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(iZone);
                // keep track of occupied hours
                state.dataThermalComforts->ZoneOccHrs(iZone) += state.dataGlobal->TimeStepZone;
                Real64 CurAirTemp = thisZoneHB.ZTAVComf;
                if (state.dataRoomAir->anyNonMixingRoomAirModel) {
                    if (state.dataRoomAir->IsZoneDispVent3Node(iZone) || state.dataRoomAir->IsZoneUFAD(iZone)) {
                        CurAirTemp = state.dataRoomAir->TCMF(iZone);
                    }
                }
                Real64 CurMeanRadiantTemp = thisZoneHB.MRT;
                OperTemp = CurAirTemp * 0.5 + CurMeanRadiantTemp * 0.5;
                // for debugging
                // ThermalComfortInASH55(iZone)%dCurAirTemp = CurAirTemp
                // ThermalComfortInASH55(iZone)%dCurMeanRadiantTemp = CurMeanRadiantTemp
                // ThermalComfortInASH55(iZone)%dOperTemp = OperTemp
                // ThermalComfortInASH55(iZone)%dHumidRatio = HumidRatio
                // From ASHRAE Standard 55-2004 Appendix D
                //  Run    AirTemp(C)   RH(%)  Season  HumidRatio
                //   1       19.6        86    Winter    0.012
                //   2       23.9        66    Winter    0.012
                //   3       25.7        15    Winter    0.003
                //   4       21.2        20    Winter    0.003
                //   5       23.6        67    Summer    0.012
                //   6       26.8        56    Summer    0.012
                //   7       27.9        13    Summer    0.003
                //   8       24.7        16    Summer    0.003
                // But the standard says "no recommended lower humidity limit" so it should
                // really extend down to the 0.0 Humidity ratio line.  Extrapolating we get
                // the values that are shown in the following table
                //  Run    AirTemp(C)    Season  HumidRatio
                //   1       19.6        Winter    0.012
                //   2       23.9        Winter    0.012
                //   3       26.3        Winter    0.000
                //   4       21.7        Winter    0.000
                //   5       23.6        Summer    0.012
                //   6       26.8        Summer    0.012
                //   7       28.3        Summer    0.000
                //   8       25.1        Summer    0.000
                // check summer clothing conditions
                isComfortableWithSummerClothes =
                    isInQuadrilateral(OperTemp, thisZoneHB.airHumRatAvgComf, 25.1, 0.0, 23.6, 0.012, 26.8, 0.012, 28.3, 0.0);
                // check winter clothing conditions
                isComfortableWithWinterClothes =
                    isInQuadrilateral(OperTemp, thisZoneHB.airHumRatAvgComf, 21.7, 0.0, 19.6, 0.012, 23.9, 0.012, 26.3, 0.0);
                if (isComfortableWithSummerClothes) {
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotSummer = 0.0;
                } else {
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotSummer = state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotSummer += state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Summer = state.dataGlobal->TimeStepZone;
                }
                if (isComfortableWithWinterClothes) {
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotWinter = 0.0;
                } else {
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotWinter = state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotWinter += state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Winter = state.dataGlobal->TimeStepZone;
                }
                if (isComfortableWithSummerClothes || isComfortableWithWinterClothes) {
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotEither = 0.0;
                } else {
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotEither = state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither += state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Either = state.dataGlobal->TimeStepZone;
                }
            } else {
                // when no one present in that portion of the zone then no one can be uncomfortable
                state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotSummer = 0.0;
                state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotWinter = 0.0;
                state.dataThermalComforts->ThermalComfortInASH55(iZone).timeNotEither = 0.0;
            }
        }
        // accumulate total time
        state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Summer += state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Summer;
        state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Winter += state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Winter;
        state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Either += state.dataThermalComforts->AnyZoneTimeNotSimpleASH55Either;

        if (state.dataGlobal->EndDesignDayEnvrnsFlag) {
            allowedHours = double(state.dataGlobal->NumOfDayInEnvrn) * 24.0 * 0.04;
            // first check if warning should be printed
            showWarning = false;
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                if (state.dataThermalComforts->ThermalComfortInASH55(iZone).Enable55Warning) {
                    if (state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither > allowedHours) {
                        showWarning = true;
                    }
                }
            }
            // if any zones should be warning print it out
            if (showWarning) {
                ShowWarningError(state, format("More than 4% of time ({:.1R} hours) uncomfortable in one or more zones ", allowedHours));
                ShowContinueError(state, "Based on ASHRAE 55-2004 graph (Section 5.2.1.1)");
                if (state.dataEnvrn->RunPeriodEnvironment) {
                    ShowContinueError(state,
                                      format("During Environment [{}]: {}", state.dataEnvrn->EnvironmentStartEnd, state.dataEnvrn->EnvironmentName));
                } else {
                    ShowContinueError(
                        state,
                        format("During SizingPeriod Environment [{}]: {}", state.dataEnvrn->EnvironmentStartEnd, state.dataEnvrn->EnvironmentName));
                }
                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    if (state.dataThermalComforts->ThermalComfortInASH55(iZone).Enable55Warning) {
                        if (state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither > allowedHours) {
                            ShowContinueError(state,
                                              format("{:.1R} hours were uncomfortable in zone: {}",
                                                     state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither,
                                                     state.dataHeatBal->Zone(iZone).Name));
                        }
                    }
                }
            }
            // put in predefined reports
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchSCwinterClothes,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotWinter);
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchSCsummerClothes,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotSummer);
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchSCeitherClothes,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither);
            }
            PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchSCwinterClothes, "Facility", state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Winter);
            PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchSCsummerClothes, "Facility", state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Summer);
            PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchSCeitherClothes, "Facility", state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Either);
            // set value for ABUPS report
            state.dataOutRptPredefined->TotalTimeNotSimpleASH55EitherForABUPS = state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Either;
            // reset accumulation for new environment
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotWinter = 0.0;
                state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotSummer = 0.0;
                state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither = 0.0;
            }
            state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Winter = 0.0;
            state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Summer = 0.0;
            state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Either = 0.0;
            // report how the aggregation is conducted
            switch (state.dataGlobal->KindOfSim) {
            case Constant::KindOfSim::DesignDay: {
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstSimpleComfort, "Aggregated over the Design Days");
            } break;
            case Constant::KindOfSim::RunPeriodDesign: {
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstSimpleComfort, "Aggregated over the RunPeriods for Design");
            } break;
            case Constant::KindOfSim::RunPeriodWeather: {
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstSimpleComfort, "Aggregated over the RunPeriods for Weather");
            } break;
            default:
                break;
            }
            // report number of occupied hours per week for LEED report
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchLeedSutHrsWeek,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 7 * 24 * (state.dataThermalComforts->ZoneOccHrs(iZone) / (state.dataGlobal->NumOfDayInEnvrn * 24)));
            }
        }
    }

    void ResetThermalComfortSimpleASH55(EnergyPlusData &state)
    {
        // Jason Glazer - October 2015
        // Reset thermal comfort table gathering arrays to zero for multi-year simulations
        // so that only last year is reported in tabular reports
        int iZone;
        for (iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotWinter = 0.0;
            state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotSummer = 0.0;
            state.dataThermalComforts->ThermalComfortInASH55(iZone).totalTimeNotEither = 0.0;
        }
        state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Winter = 0.0;
        state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Summer = 0.0;
        state.dataThermalComforts->TotalAnyZoneTimeNotSimpleASH55Either = 0.0;
    }

    void CalcIfSetPointMet(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   July 2005

        // PURPOSE OF THIS SUBROUTINE:
        //   Report if the setpoint temperature has been met.
        //   Add calculation of how far away from setpoint and if setpoint was not met
        //   during all times and during occupancy.

        // Using/Aliasing
        using namespace OutputReportPredefined;
        Real64 const deviationFromSetPtThresholdClg = state.dataHVACGlobal->deviationFromSetPtThresholdClg;
        Real64 const deviationFromSetPtThresholdHtg = state.dataHVACGlobal->deviationFromSetPtThresholdHtg;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SensibleLoadPredictedNoAdj;
        Real64 deltaT;
        int iZone;
        bool testHeating;
        bool testCooling;

        // Get the load predicted - the sign will indicate if heating or cooling
        // was called for
        state.dataThermalComforts->AnyZoneNotMetHeating = 0.0;
        state.dataThermalComforts->AnyZoneNotMetCooling = 0.0;
        state.dataThermalComforts->AnyZoneNotMetOccupied = 0.0;
        state.dataThermalComforts->AnyZoneNotMetHeatingOccupied = 0.0;
        state.dataThermalComforts->AnyZoneNotMetCoolingOccupied = 0.0;
        for (iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(iZone);

            SensibleLoadPredictedNoAdj = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(iZone).TotalOutputRequired;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetCooling = 0.0;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetHeating = 0.0;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetCoolingOccupied = 0.0;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetHeatingOccupied = 0.0;

            testHeating = (state.dataHeatBalFanSys->TempControlType(iZone) != HVAC::SetptType::SingleCool);
            testCooling = (state.dataHeatBalFanSys->TempControlType(iZone) != HVAC::SetptType::SingleHeat);

            if (testHeating && (SensibleLoadPredictedNoAdj > 0)) { // heating
                if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAir::RoomAirModel::Mixing) {
                    deltaT = state.dataHeatBalFanSys->TempTstatAir(iZone) - zoneTstatSetpt.setptLo;
                } else {
                    if (state.dataZoneTempPredictorCorrector->NumOnOffCtrZone > 0) {
                        deltaT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(iZone).ZTAV - zoneTstatSetpt.setptLoAver;
                    } else {
                        deltaT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(iZone).ZTAV - zoneTstatSetpt.setptLo;
                    }
                }
                if (deltaT < deviationFromSetPtThresholdHtg) {
                    state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetHeating = state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeating += state.dataGlobal->TimeStepZone;
                    if (state.dataThermalComforts->AnyZoneNotMetHeating == 0.0) {
                        state.dataThermalComforts->AnyZoneNotMetHeating = state.dataGlobal->TimeStepZone;
                    }
                    if (state.dataThermalComforts->ThermalComfortInASH55(iZone).ZoneIsOccupied) {
                        state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetHeatingOccupied = state.dataGlobal->TimeStepZone;
                        state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeatingOccupied += state.dataGlobal->TimeStepZone;
                        if (state.dataThermalComforts->AnyZoneNotMetHeatingOccupied == 0.0) {
                            state.dataThermalComforts->AnyZoneNotMetHeatingOccupied = state.dataGlobal->TimeStepZone;
                        }
                        if (state.dataThermalComforts->AnyZoneNotMetOccupied == 0.0) {
                            state.dataThermalComforts->AnyZoneNotMetOccupied = state.dataGlobal->TimeStepZone;
                        }
                    }
                }
            } else if (testCooling && (SensibleLoadPredictedNoAdj < 0)) { // cooling
                if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAir::RoomAirModel::Mixing) {
                    deltaT = state.dataHeatBalFanSys->TempTstatAir(iZone) - zoneTstatSetpt.setptHi;
                } else {
                    if (state.dataZoneTempPredictorCorrector->NumOnOffCtrZone > 0) {
                        deltaT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(iZone).ZTAV - zoneTstatSetpt.setptHiAver;
                    } else {
                        deltaT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(iZone).ZTAV - zoneTstatSetpt.setptHi;
                    }
                }

                if (state.dataHeatBal->Zone(iZone).HasAdjustedReturnTempByITE) {
                    deltaT = state.dataHeatBalFanSys->TempTstatAir(iZone) - state.dataHeatBal->Zone(iZone).AdjustedReturnTempByITE;
                }
                if (deltaT > deviationFromSetPtThresholdClg) {
                    state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetCooling = state.dataGlobal->TimeStepZone;
                    state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCooling += state.dataGlobal->TimeStepZone;
                    if (state.dataThermalComforts->AnyZoneNotMetCooling == 0.0) {
                        state.dataThermalComforts->AnyZoneNotMetCooling = state.dataGlobal->TimeStepZone;
                    }
                    if (state.dataThermalComforts->ThermalComfortInASH55(iZone).ZoneIsOccupied) {
                        state.dataThermalComforts->ThermalComfortSetPoint(iZone).notMetCoolingOccupied = state.dataGlobal->TimeStepZone;
                        state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCoolingOccupied += state.dataGlobal->TimeStepZone;
                        if (state.dataThermalComforts->AnyZoneNotMetCoolingOccupied == 0.0) {
                            state.dataThermalComforts->AnyZoneNotMetCoolingOccupied = state.dataGlobal->TimeStepZone;
                        }
                        if (state.dataThermalComforts->AnyZoneNotMetOccupied == 0.0) {
                            state.dataThermalComforts->AnyZoneNotMetOccupied = state.dataGlobal->TimeStepZone;
                        }
                    }
                }
            }
        }
        state.dataThermalComforts->TotalAnyZoneNotMetHeating += state.dataThermalComforts->AnyZoneNotMetHeating;
        state.dataThermalComforts->TotalAnyZoneNotMetCooling += state.dataThermalComforts->AnyZoneNotMetCooling;
        state.dataThermalComforts->TotalAnyZoneNotMetHeatingOccupied += state.dataThermalComforts->AnyZoneNotMetHeatingOccupied;
        state.dataThermalComforts->TotalAnyZoneNotMetCoolingOccupied += state.dataThermalComforts->AnyZoneNotMetCoolingOccupied;
        state.dataThermalComforts->TotalAnyZoneNotMetOccupied += state.dataThermalComforts->AnyZoneNotMetOccupied;

        // was EndEnvrnsFlag prior to CR7562
        if (state.dataGlobal->EndDesignDayEnvrnsFlag) {
            for (iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchULnotMetHeat,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeating);
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchULnotMetCool,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCooling);
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchULnotMetHeatOcc,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeatingOccupied);
                PreDefTableEntry(state,
                                 state.dataOutRptPredefined->pdchULnotMetCoolOcc,
                                 state.dataHeatBal->Zone(iZone).Name,
                                 state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCoolingOccupied);
            }
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchULnotMetHeat, "Facility", state.dataThermalComforts->TotalAnyZoneNotMetHeating);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchULnotMetCool, "Facility", state.dataThermalComforts->TotalAnyZoneNotMetCooling);
            PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchULnotMetHeatOcc, "Facility", state.dataThermalComforts->TotalAnyZoneNotMetHeatingOccupied);
            PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchULnotMetCoolOcc, "Facility", state.dataThermalComforts->TotalAnyZoneNotMetCoolingOccupied);
            // set value for ABUPS report
            state.dataOutRptPredefined->TotalNotMetHeatingOccupiedForABUPS = state.dataThermalComforts->TotalAnyZoneNotMetHeatingOccupied;
            state.dataOutRptPredefined->TotalNotMetCoolingOccupiedForABUPS = state.dataThermalComforts->TotalAnyZoneNotMetCoolingOccupied;
            state.dataOutRptPredefined->TotalNotMetOccupiedForABUPS = state.dataThermalComforts->TotalAnyZoneNotMetOccupied;
            // reset counters
            for (iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeating = 0.0;
                state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCooling = 0.0;
                state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeatingOccupied = 0.0;
                state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCoolingOccupied = 0.0;
            }
            state.dataThermalComforts->TotalAnyZoneNotMetHeating = 0.0;
            state.dataThermalComforts->TotalAnyZoneNotMetCooling = 0.0;
            state.dataThermalComforts->TotalAnyZoneNotMetHeatingOccupied = 0.0;
            state.dataThermalComforts->TotalAnyZoneNotMetCoolingOccupied = 0.0;
            state.dataThermalComforts->TotalAnyZoneNotMetOccupied = 0.0;
            // report how the aggregation is conducted
            switch (state.dataGlobal->KindOfSim) {
            case Constant::KindOfSim::DesignDay: {
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstUnmetLoads, "Aggregated over the Design Days");
            } break;
            case Constant::KindOfSim::RunPeriodDesign: {
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstUnmetLoads, "Aggregated over the RunPeriods for Design");
            } break;
            case Constant::KindOfSim::RunPeriodWeather: {
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstUnmetLoads, "Aggregated over the RunPeriods for Weather");
            } break;
            default:
                break;
            }
        }
    }

    void ResetSetPointMet(EnergyPlusData &state)
    {
        // Jason Glazer - October 2015
        // Reset set point not met table gathering arrays to zero for multi-year simulations
        // so that only last year is reported in tabular reports
        int iZone;
        for (iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeating = 0.0;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCooling = 0.0;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetHeatingOccupied = 0.0;
            state.dataThermalComforts->ThermalComfortSetPoint(iZone).totalNotMetCoolingOccupied = 0.0;
        }
        state.dataThermalComforts->TotalAnyZoneNotMetHeating = 0.0;
        state.dataThermalComforts->TotalAnyZoneNotMetCooling = 0.0;
        state.dataThermalComforts->TotalAnyZoneNotMetHeatingOccupied = 0.0;
        state.dataThermalComforts->TotalAnyZoneNotMetCoolingOccupied = 0.0;
        state.dataThermalComforts->TotalAnyZoneNotMetOccupied = 0.0;
    }

    void CalcThermalComfortAdaptiveASH55(
        EnergyPlusData &state,
        bool const initiate,                         // true if supposed to initiate
        ObjexxFCL::Optional_bool_const wthrsim,      // true if this is a weather simulation
        ObjexxFCL::Optional<Real64 const> avgdrybulb // approximate avg drybulb for design day.  will be used as previous period in design day
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Tyler Hoyt
        //       DATE WRITTEN   July 2011

        // PURPOSE OF THIS SUBROUTINE:
        // Sets up and carries out ASHRAE55-2010 adaptive comfort model calculations.
        // Output provided are state variables for the 80% and 90% acceptability limits
        // in the model, the comfort temperature, and the 30-day running average or
        // monthly average outdoor air temperature as parsed from the .STAT file.

        // METHODOLOGY EMPLOYED:
        // In order for the calculations to be possible the user must provide either
        // a .STAT file or .EPW file for the purpose of computing a monthly average
        // temperature or thirty-day running average. The subroutine need only open
        // the relevant file once to initialize, and then operates within the loop.

        // Using/Aliasing
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        using OutputReportTabular::GetColumnUsingTabs;
        using OutputReportTabular::StrToReal;

        // SUBROUTINE PARAMETER DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string lineAvg;
        std::string epwLine;
        Real64 dryBulb;
        Real64 tComf;
        Real64 numOccupants;
        int readStat;
        int jStartDay;
        int calcStartDay;
        int calcStartHr;
        int calcEndDay;
        int calcEndHr;
        std::string::size_type pos;
        int ind;
        int i;
        int j;
        bool weathersimulation;
        Real64 inavgdrybulb;

        if (initiate) { // not optional on initiate=true.  would otherwise check for presence
            weathersimulation = wthrsim;
            state.dataThermalComforts->avgDryBulbASH = 0.0;
            state.dataThermalComforts->runningAverageASH = 0.0;
            state.dataThermalComforts->monthlyTemp = 0.0;
            inavgdrybulb = avgdrybulb;
        } else {
            weathersimulation = false;
            inavgdrybulb = 0.0;
        }

        if (initiate && weathersimulation) {
            const bool statFileExists = FileSystem::fileExists(state.files.inStatFilePath.filePath);
            const bool epwFileExists = FileSystem::fileExists(state.files.inputWeatherFilePath.filePath);

            readStat = 0;
            if (statFileExists) {
                auto statFile = state.files.inStatFilePath.open(state, "CalcThermalComfortAdapctiveASH55");
                while (statFile.good()) {
                    auto lineIn = statFile.readLine();
                    if (has(lineIn.data, "Monthly Statistics for Dry Bulb temperatures")) {
                        for (i = 1; i <= 7; ++i) {
                            lineIn = statFile.readLine();
                        }
                        lineIn = statFile.readLine();
                        lineAvg = lineIn.data;
                        break;
                    }
                }
                for (i = 1; i <= 12; ++i) {
                    state.dataThermalComforts->monthlyTemp(i) = StrToReal(GetColumnUsingTabs(lineAvg, i + 2));
                }
                state.dataThermalComforts->useStatData = true;
            } else if (epwFileExists) {
                // determine number of days in year
                int DaysInYear;
                if (state.dataEnvrn->CurrentYearIsLeapYear) {
                    DaysInYear = 366;
                } else {
                    DaysInYear = 365;
                }
                state.dataThermalComforts->DailyAveOutTemp = 0.0;

                auto epwFile = state.files.inputWeatherFilePath.open(state, "CalcThermalComfortAdaptiveASH55");
                for (i = 1; i <= 8; ++i) { // Headers
                    epwLine = epwFile.readLine().data;
                }
                jStartDay = state.dataEnvrn->DayOfYear - 1;
                calcStartDay = jStartDay - 30;
                if (calcStartDay >= 0) {
                    calcStartHr = 24 * calcStartDay + 1;
                    for (i = 1; i <= calcStartHr - 1; ++i) {
                        epwFile.readLine();
                    }
                    for (i = 1; i <= 30; ++i) {
                        state.dataThermalComforts->avgDryBulbASH = 0.0;
                        for (j = 1; j <= 24; ++j) {
                            epwLine = epwFile.readLine().data;
                            for (ind = 1; ind <= 6; ++ind) {
                                pos = index(epwLine, ',');
                                epwLine.erase(0, pos + 1);
                            }
                            pos = index(epwLine, ',');
                            dryBulb = StrToReal(epwLine.substr(0, pos));
                            state.dataThermalComforts->avgDryBulbASH += (dryBulb / 24.0);
                        }
                        state.dataThermalComforts->DailyAveOutTemp(i) = state.dataThermalComforts->avgDryBulbASH;
                    }
                } else { // Do special things for wrapping the epw
                    calcEndDay = jStartDay;
                    calcStartDay += DaysInYear;
                    calcEndHr = 24 * calcEndDay;
                    calcStartHr = 24 * calcStartDay + 1;
                    for (i = 1; i <= calcEndDay; ++i) {
                        state.dataThermalComforts->avgDryBulbASH = 0.0;
                        for (j = 1; j <= 24; ++j) {
                            epwLine = epwFile.readLine().data;
                            for (ind = 1; ind <= 6; ++ind) {
                                pos = index(epwLine, ',');
                                epwLine.erase(0, pos + 1);
                            }
                            pos = index(epwLine, ',');
                            dryBulb = StrToReal(epwLine.substr(0, pos));
                            state.dataThermalComforts->avgDryBulbASH += (dryBulb / 24.0);
                        }
                        state.dataThermalComforts->DailyAveOutTemp(i + 30 - calcEndDay) = state.dataThermalComforts->avgDryBulbASH;
                    }
                    for (i = calcEndHr + 1; i <= calcStartHr - 1; ++i) {
                        epwLine = epwFile.readLine().data;
                    }
                    for (i = 1; i <= 30 - calcEndDay; ++i) {
                        state.dataThermalComforts->avgDryBulbASH = 0.0;
                        for (j = 1; j <= 24; ++j) {
                            epwLine = epwFile.readLine().data;
                            for (ind = 1; ind <= 6; ++ind) {
                                pos = index(epwLine, ',');
                                epwLine.erase(0, pos + 1);
                            }
                            pos = index(epwLine, ',');
                            dryBulb = StrToReal(epwLine.substr(0, pos));
                            state.dataThermalComforts->avgDryBulbASH += (dryBulb / 24.0);
                        }
                        state.dataThermalComforts->DailyAveOutTemp(i) = state.dataThermalComforts->avgDryBulbASH;
                    }
                }
                state.dataThermalComforts->useEpwData = true;
            }
        } else if (initiate && !weathersimulation) {
            state.dataThermalComforts->runningAverageASH = inavgdrybulb;
            state.dataThermalComforts->monthlyTemp = inavgdrybulb;
            state.dataThermalComforts->avgDryBulbASH = 0.0;
        }

        if (initiate) {
            return;
        }

        if (state.dataGlobal->BeginDayFlag && state.dataThermalComforts->useEpwData) {
            // Update the running average, reset the daily avg
            state.dataThermalComforts->DailyAveOutTemp(30) = state.dataThermalComforts->avgDryBulbASH;
            Real64 sum = 0.0;
            for (i = 1; i <= 29; i++) {
                sum += state.dataThermalComforts->DailyAveOutTemp(i);
            }
            state.dataThermalComforts->runningAverageASH = (sum + state.dataThermalComforts->avgDryBulbASH) / 30.0;
            for (i = 1; i <= 29; i++) {
                state.dataThermalComforts->DailyAveOutTemp(i) = state.dataThermalComforts->DailyAveOutTemp(i + 1);
            }
            state.dataThermalComforts->avgDryBulbASH = 0.0;
        }

        // If exists BeginMonthFlag we can use it to call InvJulianDay once per month.
        if (state.dataGlobal->BeginDayFlag && state.dataThermalComforts->useStatData) {
            //  CALL InvJulianDay(DayOfYear,pMonth,pDay,0)
            //  runningAverageASH = monthlyTemp(pMonth)
            state.dataThermalComforts->runningAverageASH = state.dataThermalComforts->monthlyTemp(state.dataEnvrn->Month);
        }

        // Update the daily average
        // IF (BeginHourFlag .and. useEpwData) THEN
        if (state.dataGlobal->BeginHourFlag) {
            state.dataThermalComforts->avgDryBulbASH += (state.dataEnvrn->OutDryBulbTemp / 24.0);
        }

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) {

            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            if (!people.AdaptiveASH55) {
                continue;
            }

            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

            state.dataThermalComforts->ZoneNum = people.ZonePtr;
            state.dataThermalComforts->AirTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataThermalComforts->ZoneNum).ZTAVComf;
            if (state.dataRoomAir->anyNonMixingRoomAirModel) {
                if (state.dataRoomAir->IsZoneDispVent3Node(state.dataThermalComforts->ZoneNum) ||
                    state.dataRoomAir->IsZoneUFAD(state.dataThermalComforts->ZoneNum)) {
                    state.dataThermalComforts->AirTemp = state.dataRoomAir->TCMF(state.dataThermalComforts->ZoneNum);
                }
            }
            state.dataThermalComforts->RadTemp = CalcRadTemp(state, state.dataThermalComforts->PeopleNum);
            state.dataThermalComforts->OpTemp = (state.dataThermalComforts->AirTemp + state.dataThermalComforts->RadTemp) / 2.0;
            comfort.ThermalComfortOpTemp = state.dataThermalComforts->OpTemp;
            comfort.ASHRAE55RunningMeanOutdoorTemp = state.dataThermalComforts->runningAverageASH;
            if (state.dataThermalComforts->runningAverageASH >= 10.0 && state.dataThermalComforts->runningAverageASH <= 33.5) {
                // Calculate the comfort here  (people/output handling loop)
                numOccupants = people.NumberOfPeople * people.sched->getCurrentVal();
                tComf = 0.31 * state.dataThermalComforts->runningAverageASH + 17.8;
                comfort.TComfASH55 = tComf;
                if (numOccupants > 0) {
                    if (state.dataThermalComforts->OpTemp < tComf + 2.5 && state.dataThermalComforts->OpTemp > tComf - 2.5) {
                        // 80% and 90% limits okay
                        comfort.ThermalComfortAdaptiveASH5590 = 1;
                        comfort.ThermalComfortAdaptiveASH5580 = 1;
                    } else if (state.dataThermalComforts->OpTemp < tComf + 3.5 && state.dataThermalComforts->OpTemp > tComf - 3.5) {
                        // 80% only
                        comfort.ThermalComfortAdaptiveASH5590 = 0;
                        comfort.ThermalComfortAdaptiveASH5580 = 1;
                        people.TimeNotMetASH5590 += SysTimeElapsed;
                    } else {
                        // Neither
                        comfort.ThermalComfortAdaptiveASH5590 = 0;
                        comfort.ThermalComfortAdaptiveASH5580 = 0;
                        people.TimeNotMetASH5580 += SysTimeElapsed;
                        people.TimeNotMetASH5590 += SysTimeElapsed;
                    }
                } else {
                    // Unoccupied
                    comfort.ThermalComfortAdaptiveASH5590 = -1;
                    comfort.ThermalComfortAdaptiveASH5580 = -1;
                }
            } else {
                // Monthly temp out of range
                comfort.ThermalComfortAdaptiveASH5590 = -1;
                comfort.ThermalComfortAdaptiveASH5580 = -1;
                comfort.TComfASH55 = -1.0;
            }
        }
    }

    void CalcThermalComfortAdaptiveCEN15251(
        EnergyPlusData &state,
        bool const initiate,                         // true if supposed to initiate
        ObjexxFCL::Optional_bool_const wthrsim,      // true if this is a weather simulation
        ObjexxFCL::Optional<Real64 const> avgdrybulb // approximate avg drybulb for design day.  will be used as previous period in design day
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Tyler Hoyt
        //       DATE WRITTEN   July 2011

        // PURPOSE OF THIS SUBROUTINE:
        // Sets up and carries out CEN-15251 adaptive comfort model calculations.
        // Output provided are state variables for the Category I, II, and III
        // limits of the model, the comfort temperature, and the 5-day weighted
        // moving average of the outdoor air temperature.

        // Using/Aliasing
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        using OutputReportTabular::GetColumnUsingTabs;
        using OutputReportTabular::StrToReal;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static Real64 constexpr alpha(0.8);
        static constexpr std::array<Real64, 7> alpha_pow = {0.262144, 0.32768, 0.4096, 0.512, 0.64, 0.8, 1.0}; // alpha^(6-0)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string epwLine;
        Real64 dryBulb;
        Real64 tComf;
        Real64 tComfLow;
        Real64 numOccupants;
        int readStat;
        int jStartDay;
        int calcStartDay;
        int calcStartHr;
        int calcEndDay;
        int calcEndHr;
        std::string::size_type pos;
        int ind;
        int i;
        int j;
        bool weathersimulation;
        Real64 inavgdrybulb;
        int constexpr numHeaderRowsInEpw = 8;

        if (initiate) { // not optional on initiate=true.  would otherwise check for presence
            weathersimulation = wthrsim;
            inavgdrybulb = avgdrybulb;
            state.dataThermalComforts->avgDryBulbCEN = 0.0;
            state.dataThermalComforts->runningAverageCEN = 0.0;
        } else {
            weathersimulation = false;
            inavgdrybulb = 0.0;
        }

        if (initiate && weathersimulation) {
            const bool epwFileExists = FileSystem::fileExists(state.files.inputWeatherFilePath.filePath);
            readStat = 0;
            if (epwFileExists) {
                // determine number of days in year
                int DaysInYear;
                if (state.dataEnvrn->CurrentYearIsLeapYear) {
                    DaysInYear = 366;
                } else {
                    DaysInYear = 365;
                }

                auto epwFile = state.files.inputWeatherFilePath.open(state, "CalcThermalComfortAdaptiveCEN15251");
                for (i = 1; i <= numHeaderRowsInEpw; ++i) {
                    epwFile.readLine();
                }
                jStartDay = state.dataEnvrn->DayOfYear - 1;
                calcStartDay = jStartDay - 7;
                if (calcStartDay > 0) {
                    calcStartHr = 24 * calcStartDay + 1;
                    for (i = 1; i <= calcStartHr - 1; ++i) {
                        epwFile.readLine();
                    }
                    state.dataThermalComforts->runningAverageCEN = 0.0;
                    for (i = 0; i < 7; ++i) {
                        state.dataThermalComforts->avgDryBulbCEN = 0.0;
                        for (j = 1; j <= 24; ++j) {
                            epwLine = epwFile.readLine().data;
                            for (ind = 1; ind <= 6; ++ind) {
                                pos = index(epwLine, ',');
                                epwLine.erase(0, pos + 1);
                            }
                            pos = index(epwLine, ',');
                            dryBulb = StrToReal(epwLine.substr(0, pos));
                            state.dataThermalComforts->avgDryBulbCEN += (dryBulb / 24.0);
                        }
                        state.dataThermalComforts->runningAverageCEN += alpha_pow[i] * state.dataThermalComforts->avgDryBulbCEN;
                    }
                } else { // Do special things for wrapping the epw
                    calcEndDay = jStartDay;
                    calcStartDay += DaysInYear;
                    calcEndHr = 24 * calcEndDay;
                    calcStartHr = 24 * calcStartDay + 1;
                    for (i = 1; i <= calcEndDay; ++i) {
                        state.dataThermalComforts->avgDryBulbCEN = 0.0;
                        for (j = 1; j <= 24; ++j) {
                            epwLine = epwFile.readLine().data;
                            for (ind = 1; ind <= 6; ++ind) {
                                pos = index(epwLine, ',');
                                epwLine.erase(0, pos + 1);
                            }
                            pos = index(epwLine, ',');
                            dryBulb = StrToReal(epwLine.substr(0, pos));
                            state.dataThermalComforts->avgDryBulbCEN += (dryBulb / 24.0);
                        }
                        state.dataThermalComforts->runningAverageCEN += std::pow(alpha, calcEndDay - i) * state.dataThermalComforts->avgDryBulbCEN;
                    }
                    for (i = calcEndHr + 1; i <= calcStartHr - 1; ++i) {
                        epwFile.readLine();
                    }
                    for (i = 0; i < 7 - calcEndDay; ++i) {
                        state.dataThermalComforts->avgDryBulbCEN = 0.0;
                        for (j = 1; j <= 24; ++j) {
                            epwLine = epwFile.readLine().data;
                            for (ind = 1; ind <= 6; ++ind) {
                                pos = index(epwLine, ',');
                                epwLine.erase(0, pos + 1);
                            }
                            pos = index(epwLine, ',');
                            dryBulb = StrToReal(epwLine.substr(0, pos));
                            state.dataThermalComforts->avgDryBulbCEN += (dryBulb / 24.0);
                        }
                        state.dataThermalComforts->runningAverageCEN += alpha_pow[i] * state.dataThermalComforts->avgDryBulbCEN;
                    }
                }
                state.dataThermalComforts->runningAverageCEN *= (1.0 - alpha);
                state.dataThermalComforts->avgDryBulbCEN = 0.0;
                state.dataThermalComforts->useEpwDataCEN = true;
                state.dataThermalComforts->firstDaySet = true;
            }
        } else if (initiate && !weathersimulation) {
            state.dataThermalComforts->runningAverageCEN = inavgdrybulb;
            state.dataThermalComforts->avgDryBulbCEN = 0.0;
        }
        if (initiate) {
            return;
        }

        if (state.dataGlobal->BeginDayFlag && !state.dataThermalComforts->firstDaySet) {
            // Update the running average, reset the daily avg
            state.dataThermalComforts->runningAverageCEN =
                alpha * state.dataThermalComforts->runningAverageCEN + (1.0 - alpha) * state.dataThermalComforts->avgDryBulbCEN;
            state.dataThermalComforts->avgDryBulbCEN = 0.0;
        }

        state.dataThermalComforts->firstDaySet = false;

        // Update the daily average
        if (state.dataGlobal->BeginHourFlag) {
            state.dataThermalComforts->avgDryBulbCEN += (state.dataEnvrn->OutDryBulbTemp / 24.0);
        }

        for (state.dataThermalComforts->PeopleNum = 1; state.dataThermalComforts->PeopleNum <= state.dataHeatBal->TotPeople;
             ++state.dataThermalComforts->PeopleNum) {
            auto &people = state.dataHeatBal->People(state.dataThermalComforts->PeopleNum);
            if (!people.AdaptiveCEN15251) {
                continue;
            }

            auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);
            state.dataThermalComforts->ZoneNum = people.ZonePtr;
            state.dataThermalComforts->AirTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataThermalComforts->ZoneNum).ZTAVComf;
            if (state.dataRoomAir->anyNonMixingRoomAirModel) {
                if (state.dataRoomAir->IsZoneDispVent3Node(state.dataThermalComforts->ZoneNum) ||
                    state.dataRoomAir->IsZoneUFAD(state.dataThermalComforts->ZoneNum)) {
                    state.dataThermalComforts->AirTemp = state.dataRoomAir->TCMF(state.dataThermalComforts->ZoneNum);
                }
            }
            state.dataThermalComforts->RadTemp = CalcRadTemp(state, state.dataThermalComforts->PeopleNum);
            state.dataThermalComforts->OpTemp = (state.dataThermalComforts->AirTemp + state.dataThermalComforts->RadTemp) / 2.0;
            comfort.ThermalComfortOpTemp = state.dataThermalComforts->OpTemp;
            comfort.CEN15251RunningMeanOutdoorTemp = state.dataThermalComforts->runningAverageCEN;
            if (state.dataThermalComforts->runningAverageCEN >= 10.0 && state.dataThermalComforts->runningAverageCEN <= 30.0) {
                // Calculate the comfort here (people/output handling loop)
                numOccupants = people.NumberOfPeople * people.sched->getCurrentVal();
                tComf = 0.33 * state.dataThermalComforts->runningAverageCEN + 18.8;
                comfort.TComfCEN15251 = tComf;
                if (numOccupants > 0) {
                    if (state.dataThermalComforts->runningAverageCEN < 15) {
                        tComfLow = 23.75; // Lower limit is constant in this region
                    } else {
                        tComfLow = tComf;
                    }
                    if (state.dataThermalComforts->OpTemp < tComf + 2.0 && state.dataThermalComforts->OpTemp > tComfLow - 2.0) {
                        // Within Cat I, II, III Limits
                        comfort.ThermalComfortAdaptiveCEN15251CatI = 1;
                        comfort.ThermalComfortAdaptiveCEN15251CatII = 1;
                        comfort.ThermalComfortAdaptiveCEN15251CatIII = 1;
                    } else if (state.dataThermalComforts->OpTemp < tComf + 3.0 && state.dataThermalComforts->OpTemp > tComfLow - 3.0) {
                        // Within Cat II, III Limits
                        comfort.ThermalComfortAdaptiveCEN15251CatI = 0;
                        comfort.ThermalComfortAdaptiveCEN15251CatII = 1;
                        comfort.ThermalComfortAdaptiveCEN15251CatIII = 1;
                        people.TimeNotMetCEN15251CatI += SysTimeElapsed;
                    } else if (state.dataThermalComforts->OpTemp < tComf + 4.0 && state.dataThermalComforts->OpTemp > tComfLow - 4.0) {
                        // Within Cat III Limits
                        comfort.ThermalComfortAdaptiveCEN15251CatI = 0;
                        comfort.ThermalComfortAdaptiveCEN15251CatII = 0;
                        comfort.ThermalComfortAdaptiveCEN15251CatIII = 1;
                        people.TimeNotMetCEN15251CatI += SysTimeElapsed;
                        people.TimeNotMetCEN15251CatII += SysTimeElapsed;
                    } else {
                        // None
                        comfort.ThermalComfortAdaptiveCEN15251CatI = 0;
                        comfort.ThermalComfortAdaptiveCEN15251CatII = 0;
                        comfort.ThermalComfortAdaptiveCEN15251CatIII = 0;
                        people.TimeNotMetCEN15251CatI += SysTimeElapsed;
                        people.TimeNotMetCEN15251CatII += SysTimeElapsed;
                        people.TimeNotMetCEN15251CatIII += SysTimeElapsed;
                    }
                } else {
                    // Unoccupied
                    comfort.ThermalComfortAdaptiveCEN15251CatI = -1;
                    comfort.ThermalComfortAdaptiveCEN15251CatII = -1;
                    comfort.ThermalComfortAdaptiveCEN15251CatIII = -1;
                }
            } else {
                // Monthly temp out of range
                comfort.ThermalComfortAdaptiveCEN15251CatI = -1;
                comfort.ThermalComfortAdaptiveCEN15251CatII = -1;
                comfort.ThermalComfortAdaptiveCEN15251CatIII = -1;
                comfort.TComfCEN15251 = -1.0;
            }
        }
    }

    void DynamicClothingModel(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Kwang Ho Lee
        //       DATE WRITTEN   June 2013

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TemporaryVariable; // LOL

        auto &comfort = state.dataThermalComforts->ThermalComfortData(state.dataThermalComforts->PeopleNum);

        if (state.dataThermalComforts->TemporarySixAMTemperature < -5.0) { // A Temporary state variable?
            comfort.ClothingValue = 1.0;
        } else if ((state.dataThermalComforts->TemporarySixAMTemperature >= -5.0) && (state.dataThermalComforts->TemporarySixAMTemperature < 5.0)) {
            comfort.ClothingValue = 0.818 - 0.0364 * state.dataThermalComforts->TemporarySixAMTemperature;
        } else if ((state.dataThermalComforts->TemporarySixAMTemperature >= 5.0) && (state.dataThermalComforts->TemporarySixAMTemperature < 26.0)) {
            TemporaryVariable = -0.1635 - 0.0066 * state.dataThermalComforts->TemporarySixAMTemperature;
            comfort.ClothingValue = std::pow(10.0, TemporaryVariable);
        } else if (state.dataThermalComforts->TemporarySixAMTemperature >= 26.0) {
            comfort.ClothingValue = 0.46;
        }
    }

} // namespace ThermalComfort

} // namespace EnergyPlus
