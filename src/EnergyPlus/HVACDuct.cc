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

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/HVACDuct.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace HVACDuct {

    // Module containing the routines dealing with the Duct component
    // in forced air air conditioning systems

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   17May2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and routines required to model duct
    // components in the EnergyPlus HVAC simulation

    // METHODOLOGY EMPLOYED:
    // At this point ducts are passive elements in the loop that just pass inlet node
    // conditions to the outlet node. The function of a duct component is to allow the
    // definition of a bypass branch: a branch must contain at least 1 component.

    // REFERENCES:
    // na

    // OTHER NOTES:
    // na

    // USE STATEMENTS:
    // <use statements for data only modules>
    // Using/Aliasing
    using namespace DataLoopNode;

    // <use statements for access to subroutines in other modules>

    // Data
    // MODULE PARAMETER DEFINITIONS:
    // na

    // DERIVED TYPE DEFINITIONS:

    // MODULE VARIABLE DECLARATIONS:

    // SUBROUTINE SPECIFICATIONS FOR MODULE HVACDuct:

    // <name Public routines, optionally name Private routines within this module>

    // Object Data

    // Functions

    void SimDuct(EnergyPlusData &state,
                 std::string_view CompName,                      // name of the duct component
                 [[maybe_unused]] bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep !unused1208
                 int &CompIndex                                  // index of duct component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   17May2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of a duct component

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int DuctNum; // index of duct being simulated

        if (state.dataHVACDuct->GetInputFlag) {
            GetDuctInput(state);
            state.dataHVACDuct->GetInputFlag = false;
        }

        // Get the duct component index
        if (CompIndex == 0) {
            DuctNum = Util::FindItemInList(CompName, state.dataHVACDuct->Duct);
            if (DuctNum == 0) {
                ShowFatalError(state, format("SimDuct: Component not found={}", CompName));
            }
            CompIndex = DuctNum;
        } else {
            DuctNum = CompIndex;
            if (DuctNum > state.dataHVACDuct->NumDucts || DuctNum < 1) {
                ShowFatalError(state,
                               format("SimDuct:  Invalid CompIndex passed={}, Number of Components={}, Entered Component name={}",
                                      DuctNum,
                                      state.dataHVACDuct->NumDucts,
                                      CompName));
            }
            if (state.dataHVACDuct->CheckEquipName(DuctNum)) {
                if (CompName != state.dataHVACDuct->Duct(DuctNum).Name) {
                    ShowFatalError(state,
                                   format("SimDuct: Invalid CompIndex passed={}, Component name={}, stored Component Name for that index={}",
                                          DuctNum,
                                          CompName,
                                          state.dataHVACDuct->Duct(DuctNum).Name));
                }
                state.dataHVACDuct->CheckEquipName(DuctNum) = false;
            }
        }

        CalcDuct(DuctNum);

        UpdateDuct(state, DuctNum);

        ReportDuct(DuctNum);
    }

    void GetDuctInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   17May2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for ducts and stores it in duct data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using NodeInputManager::GetOnlySingleNode;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int DuctNum; // duct index
        static constexpr std::string_view RoutineName("GetDuctInput:");
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "Duct";
        state.dataHVACDuct->NumDucts = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        state.dataHVACDuct->Duct.allocate(state.dataHVACDuct->NumDucts);
        state.dataHVACDuct->CheckEquipName.dimension(state.dataHVACDuct->NumDucts, true);

        for (DuctNum = 1; DuctNum <= state.dataHVACDuct->NumDucts; ++DuctNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     DuctNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            state.dataHVACDuct->Duct(DuctNum).Name = state.dataIPShortCut->cAlphaArgs(1);
            state.dataHVACDuct->Duct(DuctNum).InletNodeNum = GetOnlySingleNode(state,
                                                                               state.dataIPShortCut->cAlphaArgs(2),
                                                                               ErrorsFound,
                                                                               DataLoopNode::ConnectionObjectType::Duct,
                                                                               state.dataIPShortCut->cAlphaArgs(1),
                                                                               DataLoopNode::NodeFluidType::Air,
                                                                               DataLoopNode::ConnectionType::Inlet,
                                                                               NodeInputManager::CompFluidStream::Primary,
                                                                               ObjectIsNotParent);
            state.dataHVACDuct->Duct(DuctNum).OutletNodeNum = GetOnlySingleNode(state,
                                                                                state.dataIPShortCut->cAlphaArgs(3),
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::Duct,
                                                                                state.dataIPShortCut->cAlphaArgs(1),
                                                                                DataLoopNode::NodeFluidType::Air,
                                                                                DataLoopNode::ConnectionType::Outlet,
                                                                                NodeInputManager::CompFluidStream::Primary,
                                                                                ObjectIsNotParent);
            TestCompSet(state,
                        cCurrentModuleObject,
                        state.dataIPShortCut->cAlphaArgs(1),
                        state.dataIPShortCut->cAlphaArgs(2),
                        state.dataIPShortCut->cAlphaArgs(3),
                        "Air Nodes");
        }

        // No output variables

        if (ErrorsFound) {
            ShowFatalError(state, format("{} Errors found in input", RoutineName));
        }
    }

    void CalcDuct([[maybe_unused]] int const DuctNum) // number of the current duct being simulated !unused1208
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   17May2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // na

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na
    }

    void UpdateDuct(EnergyPlusData &state, int const DuctNum) // number of the current duct being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   17May2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Moves duct output to the outlet nodes

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InNode;  // inlet node number
        int OutNode; // outlet node number

        InNode = state.dataHVACDuct->Duct(DuctNum).InletNodeNum;
        OutNode = state.dataHVACDuct->Duct(DuctNum).OutletNodeNum;
        // Set the outlet air node conditions of the duct
        state.dataLoopNodes->Node(OutNode).MassFlowRate = state.dataLoopNodes->Node(InNode).MassFlowRate;
        state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(InNode).Temp;
        state.dataLoopNodes->Node(OutNode).HumRat = state.dataLoopNodes->Node(InNode).HumRat;
        state.dataLoopNodes->Node(OutNode).Enthalpy = state.dataLoopNodes->Node(InNode).Enthalpy;
        state.dataLoopNodes->Node(OutNode).Quality = state.dataLoopNodes->Node(InNode).Quality;
        state.dataLoopNodes->Node(OutNode).Press = state.dataLoopNodes->Node(InNode).Press;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMin = state.dataLoopNodes->Node(InNode).MassFlowRateMin;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMax = state.dataLoopNodes->Node(InNode).MassFlowRateMax;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataLoopNodes->Node(OutNode).CO2 = state.dataLoopNodes->Node(InNode).CO2;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataLoopNodes->Node(OutNode).GenContam = state.dataLoopNodes->Node(InNode).GenContam;
        }
    }

    void ReportDuct([[maybe_unused]] int const DuctNum) // number of the current duct being simulated !unused1208
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   17May2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Fill remaining report variables

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na
    }

} // namespace HVACDuct

} // namespace EnergyPlus
