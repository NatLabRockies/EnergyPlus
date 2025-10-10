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

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/AirTerminalUnit.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DualDuct.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACCooledBeam.hh>
#include <EnergyPlus/HVACFourPipeBeam.hh>
#include <EnergyPlus/HVACSingleDuctInduc.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/UserDefinedComponents.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

namespace EnergyPlus {

namespace ZoneAirLoopEquipmentManager {
    // Module containing the routines dealing with the ZoneAirLoopEquipmentManager

    // MODULE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   May 1997

    using namespace DataDefineEquip;

    // constexpr std::array<std::string_view, static_cast<int>(ZnAirLoopEquipType::Num)> ZnAirLoopEquipTypeNames = {
    //     "AirTerminal:DualDuct:ConstantVolume",
    //     "AirTerminal:DualDuct:VAV",
    //     "AirTerminal:SingleDuct:VAV:Reheat",
    //     "AirTerminal:SingleDuct:VAV:NoReheat",
    //     "AirTerminal:SingleDuct:ConstantVolume:Reheat",
    //     "AirTerminal:SingleDuct:ConstantVolume:NoReheat",
    //     "AirTerminal:SingleDuct:SeriesPIU:Reheat",
    //     "AirTerminal:SingleDuct:ParallelPIU:Reheat",
    //     "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction",
    //     "AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan",
    //     "AirTerminal:SingleDuct:VAV:HeatAndCool:Reheat",
    //     "AirTerminal:SingleDuct:VAV:HeatAndCool:NoReheat",
    //     "AirTerminal:SingleDuct:ConstantVolume:CooledBeam",
    //     "AirTerminal:DualDuct:VAV:OutdoorAir",
    //     "AirTerminal:SingleDuct:UserDefined",
    //     "AirTerminal:SingleDuct:Mixer",
    //     "AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam"};

    constexpr std::array<std::string_view, static_cast<int>(ZnAirLoopEquipType::Num)> ZnAirLoopEquipTypeNamesUC = {
        "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME",
        "AIRTERMINAL:DUALDUCT:VAV",
        "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT",
        "AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT",
        "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT",
        "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT",
        "AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT",
        "AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT",
        "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION",
        "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN",
        "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT",
        "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT",
        "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM",
        "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR",
        "AIRTERMINAL:SINGLEDUCT:USERDEFINED",
        "AIRTERMINAL:SINGLEDUCT:MIXER",
        "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEBEAM"};

    void ManageZoneAirLoopEquipment(EnergyPlusData &state,
                                    std::string const &ZoneAirLoopEquipName,
                                    bool const FirstHVACIteration,
                                    Real64 &SysOutputProvided,
                                    Real64 &NonAirSysOutput,
                                    Real64 &LatOutputProvided, // Latent add/removal supplied by air dist unit (kg/s), dehumid = negative
                                    int const ControlledZoneNum,
                                    int &CompIndex)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   May 1997
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)

        // PURPOSE OF THIS SUBROUTINE:
        // Calls the zone thermal control simulations and the interfaces
        // (water-air, refrigerant-air, steam-air, electric-electric,
        // water-water, etc)

        int AirDistUnitNum;

        // Beginning of Code

        // make sure the input data is read in only once
        if (state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag) {
            GetZoneAirLoopEquipment(state);
            state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
        }

