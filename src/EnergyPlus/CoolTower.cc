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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CoolTower.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace CoolTower {
    // Module containing the data for cooltower system

    // MODULE INFORMATION:
    //       AUTHOR         Daeho Kang
    //       DATE WRITTEN   Aug 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to manage the cooltower component.

    // REFERENCES:
    // Baruch Givoni. 1994. Passive and Low Energy Cooling of Buildings. Chapter 5: Evaporative Cooling Systems.
    //     John Wiley & Sons, Inc.
    // OTHER NOTES: none

    // Using/Aliasing
    using namespace DataHeatBalance;

    constexpr std::array<std::string_view, static_cast<int>(FlowCtrl::Num)> FlowCtrlNamesUC{"WATERFLOWSCHEDULE", "WINDDRIVENFLOW"};

    void ManageCoolTower(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Aug 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the simulation of Cooltower component.
        // This driver manages the calls to all of the other drivers and simulation algorithms.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // Obtains and allocates heat balance related parameters from input
        if (state.dataCoolTower->GetInputFlag) {
            GetCoolTower(state);
            state.dataCoolTower->GetInputFlag = false;
        }

        if ((int)state.dataCoolTower->CoolTowerSys.size() == 0) {
            return;
        }

        CalcCoolTower(state);

        UpdateCoolTower(state);

        ReportCoolTower(state);
    }

    void GetCoolTower(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Aug 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets input data for cooltower components
        // and stores it in the Cooltower data structure.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName = "GetCoolTower";

        static std::string const CurrentModuleObject("ZoneCoolTower:Shower");
        Real64 constexpr MaximumWaterFlowRate(0.016667); // Maximum limit of water flow rate in m3/s (1000 l/min)
        Real64 constexpr MinimumWaterFlowRate(0.0);      // Minimum limit of water flow rate
        Real64 constexpr MaxHeight(30.0);                // Maximum effective tower height in m
        Real64 constexpr MinHeight(1.0);                 // Minimum effective tower height in m
        Real64 constexpr MaxValue(100.0);                // Maximum limit of outlet area, airflow, and temperature
        Real64 constexpr MinValue(0.0);                  // Minimum limit of outlet area, airflow, and temperature
        Real64 constexpr MaxFrac(1.0);                   // Maximum fraction
        Real64 constexpr MinFrac(0.0);                   // Minimum fraction

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // If errors detected in input
        int NumAlphas;           // Number of Alphas for each GetobjectItem call
        int NumNumbers;          // Number of Numbers for each GetobjectItem call
        int NumArgs;
        int IOStat;
        Array1D_string cAlphaArgs;     // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> rNumericArgs;  // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.

        // Initializations and allocations
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumArgs, NumAlphas, NumNumbers);
        cAlphaArgs.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        rNumericArgs.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        auto &Zone(state.dataHeatBal->Zone);

        auto &s_ipsc = state.dataIPShortCut;

        int NumCoolTowers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        state.dataCoolTower->CoolTowerSys.allocate(NumCoolTowers);

        // Obtain inputs
        for (int CoolTowerNum = 1; CoolTowerNum <= NumCoolTowers; ++CoolTowerNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CoolTowerNum,
                                                                     s_ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).Name = s_ipsc->cAlphaArgs(1); // Name of cooltower
            if (lAlphaBlanks(2)) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataCoolTower->CoolTowerSys(CoolTowerNum).availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(3), Zone);
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->space);
            if ((state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr == 0) && (state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr == 0)) {
                if (lAlphaBlanks(3)) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\" invalid {} is required but input is blank.", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cAlphaFields(3)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                           CurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           cAlphaFields(3),
                                           s_ipsc->cAlphaArgs(3)));
                }
                ErrorsFound = true;
            } else if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr == 0) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr =
                    state.dataHeatBal->space(state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr).zoneNum;
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyName = s_ipsc->cAlphaArgs(4); // Name of water storage tank
            if (lAlphaBlanks(4)) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyMode = WaterSupplyMode::FromMains;
            } else if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                WaterManager::SetupTankDemandComponent(state,
                                                       state.dataCoolTower->CoolTowerSys(CoolTowerNum).Name,
                                                       CurrentModuleObject,
                                                       state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyName,
                                                       ErrorsFound,
                                                       state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupTankID,
                                                       state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterTankDemandARRID);
            }

            {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).FlowCtrlType =
                    static_cast<FlowCtrl>(getEnumValue(FlowCtrlNamesUC, s_ipsc->cAlphaArgs(5))); // Type of flow control
                if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FlowCtrlType == FlowCtrl::Invalid) {
                    ShowSevereError(
                        state,
                        format("{}=\"{}\" invalid {}=\"{}\".", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cAlphaFields(5), s_ipsc->cAlphaArgs(5)));
                    ErrorsFound = true;
                }
            }

            if ((state.dataCoolTower->CoolTowerSys(CoolTowerNum).pumpSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), s_ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate = s_ipsc->rNumericArgs(1); // Maximum limit of water supply
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate > MaximumWaterFlowRate) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate = MaximumWaterFlowRate;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(1), s_ipsc->rNumericArgs(1)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaximumWaterFlowRate));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate < MinimumWaterFlowRate) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate = MinimumWaterFlowRate;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(1), s_ipsc->rNumericArgs(1)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinimumWaterFlowRate));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight = s_ipsc->rNumericArgs(2); // Get effctive tower height
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight > MaxHeight) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight = MaxHeight;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(2), s_ipsc->rNumericArgs(2)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaxHeight));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight < MinHeight) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight = MinHeight;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(2), s_ipsc->rNumericArgs(2)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinHeight));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletArea = s_ipsc->rNumericArgs(3); // Get outlet area
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletArea > MaxValue) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletArea = MaxValue;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(3), s_ipsc->rNumericArgs(3)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaxValue));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletArea < MinValue) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletArea = MinValue;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(3), s_ipsc->rNumericArgs(3)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinValue));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate = s_ipsc->rNumericArgs(4); // Maximum limit of air flow to the space
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate > MaxValue) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate = MaxValue;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(4), s_ipsc->rNumericArgs(4)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaxValue));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate < MinValue) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate = MinValue;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(4), s_ipsc->rNumericArgs(4)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinValue));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).MinZoneTemp =
                s_ipsc->rNumericArgs(5); // Get minimum temp limit which gets this cooltower off
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MinZoneTemp > MaxValue) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).MinZoneTemp = MaxValue;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(5), s_ipsc->rNumericArgs(5)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaxValue));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MinZoneTemp < MinValue) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).MinZoneTemp = MinValue;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(5), s_ipsc->rNumericArgs(5)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinValue));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss = s_ipsc->rNumericArgs(6); // Fraction of water loss
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss > MaxFrac) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss = MaxFrac;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(6), s_ipsc->rNumericArgs(6)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaxFrac));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss < MinFrac) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss = MinFrac;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(6), s_ipsc->rNumericArgs(6)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinFrac));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched = s_ipsc->rNumericArgs(7); // Fraction of loss of air flow
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched > MaxFrac) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched = MaxFrac;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.2R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(7), s_ipsc->rNumericArgs(7)));
                ShowContinueError(state, format("...Maximum Allowable=[{:.2R}].", MaxFrac));
            }
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched < MinFrac) {
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched = MinFrac;
                ShowWarningError(
                    state,
                    format("{}=\"{}\" invalid {}=[{:.5R}].", CurrentModuleObject, s_ipsc->cAlphaArgs(1), cNumericFields(7), s_ipsc->rNumericArgs(7)));
                ShowContinueError(state, format("...Minimum Allowable=[{:.2R}].", MinFrac));
            }

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).RatedPumpPower = s_ipsc->rNumericArgs(8); // Get rated pump power
        }

        cAlphaArgs.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        rNumericArgs.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{} errors occurred in input.  Program terminates.", CurrentModuleObject));
        }

        for (int CoolTowerNum = 1; CoolTowerNum <= NumCoolTowers; ++CoolTowerNum) {
            SetupOutputVariable(state,
                                "Zone Cooltower Sensible Heat Loss Energy",
                                Constant::Units::J,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).SenHeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Sensible Heat Loss Rate",
                                Constant::Units::W,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).SenHeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Latent Heat Loss Energy",
                                Constant::Units::J,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).LatHeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Latent Heat Loss Rate",
                                Constant::Units::W,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).LatHeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Volume",
                                Constant::Units::m3,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTAirVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Current Density Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Standard Density Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRateStd,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Mass",
                                Constant::Units::kg,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTAirMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Inlet Temperature",
                                Constant::Units::C,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletDBTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Inlet Humidity Ratio",
                                Constant::Units::kgWater_kgDryAir,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletHumRat,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Outlet Temperature",
                                Constant::Units::C,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Outlet Humidity Ratio",
                                Constant::Units::kgWater_kgDryAir,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletHumRat,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Pump Electricity Rate",
                                Constant::Units::W,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).PumpElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Pump Electricity Energy",
                                Constant::Units::J,
                                state.dataCoolTower->CoolTowerSys(CoolTowerNum).PumpElecConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC, // System
                                OutputProcessor::EndUseCat::Cooling);
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyMode == WaterSupplyMode::FromMains) {
                SetupOutputVariable(state,
                                    "Zone Cooltower Water Volume",
                                    Constant::Units::m3,
                                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Cooltower Mains Water Volume",
                                    Constant::Units::m3,
                                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name,
                                    Constant::eResource::MainsWater,
                                    OutputProcessor::Group::HVAC, // System
                                    OutputProcessor::EndUseCat::Cooling);
            } else if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                SetupOutputVariable(state,
                                    "Zone Cooltower Water Volume",
                                    Constant::Units::m3,
                                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Cooltower Storage Tank Water Volume",
                                    Constant::Units::m3,
                                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Cooltower Starved Mains Water Volume",
                                    Constant::Units::m3,
                                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterStarvMakeup,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    Zone(state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr).Name,
                                    Constant::eResource::MainsWater,
                                    OutputProcessor::Group::HVAC, // System
                                    OutputProcessor::EndUseCat::Cooling);
            }
        }
    }

    void CalcCoolTower(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Aug 2008

        // REFERENCES:
        // Baruch Givoni. 1994. Passive and Low Energy Cooling of Buildings. Chapter 5: Evaporative Cooling Systems.
        //     John Wiley & Sons, Inc.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MinWindSpeed(0.1);  // Minimum limit of outdoor air wind speed in m/s
        Real64 constexpr MaxWindSpeed(30.0); // Maximum limit of outdoor air wind speed in m/s
        Real64 constexpr UCFactor(60000.0);  // Unit conversion factor m3/s to l/min

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CVF_ZoneNum;          // Design flow rate in m3/s
        Real64 AirMassFlowRate;      // Actual air mass flow rate in kg/s
        Real64 AirSpecHeat;          // Specific heat of air
        Real64 AirDensity;           // Density of air
        Real64 RhoWater;             // Density of water
        Real64 PumpPartLoadRat;      // Pump part load ratio (based on user schedule, or 1.0 for no schedule)
        Real64 WaterFlowRate = 0.0;  // Calculated water flow rate in m3/s
        Real64 AirVolFlowRate = 0.0; // Calculated air volume flow rate in m3/s
        Real64 InletHumRat;          // Humidity ratio of outdoor air
        Real64 OutletHumRat;         // Humidity ratio of air at the cooltower outlet
        Real64 OutletTemp = 0.0;     // Dry bulb temperature of air at the cooltower outlet
        Real64 IntHumRat;            // Humidity ratio of initialized air

        auto &Zone(state.dataHeatBal->Zone);

        for (int CoolTowerNum = 1; CoolTowerNum <= (int)state.dataCoolTower->CoolTowerSys.size(); ++CoolTowerNum) {
            int const ZoneNum = state.dataCoolTower->CoolTowerSys(CoolTowerNum).ZonePtr;
            auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
            thisZoneHB.MCPTC = 0.0;
            thisZoneHB.MCPC = 0.0;
            thisZoneHB.CTMFL = 0.0;
            if ((state.dataHeatBal->doSpaceHeatBalance) && (state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr > 0)) {
                auto &thisSpaceHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr);
                thisSpaceHB.MCPTC = 0.0;
                thisSpaceHB.MCPC = 0.0;
                thisSpaceHB.CTMFL = 0.0;
            }

            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).availSched->getCurrentVal() > 0.0) {
                // check component operation
                if (state.dataEnvrn->WindSpeed < MinWindSpeed || state.dataEnvrn->WindSpeed > MaxWindSpeed) {
                    continue;
                }
                if (state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT <
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).MinZoneTemp) {
                    continue;
                }

                // Unit is on and simulate this component
                // Determine the temperature and air flow rate at the cooltower outlet
                if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FlowCtrlType == FlowCtrl::WindDriven) {
                    Real64 const height_sqrt(std::sqrt(state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight));
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletVelocity = 0.7 * height_sqrt + 0.47 * (state.dataEnvrn->WindSpeed - 1.0);
                    AirVolFlowRate =
                        state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletArea * state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletVelocity;
                    AirVolFlowRate = min(AirVolFlowRate, state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate);
                    WaterFlowRate = (AirVolFlowRate / (0.0125 * height_sqrt));
                    if (WaterFlowRate > state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate * UCFactor) {
                        WaterFlowRate = state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate * UCFactor;
                        AirVolFlowRate = 0.0125 * WaterFlowRate * height_sqrt;
                        AirVolFlowRate = min(AirVolFlowRate, state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate);
                    }
                    WaterFlowRate = min(WaterFlowRate, (state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate * UCFactor));
                    OutletTemp =
                        state.dataEnvrn->OutDryBulbTemp - (state.dataEnvrn->OutDryBulbTemp - state.dataEnvrn->OutWetBulbTemp) *
                                                              (1.0 - std::exp(-0.8 * state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight)) *
                                                              (1.0 - std::exp(-0.15 * WaterFlowRate));
                } else if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FlowCtrlType == FlowCtrl::FlowSchedule) {
                    WaterFlowRate = state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxWaterFlowRate * UCFactor;
                    AirVolFlowRate = 0.0125 * WaterFlowRate * std::sqrt(state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight);
                    AirVolFlowRate = min(AirVolFlowRate, state.dataCoolTower->CoolTowerSys(CoolTowerNum).MaxAirVolFlowRate);
                    OutletTemp =
                        state.dataEnvrn->OutDryBulbTemp - (state.dataEnvrn->OutDryBulbTemp - state.dataEnvrn->OutWetBulbTemp) *
                                                              (1.0 - std::exp(-0.8 * state.dataCoolTower->CoolTowerSys(CoolTowerNum).TowerHeight)) *
                                                              (1.0 - std::exp(-0.15 * WaterFlowRate));
                }

                if (OutletTemp < state.dataEnvrn->OutWetBulbTemp) {
                    ShowSevereError(state, "Cooltower outlet temperature exceed the outdoor wet bulb temperature reset to input values");
                    ShowContinueError(state, format("Occurs in Cooltower ={}", state.dataCoolTower->CoolTowerSys(CoolTowerNum).Name));
                }

                WaterFlowRate /= UCFactor;
                // Determine actual water flow rate
                if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss > 0.0) {
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualWaterFlowRate =
                        WaterFlowRate * (1.0 + state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracWaterLoss);
                } else {
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualWaterFlowRate = WaterFlowRate;
                }

                // Determine actual air flow rate
                if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched > 0.0) {
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualAirVolFlowRate =
                        AirVolFlowRate * (1.0 - state.dataCoolTower->CoolTowerSys(CoolTowerNum).FracFlowSched);
                } else {
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualAirVolFlowRate = AirVolFlowRate;
                }

                // Determine pump power
                if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).pumpSched->getCurrentVal() > 0) {
                    PumpPartLoadRat = state.dataCoolTower->CoolTowerSys(CoolTowerNum).pumpSched->getCurrentVal();
                } else {
                    PumpPartLoadRat = 1.0;
                }

                // Determine air mass flow rate and volume flow rate
                InletHumRat = Psychrometrics::PsyWFnTdbTwbPb(
                    state, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutWetBulbTemp, state.dataEnvrn->OutBaroPress);
                // Assume no pressure drops and no changes in enthalpy between inlet and outlet air
                IntHumRat = Psychrometrics::PsyWFnTdbH(state, OutletTemp, state.dataEnvrn->OutEnthalpy); // Initialized humidity ratio
                AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, OutletTemp, IntHumRat);
                AirMassFlowRate = AirDensity * state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualAirVolFlowRate;
                // From the mass balance W_in*(m_air + m_water) = W_out*m_air
                RhoWater = Psychrometrics::RhoH2O(OutletTemp); // Assume T_water = T_outlet
                OutletHumRat = (InletHumRat * (AirMassFlowRate + (state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualWaterFlowRate * RhoWater))) /
                               AirMassFlowRate;
                AirSpecHeat = Psychrometrics::PsyCpAirFnW(OutletHumRat);
                AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, OutletTemp, OutletHumRat); // Outlet air density
                CVF_ZoneNum = state.dataCoolTower->CoolTowerSys(CoolTowerNum).ActualAirVolFlowRate *
                              state.dataCoolTower->CoolTowerSys(CoolTowerNum).availSched->getCurrentVal();
                Real64 thisMCPC = CVF_ZoneNum * AirDensity * AirSpecHeat;
                Real64 thisMCPTC = thisMCPC * OutletTemp;
                Real64 thisCTMFL = thisMCPC / AirSpecHeat;
                Real64 thisZT = thisZoneHB.ZT;
                Real64 thisAirHumRat = thisZoneHB.airHumRat;
                thisZoneHB.MCPC = thisMCPC;
                thisZoneHB.MCPTC = thisMCPTC;
                thisZoneHB.CTMFL = thisCTMFL;
                if ((state.dataHeatBal->doSpaceHeatBalance) && (state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr > 0)) {
                    auto &thisSpaceHB =
                        state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataCoolTower->CoolTowerSys(CoolTowerNum).spacePtr);
                    thisSpaceHB.MCPC = thisMCPC;
                    thisSpaceHB.MCPTC = thisMCPTC;
                    thisSpaceHB.CTMFL = thisCTMFL;
                    thisZT = thisSpaceHB.ZT;
                    thisAirHumRat = thisSpaceHB.airHumRat;
                }

                state.dataCoolTower->CoolTowerSys(CoolTowerNum).SenHeatPower = thisMCPC * std::abs(thisZT - OutletTemp);
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).LatHeatPower = CVF_ZoneNum * std::abs(thisAirHumRat - OutletHumRat);
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletTemp = OutletTemp;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletHumRat = OutletHumRat;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRate = CVF_ZoneNum;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirMassFlowRate = thisCTMFL;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRateStd = thisCTMFL / state.dataEnvrn->StdRhoAir;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletDBTemp = Zone(ZoneNum).OutDryBulbTemp;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletWBTemp = Zone(ZoneNum).OutWetBulbTemp;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletHumRat = state.dataEnvrn->OutHumRat;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate = (std::abs(InletHumRat - OutletHumRat) * thisCTMFL) / RhoWater;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterStarvMakeupRate = 0.0; // initialize -- calc in update
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).PumpElecPower =
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).RatedPumpPower * PumpPartLoadRat;
            } else { // Unit is off
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).SenHeatPower = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).LatHeatPower = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletTemp = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).OutletHumRat = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRate = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirMassFlowRate = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRateStd = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletDBTemp = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).InletHumRat = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).PumpElecPower = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate = 0.0;
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterStarvMakeupRate = 0.0;
            }
        }
    }

    void UpdateCoolTower(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   October 2000
        //       MODIFIED       Aug 2008 Daeho Kang

        for (int CoolTowerNum = 1; CoolTowerNum <= (int)state.dataCoolTower->CoolTowerSys.size(); ++CoolTowerNum) {

            // Set the demand request for supply water from water storage tank (if needed)
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                state.dataWaterData->WaterStorage(state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupTankID)
                    .VdotRequestDemand(state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterTankDemandARRID) =
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate;
            }

            // check if should be starved by restricted flow from tank
            if (state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                Real64 AvailWaterRate = state.dataWaterData->WaterStorage(state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterSupTankID)
                                            .VdotAvailDemand(state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterTankDemandARRID);
                if (AvailWaterRate < state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate) {
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterStarvMakeupRate =
                        state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate - AvailWaterRate;
                    state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate = AvailWaterRate;
                }
            }
        }
    }

    void ReportCoolTower(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Aut 2008

        Real64 const TSMult = state.dataHVACGlobal->TimeStepSysSec;

        for (int CoolTowerNum = 1; CoolTowerNum <= (int)state.dataCoolTower->CoolTowerSys.size(); ++CoolTowerNum) {

            state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTAirVol = state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirVolFlowRate * TSMult;
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTAirMass = state.dataCoolTower->CoolTowerSys(CoolTowerNum).AirMassFlowRate * TSMult;
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).SenHeatLoss = state.dataCoolTower->CoolTowerSys(CoolTowerNum).SenHeatPower * TSMult;
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).LatHeatLoss = state.dataCoolTower->CoolTowerSys(CoolTowerNum).LatHeatPower * TSMult;
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).PumpElecConsump = state.dataCoolTower->CoolTowerSys(CoolTowerNum).PumpElecPower * TSMult;
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsump =
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterConsumpRate * TSMult;
            state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterStarvMakeup =
                state.dataCoolTower->CoolTowerSys(CoolTowerNum).CoolTWaterStarvMakeupRate * TSMult;
        }
    }

} // namespace CoolTower

} // namespace EnergyPlus
