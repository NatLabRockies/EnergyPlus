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
// #include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BaseboardRadiator.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {
namespace BaseboardRadiator {

    // Module containing the routines dealing with the BASEBOARD HEATER
    // component(s).

    // MODULE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   Jan 1998
    //       MODIFIED       Fred Buhl, October 1999
    //       RE-ENGINEERED  na

    // Using/Aliasing
    using HVAC::SmallLoad;

    // Use statements for access to subroutines in other modules
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;

    static std::string const cCMO_BBRadiator_Water("ZoneHVAC:Baseboard:Convective:Water");

    void SimBaseboard(EnergyPlusData &state,
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

        if (state.dataBaseboardRadiator->getInputFlag) {
            GetBaseboardInput(state);
            state.dataBaseboardRadiator->getInputFlag = false;
        }

        // Find the correct Baseboard Equipment
        if (CompIndex == 0) {
            int BaseboardNum = Util::FindItemInList(EquipName, state.dataBaseboardRadiator->baseboards, &BaseboardParams::EquipID);
            if (BaseboardNum == 0) {
                ShowFatalError(state, format("SimBaseboard: Unit not found={}", EquipName));
            }
            CompIndex = BaseboardNum;
        }
        assert(CompIndex <= (int)state.dataBaseboardRadiator->baseboards.size());
        auto &thisBaseboard = state.dataBaseboardRadiator->baseboards(CompIndex);
        if (thisBaseboard.CheckEquipName) {
            if (EquipName != thisBaseboard.EquipID) {
                ShowFatalError(state,
                               format("SimBaseboard: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      CompIndex,
                                      EquipName,
                                      thisBaseboard.EquipID));
            }
            thisBaseboard.CheckEquipName = false;
        }

        thisBaseboard.InitBaseboard(state, CompIndex);

        Real64 QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;
        Real64 MaxWaterFlow = 0.0;
        Real64 MinWaterFlow = 0.0;
        Real64 DummyMdot = 0.0;

        if ((QZnReq < SmallLoad) || (thisBaseboard.WaterInletTemp <= thisBaseboard.AirInletTemp)) {
            //  IF (Baseboard(BaseboardNum)%WaterInletTemp <= Baseboard(BaseboardNum)%AirInletTemp) THEN
            // The baseboard cannot provide cooling.  Thus, if the zone required load is negative or the water inlet
            // temperature is lower than the zone air temperature, then we need to shut down the baseboard unit

            thisBaseboard.WaterOutletTemp = thisBaseboard.WaterInletTemp;
            thisBaseboard.AirOutletTemp = thisBaseboard.AirInletTemp;
            thisBaseboard.Power = 0.0;
            thisBaseboard.WaterMassFlowRate = 0.0;
            // init hot water flow rate to zero
            DummyMdot = 0.0;
            PlantUtilities::SetActuatedBranchFlowRate(state, DummyMdot, thisBaseboard.WaterInletNode, thisBaseboard.plantLoc, false);

        } else {
            // init hot water flow rate to zero
            DummyMdot = 0.0;
            PlantUtilities::SetActuatedBranchFlowRate(state, DummyMdot, thisBaseboard.WaterInletNode, thisBaseboard.plantLoc, true);

            // On the first HVAC iteration the system values are given to the controller, but after that
            // the demand limits are in place and there needs to be feedback to the Zone Equipment
            if (FirstHVACIteration) {
                MaxWaterFlow = thisBaseboard.WaterMassFlowRateMax;
                MinWaterFlow = 0.0;
            } else {
                MaxWaterFlow = state.dataLoopNodes->Node(thisBaseboard.WaterInletNode).MassFlowRateMaxAvail;
                MinWaterFlow = state.dataLoopNodes->Node(thisBaseboard.WaterInletNode).MassFlowRateMinAvail;
            }

            ControlCompOutput(state,
                              thisBaseboard.EquipID,
                              cCMO_BBRadiator_Water,
                              CompIndex,
                              FirstHVACIteration,
                              QZnReq,
                              thisBaseboard.WaterInletNode,
                              MaxWaterFlow,
                              MinWaterFlow,
                              thisBaseboard.Offset,
                              thisBaseboard.ControlCompTypeNum,
                              thisBaseboard.CompErrIndex,
                              _,
                              _,
                              _,
                              _,
                              _,
                              thisBaseboard.plantLoc);

            PowerMet = thisBaseboard.Power;
        }

        UpdateBaseboard(state, CompIndex);
        thisBaseboard.Energy = thisBaseboard.Power * state.dataHVACGlobal->TimeStepSysSec;
    }

    void GetBaseboardInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the Baseboard units.

