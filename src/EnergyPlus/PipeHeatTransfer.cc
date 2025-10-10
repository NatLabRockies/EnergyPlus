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
#include <memory>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/GroundTemperatureModeling/GroundTemperatureModelManager.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PipeHeatTransfer.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::PipeHeatTransfer {

// Module containing the routines dealing with pipes with transport delay
// and heat transfer.

// MODULE INFORMATION:
//       AUTHOR         Simon Rees
//       DATE WRITTEN   July 2007
//       MODIFIED       May 2008
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// The purpose of this module is to simulate a pipe with heat transfer

// METHODOLOGY EMPLOYED:
// An implicit finite difference method is used to solve the temperature distribution of the
// fluid in the pipe as a result of the transport delay and heat transfer to the environment.
// For buried pipes, the simulation involves an implicit finite difference model of the soil,
// which was originally based on Piechowski's thesis (below).  Equation numbers for
// pipe:underground calculations are from Piechowski's thesis.  In Piechowski, the near-pipe
// region is solved with a detailed finite difference grid, this current model makes use of
// the Hanby model to simulate the actual pipe.

// Kusuda, T. & Achenbach, P. (1965), 'Earth temperature and thermal diffusivity at
//     selected stations in the united states', ASHRAE Transactions 71(1), 61-75.
// Piechowski, M. (1996), A Ground Coupled Heat Pump System with Energy Storage,
//     PhD thesis, University of Melbourne.

// OTHER NOTES: Equation Numbers listed in buried pipe routines are from Piechowski's thesis

enum class PipeIndoorBoundaryType
{
    Invalid = -1,
    Zone,
    Schedule,
    Num
};
constexpr std::array<std::string_view, static_cast<int>(PipeIndoorBoundaryType::Num)> pipeIndoorBoundaryTypeNamesUC = {"ZONE", "SCHEDULE"};

// Functions

PlantComponent *PipeHTData::factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string const &objectName)
{
    // Process the input data for pipes if it hasn't been done already
    if (state.dataPipeHT->GetPipeInputFlag) {
        GetPipesHeatTransfer(state);
        state.dataPipeHT->GetPipeInputFlag = false;
    }
    // Now look for this particular pipe in the list
    auto thisObj = std::find_if(state.dataPipeHT->PipeHT.begin(),
                                state.dataPipeHT->PipeHT.end(),
                                [&objectType, &objectName](const PipeHTData &myObj) { return myObj.Type == objectType && myObj.Name == objectName; });
    if (thisObj != state.dataPipeHT->PipeHT.end()) {
        return thisObj;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("PipeHTFactory: Error getting inputs for pipe named: {}", objectName));
    // Shut up the compiler
    return nullptr;
}

void PipeHTData::simulate(EnergyPlusData &state,
                          [[maybe_unused]] const PlantLocation &calledFromLocation,
                          bool const FirstHVACIteration,
                          [[maybe_unused]] Real64 &CurLoad,
                          [[maybe_unused]] bool const RunFlag)
{
    this->InitPipesHeatTransfer(state, FirstHVACIteration);
    // make the calculations
    for (int InnerTimeStepCtr = 1; InnerTimeStepCtr <= state.dataPipeHT->nsvNumInnerTimeSteps; ++InnerTimeStepCtr) {
        switch (this->EnvironmentPtr) {
        case EnvrnPtr::GroundEnv: {
            this->CalcBuriedPipeSoil(state);
        } break;
        default: {
            this->CalcPipesHeatTransfer(state);
        } break;
        }
        this->PushInnerTimeStepArrays();
    }
    // update variables
    this->UpdatePipesHeatTransfer(state);
    // update report variables
    this->ReportPipesHeatTransfer(state);
}

void PipeHTData::PushInnerTimeStepArrays()
{
    if (this->EnvironmentPtr == EnvrnPtr::GroundEnv) {
        for (int LengthIndex = 2; LengthIndex <= this->NumSections; ++LengthIndex) {
            for (int DepthIndex = 1; DepthIndex <= this->NumDepthNodes; ++DepthIndex) {
                for (int WidthIndex = 2; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                    // This will store the old 'current' values as the new 'previous values'  This allows
                    // us to use the previous time array as history terms in the equations
                    this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Previous) =
                        this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Current);
                }
            }
        }
    }
    // Then update the Hanby near pipe model temperatures
    this->PreviousFluidTemp = this->FluidTemp;
    this->PreviousPipeTemp = this->PipeTemp;
}

