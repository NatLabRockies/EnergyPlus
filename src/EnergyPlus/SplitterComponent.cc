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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace SplitterComponent {
    // Module containing the Splitter simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage Air Path Splitter Components

    void SimAirLoopSplitter(EnergyPlusData &state,
                            std::string_view CompName,
                            bool const FirstHVACIteration,
                            bool const FirstCall,
                            bool &SplitterInletChanged,
                            int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   March 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages Splitter component simulation.
        // It is called from the SimAirLoopComponent
        // at the system time step.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SplitterNum; // The Splitter that you are currently loading input for

        // Obtains and Allocates Splitter related parameters from input file
        if (state.dataSplitterComponent->GetSplitterInputFlag) { // First time subroutine has been entered
            GetSplitterInput(state);
        }

        // Find the correct SplitterNumber
        if (CompIndex == 0) {
            SplitterNum = Util::FindItemInList(CompName, state.dataSplitterComponent->SplitterCond, &SplitterConditions::SplitterName);
            if (SplitterNum == 0) {
                ShowFatalError(state, std::format("SimAirLoopSplitter: Splitter not found={}", CompName));
            }
            CompIndex = SplitterNum;
        } else {
            SplitterNum = CompIndex;
            if (SplitterNum > state.dataSplitterComponent->NumSplitters || SplitterNum < 1) {
                ShowFatalError(state,
                               std::format("SimAirLoopSplitter: Invalid CompIndex passed={}, Number of Splitters={}, Splitter name={}",
                                           SplitterNum,
                                           state.dataSplitterComponent->NumSplitters,
                                           CompName));
            }
            if (state.dataSplitterComponent->CheckEquipName(SplitterNum)) {
                if (CompName != state.dataSplitterComponent->SplitterCond(SplitterNum).SplitterName) {
                    ShowFatalError(
                        state,
                        std::format("SimAirLoopSplitter: Invalid CompIndex passed={}, Splitter name={}, stored Splitter Name for that index={}",
                                    SplitterNum,
                                    CompName,
                                    state.dataSplitterComponent->SplitterCond(SplitterNum).SplitterName));
                }
                state.dataSplitterComponent->CheckEquipName(SplitterNum) = false;
            }
        }

        InitAirLoopSplitter(state, SplitterNum, FirstHVACIteration, FirstCall); // Initialize all Splitter related parameters

        CalcAirLoopSplitter(state, SplitterNum, FirstCall);

        // Update the current Splitter to the outlet nodes
        UpdateSplitter(state, SplitterNum, SplitterInletChanged, FirstCall);

        // Report the current Splitter
        ReportSplitter(SplitterNum);
    }

    //*******************************

    // Get Input Section of the Module
    //******************************************************************************

    void GetSplitterInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   March 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main routine to call other input routines and
        // Get routines.  The Splitter only gets node connection data and not mass
        // flow rates.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        // Using/Aliasing
        using Node::GetOnlySingleNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetSplitterInput: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SplitterNum; // The Splitter that you are currently loading input into
        int NumAlphas;
        int NumNums;
        int NodeNum;
        int IOStat;
        bool ErrorsFound(false);
        int NumParams;
        int OutNodeNum1;
        int OutNodeNum2;
        std::string CurrentModuleObject; // for ease in getting objects
        Array1D_string AlphArray;        // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> NumArray;        // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

        // RESET THE GETINPUT FLAG
        state.dataSplitterComponent->GetSplitterInputFlag = false;

        CurrentModuleObject = "AirLoopHVAC:ZoneSplitter";
        state.dataSplitterComponent->NumSplitters = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        if (state.dataSplitterComponent->NumSplitters > 0) {
            state.dataSplitterComponent->SplitterCond.allocate(state.dataSplitterComponent->NumSplitters);
        }
        state.dataSplitterComponent->CheckEquipName.dimension(state.dataSplitterComponent->NumSplitters, true);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
        AlphArray.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        lAlphaBlanks.dimension(NumAlphas, true);
        cNumericFields.allocate(NumNums);
        lNumericBlanks.dimension(NumNums, true);
        NumArray.dimension(NumNums, 0.0);

        for (SplitterNum = 1; SplitterNum <= state.dataSplitterComponent->NumSplitters; ++SplitterNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     SplitterNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            auto &splitter = state.dataSplitterComponent->SplitterCond(SplitterNum);

            splitter.SplitterName = AlphArray(1);
            splitter.InletNode = GetOnlySingleNode(state,
                                                   AlphArray(2),
                                                   ErrorsFound,
                                                   Node::ConnectionObjectType::AirLoopHVACZoneSplitter,
                                                   AlphArray(1),
                                                   Node::FluidType::Air,
                                                   Node::ConnectionType::Inlet,
                                                   Node::CompFluidStream::Primary,
                                                   Node::ObjectIsNotParent);
            splitter.NumOutletNodes = NumAlphas - 2;

            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletNode.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletMassFlowRate.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletMassFlowRateMaxAvail.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletMassFlowRateMinAvail.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletTemp.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletHumRat.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletEnthalpy.allocate(splitter.NumOutletNodes);
            state.dataSplitterComponent->SplitterCond(SplitterNum).OutletPressure.allocate(splitter.NumOutletNodes);

            splitter.InletMassFlowRate = 0.0;
            splitter.InletMassFlowRateMaxAvail = 0.0;
            splitter.InletMassFlowRateMinAvail = 0.0;

            for (NodeNum = 1; NodeNum <= splitter.NumOutletNodes; ++NodeNum) {

                splitter.OutletNode(NodeNum) = GetOnlySingleNode(state,
                                                                 AlphArray(2 + NodeNum),
                                                                 ErrorsFound,
                                                                 Node::ConnectionObjectType::AirLoopHVACZoneSplitter,
                                                                 AlphArray(1),
                                                                 Node::FluidType::Air,
                                                                 Node::ConnectionType::Outlet,
                                                                 Node::CompFluidStream::Primary,
                                                                 Node::ObjectIsNotParent);
                if (lAlphaBlanks(2 + NodeNum)) {
                    ShowSevereError(state, std::format("{} is Blank, {} = {}", cAlphaFields(2 + NodeNum), CurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
            }

        } // end Number of Splitter Loop

        // Check for duplicate names specified in Zone Splitter
        for (SplitterNum = 1; SplitterNum <= state.dataSplitterComponent->NumSplitters; ++SplitterNum) {
            auto &splitter = state.dataSplitterComponent->SplitterCond(SplitterNum);
            NodeNum = splitter.InletNode;
            for (OutNodeNum1 = 1; OutNodeNum1 <= splitter.NumOutletNodes; ++OutNodeNum1) {
                if (NodeNum != splitter.OutletNode(OutNodeNum1)) {
                    continue;
                }
                ShowSevereError(
                    state,
                    std::format("{} = {} specifies an outlet node name the same as the inlet node.", CurrentModuleObject, splitter.SplitterName));
                ShowContinueError(state, std::format("..{}={}", cAlphaFields(2), state.dataLoopNodes->NodeID(NodeNum)));
                ShowContinueError(state, std::format("..Outlet Node #{} is duplicate.", OutNodeNum1));
                ErrorsFound = true;
            }
            for (OutNodeNum1 = 1; OutNodeNum1 <= splitter.NumOutletNodes; ++OutNodeNum1) {
                for (OutNodeNum2 = OutNodeNum1 + 1; OutNodeNum2 <= splitter.NumOutletNodes; ++OutNodeNum2) {
                    if (splitter.OutletNode(OutNodeNum1) != splitter.OutletNode(OutNodeNum2)) {
                        continue;
                    }
                    ShowSevereError(
                        state,
                        std::format("{} = {} specifies duplicate outlet nodes in its outlet node list.", CurrentModuleObject, splitter.SplitterName));
                    ShowContinueError(state, std::format("..Outlet Node #{} Name={}", OutNodeNum1, state.dataLoopNodes->NodeID(OutNodeNum1)));
                    ShowContinueError(state, std::format("..Outlet Node #{} is duplicate.", OutNodeNum2));
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

    void InitAirLoopSplitter(EnergyPlusData &state, int const SplitterNum, bool const FirstHVACIteration, bool const FirstCall)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   March 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initialisations of the Splitter Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        using Psychrometrics::PsyHFnTdbW;

        int NodeNum;
        Real64 AirEnthalpy; // [J/kg]

        auto &splitter = state.dataSplitterComponent->SplitterCond(SplitterNum);

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataSplitterComponent->MyEnvrnFlag) {

            // Calculate the air density and enthalpy for standard conditions...
            AirEnthalpy = PsyHFnTdbW(20.0, state.dataEnvrn->OutHumRat);

            // Initialize the inlet node to s standard set of conditions so that the
            //  flows match around the loop & do not cause convergence problems.
            auto &inletNode = state.dataLoopNodes->Node(splitter.InletNode);
            inletNode.Temp = 20.0;
            inletNode.HumRat = state.dataEnvrn->OutHumRat;
            inletNode.Enthalpy = AirEnthalpy;
            inletNode.Press = state.dataEnvrn->OutBaroPress;
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                inletNode.CO2 = state.dataContaminantBalance->OutdoorCO2;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                inletNode.GenContam = state.dataContaminantBalance->OutdoorGC;
            }

            state.dataSplitterComponent->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataSplitterComponent->MyEnvrnFlag = true;
        }

        // Set the inlet node for the Splitter
        auto &inletNode = state.dataLoopNodes->Node(splitter.InletNode);

        // Do the following initializations (every time step): This should be the info from
        // the previous components outlets or the node data in this section.
        // Load the node data in this section for the component simulation

        // This section is very important to understand.  The system off condition is important
        // transfer around the loop even if the splitter does not have enough information to
        // calculate the correct flow rates since the dampers are downstream and there is no pressure
        // simulation.  What happens in this section is the flow from upstream is not zero is
        // arbitrarily split by the number of inlet nodes.  This is by no way meant to determine the
        // correct split flow!  Just to give each outlet a non-zero flow so that the Air Distribution
        // Unit(ADU) downstream knows that the system is operating or has flow.  This is only done the first
        // iteration through and the splitter first pass.  After the first iteration the ADU sets the
        // correct flow and that is used and passed back upstream.
        if (FirstHVACIteration && FirstCall) {
            if (inletNode.MassFlowRate > 0.0) {
                for (NodeNum = 1; NodeNum <= splitter.NumOutletNodes; ++NodeNum) {
                    auto &outletNode = state.dataLoopNodes->Node(splitter.OutletNode(NodeNum));
                    outletNode.MassFlowRate = inletNode.MassFlowRate / splitter.NumOutletNodes;
                }
            }
            if (inletNode.MassFlowRateMaxAvail > 0.0) {
                for (NodeNum = 1; NodeNum <= splitter.NumOutletNodes; ++NodeNum) {
                    auto &outletNode = state.dataLoopNodes->Node(splitter.OutletNode(NodeNum));
                    outletNode.MassFlowRateMaxAvail = inletNode.MassFlowRateMaxAvail / splitter.NumOutletNodes;
                }
            }

        } // For FirstHVACIteration and FirstCall

        if (FirstCall) {
            // There is one exception to the rule stated above and that is if the system shuts OFF
            // for some operational or algorithm dependency.  This IF block should catch that condition
            // and then pass the NO flow condition downstream to the waiting ADU's.  Most of the time
            // this IF is jumped over.
            if (inletNode.MassFlowRateMaxAvail == 0.0) {

                for (NodeNum = 1; NodeNum <= splitter.NumOutletNodes; ++NodeNum) {
                    auto &outletNode = state.dataLoopNodes->Node(splitter.OutletNode(NodeNum));
                    outletNode.MassFlowRate = 0.0;
                    outletNode.MassFlowRateMaxAvail = 0.0;
                    outletNode.MassFlowRateMinAvail = 0.0;
                }
            } // For Node inlet Max Avail = 0.0

            // Pass the State Properties through every time.  This is what mainly happens each time
            // through the splitter,
            splitter.InletTemp = inletNode.Temp;
            splitter.InletHumRat = inletNode.HumRat;
            splitter.InletEnthalpy = inletNode.Enthalpy;
            splitter.InletPressure = inletNode.Press;

        } else { // On the second call from the ZoneEquipManager this is where the flows are passed back to
            // the splitter inlet.
            for (NodeNum = 1; NodeNum <= splitter.NumOutletNodes; ++NodeNum) {

                auto &outletNode = state.dataLoopNodes->Node(splitter.OutletNode(NodeNum));
                splitter.OutletMassFlowRate(NodeNum) = outletNode.MassFlowRate;
                splitter.OutletMassFlowRateMaxAvail(NodeNum) = outletNode.MassFlowRateMaxAvail;
                splitter.OutletMassFlowRateMinAvail(NodeNum) = outletNode.MassFlowRateMinAvail;
            }

        } // For FirstCall
    }

    void CalcAirLoopSplitter(EnergyPlusData &state, int const SplitterNum, bool const FirstCall)
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

        int OutletNodeNum;

        auto &splitter = state.dataSplitterComponent->SplitterCond(SplitterNum);

        // The first time through the State properties are split and passed through
        if (FirstCall) {
            // Moisture balance to get outlet air humidity ratio
            for (OutletNodeNum = 1; OutletNodeNum <= splitter.NumOutletNodes; ++OutletNodeNum) {
                splitter.OutletHumRat(OutletNodeNum) = splitter.InletHumRat;
            }

            // "Momentum balance" to get outlet air pressure
            for (OutletNodeNum = 1; OutletNodeNum <= splitter.NumOutletNodes; ++OutletNodeNum) {
                splitter.OutletPressure(OutletNodeNum) = splitter.InletPressure;
            }

            // Energy balance to get outlet air enthalpy
            for (OutletNodeNum = 1; OutletNodeNum <= splitter.NumOutletNodes; ++OutletNodeNum) {
                splitter.OutletEnthalpy(OutletNodeNum) = splitter.InletEnthalpy;
            }

            // Set outlet temperatures equal to inlet temperature
            for (OutletNodeNum = 1; OutletNodeNum <= splitter.NumOutletNodes; ++OutletNodeNum) {
                splitter.OutletTemp(OutletNodeNum) = splitter.InletTemp;
            }

        } else {
            // This is the second time through and this is where the mass flows from each outlet are
            // summed and then assigned upstream to the inlet node.
            // Overall Mass Continuity Equation to get inlet mass flow rates
            // Zero the inlet Totals before the Inlets are summed
            splitter.InletMassFlowRate = 0.0;
            splitter.InletMassFlowRateMaxAvail = 0.0;
            splitter.InletMassFlowRateMinAvail = 0.0;

            for (OutletNodeNum = 1; OutletNodeNum <= splitter.NumOutletNodes; ++OutletNodeNum) {
                splitter.InletMassFlowRate += splitter.OutletMassFlowRate(OutletNodeNum);

                splitter.InletMassFlowRateMaxAvail += splitter.OutletMassFlowRateMaxAvail(OutletNodeNum);
                splitter.InletMassFlowRateMinAvail += splitter.OutletMassFlowRateMinAvail(OutletNodeNum);
            }

            // What happens if Splitter inlet mass flow rate is greater than max available
        }
    }

    // End Algorithm Section of the Module
    // *****************************************************************************

    // Beginning of Update subroutines for the Splitter Module
    // *****************************************************************************

    void UpdateSplitter(EnergyPlusData &state, int const SplitterNum, bool &SplitterInletChanged, bool const FirstCall)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   March 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        Real64 constexpr FlowRateToler(0.01); // Tolerance for mass flow rate convergence (in kg/s)

        auto &splitter = state.dataSplitterComponent->SplitterCond(SplitterNum);

        // Set the inlet node for this splitter to be used throughout subroutine for either case
        auto &inletNode = state.dataLoopNodes->Node(splitter.InletNode);

        // On the FirstCall the State properties are passed through and the mass flows are not dealt with
        // except for NO flow conditions
        if (FirstCall) {
            // Set the outlet nodes for properties that just pass through & not used
            for (int NodeNum = 1; NodeNum <= splitter.NumOutletNodes; ++NodeNum) {
                auto &outletNode = state.dataLoopNodes->Node(splitter.OutletNode(NodeNum));
                outletNode.Temp = splitter.OutletTemp(NodeNum);
                outletNode.HumRat = splitter.OutletHumRat(NodeNum);
                outletNode.Enthalpy = splitter.OutletEnthalpy(NodeNum);
                outletNode.Quality = inletNode.Quality;
                outletNode.Press = splitter.OutletPressure(NodeNum);
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    outletNode.CO2 = inletNode.CO2;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    outletNode.GenContam = inletNode.GenContam;
                }
            }

        } else {
            // The second time through just updates the mass flow conditions back upstream
            //  to the inlet.  Before it sets the inlet it checks to see that the flow rate has not
            //  changed or not.  The tolerance has been relaxed some now that the splitter has been
            //  re-written

            // Set the outlet air nodes of the Splitter if the splitter results have changed
            //  beyond the tolerance.
            if (std::abs(inletNode.MassFlowRate - splitter.InletMassFlowRate) > FlowRateToler) {
                SplitterInletChanged = true;
            }
            inletNode.MassFlowRate = splitter.InletMassFlowRate;
            inletNode.MassFlowRateMaxAvail = splitter.InletMassFlowRateMaxAvail;
            inletNode.MassFlowRateMinAvail = splitter.InletMassFlowRateMinAvail;

        } // The FirstCall END IF
    }

    void ReportSplitter([[maybe_unused]] int const SplitterNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   March 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // Write(*,*)=SplitterCond(SplitterNum)%SplitterPower    Still needs to report the Splitter power from this component
    }

    int GetSplitterOutletNumber(EnergyPlusData &state,
                                std::string const &SplitterName, // must match Splitter names for the Splitter type
                                int const SplitterNum,           // Index of Splitters
                                bool &ErrorsFound                // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Feb 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given AirLoopHVAC:ZoneSplitter and returns the number of outlet nodes.  If
        // incorrect AirLoopHVAC:ZoneSplitter name is given, ErrorsFound is returned as true
        // as zero.

        // Return value
        int SplitterOutletNumber = 0;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichSplitter;

        // Obtains and Allocates AirLoopHVAC:ZoneSplitter related parameters from input file
        if (state.dataSplitterComponent->GetSplitterInputFlag) { // First time subroutine has been entered
            GetSplitterInput(state);
            state.dataSplitterComponent->GetSplitterInputFlag = false;
        }

        if (SplitterNum == 0) {
            WhichSplitter = Util::FindItemInList(SplitterName, state.dataSplitterComponent->SplitterCond, &SplitterConditions::SplitterName);
        } else {
            WhichSplitter = SplitterNum;
        }

        if (WhichSplitter != 0) {
            SplitterOutletNumber = state.dataSplitterComponent->SplitterCond(WhichSplitter).NumOutletNodes;
        }

        if (WhichSplitter == 0) {
            ShowSevereError(state, std::format("GetSplitterOuletNumber: Could not find Splitter = \"{}\"", SplitterName));
            ErrorsFound = true;
            SplitterOutletNumber = 0;
        }

        return SplitterOutletNumber;
    }

    Array1D_int GetSplitterNodeNumbers(EnergyPlusData &state,
                                       std::string const &SplitterName, // must match Splitter names for the Splitter type
                                       int const SplitterNum,           // Index of Splitters
                                       bool &ErrorsFound                // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Feb 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given AirLoopHVAC:ZoneSplitter and returns the node numbers.  If
        // incorrect AirLoopHVAC:ZoneSplitter name is given, ErrorsFound is returned as true
        // as zero.

        // Return value
        Array1D_int SplitterNodeNumbers;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichSplitter;

        // Obtains and Allocates AirLoopHVAC:ZoneSplitter related parameters from input file
        if (state.dataSplitterComponent->GetSplitterInputFlag) { // First time subroutine has been entered
            GetSplitterInput(state);
            state.dataSplitterComponent->GetSplitterInputFlag = false;
        }

        if (SplitterNum == 0) {
            WhichSplitter = Util::FindItemInList(SplitterName, state.dataSplitterComponent->SplitterCond, &SplitterConditions::SplitterName);
        } else {
            WhichSplitter = SplitterNum;
        }

        if (WhichSplitter != 0) {
            SplitterNodeNumbers.allocate(state.dataSplitterComponent->SplitterCond(WhichSplitter).NumOutletNodes + 2);
            SplitterNodeNumbers(1) = state.dataSplitterComponent->SplitterCond(WhichSplitter).InletNode;
            SplitterNodeNumbers(2) = state.dataSplitterComponent->SplitterCond(WhichSplitter).NumOutletNodes;
            for (int i = 1; i <= SplitterNodeNumbers(2); ++i) {
                SplitterNodeNumbers(i + 2) = state.dataSplitterComponent->SplitterCond(WhichSplitter).OutletNode(i);
            }
        }

        if (WhichSplitter == 0) {
            ShowSevereError(state, std::format("GetSplitterNodeNumbers: Could not find Splitter = \"{}\"", SplitterName));
            ErrorsFound = true;
        }

        return SplitterNodeNumbers;
    }

} // namespace SplitterComponent

} // namespace EnergyPlus