        // METHODOLOGY EMPLOYED:
        // Standard input processor calls.

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using NodeInputManager::GetOnlySingleNode;
        using namespace DataLoopNode;
        using GlobalNames::VerifyUniqueBaseboardName;
        using namespace DataSizing;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName = "GetBaseboardInput: "; // include trailing blank space
        static constexpr std::string_view routineName = "GetBaseboardInput";
        int constexpr iHeatCAPMAlphaNum = 5;                   // get input index to water baseboard Radiator system heating capacity sizing method
        int constexpr iHeatDesignCapacityNumericNum = 1;       // get input index to water baseboard Radiator system electric heating capacity
        int constexpr iHeatCapacityPerFloorAreaNumericNum = 2; // index to baseboard Radiator system electric heating capacity per floor area sizing
        int constexpr iHeatFracOfAutosizedCapacityNumericNum = 3; //  index to baseboard heating capacity fraction of autosized heating capacity

        auto &s_ipsc = state.dataIPShortCut;

        s_ipsc->cCurrentModuleObject = cCMO_BBRadiator_Water;

        int NumConvHWBaseboards = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        // Calculate total number of baseboard units

        state.dataBaseboardRadiator->baseboards.allocate(NumConvHWBaseboards);

        if (NumConvHWBaseboards > 0) { // Get the data for cooling schemes
            bool ErrorsFound(false);   // If errors detected in input
            for (int ConvHWBaseboardNum = 1; ConvHWBaseboardNum <= NumConvHWBaseboards; ++ConvHWBaseboardNum) {
                int NumAlphas = 0;
                int NumNums = 0;
                int IOStat = 0;

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         s_ipsc->cCurrentModuleObject,
                                                                         ConvHWBaseboardNum,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                auto &thisBaseboard = state.dataBaseboardRadiator->baseboards(ConvHWBaseboardNum);
                thisBaseboard.FieldNames.allocate(NumNums);
                thisBaseboard.FieldNames = s_ipsc->cNumericFieldNames;

                // ErrorsFound will be set to True if problem was found, left untouched otherwise
                VerifyUniqueBaseboardName(
                    state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), ErrorsFound, s_ipsc->cCurrentModuleObject + " Name");

                thisBaseboard.EquipID = s_ipsc->cAlphaArgs(1); // name of this baseboard
                thisBaseboard.EquipType = DataPlant::PlantEquipmentType::Baseboard_Conv_Water;
                thisBaseboard.Schedule = s_ipsc->cAlphaArgs(2);
                if (s_ipsc->lAlphaFieldBlanks(2)) {
                    thisBaseboard.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisBaseboard.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                }
                // get inlet node number
                thisBaseboard.WaterInletNode = GetOnlySingleNode(state,
                                                                 s_ipsc->cAlphaArgs(3),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::ZoneHVACBaseboardConvectiveWater,
                                                                 s_ipsc->cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Water,
                                                                 DataLoopNode::ConnectionType::Inlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 ObjectIsNotParent);
                // get outlet node number
                thisBaseboard.WaterOutletNode = GetOnlySingleNode(state,
                                                                  s_ipsc->cAlphaArgs(4),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::ZoneHVACBaseboardConvectiveWater,
                                                                  s_ipsc->cAlphaArgs(1),
                                                                  DataLoopNode::NodeFluidType::Water,
                                                                  DataLoopNode::ConnectionType::Outlet,
                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                  ObjectIsNotParent);

                TestCompSet(state, cCMO_BBRadiator_Water, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(3), s_ipsc->cAlphaArgs(4), "Hot Water Nodes");

                // Determine steam baseboard radiator system heating design capacity sizing method
                if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "HeatingDesignCapacity")) {
                    thisBaseboard.HeatingCapMethod = HeatingDesignCapacity;
                    if (!s_ipsc->lNumericFieldBlanks(iHeatDesignCapacityNumericNum)) {
                        thisBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatDesignCapacityNumericNum);
                        if (thisBaseboard.ScaledHeatingCapacity < 0.0 && thisBaseboard.ScaledHeatingCapacity != AutoSize) {
                            ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                            ShowContinueError(state,
                                              format("Illegal {} = {:.7T}",
                                                     s_ipsc->cNumericFieldNames(iHeatDesignCapacityNumericNum),
                                                     s_ipsc->rNumericArgs(iHeatDesignCapacityNumericNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                        ShowContinueError(
                            state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(iHeatDesignCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "CapacityPerFloorArea")) {
                    thisBaseboard.HeatingCapMethod = CapacityPerFloorArea;
                    if (!s_ipsc->lNumericFieldBlanks(iHeatCapacityPerFloorAreaNumericNum)) {
                        thisBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum);
                        if (thisBaseboard.ScaledHeatingCapacity <= 0.0) {
                            ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                            ShowContinueError(
                                state,
                                format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                            ShowContinueError(state,
                                              format("Illegal {} = {:.7T}",
                                                     s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum),
                                                     s_ipsc->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum)));
                            ErrorsFound = true;
                        } else if (thisBaseboard.ScaledHeatingCapacity == AutoSize) {
                            ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                            ShowContinueError(
                                state,
                                format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                            ShowContinueError(state,
                                              format("Illegal {} = Autosize", s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                        ShowContinueError(
                            state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(state,
                                          format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    }
                } else if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "FractionOfAutosizedHeatingCapacity")) {
                    thisBaseboard.HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
                    if (!s_ipsc->lNumericFieldBlanks(iHeatFracOfAutosizedCapacityNumericNum)) {
                        thisBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum);
                        if (thisBaseboard.ScaledHeatingCapacity < 0.0) {
                            ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                            ShowContinueError(state,
                                              format("Illegal {} = {:.7T}",
                                                     s_ipsc->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum),
                                                     s_ipsc->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                        ShowContinueError(
                            state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(
                            state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", cCMO_BBRadiator_Water, thisBaseboard.EquipID));
                    ShowContinueError(state,
                                      format("Illegal {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ErrorsFound = true;
                }

                thisBaseboard.UA = s_ipsc->rNumericArgs(4);
                thisBaseboard.WaterVolFlowRateMax = s_ipsc->rNumericArgs(5);
                thisBaseboard.Offset = s_ipsc->rNumericArgs(6);
                // Set default convergence tolerance
                if (thisBaseboard.Offset <= 0.0) {
                    thisBaseboard.Offset = 0.001;
                }

                thisBaseboard.ZonePtr = DataZoneEquipment::GetZoneEquipControlledZoneNum(
                    state, DataZoneEquipment::ZoneEquipType::BaseboardConvectiveWater, thisBaseboard.EquipID);

                thisBaseboard.checkForZoneSizing(state); // check if any autosizing is being done
            }

            if (ErrorsFound) {
                ShowFatalError(state, format("{}Errors found in getting input.  Preceding condition(s) cause termination.", RoutineName));
            }
        }

        for (int BaseboardNum = 1; BaseboardNum <= NumConvHWBaseboards; ++BaseboardNum) {

            // Setup Report variables for the unit
            // CurrentModuleObject='ZoneHVAC:Baseboard:Convective:Water'
            auto &thisBaseboard = state.dataBaseboardRadiator->baseboards(BaseboardNum);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Energy",
                                Constant::Units::J,
                                thisBaseboard.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisBaseboard.EquipID,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Baseboard);

            SetupOutputVariable(state,
                                "Baseboard Hot Water Energy",
                                Constant::Units::J,
                                thisBaseboard.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisBaseboard.EquipID,
                                Constant::eResource::PlantLoopHeatingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Baseboard);

            SetupOutputVariable(state,
                                "Baseboard Total Heating Rate",
                                Constant::Units::W,
                                thisBaseboard.Power,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);

            SetupOutputVariable(state,
                                "Baseboard Hot Water Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisBaseboard.WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);

            SetupOutputVariable(state,
                                "Baseboard Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisBaseboard.AirMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);

            SetupOutputVariable(state,
                                "Baseboard Air Inlet Temperature",
                                Constant::Units::C,
                                thisBaseboard.AirInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);

            SetupOutputVariable(state,
                                "Baseboard Air Outlet Temperature",
                                Constant::Units::C,
                                thisBaseboard.AirOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);

            SetupOutputVariable(state,
                                "Baseboard Water Inlet Temperature",
                                Constant::Units::C,
                                thisBaseboard.WaterInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);

            SetupOutputVariable(state,
                                "Baseboard Water Outlet Temperature",
                                Constant::Units::C,
                                thisBaseboard.WaterOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipID);
        }
    }

    void BaseboardParams::InitBaseboard(EnergyPlusData &state, int baseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the Baseboard units during simulation.

        static constexpr std::string_view RoutineName = "BaseboardRadiator:InitBaseboard";

        if (this->SetLoopIndexFlag && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state, this->EquipID, this->EquipType, this->plantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "InitBaseboard: Program terminated for previous conditions.");
            }
            this->SetLoopIndexFlag = false;
        }

        if (!state.dataGlobal->SysSizingCalc && this->MySizeFlag && !this->SetLoopIndexFlag) {
            // for each coil, do the sizing once.
            this->SizeBaseboard(state, baseboardNum);

            this->MySizeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag && !this->SetLoopIndexFlag) {
            int WaterInletNode = this->WaterInletNode;
            Real64 rho = this->plantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
            this->WaterMassFlowRateMax = rho * this->WaterVolFlowRateMax;
            PlantUtilities::InitComponentNodes(state, 0.0, this->WaterMassFlowRateMax, this->WaterInletNode, this->WaterOutletNode);
            state.dataLoopNodes->Node(WaterInletNode).Temp = Constant::HWInitConvTemp;
            Real64 Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(WaterInletNode).Temp, RoutineName);
            state.dataLoopNodes->Node(WaterInletNode).Enthalpy = Cp * state.dataLoopNodes->Node(WaterInletNode).Temp;
            state.dataLoopNodes->Node(WaterInletNode).Quality = 0.0;
            state.dataLoopNodes->Node(WaterInletNode).Press = 0.0;
            state.dataLoopNodes->Node(WaterInletNode).HumRat = 0.0;
            // pick a mass flow rate that depends on the max water mass flow rate. CR 8842 changed to factor of 2.0
            if (this->AirMassFlowRate <= 0.0) {
                this->AirMassFlowRate = 2.0 * this->WaterMassFlowRateMax;
            }
            this->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        // Do the every time step initializations
        int WaterInletNode = this->WaterInletNode;
        int ZoneNode = state.dataZoneEquip->ZoneEquipConfig(this->ZonePtr).ZoneNode;
        this->WaterMassFlowRate = state.dataLoopNodes->Node(WaterInletNode).MassFlowRate;
        this->WaterInletTemp = state.dataLoopNodes->Node(WaterInletNode).Temp;
        this->WaterInletEnthalpy = state.dataLoopNodes->Node(WaterInletNode).Enthalpy;
        this->AirInletTemp = state.dataLoopNodes->Node(ZoneNode).Temp;
        this->AirInletHumRat = state.dataLoopNodes->Node(ZoneNode).HumRat;
    }

    void BaseboardParams::SizeBaseboard(EnergyPlusData &state, int baseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B.Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing hot water baseboard components for which flow rates and UAs have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data. UAs are
        // calculated by numerically inverting the baseboard calculation routine.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr Acc = 0.0001; // Accuracy of result
        int constexpr MaxIte = 500;    // Maximum number of iterations
        static std::string const RoutineName = cCMO_BBRadiator_Water + ":SizeBaseboard";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DesCoilLoad(0.0);
        Real64 UA0; // lower bound for UA
        Real64 UA1; // upper bound for UA
        Real64 UA;
        bool ErrorsFound(false);             // If errors detected in input
        Real64 rho;                          // local fluid density
        Real64 Cp;                           // local fluid specific heat
        Real64 WaterVolFlowRateMaxDes(0.0);  // Design water volume flow for reporting
        Real64 WaterVolFlowRateMaxUser(0.0); // User hard-sized volume flow for reporting
        Real64 UADes(0.0);                   // Design UA value for reporting
        Real64 UAUser(0.0);                  // User hard-sized value for reporting
        Real64 TempSize;                     // autosized value of coil input field

        // find the appropriate heating Plant Sizing object
        int PltSizHeatNum = this->plantLoc.loop->PlantSizNum;

        if (PltSizHeatNum > 0) {

            state.dataSize->DataScalableCapSizingON = false;

            if (state.dataSize->CurZoneEqNum > 0) {
                bool FlowAutoSize = false; // Indicator to autosizing water volume flow

                if (this->WaterVolFlowRateMax == DataSizing::AutoSize) {
                    FlowAutoSize = true;
                }
                if (!FlowAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation should continue
                    if (this->WaterVolFlowRateMax > 0.0) {
                        BaseSizer::reportSizerOutput(
                            state, cCMO_BBRadiator_Water, this->EquipID, "User-Specified Maximum Water Flow Rate [m3/s]", this->WaterVolFlowRateMax);
                    }
                } else {
                    auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
                    auto const &finalZoneSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum);
                    std::string_view const CompType = cCMO_BBRadiator_Water;
                    std::string_view const CompName = this->EquipID;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
                    state.dataSize->DataZoneNumber = this->ZonePtr;
                    int SizingMethod = HVAC::HeatingCapacitySizing;
                    int FieldNum = 1;
                    std::string const SizingString = format("{} [W]", this->FieldNames(FieldNum));
                    int CapSizingMethod = this->HeatingCapMethod;
                    zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                        CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {

                        if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                            if (this->ScaledHeatingCapacity == DataSizing::AutoSize) {
                                zoneEqSizing.DesHeatingLoad = finalZoneSizing.NonAirSysDesHeatLoad;
                            } else {
                                zoneEqSizing.DesHeatingLoad = this->ScaledHeatingCapacity;
                            }
                            zoneEqSizing.HeatingCapacity = true;
                            TempSize = zoneEqSizing.DesHeatingLoad;
                        } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                            zoneEqSizing.HeatingCapacity = true;
                            zoneEqSizing.DesHeatingLoad =
                                this->ScaledHeatingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                            TempSize = zoneEqSizing.DesHeatingLoad;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                            zoneEqSizing.HeatingCapacity = true;
                            state.dataSize->DataFracOfAutosizedHeatingCapacity = this->ScaledHeatingCapacity;
                            zoneEqSizing.DesHeatingLoad = finalZoneSizing.NonAirSysDesHeatLoad;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else {
                            TempSize = this->ScaledHeatingCapacity;
                        }
                        bool PrintFlag = false; // TRUE when sizing information is reported in the eio file
                        bool errorsFound = false;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                        state.dataSize->DataScalableCapSizingON = false;
                    } else {
                        DesCoilLoad = 0.0;
                    }

                    if (DesCoilLoad >= SmallLoad) {
                        Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                        rho = this->plantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        WaterVolFlowRateMaxDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                    } else {
                        WaterVolFlowRateMaxDes = 0.0;
                    }

                    if (FlowAutoSize) {
                        this->WaterVolFlowRateMax = WaterVolFlowRateMaxDes;
                        BaseSizer::reportSizerOutput(
                            state, cCMO_BBRadiator_Water, this->EquipID, "Design Size Maximum Water Flow Rate [m3/s]", WaterVolFlowRateMaxDes);
                    } else { // hard-sized with sizing data
                        if (this->WaterVolFlowRateMax > 0.0 && WaterVolFlowRateMaxDes > 0.0) {
                            WaterVolFlowRateMaxUser = this->WaterVolFlowRateMax;
                            BaseSizer::reportSizerOutput(state,
                                                         cCMO_BBRadiator_Water,
                                                         this->EquipID,
                                                         "Design Size Maximum Water Flow Rate [m3/s]",
                                                         WaterVolFlowRateMaxDes,
                                                         "User-Specified Maximum Water Flow Rate [m3/s]",
                                                         WaterVolFlowRateMaxUser);
                            // Report a warning to note difference between the two
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(WaterVolFlowRateMaxDes - WaterVolFlowRateMaxUser) / WaterVolFlowRateMaxUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(
                                        state,
                                        format("SizeBaseboard: Potential issue with equipment sizing for ZoneHVAC:Baseboard:Convective:Water=\"{}\".",
                                               this->EquipID));
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

                // UA sizing
                bool UAAutoSize = false; // Indicator to autosizing UA
                // Set hard-sized values to the local variable to correct a false indication after SolFla function calculation
                if (this->UA == DataSizing::AutoSize) {
                    UAAutoSize = true;
                } else {
                    UAUser = this->UA;
                }
                if (!UAAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation should continue
                    if (this->UA > 0.0) {
                        BaseSizer::reportSizerOutput(
                            state, cCMO_BBRadiator_Water, this->EquipID, "User-Specified U-Factor Times Area Value [W/K]", this->UA);
                    }
                } else {
                    auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
                    auto const &finalZoneSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum);
                    this->WaterInletTemp = state.dataSize->PlantSizData(PltSizHeatNum).ExitTemp;
                    this->AirInletTemp = finalZoneSizing.ZoneTempAtHeatPeak;
                    this->AirInletHumRat = finalZoneSizing.ZoneHumRatAtHeatPeak;
                    rho = this->plantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                    state.dataLoopNodes->Node(this->WaterInletNode).MassFlowRate = rho * this->WaterVolFlowRateMax;

                    std::string_view const CompType = cCMO_BBRadiator_Water;
                    std::string_view const CompName = this->EquipID;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
                    state.dataSize->DataZoneNumber = this->ZonePtr;
                    int SizingMethod = HVAC::HeatingCapacitySizing;
                    int FieldNum = 1;
                    std::string const SizingString = format("{} [W]", this->FieldNames(FieldNum));
                    int CapSizingMethod = this->HeatingCapMethod;
                    zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                        CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                        if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                            if (this->ScaledHeatingCapacity == DataSizing::AutoSize) {
                                zoneEqSizing.DesHeatingLoad = finalZoneSizing.NonAirSysDesHeatLoad;
                            } else {
                                zoneEqSizing.DesHeatingLoad = this->ScaledHeatingCapacity;
                            }
                            zoneEqSizing.HeatingCapacity = true;
                            TempSize = zoneEqSizing.DesHeatingLoad;
                        } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                            zoneEqSizing.HeatingCapacity = true;
                            zoneEqSizing.DesHeatingLoad =
                                this->ScaledHeatingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                            TempSize = zoneEqSizing.DesHeatingLoad;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                            zoneEqSizing.HeatingCapacity = true;
                            state.dataSize->DataFracOfAutosizedHeatingCapacity = this->ScaledHeatingCapacity;
                            zoneEqSizing.DesHeatingLoad = finalZoneSizing.NonAirSysDesHeatLoad;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else {
                            TempSize = this->ScaledHeatingCapacity;
                        }
                        bool PrintFlag = false;
                        bool errorsFound = false;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                        state.dataSize->DataScalableCapSizingON = false;
                    } else {
                        DesCoilLoad = 0.0; // FinalZoneSizing(CurZoneEqNum).NonAirSysDesHeatLoad;
                    }
                    if (DesCoilLoad >= SmallLoad) {
                        // pick an air  mass flow rate that is twice the water mass flow rate (CR8842)
                        this->DesAirMassFlowRate = 2.0 * rho * this->WaterVolFlowRateMax;
                        // set the lower and upper limits on the UA
                        UA0 = 0.001 * DesCoilLoad;
                        UA1 = DesCoilLoad;

                        // before iterating on a design UA check output at lower UA bound
                        this->UA = UA0;
                        Real64 LoadMet = 0.0;
                        SimHWConvective(state, baseboardNum, LoadMet);
                        if (LoadMet < DesCoilLoad) { // baseboard output should be below design load
                            // now check output at max UA (where UA = design load)
                            this->UA = UA1;
                            SimHWConvective(state, baseboardNum, LoadMet);

                            if (LoadMet > DesCoilLoad) { // if the load met is greater than design load, OK to iterate on UA
                                // Invert the baseboard model: given the design inlet conditions and the design load, find the design UA.
                                auto f = [&state, baseboardNum, DesCoilLoad](Real64 UA) {
                                    state.dataBaseboardRadiator->baseboards(baseboardNum).UA = UA;
                                    int localBaseBoardNum = baseboardNum;
                                    Real64 LoadMet = 0.0;
                                    SimHWConvective(state, localBaseBoardNum, LoadMet);
                                    return (DesCoilLoad - LoadMet) / DesCoilLoad;
                                };
                                int SolFla = 0;
                                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                                // if the numerical inversion failed, issue error messages.
                                if (SolFla == -1) {
                                    ShowSevereError(state,
                                                    format("SizeBaseboard: Autosizing of HW baseboard UA failed for {}=\"{}\"",
                                                           cCMO_BBRadiator_Water,
                                                           this->EquipID));
                                    ShowContinueError(state, "Iteration limit exceeded in calculating coil UA");
                                    if (UAAutoSize) {
                                        ErrorsFound = true;
                                    } else {
                                        ShowContinueError(
                                            state, "Could not calculate design value for comparison to user value, and the simulation continues");
                                        UA = 0.0;
                                    }
                                } else if (SolFla == -2) {
                                    ShowSevereError(state,
                                                    format("SizeBaseboard: Autosizing of HW baseboard UA failed for {}=\"{}\"",
                                                           cCMO_BBRadiator_Water,
                                                           this->EquipID));
                                    ShowContinueError(state, "Bad starting values for UA");
                                    if (UAAutoSize) {
                                        ErrorsFound = true;
                                    } else {
                                        ShowContinueError(
                                            state, "Could not calculate design value for comparison to user value, and the simulation continues");
                                        UA = 0.0;
                                    }
                                }
                                UADes = UA; // baseboard->baseboards(BaseboardNum)%UA = UA
                            } else {        // baseboard design load is greater than output at UA = design load so set UA = design load
                                UADes = UA1;
                                if (UAAutoSize) {
                                    ShowWarningError(state,
                                                     format("SizeBaseboard: Autosizing of HW baseboard UA failed for {}=\"{}\"",
                                                            cCMO_BBRadiator_Water,
                                                            this->EquipID));
                                    ShowContinueError(
                                        state, format("Design UA set equal to design coil load for {}=\"{}\"", cCMO_BBRadiator_Water, this->EquipID));
                                    ShowContinueError(state, format("Design coil load used during sizing = {:.5R} W.", DesCoilLoad));
                                    ShowContinueError(state, format("Inlet water temperature used during sizing = {:.5R} C.", this->WaterInletTemp));
                                }
                            }
                        } else { // baseboard design load is less than output at UA = 0.001 * design load so set UA to minimum value
                            UADes = UA0;
                            if (UAAutoSize) {
                                ShowWarningError(state,
                                                 format("SizeBaseboard: Autosizing of HW baseboard UA failed for {}=\"{}\"",
                                                        cCMO_BBRadiator_Water,
                                                        this->EquipID));
                                ShowContinueError(
                                    state,
                                    format("Design UA set equal to 0.001 * design coil load for {}=\"{}\"", cCMO_BBRadiator_Water, this->EquipID));
                                ShowContinueError(state, format("Design coil load used during sizing = {:.5R} W.", DesCoilLoad));
                                ShowContinueError(state, format("Inlet water temperature used during sizing = {:.5R} C.", this->WaterInletTemp));
                            }
                        }

                    } else {
                        UADes = 0.0;
                    }

                    if (UAAutoSize) {
                        this->UA = UADes;
                        BaseSizer::reportSizerOutput(
                            state, cCMO_BBRadiator_Water, this->EquipID, "Design Size U-Factor Times Area Value [W/K]", UADes);
                    } else {               // Hard-sized with sizing data
                        this->UA = UAUser; // need to put this back as HWBaseboardUAResidual will have reset it, CR9377
                        if (UAUser > 0.0 && UADes > 0.0) {
                            BaseSizer::reportSizerOutput(state,
                                                         cCMO_BBRadiator_Water,
                                                         this->EquipID,
                                                         "Design Size U-Factor Times Area Value [W/K]",
                                                         UADes,
                                                         "User-Specified U-Factor Times Area Value [W/K]",
                                                         UAUser);
                            // Report difference between design size and hard-sized values
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(UADes - UAUser) / UAUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(
                                        state,
                                        format("SizeBaseboard: Potential issue with equipment sizing for ZoneHVAC:Baseboard:Convective:Water=\"{}\".",
                                               this->EquipID));
                                    ShowContinueError(state, format("User-Specified U-Factor Times Area Value of {:.2R} [W/K]", UAUser));
                                    ShowContinueError(state, format("differs from Design Size U-Factor Times Area Value of {:.2R} [W/K]", UADes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // if there is no heating Sizing:Plant object and autosizing was requested, issue an error message
            if (this->WaterVolFlowRateMax == DataSizing::AutoSize || this->UA == DataSizing::AutoSize) {
                ShowSevereError(state, format("SizeBaseboard: {}=\"{}\"", cCMO_BBRadiator_Water, this->EquipID));
                ShowContinueError(state, "...Autosizing of hot water baseboard requires a heating loop Sizing:Plant object");
                ErrorsFound = true;
            }
        }

        // save the design water flow rate for use by the water loop sizing algorithms
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->WaterInletNode, this->WaterVolFlowRateMax);

        if (ErrorsFound) {
            ShowFatalError(state, "SizeBaseboard: Preceding sizing errors cause program termination");
        }
    }

    void BaseboardParams::checkForZoneSizing(EnergyPlusData &state)
    {
        // If any sizing is requested, check that zone sizing has been done
        // Condition 1: Is UA autosized)?
        // Condition 2: Is max flow rate  autosized?
        // Condition 3: Is HeatingDesignCapacity used and autosized)
        // Condition 4: Is FractionOfAutosizedHeatingCapacity used and heating capacity is autosized
        if ((this->UA == DataSizing::AutoSize) || (this->WaterVolFlowRateMax == DataSizing::AutoSize) ||
            ((this->HeatingCapMethod == DataSizing::HeatingDesignCapacity) && (this->ScaledHeatingCapacity == DataSizing::AutoSize)) ||
            ((this->HeatingCapMethod == DataSizing::FractionOfAutosizedHeatingCapacity) && (this->ScaledHeatingCapacity == DataSizing::AutoSize))) {
            CheckZoneSizing(state, cCMO_BBRadiator_Water, this->EquipID);
        }
    }

    void SimHWConvective(EnergyPlusData &state, int &BaseboardNum, Real64 &LoadMet)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997
        //       MODIFIED       May 2000 Fred Buhl
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE: This subroutine calculates the heat exchange rate
        // in a pure convective baseboard heater.  The heater is assumed to be crossflow
        // with both fluids unmixed. The air flow is buoyancy driven and a constant air
        // flow velocity of 0.5m/s is assumed. The solution is by the effectiveness-NTU
        // method found in Icropera and DeWitt, Fundamentals of Heat and Mass Transfer,
        // Chapter 11.4, p. 523, eq. 11.33

        // REFERENCES:
        // Icropera and DeWitt, Fundamentals of Heat and Mass Transfer,
        // Chapter 11.4, p. 523, eq. 11.33

        // Using/Aliasing
        using namespace DataSizing;
        using HVAC::SmallLoad;
        using PlantUtilities::SetActuatedBranchFlowRate;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName(cCMO_BBRadiator_Water + ":SimHWConvective");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneNum;
        Real64 WaterInletTemp;
        Real64 AirInletTemp;
        Real64 CpAir;
        Real64 CpWater;
        Real64 AirMassFlowRate;
        Real64 WaterMassFlowRate;
        Real64 CapacitanceAir;
        Real64 CapacitanceWater;
        Real64 CapacitanceMax;
        Real64 CapacitanceMin;
        Real64 CapacityRatio;
        Real64 NTU;
        Real64 Effectiveness;
        Real64 WaterOutletTemp;
        Real64 AirOutletTemp;
        Real64 AA;
        Real64 BB;
        Real64 CC;
        Real64 QZnReq;

        auto &baseboard = state.dataBaseboardRadiator->baseboards(BaseboardNum);

        ZoneNum = baseboard.ZonePtr;
        QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        if (baseboard.MySizeFlag) {
            QZnReq = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad; // If in sizing, assign design condition
        }

        WaterInletTemp = baseboard.WaterInletTemp;
        AirInletTemp = baseboard.AirInletTemp;

        CpWater = baseboard.plantLoc.loop->glycol->getSpecificHeat(state, WaterInletTemp, RoutineName);
        CpAir = PsyCpAirFnW(baseboard.AirInletHumRat);

        if (baseboard.DesAirMassFlowRate > 0.0) { // If UA is autosized, assign design condition
            AirMassFlowRate = baseboard.DesAirMassFlowRate;
        } else {
            AirMassFlowRate = baseboard.AirMassFlowRate;
            // pick a mass flow rate that depends on the max water mass flow rate. CR 8842 changed to factor of 2.0
            if (AirMassFlowRate <= 0.0) {
                AirMassFlowRate = 2.0 * baseboard.WaterMassFlowRateMax;
            }
        }

        WaterMassFlowRate = state.dataLoopNodes->Node(baseboard.WaterInletNode).MassFlowRate;
        CapacitanceAir = CpAir * AirMassFlowRate;

        if (QZnReq > SmallLoad && (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) || baseboard.MySizeFlag) &&
            (baseboard.availSched->getCurrentVal() > 0 || baseboard.MySizeFlag) && (WaterMassFlowRate > 0.0)) {

            CapacitanceWater = CpWater * WaterMassFlowRate;
            CapacitanceMax = max(CapacitanceAir, CapacitanceWater);
            CapacitanceMin = min(CapacitanceAir, CapacitanceWater);
            CapacityRatio = CapacitanceMin / CapacitanceMax;
            NTU = baseboard.UA / CapacitanceMin;
            // The effectiveness is given by the following formula:
            // Effectiveness = 1. - EXP((1./CapacityRatio)*(NTU)**0.22*(EXP(-CapacityRatio*(NTU)**0.78)-1.))
            // To prevent possible underflows (numbers smaller than the computer can handle) we must break
            // the calculation up into steps and check the size of the exponential arguments.
            AA = -CapacityRatio * std::pow(NTU, 0.78);
            if (AA < DataPrecisionGlobals::EXP_LowerLimit) {
                BB = 0.0;
            } else {
                BB = std::exp(AA);
            }
            CC = (1.0 / CapacityRatio) * std::pow(NTU, 0.22) * (BB - 1.0);
            if (CC < DataPrecisionGlobals::EXP_LowerLimit) {
                Effectiveness = 1.0;
            } else {
                Effectiveness = 1.0 - std::exp(CC);
            }
            AirOutletTemp = AirInletTemp + Effectiveness * CapacitanceMin * (WaterInletTemp - AirInletTemp) / CapacitanceAir;
            WaterOutletTemp = WaterInletTemp - CapacitanceAir * (AirOutletTemp - AirInletTemp) / CapacitanceWater;
            LoadMet = CapacitanceWater * (WaterInletTemp - WaterOutletTemp);
            baseboard.WaterOutletEnthalpy = baseboard.WaterInletEnthalpy - LoadMet / WaterMassFlowRate;
        } else {
            AirOutletTemp = AirInletTemp;
            WaterOutletTemp = WaterInletTemp;
            LoadMet = 0.0;
            baseboard.WaterOutletEnthalpy = baseboard.WaterInletEnthalpy;
            WaterMassFlowRate = 0.0;

            SetActuatedBranchFlowRate(state, WaterMassFlowRate, baseboard.WaterInletNode, baseboard.plantLoc, false);
            AirMassFlowRate = 0.0;
        }

        baseboard.WaterOutletTemp = WaterOutletTemp;
        baseboard.AirOutletTemp = AirOutletTemp;
        baseboard.Power = LoadMet;
        baseboard.WaterMassFlowRate = WaterMassFlowRate;
        baseboard.AirMassFlowRate = AirMassFlowRate;
    }

    void UpdateBaseboard(EnergyPlusData &state, int &BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // Using/Aliasing
        using PlantUtilities::SafeCopyPlantNode;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int WaterInletNode;
        int WaterOutletNode;

        auto &baseboard = state.dataBaseboardRadiator;

        WaterInletNode = baseboard->baseboards(BaseboardNum).WaterInletNode;
        WaterOutletNode = baseboard->baseboards(BaseboardNum).WaterOutletNode;

        SafeCopyPlantNode(state, WaterInletNode, WaterOutletNode);
        // Set the outlet air nodes of the Baseboard
        // Set the outlet water nodes for the Coil
        state.dataLoopNodes->Node(WaterOutletNode).Temp = baseboard->baseboards(BaseboardNum).WaterOutletTemp;
        state.dataLoopNodes->Node(WaterOutletNode).Enthalpy = baseboard->baseboards(BaseboardNum).WaterOutletEnthalpy;
    }

} // namespace BaseboardRadiator

} // namespace EnergyPlus