void GetPipesHeatTransfer(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   July 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na
    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reads the input for hydronic Pipe Heat Transfers
    // from the user input file.  This will contain all of the information
    // needed to define and simulate the surface.

    // Using/Aliasing
    using BranchNodeConnections::TestCompSet;

    using NodeInputManager::GetOnlySingleNode;
    using namespace DataLoopNode;
    using OutAirNodeManager::CheckOutAirNodeNumber;

    static constexpr std::string_view routineName = "GetPipeHeatTransfer";

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr NumPipeSections(20);
    int constexpr NumberOfDepthNodes(8); // Number of nodes in the cartesian grid-Should be an even # for now

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false); // Set to true if errors in input,

    // fatal at end of routine
    int IOStatus;       // Used in GetObjectItem
    int NumAlphas;      // Number of Alphas for each GetObjectItem call
    int NumNumbers;     // Number of Numbers for each GetObjectItem call
    int NumOfPipeHTInt; // Number of Pipe Heat Transfer objects
    int NumOfPipeHTExt; // Number of Pipe Heat Transfer objects
    int NumOfPipeHTUG;  // Number of Pipe Heat Transfer objects

    auto &s_ipsc = state.dataIPShortCut;
    auto &s_mat = state.dataMaterial;
    // Initializations and allocations
    s_ipsc->cCurrentModuleObject = "Pipe:Indoor";
    NumOfPipeHTInt = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_ipsc->cCurrentModuleObject = "Pipe:Outdoor";
    NumOfPipeHTExt = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    s_ipsc->cCurrentModuleObject = "Pipe:Underground";
    NumOfPipeHTUG = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    state.dataPipeHT->nsvNumOfPipeHT = NumOfPipeHTInt + NumOfPipeHTExt + NumOfPipeHTUG;
    // allocate data structures
    if (allocated(state.dataPipeHT->PipeHT)) {
        state.dataPipeHT->PipeHT.deallocate();
    }

    state.dataPipeHT->PipeHT.allocate(state.dataPipeHT->nsvNumOfPipeHT);
    state.dataPipeHT->PipeHTUniqueNames.reserve(static_cast<unsigned>(state.dataPipeHT->nsvNumOfPipeHT));
    int Item = 0;

    s_ipsc->cCurrentModuleObject = "Pipe:Indoor";
    for (int PipeItem = 1; PipeItem <= NumOfPipeHTInt; ++PipeItem) {
        ++Item;
        // get the object name
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 PipeItem,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 state.dataPipeHT->PipeHTUniqueNames,
                                                 s_ipsc->cAlphaArgs(1),
                                                 s_ipsc->cCurrentModuleObject,
                                                 s_ipsc->cAlphaFieldNames(1),
                                                 ErrorsFound);
        state.dataPipeHT->PipeHT(Item).Name = s_ipsc->cAlphaArgs(1);
        state.dataPipeHT->PipeHT(Item).Type = DataPlant::PlantEquipmentType::PipeInterior;

        // General user input data
        state.dataPipeHT->PipeHT(Item).Construction = s_ipsc->cAlphaArgs(2);
        state.dataPipeHT->PipeHT(Item).ConstructionNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct);

        if (state.dataPipeHT->PipeHT(Item).ConstructionNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // get inlet node data
        state.dataPipeHT->PipeHT(Item).InletNode = s_ipsc->cAlphaArgs(3);
        state.dataPipeHT->PipeHT(Item).InletNodeNum = GetOnlySingleNode(state,
                                                                        s_ipsc->cAlphaArgs(3),
                                                                        ErrorsFound,
                                                                        DataLoopNode::ConnectionObjectType::PipeIndoor,
                                                                        s_ipsc->cAlphaArgs(1),
                                                                        DataLoopNode::NodeFluidType::Water,
                                                                        DataLoopNode::ConnectionType::Inlet,
                                                                        NodeInputManager::CompFluidStream::Primary,
                                                                        ObjectIsNotParent);
        if (state.dataPipeHT->PipeHT(Item).InletNodeNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // get outlet node data
        state.dataPipeHT->PipeHT(Item).OutletNode = s_ipsc->cAlphaArgs(4);
        state.dataPipeHT->PipeHT(Item).OutletNodeNum = GetOnlySingleNode(state,
                                                                         s_ipsc->cAlphaArgs(4),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::PipeIndoor,
                                                                         s_ipsc->cAlphaArgs(1),
                                                                         DataLoopNode::NodeFluidType::Water,
                                                                         DataLoopNode::ConnectionType::Outlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         ObjectIsNotParent);
        if (state.dataPipeHT->PipeHT(Item).OutletNodeNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        TestCompSet(state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(3), s_ipsc->cAlphaArgs(4), "Pipe Nodes");

        // get environmental boundary condition type

        if (s_ipsc->lAlphaFieldBlanks(5)) {
            s_ipsc->cAlphaArgs(5) = "ZONE";
        }

        PipeIndoorBoundaryType indoorType = static_cast<PipeIndoorBoundaryType>(getEnumValue(pipeIndoorBoundaryTypeNamesUC, s_ipsc->cAlphaArgs(5)));
        switch (indoorType) {
        case PipeIndoorBoundaryType::Zone:
            state.dataPipeHT->PipeHT(Item).EnvironmentPtr = EnvrnPtr::ZoneEnv;
            state.dataPipeHT->PipeHT(Item).EnvrZonePtr = Util::FindItemInList(s_ipsc->cAlphaArgs(6), state.dataHeatBal->Zone);
            if (state.dataPipeHT->PipeHT(Item).EnvrZonePtr == 0) {
                ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            break;

        case PipeIndoorBoundaryType::Schedule:
            state.dataPipeHT->PipeHT(Item).EnvironmentPtr = EnvrnPtr::ScheduleEnv;

            state.dataPipeHT->PipeHT(Item).envrSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(7));
            state.dataPipeHT->PipeHT(Item).envrVelSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(8));
            if (state.dataPipeHT->PipeHT(Item).envrSched == nullptr) {
                ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            if (state.dataPipeHT->PipeHT(Item).envrVelSched == nullptr) {
                ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaArgs(8)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            break;

        default:
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, R"(Should be "ZONE" or "SCHEDULE")"); // TODO rename point
            ErrorsFound = true;
        }

        // dimensions
        state.dataPipeHT->PipeHT(Item).PipeID = s_ipsc->rNumericArgs(1);
        if (s_ipsc->rNumericArgs(1) <= 0.0) { // not really necessary because idd field has "minimum> 0"
            ShowSevereError(state, format("GetPipesHeatTransfer: invalid {} of {:.4R}", s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1)));
            ShowContinueError(state, format("{} must be > 0.0", s_ipsc->cNumericFieldNames(1)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));

            ErrorsFound = true;
        }

        state.dataPipeHT->PipeHT(Item).Length = s_ipsc->rNumericArgs(2);
        if (s_ipsc->rNumericArgs(2) <= 0.0) { // not really necessary because idd field has "minimum> 0"
            ShowSevereError(state, format("GetPipesHeatTransfer: invalid {} of {:.4R}", s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2)));
            ShowContinueError(state, format("{} must be > 0.0", s_ipsc->cNumericFieldNames(2)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        if (state.dataPipeHT->PipeHT(Item).ConstructionNum != 0) {
            state.dataPipeHT->PipeHT(Item).ValidatePipeConstruction(state,
                                                                    s_ipsc->cCurrentModuleObject,
                                                                    s_ipsc->cAlphaArgs(2),
                                                                    s_ipsc->cAlphaFieldNames(2),
                                                                    state.dataPipeHT->PipeHT(Item).ConstructionNum,
                                                                    ErrorsFound);
        }

    } // end of input loop

    s_ipsc->cCurrentModuleObject = "Pipe:Outdoor";
    for (int PipeItem = 1; PipeItem <= NumOfPipeHTExt; ++PipeItem) {
        ++Item;
        // get the object name
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 PipeItem,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 state.dataPipeHT->PipeHTUniqueNames,
                                                 s_ipsc->cAlphaArgs(1),
                                                 s_ipsc->cCurrentModuleObject,
                                                 s_ipsc->cAlphaFieldNames(1),
                                                 ErrorsFound);
        state.dataPipeHT->PipeHT(Item).Name = s_ipsc->cAlphaArgs(1);
        state.dataPipeHT->PipeHT(Item).Type = DataPlant::PlantEquipmentType::PipeExterior;

        // General user input data
        state.dataPipeHT->PipeHT(Item).Construction = s_ipsc->cAlphaArgs(2);
        state.dataPipeHT->PipeHT(Item).ConstructionNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct);

        if (state.dataPipeHT->PipeHT(Item).ConstructionNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // get inlet node data
        state.dataPipeHT->PipeHT(Item).InletNode = s_ipsc->cAlphaArgs(3);
        state.dataPipeHT->PipeHT(Item).InletNodeNum = GetOnlySingleNode(state,
                                                                        s_ipsc->cAlphaArgs(3),
                                                                        ErrorsFound,
                                                                        DataLoopNode::ConnectionObjectType::PipeOutdoor,
                                                                        s_ipsc->cAlphaArgs(1),
                                                                        DataLoopNode::NodeFluidType::Water,
                                                                        DataLoopNode::ConnectionType::Inlet,
                                                                        NodeInputManager::CompFluidStream::Primary,
                                                                        ObjectIsNotParent);
        if (state.dataPipeHT->PipeHT(Item).InletNodeNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // get outlet node data
        state.dataPipeHT->PipeHT(Item).OutletNode = s_ipsc->cAlphaArgs(4);
        state.dataPipeHT->PipeHT(Item).OutletNodeNum = GetOnlySingleNode(state,
                                                                         s_ipsc->cAlphaArgs(4),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::PipeOutdoor,
                                                                         s_ipsc->cAlphaArgs(1),
                                                                         DataLoopNode::NodeFluidType::Water,
                                                                         DataLoopNode::ConnectionType::Outlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         ObjectIsNotParent);
        if (state.dataPipeHT->PipeHT(Item).OutletNodeNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        TestCompSet(state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(3), s_ipsc->cAlphaArgs(4), "Pipe Nodes");

        // get environmental boundary condition type
        //    PipeHT(Item)%Environment = 'OutdoorAir'
        state.dataPipeHT->PipeHT(Item).EnvironmentPtr = EnvrnPtr::OutsideAirEnv;

        state.dataPipeHT->PipeHT(Item).EnvrAirNode = s_ipsc->cAlphaArgs(5);
        state.dataPipeHT->PipeHT(Item).EnvrAirNodeNum = GetOnlySingleNode(state,
                                                                          s_ipsc->cAlphaArgs(5),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::PipeOutdoor,
                                                                          s_ipsc->cAlphaArgs(1),
                                                                          DataLoopNode::NodeFluidType::Air,
                                                                          DataLoopNode::ConnectionType::OutsideAirReference,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          ObjectIsNotParent);
        if (!s_ipsc->lAlphaFieldBlanks(5)) {
            if (!CheckOutAirNodeNumber(state, state.dataPipeHT->PipeHT(Item).EnvrAirNodeNum)) {
                ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Outdoor Air Node not on OutdoorAir:NodeList or OutdoorAir:Node");
                ErrorsFound = true;
            }
        } else {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("An {} must be used ", s_ipsc->cAlphaFieldNames(5)));
            ErrorsFound = true;
        }

        // dimensions
        state.dataPipeHT->PipeHT(Item).PipeID = s_ipsc->rNumericArgs(1);
        if (s_ipsc->rNumericArgs(1) <= 0.0) { // not really necessary because idd field has "minimum> 0"
            ShowSevereError(state, format("Invalid {} of {:.4R}", s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1)));
            ShowContinueError(state, format("{} must be > 0.0", s_ipsc->cNumericFieldNames(1)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        state.dataPipeHT->PipeHT(Item).Length = s_ipsc->rNumericArgs(2);
        if (s_ipsc->rNumericArgs(2) <= 0.0) { // not really necessary because idd field has "minimum> 0"
            ShowSevereError(state, format("Invalid {} of {:.4R}", s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2)));
            ShowContinueError(state, format("{} must be > 0.0", s_ipsc->cNumericFieldNames(2)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        if (state.dataPipeHT->PipeHT(Item).ConstructionNum != 0) {
            state.dataPipeHT->PipeHT(Item).ValidatePipeConstruction(state,
                                                                    s_ipsc->cCurrentModuleObject,
                                                                    s_ipsc->cAlphaArgs(2),
                                                                    s_ipsc->cAlphaFieldNames(2),
                                                                    state.dataPipeHT->PipeHT(Item).ConstructionNum,
                                                                    ErrorsFound);
        }

    } // end of input loop

    s_ipsc->cCurrentModuleObject = "Pipe:Underground";
    for (int PipeItem = 1; PipeItem <= NumOfPipeHTUG; ++PipeItem) {
        ++Item;
        // get the object name
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 PipeItem,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 state.dataPipeHT->PipeHTUniqueNames,
                                                 s_ipsc->cAlphaArgs(1),
                                                 s_ipsc->cCurrentModuleObject,
                                                 s_ipsc->cAlphaFieldNames(1),
                                                 ErrorsFound);
        state.dataPipeHT->PipeHT(Item).Name = s_ipsc->cAlphaArgs(1);
        state.dataPipeHT->PipeHT(Item).Type = DataPlant::PlantEquipmentType::PipeUnderground;

        // General user input data
        state.dataPipeHT->PipeHT(Item).Construction = s_ipsc->cAlphaArgs(2);
        state.dataPipeHT->PipeHT(Item).ConstructionNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataConstruction->Construct);

        if (state.dataPipeHT->PipeHT(Item).ConstructionNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // get inlet node data
        state.dataPipeHT->PipeHT(Item).InletNode = s_ipsc->cAlphaArgs(3);
        state.dataPipeHT->PipeHT(Item).InletNodeNum = GetOnlySingleNode(state,
                                                                        s_ipsc->cAlphaArgs(3),
                                                                        ErrorsFound,
                                                                        DataLoopNode::ConnectionObjectType::PipeUnderground,
                                                                        s_ipsc->cAlphaArgs(1),
                                                                        DataLoopNode::NodeFluidType::Water,
                                                                        DataLoopNode::ConnectionType::Inlet,
                                                                        NodeInputManager::CompFluidStream::Primary,
                                                                        ObjectIsNotParent);
        if (state.dataPipeHT->PipeHT(Item).InletNodeNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // get outlet node data
        state.dataPipeHT->PipeHT(Item).OutletNode = s_ipsc->cAlphaArgs(4);
        state.dataPipeHT->PipeHT(Item).OutletNodeNum = GetOnlySingleNode(state,
                                                                         s_ipsc->cAlphaArgs(4),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::PipeUnderground,
                                                                         s_ipsc->cAlphaArgs(1),
                                                                         DataLoopNode::NodeFluidType::Water,
                                                                         DataLoopNode::ConnectionType::Outlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         ObjectIsNotParent);
        if (state.dataPipeHT->PipeHT(Item).OutletNodeNum == 0) {
            ShowSevereError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        TestCompSet(state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(3), s_ipsc->cAlphaArgs(4), "Pipe Nodes");

        state.dataPipeHT->PipeHT(Item).EnvironmentPtr = EnvrnPtr::GroundEnv;

        // Solar inclusion flag
        // A6,  \field Sun Exposure
        if (Util::SameString(s_ipsc->cAlphaArgs(5), "SUNEXPOSED")) {
            state.dataPipeHT->PipeHT(Item).SolarExposed = true;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(5), "NOSUN")) {
            state.dataPipeHT->PipeHT(Item).SolarExposed = false;
        } else {
            ShowSevereError(state, format("GetPipesHeatTransfer: invalid key for sun exposure flag for {}", s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Key should be either SunExposed or NoSun.  Entered Key: {}", s_ipsc->cAlphaArgs(5)));
            ErrorsFound = true;
        }

        // dimensions
        state.dataPipeHT->PipeHT(Item).PipeID = s_ipsc->rNumericArgs(1);
        if (s_ipsc->rNumericArgs(1) <= 0.0) { // not really necessary because idd field has "minimum> 0"
            ShowSevereError(state, format("Invalid {} of {:.4R}", s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1)));
            ShowContinueError(state, format("{} must be > 0.0", s_ipsc->cNumericFieldNames(1)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        state.dataPipeHT->PipeHT(Item).Length = s_ipsc->rNumericArgs(2);
        if (s_ipsc->rNumericArgs(2) <= 0.0) { // not really necessary because idd field has "minimum> 0"
            ShowSevereError(state, format("Invalid {} of {:.4R}", s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2)));
            ShowContinueError(state, format("{} must be > 0.0", s_ipsc->cNumericFieldNames(2)));
            ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }

        // Also get the soil material name
        // A7,  \field Soil Material
        state.dataPipeHT->PipeHT(Item).SoilMaterial = s_ipsc->cAlphaArgs(6);
        state.dataPipeHT->PipeHT(Item).SoilMaterialNum = Material::GetMaterialNum(state, s_ipsc->cAlphaArgs(6));
        if (state.dataPipeHT->PipeHT(Item).SoilMaterialNum == 0) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
            ErrorsFound = true;
        } else {
            auto const *matSoil = s_mat->materials(state.dataPipeHT->PipeHT(Item).SoilMaterialNum);

            state.dataPipeHT->PipeHT(Item).SoilDensity = matSoil->Density;
            state.dataPipeHT->PipeHT(Item).SoilDepth = matSoil->Thickness;
            state.dataPipeHT->PipeHT(Item).SoilCp = matSoil->SpecHeat;
            state.dataPipeHT->PipeHT(Item).SoilConductivity = matSoil->Conductivity;
            state.dataPipeHT->PipeHT(Item).SoilThermAbs = matSoil->AbsorpThermal;
            state.dataPipeHT->PipeHT(Item).SoilSolarAbs = matSoil->AbsorpSolar;
            state.dataPipeHT->PipeHT(Item).SoilRoughness = matSoil->Roughness;
            state.dataPipeHT->PipeHT(Item).PipeDepth = state.dataPipeHT->PipeHT(Item).SoilDepth + state.dataPipeHT->PipeHT(Item).PipeID / 2.0;
            state.dataPipeHT->PipeHT(Item).DomainDepth = state.dataPipeHT->PipeHT(Item).PipeDepth * 2.0;
            state.dataPipeHT->PipeHT(Item).SoilDiffusivity = state.dataPipeHT->PipeHT(Item).SoilConductivity /
                                                             (state.dataPipeHT->PipeHT(Item).SoilDensity * state.dataPipeHT->PipeHT(Item).SoilCp);
            state.dataPipeHT->PipeHT(Item).SoilDiffusivityPerDay = state.dataPipeHT->PipeHT(Item).SoilDiffusivity * Constant::rSecsInDay;

            // Mesh the cartesian domain
            state.dataPipeHT->PipeHT(Item).NumDepthNodes = NumberOfDepthNodes;
            state.dataPipeHT->PipeHT(Item).PipeNodeDepth = state.dataPipeHT->PipeHT(Item).NumDepthNodes / 2;
            state.dataPipeHT->PipeHT(Item).PipeNodeWidth = state.dataPipeHT->PipeHT(Item).NumDepthNodes / 2;
            state.dataPipeHT->PipeHT(Item).DomainDepth = state.dataPipeHT->PipeHT(Item).PipeDepth * 2.0;
            state.dataPipeHT->PipeHT(Item).dSregular =
                state.dataPipeHT->PipeHT(Item).DomainDepth / (state.dataPipeHT->PipeHT(Item).NumDepthNodes - 1);
        }

        if (state.dataPipeHT->PipeHT(Item).ConstructionNum != 0) {
            state.dataPipeHT->PipeHT(Item).ValidatePipeConstruction(state,
                                                                    s_ipsc->cCurrentModuleObject,
                                                                    s_ipsc->cAlphaArgs(2),
                                                                    s_ipsc->cAlphaFieldNames(2),
                                                                    state.dataPipeHT->PipeHT(Item).ConstructionNum,
                                                                    ErrorsFound);
        }

        // Get ground temperature model
        GroundTemp::ModelType gtmType = static_cast<GroundTemp::ModelType>(getEnumValue(GroundTemp::modelTypeNamesUC, s_ipsc->cAlphaArgs(7)));
        if (gtmType == GroundTemp::ModelType::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
            ErrorsFound = true;
        }

        state.dataPipeHT->PipeHT(Item).groundTempModel = GroundTemp::GetGroundTempModelAndInit(state, gtmType, s_ipsc->cAlphaArgs(8));

        // Select number of pipe sections.  Hanby's optimal number of 20 section is selected.
        state.dataPipeHT->PipeHT(Item).NumSections = NumPipeSections;

        // For buried pipes, we need to allocate the cartesian finite difference array
        state.dataPipeHT->PipeHT(Item).T.allocate(state.dataPipeHT->PipeHT(Item).PipeNodeWidth,
                                                  state.dataPipeHT->PipeHT(Item).NumDepthNodes,
                                                  state.dataPipeHT->PipeHT(Item).NumSections,
                                                  TimeIndex::Tentative);
        state.dataPipeHT->PipeHT(Item).T = 0.0;

    } // PipeUG input loop

    for (Item = 1; Item <= state.dataPipeHT->nsvNumOfPipeHT; ++Item) {
        // Select number of pipe sections.  Hanby's optimal number of 20 section is selected.
        int NumSections = NumPipeSections;
        state.dataPipeHT->PipeHT(Item).NumSections = NumPipeSections;

        // We need to allocate the Hanby model arrays for all pipes, including buried
        state.dataPipeHT->PipeHT(Item).TentativeFluidTemp.allocate({0, NumSections});
        state.dataPipeHT->PipeHT(Item).TentativePipeTemp.allocate({0, NumSections});
        state.dataPipeHT->PipeHT(Item).FluidTemp.allocate({0, NumSections});
        state.dataPipeHT->PipeHT(Item).PreviousFluidTemp.allocate({0, NumSections});
        state.dataPipeHT->PipeHT(Item).PipeTemp.allocate({0, NumSections});
        state.dataPipeHT->PipeHT(Item).PreviousPipeTemp.allocate({0, NumSections});

        state.dataPipeHT->PipeHT(Item).TentativeFluidTemp = 0.0;
        state.dataPipeHT->PipeHT(Item).FluidTemp = 0.0;
        state.dataPipeHT->PipeHT(Item).PreviousFluidTemp = 0.0;
        state.dataPipeHT->PipeHT(Item).TentativePipeTemp = 0.0;
        state.dataPipeHT->PipeHT(Item).PipeTemp = 0.0;
        state.dataPipeHT->PipeHT(Item).PreviousPipeTemp = 0.0;

        // work out heat transfer areas (area per section)
        state.dataPipeHT->PipeHT(Item).InsideArea =
            Constant::Pi * state.dataPipeHT->PipeHT(Item).PipeID * state.dataPipeHT->PipeHT(Item).Length / NumSections;
        state.dataPipeHT->PipeHT(Item).OutsideArea =
            Constant::Pi * (state.dataPipeHT->PipeHT(Item).PipeOD + 2 * state.dataPipeHT->PipeHT(Item).InsulationThickness) *
            state.dataPipeHT->PipeHT(Item).Length / NumSections;

        // cross sectional area
        state.dataPipeHT->PipeHT(Item).SectionArea = Constant::Pi * 0.25 * pow_2(state.dataPipeHT->PipeHT(Item).PipeID);

        // pipe & insulation mass
        state.dataPipeHT->PipeHT(Item).PipeHeatCapacity =
            state.dataPipeHT->PipeHT(Item).PipeCp * state.dataPipeHT->PipeHT(Item).PipeDensity *
            (Constant::Pi * 0.25 * pow_2(state.dataPipeHT->PipeHT(Item).PipeOD) - state.dataPipeHT->PipeHT(Item).SectionArea); // the metal component
    }

    // final error check
    if (ErrorsFound) {
        ShowFatalError(state, "GetPipesHeatTransfer: Errors found in input. Preceding conditions cause termination.");
    }

    // Set up the output variables CurrentModuleObject='Pipe:Indoor/Outdoor/Underground'
    for (Item = 1; Item <= state.dataPipeHT->nsvNumOfPipeHT; ++Item) {

        SetupOutputVariable(state,
                            "Pipe Fluid Heat Transfer Rate",
                            Constant::Units::W,
                            state.dataPipeHT->PipeHT(Item).FluidHeatLossRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPipeHT->PipeHT(Item).Name);
        SetupOutputVariable(state,
                            "Pipe Fluid Heat Transfer Energy",
                            Constant::Units::J,
                            state.dataPipeHT->PipeHT(Item).FluidHeatLossEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            state.dataPipeHT->PipeHT(Item).Name);

        if (state.dataPipeHT->PipeHT(Item).EnvironmentPtr == EnvrnPtr::ZoneEnv) {
            SetupOutputVariable(state,
                                "Pipe Ambient Heat Transfer Rate",
                                Constant::Units::W,
                                state.dataPipeHT->PipeHT(Item).EnvironmentHeatLossRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataPipeHT->PipeHT(Item).Name);
            SetupOutputVariable(state,
                                "Pipe Ambient Heat Transfer Energy",
                                Constant::Units::J,
                                state.dataPipeHT->PipeHT(Item).EnvHeatLossEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataPipeHT->PipeHT(Item).Name);

            SetupZoneInternalGain(state,
                                  state.dataPipeHT->PipeHT(Item).EnvrZonePtr,
                                  state.dataPipeHT->PipeHT(Item).Name,
                                  DataHeatBalance::IntGainType::PipeIndoor,
                                  &state.dataPipeHT->PipeHT(Item).ZoneHeatGainRate);
        }

        SetupOutputVariable(state,
                            "Pipe Mass Flow Rate",
                            Constant::Units::kg_s,
                            state.dataPipeHT->PipeHT(Item).MassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPipeHT->PipeHT(Item).Name);
        SetupOutputVariable(state,
                            "Pipe Volume Flow Rate",
                            Constant::Units::m3_s,
                            state.dataPipeHT->PipeHT(Item).VolumeFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPipeHT->PipeHT(Item).Name);
        SetupOutputVariable(state,
                            "Pipe Inlet Temperature",
                            Constant::Units::C,
                            state.dataPipeHT->PipeHT(Item).FluidInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPipeHT->PipeHT(Item).Name);
        SetupOutputVariable(state,
                            "Pipe Outlet Temperature",
                            Constant::Units::C,
                            state.dataPipeHT->PipeHT(Item).FluidOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPipeHT->PipeHT(Item).Name);
    }
}

void PipeHTData::ValidatePipeConstruction(EnergyPlusData &state,
                                          std::string const &PipeType,         // module object of pipe (error messages)
                                          std::string const &ConstructionName, // construction name of pipe (error messages)
                                          std::string_view FieldName,          // fieldname of pipe (error messages)
                                          int const ConstructionNum,           // pointer into construction data
                                          bool &ErrorsFound                    // set to true if errors found here
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine, called from GetInput, validates the pipe construction usage.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 Density; // average density [kg/m^3]
    Real64 SpHeat;  // average specific heat [J/kg.K]
    Real64 Resistance = 0.0;
    Real64 TotThickness = 0.0;

    auto const &s_mat = state.dataMaterial;

    // CTF stuff
    int TotalLayers = state.dataConstruction->Construct(ConstructionNum).TotLayers;
    // get pipe properties
    if (TotalLayers == 1) { // no insulation layer
        auto const *mat = s_mat->materials(state.dataConstruction->Construct(ConstructionNum).LayerPoint(1));

        this->PipeConductivity = mat->Conductivity;
        this->PipeDensity = mat->Density;
        this->PipeCp = mat->SpecHeat;
        this->PipeOD = this->PipeID + 2.0 * mat->Thickness;
        this->InsulationOD = this->PipeOD;
        this->SumTK = mat->Thickness / mat->Conductivity;

    } else if (TotalLayers >= 2) { // first layers are insulation, last layer is pipe

        for (int LayerNum = 1; LayerNum <= TotalLayers - 1; ++LayerNum) {
            auto const *mat = state.dataMaterial->materials(state.dataConstruction->Construct(ConstructionNum).LayerPoint(LayerNum));
            Resistance += mat->Thickness / mat->Conductivity;
            Density = mat->Density * mat->Thickness;
            TotThickness += mat->Thickness;
            SpHeat = mat->SpecHeat * mat->Thickness;
            this->InsulationThickness = mat->Thickness;
            this->SumTK += mat->Thickness / mat->Conductivity;
        }

        this->InsulationResistance = Resistance;
        this->InsulationConductivity = TotThickness / Resistance;
        this->InsulationDensity = Density / TotThickness;
        this->InsulationCp = SpHeat / TotThickness;
        this->InsulationThickness = TotThickness;

        auto const *mat = state.dataMaterial->materials(state.dataConstruction->Construct(ConstructionNum).LayerPoint(TotalLayers));
        this->PipeConductivity = mat->Conductivity;
        this->PipeDensity = mat->Density;
        this->PipeCp = mat->SpecHeat;

        this->PipeOD = this->PipeID + 2.0 * mat->Thickness;
        this->InsulationOD = this->PipeOD + 2.0 * this->InsulationThickness;

    } else {
        ShowSevereError(
            state, format("{}: invalid {}=\"{}\", too many layers=[{}], only 1 or 2 allowed.", PipeType, FieldName, ConstructionName, TotalLayers));
        ErrorsFound = true;
    }
}

void PipeHTData::oneTimeInit_new(EnergyPlusData &state)
{
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state, this->Name, this->Type, this->plantLoc, errFlag, _, _, _, _, _);
    if (errFlag) {
        ShowFatalError(state, "InitPipesHeatTransfer: Program terminated due to previous condition(s).");
    }
}

void PipeHTData::InitPipesHeatTransfer(EnergyPlusData &state, bool const FirstHVACIteration // component number
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   July 2007
    //       MODIFIED       L. Gu, 6/19/08, pipe wall heat capacity has metal layer only
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine Resets the elements of the data structure as necessary
    // at the first step, and start of each call to simulated

    // METHODOLOGY EMPLOYED:
    // Check flags and update data structure

    // Using/Aliasing
    Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("InitPipesHeatTransfer");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    Real64 FirstTemperatures; // initial temperature of every node in pipe (set to inlet temp) [C]
    int TimeIndex;
    int LengthIndex;
    int DepthIndex;
    int WidthIndex;
    Real64 CurrentDepth;
    Real64 CurTemp;
    Real64 CurSimDay;
    bool PushArrays;

    // Assign variable
    CurSimDay = double(state.dataGlobal->DayOfSim);

    // some useful module variables
    state.dataPipeHT->nsvInletNodeNum = this->InletNodeNum;
    state.dataPipeHT->nsvOutletNodeNum = this->OutletNodeNum;
    state.dataPipeHT->nsvMassFlowRate = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).MassFlowRate;
    state.dataPipeHT->nsvInletTemp = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).Temp;

    // initialize temperatures by inlet node temp
    if ((state.dataGlobal->BeginSimFlag && this->BeginSimInit) || (state.dataGlobal->BeginEnvrnFlag && this->BeginSimEnvrn)) {

        if (this->EnvironmentPtr == EnvrnPtr::GroundEnv) {
            for (TimeIndex = TimeIndex::Previous; TimeIndex <= TimeIndex::Tentative; ++TimeIndex) {
                // Loop through all length, depth, and width of pipe to init soil temperature
                for (LengthIndex = 1; LengthIndex <= this->NumSections; ++LengthIndex) {
                    for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes; ++DepthIndex) {
                        for (WidthIndex = 1; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                            CurrentDepth = (DepthIndex - 1) * this->dSregular;
                            this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex) = this->TBND(state, CurrentDepth);
                        }
                    }
                }
            }
        }

        // We also need to re-init the Hanby arrays for all pipes, including buried
        FirstTemperatures = 21.0; // Node(InletNodeNum)%Temp
        this->TentativeFluidTemp = FirstTemperatures;
        this->FluidTemp = FirstTemperatures;
        this->PreviousFluidTemp = FirstTemperatures;
        this->TentativePipeTemp = FirstTemperatures;
        this->PipeTemp = FirstTemperatures;
        this->PreviousPipeTemp = FirstTemperatures;
        this->PreviousSimTime = 0.0;
        state.dataPipeHT->nsvDeltaTime = 0.0;
        state.dataPipeHT->nsvOutletTemp = 0.0;
        state.dataPipeHT->nsvEnvironmentTemp = 0.0;
        state.dataPipeHT->nsvEnvHeatLossRate = 0.0;
        state.dataPipeHT->nsvFluidHeatLossRate = 0.0;

        this->BeginSimInit = false;
        this->BeginSimEnvrn = false;
    }

    if (!state.dataGlobal->BeginSimFlag) {
        this->BeginSimInit = true;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->BeginSimEnvrn = true;
    }

    // time step in seconds
    state.dataPipeHT->nsvDeltaTime = TimeStepSysSec;
    state.dataPipeHT->nsvNumInnerTimeSteps = int(state.dataPipeHT->nsvDeltaTime / InnerDeltaTime);

    // previous temps are updated if necessary at start of timestep rather than end
    if ((FirstHVACIteration && this->FirstHVACupdateFlag) || (state.dataGlobal->BeginEnvrnFlag && this->BeginEnvrnupdateFlag)) {

        // We need to update boundary conditions here, as well as updating the arrays
        if (this->EnvironmentPtr == EnvrnPtr::GroundEnv) {

            // And then update Ground Boundary Conditions
            for (TimeIndex = 1; TimeIndex <= TimeIndex::Tentative; ++TimeIndex) {
                for (LengthIndex = 1; LengthIndex <= this->NumSections; ++LengthIndex) {
                    for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes; ++DepthIndex) {
                        // Farfield boundary
                        CurrentDepth = (DepthIndex - 1) * this->dSregular;
                        CurTemp = this->TBND(state, CurrentDepth);
                        this->T(1, DepthIndex, LengthIndex, TimeIndex) = CurTemp;
                    }
                    for (WidthIndex = 1; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                        // Bottom side of boundary
                        CurrentDepth = this->DomainDepth;
                        CurTemp = this->TBND(state, CurrentDepth);
                        this->T(WidthIndex, this->NumDepthNodes, LengthIndex, TimeIndex) = CurTemp;
                    }
                }
            }
        }

        // should next choose environment temperature according to coupled with air or ground
        switch (this->EnvironmentPtr) {
        case EnvrnPtr::GroundEnv: {
            // EnvironmentTemp = GroundTemp
        } break;
        case EnvrnPtr::OutsideAirEnv: {
            state.dataPipeHT->nsvEnvironmentTemp = state.dataEnvrn->OutDryBulbTemp;
        } break;
        case EnvrnPtr::ZoneEnv: {
            state.dataPipeHT->nsvEnvironmentTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->EnvrZonePtr).MAT;
        } break;
        case EnvrnPtr::ScheduleEnv: {
            state.dataPipeHT->nsvEnvironmentTemp = this->envrSched->getCurrentVal();
        } break;
        case EnvrnPtr::None: { // default to outside temp
            state.dataPipeHT->nsvEnvironmentTemp = state.dataEnvrn->OutDryBulbTemp;
        } break;
        default:
            break;
        }

        this->BeginEnvrnupdateFlag = false;
        this->FirstHVACupdateFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->BeginEnvrnupdateFlag = true;
    }
    if (!FirstHVACIteration) {
        this->FirstHVACupdateFlag = true;
    }

    // Calculate the current sim time for this pipe (not necessarily structure variable, but it is ok for consistency)
    this->CurrentSimTime = (state.dataGlobal->DayOfSim - 1) * 24 + state.dataGlobal->HourOfDay - 1 +
                           (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone + SysTimeElapsed;
    if (std::abs(this->CurrentSimTime - this->PreviousSimTime) > 1.0e-6) {
        PushArrays = true;
        this->PreviousSimTime = this->CurrentSimTime;
    } else {
        PushArrays = false; // Time hasn't passed, don't accept the tentative values yet!
    }

    if (PushArrays) {

        // If sim time has changed all values from previous runs should have been acceptable.
        // Thus we will now shift the arrays from 2>1 and 3>2 so we can then begin
        // to update 2 and 3 again.
        if (this->EnvironmentPtr == EnvrnPtr::GroundEnv) {
            for (LengthIndex = 2; LengthIndex <= this->NumSections; ++LengthIndex) {
                for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes; ++DepthIndex) {
                    for (WidthIndex = 2; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                        // This will essentially 'accept' the tentative values that were calculated last iteration
                        // as the new officially 'current' values
                        this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Current) =
                            this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative);
                    }
                }
            }
        }

        // Then update the Hanby near pipe model temperatures
        this->FluidTemp = this->TentativeFluidTemp;
        this->PipeTemp = this->TentativePipeTemp;

    } else { //  IF(.NOT. FirstHVACIteration)THEN

        // If we don't have FirstHVAC, the last iteration values were not accepted, and we should
        // not step through time.  Thus we will revert our T(3,:,:,:) array back to T(2,:,:,:) to
        // start over with the same values as last time.
        for (LengthIndex = 2; LengthIndex <= this->NumSections; ++LengthIndex) {
            for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes; ++DepthIndex) {
                for (WidthIndex = 2; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                    // This will essentially erase the past iterations and revert back to the correct values
                    this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative) =
                        this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Current);
                }
            }
        }

        // Similarly for Hanby model arrays
        this->TentativeFluidTemp = this->FluidTemp;
        this->TentativePipeTemp = this->PipeTemp;
    }

    // This still catches even in winter design day
    // Even though the loop eventually has no flow rate, it appears it initializes to a value, then converges to OFF
    // Thus, this is called at the beginning of every time step once.

    this->FluidSpecHeat =
        state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, state.dataPipeHT->nsvInletTemp, RoutineName);
    this->FluidDensity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, state.dataPipeHT->nsvInletTemp, RoutineName);

    // At this point, for all Pipe:Interior objects we should zero out the energy and rate arrays
    this->FluidHeatLossRate = 0.0;
    this->FluidHeatLossEnergy = 0.0;
    this->EnvironmentHeatLossRate = 0.0;
    this->EnvHeatLossEnergy = 0.0;
    this->ZoneHeatGainRate = 0.0;
    state.dataPipeHT->nsvFluidHeatLossRate = 0.0;
    state.dataPipeHT->nsvEnvHeatLossRate = 0.0;
    state.dataPipeHT->nsvOutletTemp = 0.0;

    if (this->FluidDensity > 0.0) {
        // The density will only be zero the first time through, which will be a warmup day, and not reported
        state.dataPipeHT->nsvVolumeFlowRate = state.dataPipeHT->nsvMassFlowRate / this->FluidDensity;
    }
}

