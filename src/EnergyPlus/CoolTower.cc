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

            auto &coolTower = state.dataCoolTower->CoolTowerSys(CoolTowerNum);

            coolTower.Name = s_ipsc->cAlphaArgs(1); // Name of cooltower
            if (lAlphaBlanks(2)) {
                coolTower.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((coolTower.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), s_ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(3)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(3));
                ErrorsFound = true;
            } else if ((coolTower.ZonePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->Zone)) == 0 &&
                       (coolTower.spacePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->space)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else if (coolTower.ZonePtr == 0) {
                coolTower.ZonePtr = state.dataHeatBal->space(coolTower.spacePtr).zoneNum;
            }

            coolTower.CoolTWaterSupplyName = s_ipsc->cAlphaArgs(4); // Name of water storage tank
            if (lAlphaBlanks(4)) {
                coolTower.CoolTWaterSupplyMode = WaterSupplyMode::FromMains;
            } else if (coolTower.CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                WaterManager::SetupTankDemandComponent(state,
                                                       coolTower.Name,
                                                       CurrentModuleObject,
                                                       coolTower.CoolTWaterSupplyName,
                                                       ErrorsFound,
                                                       coolTower.CoolTWaterSupTankID,
                                                       coolTower.CoolTWaterTankDemandARRID);
            }

            coolTower.FlowCtrlType = static_cast<FlowCtrl>(getEnumValue(FlowCtrlNamesUC, s_ipsc->cAlphaArgs(5))); // Type of flow control
            if (coolTower.FlowCtrlType == FlowCtrl::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(5), s_ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            }

            if ((coolTower.pumpSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), s_ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            }

            coolTower.MaxWaterFlowRate = s_ipsc->rNumericArgs(1); // Maximum limit of water supply
            if (coolTower.MaxWaterFlowRate > MaximumWaterFlowRate) {
                coolTower.MaxWaterFlowRate = MaximumWaterFlowRate;
                ShowWarningBadMax(state, eoh, cNumericFields(1), s_ipsc->rNumericArgs(1), Clusive::In, MaximumWaterFlowRate);
            }
            if (coolTower.MaxWaterFlowRate < MinimumWaterFlowRate) {
                coolTower.MaxWaterFlowRate = MinimumWaterFlowRate;
                ShowWarningBadMin(state, eoh, cNumericFields(1), s_ipsc->rNumericArgs(1), Clusive::In, MinimumWaterFlowRate);
            }

            coolTower.TowerHeight = s_ipsc->rNumericArgs(2); // Get effective tower height
            if (coolTower.TowerHeight > MaxHeight) {
                coolTower.TowerHeight = MaxHeight;
                ShowWarningBadMax(state, eoh, cNumericFields(2), s_ipsc->rNumericArgs(2), Clusive::In, MaxHeight);
            }

            if (coolTower.TowerHeight < MinHeight) {
                coolTower.TowerHeight = MinHeight;
                ShowWarningBadMin(state, eoh, cNumericFields(2), s_ipsc->rNumericArgs(2), Clusive::In, MinHeight);
            }

            coolTower.OutletArea = s_ipsc->rNumericArgs(3); // Get outlet area
            if (coolTower.OutletArea > MaxValue) {
                coolTower.OutletArea = MaxValue;
                ShowWarningBadMax(state, eoh, cNumericFields(3), s_ipsc->rNumericArgs(3), Clusive::In, MaxValue);
            }
            if (coolTower.OutletArea < MinValue) {
                coolTower.OutletArea = MinValue;
                ShowWarningBadMin(state, eoh, cNumericFields(3), s_ipsc->rNumericArgs(3), Clusive::In, MinValue);
            }

            coolTower.MaxAirVolFlowRate = s_ipsc->rNumericArgs(4); // Maximum limit of air flow to the space
            if (coolTower.MaxAirVolFlowRate > MaxValue) {
                coolTower.MaxAirVolFlowRate = MaxValue;
                ShowWarningBadMax(state, eoh, cNumericFields(4), s_ipsc->rNumericArgs(4), Clusive::In, MaxValue);
            }
            if (coolTower.MaxAirVolFlowRate < MinValue) {
                coolTower.MaxAirVolFlowRate = MinValue;
                ShowWarningBadMin(state, eoh, cNumericFields(4), s_ipsc->rNumericArgs(4), Clusive::In, MinValue);
            }

            coolTower.MinZoneTemp = s_ipsc->rNumericArgs(5); // Get minimum temp limit which gets this cooltower off
            if (coolTower.MinZoneTemp > MaxValue) {
                coolTower.MinZoneTemp = MaxValue;
                ShowWarningBadMax(state, eoh, cNumericFields(5), s_ipsc->rNumericArgs(5), Clusive::In, MaxValue);
            }
            if (coolTower.MinZoneTemp < MinValue) {
                coolTower.MinZoneTemp = MinValue;
                ShowWarningBadMin(state, eoh, cNumericFields(5), s_ipsc->rNumericArgs(5), Clusive::In, MinValue);
            }

            coolTower.FracWaterLoss = s_ipsc->rNumericArgs(6); // Fraction of water loss
            if (coolTower.FracWaterLoss > MaxFrac) {
                coolTower.FracWaterLoss = MaxFrac;
                ShowWarningBadMax(state, eoh, cNumericFields(6), s_ipsc->rNumericArgs(6), Clusive::In, MaxFrac);
            }
            if (coolTower.FracWaterLoss < MinFrac) {
                coolTower.FracWaterLoss = MinFrac;
                ShowWarningBadMin(state, eoh, cNumericFields(6), s_ipsc->rNumericArgs(6), Clusive::In, MinFrac);
            }

            coolTower.FracFlowSched = s_ipsc->rNumericArgs(7); // Fraction of loss of air flow
            if (coolTower.FracFlowSched > MaxFrac) {
                coolTower.FracFlowSched = MaxFrac;
                ShowWarningBadMax(state, eoh, cNumericFields(7), s_ipsc->rNumericArgs(7), Clusive::In, MaxFrac);
            }
            if (coolTower.FracFlowSched < MinFrac) {
                coolTower.FracFlowSched = MinFrac;
                ShowWarningBadMin(state, eoh, cNumericFields(7), s_ipsc->rNumericArgs(7), Clusive::In, MinFrac);
            }

            coolTower.RatedPumpPower = s_ipsc->rNumericArgs(8); // Get rated pump power
        }

        cAlphaArgs.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        rNumericArgs.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, EnergyPlus::format("{} errors occurred in input.  Program terminates.", CurrentModuleObject));
        }

        for (int CoolTowerNum = 1; CoolTowerNum <= NumCoolTowers; ++CoolTowerNum) {
            auto &coolTower = state.dataCoolTower->CoolTowerSys(CoolTowerNum);
            auto &zone = state.dataHeatBal->Zone(coolTower.ZonePtr);

            SetupOutputVariable(state,
                                "Zone Cooltower Sensible Heat Loss Energy",
                                Constant::Units::J,
                                coolTower.SenHeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Sensible Heat Loss Rate",
                                Constant::Units::W,
                                coolTower.SenHeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Latent Heat Loss Energy",
                                Constant::Units::J,
                                coolTower.LatHeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Latent Heat Loss Rate",
                                Constant::Units::W,
                                coolTower.LatHeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Volume",
                                Constant::Units::m3,
                                coolTower.CoolTAirVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Current Density Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                coolTower.AirVolFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Standard Density Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                coolTower.AirVolFlowRateStd,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Mass",
                                Constant::Units::kg,
                                coolTower.CoolTAirMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                coolTower.AirMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Inlet Temperature",
                                Constant::Units::C,
                                coolTower.InletDBTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Inlet Humidity Ratio",
                                Constant::Units::kgWater_kgDryAir,
                                coolTower.InletHumRat,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Outlet Temperature",
                                Constant::Units::C,
                                coolTower.OutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Air Outlet Humidity Ratio",
                                Constant::Units::kgWater_kgDryAir,
                                coolTower.OutletHumRat,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Pump Electricity Rate",
                                Constant::Units::W,
                                coolTower.PumpElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Cooltower Pump Electricity Energy",
                                Constant::Units::J,
                                coolTower.PumpElecConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC, // System
                                OutputProcessor::EndUseCat::Cooling);
            if (coolTower.CoolTWaterSupplyMode == WaterSupplyMode::FromMains) {
                SetupOutputVariable(state,
                                    "Zone Cooltower Water Volume",
                                    Constant::Units::m3,
                                    coolTower.CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Zone Cooltower Mains Water Volume",
                                    Constant::Units::m3,
                                    coolTower.CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name,
                                    Constant::eResource::MainsWater,
                                    OutputProcessor::Group::HVAC, // System
                                    OutputProcessor::EndUseCat::Cooling);
            } else if (coolTower.CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                SetupOutputVariable(state,
                                    "Zone Cooltower Water Volume",
                                    Constant::Units::m3,
                                    coolTower.CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Zone Cooltower Storage Tank Water Volume",
                                    Constant::Units::m3,
                                    coolTower.CoolTWaterConsump,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name);
                SetupOutputVariable(state,
                                    "Zone Cooltower Starved Mains Water Volume",
                                    Constant::Units::m3,
                                    coolTower.CoolTWaterStarvMakeup,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    zone.Name,
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
            auto &coolTower = state.dataCoolTower->CoolTowerSys(CoolTowerNum);
            int const ZoneNum = coolTower.ZonePtr;
            auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
            thisZoneHB.MCPTC = 0.0;
            thisZoneHB.MCPC = 0.0;
            thisZoneHB.CTMFL = 0.0;
            if ((state.dataHeatBal->doSpaceHeatBalance) && (coolTower.spacePtr > 0)) {
                auto &thisSpaceHB = state.dataZoneTempPredictorCorrector->spaceHeatBalance(coolTower.spacePtr);
                thisSpaceHB.MCPTC = 0.0;
                thisSpaceHB.MCPC = 0.0;
                thisSpaceHB.CTMFL = 0.0;
            }

            if (coolTower.availSched->getCurrentVal() > 0.0) {
                // check component operation
                if (state.dataEnvrn->WindSpeed < MinWindSpeed || state.dataEnvrn->WindSpeed > MaxWindSpeed) {
                    continue;
                }
                if (state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT < coolTower.MinZoneTemp) {
                    continue;
                }

                // Unit is on and simulate this component
                // Determine the temperature and air flow rate at the cooltower outlet
                if (coolTower.FlowCtrlType == FlowCtrl::WindDriven) {
                    Real64 const height_sqrt(std::sqrt(coolTower.TowerHeight));
                    coolTower.OutletVelocity = 0.7 * height_sqrt + 0.47 * (state.dataEnvrn->WindSpeed - 1.0);
                    AirVolFlowRate = coolTower.OutletArea * coolTower.OutletVelocity;
                    AirVolFlowRate = min(AirVolFlowRate, coolTower.MaxAirVolFlowRate);
                    WaterFlowRate = (AirVolFlowRate / (0.0125 * height_sqrt));
                    if (WaterFlowRate > coolTower.MaxWaterFlowRate * UCFactor) {
                        WaterFlowRate = coolTower.MaxWaterFlowRate * UCFactor;
                        AirVolFlowRate = 0.0125 * WaterFlowRate * height_sqrt;
                        AirVolFlowRate = min(AirVolFlowRate, coolTower.MaxAirVolFlowRate);
                    }
                    WaterFlowRate = min(WaterFlowRate, (coolTower.MaxWaterFlowRate * UCFactor));
                    OutletTemp = state.dataEnvrn->OutDryBulbTemp - (state.dataEnvrn->OutDryBulbTemp - state.dataEnvrn->OutWetBulbTemp) *
                                                                       (1.0 - std::exp(-0.8 * coolTower.TowerHeight)) *
                                                                       (1.0 - std::exp(-0.15 * WaterFlowRate));
                } else if (coolTower.FlowCtrlType == FlowCtrl::FlowSchedule) {
                    WaterFlowRate = coolTower.MaxWaterFlowRate * UCFactor;
                    AirVolFlowRate = 0.0125 * WaterFlowRate * std::sqrt(coolTower.TowerHeight);
                    AirVolFlowRate = min(AirVolFlowRate, coolTower.MaxAirVolFlowRate);
                    OutletTemp = state.dataEnvrn->OutDryBulbTemp - (state.dataEnvrn->OutDryBulbTemp - state.dataEnvrn->OutWetBulbTemp) *
                                                                       (1.0 - std::exp(-0.8 * coolTower.TowerHeight)) *
                                                                       (1.0 - std::exp(-0.15 * WaterFlowRate));
                }

                if (OutletTemp < state.dataEnvrn->OutWetBulbTemp) {
                    ShowSevereError(state, "Cooltower outlet temperature exceed the outdoor wet bulb temperature reset to input values");
                    ShowContinueError(state, EnergyPlus::format("Occurs in Cooltower ={}", coolTower.Name));
                }

                WaterFlowRate /= UCFactor;
                // Determine actual water flow rate
                if (coolTower.FracWaterLoss > 0.0) {
                    coolTower.ActualWaterFlowRate = WaterFlowRate * (1.0 + coolTower.FracWaterLoss);
                } else {
                    coolTower.ActualWaterFlowRate = WaterFlowRate;
                }

                // Determine actual air flow rate
                if (coolTower.FracFlowSched > 0.0) {
                    coolTower.ActualAirVolFlowRate = AirVolFlowRate * (1.0 - coolTower.FracFlowSched);
                } else {
                    coolTower.ActualAirVolFlowRate = AirVolFlowRate;
                }

                // Determine pump power
                if (coolTower.pumpSched->getCurrentVal() > 0) {
                    PumpPartLoadRat = coolTower.pumpSched->getCurrentVal();
                } else {
                    PumpPartLoadRat = 1.0;
                }

                // Determine air mass flow rate and volume flow rate
                InletHumRat = Psychrometrics::PsyWFnTdbTwbPb(
                    state, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutWetBulbTemp, state.dataEnvrn->OutBaroPress);
                // Assume no pressure drops and no changes in enthalpy between inlet and outlet air
                IntHumRat = Psychrometrics::PsyWFnTdbH(state, OutletTemp, state.dataEnvrn->OutEnthalpy); // Initialized humidity ratio
                AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, OutletTemp, IntHumRat);
                AirMassFlowRate = AirDensity * coolTower.ActualAirVolFlowRate;
                // From the mass balance W_in*(m_air + m_water) = W_out*m_air
                RhoWater = Psychrometrics::RhoH2O(OutletTemp); // Assume T_water = T_outlet
                OutletHumRat = (InletHumRat * (AirMassFlowRate + (coolTower.ActualWaterFlowRate * RhoWater))) / AirMassFlowRate;
                AirSpecHeat = Psychrometrics::PsyCpAirFnW(OutletHumRat);
                AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, OutletTemp, OutletHumRat); // Outlet air density
                CVF_ZoneNum = coolTower.ActualAirVolFlowRate * coolTower.availSched->getCurrentVal();
                Real64 thisMCPC = CVF_ZoneNum * AirDensity * AirSpecHeat;
                Real64 thisMCPTC = thisMCPC * OutletTemp;
                Real64 thisCTMFL = thisMCPC / AirSpecHeat;
                Real64 thisZT = thisZoneHB.ZT;
                Real64 thisAirHumRat = thisZoneHB.airHumRat;
                thisZoneHB.MCPC = thisMCPC;
                thisZoneHB.MCPTC = thisMCPTC;
                thisZoneHB.CTMFL = thisCTMFL;
                if ((state.dataHeatBal->doSpaceHeatBalance) && (coolTower.spacePtr > 0)) {
                    auto &thisSpaceHB = state.dataZoneTempPredictorCorrector->spaceHeatBalance(coolTower.spacePtr);
                    thisSpaceHB.MCPC = thisMCPC;
                    thisSpaceHB.MCPTC = thisMCPTC;
                    thisSpaceHB.CTMFL = thisCTMFL;
                    thisZT = thisSpaceHB.ZT;
                    thisAirHumRat = thisSpaceHB.airHumRat;
                }

                coolTower.SenHeatPower = thisMCPC * std::abs(thisZT - OutletTemp);
                coolTower.LatHeatPower = CVF_ZoneNum * std::abs(thisAirHumRat - OutletHumRat);
                coolTower.OutletTemp = OutletTemp;
                coolTower.OutletHumRat = OutletHumRat;
                coolTower.AirVolFlowRate = CVF_ZoneNum;
                coolTower.AirMassFlowRate = thisCTMFL;
                coolTower.AirVolFlowRateStd = thisCTMFL / state.dataEnvrn->StdRhoAir;
                coolTower.InletDBTemp = Zone(ZoneNum).OutDryBulbTemp;
                coolTower.InletWBTemp = Zone(ZoneNum).OutWetBulbTemp;
                coolTower.InletHumRat = state.dataEnvrn->OutHumRat;
                coolTower.CoolTWaterConsumpRate = (std::abs(InletHumRat - OutletHumRat) * thisCTMFL) / RhoWater;
                coolTower.CoolTWaterStarvMakeupRate = 0.0; // initialize -- calc in update
                coolTower.PumpElecPower = coolTower.RatedPumpPower * PumpPartLoadRat;
            } else { // Unit is off
                coolTower.SenHeatPower = 0.0;
                coolTower.LatHeatPower = 0.0;
                coolTower.OutletTemp = 0.0;
                coolTower.OutletHumRat = 0.0;
                coolTower.AirVolFlowRate = 0.0;
                coolTower.AirMassFlowRate = 0.0;
                coolTower.AirVolFlowRateStd = 0.0;
                coolTower.InletDBTemp = 0.0;
                coolTower.InletHumRat = 0.0;
                coolTower.PumpElecPower = 0.0;
                coolTower.CoolTWaterConsumpRate = 0.0;
                coolTower.CoolTWaterStarvMakeupRate = 0.0;
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

            auto &coolTower = state.dataCoolTower->CoolTowerSys(CoolTowerNum);
            // Set the demand request for supply water from water storage tank (if needed)
            if (coolTower.CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                state.dataWaterData->WaterStorage(coolTower.CoolTWaterSupTankID).VdotRequestDemand(coolTower.CoolTWaterTankDemandARRID) =
                    coolTower.CoolTWaterConsumpRate;
            }

            // check if should be starved by restricted flow from tank
            if (coolTower.CoolTWaterSupplyMode == WaterSupplyMode::FromTank) {
                Real64 AvailWaterRate =
                    state.dataWaterData->WaterStorage(coolTower.CoolTWaterSupTankID).VdotAvailDemand(coolTower.CoolTWaterTankDemandARRID);
                if (AvailWaterRate < coolTower.CoolTWaterConsumpRate) {
                    coolTower.CoolTWaterStarvMakeupRate = coolTower.CoolTWaterConsumpRate - AvailWaterRate;
                    coolTower.CoolTWaterConsumpRate = AvailWaterRate;
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
            auto &coolTower = state.dataCoolTower->CoolTowerSys(CoolTowerNum);

            coolTower.CoolTAirVol = coolTower.AirVolFlowRate * TSMult;
            coolTower.CoolTAirMass = coolTower.AirMassFlowRate * TSMult;
            coolTower.SenHeatLoss = coolTower.SenHeatPower * TSMult;
            coolTower.LatHeatLoss = coolTower.LatHeatPower * TSMult;
            coolTower.PumpElecConsump = coolTower.PumpElecPower * TSMult;
            coolTower.CoolTWaterConsump = coolTower.CoolTWaterConsumpRate * TSMult;
            coolTower.CoolTWaterStarvMakeup = coolTower.CoolTWaterStarvMakeupRate * TSMult;
        }
    }

} // namespace CoolTower

} // namespace EnergyPlus
