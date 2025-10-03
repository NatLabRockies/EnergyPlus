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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VentilatedSlab.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace VentilatedSlab {

    // Module containing the routines dealing with the Ventilated Slab

    // MODULE INFORMATION:
    //       AUTHOR         Young Tae Chae, Rick Strand
    //       DATE WRITTEN   June 2008

    // PURPOSE OF THIS MODULE:
    // Simulate Ventilated Slab Systems.

    // METHODOLOGY EMPLOYED:
    // Systems are modeled as a collection of components: radiant panel, outside air mixer,
    // fan, heating coil and/or cooling coil plus an integrated control
    // algorithm that adjusts the hot or cold water flow to meet the setpoint
    // condition.  Outside air mixing is handled locally as either fixed percent
    // or as attempting to meet a prescribed mixed air temperature.

    // REFERENCES:
    // ASHRAE Systems and Equipment Handbook (SI), 1996. pp. 31.1-31.3
    // Fred Buhl's fan coil module (FanCoilUnits.cc)

    std::string const cMO_VentilatedSlab = "ZoneHVAC:VentilatedSlab";

    //    int constexpr NotOperating = 0; // Parameter for use with OperatingMode variable, set for no heating/cooling
    int constexpr HeatingMode = 1; // Parameter for use with OperatingMode variable, set for heating
    int constexpr CoolingMode = 2; // Parameter for use with OperatingMode variable, set for cooling

    void SimVentilatedSlab(EnergyPlusData &state,
                           std::string const &CompName,   // name of the fan coil unit
                           int const ZoneNum,             // number of zone being served
                           bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                           Real64 &PowerMet,              // Sensible power supplied (W)
                           Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                           int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)
        //       RE-ENGINEERED  Rick Strand and Young T. Chae for Ventilated Slab (June, 2008)

        // PURPOSE OF THIS SUBROUTINE:
        // This is the main driver subroutine for the Ventilated Slab simulation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item; // index of ventilated slab being simulated

        if (state.dataVentilatedSlab->GetInputFlag) {
            GetVentilatedSlabInput(state);
            state.dataVentilatedSlab->GetInputFlag = false;
        }

        // Find the correct VentilatedSlabInput
        if (CompIndex == 0) {
            Item = Util::FindItemInList(CompName, state.dataVentilatedSlab->VentSlab);
            if (Item == 0) {
                ShowFatalError(state, format("SimVentilatedSlab: system not found={}", CompName));
            }
            CompIndex = Item;
        } else {
            Item = CompIndex;
            if (Item > state.dataVentilatedSlab->NumOfVentSlabs || Item < 1) {
                ShowFatalError(state,
                               format("SimVentilatedSlab:  Invalid CompIndex passed={}, Number of Systems={}, Entered System name={}",
                                      Item,
                                      state.dataVentilatedSlab->NumOfVentSlabs,
                                      CompName));
            }
            if (state.dataVentilatedSlab->CheckEquipName(Item)) {
                if (CompName != state.dataVentilatedSlab->VentSlab(Item).Name) {
                    ShowFatalError(state,
                                   format("SimVentilatedSlab: Invalid CompIndex passed={}, System name={}, stored System Name for that index={}",
                                          Item,
                                          CompName,
                                          state.dataVentilatedSlab->VentSlab(Item).Name));
                }
                state.dataVentilatedSlab->CheckEquipName(Item) = false;
            }
        }

        state.dataSize->ZoneEqVentedSlab = true;

        InitVentilatedSlab(state, Item, ZoneNum, FirstHVACIteration);

        CalcVentilatedSlab(state, Item, ZoneNum, FirstHVACIteration, PowerMet, LatOutputProvided);

        UpdateVentilatedSlab(state, Item, FirstHVACIteration);

        ReportVentilatedSlab(state, Item);

        state.dataSize->ZoneEqVentedSlab = false;
    }

    void GetVentilatedSlabInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine obtains the input for ventilated slab and sets
        // up the appropriate derived type.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // REFERENCES:
        // Fred Buhl's fan coil module (FanCoilUnits.cc)
        // Kwang Ho Lee's Unit Ventilator Module (UnitVentilator.cc)
        // Rick Strand's Low temperature Radiant system (RadiantSystemLowTemp.cc)

        static constexpr std::string_view routineName = "GetVentilatedSlabInput";
        // Using/Aliasing
        auto &GetWaterCoilMaxFlowRate(WaterCoils::GetCoilMaxWaterFlowRate);
        auto &GetSteamCoilMaxFlowRate(SteamCoils::GetCoilMaxWaterFlowRate);
        auto &GetHXAssistedCoilFlowRate(HVACHXAssistedCoolingCoil::GetCoilMaxWaterFlowRate);

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr std::array<std::string_view, static_cast<int>(VentilatedSlabConfig::Num)> VentilatedSlabConfigNamesUC{
            "SLABONLY", "SLABANDZONE", "SERIESSLABS"};

        constexpr std::array<std::string_view, static_cast<int>(ControlType::Num)> ControlTypeNamesUC{
            "MEANAIRTEMPERATURE",
            "MEANRADIANTTEMPERATURE",
            "OPERATIVETEMPERATURE",
            "OUTDOORDRYBULBTEMPERATURE",
            "OUTDOORWETBULBTEMPERATURE",
            "SURFACETEMPERATURE",
            "ZONEAIRDEWPOINTTEMPERATURE",
        };

        static std::string const CurrentModuleObject("ZoneHVAC:VentilatedSlab");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false);       // Set to true if errors in input, fatal at end of routine
        int IOStatus;                  // Used in GetObjectItem
        bool IsNotOK;                  // TRUE if there was a problem with a list name
        int NumAlphas;                 // Number of Alphas for each GetObjectItem call
        int NumArgs;                   // Unused variable that is part of a subroutine call
        int NumNumbers;                // Number of Numbers for each GetObjectItem call
        int Item;                      // Item to be "gotten"
        int BaseNum;                   // Temporary number for creating RadiantSystemTypes structure
        bool errFlag;                  // interim error flag
        int SurfNum;                   // DO loop counter for surfaces
        bool IsValid;                  // Set for outside air node check
        Array1D_string cAlphaArgs;     // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> rNumericArgs;  // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        bool SteamMessageNeeded;

        constexpr std::array<std::string_view, static_cast<int>(OutsideAirControlType::Num)> OutsideAirControlTypeNamesUC{
            "VARIABLEPERCENT", "FIXEDTEMPERATURE", "FIXEDAMOUNT"};
        constexpr std::array<std::string_view, static_cast<int>(CoilType::Num)> CoilTypeNamesUC{"NONE", "HEATING", "COOLING", "HEATINGANDCOOLING"};

        constexpr std::array<std::string_view, static_cast<int>(HeatingCoilType::Num)> HeatingCoilTypeNamesUC{
            "COIL:HEATING:ELECTRIC", "COIL:HEATING:FUEL", "COIL:HEATING:WATER", "COIL:HEATING:STEAM"};
        constexpr std::array<std::string_view, static_cast<int>(CoolingCoilType::Num)> CoolingCoilTypeNamesUC{
            "COIL:COOLING:WATER", "COIL:COOLING:WATER:DETAILEDGEOMETRY", "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED"};

        // Figure out how many Ventilated Slab Systems there are in the input file

        SteamMessageNeeded = true;
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumArgs, NumAlphas, NumNumbers);
        cAlphaArgs.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        rNumericArgs.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // make sure data is gotten for surface lists
        BaseNum = DataSurfaceLists::GetNumberOfSurfListVentSlab(state);

        state.dataVentilatedSlab->NumOfVentSlabs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        // Allocate the local derived type and do one-time initializations for all parts of it

        state.dataVentilatedSlab->VentSlab.allocate(state.dataVentilatedSlab->NumOfVentSlabs);
        state.dataVentilatedSlab->CheckEquipName.dimension(state.dataVentilatedSlab->NumOfVentSlabs, true);
        state.dataVentilatedSlab->VentSlabNumericFields.allocate(state.dataVentilatedSlab->NumOfVentSlabs);

        for (Item = 1; Item <= state.dataVentilatedSlab->NumOfVentSlabs;
             ++Item) { // Begin looping over the entire ventilated slab systems found in the input file...

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            state.dataVentilatedSlab->VentSlabNumericFields(Item).FieldNames.allocate(NumNumbers);
            state.dataVentilatedSlab->VentSlabNumericFields(Item).FieldNames = cNumericFields;
            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), CurrentModuleObject, ErrorsFound);
            auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

            ventSlab.Name = state.dataIPShortCut->cAlphaArgs(1);
            if (lAlphaBlanks(2)) {
                ventSlab.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((ventSlab.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }

            ventSlab.ZonePtr = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataHeatBal->Zone);
            if (ventSlab.ZonePtr == 0) {
                if (lAlphaBlanks(3)) {
                    ShowSevereError(
                        state, format(R"({}="{}" invalid {} is required but input is blank.)", CurrentModuleObject, ventSlab.Name, cAlphaFields(3)));
                } else {
                    ShowSevereError(state,
                                    format(R"({}="{}" invalid {}="{}" not found.)",
                                           CurrentModuleObject,
                                           ventSlab.Name,
                                           cAlphaFields(3),
                                           state.dataIPShortCut->cAlphaArgs(3)));
                }
                ErrorsFound = true;
            }

            ventSlab.SurfListName = state.dataIPShortCut->cAlphaArgs(4);
            int SurfListNum = 0;
            //    IF (NumOfSlabLists > 0) SurfListNum = Util::FindItemInList(VentSlab(Item)%SurfListName, SlabList%Name, NumOfSlabLists)
            if (state.dataSurfLists->NumOfSurfListVentSlab > 0) {
                SurfListNum = Util::FindItemInList(ventSlab.SurfListName, state.dataSurfLists->SlabList);
            }
            if (SurfListNum > 0) { // Found a valid surface list
                ventSlab.NumOfSurfaces = state.dataSurfLists->SlabList(SurfListNum).NumOfSurfaces;
                ventSlab.ZName.allocate(ventSlab.NumOfSurfaces);
                ventSlab.ZPtr.allocate(ventSlab.NumOfSurfaces);
                ventSlab.SurfaceName.allocate(ventSlab.NumOfSurfaces);
                ventSlab.SurfacePtr.allocate(ventSlab.NumOfSurfaces);
                ventSlab.CDiameter.allocate(ventSlab.NumOfSurfaces);
                ventSlab.CLength.allocate(ventSlab.NumOfSurfaces);
                ventSlab.CNumbers.allocate(ventSlab.NumOfSurfaces);
                ventSlab.SlabIn.allocate(ventSlab.NumOfSurfaces);
                ventSlab.SlabOut.allocate(ventSlab.NumOfSurfaces);

                state.dataVentilatedSlab->MaxCloNumOfSurfaces = max(state.dataVentilatedSlab->MaxCloNumOfSurfaces, ventSlab.NumOfSurfaces);
                for (SurfNum = 1; SurfNum <= state.dataSurfLists->SlabList(SurfListNum).NumOfSurfaces; ++SurfNum) {
                    ventSlab.ZName(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).ZoneName(SurfNum);
                    ventSlab.ZPtr(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).ZonePtr(SurfNum);
                    ventSlab.SurfaceName(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).SurfName(SurfNum);
                    ventSlab.SurfacePtr(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).SurfPtr(SurfNum);
                    ventSlab.CDiameter(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).CoreDiameter(SurfNum);
                    ventSlab.CLength(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).CoreLength(SurfNum);
                    ventSlab.CNumbers(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).CoreNumbers(SurfNum);
                    ventSlab.SlabIn(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).SlabInNodeName(SurfNum);
                    ventSlab.SlabOut(SurfNum) = state.dataSurfLists->SlabList(SurfListNum).SlabOutNodeName(SurfNum);
                    if (ventSlab.SurfacePtr(SurfNum) != 0) {
                        state.dataSurface->surfIntConv(ventSlab.SurfacePtr(SurfNum)).hasActiveInIt = true;
                    }
                }

            } else { // User entered a single surface name rather than a surface list
                ventSlab.NumOfSurfaces = 1;
                ventSlab.SurfacePtr.allocate(ventSlab.NumOfSurfaces);
                ventSlab.SurfaceName.allocate(ventSlab.NumOfSurfaces);
                ventSlab.SurfaceFlowFrac.allocate(ventSlab.NumOfSurfaces);
                state.dataVentilatedSlab->MaxCloNumOfSurfaces = max(state.dataVentilatedSlab->MaxCloNumOfSurfaces, ventSlab.NumOfSurfaces);
                ventSlab.SurfaceName(1) = ventSlab.SurfListName;
                ventSlab.SurfacePtr(1) = Util::FindItemInList(ventSlab.SurfaceName(1), state.dataSurface->Surface);
                ventSlab.SurfaceFlowFrac(1) = 1.0;
                // Error checking for single surfaces
                if (ventSlab.SurfacePtr(1) == 0) {
                    ShowSevereError(state,
                                    format(R"({}="{}" invalid {}="{}" not found.)",
                                           CurrentModuleObject,
                                           ventSlab.Name,
                                           cAlphaFields(4),
                                           state.dataIPShortCut->cAlphaArgs(4)));
                    ErrorsFound = true;
                } else if (state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(ventSlab.SurfacePtr(1))) {
                    ShowSevereError(state, format("{}=\"{}\", invalid Surface", CurrentModuleObject, ventSlab.Name));
                    ShowContinueError(state,
                                      format("{}=\"{}\" has been used in another radiant system or ventilated slab.",
                                             cAlphaFields(4),
                                             state.dataIPShortCut->cAlphaArgs(4)));
                    ErrorsFound = true;
                }
                if (ventSlab.SurfacePtr(1) != 0) {
                    state.dataSurface->surfIntConv(ventSlab.SurfacePtr(1)).hasActiveInIt = true;
                    state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(ventSlab.SurfacePtr(1)) = true;
                }
            }

            // Error checking for zones and construction information

            if (SurfListNum > 0) {

                for (SurfNum = 1; SurfNum <= ventSlab.NumOfSurfaces; ++SurfNum) {

                    int const ConstrNum = state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Construction;
                    auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                    if (ventSlab.SurfacePtr(SurfNum) == 0) {
                        continue; // invalid surface -- detected earlier
                    }
                    if (ventSlab.ZPtr(SurfNum) == 0) {
                        continue; // invalid zone -- detected earlier
                    }
                    if (state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Construction == 0) {
                        continue; // invalid construction, detected earlier
                    }
                    if (!thisConstruct.SourceSinkPresent) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid surface=\"{}\".",
                                               CurrentModuleObject,
                                               ventSlab.Name,
                                               state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Name));
                        ShowContinueError(state,
                                          format("Surface Construction does not have a source/sink, Construction name= \"{}\".", thisConstruct.Name));
                        ErrorsFound = true;
                    }
                }
            } else {
                for (SurfNum = 1; SurfNum <= ventSlab.NumOfSurfaces; ++SurfNum) {
                    int const ConstrNum = state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Construction;
                    auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                    if (ventSlab.SurfacePtr(SurfNum) == 0) {
                        continue; // invalid surface -- detected earlier
                    }
                    if (ventSlab.ZonePtr == 0) {
                        continue; // invalid zone -- detected earlier
                    }
                    if (state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Zone != ventSlab.ZonePtr) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid surface=\"{}\".",
                                               CurrentModuleObject,
                                               ventSlab.Name,
                                               state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Name));
                        ShowContinueError(state,
                                          format("Surface in Zone={} {} in Zone={}",
                                                 state.dataHeatBal->Zone(state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Zone).Name,
                                                 CurrentModuleObject,
                                                 state.dataIPShortCut->cAlphaArgs(3)));
                        ErrorsFound = true;
                    }
                    if (state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Construction == 0) {
                        continue; // invalid construction, detected earlier
                    }
                    if (!thisConstruct.SourceSinkPresent) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid surface=\"{}\".",
                                               CurrentModuleObject,
                                               ventSlab.Name,
                                               state.dataSurface->Surface(ventSlab.SurfacePtr(SurfNum)).Name));
                        ShowContinueError(state,
                                          format("Surface Construction does not have a source/sink, Construction name= \"{}\".", thisConstruct.Name));
                        ErrorsFound = true;
                    }
                }
            }

            ventSlab.MaxAirVolFlow = state.dataIPShortCut->rNumericArgs(1);

            // Outside air information:
            ventSlab.MinOutAirVolFlow = state.dataIPShortCut->rNumericArgs(2);
            ventSlab.OutAirVolFlow = state.dataIPShortCut->rNumericArgs(3);

            ventSlab.outsideAirControlType =
                static_cast<OutsideAirControlType>(getEnumValue(OutsideAirControlTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(5))));

            switch (ventSlab.outsideAirControlType) {

            case OutsideAirControlType::VariablePercent: {
                if (lAlphaBlanks(7)) {
                    ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7));
                    ErrorsFound = true;
                } else if ((ventSlab.maxOASched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(7))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(7), state.dataIPShortCut->cAlphaArgs(7));
                    ErrorsFound = true;
                } else if (!ventSlab.maxOASched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(7), state.dataIPShortCut->cAlphaArgs(7), Clusive::In, 0.0, Clusive::In, 1.0);
                    ErrorsFound = true;
                }
            } break;

            case OutsideAirControlType::FixedOAControl: {
                if (lAlphaBlanks(7)) {
                    ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7));
                    ErrorsFound = true;
                } else if ((ventSlab.maxOASched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(7))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(7), state.dataIPShortCut->cAlphaArgs(7));
                    ErrorsFound = true;
                } else if (!ventSlab.maxOASched->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, cAlphaFields(7), state.dataIPShortCut->cAlphaArgs(7), Clusive::In, 0.0);
                    ErrorsFound = true;
                }
            } break;

            case OutsideAirControlType::FixedTemperature: {
                if (lAlphaBlanks(7)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFields(7));
                    ErrorsFound = true;
                } else if ((ventSlab.tempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(7))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(7), state.dataIPShortCut->cAlphaArgs(7));
                    ErrorsFound = true;
                }
            } break;

            default: {
                ShowSevereError(
                    state,
                    format(R"({}="{}" invalid {}="{}".)", CurrentModuleObject, ventSlab.Name, cAlphaFields(5), state.dataIPShortCut->cAlphaArgs(5)));
            } break;
            } // switch (outsideAirControlType)

            if (lAlphaBlanks(6)) {
            } else if ((ventSlab.minOASched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), state.dataIPShortCut->cAlphaArgs(6));
                ErrorsFound = true;
            }

            // System Configuration:
            ventSlab.SysConfg = static_cast<VentilatedSlabConfig>(getEnumValue(VentilatedSlabConfigNamesUC, state.dataIPShortCut->cAlphaArgs(8)));
            if (ventSlab.SysConfg == VentilatedSlabConfig::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(8), state.dataIPShortCut->cAlphaArgs(8), "Control reset to SLAB ONLY Configuration.");
                ventSlab.SysConfg = VentilatedSlabConfig::SlabOnly;
            }

            // Hollow Core information :
            ventSlab.CoreDiameter = state.dataIPShortCut->rNumericArgs(4);
            ventSlab.CoreLength = state.dataIPShortCut->rNumericArgs(5);
            ventSlab.CoreNumbers = state.dataIPShortCut->rNumericArgs(6);

            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(8), "SurfaceListNames")) {
                if (!lNumericBlanks(4)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\"  Core Diameter is not needed for the series slabs configuration- ignored.",
                                            CurrentModuleObject,
                                            ventSlab.Name));
                    ShowContinueError(state, "...It has been assigned on SlabGroup.");
                }
            }

            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(8), "SurfaceListNames")) {
                if (!lNumericBlanks(5)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\"  Core Length is not needed for the series slabs configuration- ignored.",
                                            CurrentModuleObject,
                                            ventSlab.Name));
                    ShowContinueError(state, "...It has been assigned on SlabGroup.");
                }
            }

            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(8), "SurfaceListNames")) {
                if (!lNumericBlanks(6)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\"  Core Numbers is not needed for the series slabs configuration- ignored.",
                                            CurrentModuleObject,
                                            ventSlab.Name));
                    ShowContinueError(state, "...It has been assigned on SlabGroup.");
                }
            }

            // Process the temperature control type
            ventSlab.controlType = static_cast<ControlType>(getEnumValue(ControlTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(9))));

            if (ventSlab.controlType == ControlType::Invalid) {
                ShowSevereError(
                    state,
                    format(R"({}="{}" invalid {}="{}".)", CurrentModuleObject, ventSlab.Name, cAlphaFields(9), state.dataIPShortCut->cAlphaArgs(9)));
                ShowContinueError(state, "Control reset to ODB control.");
                ventSlab.controlType = ControlType::OutdoorDryBulbTemp;
            }

            // Heating User Input Data For Ventilated Slab Control :

            // High Air Temp :
            if (lAlphaBlanks(10)) {
            } else if ((ventSlab.hotAirHiTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(10))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(10), state.dataIPShortCut->cAlphaArgs(10));
                ErrorsFound = true;
            }

            // Low Air Temp :
            if (lAlphaBlanks(11)) {
            } else if ((ventSlab.hotAirLoTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(11))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), state.dataIPShortCut->cAlphaArgs(11));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(12)) {
            } else if ((ventSlab.hotCtrlHiTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(12))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(12), state.dataIPShortCut->cAlphaArgs(12));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(13)) {
            } else if ((ventSlab.hotCtrlLoTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(13))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(13), state.dataIPShortCut->cAlphaArgs(13));
                ErrorsFound = true;
            }

            // Cooling User Input Data For Ventilated Slab Control :
            // Cooling High Temp Sch.
            if (lAlphaBlanks(14)) {
            } else if ((ventSlab.coldAirHiTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(14))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(14), state.dataIPShortCut->cAlphaArgs(14));
                ErrorsFound = true;
            }

            // Cooling Low Temp Sch.
            if (lAlphaBlanks(15)) {
            } else if ((ventSlab.coldAirLoTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(15))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(15), state.dataIPShortCut->cAlphaArgs(15));
                ErrorsFound = true;
            }

            // Cooling Control High Sch.
            if (lAlphaBlanks(16)) {
            } else if ((ventSlab.coldCtrlHiTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(16))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(16), state.dataIPShortCut->cAlphaArgs(16));
                ErrorsFound = true;
            }

            // Cooling Control Low Sch.
            if (lAlphaBlanks(17)) {
            } else if ((ventSlab.coldCtrlLoTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(17))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(17), state.dataIPShortCut->cAlphaArgs(17));
                ErrorsFound = true;
            }

            // Main air nodes (except outside air node):
            // Refer the Unit Ventilator Air Node note

            // MJW CR7903 - Ventilated slab was not drawing properly in HVAC Diagram svg output
            //  This object is structured differently from other zone equipment in that it functions
            //  as both a parent and non-parent, and it has an implicit OA mixer.  This makes it difficult
            //  to register the nodes in a way that HVAC Diagram can understand and in a way that satisfies
            //  node connection tests.  Here's an explanation of the changes made for this CR:
            //      In general, nodes associated with the ventilated slab system (the overall parent object)
            //         are registered with "-SYSTEM" appended to the object type and object name
            //         This same suffix is also added later when SetUpCompSets is called, for the same reason
            //      In general, nodes associated with the implicit OA mixer object
            //         are registered with "-OA MIXER" appended to the object type and object name
            //      %ReturnAirNode is one inlet to the implicit oa mixer
            //         For SlabOnly and SeriesSlab this node does nothing,
            //             so DataLoopNode::NodeConnectionType::Internal,ObjectIsNotParent, -OA MIXER
            //         For SlabandZone, this node extracts air from the zone,
            //             so DataLoopNode::NodeConnectionType::Inlet,ObjectIsNotParent, -OA MIXER
            //         For SlabandZone, this node is also used to associate the whole system with a pair of zone inlet/exhaust nodes,
            //             so it is registered again as DataLoopNode::NodeConnectionType::Inlet,1,ObjectIsParent, -SYSTEM
            //      %RadInNode is the ultimate air inlet to the slab or series of slabs
            //         For all types of ventilated slab, this is DataLoopNode::NodeConnectionType::Inlet,ObjectIsNotParent
            //      %OAMixerOutNode is the outlet from the implicit OA mixer
            //         For all types of ventilated slab, this is DataLoopNode::NodeConnectionType::Outlet,ObjectIsNotParent
            //      %FanOutletNode is the outlet from the explicit fan child object (redundant input, should mine from child)
            //         For all types of ventilated slab, this is DataLoopNode::NodeConnectionType::Internal,ObjectIsParent
            //      %ZoneAirInNode is applicable only to SlabandZone configuration. It is the node that flows into the zone,
            //         and it is also the outlet from the ventilated slab section, so it must be registered twice
            //         First for the overall system, DataLoopNode::NodeConnectionType::Outlet,ObjectIsParent, -SYSTEM
            //         Second as the slab outlet, DataLoopNode::NodeConnectionType::Outlet,ObjectIsNotParent
            //      %OutsideAirNode is the outdoor air inlet to the OA mixer
            //         For all types of ventilated slab, this is DataLoopNode::NodeConnectionType::Inlet,ObjectIsNotParent, -OA MIXER

            if (ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) {

                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name + "-OA MIXER",
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);
                ventSlab.RadInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                         state.dataIPShortCut->cAlphaArgs(19),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                         ventSlab.Name,
                                                                         DataLoopNode::NodeFluidType::Air,
                                                                         DataLoopNode::ConnectionType::Inlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         DataLoopNode::ObjectIsNotParent);

                ventSlab.OAMixerOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                              state.dataIPShortCut->cAlphaArgs(23),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                              ventSlab.Name + "-OA MIXER",
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::Outlet,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
                ventSlab.FanOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(24),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Internal,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);

            } else if (ventSlab.SysConfg == VentilatedSlabConfig::SeriesSlabs) {

                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name + "-OA MIXER",
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);
                ventSlab.RadInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                         state.dataIPShortCut->cAlphaArgs(19),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                         ventSlab.Name,
                                                                         DataLoopNode::NodeFluidType::Air,
                                                                         DataLoopNode::ConnectionType::Inlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         DataLoopNode::ObjectIsNotParent);

                ventSlab.OAMixerOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                              state.dataIPShortCut->cAlphaArgs(23),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                              ventSlab.Name + "-OA MIXER",
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::Outlet,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
                ventSlab.FanOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(24),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Internal,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);

            } else if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {

                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name + "-SYSTEM",
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);
                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);
                ventSlab.ReturnAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(18),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name + "-OA MIXER",
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
                ventSlab.RadInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                         state.dataIPShortCut->cAlphaArgs(19),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                         ventSlab.Name,
                                                                         DataLoopNode::NodeFluidType::Air,
                                                                         DataLoopNode::ConnectionType::Inlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         DataLoopNode::ObjectIsNotParent);
                ventSlab.OAMixerOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                              state.dataIPShortCut->cAlphaArgs(23),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                              ventSlab.Name + "-OA MIXER",
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::Outlet,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
                ventSlab.FanOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(24),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Internal,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);
            }

            if (ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) {
                if (!lAlphaBlanks(20)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\" {}=\"{}\" not needed - ignored.",
                                            CurrentModuleObject,
                                            ventSlab.Name,
                                            cAlphaFields(20),
                                            state.dataIPShortCut->cAlphaArgs(20)));
                    ShowContinueError(state, "It is used for \"SlabAndZone\" only");
                }

            } else if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {
                if (lAlphaBlanks(20)) {
                    ShowSevereError(
                        state, format("{}=\"{}\" invalid {} is blank and must be entered.", CurrentModuleObject, ventSlab.Name, cAlphaFields(20)));
                    ErrorsFound = true;
                }

                ventSlab.ZoneAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(20),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name + "-SYSTEM",
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsParent);

                ventSlab.ZoneAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             state.dataIPShortCut->cAlphaArgs(20),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                             ventSlab.Name,
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
            }

            //  Set connection type to 'Inlet', because it now uses an OA node
            ventSlab.OutsideAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                          state.dataIPShortCut->cAlphaArgs(21),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                          ventSlab.Name + "-OA MIXER",
                                                                          DataLoopNode::NodeFluidType::Air,
                                                                          DataLoopNode::ConnectionType::Inlet,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          DataLoopNode::ObjectIsNotParent);

            if (!lAlphaBlanks(21)) {
                OutAirNodeManager::CheckAndAddAirNodeNumber(state, ventSlab.OutsideAirNode, IsValid);
                if (!IsValid) {
                    ShowWarningError(
                        state,
                        format("{}=\"{}\", Adding OutdoorAir:Node={}", CurrentModuleObject, ventSlab.Name, state.dataIPShortCut->cAlphaArgs(21)));
                }
            }

            ventSlab.AirReliefNode = NodeInputManager::GetOnlySingleNode(state,
                                                                         state.dataIPShortCut->cAlphaArgs(22),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                         ventSlab.Name + "-OA MIXER",
                                                                         DataLoopNode::NodeFluidType::Air,
                                                                         DataLoopNode::ConnectionType::ReliefAir,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         DataLoopNode::ObjectIsNotParent);

            // Fan information:
            ventSlab.FanName = state.dataIPShortCut->cAlphaArgs(25);

            if ((ventSlab.Fan_Index = Fans::GetFanIndex(state, ventSlab.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(25), state.dataIPShortCut->cAlphaArgs(25));
                ErrorsFound = true;
            } else {
                ventSlab.fanType = state.dataFans->fans(ventSlab.Fan_Index)->type;
                if (ventSlab.fanType != HVAC::FanType::Constant && ventSlab.fanType != HVAC::FanType::SystemModel) {
                    ShowSevereCustom(state,
                                     eoh,
                                     format("Only fans of type Fan:ConstantVolume and Fan:SystemModel are supported.  {} is of type {}",
                                            ventSlab.FanName,
                                            HVAC::fanTypeNames[(int)ventSlab.fanType]));
                    ErrorsFound = true;
                }
            }
            if (ventSlab.outsideAirControlType == OutsideAirControlType::FixedOAControl) {
                ventSlab.OutAirVolFlow = ventSlab.MinOutAirVolFlow;
                ventSlab.maxOASched = ventSlab.minOASched;
            }

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 CurrentModuleObject,
                                                 ventSlab.Name,
                                                 "UNDEFINED",
                                                 state.dataIPShortCut->cAlphaArgs(25),
                                                 state.dataIPShortCut->cAlphaArgs(23),
                                                 state.dataIPShortCut->cAlphaArgs(24));

            // Coil options assign

            ventSlab.coilOption = static_cast<CoilType>(getEnumValue(CoilTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(26))));

            if (ventSlab.coilOption == CoilType::Invalid) {
                ShowSevereError(
                    state,
                    format(
                        R"({}="{}" invalid {}="{}".)", CurrentModuleObject, ventSlab.Name, cAlphaFields(26), state.dataIPShortCut->cAlphaArgs(26)));
                ErrorsFound = true;
            }

            if (ventSlab.coilOption == CoilType::Both || ventSlab.coilOption == CoilType::Heating) {
                // Heating coil information:
                //        A27, \field Heating Coil Object Type
                //             \type choice
                //             \key Coil:Heating:Water
                //             \key Coil:Heating:Electric
                //             \key Coil:Heating:Fuel
                //             \key Coil:Heating:Steam
                //        A28, \field Heating Coil Name
                //             \type object-list
                //             \object-list HeatingCoilName

                // Heating coil information:
                if (!lAlphaBlanks(28)) {
                    ventSlab.heatingCoilPresent = true;
                    ventSlab.heatingCoilTypeCh = state.dataIPShortCut->cAlphaArgs(27);
                    errFlag = false;

                    ventSlab.hCoilType =
                        static_cast<HeatingCoilType>(getEnumValue(HeatingCoilTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(27))));

                    switch (ventSlab.hCoilType) {

                    case HeatingCoilType::Water: {
                        ventSlab.heatingCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
                        break;
                    }
                    case HeatingCoilType::Steam: {
                        ventSlab.heatingCoilType = DataPlant::PlantEquipmentType::CoilSteamAirHeating;
                        ventSlab.heatingCoil_fluid = Fluid::GetSteam(state);
                        if (ventSlab.heatingCoil_fluid == nullptr) {
                            ShowSevereError(state, format("{}=\"{}Steam Properties not found.", CurrentModuleObject, ventSlab.Name));
                            if (SteamMessageNeeded) {
                                ShowContinueError(state, "Steam Fluid Properties should have been included in the input file.");
                            }
                            ErrorsFound = true;
                            SteamMessageNeeded = false;
                        }
                        break;
                    }
                    case HeatingCoilType::Electric:
                    case HeatingCoilType::Gas:
                        break;
                    default: {
                        ShowSevereError(state,
                                        format(R"({}="{}" invalid {}="{}".)",
                                               CurrentModuleObject,
                                               ventSlab.Name,
                                               cAlphaFields(27),
                                               state.dataIPShortCut->cAlphaArgs(27)));
                        ErrorsFound = true;
                        errFlag = true;
                        break;
                    }
                    }
                    if (!errFlag) {
                        ventSlab.heatingCoilName = state.dataIPShortCut->cAlphaArgs(28);
                        ValidateComponent(state, state.dataIPShortCut->cAlphaArgs(27), ventSlab.heatingCoilName, IsNotOK, CurrentModuleObject);
                        if (IsNotOK) {
                            ShowContinueError(state,
                                              format("{}=\"{}\" invalid {}=\"{}\".",
                                                     CurrentModuleObject,
                                                     ventSlab.Name,
                                                     cAlphaFields(28),
                                                     state.dataIPShortCut->cAlphaArgs(28)));
                            ShowContinueError(state, format("... not valid for {}=\"{}\".", cAlphaFields(27), state.dataIPShortCut->cAlphaArgs(27)));
                            ErrorsFound = true;
                        }
                    }

                    ventSlab.MinVolHotWaterFlow = 0.0;
                    ventSlab.MinVolHotSteamFlow = 0.0;

                    // The heating coil control node is necessary for a hot water coil, but not necessary for an
                    // electric or gas coil.
                    if (ventSlab.hCoilType == HeatingCoilType::Gas || ventSlab.hCoilType == HeatingCoilType::Electric) {
                        if (!lAlphaBlanks(29)) {
                            ShowWarningError(state,
                                             format("{}=\"{}\" {}=\"{}\" not needed - ignored.",
                                                    CurrentModuleObject,
                                                    ventSlab.Name,
                                                    cAlphaFields(29),
                                                    state.dataIPShortCut->cAlphaArgs(29)));
                            ShowContinueError(state, "..It is used for hot water coils only.");
                        }
                    } else {
                        if (lAlphaBlanks(29)) {
                            ShowSevereError(
                                state,
                                format("{}=\"{}\" invalid {} is blank and must be entered.", CurrentModuleObject, ventSlab.Name, cAlphaFields(29)));
                            ErrorsFound = true;
                        }
                        ventSlab.HotControlNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                      state.dataIPShortCut->cAlphaArgs(29),
                                                                                      ErrorsFound,
                                                                                      DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                                      ventSlab.Name,
                                                                                      DataLoopNode::NodeFluidType::Water,
                                                                                      DataLoopNode::ConnectionType::Actuator,
                                                                                      NodeInputManager::CompFluidStream::Primary,
                                                                                      DataLoopNode::ObjectIsParent);
                    }
                    ventSlab.HotControlOffset = 0.001;

                    if (ventSlab.hCoilType == HeatingCoilType::Water) {
                        ventSlab.MaxVolHotWaterFlow = GetWaterCoilMaxFlowRate(state, "Coil:Heating:Water", ventSlab.heatingCoilName, ErrorsFound);
                        ventSlab.MaxVolHotSteamFlow = GetWaterCoilMaxFlowRate(state, "Coil:Heating:Water", ventSlab.heatingCoilName, ErrorsFound);
                    } else if (ventSlab.hCoilType == HeatingCoilType::Steam) {
                        ventSlab.MaxVolHotWaterFlow = GetSteamCoilMaxFlowRate(state, "Coil:Heating:Steam", ventSlab.heatingCoilName, ErrorsFound);
                        ventSlab.MaxVolHotSteamFlow = GetSteamCoilMaxFlowRate(state, "Coil:Heating:Steam", ventSlab.heatingCoilName, ErrorsFound);
                    }

                } else { // no heating coil
                    ShowSevereError(state, format("{}=\"{}\" missing heating coil.", CurrentModuleObject, ventSlab.Name));
                    ShowContinueError(state,
                                      format("a heating coil is required for {}=\"{}\".", cAlphaFields(26), state.dataIPShortCut->cAlphaArgs(26)));
                    ErrorsFound = true;
                }
            }

            if (ventSlab.coilOption == CoilType::Both || ventSlab.coilOption == CoilType::Cooling) {
                // Cooling coil information (if one is present):
                //        A30, \field Cooling Coil Object Type
                //             \type choice
                //             \key Coil:Cooling:Water
                //             \key Coil:Cooling:Water:DetailedGeometry
                //             \key CoilSystem:Cooling:Water:HeatExchangerAssisted
                //        A31, \field Cooling Coil Name
                //             \type object-list
                //             \object-list CoolingCoilsWater
                // Cooling coil information (if one is present):
                if (!lAlphaBlanks(31)) {
                    ventSlab.coolingCoilPresent = true;
                    ventSlab.coolingCoilTypeCh = state.dataIPShortCut->cAlphaArgs(30);
                    errFlag = false;

                    ventSlab.cCoilType =
                        static_cast<CoolingCoilType>(getEnumValue(CoolingCoilTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(30))));
                    switch (ventSlab.cCoilType) {
                    case CoolingCoilType::WaterCooling: {
                        ventSlab.coolingCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
                        ventSlab.coolingCoilPlantName = state.dataIPShortCut->cAlphaArgs(31);
                        break;
                    }
                    case CoolingCoilType::DetailedCooling: {
                        ventSlab.coolingCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
                        ventSlab.coolingCoilPlantName = state.dataIPShortCut->cAlphaArgs(31);
                        break;
                    }
                    case CoolingCoilType::HXAssisted: {
                        HVACHXAssistedCoolingCoil::GetHXCoilTypeAndName(state,
                                                                        state.dataIPShortCut->cAlphaArgs(30),
                                                                        state.dataIPShortCut->cAlphaArgs(31),
                                                                        ErrorsFound,
                                                                        ventSlab.coolingCoilPlantType,
                                                                        ventSlab.coolingCoilPlantName);
                        if (Util::SameString(ventSlab.coolingCoilPlantType, "Coil:Cooling:Water")) {
                            ventSlab.coolingCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
                        } else if (Util::SameString(ventSlab.coolingCoilPlantType, "Coil:Cooling:Water:DetailedGeometry")) {
                            ventSlab.coolingCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
                        } else {
                            ShowSevereError(state, format("GetVentilatedSlabInput: {}=\"{}\", invalid", CurrentModuleObject, ventSlab.Name));
                            ShowContinueError(state, format("For: {}=\"{}\".", cAlphaFields(30), state.dataIPShortCut->cAlphaArgs(30)));
                            ShowContinueError(state,
                                              format("Invalid Coil Type={}, Name={}", ventSlab.coolingCoilPlantType, ventSlab.coolingCoilPlantName));
                            ShowContinueError(state, R"(must be "Coil:Cooling:Water" or "Coil:Cooling:Water:DetailedGeometry")");
                            ErrorsFound = true;
                        }
                        break;
                    }
                    default: {
                        ShowSevereError(state,
                                        format(R"({}="{}" invalid {}="{}".)",
                                               CurrentModuleObject,
                                               ventSlab.Name,
                                               cAlphaFields(29),
                                               state.dataIPShortCut->cAlphaArgs(29)));
                        ErrorsFound = true;
                        errFlag = true;
                        break;
                    }
                    }

                    if (!errFlag) {
                        ventSlab.coolingCoilName = state.dataIPShortCut->cAlphaArgs(31);
                        ValidateComponent(state, state.dataIPShortCut->cAlphaArgs(30), ventSlab.coolingCoilName, IsNotOK, "ZoneHVAC:VentilatedSlab ");
                        if (IsNotOK) {
                            ShowContinueError(state,
                                              format("{}=\"{}\" invalid {}=\"{}\".",
                                                     CurrentModuleObject,
                                                     ventSlab.Name,
                                                     cAlphaFields(31),
                                                     state.dataIPShortCut->cAlphaArgs(31)));
                            ShowContinueError(state, format("... not valid for {}=\"{}\".", cAlphaFields(30), state.dataIPShortCut->cAlphaArgs(30)));
                            ErrorsFound = true;
                        }
                    }

                    ventSlab.MinVolColdWaterFlow = 0.0;

                    ventSlab.ColdControlNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                   state.dataIPShortCut->cAlphaArgs(32),
                                                                                   ErrorsFound,
                                                                                   DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                                   ventSlab.Name,
                                                                                   DataLoopNode::NodeFluidType::Water,
                                                                                   DataLoopNode::ConnectionType::Actuator,
                                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                                   DataLoopNode::ObjectIsParent);

                    if (lAlphaBlanks(32)) {
                        ShowSevereError(
                            state,
                            format("{}=\"{}\" invalid {} is blank and must be entered.", CurrentModuleObject, ventSlab.Name, cAlphaFields(32)));
                        ErrorsFound = true;
                    }

                    ventSlab.ColdControlOffset = 0.001;

                    if (ventSlab.cCoilType == CoolingCoilType::WaterCooling) {
                        ventSlab.MaxVolColdWaterFlow = GetWaterCoilMaxFlowRate(state, "Coil:Cooling:Water", ventSlab.coolingCoilName, ErrorsFound);
                    } else if (ventSlab.cCoilType == CoolingCoilType::DetailedCooling) {
                        ventSlab.MaxVolColdWaterFlow =
                            GetWaterCoilMaxFlowRate(state, "Coil:Cooling:Water:DetailedGeometry", ventSlab.coolingCoilName, ErrorsFound);
                    } else if (ventSlab.cCoilType == CoolingCoilType::HXAssisted) {
                        ventSlab.MaxVolColdWaterFlow =
                            GetHXAssistedCoilFlowRate(state, "CoilSystem:Cooling:Water:HeatExchangerAssisted", ventSlab.coolingCoilName, ErrorsFound);
                    }

                } else { // No Cooling Coil
                    ShowSevereError(state, format("{}=\"{}\" missing cooling coil.", CurrentModuleObject, ventSlab.Name));
                    ShowContinueError(state,
                                      format("a cooling coil is required for {}=\"{}\".", cAlphaFields(26), state.dataIPShortCut->cAlphaArgs(26)));
                    ErrorsFound = true;
                }
            }

            ventSlab.HVACSizingIndex = 0;
            if (!lAlphaBlanks(34)) {
                ventSlab.HVACSizingIndex = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(34), state.dataSize->ZoneHVACSizing);
                if (ventSlab.HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(34), state.dataIPShortCut->cAlphaArgs(34)));
                    ShowContinueError(state, format("Occurs in {} = {}", cMO_VentilatedSlab, ventSlab.Name));
                    ErrorsFound = true;
                }
            }

            switch (ventSlab.coilOption) {
            case CoilType::Both: { // 'HeatingAndCooling'
                // Add cooling coil to component sets array when present
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     ventSlab.Name,
                                                     state.dataIPShortCut->cAlphaArgs(30),
                                                     state.dataIPShortCut->cAlphaArgs(31),
                                                     state.dataIPShortCut->cAlphaArgs(24),
                                                     "UNDEFINED");

                // Add heating coil to component sets array when cooling coil present
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     ventSlab.Name,
                                                     state.dataIPShortCut->cAlphaArgs(27),
                                                     state.dataIPShortCut->cAlphaArgs(28),
                                                     "UNDEFINED",
                                                     state.dataIPShortCut->cAlphaArgs(19));
                break;
            }
            case CoilType::Heating: { // 'Heating'
                // Add heating coil to component sets array when no cooling coil present
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     ventSlab.Name,
                                                     state.dataIPShortCut->cAlphaArgs(27),
                                                     state.dataIPShortCut->cAlphaArgs(28),
                                                     state.dataIPShortCut->cAlphaArgs(24),
                                                     state.dataIPShortCut->cAlphaArgs(19));
                break;
            }
            case CoilType::Cooling: { // 'Cooling'
                // Add cooling coil to component sets array when no heating coil present
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     ventSlab.Name,
                                                     state.dataIPShortCut->cAlphaArgs(30),
                                                     state.dataIPShortCut->cAlphaArgs(31),
                                                     state.dataIPShortCut->cAlphaArgs(24),
                                                     state.dataIPShortCut->cAlphaArgs(19));
                break;
            }
            case CoilType::None:
            default:
                break;
            }

        } // ...loop over all of the ventilated slab found in the input file

        cAlphaArgs.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        rNumericArgs.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{} errors occurred in input.  Program terminates.", CurrentModuleObject));
        }

        // Setup Report variables for the VENTILATED SLAB
        for (Item = 1; Item <= state.dataVentilatedSlab->NumOfVentSlabs; ++Item) {
            auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Radiant Heating Rate",
                                Constant::Units::W,
                                ventSlab.RadHeatingPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Radiant Heating Energy",
                                Constant::Units::J,
                                ventSlab.RadHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Radiant Cooling Rate",
                                Constant::Units::W,
                                ventSlab.RadCoolingPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Radiant Cooling Energy",
                                Constant::Units::J,
                                ventSlab.RadCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Heating Rate",
                                Constant::Units::W,
                                ventSlab.HeatCoilPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Heating Energy",
                                Constant::Units::J,
                                ventSlab.HeatCoilEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Total Cooling Rate",
                                Constant::Units::W,
                                ventSlab.TotCoolCoilPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Total Cooling Energy",
                                Constant::Units::J,
                                ventSlab.TotCoolCoilEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Sensible Cooling Rate",
                                Constant::Units::W,
                                ventSlab.SensCoolCoilPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Sensible Cooling Energy",
                                Constant::Units::J,
                                ventSlab.SensCoolCoilEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Latent Cooling Rate",
                                Constant::Units::W,
                                ventSlab.LateCoolCoilPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Coil Latent Cooling Energy",
                                Constant::Units::J,
                                ventSlab.LateCoolCoilEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                ventSlab.AirMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Fan Electricity Rate",
                                Constant::Units::W,
                                ventSlab.ElecFanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            //! Note that the ventilated slab fan electric is NOT metered because this value is already metered through the fan component
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Fan Electricity Energy",
                                Constant::Units::J,
                                ventSlab.ElecFanEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Inlet Air Temperature",
                                Constant::Units::C,
                                ventSlab.SlabInTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Outlet Air Temperature",
                                Constant::Units::C,
                                ventSlab.SlabOutTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Zone Inlet Air Temperature",
                                Constant::Units::C,
                                ventSlab.ZoneInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Return Air Temperature",
                                Constant::Units::C,
                                ventSlab.ReturnAirTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Fan Outlet Air Temperature",
                                Constant::Units::C,
                                ventSlab.FanOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
            SetupOutputVariable(state,
                                "Zone Ventilated Slab Fan Availability Status",
                                Constant::Units::None,
                                ventSlab.availStatus,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                ventSlab.Name);
        }
    }

    void InitVentilatedSlab(EnergyPlusData &state,
                            int const Item,               // index for the current ventilated slab
                            int const VentSlabZoneNum,    // number of zone being served
                            bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes all of the data elements which are necessary
        // to simulate a Ventilated Slab.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Using/Aliasing
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitVentilatedSlab");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirRelNode; // relief air node number in Ventilated Slab loop
        Real64 RhoAir;  // air density at InNode
        Real64 TempSteamIn;
        Real64 SteamDensity;
        Real64 rho;

        // Do the one time initializations

        if (state.dataVentilatedSlab->MyOneTimeFlag) {
            state.dataVentilatedSlab->MyEnvrnFlag.allocate(state.dataVentilatedSlab->NumOfVentSlabs);
            state.dataVentilatedSlab->MySizeFlag.allocate(state.dataVentilatedSlab->NumOfVentSlabs);
            state.dataVentilatedSlab->MyPlantScanFlag.allocate(state.dataVentilatedSlab->NumOfVentSlabs);
            state.dataVentilatedSlab->MyZoneEqFlag.allocate(state.dataVentilatedSlab->NumOfVentSlabs);

            // Initialize total areas for all radiant systems and dimension record keeping arrays
            for (auto &thisVentSlab : state.dataVentilatedSlab->VentSlab) {
                thisVentSlab.TotalSurfaceArea = 0.0;
                int numRadSurfs = thisVentSlab.NumOfSurfaces;
                for (int SurfNum = 1; SurfNum <= numRadSurfs; ++SurfNum) {
                    thisVentSlab.TotalSurfaceArea += state.dataSurface->Surface(thisVentSlab.SurfacePtr(SurfNum)).Area;
                }
                thisVentSlab.QRadSysSrcAvg.dimension(numRadSurfs, 0.0);
                thisVentSlab.LastQRadSysSrc.dimension(numRadSurfs, 0.0);
                thisVentSlab.LastSysTimeElapsed = 0.0;
                thisVentSlab.LastTimeStepSys = 0.0;
            }
            state.dataVentilatedSlab->MyEnvrnFlag = true;
            state.dataVentilatedSlab->MySizeFlag = true;
            state.dataVentilatedSlab->MyPlantScanFlag = true;
            state.dataVentilatedSlab->MyZoneEqFlag = true;
            state.dataVentilatedSlab->MyOneTimeFlag = false;
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::VentilatedSlab).ZoneCompAvailMgrs(Item);
            if (state.dataVentilatedSlab->MyZoneEqFlag(Item)) { // initialize the name of each availability manager list and zone number
                availMgr.AvailManagerListName = ventSlab.AvailManagerListName;
                availMgr.ZoneNum = VentSlabZoneNum;
                state.dataVentilatedSlab->MyZoneEqFlag(Item) = false;
            }
            ventSlab.availStatus = availMgr.availStatus;
        }

        if (state.dataVentilatedSlab->MyPlantScanFlag(Item) && allocated(state.dataPlnt->PlantLoop)) {
            if ((ventSlab.heatingCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) ||
                (ventSlab.heatingCoilType == DataPlant::PlantEquipmentType::CoilSteamAirHeating)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(
                    state, ventSlab.heatingCoilName, ventSlab.heatingCoilType, ventSlab.HWPlantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowContinueError(state, format("Reference Unit=\"{}\", type=ZoneHVAC:VentilatedSlab", ventSlab.Name));
                    ShowFatalError(state, "InitVentilatedSlab: Program terminated due to previous condition(s).");
                }

                ventSlab.HotCoilOutNodeNum = DataPlant::CompData::getPlantComponent(state, ventSlab.HWPlantLoc).NodeNumOut;
            }
            if ((ventSlab.coolingCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
                (ventSlab.coolingCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state, ventSlab.coolingCoilPlantName, ventSlab.coolingCoilType, ventSlab.CWPlantLoc, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("Reference Unit=\"{}\", type=ZoneHVAC:VentilatedSlab", ventSlab.Name));
                    ShowFatalError(state, "InitVentilatedSlab: Program terminated due to previous condition(s).");
                }
                ventSlab.ColdCoilOutNodeNum = DataPlant::CompData::getPlantComponent(state, ventSlab.CWPlantLoc).NodeNumOut;
            } else {
                if (ventSlab.coolingCoilPresent) {
                    ShowFatalError(state, format("InitVentilatedSlab: Unit={}, invalid cooling coil type. Program terminated.", ventSlab.Name));
                }
            }
            state.dataVentilatedSlab->MyPlantScanFlag(Item) = false;
        } else if (state.dataVentilatedSlab->MyPlantScanFlag(Item) && !state.dataGlobal->AnyPlantInModel) {
            state.dataVentilatedSlab->MyPlantScanFlag(Item) = false;
        }

        // need to check all Ventilated Slab units to see if they are on Zone Equipment List or issue warning
        if (!state.dataVentilatedSlab->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataVentilatedSlab->ZoneEquipmentListChecked = true;
            for (int RadNum = 1; RadNum <= state.dataVentilatedSlab->NumOfVentSlabs; ++RadNum) {
                if (DataZoneEquipment::CheckZoneEquipmentList(state, cMO_VentilatedSlab, state.dataVentilatedSlab->VentSlab(RadNum).Name)) {
                    continue;
                }
                ShowSevereError(
                    state,
                    format("InitVentilatedSlab: Ventilated Slab Unit=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                           cMO_VentilatedSlab,
                           state.dataVentilatedSlab->VentSlab(RadNum).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataVentilatedSlab->MySizeFlag(Item) && !state.dataVentilatedSlab->MyPlantScanFlag(Item)) {

            SizeVentilatedSlab(state, Item);

            state.dataVentilatedSlab->MySizeFlag(Item) = false;
        }

        int InNode = ventSlab.ReturnAirNode;
        int OutNode = ventSlab.RadInNode;
        int OutsideAirNode = ventSlab.OutsideAirNode;

        // Do the one time initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataVentilatedSlab->MyEnvrnFlag(Item) && !state.dataVentilatedSlab->MyPlantScanFlag(Item)) {

            // Coil Part
            RhoAir = state.dataEnvrn->StdRhoAir;

            if (state.dataVentilatedSlab->NumOfVentSlabs > 0) {
                for (auto &e : state.dataVentilatedSlab->VentSlab) {
                    e.ZeroVentSlabSourceSumHATsurf = 0.0;
                    e.RadHeatingPower = 0.0;
                    e.RadHeatingEnergy = 0.0;
                    e.RadCoolingPower = 0.0;
                    e.RadCoolingEnergy = 0.0;
                    e.QRadSysSrcAvg = 0.0;
                    e.LastQRadSysSrc = 0.0;
                    e.LastSysTimeElapsed = 0.0;
                    e.LastTimeStepSys = 0.0;
                }
            }

            // set the initial Temperature of Return Air

            // set the mass flow rates from the input volume flow rates
            ventSlab.MaxAirMassFlow = RhoAir * ventSlab.MaxAirVolFlow;
            ventSlab.OutAirMassFlow = RhoAir * ventSlab.OutAirVolFlow;
            ventSlab.MinOutAirMassFlow = RhoAir * ventSlab.MinOutAirVolFlow;
            if (ventSlab.OutAirMassFlow > ventSlab.MaxAirMassFlow) {
                ventSlab.OutAirMassFlow = ventSlab.MaxAirMassFlow;
                ventSlab.MinOutAirMassFlow = ventSlab.OutAirMassFlow * (ventSlab.MinOutAirVolFlow / ventSlab.OutAirVolFlow);
                ShowWarningError(state,
                                 format("Outdoor air mass flow rate higher than unit flow rate, reset to unit flow rate for {}", ventSlab.Name));
            }

            // set the node max and min mass flow rates
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMax = ventSlab.OutAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMin = 0.0;

            state.dataLoopNodes->Node(OutNode).MassFlowRateMax = ventSlab.MaxAirMassFlow;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;

            state.dataLoopNodes->Node(InNode).MassFlowRateMax = ventSlab.MaxAirMassFlow;
            state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;

            if (ventSlab.heatingCoilPresent) { // Only initialize these if a heating coil is actually present

                if (ventSlab.heatingCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating &&
                    !state.dataVentilatedSlab->MyPlantScanFlag(Item)) {
                    rho = state.dataPlnt->PlantLoop(ventSlab.HWPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                    ventSlab.MaxHotWaterFlow = rho * ventSlab.MaxVolHotWaterFlow;
                    ventSlab.MinHotWaterFlow = rho * ventSlab.MinVolHotWaterFlow;

                    PlantUtilities::InitComponentNodes(
                        state, ventSlab.MinHotWaterFlow, ventSlab.MaxHotWaterFlow, ventSlab.HotControlNode, ventSlab.HotCoilOutNodeNum);
                }
                if (ventSlab.heatingCoilType == DataPlant::PlantEquipmentType::CoilSteamAirHeating &&
                    !state.dataVentilatedSlab->MyPlantScanFlag(Item)) {
                    TempSteamIn = 100.00;
                    SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                    ventSlab.MaxHotSteamFlow = SteamDensity * ventSlab.MaxVolHotSteamFlow;
                    ventSlab.MinHotSteamFlow = SteamDensity * ventSlab.MinVolHotSteamFlow;

                    PlantUtilities::InitComponentNodes(
                        state, ventSlab.MinHotSteamFlow, ventSlab.MaxHotSteamFlow, ventSlab.HotControlNode, ventSlab.HotCoilOutNodeNum);
                }
            } //(VentSlab(Item)%HCoilPresent)

            if (ventSlab.coolingCoilPresent && !state.dataVentilatedSlab->MyPlantScanFlag(Item)) {
                // Only initialize these if a cooling coil is actually present
                if ((ventSlab.coolingCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
                    (ventSlab.coolingCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling)) {
                    rho = state.dataPlnt->PlantLoop(ventSlab.CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                    ventSlab.MaxColdWaterFlow = rho * ventSlab.MaxVolColdWaterFlow;
                    ventSlab.MinColdWaterFlow = rho * ventSlab.MinVolColdWaterFlow;
                    PlantUtilities::InitComponentNodes(
                        state, ventSlab.MinColdWaterFlow, ventSlab.MaxColdWaterFlow, ventSlab.ColdControlNode, ventSlab.ColdCoilOutNodeNum);
                }
            }

            state.dataVentilatedSlab->MyEnvrnFlag(Item) = false;

        } // ...end start of environment inits

        if (!state.dataGlobal->BeginEnvrnFlag) {

            state.dataVentilatedSlab->MyEnvrnFlag(Item) = true;
        }

        // These initializations are done every iteration...
        AirRelNode = ventSlab.AirReliefNode;

        // First, set the flow conditions up so that there is flow through the ventilated
        // slab system(this will be shut down if the system is not available or there
        // is no load
        state.dataLoopNodes->Node(InNode).MassFlowRate = ventSlab.MaxAirMassFlow;
        state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail = ventSlab.MaxAirMassFlow;
        state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = ventSlab.MaxAirMassFlow;
        state.dataLoopNodes->Node(OutNode).MassFlowRate = ventSlab.MaxAirMassFlow;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMaxAvail = ventSlab.MaxAirMassFlow;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMinAvail = ventSlab.MaxAirMassFlow;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = ventSlab.OutAirMassFlow;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = ventSlab.OutAirMassFlow;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = ventSlab.OutAirMassFlow;
        state.dataLoopNodes->Node(AirRelNode).MassFlowRate = ventSlab.OutAirMassFlow;
        state.dataLoopNodes->Node(AirRelNode).MassFlowRateMaxAvail = ventSlab.OutAirMassFlow;
        state.dataLoopNodes->Node(AirRelNode).MassFlowRateMinAvail = ventSlab.OutAirMassFlow;

        // Initialize the relief air (same as inlet conditions to the Ventilated Slab ..
        // Note that mass flow rates will be taken care of later.
        state.dataLoopNodes->Node(AirRelNode) = state.dataLoopNodes->Node(InNode);
        state.dataVentilatedSlab->OAMassFlowRate = 0.0;

        // Just in case the system is off and conditions do not get sent through
        // the system for some reason, set the outlet conditions equal to the inlet
        // conditions of the ventilated slab mixer
        state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(InNode).Temp;
        state.dataLoopNodes->Node(OutNode).Press = state.dataLoopNodes->Node(InNode).Press;
        state.dataLoopNodes->Node(OutNode).HumRat = state.dataLoopNodes->Node(InNode).HumRat;
        state.dataLoopNodes->Node(OutNode).Enthalpy = state.dataLoopNodes->Node(InNode).Enthalpy;

        // These initializations only need to be done once at the start of the iterations...
        if (state.dataGlobal->BeginTimeStepFlag && FirstHVACIteration) {
            // Initialize the outside air conditions...
            state.dataLoopNodes->Node(OutsideAirNode).Temp = state.dataLoopNodes->Node(OutsideAirNode).OutAirDryBulb;
            state.dataLoopNodes->Node(OutsideAirNode).HumRat = state.dataEnvrn->OutHumRat;
            state.dataLoopNodes->Node(OutsideAirNode).Press = state.dataEnvrn->OutBaroPress;

            // The first pass through in a particular time step
            int ZoneNum = ventSlab.ZonePtr;
            ventSlab.ZeroVentSlabSourceSumHATsurf =
                state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state); // Set this to figure what part of the load the radiant system meets
            ventSlab.QRadSysSrcAvg = 0.0;                           // Initialize this variable to zero (radiant system defaults to off)
            ventSlab.LastQRadSysSrc = 0.0;     // At the start of a time step, reset to zero so average calculation can begin again
            ventSlab.LastSysTimeElapsed = 0.0; // At the start of a time step, reset to zero so average calculation can begin again
            ventSlab.LastTimeStepSys = 0.0;    // At the start of a time step, reset to zero so average calculation can begin again
        }
    }

    void SizeVentilatedSlab(EnergyPlusData &state, int const Item)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       July 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Ventilated Slab components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeVentilatedSlab");

        int CurZoneEqNum = state.dataSize->CurZoneEqNum;
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound;
        Real64 DesCoilLoad;
        Real64 TempSteamIn;
        Real64 EnthSteamInDry;
        Real64 EnthSteamOutWet;
        Real64 LatentHeatSteam;
        Real64 SteamDensity;
        std::string CoolingCoilName;
        std::string CoolingCoilType;
        Real64 rho;
        Real64 Cp;
        Real64 MaxAirVolFlowDes;        // Autosized maximum air flow for reporting
        Real64 MaxAirVolFlowUser;       // Hardsized maximum air flow for reporting
        Real64 OutAirVolFlowDes;        // Autosized outdoor air flow for reporting
        Real64 OutAirVolFlowUser;       // Hardsized outdoor air flow for reporting
        Real64 MinOutAirVolFlowDes;     // Autosized minimum outdoor air flow for reporting
        Real64 MinOutAirVolFlowUser;    // Hardsized minimum outdoor air flow for reporting
        Real64 MaxVolHotWaterFlowDes;   // Autosized maximum hot water flow for reporting
        Real64 MaxVolHotWaterFlowUser;  // Hardsized maximum hot water flow for reporting
        Real64 MaxVolHotSteamFlowDes;   // Autosized maximum hot steam flow for reporting
        Real64 MaxVolHotSteamFlowUser;  // Hardsized maximum hot steam flow for reporting
        Real64 MaxVolColdWaterFlowDes;  // Autosized maximum cold water flow for reporting
        Real64 MaxVolColdWaterFlowUser; // Hardsized maximum cold water flow for reporting
        std::string CompName;           // component name
        std::string CompType;           // component type
        std::string SizingString;       // input field sizing description (e.g., Nominal Capacity)
        Real64 TempSize;                // autosized value of coil input field
        int FieldNum = 2;               // IDD numeric field number where input field description is found
        int SizingMethod;  // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing, CoolingCapacitySizing,
                           // HeatingCapacitySizing, etc.)
        bool PrintFlag;    // TRUE when sizing information is reported in the eio file
        int zoneHVACIndex; // index of zoneHVAC equipment sizing specification
        Real64 CoolingAirVolFlowScalable; // cooling airvolume for rate determined using scalable sizing method
        Real64 HeatingAirVolFlowScalable; // heating airvolume for rate determined using scalable sizing method
        bool DoWaterCoilSizing;           // if TRUE do water coil sizing calculation
        Real64 WaterCoilSizDeltaT;        // water coil deltaT for design water flow rate autosizing

        WaterCoilSizDeltaT = 0.0;
        ErrorsFound = false;
        MaxAirVolFlowDes = 0.0;
        MaxAirVolFlowUser = 0.0;
        OutAirVolFlowDes = 0.0;
        OutAirVolFlowUser = 0.0;
        MinOutAirVolFlowDes = 0.0;
        MinOutAirVolFlowUser = 0.0;
        MaxVolHotWaterFlowDes = 0.0;
        MaxVolHotWaterFlowUser = 0.0;
        MaxVolHotSteamFlowDes = 0.0;
        MaxVolHotSteamFlowUser = 0.0;
        MaxVolColdWaterFlowDes = 0.0;
        MaxVolColdWaterFlowUser = 0.0;
        CoolingAirVolFlowScalable = 0.0;
        HeatingAirVolFlowScalable = 0.0;
        state.dataSize->DataScalableSizingON = false;
        state.dataSize->DataScalableCapSizingON = false;
        CompType = cMO_VentilatedSlab;
        CompName = ventSlab.Name;
        state.dataSize->DataZoneNumber = ventSlab.ZonePtr;
        state.dataSize->DataFanType = ventSlab.fanType;
        state.dataSize->DataFanIndex = ventSlab.Fan_Index;
        // ventilated slab unit is always blow thru
        state.dataSize->DataFanPlacement = HVAC::FanPlace::BlowThru;

        if (ventSlab.HVACSizingIndex > 0) {
            zoneHVACIndex = ventSlab.HVACSizingIndex;
            // N1 , \field Maximum Supply Air Flow Rate
            FieldNum = 1;
            PrintFlag = true;
            SizingString = state.dataVentilatedSlab->VentSlabNumericFields(Item).FieldNames(FieldNum) + " [m3/s]";
            auto &zoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod > 0) {
                SizingMethod = HVAC::CoolingAirflowSizing;
                int SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                if (SAFMethod == DataSizing::None || SAFMethod == DataSizing::SupplyAirFlowRate || SAFMethod == DataSizing::FlowPerFloorArea ||
                    SAFMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                    if (SAFMethod == DataSizing::SupplyAirFlowRate) {
                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                            zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                            zoneEqSizing.SystemAirFlow = true;
                        }
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    } else if (SAFMethod == DataSizing::FlowPerFloorArea) {
                        zoneEqSizing.SystemAirFlow = true;
                        zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow *
                                                  state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        TempSize = zoneEqSizing.AirVolFlow;
                        state.dataSize->DataScalableSizingON = true;
                    } else if (SAFMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                        state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        TempSize = DataSizing::AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                    } else {
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    }
                    CoolingAirFlowSizer sizingCoolingAirFlow;
                    std::string stringOverride = "Maximum Air Flow Rate [m3/s]";
                    if (state.dataGlobal->isEpJSON) {
                        stringOverride = "maximum_air_flow_rate [m3/s]";
                    }
                    sizingCoolingAirFlow.overrideSizingString(stringOverride);
                    // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    CoolingAirVolFlowScalable = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);

                } else if (SAFMethod == DataSizing::FlowPerCoolingCapacity) {
                    TempSize = DataSizing::AutoSize;
                    PrintFlag = false;
                    state.dataSize->DataScalableSizingON = true;
                    state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                    CoolingCapacitySizer sizerCoolingCapacity;
                    sizerCoolingCapacity.overrideSizingString(SizingString);
                    sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    state.dataSize->DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                    state.dataSize->DataFlowPerCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    PrintFlag = true;
                    TempSize = DataSizing::AutoSize;
                    CoolingAirFlowSizer sizingCoolingAirFlow;
                    std::string stringOverride = "Maximum Air Flow Rate [m3/s]";
                    if (state.dataGlobal->isEpJSON) {
                        stringOverride = "maximum_air_flow_rate [m3/s]";
                    }
                    sizingCoolingAirFlow.overrideSizingString(stringOverride);
                    // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    CoolingAirVolFlowScalable = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                }
            }
            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod > 0) {
                SizingMethod = HVAC::HeatingAirflowSizing;
                int SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                if (SAFMethod == DataSizing::None || SAFMethod == DataSizing::SupplyAirFlowRate || SAFMethod == DataSizing::FlowPerFloorArea ||
                    SAFMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                    if (SAFMethod == DataSizing::SupplyAirFlowRate) {
                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow > 0.0) {
                            zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                            zoneEqSizing.SystemAirFlow = true;
                        }
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    } else if (SAFMethod == DataSizing::FlowPerFloorArea) {
                        zoneEqSizing.SystemAirFlow = true;
                        zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow *
                                                  state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        TempSize = zoneEqSizing.AirVolFlow;
                        state.dataSize->DataScalableSizingON = true;
                    } else if (SAFMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                        state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        TempSize = DataSizing::AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                    } else {
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    }
                    HeatingAirFlowSizer sizingHeatingAirFlow;
                    sizingHeatingAirFlow.overrideSizingString(SizingString);
                    // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    HeatingAirVolFlowScalable = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
                } else if (SAFMethod == DataSizing::FlowPerHeatingCapacity) {
                    TempSize = DataSizing::AutoSize;
                    PrintFlag = false;
                    state.dataSize->DataScalableSizingON = true;
                    state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                    HeatingCapacitySizer sizerHeatingCapacity;
                    sizerHeatingCapacity.overrideSizingString(SizingString);
                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    state.dataSize->DataAutosizedHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                    state.dataSize->DataFlowPerHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    PrintFlag = true;
                    TempSize = DataSizing::AutoSize;
                    HeatingAirFlowSizer sizingHeatingAirFlow;
                    sizingHeatingAirFlow.overrideSizingString(SizingString);
                    // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    HeatingAirVolFlowScalable = sizingHeatingAirFlow.size(state, TempSize, ErrorsFound);
                }
            }
            // DataScalableSizingON = false;
            ventSlab.MaxAirVolFlow = max(CoolingAirVolFlowScalable, HeatingAirVolFlowScalable);
        } else {
            // no scalble sizing method has been specified. Sizing proceeds using the method
            // specified in the zoneHVAC object
            // N1 , \field Maximum Supply Air Flow Rate
            FieldNum = 1;
            PrintFlag = true;
            SizingString = state.dataVentilatedSlab->VentSlabNumericFields(Item).FieldNames(FieldNum) + " [m3/s]";
            TempSize = ventSlab.MaxAirVolFlow;
            SystemAirFlowSizer sizerSystemAirFlow;
            sizerSystemAirFlow.overrideSizingString(SizingString);
            // sizerSystemAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            ventSlab.MaxAirVolFlow = sizerSystemAirFlow.size(state, TempSize, ErrorsFound);
        }

        bool IsAutoSize = false;
        if (ventSlab.OutAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                if (ventSlab.OutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, cMO_VentilatedSlab, ventSlab.Name, "User-Specified Maximum Outdoor Air Flow Rate [m3/s]", ventSlab.OutAirVolFlow);
                }
            } else { // Autosize or hard-size with sizing run
                CheckZoneSizing(state, cMO_VentilatedSlab, ventSlab.Name);
                OutAirVolFlowDes = ventSlab.MaxAirVolFlow;
                if (IsAutoSize) {
                    ventSlab.OutAirVolFlow = OutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, cMO_VentilatedSlab, ventSlab.Name, "Design Size Maximum Outdoor Air Flow Rate [m3/s]", OutAirVolFlowDes);
                } else {
                    if (ventSlab.OutAirVolFlow > 0.0 && OutAirVolFlowDes > 0.0) {
                        OutAirVolFlowUser = ventSlab.OutAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     cMO_VentilatedSlab,
                                                     ventSlab.Name,
                                                     "Design Size Maximum Outdoor Air Flow Rate [m3/s]",
                                                     OutAirVolFlowDes,
                                                     "User-Specified Maximum Outdoor Air Flow Rate [m3/s]",
                                                     OutAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(OutAirVolFlowDes - OutAirVolFlowUser) / OutAirVolFlowUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeVentilatedSlab: Potential issue with equipment sizing for ZoneHVAC:VentilatedSlab = \"{}\".",
                                                   ventSlab.Name));
                                ShowContinueError(state, format("User-Specified Maximum Outdoor Air Flow Rate of {:.5R} [m3/s]", OutAirVolFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Maximum Outdoor Air Flow Rate of {:.5R} [m3/s]", OutAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (ventSlab.MinOutAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                if (ventSlab.MinOutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, cMO_VentilatedSlab, ventSlab.Name, "User-Specified Minimum Outdoor Air Flow Rate [m3/s]", ventSlab.MinOutAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, cMO_VentilatedSlab, ventSlab.Name);
                MinOutAirVolFlowDes = min(state.dataSize->FinalZoneSizing(CurZoneEqNum).MinOA, ventSlab.MaxAirVolFlow);
                if (MinOutAirVolFlowDes < HVAC::SmallAirVolFlow) {
                    MinOutAirVolFlowDes = 0.0;
                }
                if (IsAutoSize) {
                    ventSlab.MinOutAirVolFlow = MinOutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, cMO_VentilatedSlab, ventSlab.Name, "Design Size Minimum Outdoor Air Flow Rate [m3/s]", MinOutAirVolFlowDes);
                } else { // Hard-size with sizing data
                    if (ventSlab.MinOutAirVolFlow > 0.0 && MinOutAirVolFlowDes > 0.0) {
                        MinOutAirVolFlowUser = ventSlab.MinOutAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     cMO_VentilatedSlab,
                                                     ventSlab.Name,
                                                     "Design Size Minimum Outdoor Air Flow Rate [m3/s]",
                                                     MinOutAirVolFlowDes,
                                                     "User-Specified Minimum Outdoor Air Flow Rate [m3/s]",
                                                     MinOutAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MinOutAirVolFlowDes - MinOutAirVolFlowUser) / MinOutAirVolFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeVentilatedSlab: Potential issue with equipment sizing for ZoneHVAC:VentilatedSlab = \"{}\".",
                                                   ventSlab.Name));
                                ShowContinueError(state,
                                                  format("User-Specified Minimum Outdoor Air Flow Rate of {:.5R} [m3/s]", MinOutAirVolFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Minimum Outdoor Air Flow Rate of {:.5R} [m3/s]", MinOutAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (ventSlab.MaxVolHotWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (ventSlab.hCoilType == HeatingCoilType::Water) {

            if (CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                    if (ventSlab.MaxVolHotWaterFlow > 0.0) {
                        BaseSizer::reportSizerOutput(
                            state, cMO_VentilatedSlab, ventSlab.Name, "User-Specified Maximum Hot Water Flow [m3/s]", ventSlab.MaxVolHotWaterFlow);
                    }
                } else { // Autosize or hard-size with sizing run
                    CheckZoneSizing(state, cMO_VentilatedSlab, ventSlab.Name);

                    int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", ventSlab.heatingCoilName, ErrorsFound);
                    int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, "Coil:Heating:Water", ventSlab.heatingCoilName, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(
                            state, "Coil:Heating:Water", ventSlab.heatingCoilName, CoilWaterInletNode, CoilWaterOutletNode, ErrorsFound);
                        int CoilNum = WaterCoils::GetWaterCoilIndex(state, "COIL:HEATING:WATER", ventSlab.heatingCoilName, ErrorsFound);
                        if (state.dataWaterCoils->WaterCoil(CoilNum).UseDesignWaterDeltaTemp) {
                            WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(CoilNum).DesignWaterDeltaTemp;
                            DoWaterCoilSizing = true;
                        } else {
                            if (PltSizHeatNum > 0) {
                                WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizHeatNum).DeltaT;
                                DoWaterCoilSizing = true;
                            } else {
                                DoWaterCoilSizing = false;
                                // If there is no heating Plant Sizing object and autosizing was requested, issue fatal error message
                                ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                                ShowContinueError(state, format("Occurs in {} Object={}", cMO_VentilatedSlab, ventSlab.Name));
                                ErrorsFound = true;
                            }
                        }
                        if (DoWaterCoilSizing) {
                            if (state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatMassFlow >= HVAC::SmallAirVolFlow) {
                                auto &zoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
                                SizingMethod = HVAC::HeatingCapacitySizing;
                                if (ventSlab.HVACSizingIndex > 0) {
                                    zoneHVACIndex = ventSlab.HVACSizingIndex;
                                    int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
                                    zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                        CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                        if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                                            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity > 0.0) {
                                                zoneEqSizing.HeatingCapacity = true;
                                                zoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                            } else {
                                                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                                            }
                                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                            zoneEqSizing.HeatingCapacity = true;
                                            zoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity *
                                                                          state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                            state.dataSize->DataFracOfAutosizedHeatingCapacity =
                                                state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                                            TempSize = DataSizing::AutoSize;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        }
                                    }
                                    SizingString = "";
                                    PrintFlag = false;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.overrideSizingString(SizingString);
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                                    state.dataSize->DataScalableCapSizingON = false;
                                } else {
                                    SizingString = "";
                                    PrintFlag = false;
                                    TempSize = DataSizing::AutoSize;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.overrideSizingString(SizingString);
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                                }
                                rho = state.dataPlnt->PlantLoop(ventSlab.HWPlantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                                Cp = state.dataPlnt->PlantLoop(ventSlab.HWPlantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                                MaxVolHotWaterFlowDes = DesCoilLoad / (WaterCoilSizDeltaT * Cp * rho);
                            } else {
                                MaxVolHotWaterFlowDes = 0.0;
                            }
                        }
                        ventSlab.MaxVolHotWaterFlow = MaxVolHotWaterFlowDes;
                        BaseSizer::reportSizerOutput(
                            state, cMO_VentilatedSlab, ventSlab.Name, "Design Size Maximum Hot Water Flow [m3/s]", MaxVolHotWaterFlowDes);
                    } else { // Hard-size with sizing data
                        if (ventSlab.MaxVolHotWaterFlow > 0.0 && MaxVolHotWaterFlowDes > 0.0) {
                            MaxVolHotWaterFlowUser = ventSlab.MaxVolHotWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         cMO_VentilatedSlab,
                                                         ventSlab.Name,
                                                         "Design Size Maximum Hot Water Flow [m3/s]",
                                                         MaxVolHotWaterFlowDes,
                                                         "User-Specified Maximum Hot Water Flow [m3/s]",
                                                         MaxVolHotWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotWaterFlowDes - MaxVolHotWaterFlowUser) / MaxVolHotWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(
                                        state,
                                        format("SizeVentilatedSlab: Potential issue with equipment sizing for ZoneHVAC:VentilatedSlab = \"{}\".",
                                               ventSlab.Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Hot Water Flow of {:.5R} [m3/s]", MaxVolHotWaterFlowUser));
                                    ShowContinueError(
                                        state, format("differs from Design Size Maximum Hot Water Flow of {:.5R} [m3/s]", MaxVolHotWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            ventSlab.MaxVolHotWaterFlow = 0.0;
        }

        IsAutoSize = false;
        if (ventSlab.MaxVolHotSteamFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (ventSlab.hCoilType == HeatingCoilType::Steam) {

            if (CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                    if (ventSlab.MaxVolHotSteamFlow > 0.0) {
                        BaseSizer::reportSizerOutput(
                            state, cMO_VentilatedSlab, ventSlab.Name, "User-Specified Maximum Steam Flow [m3/s]", ventSlab.MaxVolHotSteamFlow);
                    }
                } else { // Autosize or hard-size with sizing run
                    CheckZoneSizing(state, "ZoneHVAC:VentilatedSlab", ventSlab.Name);

                    int CoilSteamInletNode = SteamCoils::GetCoilSteamInletNode(state, "Coil:Heating:Steam", ventSlab.heatingCoilName, ErrorsFound);
                    int CoilSteamOutletNode = SteamCoils::GetCoilSteamOutletNode(state, "Coil:Heating:Steam", ventSlab.heatingCoilName, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(
                            state, "Coil:Heating:Steam", ventSlab.heatingCoilName, CoilSteamInletNode, CoilSteamOutletNode, ErrorsFound);
                        if (PltSizHeatNum > 0) {
                            if (state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatMassFlow >= HVAC::SmallAirVolFlow) {
                                SizingMethod = HVAC::HeatingCapacitySizing;
                                if (ventSlab.HVACSizingIndex > 0) {
                                    auto &zoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
                                    zoneHVACIndex = ventSlab.HVACSizingIndex;
                                    int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
                                    zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                        CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                        if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                                            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity > 0.0) {
                                                zoneEqSizing.HeatingCapacity = true;
                                                zoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                            } else {
                                                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                                            }
                                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                            zoneEqSizing.HeatingCapacity = true;
                                            zoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity *
                                                                          state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                            state.dataSize->DataFracOfAutosizedHeatingCapacity =
                                                state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                                            TempSize = DataSizing::AutoSize;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        }
                                    }
                                    SizingString = "";
                                    PrintFlag = false;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.overrideSizingString(SizingString);
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                                    state.dataSize->DataScalableCapSizingON = false;
                                } else {
                                    SizingString = "";
                                    PrintFlag = false;
                                    TempSize = DataSizing::AutoSize;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.overrideSizingString(SizingString);
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                                }
                                TempSteamIn = 100.00;
                                auto *steam = Fluid::GetSteam(state);
                                EnthSteamInDry = steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                                EnthSteamOutWet = steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                                LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
                                SteamDensity = steam->getSatDensity(state, TempSteamIn, 1.0, RoutineName);

                                auto *water = Fluid::GetWater(state);
                                Cp = water->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                                rho = water->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                                MaxVolHotSteamFlowDes =
                                    DesCoilLoad / ((state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho) + SteamDensity * LatentHeatSteam);
                            } else {
                                MaxVolHotSteamFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of Steam flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state, format("Occurs in ZoneHVAC:VentilatedSlab Object={}", ventSlab.Name));
                            ErrorsFound = true;
                        }
                        ventSlab.MaxVolHotSteamFlow = MaxVolHotSteamFlowDes;
                        BaseSizer::reportSizerOutput(
                            state, cMO_VentilatedSlab, ventSlab.Name, "Design Size Maximum Steam Flow [m3/s]", MaxVolHotSteamFlowDes);
                    } else {
                        if (ventSlab.MaxVolHotSteamFlow > 0.0 && MaxVolHotSteamFlowDes > 0.0) {
                            MaxVolHotSteamFlowUser = ventSlab.MaxVolHotSteamFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         cMO_VentilatedSlab,
                                                         ventSlab.Name,
                                                         "Design Size Maximum Steam Flow [m3/s]",
                                                         MaxVolHotSteamFlowDes,
                                                         "User-Specified Maximum Steam Flow [m3/s]",
                                                         MaxVolHotSteamFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotSteamFlowDes - MaxVolHotSteamFlowUser) / MaxVolHotSteamFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(
                                        state,
                                        format("SizeVentilatedSlab: Potential issue with equipment sizing for ZoneHVAC:VentilatedSlab = \"{}\".",
                                               ventSlab.Name));
                                    ShowContinueError(state, format("User-Specified Maximum Steam Flow of {:.5R} [m3/s]", MaxVolHotSteamFlowUser));
                                    ShowContinueError(state,
                                                      format("differs from Design Size Maximum Steam Flow of {:.5R} [m3/s]", MaxVolHotSteamFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            ventSlab.MaxVolHotSteamFlow = 0.0;
        }

        IsAutoSize = false;
        if (ventSlab.MaxVolColdWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                if (ventSlab.MaxVolColdWaterFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, cMO_VentilatedSlab, ventSlab.Name, "User-Specified Maximum Cold Water Flow [m3/s]", ventSlab.MaxVolColdWaterFlow);
                }
            } else {
                CheckZoneSizing(state, cMO_VentilatedSlab, ventSlab.Name);
                if (ventSlab.cCoilType == CoolingCoilType::HXAssisted) {
                    CoolingCoilName =
                        HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, ventSlab.coolingCoilTypeCh, ventSlab.coolingCoilName, ErrorsFound);
                    CoolingCoilType =
                        HVACHXAssistedCoolingCoil::GetHXCoilType(state, ventSlab.coolingCoilTypeCh, ventSlab.coolingCoilName, ErrorsFound);
                } else {
                    CoolingCoilName = ventSlab.coolingCoilName;
                    CoolingCoilType = ventSlab.coolingCoilTypeCh;
                }
                int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                if (IsAutoSize) {
                    int PltSizCoolNum = PlantUtilities::MyPlantSizingIndex(
                        state, CoolingCoilType, CoolingCoilName, CoilWaterInletNode, CoilWaterOutletNode, ErrorsFound);
                    int CoilNum = WaterCoils::GetWaterCoilIndex(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                    if (state.dataWaterCoils->WaterCoil(CoilNum).UseDesignWaterDeltaTemp) {
                        WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(CoilNum).DesignWaterDeltaTemp;
                        DoWaterCoilSizing = true;
                    } else {
                        if (PltSizCoolNum > 0) {
                            WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizCoolNum).DeltaT;
                            DoWaterCoilSizing = true;
                        } else {
                            DoWaterCoilSizing = false;
                            // If there is no cooling Plant Sizing object and autosizing was requested, issue fatal error message
                            ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                            ShowContinueError(state, format("Occurs in {} Object={}", cMO_VentilatedSlab, ventSlab.Name));
                            ErrorsFound = true;
                        }
                    }
                    if (DoWaterCoilSizing) {
                        if (state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolMassFlow >= HVAC::SmallAirVolFlow) {
                            SizingMethod = HVAC::CoolingCapacitySizing;
                            if (ventSlab.HVACSizingIndex > 0) {
                                auto &zoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
                                zoneHVACIndex = ventSlab.HVACSizingIndex;
                                int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod;
                                zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                if (CapSizingMethod == DataSizing::CoolingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                    CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                                    if (CapSizingMethod == DataSizing::CoolingDesignCapacity) {
                                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity > 0.0) {
                                            zoneEqSizing.CoolingCapacity = true;
                                            zoneEqSizing.DesCoolingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                                        } else {
                                            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                                        }
                                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                                    } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                        zoneEqSizing.CoolingCapacity = true;
                                        zoneEqSizing.DesCoolingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity *
                                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    } else if (CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                                        state.dataSize->DataFracOfAutosizedHeatingCapacity =
                                            state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                                        state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                                        TempSize = DataSizing::AutoSize;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    }
                                }
                                SizingString = "";
                                PrintFlag = false;
                                CoolingCapacitySizer sizerCoolingCapacity;
                                sizerCoolingCapacity.overrideSizingString(SizingString);
                                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                                state.dataSize->DataScalableCapSizingON = false;
                            } else {
                                SizingString = "";
                                PrintFlag = false;
                                TempSize = DataSizing::AutoSize;
                                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                                CoolingCapacitySizer sizerCoolingCapacity;
                                sizerCoolingCapacity.overrideSizingString(SizingString);
                                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                            }
                            rho = state.dataPlnt->PlantLoop(ventSlab.CWPlantLoc.loopNum).glycol->getDensity(state, 5., RoutineName);
                            Cp = state.dataPlnt->PlantLoop(ventSlab.CWPlantLoc.loopNum).glycol->getSpecificHeat(state, 5., RoutineName);
                            MaxVolColdWaterFlowDes = DesCoilLoad / (WaterCoilSizDeltaT * Cp * rho);
                        } else {
                            MaxVolColdWaterFlowDes = 0.0;
                        }
                    }
                    ventSlab.MaxVolColdWaterFlow = MaxVolColdWaterFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, cMO_VentilatedSlab, ventSlab.Name, "Design Size Maximum Cold Water Flow [m3/s]", MaxVolColdWaterFlowDes);
                } else {
                    if (ventSlab.MaxVolColdWaterFlow > 0.0 && MaxVolColdWaterFlowDes > 0.0) {
                        MaxVolColdWaterFlowUser = ventSlab.MaxVolColdWaterFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     cMO_VentilatedSlab,
                                                     ventSlab.Name,
                                                     "Design Size Maximum Cold Water Flow [m3/s]",
                                                     MaxVolColdWaterFlowDes,
                                                     "User-Specified Maximum Cold Water Flow [m3/s]",
                                                     MaxVolColdWaterFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxVolColdWaterFlowDes - MaxVolColdWaterFlowUser) / MaxVolColdWaterFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeVentilatedSlab: Potential issue with equipment sizing for ZoneHVAC:VentilatedSlab = \"{}\".",
                                                   ventSlab.Name));
                                ShowContinueError(state, format("User-Specified Maximum Cold Water Flow of {:.5R} [m3/s]", MaxVolColdWaterFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Maximum Cold Water Flow of {:.5R} [m3/s]", MaxVolColdWaterFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        if (ventSlab.cCoilType == CoolingCoilType::HXAssisted) {
            CoolingCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, ventSlab.coolingCoilTypeCh, ventSlab.coolingCoilName, ErrorsFound);
            CoolingCoilType = HVACHXAssistedCoolingCoil::GetHXCoilType(state, ventSlab.coolingCoilTypeCh, ventSlab.coolingCoilName, ErrorsFound);
        } else {
            CoolingCoilName = ventSlab.coolingCoilName;
            CoolingCoilType = ventSlab.coolingCoilTypeCh;
        }
        WaterCoils::SetCoilDesFlow(state, CoolingCoilType, CoolingCoilName, ventSlab.MaxAirVolFlow, ErrorsFound);
        WaterCoils::SetCoilDesFlow(state, ventSlab.heatingCoilTypeCh, ventSlab.heatingCoilName, ventSlab.MaxAirVolFlow, ErrorsFound);

        if (CurZoneEqNum > 0) {
            auto &zoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
            zoneEqSizing.MaxHWVolFlow = ventSlab.MaxVolHotWaterFlow;
            zoneEqSizing.MaxCWVolFlow = ventSlab.MaxVolColdWaterFlow;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void CalcVentilatedSlab(EnergyPlusData &state,
                            int &Item,                     // number of the current ventilated slab being simulated
                            int const ZoneNum,             // number of zone being served
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,              // power supplied (W)
                            Real64 &LatOutputProvided      // latent capacity supplied (kg/s)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)
        //                      July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine mainly controls the action of the Ventilated Slab
        // (or more exactly, it controls the amount of outside air brought in)
        // based on the user input for controls and the defined controls
        // algorithms.

        // METHODOLOGY EMPLOYED:
        // Ventilated slab is controlled based on user input and what is happening in the
        // simulation.  There are various cases to consider:
        // 1. OFF: Unit is schedule off or there is no load on it.  All flow
        //    rates are set to zero and the temperatures are set to zone conditions
        //    (except for the outside air inlet).
        // 2. HEATING/VARIABLE PERCENT: The unit is on, there is a heating load,
        //    and variable percent control is specified.  The outside air fraction
        //    is set to the minimum outside air fraction (schedule based) and the
        //    heating coil is activated.
        // 3. HEATING/FIXED TEMPERATURE: The unit is on, there is a heating load,
        //    and fixed temperature control is specified.  The outside air fraction
        //    is varied in an attempt to obtain a mixed air temperature equal to
        //    the user specified temperature (schedule based).  The heating coil
        //    is activated, if necessary.
        // 4. COOLING/NO COIL: The unit is on, there is a cooling load, and no
        //    coil is present or it has been scheduled off.  Set the amount of
        //    outside air based on the control type.  Simulate the "mixing box".
        // 5. COOLING/WITH COIL: The unit is on, there is a cooling load, and
        //    a cooling coil is present and scheduled on.  Tries to use outside
        //    air as best as possible and then calls a cooling coil
        // Note: controls are strictly temperature based and do not factor
        // humidity into the equation (not an enthalpy economy cycle but rather
        // a simple return air economy cycle).  In addition, temperature predictions
        // are not strict energy balances here in the control routine though
        // in the mixing routine an energy balance is preserved.

        // REFERENCES:
        // ASHRAE Systems and Equipment Handbook (SI), 1996. page 31.3

        // Using/Aliasing
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr LowTempDiff(0.1); // Smallest allowed temperature difference for comparisons
        // (below this value the temperatures are assumed equal)
        Real64 constexpr LowOAFracDiff(0.01); // Smallest allowed outside air fraction difference for comparison
        // (below this value the fractions are assumed equal)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QZnReq;
        Real64 AirMassFlow;  // air mass flow rate [kg/sec]
        int AirRelNode;      // outside air relief node
        int ControlNode;     // the hot water or cold water inlet node
        int InletNode;       // system air inlet node
        int FanOutletNode;   // system fan outlet node
        int ZoneAirInNode;   // zone supply air node
        Real64 MaxOAFrac;    // maximum possible outside air fraction
        Real64 MaxWaterFlow; // maximum water flow for heating or cooling [kg/sec]
        Real64 MinOAFrac;    // minimum possible outside air fraction
        Real64 MinWaterFlow; // minimum water flow for heating or cooling [kg/sec]
        int OutletNode;      // air outlet node
        int OutsideAirNode;  // outside air node
        int MixoutNode;      // oa mixer outlet node
        int ReturnAirNode;   // return air node
        Real64 QUnitOut;     // heating or sens. cooling provided by fan coil unit [watts]
        Real64 LatentOutput; // Latent (moisture) add/removal rate, negative is dehumidification [kg/s]
        Real64 Tdesired;     // desired temperature after mixing inlet and outdoor air [degrees C]
        Real64 Tinlet;       // temperature of air coming into the ventilated slab [degrees C]
        Real64 Toutdoor;     // temperature of outdoor air being introduced into the ventilated slab [degrees C]
        Real64 MaxSteamFlow;
        Real64 MinSteamFlow;
        Real64 RadInTemp;      // "Desired" radiant system air inlet temperature [Celsius]**setpoint
        Real64 SetPointTemp;   // temperature that will be used to control the radiant system [Celsius]
        Real64 SetPointTempHi; // Current high point in setpoint temperature range
        Real64 SetPointTempLo; // Current low point in setpoint temperature range
        Real64 AirTempHi;      // Current high point in water temperature range
        Real64 AirTempLo;      // Current low point in water temperature range
        Real64 AirTempHeatHi;  // Current high point in water temperature range
        Real64 AirTempCoolLo;  // Current low point in water temperature range
        Real64 CpFan;          // Intermediate calculational variable for specific heat of air <<NOV9 Updated
        Real64 ZoneRadNum;     // number of zone being served *********************
        int RadSurfNum;        // DO loop counter for the surfaces that comprise a particular radiant system
        static std::string const CurrentModuleObject("ZoneHVAC:VentilatedSlab");

        switch (ventSlab.coilOption) {
        case CoilType::Both: {

            switch (ventSlab.hCoilType) {
            case HeatingCoilType::Water: {
                WaterCoils::CheckWaterCoilSchedule(state, ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Steam: {
                SteamCoils::CheckSteamCoilSchedule(
                    state, "Coil:Heating:Steam", ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Electric: {
                HeatingCoils::CheckHeatingCoilSchedule(
                    state, "Coil:Heating:Electric", ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Gas: {
                HeatingCoils::CheckHeatingCoilSchedule(
                    state, "Coil:Heating:Fuel", ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            default:
                break;
            }

            switch (ventSlab.cCoilType) {
            case CoolingCoilType::WaterCooling:
            case CoolingCoilType::DetailedCooling: {
                WaterCoils::CheckWaterCoilSchedule(state, ventSlab.coolingCoilName, ventSlab.coolingCoilSchedValue, ventSlab.coolingCoil_Index);
                break;
            }
            case CoolingCoilType::HXAssisted: {
                HVACHXAssistedCoolingCoil::CheckHXAssistedCoolingCoilSchedule(state,
                                                                              "CoilSystem:Cooling:Water:HeatExchangerAssisted",
                                                                              ventSlab.coolingCoilName,
                                                                              ventSlab.coolingCoilSchedValue,
                                                                              ventSlab.coolingCoil_Index);
                break;
            }
            default:
                break;
            }
            break;
        }
        case CoilType::Heating: {

            switch (ventSlab.hCoilType) {
            case HeatingCoilType::Water: {
                WaterCoils::CheckWaterCoilSchedule(state, ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Steam: {
                SteamCoils::CheckSteamCoilSchedule(
                    state, "Coil:Heating:Steam", ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Electric: {
                HeatingCoils::CheckHeatingCoilSchedule(
                    state, "Coil:Heating:Electric", ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Gas: {
                HeatingCoils::CheckHeatingCoilSchedule(
                    state, "Coil:Heating:Fuel", ventSlab.heatingCoilName, ventSlab.heatingCoilSchedValue, ventSlab.heatingCoil_Index);
                break;
            }
            default:
                break;
            }
            break;
        }
        case CoilType::Cooling: {

            switch (ventSlab.cCoilType) {
            case CoolingCoilType::WaterCooling:
            case CoolingCoilType::DetailedCooling: {
                WaterCoils::CheckWaterCoilSchedule(state, ventSlab.coolingCoilName, ventSlab.coolingCoilSchedValue, ventSlab.coolingCoil_Index);
                break;
            }
            case CoolingCoilType::HXAssisted: {
                HVACHXAssistedCoolingCoil::CheckHXAssistedCoolingCoilSchedule(state,
                                                                              "CoilSystem:Cooling:Water:HeatExchangerAssisted",
                                                                              ventSlab.coolingCoilName,
                                                                              ventSlab.coolingCoilSchedValue,
                                                                              ventSlab.coolingCoil_Index);
                break;
            }
            default:
                break;
            }
        }
        case CoilType::None:
        default:
            break;
        }

        // initialize local variables
        ControlNode = 0;
        QUnitOut = 0.0;
        LatentOutput = 0.0;
        MaxWaterFlow = 0.0;
        MinWaterFlow = 0.0;
        AirMassFlow = 0.0;
        InletNode = ventSlab.ReturnAirNode;
        OutletNode = ventSlab.RadInNode;
        FanOutletNode = ventSlab.FanOutletNode;
        ZoneAirInNode = ventSlab.ZoneAirInNode;
        OutsideAirNode = ventSlab.OutsideAirNode;
        AirRelNode = ventSlab.AirReliefNode;
        MixoutNode = ventSlab.OAMixerOutNode;
        ReturnAirNode = ventSlab.ReturnAirNode;
        ZoneRadNum = ventSlab.ZonePtr;
        RadSurfNum = ventSlab.NumOfSurfaces;
        Tinlet = state.dataLoopNodes->Node(InletNode).Temp;
        Toutdoor = state.dataLoopNodes->Node(OutsideAirNode).Temp;
        auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        // Control Type Check
        switch (ventSlab.controlType) {
        case ControlType::MeanAirTemp: {
            SetPointTemp = thisZoneHB.MAT;
            break;
        }
        case ControlType::MeanRadTemp: {
            SetPointTemp = thisZoneHB.MRT;
            break;
        }
        case ControlType::OperativeTemp: {
            SetPointTemp = 0.5 * (thisZoneHB.MAT + thisZoneHB.MRT);
            break;
        }
        case ControlType::OutdoorDryBulbTemp: {
            SetPointTemp = state.dataEnvrn->OutDryBulbTemp;
            break;
        }
        case ControlType::OutdoorWetBulbTemp: {
            SetPointTemp = state.dataEnvrn->OutWetBulbTemp;
            break;
        }
        case ControlType::SurfaceTemp: {
            SetPointTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(RadSurfNum));
            break;
        }
        case ControlType::DewPointTemp: {
            SetPointTemp = Psychrometrics::PsyTdpFnWPb(
                state, state.dataZoneTempPredictorCorrector->zoneHeatBalance(ventSlab.ZonePtr).airHumRat, state.dataEnvrn->OutBaroPress);
            break;
        }
        default: {              // Should never get here
            SetPointTemp = 0.0; // Suppress uninitialized warning
            ShowSevereError(state, format("Illegal control type in low temperature radiant system: {}", ventSlab.Name));
            ShowFatalError(state, "Preceding condition causes termination.");
        } break;
        } // switch (ctrlType)

        // Load Check

        AirTempHeatHi = ventSlab.hotCtrlHiTempSched->getCurrentVal();
        AirTempCoolLo = ventSlab.coldCtrlLoTempSched->getCurrentVal();

        if (((SetPointTemp >= AirTempHeatHi) && (SetPointTemp <= AirTempCoolLo)) || (ventSlab.availSched->getCurrentVal() <= 0)) {

            // System is off or has no load upon it; set the flow rates to zero and then
            // simulate the components with the no flow conditions
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(ReturnAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(ReturnAirNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(ReturnAirNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(MixoutNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(MixoutNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRateMinAvail = 0.0;
            AirMassFlow = 0.0;
            state.dataVentilatedSlab->HCoilOn = false;

            // Node condition
            state.dataLoopNodes->Node(InletNode).Temp = state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(1));
            state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(InletNode).Temp;
            state.dataLoopNodes->Node(OutletNode).Temp = state.dataLoopNodes->Node(FanOutletNode).Temp;

            // Node condition
            if (ventSlab.SysConfg == VentilatedSlabConfig::SeriesSlabs) {
                bool ErrorsFound = false; // Set to true if errors in input, fatal at end of routine
                for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
                    std::string SlabName = ventSlab.SurfaceName(RadSurfNum);
                    std::string MSlabIn = ventSlab.SlabIn(RadSurfNum);
                    std::string MSlabOut = ventSlab.SlabOut(RadSurfNum);
                    ventSlab.MSlabInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                               MSlabIn,
                                                                               ErrorsFound,
                                                                               DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                               SlabName,
                                                                               DataLoopNode::NodeFluidType::Air,
                                                                               DataLoopNode::ConnectionType::Internal,
                                                                               NodeInputManager::CompFluidStream::Primary,
                                                                               DataLoopNode::ObjectIsNotParent);
                    ventSlab.MSlabOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                MSlabOut,
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                                SlabName,
                                                                                DataLoopNode::NodeFluidType::Air,
                                                                                DataLoopNode::ConnectionType::Internal,
                                                                                NodeInputManager::CompFluidStream::Primary,
                                                                                DataLoopNode::ObjectIsNotParent);
                    int MSlabInletNode = ventSlab.MSlabInNode;
                    int MSlabOutletNode = ventSlab.MSlabOutNode;

                    state.dataLoopNodes->Node(MSlabInletNode).Temp = state.dataLoopNodes->Node(InletNode).Temp;
                    state.dataLoopNodes->Node(MSlabOutletNode).Temp = state.dataLoopNodes->Node(MSlabInletNode).Temp;
                }
            }

            CalcVentilatedSlabComps(state, Item, FirstHVACIteration, QUnitOut);

        } else { // System On

            if (SetPointTemp < AirTempHeatHi) { // HEATING MODE
                state.dataVentilatedSlab->OperatingMode = HeatingMode;

                // Check the setpoint and temperature span
                SetPointTempHi = ventSlab.hotCtrlHiTempSched->getCurrentVal();
                SetPointTempLo = ventSlab.hotCtrlLoTempSched->getCurrentVal();
                if (SetPointTempHi < SetPointTempLo) {
                    ShowSevereError(state, format("Heating setpoint temperature mismatch in{}", ventSlab.Name));
                    ShowContinueError(state, "High setpoint temperature is less than low setpoint temperature--check your schedule input");
                    ShowFatalError(state, "Preceding condition causes termination.");
                }

                AirTempHi = ventSlab.hotAirHiTempSched->getCurrentVal();
                AirTempLo = ventSlab.hotAirLoTempSched->getCurrentVal();

                if (AirTempHi < AirTempLo) {
                    ShowSevereError(state, format("Heating Air temperature mismatch in{}", ventSlab.Name));
                    ShowContinueError(state, "High Air temperature is less than low Air temperature--check your schedule input");
                    ShowFatalError(state, "Preceding condition causes termination.");
                }

                if (SetPointTemp >= SetPointTempHi) {
                    // System is above high heating setpoint so we should be able to turn the system off
                    RadInTemp = AirTempLo;

                } else if (SetPointTemp <= SetPointTempLo) {
                    // System is running with its highest inlet temperature
                    RadInTemp = AirTempHi;
                } else {
                    // Interpolate to obtain the current radiant system inlet temperature
                    RadInTemp = AirTempHi - (AirTempHi - AirTempLo) * (SetPointTemp - SetPointTempLo) / (SetPointTempHi - SetPointTempLo);
                }

                state.dataLoopNodes->Node(ventSlab.RadInNode).Temp = RadInTemp;

                ControlNode = ventSlab.HotControlNode;
                MaxWaterFlow = ventSlab.MaxHotWaterFlow;
                MinWaterFlow = ventSlab.MinHotWaterFlow;
                MaxSteamFlow = ventSlab.MaxHotSteamFlow;
                MinSteamFlow = ventSlab.MinHotSteamFlow;

                // On the first HVAC iteration the system values are given to the controller, but after that
                // the demand limits are in place and there needs to be feedback to the Zone Equipment

                if (!FirstHVACIteration && ventSlab.hCoilType == HeatingCoilType::Water) {
                    MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                    MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                }

                if (!FirstHVACIteration && ventSlab.hCoilType == HeatingCoilType::Steam) {
                    MaxSteamFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                    MinSteamFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                }

                state.dataVentilatedSlab->HCoilOn = true;

                if (state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate > 0.0) {
                    MinOAFrac =
                        ventSlab.minOASched->getCurrentVal() * (ventSlab.MinOutAirMassFlow / state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate);
                } else {
                    MinOAFrac = 0.0;
                }

                MinOAFrac = min(1.0, max(0.0, MinOAFrac));

                if ((!ventSlab.heatingCoilPresent) || (ventSlab.heatingCoilSchedValue <= 0.0)) {
                    // In heating mode, but there is no coil to provide heating.  This is handled
                    // differently than if there was a heating coil present.  Fixed temperature
                    // will still try to vary the amount of outside air to meet the desired
                    // mixed air temperature, while variable percent will go to full ventilation
                    // when it is most advantageous.

                    // If there are no coil, Slab In Node is assumed to be Fan Outlet Node

                    OutletNode = FanOutletNode;

                    switch (ventSlab.outsideAirControlType) {
                    case OutsideAirControlType::FixedOAControl: {
                        // In this control type, the outdoor air flow rate is fixed to the maximum value
                        // which is equal to the minimum value, regardless of all the other conditions.
                        state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        break;
                    }
                    case OutsideAirControlType::VariablePercent: {
                        // This algorithm is probably a bit simplistic in that it just bounces
                        // back and forth between the maximum outside air and the minimum.  In
                        // reality, a system *might* vary between the two based on the load in
                        // the zone.  This simple flow control might cause some overcooling but
                        // chances are that if there is a cooling load and the zone temperature
                        // gets above the outside temperature that overcooling won't be significant.
                        Tinlet = state.dataLoopNodes->Node(InletNode).Temp;
                        Toutdoor = state.dataLoopNodes->Node(OutsideAirNode).Temp;

                        if (Tinlet >= Toutdoor) {

                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;

                        } else { // Tinlet < Toutdoor
                            MaxOAFrac = ventSlab.maxOASched->getCurrentVal();
                            state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        }
                        break;
                    }
                    case OutsideAirControlType::FixedTemperature: {
                        // This is basically the same algorithm as for the heating case...
                        Tdesired = ventSlab.tempSched->getCurrentVal();
                        MaxOAFrac = 1.0;

                        if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                            // Desired temperature is between the inlet and outdoor temperatures
                            // so vary the flow rate between no outside air and no recirculation air
                            // then applying the maximum and minimum limits the user has scheduled
                            // to make sure too much/little outside air is being introduced
                            state.dataVentilatedSlab->OAMassFlowRate =
                                ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * state.dataLoopNodes->Node(InletNode).MassFlowRate;
                            state.dataVentilatedSlab->OAMassFlowRate =
                                max(state.dataVentilatedSlab->OAMassFlowRate, (MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                            state.dataVentilatedSlab->OAMassFlowRate =
                                min(state.dataVentilatedSlab->OAMassFlowRate, (MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                        } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                            // Desired temperature is below both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                            // Desired temperature is above both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else {
                            // It should NEVER get to this point, but just in case...
                            ShowFatalError(state, format("Ventilated Slab simulation control: illogical condition for {}", ventSlab.Name));
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    CalcVentilatedSlabComps(state, Item, FirstHVACIteration, QUnitOut);

                } else { // Heating Coil present

                    switch (ventSlab.outsideAirControlType) {
                    case OutsideAirControlType::FixedOAControl: {
                        // In this control type, the outdoor air flow rate is fixed to the maximum value
                        // which is equal to the minimum value, regardless of all the other conditions.
                        if (state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate > 0.0) {
                            MaxOAFrac = min(1.0, max(0.0, ventSlab.maxOASched->getCurrentVal()));
                        } else {
                            MaxOAFrac = 0.0;
                        }
                        state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        break;
                    }
                    case OutsideAirControlType::VariablePercent: {
                        // In heating mode, the ouside air for "variable percent" control
                        // is set to the minimum value

                        state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        break;
                    }
                    case OutsideAirControlType::FixedTemperature: {
                        // This is basically the same algorithm as for the heating case...
                        Tdesired = ventSlab.tempSched->getCurrentVal();
                        MaxOAFrac = 1.0;

                        if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                            // Desired temperature is between the inlet and outdoor temperatures
                            // so vary the flow rate between no outside air and no recirculation air
                            // then applying the maximum and minimum limits the user has scheduled
                            // to make sure too much/little outside air is being introduced
                            state.dataVentilatedSlab->OAMassFlowRate =
                                ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * state.dataLoopNodes->Node(InletNode).MassFlowRate;
                            state.dataVentilatedSlab->OAMassFlowRate =
                                max(state.dataVentilatedSlab->OAMassFlowRate, (MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                            state.dataVentilatedSlab->OAMassFlowRate =
                                min(state.dataVentilatedSlab->OAMassFlowRate, (MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                        } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                            // Desired temperature is below both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                            // Desired temperature is above both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else {
                            // It should NEVER get to this point, but just in case...
                            ShowFatalError(state, format("Ventilated Slab simulation control: illogical condition for {}", ventSlab.Name));
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    SimVentSlabOAMixer(state, Item);

                    state.dataFans->fans(ventSlab.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

                    CpFan = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(FanOutletNode).HumRat);
                    QZnReq =
                        (state.dataLoopNodes->Node(OutletNode).MassFlowRate) * CpFan * (RadInTemp - state.dataLoopNodes->Node(FanOutletNode).Temp);

                    // Setup the coil configuration
                    switch (ventSlab.hCoilType) {

                    case HeatingCoilType::Water: {
                        // control water flow to obtain output matching QZnReq

                        ControlCompOutput(state,
                                          ventSlab.Name,
                                          cMO_VentilatedSlab,
                                          Item,
                                          FirstHVACIteration,
                                          QZnReq,
                                          ControlNode,
                                          MaxWaterFlow,
                                          MinWaterFlow,
                                          0.001,
                                          ventSlab.ControlCompTypeNum,
                                          ventSlab.CompErrIndex,
                                          _,
                                          _,
                                          _,
                                          _,
                                          _,
                                          ventSlab.HWPlantLoc);
                        break;
                    }
                    case HeatingCoilType::Gas:
                    case HeatingCoilType::Electric:
                    case HeatingCoilType::Steam: {

                        CalcVentilatedSlabComps(state, Item, FirstHVACIteration, QUnitOut);
                        break;
                    }
                    default:
                        break;
                    }

                } //  Coil/no coil block

            } else if (SetPointTemp > AirTempCoolLo) { // Cooling Mode

                state.dataVentilatedSlab->OperatingMode = CoolingMode;
                SetPointTempHi = ventSlab.coldCtrlHiTempSched->getCurrentVal();
                SetPointTempLo = ventSlab.coldCtrlLoTempSched->getCurrentVal();
                if (SetPointTempHi < SetPointTempLo) {
                    ShowSevereError(state, format("Cooling setpoint temperature mismatch in{}", ventSlab.Name));
                    ShowContinueError(state, "High setpoint temperature is less than low setpoint temperature--check your schedule input");
                    ShowFatalError(state, "Preceding condition causes termination.");
                }

                AirTempHi = ventSlab.coldAirHiTempSched->getCurrentVal();
                AirTempLo = ventSlab.coldAirLoTempSched->getCurrentVal();
                if (AirTempHi < AirTempLo) {
                    ShowSevereError(state, format("Cooling Air temperature mismatch in{}", ventSlab.Name));
                    ShowContinueError(state, "High Air temperature is less than low Air temperature--check your schedule input");
                    ShowFatalError(state, "Preceding condition causes termination.");
                }

                if (SetPointTemp <= SetPointTempLo) {
                    // System is below low cooling setpoint so we should be able to turn the system off
                    RadInTemp = AirTempHi;
                } else if (SetPointTemp >= SetPointTempHi) {
                    // System is running with its lowest inlet temperature
                    RadInTemp = AirTempLo;
                } else {
                    // Interpolate to obtain the current radiant system inlet temperature
                    RadInTemp = AirTempHi - (AirTempHi - AirTempLo) * (SetPointTemp - SetPointTempLo) / (SetPointTempHi - SetPointTempLo);
                }

                ControlNode = ventSlab.ColdControlNode;
                MaxWaterFlow = ventSlab.MaxColdWaterFlow;
                MinWaterFlow = ventSlab.MinColdWaterFlow;

                // On the first HVAC iteration the system values are given to the controller, but after that
                // the demand limits are in place and there needs to be feedback to the Zone Equipment
                if ((!FirstHVACIteration) && (ControlNode > 0) && (ventSlab.coolingCoilPresent)) {
                    MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                    MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                }
                state.dataVentilatedSlab->HCoilOn = false;

                if (state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate > 0.0) {
                    MinOAFrac =
                        ventSlab.minOASched->getCurrentVal() * (ventSlab.MinOutAirMassFlow / state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate);
                } else {
                    MinOAFrac = 0.0;
                }
                MinOAFrac = min(1.0, max(0.0, MinOAFrac));

                if ((!ventSlab.coolingCoilPresent) || (ventSlab.coolingCoilSchedValue <= 0.0)) {
                    // In cooling mode, but there is no coil to provide cooling.  This is handled
                    // differently than if there was a cooling coil present.  Fixed temperature
                    // will still try to vary the amount of outside air to meet the desired
                    // mixed air temperature, while variable percent will go to full ventilation
                    // when it is most advantageous.

                    // If there are no coil, Slab In Node is assumed to be Fan Outlet Node
                    OutletNode = FanOutletNode;

                    switch (ventSlab.outsideAirControlType) {

                    case OutsideAirControlType::FixedOAControl: {
                        // In this control type, the outdoor air flow rate is fixed to the maximum value
                        // which is equal to the minimum value, regardless of all the other conditions.
                        if (state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate > 0.0) {
                            MaxOAFrac = min(1.0, max(0.0, ventSlab.maxOASched->getCurrentVal()));
                        } else {
                            MaxOAFrac = 0.0;
                        }
                        state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        break;
                    }
                    case OutsideAirControlType::VariablePercent: {
                        // This algorithm is probably a bit simplistic in that it just bounces
                        // back and forth between the maximum outside air and the minimum.  In
                        // reality, a system *might* vary between the two based on the load in
                        // the zone.  This simple flow control might cause some overcooling but
                        // chances are that if there is a cooling load and the zone temperature
                        // gets above the outside temperature that overcooling won't be significant.

                        Tinlet = state.dataLoopNodes->Node(InletNode).Temp;
                        Toutdoor = state.dataLoopNodes->Node(OutsideAirNode).Temp;

                        if (Tinlet <= Toutdoor) {

                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;

                        } else { // Tinlet > Toutdoor
                            MaxOAFrac = ventSlab.maxOASched->getCurrentVal();
                            state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        }
                        break;
                    }
                    case OutsideAirControlType::FixedTemperature: {
                        // This is basically the same algorithm as for the heating case...
                        Tdesired = ventSlab.tempSched->getCurrentVal();
                        MaxOAFrac = 1.0;

                        if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                            // Desired temperature is between the inlet and outdoor temperatures
                            // so vary the flow rate between no outside air and no recirculation air
                            // then applying the maximum and minimum limits the user has scheduled
                            // to make sure too much/little outside air is being introduced
                            state.dataVentilatedSlab->OAMassFlowRate =
                                ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * state.dataLoopNodes->Node(InletNode).MassFlowRate;
                            state.dataVentilatedSlab->OAMassFlowRate =
                                max(state.dataVentilatedSlab->OAMassFlowRate, (MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                            state.dataVentilatedSlab->OAMassFlowRate =
                                min(state.dataVentilatedSlab->OAMassFlowRate, (MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                        } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                            // Desired temperature is below both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                            // Desired temperature is above both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else {
                            // It should NEVER get to this point, but just in case...
                            ShowFatalError(state, format("{} simulation control: illogical condition for {}", cMO_VentilatedSlab, ventSlab.Name));
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    CalcVentilatedSlabComps(state, Item, FirstHVACIteration, QUnitOut);

                } else {
                    // There is a cooling load and there is a cooling coil present (presumably).
                    // Variable percent will throttle outside air back to the minimum while
                    // fixed temperature will still try to vary the outside air amount to meet
                    // the desired mixed air temperature.

                    switch (ventSlab.outsideAirControlType) {

                    case OutsideAirControlType::FixedOAControl: {
                        // In this control type, the outdoor air flow rate is fixed to the maximum value
                        // which is equal to the minimum value, regardless of all the other conditions.
                        if (state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate > 0.0) {
                            MaxOAFrac = min(1.0, max(0.0, ventSlab.maxOASched->getCurrentVal()));
                        } else {
                            MaxOAFrac = 0.0;
                        }
                        state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        break;
                    }
                    case OutsideAirControlType::VariablePercent: {
                        // A cooling coil is present so let it try to do the cooling...
                        state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        break;
                    }
                    case OutsideAirControlType::FixedTemperature: {
                        // This is basically the same algorithm as for the heating case...
                        Tdesired = ventSlab.tempSched->getCurrentVal();

                        MaxOAFrac = 1.0;

                        if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                            state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                        } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                            // Desired temperature is between the inlet and outdoor temperatures
                            // so vary the flow rate between no outside air and no recirculation air
                            // then applying the maximum and minimum limits the user has scheduled
                            // to make sure too much/little outside air is being introduced
                            state.dataVentilatedSlab->OAMassFlowRate =
                                ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * state.dataLoopNodes->Node(InletNode).MassFlowRate;
                            state.dataVentilatedSlab->OAMassFlowRate =
                                max(state.dataVentilatedSlab->OAMassFlowRate, (MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                            state.dataVentilatedSlab->OAMassFlowRate =
                                min(state.dataVentilatedSlab->OAMassFlowRate, (MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate));
                        } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                            // Desired temperature is below both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                            // Desired temperature is above both the inlet and outdoor temperatures
                            // so use whichever flow rate (max or min) that will get closer
                            if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MinOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            } else { // Toutdoor closer to Tdesired so use maximum outside air
                                state.dataVentilatedSlab->OAMassFlowRate = MaxOAFrac * state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
                            }
                        } else {
                            // It should NEVER get to this point, but just in case...
                            ShowFatalError(state, format("{} simulation control: illogical condition for {}", cMO_VentilatedSlab, ventSlab.Name));
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    // control water flow to obtain output matching Low Setpoint Temperateure
                    state.dataVentilatedSlab->HCoilOn = false;

                    SimVentSlabOAMixer(state, Item);
                    state.dataFans->fans(ventSlab.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

                    CpFan = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(FanOutletNode).HumRat);
                    QZnReq =
                        (state.dataLoopNodes->Node(OutletNode).MassFlowRate) * CpFan * (RadInTemp - state.dataLoopNodes->Node(FanOutletNode).Temp);

                    ControlCompOutput(state,
                                      ventSlab.Name,
                                      cMO_VentilatedSlab,
                                      Item,
                                      FirstHVACIteration,
                                      QZnReq,
                                      ControlNode,
                                      MaxWaterFlow,
                                      MinWaterFlow,
                                      0.001,
                                      ventSlab.ControlCompTypeNum,
                                      ventSlab.CompErrIndex,
                                      _,
                                      _,
                                      _,
                                      _,
                                      _,
                                      ventSlab.CWPlantLoc);
                }

            } // ...end of HEATING/COOLING IF-THEN block

            CalcVentilatedSlabRadComps(state, Item, FirstHVACIteration);

        } // ...end of system ON/OFF IF-THEN block

        // Resimulate fans if AirMassFlow is zero and FanElecPower is > 0, indicating that load or condensation controls shut off the ventilated slab
        // in CalcVentilatedSlabRadComps
        AirMassFlow = state.dataLoopNodes->Node(OutletNode).MassFlowRate;
        Real64 locFanElecPower = state.dataFans->fans(ventSlab.Fan_Index)->totalPower;

        if ((AirMassFlow <= 0.0) && (locFanElecPower > 0.0)) {
            state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(MixoutNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(MixoutNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRateMinAvail = 0.0;
            state.dataFans->fans(ventSlab.Fan_Index)->simulate(state, FirstHVACIteration, _, _);
        }

        CalcVentilatedSlabCoilOutput(state, Item, PowerMet, LatOutputProvided);
    }

    void CalcVentilatedSlabComps(EnergyPlusData &state,
                                 int const Item,                // system index in ventilated slab array
                                 bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                                 Real64 &LoadMet                // load met by the system (watts)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine launches the individual component simulations.
        // This is called either when the system is off to carry null conditions
        // through the system or during control iterations to continue updating
        // what is going on within the unit.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different components in order.  Only slight wrinkles
        // here are that the ventilated slab system has it's own outside air mixed and
        // that a cooling coil must be present in order to call a cooling coil
        // simulation.  Other than that, the subroutine is very straightforward.

        // Using/Aliasing
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AirMassFlow; // total mass flow through the system
        Real64 CpAirZn;     // specific heat of dry air at zone conditions (zone conditions same as system inlet)
        int InletNode;      // system air inlet node
        int OutletNode;     // system air outlet node
        // unused0309  INTEGER        :: HCoilOutAirNode
        Real64 QCoilReq; // Heat addition required from an electric/gas heating coil
        Real64 HCoilOutAirTemp;
        Real64 HCoilInAirTemp;
        // unused1208  REAL(r64)           :: RadInTemp       ! Set temperature for "Slab In Node"

        SimVentSlabOAMixer(state, Item);
        state.dataFans->fans(ventSlab.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

        if ((ventSlab.coolingCoilPresent) && (ventSlab.coolingCoilSchedValue >= 0.0)) {
            if (ventSlab.cCoilType == CoolingCoilType::HXAssisted) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                    ventSlab.coolingCoilName,
                                                                    FirstHVACIteration,
                                                                    HVAC::CompressorOp::On,
                                                                    0.0,
                                                                    ventSlab.coolingCoil_Index,
                                                                    HVAC::FanOp::Continuous);
            } else {
                WaterCoils::SimulateWaterCoilComponents(state, ventSlab.coolingCoilName, FirstHVACIteration, ventSlab.coolingCoil_Index);
            }
        }

        if ((ventSlab.heatingCoilPresent) && (ventSlab.heatingCoilSchedValue >= 0.0)) {

            switch (ventSlab.hCoilType) {

            case HeatingCoilType::Water: {

                WaterCoils::SimulateWaterCoilComponents(state, ventSlab.heatingCoilName, FirstHVACIteration, ventSlab.heatingCoil_Index);
                break;
            }
            case HeatingCoilType::Steam: {

                if (!state.dataVentilatedSlab->HCoilOn) {
                    QCoilReq = 0.0;
                } else {
                    int HCoilInAirNode = ventSlab.FanOutletNode;
                    CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(HCoilInAirNode).HumRat);
                    QCoilReq =
                        state.dataLoopNodes->Node(HCoilInAirNode).MassFlowRate * CpAirZn * (state.dataLoopNodes->Node(ventSlab.RadInNode).Temp) -
                        (state.dataLoopNodes->Node(HCoilInAirNode).Temp);
                }

                if (QCoilReq < 0.0) {
                    QCoilReq = 0.0; // a heating coil can only heat, not cool
                }

                SteamCoils::SimulateSteamCoilComponents(state, ventSlab.heatingCoilName, FirstHVACIteration, ventSlab.heatingCoil_Index, QCoilReq);
                break;
            }
            case HeatingCoilType::Electric:
            case HeatingCoilType::Gas: {

                if (!state.dataVentilatedSlab->HCoilOn) {
                    QCoilReq = 0.0;
                } else {
                    HCoilInAirTemp = state.dataLoopNodes->Node(ventSlab.FanOutletNode).Temp;
                    HCoilOutAirTemp = state.dataLoopNodes->Node(ventSlab.RadInNode).Temp;
                    CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);
                    QCoilReq = state.dataLoopNodes->Node(ventSlab.FanOutletNode).MassFlowRate * CpAirZn * (HCoilOutAirTemp - HCoilInAirTemp);
                }

                if (QCoilReq < 0.0) {
                    QCoilReq = 0.0; // a heating coil can only heat, not cool
                }

                HeatingCoils::SimulateHeatingCoilComponents(
                    state, ventSlab.heatingCoilName, FirstHVACIteration, QCoilReq, ventSlab.heatingCoil_Index);
                break;
            }
            default:
                break;
            }
        }

        InletNode = ventSlab.FanOutletNode;
        OutletNode = ventSlab.RadInNode;
        AirMassFlow = state.dataLoopNodes->Node(OutletNode).MassFlowRate;

        LoadMet = AirMassFlow * (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat) -
                                 Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));
    }

    void CalcVentilatedSlabCoilOutput(EnergyPlusData &state,
                                      int const Item,           // system index in ventilated slab array
                                      Real64 &PowerMet,         // power supplied (W)
                                      Real64 &LatOutputProvided // latent capacity supplied (kg/s)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  July 2015, M.J. Witte, Refactored coil output calcs in to this new routine

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the output from the coils

        // METHODOLOGY EMPLOYED:
        // Calculates the sensible and total enthalpy change from the fan outlet node to the slab inlet node.

        // USING/ALIASING:
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AirMassFlow; // total mass flow through the system
        int FanOutletNode;  // system fan outlet node
        int OutletNode;     // air outlet node
        Real64 SpecHumOut;  // Specific humidity ratio of outlet air (kg moisture / kg moist air)
        Real64 SpecHumIn;   // Specific humidity ratio of inlet air (kg moisture / kg moist air)
        Real64 QTotUnitOut; // total unit output [watts]
        Real64 QUnitOut;    // heating or sens. cooling provided by fan coil unit [watts]

        OutletNode = ventSlab.RadInNode;
        FanOutletNode = ventSlab.FanOutletNode;
        AirMassFlow = state.dataLoopNodes->Node(OutletNode).MassFlowRate;

        //        QTotUnitOut = AirMassFlow * ( Node( OutletNode ).Enthalpy - Node( FanOutletNode ).Enthalpy );
        QTotUnitOut = AirMassFlow *
                      (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(OutletNode).HumRat) -
                       Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(FanOutletNode).Temp, state.dataLoopNodes->Node(FanOutletNode).HumRat));
        QUnitOut = AirMassFlow *
                   (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(FanOutletNode).HumRat) -
                    Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(FanOutletNode).Temp, state.dataLoopNodes->Node(FanOutletNode).HumRat));
        // Limit sensible <= total when cooling (which is negative, so use max)
        QUnitOut = max(QUnitOut, QTotUnitOut);

        // Report variables...
        ventSlab.HeatCoilPower = max(0.0, QUnitOut);
        ventSlab.SensCoolCoilPower = std::abs(min(0.0, QUnitOut));
        ventSlab.TotCoolCoilPower = std::abs(min(0.0, QTotUnitOut));
        ventSlab.LateCoolCoilPower = ventSlab.TotCoolCoilPower - ventSlab.SensCoolCoilPower;
        ventSlab.ElecFanPower = (ventSlab.Fan_Index == 0) ? 0.0 : state.dataFans->fans(ventSlab.Fan_Index)->totalPower;

        ventSlab.AirMassFlowRate = AirMassFlow;

        SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
        SpecHumIn = state.dataLoopNodes->Node(FanOutletNode).HumRat;
        LatOutputProvided = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative
        PowerMet = QUnitOut;
    }

    void CalcVentilatedSlabRadComps(EnergyPlusData &state,
                                    int const Item,                                // System index in ventilated slab array
                                    [[maybe_unused]] bool const FirstHVACIteration // flag for 1st HVAV iteration in the time step !unused1208
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine launches the individual component simulations.
        // This is called either when the system is off to carry null conditions
        // through the system or during control iterations to continue updating
        // what is going on within the system.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different components in order.  Only slight wrinkles
        // here are that the Ventilated Slab has it's own outside air mixed and
        // that a cooling coil must be present in order to call a cooling coil
        // simulation.  Other than that, the subroutine is very straightforward.

        // Using/Aliasing
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr CondDeltaTemp(0.001); // How close the surface temperatures can get to the dewpoint temperature
        // of a space before the radiant cooling system shuts off the flow.
        Real64 constexpr ZeroSystemResp(0.1); // Response below which the system response is really zero
        Real64 constexpr TempCheckLimit(0.1); // Maximum allowed temperature difference between outlet temperature calculations
        static std::string const CurrentModuleObject("ZoneHVAC:VentilatedSlab");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CpAirZn;        // Intermediate calculational variable for specific heat of air
        Real64 DewPointTemp;   // Dew-point temperature based on the zone air conditions
        Real64 EpsMdotCpAirZn; // Epsilon (heat exchanger terminology) times water mass flow rate times water specific heat
        Real64 Mdot;           // Intermediate calculation variable for mass flow rate in a surface within the radiant system
        int RadSurfNum;        // DO loop counter for the surfaces that comprise a particular radiant system
        // unused0309  INTEGER  :: RadSurfNum4    ! DO loop counter for the surfaces that comprise a particular radiant system

        int SurfNum; // Index for radiant surface in Surface derived type
        // unused0309  INTEGER  :: RadSurfNumNum
        Real64 TotalVentSlabRadPower; // Total heat source/sink to radiant system
        Real64 AirOutletTempCheck;    // Radiant system air outlet temperature (calculated from mixing all outlet streams together)
        Real64 AirTempIn;             // Temperature of the air entering the radiant system, in C
        Real64 Ca;                    // Coefficients to relate the inlet air temperature to the heat source
        Real64 Cb;
        Real64 Cc;
        Real64 Cd;
        Real64 Ce;
        Real64 Cf;
        Real64 Cg;
        Real64 Ch;
        Real64 Ci;
        Real64 Cj;
        Real64 Ck;
        Real64 Cl;
        // For more info on Ca through Cl, refer Constant Flow Radiant System
        // For Phase 3
        Real64 CNumDS;
        Real64 CLengDS;
        Real64 CDiaDS;
        Real64 FlowFrac;
        Real64 MSlabAirInTemp;

        if (state.dataVentilatedSlab->FirstTimeFlag) {
            state.dataVentilatedSlab->AirTempOut.allocate(state.dataVentilatedSlab->MaxCloNumOfSurfaces);
            state.dataVentilatedSlab->FirstTimeFlag = false;
        }

        int SlabInNode = ventSlab.RadInNode;
        int FanOutletNode = ventSlab.FanOutletNode;
        int OAInletNode = ventSlab.OutsideAirNode;
        int MixoutNode = ventSlab.OAMixerOutNode;
        int ReturnAirNode = ventSlab.ReturnAirNode;
        int ZoneAirInNode = ventSlab.ZoneAirInNode;

        // Set the conditions on the air side inlet
        int ZoneNum = ventSlab.ZonePtr;
        Real64 ZoneMult = double(state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier);
        Real64 AirMassFlow = state.dataLoopNodes->Node(ventSlab.RadInNode).MassFlowRate / ZoneMult;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        if (state.dataVentilatedSlab->OperatingMode == HeatingMode) {

            if ((!ventSlab.heatingCoilPresent) || (ventSlab.heatingCoilSchedValue <= 0.0)) {

                AirTempIn = state.dataLoopNodes->Node(FanOutletNode).Temp;
                state.dataLoopNodes->Node(SlabInNode).Temp =
                    state.dataLoopNodes->Node(FanOutletNode).Temp; // If coil not available or running, then coil in and out temps same

            } else {

                AirTempIn = state.dataLoopNodes->Node(SlabInNode).Temp;
            }
        }

        if (state.dataVentilatedSlab->OperatingMode == CoolingMode) {

            if ((!ventSlab.coolingCoilPresent) || (ventSlab.coolingCoilSchedValue <= 0.0)) {

                AirTempIn = state.dataLoopNodes->Node(FanOutletNode).Temp;
                state.dataLoopNodes->Node(SlabInNode).Temp =
                    state.dataLoopNodes->Node(FanOutletNode).Temp; // If coil not available or running, then coil in and out temps same

            } else {

                AirTempIn = state.dataLoopNodes->Node(SlabInNode).Temp;
            }
        }

        if (AirMassFlow <= 0.0) {
            // No flow or below minimum allowed so there is no heat source/sink
            // This is possible with a mismatch between system and plant operation
            // or a slight mismatch between zone and system controls.  This is not
            // necessarily a "problem" so this exception is necessary in the code.

            for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
                SurfNum = ventSlab.SurfacePtr(RadSurfNum);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = 0.0;
                if (state.dataSurface->Surface(SurfNum).ExtBoundCond > 0 && state.dataSurface->Surface(SurfNum).ExtBoundCond != SurfNum) {
                    state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum).ExtBoundCond) =
                        0.0; // Also zero the other side of an interzone
                }
            }

            ventSlab.SlabOutTemp = ventSlab.SlabInTemp;

            // zero out node flows
            state.dataLoopNodes->Node(SlabInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OAInletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(ReturnAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
            AirMassFlow = 0.0;
        }

        if (AirMassFlow > 0.0) {

            if ((ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) || (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone)) {

                for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
                    SurfNum = ventSlab.SurfacePtr(RadSurfNum);
                    // Determine the heat exchanger "effectiveness" term
                    EpsMdotCpAirZn = CalcVentSlabHXEffectTerm(state,
                                                              Item,
                                                              AirTempIn,
                                                              AirMassFlow,
                                                              ventSlab.SurfaceFlowFrac(RadSurfNum),
                                                              ventSlab.CoreLength,
                                                              ventSlab.CoreDiameter,
                                                              ventSlab.CoreNumbers);

                    // Obtain the heat balance coefficients and calculate the intermediate coefficients
                    // linking the inlet air temperature to the heat source/sink to the radiant system.
                    // The coefficients are based on the Constant Flow Radiation System.

                    int ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
                    auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

                    Ca = state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum);
                    Cb = state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum);
                    Cc = state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum);

                    Cd = state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum);
                    Ce = state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum);
                    Cf = state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum);

                    Cg = state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum);
                    Ch = double(thisConstruct.CTFTSourceQ[0]);
                    Ci = double(thisConstruct.CTFTSourceIn[0]);
                    Cj = double(thisConstruct.CTFTSourceOut[0]);

                    Ck = Cg + ((Ci * (Ca + Cb * Cd) + Cj * (Cd + Ce * Ca)) / (1.0 - Ce * Cb));
                    Cl = Ch + ((Ci * (Cc + Cb * Cf) + Cj * (Cf + Ce * Cc)) / (1.0 - Ce * Cb));

                    Mdot = AirMassFlow * ventSlab.SurfaceFlowFrac(RadSurfNum);
                    CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);

                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) = ventSlab.CoreNumbers * EpsMdotCpAirZn * (AirTempIn - Ck) /
                                                                      (1.0 + (EpsMdotCpAirZn * Cl / state.dataSurface->Surface(SurfNum).Area));

                    if (state.dataSurface->Surface(SurfNum).ExtBoundCond > 0 && state.dataSurface->Surface(SurfNum).ExtBoundCond != SurfNum) {
                        state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum).ExtBoundCond) =
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum);
                    }
                    // Also set the other side of an interzone!
                    state.dataVentilatedSlab->AirTempOut(RadSurfNum) =
                        AirTempIn - (state.dataHeatBalFanSys->QRadSysSource(SurfNum) / (Mdot * CpAirZn));

                    // "Temperature Comparison" Cut-off:
                    // Check to see whether or not the system should really be running.  If
                    // QRadSysSource is negative when we are in heating mode or QRadSysSource
                    // is positive when we are in cooling mode, then the radiant system will
                    // be doing the opposite of its intention.  In this case, the flow rate
                    // is set to zero to avoid heating in cooling mode or cooling in heating
                    // mode.

                    if (((state.dataVentilatedSlab->OperatingMode == HeatingMode) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) <= 0.0)) ||
                        ((state.dataVentilatedSlab->OperatingMode == CoolingMode) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) >= 0.0))) {

                        // IF (.not. WarmupFlag) THEN
                        //   TempComparisonErrorCount = TempComparisonErrorCount + 1
                        //   IF (TempComparisonErrorCount <= NumOfVentSlabs) THEN
                        //     CALL ShowWarningError(state, 'Radaint Heat exchange is negative in Heating Mode or posive in Cooling Mode')
                        //     CALL ShowContinueError(state, 'Flow to the following ventilated slab will be shut-off to avoid heating in cooling mode
                        //     or cooling &
                        //                             in heating mode')
                        //     CALL ShowContinueError(state, 'Ventilated Slab Name = '//TRIM(VentSlab(Item)%Name))
                        //     CALL ShowContinueError(state, 'All node temperature are reseted at the ventilated slab surface temperature = '// &
                        //                            RoundSigDigits(TH(VentSlab(Item)%SurfacePtr(RadSurfNum),1,2),2))
                        //     CALL ShowContinueErrorTimeStamp(state, ' ')
                        //   ELSE
                        //     CALL ShowRecurringWarningErrorAtEnd(state, 'Ventilated Slab ['//TRIM(VentSlab(Item)%Name)//  &
                        //                  '] Temperature Comparison Error shut-off occurrence continues.',  &
                        //                  VentSlab(Item)%CondErrCount)
                        //   END IF
                        // END IF

                        state.dataLoopNodes->Node(SlabInNode).MassFlowRate = 0.0;
                        state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
                        state.dataLoopNodes->Node(OAInletNode).MassFlowRate = 0.0;
                        state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
                        state.dataLoopNodes->Node(ReturnAirNode).MassFlowRate = 0.0;
                        AirMassFlow = 0.0;

                        for (int RadSurfNum2 = 1; RadSurfNum2 <= ventSlab.NumOfSurfaces; ++RadSurfNum2) {
                            int SurfNum2 = ventSlab.SurfacePtr(RadSurfNum2);
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                            if (state.dataSurface->Surface(SurfNum2).ExtBoundCond > 0 &&
                                state.dataSurface->Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum2).ExtBoundCond) =
                                    0.0; // Also zero the other side of an interzone
                            }

                            if (ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) {
                                //            state.dataLoopNodes->Node(Returnairnode)%Temp = MAT(Zonenum)
                                state.dataLoopNodes->Node(ReturnAirNode).Temp =
                                    state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(RadSurfNum));
                                state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(ReturnAirNode).Temp;
                                state.dataLoopNodes->Node(SlabInNode).Temp = state.dataLoopNodes->Node(FanOutletNode).Temp;
                            } else if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {
                                state.dataLoopNodes->Node(ReturnAirNode).Temp = thisZoneHB.MAT;
                                state.dataLoopNodes->Node(SlabInNode).Temp = state.dataLoopNodes->Node(ReturnAirNode).Temp;
                                state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                                state.dataLoopNodes->Node(ZoneAirInNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                            }
                        }
                        break; // outer do loop
                    }

                    // Condensation Cut-off:
                    // Check to see whether there are any surface temperatures within the radiant system that have
                    // dropped below the dew-point temperature.  If so, we need to shut off this radiant system.
                    // A safety parameter is added (hardwired parameter) to avoid getting too close to condensation
                    // conditions.

                    if (state.dataVentilatedSlab->OperatingMode == CoolingMode) {
                        DewPointTemp = Psychrometrics::PsyTdpFnWPb(state, thisZoneHB.airHumRat, state.dataEnvrn->OutBaroPress);
                        for (int RadSurfNum2 = 1; RadSurfNum2 <= ventSlab.NumOfSurfaces; ++RadSurfNum2) {
                            if (state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(RadSurfNum2)) < (DewPointTemp + CondDeltaTemp)) {
                                // Condensation warning--must shut off radiant system
                                state.dataLoopNodes->Node(SlabInNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(OAInletNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(ReturnAirNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                                AirMassFlow = 0.0;
                                for (int RadSurfNum3 = 1; RadSurfNum3 <= ventSlab.NumOfSurfaces; ++RadSurfNum3) {
                                    int SurfNum2 = ventSlab.SurfacePtr(RadSurfNum3);
                                    state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                                    if (state.dataSurface->Surface(SurfNum2).ExtBoundCond > 0 &&
                                        state.dataSurface->Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                        state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum2).ExtBoundCond) =
                                            0.0; // Also zero the other side of an interzone
                                    }
                                }
                                // Produce a warning message so that user knows the system was shut-off due to potential for condensation
                                if (!state.dataGlobal->WarmupFlag) {
                                    ++state.dataVentilatedSlab->CondensationErrorCount;

                                    if (ventSlab.CondErrIndex == 0) {
                                        ShowWarningMessage(state, format("{} [{}]", cMO_VentilatedSlab, ventSlab.Name));
                                        ShowContinueError(
                                            state,
                                            format("Surface [{}] temperature below dew-point temperature--potential for condensation exists",
                                                   state.dataSurface->Surface(ventSlab.SurfacePtr(RadSurfNum2)).Name));
                                        ShowContinueError(state, "Flow to the ventilated slab system will be shut-off to avoid condensation");
                                        ShowContinueError(state,
                                                          format("Predicted radiant system surface temperature = {:.2R}",
                                                                 state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(RadSurfNum2))));
                                        ShowContinueError(
                                            state, format("Zone dew-point temperature + safety factor delta= {:.2R}", DewPointTemp + CondDeltaTemp));
                                        ShowContinueErrorTimeStamp(state, "");
                                    }
                                    if (state.dataVentilatedSlab->CondensationErrorCount == 1) {
                                        ShowContinueError(
                                            state, format("Note that there is a {:.4R} C safety built-in to the shut-off criteria", CondDeltaTemp));
                                        ShowContinueError(state, "Note also that this affects all surfaces that are part of this system");
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   cMO_VentilatedSlab + " [" + ventSlab.Name +
                                                                       "] condensation shut-off occurrence continues.",
                                                                   ventSlab.CondErrIndex,
                                                                   DewPointTemp,
                                                                   DewPointTemp,
                                                                   _,
                                                                   "C",
                                                                   "C");
                                }
                                break; // outer do loop
                            }
                        }
                    }
                }

                // Total Radiant Power
                AirOutletTempCheck = 0.0;
                TotalVentSlabRadPower = 0.0;
                for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
                    SurfNum = ventSlab.SurfacePtr(RadSurfNum);
                    TotalVentSlabRadPower += state.dataHeatBalFanSys->QRadSysSource(SurfNum);
                    AirOutletTempCheck += (ventSlab.SurfaceFlowFrac(RadSurfNum) * state.dataVentilatedSlab->AirTempOut(RadSurfNum));
                }
                TotalVentSlabRadPower *= ZoneMult;

                // Return Air temp Check
                if (ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) {
                    if (AirMassFlow > 0.0) {
                        CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);
                        state.dataLoopNodes->Node(ReturnAirNode).Temp =
                            state.dataLoopNodes->Node(SlabInNode).Temp - (TotalVentSlabRadPower / (AirMassFlow * CpAirZn));
                        if ((std::abs(state.dataLoopNodes->Node(ReturnAirNode).Temp - AirOutletTempCheck) > TempCheckLimit) &&
                            (std::abs(TotalVentSlabRadPower) > ZeroSystemResp)) {

                            if (!state.dataGlobal->WarmupFlag) {
                                ++state.dataVentilatedSlab->EnergyImbalanceErrorCount;
                                if (ventSlab.EnrgyImbalErrIndex == 0) {
                                    ShowWarningMessage(state, format("{} [{}]", cMO_VentilatedSlab, ventSlab.Name));
                                    ShowContinueError(state, "Ventilated Slab (slab only type) air outlet temperature calculation mismatch.");
                                    ShowContinueError(state,
                                                      "This should not happen as it indicates a potential energy imbalance in the calculations.");
                                    ShowContinueError(state, "However, it could also result from improper input for the ventilated slab or");
                                    ShowContinueError(state, "illogical control temperatures.  Check your input for this ventilated slab and");
                                    ShowContinueError(state, "also look at the internal data shown below.");
                                    ShowContinueError(state,
                                                      format("Predicted return air temperature [C] from the overall energy balance = {:.4R}",
                                                             state.dataLoopNodes->Node(ReturnAirNode).Temp));
                                    ShowContinueError(state,
                                                      format("Predicted return air temperature [C] from the slab section energy balances = {:.4R}",
                                                             AirOutletTempCheck));
                                    ShowContinueError(state,
                                                      format("Total energy rate (power) [W] added to the slab = {:.4R}", TotalVentSlabRadPower));
                                    ShowContinueErrorTimeStamp(state, "");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cMO_VentilatedSlab + " [" + ventSlab.Name +
                                                                   "] temperature calculation mismatch occurrence continues.",
                                                               ventSlab.EnrgyImbalErrIndex);
                            }
                        }
                    } else {
                        state.dataLoopNodes->Node(ReturnAirNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                    }
                }

                if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {
                    if (AirMassFlow > 0.0) {
                        state.dataLoopNodes->Node(ZoneAirInNode).Temp =
                            state.dataLoopNodes->Node(SlabInNode).Temp - (TotalVentSlabRadPower / (AirMassFlow * CpAirZn));
                        if ((std::abs(state.dataLoopNodes->Node(ZoneAirInNode).Temp - AirOutletTempCheck) > TempCheckLimit) &&
                            (std::abs(TotalVentSlabRadPower) > ZeroSystemResp)) {

                            if (!state.dataGlobal->WarmupFlag) {
                                ++state.dataVentilatedSlab->EnergyImbalanceErrorCount;
                                if (ventSlab.EnrgyImbalErrIndex == 0) {
                                    ShowWarningMessage(state, format("{} [{}]", cMO_VentilatedSlab, ventSlab.Name));
                                    ShowContinueError(state, "Ventilated Slab (slab only type) air outlet temperature calculation mismatch.");
                                    ShowContinueError(state,
                                                      "This should not happen as it indicates a potential energy imbalance in the calculations.");
                                    ShowContinueError(state, "However, it could also result from improper input for the ventilated slab or");
                                    ShowContinueError(state, "illogical control temperatures.  Check your input for this ventilated slab and");
                                    ShowContinueError(state, "also look at the internal data shown below.");
                                    ShowContinueError(state,
                                                      format("Predicted return air temperature [C] from the overall energy balance = {:.4R}",
                                                             state.dataLoopNodes->Node(ReturnAirNode).Temp));
                                    ShowContinueError(state,
                                                      format("Predicted return air temperature [C] from the slab section energy balances = {:.4R}",
                                                             AirOutletTempCheck));
                                    ShowContinueError(state,
                                                      format("Total energy rate (power) [W] added to the slab = {:.4R}", TotalVentSlabRadPower));
                                    ShowContinueErrorTimeStamp(state, "");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cMO_VentilatedSlab + " [" + ventSlab.Name +
                                                                   "] temperature calculation mismatch occurrence continues.",
                                                               ventSlab.EnrgyImbalErrIndex);
                            }
                        }
                        //       IF ((.NOT. FirstHVACIteration) .AND. &
                        //          (ABS(Node(ReturnAirNode)%Temp-MAT(Zonenum)) > VentSlabAirTempToler))THEN
                        //          NeedtoIterate = .TRUE.
                        //      END IF
                        //         state.dataLoopNodes->Node(ReturnAirNode)%Temp = MAT(Zonenum)
                    } else {
                        state.dataLoopNodes->Node(ZoneAirInNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                        state.dataLoopNodes->Node(ReturnAirNode).Temp = thisZoneHB.MAT;
                    }
                }

                // Now that we have the source/sink term, we must redo the heat balances to obtain
                // the new SumHATsurf value for the zone.  Note that the difference between the new
                // SumHATsurf and the value originally calculated by the heat balance with a zero
                // source for all radiant systems in the zone is the load met by the system (approximately).
                HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

            } // SYSCONFIG. SLABONLY&SLABANDZONE

            if (ventSlab.SysConfg == VentilatedSlabConfig::SeriesSlabs) {

                for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {

                    CNumDS = ventSlab.CNumbers(RadSurfNum);
                    CLengDS = ventSlab.CLength(RadSurfNum);  // for check
                    CDiaDS = ventSlab.CDiameter(RadSurfNum); // for check
                    FlowFrac = 1.0;

                    SurfNum = ventSlab.SurfacePtr(RadSurfNum);

                    // Determine the heat exchanger "effectiveness" term
                    EpsMdotCpAirZn = CalcVentSlabHXEffectTerm(state, Item, AirTempIn, AirMassFlow, FlowFrac, CLengDS, CDiaDS, CNumDS);

                    // Obtain the heat balance coefficients and calculate the intermediate coefficients
                    // linking the inlet air temperature to the heat source/sink to the radiant system.
                    // The coefficients are based on the Constant Flow Radiation System.

                    int ConstrNum = state.dataSurface->Surface(SurfNum).Construction;

                    Ca = state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum);
                    Cb = state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum);
                    Cc = state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum);

                    Cd = state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum);
                    Ce = state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum);
                    Cf = state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum);

                    Cg = state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum);
                    Ch = double(state.dataConstruction->Construct(ConstrNum).CTFTSourceQ[0]);
                    Ci = double(state.dataConstruction->Construct(ConstrNum).CTFTSourceIn[0]);
                    Cj = double(state.dataConstruction->Construct(ConstrNum).CTFTSourceOut[0]);

                    Ck = Cg + ((Ci * (Ca + Cb * Cd) + Cj * (Cd + Ce * Ca)) / (1.0 - Ce * Cb));
                    Cl = Ch + ((Ci * (Cc + Cb * Cf) + Cj * (Cf + Ce * Cc)) / (1.0 - Ce * Cb));

                    Mdot = AirMassFlow * FlowFrac;
                    CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);

                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) =
                        CNumDS * EpsMdotCpAirZn * (AirTempIn - Ck) / (1.0 + (EpsMdotCpAirZn * Cl / state.dataSurface->Surface(SurfNum).Area));

                    if (state.dataSurface->Surface(SurfNum).ExtBoundCond > 0 && state.dataSurface->Surface(SurfNum).ExtBoundCond != SurfNum) {
                        state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum).ExtBoundCond) =
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum);
                    }
                    // Also set the other side of an interzone!

                    state.dataVentilatedSlab->AirTempOut(RadSurfNum) =
                        AirTempIn - (state.dataHeatBalFanSys->QRadSysSource(SurfNum) / (Mdot * CpAirZn));
                    AirTempIn = state.dataVentilatedSlab->AirTempOut(RadSurfNum);
                    // "Temperature Comparison" Cut-off:
                    // Check to see whether or not the system should really be running.  If
                    // QRadSysSource is negative when we are in heating mode or QRadSysSource
                    // is positive when we are in cooling mode, then the radiant system will
                    // be doing the opposite of its intention.  In this case, the flow rate
                    // is set to zero to avoid heating in cooling mode or cooling in heating
                    // mode.

                    if (RadSurfNum == 1) {
                        if (((state.dataVentilatedSlab->OperatingMode == HeatingMode) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) <= 0.0)) ||
                            ((state.dataVentilatedSlab->OperatingMode == CoolingMode) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) >= 0.0))) {
                            state.dataLoopNodes->Node(SlabInNode).MassFlowRate = 0.0;
                            state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
                            state.dataLoopNodes->Node(OAInletNode).MassFlowRate = 0.0;
                            state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
                            state.dataLoopNodes->Node(ReturnAirNode).MassFlowRate = 0.0;
                            AirMassFlow = 0.0;

                            for (int RadSurfNum2 = 1; RadSurfNum2 <= ventSlab.NumOfSurfaces; ++RadSurfNum2) {
                                int SurfNum2 = ventSlab.SurfacePtr(RadSurfNum2);
                                state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                                if (state.dataSurface->Surface(SurfNum2).ExtBoundCond > 0 &&
                                    state.dataSurface->Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                    state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum2).ExtBoundCond) =
                                        0.0; // Also zero the other side of an interzone
                                }
                            }
                            state.dataLoopNodes->Node(ReturnAirNode).Temp = state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(1));
                            state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(ReturnAirNode).Temp;
                            state.dataLoopNodes->Node(SlabInNode).Temp = state.dataLoopNodes->Node(FanOutletNode).Temp;
                            // Each Internal node is reseted at the surface temperature

                            break; // outer do loop
                        }
                    }
                    // Condensation Cut-off:
                    // Check to see whether there are any surface temperatures within the radiant system that have
                    // dropped below the dew-point temperature.  If so, we need to shut off this radiant system.
                    // A safety parameter is added (hardwired parameter) to avoid getting too close to condensation
                    // conditions.

                    if (state.dataVentilatedSlab->OperatingMode == CoolingMode) {
                        DewPointTemp =
                            Psychrometrics::PsyTdpFnWPb(state,
                                                        state.dataZoneTempPredictorCorrector->zoneHeatBalance(ventSlab.ZPtr(RadSurfNum)).airHumRat,
                                                        state.dataEnvrn->OutBaroPress);
                        for (int RadSurfNum2 = 1; RadSurfNum2 <= ventSlab.NumOfSurfaces; ++RadSurfNum2) {
                            if (state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(RadSurfNum2)) < (DewPointTemp + CondDeltaTemp)) {
                                // Condensation warning--must shut off radiant system
                                state.dataLoopNodes->Node(SlabInNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(FanOutletNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(OAInletNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(MixoutNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(ReturnAirNode).MassFlowRate = 0.0;
                                state.dataLoopNodes->Node(FanOutletNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                                AirMassFlow = 0.0;
                                for (int RadSurfNum3 = 1; RadSurfNum3 <= ventSlab.NumOfSurfaces; ++RadSurfNum3) {
                                    int SurfNum2 = ventSlab.SurfacePtr(RadSurfNum3);
                                    state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                                    if (state.dataSurface->Surface(SurfNum2).ExtBoundCond > 0 &&
                                        state.dataSurface->Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                        state.dataHeatBalFanSys->QRadSysSource(state.dataSurface->Surface(SurfNum2).ExtBoundCond) =
                                            0.0; // Also zero the other side of an interzone
                                    }
                                }
                                // Produce a warning message so that user knows the system was shut-off due to potential for condensation
                                if (!state.dataGlobal->WarmupFlag) {
                                    ++state.dataVentilatedSlab->CondensationErrorCount;
                                    if (ventSlab.CondErrIndex == 0) {
                                        ShowWarningMessage(state, format("{} [{}]", cMO_VentilatedSlab, ventSlab.Name));
                                        ShowContinueError(
                                            state,
                                            format("Surface [{}] temperature below dew-point temperature--potential for condensation exists",
                                                   state.dataSurface->Surface(ventSlab.SurfacePtr(RadSurfNum2)).Name));
                                        ShowContinueError(state, "Flow to the ventilated slab system will be shut-off to avoid condensation");
                                        ShowContinueError(state,
                                                          format("Predicted radiant system surface temperature = {:.2R}",
                                                                 state.dataHeatBalSurf->SurfInsideTempHist(1)(ventSlab.SurfacePtr(RadSurfNum2))));
                                        ShowContinueError(
                                            state, format("Zone dew-point temperature + safety factor delta= {:.2R}", DewPointTemp + CondDeltaTemp));
                                        ShowContinueErrorTimeStamp(state, "");
                                    }
                                    if (state.dataVentilatedSlab->CondensationErrorCount == 1) {
                                        ShowContinueError(
                                            state, format("Note that there is a {:.4R} C safety built-in to the shut-off criteria", CondDeltaTemp));
                                        ShowContinueError(state, "Note also that this affects all surfaces that are part of this system");
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   cMO_VentilatedSlab + " [" + ventSlab.Name +
                                                                       "] condensation shut-off occurrence continues.",
                                                                   ventSlab.CondErrIndex,
                                                                   DewPointTemp,
                                                                   DewPointTemp,
                                                                   _,
                                                                   "C",
                                                                   "C");
                                }
                                break; // outer do loop
                            }
                        }
                    }
                }

                // Total Radiant Power
                AirOutletTempCheck = 0.0;
                TotalVentSlabRadPower = 0.0;
                for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
                    SurfNum = ventSlab.SurfacePtr(RadSurfNum);
                    TotalVentSlabRadPower += state.dataHeatBalFanSys->QRadSysSource(SurfNum);
                    AirOutletTempCheck = state.dataVentilatedSlab->AirTempOut(RadSurfNum);
                }
                TotalVentSlabRadPower *= ZoneMult;

                // Internal Node Temperature Check

                MSlabAirInTemp = state.dataLoopNodes->Node(SlabInNode).Temp;
                bool ErrorsFound = false; // Set to true if errors in input, fatal at end of routine

                for (RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
                    std::string SlabName = ventSlab.SurfaceName(RadSurfNum);
                    std::string MSlabIn = ventSlab.SlabIn(RadSurfNum);
                    std::string MSlabOut = ventSlab.SlabOut(RadSurfNum);
                    ventSlab.MSlabInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                               MSlabIn,
                                                                               ErrorsFound,
                                                                               DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                               SlabName,
                                                                               DataLoopNode::NodeFluidType::Air,
                                                                               DataLoopNode::ConnectionType::Internal,
                                                                               NodeInputManager::CompFluidStream::Primary,
                                                                               DataLoopNode::ObjectIsNotParent);
                    ventSlab.MSlabOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                MSlabOut,
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::ZoneHVACVentilatedSlab,
                                                                                SlabName,
                                                                                DataLoopNode::NodeFluidType::Air,
                                                                                DataLoopNode::ConnectionType::Internal,
                                                                                NodeInputManager::CompFluidStream::Primary,
                                                                                DataLoopNode::ObjectIsNotParent);
                    int MSlabInletNode = ventSlab.MSlabInNode;
                    int MSlabOutletNode = ventSlab.MSlabOutNode;
                    SurfNum = ventSlab.SurfacePtr(RadSurfNum);

                    if (AirMassFlow > 0.0) {

                        CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);

                        state.dataLoopNodes->Node(MSlabInletNode).Temp = MSlabAirInTemp;
                        state.dataLoopNodes->Node(MSlabOutletNode).Temp = state.dataLoopNodes->Node(MSlabInletNode).Temp -
                                                                          (state.dataHeatBalFanSys->QRadSysSource(SurfNum) / (AirMassFlow * CpAirZn));
                        MSlabAirInTemp = state.dataLoopNodes->Node(MSlabOutletNode).Temp;
                    } else {
                        state.dataLoopNodes->Node(MSlabInletNode).Temp = state.dataLoopNodes->Node(ReturnAirNode).Temp;
                        state.dataLoopNodes->Node(MSlabOutletNode).Temp = state.dataLoopNodes->Node(MSlabInletNode).Temp;
                    }
                }

                // Return Air temp Check
                if (AirMassFlow > 0.0) {

                    CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);
                    state.dataLoopNodes->Node(ReturnAirNode).Temp =
                        state.dataLoopNodes->Node(SlabInNode).Temp - (TotalVentSlabRadPower / (AirMassFlow * CpAirZn));

                    if ((std::abs(state.dataLoopNodes->Node(ReturnAirNode).Temp - AirOutletTempCheck) > TempCheckLimit) &&
                        (std::abs(TotalVentSlabRadPower) > ZeroSystemResp)) { // Return air temperature check did not match calculated temp

                        if (!state.dataGlobal->WarmupFlag) {
                            ++state.dataVentilatedSlab->EnergyImbalanceErrorCount;
                            if (ventSlab.EnrgyImbalErrIndex == 0) {
                                ShowWarningMessage(state, format("{} [{}]", cMO_VentilatedSlab, ventSlab.Name));
                                ShowContinueError(state, "Ventilated Slab (slab only type) air outlet temperature calculation mismatch.");
                                ShowContinueError(state, "This should not happen as it indicates a potential energy imbalance in the calculations.");
                                ShowContinueError(state, "However, it could also result from improper input for the ventilated slab or");
                                ShowContinueError(state, "illogical control temperatures.  Check your input for this ventilated slab and");
                                ShowContinueError(state, "also look at the internal data shown below.");
                                ShowContinueError(state,
                                                  format("Predicted return air temperature [C] from the overall energy balance = {:.4R}",
                                                         state.dataLoopNodes->Node(ReturnAirNode).Temp));
                                ShowContinueError(state,
                                                  format("Predicted return air temperature [C] from the slab section energy balances = {:.4R}",
                                                         AirOutletTempCheck));
                                ShowContinueError(state, format("Total energy rate (power) [W] added to the slab = {:.4R}", TotalVentSlabRadPower));
                                ShowContinueErrorTimeStamp(state, "");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           cMO_VentilatedSlab + " [" + ventSlab.Name +
                                                               "] temperature calculation mismatch occurrence continues.",
                                                           ventSlab.EnrgyImbalErrIndex);
                        }
                    }

                } else {
                    state.dataLoopNodes->Node(ReturnAirNode).Temp = state.dataLoopNodes->Node(SlabInNode).Temp;
                }

                // Now that we have the source/sink term, we must redo the heat balances to obtain
                // the new SumHATsurf value for the zone.  Note that the difference between the new
                // SumHATsurf and the value originally calculated by the heat balance with a zero
                // source for all radiant systems in the zone is the load met by the system (approximately).

                HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state);
                HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state);

            } // SeriesSlabs

        } //(AirMassFlow > 0.0d0)
    }

    void SimVentSlabOAMixer(EnergyPlusData &state, int const Item) // System index in Ventilated Slab array
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This responsibility of this subroutine is to set the air flow rates
        // through the mixing box portion of the Ventilated Slab and then perform
        // an energy balance to arrive at outlet conditions which then would
        // serve as inlet conditions to the coils (or outlet conditions for
        // the device).  There is some question as to whether this needs to be
        // called every time the coils and fan are called since how the fans and
        // coil operate won't presumable change how the mixer operates.  The
        // method in which this routine is called is slightly cleaner though
        // from a code readability standpoint though less efficient.

        // METHODOLOGY EMPLOYED:
        // The OAMassFlowRate has already been calculated in the main control
        // algorithm.  Use this flow rate to establish all of the other flow
        // rates and perform an energy balance on the mixing of the return and
        // outdoor air streams.

        // Using/Aliasing
        auto const &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OAFraction; // Outside air fraction of inlet air

        int AirRelNode = ventSlab.AirReliefNode;      // relief air node number in ventilated slab loop
        int InletNode = ventSlab.ReturnAirNode;       // inlet node number for ventilated slab loop
        int OAMixOutNode = ventSlab.OAMixerOutNode;   // outside air mixer outlet node for ventilated slab loop
        int OutsideAirNode = ventSlab.OutsideAirNode; // outside air node number in ventilated slab loop

        // "Resolve" the air flow rates...

        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = state.dataVentilatedSlab->OAMassFlowRate;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = state.dataVentilatedSlab->OAMassFlowRate;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = state.dataVentilatedSlab->OAMassFlowRate;

        state.dataLoopNodes->Node(AirRelNode).MassFlowRate = state.dataVentilatedSlab->OAMassFlowRate;
        state.dataLoopNodes->Node(AirRelNode).MassFlowRateMinAvail = state.dataVentilatedSlab->OAMassFlowRate;
        state.dataLoopNodes->Node(AirRelNode).MassFlowRateMaxAvail = state.dataVentilatedSlab->OAMassFlowRate;

        state.dataLoopNodes->Node(OAMixOutNode).MassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        state.dataLoopNodes->Node(OAMixOutNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        state.dataLoopNodes->Node(OAMixOutNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(InletNode).MassFlowRate;

        // "Inlet" conditions for InletNode and OutsideAirNode have already
        // been set elsewhere so we just need to set the "outlet" conditions
        state.dataLoopNodes->Node(AirRelNode).Temp = state.dataLoopNodes->Node(InletNode).Temp;
        state.dataLoopNodes->Node(AirRelNode).Press = state.dataLoopNodes->Node(InletNode).Press;
        state.dataLoopNodes->Node(AirRelNode).HumRat = state.dataLoopNodes->Node(InletNode).HumRat;
        state.dataLoopNodes->Node(AirRelNode).Enthalpy = state.dataLoopNodes->Node(InletNode).Enthalpy;

        if (state.dataLoopNodes->Node(InletNode).MassFlowRate > 0.0) {

            OAFraction = state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate / state.dataLoopNodes->Node(InletNode).MassFlowRate;

        } else {
            OAFraction = 0.0;
        }

        state.dataLoopNodes->Node(InletNode).Enthalpy =
            Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat);

        // Perform an energy and moisture mass balance on the mixing portion of the OA Mixer of the ventilated slab
        state.dataLoopNodes->Node(OAMixOutNode).Enthalpy =
            OAFraction * state.dataLoopNodes->Node(OutsideAirNode).Enthalpy + (1.0 - OAFraction) * state.dataLoopNodes->Node(InletNode).Enthalpy;
        state.dataLoopNodes->Node(OAMixOutNode).HumRat =
            OAFraction * state.dataLoopNodes->Node(OutsideAirNode).HumRat + (1.0 - OAFraction) * state.dataLoopNodes->Node(InletNode).HumRat;

        // Find the other key state points based on calculated conditions
        state.dataLoopNodes->Node(OAMixOutNode).Temp =
            Psychrometrics::PsyTdbFnHW(state.dataLoopNodes->Node(OAMixOutNode).Enthalpy, state.dataLoopNodes->Node(OAMixOutNode).HumRat);
        state.dataLoopNodes->Node(OAMixOutNode).Press = state.dataLoopNodes->Node(InletNode).Press;
    }

    void UpdateVentilatedSlab(EnergyPlusData &state,
                              int const Item, // Index for the ventilated slab under consideration within the derived types
                              [[maybe_unused]] bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep !unused1208
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   November 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does any updating that needs to be done for low
        // temperature radiant heating and cooling systems.  One of the most
        // important functions of this routine is to update the average heat
        // source/sink for a particular system over the various system time
        // steps that make up the zone time step.  For hydronic systems,
        // this routine must also set the outlet water conditions.

        // METHODOLOGY EMPLOYED:
        // For the source/sink average update, if the system time step elapsed
        // is still what it used to be, then either we are still iterating or
        // we had to go back and shorten the time step.  As a result, we have
        // to subtract out the previous value that we added.  If the system
        // time step elapsed is different, then we just need to add the new
        // values to the running average.

        // Using/Aliasing
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OAFraction; // Outside air fraction of inlet air

        int TotRadSurfaces = ventSlab.NumOfSurfaces;
        int MixOutNode = ventSlab.OAMixerOutNode; // Node number for the water side outlet of the radiant system
        int OANode = ventSlab.OutsideAirNode;
        int AirOutletNode = ventSlab.RadInNode;
        int FanOutNode = ventSlab.FanOutletNode;
        Real64 AirMassFlow = state.dataLoopNodes->Node(AirOutletNode).MassFlowRate;
        int ZoneInletNode = ventSlab.ZoneAirInNode; // Node number for the air side inlet of the ventilated slab
        Real64 CpAppAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(AirOutletNode).HumRat);
        int AirInletNode = ventSlab.ReturnAirNode; // Node number for the air side inlet of the ventilated slab

        for (int RadSurfNum = 1; RadSurfNum <= TotRadSurfaces; ++RadSurfNum) {

            int SurfNum = ventSlab.SurfacePtr(RadSurfNum);

            if (ventSlab.LastSysTimeElapsed == SysTimeElapsed) {
                // Still iterating or reducing system time step, so subtract old values which were not valid
                ventSlab.QRadSysSrcAvg(RadSurfNum) -= ventSlab.LastQRadSysSrc(RadSurfNum) * ventSlab.LastTimeStepSys / state.dataGlobal->TimeStepZone;
            }

            // Update the running average and the "last" values with the current values of the appropriate variables
            ventSlab.QRadSysSrcAvg(RadSurfNum) += state.dataHeatBalFanSys->QRadSysSource(SurfNum) * TimeStepSys / state.dataGlobal->TimeStepZone;
            ventSlab.LastQRadSysSrc(RadSurfNum) = state.dataHeatBalFanSys->QRadSysSource(SurfNum);
        }
        ventSlab.LastSysTimeElapsed = SysTimeElapsed;
        ventSlab.LastTimeStepSys = TimeStepSys;

        // First sum up all of the heat sources/sinks associated with this system
        Real64 TotalHeatSource = 0.0;
        for (int RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
            int SurfNum = ventSlab.SurfacePtr(RadSurfNum);
            TotalHeatSource += state.dataHeatBalFanSys->QRadSysSource(SurfNum);
        }
        int ZoneNum = ventSlab.ZonePtr;
        int ZoneMult = double(state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier);
        TotalHeatSource *= ZoneMult;

        // Update the heating side of things

        if ((CpAppAir > 0.0) && (AirMassFlow > 0.0)) {

            if ((ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) || (ventSlab.SysConfg == VentilatedSlabConfig::SeriesSlabs)) {
                state.dataLoopNodes->Node(AirInletNode) = state.dataLoopNodes->Node(AirInletNode);
                state.dataLoopNodes->Node(AirInletNode).Temp =
                    state.dataLoopNodes->Node(AirOutletNode).Temp - TotalHeatSource / AirMassFlow / CpAppAir;
                state.dataLoopNodes->Node(AirInletNode).MassFlowRate = state.dataLoopNodes->Node(AirOutletNode).MassFlowRate;
                state.dataLoopNodes->Node(AirInletNode).HumRat = state.dataLoopNodes->Node(AirOutletNode).HumRat;

            } else if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {
                state.dataLoopNodes->Node(ZoneInletNode) = state.dataLoopNodes->Node(ZoneInletNode);
                state.dataLoopNodes->Node(ZoneInletNode).Temp =
                    state.dataLoopNodes->Node(AirOutletNode).Temp - TotalHeatSource / AirMassFlow / CpAppAir;
                state.dataLoopNodes->Node(ZoneInletNode).MassFlowRate = state.dataLoopNodes->Node(AirOutletNode).MassFlowRate;
                state.dataLoopNodes->Node(ZoneInletNode).HumRat = state.dataLoopNodes->Node(AirOutletNode).HumRat;
                state.dataLoopNodes->Node(ventSlab.ReturnAirNode).Temp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
            }

        } else {
            state.dataLoopNodes->Node(FanOutNode) = state.dataLoopNodes->Node(AirOutletNode);
            if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {
                state.dataLoopNodes->Node(ZoneInletNode) = state.dataLoopNodes->Node(AirInletNode);
            }
            for (int const surfNum : ventSlab.SurfacePtr) {
                state.dataHeatBalFanSys->QRadSysSource(surfNum) = 0.0;
            }
        }

        // Resolve mixouttemp

        if (state.dataLoopNodes->Node(AirInletNode).MassFlowRate > 0.0) {

            OAFraction = state.dataLoopNodes->Node(OANode).MassFlowRate / state.dataLoopNodes->Node(AirInletNode).MassFlowRate;

        } else {
            OAFraction = 0.0;
        }

        if (OAFraction <= 0.0) {

            state.dataLoopNodes->Node(MixOutNode).HumRat = state.dataLoopNodes->Node(AirInletNode).HumRat;
            state.dataLoopNodes->Node(MixOutNode).Temp = state.dataLoopNodes->Node(AirInletNode).Temp;

        } else {

            state.dataLoopNodes->Node(MixOutNode).Enthalpy =
                OAFraction * state.dataLoopNodes->Node(OANode).Enthalpy + (1.0 - OAFraction) * state.dataLoopNodes->Node(AirInletNode).Enthalpy;
            state.dataLoopNodes->Node(MixOutNode).HumRat =
                OAFraction * state.dataLoopNodes->Node(OANode).HumRat + (1.0 - OAFraction) * state.dataLoopNodes->Node(AirInletNode).HumRat;

            state.dataLoopNodes->Node(MixOutNode).Temp =
                Psychrometrics::PsyTdbFnHW(state.dataLoopNodes->Node(MixOutNode).Enthalpy, state.dataLoopNodes->Node(MixOutNode).HumRat);
        }
    }

    Real64 CalcVentSlabHXEffectTerm(EnergyPlusData &state,
                                    int const Item,            // Index number of radiant system under consideration
                                    Real64 const Temperature,  // Temperature of air entering the radiant system, in C
                                    Real64 const AirMassFlow,  // Mass flow rate of water in the radiant system, in kg/s
                                    Real64 const FlowFraction, // Mass flow rate fraction for this surface in the radiant system
                                    Real64 const CoreLength,   // Length of tubing in the radiant system, in m
                                    Real64 const CoreDiameter, // Inside diameter of the tubing in the radiant system, in m
                                    Real64 const CoreNumbers)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   December 2000
        //       MODIFIED       June 2008 (air properties)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the radiant system "heat exchanger"
        // effectiveness term.  This is equal to the mass flow rate of water
        // times the specific heat of water times the effectiveness of
        // the heat exchanger (radiant system "coil").

        // METHODOLOGY EMPLOYED:
        // Assumes that the only real heat transfer term that we have to
        // deal with is the convection from the water to the tube.  The
        // other assumptions are that the tube inside surface temperature
        // is equal to the "source location temperature" and that it is
        // a CONSTANT throughout the radiant system.  This is to make
        // the problem more tractable and to fit with other system assumptions
        // that were made elsewhere in the radiant system model.

        // REFERENCES:
        // Property data for air shown below as parameters taken from
        //   Mills, Heat Transfer, Table A.7.
        // Heat exchanger information also from Incropera and DeWitt.
        // Code based loosely on code from IBLAST program (research version)

        // Using/Aliasing
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // Return value
        Real64 CalcVentSlabHXEffectTerm;

        Real64 constexpr MaxLaminarRe(2300.0); // Maximum Reynolds number for laminar flow
        int constexpr NumOfPropDivisions(13);
        Real64 constexpr MaxExpPower(50.0); // Maximum power after which EXP argument would be zero for DP variables
        static constexpr std::array<Real64, NumOfPropDivisions> Temps = {
            1.85, 6.85, 11.85, 16.85, 21.85, 26.85, 31.85, 36.85, 41.85, 46.85, 51.85, 56.85, 61.85}; // Temperature, in C
        static constexpr std::array<Real64, NumOfPropDivisions> Mu = {0.0000088,
                                                                      0.0000176,
                                                                      0.00001781,
                                                                      0.00001802,
                                                                      0.000018225,
                                                                      0.00001843,
                                                                      0.00001865,
                                                                      0.00001887,
                                                                      0.00001908,
                                                                      0.00001929,
                                                                      0.0000195,
                                                                      0.00001971,
                                                                      0.00001992}; // Viscosity, in Ns/m2

        static constexpr std::array<Real64, NumOfPropDivisions> Conductivity = {
            0.01275, 0.0255, 0.0258, 0.0261, 0.0264, 0.0267, 0.02705, 0.0274, 0.02775, 0.0281, 0.0284, 0.0287, 0.01435}; // Conductivity, in W/mK
        static constexpr std::array<Real64, NumOfPropDivisions> Pr = {
            0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69, 0.69}; // Prandtl number (dimensionless)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Index;
        Real64 InterpFrac;
        Real64 NuD;
        Real64 ReD;
        Real64 NTU;
        Real64 CpAppAir;
        Real64 Kactual;
        Real64 MUactual;
        Real64 PRactual;
        Real64 SysAirMassFlow; // Specific heat of air

        // First find out where we are in the range of temperatures
        Index = 0;
        while (Index < NumOfPropDivisions) {
            if (Temperature < Temps[Index]) {
                break; // DO loop
            }
            ++Index;
        }

        // Initialize thermal properties of Air
        if (Index == 0) {
            MUactual = Mu[Index];
            Kactual = Conductivity[Index];
            PRactual = Pr[Index];
        } else if (Index > NumOfPropDivisions - 1) {
            Index = NumOfPropDivisions - 1;
            MUactual = Mu[Index];
            Kactual = Conductivity[Index];
            PRactual = Pr[Index];
        } else {
            InterpFrac = (Temperature - Temps[Index - 1]) / (Temps[Index] - Temps[Index - 1]);
            MUactual = Mu[Index - 1] + InterpFrac * (Mu[Index] - Mu[Index - 1]);
            Kactual = Conductivity[Index - 1] + InterpFrac * (Conductivity[Index] - Conductivity[Index - 1]);
            PRactual = Pr[Index - 1] + InterpFrac * (Pr[Index] - Pr[Index - 1]);
        }
        // arguments are glycol name, temperature, and concentration
        CpAppAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ventSlab.RadInNode).HumRat);
        SysAirMassFlow = AirMassFlow / CoreNumbers;

        // Calculate the Reynold's number from RE=(4*Mdot)/(Pi*Mu*Diameter)
        ReD = 4.0 * SysAirMassFlow * FlowFraction / (Constant::Pi * MUactual * CoreDiameter);

        // Calculate the Nusselt number based on what flow regime one is in
        if (ReD >= MaxLaminarRe) { // Turbulent flow --> use Colburn equation

            NuD = 0.023 * std::pow(ReD, 0.8) * std::pow(PRactual, 1.0 / 3.0);

        } else { // Laminar flow --> use constant surface temperature relation

            NuD = 3.66;
        }

        // Calculate the NTU parameter
        // NTU = UA/[(Mdot*Cp)min]
        // where: U = h (convection coefficient) and h = (k)(Nu)/D
        //        A = Pi*D*TubeLength
        NTU = Constant::Pi * Kactual * NuD * CoreLength / (SysAirMassFlow * CpAppAir); // FlowFraction cancels out here

        // Calculate Epsilon*MassFlowRate*Cp
        if (NTU > MaxExpPower) {
            CalcVentSlabHXEffectTerm = FlowFraction * SysAirMassFlow * CpAppAir;
        } else {
            CalcVentSlabHXEffectTerm = (1.0 - std::exp(-NTU)) * FlowFraction * SysAirMassFlow * CpAppAir;
        }

        return CalcVentSlabHXEffectTerm;
    }

    void ReportVentilatedSlab(EnergyPlusData &state, int const Item) // Index for the ventilated slab under consideration within the derived types
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simply produces output for the low temperature radiant system.

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &ventSlab = state.dataVentilatedSlab->VentSlab(Item);

        // Slab Part
        Real64 TotalVentSlabRadPower = 0.0; // Total source/sink power for the radiant system (sum of all surfaces of the system)

        for (int RadSurfNum = 1; RadSurfNum <= ventSlab.NumOfSurfaces; ++RadSurfNum) {
            int SurfNum = ventSlab.SurfacePtr(RadSurfNum);
            TotalVentSlabRadPower += state.dataHeatBalFanSys->QRadSysSource(SurfNum);
        }
        Real64 ZoneMult = double(state.dataHeatBal->Zone(ventSlab.ZonePtr).Multiplier * state.dataHeatBal->Zone(ventSlab.ZonePtr).ListMultiplier);
        TotalVentSlabRadPower *= ZoneMult;
        ventSlab.RadHeatingPower = 0.0;
        ventSlab.RadCoolingPower = 0.0;

        if (TotalVentSlabRadPower >= 0.01) {

            ventSlab.RadHeatingPower = +TotalVentSlabRadPower;
        } else {

            ventSlab.RadCoolingPower = -TotalVentSlabRadPower;
        }

        ventSlab.RadHeatingEnergy = ventSlab.RadHeatingPower * TimeStepSysSec;
        ventSlab.RadCoolingEnergy = ventSlab.RadCoolingPower * TimeStepSysSec;

        // Coil Part
        ventSlab.HeatCoilEnergy = ventSlab.HeatCoilPower * TimeStepSysSec;
        ventSlab.SensCoolCoilEnergy = ventSlab.SensCoolCoilPower * TimeStepSysSec;
        ventSlab.LateCoolCoilEnergy = ventSlab.LateCoolCoilPower * TimeStepSysSec;
        ventSlab.TotCoolCoilEnergy = ventSlab.TotCoolCoilPower * TimeStepSysSec;
        ventSlab.ElecFanEnergy = ventSlab.ElecFanPower * TimeStepSysSec;

        if ((ventSlab.SysConfg == VentilatedSlabConfig::SlabOnly) || (ventSlab.SysConfg == VentilatedSlabConfig::SeriesSlabs)) {
            ventSlab.SlabInTemp = state.dataLoopNodes->Node(ventSlab.RadInNode).Temp;
            ventSlab.SlabOutTemp = state.dataLoopNodes->Node(ventSlab.ReturnAirNode).Temp;

        } else if (ventSlab.SysConfg == VentilatedSlabConfig::SlabAndZone) {
            ventSlab.SlabInTemp = state.dataLoopNodes->Node(ventSlab.RadInNode).Temp;
            ventSlab.ZoneInletTemp = state.dataLoopNodes->Node(ventSlab.ZoneAirInNode).Temp;
            ventSlab.SlabOutTemp = state.dataLoopNodes->Node(ventSlab.ReturnAirNode).Temp;
        }

        ventSlab.ReturnAirTemp = state.dataLoopNodes->Node(ventSlab.ReturnAirNode).Temp;
        ventSlab.FanOutletTemp = state.dataLoopNodes->Node(ventSlab.FanOutletNode).Temp;

        if (ventSlab.FirstPass) { // reset sizing flags so other zone equipment can size normally
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, ventSlab.FirstPass);
            }
        }
    }

    int getVentilatedSlabIndex(EnergyPlusData &state, std::string_view CompName)
    {
        if (state.dataVentilatedSlab->GetInputFlag) {
            GetVentilatedSlabInput(state);
            state.dataVentilatedSlab->GetInputFlag = false;
        }

        for (int VentSlabNum = 1; VentSlabNum <= state.dataVentilatedSlab->NumOfVentSlabs; ++VentSlabNum) {
            if (Util::SameString(state.dataVentilatedSlab->VentSlab(VentSlabNum).Name, CompName)) {
                return VentSlabNum;
            }
        }

        return 0;
    }
    //*****************************************************************************************

} // namespace VentilatedSlab

} // namespace EnergyPlus