//==============================================================================

void PipeHTData::CalcPipesHeatTransfer(EnergyPlusData &state, ObjexxFCL::Optional_int_const LengthIndex)
{

    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   July 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does all of the stuff that is necessary to simulate
    // a Pipe Heat Transfer.  Calls are made to appropriate routines
    // for heat transfer coefficients

    // METHODOLOGY EMPLOYED:
    // Differential equations for pipe and fluid nodes along the pipe are solved
    // taking backward differences in time.
    // The heat loss/gain calculations are run continuously, even when the loop is off.
    // Fluid temps will drift according to environmental conditions when there is zero flow.

    // REFERENCES:

    // Using/Aliasing
    using namespace DataEnvironment;

    // fluid node heat balance (see engineering doc).
    Real64 A1(0.0); // sum of the heat balance terms
    Real64 A2(0.0); // mass flow term
    Real64 A3(0.0); // inside pipe wall convection term
    Real64 A4(0.0); // fluid node heat capacity term
    // pipe wall node heat balance (see engineering doc).
    Real64 B1(0.0); // sum of the heat balance terms
    Real64 B2(0.0); // inside pipe wall convection term
    Real64 B3(0.0); // outside pipe wall convection term
    Real64 B4(0.0); // fluid node heat capacity term

    Real64 AirConvCoef(0.0);           // air-pipe convection coefficient
    Real64 FluidConvCoef(0.0);         // fluid-pipe convection coefficient
    Real64 EnvHeatTransCoef(0.0);      // external convection coefficient (outside pipe)
    Real64 FluidNodeHeatCapacity(0.0); // local var for MCp for single node of pipe

    Real64 TempBelow;
    Real64 TempBeside;
    Real64 TempAbove;
    Real64 Numerator;
    Real64 Denominator;
    Real64 SurfaceTemp;

    // traps fluid properties problems such as freezing conditions
    if (this->FluidSpecHeat <= 0.0 || this->FluidDensity <= 0.0) {
        // leave the state of the pipe as it was
        state.dataPipeHT->nsvOutletTemp = this->TentativeFluidTemp(this->NumSections);
        // set heat transfer rates to zero for consistency
        state.dataPipeHT->nsvEnvHeatLossRate = 0.0;
        state.dataPipeHT->nsvFluidHeatLossRate = 0.0;
        return;
    }

    //  AirConvCoef =  OutsidePipeHeatTransCoef(PipeHTNum)
    // Revised by L. Gu by including insulation conductance 6/19/08

    if (this->EnvironmentPtr != EnvrnPtr::GroundEnv) {
        AirConvCoef = 1.0 / (1.0 / this->OutsidePipeHeatTransCoef(state) + this->InsulationResistance);
    }

    FluidConvCoef = this->CalcPipeHeatTransCoef(state, state.dataPipeHT->nsvInletTemp, state.dataPipeHT->nsvMassFlowRate, this->PipeID);

    // heat transfer to air or ground
    switch (this->EnvironmentPtr) {
    case EnvrnPtr::GroundEnv: {
        // Approximate conductance using ground conductivity, (h=k/L), where L is grid spacing
        // between pipe wall and next closest node.
        EnvHeatTransCoef = this->SoilConductivity / (this->dSregular - (this->PipeID / 2.0));
    } break;
    case EnvrnPtr::OutsideAirEnv: {
        EnvHeatTransCoef = AirConvCoef;
    } break;
    case EnvrnPtr::ZoneEnv: {
        EnvHeatTransCoef = AirConvCoef;
    } break;
    case EnvrnPtr::ScheduleEnv: {
        EnvHeatTransCoef = AirConvCoef;
    } break;
    case EnvrnPtr::None: {
        EnvHeatTransCoef = 0.0;
    } break;
    default: {
        EnvHeatTransCoef = 0.0;
    } break;
    }

    // work out the coefficients
    FluidNodeHeatCapacity =
        this->SectionArea * this->Length / this->NumSections * this->FluidSpecHeat * this->FluidDensity; // Mass of Node x Specific heat

    // coef of fluid heat balance
    A1 = FluidNodeHeatCapacity + state.dataPipeHT->nsvMassFlowRate * this->FluidSpecHeat * state.dataPipeHT->nsvDeltaTime +
         FluidConvCoef * this->InsideArea * state.dataPipeHT->nsvDeltaTime;

    A2 = state.dataPipeHT->nsvMassFlowRate * this->FluidSpecHeat * state.dataPipeHT->nsvDeltaTime;

    A3 = FluidConvCoef * this->InsideArea * state.dataPipeHT->nsvDeltaTime;

    A4 = FluidNodeHeatCapacity;

    // coef of pipe heat balance
    B1 = this->PipeHeatCapacity + FluidConvCoef * this->InsideArea * state.dataPipeHT->nsvDeltaTime +
         EnvHeatTransCoef * this->OutsideArea * state.dataPipeHT->nsvDeltaTime;

    B2 = A3;

    B3 = EnvHeatTransCoef * this->OutsideArea * state.dataPipeHT->nsvDeltaTime;

    B4 = this->PipeHeatCapacity;

    this->TentativeFluidTemp(0) = state.dataPipeHT->nsvInletTemp;

    this->TentativePipeTemp(0) = this->PipeTemp(1); // for convenience

    if (present(LengthIndex)) { // Just simulate the single section if being called from Pipe:Underground

        int PipeDepth = this->PipeNodeDepth;
        int PipeWidth = this->PipeNodeWidth;
        TempBelow = this->T(PipeWidth, PipeDepth + 1, LengthIndex, TimeIndex::Current);
        TempBeside = this->T(PipeWidth - 1, PipeDepth, LengthIndex, TimeIndex::Current);
        TempAbove = this->T(PipeWidth, PipeDepth - 1, LengthIndex, TimeIndex::Current);
        state.dataPipeHT->nsvEnvironmentTemp = (TempBelow + TempBeside + TempAbove) / 3.0;

        this->TentativeFluidTemp(LengthIndex) = (A2 * this->TentativeFluidTemp(LengthIndex - 1) +
                                                 A3 / B1 * (B3 * state.dataPipeHT->nsvEnvironmentTemp + B4 * this->PreviousPipeTemp(LengthIndex)) +
                                                 A4 * this->PreviousFluidTemp(LengthIndex)) /
                                                (A1 - A3 * B2 / B1);

        this->TentativePipeTemp(LengthIndex) =
            (B2 * this->TentativeFluidTemp(LengthIndex) + B3 * state.dataPipeHT->nsvEnvironmentTemp + B4 * this->PreviousPipeTemp(LengthIndex)) / B1;

        // Get exterior surface temperature from energy balance at the surface
        Numerator = state.dataPipeHT->nsvEnvironmentTemp - this->TentativeFluidTemp(LengthIndex);
        Denominator = EnvHeatTransCoef * ((1 / EnvHeatTransCoef) + this->SumTK);
        SurfaceTemp = state.dataPipeHT->nsvEnvironmentTemp - Numerator / Denominator;

        // keep track of environmental heat loss rate - not same as fluid loss at same time
        state.dataPipeHT->nsvEnvHeatLossRate += EnvHeatTransCoef * this->OutsideArea * (SurfaceTemp - state.dataPipeHT->nsvEnvironmentTemp);

    } else { // Simulate all sections at once if not pipe:underground

        // start loop along pipe
        // b1 must not be zero but this should have been checked on input
        for (int curnode = 1; curnode <= this->NumSections; ++curnode) {
            this->TentativeFluidTemp(curnode) = (A2 * this->TentativeFluidTemp(curnode - 1) +
                                                 A3 / B1 * (B3 * state.dataPipeHT->nsvEnvironmentTemp + B4 * this->PreviousPipeTemp(curnode)) +
                                                 A4 * this->PreviousFluidTemp(curnode)) /
                                                (A1 - A3 * B2 / B1);

            this->TentativePipeTemp(curnode) =
                (B2 * this->TentativeFluidTemp(curnode) + B3 * state.dataPipeHT->nsvEnvironmentTemp + B4 * this->PreviousPipeTemp(curnode)) / B1;

            // Get exterior surface temperature from energy balance at the surface
            Numerator = state.dataPipeHT->nsvEnvironmentTemp - this->TentativeFluidTemp(curnode);
            Denominator = EnvHeatTransCoef * ((1 / EnvHeatTransCoef) + this->SumTK);
            SurfaceTemp = state.dataPipeHT->nsvEnvironmentTemp - Numerator / Denominator;

            // Keep track of environmental heat loss
            state.dataPipeHT->nsvEnvHeatLossRate += EnvHeatTransCoef * this->OutsideArea * (SurfaceTemp - state.dataPipeHT->nsvEnvironmentTemp);
        }
    }

    state.dataPipeHT->nsvFluidHeatLossRate =
        state.dataPipeHT->nsvMassFlowRate * this->FluidSpecHeat * (this->TentativeFluidTemp(0) - this->TentativeFluidTemp(this->NumSections));

    state.dataPipeHT->nsvOutletTemp = this->TentativeFluidTemp(this->NumSections);
}

