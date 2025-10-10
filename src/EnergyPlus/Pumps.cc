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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
// #include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantPressureSystem.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Pumps.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::Pumps {

// MODULE INFORMATION:
//       AUTHOR         Dan Fisher
//       DATE WRITTEN   Sept 1998
//       MODIFIED       July 2001, Richard Liesen
//                      July 2001, Rick Strand (new "local" pump control method)
//                      Feb 2005, Rahul Chillar(added condensate pump for steam systems)
//                      Jan 2006, Sankaranarayanan (Added pump banks to the library of pumps)
//                      May 2009, Brent Griffith (added support for EMS override of massflow)
//                      Aug 2010, Edwin Lee (refactored code, significant clean-up)
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// Encapsulates the data and algorithms to simulate pumps.

// REFERENCES:
// HVAC 2 Toolkit:  A Toolkit for Secondary HVAC System
// Energy Calculations, ASHRAE, 1993, pp2-10 to 2-15

// Using/Aliasing
using DataLoopNode::ObjectIsNotParent;
using HVAC::SmallWaterVolFlow;

static constexpr std::array<std::string_view, static_cast<int>(PumpType::Num)> pumpTypeIDFNames = {
    "Pump:VariableSpeed", "Pump:ConstantSpeed", "Pump:VariableSpeed:Condensate", "HeaderedPumps:VariableSpeed", "HeaderedPumps:ConstantSpeed"};

void SimPumps(EnergyPlusData &state,
              std::string const &PumpName, // Name of pump to be managed
              int const LoopNum,           // Plant loop number
              Real64 const FlowRequest,    // requested flow from adjacent demand side
              bool &PumpRunning,           // .TRUE. if the loop pump is actually operating
              int &PumpIndex,
              Real64 &PumpHeat)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   July 2001
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages the pump operation based on the type of
    // pump and the pump controls (continuous, intermittent, etc.).  The
    // result of this subroutine is that the pump has been simulated for
    // the necessary loop and the PumpRunning has been correctly set.

    int PumpNum; // Pump index within PumpEquip derived type

    // Get input from IDF one time
    if (state.dataPumps->GetInputFlag) {
        GetPumpInput(state);
        state.dataPumps->GetInputFlag = false;
    }

    // Exit early if no pumps found
    if (state.dataPumps->NumPumps == 0) {
        PumpHeat = 0.0;
        return;
    }

    // Setup pump component index if needed
    if (PumpIndex == 0) {
        PumpNum = Util::FindItemInList(PumpName, state.dataPumps->PumpEquip); // Determine which pump to simulate
        if (PumpNum == 0) {
            ShowFatalError(state, format("ManagePumps: Pump requested not found ={}", PumpName)); // Catch any bad names before crashing
        }
        PumpIndex = PumpNum;
    } else {
        PumpNum = PumpIndex;
        if (state.dataPumps->PumpEquip(PumpNum).CheckEquipName) {
            if (PumpNum > state.dataPumps->NumPumps || PumpNum < 1) {
                ShowFatalError(
                    state,
                    format(
                        "ManagePumps: Invalid PumpIndex passed={}, Number of Pumps={}, Pump name={}", PumpNum, state.dataPumps->NumPumps, PumpName));
            }
            if (PumpName != state.dataPumps->PumpEquip(PumpNum).Name) {
                ShowFatalError(state,
                               format("ManagePumps: Invalid PumpIndex passed={}, Pump name={}, stored Pump Name for that index={}",
                                      PumpNum,
                                      PumpName,
                                      state.dataPumps->PumpEquip(PumpNum).Name));
            }
            state.dataPumps->PumpEquip(PumpNum).CheckEquipName = false;
        }
    }

    // Perform one-time and begin-environment initialization
    InitializePumps(state, PumpNum);

    // If all we need is to set outlet min/max avail, then just do it and get out.  Also, we only do min/max avail on flow query
    if (state.dataPlnt->PlantLoop(LoopNum).LoopSide(state.dataPumps->PumpEquip(PumpNum).plantLoc.loopSideNum).FlowLock ==
        DataPlant::FlowLock::PumpQuery) {
        SetupPumpMinMaxFlows(state, LoopNum, PumpNum);
        return;
    }

    // Set pump flow rate and calculate power
    CalcPumps(state, PumpNum, FlowRequest, PumpRunning);

    // Update pump reporting data
    ReportPumps(state, PumpNum);

    // Send this up to the calling routine
    PumpHeat = state.dataPumps->PumpHeattoFluid;
}

void GetPumpInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    April 1998
    //       MODIFIED:        July 2001, Rick Strand (addition of pump controls)
    //                        May 2009, Brent Griffith (added EMS calls)

    // PURPOSE OF THIS SUBROUTINE:
    // This routine will get the input
    // required by the pump simulation.

    // PUMP:VARIABLE SPEED,
    // This pump model is described in the ASHRAE secondary HVAC toolkit.

    // REFERENCES:
    // HVAC 2 Toolkit:  A Toolkit for Secondary HVAC System
    //  Energy Calculations, ASHRAE, 1993, pp2-10 to 2-15

    // Using/Aliasing
    using BranchNodeConnections::TestCompSet;
    using Curve::GetCurveIndex;
    using Curve::GetCurveMinMaxValues;
    using DataSizing::AutoSize;
    using NodeInputManager::GetOnlySingleNode;

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr StartTemp(100.0); // Standard Temperature across code to calculated Steam density
    static constexpr std::string_view RoutineName("GetPumpInput: ");
    static constexpr std::string_view routineName = "GetPumpInput";

    static constexpr std::array<std::string_view, static_cast<int>(PumpControlType::Num)> pumpCtrlTypeNamesUC{"CONTINUOUS", "INTERMITTENT"};
    static constexpr std::array<std::string_view, static_cast<int>(ControlTypeVFD::Num)> controlTypeVFDNamesUC{"MANUALCONTROL",
                                                                                                               "PRESSURESETPOINTCONTROL"};
    static constexpr std::array<std::string_view, static_cast<int>(PowerSizingMethod::Num)> powerSizingMethodNamesUC{"POWERPERFLOW",
                                                                                                                     "POWERPERFLOWPERPRESSURE"};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int PumpNum;
    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool ErrorsFound;
    int TempCurveIndex;
    int NumVarSpeedPumps = 0;
    int NumConstSpeedPumps = 0;
    int NumCondensatePumps = 0;
    int NumPumpBankSimpleVar = 0;
    int NumPumpBankSimpleConst = 0;
    Real64 SteamDensity;
    Real64 TempWaterDensity;
    int DummyWaterIndex(1);
    Real64 constexpr minToMaxRatioMax = 0.99;

    ErrorsFound = false;

    // GET NUMBER OF ALL EQUIPMENT TYPES
    NumVarSpeedPumps = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pumpTypeIDFNames[static_cast<int>(PumpType::VarSpeed)]);
    NumConstSpeedPumps = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pumpTypeIDFNames[static_cast<int>(PumpType::ConSpeed)]);
    NumCondensatePumps = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pumpTypeIDFNames[static_cast<int>(PumpType::Cond)]);
    NumPumpBankSimpleVar =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pumpTypeIDFNames[static_cast<int>(PumpType::Bank_VarSpeed)]);
    NumPumpBankSimpleConst =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, pumpTypeIDFNames[static_cast<int>(PumpType::Bank_ConSpeed)]);
    state.dataPumps->NumPumps = NumVarSpeedPumps + NumConstSpeedPumps + NumCondensatePumps + NumPumpBankSimpleVar + NumPumpBankSimpleConst;

    if (state.dataPumps->NumPumps <= 0) {
        ShowWarningError(state, "No Pumping Equipment Found");
        return;
    }

    state.dataPumps->PumpEquip.allocate(state.dataPumps->NumPumps);
    state.dataPumps->PumpUniqueNames.reserve(static_cast<unsigned>(state.dataPumps->NumPumps));
    state.dataPumps->PumpEquipReport.allocate(state.dataPumps->NumPumps);
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
    cCurrentModuleObject = pumpTypeIDFNames[static_cast<int>(PumpType::VarSpeed)];
    auto &thisInput = state.dataIPShortCut;

    for (PumpNum = 1; PumpNum <= NumVarSpeedPumps; ++PumpNum) {
        auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 PumpNum,
                                                                 thisInput->cAlphaArgs,
                                                                 NumAlphas,
                                                                 thisInput->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 thisInput->lNumericFieldBlanks,
                                                                 thisInput->lAlphaFieldBlanks,
                                                                 thisInput->cAlphaFieldNames,
                                                                 thisInput->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, thisInput->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPumps->PumpUniqueNames, thisInput->cAlphaArgs(1), cCurrentModuleObject, thisInput->cAlphaFieldNames(1), ErrorsFound);
        thisPump.Name = thisInput->cAlphaArgs(1);
        thisPump.pumpType = PumpType::VarSpeed; //'Pump:VariableSpeed'
        thisPump.TypeOf_Num = DataPlant::PlantEquipmentType::PumpVariableSpeed;

        thisPump.InletNodeNum = GetOnlySingleNode(state,
                                                  thisInput->cAlphaArgs(2),
                                                  ErrorsFound,
                                                  DataLoopNode::ConnectionObjectType::PumpVariableSpeed,
                                                  thisPump.Name,
                                                  DataLoopNode::NodeFluidType::Water,
                                                  DataLoopNode::ConnectionType::Inlet,
                                                  NodeInputManager::CompFluidStream::Primary,
                                                  ObjectIsNotParent);

        thisPump.OutletNodeNum = GetOnlySingleNode(state,
                                                   thisInput->cAlphaArgs(3),
                                                   ErrorsFound,
                                                   DataLoopNode::ConnectionObjectType::PumpVariableSpeed,
                                                   thisPump.Name,
                                                   DataLoopNode::NodeFluidType::Water,
                                                   DataLoopNode::ConnectionType::Outlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   ObjectIsNotParent);
        TestCompSet(state, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaArgs(2), thisInput->cAlphaArgs(3), "Water Nodes");

        thisPump.PumpControl = static_cast<PumpControlType>(getEnumValue(pumpCtrlTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(4))));
        if (thisPump.PumpControl == PumpControlType::Invalid) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Invalid {}", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaFieldNames(4)));
            ShowContinueError(
                state,
                format("Entered Value=[{}]. {} has been set to Continuous for this pump.", thisInput->cAlphaArgs(4), thisInput->cAlphaFieldNames(4)));
            thisPump.PumpControl = PumpControlType::Continuous;
        }

        // Input the optional schedule for the pump, this is a flow modifier schedule so blank/missing means Always On.
        if (thisInput->cAlphaArgs(5).empty()) {
            thisPump.flowRateSched = nullptr;
        } else if ((thisPump.flowRateSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(5))) == nullptr) {
            ShowWarningItemNotFound(state, eoh, thisInput->cAlphaFieldNames(5), thisInput->cAlphaArgs(5), "");
        }

        thisPump.NomVolFlowRate = thisInput->rNumericArgs(1);
        if (thisPump.NomVolFlowRate == AutoSize) {
            thisPump.NomVolFlowRateWasAutoSized = true;
        }
        thisPump.NomPumpHead = thisInput->rNumericArgs(2);
        thisPump.NomPowerUse = thisInput->rNumericArgs(3);
        if (thisPump.NomPowerUse == AutoSize) {
            thisPump.NomPowerUseWasAutoSized = true;
        }
        thisPump.MotorEffic = thisInput->rNumericArgs(4);
        thisPump.FracMotorLossToFluid = thisInput->rNumericArgs(5);
        thisPump.PartLoadCoef[0] = thisInput->rNumericArgs(6);
        thisPump.PartLoadCoef[1] = thisInput->rNumericArgs(7);
        thisPump.PartLoadCoef[2] = thisInput->rNumericArgs(8);
        thisPump.PartLoadCoef[3] = thisInput->rNumericArgs(9);
        thisPump.MinVolFlowRate = thisInput->rNumericArgs(10);
        if (thisPump.MinVolFlowRate == AutoSize) {
            thisPump.minVolFlowRateWasAutosized = true;
        } else if (!thisPump.NomVolFlowRateWasAutoSized && (thisPump.MinVolFlowRate > (minToMaxRatioMax * thisPump.NomVolFlowRate))) {
            // Check that the minimum isn't greater than the maximum
            ShowWarningError(
                state, format("{}{}=\"{}\", Invalid '{}'", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cNumericFieldNames(10)));
            ShowContinueError(state,
                              format("Entered Value=[{:.5T}] is above or too close (equal) to the {}=[{:.5T}].",
                                     thisPump.MinVolFlowRate,
                                     thisInput->cNumericFieldNames(1),
                                     thisPump.NomVolFlowRate));
            ShowContinueError(
                state,
                format("Reseting value of '{}' to the value of 99% of '{}'.", thisInput->cNumericFieldNames(10), thisInput->cNumericFieldNames(1)));
            // Set min to roughly max, but not quite, otherwise it can't turn on, ever
            thisPump.MinVolFlowRate = minToMaxRatioMax * thisPump.NomVolFlowRate;
        }
        // Probably the following two lines will be used if the team agrees on changing the F10 value from min flow rate to
        // minimum flow as a fraction of nominal flow.

        // Input pressure related data such as pressure curve and impeller size/rotational speed
        if (thisInput->cAlphaArgs(6).empty()) {
            thisPump.PressureCurve_Index = -1;
        } else {
            TempCurveIndex = GetCurveIndex(state, thisInput->cAlphaArgs(6));
            if (TempCurveIndex == 0) {
                thisPump.PressureCurve_Index = -1;
            } else {
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     TempCurveIndex,                  // Curve index
                                                     {1},                             // Valid dimensions
                                                     RoutineName,                     // Routine name
                                                     cCurrentModuleObject,            // Object Type
                                                     thisPump.Name,                   // Object Name
                                                     thisInput->cAlphaFieldNames(6)); // Field Name

                if (!ErrorsFound) {
                    thisPump.PressureCurve_Index = TempCurveIndex;
                    GetCurveMinMaxValues(state, TempCurveIndex, thisPump.MinPhiValue, thisPump.MaxPhiValue);
                }
            }
        }

        // read in the rest of the pump pressure characteristics
        thisPump.ImpellerDiameter = thisInput->rNumericArgs(11);

        // Input VFD related data
        if (thisInput->lAlphaFieldBlanks(7)) {
            thisPump.HasVFD = false;
        } else {
            thisPump.HasVFD = true;
            thisPump.VFD.VFDControlType =
                static_cast<ControlTypeVFD>(getEnumValue(controlTypeVFDNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(7))));

            switch (thisPump.VFD.VFDControlType) {

            case ControlTypeVFD::VFDManual: {
                if ((thisPump.VFD.manualRPMSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, thisInput->cAlphaFieldNames(8), thisInput->cAlphaArgs(8));
                    ErrorsFound = true;
                } else if (!thisPump.VFD.manualRPMSched->checkMinVal(state, Clusive::Ex, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, thisInput->cAlphaFieldNames(8), thisInput->cAlphaArgs(8), Clusive::Ex, 0.0);
                    ErrorsFound = true;
                }
            } break;

            case ControlTypeVFD::VFDAutomatic: {
                if (thisInput->lAlphaFieldBlanks(9)) {
                    ShowSevereEmptyField(state, eoh, thisInput->cAlphaFieldNames(9));
                    ErrorsFound = true;
                } else if ((thisPump.VFD.lowerPsetSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(9))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, thisInput->cAlphaFieldNames(9), thisInput->cAlphaArgs(9));
                    ErrorsFound = true;
                }

                if (thisInput->lAlphaFieldBlanks(10)) {
                    ShowSevereEmptyField(state, eoh, thisInput->cAlphaFieldNames(10));
                    ErrorsFound = true;
                } else if ((thisPump.VFD.upperPsetSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(10))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, thisInput->cAlphaFieldNames(10), thisInput->cAlphaArgs(10));
                    ErrorsFound = true;
                }

                if (thisInput->lAlphaFieldBlanks(11)) {
                    ShowSevereEmptyField(state, eoh, thisInput->cAlphaFieldNames(11));
                    ErrorsFound = true;
                } else if ((thisPump.VFD.minRPMSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(11))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, thisInput->cAlphaFieldNames(11), thisInput->cAlphaArgs(11));
                    ErrorsFound = true;
                } else if (!thisPump.VFD.minRPMSched->checkMinVal(state, Clusive::Ex, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, thisInput->cAlphaFieldNames(11), thisInput->cAlphaArgs(11), Clusive::Ex, 0.0);
                    ErrorsFound = true;
                }

                if (thisInput->lAlphaFieldBlanks(12)) {
                    ShowSevereEmptyField(state, eoh, thisInput->cAlphaFieldNames(12));
                    ErrorsFound = true;
                } else if ((thisPump.VFD.maxRPMSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(12))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, thisInput->cAlphaFieldNames(12), thisInput->cAlphaArgs(12));
                    ErrorsFound = true;
                } else if (!thisPump.VFD.maxRPMSched->checkMinVal(state, Clusive::Ex, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, thisInput->cAlphaFieldNames(12), thisInput->cAlphaArgs(12), Clusive::Ex, 0.0);
                    ErrorsFound = true;
                }
            } break;

            default: {
                ShowSevereError(state,
                                format("{}{}=\"{}\", VFD Control type entered is invalid.  Use one of the key choice entries.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       thisPump.Name));
                ErrorsFound = true;
            } break;
            }
        }

        if (!thisInput->lAlphaFieldBlanks(13)) { // zone named for pump skin losses
            thisPump.ZoneNum = Util::FindItemInList(thisInput->cAlphaArgs(13), state.dataHeatBal->Zone);
            if (thisPump.ZoneNum > 0) {
                thisPump.HeatLossesToZone = true;
                if (!thisInput->lNumericFieldBlanks(12)) {
                    thisPump.SkinLossRadFraction = thisInput->rNumericArgs(12);
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       cCurrentModuleObject,
                                       thisPump.Name,
                                       thisInput->cAlphaFieldNames(13),
                                       thisInput->cAlphaArgs(13)));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lAlphaFieldBlanks(14)) {
            thisPump.powerSizingMethod =
                static_cast<PowerSizingMethod>(getEnumValue(powerSizingMethodNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(14))));
            if (thisPump.powerSizingMethod == PowerSizingMethod::Invalid) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", sizing method type entered is invalid.  Use one of the key choice entries.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       thisPump.Name));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lNumericFieldBlanks(13)) {
            thisPump.powerPerFlowScalingFactor = thisInput->rNumericArgs(13);
        }

        if (!thisInput->lNumericFieldBlanks(14)) {
            thisPump.powerPerFlowPerPressureScalingFactor = thisInput->rNumericArgs(14);
        }

        if (!thisInput->lNumericFieldBlanks(15)) {
            thisPump.MinVolFlowRateFrac = thisInput->rNumericArgs(15);
        }

        if (NumAlphas > 14) {
            thisPump.EndUseSubcategoryName = thisInput->cAlphaArgs(15);
        } else {
            thisPump.EndUseSubcategoryName = "General";
        }

        // Is this really necessary for each pump GetInput loop?
        thisPump.Energy = 0.0;
        thisPump.Power = 0.0;
    }

    cCurrentModuleObject = pumpTypeIDFNames[static_cast<int>(PumpType::ConSpeed)];

    for (int NumConstPump = 1; NumConstPump <= NumConstSpeedPumps; ++NumConstPump) {
        PumpNum = NumVarSpeedPumps + NumConstPump;
        auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 NumConstPump,
                                                                 thisInput->cAlphaArgs,
                                                                 NumAlphas,
                                                                 thisInput->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 thisInput->lNumericFieldBlanks,
                                                                 thisInput->lAlphaFieldBlanks,
                                                                 thisInput->cAlphaFieldNames,
                                                                 thisInput->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPumps->PumpUniqueNames, thisInput->cAlphaArgs(1), cCurrentModuleObject, thisInput->cAlphaFieldNames(1), ErrorsFound);
        thisPump.Name = thisInput->cAlphaArgs(1);
        thisPump.pumpType = PumpType::ConSpeed; //'Pump:ConstantSpeed'
        thisPump.TypeOf_Num = DataPlant::PlantEquipmentType::PumpConstantSpeed;

        thisPump.InletNodeNum = GetOnlySingleNode(state,
                                                  thisInput->cAlphaArgs(2),
                                                  ErrorsFound,
                                                  DataLoopNode::ConnectionObjectType::PumpConstantSpeed,
                                                  thisPump.Name,
                                                  DataLoopNode::NodeFluidType::Water,
                                                  DataLoopNode::ConnectionType::Inlet,
                                                  NodeInputManager::CompFluidStream::Primary,
                                                  ObjectIsNotParent);

        thisPump.OutletNodeNum = GetOnlySingleNode(state,
                                                   thisInput->cAlphaArgs(3),
                                                   ErrorsFound,
                                                   DataLoopNode::ConnectionObjectType::PumpConstantSpeed,
                                                   thisPump.Name,
                                                   DataLoopNode::NodeFluidType::Water,
                                                   DataLoopNode::ConnectionType::Outlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   ObjectIsNotParent);
        TestCompSet(state, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaArgs(2), thisInput->cAlphaArgs(3), "Water Nodes");

        thisPump.NomVolFlowRate = thisInput->rNumericArgs(1);
        if (thisPump.NomVolFlowRate == AutoSize) {
            thisPump.NomVolFlowRateWasAutoSized = true;
        }
        thisPump.NomPumpHead = thisInput->rNumericArgs(2);
        thisPump.NomPowerUse = thisInput->rNumericArgs(3);
        if (thisPump.NomPowerUse == AutoSize) {
            thisPump.NomPowerUseWasAutoSized = true;
        }
        thisPump.MotorEffic = thisInput->rNumericArgs(4);
        thisPump.FracMotorLossToFluid = thisInput->rNumericArgs(5);
        thisPump.PartLoadCoef[0] = 1.0;
        thisPump.PartLoadCoef[1] = 0.0;
        thisPump.PartLoadCoef[2] = 0.0;
        thisPump.PartLoadCoef[3] = 0.0;
        // In a constant volume pump we previously set the minimum to the nominal capacity
        // Now we model the pump as constant speed and set flow by riding the pump curve.
        thisPump.MinVolFlowRate = 0.0;
        thisPump.Energy = 0.0;
        thisPump.Power = 0.0;

        thisPump.PumpControl = static_cast<PumpControlType>(getEnumValue(pumpCtrlTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(4))));

        if (thisPump.PumpControl == PumpControlType::Invalid) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Invalid {}", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaFieldNames(4)));
            ShowContinueError(
                state,
                format("Entered Value=[{}]. {} has been set to Continuous for this pump.", thisInput->cAlphaArgs(4), thisInput->cAlphaFieldNames(4)));
            thisPump.PumpControl = PumpControlType::Continuous;
        }

        // Input the optional schedule for the pump
        if (thisInput->lAlphaFieldBlanks(5)) {
            thisPump.flowRateSched = nullptr;
        } else if ((thisPump.flowRateSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(5))) == nullptr) {
            ShowWarningItemNotFound(state, eoh, thisInput->cAlphaFieldNames(5), thisInput->cAlphaArgs(5), "");
        }

        // Input pressure related data such as pressure curve and impeller size/rotational speed
        if (thisInput->cAlphaArgs(6).empty()) {
            thisPump.PressureCurve_Index = -1;
        } else {
            TempCurveIndex = GetCurveIndex(state, thisInput->cAlphaArgs(6));
            if (TempCurveIndex == 0) {
                thisPump.PressureCurve_Index = -1;
            } else {
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     TempCurveIndex,                  // Curve index
                                                     {1},                             // Valid dimensions
                                                     RoutineName,                     // Routine name
                                                     cCurrentModuleObject,            // Object Type
                                                     thisPump.Name,                   // Object Name
                                                     thisInput->cAlphaFieldNames(6)); // Field Name

                if (!ErrorsFound) {
                    thisPump.PressureCurve_Index = TempCurveIndex;
                    GetCurveMinMaxValues(state, TempCurveIndex, thisPump.MinPhiValue, thisPump.MaxPhiValue);
                }
            }
        }

        // read in the rest of the pump pressure characteristics
        thisPump.ImpellerDiameter = thisInput->rNumericArgs(6);
        thisPump.RotSpeed_RPM = thisInput->rNumericArgs(7); // retrieve the input rotational speed, in revs/min
        thisPump.RotSpeed = thisPump.RotSpeed_RPM / 60.0;   // convert input[rpm] to calculation units[rps]

        if (!thisInput->lAlphaFieldBlanks(7)) { // zone named for pump skin losses
            thisPump.ZoneNum = Util::FindItemInList(thisInput->cAlphaArgs(7), state.dataHeatBal->Zone);
            if (thisPump.ZoneNum > 0) {
                thisPump.HeatLossesToZone = true;
                if (!thisInput->lNumericFieldBlanks(8)) {
                    thisPump.SkinLossRadFraction = thisInput->rNumericArgs(8);
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       cCurrentModuleObject,
                                       thisPump.Name,
                                       thisInput->cAlphaFieldNames(7),
                                       thisInput->cAlphaArgs(7)));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lAlphaFieldBlanks(8)) {
            thisPump.powerSizingMethod =
                static_cast<PowerSizingMethod>(getEnumValue(powerSizingMethodNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(8))));
            if (thisPump.powerSizingMethod == PowerSizingMethod::Invalid) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", sizing method type entered is invalid.  Use one of the key choice entries.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       thisPump.Name));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lNumericFieldBlanks(9)) {
            thisPump.powerPerFlowScalingFactor = thisInput->rNumericArgs(9);
        }

        if (!thisInput->lNumericFieldBlanks(10)) {
            thisPump.powerPerFlowPerPressureScalingFactor = thisInput->rNumericArgs(10);
        }

        if (NumAlphas > 8) {
            thisPump.EndUseSubcategoryName = thisInput->cAlphaArgs(9);
        } else {
            thisPump.EndUseSubcategoryName = "General";
        }
    }

    // pumps for steam system pumping condensate
    cCurrentModuleObject = pumpTypeIDFNames[static_cast<int>(PumpType::Cond)];
    for (int NumCondPump = 1; NumCondPump <= NumCondensatePumps; ++NumCondPump) {
        PumpNum = NumCondPump + NumVarSpeedPumps + NumConstSpeedPumps;
        auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 NumCondPump,
                                                                 thisInput->cAlphaArgs,
                                                                 NumAlphas,
                                                                 thisInput->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 thisInput->lNumericFieldBlanks,
                                                                 thisInput->lAlphaFieldBlanks,
                                                                 thisInput->cAlphaFieldNames,
                                                                 thisInput->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, thisInput->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPumps->PumpUniqueNames, thisInput->cAlphaArgs(1), cCurrentModuleObject, thisInput->cAlphaFieldNames(1), ErrorsFound);
        thisPump.Name = thisInput->cAlphaArgs(1);
        thisPump.pumpType = PumpType::Cond; //'Pump:VariableSpeed:Condensate'
        thisPump.TypeOf_Num = DataPlant::PlantEquipmentType::PumpCondensate;

        thisPump.InletNodeNum = GetOnlySingleNode(state,
                                                  thisInput->cAlphaArgs(2),
                                                  ErrorsFound,
                                                  DataLoopNode::ConnectionObjectType::PumpVariableSpeedCondensate,
                                                  thisPump.Name,
                                                  DataLoopNode::NodeFluidType::Steam,
                                                  DataLoopNode::ConnectionType::Inlet,
                                                  NodeInputManager::CompFluidStream::Primary,
                                                  ObjectIsNotParent);

        thisPump.OutletNodeNum = GetOnlySingleNode(state,
                                                   thisInput->cAlphaArgs(3),
                                                   ErrorsFound,
                                                   DataLoopNode::ConnectionObjectType::PumpVariableSpeedCondensate,
                                                   thisPump.Name,
                                                   DataLoopNode::NodeFluidType::Steam,
                                                   DataLoopNode::ConnectionType::Outlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   ObjectIsNotParent);
        TestCompSet(state, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaArgs(2), thisInput->cAlphaArgs(3), "Water Nodes");

        thisPump.PumpControl = PumpControlType::Intermittent;

        // Input the optional schedule for the pump
        if (thisInput->cAlphaArgs(4).empty()) {
            thisPump.flowRateSched = nullptr;
        } else if ((thisPump.flowRateSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(4))) == nullptr) {
            ShowWarningItemNotFound(state, eoh, thisInput->cAlphaFieldNames(4), thisInput->cAlphaArgs(4));
        }

        thisPump.NomSteamVolFlowRate = thisInput->rNumericArgs(1);
        if (thisPump.NomSteamVolFlowRate == AutoSize) {
            thisPump.NomSteamVolFlowRateWasAutoSized = true;
        }
        thisPump.NomPumpHead = thisInput->rNumericArgs(2);
        thisPump.NomPowerUse = thisInput->rNumericArgs(3);
        if (thisPump.NomPowerUse == AutoSize) {
            thisPump.NomPowerUseWasAutoSized = true;
        }
        thisPump.MotorEffic = thisInput->rNumericArgs(4);
        thisPump.FracMotorLossToFluid = thisInput->rNumericArgs(5);
        thisPump.PartLoadCoef[0] = thisInput->rNumericArgs(6);
        thisPump.PartLoadCoef[1] = thisInput->rNumericArgs(7);
        thisPump.PartLoadCoef[2] = thisInput->rNumericArgs(8);
        thisPump.PartLoadCoef[3] = thisInput->rNumericArgs(9);

        if (!thisInput->lAlphaFieldBlanks(5)) { // zone named for pump skin losses
            thisPump.ZoneNum = Util::FindItemInList(thisInput->cAlphaArgs(5), state.dataHeatBal->Zone);
            if (thisPump.ZoneNum > 0) {
                thisPump.HeatLossesToZone = true;
                if (!thisInput->lNumericFieldBlanks(10)) {
                    thisPump.SkinLossRadFraction = thisInput->rNumericArgs(10);
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       cCurrentModuleObject,
                                       thisPump.Name,
                                       thisInput->cAlphaFieldNames(5),
                                       thisInput->cAlphaArgs(5)));
                ErrorsFound = true;
            }
        }

        thisPump.MinVolFlowRate = 0.0;
        thisPump.Energy = 0.0;
        thisPump.Power = 0.0;

        if (thisPump.NomSteamVolFlowRateWasAutoSized) {
            thisPump.NomVolFlowRate = AutoSize;
            thisPump.NomVolFlowRateWasAutoSized = true;
        } else {
            // Calc Condensate Pump Water Volume Flow Rate
            SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, StartTemp, 1.0, routineName);
            TempWaterDensity = Fluid::GetWater(state)->getDensity(state, Constant::InitConvTemp, routineName);
            thisPump.NomVolFlowRate = (thisPump.NomSteamVolFlowRate * SteamDensity) / TempWaterDensity;
        }

        if (!thisInput->lAlphaFieldBlanks(6)) {
            thisPump.powerSizingMethod =
                static_cast<PowerSizingMethod>(getEnumValue(powerSizingMethodNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(6))));
            if (thisPump.powerSizingMethod == PowerSizingMethod::Invalid) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", sizing method type entered is invalid.  Use one of the key choice entries.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       thisPump.Name));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lNumericFieldBlanks(11)) {
            thisPump.powerPerFlowScalingFactor = thisInput->rNumericArgs(11);
        }

        if (!thisInput->lNumericFieldBlanks(12)) {
            thisPump.powerPerFlowPerPressureScalingFactor = thisInput->rNumericArgs(12);
        }

        if (NumAlphas > 6) {
            thisPump.EndUseSubcategoryName = thisInput->cAlphaArgs(7);
        } else {
            thisPump.EndUseSubcategoryName = "General";
        }
    }

    // LOAD Variable Speed Pump Bank ARRAYS WITH VARIABLE SPEED CURVE FIT PUMP DATA
    cCurrentModuleObject = pumpTypeIDFNames[static_cast<int>(PumpType::Bank_VarSpeed)];
    for (int NumVarPumpBankSimple = 1; NumVarPumpBankSimple <= NumPumpBankSimpleVar; ++NumVarPumpBankSimple) {
        PumpNum = NumVarPumpBankSimple + NumVarSpeedPumps + NumConstSpeedPumps + NumCondensatePumps;
        auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 NumVarPumpBankSimple,
                                                                 thisInput->cAlphaArgs,
                                                                 NumAlphas,
                                                                 thisInput->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 thisInput->lNumericFieldBlanks,
                                                                 thisInput->lAlphaFieldBlanks,
                                                                 thisInput->cAlphaFieldNames,
                                                                 thisInput->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, thisInput->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPumps->PumpUniqueNames, thisInput->cAlphaArgs(1), cCurrentModuleObject, thisInput->cAlphaFieldNames(1), ErrorsFound);
        thisPump.Name = thisInput->cAlphaArgs(1);
        thisPump.pumpType = PumpType::Bank_VarSpeed; //'HeaderedPumps:VariableSpeed'
        thisPump.TypeOf_Num = DataPlant::PlantEquipmentType::PumpBankVariableSpeed;

        thisPump.InletNodeNum = GetOnlySingleNode(state,
                                                  thisInput->cAlphaArgs(2),
                                                  ErrorsFound,
                                                  DataLoopNode::ConnectionObjectType::HeaderedPumpsVariableSpeed,
                                                  thisPump.Name,
                                                  DataLoopNode::NodeFluidType::Water,
                                                  DataLoopNode::ConnectionType::Inlet,
                                                  NodeInputManager::CompFluidStream::Primary,
                                                  ObjectIsNotParent);

        thisPump.OutletNodeNum = GetOnlySingleNode(state,
                                                   thisInput->cAlphaArgs(3),
                                                   ErrorsFound,
                                                   DataLoopNode::ConnectionObjectType::HeaderedPumpsVariableSpeed,
                                                   thisPump.Name,
                                                   DataLoopNode::NodeFluidType::Water,
                                                   DataLoopNode::ConnectionType::Outlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   ObjectIsNotParent);
        TestCompSet(state, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaArgs(2), thisInput->cAlphaArgs(3), "Water Nodes");

        if (Util::SameString(thisInput->cAlphaArgs(4), "Optimal")) {
            thisPump.SequencingScheme = PumpBankControlSeq::OptimalScheme;
        } else if (Util::SameString(thisInput->cAlphaArgs(4), "Sequential")) {
            thisPump.SequencingScheme = PumpBankControlSeq::SequentialScheme;
        } else if (Util::SameString(thisInput->cAlphaArgs(4), "SupplyEquipmentAssigned")) {
            thisPump.SequencingScheme = PumpBankControlSeq::UserDefined;
        } else {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Invalid {}", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaFieldNames(4)));
            ShowContinueError(
                state,
                format("Entered Value=[{}]. {} has been set to Sequential for this pump.", thisInput->cAlphaArgs(4), thisInput->cAlphaFieldNames(4)));
            thisPump.SequencingScheme = PumpBankControlSeq::SequentialScheme;
        }

        thisPump.PumpControl = static_cast<PumpControlType>(getEnumValue(pumpCtrlTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(5))));
        if (thisPump.PumpControl == PumpControlType::Invalid) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Invalid {}", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaFieldNames(5)));
            ShowContinueError(
                state,
                format("Entered Value=[{}]. {} has been set to Continuous for this pump.", thisInput->cAlphaArgs(5), thisInput->cAlphaFieldNames(5)));
            thisPump.PumpControl = PumpControlType::Continuous;
        }

        // Input the optional schedule for the pump
        if (thisInput->cAlphaArgs(6).empty()) { // Initialized to zero, don't get a schedule for an empty
            thisPump.flowRateSched = nullptr;
        } else if ((thisPump.flowRateSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(6))) == nullptr) {
            ShowWarningItemNotFound(state, eoh, thisInput->cAlphaFieldNames(6), thisInput->cAlphaArgs(6));
        }

        thisPump.NomVolFlowRate = thisInput->rNumericArgs(1);
        if (thisPump.NomVolFlowRate == AutoSize) {
            thisPump.NomVolFlowRateWasAutoSized = true;
        }
        thisPump.NumPumpsInBank = thisInput->rNumericArgs(2);
        thisPump.NomPumpHead = thisInput->rNumericArgs(3);
        thisPump.NomPowerUse = thisInput->rNumericArgs(4);
        if (thisPump.NomPowerUse == AutoSize) {
            thisPump.NomPowerUseWasAutoSized = true;
        }
        thisPump.MotorEffic = thisInput->rNumericArgs(5);
        thisPump.FracMotorLossToFluid = thisInput->rNumericArgs(6);
        thisPump.PartLoadCoef[0] = thisInput->rNumericArgs(7);
        thisPump.PartLoadCoef[1] = thisInput->rNumericArgs(8);
        thisPump.PartLoadCoef[2] = thisInput->rNumericArgs(9);
        thisPump.PartLoadCoef[3] = thisInput->rNumericArgs(10);
        thisPump.MinVolFlowRateFrac = thisInput->rNumericArgs(11);
        thisPump.MinVolFlowRate = thisPump.NomVolFlowRate * thisPump.MinVolFlowRateFrac;

        if (!thisInput->lAlphaFieldBlanks(7)) { // zone named for pump skin losses
            thisPump.ZoneNum = Util::FindItemInList(thisInput->cAlphaArgs(7), state.dataHeatBal->Zone);
            if (thisPump.ZoneNum > 0) {
                thisPump.HeatLossesToZone = true;
                if (!thisInput->lNumericFieldBlanks(12)) {
                    thisPump.SkinLossRadFraction = thisInput->rNumericArgs(12);
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       cCurrentModuleObject,
                                       thisPump.Name,
                                       thisInput->cAlphaFieldNames(7),
                                       thisInput->cAlphaArgs(7)));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lAlphaFieldBlanks(8)) {
            thisPump.powerSizingMethod =
                static_cast<PowerSizingMethod>(getEnumValue(powerSizingMethodNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(8))));
            if (thisPump.powerSizingMethod == PowerSizingMethod::Invalid) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", sizing method type entered is invalid.  Use one of the key choice entries.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       thisPump.Name));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lNumericFieldBlanks(13)) {
            thisPump.powerPerFlowScalingFactor = thisInput->rNumericArgs(13);
        }

        if (!thisInput->lNumericFieldBlanks(14)) {
            thisPump.powerPerFlowPerPressureScalingFactor = thisInput->rNumericArgs(14);
        }

        if (NumAlphas > 8) {
            thisPump.EndUseSubcategoryName = thisInput->cAlphaArgs(9);
        } else {
            thisPump.EndUseSubcategoryName = "General";
        }

        thisPump.Energy = 0.0;
        thisPump.Power = 0.0;
    }

    cCurrentModuleObject = pumpTypeIDFNames[static_cast<int>(PumpType::Bank_ConSpeed)];
    for (int NumConstPumpBankSimple = 1; NumConstPumpBankSimple <= NumPumpBankSimpleConst; ++NumConstPumpBankSimple) {
        PumpNum = NumConstPumpBankSimple + NumVarSpeedPumps + NumConstSpeedPumps + NumCondensatePumps + NumPumpBankSimpleVar;
        auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 NumConstPumpBankSimple,
                                                                 thisInput->cAlphaArgs,
                                                                 NumAlphas,
                                                                 thisInput->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 thisInput->lNumericFieldBlanks,
                                                                 thisInput->lAlphaFieldBlanks,
                                                                 thisInput->cAlphaFieldNames,
                                                                 thisInput->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, thisInput->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPumps->PumpUniqueNames, thisInput->cAlphaArgs(1), cCurrentModuleObject, thisInput->cAlphaFieldNames(1), ErrorsFound);
        thisPump.Name = thisInput->cAlphaArgs(1);
        thisPump.pumpType = PumpType::Bank_ConSpeed; //'HeaderedPumps:ConstantSpeed'
        thisPump.TypeOf_Num = DataPlant::PlantEquipmentType::PumpBankConstantSpeed;

        thisPump.InletNodeNum = GetOnlySingleNode(state,
                                                  thisInput->cAlphaArgs(2),
                                                  ErrorsFound,
                                                  DataLoopNode::ConnectionObjectType::HeaderedPumpsConstantSpeed,
                                                  thisPump.Name,
                                                  DataLoopNode::NodeFluidType::Water,
                                                  DataLoopNode::ConnectionType::Inlet,
                                                  NodeInputManager::CompFluidStream::Primary,
                                                  ObjectIsNotParent);

        thisPump.OutletNodeNum = GetOnlySingleNode(state,
                                                   thisInput->cAlphaArgs(3),
                                                   ErrorsFound,
                                                   DataLoopNode::ConnectionObjectType::HeaderedPumpsConstantSpeed,
                                                   thisPump.Name,
                                                   DataLoopNode::NodeFluidType::Water,
                                                   DataLoopNode::ConnectionType::Outlet,
                                                   NodeInputManager::CompFluidStream::Primary,
                                                   ObjectIsNotParent);
        TestCompSet(state, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaArgs(2), thisInput->cAlphaArgs(3), "Water Nodes");

        if (Util::SameString(thisInput->cAlphaArgs(4), "Optimal")) {
            thisPump.SequencingScheme = PumpBankControlSeq::OptimalScheme;
        } else if (Util::SameString(thisInput->cAlphaArgs(4), "Sequential")) {
            thisPump.SequencingScheme = PumpBankControlSeq::SequentialScheme;
        } else {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Invalid {}", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaFieldNames(4)));
            ShowContinueError(
                state,
                format("Entered Value=[{}]. {} has been set to Sequential for this pump.", thisInput->cAlphaArgs(4), thisInput->cAlphaFieldNames(4)));
            thisPump.SequencingScheme = PumpBankControlSeq::SequentialScheme;
        }

        thisPump.PumpControl = static_cast<PumpControlType>(getEnumValue(pumpCtrlTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(5))));

        if (thisPump.PumpControl == PumpControlType::Invalid) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Invalid {}", RoutineName, cCurrentModuleObject, thisPump.Name, thisInput->cAlphaFieldNames(5)));
            ShowContinueError(
                state,
                format("Entered Value=[{}]. {} has been set to Continuous for this pump.", thisInput->cAlphaArgs(5), thisInput->cAlphaFieldNames(5)));
            thisPump.PumpControl = PumpControlType::Continuous;
        }

        // Input the optional schedule for the pump
        if (thisInput->lAlphaFieldBlanks(6)) {
            thisPump.flowRateSched = nullptr;
        } else if ((thisPump.flowRateSched = Sched::GetSchedule(state, thisInput->cAlphaArgs(6))) == nullptr) {
            ShowWarningItemNotFound(state, eoh, thisInput->cAlphaFieldNames(6), thisInput->cAlphaArgs(6), "");
        }

        thisPump.NomVolFlowRate = thisInput->rNumericArgs(1);
        if (thisPump.NomVolFlowRate == AutoSize) {
            thisPump.NomVolFlowRateWasAutoSized = true;
        }
        thisPump.NumPumpsInBank = thisInput->rNumericArgs(2);
        thisPump.NomPumpHead = thisInput->rNumericArgs(3);
        thisPump.NomPowerUse = thisInput->rNumericArgs(4);
        if (thisPump.NomPowerUse == AutoSize) {
            thisPump.NomPowerUseWasAutoSized = true;
        }
        thisPump.MotorEffic = thisInput->rNumericArgs(5);
        thisPump.FracMotorLossToFluid = thisInput->rNumericArgs(6);
        thisPump.PartLoadCoef[0] = 1.0;
        thisPump.PartLoadCoef[1] = 0.0;
        thisPump.PartLoadCoef[2] = 0.0;
        thisPump.PartLoadCoef[3] = 0.0;

        if (!thisInput->lAlphaFieldBlanks(7)) { // zone named for pump skin losses
            thisPump.ZoneNum = Util::FindItemInList(thisInput->cAlphaArgs(7), state.dataHeatBal->Zone);
            if (thisPump.ZoneNum > 0) {
                thisPump.HeatLossesToZone = true;
                if (!thisInput->lNumericFieldBlanks(7)) {
                    thisPump.SkinLossRadFraction = thisInput->rNumericArgs(7);
                }
            } else {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                       cCurrentModuleObject,
                                       thisPump.Name,
                                       thisInput->cAlphaFieldNames(7),
                                       thisInput->cAlphaArgs(7)));
                ErrorsFound = true;
            }
        }
        if (!thisInput->lAlphaFieldBlanks(8)) {
            thisPump.powerSizingMethod =
                static_cast<PowerSizingMethod>(getEnumValue(powerSizingMethodNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(8))));
            if (thisPump.powerSizingMethod == PowerSizingMethod::Invalid) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", sizing method type entered is invalid.  Use one of the key choice entries.",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       thisPump.Name));
                ErrorsFound = true;
            }
        }

        if (!thisInput->lNumericFieldBlanks(8)) {
            thisPump.powerPerFlowScalingFactor = thisInput->rNumericArgs(8);
        }

        if (!thisInput->lNumericFieldBlanks(9)) {
            thisPump.powerPerFlowPerPressureScalingFactor = thisInput->rNumericArgs(9);
        }

        if (NumAlphas > 8) {
            thisPump.EndUseSubcategoryName = thisInput->cAlphaArgs(9);
        } else {
            thisPump.EndUseSubcategoryName = "General";
        }

        thisPump.MinVolFlowRate = 0.0;
        thisPump.Energy = 0.0;
        thisPump.Power = 0.0;
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in getting Pump input");
    }

    for (PumpNum = 1; PumpNum <= state.dataPumps->NumPumps; ++PumpNum) { // CurrentModuleObject='Pumps'
        auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
        auto &thisPumpRep = state.dataPumps->PumpEquipReport(PumpNum);
        switch (thisPump.pumpType) {
        case PumpType::VarSpeed:
        case PumpType::ConSpeed:
        case PumpType::Cond: {

            SetupOutputVariable(state,
                                "Pump Electricity Energy",
                                Constant::Units::J,
                                thisPump.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisPump.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Pumps,
                                thisPump.EndUseSubcategoryName);
            SetupOutputVariable(state,
                                "Pump Electricity Rate",
                                Constant::Units::W,
                                thisPump.Power,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Shaft Power",
                                Constant::Units::W,
                                thisPumpRep.ShaftPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Fluid Heat Gain Rate",
                                Constant::Units::W,
                                thisPumpRep.PumpHeattoFluid,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Fluid Heat Gain Energy",
                                Constant::Units::J,
                                thisPumpRep.PumpHeattoFluidEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Outlet Temperature",
                                Constant::Units::C,
                                thisPumpRep.OutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisPumpRep.PumpMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
        } break;

        case PumpType::Bank_VarSpeed:
        case PumpType::Bank_ConSpeed: { // CurrentModuleObject='HeaderedPumps'

            SetupOutputVariable(state,
                                "Pump Electricity Energy",
                                Constant::Units::J,
                                thisPump.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisPump.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Pumps,
                                thisPump.EndUseSubcategoryName);
            SetupOutputVariable(state,
                                "Pump Electricity Rate",
                                Constant::Units::W,
                                thisPump.Power,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Shaft Power",
                                Constant::Units::W,
                                thisPumpRep.ShaftPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Fluid Heat Gain Rate",
                                Constant::Units::W,
                                thisPumpRep.PumpHeattoFluid,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Fluid Heat Gain Energy",
                                Constant::Units::J,
                                thisPumpRep.PumpHeattoFluidEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Outlet Temperature",
                                Constant::Units::C,
                                thisPumpRep.OutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisPumpRep.PumpMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Operating Pumps Count",
                                Constant::Units::None,
                                thisPumpRep.NumPumpsOperating,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
        } break;
        default: {
            assert(false);
        } break;
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSInternalVariable(state, "Pump Maximum Mass Flow Rate", thisPump.Name, "[kg/s]", thisPump.MassFlowRateMax);
            SetupEMSActuator(
                state, "Pump", thisPump.Name, "Pump Mass Flow Rate", "[kg/s]", thisPump.EMSMassFlowOverrideOn, thisPump.EMSMassFlowValue);
            SetupEMSActuator(
                state, "Pump", thisPump.Name, "Pump Pressure Rise", "[Pa]", thisPump.EMSPressureOverrideOn, thisPump.EMSPressureOverrideValue);
        }

        if (thisPump.HeatLossesToZone) {
            // setup skin loss output vars
            SetupOutputVariable(state,
                                "Pump Zone Total Heating Rate",
                                Constant::Units::W,
                                thisPumpRep.ZoneTotalGainRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Zone Total Heating Energy",
                                Constant::Units::J,
                                thisPumpRep.ZoneTotalGainEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Zone Convective Heating Rate",
                                Constant::Units::W,
                                thisPumpRep.ZoneConvGainRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);
            SetupOutputVariable(state,
                                "Pump Zone Radiative Heating Rate",
                                Constant::Units::W,
                                thisPumpRep.ZoneRadGainRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPump.Name);

            // setup internal gains
            switch (thisPump.pumpType) {
            case PumpType::VarSpeed: {
                SetupZoneInternalGain(state,
                                      thisPump.ZoneNum,
                                      thisPump.Name,
                                      DataHeatBalance::IntGainType::Pump_VarSpeed,
                                      &thisPumpRep.ZoneConvGainRate,
                                      nullptr,
                                      &thisPumpRep.ZoneRadGainRate);
            } break;
            case PumpType::ConSpeed: {
                SetupZoneInternalGain(state,
                                      thisPump.ZoneNum,
                                      thisPump.Name,
                                      DataHeatBalance::IntGainType::Pump_ConSpeed,
                                      &thisPumpRep.ZoneConvGainRate,
                                      nullptr,
                                      &thisPumpRep.ZoneRadGainRate);
            } break;
            case PumpType::Cond: {
                SetupZoneInternalGain(state,
                                      thisPump.ZoneNum,
                                      thisPump.Name,
                                      DataHeatBalance::IntGainType::Pump_Cond,
                                      &thisPumpRep.ZoneConvGainRate,
                                      nullptr,
                                      &thisPumpRep.ZoneRadGainRate);
            } break;
            case PumpType::Bank_VarSpeed: {
                SetupZoneInternalGain(state,
                                      thisPump.ZoneNum,
                                      thisPump.Name,
                                      DataHeatBalance::IntGainType::PumpBank_VarSpeed,
                                      &thisPumpRep.ZoneConvGainRate,
                                      nullptr,
                                      &thisPumpRep.ZoneRadGainRate);
            } break;
            case PumpType::Bank_ConSpeed: {
                SetupZoneInternalGain(state,
                                      thisPump.ZoneNum,
                                      thisPump.Name,
                                      DataHeatBalance::IntGainType::PumpBank_ConSpeed,
                                      &thisPumpRep.ZoneConvGainRate,
                                      nullptr,
                                      &thisPumpRep.ZoneRadGainRate);
            } break;
            default:
                break;
            }
        }
    }
}

void InitializePumps(EnergyPlusData &state, int const PumpNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:        Edwin Lee
    //       DATE WRITTEN:  August 2010
    //       MODIFIED       Based on the INIT section of InitSimVars, credits here:
    //                        Author:
    //                          Oct 1998 Dan Fisher
    //                        Modifications:
    //                          Jul 2001 Richard Liesen
    //                          July 2001, Rick Strand (implemented new pump controls)
    //                          May 2009, Brent Griffith (added EMS override capability)
    //                          Nov 2010, Brent Griffith (call InitComponentNodes, generalize fluid props)
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does one-time and begin-envrn inits for the pump

    // Using/Aliasing

    using PlantUtilities::InitComponentNodes;
    using PlantUtilities::ScanPlantLoopsForObject;

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr StartTemp(100.0); // Standard Temperature across code to calculated Steam density
    Real64 constexpr ZeroPowerTol(0.0000001);
    static constexpr std::string_view RoutineName("PlantPumps::InitializePumps ");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TotalEffic;
    Real64 SteamDensity; // Density of working fluid
    Real64 TempWaterDensity;
    Real64 mdotMax; // local fluid mass flow rate maximum
    Real64 mdotMin; // local fluid mass flow rate minimum
    DataPlant::LoopSideLocation lsnum;

    // Set some variables for convenience
    auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
    int InletNode = thisPump.InletNodeNum;
    int OutletNode = thisPump.OutletNodeNum;

    // One time inits
    if (thisPump.PumpOneTimeFlag) {

        bool errFlag = false;
        ScanPlantLoopsForObject(state, thisPump.Name, thisPump.TypeOf_Num, thisPump.plantLoc, errFlag, _, _, _, _, _);
        int plloopnum = thisPump.plantLoc.loopNum;
        lsnum = thisPump.plantLoc.loopSideNum;
        int brnum = thisPump.plantLoc.branchNum;
        int cpnum = thisPump.plantLoc.compNum;
        if (plloopnum > 0 && lsnum != DataPlant::LoopSideLocation::Invalid && brnum > 0 && cpnum > 0) {
            auto &thisPumpLoc = state.dataPlnt->PlantLoop(plloopnum).LoopSide(lsnum).Branch(brnum);
            if (thisPumpLoc.Comp(cpnum).NodeNumIn != InletNode || thisPumpLoc.Comp(cpnum).NodeNumOut != OutletNode) {
                ShowSevereError(
                    state,
                    format("InitializePumps: {}=\"{}\", non-matching nodes.", pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)], thisPump.Name));
                ShowContinueError(state, format("...in Branch={}, Component referenced with:", thisPumpLoc.Name));
                ShowContinueError(state, format("...Inlet Node={}", state.dataLoopNodes->NodeID(thisPumpLoc.Comp(cpnum).NodeNumIn)));
                ShowContinueError(state, format("...Outlet Node={}", state.dataLoopNodes->NodeID(thisPumpLoc.Comp(cpnum).NodeNumOut)));
                ShowContinueError(state, format("...Pump Inlet Node={}", state.dataLoopNodes->NodeID(InletNode)));
                ShowContinueError(state, format("...Pump Outlet Node={}", state.dataLoopNodes->NodeID(OutletNode)));
                errFlag = true;
            }
        } else { // CR9292
            ShowSevereError(
                state,
                format("InitializePumps: {}=\"{}\", component missing.", pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)], thisPump.Name));
            errFlag = true; // should have received warning/severe earlier, will reiterate
        }

        if (errFlag) {
            ShowFatalError(state, "InitializePumps: Program terminated due to previous condition(s).");
        }
        DataPlant::CompData::getPlantComponent(state, thisPump.plantLoc).CompNum = PumpNum;

        SizePump(state, PumpNum);

        // calculate the efficiency for each pump
        // by calculating the efficiency for each pump being simulated.  The calculation
        // is based on the PMPSIM code in the ASHRAE Secondary Toolkit
        if (thisPump.NomPowerUse > ZeroPowerTol && thisPump.MotorEffic > ZeroPowerTol) {
            TotalEffic = thisPump.NomVolFlowRate * thisPump.NomPumpHead / thisPump.NomPowerUse;
            thisPump.PumpEffic = TotalEffic / thisPump.MotorEffic;
            if (thisPump.PumpEffic < 0.50) {
                ShowWarningError(state,
                                 format("Check input. Calculated Pump Efficiency={:.2R}% which is less than 50%, for pump={}",
                                        thisPump.PumpEffic * 100.0,
                                        thisPump.Name));
                ShowContinueError(state,
                                  format("Calculated Pump_Efficiency % =Total_Efficiency % [{:.1R}] / Motor_Efficiency % [{:.1R}]",
                                         TotalEffic * 100.0,
                                         thisPump.MotorEffic * 100.0));
                ShowContinueError(
                    state,
                    format("Total_Efficiency % =(Rated_Volume_Flow_Rate [{:.3R}] * Rated_Pump_Head [{:.1R}] / Rated_Power_Use [{:.1R}]) * 100.",
                           thisPump.NomVolFlowRate,
                           thisPump.NomPumpHead,
                           thisPump.NomPowerUse));
            } else if ((thisPump.PumpEffic > 0.95) && (thisPump.PumpEffic <= 1.0)) {
                ShowWarningError(state,
                                 format("Check input.  Calculated Pump Efficiency={:.2R}% is approaching 100%, for pump={}",
                                        thisPump.PumpEffic * 100.0,
                                        thisPump.Name));
                ShowContinueError(state,
                                  format("Calculated Pump_Efficiency % =Total_Efficiency % [{:.1R}] / Motor_Efficiency % [{:.1R}]",
                                         TotalEffic * 100.0,
                                         thisPump.MotorEffic * 100.0));
                ShowContinueError(
                    state,
                    format("Total_Efficiency % =(Rated_Volume_Flow_Rate [{:.3R}] * Rated_Pump_Head [{:.1R}] / Rated_Power_Use [{:.1R}]) * 100.",
                           thisPump.NomVolFlowRate,
                           thisPump.NomPumpHead,
                           thisPump.NomPowerUse));
            } else if (thisPump.PumpEffic > 1.0) {
                ShowSevereError(state,
                                format("Check input.  Calculated Pump Efficiency={:.3R}% which is bigger than 100%, for pump={}",
                                       thisPump.PumpEffic * 100.0,
                                       thisPump.Name));
                ShowContinueError(state,
                                  format("Calculated Pump_Efficiency % =Total_Efficiency % [{:.1R}] / Motor_Efficiency % [{:.1R}]",
                                         TotalEffic * 100.0,
                                         thisPump.MotorEffic * 100.0));
                ShowContinueError(
                    state,
                    format("Total_Efficiency % =(Rated_Volume_Flow_Rate [{:.3R}] * Rated_Pump_Head [{:.1R}] / Rated_Power_Use [{:.1R}]) * 100.",
                           thisPump.NomVolFlowRate,
                           thisPump.NomPumpHead,
                           thisPump.NomPowerUse));
                ShowFatalError(state, "Errors found in Pump input");
            }
        } else {
            ShowWarningError(state, format("Check input. Pump nominal power or motor efficiency is set to 0, for pump={}", thisPump.Name));
        }

        if (thisPump.NomVolFlowRate <= SmallWaterVolFlow) {
            ShowWarningError(state, format("Check input. Pump nominal flow rate is set or calculated = 0, for pump={}", thisPump.Name));
        }

        if (thisPump.PumpControl == PumpControlType::Continuous) {
            // reset flow priority appropriately (default was for Intermittent)
            DataPlant::CompData::getPlantComponent(state, thisPump.plantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
        }

        thisPump.PumpOneTimeFlag = false;
    }

    // HVAC Sizing Simulation resizing calls if needed
    if (state.dataGlobal->RedoSizesHVACSimulation && !state.dataPlnt->PlantReSizingCompleted) {
        SizePump(state, PumpNum);
    }

    // Begin environment inits
    if (thisPump.PumpInitFlag && state.dataGlobal->BeginEnvrnFlag) {
        if (thisPump.pumpType == PumpType::Cond) {
            TempWaterDensity = Fluid::GetWater(state)->getDensity(state, Constant::InitConvTemp, RoutineName);
            SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, StartTemp, 1.0, RoutineName);
            thisPump.NomVolFlowRate = (thisPump.NomSteamVolFlowRate * SteamDensity) / TempWaterDensity;

            // set the maximum flow rate on the outlet node
            mdotMax = thisPump.NomSteamVolFlowRate * SteamDensity;
            // mdotMin = PumpEquip(PumpNum)%MinVolFlowRate      * SteamDensity
            // On a pump the 'hardware min' (MassFlowRateMin) must be defined as zero and not
            // confused with the desired pump operating scheme or the user specified
            //'minimum flow rate'.  The user specified 'minimum flow rate' determines the minimum
            // flow rate under normal operating conditions.  For cases when 'MaxAvail' on the pump
            // inlet node actually less than the 'minimum flow rate' specified by the user, than a
            // loop shutdown must  be triggered.
            mdotMin = 0.0;
            InitComponentNodes(state, mdotMin, mdotMax, InletNode, OutletNode);
            thisPump.MassFlowRateMax = mdotMax;
            thisPump.MassFlowRateMin = thisPump.MinVolFlowRate * SteamDensity;

        } else {
            auto &thisPumpPlant = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum);
            TempWaterDensity = thisPumpPlant.glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
            mdotMax = thisPump.NomVolFlowRate * TempWaterDensity;
            // mdotMin = PumpEquip(PumpNum)%MinVolFlowRate * TempWaterDensity
            // see note above
            mdotMin = 0.0;
            InitComponentNodes(state, mdotMin, mdotMax, InletNode, OutletNode);
            thisPump.MassFlowRateMax = mdotMax;
            thisPump.MassFlowRateMin = thisPump.MinVolFlowRate * TempWaterDensity;
        }
        // zero out report variables
        thisPump.Energy = 0.0;
        thisPump.Power = 0.0;
        new (&(state.dataPumps->PumpEquipReport(PumpNum))) ReportVars();
        thisPump.PumpInitFlag = false;
    }

    // Reset the local environment flag for the next environment
    if (!state.dataGlobal->BeginEnvrnFlag) {
        thisPump.PumpInitFlag = true;
    }

    // zero out module level working variables
    auto const &daPumps = state.dataPumps;
    daPumps->PumpMassFlowRate = 0.0;
    daPumps->PumpHeattoFluid = 0.0;
    daPumps->Power = 0.0;
    daPumps->ShaftPower = 0.0;
}

