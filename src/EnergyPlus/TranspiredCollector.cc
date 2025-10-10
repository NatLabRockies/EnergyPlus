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
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarCollectors.hh>
#include <EnergyPlus/TranspiredCollector.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace TranspiredCollector {

    // Module containing routines and data dealing with the Transpired Collectors

    // MODULE INFORMATION:
    //       AUTHOR         B.T. Griffith
    //       DATE WRITTEN   November 2004
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Encapsulates data and routines for simulating unglazed transpired solar collectors (UTSC)
    //   as a component on the HVAC air system.

    // METHODOLOGY EMPLOYED:
    // Two modes, passive and active.  Active is when air is purposely drawn through collector.
    // Passive is when air exchanges are driven by Natural Ventilation rather than outside air system

    // REFERENCES:
    // Heat Exchange effectiveness relations:
    // Kutscher, C.F. 1994. Heat exchange effectiveness and pressure drop for air flow through perforated plates
    //     with and without crosswind. Journal of Heat Transfer. May 1994, Vol. 116, p. 391.
    //     American Society of Mechanical Engineers.
    // Van Decker, G.W.E., K.G.T. Hollands, and A.P. Brunger. 2001. Heat-exchange relations for unglazed transpired
    //     solar collectors with circular holes on a square of triangular pitch. Solar Energy. Vol. 71, No. 1. pp 33-45, 2001.
    // .

    // OTHER NOTES:
    // EnergyPlus implementation is unique and adds new modeling not described in Literature.
    //   See EngineeringReference for details

    // Using/Aliasing
    using DataVectorTypes::Vector;

    int constexpr Layout_Square = 1;
    int constexpr Layout_Triangle = 2;
    int constexpr Correlation_Kutscher1994 = 1;
    int constexpr Correlation_VanDeckerHollandsBrunger2001 = 2;

    void SimTranspiredCollector(EnergyPlusData &state,
                                std::string_view CompName, // component name
                                int &CompIndex             // component index (to reduce string compares during simulation)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage simulation of Transpired Collectors

        // METHODOLOGY EMPLOYED:
        // Setup to avoid string comparisons after first call

        // Using/Aliasing
        using HVAC::TempControlTol;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int UTSCNum(0); // local number index for UTSC

        if (state.dataTranspiredCollector->GetInputFlag) {
            GetTranspiredCollectorInput(state);
            state.dataTranspiredCollector->GetInputFlag = false;
        }

        // Find the correct transpired collector with the Component name and/or index
        if (CompIndex == 0) {
            UTSCNum = Util::FindItemInList(CompName, state.dataTranspiredCollector->UTSC);
            if (UTSCNum == 0) {
                ShowFatalError(state, format("Transpired Collector not found={}", CompName));
            }
            CompIndex = UTSCNum;
        } else {
            UTSCNum = CompIndex;
            if (UTSCNum > state.dataTranspiredCollector->NumUTSC || UTSCNum < 1) {
                ShowFatalError(state,
                               format("SimTranspiredCollector: Invalid CompIndex passed={}, Number of Transpired Collectors={}, UTSC name={}",
                                      UTSCNum,
                                      state.dataTranspiredCollector->NumUTSC,
                                      CompName));
            }
            if (state.dataTranspiredCollector->CheckEquipName(UTSCNum)) {
                if (CompName != state.dataTranspiredCollector->UTSC(UTSCNum).Name) {
                    ShowFatalError(state,
                                   format("SimTranspiredCollector: Invalid CompIndex passed={}, Transpired Collector name={}, stored Transpired "
                                          "Collector Name for that index={}",
                                          UTSCNum,
                                          CompName,
                                          state.dataTranspiredCollector->UTSC(UTSCNum).Name));
                }
                state.dataTranspiredCollector->CheckEquipName(UTSCNum) = false;
            }
        }

        InitTranspiredCollector(state, CompIndex);

        // Control point of deciding if transpired collector is active or not.
        auto &UTSC_CI = state.dataTranspiredCollector->UTSC(CompIndex);
        auto &InletNode = UTSC_CI.InletNode;
        auto &ControlNode = UTSC_CI.ControlNode;
        UTSC_CI.IsOn = false;
        if ((UTSC_CI.availSched->getCurrentVal() > 0.0) && (UTSC_CI.InletMDot > 0.0)) { // availability Schedule | OA system is setting mass flow
            bool ControlLTSet(false);
            bool ControlLTSchedule(false);
            bool ZoneLTSchedule(false);
            assert(equal_dimensions(InletNode, ControlNode));
            assert(equal_dimensions(InletNode, UTSC_CI.ZoneNode));
            for (int i = InletNode.l(), e = InletNode.u(); i <= e; ++i) {
                if (state.dataLoopNodes->Node(InletNode(i)).Temp + TempControlTol < state.dataLoopNodes->Node(ControlNode(i)).TempSetPoint) {
                    ControlLTSet = true;
                }
                if (state.dataLoopNodes->Node(InletNode(i)).Temp + TempControlTol < UTSC_CI.freeHeatSetPointSched->getCurrentVal()) {
                    ControlLTSchedule = true;
                }
                if (state.dataLoopNodes->Node(UTSC_CI.ZoneNode(i)).Temp + TempControlTol < UTSC_CI.freeHeatSetPointSched->getCurrentVal()) {
                    ZoneLTSchedule = true;
                }
            }
            if (ControlLTSet || (ControlLTSchedule && ZoneLTSchedule)) {
                UTSC_CI.IsOn = true; // heating required | free heating helpful | free heating helpful
            }
        }

        if (state.dataTranspiredCollector->UTSC(UTSCNum).IsOn) {
            CalcActiveTranspiredCollector(state, UTSCNum);
        } else {
            CalcPassiveTranspiredCollector(state, UTSCNum);
        }

        UpdateTranspiredCollector(state, UTSCNum);
    }

    void GetTranspiredCollectorInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //  Retrieve user input and set up data structure

        // METHODOLOGY EMPLOYED:
        // usual EnergyPlus input
        // Extensible UTSC object for underlying heat transfer surfaces and for multisystem

        static constexpr std::string_view routineName = "GetTranspiredCollectorInput";

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using DataLoopNode::ObjectIsNotParent;
        using DataSurfaces::OtherSideCondModeledExt;
        using DataSurfaces::SurfaceData;
        using NodeInputManager::GetOnlySingleNode;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Array1D_string Alphas; // Alpha items for extensible
        // Solar Collectors:Unglazed Transpired object
        int Item;                    // Item to be "gotten"
        Array1D<Real64> Numbers(11); // Numeric items for object
        int NumAlphas;               // Number of Alphas for each GetObjectItem call
        int NumNumbers;              // Number of Numbers for each GetObjectItem call
        int MaxNumAlphas;            // argument for call to GetObjectDefMaxArgs
        int MaxNumNumbers;           // argument for call to GetObjectDefMaxArgs
        int Dummy;                   // argument for call to GetObjectDefMaxArgs
        int IOStatus;                // Used in GetObjectItem
        bool ErrorsFound(false);     // Set to true if errors in input, fatal at end of routine
        int Found;
        int AlphaOffset; // local temp var
        std::string Roughness;
        int ThisSurf;         // do loop counter
        Real64 AvgAzimuth;    // temp for error checking
        Real64 AvgTilt;       // temp for error checking
        int SurfID;           // local surface "pointer"
        Real64 TiltRads;      // average tilt of collector in radians
        Real64 tempHdeltaNPL; // temporary variable for buoyancy length scale
        int NumUTSCSplitter(0);
        Array1D_string AlphasSplit; // Alpha items for extensible
        // Solar Collectors:Unglazed Transpired object
        int ItemSplit;                        // Item to be "gotten"
        Array1D<Real64> NumbersSplit(1);      // Numeric items for object
        int NumAlphasSplit;                   // Number of Alphas for each GetObjectItem call
        int NumNumbersSplit;                  // Number of Numbers for each GetObjectItem call
        int MaxNumAlphasSplit;                // argument for call to GetObjectDefMaxArgs
        int MaxNumNumbersSplit;               // argument for call to GetObjectDefMaxArgs
        int IOStatusSplit;                    // Used in GetObjectItem
        int NumOASys;                         // do loop counter
        int ACountBase;                       // counter for alhpasSplit
        Array1D_bool SplitterNameOK;          // check for correct association of
        std::string CurrentModuleObject;      // for ease in renaming.
        std::string CurrentModuleMultiObject; // for ease in renaming.

        CurrentModuleObject = "SolarCollector:UnglazedTranspired";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Dummy, MaxNumAlphas, MaxNumNumbers);

        if (MaxNumNumbers != 11) {
            ShowSevereError(state,
                            format("GetTranspiredCollectorInput: {} Object Definition indicates not = 11 Number Objects, Number Indicated={}",
                                   CurrentModuleObject,
                                   MaxNumNumbers));
            ErrorsFound = true;
        }
        Alphas.allocate(MaxNumAlphas);
        Numbers = 0.0;
        Alphas = "";

        state.dataTranspiredCollector->NumUTSC = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        CurrentModuleMultiObject = "SolarCollector:UnglazedTranspired:Multisystem";
        NumUTSCSplitter = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleMultiObject);

        state.dataTranspiredCollector->UTSC.allocate(state.dataTranspiredCollector->NumUTSC);
        state.dataTranspiredCollector->CheckEquipName.dimension(state.dataTranspiredCollector->NumUTSC, true);
        SplitterNameOK.dimension(NumUTSCSplitter, false);

        for (Item = 1; Item <= state.dataTranspiredCollector->NumUTSC; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            // first handle alphas
            state.dataTranspiredCollector->UTSC(Item).Name = Alphas(1);

            // now check for multisystem
            if (NumUTSCSplitter > 0) {
                state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
                    state, CurrentModuleMultiObject, Dummy, MaxNumAlphasSplit, MaxNumNumbersSplit);

                if (MaxNumNumbersSplit != 0) {
                    ShowSevereError(state,
                                    format("GetTranspiredCollectorInput: {} Object Definition indicates not = 0 Number Objects, Number Indicated={}",
                                           CurrentModuleMultiObject,
                                           MaxNumNumbersSplit));
                    ErrorsFound = true;
                }
                if (!allocated(AlphasSplit)) {
                    AlphasSplit.allocate(MaxNumAlphasSplit);
                }
                NumbersSplit = 0.0;
                AlphasSplit = "";
                for (ItemSplit = 1; ItemSplit <= NumUTSCSplitter; ++ItemSplit) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(
                        state, CurrentModuleMultiObject, ItemSplit, AlphasSplit, NumAlphasSplit, NumbersSplit, NumNumbersSplit, IOStatusSplit);
                    if (!(Util::SameString(AlphasSplit(1), Alphas(1)))) {
                        continue;
                    }
                    SplitterNameOK(ItemSplit) = true;
                    state.dataTranspiredCollector->UTSC(Item).NumOASysAttached = std::floor(NumAlphasSplit / 4.0);
                    if (mod((NumAlphasSplit), 4) != 1) {
                        ShowSevereError(state,
                                        format("GetTranspiredCollectorInput: {} Object Definition indicates not uniform quadtuples of nodes for {}",
                                               CurrentModuleMultiObject,
                                               AlphasSplit(1)));
                        ErrorsFound = true;
                    }
                    state.dataTranspiredCollector->UTSC(Item).InletNode.allocate(state.dataTranspiredCollector->UTSC(Item).NumOASysAttached);
                    state.dataTranspiredCollector->UTSC(Item).InletNode = 0;
                    state.dataTranspiredCollector->UTSC(Item).OutletNode.allocate(state.dataTranspiredCollector->UTSC(Item).NumOASysAttached);
                    state.dataTranspiredCollector->UTSC(Item).OutletNode = 0;
                    state.dataTranspiredCollector->UTSC(Item).ControlNode.allocate(state.dataTranspiredCollector->UTSC(Item).NumOASysAttached);
                    state.dataTranspiredCollector->UTSC(Item).ControlNode = 0;
                    state.dataTranspiredCollector->UTSC(Item).ZoneNode.allocate(state.dataTranspiredCollector->UTSC(Item).NumOASysAttached);
                    state.dataTranspiredCollector->UTSC(Item).ZoneNode = 0;
                    for (NumOASys = 1; NumOASys <= state.dataTranspiredCollector->UTSC(Item).NumOASysAttached; ++NumOASys) {
                        ACountBase = (NumOASys - 1) * 4 + 2;
                        state.dataTranspiredCollector->UTSC(Item).InletNode(NumOASys) =
                            GetOnlySingleNode(state,
                                              AlphasSplit(ACountBase),
                                              ErrorsFound,
                                              DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                              AlphasSplit(1),
                                              DataLoopNode::NodeFluidType::Air,
                                              DataLoopNode::ConnectionType::Inlet,
                                              static_cast<NodeInputManager::CompFluidStream>(NumOASys),
                                              ObjectIsNotParent);

                        state.dataTranspiredCollector->UTSC(Item).OutletNode(NumOASys) =
                            GetOnlySingleNode(state,
                                              AlphasSplit(ACountBase + 1),
                                              ErrorsFound,
                                              DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                              AlphasSplit(1),
                                              DataLoopNode::NodeFluidType::Air,
                                              DataLoopNode::ConnectionType::Outlet,
                                              static_cast<NodeInputManager::CompFluidStream>(NumOASys),
                                              ObjectIsNotParent);
                        TestCompSet(state,
                                    CurrentModuleObject,
                                    AlphasSplit(1),
                                    AlphasSplit(ACountBase),
                                    AlphasSplit(ACountBase + 1),
                                    "Transpired Collector Air Nodes"); // appears that test fails by design??
                        state.dataTranspiredCollector->UTSC(Item).ControlNode(NumOASys) =
                            GetOnlySingleNode(state,
                                              AlphasSplit(ACountBase + 2),
                                              ErrorsFound,
                                              DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                              AlphasSplit(1),
                                              DataLoopNode::NodeFluidType::Air,
                                              DataLoopNode::ConnectionType::Sensor,
                                              NodeInputManager::CompFluidStream::Primary,
                                              ObjectIsNotParent);

                        state.dataTranspiredCollector->UTSC(Item).ZoneNode(NumOASys) =
                            GetOnlySingleNode(state,
                                              AlphasSplit(ACountBase + 3),
                                              ErrorsFound,
                                              DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                              AlphasSplit(1),
                                              DataLoopNode::NodeFluidType::Air,
                                              DataLoopNode::ConnectionType::Sensor,
                                              NodeInputManager::CompFluidStream::Primary,
                                              ObjectIsNotParent);

                    } // Each OA System in a Multisystem
                      // DEALLOCATE(AlphasSplit)
                } // each Multisystem present
            } // any UTSC Multisystem present

            state.dataTranspiredCollector->UTSC(Item).OSCMName = Alphas(2);
            Found = Util::FindItemInList(state.dataTranspiredCollector->UTSC(Item).OSCMName, state.dataSurface->OSCM);
            if (Found == 0) {
                ShowSevereError(state,
                                format("{} not found={} in {} ={}",
                                       state.dataIPShortCut->cAlphaFieldNames(2),
                                       state.dataTranspiredCollector->UTSC(Item).OSCMName,
                                       CurrentModuleObject,
                                       state.dataTranspiredCollector->UTSC(Item).Name));
                ErrorsFound = true;
            }
            state.dataTranspiredCollector->UTSC(Item).OSCMPtr = Found;
            if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
                state.dataTranspiredCollector->UTSC(Item).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataTranspiredCollector->UTSC(Item).availSched = Sched::GetSchedule(state, Alphas(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), Alphas(3));
                ErrorsFound = true;
                continue;
            }

            // now if UTSC(Item)%NumOASysAttached still not set, assume no multisystem
            if (state.dataTranspiredCollector->UTSC(Item).NumOASysAttached == 0) {
                state.dataTranspiredCollector->UTSC(Item).NumOASysAttached = 1;
                state.dataTranspiredCollector->UTSC(Item).InletNode.allocate(1);
                state.dataTranspiredCollector->UTSC(Item).InletNode(1) = 0;
                state.dataTranspiredCollector->UTSC(Item).OutletNode.allocate(1);
                state.dataTranspiredCollector->UTSC(Item).OutletNode(1) = 0;
                state.dataTranspiredCollector->UTSC(Item).ControlNode.allocate(1);
                state.dataTranspiredCollector->UTSC(Item).ControlNode(1) = 0;
                state.dataTranspiredCollector->UTSC(Item).ZoneNode.allocate(1);
                state.dataTranspiredCollector->UTSC(Item).ZoneNode(1) = 0;

                state.dataTranspiredCollector->UTSC(Item).InletNode(1) =
                    GetOnlySingleNode(state,
                                      Alphas(4),
                                      ErrorsFound,
                                      DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                      Alphas(1),
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Inlet,
                                      NodeInputManager::CompFluidStream::Primary,
                                      ObjectIsNotParent);
                state.dataTranspiredCollector->UTSC(Item).OutletNode(1) =
                    GetOnlySingleNode(state,
                                      Alphas(5),
                                      ErrorsFound,
                                      DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                      Alphas(1),
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Outlet,
                                      NodeInputManager::CompFluidStream::Primary,
                                      ObjectIsNotParent);
                TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(4), Alphas(5), "Transpired Collector Air Nodes");

                state.dataTranspiredCollector->UTSC(Item).ControlNode(1) =
                    GetOnlySingleNode(state,
                                      Alphas(6),
                                      ErrorsFound,
                                      DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                      Alphas(1),
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Sensor,
                                      NodeInputManager::CompFluidStream::Primary,
                                      ObjectIsNotParent);
                state.dataTranspiredCollector->UTSC(Item).ZoneNode(1) =
                    GetOnlySingleNode(state,
                                      Alphas(7),
                                      ErrorsFound,
                                      DataLoopNode::ConnectionObjectType::SolarCollectorUnglazedTranspired,
                                      Alphas(1),
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Sensor,
                                      NodeInputManager::CompFluidStream::Primary,
                                      ObjectIsNotParent);
            } // no splitter

            if (state.dataIPShortCut->lAlphaFieldBlanks(8)) {
                ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8));
                ErrorsFound = true;
            } else if ((state.dataTranspiredCollector->UTSC(Item).freeHeatSetPointSched = Sched::GetSchedule(state, Alphas(8))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8), Alphas(8));
                ErrorsFound = true;
                continue;
            }

            if (Util::SameString(Alphas(9), "Triangle")) {
                state.dataTranspiredCollector->UTSC(Item).Layout = Layout_Triangle;
            } else if (Util::SameString(Alphas(9), "Square")) {
                state.dataTranspiredCollector->UTSC(Item).Layout = Layout_Square;
            } else {
                ShowSevereError(state,
                                format("{} has incorrect entry of {} in {} ={}",
                                       state.dataIPShortCut->cAlphaFieldNames(9),
                                       Alphas(9),
                                       CurrentModuleObject,
                                       state.dataTranspiredCollector->UTSC(Item).Name));
                ErrorsFound = true;
                continue;
            }

            if (Util::SameString(Alphas(10), "Kutscher1994")) {
                state.dataTranspiredCollector->UTSC(Item).Correlation = Correlation_Kutscher1994;
            } else if (Util::SameString(Alphas(10), "VanDeckerHollandsBrunger2001")) {
                state.dataTranspiredCollector->UTSC(Item).Correlation = Correlation_VanDeckerHollandsBrunger2001;
            } else {
                ShowSevereError(state,
                                format("{} has incorrect entry of {} in {} ={}",
                                       state.dataIPShortCut->cAlphaFieldNames(10),
                                       Alphas(9),
                                       CurrentModuleObject,
                                       state.dataTranspiredCollector->UTSC(Item).Name));
                ErrorsFound = true;
                continue;
            }

            Roughness = Alphas(11);
            // Select the correct Number for the associated ascii name for the roughness type
            if (Util::SameString(Roughness, "VeryRough")) {
                state.dataTranspiredCollector->UTSC(Item).CollRoughness = Material::SurfaceRoughness::VeryRough;
            }
            if (Util::SameString(Roughness, "Rough")) {
                state.dataTranspiredCollector->UTSC(Item).CollRoughness = Material::SurfaceRoughness::Rough;
            }
            if (Util::SameString(Roughness, "MediumRough")) {
                state.dataTranspiredCollector->UTSC(Item).CollRoughness = Material::SurfaceRoughness::MediumRough;
            }
            if (Util::SameString(Roughness, "MediumSmooth")) {
                state.dataTranspiredCollector->UTSC(Item).CollRoughness = Material::SurfaceRoughness::MediumSmooth;
            }
            if (Util::SameString(Roughness, "Smooth")) {
                state.dataTranspiredCollector->UTSC(Item).CollRoughness = Material::SurfaceRoughness::Smooth;
            }
            if (Util::SameString(Roughness, "VerySmooth")) {
                state.dataTranspiredCollector->UTSC(Item).CollRoughness = Material::SurfaceRoughness::VerySmooth;
            }

            // Was it set?
            if (state.dataTranspiredCollector->UTSC(Item).CollRoughness == Material::SurfaceRoughness::Invalid) {
                ShowSevereError(state,
                                format("{} has incorrect entry of {} in {} ={}",
                                       state.dataIPShortCut->cAlphaFieldNames(11),
                                       Alphas(11),
                                       CurrentModuleObject,
                                       state.dataTranspiredCollector->UTSC(Item).Name));
                ErrorsFound = true;
            }

            AlphaOffset = 11;
            state.dataTranspiredCollector->UTSC(Item).NumSurfs = NumAlphas - AlphaOffset;
            if (state.dataTranspiredCollector->UTSC(Item).NumSurfs == 0) {
                ShowSevereError(
                    state, format("No underlying surfaces specified in {} ={}", CurrentModuleObject, state.dataTranspiredCollector->UTSC(Item).Name));
                ErrorsFound = true;
                continue;
            }
            state.dataTranspiredCollector->UTSC(Item).SurfPtrs.allocate(state.dataTranspiredCollector->UTSC(Item).NumSurfs);
            state.dataTranspiredCollector->UTSC(Item).SurfPtrs = 0;
            for (ThisSurf = 1; ThisSurf <= state.dataTranspiredCollector->UTSC(Item).NumSurfs; ++ThisSurf) {
                Found = Util::FindItemInList(Alphas(ThisSurf + AlphaOffset), state.dataSurface->Surface);
                if (Found == 0) {
                    ShowSevereError(state,
                                    format("Surface Name not found={} in {} ={}",
                                           Alphas(ThisSurf + AlphaOffset),
                                           CurrentModuleObject,
                                           state.dataTranspiredCollector->UTSC(Item).Name));
                    ErrorsFound = true;
                    continue;
                }
                // check that surface is appropriate, Heat transfer, Sun, Wind,
                if (!state.dataSurface->Surface(Found).HeatTransSurf) {
                    ShowSevereError(state,
                                    format("Surface {} not of Heat Transfer type in {} ={}",
                                           Alphas(ThisSurf + AlphaOffset),
                                           CurrentModuleObject,
                                           state.dataTranspiredCollector->UTSC(Item).Name));
                    ErrorsFound = true;
                    continue;
                }
                if (!state.dataSurface->Surface(Found).ExtSolar) {
                    ShowSevereError(state,
                                    format("Surface {} not exposed to sun in {} ={}",
                                           Alphas(ThisSurf + AlphaOffset),
                                           CurrentModuleObject,
                                           state.dataTranspiredCollector->UTSC(Item).Name));
                    ErrorsFound = true;
                    continue;
                }
                if (!state.dataSurface->Surface(Found).ExtWind) {
                    ShowSevereError(state,
                                    format("Surface {} not exposed to wind in {} ={}",
                                           Alphas(ThisSurf + AlphaOffset),
                                           CurrentModuleObject,
                                           state.dataTranspiredCollector->UTSC(Item).Name));
                    ErrorsFound = true;
                    continue;
                }
                if (state.dataSurface->Surface(Found).ExtBoundCond != OtherSideCondModeledExt) {
                    ShowSevereError(state,
                                    format("Surface {} does not have OtherSideConditionsModel for exterior boundary conditions in {} ={}",
                                           Alphas(ThisSurf + AlphaOffset),
                                           CurrentModuleObject,
                                           state.dataTranspiredCollector->UTSC(Item).Name));
                    ErrorsFound = true;
                    continue;
                }
                // check surface orientation, warn if upside down
                if ((state.dataSurface->Surface(Found).Tilt < -95.0) || (state.dataSurface->Surface(Found).Tilt > 95.0)) {
                    ShowWarningError(state, format("Suspected input problem with collector surface = {}", Alphas(ThisSurf + AlphaOffset)));
                    ShowContinueError(
                        state,
                        format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, state.dataTranspiredCollector->UTSC(Item).Name));
                    ShowContinueError(state, "Surface used for solar collector faces down");
                    ShowContinueError(
                        state, format("Surface tilt angle (degrees from ground outward normal) = {:.2R}", state.dataSurface->Surface(Found).Tilt));
                }

                state.dataTranspiredCollector->UTSC(Item).SurfPtrs(ThisSurf) = Found;
            }

            if (ErrorsFound) {
                continue; // previous inner do loop may have detected problems that need to be cycle'd again to avoid crash
            }

            // now that we should have all the surfaces, do some preparations and checks.

            // are they all similar tilt and azimuth? Issue warnings so people can do it if they really want
            Real64 const surfaceArea(sum_sub(state.dataSurface->Surface, &SurfaceData::Area, state.dataTranspiredCollector->UTSC(Item).SurfPtrs));
            //            AvgAzimuth = sum( Surface( UTSC( Item ).SurfPtrs ).Azimuth * Surface( UTSC( Item ).SurfPtrs ).Area ) / sum( Surface(
            // UTSC(  Item
            //).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
            AvgAzimuth =
                sum_product_sub(
                    state.dataSurface->Surface, &SurfaceData::Azimuth, &SurfaceData::Area, state.dataTranspiredCollector->UTSC(Item).SurfPtrs) /
                surfaceArea; // Autodesk:F2C++ Functions handle array subscript usage
            //            AvgTilt = sum( Surface( UTSC( Item ).SurfPtrs ).Tilt * Surface( UTSC( Item ).SurfPtrs ).Area ) / sum( Surface( UTSC(
            // Item
            //).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
            AvgTilt = sum_product_sub(
                          state.dataSurface->Surface, &SurfaceData::Tilt, &SurfaceData::Area, state.dataTranspiredCollector->UTSC(Item).SurfPtrs) /
                      surfaceArea; // Autodesk:F2C++ Functions handle array subscript usage
            for (ThisSurf = 1; ThisSurf <= state.dataTranspiredCollector->UTSC(Item).NumSurfs; ++ThisSurf) {
                SurfID = state.dataTranspiredCollector->UTSC(Item).SurfPtrs(ThisSurf);
                if (General::rotAzmDiffDeg(state.dataSurface->Surface(SurfID).Azimuth, AvgAzimuth) > 15.0) {
                    ShowWarningError(state,
                                     format("Surface {} has Azimuth different from others in the group associated with {} ={}",
                                            state.dataSurface->Surface(SurfID).Name,
                                            CurrentModuleObject,
                                            state.dataTranspiredCollector->UTSC(Item).Name));
                }
                if (std::abs(state.dataSurface->Surface(SurfID).Tilt - AvgTilt) > 10.0) {
                    ShowWarningError(state,
                                     format("Surface {} has Tilt different from others in the group associated with {} ={}",
                                            state.dataSurface->Surface(SurfID).Name,
                                            CurrentModuleObject,
                                            state.dataTranspiredCollector->UTSC(Item).Name));
                }

                // test that there are no windows.  Now allow windows
                // If (Surface(SurfID)%GrossArea >  Surface(SurfID)%Area) Then
                //      Call ShowWarningError(state, 'Surface '//TRIM(Surface(SurfID)%name)//' has a subsurface whose area is not being ' &
                //         //'subtracted in the group of surfaces associated with '//TRIM(UTSC(Item)%Name))
                // endif
            }
            state.dataTranspiredCollector->UTSC(Item).Tilt = AvgTilt;
            state.dataTranspiredCollector->UTSC(Item).Azimuth = AvgAzimuth;

            // find area weighted centroid.
            //    UTSC(Item)%Centroid%x = SUM(Surface(UTSC(Item)%SurfPtrs)%Centroid%x*Surface(UTSC(Item)%SurfPtrs)%Area) &
            //                            /SUM(Surface(UTSC(Item)%SurfPtrs)%Area)
            //    UTSC(Item)%Centroid%y = SUM(Surface(UTSC(Item)%SurfPtrs)%Centroid%y*Surface(UTSC(Item)%SurfPtrs)%Area) &
            //                            /SUM(Surface(UTSC(Item)%SurfPtrs)%Area)
            //            UTSC( Item ).Centroid.z = sum( Surface( UTSC( Item ).SurfPtrs ).Centroid.z * Surface( UTSC( Item ).SurfPtrs ).Area ) /
            // sum(  Surface( UTSC( Item ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
            state.dataTranspiredCollector->UTSC(Item).Centroid.z = sum_product_sub(state.dataSurface->Surface,
                                                                                   &SurfaceData::Centroid,
                                                                                   &Vector::z,
                                                                                   state.dataSurface->Surface,
                                                                                   &SurfaceData::Area,
                                                                                   state.dataTranspiredCollector->UTSC(Item).SurfPtrs) /
                                                                   surfaceArea; // Autodesk:F2C++ Functions handle array subscript usage

            // now handle numbers from input object
            state.dataTranspiredCollector->UTSC(Item).HoleDia = Numbers(1);
            state.dataTranspiredCollector->UTSC(Item).Pitch = Numbers(2);
            state.dataTranspiredCollector->UTSC(Item).LWEmitt = Numbers(3);
            state.dataTranspiredCollector->UTSC(Item).SolAbsorp = Numbers(4);
            state.dataTranspiredCollector->UTSC(Item).Height = Numbers(5);
            state.dataTranspiredCollector->UTSC(Item).PlenGapThick = Numbers(6);
            if (state.dataTranspiredCollector->UTSC(Item).PlenGapThick <= 0.0) {
                ShowSevereError(
                    state,
                    format("Plenum gap must be greater than Zero in {} ={}", CurrentModuleObject, state.dataTranspiredCollector->UTSC(Item).Name));
                continue;
            }
            state.dataTranspiredCollector->UTSC(Item).PlenCrossArea = Numbers(7);
            state.dataTranspiredCollector->UTSC(Item).AreaRatio = Numbers(8);
            state.dataTranspiredCollector->UTSC(Item).CollectThick = Numbers(9);
            state.dataTranspiredCollector->UTSC(Item).Cv = Numbers(10);
            state.dataTranspiredCollector->UTSC(Item).Cd = Numbers(11);

            // Fill out data we now know
            // sum areas of HT surface areas
            //            UTSC( Item ).ProjArea = sum( Surface( UTSC( Item ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced
            // by  below
            state.dataTranspiredCollector->UTSC(Item).ProjArea = surfaceArea;
            if (state.dataTranspiredCollector->UTSC(Item).ProjArea == 0) {
                ShowSevereError(state,
                                format("Gross area of underlying surfaces is zero in {} ={}",
                                       CurrentModuleObject,
                                       state.dataTranspiredCollector->UTSC(Item).Name));
                continue;
            }
            state.dataTranspiredCollector->UTSC(Item).ActualArea =
                state.dataTranspiredCollector->UTSC(Item).ProjArea * state.dataTranspiredCollector->UTSC(Item).AreaRatio;
            //  need to update this for slots as well as holes
            switch (state.dataTranspiredCollector->UTSC(Item).Layout) {
            case Layout_Triangle: { // 'TRIANGLE'
                state.dataTranspiredCollector->UTSC(Item).Porosity =
                    0.907 * pow_2(state.dataTranspiredCollector->UTSC(Item).HoleDia /
                                  state.dataTranspiredCollector->UTSC(Item).Pitch); // Kutscher equation, Triangle layout
            } break;
            case Layout_Square: { // 'SQUARE'
                state.dataTranspiredCollector->UTSC(Item).Porosity =
                    (Constant::Pi / 4.0) * pow_2(state.dataTranspiredCollector->UTSC(Item).HoleDia) /
                    pow_2(state.dataTranspiredCollector->UTSC(Item).Pitch); // Waterloo equation, square layout
            } break;
            default:
                break;
            }
            TiltRads = std::abs(AvgTilt) * Constant::DegToRad;
            tempHdeltaNPL = std::sin(TiltRads) * state.dataTranspiredCollector->UTSC(Item).Height / 4.0;
            state.dataTranspiredCollector->UTSC(Item).HdeltaNPL = max(tempHdeltaNPL, state.dataTranspiredCollector->UTSC(Item).PlenGapThick);

            SetupOutputVariable(state,
                                "Solar Collector Heat Exchanger Effectiveness",
                                Constant::Units::None,
                                state.dataTranspiredCollector->UTSC(Item).HXeff,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Leaving Air Temperature",
                                Constant::Units::C,
                                state.dataTranspiredCollector->UTSC(Item).TairHX,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Outside Face Suction Velocity",
                                Constant::Units::m_s,
                                state.dataTranspiredCollector->UTSC(Item).Vsuction,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Surface Temperature",
                                Constant::Units::C,
                                state.dataTranspiredCollector->UTSC(Item).Tcoll,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Plenum Air Temperature",
                                Constant::Units::C,
                                state.dataTranspiredCollector->UTSC(Item).Tplen,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Sensible Heating Rate",
                                Constant::Units::W,
                                state.dataTranspiredCollector->UTSC(Item).SensHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Sensible Heating Energy",
                                Constant::Units::J,
                                state.dataTranspiredCollector->UTSC(Item).SensHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataTranspiredCollector->UTSC(Item).Name,
                                Constant::eResource::SolarAir,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatProduced);

            SetupOutputVariable(state,
                                "Solar Collector Natural Ventilation Air Change Rate",
                                Constant::Units::ach,
                                state.dataTranspiredCollector->UTSC(Item).PassiveACH,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Natural Ventilation Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataTranspiredCollector->UTSC(Item).PassiveMdotVent,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Wind Natural Ventilation Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataTranspiredCollector->UTSC(Item).PassiveMdotWind,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Buoyancy Natural Ventilation Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataTranspiredCollector->UTSC(Item).PassiveMdotTherm,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Incident Solar Radiation",
                                Constant::Units::W_m2,
                                state.dataTranspiredCollector->UTSC(Item).Isc,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector System Efficiency",
                                Constant::Units::None,
                                state.dataTranspiredCollector->UTSC(Item).UTSCEfficiency,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
            SetupOutputVariable(state,
                                "Solar Collector Surface Efficiency",
                                Constant::Units::None,
                                state.dataTranspiredCollector->UTSC(Item).UTSCCollEff,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataTranspiredCollector->UTSC(Item).Name);
        }

        for (ItemSplit = 1; ItemSplit <= NumUTSCSplitter; ++ItemSplit) {
            if (!SplitterNameOK(ItemSplit)) {
                ShowSevereError(state, "Did not find a match, check names for Solar Collectors:Transpired Collector:Multisystem");
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "GetTranspiredCollectorInput: Errors found in input");
        }

        Alphas.deallocate();
    }

    void InitTranspiredCollector(EnergyPlusData &state, int const UTSCNum) // compindex already checked in calling routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       B. Griffith, May 2009, added EMS setpoint check
        //       RE-ENGINEERED  na

        // Using/Aliasing
        bool DoSetPointTest = state.dataHVACGlobal->DoSetPointTest;
        using namespace DataLoopNode;
        using DataSurfaces::SurfaceData;
        using EMSManager::CheckIfNodeSetPointManagedByEMS;

        int UTSCUnitNum;
        int ControlNode;
        int SplitBranch;
        int thisUTSC;
        Real64 Tamb;

        if (state.dataTranspiredCollector->MyOneTimeFlag) {
            // do various one time setups and pitch adjustments across all UTSC
            for (thisUTSC = 1; thisUTSC <= state.dataTranspiredCollector->NumUTSC; ++thisUTSC) {
                if (state.dataTranspiredCollector->UTSC(thisUTSC).Layout == Layout_Triangle) {
                    switch (state.dataTranspiredCollector->UTSC(thisUTSC).Correlation) {
                    case Correlation_Kutscher1994: { // Kutscher1994
                        state.dataTranspiredCollector->UTSC(thisUTSC).Pitch = state.dataTranspiredCollector->UTSC(thisUTSC).Pitch;
                    } break;
                    case Correlation_VanDeckerHollandsBrunger2001: { // VanDeckerHollandsBrunger2001
                        state.dataTranspiredCollector->UTSC(thisUTSC).Pitch /= 1.6;
                    } break;
                    default:
                        break;
                    }
                }
                if (state.dataTranspiredCollector->UTSC(thisUTSC).Layout == Layout_Square) {
                    switch (state.dataTranspiredCollector->UTSC(thisUTSC).Correlation) {
                    case Correlation_Kutscher1994: { // Kutscher1994
                        state.dataTranspiredCollector->UTSC(thisUTSC).Pitch *= 1.6;
                    } break;
                    case Correlation_VanDeckerHollandsBrunger2001: { // VanDeckerHollandsBrunger2001
                        state.dataTranspiredCollector->UTSC(thisUTSC).Pitch = state.dataTranspiredCollector->UTSC(thisUTSC).Pitch;
                    } break;
                    default:
                        break;
                    }
                }
            }

            state.dataTranspiredCollector->MyEnvrnFlag.dimension(state.dataTranspiredCollector->NumUTSC, true);
            state.dataTranspiredCollector->MyOneTimeFlag = false;
        } // first time

        // Check that setpoint is active (from test by RJL in HVACEvapComponent)
        if (!state.dataGlobal->SysSizingCalc && state.dataTranspiredCollector->MySetPointCheckFlag && DoSetPointTest) {
            for (UTSCUnitNum = 1; UTSCUnitNum <= state.dataTranspiredCollector->NumUTSC; ++UTSCUnitNum) {
                for (SplitBranch = 1; SplitBranch <= state.dataTranspiredCollector->UTSC(UTSCUnitNum).NumOASysAttached; ++SplitBranch) {
                    ControlNode = state.dataTranspiredCollector->UTSC(UTSCUnitNum).ControlNode(SplitBranch);
                    if (ControlNode > 0) {
                        if (state.dataLoopNodes->Node(ControlNode).TempSetPoint == SensedNodeFlagValue) {
                            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                                ShowSevereError(
                                    state, format("Missing temperature setpoint for UTSC {}", state.dataTranspiredCollector->UTSC(UTSCUnitNum).Name));
                                ShowContinueError(state, " use a Setpoint Manager to establish a setpoint at the unit control node.");
                                state.dataHVACGlobal->SetPointErrorFlag = true;
                            } else {
                                // need call to EMS to check node
                                CheckIfNodeSetPointManagedByEMS(state, ControlNode, HVAC::CtrlVarType::Temp, state.dataHVACGlobal->SetPointErrorFlag);
                                if (state.dataHVACGlobal->SetPointErrorFlag) {
                                    ShowSevereError(
                                        state,
                                        format("Missing temperature setpoint for UTSC {}", state.dataTranspiredCollector->UTSC(UTSCUnitNum).Name));
                                    ShowContinueError(state, " use a Setpoint Manager to establish a setpoint at the unit control node.");
                                    ShowContinueError(state, "Or add EMS Actuator to provide temperature setpoint at this node");
                                }
                            }
                        }
                    }
                }
            }
            state.dataTranspiredCollector->MySetPointCheckFlag = false;
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataTranspiredCollector->MyEnvrnFlag(UTSCNum)) {
            state.dataTranspiredCollector->UTSC(UTSCNum).TplenLast = 22.5;
            state.dataTranspiredCollector->UTSC(UTSCNum).TcollLast = 22.0;

            state.dataTranspiredCollector->MyEnvrnFlag(UTSCNum) = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataTranspiredCollector->MyEnvrnFlag(UTSCNum) = true;
        }

        // determine average ambient temperature
        Real64 sum_area = 0.0;
        for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
            sum_area += state.dataSurface->Surface(SurfNum).Area;
        }
        if (!state.dataEnvrn->IsRain) {
            Real64 sum_produc_area_drybulb = 0.0;
            for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
                sum_produc_area_drybulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutDryBulbTemp(SurfNum);
            }
            Tamb = sum_produc_area_drybulb / sum_area;
        } else { // when raining we use wet bulb not drybulb
            Real64 sum_produc_area_wetbulb = 0.0;
            for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
                sum_produc_area_wetbulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutWetBulbTemp(SurfNum);
            }
            Tamb = sum_produc_area_wetbulb / sum_area;
        }

        // inits for each iteration
        //        UTSC( UTSCNum ).InletMDot = sum( Node( UTSC( UTSCNum ).InletNode ).MassFlowRate ); //Autodesk:F2C++ Array subscript usage:
        // Replaced by below
        state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot =
            sum_sub(state.dataLoopNodes->Node,
                    &DataLoopNode::NodeData::MassFlowRate,
                    state.dataTranspiredCollector->UTSC(UTSCNum).InletNode); // Autodesk:F2C++ Functions handle array subscript usage
        state.dataTranspiredCollector->UTSC(UTSCNum).IsOn = false;           // initialize then turn on if appropriate
        state.dataTranspiredCollector->UTSC(UTSCNum).Tplen = state.dataTranspiredCollector->UTSC(UTSCNum).TplenLast;
        state.dataTranspiredCollector->UTSC(UTSCNum).Tcoll = state.dataTranspiredCollector->UTSC(UTSCNum).TcollLast;
        state.dataTranspiredCollector->UTSC(UTSCNum).TairHX = Tamb;
        state.dataTranspiredCollector->UTSC(UTSCNum).MdotVent = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).HXeff = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).Isc = 0.0;

        state.dataTranspiredCollector->UTSC(UTSCNum).UTSCEfficiency = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).UTSCCollEff = 0.0;
    }

    void CalcActiveTranspiredCollector(EnergyPlusData &state, int const UTSCNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        using DataSurfaces::SurfaceData;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using namespace DataHeatBalance; // , ONLY: SurfQRadSWOutIncident, Construct, Material

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr nu(15.66e-6); // kinematic viscosity (m**2/s) for air at 300 K
        // (Mills 1999 Heat Transfer)
        Real64 constexpr k(0.0267); // thermal conductivity (W/m K) for air at 300 K
        // (Mills 1999 Heat Transfer)
        Real64 constexpr Sigma(5.6697e-08); // Stefan-Boltzmann constant

        // following arrays are used to temporarily hold results from multiple underlying surfaces
        Array1D<Real64> HSkyARR;
        Array1D<Real64> HGroundARR;
        Array1D<Real64> HAirARR;
        Array1D<Real64> HPlenARR;
        Array1D<Real64> LocalWindArr;
        Array1D<Real64> HSrdSurfARR;

        // working variables
        Real64 RhoAir;                        // density of air
        Real64 CpAir;                         // specific heat of air
        Real64 holeArea;                      // area of perforations, includes corrugation of surface
        Real64 Tamb;                          // outdoor drybulb
        Real64 A;                             // projected area of collector, from sum of underlying surfaces
        Real64 Vholes;                        // mean velocity of air as it passes through collector holes
        Real64 Vsuction;                      // mean velocity of air as is approaches the collector
        Real64 Vplen;                         // mean velocity of air inside plenum
        Real64 HcPlen;                        // surface convection heat transfer coefficient for plenum surfaces
        Real64 D;                             // hole diameter
        Real64 ReD;                           // Reynolds number for holes
        Real64 P;                             // pitch, distance between holes
        Real64 Por;                           // porosity, area fraction of collector that is open because of holes
        Real64 Mdot;                          // mass flow rate of suction air
        Real64 QdotSource;                    // energy flux for source/sink inside collector surface (for hybrid PV UTSC)
        int ThisSurf;                         // do loop counter
        int NumSurfs;                         // number of underlying HT surfaces associated with UTSC
        Material::SurfaceRoughness Roughness; // parameters for surface roughness, defined in DataHeatBalance
        Real64 SolAbs;                        // solar absorptivity of collector
        Real64 AbsExt;                        // thermal emittance of collector
        Real64 TempExt;                       // collector temperature
        int SurfPtr;                          // index of surface in main surface structure
        Real64 HMovInsul;                     // dummy for call to InitExteriorConvectionCoeff
        Real64 HExt;                          // dummy for call to InitExteriorConvectionCoeff
        int ConstrNum;                        // index of construction in main construction structure
        Real64 AbsThermSurf;                  // thermal emittance of underlying wall.
        Real64 TsoK;                          // underlying surface temperature in Kelvin
        Real64 TscollK;                       // collector temperature in Kelvin  (lagged)
        Real64 AreaSum;                       // sum of contributing surfaces for area-weighted averages.
        Real64 Vwind;                         // localized, and area-weighted average for wind speed
        Real64 HrSky;                         // radiation coeff for sky, area-weighted average
        Real64 HrGround;                      // radiation coeff for ground, area-weighted average
        Real64 HrAtm;                         // radiation coeff for air (bulk atmosphere), area-weighted average
        Real64 Isc;                           // Incoming combined solar radiation, area-weighted average
        Real64 HrPlen;                        // radiation coeff for plenum surfaces, area-weighted average
        Real64 Tso;                           // temperature of underlying surface, area-weighted average
        Real64 HcWind;                        // convection coeff for high speed wind situations
        Real64 NuD;                           // nusselt number for Reynolds based on hole
        Real64 U;                             // overall heat exchanger coefficient
        Real64 HXeff;                         // effectiveness for heat exchanger
        Real64 t;                             // collector thickness
        Real64 ReS;                           // Reynolds number based on suction velocity and pitch
        Real64 ReW;                           // Reynolds number based on Wind and pitch
        Real64 ReB;                           // Reynolds number based on hole velocity and pitch
        Real64 ReH;                           // Reynolds number based on hole velocity and diameter
        Real64 Tscoll;                        // temperature of collector
        Real64 TaHX;                          // leaving air temperature from heat exchanger (entering plenum)
        Real64 Taplen;                        // Air temperature in plen and outlet node.
        Real64 SensHeatingRate;               // Rate at which the system is heating outdoor air
        Real64 AlessHoles;                    // Area for Kutscher's relation

        auto &s_mat = state.dataMaterial;
        // Active UTSC calculation
        // first do common things for both correlations
        Real64 sum_area = 0.0;
        for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
            sum_area += state.dataSurface->Surface(SurfNum).Area;
        }
        if (!state.dataEnvrn->IsRain) {
            Real64 sum_produc_area_drybulb = 0.0;
            for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
                sum_produc_area_drybulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutDryBulbTemp(SurfNum);
            }
            Tamb = sum_produc_area_drybulb / sum_area;
        } else { // when raining we use wet bulb not drybulb
            Real64 sum_produc_area_wetbulb = 0.0;
            for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
                sum_produc_area_wetbulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutWetBulbTemp(SurfNum);
            }
            Tamb = sum_produc_area_wetbulb / sum_area;
        }

        RhoAir = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, Tamb, state.dataEnvrn->OutHumRat);

        CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);

        holeArea = state.dataTranspiredCollector->UTSC(UTSCNum).ActualArea * state.dataTranspiredCollector->UTSC(UTSCNum).Porosity;

        A = state.dataTranspiredCollector->UTSC(UTSCNum).ProjArea;

        Vholes = state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot / RhoAir / holeArea;

        Vplen = state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot / RhoAir / state.dataTranspiredCollector->UTSC(UTSCNum).PlenCrossArea;

        Vsuction = state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot / RhoAir / A;

        if ((Vsuction < 0.001) || (Vsuction > 0.08)) { // warn that collector is not sized well
            if (state.dataTranspiredCollector->UTSC(UTSCNum).VsucErrIndex == 0) {
                ShowWarningMessage(state,
                                   format("Solar Collector:Unglazed Transpired=\"{}\", Suction velocity is outside of range for a good design",
                                          state.dataTranspiredCollector->UTSC(UTSCNum).Name));
                ShowContinueErrorTimeStamp(state, format("Suction velocity ={:.4R}", Vsuction));
                if (Vsuction < 0.003) {
                    ShowContinueError(state, "Velocity is low -- suggest decreasing area of transpired collector");
                }
                if (Vsuction > 0.08) {
                    ShowContinueError(state, "Velocity is high -- suggest increasing area of transpired collector");
                }
                ShowContinueError(state, "Occasional suction velocity messages are not unexpected when simulating actual conditions");
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "Solar Collector:Unglazed Transpired=\"" + state.dataTranspiredCollector->UTSC(UTSCNum).Name +
                                               "\", Suction velocity is outside of range",
                                           state.dataTranspiredCollector->UTSC(UTSCNum).VsucErrIndex,
                                           Vsuction,
                                           Vsuction,
                                           _,
                                           "[m/s]",
                                           "[m/s]");
        }

        HcPlen = 5.62 + 3.92 * Vplen;

        D = state.dataTranspiredCollector->UTSC(UTSCNum).HoleDia;

        ReD = Vholes * D / nu;

        P = state.dataTranspiredCollector->UTSC(UTSCNum).Pitch;

        Por = state.dataTranspiredCollector->UTSC(UTSCNum).Porosity;

        Mdot = state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot;

        QdotSource = state.dataTranspiredCollector->UTSC(UTSCNum).QdotSource; // for hybrid PV transpired collectors

        // loop through underlying surfaces and collect needed data
        // now collect average values for things associated with the underlying surface(s)
        NumSurfs = state.dataTranspiredCollector->UTSC(UTSCNum).NumSurfs;
        HSkyARR.dimension(NumSurfs, 0.0);
        HGroundARR.dimension(NumSurfs, 0.0);
        HAirARR.dimension(NumSurfs, 0.0);
        LocalWindArr.dimension(NumSurfs, 0.0);
        // ALLOCATE(IscARR(NumSurfs))
        // IscARR = 0.0
        HPlenARR.dimension(NumSurfs, 0.0);
        //  ALLOCATE(TsoARR(NumSurfs))
        //  TsoARR = 0.0
        HSrdSurfARR.dimension(NumSurfs, 0.0);

        Roughness = state.dataTranspiredCollector->UTSC(UTSCNum).CollRoughness;
        SolAbs = state.dataTranspiredCollector->UTSC(UTSCNum).SolAbsorp;
        AbsExt = state.dataTranspiredCollector->UTSC(UTSCNum).LWEmitt;
        TempExt = state.dataTranspiredCollector->UTSC(UTSCNum).TcollLast;
        for (ThisSurf = 1; ThisSurf <= NumSurfs; ++ThisSurf) {
            SurfPtr = state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs(ThisSurf);
            // Initializations for this surface
            HMovInsul = 0.0;
            HExt = 0.0;
            LocalWindArr(ThisSurf) = state.dataSurface->SurfOutWindSpeed(SurfPtr);
            Convect::InitExtConvCoeff(state,
                                      SurfPtr,
                                      HMovInsul,
                                      Roughness,
                                      AbsExt,
                                      TempExt,
                                      HExt,
                                      HSkyARR(ThisSurf),
                                      HGroundARR(ThisSurf),
                                      HAirARR(ThisSurf),
                                      HSrdSurfARR(ThisSurf));
            ConstrNum = state.dataSurface->Surface(SurfPtr).Construction;
            AbsThermSurf = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1))->AbsorpThermal;
            TsoK = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfPtr) + Constant::Kelvin;
            TscollK = state.dataTranspiredCollector->UTSC(UTSCNum).TcollLast + Constant::Kelvin;
            HPlenARR(ThisSurf) = Sigma * AbsExt * AbsThermSurf * (pow_4(TscollK) - pow_4(TsoK)) / (TscollK - TsoK);
        }
        //        AreaSum = sum( Surface( UTSC( UTSCNum ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
        auto Area = // (AUTO_OK_OBJ)
            array_sub(state.dataSurface->Surface,
                      &SurfaceData::Area,
                      state.dataTranspiredCollector->UTSC(UTSCNum)
                          .SurfPtrs); // Autodesk:F2C++ Copy of subscripted Area array for use below: This makes a copy so review wrt performance
        AreaSum = sum(Area);
        // now figure area-weighted averages from underlying surfaces.
        //        Vwind = sum( LocalWindArr * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum; //Autodesk:F2C++ Array subscript usage:
        // Replaced by below
        Vwind = sum(LocalWindArr * Area) / AreaSum;
        LocalWindArr.deallocate();
        //        HrSky = sum( HSkyARR * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum; //Autodesk:F2C++ Array subscript usage: Replaced
        // by  below
        HrSky = sum(HSkyARR * Area) / AreaSum;
        HSkyARR.deallocate();
        //        HrGround = sum( HGroundARR * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum; //Autodesk:F2C++ Array subscript usage:
        // Replaced by below
        HrGround = sum(HGroundARR * Area) / AreaSum;
        HGroundARR.deallocate();
        //        HrAtm = sum( HAirARR * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum; //Autodesk:F2C++ Array subscript usage: Replaced
        // by  below
        HrAtm = sum(HAirARR * Area) / AreaSum;
        HAirARR.deallocate();
        //        HrPlen = sum( HPlenARR * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum; //Autodesk:F2C++ Array subscript usage:
        // Replaced  by below
        HrPlen = sum(HPlenARR * Area) / AreaSum;
        HPlenARR.deallocate();

        //        Isc = sum( SurfQRadSWOutIncident( UTSC( UTSCNum ).SurfPtrs ) * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum;
        ////Autodesk:F2C++ Array subscript usage: Replaced by below
        Isc = sum_product_sub(state.dataHeatBal->SurfQRadSWOutIncident,
                              state.dataSurface->Surface,
                              &SurfaceData::Area,
                              state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) /
              AreaSum; // Autodesk:F2C++ Functions handle array subscript usage
        //        Tso = sum( TH( UTSC( UTSCNum ).SurfPtrs, 1, 1 ) * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / AreaSum; //Autodesk:F2C++ Array
        // subscript usage: Replaced by below
        Tso = sum_product_sub(state.dataHeatBalSurf->SurfOutsideTempHist(1),
                              state.dataSurface->Surface,
                              &SurfaceData::Area,
                              state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) /
              AreaSum; // Autodesk:F2C++ Functions handle array subscript usage

        if (Vwind > 5.0) {
            HcWind = 5.62 + 3.9 * (Vwind - 5.0); // McAdams forced convection correlation
        } else {
            HcWind = 0.0;
        }

        if (state.dataEnvrn->IsRain) {
            HcWind = 1000.0;
        }

        HXeff = 0.0; // init

        switch (state.dataTranspiredCollector->UTSC(UTSCNum).Correlation) {
        case Correlation_Kutscher1994: { // Kutscher1994
            AlessHoles = A - holeArea;

            NuD = 2.75 * ((std::pow(P / D, -1.2) * std::pow(ReD, 0.43)) + (0.011 * Por * ReD * std::pow(Vwind / Vsuction, 0.48)));
            U = k * NuD / D;
            HXeff = 1.0 - std::exp(-1.0 * ((U * AlessHoles) / (Mdot * CpAir)));
        } break;
        case Correlation_VanDeckerHollandsBrunger2001: { // VanDeckerHollandsBrunger2001
            t = state.dataTranspiredCollector->UTSC(UTSCNum).CollectThick;
            ReS = Vsuction * P / nu;
            ReW = Vwind * P / nu;
            ReB = Vholes * P / nu;
            ReH = (Vsuction * D) / (nu * Por);
            if (ReD > 0.0) {
                if (ReW > 0.0) {
                    HXeff = (1.0 - std::pow(1.0 + ReS * max(1.733 * std::pow(ReW, -0.5), 0.02136), -1.0)) *
                            (1.0 - std::pow(1.0 + 0.2273 * std::sqrt(ReB), -1.0)) * std::exp(-0.01895 * (P / D) - (20.62 / ReH) * (t / D));
                } else {
                    HXeff = (1.0 - std::pow(1.0 + ReS * 0.02136, -1.0)) * (1.0 - std::pow(1.0 + 0.2273 * std::sqrt(ReB), -1.0)) *
                            std::exp(-0.01895 * (P / D) - (20.62 / ReH) * (t / D));
                }
            } else {
                HXeff = 0.0;
            }
        } break;
        default:
            break;
        }

        // now calculate collector temperature

        Tscoll = (Isc * SolAbs + HrAtm * Tamb + HrSky * state.dataEnvrn->SkyTemp + HrGround * Tamb + HrPlen * Tso + HcWind * Tamb +
                  (Mdot * CpAir / A) * Tamb - (Mdot * CpAir / A) * (1.0 - HXeff) * Tamb + QdotSource) /
                 (HrAtm + HrSky + HrGround + HrPlen + HcWind + (Mdot * CpAir / A) * HXeff);

        // Heat exchanger leaving temperature
        TaHX = HXeff * Tscoll + (1.0 - HXeff) * Tamb;

        // now calculate plenum air temperature

        Taplen = (Mdot * CpAir * TaHX + HcPlen * A * Tso) / (Mdot * CpAir + HcPlen * A);

        // calculate Sensible Heating Rate
        if (Taplen > Tamb) {
            SensHeatingRate = Mdot * CpAir * (Taplen - Tamb);
        } else {
            SensHeatingRate = 0.0;
        }

        // now fill results into derived types
        state.dataTranspiredCollector->UTSC(UTSCNum).Isc = Isc;
        state.dataTranspiredCollector->UTSC(UTSCNum).HXeff = HXeff;
        state.dataTranspiredCollector->UTSC(UTSCNum).Tplen = Taplen;
        state.dataTranspiredCollector->UTSC(UTSCNum).Tcoll = Tscoll;
        state.dataTranspiredCollector->UTSC(UTSCNum).HrPlen = HrPlen;
        state.dataTranspiredCollector->UTSC(UTSCNum).HcPlen = HcPlen;
        state.dataTranspiredCollector->UTSC(UTSCNum).TairHX = TaHX;
        state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot = Mdot;
        state.dataTranspiredCollector->UTSC(UTSCNum).InletTempDB = Tamb;
        state.dataTranspiredCollector->UTSC(UTSCNum).Vsuction = Vsuction;
        state.dataTranspiredCollector->UTSC(UTSCNum).PlenumVelocity = Vplen;
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutTemp = Taplen;
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutHumRat = state.dataEnvrn->OutHumRat; // stays the same with sensible heating
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutEnth =
            PsyHFnTdbW(state.dataTranspiredCollector->UTSC(UTSCNum).SupOutTemp, state.dataTranspiredCollector->UTSC(UTSCNum).SupOutHumRat);
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutMassFlow = Mdot;
        state.dataTranspiredCollector->UTSC(UTSCNum).SensHeatingRate = SensHeatingRate;
        state.dataTranspiredCollector->UTSC(UTSCNum).SensHeatingEnergy = SensHeatingRate * TimeStepSysSec;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveACH = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveMdotVent = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveMdotWind = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveMdotTherm = 0.0;
        if (Isc > 10.0) {
            state.dataTranspiredCollector->UTSC(UTSCNum).UTSCEfficiency = SensHeatingRate / (Isc * A);
            if (TaHX > Tamb) {
                state.dataTranspiredCollector->UTSC(UTSCNum).UTSCCollEff = Mdot * CpAir * (TaHX - Tamb) / (Isc * A);
            } else {
                state.dataTranspiredCollector->UTSC(UTSCNum).UTSCCollEff = 0.0;
            }
        } else {
            state.dataTranspiredCollector->UTSC(UTSCNum).UTSCEfficiency = 0.0;
            state.dataTranspiredCollector->UTSC(UTSCNum).UTSCCollEff = 0.0;
        }
    }

    void CalcPassiveTranspiredCollector(EnergyPlusData &state, int const UTSCNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // model the effect of the a ventilated baffle covering the outside of a heat transfer surface.

        // METHODOLOGY EMPLOYED:
        // All the work is done in a subroutine .

        // REFERENCES:
        // Nat. Vent. equations from ASHRAE HoF 2001 Chapt. 26

        // Using/Aliasing
        using DataSurfaces::SurfaceData;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyWFnTdbTwbPb;

        // local working variables
        Real64 AspRat; // Aspect Ratio of gap
        Real64 TmpTscoll;
        Real64 TmpTaPlen;
        Real64 RhoAir;
        Real64 holeArea;
        Real64 Tamb;
        Real64 HrPlen;
        Real64 HcPlen;
        Real64 Isc;
        Real64 MdotVent;
        Real64 VdotWind;
        Real64 VdotThermal;
        Real64 Twbamb;
        Real64 OutHumRatAmb;

        //        Tamb = sum( Surface( UTSC( UTSCNum ).SurfPtrs ).OutDryBulbTemp * Surface( UTSC( UTSCNum ).SurfPtrs ).Area ) / sum( Surface(
        // UTSC( UTSCNum ).SurfPtrs ).Area ); //Autodesk:F2C++ Array subscript usage: Replaced by below
        Real64 sum_area = 0.0;
        Real64 sum_produc_area_drybulb = 0.0;
        Real64 sum_produc_area_wetbulb = 0.0;
        for (int SurfNum : state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs) {
            sum_area += state.dataSurface->Surface(SurfNum).Area;
            sum_produc_area_wetbulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutWetBulbTemp(SurfNum);
            sum_produc_area_drybulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutDryBulbTemp(SurfNum);
        }
        Tamb = sum_produc_area_drybulb / sum_area;
        Twbamb = sum_produc_area_wetbulb / sum_area;

        OutHumRatAmb = PsyWFnTdbTwbPb(state, Tamb, Twbamb, state.dataEnvrn->OutBaroPress);

        RhoAir = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, Tamb, OutHumRatAmb);
        holeArea = state.dataTranspiredCollector->UTSC(UTSCNum).ActualArea * state.dataTranspiredCollector->UTSC(UTSCNum).Porosity;

        AspRat = state.dataTranspiredCollector->UTSC(UTSCNum).Height / state.dataTranspiredCollector->UTSC(UTSCNum).PlenGapThick;
        TmpTscoll = state.dataTranspiredCollector->UTSC(UTSCNum).TcollLast;
        TmpTaPlen = state.dataTranspiredCollector->UTSC(UTSCNum).TplenLast;

        // all the work is done in this routine located in GeneralRoutines.cc

        CalcPassiveExteriorBaffleGap(state,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).SurfPtrs,
                                     holeArea,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).Cv,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).Cd,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).HdeltaNPL,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).SolAbsorp,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).LWEmitt,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).Tilt,
                                     AspRat,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).PlenGapThick,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).CollRoughness,
                                     state.dataTranspiredCollector->UTSC(UTSCNum).QdotSource,
                                     TmpTscoll,
                                     TmpTaPlen,
                                     HcPlen,
                                     HrPlen,
                                     Isc,
                                     MdotVent,
                                     VdotWind,
                                     VdotThermal);

        // now fill results into derived types
        state.dataTranspiredCollector->UTSC(UTSCNum).Isc = Isc;
        state.dataTranspiredCollector->UTSC(UTSCNum).Tplen = TmpTaPlen;
        state.dataTranspiredCollector->UTSC(UTSCNum).Tcoll = TmpTscoll;
        state.dataTranspiredCollector->UTSC(UTSCNum).HrPlen = HrPlen;
        state.dataTranspiredCollector->UTSC(UTSCNum).HcPlen = HcPlen;
        state.dataTranspiredCollector->UTSC(UTSCNum).TairHX = Tamb;
        state.dataTranspiredCollector->UTSC(UTSCNum).InletMDot = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).InletTempDB = Tamb;
        state.dataTranspiredCollector->UTSC(UTSCNum).Vsuction = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).PlenumVelocity = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutTemp = TmpTaPlen;
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutHumRat = OutHumRatAmb;
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutEnth = PsyHFnTdbW(TmpTaPlen, OutHumRatAmb);
        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutMassFlow = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).SensHeatingRate = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).SensHeatingEnergy = 0.0;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveACH =
            (MdotVent / RhoAir) *
            (1.0 / (state.dataTranspiredCollector->UTSC(UTSCNum).ProjArea * state.dataTranspiredCollector->UTSC(UTSCNum).PlenGapThick)) *
            Constant::rSecsInHour;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveMdotVent = MdotVent;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveMdotWind = VdotWind * RhoAir;
        state.dataTranspiredCollector->UTSC(UTSCNum).PassiveMdotTherm = VdotThermal * RhoAir;
        state.dataTranspiredCollector->UTSC(UTSCNum).UTSCEfficiency = 0.0;
    }

    void UpdateTranspiredCollector(EnergyPlusData &state, int const UTSCNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        int OutletNode;
        int InletNode;
        int thisOSCM;
        int thisOASys;

        // update "last" values in Derived type
        state.dataTranspiredCollector->UTSC(UTSCNum).TplenLast = state.dataTranspiredCollector->UTSC(UTSCNum).Tplen;
        state.dataTranspiredCollector->UTSC(UTSCNum).TcollLast = state.dataTranspiredCollector->UTSC(UTSCNum).Tcoll;

        // Set the outlet air nodes of the UTSC

        if (state.dataTranspiredCollector->UTSC(UTSCNum).IsOn) { // Active
            if (state.dataTranspiredCollector->UTSC(UTSCNum).NumOASysAttached == 1) {
                OutletNode = state.dataTranspiredCollector->UTSC(UTSCNum).OutletNode(1);
                InletNode = state.dataTranspiredCollector->UTSC(UTSCNum).InletNode(1);
                state.dataLoopNodes->Node(OutletNode).MassFlowRate = state.dataTranspiredCollector->UTSC(UTSCNum).SupOutMassFlow;
                state.dataLoopNodes->Node(OutletNode).Temp = state.dataTranspiredCollector->UTSC(UTSCNum).SupOutTemp;
                state.dataLoopNodes->Node(OutletNode).HumRat = state.dataTranspiredCollector->UTSC(UTSCNum).SupOutHumRat;
                state.dataLoopNodes->Node(OutletNode).Enthalpy = state.dataTranspiredCollector->UTSC(UTSCNum).SupOutEnth;
            } else if (state.dataTranspiredCollector->UTSC(UTSCNum).NumOASysAttached > 1) {
                for (thisOASys = 1; thisOASys <= state.dataTranspiredCollector->UTSC(UTSCNum).NumOASysAttached; ++thisOASys) {
                    state.dataLoopNodes->Node(state.dataTranspiredCollector->UTSC(UTSCNum).OutletNode(thisOASys)).MassFlowRate =
                        state.dataLoopNodes->Node(state.dataTranspiredCollector->UTSC(UTSCNum).InletNode(thisOASys))
                            .MassFlowRate; // system gets what it asked for at inlet
                    state.dataLoopNodes->Node(state.dataTranspiredCollector->UTSC(UTSCNum).OutletNode(thisOASys)).Temp =
                        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutTemp;
                    state.dataLoopNodes->Node(state.dataTranspiredCollector->UTSC(UTSCNum).OutletNode(thisOASys)).HumRat =
                        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutHumRat;
                    state.dataLoopNodes->Node(state.dataTranspiredCollector->UTSC(UTSCNum).OutletNode(thisOASys)).Enthalpy =
                        state.dataTranspiredCollector->UTSC(UTSCNum).SupOutEnth;
                }
            }
        } else { // Passive and/or bypassed           Note Array assignments in following
                 // Autodesk:F2C++ Array subscript usage: Replaced by below
            //            Node( UTSC( UTSCNum ).OutletNode ).MassFlowRate = Node( UTSC( UTSCNum ).InletNode ).MassFlowRate;
            //            Node( UTSC( UTSCNum ).OutletNode ).Temp = Node( UTSC( UTSCNum ).InletNode ).Temp;
            //            Node( UTSC( UTSCNum ).OutletNode ).HumRat = Node( UTSC( UTSCNum ).InletNode ).HumRat;
            //            Node( UTSC( UTSCNum ).OutletNode ).Enthalpy = Node( UTSC( UTSCNum ).InletNode ).Enthalpy;
            auto const &OutletNode = state.dataTranspiredCollector->UTSC(UTSCNum).OutletNode;
            auto const &InletNode = state.dataTranspiredCollector->UTSC(UTSCNum).InletNode;
            assert(OutletNode.size() == InletNode.size());
            for (int io = OutletNode.l(), ii = InletNode.l(), eo = OutletNode.u(); io <= eo; ++io, ++ii) {
                auto &outNode = state.dataLoopNodes->Node(OutletNode(io));
                auto const &inNode = state.dataLoopNodes->Node(InletNode(ii));
                outNode.MassFlowRate = inNode.MassFlowRate;
                outNode.Temp = inNode.Temp;
                outNode.HumRat = inNode.HumRat;
                outNode.Enthalpy = inNode.Enthalpy;
            }
        }

        // update the OtherSideConditionsModel coefficients.
        thisOSCM = state.dataTranspiredCollector->UTSC(UTSCNum).OSCMPtr;

        state.dataSurface->OSCM(thisOSCM).TConv = state.dataTranspiredCollector->UTSC(UTSCNum).Tplen;
        state.dataSurface->OSCM(thisOSCM).HConv = state.dataTranspiredCollector->UTSC(UTSCNum).HcPlen;
        state.dataSurface->OSCM(thisOSCM).TRad = state.dataTranspiredCollector->UTSC(UTSCNum).Tcoll;
        state.dataSurface->OSCM(thisOSCM).HRad = state.dataTranspiredCollector->UTSC(UTSCNum).HrPlen;
    }

    void SetUTSCQdotSource(EnergyPlusData &state,
                           int const UTSCNum,
                           Real64 const QSource // source term in Watts
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Set" routine for updating sink term without exposing variables

        // METHODOLOGY EMPLOYED:
        // update derived type with new data , turn power into W/m2

        state.dataTranspiredCollector->UTSC(UTSCNum).QdotSource = QSource / state.dataTranspiredCollector->UTSC(UTSCNum).ProjArea;
    }

    void GetTranspiredCollectorIndex(EnergyPlusData &state, int const SurfacePtr, int &UTSCIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Get" routine for establishing correct integer index from outside this module

        // METHODOLOGY EMPLOYED:
        // mine Surface derived type for correct index/number of surface
        // mine UTSC derived type that has the surface.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int UTSCNum;  // temporary
        int ThisSurf; // temporary
        int thisUTSC;
        bool Found;

        if (state.dataTranspiredCollector->GetInputFlag) {
            GetTranspiredCollectorInput(state);
            state.dataTranspiredCollector->GetInputFlag = false;
        }

        if (SurfacePtr == 0) {
            ShowFatalError(
                state,
                format("Invalid surface passed to GetTranspiredCollectorIndex, Surface name = {}", state.dataSurface->Surface(SurfacePtr).Name));
        }

        UTSCNum = 0;
        Found = false;
        for (thisUTSC = 1; thisUTSC <= state.dataTranspiredCollector->NumUTSC; ++thisUTSC) {
            for (ThisSurf = 1; ThisSurf <= state.dataTranspiredCollector->UTSC(thisUTSC).NumSurfs; ++ThisSurf) {
                if (SurfacePtr == state.dataTranspiredCollector->UTSC(thisUTSC).SurfPtrs(ThisSurf)) {
                    Found = true;
                    UTSCNum = thisUTSC;
                }
            }
        }

        if (!Found) {
            ShowFatalError(state,
                           format("Did not find surface in UTSC description in GetTranspiredCollectorIndex, Surface name = {}",
                                  state.dataSurface->Surface(SurfacePtr).Name));
        } else {

            UTSCIndex = UTSCNum;
        }
    }

    void GetUTSCTsColl(EnergyPlusData &state, int const UTSCNum, Real64 &TsColl)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // object oriented "Get" routine for collector surface temperature

        // METHODOLOGY EMPLOYED:
        // access derived type

        TsColl = state.dataTranspiredCollector->UTSC(UTSCNum).Tcoll;
    }

    int GetAirInletNodeNum(EnergyPlusData &state, std::string const &UTSCName, bool &ErrorsFound)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   May 2019
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given UTSC and returns the air inlet node number.
        // If incorrect UTSC name is given, ErrorsFound is returned as true and node number as zero.

        // Return value
        int NodeNum; // node number returned

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichUTSC;

        if (state.dataTranspiredCollector->GetInputFlag) {
            GetTranspiredCollectorInput(state);
            state.dataTranspiredCollector->GetInputFlag = false;
        }

        WhichUTSC = Util::FindItemInList(UTSCName, state.dataTranspiredCollector->UTSC);
        if (WhichUTSC != 0) {
            NodeNum = state.dataTranspiredCollector->UTSC(WhichUTSC).InletNode(1);
        } else {
            ShowSevereError(state, format("GetAirInletNodeNum: Could not find TranspiredCollector = \"{}\"", UTSCName));
            ErrorsFound = true;
            NodeNum = 0;
        }

        return NodeNum;
    }

    int GetAirOutletNodeNum(EnergyPlusData &state, std::string const &UTSCName, bool &ErrorsFound)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   May 2019
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given UTSC and returns the air outlet node number.
        // If incorrect UTSC name is given, ErrorsFound is returned as true and node number as zero.

        // Return value
        int NodeNum; // node number returned

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichUTSC;

        if (state.dataTranspiredCollector->GetInputFlag) {
            GetTranspiredCollectorInput(state);
            state.dataTranspiredCollector->GetInputFlag = false;
        }

        WhichUTSC = Util::FindItemInList(UTSCName, state.dataTranspiredCollector->UTSC);
        if (WhichUTSC != 0) {
            NodeNum = state.dataTranspiredCollector->UTSC(WhichUTSC).OutletNode(1);
        } else {
            ShowSevereError(state, format("GetAirOutletNodeNum: Could not find TranspiredCollector = \"{}\"", UTSCName));
            ErrorsFound = true;
            NodeNum = 0;
        }

        return NodeNum;
    }

    void CalcPassiveExteriorBaffleGap(EnergyPlusData &state,
                                      const Array1D_int &SurfPtrARR, // Array of indexes pointing to Surface structure in DataSurfaces
                                      Real64 const VentArea,         // Area available for venting the gap [m2]
                                      Real64 const Cv,               // Orifice coefficient for volume-based discharge, wind-driven [--]
                                      Real64 const Cd,               // Orifice coefficient for discharge,  buoyancy-driven [--]
                                      Real64 const HdeltaNPL,        // Height difference from neutral pressure level [m]
                                      Real64 const SolAbs,           // solar absorptivity of baffle [--]
                                      Real64 const AbsExt,           // thermal absorptance/emittance of baffle material [--]
                                      Real64 const Tilt,             // Tilt of gap [Degrees]
                                      Real64 const AspRat,           // aspect ratio of gap  Height/gap [--]
                                      Real64 const GapThick,         // Thickness of air space between baffle and underlying heat transfer surface
                                      Material::SurfaceRoughness const Roughness, // Roughness index (1-6), see DataHeatBalance parameters
                                      Real64 const QdotSource,                    // Source/sink term, e.g. electricity exported from solar cell [W]
                                      Real64 &TsBaffle,                           // Temperature of baffle (both sides) use lagged value on input [C]
                                      Real64 &TaGap, // Temperature of air gap (assumed mixed) use lagged value on input [C]
                                      ObjexxFCL::Optional<Real64> HcGapRpt,
                                      ObjexxFCL::Optional<Real64> HrGapRpt,
                                      ObjexxFCL::Optional<Real64> IscRpt,
                                      ObjexxFCL::Optional<Real64> MdotVentRpt,
                                      ObjexxFCL::Optional<Real64> VdotWindRpt,
                                      ObjexxFCL::Optional<Real64> VdotBuoyRpt)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.T. Griffith
        //       DATE WRITTEN   November 2004
        //       MODIFIED       BG March 2007 outdoor conditions from surface for height-dependent conditions

        // PURPOSE OF THIS SUBROUTINE:
        // model the effect of the a ventilated baffle covering the outside of a heat transfer surface.
        // return calculated temperatures and certain intermediate values for reporting

        // METHODOLOGY EMPLOYED:
        // Heat balances on baffle and air space.
        // Natural ventilation calculations use buoyancy and wind.

        // REFERENCES:
        // Nat. Vent. equations from ASHRAE HoF 2001 Chapt. 26

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr g = 9.807;          // gravitational constant (m/s**2)
        Real64 constexpr nu = 15.66e-6;      // kinematic viscosity (m**2/s) for air at 300 K (Mills 1999 Heat Transfer)
        Real64 constexpr k = 0.0267;         // thermal conductivity (W/m K) for air at 300 K (Mills 1999 Heat Transfer)
        Real64 constexpr Sigma = 5.6697e-08; // Stefan-Boltzmann constant
        static constexpr std::string_view RoutineName = "CalcPassiveExteriorBaffleGap";
        // INTERFACE BLOCK SPECIFICATIONS:

        // DERIVED TYPE DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // following arrays are used to temporarily hold results from multiple underlying surfaces
        Array1D<Real64> HSkyARR;
        Array1D<Real64> HGroundARR;
        Array1D<Real64> HAirARR;
        Array1D<Real64> HPlenARR;
        Array1D<Real64> HExtARR;
        Array1D<Real64> LocalWindArr;
        Array1D<Real64> HSrdSurfARR;

        // local working variables
        Real64 Tamb;                  // outdoor drybulb
        Real64 VdotThermal;           // Volume flow rate of nat. vent due to buoyancy
        Real64 LocalOutDryBulbTemp;   // OutDryBulbTemp for here
        Real64 LocalWetBulbTemp;      // OutWetBulbTemp for here
        Real64 LocalOutHumRat;        // OutHumRat for here
        bool ICSCollectorIsOn(false); // ICS collector has OSCM on
        int CollectorNum;             // current solar collector index
        Real64 ICSWaterTemp;          // ICS solar collector water temp
        Real64 ICSULossbottom;        // ICS solar collector bottom loss Conductance
        Real64 sum_area = 0.0;
        Real64 sum_produc_area_drybulb = 0.0;
        Real64 sum_produc_area_wetbulb = 0.0;

        auto &s_mat = state.dataMaterial;

        for (int SurfNum : SurfPtrARR) {
            sum_area += state.dataSurface->Surface(SurfNum).Area;
            sum_produc_area_drybulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutDryBulbTemp(SurfNum);
            sum_produc_area_wetbulb += state.dataSurface->Surface(SurfNum).Area * state.dataSurface->SurfOutWetBulbTemp(SurfNum);
        }
        //    LocalOutDryBulbTemp = sum( Surface( SurfPtrARR ).Area * Surface( SurfPtrARR ).OutDryBulbTemp ) / sum( Surface( SurfPtrARR ).Area );
        LocalOutDryBulbTemp = sum_produc_area_drybulb / sum_area; // Autodesk:F2C++ Functions handle array subscript usage
        //    LocalWetBulbTemp = sum( Surface( SurfPtrARR ).Area * Surface( SurfPtrARR ).OutWetBulbTemp ) / sum( Surface( SurfPtrARR ).Area );
        LocalWetBulbTemp = sum_produc_area_wetbulb / sum_area;

        LocalOutHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, LocalOutDryBulbTemp, LocalWetBulbTemp, state.dataEnvrn->OutBaroPress, RoutineName);

        Real64 RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, LocalOutDryBulbTemp, LocalOutHumRat, RoutineName);
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(LocalOutHumRat);
        if (!state.dataEnvrn->IsRain) {
            Tamb = LocalOutDryBulbTemp;
        } else { // when raining we use wetbulb not drybulb
            Tamb = LocalWetBulbTemp;
        }
        Real64 A = sum_area;        // projected area of baffle from sum of underlying surfaces
        Real64 TmpTsBaf = TsBaffle; // baffle temperature

        // loop through underlying surfaces and collect needed data
        int NumSurfs = size(SurfPtrARR); // number of underlying HT surfaces associated with UTSC
        HSkyARR.dimension(NumSurfs, 0.0);
        HGroundARR.dimension(NumSurfs, 0.0);
        HAirARR.dimension(NumSurfs, 0.0);
        LocalWindArr.dimension(NumSurfs, 0.0);
        HPlenARR.dimension(NumSurfs, 0.0);
        HExtARR.dimension(NumSurfs, 0.0);
        HSrdSurfARR.dimension(NumSurfs, 0.0);

        for (int ThisSurf = 1; ThisSurf <= NumSurfs; ++ThisSurf) {
            int SurfPtr = SurfPtrARR(ThisSurf);
            // Initializations for this surface
            Real64 HMovInsul = 0.0;
            LocalWindArr(ThisSurf) = state.dataSurface->SurfOutWindSpeed(SurfPtr);
            Convect::InitExtConvCoeff(state,
                                      SurfPtr,
                                      HMovInsul,
                                      Roughness,
                                      AbsExt,
                                      TmpTsBaf,
                                      HExtARR(ThisSurf),
                                      HSkyARR(ThisSurf),
                                      HGroundARR(ThisSurf),
                                      HAirARR(ThisSurf),
                                      HSrdSurfARR(ThisSurf));
            int ConstrNum = state.dataSurface->Surface(SurfPtr).Construction;
            Real64 AbsThermSurf = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1))->AbsorpThermal;
            Real64 TsoK = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfPtr) + Constant::Kelvin;
            Real64 TsBaffK = TmpTsBaf + Constant::Kelvin;
            if (TsBaffK == TsoK) {        // avoid divide by zero
                HPlenARR(ThisSurf) = 0.0; // no net heat transfer if same temperature
            } else {
                HPlenARR(ThisSurf) = Sigma * AbsExt * AbsThermSurf * (pow_4(TsBaffK) - pow_4(TsoK)) / (TsBaffK - TsoK);
            }
            // Added for ICS collector OSCM
            if (state.dataSurface->SurfIsICS(SurfPtr)) {
                ICSCollectorIsOn = true;
                CollectorNum = state.dataSurface->SurfICSPtr(SurfPtr);
            }
        }

        if (ICSCollectorIsOn) {
            if (state.dataGlobal->BeginEnvrnFlag && state.dataGeneralRoutines->MyICSEnvrnFlag) {
                ICSULossbottom = 0.40;
                ICSWaterTemp = 20.0;
            } else {
                if (!state.dataSolarCollectors->Collector.allocated()) {
                    ICSULossbottom = 0.40;
                    ICSWaterTemp = 20.0;
                } else {
                    ICSULossbottom = state.dataSolarCollectors->Collector(CollectorNum).UbLoss;
                    ICSWaterTemp = state.dataSolarCollectors->Collector(CollectorNum).TempOfWater;
                    state.dataGeneralRoutines->MyICSEnvrnFlag = false;
                }
            }
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataGeneralRoutines->MyICSEnvrnFlag = true;
        }
        if (A == 0.0) { // should have been caught earlier
        }
        Array1D<Real64> Area(
            array_sub(state.dataSurface->Surface,
                      &DataSurfaces::SurfaceData::Area,
                      SurfPtrARR)); // Autodesk:F2C++ Copy of subscripted Area array for use below: This makes a copy so review wrt performance
        // now figure area-weighted averages from underlying surfaces.
        Real64 Vwind = sum(LocalWindArr * Area) / A; // area weighted average of wind velocity
        LocalWindArr.deallocate();
        Real64 HrSky = sum(HSkyARR * Area) / A; // radiation coeff for sky, area-weighted average
        HSkyARR.deallocate();
        Real64 HrGround = sum(HGroundARR * Area) / A; // radiation coeff for ground, area-weighted average
        HGroundARR.deallocate();
        Real64 HrAtm = sum(HAirARR * Area) / A; // radiation coeff for air (bulk atmosphere), area-weighted average
        HAirARR.deallocate();
        Real64 HrPlen = sum(HPlenARR * Area) / A; // radiation coeff for plenum surfaces, area-weighted average
        HPlenARR.deallocate();
        Real64 HExt = sum(HExtARR * Area) / A; // dummy for call to InitExteriorConvectionCoeff
        HExtARR.deallocate();
        HSrdSurfARR.deallocate();

        if (state.dataEnvrn->IsRain) {
            HExt = 1000.0;
        }

        // temperature of underlying surface, area-weighted average
        Real64 Tso =
            sum_product_sub(state.dataHeatBalSurf->SurfOutsideTempHist(1), state.dataSurface->Surface, &DataSurfaces::SurfaceData::Area, SurfPtrARR) /
            A;
        // Incoming combined solar radiation, area-weighted average
        Real64 Isc =
            sum_product_sub(state.dataHeatBal->SurfQRadSWOutIncident, state.dataSurface->Surface, &DataSurfaces::SurfaceData::Area, SurfPtrARR) / A;
        // average of surface temps , for Beta in Grashoff no.
        Real64 TmeanK = 0.5 * (TmpTsBaf + Tso) + Constant::Kelvin;
        // Grasshof number for natural convection calc
        Real64 Gr = g * pow_3(GapThick) * std::abs(Tso - TmpTsBaf) * pow_2(RhoAir) / (TmeanK * pow_2(nu));

        Real64 NuPlen = PassiveGapNusseltNumber(AspRat, Tilt, TmpTsBaf, Tso, Gr); // intentionally switch Tso to Tsi
        Real64 HcPlen = NuPlen * (k / GapThick);                                  // surface convection heat transfer coefficient for plenum surfaces

        // now model natural ventilation of plenum gap.
        Real64 VdotWind = Cv * (VentArea / 2.0) * Vwind; // volume flow rate of nat. vent due to wind

        if (TaGap > Tamb) {
            VdotThermal = Cd * (VentArea / 2.0) * std::sqrt(2.0 * g * HdeltaNPL * (TaGap - Tamb) / (TaGap + Constant::Kelvin));
        } else if (TaGap == Tamb) {
            VdotThermal = 0.0;
        } else {
            if ((std::abs(Tilt) < 5.0) || (std::abs(Tilt - 180.0) < 5.0)) {
                VdotThermal = 0.0; // stable buoyancy situation
            } else {
                VdotThermal = Cd * (VentArea / 2.0) * std::sqrt(2.0 * g * HdeltaNPL * (Tamb - TaGap) / (Tamb + Constant::Kelvin));
            }
        }

        Real64 VdotVent = VdotWind + VdotThermal; // total volume flow rate of nat vent
        Real64 MdotVent = VdotVent * RhoAir;      // total mass flow rate of nat vent

        // now calculate baffle temperature
        if (!ICSCollectorIsOn) {
            TsBaffle = (Isc * SolAbs + HExt * Tamb + HrAtm * Tamb + HrSky * state.dataEnvrn->SkyTemp + HrGround * Tamb + HrPlen * Tso +
                        HcPlen * TaGap + QdotSource) /
                       (HExt + HrAtm + HrSky + HrGround + HrPlen + HcPlen);
        } else {

            TsBaffle = (ICSULossbottom * ICSWaterTemp + HrPlen * Tso + HcPlen * TaGap + QdotSource) / (ICSULossbottom + HrPlen + HcPlen);
        }
        // now calculate gap air temperature

        TaGap = (HcPlen * A * Tso + MdotVent * CpAir * Tamb + HcPlen * A * TsBaffle) / (HcPlen * A + MdotVent * CpAir + HcPlen * A);

        if (present(HcGapRpt)) {
            HcGapRpt = HcPlen;
        }
        if (present(HrGapRpt)) {
            HrGapRpt = HrPlen;
        }
        if (present(IscRpt)) {
            IscRpt = Isc;
        }
        if (present(MdotVentRpt)) {
            MdotVentRpt = MdotVent;
        }
        if (present(VdotWindRpt)) {
            VdotWindRpt = VdotWind;
        }
        if (present(VdotBuoyRpt)) {
            VdotBuoyRpt = VdotThermal;
        }
    }

    //****************************************************************************

    Real64 PassiveGapNusseltNumber(Real64 const AspRat, // Aspect Ratio of Gap height to gap width
                                   Real64 const Tilt,   // Tilt of gap, degrees
                                   Real64 const Tso,    // Temperature of gap surface closest to outside (K)
                                   Real64 const Tsi,    // Temperature of gap surface closest to zone (K)
                                   Real64 const Gr      // Gap gas Grashof number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Adapted by B. Griffith from Fred Winkelmann's from NusseltNumber in WindowManager.cc
        //       DATE WRITTEN   September 2001
        //       MODIFIED       B. Griffith November 2004  (same models but slightly different for general use)

        // PURPOSE OF THIS SUBROUTINE:
        // Finds the Nusselt number for air-filled gaps between isothermal solid layers.

        // METHODOLOGY EMPLOYED:
        // Based on methodology in Chapter 5 of the July 18, 2001 draft of ISO 15099,
        // "Thermal Performance of Windows, Doors and Shading Devices--Detailed Calculations."
        // The equation numbers below correspond to those in the standard.

        // REFERENCES:
        // Window5 source code; ISO 15099

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr Pr(0.71); // Prandtl number for air

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS
        Real64 gnu901; // Nusselt number temporary variables for

        Real64 tiltr = Tilt * Constant::DegToRad;
        Real64 Ra = Gr * Pr; // Rayleigh number

        if (Ra <= 1.0e4) {
            gnu901 = 1.0 + 1.7596678e-10 * std::pow(Ra, 2.2984755); // eq. 51
        }
        if (Ra > 1.0e4 && Ra <= 5.0e4) {
            gnu901 = 0.028154 * std::pow(Ra, 0.4134); // eq. 50
        }
        if (Ra > 5.0e4) {
            gnu901 = 0.0673838 * std::pow(Ra, 1.0 / 3.0); // eq. 49
        }

        Real64 gnu902 = 0.242 * std::pow(Ra / AspRat, 0.272); // eq. 52
        Real64 gnu90 = max(gnu901, gnu902);

        if (Tso > Tsi) {                                  // window heated from above
            return 1.0 + (gnu90 - 1.0) * std::sin(tiltr); // eq. 53
        } else if (Tilt >= 60.0) {
            Real64 g = 0.5 * std::pow(1.0 + std::pow(Ra / 3160.0, 20.6), -0.1);     // eq. 47
            Real64 gnu601a = 1.0 + pow_7(0.0936 * std::pow(Ra, 0.314) / (1.0 + g)); // eq. 45
            Real64 gnu601 = std::pow(gnu601a, 0.142857);

            // For any aspect ratio
            Real64 gnu602 = (0.104 + 0.175 / AspRat) * std::pow(Ra, 0.283); // eq. 46
            Real64 gnu60 = max(gnu601, gnu602);

            // linear interpolation for layers inclined at angles between 60 and 90 deg
            return ((90.0 - Tilt) * gnu60 + (Tilt - 60.0) * gnu90) / 30.0;
        } else { // eq. 42
            Real64 cra = Ra * std::cos(tiltr);
            Real64 a = 1.0 - 1708.0 / cra;
            Real64 b = std::pow(cra / 5830.0, 0.33333) - 1.0;
            Real64 gnua = (std::abs(a) + a) / 2.0;
            Real64 gnub = (std::abs(b) + b) / 2.0;
            Real64 ang = 1708.0 * std::pow(std::sin(1.8 * tiltr), 1.6);
            return 1.0 + 1.44 * gnua * (1.0 - ang / cra) + gnub;
        }
    }

} // namespace TranspiredCollector

} // namespace EnergyPlus