//==============================================================================

void PipeHTData::CalcBuriedPipeSoil(EnergyPlusData &state) // Current Simulation Pipe Number
{

    //       AUTHOR         Edwin Lee
    //       DATE WRITTEN   May 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does all of the stuff that is necessary to simulate
    // soil heat transfer with a Buried Pipe.

    // METHODOLOGY EMPLOYED:
    // An implicit pseudo 3D finite difference grid
    // is set up, which simulates transient behavior in the soil.
    // This then interfaces with the Hanby model for near-pipe region

    // Using/Aliasing
    using Convect::CalcASHRAESimpExtConvCoeff;

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr NumSections(20);
    Real64 constexpr ConvCrit(0.05);
    int constexpr MaxIterations(200);
    Real64 constexpr StefBoltzmann(5.6697e-08); // Stefan-Boltzmann constant

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int IterationIndex(0);    // Index when stepping through equations
    int DepthIndex(0);        // Index for nodes in the depth direction
    int WidthIndex(0);        // Index for nodes in the width direction
    Real64 ConvCoef(0.0);     // Current convection coefficient = f(Wind Speed,Roughness)
    Real64 RadCoef(0.0);      // Current radiation coefficient
    Real64 QSolAbsorbed(0.0); // Current total solar energy absorbed
    Array3D<Real64> T_O(this->PipeNodeWidth, this->NumDepthNodes, NumSections);

    // Local variable placeholders for code readability
    Real64 A1(0.0);                                                               // Placeholder for CoefA1
    Real64 A2(0.0);                                                               // Placeholder for CoefA2
    Real64 NodeBelow(0.0);                                                        // Placeholder for Node temp below current node
    Real64 NodeAbove(0.0);                                                        // Placeholder for Node temp above current node
    Real64 NodeRight(0.0);                                                        // Placeholder for Node temp to the right of current node
    Real64 NodeLeft(0.0);                                                         // Placeholder for Node temp to the left of current node
    Real64 NodePast(0.0);                                                         // Placeholder for Node temp at current node but previous time step
    Real64 PastNodeTempAbs(0.0);                                                  // Placeholder for absolute temperature (K) version of NodePast
    Real64 Ttemp(0.0);                                                            // Placeholder for a current temperature node in convergence check
    Real64 SkyTempAbs(0.0);                                                       // Placeholder for current sky temperature in Kelvin
    Material::SurfaceRoughness TopRoughness(Material::SurfaceRoughness::Invalid); // Placeholder for soil surface roughness
    Real64 TopThermAbs(0.0);                                                      // Placeholder for soil thermal radiation absorptivity
    Real64 TopSolarAbs(0.0);                                                      // Placeholder for soil solar radiation absorptivity
    Real64 kSoil(0.0);                                                            // Placeholder for soil conductivity
    Real64 dS(0.0);                                                               // Placeholder for soil grid spacing
    Real64 rho(0.0);                                                              // Placeholder for soil density
    Real64 Cp(0.0);                                                               // Placeholder for soil specific heat

    // There are a number of coefficients which change through the simulation, and they are updated here
    this->FourierDS = this->SoilDiffusivity * state.dataPipeHT->nsvDeltaTime / pow_2(this->dSregular); // Eq. D4
    this->CoefA1 = this->FourierDS / (1 + 4 * this->FourierDS);                                        // Eq. D2
    this->CoefA2 = 1 / (1 + 4 * this->FourierDS);                                                      // Eq. D3

    for (IterationIndex = 1; IterationIndex <= MaxIterations; ++IterationIndex) {
        if (IterationIndex == MaxIterations) {
            ShowWarningError(state, format("BuriedPipeHeatTransfer: Large number of iterations detected in object: {}", this->Name));
        }

        // Store computed values in T_O array
        for (int LengthIndex = 2; LengthIndex <= this->NumSections; ++LengthIndex) {
            for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes - 1; ++DepthIndex) {
                for (WidthIndex = 2; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                    T_O(WidthIndex, DepthIndex, LengthIndex) = this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative);
                }
            }
        }

        // Loop along entire length of pipe, analyzing cross sects
        for (int LengthIndex = 1; LengthIndex <= this->NumSections; ++LengthIndex) {
            for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes - 1; ++DepthIndex) {
                for (WidthIndex = 2; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {

                    if (DepthIndex == 1) { // Soil Surface Boundary

                        // If on soil boundary, load up local variables and perform calculations
                        NodePast = this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Previous);
                        PastNodeTempAbs = NodePast + Constant::Kelvin;
                        SkyTempAbs = state.dataEnvrn->SkyTemp + Constant::Kelvin;
                        TopRoughness = this->SoilRoughness;
                        TopThermAbs = this->SoilThermAbs;
                        TopSolarAbs = this->SoilSolarAbs;
                        kSoil = this->SoilConductivity;
                        dS = this->dSregular;
                        rho = this->SoilDensity;
                        Cp = this->SoilCp;

                        // ASHRAE simple convection coefficient model for external surfaces.
                        this->OutdoorConvCoef = CalcASHRAESimpExtConvCoeff(TopRoughness, state.dataEnvrn->WindSpeed);
                        ConvCoef = this->OutdoorConvCoef;

                        // thermal radiation coefficient using surf temp from past time step
                        if (std::abs(PastNodeTempAbs - SkyTempAbs) > Constant::rTinyValue) {
                            RadCoef = StefBoltzmann * TopThermAbs * (pow_4(PastNodeTempAbs) - pow_4(SkyTempAbs)) / (PastNodeTempAbs - SkyTempAbs);
                        } else {
                            RadCoef = 0.0;
                        }

                        // total absorbed solar - no ground solar
                        QSolAbsorbed =
                            TopSolarAbs * (max(state.dataEnvrn->SOLCOS(3), 0.0) * state.dataEnvrn->BeamSolarRad + state.dataEnvrn->DifSolarRad);

                        // If sun is not exposed, then turn off both solar and thermal radiation
                        if (!this->SolarExposed) {
                            RadCoef = 0.0;
                            QSolAbsorbed = 0.0;
                        }

                        if (WidthIndex == this->PipeNodeWidth) { // Symmetric centerline boundary

                            //-Coefficients and Temperatures
                            NodeBelow = this->T(WidthIndex, DepthIndex + 1, LengthIndex, TimeIndex::Current);
                            NodeLeft = this->T(WidthIndex - 1, DepthIndex, LengthIndex, TimeIndex::Current);

                            //-Update Equation, basically a detailed energy balance at the surface
                            this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative) =
                                (QSolAbsorbed + RadCoef * state.dataEnvrn->SkyTemp + ConvCoef * state.dataEnvrn->OutDryBulbTemp +
                                 (kSoil / dS) * (NodeBelow + 2 * NodeLeft) + (rho * Cp / state.dataPipeHT->nsvDeltaTime) * NodePast) /
                                (RadCoef + ConvCoef + 3 * (kSoil / dS) + (rho * Cp / state.dataPipeHT->nsvDeltaTime));

                        } else { // Soil surface, but not on centerline

                            //-Coefficients and Temperatures
                            NodeBelow = this->T(WidthIndex, DepthIndex + 1, LengthIndex, TimeIndex::Current);
                            NodeLeft = this->T(WidthIndex - 1, DepthIndex, LengthIndex, TimeIndex::Current);
                            NodeRight = this->T(WidthIndex + 1, DepthIndex, LengthIndex, TimeIndex::Current);

                            //-Update Equation
                            this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative) =
                                (QSolAbsorbed + RadCoef * state.dataEnvrn->SkyTemp + ConvCoef * state.dataEnvrn->OutDryBulbTemp +
                                 (kSoil / dS) * (NodeBelow + NodeLeft + NodeRight) + (rho * Cp / state.dataPipeHT->nsvDeltaTime) * NodePast) /
                                (RadCoef + ConvCoef + 3 * (kSoil / dS) + (rho * Cp / state.dataPipeHT->nsvDeltaTime));

                        } // Soil-to-air surface node structure

                    } else if (WidthIndex == this->PipeNodeWidth) { // On Symmetric centerline boundary

                        if (DepthIndex == this->PipeNodeDepth) { // On the node containing the pipe

                            //-Call to simulate a single pipe segment (by passing OPTIONAL LengthIndex argument)
                            this->CalcPipesHeatTransfer(state, LengthIndex);

                            //-Update node for cartesian system
                            this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative) = this->PipeTemp(LengthIndex);

                        } else { // Not surface node

                            //-Coefficients and Temperatures
                            NodeLeft = this->T(WidthIndex - 1, DepthIndex, LengthIndex, TimeIndex::Current);
                            NodeAbove = this->T(WidthIndex, DepthIndex - 1, LengthIndex, TimeIndex::Current);
                            NodeBelow = this->T(WidthIndex, DepthIndex + 1, LengthIndex, TimeIndex::Current);
                            NodePast = this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Current - 1);
                            A1 = this->CoefA1;
                            A2 = this->CoefA2;

                            //-Update Equation
                            this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative) =
                                A1 * (NodeBelow + NodeAbove + 2 * NodeLeft) + A2 * NodePast;

                        } // Symmetric centerline node structure

                    } else { // All Normal Interior Nodes

                        //-Coefficients and Temperatures
                        A1 = this->CoefA1;
                        A2 = this->CoefA2;
                        NodeBelow = this->T(WidthIndex, DepthIndex + 1, LengthIndex, TimeIndex::Current);
                        NodeAbove = this->T(WidthIndex, DepthIndex - 1, LengthIndex, TimeIndex::Current);
                        NodeRight = this->T(WidthIndex + 1, DepthIndex, LengthIndex, TimeIndex::Current);
                        NodeLeft = this->T(WidthIndex - 1, DepthIndex, LengthIndex, TimeIndex::Current);
                        NodePast = this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Current - 1);

                        //-Update Equation
                        this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative) =
                            A1 * (NodeBelow + NodeAbove + NodeRight + NodeLeft) + A2 * NodePast; // Eq. D1
                    }
                }
            }
        }

        // Check for convergence
        for (int LengthIndex = 2; LengthIndex <= this->NumSections; ++LengthIndex) {
            for (DepthIndex = 1; DepthIndex <= this->NumDepthNodes - 1; ++DepthIndex) {
                for (WidthIndex = 2; WidthIndex <= this->PipeNodeWidth; ++WidthIndex) {
                    Ttemp = this->T(WidthIndex, DepthIndex, LengthIndex, TimeIndex::Tentative);
                    if (std::abs(T_O(WidthIndex, DepthIndex, LengthIndex) - Ttemp) > ConvCrit) {
                        goto IterationLoop_loop;
                    }
                }
            }
        }

        // If we didn't cycle back, then the system is converged
        // PipeHT(PipeHTNum)%PipeUGIters=IterationIndex
        goto IterationLoop_exit;

    IterationLoop_loop:;
    }
