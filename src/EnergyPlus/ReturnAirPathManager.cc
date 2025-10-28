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
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DuctLoss.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/ReturnAirPathManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

namespace ReturnAirPathManager {
    // Module containing the routines dealing with the AirLoopHVAC:ReturnPath (formerly Return Air Path)

    // MODULE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   January 1998
    //       MODIFIED       Lawrie, September 1999 -- consolidate ReturnAirPath data structure
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To manage the return air path.

    void SimReturnAirPath(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Russ Taylor
        //       DATE WRITTEN:    Nov 1997

        // Locals
        int ReturnAirPathNum;

        // Obtains and Allocates Mixer related parameters from input file
        if (state.dataRetAirPathMrg->GetInputFlag) { // First time subroutine has been entered
            GetReturnAirPathInput(state);
            state.dataRetAirPathMrg->GetInputFlag = false;
        }

        for (ReturnAirPathNum = 1; ReturnAirPathNum <= state.dataZoneEquip->NumReturnAirPaths; ++ReturnAirPathNum) {
            CalcReturnAirPath(state, ReturnAirPathNum);
        }
    }

    void GetReturnAirPathInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Russ Taylor
        //       DATE WRITTEN:    Nov 1997

        // PURPOSE OF THIS SUBROUTINE: This subroutine

        // Using/Aliasing
        using NodeInputManager::GetOnlySingleNode;
        using namespace DataLoopNode;

        //////////// hoisted into namespace ////////////////////////////////////////////////
        // static bool ErrorsFound( false );
        ////////////////////////////////////////////////////////////////////////////////////

        bool ErrorsFound = false;

        if (allocated(state.dataZoneEquip->ReturnAirPath)) {
            return;
        }
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "AirLoopHVAC:ReturnPath";
        state.dataZoneEquip->NumReturnAirPaths = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataZoneEquip->NumReturnAirPaths > 0) {
            int NumAlphas;
            int NumNums;
            int IOStat;

            state.dataZoneEquip->ReturnAirPath.allocate(state.dataZoneEquip->NumReturnAirPaths);

            for (int PathNum = 1; PathNum <= state.dataZoneEquip->NumReturnAirPaths; ++PathNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         PathNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNums,
                                                                         IOStat);
                Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

                state.dataZoneEquip->ReturnAirPath(PathNum).Name = state.dataIPShortCut->cAlphaArgs(1);
                state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents = nint((NumAlphas - 2.0) / 2.0);

                state.dataZoneEquip->ReturnAirPath(PathNum).OutletNodeNum =
                    GetOnlySingleNode(state,
                                      state.dataIPShortCut->cAlphaArgs(2),
                                      ErrorsFound,
                                      DataLoopNode::ConnectionObjectType::AirLoopHVACReturnPath,
                                      state.dataIPShortCut->cAlphaArgs(1),
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::Outlet,
                                      NodeInputManager::CompFluidStream::Primary,
                                      ObjectIsParent);

                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType = "";
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum = DataZoneEquipment::AirLoopHVACZone::Invalid;
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName = "";
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentIndex.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentIndex = 0;
                int Counter = 3;

                for (int CompNum = 1; CompNum <= state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents; ++CompNum) {

                    if ((Util::SameString(state.dataIPShortCut->cAlphaArgs(Counter), "AirLoopHVAC:ZoneMixer")) ||
                        (Util::SameString(state.dataIPShortCut->cAlphaArgs(Counter), "AirLoopHVAC:ReturnPlenum"))) {

                        bool IsNotOK; // Flag to verify name
                        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType(CompNum) = state.dataIPShortCut->cAlphaArgs(Counter);
                        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName(CompNum) = state.dataIPShortCut->cAlphaArgs(Counter + 1);
                        ValidateComponent(state,
                                          state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType(CompNum),
                                          state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName(CompNum),
                                          IsNotOK,
                                          "AirLoopHVAC:ReturnPath");
                        if (IsNotOK) {
                            ShowContinueError(state, format("In AirLoopHVAC:ReturnPath ={}", state.dataZoneEquip->ReturnAirPath(PathNum).Name));
                            ErrorsFound = true;
                        }
                        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum(CompNum) = static_cast<DataZoneEquipment::AirLoopHVACZone>(
                            getEnumValue(DataZoneEquipment::AirLoopHVACTypeNamesCC, state.dataIPShortCut->cAlphaArgs(Counter)));

                    } else {
                        ShowSevereError(
                            state, format("Unhandled component type in AirLoopHVAC:ReturnPath of {}", state.dataIPShortCut->cAlphaArgs(Counter)));
                        ShowContinueError(state, format("Occurs in AirLoopHVAC:ReturnPath = {}", state.dataZoneEquip->ReturnAirPath(PathNum).Name));
                        ShowContinueError(state, "Must be \"AirLoopHVAC:ZoneMixer\" or \"AirLoopHVAC:ReturnPlenum\"");
                        ErrorsFound = true;
                    }

                    Counter += 2;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found getting AirLoopHVAC:ReturnPath.  Preceding condition(s) causes termination.");
        }
    }

    void CalcReturnAirPath(EnergyPlusData &state, int &ReturnAirPathNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Russ Taylor
        //       DATE WRITTEN:    Nov 1997

        // PURPOSE OF THIS SUBROUTINE: This subroutine

        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // USE STATEMENTS:

        // Using/Aliasing
        using MixerComponent::SimAirMixer;
        using ZonePlenum::SimAirZonePlenum;

        // Locals
        int ComponentNum;

        for (ComponentNum = 1; ComponentNum <= state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).NumOfComponents; ++ComponentNum) {
            switch (state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentTypeEnum(ComponentNum)) {
            case DataZoneEquipment::AirLoopHVACZone::Mixer: // 'AirLoopHVAC:ZoneMixer'

                if (!(state.afn->AirflowNetworkFanActivated && state.afn->distribution_simulated)) {
                    SimAirMixer(state,
                                state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentName(ComponentNum),
                                state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentIndex(ComponentNum));
                    if (state.dataDuctLoss->DuctLossSimu) {
                        DuctLoss::SimulateDuctLoss(
                            state, DuctLoss::AirPath::Return, state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentIndex(ComponentNum));
                    }
                }

                break;

            case DataZoneEquipment::AirLoopHVACZone::ReturnPlenum: // 'AirLoopHVAC:ReturnPlenum'

                SimAirZonePlenum(state,
                                 state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentName(ComponentNum),
                                 DataZoneEquipment::AirLoopHVACZone::ReturnPlenum,
                                 state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentIndex(ComponentNum));
                break;

            default:
                ShowSevereError(state,
                                format("Invalid AirLoopHVAC:ReturnPath Component={}",
                                       state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).ComponentType(ComponentNum)));
                ShowContinueError(state, format("Occurs in AirLoopHVAC:ReturnPath ={}", state.dataZoneEquip->ReturnAirPath(ReturnAirPathNum).Name));
                ShowFatalError(state, "Preceding condition causes termination.");
                break;
            }
        }
    }

    void ReportReturnAirPath([[maybe_unused]] int &ReturnAirPathNum) // unused1208
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Russ Taylor
        //       DATE WRITTEN:    Nov 1997

        // PURPOSE OF THIS SUBROUTINE: This subroutine

        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // USE STATEMENTS:
    }

} // namespace ReturnAirPathManager

} // namespace EnergyPlus
