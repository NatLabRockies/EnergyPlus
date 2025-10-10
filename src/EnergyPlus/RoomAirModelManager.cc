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
#include <algorithm>
#include <cmath>
#include <limits>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
// #include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/BaseboardElectric.hh>
#include <EnergyPlus/CrossVentMgr.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DisplacementVentMgr.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HybridUnitaryAirConditioners.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MundtSimMgr.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/RoomAirModelAirflowNetwork.hh>
#include <EnergyPlus/RoomAirModelManager.hh>
#include <EnergyPlus/RoomAirModelUserTempPattern.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UFADManager.hh>
#include <EnergyPlus/UnitHeater.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VentilatedSlab.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WindowAC.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace RoomAir {

    // MODULE INFORMATION
    //       AUTHOR         Weixiu Kong
    //       DATE WRITTEN   March 2003
    //       MODIFIED       July 2003, CC
    //                      Aug, 2005, BG

    // PURPOSE OF THIS MODULE:
    // Contains subroutines for managing the room air models

    constexpr std::array<std::string_view, (int)RoomAirModel::Num> roomAirModelNamesUC = {
        "USERDEFINED", "MIXING", "MUNDT", "UCSD_DV", "UCSD_CV", "UCSD_UFI", "UCSD_UFE", "AIRFLOWNETWORK"};

    constexpr std::array<std::string_view, (int)AirNodeType::Num> airNodeTypeNamesUC = {
        "INLET", "FLOOR", "CONTROL", "CEILING", "MUNDTROOM", "RETURN", "AIRFLOWNETWORK", "PLUME", "REESROOM"};

    constexpr std::array<std::string_view, (int)Comfort::Num> comfortNamesUC = {"JET", "RECIRCULATION"};

    constexpr std::array<std::string_view, (int)Diffuser::Num> diffuserNamesUC = {
        "SWIRL", "VARIABLEAREA", "HORIZONTALSWIRL", "LINEARBARGRILLE", "CUSTOM"};

    constexpr std::array<std::string_view, (int)UserDefinedPatternMode::Num> userDefinedPatternModeNamesUC = {
        "OUTDOORDRYBULBTEMPERATURE", "SENSIBLECOOLINGLOAD", "SENSIBLEHEATINGLOAD", "ZONEDRYBULBTEMPERATURE", "ZONEANDOUTDOORTEMPERATUREDIFFERENCE"};

    void ManageAirModel(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Weixiu Kong
        //       DATE WRITTEN   April 2003
        //       MODIFIED       July 2003, CC
        //                      Jan 2004, CC

        // PURPOSE OF THIS SUBROUTINE:
        //     manage room air models.

        if (state.dataRoomAir->GetAirModelData) {
            GetAirModelDatas(state);
            state.dataRoomAir->GetAirModelData = false;
        }

        if (!state.dataRoomAir->anyNonMixingRoomAirModel) {
            return;
        }

        if (state.dataRoomAir->UCSDModelUsed) {
            SharedDVCVUFDataInit(state, ZoneNum);
        }

        switch (state.dataRoomAir->AirModel(ZoneNum).AirModel) {
        case RoomAirModel::UserDefined:
            ManageUserDefinedPatterns(state, ZoneNum);
            break;

        case RoomAirModel::Mixing: // Mixing air model
            break;                 // do nothing

        case RoomAirModel::DispVent1Node: // Mundt air model
            // simulate room airflow using Mundt model
            ManageDispVent1Node(state, ZoneNum);
            break;

        case RoomAirModel::DispVent3Node: // UCDV Displacement Ventilation model
            // simulate room airflow using UCSDDV model
            ManageDispVent3Node(state, ZoneNum);
            break;

        case RoomAirModel::CrossVent: // UCSD Cross Ventilation model
            // simulate room airflow using UCSDDV model
            ManageCrossVent(state, ZoneNum);
            break;

        case RoomAirModel::UFADInt: // UCSD UFAD interior zone model
            // simulate room airflow using the UCSDUFI model
            ManageUFAD(state, ZoneNum, RoomAirModel::UFADInt);
            break;

        case RoomAirModel::UFADExt: // UCSD UFAD exterior zone model
            // simulate room airflow using the UCSDUFE model
            ManageUFAD(state, ZoneNum, RoomAirModel::UFADExt);
            break;

        case RoomAirModel::AirflowNetwork: // RoomAirflowNetwork zone model
            // simulate room airflow using the AirflowNetwork - based model
            SimRoomAirModelAFN(state, ZoneNum);
            break;

        default:   // mixing air model
            break; // do nothing
        }
    }

    //*****************************************************************************************

    void GetAirModelDatas(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine "gets" all the data for the "RoomAir" models by calling individual
        // routines.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound;

        ErrorsFound = false;
        // get air node input data for all zones
        GetAirNodeData(state, ErrorsFound);

        // get mundt model controls for all zones
        GetMundtData(state, ErrorsFound);

        // get airflow network model info for all zones
        GetRoomAirflowNetworkData(state, ErrorsFound);

        // get UCSDDV model controls for all zones
        GetDisplacementVentData(state, ErrorsFound);

        // get UCSDCV model controls for all zones
        GetCrossVentData(state, ErrorsFound);

        // get BTG's user-defined patterns for all zones
        GetUserDefinedPatternData(state, ErrorsFound);

        // get UCSD UFAD interior zone model controls for all zones
        // get UCSD UFAD exterior zone model controls for all zones
        GetUFADZoneData(state, ErrorsFound);

        if (ErrorsFound) {
            ShowFatalError(state, "GetAirModelData: Errors found getting air model input.  Program terminates.");
        }
    }

    void GetUserDefinedPatternData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Aug 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine "gets" all the data for the "User-Defined RoomAir"

        // METHODOLOGY EMPLOYED:
        // usual energyplus input routines
        // for the actual patterns, a single structure array holds
        // different patterns in nested derived types.

        // Using/Aliasing
        using DataZoneEquipment::EquipConfiguration;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName = "GetUserDefinedPatternData: ";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;  // number of alphas
        int NumNumbers; // Number of numbers encountered
        int Status;     // Notes if there was an error in processing the input

        auto &ipsc = state.dataIPShortCut;

        // access input file and setup
        state.dataRoomAir->numTempDistContrldZones = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cUserDefinedControlObject);

        state.dataRoomAir->NumConstantGradient =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cTempPatternConstGradientObject);
        state.dataRoomAir->NumTwoGradientInterp = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cTempPatternTwoGradientObject);
        state.dataRoomAir->NumNonDimensionalHeight = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cTempPatternNDHeightObject);
        state.dataRoomAir->NumSurfaceMapping = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cTempPatternSurfMapObject);

        state.dataRoomAir->NumAirTempPatterns = state.dataRoomAir->NumConstantGradient + state.dataRoomAir->NumTwoGradientInterp +
                                                state.dataRoomAir->NumNonDimensionalHeight + state.dataRoomAir->NumSurfaceMapping;
        ipsc->cCurrentModuleObject = cUserDefinedControlObject;
        if (state.dataRoomAir->numTempDistContrldZones == 0) {
            if (state.dataRoomAir->NumAirTempPatterns != 0) { // user may have missed control object
                ShowWarningError(state, format("Missing {} object needed to use roomair temperature patterns", ipsc->cCurrentModuleObject));
                // ErrorsFound = .TRUE.
            }
            return;
        }

        // now allocate AirPatternZoneInfo to length of all zones for easy indexing
        if (!allocated(state.dataRoomAir->AirPatternZoneInfo)) {
            state.dataRoomAir->AirPatternZoneInfo.allocate(state.dataGlobal->NumOfZones);
        }

        for (int ObjNum = 1; ObjNum <= state.dataRoomAir->numTempDistContrldZones; ++ObjNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     ObjNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            // first get zone ID
            int ZoneNum = Util::FindItemInList(ipsc->cAlphaArgs(2), state.dataHeatBal->Zone);
            if (ZoneNum == 0) { // throw error
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
                return; // halt to avoid hard crash
            }

            auto &airPatternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
            airPatternZoneInfo.IsUsed = true;
            airPatternZoneInfo.Name = ipsc->cAlphaArgs(1);     // Name of this Control Object
            airPatternZoneInfo.ZoneName = ipsc->cAlphaArgs(2); // Zone Name

            if (ipsc->lAlphaFieldBlanks(3)) {
                airPatternZoneInfo.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((airPatternZoneInfo.availSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            }

            if (ipsc->lAlphaFieldBlanks(4)) {
            } else if ((airPatternZoneInfo.patternSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(4), ipsc->cAlphaArgs(4));
                ErrorsFound = true;
            }

            airPatternZoneInfo.ZoneID = ZoneNum;

            //   figure number of surfaces for this zone
            airPatternZoneInfo.totNumSurfs = 0;
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                airPatternZoneInfo.totNumSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
            }
            //   allocate nested derived type for surface info
            airPatternZoneInfo.Surf.allocate(airPatternZoneInfo.totNumSurfs);

            //   Fill in what we know for nested structure for surfaces
            int thisSurfinZone = 0;
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int thisHBsurfID = thisSpace.HTSurfaceFirst; thisHBsurfID <= thisSpace.HTSurfaceLast; ++thisHBsurfID) {
                    ++thisSurfinZone;
                    if (state.dataSurface->Surface(thisHBsurfID).Class == DataSurfaces::SurfaceClass::IntMass) {
                        airPatternZoneInfo.Surf(thisSurfinZone).SurfID = thisHBsurfID;
                        airPatternZoneInfo.Surf(thisSurfinZone).Zeta = 0.5;
                        continue;
                    }

                    airPatternZoneInfo.Surf(thisSurfinZone).SurfID = thisHBsurfID;

                    airPatternZoneInfo.Surf(thisSurfinZone).Zeta = FigureNDheightInZone(state, thisHBsurfID);
                }
            } // loop through surfaces in this zone

        } // loop through number of 'RoomAir:TemperaturePattern:UserDefined' objects

        // Check against AirModel.  Make sure there is a match here.
        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAirModel::UserDefined) {
                continue;
            }
            if (state.dataRoomAir->AirPatternZoneInfo(iZone).IsUsed) {
                continue; // There is a Room Air Temperatures object for this zone
            }
            ShowSevereError(state,
                            format("{}AirModel for Zone=[{}] is indicated as \"User Defined\".", routineName, state.dataHeatBal->Zone(iZone).Name));
            ShowContinueError(state, format("...but missing a {} object for control.", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }

        // now get user defined temperature patterns
        if (!allocated(state.dataRoomAir->AirPattern)) {
            state.dataRoomAir->AirPattern.allocate(state.dataRoomAir->NumAirTempPatterns);
        }

        // Four different objects to get
        ipsc->cCurrentModuleObject = cTempPatternConstGradientObject;
        for (int ObjNum = 1; ObjNum <= state.dataRoomAir->NumConstantGradient; ++ObjNum) {
            int thisPattern = ObjNum;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     ObjNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            auto &roomAirPattern = state.dataRoomAir->AirPattern(thisPattern);
            roomAirPattern.Name = ipsc->cAlphaArgs(1);
            roomAirPattern.PatrnID = ipsc->rNumericArgs(1);
            roomAirPattern.PatternMode = UserDefinedPatternType::ConstGradTemp;
            roomAirPattern.DeltaTstat = ipsc->rNumericArgs(2);
            roomAirPattern.DeltaTleaving = ipsc->rNumericArgs(3);
            roomAirPattern.DeltaTexhaust = ipsc->rNumericArgs(4);
            roomAirPattern.GradPatrn.Gradient = ipsc->rNumericArgs(5);
        }

        ipsc->cCurrentModuleObject = cTempPatternTwoGradientObject;
        for (int ObjNum = 1; ObjNum <= state.dataRoomAir->NumTwoGradientInterp; ++ObjNum) {
            int thisPattern = state.dataRoomAir->NumConstantGradient + ObjNum;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     ObjNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            auto &roomAirPattern = state.dataRoomAir->AirPattern(thisPattern);
            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            roomAirPattern.PatternMode = UserDefinedPatternType::TwoGradInterp;
            roomAirPattern.Name = ipsc->cAlphaArgs(1);
            roomAirPattern.PatrnID = ipsc->rNumericArgs(1);
            roomAirPattern.TwoGradPatrn.TstatHeight = ipsc->rNumericArgs(2);
            roomAirPattern.TwoGradPatrn.TleavingHeight = ipsc->rNumericArgs(3);
            roomAirPattern.TwoGradPatrn.TexhaustHeight = ipsc->rNumericArgs(4);
            roomAirPattern.TwoGradPatrn.LowGradient = ipsc->rNumericArgs(5);
            roomAirPattern.TwoGradPatrn.HiGradient = ipsc->rNumericArgs(6);

            roomAirPattern.TwoGradPatrn.InterpolationMode =
                static_cast<UserDefinedPatternMode>(getEnumValue(userDefinedPatternModeNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
            if (roomAirPattern.TwoGradPatrn.InterpolationMode == UserDefinedPatternMode::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            roomAirPattern.TwoGradPatrn.UpperBoundTempScale = ipsc->rNumericArgs(7);
            roomAirPattern.TwoGradPatrn.LowerBoundTempScale = ipsc->rNumericArgs(8);

            roomAirPattern.TwoGradPatrn.UpperBoundHeatRateScale = ipsc->rNumericArgs(9);
            roomAirPattern.TwoGradPatrn.LowerBoundHeatRateScale = ipsc->rNumericArgs(10);

            // now test the input some
            if (roomAirPattern.TwoGradPatrn.HiGradient == roomAirPattern.TwoGradPatrn.LowGradient) {
                ShowWarningError(state, format("Upper and lower gradients equal, use {} instead ", cTempPatternConstGradientObject));
                ShowContinueError(state, format("Entered in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
            }
            if ((roomAirPattern.TwoGradPatrn.UpperBoundTempScale == roomAirPattern.TwoGradPatrn.LowerBoundTempScale) &&
                ((roomAirPattern.TwoGradPatrn.InterpolationMode == UserDefinedPatternMode::OutdoorDryBulb) ||
                 (roomAirPattern.TwoGradPatrn.InterpolationMode == UserDefinedPatternMode::ZoneAirTemp) ||
                 (roomAirPattern.TwoGradPatrn.InterpolationMode == UserDefinedPatternMode::DeltaOutdoorZone))) {
                // throw error, will cause divide by zero when used for scaling
                ShowSevereError(state, format("Error in temperature scale in {}: {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            if ((roomAirPattern.TwoGradPatrn.HiGradient == roomAirPattern.TwoGradPatrn.LowGradient) &&
                ((roomAirPattern.TwoGradPatrn.InterpolationMode == UserDefinedPatternMode::SensibleCooling) ||
                 (roomAirPattern.TwoGradPatrn.InterpolationMode == UserDefinedPatternMode::SensibleHeating))) {
                // throw error, will cause divide by zero when used for scaling
                ShowSevereError(state, format("Error in load scale in {}: {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
        }

        ipsc->cCurrentModuleObject = cTempPatternNDHeightObject;
        for (int ObjNum = 1; ObjNum <= state.dataRoomAir->NumNonDimensionalHeight; ++ObjNum) {
            int thisPattern = state.dataRoomAir->NumConstantGradient + state.dataRoomAir->NumTwoGradientInterp + ObjNum;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     ObjNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);
            auto &roomAirPattern = state.dataRoomAir->AirPattern(thisPattern);
            roomAirPattern.PatternMode = UserDefinedPatternType::NonDimenHeight;

            roomAirPattern.Name = ipsc->cAlphaArgs(1);
            roomAirPattern.PatrnID = ipsc->rNumericArgs(1);
            roomAirPattern.DeltaTstat = ipsc->rNumericArgs(2);
            roomAirPattern.DeltaTleaving = ipsc->rNumericArgs(3);
            roomAirPattern.DeltaTexhaust = ipsc->rNumericArgs(4);

            int NumPairs = std::floor((double(NumNumbers) - 4.0) / 2.0);

            // TODO error checking

            roomAirPattern.VertPatrn.ZetaPatrn.allocate(NumPairs);
            roomAirPattern.VertPatrn.DeltaTaiPatrn.allocate(NumPairs);

            // init these since they can't be in derived type
            roomAirPattern.VertPatrn.ZetaPatrn = 0.0;
            roomAirPattern.VertPatrn.DeltaTaiPatrn = 0.0;

            for (int i = 0; i <= NumPairs - 1; ++i) {

                roomAirPattern.VertPatrn.ZetaPatrn(i + 1) = ipsc->rNumericArgs(2 * i + 5);
                roomAirPattern.VertPatrn.DeltaTaiPatrn(i + 1) = ipsc->rNumericArgs(2 * i + 6);
            }

            // TODO  check order (TODO sort ? )
            for (int i = 2; i <= NumPairs; ++i) {
                if (roomAirPattern.VertPatrn.ZetaPatrn(i) < roomAirPattern.VertPatrn.ZetaPatrn(i - 1)) {
                    ShowSevereError(state, format("Zeta values not in increasing order in {}: {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
            }
        }

        ipsc->cCurrentModuleObject = cTempPatternSurfMapObject;
        for (int ObjNum = 1; ObjNum <= state.dataRoomAir->NumSurfaceMapping; ++ObjNum) {
            int thisPattern = state.dataRoomAir->NumConstantGradient + state.dataRoomAir->NumTwoGradientInterp +
                              state.dataRoomAir->NumNonDimensionalHeight + ObjNum;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     ObjNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            auto &roomAirPattern = state.dataRoomAir->AirPattern(thisPattern);
            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};

            roomAirPattern.PatternMode = UserDefinedPatternType::SurfMapTemp;
            roomAirPattern.Name = ipsc->cAlphaArgs(1);
            roomAirPattern.PatrnID = ipsc->rNumericArgs(1);
            roomAirPattern.DeltaTstat = ipsc->rNumericArgs(2);
            roomAirPattern.DeltaTleaving = ipsc->rNumericArgs(3);
            roomAirPattern.DeltaTexhaust = ipsc->rNumericArgs(4);

            int NumPairs = NumNumbers - 4;

            if (NumPairs != (NumAlphas - 1)) {
                ShowSevereError(state, format("Error in number of entries in {} object: {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }
            roomAirPattern.MapPatrn.SurfName.allocate(NumPairs);
            roomAirPattern.MapPatrn.DeltaTai.allocate(NumPairs);
            roomAirPattern.MapPatrn.SurfID.allocate(NumPairs);

            // init just allocated
            roomAirPattern.MapPatrn.SurfName = "";
            roomAirPattern.MapPatrn.DeltaTai = 0.0;
            roomAirPattern.MapPatrn.SurfID = 0;

            for (int i = 1; i <= NumPairs; ++i) {
                roomAirPattern.MapPatrn.SurfName(i) = ipsc->cAlphaArgs(i + 1);
                roomAirPattern.MapPatrn.DeltaTai(i) = ipsc->rNumericArgs(i + 4);
                roomAirPattern.MapPatrn.SurfID(i) = Util::FindItemInList(ipsc->cAlphaArgs(i + 1), state.dataSurface->Surface);
                if (roomAirPattern.MapPatrn.SurfID(i) == 0) {
                    ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(i + 1), ipsc->cAlphaArgs(i + 1));
                    ErrorsFound = true;
                }
            }
            roomAirPattern.MapPatrn.NumSurfs = NumPairs;
        }

        if (state.dataErrTracking->TotalRoomAirPatternTooLow > 0) {
            ShowWarningError(state,
                             format("GetUserDefinedPatternData: RoomAirModelUserTempPattern: {} problem(s) in non-dimensional height calculations, "
                                    "too low surface height(s) in relation to floor height of zone(s).",
                                    state.dataErrTracking->TotalRoomAirPatternTooLow));
            ShowContinueError(state, "...Use OutputDiagnostics,DisplayExtraWarnings; to see details.");
            state.dataErrTracking->TotalWarningErrors += state.dataErrTracking->TotalRoomAirPatternTooLow;
        }
        if (state.dataErrTracking->TotalRoomAirPatternTooHigh > 0) {
            ShowWarningError(state,
                             format("GetUserDefinedPatternData: RoomAirModelUserTempPattern: {} problem(s) in non-dimensional height calculations, "
                                    "too high surface height(s) in relation to ceiling height of zone(s).",
                                    state.dataErrTracking->TotalRoomAirPatternTooHigh));
            ShowContinueError(state, "...Use OutputDiagnostics,DisplayExtraWarnings; to see details.");
            state.dataErrTracking->TotalWarningErrors += state.dataErrTracking->TotalRoomAirPatternTooHigh;
        }

        // now do one time setups from and checks on user data

        // Find and set return and exhaust node ids

        for (int i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
            if (state.dataRoomAir->AirPatternZoneInfo(i).IsUsed) {
                // first get return and exhaust air node index
                int found = Util::FindItemInList(
                    state.dataRoomAir->AirPatternZoneInfo(i).ZoneName, state.dataZoneEquip->ZoneEquipConfig, &EquipConfiguration::ZoneName);

                if (found != 0) {

                    state.dataRoomAir->AirPatternZoneInfo(i).ZoneNodeID = state.dataZoneEquip->ZoneEquipConfig(found).ZoneNode;
                    if (allocated(state.dataZoneEquip->ZoneEquipConfig(found).ExhaustNode)) {
                        state.dataRoomAir->AirPatternZoneInfo(i).ExhaustAirNodeID.allocate(
                            state.dataZoneEquip->ZoneEquipConfig(found).NumExhaustNodes);
                        state.dataRoomAir->AirPatternZoneInfo(i).ExhaustAirNodeID = state.dataZoneEquip->ZoneEquipConfig(found).ExhaustNode;
                    } // exhaust nodes present
                } // found ZoneEquipConf

                // second get zone height values
                state.dataRoomAir->AirPatternZoneInfo(i).ZoneHeight = state.dataHeatBal->Zone(i).CeilingHeight;

            } // air pattern is used
        }
    }

    void GetAirNodeData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   August 2001
        //       RE-ENGINEERED  April 2003, Weixiu Kong
        //       MODIFIED       July 2003, CC
        //                      Jan 2004, CC

        // PURPOSE OF THIS SUBROUTINE:
        //     Get AirNode data for all zones at once

        constexpr std::string_view routineName = "GetAirNodeData";
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas; // States which alpha value to read from a
        // "Number" line
        int NumNumbers; // Number of numbers encountered
        int Status;     // Notes if there was an error in processing the input

        if (!state.dataRoomAir->DispVent1NodeModelUsed) {
            return;
        }

        auto &ipsc = state.dataIPShortCut;

        // Initialize default values for air nodes
        state.dataRoomAir->TotNumOfZoneAirNodes.allocate(state.dataGlobal->NumOfZones);
        state.dataRoomAir->TotNumOfAirNodes = 0;
        state.dataRoomAir->TotNumOfZoneAirNodes = 0;
        ipsc->cCurrentModuleObject = "RoomAir:Node";
        state.dataRoomAir->TotNumOfAirNodes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

        if (state.dataRoomAir->TotNumOfAirNodes <= 0) {
            // no air node object is found, terminate the program
            ShowSevereError(state, format("No {} objects found in input.", ipsc->cCurrentModuleObject));
            ShowContinueError(state, format("The OneNodeDisplacementVentilation model requires {} objects", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
            return;
        } else {
            // air node objects are found so allocate airnode variable
            state.dataRoomAir->AirNode.allocate(state.dataRoomAir->TotNumOfAirNodes);
        }

        for (int AirNodeNum = 1; AirNodeNum <= state.dataRoomAir->TotNumOfAirNodes; ++AirNodeNum) {

            // get air node objects
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     AirNodeNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            auto &airNode = state.dataRoomAir->AirNode(AirNodeNum);
            airNode.Name = ipsc->cAlphaArgs(1);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, airNode.Name};

            airNode.ZoneName = ipsc->cAlphaArgs(3); // Zone name
            airNode.ZonePtr = Util::FindItemInList(airNode.ZoneName, state.dataHeatBal->Zone);
            if (airNode.ZonePtr == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else {
                int NumOfSurfs = 0;
                for (int spaceNum : state.dataHeatBal->Zone(airNode.ZonePtr).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    NumOfSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
                }
                airNode.SurfMask.allocate(NumOfSurfs);
            }

            airNode.ClassType = static_cast<AirNodeType>(getEnumValue(airNodeTypeNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
            if (airNode.ClassType == AirNodeType::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            airNode.Height = ipsc->rNumericArgs(1); // Air node height
            int NumSurfsInvolved = NumAlphas - 3;   // Number of surfaces involved with air nodes

            // Initialize
            airNode.SurfMask = false;

            if (NumSurfsInvolved <= 0) {

                // report severe error since the following air nodes require surfaces associated with them
                if (airNode.ClassType == AirNodeType::Floor || airNode.ClassType == AirNodeType::Ceiling || airNode.ClassType == AirNodeType::Mundt ||
                    airNode.ClassType == AirNodeType::Plume || airNode.ClassType == AirNodeType::Rees) { // Are there really Rees 1-4?
                    // terminate the program due to a severe error in the specified input
                    ShowSevereError(state,
                                    format("GetAirNodeData: {}=\"{}\" invalid air node specification.", ipsc->cCurrentModuleObject, airNode.Name));
                    ShowContinueError(state,
                                      format("Mundt Room Air Model: No surface names specified.  Air node=\"{} requires surfaces associated with it.",
                                             airNode.Name));
                    ErrorsFound = true;
                }
                continue;
            }

            // report warning error since the following air nodes do not require surfaces associated with them
            // and assign .FALSE. to 'SurfNeeded'
            if (airNode.ClassType == AirNodeType::Inlet || airNode.ClassType == AirNodeType::Control || airNode.ClassType == AirNodeType::Return ||
                airNode.ClassType == AirNodeType::Plume) {
                ShowWarningError(state, format("GetAirNodeData: {}=\"{}\" invalid linkage", ipsc->cCurrentModuleObject, airNode.Name));
                ShowContinueError(
                    state, format("Mundt Room Air Model: No surface names needed.  Air node=\"{} does not relate to any surfaces.", airNode.Name));
                continue;
            }

            // this air node is in this zone; hence, first get name of all surfaces in this zone
            auto const &zone = state.dataHeatBal->Zone(airNode.ZonePtr);
            int NumOfSurfs = 0;
            for (int spaceNum : zone.spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                NumOfSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
            }

            // terminate the program due to a severe error in the specified input
            if (NumSurfsInvolved > NumOfSurfs) {
                ShowFatalError(
                    state,
                    format("GetAirNodeData: Mundt Room Air Model: Number of surfaces connected to {} is greater than number of surfaces in {}",
                           airNode.Name,
                           zone.Name));
                return;
            }

            // relate surfaces to this air node and check to see whether surface names are specified correctly or not
            int SurfCount = 0;
            for (int ListSurfNum = 4; ListSurfNum <= NumAlphas; ++ListSurfNum) {
                int thisSurfinZone = 0;
                for (int spaceNum : zone.spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                        ++thisSurfinZone;
                        if (ipsc->cAlphaArgs(ListSurfNum) == state.dataSurface->Surface(SurfNum).Name) {
                            airNode.SurfMask(thisSurfinZone) = true;
                            ++SurfCount;
                            break;
                        }
                    }
                    if (SurfCount > 0) {
                        break;
                    }
                }
            }

            // report warning error since surface names are specified correctly
            if ((NumSurfsInvolved) != SurfCount) {
                ShowWarningError(
                    state,
                    format("GetAirNodeData: Mundt Room Air Model: Some surface names specified for {} are not in {}", airNode.Name, zone.Name));
            }
        } // for (AirNodeNum)

        // get number of air nodes in each zone
        for (int AirNodeNum = 1; AirNodeNum <= state.dataRoomAir->TotNumOfAirNodes; ++AirNodeNum) {
            auto const &airNode = state.dataRoomAir->AirNode(AirNodeNum);
            // this zone uses other air model so skip the rest
            if (state.dataRoomAir->AirModel(airNode.ZonePtr).AirModel == RoomAirModel::DispVent1Node) {
                ++state.dataRoomAir->TotNumOfZoneAirNodes(airNode.ZonePtr);
            }
        }
    }

    //*****************************************************************************************

    void GetMundtData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  April 2003, Weixiu Kong
        //                      July 2003, CC

        // PURPOSE OF THIS SUBROUTINE:
        //     Get Mundt model controls for all zones at once

        // METHODOLOGY EMPLOYED:
        //     Use input processer to get input from idf file

        // Using/Aliasing
        constexpr std::string_view routineName = "GetMundtData";
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumNumbers;       // Number of numbers encountered
        int Status;           // Notes if there was an error in processing the input
        int NumOfMundtContrl; // Number of Mundt Model Controls

        auto &ipsc = state.dataIPShortCut;

        if (!state.dataRoomAir->DispVent1NodeModelUsed) {
            return;
        }

        // Initialize default values for Mundt model controls
        state.dataRoomAir->ConvectiveFloorSplit.allocate(state.dataGlobal->NumOfZones);
        state.dataRoomAir->InfiltratFloorSplit.allocate(state.dataGlobal->NumOfZones);
        state.dataRoomAir->ConvectiveFloorSplit = 0.0;
        state.dataRoomAir->InfiltratFloorSplit = 0.0;
        auto &cCurrentModuleObject = ipsc->cCurrentModuleObject;
        cCurrentModuleObject = "RoomAirSettings:OneNodeDisplacementVentilation";
        NumOfMundtContrl = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (NumOfMundtContrl > state.dataGlobal->NumOfZones) {
            ShowSevereError(state, format("Too many {} objects in input file", cCurrentModuleObject));
            ShowContinueError(state, format("There cannot be more {} objects than number of zones.", cCurrentModuleObject));
            ErrorsFound = true;
        }

        if (NumOfMundtContrl == 0) {
            ShowWarningError(state,
                             format("No {} objects found, program assumes no convection or infiltration gains near floors", cCurrentModuleObject));
            return;
        }

        // this zone uses Mundt model so get Mundt Model Control
        // loop through all 'RoomAirSettings:OneNodeDisplacementVentilation' objects
        for (int ControlNum = 1; ControlNum <= NumOfMundtContrl; ++ControlNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     ControlNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     Status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ""};
            int ZoneNum = Util::FindItemInList(ipsc->cAlphaArgs(1), state.dataHeatBal->Zone);
            if (ZoneNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
                ErrorsFound = true;
                continue;
            }
            if (state.dataRoomAir->AirModel(ZoneNum).AirModel != RoomAirModel::DispVent1Node) {
                ShowSevereError(state, format("Zone specified=\"{}\", Air Model type is not OneNodeDisplacementVentilation.", ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Air Model Type for zone={}", roomAirModelNamesUC[(int)state.dataRoomAir->AirModel(ZoneNum).AirModel]));
                ErrorsFound = true;
                continue;
            }
            state.dataRoomAir->ConvectiveFloorSplit(ZoneNum) = ipsc->rNumericArgs(1);
            state.dataRoomAir->InfiltratFloorSplit(ZoneNum) = ipsc->rNumericArgs(2);
        }
    }

    void GetDisplacementVentData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   January 2004

        // PURPOSE OF THIS SUBROUTINE:
        //  Get UCSD Displacement ventilation model controls for all zones at once

        // METHODOLOGY EMPLOYED:
        // Use input processor to get input from idf file

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        constexpr std::string_view routineName = "GetDisplacementVentData";
        int IOStat;
        int NumAlpha;
        int NumNumber;

        auto &ipsc = state.dataIPShortCut;

        if (!state.dataRoomAir->UCSDModelUsed) {
            return;
        }
        ipsc->cCurrentModuleObject = "RoomAirSettings:ThreeNodeDisplacementVentilation";
        state.dataRoomAir->TotDispVent3Node = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

        if (state.dataRoomAir->TotDispVent3Node <= 0) {
            return;
        }

        state.dataRoomAir->ZoneDispVent3Node.allocate(state.dataRoomAir->TotDispVent3Node);

        for (int Loop = 1; Loop <= state.dataRoomAir->TotDispVent3Node; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlpha,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            auto &zoneDV3N = state.dataRoomAir->ZoneDispVent3Node(Loop);
            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            // First is Zone Name
            // zoneUCSDDV.ZoneName = ipsc->cAlphaArgs(1);
            zoneDV3N.ZonePtr = Util::FindItemInList(ipsc->cAlphaArgs(1), state.dataHeatBal->Zone);
            if (zoneDV3N.ZonePtr == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
                ErrorsFound = true;
            } else {
                state.dataRoomAir->IsZoneDispVent3Node(zoneDV3N.ZonePtr) = true;
            }
            // Second Alpha is Schedule Name
            // zoneUCSDDV.SchedGainsName = ipsc->cAlphaArgs(2);
            if (ipsc->lAlphaFieldBlanks(2)) {
                ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(2));
                ErrorsFound = true;
            } else if ((zoneDV3N.gainsSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            zoneDV3N.NumPlumesPerOcc = ipsc->rNumericArgs(1);
            zoneDV3N.ThermostatHeight = ipsc->rNumericArgs(2);
            zoneDV3N.ComfortHeight = ipsc->rNumericArgs(3);
            zoneDV3N.TempTrigger = ipsc->rNumericArgs(4);
        }
    }

    void GetCrossVentData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         G. Carrilho da Graca
        //       DATE WRITTEN   October 2004

        // PURPOSE OF THIS SUBROUTINE:
        //  Get UCSD Cross ventilation model controls for all zones at once

        // METHODOLOGY EMPLOYED:
        // Use input processor to get input from idf file

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        constexpr std::string_view routineName = "GetCrossVentData";

        int IOStat;
        int NumAlpha;
        int NumNumber;

        auto &ipsc = state.dataIPShortCut;
        if (!state.dataRoomAir->UCSDModelUsed) {
            return;
        }
        ipsc->cCurrentModuleObject = "RoomAirSettings:CrossVentilation";
        state.dataRoomAir->TotCrossVent = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

        if (state.dataRoomAir->TotCrossVent <= 0) {
            return;
        }

        state.dataRoomAir->ZoneCrossVent.allocate(state.dataRoomAir->TotCrossVent);

        for (int Loop = 1; Loop <= state.dataRoomAir->TotCrossVent; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlpha,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            auto &zoneCV = state.dataRoomAir->ZoneCrossVent(Loop);
            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            // First is Zone Name
            // state.dataRoomAir->ZoneUCSDCV(Loop).ZoneName = ipsc->cAlphaArgs(1);
            if ((zoneCV.ZonePtr = Util::FindItemInList(ipsc->cAlphaArgs(1), state.dataHeatBal->Zone)) == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
                ErrorsFound = true;
            } else {
                state.dataRoomAir->IsZoneCrossVent(zoneCV.ZonePtr) = true;
            }
            // Second Alpha is Schedule Name
            // zoneUCSDCV.SchedGainsName = ipsc->cAlphaArgs(2);
            if (ipsc->lAlphaFieldBlanks(2)) {
                ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(2));
                ErrorsFound = true;
            } else if ((zoneCV.gainsSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }

            // Third Alpha is a string: JET or RECIRCULATION
            if (ipsc->lAlphaFieldBlanks(3)) {
                for (int Loop2 = 1; Loop2 <= state.dataHeatBal->TotPeople; ++Loop2) {
                    if (state.dataHeatBal->People(Loop2).ZonePtr != zoneCV.ZonePtr) {
                        continue;
                    }
                    if (!state.dataHeatBal->People(Loop2).Fanger) {
                        continue;
                    }
                    ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(3));
                    ErrorsFound = true;
                }
            } else if ((zoneCV.VforComfort = static_cast<Comfort>(getEnumValue(comfortNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(3))))) ==
                       Comfort::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            }

            if (zoneCV.ZonePtr == 0) {
                continue;
            }

            // Following depend on valid zone

            if (Util::FindItemInList(
                    state.dataHeatBal->Zone(zoneCV.ZonePtr).Name, state.afn->MultizoneZoneData, &AirflowNetwork::MultizoneZoneProp::ZoneName) == 0) {
                ShowSevereError(state, format("Problem with {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "AirflowNetwork airflow model must be active in this zone");
                ErrorsFound = true;
            }

            // If a crack is used it must have an air flow coefficient = 0.5
            for (int iLink = 1; iLink <= state.afn->NumOfLinksMultiZone; ++iLink) {
                auto const &mzSurfaceData = state.afn->MultizoneSurfaceData(iLink);
                int nodeNum1 = mzSurfaceData.NodeNums[0];
                int nodeNum2 = mzSurfaceData.NodeNums[1];
                if (state.dataSurface->Surface(mzSurfaceData.SurfNum).Zone == zoneCV.ZonePtr ||
                    (state.afn->AirflowNetworkNodeData(nodeNum2).EPlusZoneNum == zoneCV.ZonePtr &&
                     state.afn->AirflowNetworkNodeData(nodeNum1).EPlusZoneNum > 0) ||
                    (state.afn->AirflowNetworkNodeData(nodeNum2).EPlusZoneNum > 0 &&
                     state.afn->AirflowNetworkNodeData(nodeNum1).EPlusZoneNum == zoneCV.ZonePtr)) {
                    int compNum = state.afn->AirflowNetworkLinkageData(iLink).CompNum;
                    int typeNum = state.afn->AirflowNetworkCompData(compNum).TypeNum;
                    if (state.afn->AirflowNetworkCompData(compNum).CompTypeNum == AirflowNetwork::iComponentTypeNum::SCR) {
                        if (state.afn->MultizoneSurfaceCrackData(typeNum).exponent != 0.50) {
                            state.dataRoomAir->AirModel(zoneCV.ZonePtr).AirModel = RoomAirModel::Mixing;
                            ShowWarningError(state, format("Problem with {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                            ShowWarningError(state, format("Roomair model will not be applied for Zone={}.", ipsc->cAlphaArgs(1)));
                            ShowContinueError(
                                state,
                                format("AirflowNetwrok:Multizone:Surface crack object must have an air flow coefficient = 0.5, value was={:.2R}",
                                       state.afn->MultizoneSurfaceCrackData(typeNum).exponent));
                        }
                    }
                } // if
            } // for (iLink)
        } // for (Loop)
    }

    void GetUFADZoneData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2005

        // PURPOSE OF THIS SUBROUTINE:
        //  Get UCSD UFAD interior zone model controls for all zones at once

        // METHODOLOGY EMPLOYED:
        // Use input processor to get input from idf file

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        constexpr std::string_view routineName = "GetUFADZoneData";

        int IOStat;
        int NumAlpha;
        int NumNumber;

        if (!state.dataRoomAir->UCSDModelUsed) {
            state.dataRoomAir->TotUFADInt = 0;
            state.dataRoomAir->TotUFADExt = 0;
            return;
        }

        auto &ipsc = state.dataIPShortCut;

        ipsc->cCurrentModuleObject = "RoomAirSettings:UnderFloorAirDistributionInterior";
        state.dataRoomAir->TotUFADInt = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
        ipsc->cCurrentModuleObject = "RoomAirSettings:UnderFloorAirDistributionExterior";
        state.dataRoomAir->TotUFADExt = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);

        if (state.dataRoomAir->TotUFADInt <= 0 && state.dataRoomAir->TotUFADExt <= 0) {
            return;
        }

        state.dataRoomAir->ZoneUFAD.allocate(state.dataRoomAir->TotUFADInt + state.dataRoomAir->TotUFADExt);
        state.dataRoomAir->ZoneUFADPtr.dimension(state.dataGlobal->NumOfZones, 0);

        ipsc->cCurrentModuleObject = "RoomAirSettings:UnderFloorAirDistributionInterior";
        for (int Loop = 1; Loop <= state.dataRoomAir->TotUFADInt; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlpha,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);
            // First is Zone Name
            auto &zoneUI = state.dataRoomAir->ZoneUFAD(Loop);
            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};

            zoneUI.ZoneName = ipsc->cAlphaArgs(1);
            zoneUI.ZonePtr = Util::FindItemInList(ipsc->cAlphaArgs(1), state.dataHeatBal->Zone);
            state.dataRoomAir->ZoneUFADPtr(zoneUI.ZonePtr) = Loop;
            if (zoneUI.ZonePtr == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
                ErrorsFound = true;
            } else {
                state.dataRoomAir->IsZoneUFAD(zoneUI.ZonePtr) = true;
                state.dataRoomAir->ZoneUFADPtr(zoneUI.ZonePtr) = Loop;
            }

            // 2nd alpha is diffuser type
            zoneUI.DiffuserType = static_cast<Diffuser>(getEnumValue(diffuserNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
            if (zoneUI.DiffuserType == Diffuser::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }
            // 1st number is Number of Diffusers per Zone
            zoneUI.DiffusersPerZone = ipsc->rNumericArgs(1);
            // 2nd number is Power per Plume
            zoneUI.PowerPerPlume = ipsc->rNumericArgs(2);
            // 3rd number is Design Effective Area of Diffuser
            zoneUI.DiffArea = ipsc->rNumericArgs(3);
            // 4th number is Diffuser Slot Angle from Vertical
            zoneUI.DiffAngle = ipsc->rNumericArgs(4);
            // 5th number is Thermostat Height
            zoneUI.ThermostatHeight = ipsc->rNumericArgs(5);
            // 6th number is Comfort Height
            zoneUI.ComfortHeight = ipsc->rNumericArgs(6);
            // 7th number is Temperature Difference Threshold for Reporting
            zoneUI.TempTrigger = ipsc->rNumericArgs(7);
            // 8th number user-specified transition height
            zoneUI.TransHeight = ipsc->rNumericArgs(8);
            // 9th number is Coefficient A in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUI.A_Kc = ipsc->rNumericArgs(9);
            // 10th number is Coefficient B in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUI.B_Kc = ipsc->rNumericArgs(10);
            // 11th number is Coefficient C in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUI.C_Kc = ipsc->rNumericArgs(11);
            // 12th number is Coefficient D in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUI.D_Kc = ipsc->rNumericArgs(12);
            // 13th number is Coefficient E in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUI.E_Kc = ipsc->rNumericArgs(13);
        }

        ipsc->cCurrentModuleObject = "RoomAirSettings:UnderFloorAirDistributionExterior";
        for (int Loop = 1; Loop <= state.dataRoomAir->TotUFADExt; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlpha,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);
            // First is Zone Name
            auto &zoneUE = state.dataRoomAir->ZoneUFAD(Loop + state.dataRoomAir->TotUFADInt);
            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            zoneUE.ZoneName = ipsc->cAlphaArgs(1);
            zoneUE.ZonePtr = Util::FindItemInList(ipsc->cAlphaArgs(1), state.dataHeatBal->Zone);
            if (zoneUE.ZonePtr == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
                ErrorsFound = true;
            } else {
                state.dataRoomAir->IsZoneUFAD(zoneUE.ZonePtr) = true;
                state.dataRoomAir->ZoneUFADPtr(zoneUE.ZonePtr) = Loop + state.dataRoomAir->TotUFADInt;
            }
            zoneUE.DiffuserType = static_cast<Diffuser>(getEnumValue(diffuserNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
            if (zoneUE.DiffuserType == Diffuser::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
            }
            // 1st number is Number of Diffusers per Zone
            zoneUE.DiffusersPerZone = ipsc->rNumericArgs(1);
            // 2nd number is Power per Plume
            zoneUE.PowerPerPlume = ipsc->rNumericArgs(2);
            // 3rd number is Design Effective Area of Diffuser
            zoneUE.DiffArea = ipsc->rNumericArgs(3);
            // 4th number is Diffuser Slot Angle from Vertical
            zoneUE.DiffAngle = ipsc->rNumericArgs(4);
            // 5th number is Thermostat Height
            zoneUE.ThermostatHeight = ipsc->rNumericArgs(5);
            // 6th number is Comfort Height
            zoneUE.ComfortHeight = ipsc->rNumericArgs(6);
            // 7th number is Temperature Difference Threshold for Reporting
            zoneUE.TempTrigger = ipsc->rNumericArgs(7);
            // 8th number user-specified transition height
            zoneUE.TransHeight = ipsc->rNumericArgs(8);
            // 9th number is Coefficient A in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUE.A_Kc = ipsc->rNumericArgs(9);
            // 10th number is Coefficient B in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUE.B_Kc = ipsc->rNumericArgs(10);
            // 11th number is Coefficient C in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUE.C_Kc = ipsc->rNumericArgs(11);
            // 12th number is Coefficient D in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUE.D_Kc = ipsc->rNumericArgs(12);
            // 13th number is Coefficient E in formula Kc = A*Gamma**B + C + D*Gamma + E*Gamma**2
            zoneUE.E_Kc = ipsc->rNumericArgs(13);
        }
    }

    void GetRoomAirflowNetworkData(EnergyPlusData &state, bool &ErrorsFound) // True if errors found during this get input routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   November 2009

        // PURPOSE OF THIS SUBROUTINE:
        //  Get RoomAirflowNetwork data for all zones at once

        // METHODOLOGY EMPLOYED:
        // Use input processor to get input from idf file

        // Using/Aliasing
        using InternalHeatGains::GetInternalGainDeviceIndex;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        constexpr std::string_view routineName = "GetRoomAirflowNetworkData";
        int NumAlphas;
        int NumNumbers;
        int status;
        int TotNumOfRAFNNodeSurfLists;
        int TotNumOfRAFNNodeGainsLists;
        int TotNumOfRAFNNodeHVACLists;
        int TotNumEquip;
        bool IntEquipFound;

        auto &ipsc = state.dataIPShortCut;
        ipsc->cCurrentModuleObject = "RoomAirSettings:AirflowNetwork";
        state.dataRoomAir->NumOfRoomAFNControl = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
        if (state.dataRoomAir->NumOfRoomAFNControl == 0) {
            return;
        }
        if (state.dataRoomAir->NumOfRoomAFNControl > state.dataGlobal->NumOfZones) {
            ShowSevereError(state, format("Too many {} objects in input file", ipsc->cCurrentModuleObject));
            ShowContinueError(state, format("There cannot be more {} objects than number of zones.", ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }

        if (!allocated(state.dataRoomAir->AFNZoneInfo)) {
            state.dataRoomAir->AFNZoneInfo.allocate(state.dataGlobal->NumOfZones);
        }
        for (int Loop = 1; Loop <= state.dataRoomAir->NumOfRoomAFNControl; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     status,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            int ZoneNum = Util::FindItemInList(ipsc->cAlphaArgs(2), state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);

            if (ZoneNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
                continue;
            }
            if (state.dataRoomAir->AirModel(ZoneNum).AirModel != RoomAirModel::AirflowNetwork) {
                ShowSevereError(state,
                                format("GetRoomAirflowNetworkData: Zone specified='{}', Air Model type is not AirflowNetwork.", ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Air Model Type for zone ={}", roomAirModelNamesUC[(int)state.dataRoomAir->AirModel(ZoneNum).AirModel]));
                ErrorsFound = true;
                continue;
            }

            auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
            roomAFNZoneInfo.ZoneID = ZoneNum;
            roomAFNZoneInfo.IsUsed = true;
            roomAFNZoneInfo.Name = ipsc->cAlphaArgs(1);
            roomAFNZoneInfo.ZoneName = ipsc->cAlphaArgs(2); // Zone Name

            roomAFNZoneInfo.NumOfAirNodes = (NumAlphas - 3);

            if (roomAFNZoneInfo.NumOfAirNodes > 0) {
                roomAFNZoneInfo.Node.allocate(roomAFNZoneInfo.NumOfAirNodes);
            } else {
                ShowSevereError(state,
                                format("GetRoomAirflowNetworkData: Incomplete input in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            for (int iAirNode = 1; iAirNode <= roomAFNZoneInfo.NumOfAirNodes; ++iAirNode) {
                roomAFNZoneInfo.Node(iAirNode).Name = ipsc->cAlphaArgs(iAirNode + 3);
            }
            // control point node

            roomAFNZoneInfo.ControlAirNodeID = Util::FindItemInList(ipsc->cAlphaArgs(3), roomAFNZoneInfo.Node, roomAFNZoneInfo.NumOfAirNodes);
            if (roomAFNZoneInfo.ControlAirNodeID == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
                continue;
            }

            roomAFNZoneInfo.totNumSurfs = 0;
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                roomAFNZoneInfo.totNumSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
            }
        } // for (Loop)

        ipsc->cCurrentModuleObject = "RoomAir:Node:AirflowNetwork";
        state.dataRoomAir->TotNumOfRoomAFNNodes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
        for (int Loop = 1; Loop <= state.dataRoomAir->TotNumOfRoomAFNNodes; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     status,
                                                                     _,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            int ZoneNum = Util::FindItemInList(ipsc->cAlphaArgs(2), state.dataHeatBal->Zone, state.dataGlobal->NumOfZones);
            if (ZoneNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
                ErrorsFound = true;
                continue;
            }

            auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
            int RAFNNodeNum = Util::FindItemInList(ipsc->cAlphaArgs(1), roomAFNZoneInfo.Node, roomAFNZoneInfo.NumOfAirNodes);
            if (RAFNNodeNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1));
                ErrorsFound = true;
                continue;
            }

            auto &roomAFNZoneNode = roomAFNZoneInfo.Node(RAFNNodeNum);
            roomAFNZoneNode.ZoneVolumeFraction = ipsc->rNumericArgs(1);
            if (!ipsc->lAlphaFieldBlanks(3)) {
                roomAFNZoneNode.NodeSurfListName = ipsc->cAlphaArgs(3);
            } else {
                roomAFNZoneNode.HasSurfacesAssigned = false;
            }
            if (!ipsc->lAlphaFieldBlanks(4)) {
                roomAFNZoneNode.NodeIntGainsListName = ipsc->cAlphaArgs(4);
            } else {
                roomAFNZoneNode.HasIntGainsAssigned = false;
            }
            if (!ipsc->lAlphaFieldBlanks(5)) {
                roomAFNZoneNode.NodeHVACListName = ipsc->cAlphaArgs(5);
            } else {
                roomAFNZoneNode.HasHVACAssigned = false;
            }

        } // loop thru TotNumOfRoomAFNNodes

        ipsc->cCurrentModuleObject = "RoomAir:Node:AirflowNetwork:AdjacentSurfaceList";
        TotNumOfRAFNNodeSurfLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
        for (int Loop = 1; Loop <= TotNumOfRAFNNodeSurfLists; ++Loop) {
            bool foundList = false;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                // find surface list
                int RAFNNodeNum = 0;
                auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
                if (roomAFNZoneInfo.NumOfAirNodes > 0) {
                    RAFNNodeNum = Util::FindItemInList(
                        ipsc->cAlphaArgs(1), roomAFNZoneInfo.Node, &AFNAirNodeNested::NodeSurfListName, roomAFNZoneInfo.NumOfAirNodes);
                }

                if (RAFNNodeNum == 0) {
                    continue;
                }

                // found it
                foundList = true;
                int NumSurfsThisNode = NumAlphas - 1;
                int NumOfSurfs = 0; // What is this used for?
                for (int spaceNum : state.dataHeatBal->Zone(iZone).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    NumOfSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
                }

                auto &roomAFNZoneNode = roomAFNZoneInfo.Node(RAFNNodeNum);
                if (allocated(roomAFNZoneNode.SurfMask)) {
                    // throw error found twice
                    ShowSevereError(state, format("GetRoomAirflowNetworkData: Invalid {} = {}", ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Entered in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, "Duplicate RoomAir:Node:AirflowNetwork:AdjacentSurfaceList name.");
                    ErrorsFound = true;
                    continue;
                }

                roomAFNZoneNode.SurfMask.allocate(roomAFNZoneInfo.totNumSurfs);
                roomAFNZoneNode.SurfMask = false; // init
                roomAFNZoneNode.HasSurfacesAssigned = true;
                // relate surfaces to this air node and check to see whether surface names are specified correctly or not
                int SurfCount = 0;
                int thisSurfinZone = 0;
                for (int ListSurfNum = 2; ListSurfNum <= NumAlphas; ++ListSurfNum) {
                    for (int spaceNum : state.dataHeatBal->Zone(iZone).spaceIndexes) {
                        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                            ++thisSurfinZone;
                            if (ipsc->cAlphaArgs(ListSurfNum) == state.dataSurface->Surface(SurfNum).Name) {
                                roomAFNZoneNode.SurfMask(thisSurfinZone) = true;
                                ++SurfCount;
                                break;
                            }
                        }
                        if (SurfCount > 0) {
                            break;
                        }
                    }
                }
                if (NumSurfsThisNode != SurfCount) {
                    ShowSevereError(state, format("GetRoomAirflowNetworkData: Invalid {} = {}", ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Entered in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, "Some surface names were not found in the zone");
                    ErrorsFound = true;
                }
            } // for (iZone)

            if (!foundList) { // throw error
                ShowSevereError(state, format("GetRoomAirflowNetworkData: Invalid {} = {}", ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Entered in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Did not find a RoomAir:Node:AirflowNetwork object that references this object");
                ErrorsFound = true;
            }
        } // loop thru TotNumOfRAFNNodeSurfLists

        ipsc->cCurrentModuleObject = "RoomAir:Node:AirflowNetwork:InternalGains";
        TotNumOfRAFNNodeGainsLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cCurrentModuleObject);
        for (int Loop = 1; Loop <= TotNumOfRAFNNodeGainsLists; ++Loop) {
            int foundList = false;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     ipsc->cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)};

            if (mod((NumAlphas + NumNumbers - 1), 3) != 0) {
                ShowSevereError(state, format("GetRoomAirflowNetworkData: For {}: {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ShowContinueError(
                    state, format("Extensible field set are not evenly divisable by 3. Number of data entries = {}", NumAlphas + NumNumbers - 1));
                ErrorsFound = true;
                break;
            }

            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
                // find surface list
                int RAFNNodeNum = 0;
                if (roomAFNZoneInfo.NumOfAirNodes > 0) {
                    RAFNNodeNum = Util::FindItemInList(
                        ipsc->cAlphaArgs(1), roomAFNZoneInfo.Node, &AFNAirNodeNested::NodeIntGainsListName, roomAFNZoneInfo.NumOfAirNodes);
                }
                if (RAFNNodeNum == 0) {
                    continue;
                }

                // found it
                foundList = true;
                int numInputGains = (NumAlphas + NumNumbers - 1) / 3;
                int numSpacesInZone = state.dataHeatBal->Zone(iZone).numSpaces;
                int maxNumGains = numInputGains * numSpacesInZone;
                auto &roomAFNZoneNode = roomAFNZoneInfo.Node(RAFNNodeNum);
                if (allocated(roomAFNZoneNode.IntGain)) {
                    ShowSevereError(state, format("GetRoomAirflowNetworkData: Invalid {} = {}", ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Entered in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Duplicate {} name.", ipsc->cCurrentModuleObject));
                    ErrorsFound = true;
                    continue;
                }

                roomAFNZoneNode.IntGain.allocate(maxNumGains);
                roomAFNZoneNode.IntGainsDeviceIndices.allocate(maxNumGains);
                roomAFNZoneNode.intGainsDeviceSpaces.allocate(maxNumGains);
                roomAFNZoneNode.IntGainsFractions.allocate(maxNumGains);
                roomAFNZoneNode.HasIntGainsAssigned = true;
                int numGainsFound = 0;
                for (int gainsLoop = 1; gainsLoop <= numInputGains; ++gainsLoop) {
                    auto &intGain = roomAFNZoneNode.IntGain(gainsLoop);
                    intGain.type = static_cast<DataHeatBalance::IntGainType>(
                        getEnumValue(DataHeatBalance::IntGainTypeNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(gainsLoop * 2))));

                    if (intGain.type == DataHeatBalance::IntGainType::Invalid) {
                        ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(gainsLoop * 2), ipsc->cAlphaArgs(gainsLoop * 2));
                        ErrorsFound = true;
                        continue;
                    }
                    intGain.Name = ipsc->cAlphaArgs(gainsLoop * 2 + 1);

                    bool gainFound = false;
                    // check all spaces in this zone for matching gains
                    for (int spaceNum : state.dataHeatBal->Zone(iZone).spaceIndexes) {
                        // verify type and name and get pointer to device in internal gains structure array
                        int intGainIndex = GetInternalGainDeviceIndex(state, spaceNum, intGain.type, intGain.Name);
                        if (intGainIndex >= 0) {
                            gainFound = true;
                            ++numGainsFound;
                            roomAFNZoneNode.intGainsDeviceSpaces(numGainsFound) = spaceNum;
                            roomAFNZoneNode.IntGainsDeviceIndices(numGainsFound) = intGainIndex;
                            roomAFNZoneNode.IntGainsFractions(numGainsFound) = ipsc->rNumericArgs(gainsLoop);
                        }
                    }
                    if (gainFound) {
                        roomAFNZoneNode.NumIntGains = numGainsFound;
                    } else {
                        ShowSevereError(state,
                                        format("GetRoomAirflowNetworkData: Invalid {} = {}",
                                               ipsc->cAlphaFieldNames(gainsLoop * 2 + 1),
                                               ipsc->cAlphaArgs(gainsLoop * 2 + 1)));
                        ShowContinueError(state, format("Entered in {} = {}", ipsc->cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                        ShowContinueError(state, "Internal gain did not match correctly");
                        ErrorsFound = true;
                    }
                } // for (gainsLoop)
            } // for (iZone)
        } // loop thru TotNumOfRAFNNodeGainsLists

        // Get data of HVAC equipment
        std::string const cCurrentModuleObject = "RoomAir:Node:AirflowNetwork:HVACEquipment";
        TotNumOfRAFNNodeHVACLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        for (int Loop = 1; Loop <= TotNumOfRAFNNodeHVACLists; ++Loop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     status,
                                                                     _,
                                                                     _,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)};

            if (mod((NumAlphas + NumNumbers - 1), 4) != 0) {
                ShowSevereError(state, format("GetRoomAirflowNetworkData: For {}: {}", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Extensible field set are not evenly divisable by 4. Number of data entries = {}",
                                         fmt::to_string(NumAlphas + NumNumbers - 1)));
                ErrorsFound = true;
                break;
            }

            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
                // find surface list
                int RAFNNodeNum = 0;
                if (roomAFNZoneInfo.NumOfAirNodes > 0) {
                    RAFNNodeNum = Util::FindItemInList(
                        ipsc->cAlphaArgs(1), roomAFNZoneInfo.Node, &AFNAirNodeNested::NodeHVACListName, roomAFNZoneInfo.NumOfAirNodes);
                }

                if (RAFNNodeNum == 0) {
                    continue;
                }

                // found it
                auto &roomAFNNode = roomAFNZoneInfo.Node(RAFNNodeNum);
                if (allocated(roomAFNNode.HVAC)) {
                    ShowSevereError(state, format("GetRoomAirflowNetworkData: Invalid {} = {}", ipsc->cAlphaFieldNames(1), ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Duplicate {} name.", cCurrentModuleObject));
                    ErrorsFound = true;
                    continue;
                }

                roomAFNNode.NumHVACs = (NumAlphas + NumNumbers - 1) / 4;
                roomAFNNode.HVAC.allocate(roomAFNNode.NumHVACs);
                roomAFNNode.HasHVACAssigned = true;
                for (int iEquip = 1; iEquip <= roomAFNNode.NumHVACs; ++iEquip) {
                    int iEquipArg = 2 + (iEquip - 1) * 2;
                    auto &roomAFNNodeHVAC = roomAFNNode.HVAC(iEquip);
                    roomAFNNodeHVAC.zoneEquipType = static_cast<DataZoneEquipment::ZoneEquipType>(
                        getEnumValue(DataZoneEquipment::zoneEquipTypeNamesUC, ipsc->cAlphaArgs(iEquipArg)));
                    if (roomAFNNodeHVAC.zoneEquipType == DataZoneEquipment::ZoneEquipType::Invalid) {
                        ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(iEquipArg), ipsc->cAlphaArgs(iEquipArg));
                        ErrorsFound = true;
                    }
                    roomAFNNodeHVAC.Name = ipsc->cAlphaArgs(3 + (iEquip - 1) * 2);

                    // verify type and name and get pointer to device in HVAC equipment type and name structure array
                    TotNumEquip = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ipsc->cAlphaArgs(iEquipArg));
                    if (TotNumEquip == 0) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(iEquipArg), ipsc->cAlphaArgs(iEquipArg));
                        ErrorsFound = true;
                    }
                    roomAFNNodeHVAC.SupplyFraction = ipsc->rNumericArgs(iEquipArg);
                    roomAFNNodeHVAC.ReturnFraction = ipsc->rNumericArgs(iEquipArg);

                    // get equipment type
                    for (int thisZoneEquipNum = 1; thisZoneEquipNum <= state.dataZoneEquip->ZoneEquipList(iZone).NumOfEquipTypes;
                         ++thisZoneEquipNum) {
                        if (Util::SameString(state.dataZoneEquip->ZoneEquipList(iZone).EquipName(thisZoneEquipNum), roomAFNNodeHVAC.Name) &&
                            roomAFNNodeHVAC.zoneEquipType == state.dataZoneEquip->ZoneEquipList(iZone).EquipType(thisZoneEquipNum)) {
                            break;
                        }
                    }
                    IntEquipFound = CheckEquipName(
                        state, roomAFNNodeHVAC.Name, roomAFNNodeHVAC.SupplyNodeName, roomAFNNodeHVAC.ReturnNodeName, roomAFNNodeHVAC.zoneEquipType);

                    if (!IntEquipFound) {
                        ShowSevereError(state,
                                        format("GetRoomAirflowNetworkData: Invalid {} = {}",
                                               ipsc->cAlphaFieldNames(3 + (iEquip - 1) * 2),
                                               ipsc->cAlphaArgs(2 + (iEquip - 1) * 2)));
                        ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                        ShowContinueError(state, "Internal gain did not match correctly");
                        ErrorsFound = true;
                    }
                    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    // TYPE RoomAirflowNetworkHVACStruct
                    // INTEGER::EquipConfigIndex = 0
                } // for (iEquip)
            } // for (Zone)
        } // loop thru TotNumOfRAFNNodeHVACLists

        // do some checks on input data
        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
            if (roomAFNZoneInfo.NumOfAirNodes == 0) {
                continue;
            }

            // Check zone volume fraction
            Real64 SumFraction = 0.0;
            for (int iRoomAFNNode = 1; iRoomAFNNode <= roomAFNZoneInfo.NumOfAirNodes; ++iRoomAFNNode) {
                SumFraction += roomAFNZoneInfo.Node(iRoomAFNNode).ZoneVolumeFraction;
            }
            if (std::abs(SumFraction - 1.0) > 0.001) {
                ShowSevereError(state, "GetRoomAirflowNetworkData: Invalid, zone volume fractions do not sum to 1.0");
                ShowContinueError(state, format("Entered in RoomAir:Node:AirflowNetwork with Zone Name = {}", state.dataHeatBal->Zone(iZone).Name));
                ShowContinueError(state, "The Fraction of Zone Air Volume values across all the nodes needs to sum to 1.0.");
                ShowContinueError(state, format("The sum of fractions entered = {:.3R}", SumFraction));
                ErrorsFound = true;
            }

            // Check internal gain fraction
            for (int iRoomAFNNode = 1; iRoomAFNNode <= roomAFNZoneInfo.NumOfAirNodes; ++iRoomAFNNode) {
                auto &roomAFNNode = roomAFNZoneInfo.Node(iRoomAFNNode);
                for (int iGain = 1; iGain <= roomAFNNode.NumIntGains; ++iGain) {
                    auto &intGain = roomAFNNode.IntGain(iGain);
                    if (intGain.FractionCheck) {
                        continue;
                    }
                    SumFraction = roomAFNNode.IntGainsFractions(iGain);
                    intGain.FractionCheck = true;

                    for (int iRoomAFNNode2 = 1; iRoomAFNNode2 <= roomAFNZoneInfo.NumOfAirNodes; ++iRoomAFNNode2) {
                        auto &roomAFNNode2 = roomAFNZoneInfo.Node(iRoomAFNNode2);
                        for (int iGain2 = 1; iGain2 <= roomAFNNode2.NumIntGains; ++iGain2) {
                            auto &intGain2 = roomAFNNode2.IntGain(iGain2);
                            if (intGain2.FractionCheck) {
                                continue;
                            }
                            if (intGain.type == intGain2.type && Util::SameString(intGain.Name, intGain2.Name)) {
                                SumFraction += roomAFNNode2.IntGainsFractions(iGain2);
                                intGain2.FractionCheck = true;
                            }
                        }
                    }
                    if (std::abs(SumFraction - 1.0) > 0.001) {
                        ShowSevereError(state, "GetRoomAirflowNetworkData: Invalid, internal gain fractions do not sum to 1.0");
                        ShowContinueError(state,
                                          format("Entered in RoomAir:Node:AirflowNetwork with Zone Name = {}, Intrnal gain name = {}",
                                                 state.dataHeatBal->Zone(iZone).Name,
                                                 intGain.Name));
                        ShowContinueError(state, "The Fraction of internal gain across all the nodes needs to sum to 1.0.");
                        ShowContinueError(state, format("The sum of fractions entered = {:.3R}", SumFraction));
                        ErrorsFound = true;
                    }
                } // for (iGain)
            } // for (iRoomAFNNode)
        } // for (iZone)

        if (ErrorsFound) {
            return;
        }

        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            auto &roomAFNZoneInfo = state.dataRoomAir->AFNZoneInfo(iZone);
            if (!roomAFNZoneInfo.IsUsed || roomAFNZoneInfo.NumOfAirNodes == 0) {
                continue;
            }

            for (int iAirNode = 1; iAirNode <= roomAFNZoneInfo.NumOfAirNodes; ++iAirNode) {
                auto &roomAFNZoneNode = roomAFNZoneInfo.Node(iAirNode);
                SetupOutputVariable(state,
                                    "RoomAirflowNetwork Node Temperature",
                                    Constant::Units::C,
                                    roomAFNZoneNode.AirTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    roomAFNZoneNode.Name);
                SetupOutputVariable(state,
                                    "RoomAirflowNetwork Node Humidity Ratio",
                                    Constant::Units::kgWater_kgDryAir,
                                    roomAFNZoneNode.HumRat,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    roomAFNZoneNode.Name);
                SetupOutputVariable(state,
                                    "RoomAirflowNetwork Node Relative Humidity",
                                    Constant::Units::Perc,
                                    roomAFNZoneNode.RelHumidity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    roomAFNZoneNode.Name);
            } // for (iAirNodE)
        } // for (iZone)
    }

    void SharedDVCVUFDataInit(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   March 2005
        //       MODIFIED       Aug, 2013, Sam Brunswick -- for RoomAirCrossCrossVent modifications

        // PURPOSE OF THIS SUBROUTINE:
        // This routine allocates and initializes(?) the data that is shared between the
        // UCSD models (DV and CV)

        // Using/Aliasing
        using namespace DataEnvironment;
        using namespace DataSurfaces;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr BaseDischargeCoef(0.62);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_int AuxSurf;
        Array2D_int AuxAirflowNetworkSurf;

        // Do the one time initializations
        if (state.dataRoomAir->MyOneTimeFlag) {

            state.dataRoomAir->MyEnvrnFlag.allocate(state.dataGlobal->NumOfZones);

            state.dataRoomAir->APos_Wall.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->APos_Floor.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->APos_Ceiling.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->PosZ_Wall.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->PosZ_Floor.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->PosZ_Ceiling.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->APos_Window.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->APos_Door.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->APos_Internal.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->PosZ_Window.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->PosZ_Door.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->PosZ_Internal.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->HCeiling.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->HWall.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->HFloor.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->HInternal.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->HWindow.allocate(state.dataSurface->TotSurfaces);
            state.dataRoomAir->HDoor.allocate(state.dataSurface->TotSurfaces);

            AuxSurf.allocate(state.dataGlobal->NumOfZones);

            state.dataRoomAir->ZoneCeilingHeight1.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->ZoneCeilingHeight2.allocate(state.dataGlobal->NumOfZones);
            state.dataRoomAir->ZoneCeilingHeight1 = 0.0;
            state.dataRoomAir->ZoneCeilingHeight2 = 0.0;

            // Arrays initializations
            state.dataRoomAir->APos_Wall = 0;
            state.dataRoomAir->APos_Floor = 0;
            state.dataRoomAir->APos_Ceiling = 0;
            std::fill(state.dataRoomAir->PosZ_Wall.begin(), state.dataRoomAir->PosZ_Wall.end(), BegEnd());
            std::fill(state.dataRoomAir->PosZ_Floor.begin(), state.dataRoomAir->PosZ_Floor.end(), BegEnd());
            std::fill(state.dataRoomAir->PosZ_Ceiling.begin(), state.dataRoomAir->PosZ_Ceiling.end(), BegEnd());
            state.dataRoomAir->APos_Window = 0;
            state.dataRoomAir->APos_Door = 0;
            state.dataRoomAir->APos_Internal = 0;
            std::fill(state.dataRoomAir->PosZ_Window.begin(), state.dataRoomAir->PosZ_Window.end(), BegEnd());
            std::fill(state.dataRoomAir->PosZ_Door.begin(), state.dataRoomAir->PosZ_Door.end(), BegEnd());
            std::fill(state.dataRoomAir->PosZ_Internal.begin(), state.dataRoomAir->PosZ_Internal.end(), BegEnd());
            state.dataRoomAir->HCeiling = 0.0;
            state.dataRoomAir->HWall = 0.0;
            state.dataRoomAir->HFloor = 0.0;
            state.dataRoomAir->HInternal = 0.0;
            state.dataRoomAir->HWindow = 0.0;
            state.dataRoomAir->HDoor = 0.0;

            int contWall = 0, contFloor = 0, contCeiling = 0, contWindow = 0, contInternal = 0, contDoor = 0;

            // Put the surface and zone information in Apos and PosZ arrays
            for (int ZNum = 1; ZNum <= state.dataGlobal->NumOfZones; ++ZNum) {
                // advance ONE position in the arrays PosZ because this is a new zone
                int contWallBeg = contWall + 1;
                int contFloorBeg = contFloor + 1;
                int contCeilingBeg = contCeiling + 1;
                int contWindowBeg = contWindow + 1;
                int contInternalBeg = contInternal + 1;
                int contDoorBeg = contDoor + 1;

                Real64 Z1ofZone = std::numeric_limits<Real64>::max();
                Real64 Z2ofZone = std::numeric_limits<Real64>::lowest();

                // cycle in this zone for all the surfaces
                for (int spaceNum : state.dataHeatBal->Zone(ZNum).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);

                    for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                        auto const &surf = state.dataSurface->Surface(SurfNum);
                        if (surf.Class != DataSurfaces::SurfaceClass::IntMass) {
                            // Recalculate lowest and highest height for the zone
                            for (int i = 1, u = surf.Sides; i <= u; ++i) {
                                Real64 const z_i = surf.Vertex(i).z;
                                if (z_i < Z1ofZone) {
                                    Z1ofZone = z_i;
                                };
                                if (z_i > Z2ofZone) {
                                    Z2ofZone = z_i;
                                };
                            }
                        }

                        // Put the reference to this surface in the appropriate array
                        if (surf.Class == SurfaceClass::Floor) {
                            ++contFloor;
                            state.dataRoomAir->APos_Floor(contFloor) = SurfNum;
                        } else if (surf.Class == SurfaceClass::Wall) {
                            ++contWall;
                            state.dataRoomAir->APos_Wall(contWall) = SurfNum;
                        } else if (surf.Class == SurfaceClass::Window) {
                            ++contWindow;
                            state.dataRoomAir->APos_Window(contWindow) = SurfNum;
                        } else if (surf.Class == SurfaceClass::IntMass) {
                            ++contInternal;
                            state.dataRoomAir->APos_Internal(contInternal) = SurfNum;
                        } else if (surf.Class == SurfaceClass::Door) {
                            ++contDoor;
                            state.dataRoomAir->APos_Door(contDoor) = SurfNum;
                        } else {
                            ++contCeiling;
                            state.dataRoomAir->APos_Ceiling(contCeiling) = SurfNum;
                        }
                    }
                } // for (SurfNum)

                int contWallLast = contWall;
                int contFloorLast = contFloor;
                int contCeilingLast = contCeiling;
                int contWindowLast = contWindow;
                int contDoorLast = contDoor;
                int contInternalLast = contInternal;
                // PosZ_Wall (... + 1) has the Begin Wall reference in Apos_Wall for the ZNum
                // PosZ_Wall (... + 2) has the End Wall reference in Apos_Wall for the ZNum
                state.dataRoomAir->PosZ_Wall(ZNum).beg = contWallBeg;
                state.dataRoomAir->PosZ_Wall(ZNum).end = contWallLast;
                state.dataRoomAir->PosZ_Floor(ZNum).beg = contFloorBeg;
                state.dataRoomAir->PosZ_Floor(ZNum).end = contFloorLast;
                state.dataRoomAir->PosZ_Ceiling(ZNum).beg = contCeilingBeg;
                state.dataRoomAir->PosZ_Ceiling(ZNum).end = contCeilingLast;
                state.dataRoomAir->PosZ_Window(ZNum).beg = contWindowBeg;
                state.dataRoomAir->PosZ_Window(ZNum).end = contWindowLast;
                state.dataRoomAir->PosZ_Door(ZNum).beg = contDoorBeg;
                state.dataRoomAir->PosZ_Door(ZNum).end = contDoorLast;
                state.dataRoomAir->PosZ_Internal(ZNum).beg = contInternalBeg;
                state.dataRoomAir->PosZ_Internal(ZNum).end = contInternalLast;
                // Save the highest and lowest height for this zone
                state.dataRoomAir->ZoneCeilingHeight1(ZNum) = Z1ofZone;
                state.dataRoomAir->ZoneCeilingHeight2(ZNum) = Z2ofZone;

                constexpr Real64 CeilingHeightDiffMaximum = 0.1;
                if (std::abs((Z2ofZone - Z1ofZone) - state.dataHeatBal->Zone(ZNum).CeilingHeight) > CeilingHeightDiffMaximum) {
                    ShowWarningError(state, format("RoomAirManager: Inconsistent ceiling heights in Zone: {}", state.dataHeatBal->Zone(ZNum).Name));
                    ShowContinueError(state, format("Lowest height=[{:.3R}].", Z1ofZone));
                    ShowContinueError(state, format("Highest height=[{:.3R}].", Z2ofZone));
                    ShowContinueError(state, format("Ceiling height=[{:.3R}].", state.dataHeatBal->Zone(ZNum).CeilingHeight));
                }
            } // for (ZoneNum)

            AuxSurf = 0;
            state.dataRoomAir->CrossVentNumAFNSurfaces = 0;

            // calculate maximum number of airflow network surfaces in each zone
            for (int iMzLink = 1; iMzLink <= state.afn->NumOfLinksMultiZone; ++iMzLink) {
                auto const &mzSurf = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(iMzLink).SurfNum);
                ++AuxSurf(mzSurf.Zone);
                ++state.dataRoomAir->CrossVentNumAFNSurfaces;
                // Check if this is an interzone airflow network surface
                if (mzSurf.ExtBoundCond > 0 && (state.afn->MultizoneSurfaceData(iMzLink).SurfNum != mzSurf.ExtBoundCond)) {
                    ++AuxSurf(state.dataSurface->Surface(mzSurf.ExtBoundCond).Zone);
                    ++state.dataRoomAir->CrossVentNumAFNSurfaces;
                }
            }
            // calculate maximum number of airflow network surfaces in a single zone
            int MaxSurf = AuxSurf(1);
            for (int iZone = 2; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                if (AuxSurf(iZone) > MaxSurf) {
                    MaxSurf = AuxSurf(iZone);
                }
            }

            if (!allocated(state.dataRoomAir->AFNSurfaceCrossVent)) {
                state.dataRoomAir->AFNSurfaceCrossVent.allocate({0, MaxSurf}, state.dataGlobal->NumOfZones);
            }
            if (!allocated(state.dataRoomAir->CrossVentJetRecFlows)) {
                state.dataRoomAir->CrossVentJetRecFlows.allocate({0, MaxSurf}, state.dataGlobal->NumOfZones);
            }
            AuxAirflowNetworkSurf.allocate({0, MaxSurf}, state.dataGlobal->NumOfZones);
            // Width and Height for airflow network surfaces
            if (!allocated(state.dataRoomAir->SurfParametersCrossDispVent)) {
                state.dataRoomAir->SurfParametersCrossDispVent.allocate(state.afn->NumOfLinksMultiZone);
            }

            state.dataRoomAir->AFNSurfaceCrossVent = 0;
            // Organize surfaces in vector AirflowNetworkSurfaceUCSDCV(Zone, surface indexes)
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                // the 0 component of the array has the number of relevant AirflowNetwork surfaces for the zone
                state.dataRoomAir->AFNSurfaceCrossVent(0, iZone) = AuxSurf(iZone);
                if (AuxSurf(iZone) == 0) {
                    continue;
                }

                Real64 const ceilingHeight = state.dataRoomAir->ZoneCeilingHeight1(iZone);
                int SurfNum = 1;

                for (int iMzLink = 1; iMzLink <= state.afn->NumOfLinksMultiZone; ++iMzLink) {
                    auto const &mzSurf = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(iMzLink).SurfNum);
                    auto &surfParams = state.dataRoomAir->SurfParametersCrossDispVent(iMzLink);

                    if (mzSurf.Zone == iZone) {
                        // SurfNum has the reference surface number relative to AirflowNetworkSurfaceData
                        state.dataRoomAir->AFNSurfaceCrossVent(SurfNum, iZone) = iMzLink;
                        // calculate the surface width and height
                        int compNum = state.afn->AirflowNetworkLinkageData(iMzLink).CompNum;
                        int typeNum = state.afn->AirflowNetworkCompData(compNum).TypeNum;
                        if (state.afn->AirflowNetworkCompData(compNum).CompTypeNum == AirflowNetwork::iComponentTypeNum::DOP) {
                            Real64 WidthFactMax = 0.0;
                            Real64 HeightFactMax = 0.0;

                            Real64 WidthFact = 0.0;
                            Real64 HeightFact = 0.0;
                            auto const &mzCompDetOpening = state.afn->MultizoneCompDetOpeningData(typeNum);
                            for (int Loop3 = 1; Loop3 <= mzCompDetOpening.NumFac; ++Loop3) {
                                if (Loop3 == 1) {
                                    WidthFact = mzCompDetOpening.WidthFac1;
                                    HeightFact = mzCompDetOpening.HeightFac1;
                                } else if (Loop3 == 2) {
                                    WidthFact = mzCompDetOpening.WidthFac2;
                                    HeightFact = mzCompDetOpening.HeightFac2;
                                } else if (Loop3 == 3) {
                                    WidthFact = mzCompDetOpening.WidthFac3;
                                    HeightFact = mzCompDetOpening.HeightFac3;
                                } else if (Loop3 == 4) {
                                    WidthFact = mzCompDetOpening.WidthFac4;
                                    HeightFact = mzCompDetOpening.HeightFac4;
                                }
                                if (WidthFact > WidthFactMax) {
                                    WidthFactMax = WidthFact;
                                }
                                if (HeightFact > HeightFactMax) {
                                    HeightFactMax = HeightFact;
                                }
                            }
                            surfParams.Width = WidthFactMax * mzSurf.Width;
                            surfParams.Height = HeightFactMax * mzSurf.Height;

                        } else if (state.afn->AirflowNetworkCompData(compNum).CompTypeNum ==
                                   AirflowNetwork::iComponentTypeNum::SCR) { // surface type = CRACK
                            surfParams.Width = mzSurf.Width / 2;
                            auto const &zoneHeatBal = state.dataZoneTempPredictorCorrector->zoneHeatBalance(iZone);
                            Real64 AinCV =
                                state.afn->MultizoneSurfaceCrackData(typeNum).coefficient /
                                (BaseDischargeCoef *
                                 std::sqrt(2.0 / PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, zoneHeatBal.MAT, zoneHeatBal.airHumRat)));
                            surfParams.Height = AinCV / surfParams.Width;
                        }

                        // calculate the surface Zmin and Zmax
                        if (state.afn->AirflowNetworkCompData(compNum).CompTypeNum == AirflowNetwork::iComponentTypeNum::DOP ||
                            state.afn->AirflowNetworkCompData(compNum).CompTypeNum ==
                                AirflowNetwork::iComponentTypeNum::SCR) { // surface type = CRACK
                            Real64 z_min(std::numeric_limits<Real64>::max()), z_max(std::numeric_limits<Real64>::lowest());
                            for (int i = 1; i <= mzSurf.Sides; ++i) {
                                Real64 const z_i = mzSurf.Vertex(i).z;
                                z_min = std::min(z_min, z_i);
                                z_max = std::max(z_max, z_i);
                            }
                            surfParams.Zmin = z_min - ceilingHeight;
                            surfParams.Zmax = z_max - ceilingHeight;
                        }

                        ++SurfNum;
                        // Check if airflow network Surface is an interzone surface:

                    } else { // if (mzSurf.Zone == iZone)
                        int nodeNum1 = state.afn->MultizoneSurfaceData(iMzLink).NodeNums[0];
                        int nodeNum2 = state.afn->MultizoneSurfaceData(iMzLink).NodeNums[1];
                        if ((state.afn->AirflowNetworkNodeData(nodeNum2).EPlusZoneNum == iZone &&
                             state.afn->AirflowNetworkNodeData(nodeNum1).EPlusZoneNum > 0) ||
                            (state.afn->AirflowNetworkNodeData(nodeNum2).EPlusZoneNum > 0 &&
                             state.afn->AirflowNetworkNodeData(nodeNum1).EPlusZoneNum == iZone)) {
                            state.dataRoomAir->AFNSurfaceCrossVent(SurfNum, iZone) = iMzLink;
                            ++SurfNum;
                        }
                    }
                } // for (Loop2)
            } // for (iZone)

            AuxSurf.deallocate();

            if (any(state.dataRoomAir->IsZoneDispVent3Node) || any(state.dataRoomAir->IsZoneUFAD)) {
                state.dataRoomAir->MaxTempGrad.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->AvgTempGrad.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->TCMF.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->FracMinFlow.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneAirSystemON.allocate(state.dataGlobal->NumOfZones);
                // Allocate histories of displacement ventilation temperatures PH 3/5/04
                state.dataRoomAir->MATFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->XMATFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->DSXMATFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->MATOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->XMATOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->DSXMATOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->MATMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->XMATMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->DSXMATMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZTMFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZTMOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZTMMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->AIRRATFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->AIRRATOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->AIRRATMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZTOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZTMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZTFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->HeightTransition.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Phi.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Zone1Floor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneMXFloor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneM2Floor.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Zone1OC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneMXOC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneM2OC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Zone1MX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneMXMX.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneM2MX.allocate(state.dataGlobal->NumOfZones);

                state.dataRoomAir->MaxTempGrad = 0.0;
                state.dataRoomAir->AvgTempGrad = 0.0;
                state.dataRoomAir->TCMF = 23.0;
                state.dataRoomAir->FracMinFlow = 0.0;
                //      ZoneDVMixedFlagRep    = 0.0
                state.dataRoomAir->ZoneAirSystemON = false;
                //      ZoneDVMixedFlag=0
                state.dataRoomAir->MATFloor = 23.0;
                state.dataRoomAir->XMATFloor = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->DSXMATFloor = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->MATOC = 23.0;
                state.dataRoomAir->XMATOC = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->DSXMATOC = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->MATMX = 23.0;
                state.dataRoomAir->XMATMX = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->DSXMATMX = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->ZTMFloor = {23.0, 23.0, 23.0};
                state.dataRoomAir->ZTMOC = {23.0, 23.0, 23.0};
                state.dataRoomAir->ZTMMX = {23.0, 23.0, 23.0};
                state.dataRoomAir->Zone1Floor = 23.0;
                state.dataRoomAir->ZoneMXFloor = 23.0;
                state.dataRoomAir->ZoneM2Floor = 23.0;
                state.dataRoomAir->Zone1OC = 23.0;
                state.dataRoomAir->ZoneMXOC = 23.0;
                state.dataRoomAir->ZoneM2OC = 23.0;
                state.dataRoomAir->Zone1MX = 23.0;
                state.dataRoomAir->ZoneMXMX = 23.0;
                state.dataRoomAir->ZoneM2MX = 23.0;
                state.dataRoomAir->AIRRATFloor = 0.0;
                state.dataRoomAir->AIRRATOC = 0.0;
                state.dataRoomAir->AIRRATMX = 0.0;
                state.dataRoomAir->ZTOC = 23.0;
                state.dataRoomAir->ZTMX = 23.0;
                state.dataRoomAir->ZTFloor = 23.0;
                state.dataRoomAir->HeightTransition = 0.0;
                state.dataRoomAir->Phi = 0.0;
                state.dataRoomAir->HCeiling = 0.0;
                state.dataRoomAir->HWall = 0.0;
                state.dataRoomAir->HFloor = 0.0;
                state.dataRoomAir->HInternal = 0.0;
                state.dataRoomAir->HWindow = 0.0;
                state.dataRoomAir->HDoor = 0.0;
            }

            if (any(state.dataRoomAir->IsZoneDispVent3Node)) {

                state.dataRoomAir->DispVent3NodeHcIn.allocate(state.dataSurface->TotSurfaces);
                state.dataRoomAir->ZoneDispVent3NodeMixedFlagRep.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneDispVent3NodeMixedFlag.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->DispVent3NodeHcIn = 0.0;
                state.dataRoomAir->ZoneDispVent3NodeMixedFlagRep = 0.0;
                state.dataRoomAir->ZoneDispVent3NodeMixedFlag = 0;
                // Output variables and DV zone flag
                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAirModel::DispVent3Node) {
                        continue; // don't set these up if they don't make sense
                    }
                    // CurrentModuleObject='RoomAirSettings:ThreeNodeDisplacementVentilation'
                    SetupOutputVariable(state,
                                        "Room Air Zone Mixed Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTMX(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Occupied Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTOC(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Floor Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTFloor(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Transition Height",
                                        Constant::Units::m,
                                        state.dataRoomAir->HeightTransition(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Recommended Minimum Flow Fraction",
                                        Constant::Units::None,
                                        state.dataRoomAir->FracMinFlow(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Is Mixed Status",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneDispVent3NodeMixedFlagRep(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Average Temperature Gradient",
                                        Constant::Units::K_m,
                                        state.dataRoomAir->AvgTempGrad(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Maximum Temperature Gradient",
                                        Constant::Units::K_m,
                                        state.dataRoomAir->MaxTempGrad(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Thermal Comfort Effective Air Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->TCMF(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Thermostat Temperature",
                                        Constant::Units::C,
                                        state.dataHeatBalFanSys->TempTstatAir(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(iZone).Name);
                } // for (iZone)
            } // if (any(IsZoneDV))

            if (any(state.dataRoomAir->IsZoneUFAD)) {
                state.dataRoomAir->ZoneUFADMixedFlag.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneUFADMixedFlagRep.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->UFADHcIn.allocate(state.dataSurface->TotSurfaces);
                state.dataRoomAir->ZoneUFADGamma.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneUFADPowInPlumes.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneUFADPowInPlumesfromWindows.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneUFADMixedFlag = 0;
                state.dataRoomAir->ZoneUFADMixedFlagRep = 0.0;
                state.dataRoomAir->UFADHcIn = 0.0;
                state.dataRoomAir->ZoneUFADGamma = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumes = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumesfromWindows = 0.0;

                // Output variables and UF zone flag
                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAirModel::UFADInt) {
                        continue; // don't set these up if they don't make sense
                    }

                    auto &zone = state.dataHeatBal->Zone(iZone);
                    // CurrentModuleObject='RoomAirSettings:UnderFloorAirDistributionInterior'
                    SetupOutputVariable(state,
                                        "Room Air Zone Mixed Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTMX(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Occupied Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTOC(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Transition Height",
                                        Constant::Units::m,
                                        state.dataRoomAir->HeightTransition(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Is Mixed Status",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneUFADMixedFlagRep(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Average Temperature Gradient",
                                        Constant::Units::K_m,
                                        state.dataRoomAir->AvgTempGrad(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Effective Comfort Air Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->TCMF(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Thermostat Temperature",
                                        Constant::Units::C,
                                        state.dataHeatBalFanSys->TempTstatAir(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Transition Height Gamma Value",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneUFADGamma(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Plume Heat Transfer Rate",
                                        Constant::Units::W,
                                        state.dataRoomAir->ZoneUFADPowInPlumes(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Temperature Stratification Fraction",
                                        Constant::Units::None,
                                        state.dataRoomAir->Phi(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);

                    // set zone equip pointer in the UCSDUI data structure
                    state.dataRoomAir->ZoneUFAD(state.dataRoomAir->ZoneUFADPtr(iZone)).ZoneEquipPtr = iZone;
                } // for (iZone)

                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAirModel::UFADExt) {
                        continue; // don't set these up if they don't make sense
                    }
                    // CurrentModuleObject='RoomAirSettings:UnderFloorAirDistributionExterior'
                    auto const &zone = state.dataHeatBal->Zone(iZone);
                    SetupOutputVariable(state,
                                        "Room Air Zone Mixed Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTMX(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Occupied Subzone Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTOC(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Transition Height",
                                        Constant::Units::m,
                                        state.dataRoomAir->HeightTransition(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Is Mixed Status",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneUFADMixedFlagRep(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Average Temperature Gradient",
                                        Constant::Units::K_m,
                                        state.dataRoomAir->AvgTempGrad(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Effective Comfort Air Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->TCMF(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Thermostat Temperature",
                                        Constant::Units::C,
                                        state.dataHeatBalFanSys->TempTstatAir(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Transition Height Gamma Value",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneUFADGamma(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Plume Heat Transfer Rate",
                                        Constant::Units::W,
                                        state.dataRoomAir->ZoneUFADPowInPlumes(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Window Plume Heat Transfer Rate",
                                        Constant::Units::W,
                                        state.dataRoomAir->ZoneUFADPowInPlumesfromWindows(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Temperature Stratification Fraction",
                                        Constant::Units::None,
                                        state.dataRoomAir->Phi(iZone),
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    // set zone equip pointer in the UCSDUE data structure
                    state.dataRoomAir->ZoneUFAD(state.dataRoomAir->ZoneUFADPtr(iZone)).ZoneEquipPtr = iZone;
                }
            }

            if (any(state.dataRoomAir->IsZoneCrossVent)) {
                state.dataRoomAir->CrossVentHcIn.allocate(state.dataSurface->TotSurfaces);
                state.dataRoomAir->ZTJET.allocate(state.dataGlobal->NumOfZones);
                // Most ZTJet takes defaults
                state.dataRoomAir->ZTREC.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->RoomOutflowTemp.allocate(state.dataGlobal->NumOfZones);
                // Most ZTREC takes defaults
                state.dataRoomAir->JetRecAreaRatio.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Urec.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Ujet.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Qrec.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Qtot.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->RecInflowRatio.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Uhc.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Ain.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Tin.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Droom.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Dstar.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneCrossVentIsMixing.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->Rfr.allocate(state.dataGlobal->NumOfZones);
                state.dataRoomAir->ZoneCrossVentHasREC.allocate(state.dataGlobal->NumOfZones);

                state.dataRoomAir->ZTJET = 23.0;
                state.dataRoomAir->RoomOutflowTemp = 23.0;
                state.dataRoomAir->ZTREC = 23.0;
                state.dataRoomAir->CrossVentHcIn = 0.0;
                state.dataRoomAir->JetRecAreaRatio = 0.2;
                state.dataRoomAir->Urec = 0.2;
                state.dataRoomAir->Ujet = 0.2;
                state.dataRoomAir->Qrec = 0.2;
                state.dataRoomAir->Uhc = 0.2;
                state.dataRoomAir->Ain = 1.0;
                state.dataRoomAir->Tin = 23.0;
                state.dataRoomAir->Droom = 6.0;
                state.dataRoomAir->ZoneCrossVentIsMixing = 0.0;
                state.dataRoomAir->Rfr = 10.0;
                state.dataRoomAir->ZoneCrossVentHasREC = 1.0;
                state.dataRoomAir->HCeiling = 0.0;
                state.dataRoomAir->HWall = 0.0;
                state.dataRoomAir->HFloor = 0.0;
                state.dataRoomAir->HInternal = 0.0;
                state.dataRoomAir->HWindow = 0.0;
                state.dataRoomAir->HDoor = 0.0;

                for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                    if (state.dataRoomAir->AirModel(iZone).AirModel != RoomAirModel::CrossVent) {
                        continue; // don't set these up if they don't make sense
                    }

                    int ZoneEquipConfigNum = ZoneNum; // Where does this ZoneNum come from?

                    auto const &zone = state.dataHeatBal->Zone(iZone);
                    // check whether this zone is a controlled zone or not
                    if (state.dataZoneEquip->ZoneEquipConfig(ZoneEquipConfigNum).IsControlled) {
                        state.dataRoomAir->IsZoneCrossVent(iZone) = false;
                        state.dataRoomAir->AirModel(iZone).SimAirModel = false;
                        ShowSevereError(state, format("Unmixed Flow: Cross Ventilation cannot be applied for Zone={}", zone.Name));
                        ShowContinueError(
                            state, format("An HVAC system is present in the zone. Fully mixed airflow model will be used for Zone={}", zone.Name));
                        continue;
                    }
                    // CurrentModuleObject='RoomAirSettings:CrossVentilation'
                    SetupOutputVariable(state,
                                        "Room Air Zone Jet Region Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTJET(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Recirculation Region Temperature",
                                        Constant::Units::C,
                                        state.dataRoomAir->ZTREC(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Jet Region Average Air Velocity",
                                        Constant::Units::m_s,
                                        state.dataRoomAir->Ujet(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Recirculation Region Average Air Velocity",
                                        Constant::Units::m_s,
                                        state.dataRoomAir->Urec(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Recirculation and Inflow Rate Ratio",
                                        Constant::Units::None,
                                        state.dataRoomAir->RecInflowRatio(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Inflow Opening Area",
                                        Constant::Units::m2,
                                        state.dataRoomAir->Ain(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Room Length",
                                        Constant::Units::m,
                                        state.dataRoomAir->Dstar(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Is Mixing Status",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneCrossVentIsMixing(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    SetupOutputVariable(state,
                                        "Room Air Zone Is Recirculating Status",
                                        Constant::Units::None,
                                        state.dataRoomAir->ZoneCrossVentHasREC(iZone),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        zone.Name);
                    for (int i = 1; i <= state.dataRoomAir->AFNSurfaceCrossVent(0, ZoneNum); ++i) {
                        int N = state.afn->AirflowNetworkLinkageData(i).CompNum;
                        if (state.afn->AirflowNetworkCompData(N).CompTypeNum == AirflowNetwork::iComponentTypeNum::DOP) {
                            SetupOutputVariable(state,
                                                "Room Air Window Jet Region Average Air Velocity",
                                                Constant::Units::m_s,
                                                state.dataRoomAir->CrossVentJetRecFlows(i, iZone).Ujet,
                                                OutputProcessor::TimeStepType::Zone,
                                                OutputProcessor::StoreType::Average,
                                                state.afn->MultizoneSurfaceData(i).SurfName);
                        }
                    }
                } // for (iZone)
            } // if (any(isZoneCV))

            state.dataRoomAir->MyEnvrnFlag = true;

            state.dataRoomAir->MyOneTimeFlag = false;
        } // if (myOneTimeFlag)

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataRoomAir->MyEnvrnFlag(ZoneNum)) {

            if (state.dataRoomAir->IsZoneDispVent3Node(ZoneNum) || state.dataRoomAir->IsZoneUFAD(ZoneNum)) {

                state.dataRoomAir->MaxTempGrad(ZoneNum) = 0.0;
                state.dataRoomAir->AvgTempGrad(ZoneNum) = 0.0;
                state.dataRoomAir->TCMF(ZoneNum) = 23.0;
                state.dataRoomAir->FracMinFlow(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneAirSystemON(ZoneNum) = false;
                state.dataRoomAir->MATFloor(ZoneNum) = 23.0;
                state.dataRoomAir->XMATFloor(ZoneNum) = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->DSXMATFloor(ZoneNum) = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->MATOC(ZoneNum) = 23.0;
                state.dataRoomAir->XMATOC(ZoneNum) = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->DSXMATOC(ZoneNum) = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->MATMX(ZoneNum) = 23.0;
                state.dataRoomAir->XMATMX(ZoneNum) = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->DSXMATMX(ZoneNum) = {23.0, 23.0, 23.0, 23.0};
                state.dataRoomAir->ZTMFloor(ZoneNum) = {23.0, 23.0, 23.0};
                state.dataRoomAir->Zone1Floor(ZoneNum) = 23.0;
                state.dataRoomAir->ZoneMXFloor(ZoneNum) = 23.0;
                state.dataRoomAir->ZoneM2Floor(ZoneNum) = 23.0;
                state.dataRoomAir->ZTMOC(ZoneNum) = {23.0, 23.0, 23.0};
                state.dataRoomAir->Zone1OC(ZoneNum) = 23.0;
                state.dataRoomAir->ZoneMXOC(ZoneNum) = 23.0;
                state.dataRoomAir->ZoneM2OC(ZoneNum) = 23.0;
                state.dataRoomAir->ZTMMX(ZoneNum) = {23.0, 23.0, 23.0};
                state.dataRoomAir->Zone1MX(ZoneNum) = 23.0;
                state.dataRoomAir->ZoneMXMX(ZoneNum) = 23.0;
                state.dataRoomAir->ZoneM2MX(ZoneNum) = 23.0;
                state.dataRoomAir->AIRRATFloor(ZoneNum) = 0.0;
                state.dataRoomAir->AIRRATOC(ZoneNum) = 0.0;
                state.dataRoomAir->AIRRATMX(ZoneNum) = 0.0;
                state.dataRoomAir->ZTOC(ZoneNum) = 23.0;
                state.dataRoomAir->ZTMX(ZoneNum) = 23.0;
                state.dataRoomAir->ZTFloor(ZoneNum) = 23.0;
                state.dataRoomAir->HeightTransition(ZoneNum) = 0.0;
                state.dataRoomAir->Phi(ZoneNum) = 0.0;
                state.dataRoomAir->HCeiling = 0.0;
                state.dataRoomAir->HWall = 0.0;
                state.dataRoomAir->HFloor = 0.0;
                state.dataRoomAir->HInternal = 0.0;
                state.dataRoomAir->HWindow = 0.0;
                state.dataRoomAir->HDoor = 0.0;
            }

            if (state.dataRoomAir->IsZoneDispVent3Node(ZoneNum)) {

                state.dataRoomAir->DispVent3NodeHcIn = 0.0;
                state.dataRoomAir->ZoneDispVent3NodeMixedFlagRep(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneDispVent3NodeMixedFlag(ZoneNum) = 0;
            }

            if (state.dataRoomAir->IsZoneUFAD(ZoneNum)) {

                state.dataRoomAir->UFADHcIn = 0.0;
                state.dataRoomAir->ZoneUFADMixedFlag(ZoneNum) = 0;
                state.dataRoomAir->ZoneUFADMixedFlagRep(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneUFADGamma(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumes(ZoneNum) = 0.0;
                state.dataRoomAir->ZoneUFADPowInPlumesfromWindows(ZoneNum) = 0.0;
            }

            if (state.dataRoomAir->IsZoneCrossVent(ZoneNum)) {
                state.dataRoomAir->ZTJET(ZoneNum) = 23.0;
                state.dataRoomAir->RoomOutflowTemp(ZoneNum) = 23.0;
                state.dataRoomAir->ZTREC(ZoneNum) = 23.0;
                state.dataRoomAir->CrossVentHcIn = 0.0;
                state.dataRoomAir->JetRecAreaRatio(ZoneNum) = 0.2;
                state.dataRoomAir->Urec(ZoneNum) = 0.2;
                state.dataRoomAir->Ujet(ZoneNum) = 0.2;
                state.dataRoomAir->Uhc(ZoneNum) = 0.2;
                state.dataRoomAir->Ain(ZoneNum) = 1.0;
                state.dataRoomAir->Tin(ZoneNum) = 23.0;
                state.dataRoomAir->Droom(ZoneNum) = 6.0;
                state.dataRoomAir->Dstar(ZoneNum) = 6.0;
                state.dataRoomAir->ZoneCrossVentIsMixing(ZoneNum) = 0.0;
                state.dataRoomAir->Rfr(ZoneNum) = 10.0;
                state.dataRoomAir->ZoneCrossVentHasREC(ZoneNum) = 1.0;
                state.dataRoomAir->HCeiling = 0.0;
                state.dataRoomAir->HWall = 0.0;
                state.dataRoomAir->HFloor = 0.0;
                state.dataRoomAir->HInternal = 0.0;
                state.dataRoomAir->HWindow = 0.0;
                state.dataRoomAir->HDoor = 0.0;
            }

            state.dataRoomAir->MyEnvrnFlag(ZoneNum) = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataRoomAir->MyEnvrnFlag(ZoneNum) = true;
        }
    }

    void GetRAFNNodeNum(EnergyPlusData &state,
                        std::string const &RAFNNodeName, // Name of RoomAir:Node:AirflowNetwork
                        int &ZoneNum,                    // The zone number associate with the node name
                        int &RAFNNodeNum,                // RoomAir:Node:AirflowNetwork Number
                        bool &Errorfound                 // true if an error is found (TODO: Useless, RAFNodeNum is 0 when Errorfound is true)
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   November 2014

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given RoomAirNode name and returns the Zone number and RoomAir node
        // number. If incorrect name is given, errorsfound is returned as true and value is returned
        // as zero.

        // Obtains and Allocates RoomAirSettings : AirflowNetwork
        if (state.dataRoomAir->GetAirModelData) {
            GetAirModelDatas(state);
            state.dataRoomAir->GetAirModelData = false;
        }

        Errorfound = false;
        RAFNNodeNum = 0;
        for (int I = 1; I <= state.dataGlobal->NumOfZones; ++I) {
            auto const &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(I);
            if (afnZoneInfo.NumOfAirNodes > 0) {
                RAFNNodeNum = Util::FindItemInList(RAFNNodeName, afnZoneInfo.Node, afnZoneInfo.NumOfAirNodes);
                if (RAFNNodeNum > 0) {
                    ZoneNum = I;
                    break;
                }
            }
        }

        if (RAFNNodeNum == 0) {
            Errorfound = true;
            ShowSevereError(state,
                            format("Could not find RoomAir:Node:AirflowNetwork number with AirflowNetwork:IntraZone:Node Name='{}", RAFNNodeName));
        }
    }

    bool CheckEquipName(EnergyPlusData &state,
                        std::string const &EquipName, // Equipment Name
                        std::string &SupplyNodeName,  // Supply node name
                        std::string &ReturnNodeName,  // Return node name
                        DataZoneEquipment::ZoneEquipType zoneEquipType)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2014

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given RoomAirNode name and returns the Zone number and RoomAir node
        // number.If incorrect name is given, errorsfound is returned as true and value is returned
        // as zero.

        // Return value
        bool EquipFind = false; // if true, equip is found

        int SupplyNodeNum = 0;
        int ReturnNodeNum = 0;

        SupplyNodeName = "";
        int EquipIndex;

        if (zoneEquipType == DataZoneEquipment::ZoneEquipType::Invalid) {
            return EquipFind;
        }

        switch (zoneEquipType) {
        case DataZoneEquipment::ZoneEquipType::VariableRefrigerantFlowTerminal: { // ZoneHVAC:TerminalUnit : VariableRefrigerantFlow
            EquipIndex = HVACVariableRefrigerantFlow::getEqIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataHVACVarRefFlow->VRFTU(EquipIndex).VRFTUOutletNodeNum;
        } break;
        case DataZoneEquipment::ZoneEquipType::EnergyRecoveryVentilator: { // ZoneHVAC : EnergyRecoveryVentilator
            EquipIndex = HVACStandAloneERV::getEqIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataHVACStandAloneERV->StandAloneERV(EquipIndex).SupplyAirInletNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::FourPipeFanCoil: { // ZoneHVAC : FourPipeFanCoil
            EquipIndex = FanCoilUnits::getEqIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataFanCoilUnits->FanCoil(EquipIndex).AirOutNode;
            ReturnNodeNum = state.dataFanCoilUnits->FanCoil(EquipIndex).AirInNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::OutdoorAirUnit: { // ZoneHVAC : OutdoorAirUnit
            EquipIndex = OutdoorAirUnit::getOutdoorAirUnitEqIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataOutdoorAirUnit->OutAirUnit(EquipIndex).AirOutletNode;
            ReturnNodeNum = state.dataOutdoorAirUnit->OutAirUnit(EquipIndex).AirInletNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::PackagedTerminalAirConditioner: { // ZoneHVAC : PackagedTerminalAirConditioner
            EquipIndex = UnitarySystems::getZoneEqIndex(state, EquipName, zoneEquipType);
            if (EquipIndex == -1) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataUnitarySystems->unitarySys[EquipIndex].AirOutNode;
            ReturnNodeNum = state.dataUnitarySystems->unitarySys[EquipIndex].AirInNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPump: { // ZoneHVAC : PackagedTerminalHeatPump
            EquipIndex = UnitarySystems::getZoneEqIndex(state, EquipName, zoneEquipType);
            if (EquipIndex == -1) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataUnitarySystems->unitarySys[EquipIndex].AirOutNode;
            ReturnNodeNum = state.dataUnitarySystems->unitarySys[EquipIndex].AirInNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::UnitHeater: { // ZoneHVAC : UnitHeater
            EquipIndex = UnitHeater::getUnitHeaterIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataUnitHeaters->UnitHeat(EquipIndex).AirInNode;
            SupplyNodeNum = state.dataUnitHeaters->UnitHeat(EquipIndex).AirOutNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::UnitVentilator: { // ZoneHVAC : UnitVentilator
            EquipIndex = UnitVentilator::getUnitVentilatorIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataUnitVentilators->UnitVent(EquipIndex).AirInNode;
            SupplyNodeNum = state.dataUnitVentilators->UnitVent(EquipIndex).AirOutNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::VentilatedSlab: { // ZoneHVAC : VentilatedSlab
            EquipIndex = VentilatedSlab::getVentilatedSlabIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataVentilatedSlab->VentSlab(EquipIndex).ReturnAirNode;
            SupplyNodeNum = state.dataVentilatedSlab->VentSlab(EquipIndex).ZoneAirInNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPumpWaterToAir: { // ZoneHVAC : WaterToAirHeatPump
            EquipIndex = UnitarySystems::getZoneEqIndex(state, EquipName, zoneEquipType);
            if (EquipIndex == -1) {
                return EquipFind;
            }
            SupplyNodeNum = state.dataUnitarySystems->unitarySys[EquipIndex].AirOutNode;
            ReturnNodeNum = state.dataUnitarySystems->unitarySys[EquipIndex].AirInNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::WindowAirConditioner: { // ZoneHVAC : WindowAirConditioner
            EquipIndex = WindowAC::getWindowACIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataWindowAC->WindAC(EquipIndex).AirInNode;
            SupplyNodeNum = state.dataWindowAC->WindAC(EquipIndex).AirOutNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::BaseboardElectric: { // ZoneHVAC : Baseboard : RadiantConvective : Electric
                                                                    // convective equipment without node connection. Will handle later
            SupplyNodeName = "";                                    // convection only
        } break;
        case DataZoneEquipment::ZoneEquipType::BaseboardWater: { // ZoneHVAC : Baseboard : RadiantConvective : Water
                                                                 // convective equipment without node connection. Will handle later
            SupplyNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::BaseboardSteam: { // ZoneHVAC : Baseboard : RadiantConvective : Steam
                                                                 // convective equipment without node connection. Will handle later
            SupplyNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveElectric: { // ZoneHVAC : Baseboard : Convective : Electric
                                                                              // convective equipment without node connection. Will handle later
            SupplyNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveWater: { // ZoneHVAC : Baseboard : Convective : Water
                                                                           // convective equipment without node connection. Will handle later
            SupplyNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::HighTemperatureRadiant: { // ZoneHVAC : HighTemperatureRadiant
                                                                         // Radiative equipment without node connection. Will handle later
            SupplyNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::DehumidifierDX: { // ZoneHVAC : Dehumidifier : DX
            EquipIndex = ZoneDehumidifier::getZoneDehumidifierIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataZoneDehumidifier->ZoneDehumid(EquipIndex).AirInletNodeNum;
            SupplyNodeNum = state.dataZoneDehumidifier->ZoneDehumid(EquipIndex).AirOutletNodeNum;
        } break;
        case DataZoneEquipment::ZoneEquipType::PurchasedAir: { // ZoneHVAC : IdealLoadsAirSystem
            EquipIndex = PurchasedAirManager::getPurchasedAirIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataPurchasedAirMgr->PurchAir(EquipIndex).ZoneExhaustAirNodeNum;
            SupplyNodeNum = state.dataPurchasedAirMgr->PurchAir(EquipIndex).ZoneSupplyAirNodeNum;
        } break;
        case DataZoneEquipment::ZoneEquipType::RefrigerationChillerSet: { // ZoneHVAC : RefrigerationChillerSet
            // May not apply
            // SupplyNodeName = Alphas(5);
            // ReturnNodeName = Alphas(4);
        } break;
        case DataZoneEquipment::ZoneEquipType::HybridEvaporativeCooler: { // ZoneHVAC : HybridUnitaryAirConditioners
            EquipIndex = HybridUnitaryAirConditioners::getHybridUnitaryACIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(EquipIndex).InletNode;
            SupplyNodeNum = state.dataHybridUnitaryAC->ZoneHybridUnitaryAirConditioner(EquipIndex).OutletNode;
        } break;
        case DataZoneEquipment::ZoneEquipType::ExhaustFan: { // Fan : ZoneExhaust

            //            SupplyNodeName = "";                             // ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? May not use
        } break;
        case DataZoneEquipment::ZoneEquipType::HeatPumpWaterHeaterPumpedCondenser:
        case DataZoneEquipment::ZoneEquipType::HeatPumpWaterHeaterWrappedCondenser: { // WaterHeater : HeatPump
            EquipIndex = WaterThermalTanks::getHeatPumpWaterHeaterIndex(state, EquipName);
            if (EquipIndex == 0) {
                return EquipFind;
            }
            ReturnNodeNum = state.dataWaterThermalTanks->HPWaterHeater(EquipIndex).HeatPumpAirInletNode;
            SupplyNodeNum = state.dataWaterThermalTanks->HPWaterHeater(EquipIndex).HeatPumpAirOutletNode;
            // For AirTerminals, find matching return node later
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalDualDuctConstantVolume: { // AirTerminal : DualDuct : ConstantVolume
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalDualDuctVAV: { // AirTerminal : DualDuct : VAV
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctConstantVolumeReheat: { // AirTerminal : SingleDuct : ConstantVolume : Reheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctConstantVolumeNoReheat: { // AirTerminal : SingleDuct :
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(4);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctVAVReheat: { // AirTerminal : SingleDuct : VAV : Reheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctVAVNoReheat: { // AirTerminal : SingleDuct : VAV : NoReheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctSeriesPIUReheat: { // AirTerminal : SingleDuct : SeriesPIU : Reheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctParallelPIUReheat: { // AirTerminal : SingleDuct : ParallelPIU : Reheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctCAVFourPipeInduction: { // AirTerminal : SingleDuct :
            // Air teminal components are handled later
            // ConstantVolume : FourPipeInduction
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctVAVReheatVariableSpeedFan: { // AirTerminal : SingleDuct : VAV
            // Air teminal components are handled later
            // : Reheat : VariableSpeedFan
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctVAVHeatAndCoolReheat: { // AirTerminal : SingleDuct : VAV :
                                                                                            // HeatAndCool : Reheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctVAVHeatAndCoolNoReheat: { // AirTerminal : SingleDuct : VAV :
                                                                                              // HeatAndCool : NoReheat
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(1);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalSingleDuctConstantVolumeCooledBeam: { // AirTerminal : SingleDuct :
                                                                                                // ConstantVolume : CooledBeam
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(5);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirTerminalDualDuctVAVOutdoorAir: { // AirTerminal : DualDuct : VAV : OutdoorAir
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(3);
            // ReturnNodeName = "";
        } break;
        case DataZoneEquipment::ZoneEquipType::AirLoopHVACReturnAir: { // AirLoopHVACReturnAir
            // Air teminal components are handled later
            // SupplyNodeName = Alphas(4);                                //
            // ReturnNodeName = "";                                       //
        } break;
        default: {
            assert(false);
        } break;

        } // switch

        if (SupplyNodeNum > 0) {
            SupplyNodeName = state.dataLoopNodes->NodeID(SupplyNodeNum);
            EquipFind = true;
        }
        if (ReturnNodeNum > 0) {
            ReturnNodeName = state.dataLoopNodes->NodeID(ReturnNodeNum);
        } else {
            ReturnNodeName = "";
        }
        return EquipFind;
    }

    //*****************************************************************************************

} // namespace RoomAir

} // namespace EnergyPlus
