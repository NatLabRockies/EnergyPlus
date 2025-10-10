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
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HWBaseboardRadiator.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace HWBaseboardRadiator {

    // Module -- (ref: Object: ZoneHVAC:Baseboard:RadiantConvective:Water)

    // Module containing the routines dealing with the hot water baseboard heaters

    // MODULE INFORMATION:
    //       AUTHOR         Daeho Kang
    //       DATE WRITTEN   Aug 2007

    // PURPOSE OF THIS MODULE:
    // The purpose of this module is to simulate hot water baseboard heaters.

    // REFERENCES:
    // 1. I=B=R Ratings for Baseboards, Baseboard Radiation,
    //   Finned Tube (Commercial) Radiation, and Indirect Fired Water Heaters, January 2007 Edition
    // 2. Incropera and DeWitt, Fundamentals of Heat and Mass Transfer, Chapter 11.3 and 11.4,
    //   eq. 11.15, 11.17, and 11.33

    std::string const cCMO_BBRadiator_Water("ZoneHVAC:Baseboard:RadiantConvective:Water");
    std::string const cCMO_BBRadiator_Water_Design("ZoneHVAC:Baseboard:RadiantConvective:Water:Design");

    void SimHWBaseboard(EnergyPlusData &state,
                        std::string const &EquipName,
                        int const ControlledZoneNum,
                        bool const FirstHVACIteration,
                        Real64 &PowerMet,
                        int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the Baseboard Radiators.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int BaseboardNum; // Index of unit in baseboard array
        Real64 QZnReq;    // Zone load not yet satisfied
        Real64 MaxWaterFlow;
        Real64 MinWaterFlow;

        if (state.dataHWBaseboardRad->GetInputFlag) {
            GetHWBaseboardInput(state);
            state.dataHWBaseboardRad->GetInputFlag = false;
        }

        int const NumHWBaseboards = state.dataHWBaseboardRad->NumHWBaseboards;

        // Find the correct Baseboard Equipment
        if (CompIndex == 0) {
            BaseboardNum = Util::FindItemInList(EquipName, state.dataHWBaseboardRad->HWBaseboard, &HWBaseboardParams::Name);
            if (BaseboardNum == 0) {
                ShowFatalError(state, format("SimHWBaseboard: Unit not found={}", EquipName));
            }
            CompIndex = BaseboardNum;
        } else {
            BaseboardNum = CompIndex;
            if (BaseboardNum > NumHWBaseboards || BaseboardNum < 1) {
                ShowFatalError(state,
                               format("SimHWBaseboard:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      BaseboardNum,
                                      NumHWBaseboards,
                                      EquipName));
            }
            if (state.dataHWBaseboardRad->CheckEquipName(BaseboardNum)) {
                if (EquipName != state.dataHWBaseboardRad->HWBaseboard(BaseboardNum).Name) {
                    ShowFatalError(state,
                                   format("SimHWBaseboard: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          BaseboardNum,
                                          EquipName,
                                          state.dataHWBaseboardRad->HWBaseboard(BaseboardNum).Name));
                }
                state.dataHWBaseboardRad->CheckEquipName(BaseboardNum) = false;
            }
        }

        if (CompIndex > 0) {
            auto &HWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);

            HWBaseboardDesignData const &HWBaseboardDesignDataObject = state.dataHWBaseboardRad->HWBaseboardDesignObject(
                HWBaseboard.DesignObjectPtr); // Contains the data for variable flow hydronic systems

            InitHWBaseboard(state, BaseboardNum, ControlledZoneNum, FirstHVACIteration);

            QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;

            // On the first HVAC iteration the system values are given to the controller, but after that
            // the demand limits are in place and there needs to be feedback to the Zone Equipment
            if (FirstHVACIteration) {
                MaxWaterFlow = HWBaseboard.WaterMassFlowRateMax;
                MinWaterFlow = 0.0;
            } else {
                MaxWaterFlow = state.dataLoopNodes->Node(HWBaseboard.WaterInletNode).MassFlowRateMaxAvail;
                MinWaterFlow = state.dataLoopNodes->Node(HWBaseboard.WaterInletNode).MassFlowRateMinAvail;
            }

            switch (HWBaseboard.EquipType) {
            case DataPlant::PlantEquipmentType::Baseboard_Rad_Conv_Water: { // 'ZoneHVAC:Baseboard:RadiantConvective:Water'
                ControlCompOutput(state,
                                  HWBaseboard.Name,
                                  cCMO_BBRadiator_Water,
                                  BaseboardNum,
                                  FirstHVACIteration,
                                  QZnReq,
                                  HWBaseboard.WaterInletNode,
                                  MaxWaterFlow,
                                  MinWaterFlow,
                                  HWBaseboardDesignDataObject.Offset,
                                  HWBaseboard.ControlCompTypeNum,
                                  HWBaseboard.CompErrIndex,
                                  _,
                                  _,
                                  _,
                                  _,
                                  _,
                                  HWBaseboard.plantLoc);
            } break;
            default: {
                ShowSevereError(state, format("SimBaseboard: Errors in Baseboard={}", HWBaseboard.Name));
                ShowContinueError(state, format("Invalid or unimplemented equipment type={}", HWBaseboard.EquipType));
                ShowFatalError(state, "Preceding condition causes termination.");
            } break;
            }

            PowerMet = HWBaseboard.TotPower;

            UpdateHWBaseboard(state, BaseboardNum);

            ReportHWBaseboard(state, BaseboardNum);

        } else {
            ShowFatalError(state, format("SimHWBaseboard: Unit not found={}", EquipName));
        }
    }

    void GetHWBaseboardInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Aug 2007

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the baseboard units.

        // METHODOLOGY EMPLOYED:
        // Standard input processor calls.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetHWBaseboardInput:");
        static constexpr std::string_view routineName = "GetHWBaseboardInput";

        Real64 constexpr MaxFraction(1.0);
        Real64 constexpr MinFraction(0.0);
        Real64 constexpr MaxWaterTempAvg(150.0);              // Maximum limit of average water temperature in degree C
        Real64 constexpr MinWaterTempAvg(20.0);               // Minimum limit of average water temperature in degree C
        Real64 constexpr HighWaterMassFlowRate(10.0);         // Maximum limit of water mass flow rate in kg/s
        Real64 constexpr LowWaterMassFlowRate(0.00001);       // Minimum limit of water mass flow rate in kg/s
        Real64 constexpr MaxWaterFlowRate(10.0);              // Maximum limit of water volume flow rate in m3/s
        Real64 constexpr MinWaterFlowRate(0.00001);           // Minimum limit of water volume flow rate in m3/s
        Real64 constexpr WaterMassFlowDefault(0.063);         // Default water mass flow rate in kg/s
        int constexpr MinDistribSurfaces(1);                  // Minimum number of surfaces that a baseboard heater can radiate to
        int constexpr iHeatCAPMAlphaNum(2);                   // get input index to HW baseboard heating capacity sizing method
        int constexpr iHeatDesignCapacityNumericNum(3);       // get input index to HW baseboard heating capacity
        int constexpr iHeatCapacityPerFloorAreaNumericNum(1); // get input index to HW baseboard heating capacity per floor area sizing
        int constexpr iHeatFracOfAutosizedCapacityNumericNum(
            2); //  get input index to HW baseboard heating capacity sizing as fraction of autosized heating capacity

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int BaseboardNum; // Baseboard number
        int BaseboardDesignNum;
        int NumAlphas;  // Number of Alphas for each GetobjectItem call
        int NumNumbers; // Number of Numbers for each GetobjectItem call
        int SurfNum;    // Surface number Do loop counter
        int IOStat;
        bool ErrorsFound(false); // If errors detected in input

        // Update Nums in state and make local convenience copies
        int NumHWBaseboards = state.dataHWBaseboardRad->NumHWBaseboards =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_BBRadiator_Water);
        int NumHWBaseboardDesignObjs = state.dataHWBaseboardRad->NumHWBaseboardDesignObjs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_BBRadiator_Water_Design);

        // Count total number of baseboard units

        state.dataHWBaseboardRad->HWBaseboard.allocate(NumHWBaseboards);
        state.dataHWBaseboardRad->HWBaseboardDesignObject.allocate(NumHWBaseboardDesignObjs);
        state.dataHWBaseboardRad->CheckEquipName.allocate(NumHWBaseboards);
        state.dataHWBaseboardRad->HWBaseboardNumericFields.allocate(NumHWBaseboards);
        Array1D_string HWBaseboardDesignNames;
        HWBaseboardDesignNames.allocate(NumHWBaseboardDesignObjs);
        state.dataHWBaseboardRad->CheckEquipName = true;

        // Get the data from the user input related to design data for baseboard heaters
        for (BaseboardDesignNum = 1; BaseboardDesignNum <= NumHWBaseboardDesignObjs; ++BaseboardDesignNum) {
            auto &thisHWBaseboardDesign = state.dataHWBaseboardRad->HWBaseboardDesignObject(BaseboardDesignNum);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCMO_BBRadiator_Water_Design,
                                                                     BaseboardDesignNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            // ErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueBaseboardName(
                state, cCMO_BBRadiator_Water_Design, state.dataIPShortCut->cAlphaArgs(1), ErrorsFound, cCMO_BBRadiator_Water_Design + " Name");

            thisHWBaseboardDesign.designName = state.dataIPShortCut->cAlphaArgs(1); // Name of this baseboard design object
            HWBaseboardDesignNames(BaseboardDesignNum) = thisHWBaseboardDesign.designName;

            // Determine HW radiant baseboard heating design capacity sizing method
            thisHWBaseboardDesign.HeatingCapMethod = static_cast<DataSizing::DesignSizingType>(
                getEnumValue(DataSizing::DesignSizingTypeNamesUC, state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
            if (thisHWBaseboardDesign.HeatingCapMethod == DataSizing::DesignSizingType::CapacityPerFloorArea) {
                if (!state.dataIPShortCut->lNumericFieldBlanks(iHeatCapacityPerFloorAreaNumericNum)) {
                    thisHWBaseboardDesign.ScaledHeatingCapacity = state.dataIPShortCut->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum);
                    if (thisHWBaseboardDesign.ScaledHeatingCapacity <= 0.0) {
                        ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboardDesign.designName));
                        ShowContinueError(state,
                                          format("Input for {} = {}",
                                                 state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                                 state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 state.dataIPShortCut->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum),
                                                 state.dataIPShortCut->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    } else if (thisHWBaseboardDesign.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboardDesign.designName));
                        ShowContinueError(state,
                                          format("Input for {} = {}",
                                                 state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                                 state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(
                            state, format("Illegal {} = Autosize", state.dataIPShortCut->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboardDesign.designName));
                    ShowContinueError(state,
                                      format("Input for {} = {}",
                                             state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                             state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ShowContinueError(
                        state,
                        format("Blank field not allowed for {}", state.dataIPShortCut->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                    ErrorsFound = true;
                }
            } else if (thisHWBaseboardDesign.HeatingCapMethod == DataSizing::DesignSizingType::FractionOfAutosizedHeatingCapacity) {
                if (!state.dataIPShortCut->lNumericFieldBlanks(iHeatFracOfAutosizedCapacityNumericNum)) {
                    thisHWBaseboardDesign.ScaledHeatingCapacity = state.dataIPShortCut->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum);
                    if (thisHWBaseboardDesign.ScaledHeatingCapacity < 0.0) {
                        ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboardDesign.designName));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 state.dataIPShortCut->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum),
                                                 state.dataIPShortCut->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboardDesign.designName));
                    ShowContinueError(state,
                                      format("Input for {} = {}",
                                             state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                             state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ShowContinueError(
                        state,
                        format("Blank field not allowed for {}", state.dataIPShortCut->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum)));
                    ErrorsFound = true;
                }
            }

            thisHWBaseboardDesign.Offset = state.dataIPShortCut->rNumericArgs(3);
            // Set default convergence tolerance
            if (thisHWBaseboardDesign.Offset <= 0.0) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was less than the allowable minimum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water_Design,
                                        thisHWBaseboardDesign.designName,
                                        state.dataIPShortCut->cNumericFieldNames(3)));
                ShowContinueError(state, format("...reset to a default value=[{:.2R}].", MaxFraction));
                thisHWBaseboardDesign.Offset = 0.001;
            }

            thisHWBaseboardDesign.FracRadiant = state.dataIPShortCut->rNumericArgs(4);
            if (thisHWBaseboardDesign.FracRadiant < MinFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        thisHWBaseboardDesign.designName,
                                        state.dataIPShortCut->cNumericFieldNames(4)));
                ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinFraction));
                thisHWBaseboardDesign.FracRadiant = MinFraction;
            }
            if (thisHWBaseboardDesign.FracRadiant > MaxFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        thisHWBaseboardDesign.designName,
                                        state.dataIPShortCut->cNumericFieldNames(4)));
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
                thisHWBaseboardDesign.FracRadiant = MaxFraction;
            }

            thisHWBaseboardDesign.FracDistribPerson = state.dataIPShortCut->rNumericArgs(5);
            if (thisHWBaseboardDesign.FracDistribPerson < MinFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        thisHWBaseboardDesign.designName,
                                        state.dataIPShortCut->cNumericFieldNames(5)));
                ShowContinueError(state, format("...reset to minimum value=[{:.3R}].", MinFraction));
                thisHWBaseboardDesign.FracDistribPerson = MinFraction;
            }
            if (thisHWBaseboardDesign.FracDistribPerson > MaxFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        thisHWBaseboardDesign.designName,
                                        state.dataIPShortCut->cNumericFieldNames(5)));
                ShowContinueError(state, format("...reset to maximum value=[{:.3R}].", MaxFraction));
                thisHWBaseboardDesign.FracDistribPerson = MaxFraction;
            }
        }

        // Get the data from the user input related to baseboard heaters
        for (BaseboardNum = 1; BaseboardNum <= NumHWBaseboards; ++BaseboardNum) {
            auto &thisHWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);
            auto &HWBaseboardNumericFields = state.dataHWBaseboardRad->HWBaseboardNumericFields(BaseboardNum);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCMO_BBRadiator_Water,
                                                                     BaseboardNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCMO_BBRadiator_Water, state.dataIPShortCut->cAlphaArgs(1)};

            HWBaseboardNumericFields.FieldNames.allocate(NumNumbers);
            HWBaseboardNumericFields.FieldNames = state.dataIPShortCut->cNumericFieldNames;

            // ErrorsFound will be set to True if problem was found, left untouched otherwise
            GlobalNames::VerifyUniqueBaseboardName(
                state, cCMO_BBRadiator_Water, state.dataIPShortCut->cAlphaArgs(1), ErrorsFound, cCMO_BBRadiator_Water + " Name");

            thisHWBaseboard.Name = state.dataIPShortCut->cAlphaArgs(1);                          // Name of this baseboard
            thisHWBaseboard.EquipType = DataPlant::PlantEquipmentType::Baseboard_Rad_Conv_Water; //'ZoneHVAC:Baseboard:RadiantConvective:Water'

            Util::setDesignObjectNameAndPointer(state,
                                                thisHWBaseboard.designObjectName,
                                                thisHWBaseboard.DesignObjectPtr,
                                                state.dataIPShortCut->cAlphaArgs(2),
                                                HWBaseboardDesignNames,
                                                cCMO_BBRadiator_Water,
                                                state.dataIPShortCut->cAlphaArgs(1),
                                                ErrorsFound);
            if (ErrorsFound) {
                break;
            }
            HWBaseboardDesignData &HWBaseboardDesignDataObject =
                state.dataHWBaseboardRad->HWBaseboardDesignObject(thisHWBaseboard.DesignObjectPtr); // Contains the data for the design object

            // Get schedule
            if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
                thisHWBaseboard.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisHWBaseboard.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
                ErrorsFound = true;
            }

            // Get inlet node number
            thisHWBaseboard.WaterInletNode = GetOnlySingleNode(state,
                                                               state.dataIPShortCut->cAlphaArgs(4),
                                                               ErrorsFound,
                                                               DataLoopNode::ConnectionObjectType::ZoneHVACBaseboardRadiantConvectiveWater,
                                                               state.dataIPShortCut->cAlphaArgs(1),
                                                               DataLoopNode::NodeFluidType::Water,
                                                               DataLoopNode::ConnectionType::Inlet,
                                                               NodeInputManager::CompFluidStream::Primary,
                                                               DataLoopNode::ObjectIsNotParent);

            // Get outlet node number
            thisHWBaseboard.WaterOutletNode = GetOnlySingleNode(state,
                                                                state.dataIPShortCut->cAlphaArgs(5),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::ZoneHVACBaseboardRadiantConvectiveWater,
                                                                state.dataIPShortCut->cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Water,
                                                                DataLoopNode::ConnectionType::Outlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state,
                                               cCMO_BBRadiator_Water,
                                               state.dataIPShortCut->cAlphaArgs(1),
                                               state.dataIPShortCut->cAlphaArgs(4),
                                               state.dataIPShortCut->cAlphaArgs(5),
                                               "Hot Water Nodes");

            thisHWBaseboard.WaterTempAvg = state.dataIPShortCut->rNumericArgs(1);
            if (thisHWBaseboard.WaterTempAvg > MaxWaterTempAvg + 0.001) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        state.dataIPShortCut->cNumericFieldNames(1)));
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxWaterTempAvg));
                thisHWBaseboard.WaterTempAvg = MaxWaterTempAvg;
            } else if (thisHWBaseboard.WaterTempAvg < MinWaterTempAvg - 0.001) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        state.dataIPShortCut->cNumericFieldNames(1)));
                ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinWaterTempAvg));
                thisHWBaseboard.WaterTempAvg = MinWaterTempAvg;
            }

            thisHWBaseboard.WaterMassFlowRateStd = state.dataIPShortCut->rNumericArgs(2);
            if (thisHWBaseboard.WaterMassFlowRateStd < LowWaterMassFlowRate - 0.0001 ||
                thisHWBaseboard.WaterMassFlowRateStd > HighWaterMassFlowRate + 0.0001) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} is an invalid Standard Water mass flow rate.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        state.dataIPShortCut->cNumericFieldNames(2)));
                ShowContinueError(state, format("...reset to a default value=[{:.1R}].", WaterMassFlowDefault));
                thisHWBaseboard.WaterMassFlowRateStd = WaterMassFlowDefault;
            }

            // Determine HW radiant baseboard heating design capacity sizing method
            thisHWBaseboard.HeatingCapMethod = static_cast<int>(HWBaseboardDesignDataObject.HeatingCapMethod);
            if (thisHWBaseboard.HeatingCapMethod == DataSizing::HeatingDesignCapacity) {
                if (!state.dataIPShortCut->lNumericFieldBlanks(iHeatDesignCapacityNumericNum)) {
                    thisHWBaseboard.ScaledHeatingCapacity = state.dataIPShortCut->rNumericArgs(iHeatDesignCapacityNumericNum);
                    if (thisHWBaseboard.ScaledHeatingCapacity < 0.0 && thisHWBaseboard.ScaledHeatingCapacity != DataSizing::AutoSize) {
                        ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboard.Name));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 state.dataIPShortCut->cNumericFieldNames(iHeatDesignCapacityNumericNum),
                                                 state.dataIPShortCut->rNumericArgs(iHeatDesignCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboard.Name));
                    ShowContinueError(state,
                                      format("Input for {} = {}",
                                             state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                             state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ShowContinueError(
                        state, format("Blank field not allowed for {}", state.dataIPShortCut->cNumericFieldNames(iHeatDesignCapacityNumericNum)));
                    ErrorsFound = true;
                }
            } else if (thisHWBaseboard.HeatingCapMethod == DataSizing::CapacityPerFloorArea) {
                thisHWBaseboard.ScaledHeatingCapacity = HWBaseboardDesignDataObject.ScaledHeatingCapacity;

            } else if (thisHWBaseboard.HeatingCapMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                thisHWBaseboard.ScaledHeatingCapacity = HWBaseboardDesignDataObject.ScaledHeatingCapacity;

            } else {
                ShowSevereError(state, format("{} = {}", state.dataIPShortCut->cCurrentModuleObject, thisHWBaseboard.Name));
                ShowContinueError(state,
                                  format("Illegal {} = {}",
                                         state.dataIPShortCut->cAlphaFieldNames(iHeatCAPMAlphaNum),
                                         state.dataIPShortCut->cAlphaArgs(iHeatCAPMAlphaNum)));
                ErrorsFound = true;
            }

            thisHWBaseboard.WaterVolFlowRateMax = state.dataIPShortCut->rNumericArgs(4);
            if (std::abs(thisHWBaseboard.WaterVolFlowRateMax) <= MinWaterFlowRate) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was less than the allowable minimum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        state.dataIPShortCut->cNumericFieldNames(4)));
                ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinWaterFlowRate));
                thisHWBaseboard.WaterVolFlowRateMax = MinWaterFlowRate;
            } else if (thisHWBaseboard.WaterVolFlowRateMax > MaxWaterFlowRate) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        state.dataIPShortCut->cNumericFieldNames(4)));
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxWaterFlowRate));
                thisHWBaseboard.WaterVolFlowRateMax = MaxWaterFlowRate;
            }

            // Remaining fraction is added to the zone as convective heat transfer
            if (HWBaseboardDesignDataObject.FracRadiant > MaxFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", Fraction Radiant was higher than the allowable maximum.",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1)));
                HWBaseboardDesignDataObject.FracRadiant = MaxFraction;
                thisHWBaseboard.FracConvect = 0.0;
            } else {
                thisHWBaseboard.FracConvect = 1.0 - HWBaseboardDesignDataObject.FracRadiant;
            }

            thisHWBaseboard.TotSurfToDistrib = NumNumbers - 4;
            //      IF (thisHWBaseboard%TotSurfToDistrib > MaxDistribSurfaces) THEN
            //        CALL ShowWarningError(state, RoutineName//cCMO_BBRadiator_Water//'="'//TRIM(state.dataIPShortCut->cAlphaArgs(1))// &
            //          '", the number of surface/radiant fraction groups entered was higher than the allowable maximum.')
            //        CALL ShowContinueError(state, '...only the maximum value=['//TRIM(RoundSigDigits(MaxDistribSurfaces))// &
            //           '] will be processed.')
            //        thisHWBaseboard%TotSurfToDistrib = MaxDistribSurfaces
            //      END IF
            if ((thisHWBaseboard.TotSurfToDistrib < MinDistribSurfaces) && (HWBaseboardDesignDataObject.FracRadiant > MinFraction)) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCMO_BBRadiator_Water + "=\"" + state.dataIPShortCut->cAlphaArgs(1) +
                                    "\", the number of surface/radiant fraction groups entered was less than the allowable minimum.");
                ShowContinueError(state, format("...the minimum that must be entered=[{}].", MinDistribSurfaces));
                ErrorsFound = true;
                thisHWBaseboard.TotSurfToDistrib = 0; // error
            }

            thisHWBaseboard.SurfacePtr.allocate(thisHWBaseboard.TotSurfToDistrib);
            thisHWBaseboard.SurfacePtr = 0;
            thisHWBaseboard.FracDistribToSurf.allocate(thisHWBaseboard.TotSurfToDistrib);
            thisHWBaseboard.FracDistribToSurf = 0.0;

            thisHWBaseboard.ZonePtr =
                DataZoneEquipment::GetZoneEquipControlledZoneNum(state, DataZoneEquipment::ZoneEquipType::BaseboardWater, thisHWBaseboard.Name);

            Real64 AllFracsSummed = HWBaseboardDesignDataObject.FracDistribPerson;
            for (SurfNum = 1; SurfNum <= thisHWBaseboard.TotSurfToDistrib; ++SurfNum) {
                thisHWBaseboard.SurfacePtr(SurfNum) =
                    HeatBalanceIntRadExchange::GetRadiantSystemSurface(state,
                                                                       cCMO_BBRadiator_Water,
                                                                       thisHWBaseboard.Name,
                                                                       thisHWBaseboard.ZonePtr,
                                                                       state.dataIPShortCut->cAlphaArgs(SurfNum + 5),
                                                                       ErrorsFound);
                thisHWBaseboard.FracDistribToSurf(SurfNum) = state.dataIPShortCut->rNumericArgs(SurfNum + 4);
                if (thisHWBaseboard.FracDistribToSurf(SurfNum) > MaxFraction) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", {}was greater than the allowable maximum.",
                                            RoutineName,
                                            cCMO_BBRadiator_Water,
                                            state.dataIPShortCut->cAlphaArgs(1),
                                            state.dataIPShortCut->cNumericFieldNames(SurfNum + 4)));
                    ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
                    thisHWBaseboard.TotSurfToDistrib = MaxFraction;
                }
                if (thisHWBaseboard.FracDistribToSurf(SurfNum) < MinFraction) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", {}was less than the allowable minimum.",
                                            RoutineName,
                                            cCMO_BBRadiator_Water,
                                            state.dataIPShortCut->cAlphaArgs(1),
                                            state.dataIPShortCut->cNumericFieldNames(SurfNum + 4)));
                    ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MinFraction));
                    thisHWBaseboard.TotSurfToDistrib = MinFraction;
                }
                if (thisHWBaseboard.SurfacePtr(SurfNum) != 0) {
                    state.dataSurface->surfIntConv(thisHWBaseboard.SurfacePtr(SurfNum)).getsRadiantHeat = true;
                    state.dataSurface->allGetsRadiantHeatSurfaceList.emplace_back(thisHWBaseboard.SurfacePtr(SurfNum));
                }

                AllFracsSummed += thisHWBaseboard.FracDistribToSurf(SurfNum);
            } // Surfaces

            if (AllFracsSummed > (MaxFraction + 0.01)) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", Summed radiant fractions for people + surface groups > 1.0",
                                       RoutineName,
                                       cCMO_BBRadiator_Water,
                                       state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            if ((AllFracsSummed < (MaxFraction - 0.01)) &&
                (HWBaseboardDesignDataObject.FracRadiant >
                 MinFraction)) { // User didn't distribute all of the | radiation warn that some will be lost
                ShowWarningError(state,
                                 format("{}{}=\"{}\", Summed radiant fractions for people + surface groups < 1.0",
                                        RoutineName,
                                        cCMO_BBRadiator_Water,
                                        state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "The rest of the radiant energy delivered by the baseboard heater will be lost");
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}{}Errors found getting input. Program terminates.", RoutineName, cCMO_BBRadiator_Water));
        }

        // Setup Report variables for the Coils
        for (BaseboardNum = 1; BaseboardNum <= NumHWBaseboards; ++BaseboardNum) {
            // CurrentModuleObject='ZoneHVAC:Baseboard:RadiantConvective:Water'
            auto &thisHWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Rate",
                                Constant::Units::W,
                                thisHWBaseboard.TotPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);

            SetupOutputVariable(state,
                                "Baseboard Convective Heating Rate",
                                Constant::Units::W,
                                thisHWBaseboard.ConvPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Radiant Heating Rate",
                                Constant::Units::W,
                                thisHWBaseboard.RadPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Energy",
                                Constant::Units::J,
                                thisHWBaseboard.TotEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHWBaseboard.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Baseboard);

            SetupOutputVariable(state,
                                "Baseboard Convective Heating Energy",
                                Constant::Units::J,
                                thisHWBaseboard.ConvEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Radiant Heating Energy",
                                Constant::Units::J,
                                thisHWBaseboard.RadEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Hot Water Energy",
                                Constant::Units::J,
                                thisHWBaseboard.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHWBaseboard.Name,
                                Constant::eResource::PlantLoopHeatingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Baseboard);
            SetupOutputVariable(state,
                                "Baseboard Hot Water Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisHWBaseboard.WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisHWBaseboard.AirMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Air Inlet Temperature",
                                Constant::Units::C,
                                thisHWBaseboard.AirInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Air Outlet Temperature",
                                Constant::Units::C,
                                thisHWBaseboard.AirOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Water Inlet Temperature",
                                Constant::Units::C,
                                thisHWBaseboard.WaterInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
            SetupOutputVariable(state,
                                "Baseboard Water Outlet Temperature",
                                Constant::Units::C,
                                thisHWBaseboard.WaterOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHWBaseboard.Name);
        }
    }

    void InitHWBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum, bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //                      Rick Strand
        //       DATE WRITTEN   Nov 1997
        //                      Feb 2001
        //       MODIFIED       Aug 2007 Daeho Kang (Add radiant component)
        //                      Sept 2010 Brent Griffith (plant interactions)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the baseboard units, and determines the UA values during simulation.

        // METHODOLOGY EMPLOYED:
        // The initialization subroutines both in high temperature radiant radiator
        // and convective only baseboard radiator are combined and modified. In addition,
        // an UA value calculation by LMTD method is added.
        // The heater is assumed to be crossflow with both fluids unmixed.

        // REFERENCES:
        // 1. Incropera and DeWitt, Fundamentals of Heat and Mass Transfer
        // Chapter 11.3, p. 510, eq. 11.15 and 11.17
        // 2. I=B=R Ratings for Baseboards, Baseboard Radiation, Finned Tube (Commercial) Radiation,
        // and Indirect Fired Water Heaters, January 2007 Edition

        Real64 constexpr Constant(0.0062); // Constant of linear equation for air mass flow rate
        Real64 constexpr Coeff(0.0000275); // Correlation coefficient to capacity
        static constexpr std::string_view RoutineName("BaseboardRadiatorWater:InitHWBaseboard");

        int WaterInletNode;
        Real64 rho; // local fluid density
        Real64 Cp;  // local fluid specific heat

        int NumHWBaseboards = state.dataHWBaseboardRad->NumHWBaseboards;
        auto &HWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);

        // Do the one time initializations
        if (state.dataHWBaseboardRad->MyOneTimeFlag) {

            // Initialize the environment and sizing flags
            state.dataHWBaseboardRad->MyEnvrnFlag.dimension(NumHWBaseboards, true);
            state.dataHWBaseboardRad->MySizeFlag.dimension(NumHWBaseboards, true);
            state.dataHWBaseboardRad->SetLoopIndexFlag.dimension(NumHWBaseboards, true);
            state.dataHWBaseboardRad->MyOneTimeFlag = false;

            for (auto &hWBB : state.dataHWBaseboardRad->HWBaseboard) {
                // Air mass flow rate is obtained from the following linear equation (reset if autosize is used)
                // m_dot = 0.0062 + 2.75e-05*q
                hWBB.ZeroBBSourceSumHATsurf = 0.0;
                hWBB.QBBRadSource = 0.0;
                hWBB.QBBRadSrcAvg = 0.0;
                hWBB.LastQBBRadSrc = 0.0;
                hWBB.LastSysTimeElapsed = 0.0;
                hWBB.LastTimeStepSys = 0.0;
                hWBB.AirMassFlowRateStd = Constant + Coeff * hWBB.RatedCapacity;
            }
        }

        if (state.dataHWBaseboardRad->SetLoopIndexFlag(BaseboardNum)) {
            if (allocated(state.dataPlnt->PlantLoop)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state, HWBaseboard.Name, HWBaseboard.EquipType, HWBaseboard.plantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowFatalError(state, "InitHWBaseboard: Program terminated for previous conditions.");
                }
                state.dataHWBaseboardRad->SetLoopIndexFlag(BaseboardNum) = false;
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataHWBaseboardRad->MySizeFlag(BaseboardNum) &&
            !state.dataHWBaseboardRad->SetLoopIndexFlag(BaseboardNum)) {
            // For each coil, do the sizing once
            SizeHWBaseboard(state, BaseboardNum);
            state.dataHWBaseboardRad->MySizeFlag(BaseboardNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataHWBaseboardRad->MyEnvrnFlag(BaseboardNum)) {
            // Initialize
            WaterInletNode = HWBaseboard.WaterInletNode;

            rho = state.dataPlnt->PlantLoop(HWBaseboard.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

            HWBaseboard.WaterMassFlowRateMax = rho * HWBaseboard.WaterVolFlowRateMax;

            PlantUtilities::InitComponentNodes(state, 0.0, HWBaseboard.WaterMassFlowRateMax, HWBaseboard.WaterInletNode, HWBaseboard.WaterOutletNode);

            state.dataLoopNodes->Node(WaterInletNode).Temp = 60.0;

            Cp = state.dataPlnt->PlantLoop(HWBaseboard.plantLoc.loopNum)
                     .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(WaterInletNode).Temp, RoutineName);

            state.dataLoopNodes->Node(WaterInletNode).Enthalpy = Cp * state.dataLoopNodes->Node(WaterInletNode).Temp;
            state.dataLoopNodes->Node(WaterInletNode).Quality = 0.0;
            state.dataLoopNodes->Node(WaterInletNode).Press = 0.0;
            state.dataLoopNodes->Node(WaterInletNode).HumRat = 0.0;

            HWBaseboard.ZeroBBSourceSumHATsurf = 0.0;
            HWBaseboard.QBBRadSource = 0.0;
            HWBaseboard.QBBRadSrcAvg = 0.0;
            HWBaseboard.LastQBBRadSrc = 0.0;
            HWBaseboard.LastSysTimeElapsed = 0.0;
            HWBaseboard.LastTimeStepSys = 0.0;

            state.dataHWBaseboardRad->MyEnvrnFlag(BaseboardNum) = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHWBaseboardRad->MyEnvrnFlag(BaseboardNum) = true;
        }

        if (state.dataGlobal->BeginTimeStepFlag && FirstHVACIteration) {
            int ZoneNum = HWBaseboard.ZonePtr;
            HWBaseboard.ZeroBBSourceSumHATsurf = state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state);
            HWBaseboard.QBBRadSrcAvg = 0.0;
            HWBaseboard.LastQBBRadSrc = 0.0;
            HWBaseboard.LastSysTimeElapsed = 0.0;
            HWBaseboard.LastTimeStepSys = 0.0;
        }

        // Do the every time step initializations
        WaterInletNode = HWBaseboard.WaterInletNode;
        int ZoneNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
        HWBaseboard.WaterMassFlowRate = state.dataLoopNodes->Node(WaterInletNode).MassFlowRate;
        HWBaseboard.WaterInletTemp = state.dataLoopNodes->Node(WaterInletNode).Temp;
        HWBaseboard.WaterInletEnthalpy = state.dataLoopNodes->Node(WaterInletNode).Enthalpy;
        HWBaseboard.AirInletTemp = state.dataLoopNodes->Node(ZoneNode).Temp;
        HWBaseboard.AirInletHumRat = state.dataLoopNodes->Node(ZoneNode).HumRat;

        HWBaseboard.TotPower = 0.0;
        HWBaseboard.Power = 0.0;
        HWBaseboard.ConvPower = 0.0;
        HWBaseboard.RadPower = 0.0;
        HWBaseboard.TotEnergy = 0.0;
        HWBaseboard.Energy = 0.0;
        HWBaseboard.ConvEnergy = 0.0;
        HWBaseboard.RadEnergy = 0.0;
    }

    void SizeHWBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2009 Daeho Kang (Add UA autosizing by LMTD)
        //                      Aug 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B.Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing hot water baseboard components

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr AirInletTempStd(18.0); // I=B=R rating document
        Real64 constexpr CPAirStd(1005.0);      // Average specific heat of air at between 25C and 40C in J/kg-k
        Real64 constexpr Constant(0.0062);      // Constant of linear equation for air mass flow rate
        Real64 constexpr Coeff(0.0000275);      // Correlation coefficient to capacity
        static constexpr std::string_view RoutineName("SizeHWBaseboard");
        static constexpr std::string_view RoutineNameFull("BaseboardRadiatorWater:SizeHWBaseboard");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 WaterInletTempStd;
        Real64 WaterOutletTempStd;
        Real64 AirOutletTempStd;
        Real64 DeltaT1;
        Real64 DeltaT2;
        Real64 LMTD;
        Real64 AirMassFlowRate;
        Real64 WaterMassFlowRateStd;
        Real64 rho;      // local fluid density
        Real64 Cp;       // local fluid specific heat
        Real64 TempSize; // autosized value of coil input field

        int PltSizHeatNum = 0;
        Real64 DesCoilLoad = 0.0;
        bool ErrorsFound = false;
        Real64 WaterVolFlowRateMaxDes = 0.0;
        Real64 WaterVolFlowRateMaxUser = 0.0;
        Real64 RatedCapacityDes = 0.0;
        state.dataSize->DataScalableCapSizingON = false;

        auto &hWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);
        if (state.dataSize->CurZoneEqNum > 0) {
            auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

            std::string CompType = cCMO_BBRadiator_Water;
            std::string CompName = hWBaseboard.Name;
            state.dataSize->DataHeatSizeRatio = 1.0;
            state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
            state.dataSize->DataZoneNumber = hWBaseboard.ZonePtr;
            int SizingMethod = HVAC::HeatingCapacitySizing;
            int FieldNum = 3; // IDD numeric field number where input field description is found
            std::string SizingString = state.dataHWBaseboardRad->HWBaseboardNumericFields(BaseboardNum).FieldNames(FieldNum) + " [W]";
            int CapSizingMethod = hWBaseboard.HeatingCapMethod;
            zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
            if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                    if (hWBaseboard.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        CheckZoneSizing(state, CompType, CompName);
                        zoneEqSizing.HeatingCapacity = true;
                        zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    }
                    TempSize = hWBaseboard.ScaledHeatingCapacity;

                } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                    zoneEqSizing.HeatingCapacity = true;
                    zoneEqSizing.DesHeatingLoad =
                        hWBaseboard.ScaledHeatingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                    TempSize = zoneEqSizing.DesHeatingLoad;
                    state.dataSize->DataScalableCapSizingON = true;
                } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                    CheckZoneSizing(state, CompType, CompName);
                    zoneEqSizing.HeatingCapacity = true;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = hWBaseboard.ScaledHeatingCapacity;
                    zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    TempSize = DataSizing::AutoSize;
                    state.dataSize->DataScalableCapSizingON = true;
                } else {
                    TempSize = hWBaseboard.ScaledHeatingCapacity;
                }
                bool PrintFlag = false;
                bool errorsFound = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                TempSize = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                if (hWBaseboard.ScaledHeatingCapacity == DataSizing::AutoSize) {
                    hWBaseboard.RatedCapacity = DataSizing::AutoSize;
                } else {
                    hWBaseboard.RatedCapacity = TempSize;
                }
                RatedCapacityDes = TempSize;
                state.dataSize->DataScalableCapSizingON = false;
            }
        }

        // find the appropriate heating Plant Sizing object
        PltSizHeatNum = state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum).PlantSizNum;

        if (PltSizHeatNum > 0) {
            if (state.dataSize->CurZoneEqNum > 0) {
                bool FlowAutoSize = false;

                if (hWBaseboard.WaterVolFlowRateMax == DataSizing::AutoSize) {
                    FlowAutoSize = true;
                }
                if (!FlowAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                    if (hWBaseboard.WaterVolFlowRateMax > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     cCMO_BBRadiator_Water,
                                                     hWBaseboard.Name,
                                                     "User-Specified Maximum Water Flow Rate [m3/s]",
                                                     hWBaseboard.WaterVolFlowRateMax);
                    }
                } else {
                    CheckZoneSizing(state, cCMO_BBRadiator_Water, hWBaseboard.Name);
                    DesCoilLoad = RatedCapacityDes;
                    if (DesCoilLoad >= HVAC::SmallLoad) {
                        Cp = state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum)
                                 .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                        rho =
                            state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        WaterVolFlowRateMaxDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                    } else {
                        WaterVolFlowRateMaxDes = 0.0;
                    }

                    if (FlowAutoSize) {
                        hWBaseboard.WaterVolFlowRateMax = WaterVolFlowRateMaxDes;
                        BaseSizer::reportSizerOutput(
                            state, cCMO_BBRadiator_Water, hWBaseboard.Name, "Design Size Maximum Water Flow Rate [m3/s]", WaterVolFlowRateMaxDes);
                    } else { // Hard-sized with sizing data
                        if (hWBaseboard.WaterVolFlowRateMax > 0.0 && WaterVolFlowRateMaxDes > 0.0) {
                            WaterVolFlowRateMaxUser = hWBaseboard.WaterVolFlowRateMax;
                            BaseSizer::reportSizerOutput(state,
                                                         cCMO_BBRadiator_Water,
                                                         hWBaseboard.Name,
                                                         "Design Size Maximum Water Flow Rate [m3/s]",
                                                         WaterVolFlowRateMaxDes,
                                                         "User-Specified Maximum Water Flow Rate [m3/s]",
                                                         WaterVolFlowRateMaxUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(WaterVolFlowRateMaxDes - WaterVolFlowRateMaxUser) / WaterVolFlowRateMaxUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeHWBaseboard: Potential issue with equipment sizing for "
                                                       "ZoneHVAC:Baseboard:RadiantConvective:Water=\"{}\".",
                                                       hWBaseboard.Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Water Flow Rate of {:.5R} [m3/s]", WaterVolFlowRateMaxUser));
                                    ShowContinueError(
                                        state, format("differs from Design Size Maximum Water Flow Rate of {:.5R} [m3/s]", WaterVolFlowRateMaxDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
                if (hWBaseboard.WaterTempAvg > 0.0 && hWBaseboard.WaterMassFlowRateStd > 0.0 && hWBaseboard.RatedCapacity > 0.0) {
                    DesCoilLoad = hWBaseboard.RatedCapacity;
                    WaterMassFlowRateStd = hWBaseboard.WaterMassFlowRateStd;
                } else if (hWBaseboard.RatedCapacity == DataSizing::AutoSize || hWBaseboard.RatedCapacity == 0.0) {
                    DesCoilLoad = RatedCapacityDes;
                    rho =
                        state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineNameFull);
                    WaterMassFlowRateStd = hWBaseboard.WaterVolFlowRateMax * rho;
                }
                if (DesCoilLoad >= HVAC::SmallLoad) {
                    // Calculate UA value
                    // Air mass flow rate is obtained from the following linear equation
                    // m_dot = 0.0062 + 2.75e-05*q
                    AirMassFlowRate = Constant + Coeff * DesCoilLoad;
                    Cp =
                        state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum).glycol->getSpecificHeat(state, hWBaseboard.WaterTempAvg, RoutineName);
                    WaterInletTempStd = (DesCoilLoad / (2.0 * WaterMassFlowRateStd * Cp)) + hWBaseboard.WaterTempAvg;
                    WaterOutletTempStd = std::abs((2.0 * hWBaseboard.WaterTempAvg) - WaterInletTempStd);
                    AirOutletTempStd = (DesCoilLoad / (AirMassFlowRate * CPAirStd)) + AirInletTempStd;
                    hWBaseboard.AirMassFlowRateStd = AirMassFlowRate;
                    // Check Ta,out < Tw,in
                    if (AirOutletTempStd >= WaterInletTempStd) {
                        ShowSevereError(state, format("SizeHWBaseboard: ZoneHVAC:Baseboard:RadiantConvective:Water=\"{}\".", hWBaseboard.Name));
                        ShowContinueError(state, "...Air Outlet temperature must be below the Water Inlet temperature");
                        ShowContinueError(
                            state,
                            format("...Air Outlet Temperature=[{:.2R}], Water Inlet Temperature=[{:.2R}].", AirOutletTempStd, WaterInletTempStd));
                        AirOutletTempStd = WaterInletTempStd - 0.01;
                        ShowContinueError(state, format("...Air Outlet Temperature set to [{:.2R}].", AirOutletTempStd));
                    }
                    // Check Tw,out < Ta,in
                    if (AirInletTempStd >= WaterOutletTempStd) {
                        ShowSevereError(state, format("SizeHWBaseboard: ZoneHVAC:Baseboard:RadiantConvective:Water=\"{}\".", hWBaseboard.Name));
                        ShowContinueError(state, "...Water Outlet temperature must be below the Air Inlet temperature");
                        ShowContinueError(
                            state,
                            format("...Air Inlet Temperature=[{:.2R}], Water Outlet Temperature=[{:.2R}].", AirInletTempStd, WaterOutletTempStd));
                        WaterOutletTempStd = AirInletTempStd + 0.01;
                        ShowContinueError(state, format("...Water Outlet Temperature set to [{:.2R}].", WaterOutletTempStd));
                    }
                    // LMTD calculation
                    DeltaT1 = WaterInletTempStd - AirOutletTempStd;
                    DeltaT2 = WaterOutletTempStd - AirInletTempStd;
                    LMTD = (DeltaT1 - DeltaT2) / (std::log(DeltaT1 / DeltaT2));
                    hWBaseboard.UA = DesCoilLoad / LMTD;
                } else {
                    hWBaseboard.UA = 0.0;
                }
                // Report an UA value
                BaseSizer::reportSizerOutput(state, cCMO_BBRadiator_Water, hWBaseboard.Name, "U-Factor times Area [W/C]", hWBaseboard.UA);
            }
        } else {
            // if there is no heating Sizing:Plant object and autosizing was requested, issue an error message
            if (hWBaseboard.WaterVolFlowRateMax == DataSizing::AutoSize || hWBaseboard.RatedCapacity == DataSizing::AutoSize ||
                hWBaseboard.RatedCapacity == 0.0) {
                ShowSevereError(state, "Autosizing of hot water baseboard requires a heating loop Sizing:Plant object");
                ShowContinueError(state, format("Occurs in Hot Water Baseboard Heater={}", hWBaseboard.Name));
                ErrorsFound = true;
            }
            // calculate UA from rated capacities
            hWBaseboard.RatedCapacity = RatedCapacityDes;
            DesCoilLoad = RatedCapacityDes;

            if (DesCoilLoad >= HVAC::SmallLoad) {
                WaterMassFlowRateStd = hWBaseboard.WaterMassFlowRateStd;
                // m_dot = 0.0062 + 2.75e-05*q
                AirMassFlowRate = Constant + Coeff * DesCoilLoad;
                Cp = state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum).glycol->getSpecificHeat(state, hWBaseboard.WaterTempAvg, RoutineName);
                WaterInletTempStd = (DesCoilLoad / (2.0 * WaterMassFlowRateStd * Cp)) + hWBaseboard.WaterTempAvg;
                WaterOutletTempStd = std::abs((2.0 * hWBaseboard.WaterTempAvg) - WaterInletTempStd);
                AirOutletTempStd = (DesCoilLoad / (AirMassFlowRate * CPAirStd)) + AirInletTempStd;
                hWBaseboard.AirMassFlowRateStd = AirMassFlowRate;

                // Check Ta,out < Tw,in
                if (AirOutletTempStd >= WaterInletTempStd) {
                    ShowSevereError(state, format("SizeHWBaseboard: ZoneHVAC:Baseboard:RadiantConvective:Water=\"{}\".", hWBaseboard.Name));
                    ShowContinueError(state, "...Air Outlet temperature must be below the Water Inlet temperature");
                    ShowContinueError(
                        state, format("...Air Outlet Temperature=[{:.2R}], Water Inlet Temperature=[{:.2R}].", AirOutletTempStd, WaterInletTempStd));
                    AirOutletTempStd = WaterInletTempStd - 0.01;
                    ShowContinueError(state, format("...Air Outlet Temperature set to [{:.2R}].", AirOutletTempStd));
                }
                // Check Tw,out < Ta,in
                if (AirInletTempStd >= WaterOutletTempStd) {
                    ShowSevereError(state, format("SizeHWBaseboard: ZoneHVAC:Baseboard:RadiantConvective:Water=\"{}\".", hWBaseboard.Name));
                    ShowContinueError(state, "...Water Outlet temperature must be below the Air Inlet temperature");
                    ShowContinueError(
                        state, format("...Air Inlet Temperature=[{:.2R}], Water Outlet Temperature=[{:.2R}].", AirInletTempStd, WaterOutletTempStd));
                    WaterOutletTempStd = AirInletTempStd + 0.01;
                    ShowContinueError(state, format("...Water Outlet Temperature set to [{:.2R}].", WaterOutletTempStd));
                }
                // LMTD calculation
                DeltaT1 = WaterInletTempStd - AirOutletTempStd;
                DeltaT2 = WaterOutletTempStd - AirInletTempStd;
                LMTD = (DeltaT1 - DeltaT2) / (std::log(DeltaT1 / DeltaT2));
                hWBaseboard.UA = DesCoilLoad / LMTD;
            } else {
                hWBaseboard.UA = 0.0;
            }
            // Report an UA value
            BaseSizer::reportSizerOutput(state, cCMO_BBRadiator_Water, hWBaseboard.Name, "U-Factor times Area [W/C]", hWBaseboard.UA);
        }
        // save the design water flow rate for use by the water loop sizing algorithms
        PlantUtilities::RegisterPlantCompDesignFlow(state, hWBaseboard.WaterInletNode, hWBaseboard.WaterVolFlowRateMax);

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void CalcHWBaseboard(EnergyPlusData &state, int &BaseboardNum, Real64 &LoadMet)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997
        //       MODIFIED       May 2000 Fred Buhl
        //                      Aug 2007 Daeho Kang (Add the calculation of radiant heat source)
        //                      Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates both the convective and radiant heat transfer rate
        // in a hot water baseboard heater.  The heater is assumed to be crossflow with
        // both fluids unmixed.  The air flow is buoyancy driven and a constant airflow
        // and a constant airflow velocity of 0.5m/s is assumed.

        // REFERENCES:
        // Incropera and DeWitt, Fundamentals of Heat and Mass Transfer
        // Chapter 11.4, p. 523, eq. 11.33

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MinFrac(0.0005); // Minimum fraction that delivers radiant heats to surfaces
        static constexpr std::string_view RoutineName("CalcHWBaseboard");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 RadHeat;
        Real64 BBHeat;
        Real64 AirOutletTemp;
        Real64 WaterOutletTemp;
        Real64 AirMassFlowRate;
        Real64 CapacitanceAir;
        Real64 CapacitanceWater;
        Real64 CapacitanceMax;
        Real64 CapacitanceMin;
        Real64 CapacityRatio;
        Real64 NTU;
        Real64 Effectiveness;
        Real64 AA;
        Real64 BB;
        Real64 CC;
        Real64 Cp;

        auto &hWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);

        int const ZoneNum = hWBaseboard.ZonePtr;
        Real64 QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        Real64 AirInletTemp = hWBaseboard.AirInletTemp;
        Real64 WaterInletTemp = hWBaseboard.WaterInletTemp;
        Real64 WaterMassFlowRate = state.dataLoopNodes->Node(hWBaseboard.WaterInletNode).MassFlowRate;

        if (QZnReq > HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) && (hWBaseboard.availSched->getCurrentVal() > 0) &&
            (WaterMassFlowRate > 0.0)) {

            HWBaseboardDesignData const &HWBaseboardDesignDataObject{
                state.dataHWBaseboardRad->HWBaseboardDesignObject(hWBaseboard.DesignObjectPtr)}; // Contains the data for the design object

            // Calculate air mass flow rate
            AirMassFlowRate = hWBaseboard.AirMassFlowRateStd * (WaterMassFlowRate / hWBaseboard.WaterMassFlowRateMax);
            CapacitanceAir = Psychrometrics::PsyCpAirFnW(hWBaseboard.AirInletHumRat) * AirMassFlowRate;
            Cp = state.dataPlnt->PlantLoop(hWBaseboard.plantLoc.loopNum).glycol->getSpecificHeat(state, WaterInletTemp, RoutineName);

            CapacitanceWater = Cp * WaterMassFlowRate;
            CapacitanceMax = max(CapacitanceAir, CapacitanceWater);
            CapacitanceMin = min(CapacitanceAir, CapacitanceWater);
            CapacityRatio = CapacitanceMin / CapacitanceMax;
            NTU = hWBaseboard.UA / CapacitanceMin;

            // The effectiveness is given by the following formula:
            // Effectiveness = 1. - EXP((1./CapacityRatio)*(NTU)**0.22*(EXP(-CapacityRatio*(NTU)**0.78)-1.))
            // To prevent possible underflows (numbers smaller than the computer can handle) we must break
            // the calculation up into steps and check the size of the exponential arguments.
            AA = -CapacityRatio * std::pow(NTU, 0.78);
            if (AA < -20.0) {
                BB = 0.0;
            } else {
                BB = std::exp(AA);
            }
            CC = (1.0 / CapacityRatio) * std::pow(NTU, 0.22) * (BB - 1.0);
            if (CC < -20.0) {
                Effectiveness = 1.0;
            } else {
                Effectiveness = 1.0 - std::exp(CC);
            }

            AirOutletTemp = AirInletTemp + Effectiveness * CapacitanceMin * (WaterInletTemp - AirInletTemp) / CapacitanceAir;
            WaterOutletTemp = WaterInletTemp - CapacitanceAir * (AirOutletTemp - AirInletTemp) / CapacitanceWater;
            BBHeat = CapacitanceWater * (WaterInletTemp - WaterOutletTemp);
            RadHeat = BBHeat * HWBaseboardDesignDataObject.FracRadiant;
            hWBaseboard.QBBRadSource = RadHeat;

            if (HWBaseboardDesignDataObject.FracRadiant <= MinFrac) {
                LoadMet = BBHeat;
            } else {

                // Now, distribute the radiant energy of all systems to the appropriate surfaces, to people, and the air
                DistributeBBRadGains(state);
                // Now "simulate" the system by recalculating the heat balances
                HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);

                HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

                // Here an assumption is made regarding radiant heat transfer to people.
                // While the radiant heat transfer to people array will be used by the thermal comfort
                // routines, the energy transfer to people would get lost from the perspective
                // of the heat balance.  So, to avoid this net loss of energy which clearly
                // gets added to the zones, we must account for it somehow.  This assumption
                // that all energy radiated to people is converted to convective energy is
                // not very precise, but at least it conserves energy. The system impact to heat balance
                // should include this.

                LoadMet = (state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - hWBaseboard.ZeroBBSourceSumHATsurf) +
                          (BBHeat * hWBaseboard.FracConvect) + (RadHeat * HWBaseboardDesignDataObject.FracDistribPerson);
            }
            hWBaseboard.WaterOutletEnthalpy = hWBaseboard.WaterInletEnthalpy - BBHeat / WaterMassFlowRate;
        } else {
            CapacitanceWater = 0.0;
            CapacitanceMax = 0.0;
            CapacitanceMin = 0.0;
            NTU = 0.0;
            Effectiveness = 0.0;
            AirOutletTemp = AirInletTemp;
            WaterOutletTemp = WaterInletTemp;
            BBHeat = 0.0;
            LoadMet = 0.0;
            RadHeat = 0.0;
            WaterMassFlowRate = 0.0;
            AirMassFlowRate = 0.0;
            hWBaseboard.QBBRadSource = 0.0;
            hWBaseboard.WaterOutletEnthalpy = hWBaseboard.WaterInletEnthalpy;
            PlantUtilities::SetActuatedBranchFlowRate(state, WaterMassFlowRate, hWBaseboard.WaterInletNode, hWBaseboard.plantLoc, false);
        }

        hWBaseboard.WaterOutletTemp = WaterOutletTemp;
        hWBaseboard.AirOutletTemp = AirOutletTemp;
        hWBaseboard.WaterMassFlowRate = WaterMassFlowRate;
        hWBaseboard.AirMassFlowRate = AirMassFlowRate;
        hWBaseboard.TotPower = LoadMet;
        hWBaseboard.Power = BBHeat;
        hWBaseboard.ConvPower = BBHeat - RadHeat;
        hWBaseboard.RadPower = RadHeat;
    }

    void UpdateHWBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //                      Rick Strand
        //       DATE WRITTEN   Nov 1997
        //                      February 2001
        //       MODIFIED       Aug 2007 Daeho Kang (Add the update of radiant source)

        // METHODOLOGY EMPLOYED:
        // The update subroutines both in high temperature radiant radiator
        // and convective only baseboard radiator are combined and modified.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int WaterInletNode;
        int WaterOutletNode;
        auto &thisHWBB = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);

        if (state.dataGlobal->BeginEnvrnFlag && state.dataHWBaseboardRad->MyEnvrnFlag2) {
            state.dataHWBaseboardRad->Iter = 0;
            state.dataHWBaseboardRad->MyEnvrnFlag2 = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHWBaseboardRad->MyEnvrnFlag2 = true;
        }

        // First, update the running average if necessary...
        if (thisHWBB.LastSysTimeElapsed == state.dataHVACGlobal->SysTimeElapsed) {
            thisHWBB.QBBRadSrcAvg -= thisHWBB.LastQBBRadSrc * thisHWBB.LastTimeStepSys / state.dataGlobal->TimeStepZone;
        }
        // Update the running average and the "last" values with the current values of the appropriate variables
        thisHWBB.QBBRadSrcAvg += thisHWBB.QBBRadSource * state.dataHVACGlobal->TimeStepSys / state.dataGlobal->TimeStepZone;

        thisHWBB.LastQBBRadSrc = thisHWBB.QBBRadSource;
        thisHWBB.LastSysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        thisHWBB.LastTimeStepSys = state.dataHVACGlobal->TimeStepSys;

        WaterInletNode = thisHWBB.WaterInletNode;
        WaterOutletNode = thisHWBB.WaterOutletNode;

        // Set the outlet air nodes of the Baseboard
        // Set the outlet water nodes for the Coil
        PlantUtilities::SafeCopyPlantNode(state, WaterInletNode, WaterOutletNode);
        state.dataLoopNodes->Node(WaterOutletNode).Temp = thisHWBB.WaterOutletTemp;
        state.dataLoopNodes->Node(WaterOutletNode).Enthalpy = thisHWBB.WaterOutletEnthalpy;
    }

    void UpdateBBRadSourceValAvg(EnergyPlusData &state, bool &HWBaseboardSysOn) // .TRUE. if the radiant system has run this zone time step
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001
        //       MODIFIED       Aug 2007 Daeho Kang (Modification only for baseboard)

        // PURPOSE OF THIS SUBROUTINE:
        // To transfer the average value of the heat source over the entire
        // zone time step back to the heat balance routines so that the heat
        // balance algorithms can simulate one last time with the average source
        // to maintain some reasonable amount of continuity and energy balance
        // in the temperature and flux histories.

        // METHODOLOGY EMPLOYED:
        // All of the record keeping for the average term is done in the Update
        // routine so the only other thing that this subroutine does is check to
        // see if the system was even on.  If any average term is non-zero, then
        // one or more of the radiant systems was running.

        HWBaseboardSysOn = false;

        // If there are no baseboards in this input file, just RETURN
        if (state.dataHWBaseboardRad->NumHWBaseboards == 0) {
            return;
        }

        // If there are baseboards, then we have to check to see if this was running at all...
        for (auto &thisHWBaseboard : state.dataHWBaseboardRad->HWBaseboard) {
            thisHWBaseboard.QBBRadSource = thisHWBaseboard.QBBRadSrcAvg;
            if (thisHWBaseboard.QBBRadSrcAvg != 0.0) {
                HWBaseboardSysOn = true;
            }
        }

        DistributeBBRadGains(state); // QBBRadSource has been modified so we need to redistribute gains
    }

    void DistributeBBRadGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2001
        //       MODIFIED       Aug. 2007 Daeho Kang (Modification only for baseboard)
        //                      April 2010 Brent Griffith, max limit to protect surface temperature calcs

        // PURPOSE OF THIS SUBROUTINE:
        // To distribute the gains from the hot water baseboard heater
        // as specified in the user input file.  This includes distribution
        // of long wavelength radiant gains to surfaces and "people."

        // METHODOLOGY EMPLOYED:
        // We must cycle through all of the radiant systems because each
        // surface could feel the effect of more than one radiant system.
        // Note that the energy radiated to people is assumed to affect them
        // but them it is assumed to be convected to the air.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr SmallestArea(0.001); // Smallest area in meters squared (to avoid a divide by zero)

        Real64 ThisSurfIntensity; // temporary for W/m2 term for rad on a surface

        // Initialize arrays
        for (auto &thisHWBB : state.dataHWBaseboardRad->HWBaseboard) {
            for (int radSurfNum = 1; radSurfNum <= thisHWBB.TotSurfToDistrib; ++radSurfNum) {
                int surfNum = thisHWBB.SurfacePtr(radSurfNum);
                state.dataHeatBalFanSys->surfQRadFromHVAC(surfNum).HWBaseboard = 0.0;
            }
        }
        state.dataHeatBalFanSys->ZoneQHWBaseboardToPerson = 0.0;

        for (auto &thisHWBB : state.dataHWBaseboardRad->HWBaseboard) {
            HWBaseboardDesignData const &HWBaseboardDesignDataObject =
                state.dataHWBaseboardRad->HWBaseboardDesignObject(thisHWBB.DesignObjectPtr); // Contains the data for the design object
            int ZoneNum = thisHWBB.ZonePtr;
            if (ZoneNum <= 0) {
                continue;
            }
            state.dataHeatBalFanSys->ZoneQHWBaseboardToPerson(ZoneNum) += thisHWBB.QBBRadSource * HWBaseboardDesignDataObject.FracDistribPerson;

            for (int RadSurfNum = 1; RadSurfNum <= thisHWBB.TotSurfToDistrib; ++RadSurfNum) {
                int SurfNum = thisHWBB.SurfacePtr(RadSurfNum);
                if (state.dataSurface->Surface(SurfNum).Area > SmallestArea) {
                    ThisSurfIntensity = (thisHWBB.QBBRadSource * thisHWBB.FracDistribToSurf(RadSurfNum) / state.dataSurface->Surface(SurfNum).Area);
                    state.dataHeatBalFanSys->surfQRadFromHVAC(SurfNum).HWBaseboard += ThisSurfIntensity;
                    // CR 8074, trap for excessive intensity (throws off surface balance )
                    if (ThisSurfIntensity > DataHeatBalFanSys::MaxRadHeatFlux) {
                        ShowSevereError(state, "DistributeBBRadGains:  excessive thermal radiation heat flux intensity detected");
                        ShowContinueError(state, format("Surface = {}", state.dataSurface->Surface(SurfNum).Name));
                        ShowContinueError(state, format("Surface area = {:.3R} [m2]", state.dataSurface->Surface(SurfNum).Area));
                        ShowContinueError(state, format("Occurs in {} = {}", cCMO_BBRadiator_Water, thisHWBB.Name));
                        ShowContinueError(state, format("Radiation intensity = {:.2R} [W/m2]", ThisSurfIntensity));
                        ShowContinueError(state, format("Assign a larger surface area or more surfaces in {}", cCMO_BBRadiator_Water));
                        ShowFatalError(state, "DistributeBBRadGains:  excessive thermal radiation heat flux intensity detected");
                    }
                } else {
                    ShowSevereError(state, "DistributeBBRadGains:  surface not large enough to receive thermal radiation heat flux");
                    ShowContinueError(state, format("Surface = {}", state.dataSurface->Surface(SurfNum).Name));
                    ShowContinueError(state, format("Surface area = {:.3R} [m2]", state.dataSurface->Surface(SurfNum).Area));
                    ShowContinueError(state, format("Occurs in {} = {}", cCMO_BBRadiator_Water, thisHWBB.Name));
                    ShowContinueError(state, format("Assign a larger surface area or more surfaces in {}", cCMO_BBRadiator_Water));
                    ShowFatalError(state, "DistributeBBRadGains:  surface not large enough to receive thermal radiation heat flux");
                }
            }
        }
    }

    void ReportHWBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Aug 2007

        auto &thisHWBB = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);
        Real64 const timeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        thisHWBB.TotEnergy = thisHWBB.TotPower * timeStepSysSec;
        thisHWBB.Energy = thisHWBB.Power * timeStepSysSec;
        thisHWBB.ConvEnergy = thisHWBB.ConvPower * timeStepSysSec;
        thisHWBB.RadEnergy = thisHWBB.RadPower * timeStepSysSec;
    }

    void UpdateHWBaseboardPlantConnection(EnergyPlusData &state,
                                          int const BaseboardTypeNum,                                  // type index
                                          std::string const &BaseboardName,                            // component name
                                          [[maybe_unused]] int const EquipFlowCtrl,                    // Flow control mode for the equipment
                                          [[maybe_unused]] int const LoopNum,                          // Plant loop index for where called from
                                          [[maybe_unused]] const DataPlant::LoopSideLocation LoopSide, // Plant loop side index for where called from
                                          int &CompIndex,                                              // Chiller number pointer
                                          [[maybe_unused]] bool const FirstHVACIteration,
                                          bool &InitLoopEquip // If not zero, calculate the max load for operating conditions
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Sept. 2010

        // PURPOSE OF THIS SUBROUTINE:
        // update sim routine called from plant

        // METHODOLOGY EMPLOYED:
        // check input, provide comp index, call utility routines

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int const NumHWBaseboards = state.dataHWBaseboardRad->NumHWBaseboards;
        int BaseboardNum;

        // Find the correct baseboard
        if (CompIndex == 0) {
            BaseboardNum = Util::FindItemInList(BaseboardName, state.dataHWBaseboardRad->HWBaseboard, &HWBaseboardParams::Name);
            if (BaseboardNum == 0) {
                ShowFatalError(state, format("UpdateHWBaseboardPlantConnection: Specified baseboard not valid ={}", BaseboardName));
            }
            CompIndex = BaseboardNum;
        } else {
            BaseboardNum = CompIndex;
            if (BaseboardNum > NumHWBaseboards || BaseboardNum < 1) {
                ShowFatalError(
                    state,
                    format("UpdateHWBaseboardPlantConnection:  Invalid CompIndex passed={}, Number of baseboards={}, Entered baseboard name={}",
                           BaseboardNum,
                           NumHWBaseboards,
                           BaseboardName));
            }
            if (state.dataGlobal->KickOffSimulation) {
                if (BaseboardName != state.dataHWBaseboardRad->HWBaseboard(BaseboardNum).Name) {
                    ShowFatalError(state,
                                   format("UpdateHWBaseboardPlantConnection: Invalid CompIndex passed={}, baseboard name={}, stored baseboard Name "
                                          "for that index={}",
                                          BaseboardNum,
                                          BaseboardName,
                                          state.dataHWBaseboardRad->HWBaseboard(BaseboardNum).Name));
                }
                if (BaseboardTypeNum != static_cast<int>(DataPlant::PlantEquipmentType::Baseboard_Rad_Conv_Water)) {
                    ShowFatalError(state,
                                   format("UpdateHWBaseboardPlantConnection: Invalid CompIndex passed={}, baseboard name={}, stored baseboard Name "
                                          "for that index={}",
                                          BaseboardNum,
                                          BaseboardName,
                                          DataPlant::PlantEquipTypeNames[BaseboardTypeNum]));
                }
            }
        }

        if (InitLoopEquip) {
            return;
        }

        auto &thisHWBaseboard = state.dataHWBaseboardRad->HWBaseboard(BaseboardNum);
        PlantUtilities::PullCompInterconnectTrigger(state,
                                                    thisHWBaseboard.plantLoc,
                                                    thisHWBaseboard.BBLoadReSimIndex,
                                                    thisHWBaseboard.plantLoc,
                                                    DataPlant::CriteriaType::HeatTransferRate,
                                                    thisHWBaseboard.Power);

        PlantUtilities::PullCompInterconnectTrigger(state,
                                                    thisHWBaseboard.plantLoc,
                                                    thisHWBaseboard.BBMassFlowReSimIndex,
                                                    thisHWBaseboard.plantLoc,
                                                    DataPlant::CriteriaType::MassFlowRate,
                                                    thisHWBaseboard.WaterMassFlowRate);

        PlantUtilities::PullCompInterconnectTrigger(state,
                                                    thisHWBaseboard.plantLoc,
                                                    thisHWBaseboard.BBInletTempFlowReSimIndex,
                                                    thisHWBaseboard.plantLoc,
                                                    DataPlant::CriteriaType::Temperature,
                                                    thisHWBaseboard.WaterOutletTemp);
    }

    //*****************************************************************************************

} // namespace HWBaseboardRadiator

} // namespace EnergyPlus
