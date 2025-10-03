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
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/EvaporativeFluidCoolers.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>

namespace EnergyPlus {

namespace EvaporativeFluidCoolers {

    // MODULE INFORMATION:
    //       AUTHOR         Chandan Sharma
    //       DATE WRITTEN   May 2009
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Model the performance of evaporative fluid coolers

    // METHODOLOGY EMPLOYED:
    // Based on cooling tower by Shirey, Raustad: Dec 2000; Shirey, Sept 2002

    constexpr std::string_view cEvapFluidCooler_SingleSpeed("EvaporativeFluidCooler:SingleSpeed");
    constexpr std::string_view cEvapFluidCooler_TwoSpeed("EvaporativeFluidCooler:TwoSpeed");
    constexpr std::array<std::string_view, static_cast<int>(CapacityControl::Num)> controlNamesUC = {"FANCYCLING", "FLUIDBYPASS"};
    constexpr std::array<std::string_view, static_cast<int>(EvapLoss::Num)> evapLossNamesUC = {"LOSSFACTOR", "SATURATEDEXIT"};
    constexpr std::array<std::string_view, static_cast<int>(Blowdown::Num)> blowDownNamesUC = {"CONCENTRATIONRATIO", "SCHEDULEDRATE"};

    EvapFluidCoolerSpecs *
    EvapFluidCoolerSpecs::factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string const &objectName)
    {
        // Process the input data if it hasn't been done already
        if (state.dataEvapFluidCoolers->GetEvapFluidCoolerInputFlag) {
            GetEvapFluidCoolerInput(state);
            state.dataEvapFluidCoolers->GetEvapFluidCoolerInputFlag = false;
        }

        // Now look for this particular object
        auto thisObj = std::find_if(
            state.dataEvapFluidCoolers->SimpleEvapFluidCooler.begin(),
            state.dataEvapFluidCoolers->SimpleEvapFluidCooler.end(),
            [&objectType, &objectName](const EvapFluidCoolerSpecs &myObj) { return myObj.Type == objectType && myObj.Name == objectName; });
        if (thisObj != state.dataEvapFluidCoolers->SimpleEvapFluidCooler.end()) {
            return thisObj;
        }
        // If we didn't find it, fatal
        ShowFatalError(state, format("LocalEvapFluidCoolerFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void GetEvapFluidCoolerInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Chandan Sharma
        //       DATE WRITTEN:    May 2009
        //       MODIFIED         Chandan Sharma, April 2010
        //       RE-ENGINEERED    na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for evaporative fluid coolers and stores it in SimpleEvapFluidCooler data structure.

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in the data.

        // REFERENCES:
        // Based on GetTowerInput subroutine from Don Shirey, Jan 2001 and Sept/Oct 2002
        // B.A. Qureshi and S.M. Zubair , Prediction of evaporation losses in evaporative fluid coolers
        // Applied thermal engineering 27 (2007) 520-527

        static constexpr std::string_view routineName = "GetEvapFluidCoolerInput";

        int NumAlphas;                // Number of elements in the alpha array
        int NumNums;                  // Number of elements in the numeric array
        int IOStat;                   // IO Status when calling get input subroutine
        bool ErrorsFound(false);      // Logical flag set .TRUE. if errors found while getting input data
        Array1D<Real64> NumArray(25); // Numeric input data array
        Array1D_string AlphArray(13); // Character string input data array

        // Get number of all evaporative fluid coolers specified in the input data file (idf)
        int NumSingleSpeedEvapFluidCoolers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cEvapFluidCooler_SingleSpeed);
        int NumTwoSpeedEvapFluidCoolers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cEvapFluidCooler_TwoSpeed);
        int NumSimpleEvapFluidCoolers = NumSingleSpeedEvapFluidCoolers + NumTwoSpeedEvapFluidCoolers;

        if (NumSimpleEvapFluidCoolers <= 0) {
            ShowFatalError(state,
                           "No evaporative fluid cooler objects found in input, however, a branch object has specified an evaporative fluid cooler. "
                           "Search the input for evaporative fluid cooler to determine the cause for this error.");
        }

        // Allocate data structures to hold evaporative fluid cooler input data,
        // report data and evaporative fluid cooler inlet conditions
        state.dataEvapFluidCoolers->SimpleEvapFluidCooler.allocate(NumSimpleEvapFluidCoolers);
        state.dataEvapFluidCoolers->UniqueSimpleEvapFluidCoolerNames.reserve(NumSimpleEvapFluidCoolers);

        // Load data structures with evaporative fluid cooler input data
        state.dataIPShortCut->cCurrentModuleObject = cEvapFluidCooler_SingleSpeed;
        for (int SingleSpeedEvapFluidCoolerNumber = 1; SingleSpeedEvapFluidCoolerNumber <= NumSingleSpeedEvapFluidCoolers;
             ++SingleSpeedEvapFluidCoolerNumber) {
            int EvapFluidCoolerNum = SingleSpeedEvapFluidCoolerNumber;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                                     SingleSpeedEvapFluidCoolerNumber,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)};

            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataEvapFluidCoolers->UniqueSimpleEvapFluidCoolerNames,
                                                     AlphArray(1),
                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);

            auto &thisEFC = state.dataEvapFluidCoolers->SimpleEvapFluidCooler(EvapFluidCoolerNum);

            thisEFC.Name = AlphArray(1);
            thisEFC.EvapFluidCoolerType = state.dataIPShortCut->cCurrentModuleObject;
            thisEFC.Type = DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd;
            thisEFC.EvapFluidCoolerMassFlowRateMultiplier = 2.5;
            thisEFC.WaterInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            AlphArray(2),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::EvaporativeFluidCoolerSingleSpeed,
                                                                            AlphArray(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
            thisEFC.WaterOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                             AlphArray(3),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::EvaporativeFluidCoolerSingleSpeed,
                                                                             AlphArray(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(
                state, state.dataIPShortCut->cCurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Chilled Water Nodes");
            thisEFC.HighSpeedAirFlowRate = NumArray(1);
            if (thisEFC.HighSpeedAirFlowRate == DataSizing::AutoSize) {
                thisEFC.HighSpeedAirFlowRateWasAutoSized = true;
            }
            thisEFC.HighSpeedFanPower = NumArray(2);
            if (thisEFC.HighSpeedFanPower == DataSizing::AutoSize) {
                thisEFC.HighSpeedFanPowerWasAutoSized = true;
            }
            thisEFC.DesignSprayWaterFlowRate = NumArray(3);
            thisEFC.HeatRejectCapNomCapSizingRatio = NumArray(4);
            thisEFC.HighSpeedStandardDesignCapacity = NumArray(5);
            thisEFC.HighSpeedEvapFluidCoolerUA = NumArray(6);
            if (thisEFC.HighSpeedEvapFluidCoolerUA == DataSizing::AutoSize) {
                thisEFC.HighSpeedEvapFluidCoolerUAWasAutoSized = true;
            }
            thisEFC.DesignWaterFlowRate = NumArray(7);
            if (thisEFC.DesignWaterFlowRate == DataSizing::AutoSize) {
                thisEFC.DesignWaterFlowRateWasAutoSized = true;
            }
            thisEFC.HighSpeedUserSpecifiedDesignCapacity = NumArray(8);
            thisEFC.DesignEnteringWaterTemp = NumArray(9);
            thisEFC.DesignEnteringAirTemp = NumArray(10);
            thisEFC.DesignEnteringAirWetBulbTemp = NumArray(11);

            if (state.dataIPShortCut->lAlphaFieldBlanks(4) || AlphArray(4).empty()) {
                ShowSevereError(
                    state,
                    format("{}, \"{}\" Performance input method is not specified. ", state.dataIPShortCut->cCurrentModuleObject, thisEFC.Name));
                ErrorsFound = true;
            } else {
                thisEFC.PerformanceInputMethod = AlphArray(4);
            }

            // outdoor air inlet node
            if (state.dataIPShortCut->lAlphaFieldBlanks(5)) {
                thisEFC.OutdoorAirInletNodeNum = 0;
            } else {
                thisEFC.OutdoorAirInletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(5),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::EvaporativeFluidCoolerSingleSpeed,
                                                        thisEFC.Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, thisEFC.OutdoorAirInletNodeNum)) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" Outdoor Air Inlet DataLoopNode::Node Name not valid Outdoor Air DataLoopNode::Node= {}",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           thisEFC.Name,
                                           AlphArray(5)));
                    ShowContinueError(state, "...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:DataLoopNode::Node.");
                    ErrorsFound = true;
                }
            }

            //   fluid bypass for single speed evaporative fluid cooler
            if (state.dataIPShortCut->lAlphaFieldBlanks(6) || AlphArray(6).empty()) {
                thisEFC.capacityControl = CapacityControl::FanCycling; // FanCycling
            } else {
                thisEFC.capacityControl = static_cast<CapacityControl>(getEnumValue(controlNamesUC, Util::makeUPPER(AlphArray(6))));
                if (thisEFC.capacityControl == CapacityControl::Invalid) {
                    thisEFC.capacityControl = CapacityControl::FanCycling;
                    ShowWarningError(state,
                                     format("{}, \"{}\" The Capacity Control is not specified correctly. The default Fan Cycling is used.",
                                            state.dataIPShortCut->cCurrentModuleObject,
                                            thisEFC.Name));
                }
            }

            thisEFC.SizFac = NumArray(12); //  N11  \field Sizing Factor
            if (thisEFC.SizFac <= 0.0) {
                thisEFC.SizFac = 1.0;
            }

            if (AlphArray(7).empty()) {
                thisEFC.EvapLossMode = EvapLoss::ByMoistTheory;
            } else {
                thisEFC.EvapLossMode = static_cast<EvapLoss>(getEnumValue(evapLossNamesUC, Util::makeUPPER(AlphArray(7))));
                if (thisEFC.EvapLossMode == EvapLoss::Invalid) {
                    ShowSevereError(state, format("Invalid, {} = {}", state.dataIPShortCut->cAlphaFieldNames(7), AlphArray(7)));
                    ShowContinueError(state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
            }

            thisEFC.UserEvapLossFactor = NumArray(13); //  N13 , \field Evaporation Loss Factor
            if ((NumNums < 13) && (thisEFC.UserEvapLossFactor == 0.0)) {
                // assume Evaporation loss factor not entered and should be calculated
                if ((state.dataEnvrn->OutRelHumValue >= 0.1) && (state.dataEnvrn->OutRelHumValue <= 0.7)) {
                    // Use correlation by B.A. Qureshi and S.M. Zubair if within these limits
                    thisEFC.UserEvapLossFactor =
                        (113.0 - 8.417 * state.dataEnvrn->OutRelHumValue + 1.6147 * state.dataEnvrn->OutDryBulbTemp) * 1.0e-5;
                } else { // Inlet conditions are out of the limit of correlation; An approximate default value of loss factor is used
                    thisEFC.UserEvapLossFactor = 0.2;
                }
            }

            thisEFC.DriftLossFraction = NumArray(14) / 100.0; //  N14, \field Drift Loss Percent

            if ((NumNums < 13) && (thisEFC.DriftLossFraction == 0.0)) {
                // assume Drift loss not entered and should be defaulted
                thisEFC.DriftLossFraction = 0.008 / 100.0;
            }
            thisEFC.ConcentrationRatio = NumArray(15); //  N15, \field Blowdown Concentration Ratio

            if (AlphArray(8).empty()) {
                thisEFC.BlowdownMode = Blowdown::ByConcentration;
                if ((NumNums < 15) && (thisEFC.ConcentrationRatio == 0.0)) {
                    // assume Concentration ratio was omitted and should be defaulted
                    thisEFC.ConcentrationRatio = 3.0;
                }
            } else {
                thisEFC.BlowdownMode = static_cast<Blowdown>(getEnumValue(blowDownNamesUC, Util::makeUPPER(AlphArray(8))));
                if (thisEFC.BlowdownMode == Blowdown::Invalid) {
                    ShowSevereError(state, format("Invalid, {} = {}", state.dataIPShortCut->cAlphaFieldNames(8), AlphArray(8)));
                    ShowContinueError(state, format("Entered in {} ={}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
            }

            if (thisEFC.BlowdownMode == Blowdown::BySchedule) {
                if ((thisEFC.blowdownSched = Sched::GetSchedule(state, AlphArray(9))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(9), AlphArray(9));
                    ErrorsFound = true;
                }
            }

            if (AlphArray(10).empty()) {
                thisEFC.SuppliedByWaterSystem = false;
            } else { // water from storage tank
                WaterManager::SetupTankDemandComponent(state,
                                                       AlphArray(1),
                                                       state.dataIPShortCut->cCurrentModuleObject,
                                                       AlphArray(10),
                                                       ErrorsFound,
                                                       thisEFC.WaterTankID,
                                                       thisEFC.WaterTankDemandARRID);
                thisEFC.SuppliedByWaterSystem = true;
            }

            //   Check various inputs to ensure that all the required variables are specified.

            if (thisEFC.DesignSprayWaterFlowRate <= 0.0) {
                ShowSevereError(state,
                                format("{} \"{}\". Evaporative fluid cooler input requires a design spray water flow rate greater than zero for all "
                                       "performance input methods.",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ErrorsFound = true;
            }
            if (thisEFC.HighSpeedAirFlowRate <= 0.0 && thisEFC.HighSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       AlphArray(1),
                                       state.dataIPShortCut->cNumericFieldNames(1),
                                       state.dataIPShortCut->cAlphaFieldNames(4),
                                       AlphArray(4)));
                ErrorsFound = true;
            }
            if (thisEFC.HighSpeedFanPower <= 0.0 && thisEFC.HighSpeedFanPower != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       AlphArray(1),
                                       state.dataIPShortCut->cNumericFieldNames(2),
                                       state.dataIPShortCut->cAlphaFieldNames(4),
                                       AlphArray(4)));
                ErrorsFound = true;
            }

            if (Util::SameString(AlphArray(4), "UFACTORTIMESAREAANDDESIGNWATERFLOWRATE")) {
                thisEFC.PerformanceInputMethod_Num = PIM::UFactor;
                if (thisEFC.HighSpeedEvapFluidCoolerUA <= 0.0 && thisEFC.HighSpeedEvapFluidCoolerUA != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(6),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignWaterFlowRate <= 0.0 && thisEFC.DesignWaterFlowRate != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(7),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(AlphArray(4), "STANDARDDESIGNCAPACITY")) {
                thisEFC.PerformanceInputMethod_Num = PIM::StandardDesignCapacity;
                if (thisEFC.HighSpeedStandardDesignCapacity <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(5),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(AlphArray(4), "USERSPECIFIEDDESIGNCAPACITY")) {
                thisEFC.PerformanceInputMethod_Num = PIM::UserSpecifiedDesignCapacity;
                if (thisEFC.DesignWaterFlowRate <= 0.0 && thisEFC.DesignWaterFlowRate != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(7),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.HighSpeedUserSpecifiedDesignCapacity <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(8),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringWaterTemp <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be >0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(9),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringAirTemp <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be >0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(10),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringAirWetBulbTemp <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be >0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(11),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringWaterTemp <= thisEFC.DesignEnteringAirWetBulbTemp) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", {} must be greater than {}.",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(9),
                                           state.dataIPShortCut->cNumericFieldNames(11)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringAirTemp <= thisEFC.DesignEnteringAirWetBulbTemp) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", {} must be greater than {}.",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(10),
                                           state.dataIPShortCut->cNumericFieldNames(11)));
                    ErrorsFound = true;
                }
            } else { // Evaporative fluid cooler performance input method is not specified as a valid "choice"
                ShowSevereError(state,
                                format("{} = \"{}\". Evaporative fluid cooler Performance Input Method must be "
                                       "\"UFactorTimesAreaAndDesignWaterFlowRate\" or \"StandardDesignCapacity\" or \"UserSpecifiedDesignCapacity\".",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ShowContinueError(state, format("Evaporative fluid cooler Performance Input Method currently specified as: {}", AlphArray(4)));
                ErrorsFound = true;
            }

        } // End Single-Speed Evaporative Fluid Cooler Loop

        state.dataIPShortCut->cCurrentModuleObject = cEvapFluidCooler_TwoSpeed;
        for (int TwoSpeedEvapFluidCoolerNumber = 1; TwoSpeedEvapFluidCoolerNumber <= NumTwoSpeedEvapFluidCoolers; ++TwoSpeedEvapFluidCoolerNumber) {
            int EvapFluidCoolerNum = NumSingleSpeedEvapFluidCoolers + TwoSpeedEvapFluidCoolerNumber;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                                     TwoSpeedEvapFluidCoolerNumber,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)};

            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataEvapFluidCoolers->UniqueSimpleEvapFluidCoolerNames,
                                                     AlphArray(1),
                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);

            auto &thisEFC = state.dataEvapFluidCoolers->SimpleEvapFluidCooler(EvapFluidCoolerNum);

            thisEFC.Name = AlphArray(1);
            thisEFC.EvapFluidCoolerType = state.dataIPShortCut->cCurrentModuleObject;
            thisEFC.Type = DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd;
            thisEFC.EvapFluidCoolerMassFlowRateMultiplier = 2.5;
            thisEFC.WaterInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            AlphArray(2),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::EvaporativeFluidCoolerTwoSpeed,
                                                                            AlphArray(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
            thisEFC.WaterOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                             AlphArray(3),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::EvaporativeFluidCoolerTwoSpeed,
                                                                             AlphArray(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(
                state, state.dataIPShortCut->cCurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Chilled Water Nodes");

            thisEFC.HighSpeedAirFlowRate = NumArray(1);
            if (thisEFC.HighSpeedAirFlowRate == DataSizing::AutoSize) {
                thisEFC.HighSpeedAirFlowRateWasAutoSized = true;
            }
            thisEFC.HighSpeedFanPower = NumArray(2);
            if (thisEFC.HighSpeedFanPower == DataSizing::AutoSize) {
                thisEFC.HighSpeedFanPowerWasAutoSized = true;
            }
            thisEFC.LowSpeedAirFlowRate = NumArray(3);
            if (thisEFC.LowSpeedAirFlowRate == DataSizing::AutoSize) {
                thisEFC.LowSpeedAirFlowRateWasAutoSized = true;
            }
            thisEFC.LowSpeedAirFlowRateSizingFactor = NumArray(4);
            thisEFC.LowSpeedFanPower = NumArray(5);
            if (thisEFC.LowSpeedFanPower == DataSizing::AutoSize) {
                thisEFC.LowSpeedFanPowerWasAutoSized = true;
            }
            thisEFC.LowSpeedFanPowerSizingFactor = NumArray(6);
            thisEFC.DesignSprayWaterFlowRate = NumArray(7);
            thisEFC.HeatRejectCapNomCapSizingRatio = NumArray(8);
            thisEFC.HighSpeedStandardDesignCapacity = NumArray(9);
            thisEFC.LowSpeedStandardDesignCapacity = NumArray(10);
            thisEFC.HighSpeedEvapFluidCoolerUA = NumArray(12);
            if (thisEFC.HighSpeedEvapFluidCoolerUA == DataSizing::AutoSize) {
                thisEFC.HighSpeedEvapFluidCoolerUAWasAutoSized = true;
            }
            thisEFC.LowSpeedEvapFluidCoolerUA = NumArray(13);
            if (thisEFC.LowSpeedEvapFluidCoolerUA == DataSizing::AutoSize) {
                thisEFC.LowSpeedEvapFluidCoolerUAWasAutoSized = true;
            }
            thisEFC.LowSpeedEvapFluidCoolerUASizingFactor = NumArray(14);
            thisEFC.DesignWaterFlowRate = NumArray(15);
            if (thisEFC.DesignWaterFlowRate == DataSizing::AutoSize) {
                thisEFC.DesignWaterFlowRateWasAutoSized = true;
            }
            thisEFC.HighSpeedUserSpecifiedDesignCapacity = NumArray(16);
            thisEFC.LowSpeedUserSpecifiedDesignCapacity = NumArray(17);
            thisEFC.DesignEnteringWaterTemp = NumArray(19);
            thisEFC.DesignEnteringAirTemp = NumArray(20);
            thisEFC.DesignEnteringAirWetBulbTemp = NumArray(21);

            if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    format("{}, \"{}\" Performance input method is not specified. ", state.dataIPShortCut->cCurrentModuleObject, thisEFC.Name));
                ErrorsFound = true;
            } else {
                thisEFC.PerformanceInputMethod = AlphArray(4);
            }

            // outdoor air inlet node
            if (state.dataIPShortCut->lAlphaFieldBlanks(5)) {
                thisEFC.OutdoorAirInletNodeNum = 0;
            } else {
                thisEFC.OutdoorAirInletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(5),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::EvaporativeFluidCoolerTwoSpeed,
                                                        thisEFC.Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, thisEFC.OutdoorAirInletNodeNum)) {
                    ShowSevereError(state,
                                    format("{}, \"{}\" Outdoor Air Inlet DataLoopNode::Node Name not valid Outdoor Air DataLoopNode::Node= {}",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           thisEFC.Name,
                                           AlphArray(5)));
                    ShowContinueError(state, "...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:DataLoopNode::Node.");
                    ErrorsFound = true;
                }
            }

            thisEFC.SizFac = NumArray(22); //  N16  \field Sizing Factor
            if (thisEFC.SizFac <= 0.0) {
                thisEFC.SizFac = 1.0;
            }

            if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
                thisEFC.EvapLossMode = EvapLoss::ByMoistTheory;
            } else {
                thisEFC.EvapLossMode = static_cast<EvapLoss>(getEnumValue(evapLossNamesUC, Util::makeUPPER(AlphArray(6))));
                if (thisEFC.EvapLossMode == EvapLoss::Invalid) {
                    ShowSevereError(state, format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(6), AlphArray(6)));
                    ShowContinueError(state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
            }

            thisEFC.UserEvapLossFactor = NumArray(23); //  N23 , \field Evaporation Loss Factor
            if ((NumNums < 23) && (thisEFC.UserEvapLossFactor == 0.0)) {
                // assume Evaporation loss factor not entered and should be calculated
                if ((state.dataEnvrn->OutRelHumValue >= 0.1) && (state.dataEnvrn->OutRelHumValue <= 0.7)) {
                    // Use correlation by B.A. Qureshi and S.M. Zubair if within these limits
                    thisEFC.UserEvapLossFactor =
                        (113.0 - 8.417 * state.dataEnvrn->OutRelHumValue + 1.6147 * state.dataEnvrn->OutDryBulbTemp) * 1.0e-5;
                } else { // Inlet conditions are out of the limit of correlation; An approximate default value of loss factor is used
                    thisEFC.UserEvapLossFactor = 0.2;
                }
            }
            thisEFC.DriftLossFraction = NumArray(24) / 100.0; //  N24, \field Drift Loss Percent
            if ((NumNums < 24) && (thisEFC.DriftLossFraction == 0.0)) {
                // assume Drift loss not entered and should be defaulted
                thisEFC.DriftLossFraction = 0.008 / 100.0;
            }

            thisEFC.ConcentrationRatio = NumArray(25); //  N25, \field Blowdown Concentration Ratio

            if (state.dataIPShortCut->lAlphaFieldBlanks(7)) {
                thisEFC.BlowdownMode = Blowdown::ByConcentration;
                if ((NumNums < 25) && (thisEFC.ConcentrationRatio == 0.0)) {
                    // assume Concentration ratio was omitted and should be defaulted
                    thisEFC.ConcentrationRatio = 3.0;
                }
            } else {
                thisEFC.BlowdownMode = static_cast<Blowdown>(getEnumValue(blowDownNamesUC, Util::makeUPPER(AlphArray(7))));
                if (thisEFC.BlowdownMode == Blowdown::Invalid) {
                    ShowSevereError(state, format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(7), AlphArray(7)));
                    ShowContinueError(state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
            }

            if (thisEFC.BlowdownMode == Blowdown::BySchedule) {
                if ((thisEFC.blowdownSched = Sched::GetSchedule(state, AlphArray(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8), AlphArray(8));
                    ErrorsFound = true;
                }
            }

            if (state.dataIPShortCut->lAlphaFieldBlanks(9)) {
                thisEFC.SuppliedByWaterSystem = false;
            } else { // water from storage tank
                WaterManager::SetupTankDemandComponent(state,
                                                       AlphArray(1),
                                                       state.dataIPShortCut->cCurrentModuleObject,
                                                       AlphArray(9),
                                                       ErrorsFound,
                                                       thisEFC.WaterTankID,
                                                       thisEFC.WaterTankDemandARRID);
                thisEFC.SuppliedByWaterSystem = true;
            }

            //   Check various inputs to ensure that all the required variables are specified.

            if (thisEFC.DesignSprayWaterFlowRate <= 0.0) {
                ShowSevereError(state,
                                format("{} \"{}\". Evaporative fluid cooler input requires a design spray water flow rate greater than zero for all "
                                       "performance input methods.",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ErrorsFound = true;
            }
            if (thisEFC.HighSpeedAirFlowRate <= 0.0 && thisEFC.HighSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{}= \"{}\". Evaporative fluid cooler input requires design air flow rate at high fan speed to be greater "
                                       "than zero for all performance input methods.",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ErrorsFound = true;
            }
            if (thisEFC.LowSpeedAirFlowRate <= 0.0 && thisEFC.LowSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{}= \"{}\". Evaporative fluid cooler input requires design air flow rate at low fan speed to be greater than "
                                       "zero for all performance input methods.",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ErrorsFound = true;
            }
            //   High speed air flow rate must be greater than low speed air flow rate.
            //   Can't tell yet if autosized, check later in InitEvapFluidCooler.
            if (thisEFC.HighSpeedAirFlowRate <= thisEFC.LowSpeedAirFlowRate && thisEFC.HighSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereError(
                    state,
                    format(
                        "{} = \"{}\". Evaporative fluid cooler air flow rate at low fan speed must be less than the air flow rate at high fan speed.",
                        state.dataIPShortCut->cCurrentModuleObject,
                        thisEFC.Name));
                ErrorsFound = true;
            }
            if (thisEFC.HighSpeedFanPower <= 0.0 && thisEFC.HighSpeedFanPower != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       AlphArray(1),
                                       state.dataIPShortCut->cNumericFieldNames(2),
                                       state.dataIPShortCut->cAlphaFieldNames(4),
                                       AlphArray(4)));
                ErrorsFound = true;
            }
            if (thisEFC.LowSpeedFanPower <= 0.0 && thisEFC.LowSpeedFanPower != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       AlphArray(1),
                                       state.dataIPShortCut->cNumericFieldNames(5),
                                       state.dataIPShortCut->cAlphaFieldNames(4),
                                       AlphArray(4)));
                ErrorsFound = true;
            }
            if (thisEFC.HighSpeedFanPower <= thisEFC.LowSpeedFanPower && thisEFC.HighSpeedFanPower != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} = \"{}\". Evaporative fluid cooler low speed fan power must be less than the high speed fan power .",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ErrorsFound = true;
            }

            if (Util::SameString(AlphArray(4), "UFACTORTIMESAREAANDDESIGNWATERFLOWRATE")) {
                thisEFC.PerformanceInputMethod_Num = PIM::UFactor;
                if (thisEFC.HighSpeedEvapFluidCoolerUA <= 0.0 && thisEFC.HighSpeedEvapFluidCoolerUA != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(12),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.LowSpeedEvapFluidCoolerUA <= 0.0 && thisEFC.LowSpeedEvapFluidCoolerUA != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(13),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.HighSpeedEvapFluidCoolerUA <= thisEFC.LowSpeedEvapFluidCoolerUA &&
                    thisEFC.HighSpeedEvapFluidCoolerUA != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Evaporative fluid cooler U-factor Times Area Value at Low Fan Speed must be less than the "
                                           "U-factor Times Area Value at High Fan Speed.",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           thisEFC.Name));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignWaterFlowRate <= 0.0 && thisEFC.DesignWaterFlowRate != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(15),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(AlphArray(4), "STANDARDDESIGNCAPACITY")) {
                thisEFC.PerformanceInputMethod_Num = PIM::StandardDesignCapacity;
                if (thisEFC.HighSpeedStandardDesignCapacity <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(9),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.LowSpeedStandardDesignCapacity <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(10),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.LowSpeedStandardDesignCapacity >= thisEFC.HighSpeedStandardDesignCapacity) {
                    ShowSevereError(
                        state,
                        format("{} = \"{}\". Low-Speed Standard Design Capacity must be less than the High-Speed Standard Design Capacity.",
                               state.dataIPShortCut->cCurrentModuleObject,
                               thisEFC.Name));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(AlphArray(4), "USERSPECIFIEDDESIGNCAPACITY")) {
                thisEFC.PerformanceInputMethod_Num = PIM::UserSpecifiedDesignCapacity;
                if (thisEFC.DesignWaterFlowRate <= 0.0 && thisEFC.DesignWaterFlowRate != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(15),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.HighSpeedUserSpecifiedDesignCapacity <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(16),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.LowSpeedUserSpecifiedDesignCapacity <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be > 0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(17),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.HighSpeedEvapFluidCoolerUA != 0.0) {
                    if (thisEFC.HighSpeedEvapFluidCoolerUA > 0.0) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". UserSpecifiedDesignCapacity performance input method and evaporative fluid cooler UA at "
                                               "high fan speed have been specified.",
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               thisEFC.Name));
                    } else {
                        ShowSevereError(state,
                                        format("{} = \"{}\". UserSpecifiedDesignCapacity performance input method has been specified and evaporative "
                                               "fluid cooler UA at high fan speed is being autosized.",
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               thisEFC.Name));
                    }
                    ShowContinueError(state,
                                      "Evaporative fluid cooler UA at high fan speed must be left blank when UserSpecifiedDesignCapacity performance "
                                      "input method is used.");
                    ErrorsFound = true;
                }
                if (thisEFC.LowSpeedEvapFluidCoolerUA != 0.0) {
                    if (thisEFC.LowSpeedEvapFluidCoolerUA > 0.0) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". UserSpecifiedDesignCapacity performance input method and evaporative fluid cooler UA at "
                                               "low fan speed have been specified.",
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               thisEFC.Name));
                    } else {
                        ShowSevereError(state,
                                        format("{} = \"{}\". UserSpecifiedDesignCapacity performance input method has been specified and evaporative "
                                               "fluid cooler UA at low fan speed is being autosized.",
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               thisEFC.Name));
                    }
                    ShowContinueError(state,
                                      "Evaporative fluid cooler UA at low fan speed must be left blank when UserSpecifiedDesignCapacity performance "
                                      "input method is used.");
                    ErrorsFound = true;
                }
                if (thisEFC.LowSpeedUserSpecifiedDesignCapacity >= thisEFC.HighSpeedUserSpecifiedDesignCapacity) {
                    ShowSevereError(
                        state,
                        format(
                            "{} = \"{}\". Low-Speed User Specified Design Capacity must be less than the High-Speed User Specified Design Capacity.",
                            state.dataIPShortCut->cCurrentModuleObject,
                            thisEFC.Name));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringWaterTemp <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be >0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(19),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringAirTemp <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, buy must be >0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(20),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringAirWetBulbTemp <= 0.0) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", invalid data for \"{}\", entered value <= 0.0, but must be >0 for {} = \"{}\".",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(21),
                                           state.dataIPShortCut->cAlphaFieldNames(4),
                                           AlphArray(4)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringWaterTemp <= thisEFC.DesignEnteringAirWetBulbTemp) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", {} must be greater than {}.",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(19),
                                           state.dataIPShortCut->cNumericFieldNames(15)));
                    ErrorsFound = true;
                }
                if (thisEFC.DesignEnteringAirTemp <= thisEFC.DesignEnteringAirWetBulbTemp) {
                    ShowSevereError(state,
                                    format("{} = \"{}\", {} must be greater than {}.",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           AlphArray(1),
                                           state.dataIPShortCut->cNumericFieldNames(20),
                                           state.dataIPShortCut->cNumericFieldNames(15)));
                    ErrorsFound = true;
                }
            } else { // Evaporative fluid cooler performance input method is not specified as a valid "choice"
                ShowSevereError(state,
                                format("{} = \"{}\". Evaporative fluid cooler Performance Input Method must be "
                                       "\"UFactorTimesAreaAndDesignWaterFlowRate\" or \"StandardDesignCapacity\" or \"UserSpecifiedDesignCapacity\".",
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       thisEFC.Name));
                ShowContinueError(state, format("Evaporative fluid cooler Performance Input Method currently specified as: {}", AlphArray(4)));
                ErrorsFound = true;
            }

        } // End Two-Speed Evaporative Fluid Cooler Loop

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting evaporative fluid cooler input.");
        }
    }

    void EvapFluidCoolerSpecs::setupOutputVars(EnergyPlusData &state)
    {
        // Set up output variables
        // CurrentModuleObject='EvaporativeFluidCooler:SingleSpeed'
        if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {

            // Added for fluid bypass
            SetupOutputVariable(state,
                                "Cooling Tower Bypass Fraction",
                                Constant::Units::None,
                                this->BypassFraction,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
        }

        // setup common water reporting for all types of evaporative fluid coolers.
        // CurrentModuleObject='EvaporativeFluidCooler:*'
        if (this->SuppliedByWaterSystem) {
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->MakeUpVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume",
                                Constant::Units::m3,
                                this->MakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name);

            SetupOutputVariable(state,
                                "Cooling Tower Storage Tank Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->TankSupplyVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Cooling Tower Storage Tank Water Volume",
                                Constant::Units::m3,
                                this->TankSupplyVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);

            SetupOutputVariable(state,
                                "Cooling Tower Starved Storage Tank Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->StarvedMakeUpVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Cooling Tower Starved Storage Tank Water Volume",
                                Constant::Units::m3,
                                this->StarvedMakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);

            SetupOutputVariable(state,
                                "Cooling Tower Make Up Mains Water Volume",
                                Constant::Units::m3,
                                this->StarvedMakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);

        } else { // Evaporative fluid cooler water from mains and gets metered
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->MakeUpVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume",
                                Constant::Units::m3,
                                this->MakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);

            SetupOutputVariable(state,
                                "Cooling Tower Make Up Mains Water Volume",
                                Constant::Units::m3,
                                this->MakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);
        }

        SetupOutputVariable(state,
                            "Cooling Tower Inlet Temperature",
                            Constant::Units::C,
                            this->fluidCoolerInletWaterTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Outlet Temperature",
                            Constant::Units::C,
                            this->fluidCoolerOutletWaterTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->WaterMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Heat Transfer Rate",
                            Constant::Units::W,
                            this->Qactual,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Fan Electricity Rate",
                            Constant::Units::W,
                            this->FanPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Fan Electricity Energy",
                            Constant::Units::J,
                            this->FanEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::HeatRejection);

        SetupOutputVariable(state,
                            "Cooling Tower Water Evaporation Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->EvaporationVdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Water Evaporation Volume",
                            Constant::Units::m3,
                            this->EvaporationVol,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Water Drift Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->DriftVdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Water Drift Volume",
                            Constant::Units::m3,
                            this->DriftVol,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Water Blowdown Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->BlowdownVdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Cooling Tower Water Blowdown Volume",
                            Constant::Units::m3,
                            this->BlowdownVol,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
    }

    void EvapFluidCoolerSpecs::getSizingFactor(Real64 &_sizFac)
    {
        _sizFac = this->SizFac;
    }

    void EvapFluidCoolerSpecs::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
    {
        this->InitEvapFluidCooler(state);
        this->SizeEvapFluidCooler(state);
    }

    void EvapFluidCoolerSpecs::getDesignCapacities(EnergyPlusData &state, const PlantLocation &, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
    {
        if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd ||
            this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
            MinLoad = 0.0; // signifies non-load based model (i.e. forward heat exchanger model)
            MaxLoad = this->HighSpeedStandardDesignCapacity * this->HeatRejectCapNomCapSizingRatio;
            OptLoad = this->HighSpeedStandardDesignCapacity;
        } else {
            ShowFatalError(state, format("SimEvapFluidCoolers: Invalid evaporative fluid cooler Type Requested = {}", EvapFluidCoolerType));
        }
    }

    void EvapFluidCoolerSpecs::simulate(EnergyPlusData &state,
                                        [[maybe_unused]] const PlantLocation &calledFromLocation,
                                        [[maybe_unused]] bool FirstHVACIteration,
                                        [[maybe_unused]] Real64 &CurLoad,
                                        bool RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Main evaporative fluid cooler driver subroutine.  Gets called from
        // PlantCondLoopSupplySideManager.

        // REFERENCES:
        // Based on SimTowers subroutine by Fred Buhl, May 2002

        this->AirFlowRateRatio = 0.0; // Ratio of air flow rate through VS Evaporative fluid cooler to design air flow rate

        this->InitEvapFluidCooler(state);

        if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
            this->CalcSingleSpeedEvapFluidCooler(state);
        } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
            this->CalcTwoSpeedEvapFluidCooler(state);
        } else {
            ShowFatalError(state, format("SimEvapFluidCoolers: Invalid evaporative fluid cooler Type Requested = {}", EvapFluidCoolerType));
        }

        this->CalculateWaterUsage(state);
        this->UpdateEvapFluidCooler(state);
        this->ReportEvapFluidCooler(state, RunFlag);
    }

    void EvapFluidCoolerSpecs::InitEvapFluidCooler(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the evaporative fluid cooler components and for
        // final checking of evaporative fluid cooler inputs (post autosizing)

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // REFERENCES:
        // Based on InitTower subroutine by Don Shirey Sept/Oct 2002, F Buhl Oct 2002

        static constexpr std::string_view RoutineName("InitEvapFluidCooler");

        this->oneTimeInit(state);

        // Begin environment initializations
        if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {

            Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
            this->DesWaterMassFlowRate = this->DesignWaterFlowRate * rho;
            PlantUtilities::InitComponentNodes(state, 0.0, this->DesWaterMassFlowRate, this->WaterInletNodeNum, this->WaterOutletNodeNum);
            this->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        // Each time initializations
        this->WaterInletNode = this->WaterInletNodeNum;
        this->inletConds.WaterTemp = state.dataLoopNodes->Node(this->WaterInletNode).Temp;

        if (this->OutdoorAirInletNodeNum != 0) {
            this->inletConds.AirTemp = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).Temp;
            this->inletConds.AirHumRat = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).HumRat;
            this->inletConds.AirPress = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).Press;
            this->inletConds.AirWetBulb = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).OutAirWetBulb;
        } else {
            this->inletConds.AirTemp = state.dataEnvrn->OutDryBulbTemp;
            this->inletConds.AirHumRat = state.dataEnvrn->OutHumRat;
            this->inletConds.AirPress = state.dataEnvrn->OutBaroPress;
            this->inletConds.AirWetBulb = state.dataEnvrn->OutWetBulbTemp;
        }

        this->WaterMassFlowRate = PlantUtilities::RegulateCondenserCompFlowReqOp(
            state, this->plantLoc, this->DesWaterMassFlowRate * this->EvapFluidCoolerMassFlowRateMultiplier);

        PlantUtilities::SetComponentFlowRate(state, this->WaterMassFlowRate, this->WaterInletNodeNum, this->WaterOutletNodeNum, this->plantLoc);
    }

    void EvapFluidCoolerSpecs::SizeEvapFluidCooler(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       Chandan Sharma, April 2010
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing evaporative fluid cooler Components for which capacities and flow rates
        // have not been specified in the input. This subroutine also calculates evaporative fluid cooler UA if the user
        // has specified evaporative fluid cooler performance via the "Standard Design Capacity" method.

        // METHODOLOGY EMPLOYED:
        // Obtains condenser flow rate from the plant sizing array. If evaporative fluid cooler performance is specified
        // via the "Standard Design Capacity" method, the water flow rate is directly proportional to capacity.

        // REFERENCES:
        // Based on SizeTower by Don Shirey, Sept/Oct 2002; Richard Raustad, Feb 2005

        int constexpr MaxIte(500);    // Maximum number of iterations
        Real64 constexpr Acc(0.0001); // Accuracy of result
        std::string const CalledFrom("SizeEvapFluidCooler");

        int SolFla = 0;                    // Flag of solver
        Real64 UA = 0.0;                   // Calculated UA value [W/C]
        Real64 OutWaterTempAtUA0 = -999.0; // Water outlet temperature at UA0
        Real64 OutWaterTempAtUA1 = -999.0; // Water outlet temperature at UA1

        Real64 DesEvapFluidCoolerLoad = 0.0; // Design evaporative fluid cooler load [W]
        Real64 tmpDesignWaterFlowRate = this->DesignWaterFlowRate;
        Real64 tmpHighSpeedFanPower = this->HighSpeedFanPower;
        Real64 tmpHighSpeedAirFlowRate = this->HighSpeedAirFlowRate;

        int PltSizCondNum = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).PlantSizNum;
        if (PltSizCondNum > 0) {
            this->DesignExitWaterTemp = state.dataSize->PlantSizData(PltSizCondNum).ExitTemp;
            if (this->DesignEnteringWaterTemp == DataSizing::AutoSize) {
                this->DesignEnteringWaterTemp =
                    state.dataSize->PlantSizData(PltSizCondNum).ExitTemp + state.dataSize->PlantSizData(PltSizCondNum).DeltaT;
            }
        }

        if (this->DesignWaterFlowRateWasAutoSized && this->PerformanceInputMethod_Num != PIM::StandardDesignCapacity) {
            if (PltSizCondNum > 0) {

                // Check when the user specified Condenser/Evaporative Fluid Cooler water design setpoint
                // temperature is less than design inlet air wet bulb temperature
                Real64 DesignEnteringAirWetBulb = 0;
                if (this->PerformanceInputMethod_Num == PIM::UFactor) {
                    DesignEnteringAirWetBulb = 25.6;
                } else {
                    DesignEnteringAirWetBulb = this->DesignEnteringAirWetBulbTemp;
                }
                if (this->DesignExitWaterTemp <= DesignEnteringAirWetBulb) {
                    ShowSevereError(state, format("Error when autosizing the UA value for Evaporative Fluid Cooler = {}.", this->Name));
                    ShowContinueError(state,
                                      format("Design Loop Exit Temperature ({:.2R} C) must be greater than design entering air wet-bulb temperature "
                                             "({:.2R} C) when autosizing the Evaporative Fluid Cooler UA.",
                                             this->DesignExitWaterTemp,
                                             DesignEnteringAirWetBulb));
                    ShowContinueError(
                        state,
                        "It is recommended that the Design Loop Exit Temperature = Design Entering Air Wet-bulb Temp plus the Evaporative "
                        "Fluid Cooler design approach temperature (e.g., 4 C).");
                    ShowContinueError(
                        state,
                        "If using HVACTemplate:Plant:ChilledWaterLoop, then check that input field Condenser Water Design Setpoint must be "
                        "> Design Entering Air Wet-bulb Temp if autosizing the Evaporative Fluid Cooler.");
                    ShowFatalError(state, "Review and revise design input values as appropriate.");
                }

                if (state.dataSize->PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                    tmpDesignWaterFlowRate = state.dataSize->PlantSizData(PltSizCondNum).DesVolFlowRate * this->SizFac;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                    }

                } else {
                    tmpDesignWaterFlowRate = 0.0;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                    }
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, this->EvapFluidCoolerType, this->Name, "Design Water Flow Rate [m3/s]", this->DesignWaterFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, this->EvapFluidCoolerType, this->Name, "Initial Design Water Flow Rate [m3/s]", this->DesignWaterFlowRate);
                    }
                }
            } else {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    ShowSevereError(state, format("Autosizing error for evaporative fluid cooler object = {}", this->Name));
                    ShowFatalError(state, "Autosizing of evaporative fluid cooler condenser flow rate requires a loop Sizing:Plant object.");
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::UFactor && !this->HighSpeedEvapFluidCoolerUAWasAutoSized) {
            if (PltSizCondNum > 0) {
                Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                Real64 Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->DesignExitWaterTemp, CalledFrom);
                DesEvapFluidCoolerLoad = rho * Cp * tmpDesignWaterFlowRate * state.dataSize->PlantSizData(PltSizCondNum).DeltaT;
                this->HighSpeedStandardDesignCapacity = DesEvapFluidCoolerLoad / this->HeatRejectCapNomCapSizingRatio;
            } else {
                this->HighSpeedStandardDesignCapacity = 0.0;
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::StandardDesignCapacity) {
            // Design water flow rate is assumed to be 3 gpm per ton (SI equivalent 5.382E-8 m3/s per watt)
            tmpDesignWaterFlowRate = 5.382e-8 * this->HighSpeedStandardDesignCapacity;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_SingleSpeed,
                                                     this->Name,
                                                     "Design Water Flow Rate based on evaporative fluid cooler Standard Design Capacity [m3/s]",
                                                     this->DesignWaterFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state,
                            cEvapFluidCooler_SingleSpeed,
                            this->Name,
                            "Initial Design Water Flow Rate based on evaporative fluid cooler Standard Design Capacity [m3/s]",
                            this->DesignWaterFlowRate);
                    }
                } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state,
                            cEvapFluidCooler_TwoSpeed,
                            this->Name,
                            "Design Water Flow Rate based on evaporative fluid cooler high-speed Standard Design Capacity [m3/s]",
                            this->DesignWaterFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state,
                            cEvapFluidCooler_TwoSpeed,
                            this->Name,
                            "Initial Design Water Flow Rate based on evaporative fluid cooler high-speed Standard Design Capacity [m3/s]",
                            this->DesignWaterFlowRate);
                    }
                }
            }
        }

        PlantUtilities::RegisterPlantCompDesignFlow(state, this->WaterInletNodeNum, tmpDesignWaterFlowRate);

        if (this->HighSpeedFanPowerWasAutoSized) {
            // We assume the nominal fan power is 0.0105 times the design load
            if (this->PerformanceInputMethod_Num == PIM::StandardDesignCapacity) {
                tmpHighSpeedFanPower = 0.0105 * this->HighSpeedStandardDesignCapacity;
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->HighSpeedFanPower = tmpHighSpeedFanPower;
                }
            } else if (this->PerformanceInputMethod_Num == PIM::UserSpecifiedDesignCapacity) {
                tmpHighSpeedFanPower = 0.0105 * this->HighSpeedUserSpecifiedDesignCapacity;
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->HighSpeedFanPower = tmpHighSpeedFanPower;
                }
            } else {
                if (DesEvapFluidCoolerLoad > 0) {
                    tmpHighSpeedFanPower = 0.0105 * DesEvapFluidCoolerLoad;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedFanPower = tmpHighSpeedFanPower;
                    }
                } else if (PltSizCondNum > 0) {
                    if (state.dataSize->PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                        Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                        Real64 Cp =
                            state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->DesignExitWaterTemp, CalledFrom);
                        DesEvapFluidCoolerLoad = rho * Cp * tmpDesignWaterFlowRate * state.dataSize->PlantSizData(PltSizCondNum).DeltaT;
                        tmpHighSpeedFanPower = 0.0105 * DesEvapFluidCoolerLoad;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->HighSpeedFanPower = tmpHighSpeedFanPower;
                        }
                    } else {
                        tmpHighSpeedFanPower = 0.0;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->HighSpeedFanPower = tmpHighSpeedFanPower;
                        }
                    }
                } else {
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        ShowSevereError(state, "Autosizing of evaporative fluid cooler fan power requires a loop Sizing:Plant object.");
                        ShowFatalError(state, format(" Occurs in evaporative fluid cooler object= {}", this->Name));
                    }
                }
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, cEvapFluidCooler_SingleSpeed, this->Name, "Fan Power at Design Air Flow Rate [W]", this->HighSpeedFanPower);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_SingleSpeed,
                                                     this->Name,
                                                     "Initial Fan Power at Design Air Flow Rate [W]",
                                                     this->HighSpeedFanPower);
                    }
                } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, cEvapFluidCooler_TwoSpeed, this->Name, "Fan Power at High Fan Speed [W]", this->HighSpeedFanPower);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, cEvapFluidCooler_TwoSpeed, this->Name, "Initial Fan Power at High Fan Speed [W]", this->HighSpeedFanPower);
                    }
                }
            }
        }

        if (this->HighSpeedAirFlowRateWasAutoSized) {
            // Plant Sizing Object is not required to AUTOSIZE this field since its simply a multiple of another field.

            tmpHighSpeedAirFlowRate = tmpHighSpeedFanPower * 0.5 * (101325.0 / state.dataEnvrn->StdBaroPress) / 190.0;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->HighSpeedAirFlowRate = tmpHighSpeedAirFlowRate;

                if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, cEvapFluidCooler_SingleSpeed, this->Name, "Design Air Flow Rate [m3/s]", this->HighSpeedAirFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, cEvapFluidCooler_SingleSpeed, this->Name, "Initial Design Air Flow Rate [m3/s]", this->HighSpeedAirFlowRate);
                    }
                } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(
                            state, cEvapFluidCooler_TwoSpeed, this->Name, "Air Flow Rate at High Fan Speed [m3/s]", this->HighSpeedAirFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_TwoSpeed,
                                                     this->Name,
                                                     "Initial Air Flow Rate at High Fan Speed [m3/s]",
                                                     this->HighSpeedAirFlowRate);
                    }
                }
            }
        }

        if (this->HighSpeedEvapFluidCoolerUAWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize &&
            this->PerformanceInputMethod_Num == PIM::UFactor) {
            if (PltSizCondNum > 0) {
                if (state.dataSize->PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                    // This conditional statement is to trap when the user specified Condenser/Evaporative Fluid Cooler water design setpoint
                    // temperature is less than design inlet air wet bulb temperature of 25.6 C
                    if (this->DesignExitWaterTemp <= 25.6) {
                        ShowSevereError(state, format("Error when autosizing the UA value for Evaporative Fluid Cooler = {}.", this->Name));
                        ShowContinueError(state,
                                          format("Design Loop Exit Temperature ({:.2R} C) must be greater than 25.6 C when autosizing the "
                                                 "Evaporative Fluid Cooler UA.",
                                                 this->DesignExitWaterTemp));
                        ShowContinueError(state,
                                          format("The Design Loop Exit Temperature specified in Sizing:Plant object = {}",
                                                 state.dataSize->PlantSizData(PltSizCondNum).PlantLoopName));
                        ShowContinueError(state,
                                          "It is recommended that the Design Loop Exit Temperature = 25.6 C plus the Evaporative Fluid Cooler design "
                                          "approach temperature (e.g., 4 C).");
                        ShowContinueError(state,
                                          "If using HVACTemplate:Plant:ChilledWaterLoop, then check that input field Condenser Water Design Setpoint "
                                          "must be > 25.6 C if autosizing the Evaporative Fluid Cooler.");
                        ShowFatalError(state, "Review and revise design input values as appropriate.");
                    }
                    Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                    Real64 Cp =
                        state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->DesignExitWaterTemp, CalledFrom);
                    DesEvapFluidCoolerLoad = rho * Cp * tmpDesignWaterFlowRate * state.dataSize->PlantSizData(PltSizCondNum).DeltaT;
                    Real64 const par1 = rho * tmpDesignWaterFlowRate; // Design water mass flow rate
                    Real64 const par2 = tmpHighSpeedAirFlowRate;      // Design air volume flow rate
                    // Lower bound for UA [W/C]
                    Real64 UA0 = 0.0001 * DesEvapFluidCoolerLoad; // Assume deltaT = 10000K (limit)
                    Real64 UA1 = DesEvapFluidCoolerLoad;          // Assume deltaT = 1K
                    this->inletConds.WaterTemp = this->DesignExitWaterTemp + state.dataSize->PlantSizData(PltSizCondNum).DeltaT;
                    this->inletConds.AirTemp = 35.0;
                    this->inletConds.AirWetBulb = 25.6;
                    this->inletConds.AirPress = state.dataEnvrn->StdBaroPress;
                    this->inletConds.AirHumRat =
                        Psychrometrics::PsyWFnTdbTwbPb(state, this->inletConds.AirTemp, this->inletConds.AirWetBulb, this->inletConds.AirPress);
                    auto f = [&state, this, DesEvapFluidCoolerLoad, par1, par2, Cp](Real64 UA) {
                        this->SimSimpleEvapFluidCooler(state, par1, par2, UA, this->DesignExitWaterTemp);
                        Real64 const CoolingOutput = Cp * par1 * (this->inletConds.WaterTemp - this->DesignExitWaterTemp);
                        return (DesEvapFluidCoolerLoad - CoolingOutput) / DesEvapFluidCoolerLoad;
                    };
                    General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                    if (SolFla == -1) {
                        ShowWarningError(state, "Iteration limit exceeded in calculating evaporative fluid cooler UA.");
                        ShowContinueError(state, format("Autosizing of fluid cooler UA failed for evaporative fluid cooler = {}", this->Name));
                        ShowContinueError(state, format("The final UA value = {:.2R}W/C, and the simulation continues...", UA));
                    } else if (SolFla == -2) {
                        this->SimSimpleEvapFluidCooler(state, par1, par2, UA0, OutWaterTempAtUA0);
                        this->SimSimpleEvapFluidCooler(state, par1, par2, UA1, OutWaterTempAtUA1);
                        ShowSevereError(state, format("{}: The combination of design input values did not allow the calculation of a ", CalledFrom));
                        ShowContinueError(state, "reasonable UA value. Review and revise design input values as appropriate. Specifying hard");
                        ShowContinueError(state,
                                          "sizes for some \"autosizable\" fields while autosizing other \"autosizable\" fields may be contributing "
                                          "to this problem.");
                        ShowContinueError(state, "This model iterates on UA to find the heat transfer required to provide the design outlet ");
                        ShowContinueError(state, "water temperature. Initially, the outlet water temperatures at high and low UA values are ");
                        ShowContinueError(state, "calculated. The Design Exit Water Temperature should be between the outlet water ");
                        ShowContinueError(state, "temperatures calculated at high and low UA values. If the Design Exit Water Temperature is ");
                        ShowContinueError(state, "out of this range, the solution will not converge and UA will not be calculated. ");
                        ShowContinueError(state, "The possible solutions could be to manually input adjusted water and/or air flow rates ");
                        ShowContinueError(
                            state,
                            "based on the autosized values shown below or to adjust design evaporative fluid cooler air inlet wet-bulb temperature.");
                        ShowContinueError(state, "Plant:Sizing object inputs also influence these results (e.g. DeltaT and ExitTemp).");
                        ShowContinueError(state, "Inputs to the evaporative fluid cooler object:");
                        ShowContinueError(state,
                                          format("Design Evaporative Fluid Cooler Load [W]                      = {:.2R}", DesEvapFluidCoolerLoad));
                        ShowContinueError(
                            state, format("Design Evaporative Fluid Cooler Water Volume Flow Rate [m3/s] = {:.6R}", this->DesignWaterFlowRate));
                        ShowContinueError(state, format("Design Evaporative Fluid Cooler Air Volume Flow Rate [m3/s]   = {:.2R}", par2));
                        ShowContinueError(
                            state, format("Design Evaporative Fluid Cooler Air Inlet Wet-bulb Temp [C]   = {:.2R}", this->inletConds.AirWetBulb));
                        ShowContinueError(
                            state, format("Design Evaporative Fluid Cooler Water Inlet Temp [C]          = {:.2R}", this->inletConds.WaterTemp));
                        ShowContinueError(state, "Inputs to the plant sizing object:");
                        ShowContinueError(
                            state, format("Design Exit Water Temp [C]                                    = {:.2R}", this->DesignExitWaterTemp));
                        ShowContinueError(state,
                                          format("Loop Design Temperature Difference [C]                        = {:.2R}",
                                                 state.dataSize->PlantSizData(PltSizCondNum).DeltaT));
                        ShowContinueError(
                            state, format("Design Evaporative Fluid Cooler Water Inlet Temp [C]          = {:.2R}", this->inletConds.WaterTemp));
                        ShowContinueError(
                            state, format("Calculated water outlet temperature at low UA [C](UA = {:.2R} W/C)  = {:.2R}", UA0, OutWaterTempAtUA0));
                        ShowContinueError(
                            state, format("Calculated water outlet temperature at high UA [C](UA = {:.2R} W/C)  = {:.2R}", UA1, OutWaterTempAtUA1));
                        ShowFatalError(state,
                                       format("Autosizing of Evaporative Fluid Cooler UA failed for Evaporative Fluid Cooler = {}", this->Name));
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedEvapFluidCoolerUA = UA;
                    }
                    this->HighSpeedStandardDesignCapacity = DesEvapFluidCoolerLoad / this->HeatRejectCapNomCapSizingRatio;
                } else {
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedEvapFluidCoolerUA = 0.0;
                    }
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         cEvapFluidCooler_SingleSpeed,
                                                         this->Name,
                                                         "U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                         this->HighSpeedEvapFluidCoolerUA);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         cEvapFluidCooler_SingleSpeed,
                                                         this->Name,
                                                         "Initial U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                         this->HighSpeedEvapFluidCoolerUA);
                        }
                    } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         cEvapFluidCooler_TwoSpeed,
                                                         this->Name,
                                                         "U-Factor Times Area Value at High Fan Speed [W/C]",
                                                         this->HighSpeedEvapFluidCoolerUA);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         cEvapFluidCooler_TwoSpeed,
                                                         this->Name,
                                                         "Initial U-Factor Times Area Value at High Fan Speed [W/C]",
                                                         this->HighSpeedEvapFluidCoolerUA);
                        }
                    }
                }
            } else {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    ShowSevereError(state, format("Autosizing error for evaporative fluid cooler object = {}", this->Name));
                    ShowFatalError(state, "Autosizing of evaporative fluid cooler UA requires a loop Sizing:Plant object.");
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::StandardDesignCapacity) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {
                // Standard Design Capacity doesn't include compressor heat;
                // predefined factor was 1.25 W heat rejection per W of delivered cooling, now a user input with 1.25 default
                Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                Real64 Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, 35.0, CalledFrom);
                DesEvapFluidCoolerLoad = this->HighSpeedStandardDesignCapacity * this->HeatRejectCapNomCapSizingRatio;
                Real64 const par1 = rho * this->DesignWaterFlowRate; // Design water mass flow rate
                Real64 const par2 = this->HighSpeedAirFlowRate;      // Design air volume flow rate
                Real64 UA0 = 0.0001 * DesEvapFluidCoolerLoad;        // Assume deltaT = 10000K (limit)
                Real64 UA1 = DesEvapFluidCoolerLoad;                 // Assume deltaT = 1K
                this->inletConds.WaterTemp = 35.0;                   // 95F design inlet water temperature
                this->DesignEnteringWaterTemp = this->inletConds.WaterTemp;
                this->inletConds.AirTemp = 35.0;    // 95F design inlet air dry-bulb temp
                this->inletConds.AirWetBulb = 25.6; // 78F design inlet air wet-bulb temp
                this->DesignEnteringAirWetBulbTemp = this->inletConds.AirWetBulb;
                this->inletConds.AirPress = state.dataEnvrn->StdBaroPress;
                this->inletConds.AirHumRat =
                    Psychrometrics::PsyWFnTdbTwbPb(state, this->inletConds.AirTemp, this->inletConds.AirWetBulb, this->inletConds.AirPress);
                auto f = [&state, this, DesEvapFluidCoolerLoad, par1, par2, Cp](Real64 UA) {
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA, this->DesignExitWaterTemp);
                    Real64 const CoolingOutput = Cp * par1 * (this->inletConds.WaterTemp - this->DesignExitWaterTemp);
                    return (DesEvapFluidCoolerLoad - CoolingOutput) / DesEvapFluidCoolerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowWarningError(state, "Iteration limit exceeded in calculating evaporative fluid cooler UA.");
                    ShowContinueError(state, format("Autosizing of fluid cooler UA failed for evaporative fluid cooler = {}", this->Name));
                    ShowContinueError(state, format("The final UA value = {:.2R}W/C, and the simulation continues...", UA));
                } else if (SolFla == -2) {
                    ShowSevereError(state, format("{}: The combination of design input values did not allow the calculation of a ", CalledFrom));
                    ShowContinueError(state, "reasonable UA value. Review and revise design input values as appropriate. ");
                    ShowFatalError(state, format("Autosizing of Evaporative Fluid Cooler UA failed for Evaporative Fluid Cooler = {}", this->Name));
                }
                this->HighSpeedEvapFluidCoolerUA = UA;
            } else {
                this->HighSpeedEvapFluidCoolerUA = 0.0;
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_SingleSpeed,
                                                     this->Name,
                                                     "U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_SingleSpeed,
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_TwoSpeed,
                                                     this->Name,
                                                     "U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_TwoSpeed,
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::UserSpecifiedDesignCapacity) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {
                Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                Real64 Cp =
                    state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->DesignEnteringWaterTemp, CalledFrom);
                DesEvapFluidCoolerLoad = this->HighSpeedUserSpecifiedDesignCapacity;
                Real64 const par1 = rho * tmpDesignWaterFlowRate; // Design water mass flow rate
                Real64 const par2 = tmpHighSpeedAirFlowRate;      // Design air volume flow rate
                Real64 UA0 = 0.0001 * DesEvapFluidCoolerLoad;     // Assume deltaT = 10000K (limit)
                Real64 UA1 = DesEvapFluidCoolerLoad;              // Assume deltaT = 1K

                this->inletConds.WaterTemp = this->DesignEnteringWaterTemp;
                this->inletConds.AirTemp = this->DesignEnteringAirTemp;
                this->inletConds.AirWetBulb = this->DesignEnteringAirWetBulbTemp;
                this->inletConds.AirPress = state.dataEnvrn->StdBaroPress;
                this->inletConds.AirHumRat =
                    Psychrometrics::PsyWFnTdbTwbPb(state, this->inletConds.AirTemp, this->inletConds.AirWetBulb, this->inletConds.AirPress);
                auto f = [&state, this, DesEvapFluidCoolerLoad, par1, par2, Cp](Real64 UA) {
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA, this->DesignExitWaterTemp);
                    Real64 const CoolingOutput = Cp * par1 * (this->inletConds.WaterTemp - this->DesignExitWaterTemp);
                    return (DesEvapFluidCoolerLoad - CoolingOutput) / DesEvapFluidCoolerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowWarningError(state, "Iteration limit exceeded in calculating evaporative fluid cooler UA.");
                    ShowContinueError(state, format("Autosizing of fluid cooler UA failed for evaporative fluid cooler = {}", this->Name));
                    ShowContinueError(state, format("The final UA value = {:.2R}W/C, and the simulation continues...", UA));
                } else if (SolFla == -2) {
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA0, OutWaterTempAtUA0);
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA1, OutWaterTempAtUA1);
                    ShowSevereError(state, format("{}: The combination of design input values did not allow the calculation of a ", CalledFrom));
                    ShowContinueError(state, "reasonable UA value. Review and revise design input values as appropriate. Specifying hard");
                    ShowContinueError(
                        state,
                        R"(sizes for some "autosizable" fields while autosizing other "autosizable" fields may be contributing to this problem.)");
                    ShowContinueError(state, "This model iterates on UA to find the heat transfer required to provide the design outlet ");
                    ShowContinueError(state, "water temperature. Initially, the outlet water temperatures at high and low UA values are ");
                    ShowContinueError(state, "calculated. The Design Exit Water Temperature should be between the outlet water ");
                    ShowContinueError(state, "temperatures calculated at high and low UA values. If the Design Exit Water Temperature is ");
                    ShowContinueError(state, "out of this range, the solution will not converge and UA will not be calculated. ");
                    ShowContinueError(state, "The possible solutions could be to manually input adjusted water and/or air flow rates ");
                    ShowContinueError(
                        state,
                        "based on the autosized values shown below or to adjust design evaporative fluid cooler air inlet wet-bulb temperature.");
                    ShowContinueError(state, "Plant:Sizing object inputs also influence these results (e.g. DeltaT and ExitTemp).");
                    ShowContinueError(state, "Inputs to the evaporative fluid cooler object:");
                    ShowContinueError(state,
                                      format("Design Evaporative Fluid Cooler Load [W]                      = {:.2R}", DesEvapFluidCoolerLoad));
                    ShowContinueError(state,
                                      format("Design Evaporative Fluid Cooler Water Volume Flow Rate [m3/s] = {:.6R}", this->DesignWaterFlowRate));
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Air Volume Flow Rate [m3/s]   = {:.2R}", par2));
                    ShowContinueError(state,
                                      format("Design Evaporative Fluid Cooler Air Inlet Wet-bulb Temp [C]   = {:.2R}", this->inletConds.AirWetBulb));
                    ShowContinueError(state,
                                      format("Design Evaporative Fluid Cooler Water Inlet Temp [C]          = {:.2R}", this->inletConds.WaterTemp));
                    ShowContinueError(state, "Inputs to the plant sizing object:");
                    ShowContinueError(state,
                                      format("Design Exit Water Temp [C]                                    = {:.2R}", this->DesignExitWaterTemp));
                    ShowContinueError(state,
                                      format("Loop Design Temperature Difference [C]                        = {:.2R}",
                                             state.dataSize->PlantSizData(PltSizCondNum).DeltaT));
                    ShowContinueError(state,
                                      format("Design Evaporative Fluid Cooler Water Inlet Temp [C]          = {:.2R}", this->inletConds.WaterTemp));
                    ShowContinueError(state,
                                      format("Calculated water outlet temperature at low UA [C](UA = {:.2R} W/C)  = {:.2R}", UA0, OutWaterTempAtUA0));
                    ShowContinueError(
                        state, format("Calculated water outlet temperature at high UA [C](UA = {:.2R} W/C)  = {:.2R}", UA1, OutWaterTempAtUA1));
                    ShowFatalError(state, format("Autosizing of Evaporative Fluid Cooler UA failed for Evaporative Fluid Cooler = {}", this->Name));
                }
                this->HighSpeedEvapFluidCoolerUA = UA;
            } else {
                this->HighSpeedEvapFluidCoolerUA = 0.0;
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_SingleSpeed,
                                                     this->Name,
                                                     "U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_SingleSpeed,
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                } else if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_TwoSpeed,
                                                     this->Name,
                                                     "U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     cEvapFluidCooler_TwoSpeed,
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedEvapFluidCoolerUA);
                    }
                }
            }
        }

        if (this->LowSpeedAirFlowRateWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            this->LowSpeedAirFlowRate = this->LowSpeedAirFlowRateSizingFactor * this->HighSpeedAirFlowRate;
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(
                    state, this->EvapFluidCoolerType, this->Name, "Air Flow Rate at Low Fan Speed [m3/s]", this->LowSpeedAirFlowRate);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(
                    state, this->EvapFluidCoolerType, this->Name, "Initial Air Flow Rate at Low Fan Speed [m3/s]", this->LowSpeedAirFlowRate);
            }
        }

        if (this->LowSpeedFanPowerWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            this->LowSpeedFanPower = this->LowSpeedFanPowerSizingFactor * this->HighSpeedFanPower;
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state, this->EvapFluidCoolerType, this->Name, "Fan Power at Low Fan Speed [W]", this->LowSpeedFanPower);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(
                    state, this->EvapFluidCoolerType, this->Name, "Initial Fan Power at Low Fan Speed [W]", this->LowSpeedFanPower);
            }
        }

        if (this->LowSpeedEvapFluidCoolerUAWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            this->LowSpeedEvapFluidCoolerUA = this->LowSpeedEvapFluidCoolerUASizingFactor * this->HighSpeedEvapFluidCoolerUA;
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             this->EvapFluidCoolerType,
                                             this->Name,
                                             "U-Factor Times Area Value at Low Fan Speed [W/C]",
                                             this->LowSpeedEvapFluidCoolerUA);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             this->EvapFluidCoolerType,
                                             this->Name,
                                             "Initial U-Factor Times Area Value at Low Fan Speed [W/C]",
                                             this->LowSpeedEvapFluidCoolerUA);
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::StandardDesignCapacity && this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow && this->LowSpeedStandardDesignCapacity > 0.0) {
                // Standard design capacity doesn't include compressor heat;
                // predefined factor was 1.25 W heat rejection per W of delivered cooling, now user input with default 1.25
                Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                Real64 Cp =
                    state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->DesignEnteringWaterTemp, CalledFrom);
                DesEvapFluidCoolerLoad = this->LowSpeedStandardDesignCapacity * this->HeatRejectCapNomCapSizingRatio;
                Real64 const par1 = rho * tmpDesignWaterFlowRate; // Design water mass flow rate
                Real64 const par2 = this->LowSpeedAirFlowRate;    // Air volume flow rate at low fan speed
                Real64 UA0 = 0.0001 * DesEvapFluidCoolerLoad;     // Assume deltaT = 10000K (limit)
                Real64 UA1 = DesEvapFluidCoolerLoad;              // Assume deltaT = 1K
                this->inletConds.WaterTemp = 35.0;                // 95F design inlet water temperature
                this->inletConds.AirTemp = 35.0;                  // 95F design inlet air dry-bulb temp
                this->inletConds.AirWetBulb = 25.6;               // 78F design inlet air wet-bulb temp
                this->inletConds.AirPress = state.dataEnvrn->StdBaroPress;
                this->inletConds.AirHumRat =
                    Psychrometrics::PsyWFnTdbTwbPb(state, this->inletConds.AirTemp, this->inletConds.AirWetBulb, this->inletConds.AirPress);
                auto f = [&state, this, DesEvapFluidCoolerLoad, par1, par2, Cp](Real64 UA) {
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA, this->DesignExitWaterTemp);
                    Real64 const CoolingOutput = Cp * par1 * (this->inletConds.WaterTemp - this->DesignExitWaterTemp);
                    return (DesEvapFluidCoolerLoad - CoolingOutput) / DesEvapFluidCoolerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowWarningError(state, "Iteration limit exceeded in calculating evaporative fluid cooler UA.");
                    ShowContinueError(state, format("Autosizing of fluid cooler UA failed for evaporative fluid cooler = {}", this->Name));
                    ShowContinueError(state, format("The final UA value = {:.2R}W/C, and the simulation continues...", UA));
                } else if (SolFla == -2) {
                    ShowSevereError(state, format("{}: The combination of design input values did not allow the calculation of a ", CalledFrom));
                    ShowContinueError(state, "reasonable low-speed UA value. Review and revise design input values as appropriate. ");
                    ShowFatalError(state, format("Autosizing of Evaporative Fluid Cooler UA failed for Evaporative Fluid Cooler = {}", this->Name));
                }
                this->LowSpeedEvapFluidCoolerUA = UA;
            } else {
                this->LowSpeedEvapFluidCoolerUA = 0.0;
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 this->EvapFluidCoolerType,
                                                 this->Name,
                                                 "U-Factor Times Area Value at Low Fan Speed [W/C]",
                                                 this->LowSpeedEvapFluidCoolerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 this->EvapFluidCoolerType,
                                                 this->Name,
                                                 "Initial U-Factor Times Area Value at Low Fan Speed [W/C]",
                                                 this->LowSpeedEvapFluidCoolerUA);
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::UserSpecifiedDesignCapacity &&
            this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow && this->LowSpeedUserSpecifiedDesignCapacity > 0.0) {
                Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, Constant::InitConvTemp, CalledFrom);
                Real64 Cp =
                    state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->DesignEnteringWaterTemp, CalledFrom);
                DesEvapFluidCoolerLoad = this->LowSpeedUserSpecifiedDesignCapacity;
                Real64 const par1 = rho * tmpDesignWaterFlowRate; // Design water mass flow rate
                Real64 const par2 = this->LowSpeedAirFlowRate;    // Air volume flow rate at low fan speed
                Real64 UA0 = 0.0001 * DesEvapFluidCoolerLoad;     // Assume deltaT = 10000K (limit)
                Real64 UA1 = DesEvapFluidCoolerLoad;              // Assume deltaT = 1K
                this->inletConds.WaterTemp = this->DesignEnteringWaterTemp;
                this->inletConds.AirTemp = this->DesignEnteringAirTemp;
                this->inletConds.AirWetBulb = this->DesignEnteringAirWetBulbTemp;
                this->inletConds.AirPress = state.dataEnvrn->StdBaroPress;
                this->inletConds.AirHumRat =
                    Psychrometrics::PsyWFnTdbTwbPb(state, this->inletConds.AirTemp, this->inletConds.AirWetBulb, this->inletConds.AirPress);
                auto f = [&state, this, DesEvapFluidCoolerLoad, par1, par2, Cp](Real64 UA) {
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA, this->DesignExitWaterTemp);
                    Real64 const CoolingOutput = Cp * par1 * (this->inletConds.WaterTemp - this->DesignExitWaterTemp);
                    return (DesEvapFluidCoolerLoad - CoolingOutput) / DesEvapFluidCoolerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating EvaporativeFluidCooler UA");
                    ShowFatalError(state, format("Autosizing of EvaporativeFluidCooler UA failed for EvaporativeFluidCooler {}", this->Name));
                } else if (SolFla == -2) {
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA0, OutWaterTempAtUA0);
                    this->SimSimpleEvapFluidCooler(state, par1, par2, UA1, OutWaterTempAtUA1);
                    ShowSevereError(state, format("{}: The combination of design input values did not allow the calculation of a ", CalledFrom));
                    ShowContinueError(state, "reasonable UA value. Review and revise design input values as appropriate. Specifying hard");
                    ShowContinueError(
                        state,
                        R"(sizes for some "autosizable" fields while autosizing other "autosizable" fields may be contributing to this problem.)");
                    ShowContinueError(state, "This model iterates on UA to find the heat transfer required to provide the design outlet ");
                    ShowContinueError(state, "water temperature. Initially, the outlet water temperatures at high and low UA values are ");
                    ShowContinueError(state, "calculated. The Design Exit Water Temperature should be between the outlet water ");
                    ShowContinueError(state, "temperatures calculated at high and low UA values. If the Design Exit Water Temperature is ");
                    ShowContinueError(state, "out of this range, the solution will not converge and UA will not be calculated. ");
                    ShowContinueError(state, "Inputs to the Evaporative Fluid Cooler model are:");
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Load                    = {:.2R}", DesEvapFluidCoolerLoad));
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Water Volume Flow Rate  = {:.2R}", par1));
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Air Volume Flow Rate    = {:.2R}", par2));
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Air Inlet Wet-bulb Temp = {:.2R}", this->inletConds.AirWetBulb));
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Water Inlet Temp        = {:.2R}", this->inletConds.WaterTemp));
                    ShowContinueError(state, format("Design Exit Water Temp                                  = {:.2R}", this->DesignExitWaterTemp));
                    ShowContinueError(state, format("Design Evaporative Fluid Cooler Water Inlet Temp [C]    = {:.2R}", this->inletConds.WaterTemp));
                    ShowContinueError(state, format("Calculated water outlet temperature at low UA({:.2R})  = {:.2R}", UA0, OutWaterTempAtUA0));
                    ShowContinueError(state, format("Calculated water outlet temperature at high UA({:.2R})  = {:.2R}", UA1, OutWaterTempAtUA1));
                    ShowFatalError(state, format("Autosizing of Evaporative Fluid Cooler UA failed for Evaporative Fluid Cooler = {}", this->Name));
                }
                this->LowSpeedEvapFluidCoolerUA = UA;
            } else {
                this->LowSpeedEvapFluidCoolerUA = 0.0;
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 this->EvapFluidCoolerType,
                                                 this->Name,
                                                 "U-Factor Times Area Value at Low Fan Speed [W/C]",
                                                 this->LowSpeedEvapFluidCoolerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 this->EvapFluidCoolerType,
                                                 this->Name,
                                                 "Initial U-Factor Times Area Value at Low Fan Speed [W/C]",
                                                 this->LowSpeedEvapFluidCoolerUA);
                }
            }
        }

        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            // create predefined report
            std::string equipName = this->Name;
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, equipName, this->EvapFluidCoolerType);
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchMechNomCap,
                                                     equipName,
                                                     max(this->HighSpeedStandardDesignCapacity, this->HighSpeedUserSpecifiedDesignCapacity));
        }

        // create std 229 new table for cooling towers and fluid coolers
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCTFCType, this->Name, DataPlant::PlantEquipTypeNames[static_cast<int>(this->Type)]);
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchCTFCCondLoopName,
                                                 this->Name,
                                                 this->plantLoc.loopNum > 0 ? state.dataPlnt->PlantLoop(this->plantLoc.loopNum).Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchCTFCCondLoopBranchName,
            this->Name,
            this->plantLoc.loopNum > 0 ? state.dataPlnt->PlantLoop(plantLoc.loopNum).LoopSide(plantLoc.loopSideNum).Branch(plantLoc.branchNum).Name
                                       : "N/A");
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchCTFCFluidType,
            this->Name,
            state.dataPlnt->PlantLoop(this->plantLoc.loopNum).FluidName); // Fluid Name more reasonable than FluidType
        if ((this->DesignExitWaterTemp > -999.0) && (this->DesignEnteringWaterTemp > 0)) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCRange, this->Name, this->DesignEnteringWaterTemp - this->DesignExitWaterTemp);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCRange, this->Name, "N/A");
        }
        if ((this->DesignExitWaterTemp > -999.0) && (this->DesignEnteringAirWetBulbTemp > 0)) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCApproach, this->Name, this->DesignExitWaterTemp - this->DesignEnteringAirWetBulbTemp);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCApproach, this->Name, "N/A");
        }
        if (this->DesignExitWaterTemp > -999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCLevWaterSPTemp, this->Name, this->DesignExitWaterTemp);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCLevWaterSPTemp, this->Name, "N/A");
        }
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCTFCDesFanPwr, this->Name, this->HighSpeedFanPower); // equivalent to Design Fan Power?
        if (this->DesignEnteringAirWetBulbTemp > 0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCDesInletAirWBT, this->Name, this->DesignEnteringAirWetBulbTemp);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCDesInletAirWBT, this->Name, "N/A");
        }
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCTFCDesWaterFlowRate, this->Name, this->DesignWaterFlowRate, 6);
    }

    void EvapFluidCoolerSpecs::CalcSingleSpeedEvapFluidCooler(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To simulate the operation of a single-speed fan evaporative fluid cooler.

        // METHODOLOGY EMPLOYED:
        // The evaporative fluid cooler is modeled using effectiveness-NTU relationships for
        // counterflow heat exchangers based on Merkel's theory.
        // The subroutine calculates the period of time required to meet a
        // leaving water temperature setpoint. It assumes that part-load
        // operation represents a linear interpolation of two steady-state regimes.
        // Cyclic losses are neglected. The period of time required to meet the
        // leaving water temperature setpoint is used to determine the required
        // fan power and energy.
        // A RunFlag is passed by the upper level manager to indicate the ON/OFF status,
        // or schedule, of the evaporative fluid cooler. If the evaporative fluid cooler is OFF, outlet water
        // temperature and flow rate are passed through the model from inlet node to
        // outlet node without intervention. Reports are also updated with fan power and energy being zero.
        // When the RunFlag indicates an ON condition for the evaporative fluid cooler, the
        // mass flow rate and water temperature are read from the inlet node of the
        // evaporative fluid cooler (water-side). The outdoor air wet-bulb temperature is used
        // as the entering condition to the evaporative fluid cooler (air-side).
        // The evaporative fluid cooler fan is turned on and design parameters are used
        // to calculate the leaving water temperature.
        // If the calculated leaving water temperature is below the setpoint, a fan
        // run-time fraction is calculated and used to determine fan power. The leaving
        // water temperature setpoint is placed on the outlet node. If the calculated
        // leaving water temperature is at or above the setpoint, the calculated
        // leaving water temperature is placed on the outlet node and the fan runs at
        // full power. Water mass flow rate is passed from inlet node to outlet node
        // with no intervention.
        // REFERENCES:
        // ASHRAE HVAC1KIT: A Toolkit for Primary HVAC System Energy Calculation. 1999.

        // Based on SingleSpeedTower subroutine by Dan Fisher ,Sept 1998
        // Dec. 2008. BG. added RunFlag logic per original methodology

        static constexpr std::string_view RoutineName("CalcSingleSpeedEvapFluidCooler");
        int constexpr MaxIteration(100); // Maximum fluid bypass iteration calculations
        std::string const MaxItChar("100");
        Real64 constexpr BypassFractionThreshold(0.01); // Threshold to stop bypass iteration
        Real64 constexpr OWTLowerLimit(0.0);            // The limit of evaporative fluid cooler exit fluid temperature used
        // in the fluid bypass calculation to avoid fluid freezing. For water,
        // it is 0 degreeC and for glycols, it can be much lower. The fluid type
        // is stored at the loop. Current choices are Water and Steam,
        // needs to expand for glycols

        Real64 AirFlowRate;
        Real64 UAdesign = 0.0; // UA value at design conditions (entered by user or calculated)
        Real64 CpWater;
        Real64 TempSetPoint = 0.0;

        // set inlet and outlet nodes
        this->WaterInletNode = this->WaterInletNodeNum;
        this->WaterOutletNode = this->WaterOutletNodeNum;
        this->Qactual = 0.0;
        this->FanPower = 0.0;
        Real64 inletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNode).Temp;
        this->OutletWaterTemp = inletWaterTemp;
        AirFlowRate = 0.0;

        auto const &loopSide = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum);
        DataPlant::LoopDemandCalcScheme calcScheme = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme;
        if (calcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {
            TempSetPoint = loopSide.TempSetPoint;
        } else if (calcScheme == DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand) {
            TempSetPoint = loopSide.TempSetPointHi;
        }

        // Added for fluid bypass. First assume no fluid bypass
        int BypassFlag = 0;
        this->BypassFraction = 0.0;

        //   MassFlowTol is a parameter to indicate a no flow condition
        if (this->WaterMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance ||
            state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).FlowLock == DataPlant::FlowLock::Unlocked) {
            return;
        }

        if (inletWaterTemp > TempSetPoint) {
            //     Turn on evaporative fluid cooler fan
            UAdesign = this->HighSpeedEvapFluidCoolerUA;
            AirFlowRate = this->HighSpeedAirFlowRate;
            Real64 FanPowerOn = this->HighSpeedFanPower;

            this->SimSimpleEvapFluidCooler(state, this->WaterMassFlowRate, AirFlowRate, UAdesign, this->OutletWaterTemp);

            if (this->OutletWaterTemp <= TempSetPoint) {
                if (this->capacityControl == CapacityControl::FanCycling || this->OutletWaterTemp <= OWTLowerLimit) {
                    //         Setpoint was met with pump ON and fan ON, calculate run-time fraction
                    Real64 FanModeFrac = (TempSetPoint - inletWaterTemp) / (this->OutletWaterTemp - inletWaterTemp);
                    this->FanPower = FanModeFrac * FanPowerOn;
                    this->OutletWaterTemp = TempSetPoint;
                } else {
                    // FluidBypass, fan runs at full speed for the entire time step
                    // FanModeFrac = 1.0;
                    this->FanPower = FanPowerOn;
                    BypassFlag = 1;
                }
            } else {
                //       Setpoint was not met, evaporative fluid cooler ran at full capacity
                // FanModeFrac = 1.0;
                this->FanPower = FanPowerOn;
            }
        } else {
            // Inlet water temperature lower than setpoint, assume 100% bypass, evaporative fluid cooler fan off
            if (this->capacityControl == CapacityControl::FluidBypass) {
                if (inletWaterTemp > OWTLowerLimit) {
                    this->FanPower = 0.0;
                    this->BypassFraction = 1.0;
                    this->OutletWaterTemp = inletWaterTemp;
                }
            }
        }

        // Calculate bypass fraction since OWTLowerLimit < OutletWaterTemp < TempSetPoint.
        // The iteration ends when the number of iteration exceeds the limit or the difference
        //  between the new and old bypass fractions is less than the threshold.
        if (BypassFlag == 1) {
            Real64 bypassFraction = (TempSetPoint - this->OutletWaterTemp) / (inletWaterTemp - this->OutletWaterTemp);
            if (bypassFraction > 1.0 || bypassFraction < 0.0) {
                // Bypass cannot meet setpoint, assume no bypass
                this->BypassFraction = 0.0;
                AirFlowRate = 0.0;
            } else {
                int NumIteration = 0;
                Real64 BypassFraction2 = 0.0; // Fluid bypass fraction
                Real64 BypassFractionPrev = bypassFraction;
                Real64 OutletWaterTempPrev = this->OutletWaterTemp;
                while (NumIteration < MaxIteration) {
                    ++NumIteration;
                    // need to iterate for the new OutletWaterTemp while bypassing evaporative fluid cooler water
                    this->SimSimpleEvapFluidCooler(
                        state, this->WaterMassFlowRate * (1.0 - bypassFraction), AirFlowRate, UAdesign, this->OutletWaterTemp);
                    // Calc new bypassFraction based on the new OutletWaterTemp
                    if (std::abs(this->OutletWaterTemp - OWTLowerLimit) <= 0.01) {
                        BypassFraction2 = bypassFraction;
                        break;
                    } else if (this->OutletWaterTemp < OWTLowerLimit) {
                        // Set OutletWaterTemp = OWTLowerLimit, and use linear interpolation to calculate the bypassFraction
                        BypassFraction2 = BypassFractionPrev - (BypassFractionPrev - bypassFraction) * (OutletWaterTempPrev - OWTLowerLimit) /
                                                                   (OutletWaterTempPrev - this->OutletWaterTemp);
                        this->SimSimpleEvapFluidCooler(
                            state, this->WaterMassFlowRate * (1.0 - BypassFraction2), AirFlowRate, UAdesign, this->OutletWaterTemp);
                        if (this->OutletWaterTemp < OWTLowerLimit) {
                            // Use previous iteration values
                            BypassFraction2 = BypassFractionPrev;
                            this->OutletWaterTemp = OutletWaterTempPrev;
                        }
                        break;
                    } else {
                        BypassFraction2 = (TempSetPoint - this->OutletWaterTemp) / (inletWaterTemp - this->OutletWaterTemp);
                    }
                    // Compare two bypassFraction to determine when to stop
                    if (std::abs(BypassFraction2 - bypassFraction) <= BypassFractionThreshold) {
                        break;
                    }
                    BypassFractionPrev = bypassFraction;
                    OutletWaterTempPrev = this->OutletWaterTemp;
                    bypassFraction = BypassFraction2;
                }
                if (NumIteration > MaxIteration) {
                    ShowWarningError(
                        state, format("Evaporative fluid cooler fluid bypass iteration exceeds maximum limit of {} for {}", MaxItChar, this->Name));
                }
                this->BypassFraction = BypassFraction2;
                // may not meet TempSetPoint due to limit of evaporative fluid cooler outlet temp to OWTLowerLimit
                this->OutletWaterTemp = (1.0 - BypassFraction2) * this->OutletWaterTemp + BypassFraction2 * inletWaterTemp;
            }
        }

        // Should this be water inlet node num?????
        CpWater = state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                      .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNode).Temp, RoutineName);
        this->Qactual = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNode).Temp - this->OutletWaterTemp);
        this->AirFlowRateRatio = AirFlowRate / this->HighSpeedAirFlowRate;
    }

    void EvapFluidCoolerSpecs::CalcTwoSpeedEvapFluidCooler(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To simulate the operation of a evaporative fluid cooler with a two-speed fan.

        // METHODOLOGY EMPLOYED:
        // The evaporative fluid cooler is modeled using effectiveness-NTU relationships for
        // counterflow heat exchangers based on Merkel's theory.
        // The subroutine calculates the period of time required to meet a
        // leaving water temperature setpoint. It assumes that part-load
        // operation represents a linear interpolation of three steady-state regimes
        // (high-speed fan operation and low-speed fan operation ).
        // Cyclic losses are neglected. The period of time required to meet the
        // leaving water temperature setpoint is used to determine the required
        // fan power and energy. When the leaving water temperature is at or above the setpoint
        // the evaporative fluid cooler fan is turned on,
        // .
        // A RunFlag is passed by the upper level manager to indicate the ON/OFF status,
        // or schedule, of the evaporative fluid cooler. If the evaporative fluid cooler is OFF, outlet water
        // temperature and flow rate are passed through the model from inlet node to
        // outlet node without intervention. Reports are also updated with fan power and fan energy being zero.
        // When the RunFlag indicates an ON condition for the evaporative fluid cooler, the
        // mass flow rate and water temperature are read from the inlet node of the
        // evaporative fluid cooler (water-side). The outdoor air wet-bulb temperature is used
        // as the entering condition to the evaporative fluid cooler (air-side). If the incoming
        // water temperature is above the setpoint, the evaporative fluid cooler fan is turned on
        // and parameters for low fan speed are used to again calculate the leaving
        // water temperature. If the calculated leaving water temperature is
        // below the setpoint, a fan run-time fraction (FanModeFrac) is calculated and
        // used to determine fan power. The leaving water temperature setpoint is placed
        // on the outlet node. If the calculated leaving water temperature is at or above
        // the setpoint, the evaporative fluid cooler fan is turned on 'high speed' and the routine is
        // repeated. If the calculated leaving water temperature is below the setpoint,
        // a fan run-time fraction is calculated for the second stage fan and fan power
        // is calculated as FanModeFrac*HighSpeedFanPower+(1-FanModeFrac)*LowSpeedFanPower.
        // If the calculated leaving water temperature is above the leaving water temp.
        // setpoint, the calculated leaving water temperature is placed on the outlet
        // node and the fan runs at full power (High Speed Fan Power). Water mass flow
        // rate is passed from inlet node to outlet node with no intervention.
        // REFERENCES:
        // ASHRAE HVAC1KIT: A Toolkit for Primary HVAC System Energy Calculation. 1999.
        // Based on TwoSpeedTower by Dan Fisher ,Sept. 1998
        // Dec. 2008. BG. added RunFlag logic per original methodology

        static constexpr std::string_view RoutineName("CalcTwoSpeedEvapFluidCooler");

        this->WaterInletNode = this->WaterInletNodeNum;
        this->WaterOutletNode = this->WaterOutletNodeNum;
        this->Qactual = 0.0;
        this->FanPower = 0.0;
        this->InletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNode).Temp;
        this->OutletWaterTemp = this->InletWaterTemp;

        Real64 OutletWaterTemp1stStage = this->OutletWaterTemp;
        Real64 OutletWaterTemp2ndStage = this->OutletWaterTemp;
        Real64 AirFlowRate = 0.0;
        Real64 TempSetPoint = 0.0;
        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            TempSetPoint = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            TempSetPoint = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).TempSetPointHi;
        } break;
        default:
            break;
        }

        //   MassFlowTol is a parameter to indicate a no flow condition
        if (this->WaterMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance ||
            state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).FlowLock == DataPlant::FlowLock::Unlocked) {
            return;
        }

        if (this->InletWaterTemp > TempSetPoint) {
            //     Setpoint was not met ,turn on evaporative fluid cooler 1st stage fan
            Real64 UAdesign = this->LowSpeedEvapFluidCoolerUA;
            AirFlowRate = this->LowSpeedAirFlowRate;
            Real64 FanPowerLow = this->LowSpeedFanPower;
            Real64 FanPowerHigh;
            this->SimSimpleEvapFluidCooler(state, this->WaterMassFlowRate, AirFlowRate, UAdesign, OutletWaterTemp1stStage);

            if (OutletWaterTemp1stStage <= TempSetPoint) {
                //         Setpoint was met with pump ON and fan ON 1st stage, calculate fan mode fraction
                Real64 FanModeFrac = (TempSetPoint - this->InletWaterTemp) / (OutletWaterTemp1stStage - this->InletWaterTemp);
                this->FanPower = FanModeFrac * FanPowerLow;
                this->OutletWaterTemp = TempSetPoint;
                this->Qactual *= FanModeFrac;
            } else {
                //         Setpoint was not met, turn on evaporative fluid cooler 2nd stage fan
                UAdesign = this->HighSpeedEvapFluidCoolerUA;
                AirFlowRate = this->HighSpeedAirFlowRate;
                FanPowerHigh = this->HighSpeedFanPower;

                this->SimSimpleEvapFluidCooler(state, this->WaterMassFlowRate, AirFlowRate, UAdesign, OutletWaterTemp2ndStage);

                if ((OutletWaterTemp2ndStage <= TempSetPoint) && UAdesign > 0.0) {
                    //           Setpoint was met with pump ON and fan ON 2nd stage, calculate fan mode fraction
                    Real64 FanModeFrac = (TempSetPoint - OutletWaterTemp1stStage) / (OutletWaterTemp2ndStage - OutletWaterTemp1stStage);
                    this->FanPower = (FanModeFrac * FanPowerHigh) + (1.0 - FanModeFrac) * FanPowerLow;
                    this->OutletWaterTemp = TempSetPoint;
                } else {
                    //           Setpoint was not met, evaporative fluid cooler ran at full capacity
                    this->OutletWaterTemp = OutletWaterTemp2ndStage;
                    this->FanPower = FanPowerHigh;
                }
            }
        }

        // Should this be water inlet node num??
        Real64 CpWater = state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                             .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNode).Temp, RoutineName);
        this->Qactual = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNode).Temp - this->OutletWaterTemp);
        this->AirFlowRateRatio = AirFlowRate / this->HighSpeedAirFlowRate;
    }

    void EvapFluidCoolerSpecs::SimSimpleEvapFluidCooler(
        EnergyPlusData &state, Real64 const waterMassFlowRate, Real64 const AirFlowRate, Real64 const UAdesign, Real64 &outletWaterTemp)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // See purpose for single speed or Two speed evaporative fluid cooler model

        // METHODOLOGY EMPLOYED:
        // See methodology for single speed or two speed evaporative fluid cooler model

        // REFERENCES:
        // Based on SimTower subroutine by Dan Fisher Sept. 1998
        // Merkel, F. 1925.  Verduftungskuhlung. VDI Forschungsarbeiten, Nr 275, Berlin.
        // ASHRAE     1999.  HVAC1KIT: A Toolkit for Primary HVAC System Energy Calculations.

        int constexpr IterMax(50);                  // Maximum number of iterations allowed
        Real64 constexpr WetBulbTolerance(0.00001); // Maximum error for exiting wet-bulb temperature between iterations
        Real64 constexpr DeltaTwbTolerance(0.001);  // Maximum error (tolerance) in DeltaTwb for iteration convergence [C]
        static constexpr std::string_view RoutineName("SimSimpleEvapFluidCooler");

        this->WaterInletNode = this->WaterInletNodeNum;
        this->WaterOutletNode = this->WaterOutletNodeNum;
        Real64 qActual = 0.0;
        Real64 WetBulbError = 1.0;
        Real64 DeltaTwb = 1.0;

        // set local evaporative fluid cooler inlet and outlet temperature variables
        this->InletWaterTemp = this->inletConds.WaterTemp;
        outletWaterTemp = this->InletWaterTemp;
        Real64 InletAirTemp = this->inletConds.AirTemp;
        Real64 InletAirWetBulb = this->inletConds.AirWetBulb;

        if (UAdesign == 0.0) {
            return;
        }

        // set water and air properties
        Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, this->inletConds.AirPress, InletAirTemp, this->inletConds.AirHumRat);
        Real64 AirMassFlowRate = AirFlowRate * AirDensity;
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->inletConds.AirHumRat);
        Real64 CpWater = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->InletWaterTemp, RoutineName);
        Real64 InletAirEnthalpy = Psychrometrics::PsyHFnTdbRhPb(state, InletAirWetBulb, 1.0, this->inletConds.AirPress);

        // initialize exiting wet bulb temperature before iterating on final solution
        Real64 OutletAirWetBulb = InletAirWetBulb + 6.0;

        // Calculate mass flow rates
        Real64 MdotCpWater = waterMassFlowRate * CpWater;
        int Iter = 0;
        while ((WetBulbError > WetBulbTolerance) && (Iter <= IterMax) && (DeltaTwb > DeltaTwbTolerance)) {
            ++Iter;
            Real64 OutletAirEnthalpy = Psychrometrics::PsyHFnTdbRhPb(state, OutletAirWetBulb, 1.0, this->inletConds.AirPress);
            // calculate the airside specific heat and capacity
            Real64 CpAirside = (OutletAirEnthalpy - InletAirEnthalpy) / (OutletAirWetBulb - InletAirWetBulb);
            Real64 AirCapacity = AirMassFlowRate * CpAirside;
            // calculate the minimum to maximum capacity ratios of airside and waterside
            Real64 CapacityRatioMin = min(AirCapacity, MdotCpWater);
            Real64 CapacityRatioMax = max(AirCapacity, MdotCpWater);
            Real64 CapacityRatio = CapacityRatioMin / CapacityRatioMax;
            // Calculate heat transfer coefficient and number of transfer units (NTU)
            Real64 UAactual = UAdesign * CpAirside / CpAir;
            Real64 NumTransferUnits = UAactual / CapacityRatioMin;
            // calculate heat exchanger effectiveness
            Real64 effectiveness;
            if (CapacityRatio <= 0.995) {
                effectiveness = (1.0 - std::exp(-1.0 * NumTransferUnits * (1.0 - CapacityRatio))) /
                                (1.0 - CapacityRatio * std::exp(-1.0 * NumTransferUnits * (1.0 - CapacityRatio)));
            } else {
                effectiveness = NumTransferUnits / (1.0 + NumTransferUnits);
            }
            // calculate water to air heat transfer and store last exiting WB temp of air
            qActual = effectiveness * CapacityRatioMin * (this->InletWaterTemp - InletAirWetBulb);
            Real64 OutletAirWetBulbLast = OutletAirWetBulb;
            // calculate new exiting wet bulb temperature of airstream
            OutletAirWetBulb = InletAirWetBulb + qActual / AirCapacity;
            // Check error tolerance and exit if satisfied
            DeltaTwb = std::abs(OutletAirWetBulb - InletAirWetBulb);
            // Add Constant::Kelvin() to denominator below convert OutletAirWetBulbLast to Kelvin to avoid divide by zero.
            // Wet bulb error units are delta K/K
            WetBulbError = std::abs((OutletAirWetBulb - OutletAirWetBulbLast) / (OutletAirWetBulbLast + Constant::Kelvin));
        }

        if (qActual >= 0.0) {
            outletWaterTemp = this->InletWaterTemp - qActual / MdotCpWater;
        } else {
            outletWaterTemp = this->InletWaterTemp;
        }
    }

    void EvapFluidCoolerSpecs::CalculateWaterUsage(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   May 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Collect evaporative fluid cooler water usage calculations for
        // reuse by all the evaporative fluid cooler models.

        // REFERENCES:
        // Based on CalculateWaterUsage subroutine for cooling tower by B. Griffith, August 2006

        static constexpr std::string_view RoutineName("CalculateWaterUsage");

        this->BlowdownVdot = 0.0;
        this->EvaporationVdot = 0.0;

        Real64 AverageWaterTemp = (this->InletWaterTemp + this->OutletWaterTemp) / 2.0;

        // Set water and air properties
        if (this->EvapLossMode == EvapLoss::ByMoistTheory) {

            Real64 AirDensity =
                Psychrometrics::PsyRhoAirFnPbTdbW(state, this->inletConds.AirPress, this->inletConds.AirTemp, this->inletConds.AirHumRat);
            Real64 AirMassFlowRate = this->AirFlowRateRatio * this->HighSpeedAirFlowRate * AirDensity;
            Real64 InletAirEnthalpy = Psychrometrics::PsyHFnTdbRhPb(state, this->inletConds.AirWetBulb, 1.0, this->inletConds.AirPress);

            if (AirMassFlowRate > 0.0) {
                // Calculate outlet air conditions for determining water usage

                Real64 OutletAirEnthalpy = InletAirEnthalpy + this->Qactual / AirMassFlowRate;
                Real64 OutletAirTSat = Psychrometrics::PsyTsatFnHPb(state, OutletAirEnthalpy, this->inletConds.AirPress);
                Real64 OutletAirHumRatSat = Psychrometrics::PsyWFnTdbH(state, OutletAirTSat, OutletAirEnthalpy);

                // calculate specific humidity ratios (HUMRAT to mass of moist air not dry air)
                Real64 InSpecificHumRat = this->inletConds.AirHumRat / (1 + this->inletConds.AirHumRat);
                Real64 OutSpecificHumRat = OutletAirHumRatSat / (1 + OutletAirHumRatSat);

                // calculate average air temp for density call
                Real64 TairAvg = (this->inletConds.AirTemp + OutletAirTSat) / 2.0;

                // Amount of water evaporated
                Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, TairAvg, RoutineName);
                this->EvaporationVdot = (AirMassFlowRate * (OutSpecificHumRat - InSpecificHumRat)) / rho; // [m3/s]
                if (this->EvaporationVdot < 0.0) {
                    this->EvaporationVdot = 0.0;
                }
            } else {
                this->EvaporationVdot = 0.0;
            }

        } else if (this->EvapLossMode == EvapLoss::ByUserFactor) {
            Real64 rho = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, AverageWaterTemp, RoutineName);
            this->EvaporationVdot = this->UserEvapLossFactor * (this->InletWaterTemp - this->OutletWaterTemp) * (this->WaterMassFlowRate / rho);
            if (this->EvaporationVdot < 0.0) {
                this->EvaporationVdot = 0.0;
            }
        } else {
            // should never come here
        }

        //   amount of water lost due to drift
        this->DriftVdot = this->DesignSprayWaterFlowRate * this->DriftLossFraction * this->AirFlowRateRatio;

        if (this->BlowdownMode == Blowdown::BySchedule) {
            // Amount of water lost due to blow down (purging contaminants from evaporative fluid cooler basin)
            this->BlowdownVdot = (this->blowdownSched != nullptr) ? this->blowdownSched->getCurrentVal() : 0.0;
        } else if (this->BlowdownMode == Blowdown::ByConcentration) {
            if (this->ConcentrationRatio > 2.0) { // protect divide by zero
                this->BlowdownVdot = this->EvaporationVdot / (this->ConcentrationRatio - 1) - this->DriftVdot;
            } else {
                this->BlowdownVdot = this->EvaporationVdot - this->DriftVdot;
            }
            if (this->BlowdownVdot < 0.0) {
                this->BlowdownVdot = 0.0;
            }
        } else {
            // should never come here
        }

        // Added for fluid bypass
        if (this->capacityControl == CapacityControl::FluidBypass) {
            if (this->EvapLossMode == EvapLoss::ByUserFactor) {
                this->EvaporationVdot *= (1 - this->BypassFraction);
            }
            this->DriftVdot *= (1 - this->BypassFraction);
            this->BlowdownVdot *= (1 - this->BypassFraction);
        }

        this->MakeUpVdot = this->EvaporationVdot + this->DriftVdot + this->BlowdownVdot;

        // set demand request in Water Storage if needed
        this->StarvedMakeUpVdot = 0.0;
        this->TankSupplyVdot = 0.0;
        if (this->SuppliedByWaterSystem) {

            // set demand request
            state.dataWaterData->WaterStorage(this->WaterTankID).VdotRequestDemand(this->WaterTankDemandARRID) = this->MakeUpVdot;

            Real64 AvailTankVdot = state.dataWaterData->WaterStorage(this->WaterTankID)
                                       .VdotAvailDemand(this->WaterTankDemandARRID); // check what tank can currently provide

            this->TankSupplyVdot = this->MakeUpVdot; // init
            if (AvailTankVdot < this->MakeUpVdot) {  // calculate starved flow
                this->StarvedMakeUpVdot = this->MakeUpVdot - AvailTankVdot;
                this->TankSupplyVdot = AvailTankVdot;
            }
        } else { // supplied by mains
        }

        //   total water usage
        // update report variables
        this->EvaporationVol = this->EvaporationVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->DriftVol = this->DriftVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->BlowdownVol = this->BlowdownVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->MakeUpVol = this->MakeUpVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->TankSupplyVol = this->TankSupplyVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->StarvedMakeUpVol = this->StarvedMakeUpVdot * (state.dataHVACGlobal->TimeStepSysSec);
    }

    void EvapFluidCoolerSpecs::UpdateEvapFluidCooler(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Chandan Sharma
        //       DATE WRITTEN:    May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for passing results to the outlet water node.

        Real64 constexpr TempAllowance(0.02); // Minimum difference b/w fluid cooler water outlet temp and

        state.dataLoopNodes->Node(this->WaterOutletNode).Temp = this->OutletWaterTemp;

        if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).FlowLock == DataPlant::FlowLock::Unlocked ||
            state.dataGlobal->WarmupFlag) {
            return;
        }

        // Check flow rate through evaporative fluid cooler and compare to design flow rate,
        // show warning if greater than Design * Multiplier
        if (state.dataLoopNodes->Node(this->WaterOutletNode).MassFlowRate >
            this->DesWaterMassFlowRate * this->EvapFluidCoolerMassFlowRateMultiplier) {
            ++this->HighMassFlowErrorCount;
            if (this->HighMassFlowErrorCount < 2) {
                ShowWarningError(state, format("{} \"{}\"", this->EvapFluidCoolerType, this->Name));
                ShowContinueError(state, " Condenser Loop Mass Flow Rate is much greater than the evaporative fluid coolers design mass flow rate.");
                ShowContinueError(state,
                                  format(" Condenser Loop Mass Flow Rate = {:.6T}", state.dataLoopNodes->Node(this->WaterOutletNode).MassFlowRate));
                ShowContinueError(state, format(" Evaporative Fluid Cooler Design Mass Flow Rate   = {:.6T}", this->DesWaterMassFlowRate));
                ShowContinueErrorTimeStamp(state, "");
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state,
                    this->EvapFluidCoolerType + " \"" + this->Name +
                        "\"  Condenser Loop Mass Flow Rate is much greater than the evaporative fluid coolers design mass flow rate error",
                    this->HighMassFlowErrorIndex,
                    state.dataLoopNodes->Node(this->WaterOutletNode).MassFlowRate,
                    state.dataLoopNodes->Node(this->WaterOutletNode).MassFlowRate);
            }
        }

        // Check if OutletWaterTemp is below the minimum condenser loop temp and warn user
        Real64 LoopMinTemp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).MinTemp;
        Real64 TempDifference = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).MinTemp - this->OutletWaterTemp;
        if (TempDifference > TempAllowance && this->WaterMassFlowRate > 0.0) {
            ++this->OutletWaterTempErrorCount;
            if (this->OutletWaterTempErrorCount < 2) {
                ShowWarningError(state, format("{} \"{}\"", this->EvapFluidCoolerType, this->Name));
                ShowContinueError(state,
                                  format("Evaporative fluid cooler water outlet temperature ({:6.2F} C) is below the specified minimum condenser "
                                         "loop temp of {:6.2F} C",
                                         this->OutletWaterTemp,
                                         LoopMinTemp));
                ShowContinueErrorTimeStamp(state, "");
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state,
                    this->EvapFluidCoolerType + " \"" + this->Name +
                        "\" Evaporative fluid cooler water outlet temperature is below the specified minimum condenser loop temp error",
                    this->OutletWaterTempErrorIndex,
                    this->OutletWaterTemp,
                    this->OutletWaterTemp);
            }
        }

        // Check if water mass flow rate is small (e.g. no flow) and warn user
        if (this->WaterMassFlowRate > 0.0 && this->WaterMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance) {
            ++this->SmallWaterMassFlowErrorCount;
            if (this->SmallWaterMassFlowErrorCount < 2) {
                ShowWarningError(state, format("{} \"{}\"", this->EvapFluidCoolerType, this->Name));
                ShowContinueError(state, "Evaporative fluid cooler water mass flow rate near zero.");
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Actual Mass flow = {:.2T}", this->WaterMassFlowRate));
            } else {
                ShowRecurringWarningErrorAtEnd(state,
                                               this->EvapFluidCoolerType + " \"" + this->Name +
                                                   "\" Evaporative fluid cooler water mass flow rate near zero error continues...",
                                               this->SmallWaterMassFlowErrorIndex,
                                               this->WaterMassFlowRate,
                                               this->WaterMassFlowRate);
            }
        }
    }

    void EvapFluidCoolerSpecs::ReportEvapFluidCooler(EnergyPlusData &state, bool const RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Chandan Sharma
        //       DATE WRITTEN:    May 2009
        //       MODIFIED         na
        //       RE-ENGINEERED    na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variables for the evaporative fluid cooler.

        Real64 const ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        if (!RunFlag) {
            this->fluidCoolerInletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNode).Temp;
            this->fluidCoolerOutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNode).Temp;
            this->Qactual = 0.0;
            this->FanPower = 0.0;
            this->FanEnergy = 0.0;
            this->AirFlowRateRatio = 0.0;
            this->WaterAmountUsed = 0.0;
            this->BypassFraction = 0.0;
        } else {
            this->fluidCoolerInletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNode).Temp;
            this->fluidCoolerOutletWaterTemp = this->OutletWaterTemp;
            this->FanEnergy = this->FanPower * ReportingConstant;
            this->WaterAmountUsed = this->WaterUsage * ReportingConstant;
        }
    }

    void EvapFluidCoolerSpecs::oneTimeInit(EnergyPlusData &state)
    {
        bool ErrorsFound(false); // Flag if input data errors are found

        if (this->MyOneTimeFlag) {

            this->setupOutputVars(state);

            this->glycol = state.dataPlnt->PlantLoop(state.dataSize->CurLoopNum).glycol;
            std::string const &FluidName = this->glycol->Name;

            if (Util::SameString(this->PerformanceInputMethod, "STANDARDDESIGNCAPACITY")) {
                this->PerformanceInputMethod_Num = PIM::StandardDesignCapacity;
                if (FluidName != "WATER") {
                    ShowSevereError(state,
                                    format("{} = \"{}{}",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           this->Name,
                                           R"(". StandardDesignCapacity performance input method is only valid for fluid type = "Water".)"));
                    ShowContinueError(state,
                                      format("Currently, Fluid Type = {} in CondenserLoop = {}",
                                             FluidName,
                                             state.dataPlnt->PlantLoop(state.dataSize->CurLoopNum).Name));
                    ErrorsFound = true;
                }
            }

            this->MyOneTimeFlag = false;
        }

        if (this->OneTimeFlagForEachEvapFluidCooler) {
            // Locate the tower on the plant loops for later usage
            PlantUtilities::ScanPlantLoopsForObject(state, this->Name, this->Type, this->plantLoc, ErrorsFound, _, _, _, _, _);

            if (ErrorsFound) {
                ShowFatalError(state, "InitEvapFluidCooler: Program terminated due to previous condition(s).");
            }

            if (this->Type == DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd) {
                if (this->DesignWaterFlowRate > 0.0) {
                    if (this->HighSpeedAirFlowRate <= this->LowSpeedAirFlowRate) {
                        ShowSevereError(
                            state,
                            format("EvaporativeFluidCooler:TwoSpeed \"{}\". Low speed air flow rate must be less than the high speed air flow rate.",
                                   this->Name));
                        ErrorsFound = true;
                    }
                    if ((this->HighSpeedEvapFluidCoolerUA > 0.0) && (this->LowSpeedEvapFluidCoolerUA > 0.0) &&
                        (this->HighSpeedEvapFluidCoolerUA <= this->LowSpeedEvapFluidCoolerUA)) {
                        ShowSevereError(state,
                                        format("EvaporativeFluidCooler:TwoSpeed \"{}\". Evaporative fluid cooler UA at low fan speed must be less "
                                               "than the evaporative fluid cooler UA at high fan speed.",
                                               this->Name));
                        ErrorsFound = true;
                    }
                }
            }

            if (ErrorsFound) {
                ShowFatalError(state, "InitEvapFluidCooler: Program terminated due to previous condition(s).");
            }

            this->OneTimeFlagForEachEvapFluidCooler = false;
        }
    }

} // namespace EvaporativeFluidCoolers

} // namespace EnergyPlus