//*************************************************************************!

//*************************************************************************!

void SetupPumpMinMaxFlows(EnergyPlusData &state, int const LoopNum, int const PumpNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:        Edwin Lee
    //       DATE WRITTEN:  Aug 2010
    //       MODIFIED       Based on the Flow control portion of what was previously Pumps::InitSimVars, by:
    //                        Dan Fisher October 1998
    //                        Richard Liesen July 2001
    //                        July 2001, Rick Strand (implemented new pump controls)
    //                        May 2009, Brent Griffith (added EMS override capability)
    //                        B. Griffith, Nov 2011 Pump control: Intermittent vs Continuous
    //       RE-ENGINEERED

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the pump minAvail and maxAvail flow rates, and assigns them to the
    //  outlet min/max avail according to inlet min/max constraints and zero flow request
    // The loop solver then uses this information to set up the flow bounds for the loop side
    //  for the current iteration.

    // METHODOLOGY EMPLOYED:
    //  Design flow rate and user specified minimum flow rate is compared in the inlet node
    //  min/maxavail.  The pump output is appropriately constrained.
    //  Design flow is rated flow times schedule fraction
    //  Inlet node max will represent the rated flow rate according to pump init routines.
    //  These values are bounded by hardware min constraints on the inlet node, which is likely zero.
    //  These values are also bounded by EMS overridable limit of max flow rate.

    // Using/Aliasing
    using PlantPressureSystem::ResolveLoopFlowVsPressure;
    using PlantUtilities::BoundValueToWithinTwoValues;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int InletNode;  // pump inlet node number
    int OutletNode; // pump outlet node number
    Real64 InletNodeMax;
    Real64 InletNodeMin;
    Real64 PumpMassFlowRateMax; // max allowable flow rate at the pump
    Real64 PumpMassFlowRateMin; // min allowable flow rate at the pump
    Real64 PumpSchedFraction;
    Real64 PumpOverridableMaxLimit;
    Real64 PumpMassFlowRateMinLimit;
    Real64 PumpSchedRPM; // Pump RPM Optional Input

    auto &thisPump = state.dataPumps->PumpEquip(PumpNum);

    // Inlet/Outlet Node Numbers
    InletNode = thisPump.InletNodeNum;
    OutletNode = thisPump.OutletNodeNum;
    auto &thisInNode = state.dataLoopNodes->Node(InletNode);
    auto &thisOutNode = state.dataLoopNodes->Node(OutletNode);

    // Inlet node Min/MaxAvail
    InletNodeMax = thisInNode.MassFlowRateMaxAvail;
    InletNodeMin = thisInNode.MassFlowRateMinAvail;

    // Retrieve the pump speed fraction from the pump schedule (if any)
    PumpSchedFraction = (thisPump.flowRateSched != nullptr) ? std::clamp(thisPump.flowRateSched->getCurrentVal(), 0.0, 1.0) : 1.0;

    // User specified min/max mass flow rates for pump
    PumpOverridableMaxLimit = thisPump.MassFlowRateMax;

    // override the user specified min to allow pump to turn off when no flow is required.
    if (thisPump.LoopSolverOverwriteFlag) {
        PumpMassFlowRateMinLimit = 0.0;
    } else {
        PumpMassFlowRateMinLimit = thisPump.MassFlowRateMin;
    }

    // The pump outlet node Min/MaxAvail
    PumpMassFlowRateMin = max(InletNodeMin, PumpMassFlowRateMinLimit);
    PumpMassFlowRateMax = min(InletNodeMax, PumpOverridableMaxLimit * PumpSchedFraction);

    // Check for conflicts (MaxAvail < MinAvail)
    if (PumpMassFlowRateMin > PumpMassFlowRateMax) { // the demand side wants to operate outside of the pump range
        // shut the pump (and the loop) down
        PumpMassFlowRateMin = 0.0;
        PumpMassFlowRateMax = 0.0;
        // Let the user know that his input file is overconstrained
    }

    auto const &thisPumpPlant = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum);

    switch (thisPump.pumpType) {
    case PumpType::VarSpeed: {
        if (thisPump.HasVFD) {
            switch (thisPump.VFD.VFDControlType) {
            case ControlTypeVFD::VFDManual: {
                // Evaluate the schedule if it exists and put the fraction into a local variable
                PumpSchedRPM = thisPump.VFD.manualRPMSched->getCurrentVal();
                // Convert the RPM to rot/sec for calculation routine
                thisPump.RotSpeed = PumpSchedRPM / 60.0;
                // Resolve the new mass flow rate based on current pressure characteristics
                if (thisPumpPlant.UsePressureForPumpCalcs && thisPumpPlant.PressureSimType == DataPlant::PressSimType::FlowCorrection &&
                    thisPumpPlant.PressureDrop > 0.0) {

                    state.dataPumps->PumpMassFlowRate = ResolveLoopFlowVsPressure(state,
                                                                                  thisPump.plantLoc.loopNum,
                                                                                  state.dataLoopNodes->Node(thisPump.InletNodeNum).MassFlowRate,
                                                                                  thisPump.PressureCurve_Index,
                                                                                  thisPump.RotSpeed,
                                                                                  thisPump.ImpellerDiameter,
                                                                                  thisPump.MinPhiValue,
                                                                                  thisPump.MaxPhiValue);

                    PumpMassFlowRateMax = state.dataPumps->PumpMassFlowRate;
                    PumpMassFlowRateMin = state.dataPumps->PumpMassFlowRate;
                }
            } break;
            case ControlTypeVFD::VFDAutomatic: {
                if (thisPumpPlant.UsePressureForPumpCalcs && thisPumpPlant.PressureSimType == DataPlant::PressSimType::FlowCorrection &&
                    thisPumpPlant.PressureDrop > 0.0) {

                    GetRequiredMassFlowRate(state,
                                            LoopNum,
                                            PumpNum,
                                            state.dataLoopNodes->Node(thisPump.InletNodeNum).MassFlowRate,
                                            state.dataPumps->PumpMassFlowRate,
                                            PumpMassFlowRateMin,
                                            PumpMassFlowRateMax);
                }
            } break;
            default:
                break;
            } // VFDControlType
        }

        if (thisPump.PumpControl == PumpControlType::Continuous) {
            thisInNode.MassFlowRateRequest = PumpMassFlowRateMin;
        }
    } break;
    case PumpType::ConSpeed: {
        if (thisPump.PumpControl == PumpControlType::Continuous) {
            PumpMassFlowRateMin = PumpMassFlowRateMax;
            thisInNode.MassFlowRateRequest = PumpMassFlowRateMin;
        }

        // Override (lock down flow) for pressure drop if applicable
        if (thisPump.plantLoc.loopNum > 0) {
            if (thisPumpPlant.UsePressureForPumpCalcs && thisPumpPlant.PressureSimType == DataPlant::PressSimType::FlowCorrection &&
                thisPumpPlant.PressureDrop > 0.0) {
                state.dataPumps->PumpMassFlowRate = ResolveLoopFlowVsPressure(state,
                                                                              thisPump.plantLoc.loopNum,
                                                                              state.dataLoopNodes->Node(thisPump.InletNodeNum).MassFlowRate,
                                                                              thisPump.PressureCurve_Index,
                                                                              thisPump.RotSpeed,
                                                                              thisPump.ImpellerDiameter,
                                                                              thisPump.MinPhiValue,
                                                                              thisPump.MaxPhiValue);
                PumpMassFlowRateMax = state.dataPumps->PumpMassFlowRate;
                PumpMassFlowRateMin = state.dataPumps->PumpMassFlowRate;
            }
        }
    } break;
    default:
        break;
    }

    // Override pump operation based on System Availability Managers, should be done elsewhere?  I suppose this should be OK though
    if (allocated(state.dataAvail->PlantAvailMgr)) {
        if (state.dataAvail->PlantAvailMgr(LoopNum).availStatus == Avail::Status::ForceOff) {
            PumpMassFlowRateMax = 0.0;
            PumpMassFlowRateMin = 0.0;
        }
    }

    // Check if EMS is overriding flow
    if (thisPump.EMSMassFlowOverrideOn) {
        PumpMassFlowRateMax = thisPump.EMSMassFlowValue;
        PumpMassFlowRateMin = thisPump.EMSMassFlowValue;
    }

    // Update outlet node to allow loop solver to get data
    // could avoid this by passing data in/out to avoid putting things on nodes
    thisOutNode.MassFlowRateMinAvail = PumpMassFlowRateMin;
    thisOutNode.MassFlowRateMaxAvail = PumpMassFlowRateMax;
}