IterationLoop_exit:;
}

//==============================================================================

void PipeHTData::UpdatePipesHeatTransfer(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   July 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does any updating that needs to be done for
    // Pipe Heat Transfers. This routine must also set the outlet water conditions.

    // METHODOLOGY EMPLOYED:

    // REFERENCES:
    // na

    // SUBROUTINE ARGUMENT DEFINITIONS:
    // INTEGER, INTENT(IN) :: PipeHTNum       ! Index for the surface

    // SUBROUTINE PARAMETER DEFINITIONS:

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    // only outlet node temp should need updating
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).Temp = state.dataPipeHT->nsvOutletTemp;

    // pass everything else through
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).TempMin = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).TempMin;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).TempMax = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).TempMax;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).MassFlowRate =
        state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).MassFlowRateMin =
        state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).MassFlowRateMin;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).MassFlowRateMax =
        state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).MassFlowRateMax;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).MassFlowRateMinAvail =
        state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).MassFlowRateMinAvail;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).MassFlowRateMaxAvail =
        state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).MassFlowRateMaxAvail;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).Quality = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).Quality;
    // Only pass pressure if we aren't doing a pressure simulation
    switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).PressureSimType) {
    case DataPlant::PressSimType::NoPressure:
        state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).Press = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).Press;
        break;
    default:
        // Don't do anything
        break;
    }
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).Enthalpy = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).Enthalpy;
    state.dataLoopNodes->Node(state.dataPipeHT->nsvOutletNodeNum).HumRat = state.dataLoopNodes->Node(state.dataPipeHT->nsvInletNodeNum).HumRat;
}

