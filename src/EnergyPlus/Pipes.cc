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
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Pipes.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::Pipes {

// Module containing the routines dealing with the <module_name>

// MODULE INFORMATION:
//       AUTHOR         <author>
//       DATE WRITTEN   <date_written>
//       MODIFIED       Rahul Chillar , Jan 2005
//       RE-ENGINEERED  na

PlantComponent *LocalPipeData::factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string const &objectName)
{
    // Process the input data for pipes if it hasn't been done already
    if (state.dataPipes->GetPipeInputFlag) {
        GetPipeInput(state);
        state.dataPipes->GetPipeInputFlag = false;
    }
    // Now look for this particular pipe in the list
    for (auto &pipe : state.dataPipes->LocalPipe) {
        if (pipe.Type == objectType && pipe.Name == objectName) {
            return &pipe;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalPipeDataFactory: Error getting inputs for pipe named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void LocalPipeData::simulate(EnergyPlusData &state,
                             [[maybe_unused]] const PlantLocation &calledFromLocation,
                             [[maybe_unused]] bool const FirstHVACIteration,
                             [[maybe_unused]] Real64 &CurLoad,
                             [[maybe_unused]] bool const RunFlag)
{

    if (state.dataGlobal->BeginEnvrnFlag && this->EnvrnFlag) {
        this->initEachEnvironment(state);
        this->EnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->EnvrnFlag = true;
    }

    PlantUtilities::SafeCopyPlantNode(state, this->InletNodeNum, this->OutletNodeNum, this->plantLoc.loopNum);
}

void LocalPipeData::oneTimeInit_new(EnergyPlusData &state)
{
    int FoundOnLoop = 0;
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state, this->Name, this->Type, this->plantLoc, errFlag, _, _, FoundOnLoop, _, _);
    // Clang can't tell that the FoundOnLoop argument is actually passed by reference since it is an optional, so it thinks FoundOnLoop is always 0.
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
    if (FoundOnLoop == 0) {
        ShowFatalError(state, format("SimPipes: Pipe=\"{}\" not found on a Plant Loop.", this->Name)); // LCOV_EXCL_LINE
    }
#pragma clang diagnostic pop
    if (errFlag) {
        ShowFatalError(state, "SimPipes: Program terminated due to previous condition(s)."); // LCOV_EXCL_LINE
    }
}

void LocalPipeData::initEachEnvironment(EnergyPlusData &state) const
{
    PlantUtilities::InitComponentNodes(
        state, 0.0, state.dataPlnt->PlantLoop(this->plantLoc.loopNum).MaxMassFlowRate, this->InletNodeNum, this->OutletNodeNum);
}
void LocalPipeData::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

void GetPipeInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    April 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // Using/Aliasing
    using BranchNodeConnections::TestCompSet;
    using NodeInputManager::GetOnlySingleNode;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int PipeNum = 0;
    int NumAlphas = 0; // Number of elements in the alpha array
    int NumNums = 0;   // Number of elements in the numeric array
    int IOStat = 0;    // IO Status when calling get input subroutine
    bool ErrorsFound(false);

    // GET NUMBER OF ALL EQUIPMENT TYPES
    int NumWaterPipes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Pipe:Adiabatic");
    int NumSteamPipes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Pipe:Adiabatic:Steam");
    int const NumLocalPipes = NumWaterPipes + NumSteamPipes;
    state.dataPipes->LocalPipe.allocate(NumLocalPipes);
    state.dataPipes->LocalPipeUniqueNames.reserve(static_cast<unsigned>(NumLocalPipes));
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
    cCurrentModuleObject = "Pipe:Adiabatic";
    for (int PipeWaterNum = 1; PipeWaterNum <= NumWaterPipes; ++PipeWaterNum) {
        ++PipeNum;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 PipeWaterNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat);
        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPipes->LocalPipeUniqueNames, state.dataIPShortCut->cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
        state.dataPipes->LocalPipe(PipeNum).Name = state.dataIPShortCut->cAlphaArgs(1);
        state.dataPipes->LocalPipe(PipeNum).Type = DataPlant::PlantEquipmentType::Pipe;

        state.dataPipes->LocalPipe(PipeNum).InletNodeNum = GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(2),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::PipeAdiabatic,
                                                                             state.dataIPShortCut->cAlphaArgs(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
        state.dataPipes->LocalPipe(PipeNum).OutletNodeNum = GetOnlySingleNode(state,
                                                                              state.dataIPShortCut->cAlphaArgs(3),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::PipeAdiabatic,
                                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                                              DataLoopNode::NodeFluidType::Water,
                                                                              DataLoopNode::ConnectionType::Outlet,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
        TestCompSet(state,
                    cCurrentModuleObject,
                    state.dataIPShortCut->cAlphaArgs(1),
                    state.dataIPShortCut->cAlphaArgs(2),
                    state.dataIPShortCut->cAlphaArgs(3),
                    "Pipe Nodes");
    }

    PipeNum = NumWaterPipes;
    cCurrentModuleObject = "Pipe:Adiabatic:Steam";

    for (int PipeSteamNum = 1; PipeSteamNum <= NumSteamPipes; ++PipeSteamNum) {
        ++PipeNum;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 PipeSteamNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat);
        GlobalNames::VerifyUniqueInterObjectName(
            state, state.dataPipes->LocalPipeUniqueNames, state.dataIPShortCut->cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
        state.dataPipes->LocalPipe(PipeNum).Name = state.dataIPShortCut->cAlphaArgs(1);
        state.dataPipes->LocalPipe(PipeNum).Type = DataPlant::PlantEquipmentType::PipeSteam;
        state.dataPipes->LocalPipe(PipeNum).InletNodeNum = GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(2),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::PipeAdiabaticSteam,
                                                                             state.dataIPShortCut->cAlphaArgs(1),
                                                                             DataLoopNode::NodeFluidType::Steam,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
        state.dataPipes->LocalPipe(PipeNum).OutletNodeNum = GetOnlySingleNode(state,
                                                                              state.dataIPShortCut->cAlphaArgs(3),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::PipeAdiabaticSteam,
                                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                                              DataLoopNode::NodeFluidType::Steam,
                                                                              DataLoopNode::ConnectionType::Outlet,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
        TestCompSet(state,
                    cCurrentModuleObject,
                    state.dataIPShortCut->cAlphaArgs(1),
                    state.dataIPShortCut->cAlphaArgs(2),
                    state.dataIPShortCut->cAlphaArgs(3),
                    "Pipe Nodes");
    }

    if (ErrorsFound) {
        ShowFatalError(state, "GetPipeInput: Errors getting input for pipes"); // LCOV_EXCL_LINE
    }
}

} // namespace EnergyPlus::Pipes