void CalcPumps(EnergyPlusData &state, int const PumpNum, Real64 const FlowRequest, bool &PumpRunning)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   Sept. 1998
    //       MODIFIED       July 2001, Rick Strand
    //       RE-ENGINEERED  Sept 2010, Edwin Lee

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutines simulates a pump following
    // the methodology oulined in ASHRAE's secondary toolkit.

    // METHODOLOGY EMPLOYED:
    // Calculates power and updates other pump things.

    // REFERENCES:
    // HVAC 2 Toolkit:  A Toolkit for Secondary HVAC System
    // Energy Calculations, ASHRAE, 1993, pp2-10 to 2-15

    // Using/Aliasing
    using PlantUtilities::SetComponentFlowRate;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("PlantPumps:CalcPumps: ");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int InletNode;
    int OutletNode;
    Real64 LoopDensity;
    Real64 VolFlowRate = 0.0;
    Real64 PartLoadRatio;
    Real64 FracFullLoadPower;
    Real64 FullLoadVolFlowRate;
    Real64 PartLoadVolFlowRate;
    Real64 FullLoadPower;
    Real64 FullLoadPowerRatio;
    Real64 TotalEffic;
    PumpType pumpType;
    Real64 RotSpeed_Min;
    Real64 RotSpeed_Max;
    Real64 PumpActualRPMValueOne;
    Real64 PumpActualRPMValueTwo;

    auto &daPumps = state.dataPumps;
    auto &thisPump = state.dataPumps->PumpEquip(PumpNum);

    InletNode = thisPump.InletNodeNum;
    OutletNode = thisPump.OutletNodeNum;
    pumpType = thisPump.pumpType;

    auto &thisInNode = state.dataLoopNodes->Node(InletNode);
    auto &thisOutNode = state.dataLoopNodes->Node(OutletNode);

    //****************************!
    //** SETTING PUMP FLOW RATE **!
    //****************************!
    // So the loop solver always passes in the full loop side flow request to each pump called
    // The pump will try to use this value according to its inlet conditions via the SetComponentFlowRate routine.
    // If the loop solver is doing branch pumps, then individual parallel branch inlet nodes would have been previously
    // constrained, so even though we pass in a full flow request, each pump will "pull down" to the min/max avail.
    // Also, on flowlock == locked, we will just use the inlet node flow rate
    // The flow resolver can take care of argument resolution beyond that.
    // For a typical situation, the flow request should be within the values of min/max avail, so the pump will get this flow rate.
    if (FlowRequest > DataBranchAirLoopPlant::MassFlowTolerance) {
        daPumps->PumpMassFlowRate = FlowRequest;
    } else {
        daPumps->PumpMassFlowRate = 0.0;
    }

    // For variable speed branch pumps, with other components
    //  on the branch, we are not going to assign a request.
    // Other components on this branch will request flow for this branch

    //  ! If this is a variable speed pump
    if (thisPump.pumpType == PumpType::VarSpeed || thisPump.pumpType == PumpType::Bank_VarSpeed || thisPump.pumpType == PumpType::Cond) {
        if (DataPlant::CompData::getPlantComponent(state, thisPump.plantLoc).FlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
            daPumps->PumpMassFlowRate = 0.0;
        }
    }

    // bound flow request by pump max limit, the Flow Request is total loop flow and if this is a branch pump that is not appropriate
    daPumps->PumpMassFlowRate = min(thisPump.MassFlowRateMax, daPumps->PumpMassFlowRate);
    daPumps->PumpMassFlowRate = max(thisPump.MassFlowRateMin, daPumps->PumpMassFlowRate);

    SetComponentFlowRate(state, daPumps->PumpMassFlowRate, InletNode, OutletNode, thisPump.plantLoc);

    auto &thisPumpPlant = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum);

    // Get RPM value for reporting as output
    // RPM is calculated using pump affinity laws for rotation speed
    if (thisPumpPlant.UsePressureForPumpCalcs && thisPump.HasVFD) {
        RotSpeed_Min = thisPump.VFD.minRPMSched ? thisPump.VFD.minRPMSched->getCurrentVal() : 0.0;
        RotSpeed_Max = thisPump.VFD.maxRPMSched ? thisPump.VFD.maxRPMSched->getCurrentVal() : 0.0;
        if (thisPump.PumpMassFlowRateMaxRPM < DataBranchAirLoopPlant::MassFlowTolerance ||
            thisPump.PumpMassFlowRateMinRPM < DataBranchAirLoopPlant::MassFlowTolerance) {
            thisPump.VFD.PumpActualRPM = 0.0;
        } else {
            PumpActualRPMValueOne = (daPumps->PumpMassFlowRate / thisPump.PumpMassFlowRateMaxRPM) * RotSpeed_Max;
            PumpActualRPMValueTwo = (daPumps->PumpMassFlowRate / thisPump.PumpMassFlowRateMinRPM) * RotSpeed_Min;
            thisPump.VFD.PumpActualRPM = (PumpActualRPMValueOne + PumpActualRPMValueTwo) / 2;
        }
    }

    //****************************!
    //** DETERMINE IF PUMP IS ON *!
    //****************************!
    // Since we don't allow series pumping, if there is ANY flow rate for this pump, THIS PUMP is driving the flow!  Therefore...
    PumpRunning = (daPumps->PumpMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance);

    //****************************!
    //** UPDATE PUMP BANK USAGE **!
    //****************************!
    switch (thisPump.pumpType) {
    case PumpType::Bank_VarSpeed:
    case PumpType::Bank_ConSpeed: {
        // previously, pumps did whatever they wanted
        // because of this a constant speed pump bank could adjust the flow rate as-desired
        //  even if it was not allowed
        // since pumps now must behave nicely like all other components, the calculation of number
        //  of running pumps in a pump bank is the same for both bank types
        // the pumps are loaded sequentially, and the last pump can have full or non-full part load
        //  status...this is just how it works now.  The pump cannot *bump* up the flow on the loop
        //  to make sure the last running pump is fully loaded anymore for constant speed pumps...sorry
        if (daPumps->PumpMassFlowRate >= thisPump.MassFlowRateMax) {
            // running full on
            daPumps->NumPumpsRunning = thisPump.NumPumpsInBank;
        } else {
            // running at some sort of part load
            daPumps->NumPumpsRunning = CEILING((daPumps->PumpMassFlowRate / (thisPump.MassFlowRateMax) * thisPump.NumPumpsInBank));
            daPumps->NumPumpsRunning = min(daPumps->NumPumpsRunning, thisPump.NumPumpsInBank);
        }
    } break;
    default:
        break;
    }

    //****************************!
    //***** EXIT IF NO FLOW ******!
    //****************************!
    if (daPumps->PumpMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance) {
        thisOutNode.Temp = thisInNode.Temp;
        thisOutNode.Press = thisInNode.Press;
        thisOutNode.Quality = thisInNode.Quality;
        return;
    }

    // density used for volumetric flow calculations
    LoopDensity = thisPumpPlant.glycol->getDensity(state, thisInNode.Temp, RoutineName);

    //****************************!
    //***** CALCULATE POWER (1) **!
    //****************************!
    switch (pumpType) {
    case PumpType::ConSpeed:
    case PumpType::VarSpeed:
    case PumpType::Cond: {
        VolFlowRate = daPumps->PumpMassFlowRate / LoopDensity;
        PartLoadRatio = min(1.0, (VolFlowRate / thisPump.NomVolFlowRate));
        FracFullLoadPower = thisPump.PartLoadCoef[0] + thisPump.PartLoadCoef[1] * PartLoadRatio + thisPump.PartLoadCoef[2] * pow_2(PartLoadRatio) +
                            thisPump.PartLoadCoef[3] * pow_3(PartLoadRatio);
        daPumps->Power = FracFullLoadPower * thisPump.NomPowerUse;

    } break;
    case PumpType::Bank_ConSpeed:
    case PumpType::Bank_VarSpeed: {
        // now just assume the last one is (or is not) running at part load
        // if it is actually at full load, the calculations work out to PLR = 1
        // for the last pump, so all is OK
        daPumps->NumPumpsFullLoad = daPumps->NumPumpsRunning - 1;
        FullLoadVolFlowRate = thisPump.NomVolFlowRate / thisPump.NumPumpsInBank;
        PartLoadVolFlowRate = daPumps->PumpMassFlowRate / LoopDensity - FullLoadVolFlowRate * daPumps->NumPumpsFullLoad;
        FullLoadPower = thisPump.NomPowerUse / thisPump.NumPumpsInBank;
        FullLoadPowerRatio = thisPump.PartLoadCoef[0] + thisPump.PartLoadCoef[1] + thisPump.PartLoadCoef[2] + thisPump.PartLoadCoef[3];
        PartLoadRatio = min(1.0, (PartLoadVolFlowRate / FullLoadVolFlowRate));
        FracFullLoadPower = thisPump.PartLoadCoef[0] + thisPump.PartLoadCoef[1] * PartLoadRatio + thisPump.PartLoadCoef[2] * pow_2(PartLoadRatio) +
                            thisPump.PartLoadCoef[3] * pow_3(PartLoadRatio);
        daPumps->Power = (FullLoadPowerRatio * daPumps->NumPumpsFullLoad + FracFullLoadPower) * FullLoadPower;
        if (thisPump.EMSPressureOverrideOn) {
            VolFlowRate = PartLoadVolFlowRate;
        }
    } break;
    default: {
        assert(false);
    } break;
    }

    //****************************!
    //***** CALCULATE POWER (2) **!
    //****************************!
    if (daPumps->Power < 0.0) {
        if (thisPump.PowerErrIndex1 == 0) {
            ShowWarningMessage(
                state,
                format("{} Calculated Pump Power < 0, Type={}, Name={}", RoutineName, pumpTypeIDFNames[static_cast<int>(pumpType)], thisPump.Name));
            ShowContinueErrorTimeStamp(state, "");
            ShowContinueError(state, format("...PartLoadRatio=[{:.4R}], Fraction Full Load Power={:.4R}]", PartLoadRatio, FracFullLoadPower));
            ShowContinueError(state, "...Power is set to 0 for continuing the simulation.");
            ShowContinueError(state, "...Pump coefficients should be checked for producing this negative value.");
        }
        daPumps->Power = 0.0;
        ShowRecurringWarningErrorAtEnd(
            state,
            format("{} Calculated Pump Power < 0, {}, Name={}, PLR=", RoutineName, pumpTypeIDFNames[static_cast<int>(pumpType)], thisPump.Name),
            thisPump.PowerErrIndex1,
            PartLoadRatio,
            PartLoadRatio);
        ShowRecurringContinueErrorAtEnd(state, "...Fraction Full Load Power=", thisPump.PowerErrIndex2, FracFullLoadPower, FracFullLoadPower);
    }

    //****************************!
    //***** CALCULATE POWER (3) **!
    //****************************!
    // Now if we are doing pressure-based simulation, then we have a means to calculate power exactly based on current
    // simulation conditions (flow rate and pressure drop) along with knowledge about pump impeller and motor efficiencies
    // Thus we will override the power that was calculated based on nominal values with the corrected pressure-based power
    if (thisPump.plantLoc.loopNum > 0) {
        if (thisPumpPlant.UsePressureForPumpCalcs) {
            TotalEffic = thisPump.PumpEffic * thisPump.MotorEffic;
            // Efficiency errors are caught previously, but it doesn't hurt to add another catch before dividing by zero!!!
            if (TotalEffic == 0.0) {
                ShowSevereError(state,
                                format("{} Plant pressure simulation encountered a pump with zero efficiency: {}", RoutineName, thisPump.Name));
                ShowContinueError(state, "Check efficiency inputs for this pump component.");
                ShowFatalError(state, "Errors in plant calculation would result in divide-by-zero cause program termination.");
            }
            daPumps->Power = VolFlowRate * thisPumpPlant.PressureDrop / TotalEffic;
        }
    }

    // if user has specified a pressure value, then use it, same as for pressure-based simulation
    if (thisPump.EMSPressureOverrideOn) {
        TotalEffic = thisPump.PumpEffic * thisPump.MotorEffic;
        // Efficiency errors are caught previously, but it doesn't hurt to add another catch before dividing by zero!!!
        if (TotalEffic == 0.0) {
            ShowSevereError(state, format("{} Plant pump simulation encountered a pump with zero efficiency: {}", RoutineName, thisPump.Name));
            ShowContinueError(state, "Check efficiency inputs for this pump component.");
            ShowFatalError(state, "Errors in plant calculation would result in divide-by-zero cause program termination.");
        }
        daPumps->Power = VolFlowRate * thisPump.EMSPressureOverrideValue / TotalEffic;
    }

    //****************************!
    //***** CALCULATE POWER (4) **!
    //****************************!
    // This adds the pump heat based on User input for the pump
    // We assume that all of the heat ends up in the fluid eventually since this is a closed loop
    daPumps->ShaftPower = daPumps->Power * thisPump.MotorEffic;
    daPumps->PumpHeattoFluid = daPumps->ShaftPower + (daPumps->Power - daPumps->ShaftPower) * thisPump.FracMotorLossToFluid;

    //****************************!
    //***** UPDATE INFORMATION ***!
    //****************************!
    // Update data structure variables
    thisPump.Power = daPumps->Power;

    // Update outlet node conditions
    thisOutNode.Temp = thisInNode.Temp;
    thisOutNode.Press = thisInNode.Press;
    thisOutNode.Quality = thisInNode.Quality;
}

