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
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::MixerComponent {

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen
//       DATE WRITTEN   March 2000
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage Air Path Mixer Components

// METHODOLOGY EMPLOYED:
// This Mixer is very simple.  It just takes the inlets and sums them
// and sets that to the outlet conditions.  For the State Properties
// it just takes the flow weighted averages of them.

void SimAirMixer(EnergyPlusData &state, std::string_view CompName, int &CompIndex)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages Mixer component simulation.
    // It is called from the SimAirLoopComponent
    // at the system time step.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int MixerNum; // The Mixer that you are currently loading input into

    // Obtains and Allocates Mixer related parameters from input file
    if (state.dataMixerComponent->SimAirMixerInputFlag) { // First time subroutine has been entered
        GetMixerInput(state);
        state.dataMixerComponent->SimAirMixerInputFlag = false;
    }

    // Find the correct MixerNumber
    if (CompIndex == 0) {
        MixerNum = Util::FindItemInList(CompName, state.dataMixerComponent->MixerCond, &MixerConditions::MixerName);
        if (MixerNum == 0) {
            ShowFatalError(state, std::format("SimAirLoopMixer: Mixer not found={}", CompName));
        }
        CompIndex = MixerNum;
    } else {
        MixerNum = CompIndex;
        if (MixerNum > state.dataMixerComponent->NumMixers || MixerNum < 1) {
            ShowFatalError(state,
                           std::format("SimAirLoopMixer: Invalid CompIndex passed={}, Number of Mixers={}, Mixer name={}",
                                       MixerNum,
                                       state.dataMixerComponent->NumMixers,
                                       CompName));
        }
        if (state.dataMixerComponent->CheckEquipName(MixerNum)) {
            if (CompName != state.dataMixerComponent->MixerCond(MixerNum).MixerName) {
                ShowFatalError(state,
                               std::format("SimAirLoopMixer: Invalid CompIndex passed={}, Mixer name={}, stored Mixer Name for that index={}",
                                           MixerNum,
                                           CompName,
                                           state.dataMixerComponent->MixerCond(MixerNum).MixerName));
            }
            state.dataMixerComponent->CheckEquipName(MixerNum) = false;
        }
    }

    // With the correct MixerNum Initialize
    InitAirMixer(state, MixerNum); // Initialize all Mixer related parameters

    CalcAirMixer(state, MixerNum);

    // Update the current Mixer to the outlet nodes
    UpdateAirMixer(state, MixerNum);

    // Report the current Mixer
    ReportMixer(MixerNum);
}

// Get Input Section of the Module
//******************************************************************************

void GetMixerInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the main routine to call other input routines and Get routines

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // Using/Aliasing
    using Node::GetOnlySingleNode;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetMixerInput: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int MixerNum; // The Mixer that you are currently loading input into
    int NumAlphas;
    int NumNums;
    int NodeNum;
    int IOStat;
    bool ErrorsFound(false);
    int NumParams;
    int InNodeNum1;
    int InNodeNum2;
    std::string CurrentModuleObject; // for ease in getting objects
    Array1D_string AlphArray;        // Alpha input items for object
    Array1D_string cAlphaFields;     // Alpha field names
    Array1D_string cNumericFields;   // Numeric field names
    Array1D<Real64> NumArray;        // Numeric input items for object
    Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

    CurrentModuleObject = "AirLoopHVAC:ZoneMixer";
    state.dataMixerComponent->NumMixers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

    if (state.dataMixerComponent->NumMixers > 0) {
        state.dataMixerComponent->MixerCond.allocate(state.dataMixerComponent->NumMixers);
    }
    state.dataMixerComponent->CheckEquipName.dimension(state.dataMixerComponent->NumMixers, true);

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
    AlphArray.allocate(NumAlphas);
    cAlphaFields.allocate(NumAlphas);
    lAlphaBlanks.dimension(NumAlphas, true);
    cNumericFields.allocate(NumNums);
    lNumericBlanks.dimension(NumNums, true);
    NumArray.dimension(NumNums, 0.0);

    for (MixerNum = 1; MixerNum <= state.dataMixerComponent->NumMixers; ++MixerNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 MixerNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields);

        auto &mixer = state.dataMixerComponent->MixerCond(MixerNum);

        mixer.MixerName = AlphArray(1);

        mixer.OutletNode = GetOnlySingleNode(state,
                                             AlphArray(2),
                                             ErrorsFound,
                                             Node::ConnectionObjectType::AirLoopHVACZoneMixer,
                                             AlphArray(1),
                                             Node::FluidType::Air,
                                             Node::ConnectionType::Outlet,
                                             Node::CompFluidStream::Primary,
                                             Node::ObjectIsNotParent);
        mixer.NumInletNodes = NumAlphas - 2;

        for (auto &e : state.dataMixerComponent->MixerCond) {
            e.InitFlag = true;
        }

        mixer.InletNode.allocate(mixer.NumInletNodes);
        mixer.InletMassFlowRate.allocate(mixer.NumInletNodes);
        mixer.InletMassFlowRateMaxAvail.allocate(mixer.NumInletNodes);
        mixer.InletMassFlowRateMinAvail.allocate(mixer.NumInletNodes);
        mixer.InletTemp.allocate(mixer.NumInletNodes);
        mixer.InletHumRat.allocate(mixer.NumInletNodes);
        mixer.InletEnthalpy.allocate(mixer.NumInletNodes);
        mixer.InletPressure.allocate(mixer.NumInletNodes);

        mixer.InletNode = 0;
        mixer.InletMassFlowRate = 0.0;
        mixer.InletMassFlowRateMaxAvail = 0.0;
        mixer.InletMassFlowRateMinAvail = 0.0;
        mixer.InletTemp = 0.0;
        mixer.InletHumRat = 0.0;
        mixer.InletEnthalpy = 0.0;
        mixer.InletPressure = 0.0;
        mixer.OutletMassFlowRate = 0.0;
        mixer.OutletMassFlowRateMaxAvail = 0.0;
        mixer.OutletMassFlowRateMinAvail = 0.0;
        mixer.OutletTemp = 0.0;
        mixer.OutletHumRat = 0.0;
        mixer.OutletEnthalpy = 0.0;
        mixer.OutletPressure = 0.0;

        for (NodeNum = 1; NodeNum <= mixer.NumInletNodes; ++NodeNum) {

            mixer.InletNode(NodeNum) = GetOnlySingleNode(state,
                                                         AlphArray(2 + NodeNum),
                                                         ErrorsFound,
                                                         Node::ConnectionObjectType::AirLoopHVACZoneMixer,
                                                         AlphArray(1),
                                                         Node::FluidType::Air,
                                                         Node::ConnectionType::Inlet,
                                                         Node::CompFluidStream::Primary,
                                                         Node::ObjectIsNotParent);
            if (lAlphaBlanks(2 + NodeNum)) {
                ShowSevereError(state, std::format("{} is Blank, {} = {}", cAlphaFields(2 + NodeNum), CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

    } // end Number of Mixer Loop

    // Check for duplicate names specified in Zone Mixer
    for (MixerNum = 1; MixerNum <= state.dataMixerComponent->NumMixers; ++MixerNum) {
        auto &mixer = state.dataMixerComponent->MixerCond(MixerNum);
        NodeNum = mixer.OutletNode;
        for (InNodeNum1 = 1; InNodeNum1 <= mixer.NumInletNodes; ++InNodeNum1) {
            if (NodeNum != mixer.InletNode(InNodeNum1)) {
                continue;
            }
            ShowSevereError(state,
                            std::format("{} = {} specifies an inlet node name the same as the outlet node.", CurrentModuleObject, mixer.MixerName));
            ShowContinueError(state, std::format("..{} = {}", cAlphaFields(2), state.dataLoopNodes->NodeID(NodeNum)));
            ShowContinueError(state, std::format("..Inlet Node #{} is duplicate.", InNodeNum1));
            ErrorsFound = true;
        }
        for (InNodeNum1 = 1; InNodeNum1 <= mixer.NumInletNodes; ++InNodeNum1) {
            for (InNodeNum2 = InNodeNum1 + 1; InNodeNum2 <= mixer.NumInletNodes; ++InNodeNum2) {
                if (mixer.InletNode(InNodeNum1) != mixer.InletNode(InNodeNum2)) {
                    continue;
                }
                ShowSevereError(state,
                                std::format("{} = {} specifies duplicate inlet nodes in its inlet node list.", CurrentModuleObject, mixer.MixerName));
                ShowContinueError(state, std::format("..Inlet Node #{} Name={}", InNodeNum1, state.dataLoopNodes->NodeID(InNodeNum1)));
                ShowContinueError(state, std::format("..Inlet Node #{} is duplicate.", InNodeNum2));
                ErrorsFound = true;
            }
        }
    }

    AlphArray.deallocate();
    NumArray.deallocate();
    cAlphaFields.deallocate();
    lAlphaBlanks.deallocate();
    cNumericFields.deallocate();
    lNumericBlanks.deallocate();

    if (ErrorsFound) {
        ShowFatalError(state, std::format("{}Errors found in getting input.", RoutineName));
    }
}

// End of Get Input subroutines for the HB Module
//******************************************************************************

// Beginning Initialization Section of the Module
//******************************************************************************

void InitAirMixer(EnergyPlusData &state, int const MixerNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for  initializations of the Mixer Components.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // REFERENCES:
    // na

    // USE STATEMENTS:
    // na

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NodeNum;

    auto &mixer = state.dataMixerComponent->MixerCond(MixerNum);

    // Do the following initializations (every time step): This should be the info from
    // the previous components outlets or the node data in this section.

    // Transfer the node data to MixerCond data structure
    for (NodeNum = 1; NodeNum <= mixer.NumInletNodes; ++NodeNum) {

        auto &inletNode = state.dataLoopNodes->Node(mixer.InletNode(NodeNum));
        // Set all of the inlet mass flow variables from the nodes
        mixer.InletMassFlowRate(NodeNum) = inletNode.MassFlowRate;
        mixer.InletMassFlowRateMaxAvail(NodeNum) = inletNode.MassFlowRateMaxAvail;
        mixer.InletMassFlowRateMinAvail(NodeNum) = inletNode.MassFlowRateMinAvail;
        // Set all of the inlet state variables from the inlet nodes
        mixer.InletTemp(NodeNum) = inletNode.Temp;
        mixer.InletHumRat(NodeNum) = inletNode.HumRat;
        mixer.InletEnthalpy(NodeNum) = inletNode.Enthalpy;
        mixer.InletPressure(NodeNum) = inletNode.Press;
    }
}

// End Initialization Section of the Module
//******************************************************************************

// Begin Algorithm Section of the Module
//******************************************************************************

void CalcAirMixer(EnergyPlusData &state, int &MixerNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine needs a description.

    // METHODOLOGY EMPLOYED:
    // Needs description, as appropriate.

    // REFERENCES:
    // na

    // Using/Aliasing
    using Psychrometrics::PsyTdbFnHW;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int InletNodeNum;

    auto &mixer = state.dataMixerComponent->MixerCond(MixerNum);

    // Reset the totals to zero before they are summed.
    mixer.OutletMassFlowRate = 0.0;
    mixer.OutletMassFlowRateMaxAvail = 0.0;
    mixer.OutletMassFlowRateMinAvail = 0.0;
    mixer.OutletTemp = 0.0;
    mixer.OutletHumRat = 0.0;
    mixer.OutletPressure = 0.0;
    mixer.OutletEnthalpy = 0.0;
    Real64 massFlowRateParallelPIULk = 0.0;
    Real64 massFlowRateHumRatParallelPIULk = 0.0;
    Real64 massFlowRatePressureParallelPIULk = 0.0;
    Real64 massFlowRateEnthalpyParallelPIULk = 0.0;
    for (InletNodeNum = 1; InletNodeNum <= mixer.NumInletNodes; ++InletNodeNum) {

        // Get PIU leakage flow rate only first mixer in return path
        // First: check if this mixer is in a return path
        if (state.dataPowerInductionUnits->NumParallelPIUs > 0) {
            for (int returnAirPathNum = 1; returnAirPathNum <= (int)state.dataZoneEquip->ReturnAirPath.size(); ++returnAirPathNum) {
                auto &returnAirPath = state.dataZoneEquip->ReturnAirPath(returnAirPathNum);
                const int returnAirPathCompNumOfComponents = returnAirPath.NumOfComponents;
                for (int returnPathCompNum = 1; returnPathCompNum <= returnAirPathCompNumOfComponents; ++returnPathCompNum) {
                    if (returnAirPath.ComponentName(returnPathCompNum) == mixer.MixerName &&
                        returnAirPath.ComponentTypeEnum(returnPathCompNum) == DataZoneEquipment::AirLoopHVACZone::Mixer) {
                        // Second: check if inlet nodes in the mixer are return nodes of zones served by a ADU that includes a parallel PIU
                        if (!state.dataDefineEquipment->AirDistUnit.empty()) {
                            for (int airDistUnitNum = 1; airDistUnitNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++airDistUnitNum) {
                                if (const auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(airDistUnitNum); airDistUnit.piuLkZoneNum > 0) {
                                    if (const int airDistUnitZoneNum = airDistUnit.ZoneNum; airDistUnitZoneNum > 0) {
                                        const int numRetNodes = state.dataZoneEquip->ZoneEquipConfig(airDistUnitZoneNum).NumReturnNodes;
                                        for (int retZoneAirNodeNum = 1; retZoneAirNodeNum <= numRetNodes; ++retZoneAirNodeNum) {
                                            if (const int retZoneAirNode =
                                                    state.dataZoneEquip->ZoneEquipConfig(airDistUnitZoneNum).ReturnNodeAirLoopNum(retZoneAirNodeNum);
                                                retZoneAirNode == InletNodeNum) {
                                                // Third: increment to get the mixer leakage
                                                massFlowRateParallelPIULk += airDistUnit.massFlowRateParallelPIULk;
                                                massFlowRateHumRatParallelPIULk += airDistUnit.massFlowRateParallelPIULk *
                                                                                   state.dataLoopNodes->Node(airDistUnit.piuLkZoneNum).HumRat;
                                                massFlowRatePressureParallelPIULk +=
                                                    airDistUnit.massFlowRateParallelPIULk * state.dataLoopNodes->Node(airDistUnit.piuLkZoneNum).Press;
                                                massFlowRateEnthalpyParallelPIULk += airDistUnit.massFlowRateParallelPIULk *
                                                                                     state.dataLoopNodes->Node(airDistUnit.piuLkZoneNum).Enthalpy;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        mixer.OutletMassFlowRate += mixer.InletMassFlowRate(InletNodeNum);
        mixer.OutletMassFlowRateMaxAvail += mixer.InletMassFlowRateMaxAvail(InletNodeNum);
        mixer.OutletMassFlowRateMinAvail += mixer.InletMassFlowRateMinAvail(InletNodeNum);
    }

    if (mixer.OutletMassFlowRate > 0.0) {

        // Mass balance on moisture to get outlet air humidity ratio

        for (InletNodeNum = 1; InletNodeNum <= mixer.NumInletNodes; ++InletNodeNum) {
            mixer.OutletHumRat += mixer.InletMassFlowRate(InletNodeNum) * mixer.InletHumRat(InletNodeNum) / mixer.OutletMassFlowRate;
        }

        // "Momentum balance" to get outlet air pressure

        for (InletNodeNum = 1; InletNodeNum <= mixer.NumInletNodes; ++InletNodeNum) {
            mixer.OutletPressure += mixer.InletPressure(InletNodeNum) * mixer.InletMassFlowRate(InletNodeNum) / mixer.OutletMassFlowRate;
        }

        // Energy balance to get outlet air enthalpy

        for (InletNodeNum = 1; InletNodeNum <= mixer.NumInletNodes; ++InletNodeNum) {
            mixer.OutletEnthalpy += mixer.InletEnthalpy(InletNodeNum) * mixer.InletMassFlowRate(InletNodeNum) / mixer.OutletMassFlowRate;
        }

        // Balance the leaks
        if (massFlowRateParallelPIULk > 0) {
            const Real64 noLeakMassFlowRate = mixer.OutletMassFlowRate;
            const Real64 totMassFlowRate = noLeakMassFlowRate + massFlowRateParallelPIULk;

            // Humidity ratio
            mixer.OutletHumRat = (noLeakMassFlowRate * mixer.OutletHumRat + massFlowRateHumRatParallelPIULk) / totMassFlowRate;

            // Pressure
            mixer.OutletPressure = (noLeakMassFlowRate * mixer.OutletPressure + massFlowRatePressureParallelPIULk) / totMassFlowRate;

            // Enthalpy
            mixer.OutletEnthalpy = (noLeakMassFlowRate * mixer.OutletEnthalpy + massFlowRateEnthalpyParallelPIULk) / totMassFlowRate;

            // Flow rate
            mixer.OutletMassFlowRate = totMassFlowRate;
        }

        // Use Enthalpy and humidity ratio to get outlet temperature from psych chart
        mixer.OutletTemp = PsyTdbFnHW(mixer.OutletEnthalpy, mixer.OutletHumRat);

    } else {
        // Mass Flow in air loop is zero and loop is not operating.
        // Arbitrarily set the output to the first inlet leg
        mixer.OutletHumRat = mixer.InletHumRat(1);
        mixer.OutletPressure = mixer.InletPressure(1);
        mixer.OutletEnthalpy = mixer.InletEnthalpy(1);
        mixer.OutletTemp = mixer.InletTemp(1);
    }

    // make sure MassFlowRateMaxAvail is >= MassFlowRate
    mixer.OutletMassFlowRateMaxAvail = max(mixer.OutletMassFlowRateMaxAvail, mixer.OutletMassFlowRate);
}

// End Algorithm Section of the Module
// *****************************************************************************

// Beginning of Update subroutines for the Mixer Module
// *****************************************************************************

void UpdateAirMixer(EnergyPlusData &state, int const MixerNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int InletNodeNum;

    auto &mixer = state.dataMixerComponent->MixerCond(MixerNum);

    auto &outletNode = state.dataLoopNodes->Node(mixer.OutletNode);
    auto &inletNode = state.dataLoopNodes->Node(mixer.InletNode(1)); // For now use first inlet node

    // Set the outlet air nodes of the Mixer
    outletNode.MassFlowRate = mixer.OutletMassFlowRate;
    outletNode.MassFlowRateMaxAvail = mixer.OutletMassFlowRateMaxAvail;
    outletNode.MassFlowRateMinAvail = mixer.OutletMassFlowRateMinAvail;
    outletNode.Temp = mixer.OutletTemp;
    outletNode.HumRat = mixer.OutletHumRat;
    outletNode.Enthalpy = mixer.OutletEnthalpy;
    outletNode.Press = mixer.OutletPressure;
    // Set the outlet nodes for properties that just pass through & not used
    outletNode.Quality = inletNode.Quality;

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        if (mixer.OutletMassFlowRate > 0.0) {
            // CO2 balance to get outlet air CO2
            outletNode.CO2 = 0.0;
            for (InletNodeNum = 1; InletNodeNum <= mixer.NumInletNodes; ++InletNodeNum) {
                outletNode.CO2 +=
                    state.dataLoopNodes->Node(mixer.InletNode(InletNodeNum)).CO2 * mixer.InletMassFlowRate(InletNodeNum) / mixer.OutletMassFlowRate;
            }
        } else {
            outletNode.CO2 = inletNode.CO2;
        }
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        if (mixer.OutletMassFlowRate > 0.0) {
            // Generic contaminant balance to get outlet air CO2
            outletNode.GenContam = 0.0;
            for (InletNodeNum = 1; InletNodeNum <= mixer.NumInletNodes; ++InletNodeNum) {
                outletNode.GenContam += state.dataLoopNodes->Node(mixer.InletNode(InletNodeNum)).GenContam * mixer.InletMassFlowRate(InletNodeNum) /
                                        mixer.OutletMassFlowRate;
            }
        } else {
            outletNode.GenContam = inletNode.GenContam;
        }
    }
}

//        End of Update subroutines for the Mixer Module
// *****************************************************************************

// Beginning of Reporting subroutines for the Mixer Module
// *****************************************************************************

void ReportMixer([[maybe_unused]] int const MixerNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine needs a description.

    // METHODOLOGY EMPLOYED:
    // Needs description, as appropriate.

    // REFERENCES:
    // na

    // USE STATEMENTS:
    // na

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    // na

    // Write(*,*)=MixerCond(MixerNum)%MixerPower    Still needs to report the Mixer power from this component
}

//        End of Reporting subroutines for the Mixer Module

// Beginning of Utility subroutines for the Mixer Component
// *****************************************************************************

void GetZoneMixerIndex(EnergyPlusData &state, std::string const &MixerName, int &MixerIndex, bool &ErrorsFound, std::string const &ThisObjectType)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   March 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets an index for a given zone mixer -- issues error message if that mixer
    // is not legal mixer.

    if (state.dataMixerComponent->GetZoneMixerIndexInputFlag) { // First time subroutine has been entered
        GetMixerInput(state);
        state.dataMixerComponent->GetZoneMixerIndexInputFlag = false;
    }

    MixerIndex = Util::FindItemInList(MixerName, state.dataMixerComponent->MixerCond, &MixerConditions::MixerName);
    if (MixerIndex == 0) {
        if (!ThisObjectType.empty()) {
            ShowSevereError(state, std::format("{}, GetZoneMixerIndex: Zone Mixer not found={}", ThisObjectType, MixerName));
        } else {
            ShowSevereError(state, std::format("GetZoneMixerIndex: Zone Mixer not found={}", MixerName));
        }
        ErrorsFound = true;
    }
}

int getZoneMixerIndexFromInletNode(EnergyPlusData &state, int const InNodeNum)
{

    if (state.dataMixerComponent->GetZoneMixerIndexInputFlag) { // First time subroutine has been entered
        GetMixerInput(state);
        state.dataMixerComponent->GetZoneMixerIndexInputFlag = false;
    }

    if (state.dataMixerComponent->NumMixers > 0) {
        for (int MixerNum = 1; MixerNum <= state.dataMixerComponent->NumMixers; ++MixerNum) {
            auto &mixer = state.dataMixerComponent->MixerCond(MixerNum);
            for (int InNodeCtr = 1; InNodeCtr <= mixer.NumInletNodes; ++InNodeCtr) {
                if (InNodeNum == mixer.InletNode(InNodeCtr)) {
                    return MixerNum;
                }
            }
        }
    }

    return 0;
}

// End of Utility subroutines for the Mixer Component
// *****************************************************************************

} // namespace EnergyPlus::MixerComponent
