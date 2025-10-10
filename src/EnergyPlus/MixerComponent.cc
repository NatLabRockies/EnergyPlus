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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
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

// Using/Aliasing
using namespace DataLoopNode;

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
            ShowFatalError(state, format("SimAirLoopMixer: Mixer not found={}", CompName));
        }
        CompIndex = MixerNum;
    } else {
        MixerNum = CompIndex;
        if (MixerNum > state.dataMixerComponent->NumMixers || MixerNum < 1) {
            ShowFatalError(state,
                           format("SimAirLoopMixer: Invalid CompIndex passed={}, Number of Mixers={}, Mixer name={}",
                                  MixerNum,
                                  state.dataMixerComponent->NumMixers,
                                  CompName));
        }
        if (state.dataMixerComponent->CheckEquipName(MixerNum)) {
            if (CompName != state.dataMixerComponent->MixerCond(MixerNum).MixerName) {
                ShowFatalError(state,
                               format("SimAirLoopMixer: Invalid CompIndex passed={}, Mixer name={}, stored Mixer Name for that index={}",
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
    using NodeInputManager::GetOnlySingleNode;

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
        Util::IsNameEmpty(state, AlphArray(1), CurrentModuleObject, ErrorsFound);

        state.dataMixerComponent->MixerCond(MixerNum).MixerName = AlphArray(1);

        state.dataMixerComponent->MixerCond(MixerNum).OutletNode = GetOnlySingleNode(state,
                                                                                     AlphArray(2),
                                                                                     ErrorsFound,
                                                                                     DataLoopNode::ConnectionObjectType::AirLoopHVACZoneMixer,
                                                                                     AlphArray(1),
                                                                                     DataLoopNode::NodeFluidType::Air,
                                                                                     DataLoopNode::ConnectionType::Outlet,
                                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                                     ObjectIsNotParent);
        state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes = NumAlphas - 2;

        for (auto &e : state.dataMixerComponent->MixerCond) {
            e.InitFlag = true;
        }

        state.dataMixerComponent->MixerCond(MixerNum).InletNode.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMaxAvail.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMinAvail.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletTemp.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletHumRat.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletEnthalpy.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);
        state.dataMixerComponent->MixerCond(MixerNum).InletPressure.allocate(state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes);

        state.dataMixerComponent->MixerCond(MixerNum).InletNode = 0;
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMaxAvail = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMinAvail = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).InletTemp = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).InletHumRat = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).InletEnthalpy = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).InletPressure = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMaxAvail = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMinAvail = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletTemp = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy = 0.0;
        state.dataMixerComponent->MixerCond(MixerNum).OutletPressure = 0.0;

        for (NodeNum = 1; NodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++NodeNum) {

            state.dataMixerComponent->MixerCond(MixerNum).InletNode(NodeNum) =
                GetOnlySingleNode(state,
                                  AlphArray(2 + NodeNum),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirLoopHVACZoneMixer,
                                  AlphArray(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Inlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsNotParent);
            if (lAlphaBlanks(2 + NodeNum)) {
                ShowSevereError(state, format("{} is Blank, {} = {}", cAlphaFields(2 + NodeNum), CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

    } // end Number of Mixer Loop

    // Check for duplicate names specified in Zone Mixer
    for (MixerNum = 1; MixerNum <= state.dataMixerComponent->NumMixers; ++MixerNum) {
        NodeNum = state.dataMixerComponent->MixerCond(MixerNum).OutletNode;
        for (InNodeNum1 = 1; InNodeNum1 <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InNodeNum1) {
            if (NodeNum != state.dataMixerComponent->MixerCond(MixerNum).InletNode(InNodeNum1)) {
                continue;
            }
            ShowSevereError(state,
                            format("{} = {} specifies an inlet node name the same as the outlet node.",
                                   CurrentModuleObject,
                                   state.dataMixerComponent->MixerCond(MixerNum).MixerName));
            ShowContinueError(state, format("..{} = {}", cAlphaFields(2), state.dataLoopNodes->NodeID(NodeNum)));
            ShowContinueError(state, format("..Inlet Node #{} is duplicate.", InNodeNum1));
            ErrorsFound = true;
        }
        for (InNodeNum1 = 1; InNodeNum1 <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InNodeNum1) {
            for (InNodeNum2 = InNodeNum1 + 1; InNodeNum2 <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InNodeNum2) {
                if (state.dataMixerComponent->MixerCond(MixerNum).InletNode(InNodeNum1) !=
                    state.dataMixerComponent->MixerCond(MixerNum).InletNode(InNodeNum2)) {
                    continue;
                }
                ShowSevereError(state,
                                format("{} = {} specifies duplicate inlet nodes in its inlet node list.",
                                       CurrentModuleObject,
                                       state.dataMixerComponent->MixerCond(MixerNum).MixerName));
                ShowContinueError(state, format("..Inlet Node #{} Name={}", InNodeNum1, state.dataLoopNodes->NodeID(InNodeNum1)));
                ShowContinueError(state, format("..Inlet Node #{} is duplicate.", InNodeNum2));
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
        ShowFatalError(state, format("{}Errors found in getting input.", RoutineName));
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

    // Do the following initializations (every time step): This should be the info from
    // the previous components outlets or the node data in this section.

    // Transfer the node data to MixerCond data structure
    for (NodeNum = 1; NodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++NodeNum) {

        int InletNode = state.dataMixerComponent->MixerCond(MixerNum).InletNode(NodeNum);
        // Set all of the inlet mass flow variables from the nodes
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(NodeNum) = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMaxAvail(NodeNum) = state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail;
        state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMinAvail(NodeNum) = state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail;
        // Set all of the inlet state variables from the inlet nodes
        state.dataMixerComponent->MixerCond(MixerNum).InletTemp(NodeNum) = state.dataLoopNodes->Node(InletNode).Temp;
        state.dataMixerComponent->MixerCond(MixerNum).InletHumRat(NodeNum) = state.dataLoopNodes->Node(InletNode).HumRat;
        state.dataMixerComponent->MixerCond(MixerNum).InletEnthalpy(NodeNum) = state.dataLoopNodes->Node(InletNode).Enthalpy;
        state.dataMixerComponent->MixerCond(MixerNum).InletPressure(NodeNum) = state.dataLoopNodes->Node(InletNode).Press;
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

    // Reset the totals to zero before they are summed.
    state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate = 0.0;
    state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMaxAvail = 0.0;
    state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMinAvail = 0.0;
    state.dataMixerComponent->MixerCond(MixerNum).OutletTemp = 0.0;
    state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat = 0.0;
    state.dataMixerComponent->MixerCond(MixerNum).OutletPressure = 0.0;
    state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy = 0.0;

    for (InletNodeNum = 1; InletNodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InletNodeNum) {
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate +=
            state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(InletNodeNum);
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMaxAvail +=
            state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMaxAvail(InletNodeNum);
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMinAvail +=
            state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRateMinAvail(InletNodeNum);
    }

    if (state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate > 0.0) {

        // Mass balance on moisture to get outlet air humidity ratio

        for (InletNodeNum = 1; InletNodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InletNodeNum) {
            state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat +=
                state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(InletNodeNum) *
                state.dataMixerComponent->MixerCond(MixerNum).InletHumRat(InletNodeNum) /
                state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate;
        }

        // "Momentum balance" to get outlet air pressure

        for (InletNodeNum = 1; InletNodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InletNodeNum) {
            state.dataMixerComponent->MixerCond(MixerNum).OutletPressure +=
                state.dataMixerComponent->MixerCond(MixerNum).InletPressure(InletNodeNum) *
                state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(InletNodeNum) /
                state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate;
        }

        // Energy balance to get outlet air enthalpy

        for (InletNodeNum = 1; InletNodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InletNodeNum) {
            state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy +=
                state.dataMixerComponent->MixerCond(MixerNum).InletEnthalpy(InletNodeNum) *
                state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(InletNodeNum) /
                state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate;
        }

        // Use Enthalpy and humidity ratio to get outlet temperature from psych chart

        state.dataMixerComponent->MixerCond(MixerNum).OutletTemp =
            PsyTdbFnHW(state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy, state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat);

    } else {
        // Mass Flow in air loop is zero and loop is not operating.
        // Arbitrarily set the output to the first inlet leg
        state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat = state.dataMixerComponent->MixerCond(MixerNum).InletHumRat(1);
        state.dataMixerComponent->MixerCond(MixerNum).OutletPressure = state.dataMixerComponent->MixerCond(MixerNum).InletPressure(1);
        state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy = state.dataMixerComponent->MixerCond(MixerNum).InletEnthalpy(1);
        state.dataMixerComponent->MixerCond(MixerNum).OutletTemp = state.dataMixerComponent->MixerCond(MixerNum).InletTemp(1);
    }

    // make sure MassFlowRateMaxAvail is >= MassFlowRate
    state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMaxAvail = max(
        state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMaxAvail, state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate);
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
    int OutletNode;
    int InletNode;
    int InletNodeNum;

    OutletNode = state.dataMixerComponent->MixerCond(MixerNum).OutletNode;
    InletNode = state.dataMixerComponent->MixerCond(MixerNum).InletNode(1); // For now use first inlet node

    // Set the outlet air nodes of the Mixer
    state.dataLoopNodes->Node(OutletNode).MassFlowRate = state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate;
    state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMaxAvail;
    state.dataLoopNodes->Node(OutletNode).MassFlowRateMinAvail = state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRateMinAvail;
    state.dataLoopNodes->Node(OutletNode).Temp = state.dataMixerComponent->MixerCond(MixerNum).OutletTemp;
    state.dataLoopNodes->Node(OutletNode).HumRat = state.dataMixerComponent->MixerCond(MixerNum).OutletHumRat;
    state.dataLoopNodes->Node(OutletNode).Enthalpy = state.dataMixerComponent->MixerCond(MixerNum).OutletEnthalpy;
    state.dataLoopNodes->Node(OutletNode).Press = state.dataMixerComponent->MixerCond(MixerNum).OutletPressure;
    // Set the outlet nodes for properties that just pass through & not used
    state.dataLoopNodes->Node(OutletNode).Quality = state.dataLoopNodes->Node(InletNode).Quality;

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        if (state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate > 0.0) {
            // CO2 balance to get outlet air CO2
            state.dataLoopNodes->Node(OutletNode).CO2 = 0.0;
            for (InletNodeNum = 1; InletNodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InletNodeNum) {
                state.dataLoopNodes->Node(OutletNode).CO2 +=
                    state.dataLoopNodes->Node(state.dataMixerComponent->MixerCond(MixerNum).InletNode(InletNodeNum)).CO2 *
                    state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(InletNodeNum) /
                    state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate;
            }
        } else {
            state.dataLoopNodes->Node(OutletNode).CO2 = state.dataLoopNodes->Node(InletNode).CO2;
        }
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        if (state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate > 0.0) {
            // Generic contaminant balance to get outlet air CO2
            state.dataLoopNodes->Node(OutletNode).GenContam = 0.0;
            for (InletNodeNum = 1; InletNodeNum <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InletNodeNum) {
                state.dataLoopNodes->Node(OutletNode).GenContam +=
                    state.dataLoopNodes->Node(state.dataMixerComponent->MixerCond(MixerNum).InletNode(InletNodeNum)).GenContam *
                    state.dataMixerComponent->MixerCond(MixerNum).InletMassFlowRate(InletNodeNum) /
                    state.dataMixerComponent->MixerCond(MixerNum).OutletMassFlowRate;
            }
        } else {
            state.dataLoopNodes->Node(OutletNode).GenContam = state.dataLoopNodes->Node(InletNode).GenContam;
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
            ShowSevereError(state, format("{}, GetZoneMixerIndex: Zone Mixer not found={}", ThisObjectType, MixerName));
        } else {
            ShowSevereError(state, format("GetZoneMixerIndex: Zone Mixer not found={}", MixerName));
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
            for (int InNodeCtr = 1; InNodeCtr <= state.dataMixerComponent->MixerCond(MixerNum).NumInletNodes; ++InNodeCtr) {
                if (InNodeNum == state.dataMixerComponent->MixerCond(MixerNum).InletNode(InNodeCtr)) {
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