void SizePump(EnergyPlusData &state, int const PumpNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   December 2001
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing Pump Components for which flow rates have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the plant sizing array.

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr StartTemp(100.0); // Standard Temperature across code to calculated Steam density
    static constexpr std::string_view RoutineName("PlantPumps::InitSimVars ");
    static constexpr std::string_view RoutineNameSizePumps("SizePumps");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int PlantSizNum; // index of Plant Sizing array
    bool ErrorsFound;
    Real64 TotalEffic = 0.0; // pump total efficiency
    Real64 PumpSizFac;       // pump sizing factor
    Real64 SteamDensity;
    Real64 TempWaterDensity;
    int DummyWaterIndex(1);
    Real64 DesVolFlowRatePerBranch; // local temporary for split of branch pumps

    auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
    bool thisOkToReport = state.dataPlnt->PlantFinalSizesOkayToReport;

    // Calculate density at InitConvTemp once here, to remove RhoH2O calls littered throughout
    if (thisPump.plantLoc.loopNum > 0) {
        auto &thisPumpPlant = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum);
        TempWaterDensity = thisPumpPlant.glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
    } else {
        TempWaterDensity = Fluid::GetWater(state)->getDensity(state, Constant::InitConvTemp, RoutineName);
    }

    PlantSizNum = 0;
    PumpSizFac = 1.0;
    ErrorsFound = false;

    if (thisPump.plantLoc.loopNum > 0) {
        PlantSizNum = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum).PlantSizNum;
    }
    // use pump sizing factor stored in plant sizing data structure
    if (PlantSizNum > 0) {
        PumpSizFac = state.dataSize->PlantSizData(PlantSizNum).PlantSizFac;
    } else {
        // might be able to remove this next block
        if (thisPump.plantLoc.loopNum > 0) {
            for (DataPlant::LoopSideLocation Side : DataPlant::LoopSideKeys) {
                auto &thisPumpLoop = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum).LoopSide(Side);
                for (int BranchNum = 1; BranchNum <= thisPumpLoop.TotalBranches; ++BranchNum) {
                    auto &thisPumpBranch = thisPumpLoop.Branch(BranchNum);
                    for (int CompNum = 1; CompNum <= thisPumpBranch.TotalComponents; ++CompNum) {
                        auto const &thisPumpComp = thisPumpBranch.Comp(CompNum);
                        if (thisPump.InletNodeNum == thisPumpComp.NodeNumIn && thisPump.OutletNodeNum == thisPumpComp.NodeNumOut) {
                            if (thisPumpBranch.PumpSizFac > 0.0) {
                                PumpSizFac = thisPumpBranch.PumpSizFac;
                            } else {
                                PumpSizFac = 1.0;
                            }
                            goto SideLoop_exit;
                        }
                    }
                }
            }
        SideLoop_exit:;
        }
    }

    if (thisPump.NomVolFlowRateWasAutoSized) {

        if (PlantSizNum > 0) {
            auto &thisPumpPlant = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum);
            auto &thisPlantSize = state.dataSize->PlantSizData(PlantSizNum);
            if (thisPlantSize.DesVolFlowRate >= SmallWaterVolFlow) {
                if (!thisPumpPlant.LoopSide(thisPump.plantLoc.loopSideNum).BranchPumpsExist) {
                    // size pump to full flow of plant loop
                    if (thisPump.pumpType == PumpType::Cond) {
                        TempWaterDensity = Fluid::GetWater(state)->getDensity(state, Constant::InitConvTemp, RoutineName);
                        SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, StartTemp, 1.0, RoutineNameSizePumps);
                        thisPump.NomSteamVolFlowRate = thisPlantSize.DesVolFlowRate * PumpSizFac;
                        thisPump.NomVolFlowRate = thisPump.NomSteamVolFlowRate * SteamDensity / TempWaterDensity;
                    } else {
                        thisPump.NomVolFlowRate = thisPlantSize.DesVolFlowRate * PumpSizFac;
                    }
                } else {
                    // Distribute sizes evenly across all branch pumps
                    DesVolFlowRatePerBranch = thisPlantSize.DesVolFlowRate / thisPumpPlant.LoopSide(thisPump.plantLoc.loopSideNum).TotalPumps;
                    if (thisPump.pumpType == PumpType::Cond) {
                        TempWaterDensity = Fluid::GetWater(state)->getDensity(state, Constant::InitConvTemp, RoutineName);
                        SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, StartTemp, 1.0, RoutineNameSizePumps);
                        thisPump.NomSteamVolFlowRate = DesVolFlowRatePerBranch * PumpSizFac;
                        thisPump.NomVolFlowRate = thisPump.NomSteamVolFlowRate * SteamDensity / TempWaterDensity;
                    } else {
                        thisPump.NomVolFlowRate = DesVolFlowRatePerBranch * PumpSizFac;
                    }
                }

            } else {
                if (thisOkToReport) {
                    thisPump.NomVolFlowRate = 0.0;
                    ShowWarningError(
                        state,
                        format("SizePump: Calculated Pump Nominal Volume Flow Rate=[{:.2R}] is too small. Set to 0.0", thisPlantSize.DesVolFlowRate));
                    ShowContinueError(state, format("..occurs for Pump={}", thisPump.Name));
                }
            }
            if (thisOkToReport) {
                BaseSizer::reportSizerOutput(
                    state, pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)], thisPump.Name, "Design Flow Rate [m3/s]", thisPump.NomVolFlowRate);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)],
                                             thisPump.Name,
                                             "Initial Design Flow Rate [m3/s]",
                                             thisPump.NomVolFlowRate);
            }
        } else {
            if (thisOkToReport) {
                ShowSevereError(state, "Autosizing of plant loop pump flow rate requires a loop Sizing:Plant object");
                ShowContinueError(state, format("Occurs in plant pump object={}", thisPump.Name));
                ErrorsFound = true;
            }
        }
    }

    // Note that autocalculation of power is based on nominal volume flow, regardless of whether the flow was
    //  auto-sized or manually sized.  Thus, this must go after the flow sizing block above.
    if (thisPump.NomPowerUseWasAutoSized) {
        if (thisPump.NomVolFlowRate >= SmallWaterVolFlow) {
            switch (thisPump.powerSizingMethod) {

            case PowerSizingMethod::SizePowerPerFlow: {
                TotalEffic = thisPump.NomPumpHead / thisPump.powerPerFlowScalingFactor;
                break;
            }

            case PowerSizingMethod::SizePowerPerFlowPerPressure: {
                TotalEffic = (1 / thisPump.powerPerFlowPerPressureScalingFactor) * thisPump.MotorEffic;
                break;
            }
            default:
                assert(false);
            }

            thisPump.NomPowerUse = (thisPump.NomPumpHead * thisPump.NomVolFlowRate) / TotalEffic;
        } else {
            thisPump.NomPowerUse = 0.0;
        }
        if (thisOkToReport) {
            BaseSizer::reportSizerOutput(
                state, pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)], thisPump.Name, "Design Power Consumption [W]", thisPump.NomPowerUse);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state,
                                         pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)],
                                         thisPump.Name,
                                         "Initial Design Power Consumption [W]",
                                         thisPump.NomPowerUse);
        }
    }

    if (thisPump.minVolFlowRateWasAutosized) {
        thisPump.MinVolFlowRate = thisPump.NomVolFlowRate * thisPump.MinVolFlowRateFrac;
        if (thisOkToReport) {
            BaseSizer::reportSizerOutput(state,
                                         pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)],
                                         thisPump.Name,
                                         "Design Minimum Flow Rate [m3/s]",
                                         thisPump.MinVolFlowRate);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
            BaseSizer::reportSizerOutput(state,
                                         pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)],
                                         thisPump.Name,
                                         "Initial Design Minimum Flow Rate [m3/s]",
                                         thisPump.MinVolFlowRate);
        }
    }

    if (thisOkToReport) {
        PumpDataForTable(state, PumpNum);
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void ReportPumps(EnergyPlusData &state, int const PumpNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    October 1998
    //       MODIFIED         July 2001, Rick Strand (revision of pump module)
    //       RE-ENGINEERED    na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets the pump reporting variables.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int OutletNode;    // pump outlet node number
    PumpType PumpType; // Current pump type

    auto &thisPump = state.dataPumps->PumpEquip(PumpNum);
    auto &thisPumpRep = state.dataPumps->PumpEquipReport(PumpNum);

    PumpType = thisPump.pumpType;
    OutletNode = thisPump.OutletNodeNum;
    auto const &thisOutNode = state.dataLoopNodes->Node(OutletNode);
    auto const &daPumps = state.dataPumps;

    if (daPumps->PumpMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance) {
        new (&(state.dataPumps->PumpEquipReport(PumpNum))) ReportVars();
        thisPumpRep.OutletTemp = thisOutNode.Temp;
        thisPump.Power = 0.0;
        thisPump.Energy = 0.0;
    } else {
        thisPumpRep.PumpMassFlowRate = daPumps->PumpMassFlowRate;
        thisPumpRep.PumpHeattoFluid = daPumps->PumpHeattoFluid;
        thisPumpRep.OutletTemp = thisOutNode.Temp;
        thisPump.Power = daPumps->Power;
        thisPump.Energy = thisPump.Power * state.dataHVACGlobal->TimeStepSysSec;
        thisPumpRep.ShaftPower = daPumps->ShaftPower;
        thisPumpRep.PumpHeattoFluidEnergy = daPumps->PumpHeattoFluid * state.dataHVACGlobal->TimeStepSysSec;
        switch (PumpType) {
        case PumpType::ConSpeed:
        case PumpType::VarSpeed:
        case PumpType::Cond:
            thisPumpRep.NumPumpsOperating = 1;
            break;

        case PumpType::Bank_ConSpeed:
        case PumpType::Bank_VarSpeed:
            thisPumpRep.NumPumpsOperating = daPumps->NumPumpsRunning;
            break;
        default:
            assert(false);
            break;
        }
        thisPumpRep.ZoneTotalGainRate = daPumps->Power - daPumps->PumpHeattoFluid;
        thisPumpRep.ZoneTotalGainEnergy = thisPumpRep.ZoneTotalGainRate * state.dataHVACGlobal->TimeStepSysSec;
        thisPumpRep.ZoneConvGainRate = (1 - thisPump.SkinLossRadFraction) * thisPumpRep.ZoneTotalGainRate;
        thisPumpRep.ZoneRadGainRate = thisPump.SkinLossRadFraction * thisPumpRep.ZoneTotalGainRate;
    }
}

void PumpDataForTable(EnergyPlusData &state, int const NumPump)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Jason Glazer
    //       DATE WRITTEN:    September 2006
    //       MODIFIED         na
    //       RE-ENGINEERED    na

    // PURPOSE OF THIS SUBROUTINE:
    // Pull data together for predefined tables.

    // Using/Aliasing
    using namespace OutputReportPredefined;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string equipName;

    auto &thisPump = state.dataPumps->PumpEquip(NumPump);
    auto &thisReport = state.dataOutRptPredefined;

    equipName = thisPump.Name;
    PreDefTableEntry(state, thisReport->pdchPumpType, equipName, pumpTypeIDFNames[static_cast<int>(thisPump.pumpType)]);
    if (thisPump.PumpControl == PumpControlType::Continuous) {
        PreDefTableEntry(state, thisReport->pdchPumpControl, equipName, "Continuous");
    } else if (thisPump.PumpControl == PumpControlType::Intermittent) {
        PreDefTableEntry(state, thisReport->pdchPumpControl, equipName, "Intermittent");
    } else {
        PreDefTableEntry(state, thisReport->pdchPumpControl, equipName, "Unknown");
    }
    PreDefTableEntry(state, thisReport->pdchPumpHead, equipName, thisPump.NomPumpHead);
    PreDefTableEntry(state, thisReport->pdchPumpFlow, equipName, thisPump.NomVolFlowRate, 6);
    PreDefTableEntry(state, thisReport->pdchPumpPower, equipName, thisPump.NomPowerUse);
    if (thisPump.NomVolFlowRate != 0) {
        PreDefTableEntry(state, thisReport->pdchPumpPwrPerFlow, equipName, thisPump.NomPowerUse / thisPump.NomVolFlowRate);
    } else {
        PreDefTableEntry(state, thisReport->pdchPumpPwrPerFlow, equipName, "-");
    }
    PreDefTableEntry(state, thisReport->pdchPumpEndUse, equipName, thisPump.EndUseSubcategoryName);
    PreDefTableEntry(state, thisReport->pdchMotEff, equipName, thisPump.MotorEffic);
    // Std 229
    PreDefTableEntry(state, thisReport->pdchPumpAutosized, equipName, thisPump.NomVolFlowRateWasAutoSized ? "Yes" : "No");
    PreDefTableEntry(state,
                     thisReport->pdchPumpPlantloopName,
                     equipName,
                     thisPump.plantLoc.loopNum > 0 ? state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum).Name : "N/A");
    PreDefTableEntry(
        state,
        thisReport->pdchPumpPlantloopBranchName,
        equipName,
        thisPump.plantLoc.loopNum > 0
            ? state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum).LoopSide(thisPump.plantLoc.loopSideNum).Branch(thisPump.plantLoc.branchNum).Name
            : "N/A");
}