        // Find the correct Zone Air Distribution Unit Equipment
        if (CompIndex == 0) {
            AirDistUnitNum = Util::FindItemInList(ZoneAirLoopEquipName, state.dataDefineEquipment->AirDistUnit);
            if (AirDistUnitNum == 0) {
                ShowFatalError(state, format("ManageZoneAirLoopEquipment: Unit not found={}", ZoneAirLoopEquipName));
            }
            CompIndex = AirDistUnitNum;
        } else {
            AirDistUnitNum = CompIndex;
            if (AirDistUnitNum > (int)state.dataDefineEquipment->AirDistUnit.size() || AirDistUnitNum < 1) {
                ShowFatalError(state,
                               format("ManageZoneAirLoopEquipment:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      AirDistUnitNum,
                                      (int)state.dataDefineEquipment->AirDistUnit.size(),
                                      ZoneAirLoopEquipName));
            }
            if (ZoneAirLoopEquipName != state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).Name) {
                ShowFatalError(state,
                               format("ManageZoneAirLoopEquipment: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      AirDistUnitNum,
                                      ZoneAirLoopEquipName,
                                      state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).Name));
            }
        }
        state.dataSize->CurTermUnitSizingNum = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).TermUnitSizingNum;
        InitZoneAirLoopEquipment(state, AirDistUnitNum, ControlledZoneNum);
        InitZoneAirLoopEquipmentTimeStep(state, AirDistUnitNum);

        SimZoneAirLoopEquipment(state, AirDistUnitNum, SysOutputProvided, NonAirSysOutput, LatOutputProvided, FirstHVACIteration, ControlledZoneNum);

        // Call one-time init to fill termunit sizing and other data for the ADU - can't do this until the actual terminal unit nodes have been
        // matched to zone euqip config nodes
        InitZoneAirLoopEquipment(state, AirDistUnitNum, ControlledZoneNum);
    }

    void GetZoneAirLoopEquipment(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   June 1997
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Get all the system related equipment which may be attached to
        // a zone

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // REFERENCES:
        // na

        // Using/Aliasing
        using NodeInputManager::GetOnlySingleNode;
        using namespace DataLoopNode;
        using BranchNodeConnections::SetUpCompSets;
        using DualDuct::GetDualDuctOutdoorAirRecircUse;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetZoneAirLoopEquipment: ");   // include trailing blank space
        static std::string const CurrentModuleObject("ZoneHVAC:AirDistributionUnit"); // Object type for getting and error messages

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // If errors detected in input
        Array1D_string AlphArray(5);
        Array1D<Real64> NumArray(2);
        Array1D_string cAlphaFields(5);   // Alpha field names
        Array1D_string cNumericFields(2); // Numeric field names
        Array1D_bool lAlphaBlanks(5);     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks(2);   // Logical array, numeric field input BLANK = .TRUE.

        int NumAirDistUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        state.dataDefineEquipment->AirDistUnit.allocate(NumAirDistUnits);

        if (NumAirDistUnits > 0) {
            int NumAlphas;
            int NumNums;
            int IOStat;
            bool IsNotOK; // Flag to verify name

            for (int AirDistUnitNum = 1; AirDistUnitNum <= NumAirDistUnits; ++AirDistUnitNum) {
                auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum);
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         AirDistUnitNum,
                                                                         AlphArray,
                                                                         NumAlphas,
                                                                         NumArray,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields); //  data for one zone
                Util::IsNameEmpty(state, AlphArray(1), CurrentModuleObject, ErrorsFound);

                airDistUnit.Name = AlphArray(1);
                // Input Outlet Node Num
                airDistUnit.OutletNodeNum = GetOnlySingleNode(state,
                                                              AlphArray(2),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::ZoneHVACAirDistributionUnit,
                                                              AlphArray(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Outlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              ObjectIsParent);
                airDistUnit.InletNodeNum = 0;
                airDistUnit.NumComponents = 1;
                int AirDistCompUnitNum = 1;
                // Load the air Distribution Unit Equip and Name
                airDistUnit.EquipType(AirDistCompUnitNum) = AlphArray(3);
                airDistUnit.EquipName(AirDistCompUnitNum) = AlphArray(4);
                ValidateComponent(state, AlphArray(3), AlphArray(4), IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("In {} = {}", CurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
                airDistUnit.UpStreamLeakFrac = NumArray(1);
                airDistUnit.DownStreamLeakFrac = NumArray(2);
                if (airDistUnit.DownStreamLeakFrac <= 0.0) {
                    airDistUnit.LeakLoadMult = 1.0;
                } else if (airDistUnit.DownStreamLeakFrac < 1.0 && airDistUnit.DownStreamLeakFrac > 0.0) {
                    airDistUnit.LeakLoadMult = 1.0 / (1.0 - airDistUnit.DownStreamLeakFrac);
                } else {
                    ShowSevereError(state, format("Error found in {} = {}", CurrentModuleObject, airDistUnit.Name));
                    ShowContinueError(state, format("{} must be less than 1.0", cNumericFields(2)));
                    ErrorsFound = true;
                }
                if (airDistUnit.UpStreamLeakFrac > 0.0) {
                    airDistUnit.UpStreamLeak = true;
                } else {
                    airDistUnit.UpStreamLeak = false;
                }
                if (airDistUnit.DownStreamLeakFrac > 0.0) {
                    airDistUnit.DownStreamLeak = true;
                } else {
                    airDistUnit.DownStreamLeak = false;
                }

                // DesignSpecification:AirTerminal:Sizing name
                airDistUnit.AirTerminalSizingSpecIndex = 0;
                if (!lAlphaBlanks(5)) {
                    airDistUnit.AirTerminalSizingSpecIndex = Util::FindItemInList(AlphArray(5), state.dataSize->AirTerminalSizingSpec);
                    if (airDistUnit.AirTerminalSizingSpecIndex == 0) {
                        ShowSevereError(state, format("{} = {} not found.", cAlphaFields(5), AlphArray(5)));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, airDistUnit.Name));
                        ErrorsFound = true;
                    }
                }

                const std::string typeNameUC = Util::makeUPPER(airDistUnit.EquipType(AirDistCompUnitNum));
                airDistUnit.EquipTypeEnum(AirDistCompUnitNum) = static_cast<ZnAirLoopEquipType>(getEnumValue(ZnAirLoopEquipTypeNamesUC, typeNameUC));
                // Validate EquipType for Air Distribution Unit
                switch (airDistUnit.EquipTypeEnum(AirDistCompUnitNum)) {
                case DataDefineEquip::ZnAirLoopEquipType::DualDuctConstVolume:
                case DataDefineEquip::ZnAirLoopEquipType::DualDuctVAV:
                case DataDefineEquip::ZnAirLoopEquipType::DualDuctVAVOutdoorAir:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ParallelPIU_Reheat:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ConstVol_4PipeInduc:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheatVSFan:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolCooledBeam:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctUserDefined:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctATMixer:
                    if (airDistUnit.UpStreamLeak || airDistUnit.DownStreamLeak) {
                        ShowSevereError(state, format("Error found in {} = {}", CurrentModuleObject, airDistUnit.Name));
                        ShowContinueError(state,
                                          format("Simple duct leakage model not available for {} = {}",
                                                 cAlphaFields(3),
                                                 airDistUnit.EquipType(AirDistCompUnitNum)));
                        ErrorsFound = true;
                    }
                    break;
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolFourPipeBeam:
                    airDistUnit.airTerminalPtr = FourPipeBeam::HVACFourPipeBeam::fourPipeBeamFactory(state, airDistUnit.EquipName(1));
                    if (airDistUnit.UpStreamLeak || airDistUnit.DownStreamLeak) {
                        ShowSevereError(state, format("Error found in {} = {}", CurrentModuleObject, airDistUnit.Name));
                        ShowContinueError(state,
                                          format("Simple duct leakage model not available for {} = {}",
                                                 cAlphaFields(3),
                                                 airDistUnit.EquipType(AirDistCompUnitNum)));
                        ErrorsFound = true;
                    }
                    break;
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolReheat:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolNoReheat:
                    break;
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheat:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVNoReheat:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVReheat:
                case DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVNoReheat:
                    airDistUnit.IsConstLeakageRate = true;
                    break;
                default:
                    ShowSevereError(state, format("Error found in {} = {}", CurrentModuleObject, airDistUnit.Name));
                    ShowContinueError(state, format("Invalid {} = {}", cAlphaFields(3), airDistUnit.EquipType(AirDistCompUnitNum)));
                    ErrorsFound = true;
                    break;
                } // end switch

                // Set up component set for air terminal unit
                if ((airDistUnit.EquipTypeEnum(AirDistCompUnitNum) == DataDefineEquip::ZnAirLoopEquipType::DualDuctConstVolume) ||
                    (airDistUnit.EquipTypeEnum(AirDistCompUnitNum) == DataDefineEquip::ZnAirLoopEquipType::DualDuctVAV)) {
                    //  For dual duct units, set up two component sets, one for heat and one for cool
                    SetUpCompSets(state,
                                  CurrentModuleObject,
                                  airDistUnit.Name,
                                  airDistUnit.EquipType(AirDistCompUnitNum) + ":HEAT",
                                  airDistUnit.EquipName(AirDistCompUnitNum),
                                  "UNDEFINED",
                                  AlphArray(2));
                    SetUpCompSets(state,
                                  CurrentModuleObject,
                                  airDistUnit.Name,
                                  airDistUnit.EquipType(AirDistCompUnitNum) + ":COOL",
                                  airDistUnit.EquipName(AirDistCompUnitNum),
                                  "UNDEFINED",
                                  AlphArray(2));
                    //  For dual duct units with decoupled OA and RA, set up two component sets, one for OA (Outdoor Air)
                    //  and one for RA (Recirculated Air)
                } else if (airDistUnit.EquipTypeEnum(AirDistCompUnitNum) == DataDefineEquip::ZnAirLoopEquipType::DualDuctVAVOutdoorAir) {
                    SetUpCompSets(state,
                                  CurrentModuleObject,
                                  airDistUnit.Name,
                                  airDistUnit.EquipType(AirDistCompUnitNum) + ":OutdoorAir",
                                  airDistUnit.EquipName(AirDistCompUnitNum),
                                  "UNDEFINED",
                                  AlphArray(2));
                    bool DualDuctRecircIsUsed; // local temporary for deciding if recirc side used by dual duct terminal
                    GetDualDuctOutdoorAirRecircUse(
                        state, airDistUnit.EquipType(AirDistCompUnitNum), airDistUnit.EquipName(AirDistCompUnitNum), DualDuctRecircIsUsed);
                    if (DualDuctRecircIsUsed) {
                        SetUpCompSets(state,
                                      CurrentModuleObject,
                                      airDistUnit.Name,
                                      airDistUnit.EquipType(AirDistCompUnitNum) + ":RecirculatedAir",
                                      airDistUnit.EquipName(AirDistCompUnitNum),
                                      "UNDEFINED",
                                      AlphArray(2));
                    }
                } else {
                    SetUpCompSets(state,
                                  CurrentModuleObject,
                                  airDistUnit.Name,
                                  airDistUnit.EquipType(AirDistCompUnitNum),
                                  airDistUnit.EquipName(AirDistCompUnitNum),
                                  "UNDEFINED",
                                  AlphArray(2));
                }

            } // End of Air Dist Do Loop
            for (int AirDistUnitNum = 1; AirDistUnitNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++AirDistUnitNum) {
                auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Sensible Heating Energy",
                                    Constant::Units::J,
                                    airDistUnit.HeatGain,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    airDistUnit.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Sensible Cooling Energy",
                                    Constant::Units::J,
                                    airDistUnit.CoolGain,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    airDistUnit.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Sensible Heating Rate",
                                    Constant::Units::W,
                                    airDistUnit.HeatRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airDistUnit.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Sensible Cooling Rate",
                                    Constant::Units::W,
                                    airDistUnit.CoolRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    airDistUnit.Name);
            }
        }
        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in getting {} Input", RoutineName, CurrentModuleObject));
        }
    }

    void InitZoneAirLoopEquipment(EnergyPlusData &state, int const AirDistUnitNum, int const ControlledZoneNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   Nov 1997

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is left for Module format consistency -- not needed in this module.

        // Do the Begin Simulation initializations
        if (!state.dataZoneAirLoopEquipmentManager->InitAirDistUnitsFlag) {
            return;
        }
        if (state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).EachOnceFlag &&
            (state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).TermUnitSizingNum > 0)) {

            {
                auto &thisADU = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum);
                {
                    auto &thisZoneEqConfig(state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum));
                    thisADU.ZoneNum = ControlledZoneNum;
                    for (int inletNum = 1; inletNum <= thisZoneEqConfig.NumInletNodes; ++inletNum) {
                        if (thisZoneEqConfig.InletNode(inletNum) == thisADU.OutletNodeNum) {
                            thisZoneEqConfig.InletNodeADUNum(inletNum) = AirDistUnitNum;
                        }
                    }
                }

                // Fill TermUnitSizing with specs from DesignSpecification:AirTerminal:Sizing
                {
                    auto &thisTermUnitSizingData(state.dataSize->TermUnitSizing(thisADU.TermUnitSizingNum));
                    thisTermUnitSizingData.ADUName = thisADU.Name;
                    if (thisADU.AirTerminalSizingSpecIndex > 0) {
                        {
                            auto const &thisAirTermSizingSpec(state.dataSize->AirTerminalSizingSpec(thisADU.AirTerminalSizingSpecIndex));
                            thisTermUnitSizingData.SpecDesCoolSATRatio = thisAirTermSizingSpec.DesCoolSATRatio;
                            thisTermUnitSizingData.SpecDesHeatSATRatio = thisAirTermSizingSpec.DesHeatSATRatio;
                            thisTermUnitSizingData.SpecDesSensCoolingFrac = thisAirTermSizingSpec.DesSensCoolingFrac;
                            thisTermUnitSizingData.SpecDesSensHeatingFrac = thisAirTermSizingSpec.DesSensHeatingFrac;
                            thisTermUnitSizingData.SpecMinOAFrac = thisAirTermSizingSpec.MinOAFrac;
                        }
                    }
                }
            }

            if (state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).ZoneNum != 0 &&
                state.dataHeatBal->Zone(state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).ZoneNum).HasAdjustedReturnTempByITE) {
                for (int AirDistCompNum = 1; AirDistCompNum <= state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).NumComponents;
                     ++AirDistCompNum) {
                    if (state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).EquipTypeEnum(AirDistCompNum) !=
                            DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheat &&
                        state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).EquipTypeEnum(AirDistCompNum) !=
                            DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVNoReheat) {
                        ShowSevereError(state,
                                        "The FlowControlWithApproachTemperatures only works with ITE zones with single duct VAV terminal unit.");
                        ShowContinueError(state, "The return air temperature of the ITE will not be overwritten.");
                        ShowFatalError(state, "Preceding condition causes termination.");
                    }
                }
            }
            state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).EachOnceFlag = false;
            ++state.dataZoneAirLoopEquipmentManager->numADUInitialized;
            if (state.dataZoneAirLoopEquipmentManager->numADUInitialized == (int)state.dataDefineEquipment->AirDistUnit.size()) {
                // If all ADUs are initialized, set InitAirDistUnitsFlag to false
                state.dataZoneAirLoopEquipmentManager->InitAirDistUnitsFlag = false;
            }
        }
    }

    void InitZoneAirLoopEquipmentTimeStep(EnergyPlusData &state, int const AirDistUnitNum)
    {
        auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum);
        // every time step
        airDistUnit.MassFlowRateDnStrLk = 0.0;
        airDistUnit.MassFlowRateUpStrLk = 0.0;
        airDistUnit.MassFlowRateTU = 0.0;
        airDistUnit.MassFlowRateZSup = 0.0;
        airDistUnit.MassFlowRateSup = 0.0;
        airDistUnit.HeatRate = 0.0;
        airDistUnit.CoolRate = 0.0;
        airDistUnit.HeatGain = 0.0;
        airDistUnit.CoolGain = 0.0;
    }

    void SimZoneAirLoopEquipment(EnergyPlusData &state,
                                 int const AirDistUnitNum,
                                 Real64 &SysOutputProvided,
                                 Real64 &NonAirSysOutput,
                                 Real64 &LatOutputProvided, // Latent add/removal provided by this unit (kg/s), dehumidify = negative
                                 bool const FirstHVACIteration,
                                 int const ControlledZoneNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Russ Taylor
        //       DATE WRITTEN   May 1997
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates primary system air supplied to a zone and calculates
        // airflow requirements

        using DualDuct::SimulateDualDuct;
        using HVACCooledBeam::SimCoolBeam;
        using HVACSingleDuctInduc::SimIndUnit;
        using PoweredInductionUnits::SimPIU;
        using Psychrometrics::PsyCpAirFnW;
        using SingleDuct::GetATMixers;
        using SingleDuct::SimulateSingleDuct;
        using UserDefinedComponents::SimAirTerminalUserDefined;

        bool ProvideSysOutput;
        int AirDistCompNum;
        int AirLoopNum(0);                  // index of air loop
        Real64 MassFlowRateMaxAvail;        // max avail mass flow rate excluding leaks [kg/s]
        Real64 MassFlowRateMinAvail;        // min avail mass flow rate excluding leaks [kg/s]
        Real64 MassFlowRateUpStreamLeakMax; // max upstream leak flow rate [kg/s]
        Real64 DesFlowRatio(0.0);           // ratio of system to sum of zones design flow rate
        Real64 SpecHumOut(0.0);             // Specific humidity ratio of outlet air (kg moisture / kg moist air)
        Real64 SpecHumIn(0.0);              // Specific humidity ratio of inlet air (kg moisture / kg moist air)

        auto &controlledZoneAirNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;

        ProvideSysOutput = true;
        for (AirDistCompNum = 1; AirDistCompNum <= state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).NumComponents; ++AirDistCompNum) {
            NonAirSysOutput = 0.0;

            auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum);
            int InNodeNum = airDistUnit.InletNodeNum;
            int OutNodeNum = airDistUnit.OutletNodeNum;
            MassFlowRateMaxAvail = 0.0;
            MassFlowRateMinAvail = 0.0;
            // check for no plenum
            // set the max and min avail flow rates taking into acount the upstream leak
            if (airDistUnit.UpStreamLeak) {
                if (InNodeNum > 0) {
                    MassFlowRateMaxAvail = state.dataLoopNodes->Node(InNodeNum).MassFlowRateMaxAvail;
                    MassFlowRateMinAvail = state.dataLoopNodes->Node(InNodeNum).MassFlowRateMinAvail;
                    if (airDistUnit.IsConstLeakageRate) {
                        AirLoopNum = airDistUnit.AirLoopNum;
                        if (AirLoopNum > 0) {
                            DesFlowRatio = state.dataAirLoop->AirLoopFlow(AirLoopNum).SysToZoneDesFlowRatio;
                        } else {
                            DesFlowRatio = 1.0;
                        }
                        MassFlowRateUpStreamLeakMax =
                            max(airDistUnit.UpStreamLeakFrac * state.dataLoopNodes->Node(InNodeNum).MassFlowRateMax * DesFlowRatio, 0.0);
                    } else {
                        MassFlowRateUpStreamLeakMax = max(airDistUnit.UpStreamLeakFrac * MassFlowRateMaxAvail, 0.0);
                    }
                    if (MassFlowRateMaxAvail > MassFlowRateUpStreamLeakMax) {
                        airDistUnit.MassFlowRateUpStrLk = MassFlowRateUpStreamLeakMax;
                        state.dataLoopNodes->Node(InNodeNum).MassFlowRateMaxAvail = MassFlowRateMaxAvail - MassFlowRateUpStreamLeakMax;
                    } else {
                        airDistUnit.MassFlowRateUpStrLk = MassFlowRateMaxAvail;
                        state.dataLoopNodes->Node(InNodeNum).MassFlowRateMaxAvail = 0.0;
                    }
                    state.dataLoopNodes->Node(InNodeNum).MassFlowRateMinAvail = max(0.0, MassFlowRateMinAvail - airDistUnit.MassFlowRateUpStrLk);
                }
            }

            switch (airDistUnit.EquipTypeEnum(AirDistCompNum)) {
            case DataDefineEquip::ZnAirLoopEquipType::DualDuctConstVolume: {
                SimulateDualDuct(state,
                                 airDistUnit.EquipName(AirDistCompNum),
                                 FirstHVACIteration,
                                 ControlledZoneNum,
                                 controlledZoneAirNode,
                                 airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::DualDuctVAV: {
                SimulateDualDuct(state,
                                 airDistUnit.EquipName(AirDistCompNum),
                                 FirstHVACIteration,
                                 ControlledZoneNum,
                                 controlledZoneAirNode,
                                 airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::DualDuctVAVOutdoorAir: {
                SimulateDualDuct(state,
                                 airDistUnit.EquipName(AirDistCompNum),
                                 FirstHVACIteration,
                                 ControlledZoneNum,
                                 controlledZoneAirNode,
                                 airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheat: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVReheat: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVNoReheat: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctCBVAVNoReheat: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolReheat: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolNoReheat: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat: {
                SimPIU(state,
                       airDistUnit.EquipName(AirDistCompNum),
                       FirstHVACIteration,
                       ControlledZoneNum,
                       controlledZoneAirNode,
                       airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ParallelPIU_Reheat: {
                SimPIU(state,
                       airDistUnit.EquipName(AirDistCompNum),
                       FirstHVACIteration,
                       ControlledZoneNum,
                       controlledZoneAirNode,
                       airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ConstVol_4PipeInduc: {
                SimIndUnit(state,
                           airDistUnit.EquipName(AirDistCompNum),
                           FirstHVACIteration,
                           ControlledZoneNum,
                           controlledZoneAirNode,
                           airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctVAVReheatVSFan: {
                SimulateSingleDuct(state,
                                   airDistUnit.EquipName(AirDistCompNum),
                                   FirstHVACIteration,
                                   ControlledZoneNum,
                                   controlledZoneAirNode,
                                   airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolCooledBeam: {
                SimCoolBeam(state,
                            airDistUnit.EquipName(AirDistCompNum),
                            FirstHVACIteration,
                            ControlledZoneNum,
                            controlledZoneAirNode,
                            airDistUnit.EquipIndex(AirDistCompNum),
                            NonAirSysOutput);
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctConstVolFourPipeBeam: {
                airDistUnit.airTerminalPtr->simulate(state, FirstHVACIteration, NonAirSysOutput);
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctUserDefined: {
                SimAirTerminalUserDefined(state,
                                          airDistUnit.EquipName(AirDistCompNum),
                                          FirstHVACIteration,
                                          ControlledZoneNum,
                                          controlledZoneAirNode,
                                          airDistUnit.EquipIndex(AirDistCompNum));
            } break;
            case DataDefineEquip::ZnAirLoopEquipType::SingleDuctATMixer: {
                GetATMixers(state); // Needed here if mixer used only with unitarysystem which gets its input late
                ProvideSysOutput = false;
            } break;
            default: {
                ShowSevereError(state, format("Error found in ZoneHVAC:AirDistributionUnit={}", airDistUnit.Name));
                ShowContinueError(state, format("Invalid Component={}", airDistUnit.EquipType(AirDistCompNum)));
                ShowFatalError(state, "Preceding condition causes termination.");
            } break;
            }

            // do leak mass flow calcs
            if (InNodeNum > 0) { // InNodeNum is not always known when this is called, eg FPIU
                InNodeNum = airDistUnit.InletNodeNum;
                if (airDistUnit.UpStreamLeak) {
                    state.dataLoopNodes->Node(InNodeNum).MassFlowRateMaxAvail = MassFlowRateMaxAvail;
                    state.dataLoopNodes->Node(InNodeNum).MassFlowRateMinAvail = MassFlowRateMinAvail;
                }
                if ((airDistUnit.UpStreamLeak || airDistUnit.DownStreamLeak) && MassFlowRateMaxAvail > 0.0) {
                    airDistUnit.MassFlowRateTU = state.dataLoopNodes->Node(InNodeNum).MassFlowRate;
                    airDistUnit.MassFlowRateZSup = airDistUnit.MassFlowRateTU * (1.0 - airDistUnit.DownStreamLeakFrac);
                    airDistUnit.MassFlowRateDnStrLk = airDistUnit.MassFlowRateTU * airDistUnit.DownStreamLeakFrac;
                    airDistUnit.MassFlowRateSup = airDistUnit.MassFlowRateTU + airDistUnit.MassFlowRateUpStrLk;
                    state.dataLoopNodes->Node(InNodeNum).MassFlowRate = airDistUnit.MassFlowRateSup;
                    state.dataLoopNodes->Node(OutNodeNum).MassFlowRate = airDistUnit.MassFlowRateZSup;
                    state.dataLoopNodes->Node(OutNodeNum).MassFlowRateMaxAvail =
                        max(0.0, MassFlowRateMaxAvail - airDistUnit.MassFlowRateDnStrLk - airDistUnit.MassFlowRateUpStrLk);
                    state.dataLoopNodes->Node(OutNodeNum).MassFlowRateMinAvail =
                        max(0.0, MassFlowRateMinAvail - airDistUnit.MassFlowRateDnStrLk - airDistUnit.MassFlowRateUpStrLk);
                    airDistUnit.MaxAvailDelta = MassFlowRateMaxAvail - state.dataLoopNodes->Node(OutNodeNum).MassFlowRateMaxAvail;
                    airDistUnit.MinAvailDelta = MassFlowRateMinAvail - state.dataLoopNodes->Node(OutNodeNum).MassFlowRateMinAvail;
                } else {
                    // if no leaks, or a terminal unit type not supported for leaks
                    DataDefineEquip::ZnAirLoopEquipType termUnitType = airDistUnit.EquipTypeEnum(AirDistCompNum);
                    if ((termUnitType == DataDefineEquip::ZnAirLoopEquipType::DualDuctConstVolume) ||
                        (termUnitType == DataDefineEquip::ZnAirLoopEquipType::DualDuctVAV) ||
                        (termUnitType == DataDefineEquip::ZnAirLoopEquipType::DualDuctVAVOutdoorAir)) {
                        // Use ADU outlet node flow for dual duct terminal units (which don't support leaks)
                        airDistUnit.MassFlowRateTU = state.dataLoopNodes->Node(OutNodeNum).MassFlowRate;
                        airDistUnit.MassFlowRateZSup = state.dataLoopNodes->Node(OutNodeNum).MassFlowRate;
                        airDistUnit.MassFlowRateSup = state.dataLoopNodes->Node(OutNodeNum).MassFlowRate;
                    } else {
                        airDistUnit.MassFlowRateTU = state.dataLoopNodes->Node(InNodeNum).MassFlowRate;
                        airDistUnit.MassFlowRateZSup = state.dataLoopNodes->Node(InNodeNum).MassFlowRate;
                        airDistUnit.MassFlowRateSup = state.dataLoopNodes->Node(InNodeNum).MassFlowRate;
                    }
                }
            }
        }
        if (ProvideSysOutput) {
            int OutletNodeNum = state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).OutletNodeNum;
            SpecHumOut = state.dataLoopNodes->Node(OutletNodeNum).HumRat;
            SpecHumIn = state.dataLoopNodes->Node(controlledZoneAirNode).HumRat;
            // Sign convention: SysOutputProvided <0 Zone is cooled
            //                  SysOutputProvided >0 Zone is heated
            SysOutputProvided = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate *
                                Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(state.dataLoopNodes->Node(OutletNodeNum).Temp,
                                                                           SpecHumOut,
                                                                           state.dataLoopNodes->Node(controlledZoneAirNode).Temp,
                                                                           SpecHumIn); // sensible {W};
            // Sign convention: LatOutputProvided <0 Zone is dehumidified
            //                  LatOutputProvided >0 Zone is humidified
            // CR9155 Remove specific humidity calculations
            LatOutputProvided =
                state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative
        } else {
            SysOutputProvided = 0.0;
            LatOutputProvided = 0.0;
        }
    }

} // namespace ZoneAirLoopEquipmentManager

} // namespace EnergyPlus