//==============================================================================

void PipeHTData::ReportPipesHeatTransfer(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   July 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simply updates the report data

    // METHODOLOGY EMPLOYED:
    // Standard EnergyPlus methodology.

    // REFERENCES:
    // na

    // USE STATEMENTS:

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    // update flows and temps from module variables
    this->FluidInletTemp = state.dataPipeHT->nsvInletTemp;
    this->FluidOutletTemp = state.dataPipeHT->nsvOutletTemp;
    this->MassFlowRate = state.dataPipeHT->nsvMassFlowRate;
    this->VolumeFlowRate = state.dataPipeHT->nsvVolumeFlowRate;

    // update other variables from module variables
    this->FluidHeatLossRate = state.dataPipeHT->nsvFluidHeatLossRate;
    this->FluidHeatLossEnergy = state.dataPipeHT->nsvFluidHeatLossRate * state.dataPipeHT->nsvDeltaTime; // DeltaTime is in seconds
    this->PipeInletTemp = this->PipeTemp(1);
    this->PipeOutletTemp = this->PipeTemp(this->NumSections);

    // need to average the heat rate because it is now summing over multiple inner time steps
    this->EnvironmentHeatLossRate = state.dataPipeHT->nsvEnvHeatLossRate / state.dataPipeHT->nsvNumInnerTimeSteps;
    this->EnvHeatLossEnergy = this->EnvironmentHeatLossRate * state.dataPipeHT->nsvDeltaTime;

    // for zone heat gains, we assign the averaged heat rate over all inner time steps
    if (this->EnvironmentPtr == EnvrnPtr::ZoneEnv) {
        this->ZoneHeatGainRate = this->EnvironmentHeatLossRate;
    }
}