void GetRequiredMassFlowRate(EnergyPlusData &state,
                             int const LoopNum,
                             int const PumpNum,
                             Real64 const InletNodeMassFlowRate,
                             Real64 &ActualFlowRate,
                             Real64 &PumpMinMassFlowRateVFDRange,
                             Real64 &PumpMaxMassFlowRateVFDRange)
{
    // Using/Aliasing
    using PlantPressureSystem::ResolveLoopFlowVsPressure;
    using PlantUtilities::SetComponentFlowRate;

    Real64 PumpMassFlowRateMaxPress(0.0); // Maximum mass flow rate associated with maximum pressure limit
    Real64 PumpMassFlowRateMinPress(0.0); // Minimum mass flow rate associated with minimum pressure limit
    Real64 RotSpeed_Max(0.0);             // Maximum rotational speed in rps
    Real64 RotSpeed_Min(0.0);             // Minimum rotational speed in rps
    Real64 MinPress(0.0);                 // Minimum pressure
    Real64 MaxPress(0.0);                 // Maximum pressure

    auto &thisPump = state.dataPumps->PumpEquip(PumpNum);

    RotSpeed_Min = thisPump.VFD.minRPMSched->getCurrentVal();
    RotSpeed_Max = thisPump.VFD.maxRPMSched->getCurrentVal();
    MinPress = thisPump.VFD.lowerPsetSched->getCurrentVal();
    MaxPress = thisPump.VFD.upperPsetSched->getCurrentVal();

    // Calculate maximum and minimum mass flow rate associated with maximun and minimum RPM
    if (thisPump.plantLoc.loopNum > 0) {
        auto const &thisPlantLoop = state.dataPlnt->PlantLoop(thisPump.plantLoc.loopNum);
        if (thisPlantLoop.UsePressureForPumpCalcs && thisPlantLoop.PressureSimType == DataPlant::PressSimType::FlowCorrection &&
            thisPlantLoop.PressureDrop > 0.0) {
            thisPump.PumpMassFlowRateMaxRPM = ResolveLoopFlowVsPressure(state,
                                                                        thisPump.plantLoc.loopNum,
                                                                        InletNodeMassFlowRate,
                                                                        thisPump.PressureCurve_Index,
                                                                        RotSpeed_Max,
                                                                        thisPump.ImpellerDiameter,
                                                                        thisPump.MinPhiValue,
                                                                        thisPump.MaxPhiValue);
            thisPump.PumpMassFlowRateMinRPM = ResolveLoopFlowVsPressure(state,
                                                                        thisPump.plantLoc.loopNum,
                                                                        InletNodeMassFlowRate,
                                                                        thisPump.PressureCurve_Index,
                                                                        RotSpeed_Min,
                                                                        thisPump.ImpellerDiameter,
                                                                        thisPump.MinPhiValue,
                                                                        thisPump.MaxPhiValue);
        }
    }

    // Not correct necessarily, but values are coming out way wrong here, maxRPMmdot~3, minRPMmdot~62!
    if (thisPump.PumpMassFlowRateMaxRPM < thisPump.PumpMassFlowRateMinRPM) {
        thisPump.PumpMassFlowRateMaxRPM = thisPump.PumpMassFlowRateMinRPM;
    }

    // Calculate maximum and minimum mass flow rate associated with operating pressure range
    if (thisPump.plantLoc.loopNum > 0) {
        auto const &thisPlantLoop = state.dataPlnt->PlantLoop(LoopNum);
        if (thisPlantLoop.PressureEffectiveK > 0.0) {
            PumpMassFlowRateMaxPress = std::sqrt(MaxPress / thisPlantLoop.PressureEffectiveK);
            PumpMassFlowRateMinPress = std::sqrt(MinPress / thisPlantLoop.PressureEffectiveK);
        }
    }

    // Decide operating range for mass flow rate
    // Maximum mass flow rate value of the range
    if (thisPump.PumpMassFlowRateMaxRPM > PumpMassFlowRateMaxPress) {
        // Maximum pressure value governs maximum VFD range value
        PumpMaxMassFlowRateVFDRange = PumpMassFlowRateMaxPress;
    } else {
        // Maximum RPM value governs maximum VFD range value
        PumpMaxMassFlowRateVFDRange = thisPump.PumpMassFlowRateMaxRPM;
    }

    // Minimum mass flow rate value of the range
    if (thisPump.PumpMassFlowRateMinRPM > PumpMassFlowRateMinPress) {
        // Minimum pressure value governs minimum VFD range value
        PumpMinMassFlowRateVFDRange = thisPump.PumpMassFlowRateMinRPM;
    } else {
        // Minimum pressure range value governs minimum VFD range value
        PumpMinMassFlowRateVFDRange = PumpMassFlowRateMinPress;
    }

    // Set the mass flow rate within VFD operating range
    if (InletNodeMassFlowRate > PumpMinMassFlowRateVFDRange) {
        if (InletNodeMassFlowRate < PumpMaxMassFlowRateVFDRange) {
            // Flow request is within VFD operating range
            ActualFlowRate = InletNodeMassFlowRate;
        } else {
            // Flow request is outside VFD operating range
            // Flow is set to maximum VFD operating range
            ActualFlowRate = PumpMaxMassFlowRateVFDRange;
        }
    } else {
        // Flow request is outside VFD operating range
        // Flow is set to minimum VFD operating Range
        ActualFlowRate = PumpMinMassFlowRateVFDRange;
    }
}

} // namespace EnergyPlus::Pumps