//==============================================================================

void PipeHTData::CalcZonePipesHeatGain(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Edwin Lee
    //       DATE WRITTEN   September 2008
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the zone internal gains due to pipe heat transfer objects.

    // METHODOLOGY EMPLOYED:
    // Sums the heat losses from all of the water heaters in the zone to add as a gain to the zone.

    // Using/Aliasing
    if (state.dataPipeHT->nsvNumOfPipeHT == 0) {
        return;
    }

    if (state.dataGlobal->BeginEnvrnFlag && state.dataPipeHT->MyEnvrnFlag) {
        for (auto &e : state.dataPipeHT->PipeHT) {
            e.ZoneHeatGainRate = 0.0;
        }
        state.dataPipeHT->MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataPipeHT->MyEnvrnFlag = true;
    }
}

//==============================================================================

Real64 PipeHTData::CalcPipeHeatTransCoef(EnergyPlusData &state,
                                         Real64 const Temperature,  // Temperature of water entering the surface, in C
                                         Real64 const MassFlowRate, // Mass flow rate, in kg/s
                                         Real64 const Diameter      // Pipe diameter, m
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Simon Rees
    //       DATE WRITTEN   July 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates pipe/fluid heat transfer coefficients.
    // This routine is adapted from that in the low temp radiant surface model.

    // METHODOLOGY EMPLOYED:
    // Currently assumes water data when calculating Pr and Re

    // REFERENCES:
    // See RadiantSystemLowTemp module.
    // Property data for water shown below as parameters taken from
    // Incropera and DeWitt, Introduction to Heat Transfer, Table A.6.
    // Heat exchanger information also from Incropera and DeWitt.
    // Code based loosely on code from IBLAST program (research version)

    // Return value
    Real64 CalcPipeHeatTransCoef;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("PipeHeatTransfer::CalcPipeHeatTransCoef: ");
    Real64 constexpr MaxLaminarRe(2300.0); // Maximum Reynolds number for laminar flow
    int constexpr NumOfPropDivisions(13);  // intervals in property correlation
    static constexpr std::array<Real64, NumOfPropDivisions> Temps = {
        1.85, 6.85, 11.85, 16.85, 21.85, 26.85, 31.85, 36.85, 41.85, 46.85, 51.85, 56.85, 61.85}; // Temperature, in C
    static constexpr std::array<Real64, NumOfPropDivisions> Pr = {
        12.22, 10.26, 8.81, 7.56, 6.62, 5.83, 5.20, 4.62, 4.16, 3.77, 3.42, 3.15, 2.88}; // Prandtl number (dimensionless)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 InterpFrac;
    Real64 NuD;
    Real64 ReD;
    Real64 Kactual;
    Real64 MUactual;
    Real64 PRactual;
    int LoopNum;

    // retrieve loop index for this component so we can look up fluid properties
    LoopNum = this->plantLoc.loopNum;

    // since the fluid properties routine doesn't have Prandtl, we'll just use water values
    int idx = 0;
    while (idx < NumOfPropDivisions) {
        if (Temperature < Temps[idx]) {
            break;
        }
        ++idx;
    }

    if (idx == 0) {
        PRactual = Pr[idx];
    } else if (idx >= NumOfPropDivisions) {
        PRactual = Pr[NumOfPropDivisions - 1];
    } else {
        InterpFrac = (Temperature - Temps[idx - 1]) / (Temps[idx] - Temps[idx - 1]);
        PRactual = Pr[idx - 1] + InterpFrac * (Pr[idx] - Pr[idx - 1]);
    }

    // look up conductivity and viscosity
    Kactual = state.dataPlnt->PlantLoop(LoopNum).glycol->getConductivity(state, this->FluidTemp(0), RoutineName); // W/m-K
    MUactual = state.dataPlnt->PlantLoop(LoopNum).glycol->getViscosity(state, this->FluidTemp(0), RoutineName) /
               1000.0; // Note fluid properties routine returns mPa-s, we need Pa-s

    // Calculate the Reynold's number from RE=(4*Mdot)/(Pi*Mu*Diameter) - as RadiantSysLowTemp
    ReD = 4.0 * MassFlowRate / (Constant::Pi * MUactual * Diameter);

    if (ReD == 0.0) { // No flow

        // For now just leave it how it was doing it before
        NuD = 3.66;
        // Although later it would be nice to have a natural convection correlation

    } else { // Calculate the Nusselt number based on what flow regime one is in

        if (ReD >= MaxLaminarRe) { // Turbulent flow --> use Colburn equation
            NuD = 0.023 * std::pow(ReD, 0.8) * std::pow(PRactual, 1.0 / 3.0);
        } else { // Laminar flow --> use constant surface temperature relation
            NuD = 3.66;
        }
    }

    CalcPipeHeatTransCoef = Kactual * NuD / Diameter;

    return CalcPipeHeatTransCoef;
}

//==============================================================================

Real64 PipeHTData::OutsidePipeHeatTransCoef(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   July 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the convection heat transfer
    // coefficient for a cylinder in cross flow.

    // REFERENCES:
    // Fundamentals of Heat and Mass Transfer: Incropera and DeWitt, 4th ed.
    // p. 369-370 (Eq. 7:55b)

    // Return value
    Real64 OutsidePipeHeatTransCoef;

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr Pr(0.7);           // Prandl number for air (assume constant)
    Real64 constexpr CondAir(0.025);    // thermal conductivity of air (assume constant) [W/m.K]
    Real64 constexpr RoomAirVel(0.381); // room air velocity of 75 ft./min [m/s]
    Real64 constexpr NaturalConvNusselt(0.36);
    // Nusselt for natural convection for horizontal cylinder
    // from: Correlations for Convective Heat Transfer
    //      Dr. Bernhard Spang
    //      Chemical Engineers' Resource Page: http://www.cheresources.com/convection.pdf
    int constexpr NumOfParamDivisions(5); // intervals in property correlation
    int constexpr NumOfPropDivisions(12); // intervals in property correlation

    static constexpr std::array<Real64, NumOfParamDivisions> CCoef = {0.989, 0.911, 0.683, 0.193, 0.027};         // correlation coefficient
    static constexpr std::array<Real64, NumOfParamDivisions> mExp = {0.33, 0.385, 0.466, 0.618, 0.805};           // exponent
    static constexpr std::array<Real64, NumOfParamDivisions> UpperBound = {4.0, 40.0, 4000.0, 40000.0, 400000.0}; // upper bound of correlation range
    static constexpr std::array<Real64, NumOfPropDivisions> Temperature = {
        -73.0, -23.0, -10.0, 0.0, 10.0, 20.0, 27.0, 30.0, 40.0, 50.0, 76.85, 126.85}; // temperature [C]
    static constexpr std::array<Real64, NumOfPropDivisions> DynVisc = {
        75.52e-7, 11.37e-6, 12.44e-6, 13.3e-6, 14.18e-6, 15.08e-6, 15.75e-6, 16e-6, 16.95e-6, 17.91e-6, 20.92e-6, 26.41e-6}; // dynamic
    // viscosity
    // [m^2/s]

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int idx;
    Real64 NuD;
    Real64 ReD;
    Real64 Coef;
    Real64 rExp;
    Real64 AirVisc;
    Real64 AirVel;
    Real64 AirTemp;
    Real64 PipeOD;
    bool ViscositySet;
    bool CoefSet;

    // Set environmental variables
    switch (this->Type) {
    case DataPlant::PlantEquipmentType::PipeInterior: {
        switch (this->EnvironmentPtr) {
        case EnvrnPtr::ScheduleEnv: {
            AirTemp = this->envrSched->getCurrentVal();
            AirVel = this->envrVelSched->getCurrentVal();
        } break;
        case EnvrnPtr::ZoneEnv: {
            AirTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->EnvrZonePtr).MAT;
            AirVel = RoomAirVel;
        } break;
        default:
            break;
        }
    } break;
    case DataPlant::PlantEquipmentType::PipeExterior: {
        switch (this->EnvironmentPtr) {
        case EnvrnPtr::OutsideAirEnv: {
            AirTemp = state.dataLoopNodes->Node(this->EnvrAirNodeNum).Temp;
            AirVel = state.dataEnvrn->WindSpeed;
        } break;
        default:
            break;
        }
    } break;
    default:
        break;
    }

    PipeOD = this->InsulationOD;

    ViscositySet = false;
    for (idx = 0; idx < NumOfPropDivisions; ++idx) {
        if (AirTemp <= Temperature[idx]) {
            AirVisc = DynVisc[idx];
            ViscositySet = true;
            break;
        }
    }

    if (!ViscositySet) {
        AirVisc = DynVisc[NumOfPropDivisions - 1];
        if (AirTemp > Temperature[NumOfPropDivisions - 1]) {
            ShowWarningError(state,
                             format("Heat Transfer Pipe = {}Viscosity out of range, air temperature too high, setting to upper limit.", this->Name));
        }
    }

    // Calculate the Reynold's number
    CoefSet = false;
    if (AirVisc > 0.0) {
        ReD = AirVel * PipeOD / (AirVisc);
    }

    for (idx = 0; idx < NumOfParamDivisions; ++idx) {
        if (ReD <= UpperBound[idx]) {
            Coef = CCoef[idx];
            rExp = mExp[idx];
            CoefSet = true;
            break;
        }
    }

    if (!CoefSet) {
        Coef = CCoef[NumOfParamDivisions - 1];
        rExp = mExp[NumOfParamDivisions - 1];
        if (ReD > UpperBound[NumOfParamDivisions - 1]) {
            ShowWarningError(state, format("Heat Transfer Pipe = {}Reynolds Number out of range, setting coefficients to upper limit.", this->Name));
        }
    }

    // Calculate the Nusselt number
    NuD = Coef * std::pow(ReD, rExp) * std::pow(Pr, 1.0 / 3.0);

    // If the wind speed is too small, we need to use natural convection behavior:
    NuD = max(NuD, NaturalConvNusselt);

    // h = (k)(Nu)/D
    OutsidePipeHeatTransCoef = CondAir * NuD / PipeOD;

    return OutsidePipeHeatTransCoef;
}

//==============================================================================

Real64 PipeHTData::TBND(EnergyPlusData &state,
                        Real64 const z // Current Depth
)
{

    //       AUTHOR         Edwin Lee
    //       DATE WRITTEN   December 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Returns a temperature to be used on the boundary of the buried pipe model domain

    // METHODOLOGY EMPLOYED:

    // REFERENCES: See Module Level Description

    // Using/Aliasing
    Real64 curSimTime = state.dataGlobal->DayOfSim * Constant::rSecsInDay;
    return this->groundTempModel->getGroundTempAtTimeInSeconds(state, z, curSimTime);
}

void PipeHTData::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

//===============================================================================

//===============================================================================

} // namespace EnergyPlus::PipeHeatTransfer
