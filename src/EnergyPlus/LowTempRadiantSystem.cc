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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/LowTempRadiantSystem.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace LowTempRadiantSystem {

    // Module containing the routines dealing with the low temperature radiant systems

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   November 2000
    //       MODIFIED       Rick Strand March 2001 (additional controls, etc.)
    //                      Rick Strand July 2003 (added constant flow hydronic system)
    //                      B. Griffith Sept 2010, plant upgrades, generalize fluid properties
    //                      Rick Strand August 2011 (improved condensation handling)

    // PURPOSE OF THIS MODULE:
    // The purpose of this module is to simulate low temperature radiant systems.
    // It is the intention of this module to cover all types of low temperature
    // radiant systems: wall, ceiling, floor, heating, cooling, panels, etc.

    // METHODOLOGY EMPLOYED:
    // Based on work done in IBLAST, this model has been revised for the structure
    // of EnergyPlus.  It is still based on the QTF formulation of heat transfer
    // through building elements with embedded heat sources/sinks.  Note that due
    // to the fact that a radiant system is both a building heat transfer element
    // and a controllable system that some iteration between the system and the
    // surface heat balance routine is necessary.
    // REFERENCES:
    // IBLAST-QTF research program, completed in January 1995 (unreleased)
    // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
    //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
    //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
    //   Engineering.
    // Seem, J.E. 1986. "Heat Transfer in Buildings", Ph.D. dissertation, University
    //   of Wisconsin-Madison.

    // OTHER NOTES: This module contains three different types of radiant system
    // models: (a) variable flow hydronic heating/cooling radiant system;
    // (b) constant flow, variable controlled temperature heating/cooling radiant
    // system; (c) electric resistance heating radiant system.  Systems (a) and
    // (b) are hydronic systems--one which varies hydronic flow as the key control
    // parameter (a) and one which varies the inlet hydronic temperature while
    // keeping the flow rate through the radiant system constant (b).  In system
    // (b), the injection rate from the main water loop is varied to obtain the
    // proper inlet temperature.

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using HVAC::SmallLoad;
    using Psychrometrics::PsyTdpFnWPb;

    // Data
    // MODULE PARAMETER DEFINITIONS:
    // System types:
    constexpr std::string_view cHydronicSystem("ZoneHVAC:LowTemperatureRadiant:VariableFlow");
    constexpr std::string_view cConstantFlowSystem("ZoneHVAC:LowTemperatureRadiant:ConstantFlow");

    // DERIVED TYPE DEFINITIONS:

    // MODULE VARIABLE DECLARATIONS:
    // Standard, run-of-the-mill variables...

    // Object Data
    [[maybe_unused]] constexpr std::array<std::string_view, (int)CtrlType::Num> ctrlTypeNames = {"MeanAirTemperature",
                                                                                                 "MeanRadiantTemperature",
                                                                                                 "OperativeTemperature",
                                                                                                 "OutdoorDryBulbTemperature",
                                                                                                 "OutdoorWetBulbTemperature",
                                                                                                 "SurfaceFaceTemperature",
                                                                                                 "SurfaceInteriorTemperature",
                                                                                                 "RunningMeanOutdoorDryBulbTemperature"};
    constexpr std::array<std::string_view, (int)CtrlType::Num> ctrlTypeNamesUC = {"MEANAIRTEMPERATURE",
                                                                                  "MEANRADIANTTEMPERATURE",
                                                                                  "OPERATIVETEMPERATURE",
                                                                                  "OUTDOORDRYBULBTEMPERATURE",
                                                                                  "OUTDOORWETBULBTEMPERATURE",
                                                                                  "SURFACEFACETEMPERATURE",
                                                                                  "SURFACEINTERIORTEMPERATURE",
                                                                                  "RUNNINGMEANOUTDOORDRYBULBTEMPERATURE"};

    [[maybe_unused]] constexpr std::array<std::string_view, (int)SetpointType::Num> setpointTypeNames = {"HalfFlowPower", "ZeroFlowPower"};
    constexpr std::array<std::string_view, (int)SetpointType::Num> setpointTypeNamesUC = {"HALFFLOWPOWER", "ZEROFLOWPOWER"};

    [[maybe_unused]] constexpr std::array<std::string_view, (int)FluidToSlabHeatTransferType::Num> fluidToSlabHeatTransferTypeNames = {
        "ConvectionOnly", "ISOStandard"};
    constexpr std::array<std::string_view, (int)FluidToSlabHeatTransferType::Num> fluidToSlabHeatTransferTypeNamesUC = {"CONVECTIONONLY",
                                                                                                                        "ISOSTANDARD"};

    constexpr std::array<std::string_view, (int)CondCtrlType::Num> condCtrlTypeNamesUC = {"OFF", "SIMPLEOFF", "VARIABLEOFF"};

    [[maybe_unused]] constexpr std::array<std::string_view, (int)CircuitCalc::Num> circuitCalcNames = {"OnePerSurface", "CalculateFromCircuitLength"};
    constexpr std::array<std::string_view, (int)CircuitCalc::Num> circuitCalcNamesUC = {"ONEPERSURFACE", "CALCULATEFROMCIRCUITLENGTH"};

    void SimLowTempRadiantSystem(EnergyPlusData &state,
                                 std::string_view CompName,     // name of the low temperature radiant system
                                 bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                 Real64 &LoadMet,               // load met by the radiant system, in Watts
                                 int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int RadSysNum;         // Radiant system number/index in local derived types
        SystemType systemType; // Type of radiant system: hydronic, constant flow, or electric
        bool InitErrorFound(false);

        if (state.dataLowTempRadSys->GetInputFlag) {
            GetLowTempRadiantSystem(state);
            state.dataLowTempRadSys->GetInputFlag = false;
        }

        // Find the correct Low Temp Radiant System
        if (CompIndex == 0) {
            RadSysNum = Util::FindItemInList(CompName, state.dataLowTempRadSys->RadSysTypes);
            if (RadSysNum == 0) {
                ShowFatalError(state, format("SimLowTempRadiantSystem: Unit not found={}", CompName));
            }
            CompIndex = RadSysNum;
            systemType = state.dataLowTempRadSys->RadSysTypes(RadSysNum).systemType;
            switch (systemType) {
            case SystemType::Hydronic: {
                state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex = Util::FindItemInList(CompName, state.dataLowTempRadSys->HydrRadSys);
            } break;
            case SystemType::ConstantFlow: {
                state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex = Util::FindItemInList(CompName, state.dataLowTempRadSys->CFloRadSys);
            } break;
            case SystemType::Electric: {
                state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex = Util::FindItemInList(CompName, state.dataLowTempRadSys->ElecRadSys);
            } break;
            default:
                break;
            }
        } else {
            RadSysNum = CompIndex;
            systemType = state.dataLowTempRadSys->RadSysTypes(RadSysNum).systemType;
            if (RadSysNum > state.dataLowTempRadSys->TotalNumOfRadSystems || RadSysNum < 1) {
                ShowFatalError(state,
                               format("SimLowTempRadiantSystem:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      RadSysNum,
                                      state.dataLowTempRadSys->TotalNumOfRadSystems,
                                      CompName));
            }
            if (state.dataLowTempRadSys->CheckEquipName(RadSysNum)) {
                if (CompName != state.dataLowTempRadSys->RadSysTypes(RadSysNum).Name) {
                    ShowFatalError(state,
                                   format("SimLowTempRadiantSystem: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          RadSysNum,
                                          CompName,
                                          state.dataLowTempRadSys->RadSysTypes(RadSysNum).Name));
                }
                state.dataLowTempRadSys->CheckEquipName(RadSysNum) = false;
            }
        }

        InitLowTempRadiantSystem(state, FirstHVACIteration, state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex, systemType, InitErrorFound);
        if (InitErrorFound) {
            ShowFatalError(state,
                           "InitLowTempRadiantSystem: Preceding error is not allowed to proceed with the simulation.  Correct this input problem.");
        }

        // Simulate, update, and report based on the type of radiant system
        {
            RadiantSystemBaseData *baseSystem;
            if (systemType == SystemType::Hydronic) {
                baseSystem = &state.dataLowTempRadSys->HydrRadSys(state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex);
            } else if (systemType == SystemType::ConstantFlow) {
                baseSystem = &state.dataLowTempRadSys->CFloRadSys(state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex);
            } else if (systemType == SystemType::Electric) {
                baseSystem = &state.dataLowTempRadSys->ElecRadSys(state.dataLowTempRadSys->RadSysTypes(RadSysNum).CompIndex);
            } else {
                ShowFatalError(state, format("SimLowTempRadiantSystem: Illegal system type for system {}", CompName));
            }

            if ((systemType == SystemType::Hydronic) || (systemType == SystemType::ConstantFlow) || (systemType == SystemType::Electric)) {
                baseSystem->calculateLowTemperatureRadiantSystem(state, LoadMet);
                baseSystem->updateLowTemperatureRadiantSystemSurfaces(state);
                baseSystem->updateLowTemperatureRadiantSystem(state); // Nothing to update for electric systems
                baseSystem->reportLowTemperatureRadiantSystem(state);
            }
        }
    }

    void GetLowTempRadiantSystem(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000
        //       MODIFIED       August 2003 (added constant flow system, made input extensible)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads the input for low temperature radiant systems
        // from the user input file.  This will contain all of the information
        // needed to simulate a low temperature radiant system.

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using DataSizing::AutoSize;
        using DataSizing::CapacityPerFloorArea;
        using DataSizing::CoolingDesignCapacity;
        using DataSizing::FractionOfAutosizedCoolingCapacity;
        using DataSizing::FractionOfAutosizedHeatingCapacity;
        using DataSizing::HeatingDesignCapacity;

        using NodeInputManager::GetOnlySingleNode;
        using namespace DataLoopNode;
        using namespace DataSurfaceLists;

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr std::string_view RoutineName("GetLowTempRadiantSystem: "); // include trailing blank space
        constexpr std::string_view routineName = "GetLowTempRadiantSystem";

        int constexpr iHeatCAPMAlphaNum(5);             // get input index to Low Temperature Radiant system heating capacity sizing method
        int constexpr iHeatDesignCapacityNumericNum(1); // get input index to Low Temperature Radiant system electric heating capacity
        int constexpr iHeatCapacityPerFloorAreaNumericNum(
            2); // get input index to Low Temperature Radiant system electric heating capacity per floor area sizing
        int constexpr iHeatFracOfAutosizedCapacityNumericNum(
            3); //  get input index to Low Temperature Radiant system electric heating capacity sizing as fraction of autosized heating capacity

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string CurrentModuleObject;       // for ease in getting objects
        Array1D_string Alphas;                 // Alpha items for object
        Array1D_string cAlphaFields;           // Alpha field names
        Array1D_string cNumericFields;         // Numeric field names
        Array1D_bool AssignedAsRadiantSurface; // Set to true when a surface is part of a radiant system
        int CheckSurfNum;                      // Surface number to check to see if it has already been used by a radiant system
        bool ErrorsFound(false);               // Set to true if errors in input, fatal at end of routine
        int IOStatus;                          // Used in GetObjectItem
        int Item;                              // Item to be "gotten"
        int MaxAlphas;                         // Maximum number of alphas for these input keywords
        int MaxNumbers;                        // Maximum number of numbers for these input keywords
        Array1D<Real64> Numbers;               // Numeric items for object
        int NumAlphas;                         // Number of Alphas for each GetObjectItem call
        int NumArgs;                           // Unused variable that is part of a subroutine call
        int NumNumbers;                        // Number of Numbers for each GetObjectItem call
        int SurfListNum;                       // Index within the SurfList derived type for a surface list name
        int SurfNum;                           // DO loop counter for surfaces
        int BaseNum;                           // Temporary number for creating RadiantSystemTypes structure
        Array1D_bool lAlphaBlanks;             // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;           // Logical array, numeric field input BLANK = .TRUE.

        auto &Zone = state.dataHeatBal->Zone;
        auto &Surface = state.dataSurface->Surface;

        Array1D_string VarFlowRadDesignNames;
        Array1D_string CFlowRadDesignNames;

        MaxAlphas = 0;
        MaxNumbers = 0;

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design", NumArgs, NumAlphas, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNumbers);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design", NumArgs, NumAlphas, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNumbers);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "ZoneHVAC:LowTemperatureRadiant:VariableFlow", NumArgs, NumAlphas, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNumbers);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "ZoneHVAC:LowTemperatureRadiant:ConstantFlow", NumArgs, NumAlphas, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNumbers);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "ZoneHVAC:LowTemperatureRadiant:Electric", NumArgs, NumAlphas, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNumbers);

        Alphas.allocate(MaxAlphas);
        Numbers.dimension(MaxNumbers, 0.0);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNumbers);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNumbers, true);

        state.dataLowTempRadSys->NumOfHydrLowTempRadSys =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:LowTemperatureRadiant:VariableFlow");
        state.dataLowTempRadSys->NumOfCFloLowTempRadSys =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:LowTemperatureRadiant:ConstantFlow");
        state.dataLowTempRadSys->NumOfElecLowTempRadSys =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:LowTemperatureRadiant:Electric");

        state.dataLowTempRadSys->NumOfHydrLowTempRadSysDes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design");
        state.dataLowTempRadSys->NumOfCFloLowTempRadSysDes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design");

        state.dataLowTempRadSys->TotalNumOfRadSystems = state.dataLowTempRadSys->NumOfHydrLowTempRadSys +
                                                        state.dataLowTempRadSys->NumOfElecLowTempRadSys +
                                                        state.dataLowTempRadSys->NumOfCFloLowTempRadSys;
        state.dataLowTempRadSys->RadSysTypes.allocate(state.dataLowTempRadSys->TotalNumOfRadSystems);
        state.dataLowTempRadSys->LowTempRadUniqueNames.reserve(static_cast<unsigned>(state.dataLowTempRadSys->TotalNumOfRadSystems));
        state.dataLowTempRadSys->CheckEquipName.dimension(state.dataLowTempRadSys->TotalNumOfRadSystems, true);

        state.dataLowTempRadSys->HydrRadSys.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSys);
        if (state.dataLowTempRadSys->NumOfHydrLowTempRadSys > 0) {
            auto *water = Fluid::GetWater(state);
            if (water == nullptr) {
                ShowSevereError(state, "Hydronic radiant systems: no water property data found in input");
                ErrorsFound = true;
            }

            for (auto &e : state.dataLowTempRadSys->HydrRadSys) {
                e.water = water;
            }
        }

        state.dataLowTempRadSys->CFloRadSys.allocate(state.dataLowTempRadSys->NumOfCFloLowTempRadSys);
        if (state.dataLowTempRadSys->NumOfCFloLowTempRadSys > 0) {
            auto *water = Fluid::GetWater(state);
            if (water == nullptr) {
                ShowSevereError(state, "Constant flow radiant systems: no water property data found in input");
                ErrorsFound = true;
            }

            for (auto &e : state.dataLowTempRadSys->CFloRadSys) {
                e.water = water;
            }
        }

        state.dataLowTempRadSys->ElecRadSys.allocate(state.dataLowTempRadSys->NumOfElecLowTempRadSys);
        state.dataLowTempRadSys->ElecRadSysNumericFields.allocate(state.dataLowTempRadSys->NumOfElecLowTempRadSys);

        state.dataLowTempRadSys->HydronicRadiantSysNumericFields.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSys);
        state.dataLowTempRadSys->HydronicRadiantSysDesign.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSysDes);
        VarFlowRadDesignNames.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSysDes);

        state.dataLowTempRadSys->CflowRadiantSysDesign.allocate(state.dataLowTempRadSys->NumOfCFloLowTempRadSysDes);
        CFlowRadDesignNames.allocate(state.dataLowTempRadSys->NumOfCFloLowTempRadSysDes);

        // make sure data is gotten for surface lists
        GetNumberOfSurfaceLists(state);

        // Obtain all of the design data related to hydronic low temperature radiant systems...
        CurrentModuleObject = "ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design";
        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfHydrLowTempRadSysDes; ++Item) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            state.dataLowTempRadSys->HydronicRadiantSysDesign(Item).FieldNames.allocate(NumNumbers);
            state.dataLowTempRadSys->HydronicRadiantSysDesign(Item).FieldNames = "";
            state.dataLowTempRadSys->HydronicRadiantSysDesign(Item).FieldNames = cNumericFields;
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataLowTempRadSys->LowTempRadUniqueNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            auto &thisRadSysDesign = state.dataLowTempRadSys->HydronicRadiantSysDesign(Item);

            // General user input data
            thisRadSysDesign.designName = Alphas(1);

            if (lAlphaBlanks(2)) {
                thisRadSysDesign.FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ConvectionOnly;
            } else if ((thisRadSysDesign.FluidToSlabHeatTransfer = static_cast<FluidToSlabHeatTransferType>(
                            getEnumValue(fluidToSlabHeatTransferTypeNamesUC, Alphas(2)))) == FluidToSlabHeatTransferType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(2), Alphas(2), "ConvectionOnly");
                thisRadSysDesign.FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ConvectionOnly;
            }

            thisRadSysDesign.TubeDiameterInner = Numbers(1);
            thisRadSysDesign.TubeDiameterOuter = Numbers(2);

            thisRadSysDesign.VarFlowTubeConductivity = Numbers(3);

            // Process the temperature control type
            if (lAlphaBlanks(3)) {
                thisRadSysDesign.VarFlowControlType = CtrlType::MAT;
            } else if ((thisRadSysDesign.VarFlowControlType = static_cast<CtrlType>(getEnumValue(ctrlTypeNamesUC, Alphas(3)))) == CtrlType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(3), Alphas(3), "MeanAirTemperature");
                thisRadSysDesign.VarFlowControlType = CtrlType::MAT;
            }

            // Process the setpoint type
            if (lAlphaBlanks(4)) {
                thisRadSysDesign.VarFlowSetpointType = SetpointType::HalfFlowPower;
            } else if ((thisRadSysDesign.VarFlowSetpointType = static_cast<SetpointType>(getEnumValue(setpointTypeNamesUC, Alphas(4)))) ==
                       SetpointType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(4), Alphas(4), "HalfFlowPower");
                thisRadSysDesign.VarFlowSetpointType = SetpointType::HalfFlowPower;
            }

            // Refactor everything below to Alphas as HCMethod, etc

            // Determine Low Temp Radiant heating design capacity sizing method
            thisRadSysDesign.DesignHeatingCapMethodInput = Alphas(5);
            if (Util::SameString(thisRadSysDesign.DesignHeatingCapMethodInput, "HeatingDesignCapacity")) {
                thisRadSysDesign.DesignHeatingCapMethod = HeatingDesignCapacity;
            } else if (Util::SameString(thisRadSysDesign.DesignHeatingCapMethodInput, "CapacityPerFloorArea")) {
                thisRadSysDesign.DesignHeatingCapMethod = CapacityPerFloorArea;
                if (!lNumericBlanks(4)) {
                    thisRadSysDesign.DesignScaledHeatingCapacity = Numbers(4);
                    if (thisRadSysDesign.DesignScaledHeatingCapacity <= 0.0) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                        ShowContinueError(state, format("Input for {} = {}", cAlphaFields(5), thisRadSysDesign.DesignHeatingCapMethodInput));
                        ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(4), Numbers(4)));
                        ErrorsFound = true;
                    } else if (thisRadSysDesign.DesignScaledHeatingCapacity == AutoSize) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                        ShowContinueError(state, format("Input for {} = {}", cAlphaFields(5), thisRadSysDesign.DesignHeatingCapMethodInput));
                        ShowContinueError(state, format("Illegal {} = Autosize", cNumericFields(4)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.Name));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(5), thisRadSysDesign.DesignHeatingCapMethodInput));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(4)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(thisRadSysDesign.DesignHeatingCapMethodInput, "FractionOfAutosizedHeatingCapacity")) {
                thisRadSysDesign.DesignHeatingCapMethod = FractionOfAutosizedHeatingCapacity;
                if (!lNumericBlanks(5)) {
                    thisRadSysDesign.DesignScaledHeatingCapacity = Numbers(5);
                    if (thisRadSysDesign.DesignScaledHeatingCapacity < 0.0) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                        ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(5), Numbers(5)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(5), thisRadSysDesign.DesignHeatingCapMethodInput));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(5)));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), thisRadSysDesign.DesignHeatingCapMethodInput));
                ErrorsFound = true;
            }

            thisRadSysDesign.HotThrottlRange = Numbers(6);

            if (lAlphaBlanks(6)) {
            } else if ((thisRadSysDesign.heatSetptSched = Sched::GetSchedule(state, Alphas(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), Alphas(6));
                ErrorsFound = true;
            }

            // Determine Low Temp Radiant cooling design capacity sizing method
            thisRadSysDesign.DesignCoolingCapMethodInput = Alphas(7);
            if (Util::SameString(thisRadSysDesign.DesignCoolingCapMethodInput, "CoolingDesignCapacity")) {
                thisRadSysDesign.DesignCoolingCapMethod = CoolingDesignCapacity;
            } else if (Util::SameString(thisRadSysDesign.DesignCoolingCapMethodInput, "CapacityPerFloorArea")) {
                thisRadSysDesign.DesignCoolingCapMethod = CapacityPerFloorArea;
                if (!lNumericBlanks(7)) {
                    thisRadSysDesign.DesignScaledCoolingCapacity = Numbers(7);
                    if (thisRadSysDesign.DesignScaledCoolingCapacity <= 0.0) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                        ShowContinueError(state, format("Input for {} = {}", cAlphaFields(7), thisRadSysDesign.DesignCoolingCapMethodInput));
                        ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(7), thisRadSysDesign.DesignScaledCoolingCapacity));
                        ErrorsFound = true;
                    } else if (thisRadSysDesign.DesignScaledCoolingCapacity == AutoSize) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                        ShowContinueError(state, format("Input for {} = {}", cAlphaFields(7), thisRadSysDesign.DesignCoolingCapMethodInput));
                        ShowContinueError(state, format("Illegal {} = Autosize", cNumericFields(7)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(7), thisRadSysDesign.DesignCoolingCapMethodInput));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(7)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(thisRadSysDesign.DesignCoolingCapMethodInput, "FractionOfAutosizedCoolingCapacity")) {
                thisRadSysDesign.DesignCoolingCapMethod = FractionOfAutosizedCoolingCapacity;
                if (!lNumericBlanks(8)) {
                    thisRadSysDesign.DesignScaledCoolingCapacity = Numbers(8);
                    if (thisRadSysDesign.DesignScaledCoolingCapacity < 0.0) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                        ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(8), Numbers(8)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(7), thisRadSysDesign.DesignCoolingCapMethodInput));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(8)));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSysDesign.designName));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(7), thisRadSysDesign.DesignCoolingCapMethodInput));
                ErrorsFound = true;
            }

            thisRadSysDesign.ColdThrottlRange = Numbers(9);

            if (lAlphaBlanks(8)) {
            } else if ((thisRadSysDesign.coolSetptSched = Sched::GetSchedule(state, Alphas(8))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(8), Alphas(8));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(9)) {
                thisRadSysDesign.condCtrlType = CondCtrlType::SimpleOff;
            } else if ((thisRadSysDesign.condCtrlType = static_cast<CondCtrlType>(getEnumValue(condCtrlTypeNamesUC, Alphas(9)))) ==
                       CondCtrlType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(9), Alphas(9), "SimpleOff");
                thisRadSysDesign.condCtrlType = CondCtrlType::SimpleOff;
            }

            thisRadSysDesign.CondDewPtDeltaT = Numbers(10);

            if (lAlphaBlanks(10)) {
            } else if ((thisRadSysDesign.changeoverDelaySched = Sched::GetSchedule(state, Alphas(10))) == nullptr) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(10), Alphas(10), "No changeover delay will be used for this radiant system.");
            }

            VarFlowRadDesignNames(Item) = Alphas(1);
        }

        // Obtain all of the user data related to hydronic low temperature radiant systems...
        BaseNum = 0;
        CurrentModuleObject = "ZoneHVAC:LowTemperatureRadiant:VariableFlow";
        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfHydrLowTempRadSys; ++Item) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            state.dataLowTempRadSys->HydronicRadiantSysNumericFields(Item).FieldNames.allocate(NumNumbers);
            state.dataLowTempRadSys->HydronicRadiantSysNumericFields(Item).FieldNames = "";
            state.dataLowTempRadSys->HydronicRadiantSysNumericFields(Item).FieldNames = cNumericFields;
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataLowTempRadSys->LowTempRadUniqueNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            ++BaseNum;
            state.dataLowTempRadSys->RadSysTypes(BaseNum).Name = Alphas(1);
            state.dataLowTempRadSys->RadSysTypes(BaseNum).systemType = SystemType::Hydronic;

            auto &thisRadSys = state.dataLowTempRadSys->HydrRadSys(Item);

            // General user input data
            thisRadSys.Name = Alphas(1);

            Util::setDesignObjectNameAndPointer(state,
                                                thisRadSys.designObjectName,
                                                thisRadSys.DesignObjectPtr,
                                                Alphas(2),
                                                VarFlowRadDesignNames,
                                                CurrentModuleObject,
                                                Alphas(1),
                                                ErrorsFound);
            if (ErrorsFound) {
                break;
            }
            VarFlowRadDesignData variableFlowDesignDataObject{state.dataLowTempRadSys->HydronicRadiantSysDesign(
                thisRadSys.DesignObjectPtr)}; // Contains the data for variable flow hydronic systems

            if (lAlphaBlanks(3)) {
                thisRadSys.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisRadSys.availSched = Sched::GetSchedule(state, Alphas(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(3), Alphas(3));
                ErrorsFound = true;
            }

            thisRadSys.ZoneName = Alphas(4);
            thisRadSys.ZonePtr = Util::FindItemInList(Alphas(4), Zone);
            if (thisRadSys.ZonePtr == 0) {
                ShowSevereError(state, format("{}Invalid {} = {}", RoutineName, cAlphaFields(3), Alphas(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            thisRadSys.SurfListName = Alphas(5);
            SurfListNum = 0;
            if (state.dataSurfLists->NumOfSurfaceLists > 0) {
                SurfListNum = Util::FindItemInList(thisRadSys.SurfListName, state.dataSurfLists->SurfList);
            }
            if (SurfListNum > 0) { // Found a valid surface list
                thisRadSys.NumOfSurfaces = state.dataSurfLists->SurfList(SurfListNum).NumOfSurfaces;
                thisRadSys.SurfacePtr.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.SurfaceName.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.SurfaceFrac.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.NumCircuits.allocate(thisRadSys.NumOfSurfaces);
                for (SurfNum = 1; SurfNum <= state.dataSurfLists->SurfList(SurfListNum).NumOfSurfaces; ++SurfNum) {
                    thisRadSys.SurfacePtr(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfPtr(SurfNum);
                    thisRadSys.SurfaceName(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfName(SurfNum);
                    thisRadSys.SurfaceFrac(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfFlowFrac(SurfNum);
                    if (thisRadSys.SurfacePtr(SurfNum) > 0) {
                        state.dataSurface->surfIntConv(thisRadSys.SurfacePtr(SurfNum)).hasActiveInIt = true;
                    }
                }
            } else { // User entered a single surface name rather than a surface list
                thisRadSys.NumOfSurfaces = 1;
                thisRadSys.SurfacePtr.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.SurfaceName.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.SurfaceFrac.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.NumCircuits.allocate(thisRadSys.NumOfSurfaces);
                thisRadSys.SurfaceName(1) = thisRadSys.SurfListName;
                thisRadSys.SurfacePtr(1) = Util::FindItemInList(thisRadSys.SurfaceName(1), Surface);
                thisRadSys.SurfaceFrac(1) = 1.0;
                thisRadSys.NumCircuits(1) = 0.0;
                // Error checking for single surfaces
                if (thisRadSys.SurfacePtr(1) == 0) {
                    ShowSevereError(state, format("{}Invalid {} = {}", RoutineName, cAlphaFields(5), Alphas(5)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else if (state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(thisRadSys.SurfacePtr(1))) {
                    ShowSevereError(state, format("{}{}=\"{}\", Invalid Surface", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      format("{}=\"{}\" has been used in another radiant system or ventilated slab.", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
                if (thisRadSys.SurfacePtr(1) != 0) {
                    state.dataSurface->surfIntConv(thisRadSys.SurfacePtr(1)).hasActiveInIt = true;
                    state.dataSurface->surfIntConv(thisRadSys.SurfacePtr(1)).hasActiveInIt = true; // Ummmm ... what?
                }
            }

            // Error checking for zones and construction information
            thisRadSys.errorCheckZonesAndConstructions(state, ErrorsFound);

            thisRadSys.TubeLength = Numbers(1);

            // Determine Low Temp Radiant heating design capacity sizing method
            if (variableFlowDesignDataObject.DesignHeatingCapMethod == HeatingDesignCapacity) {
                thisRadSys.HeatingCapMethod = HeatingDesignCapacity;
                if (!lNumericBlanks(2)) {
                    thisRadSys.ScaledHeatingCapacity = Numbers(2);
                    if (thisRadSys.ScaledHeatingCapacity < 0.0 && thisRadSys.ScaledHeatingCapacity != AutoSize) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSys.Name));
                        ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(2), Numbers(2)));
                        ErrorsFound = true;
                    }
                } else {
                    if ((!lAlphaBlanks(6)) || (!lAlphaBlanks(7))) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSys.Name));
                        ShowContinueError(state, "Input for Heating Design Capacity Method = HeatingDesignCapacity");
                        ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(2)));
                        ErrorsFound = true;
                    }
                }
            } else if (variableFlowDesignDataObject.DesignHeatingCapMethod == CapacityPerFloorArea) {
                thisRadSys.HeatingCapMethod = CapacityPerFloorArea;
                thisRadSys.ScaledHeatingCapacity = variableFlowDesignDataObject.DesignScaledHeatingCapacity;
            } else if (variableFlowDesignDataObject.DesignHeatingCapMethod == FractionOfAutosizedHeatingCapacity) {
                thisRadSys.HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
                thisRadSys.ScaledHeatingCapacity = variableFlowDesignDataObject.DesignScaledHeatingCapacity;
            }

            // Heating user input data
            thisRadSys.WaterVolFlowMaxHeat = Numbers(3);

            thisRadSys.HotWaterInNode = GetOnlySingleNode(state,
                                                          Alphas(6),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantVariableFlow,
                                                          Alphas(1),
                                                          DataLoopNode::NodeFluidType::Water,
                                                          DataLoopNode::ConnectionType::Inlet,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          ObjectIsNotParent);

            thisRadSys.HotWaterOutNode = GetOnlySingleNode(state,
                                                           Alphas(7),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantVariableFlow,
                                                           Alphas(1),
                                                           DataLoopNode::NodeFluidType::Water,
                                                           DataLoopNode::ConnectionType::Outlet,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsNotParent);

            if ((!lAlphaBlanks(6)) || (!lAlphaBlanks(7))) {
                TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(6), Alphas(7), "Hot Water Nodes");
            }
            if ((thisRadSys.WaterVolFlowMaxHeat == AutoSize) &&
                (lAlphaBlanks(6) || lAlphaBlanks(7) || (thisRadSys.HotWaterInNode <= 0) || (thisRadSys.HotWaterOutNode <= 0) ||
                 (variableFlowDesignDataObject.heatSetptSched == nullptr))) {
                ShowSevereError(state, "Hydronic radiant systems may not be autosized without specification of nodes or schedules.");
                ShowContinueError(state, format("Occurs in {} (heating input) = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            // Determine Low Temp Radiant cooling design capacity sizing method
            if (variableFlowDesignDataObject.DesignCoolingCapMethod == CoolingDesignCapacity) {
                thisRadSys.CoolingCapMethod = CoolingDesignCapacity;
                if (!lNumericBlanks(4)) {
                    thisRadSys.ScaledCoolingCapacity = Numbers(4);
                    if (thisRadSys.ScaledCoolingCapacity < 0.0 && thisRadSys.ScaledCoolingCapacity != AutoSize) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSys.Name));
                        ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(4), Numbers(4)));
                        ErrorsFound = true;
                    }
                } else {
                    if ((!lAlphaBlanks(8)) || (!lAlphaBlanks(9))) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisRadSys.Name));
                        ShowContinueError(state, "Input for Cooling Design Capacity Method = CoolingDesignCapacity");
                        ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(4)));
                        ErrorsFound = true;
                    }
                }
            } else if (variableFlowDesignDataObject.DesignCoolingCapMethod == CapacityPerFloorArea) {
                thisRadSys.CoolingCapMethod = CapacityPerFloorArea;
                thisRadSys.ScaledCoolingCapacity = variableFlowDesignDataObject.DesignScaledCoolingCapacity;
            } else if (variableFlowDesignDataObject.DesignCoolingCapMethod == FractionOfAutosizedCoolingCapacity) {
                thisRadSys.CoolingCapMethod = FractionOfAutosizedCoolingCapacity;
                thisRadSys.ScaledCoolingCapacity = variableFlowDesignDataObject.DesignScaledCoolingCapacity;
            }

            // Cooling user input data
            thisRadSys.WaterVolFlowMaxCool = Numbers(5);

            thisRadSys.ColdWaterInNode = GetOnlySingleNode(state,
                                                           Alphas(8),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantVariableFlow,
                                                           Alphas(1),
                                                           DataLoopNode::NodeFluidType::Water,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Secondary,
                                                           ObjectIsNotParent);

            thisRadSys.ColdWaterOutNode = GetOnlySingleNode(state,
                                                            Alphas(9),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantVariableFlow,
                                                            Alphas(1),
                                                            DataLoopNode::NodeFluidType::Water,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Secondary,
                                                            ObjectIsNotParent);

            if ((!lAlphaBlanks(8)) || (!lAlphaBlanks(9))) {
                TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9), "Chilled Water Nodes");
            }

            if (lAlphaBlanks(10)) {
                thisRadSys.NumCircCalcMethod = CircuitCalc::OneCircuit;
            } else if ((thisRadSys.NumCircCalcMethod = static_cast<CircuitCalc>(getEnumValue(circuitCalcNamesUC, Alphas(10)))) ==
                       CircuitCalc::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(10), Alphas(10), "OnePerSurface");
                thisRadSys.NumCircCalcMethod = CircuitCalc::OneCircuit;
            }

            thisRadSys.changeoverDelaySched = variableFlowDesignDataObject.changeoverDelaySched;

            thisRadSys.CircLength = Numbers(6);

            if ((thisRadSys.WaterVolFlowMaxCool == AutoSize) &&
                (variableFlowDesignDataObject.DesignCoolingCapMethod == 0 || lAlphaBlanks(8) || lAlphaBlanks(9) ||
                 (thisRadSys.ColdWaterInNode <= 0) || (thisRadSys.ColdWaterOutNode <= 0) ||
                 (variableFlowDesignDataObject.coolSetptSched == nullptr))) {
                ShowSevereError(state, "Hydronic radiant systems may not be autosized without specification of nodes or schedules");
                ShowContinueError(state, format("Occurs in {} (cooling input) ={}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }
        }

        // Obtain all of the design data related to Constant flow low temperature radiant systems...
        CurrentModuleObject = "ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design";
        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfCFloLowTempRadSysDes; ++Item) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            state.dataLowTempRadSys->CflowRadiantSysDesign(Item).FieldNames.allocate(NumNumbers);
            state.dataLowTempRadSys->CflowRadiantSysDesign(Item).FieldNames = "";
            state.dataLowTempRadSys->CflowRadiantSysDesign(Item).FieldNames = cNumericFields;
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataLowTempRadSys->LowTempRadUniqueNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            auto &thisRadSysDesign = state.dataLowTempRadSys->CflowRadiantSysDesign(Item);

            // General user input data
            thisRadSysDesign.designName = Alphas(1);

            if (lAlphaBlanks(2)) {
                thisRadSysDesign.FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ConvectionOnly;
            } else if ((thisRadSysDesign.FluidToSlabHeatTransfer = static_cast<FluidToSlabHeatTransferType>(
                            getEnumValue(fluidToSlabHeatTransferTypeNamesUC, Alphas(2)))) == FluidToSlabHeatTransferType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(2), Alphas(2), "ConvectionOnly");
                thisRadSysDesign.FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ConvectionOnly;
            }

            thisRadSysDesign.TubeDiameterInner = Numbers(1);
            thisRadSysDesign.TubeDiameterOuter = Numbers(2);
            thisRadSysDesign.ConstFlowTubeConductivity = Numbers(3);

            // Process the temperature control type
            if (lAlphaBlanks(3)) {
                thisRadSysDesign.ConstFlowControlType = CtrlType::MAT;
            } else if ((thisRadSysDesign.ConstFlowControlType = static_cast<CtrlType>(getEnumValue(ctrlTypeNamesUC, Alphas(3)))) ==
                       CtrlType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(3), Alphas(3), "MeanAirTemperature");
                thisRadSysDesign.ConstFlowControlType = CtrlType::MAT;
            } else if (thisRadSysDesign.ConstFlowControlType == CtrlType::RunningMeanODB) {
                state.dataLowTempRadSys->anyRadiantSystemUsingRunningMeanAverage = true;
            }

            thisRadSysDesign.runningMeanOutdoorAirTemperatureWeightingFactor = Numbers(4);
            thisRadSysDesign.MotorEffic = Numbers(5);
            thisRadSysDesign.FracMotorLossToFluid = Numbers(6);

            if (lAlphaBlanks(4)) {
                thisRadSysDesign.condCtrlType = CondCtrlType::SimpleOff;
            } else if ((thisRadSysDesign.condCtrlType = static_cast<CondCtrlType>(getEnumValue(condCtrlTypeNamesUC, Alphas(4)))) ==
                       CondCtrlType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(4), Alphas(4), "SimpleOff");
                thisRadSysDesign.condCtrlType = CondCtrlType::SimpleOff;
            }

            thisRadSysDesign.CondDewPtDeltaT = Numbers(7);

            if (lAlphaBlanks(5)) {
            } else if ((thisRadSysDesign.changeoverDelaySched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(5), Alphas(5), "No changeover delay will be used for this radiant system.");
            }
            CFlowRadDesignNames(Item) = Alphas(1);
        }

        // Obtain all of the user data related to constant flow (hydronic) low temperature radiant systems...
        CurrentModuleObject = "ZoneHVAC:LowTemperatureRadiant:ConstantFlow";
        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfCFloLowTempRadSys; ++Item) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataLowTempRadSys->LowTempRadUniqueNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
            ++BaseNum;
            state.dataLowTempRadSys->RadSysTypes(BaseNum).Name = Alphas(1);
            state.dataLowTempRadSys->RadSysTypes(BaseNum).systemType = SystemType::ConstantFlow;

            // General user input data
            auto &thisCFloSys = state.dataLowTempRadSys->CFloRadSys(Item);

            thisCFloSys.Name = Alphas(1);
            Util::setDesignObjectNameAndPointer(state,
                                                thisCFloSys.designObjectName,
                                                thisCFloSys.DesignObjectPtr,
                                                Alphas(2),
                                                CFlowRadDesignNames,
                                                CurrentModuleObject,
                                                Alphas(1),
                                                ErrorsFound);
            if (ErrorsFound) {
                break;
            }
            ConstantFlowRadDesignData ConstantFlowRadDesignDataObject{
                state.dataLowTempRadSys->CflowRadiantSysDesign(thisCFloSys.DesignObjectPtr)}; // Contains the data for variable flow hydronic systems

            if (lAlphaBlanks(3)) {
                thisCFloSys.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisCFloSys.availSched = Sched::GetSchedule(state, Alphas(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(3), Alphas(3));
                ErrorsFound = true;
            }

            thisCFloSys.ZoneName = Alphas(4);
            thisCFloSys.ZonePtr = Util::FindItemInList(Alphas(4), Zone);
            if (thisCFloSys.ZonePtr == 0) {
                ShowSevereError(state, format("{}Invalid {} = {}", RoutineName, cAlphaFields(4), Alphas(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            thisCFloSys.SurfListName = Alphas(5);
            SurfListNum = 0;
            if (state.dataSurfLists->NumOfSurfaceLists > 0) {
                SurfListNum = Util::FindItemInList(thisCFloSys.SurfListName, state.dataSurfLists->SurfList);
            }
            if (SurfListNum > 0) { // Found a valid surface list
                thisCFloSys.NumOfSurfaces = state.dataSurfLists->SurfList(SurfListNum).NumOfSurfaces;
                thisCFloSys.SurfacePtr.allocate(thisCFloSys.NumOfSurfaces);
                thisCFloSys.SurfaceName.allocate(thisCFloSys.NumOfSurfaces);
                thisCFloSys.SurfaceFrac.allocate(thisCFloSys.NumOfSurfaces);
                thisCFloSys.NumCircuits.allocate(thisCFloSys.NumOfSurfaces);
                state.dataLowTempRadSys->MaxCloNumOfSurfaces = max(state.dataLowTempRadSys->MaxCloNumOfSurfaces, thisCFloSys.NumOfSurfaces);
                for (SurfNum = 1; SurfNum <= state.dataSurfLists->SurfList(SurfListNum).NumOfSurfaces; ++SurfNum) {
                    thisCFloSys.SurfacePtr(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfPtr(SurfNum);
                    thisCFloSys.SurfaceName(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfName(SurfNum);
                    thisCFloSys.SurfaceFrac(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfFlowFrac(SurfNum);
                    thisCFloSys.NumCircuits(SurfNum) = 0.0;
                    if (thisCFloSys.SurfacePtr(SurfNum) != 0) {
                        state.dataSurface->surfIntConv(thisCFloSys.SurfacePtr(SurfNum)).hasActiveInIt = true;
                    }
                }
            } else { // User entered a single surface name rather than a surface list
                thisCFloSys.NumOfSurfaces = 1;
                thisCFloSys.SurfacePtr.allocate(thisCFloSys.NumOfSurfaces);
                thisCFloSys.SurfaceName.allocate(thisCFloSys.NumOfSurfaces);
                thisCFloSys.SurfaceFrac.allocate(thisCFloSys.NumOfSurfaces);
                thisCFloSys.NumCircuits.allocate(thisCFloSys.NumOfSurfaces);
                state.dataLowTempRadSys->MaxCloNumOfSurfaces = max(state.dataLowTempRadSys->MaxCloNumOfSurfaces, thisCFloSys.NumOfSurfaces);
                thisCFloSys.SurfaceName(1) = thisCFloSys.SurfListName;
                thisCFloSys.SurfacePtr(1) = Util::FindItemInList(thisCFloSys.SurfaceName(1), Surface);
                thisCFloSys.SurfaceFrac(1) = 1.0;
                thisCFloSys.NumCircuits(1) = 0.0;
                // Error checking for single surfaces
                if (thisCFloSys.SurfacePtr(1) == 0) {
                    ShowSevereError(state, format("{}Invalid {} = {}", RoutineName, cAlphaFields(4), Alphas(4)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else if (state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(thisCFloSys.SurfacePtr(1))) {
                    ShowSevereError(state, format("{}{}=\"{}\", Invalid Surface", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      format("{}=\"{}\" has been used in another radiant system or ventilated slab.", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
                if (thisCFloSys.SurfacePtr(1) != 0) {
                    state.dataSurface->surfIntConv(thisCFloSys.SurfacePtr(1)).hasActiveInIt = true;
                    state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(thisCFloSys.SurfacePtr(1)) = true;
                }
            }

            // Error checking for zones and construction information
            thisCFloSys.errorCheckZonesAndConstructions(state, ErrorsFound);

            thisCFloSys.TubeLength = Numbers(1);

            // Process pump input for constant flow (hydronic) radiant system
            thisCFloSys.WaterVolFlowMax = Numbers(2);

            if (lAlphaBlanks(6)) {
            } else if ((thisCFloSys.volFlowSched = Sched::GetSchedule(state, Alphas(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), Alphas(6));
                ErrorsFound = true;
            }
            thisCFloSys.NomPumpHead = Numbers(3);
            thisCFloSys.NomPowerUse = Numbers(4);

            // Heating user input data
            thisCFloSys.HotWaterInNode = GetOnlySingleNode(state,
                                                           Alphas(7),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantConstantFlow,
                                                           Alphas(1),
                                                           DataLoopNode::NodeFluidType::Water,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsNotParent);

            thisCFloSys.HotWaterOutNode = GetOnlySingleNode(state,
                                                            Alphas(8),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantConstantFlow,
                                                            Alphas(1),
                                                            DataLoopNode::NodeFluidType::Water,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            ObjectIsNotParent);

            if ((!lAlphaBlanks(7)) || (!lAlphaBlanks(8))) {
                TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(7), Alphas(8), "Hot Water Nodes");
            }

            if (lAlphaBlanks(9)) {
            } else if ((thisCFloSys.hotWaterHiTempSched = Sched::GetSchedule(state, Alphas(9))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(9), Alphas(9));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(10)) {
            } else if ((thisCFloSys.hotWaterLoTempSched = Sched::GetSchedule(state, Alphas(10))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(10), Alphas(10));
                ErrorsFound = true;
            }

            // This may look like a weird thing to do, but it's equivalent to a nested if and also uses less nesting
            if (lAlphaBlanks(11)) {
            } else if ((thisCFloSys.hotCtrlHiTempSched = Sched::GetSchedule(state, Alphas(11))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), Alphas(11));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(12)) {
            } else if ((thisCFloSys.hotCtrlLoTempSched = Sched::GetSchedule(state, Alphas(12))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(12), Alphas(12));
                ErrorsFound = true;
            }

            // Cooling user input data
            thisCFloSys.ColdWaterInNode = GetOnlySingleNode(state,
                                                            Alphas(13),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantConstantFlow,
                                                            Alphas(1),
                                                            DataLoopNode::NodeFluidType::Water,
                                                            DataLoopNode::ConnectionType::Inlet,
                                                            NodeInputManager::CompFluidStream::Secondary,
                                                            ObjectIsNotParent);

            thisCFloSys.ColdWaterOutNode = GetOnlySingleNode(state,
                                                             Alphas(14),
                                                             ErrorsFound,
                                                             DataLoopNode::ConnectionObjectType::ZoneHVACLowTemperatureRadiantConstantFlow,
                                                             Alphas(1),
                                                             DataLoopNode::NodeFluidType::Water,
                                                             DataLoopNode::ConnectionType::Outlet,
                                                             NodeInputManager::CompFluidStream::Secondary,
                                                             ObjectIsNotParent);

            if ((!lAlphaBlanks(13)) || (!lAlphaBlanks(14))) {
                TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(13), Alphas(14), "Chilled Water Nodes");
            }

            if (lAlphaBlanks(15)) {
            } else if ((thisCFloSys.coldWaterHiTempSched = Sched::GetSchedule(state, Alphas(15))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(15), Alphas(15));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(16)) {
            } else if ((thisCFloSys.coldWaterLoTempSched = Sched::GetSchedule(state, Alphas(16))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(16), Alphas(16));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(17)) {
            } else if ((thisCFloSys.coldCtrlHiTempSched = Sched::GetSchedule(state, Alphas(17))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(17), Alphas(17));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(18)) {
            } else if ((thisCFloSys.coldCtrlLoTempSched = Sched::GetSchedule(state, Alphas(18))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(18), Alphas(18));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(19)) {
                thisCFloSys.NumCircCalcMethod = CircuitCalc::OneCircuit;
            } else if ((thisCFloSys.NumCircCalcMethod = static_cast<CircuitCalc>(getEnumValue(circuitCalcNamesUC, Alphas(19)))) ==
                       CircuitCalc::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(19), Alphas(19), "OnePerSurface");
                thisCFloSys.NumCircCalcMethod = CircuitCalc::OneCircuit;
            }

            thisCFloSys.changeoverDelaySched = ConstantFlowRadDesignDataObject.changeoverDelaySched;

            thisCFloSys.CircLength = Numbers(5);
        }

        // Obtain all of the user data related to electric low temperature radiant systems...
        CurrentModuleObject = "ZoneHVAC:LowTemperatureRadiant:Electric";

        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfElecLowTempRadSys; ++Item) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     Item,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            state.dataLowTempRadSys->ElecRadSysNumericFields(Item).FieldNames.allocate(NumNumbers);
            state.dataLowTempRadSys->ElecRadSysNumericFields(Item).FieldNames = "";
            state.dataLowTempRadSys->ElecRadSysNumericFields(Item).FieldNames = cNumericFields;

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataLowTempRadSys->LowTempRadUniqueNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
            ++BaseNum;
            state.dataLowTempRadSys->RadSysTypes(BaseNum).Name = Alphas(1);
            state.dataLowTempRadSys->RadSysTypes(BaseNum).systemType = SystemType::Electric;

            // General user input data
            auto &thisElecSys = state.dataLowTempRadSys->ElecRadSys(Item);

            thisElecSys.Name = Alphas(1);

            if (lAlphaBlanks(2)) {
                thisElecSys.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisElecSys.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            thisElecSys.ZoneName = Alphas(3);
            thisElecSys.ZonePtr = Util::FindItemInList(Alphas(3), Zone);
            if (thisElecSys.ZonePtr == 0) {
                ShowSevereError(state, format("{}Invalid {} = {}", RoutineName, cAlphaFields(3), Alphas(3)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            thisElecSys.SurfListName = Alphas(4);
            SurfListNum = 0;
            if (state.dataSurfLists->NumOfSurfaceLists > 0) {
                SurfListNum = Util::FindItemInList(thisElecSys.SurfListName, state.dataSurfLists->SurfList);
            }
            if (SurfListNum > 0) { // Found a valid surface list
                thisElecSys.NumOfSurfaces = state.dataSurfLists->SurfList(SurfListNum).NumOfSurfaces;
                thisElecSys.SurfacePtr.allocate(thisElecSys.NumOfSurfaces);
                thisElecSys.SurfaceName.allocate(thisElecSys.NumOfSurfaces);
                thisElecSys.SurfaceFrac.allocate(thisElecSys.NumOfSurfaces);
                for (SurfNum = 1; SurfNum <= state.dataSurfLists->SurfList(SurfListNum).NumOfSurfaces; ++SurfNum) {
                    thisElecSys.SurfacePtr(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfPtr(SurfNum);
                    thisElecSys.SurfaceName(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfName(SurfNum);
                    thisElecSys.SurfaceFrac(SurfNum) = state.dataSurfLists->SurfList(SurfListNum).SurfFlowFrac(SurfNum);
                }
            } else { // User entered a single surface name rather than a surface list
                thisElecSys.NumOfSurfaces = 1;
                thisElecSys.SurfacePtr.allocate(thisElecSys.NumOfSurfaces);
                thisElecSys.SurfaceName.allocate(thisElecSys.NumOfSurfaces);
                thisElecSys.SurfaceFrac.allocate(thisElecSys.NumOfSurfaces);
                thisElecSys.SurfaceName(1) = thisElecSys.SurfListName;
                thisElecSys.SurfacePtr(1) = Util::FindItemInList(thisElecSys.SurfaceName(1), Surface);
                thisElecSys.SurfaceFrac(1) = 1.0;
                // Error checking for single surfaces
                if (thisElecSys.SurfacePtr(1) == 0) {
                    ShowSevereError(state, format("{}Invalid {} = {}", RoutineName, cAlphaFields(4), Alphas(4)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else if (state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(thisElecSys.SurfacePtr(1))) {
                    ShowSevereError(state, format("{}{}=\"{}\", Invalid Surface", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      format("{}=\"{}\" has been used in another radiant system or ventilated slab.", cAlphaFields(4), Alphas(4)));
                    ErrorsFound = true;
                }
                if (thisElecSys.SurfacePtr(1) != 0) {
                    state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(state.dataLowTempRadSys->ElecRadSys(Item).SurfacePtr(1)) = true;
                }
            }

            // Error checking for zones and construction information
            thisElecSys.errorCheckZonesAndConstructions(state, ErrorsFound);

            // Heating user input data
            // Determine Low Temp Radiant heating design capacity sizing method
            if (Util::SameString(Alphas(iHeatCAPMAlphaNum), "HeatingDesignCapacity")) {
                thisElecSys.HeatingCapMethod = HeatingDesignCapacity;
                if (!lNumericBlanks(iHeatDesignCapacityNumericNum)) {
                    thisElecSys.ScaledHeatingCapacity = Numbers(iHeatDesignCapacityNumericNum);
                    thisElecSys.MaxElecPower = thisElecSys.ScaledHeatingCapacity;
                    if (thisElecSys.ScaledHeatingCapacity < 0.0 && thisElecSys.ScaledHeatingCapacity != AutoSize) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                        ShowContinueError(
                            state,
                            format("Illegal {} = {:.7T}", cNumericFields(iHeatDesignCapacityNumericNum), Numbers(iHeatDesignCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(iHeatCAPMAlphaNum), Alphas(iHeatCAPMAlphaNum)));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(iHeatDesignCapacityNumericNum)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(Alphas(iHeatCAPMAlphaNum), "CapacityPerFloorArea")) {
                thisElecSys.HeatingCapMethod = CapacityPerFloorArea;
                if (!lNumericBlanks(iHeatCapacityPerFloorAreaNumericNum)) {
                    thisElecSys.ScaledHeatingCapacity = Numbers(iHeatCapacityPerFloorAreaNumericNum);
                    thisElecSys.MaxElecPower = thisElecSys.ScaledHeatingCapacity;
                    if (thisElecSys.ScaledHeatingCapacity <= 0.0) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                        ShowContinueError(state, format("Input for {} = {}", cAlphaFields(iHeatCAPMAlphaNum), Alphas(iHeatCAPMAlphaNum)));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 cNumericFields(iHeatCapacityPerFloorAreaNumericNum),
                                                 Numbers(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    } else if (thisElecSys.ScaledHeatingCapacity == AutoSize) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                        ShowContinueError(state, format("Input for {} = {}", cAlphaFields(iHeatCAPMAlphaNum), Alphas(iHeatCAPMAlphaNum)));
                        ShowContinueError(state, format("Illegal {} = Autosize", cNumericFields(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(iHeatCAPMAlphaNum), Alphas(iHeatCAPMAlphaNum)));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(iHeatCapacityPerFloorAreaNumericNum)));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(Alphas(iHeatCAPMAlphaNum), "FractionOfAutosizedHeatingCapacity")) {
                thisElecSys.HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
                if (!lNumericBlanks(iHeatFracOfAutosizedCapacityNumericNum)) {
                    thisElecSys.ScaledHeatingCapacity = Numbers(iHeatFracOfAutosizedCapacityNumericNum);
                    thisElecSys.MaxElecPower = thisElecSys.ScaledHeatingCapacity;
                    if (thisElecSys.ScaledHeatingCapacity < 0.0) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                        ShowContinueError(state,
                                          format("Illegal {} = {:.7T}",
                                                 cNumericFields(iHeatFracOfAutosizedCapacityNumericNum),
                                                 Numbers(iHeatFracOfAutosizedCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                    ShowContinueError(state, format("Input for {} = {}", cAlphaFields(iHeatCAPMAlphaNum), Alphas(iHeatCAPMAlphaNum)));
                    ShowContinueError(state, format("Blank field not allowed for {}", cNumericFields(iHeatFracOfAutosizedCapacityNumericNum)));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, thisElecSys.Name));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(iHeatCAPMAlphaNum), Alphas(iHeatCAPMAlphaNum)));
                ErrorsFound = true;
            }

            // Process the temperature control type
            if (lAlphaBlanks(6)) {
                thisElecSys.controlType = CtrlType::MAT;
            } else if ((thisElecSys.controlType = static_cast<CtrlType>(getEnumValue(ctrlTypeNamesUC, Alphas(6)))) == CtrlType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(6), Alphas(6), "MeanAirTemperature");
                thisElecSys.controlType = CtrlType::MAT;
            }

            // Process the setpoint type
            if (lAlphaBlanks(7)) {
                thisElecSys.setpointType = SetpointType::HalfFlowPower;
            } else if ((thisElecSys.setpointType = static_cast<SetpointType>(getEnumValue(setpointTypeNamesUC, Alphas(7)))) ==
                       SetpointType::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(7), Alphas(7), "HalfFlowPower");
                thisElecSys.setpointType = SetpointType::HalfFlowPower;
            }

            thisElecSys.ThrottlRange = Numbers(4);

            if (lAlphaBlanks(8)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(8));
                ErrorsFound = true;
            } else if ((thisElecSys.setptSched = Sched::GetSchedule(state, Alphas(8))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(8), Alphas(8));
                ErrorsFound = true;
            }
        }

        // Check to see if any surface is included in more than one radiant system.  This is not allowed
        // and thus indicative that there is an error in the input file.  This is to make sure that two
        // different radiant systems are competing for the same surface.  Allowing this to happen would
        // result in lost energy somewhere and the situation really is not physically possible anyway.
        AssignedAsRadiantSurface.dimension(state.dataSurface->TotSurfaces, false);

        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfHydrLowTempRadSys; ++Item) {
            for (SurfNum = 1; SurfNum <= state.dataLowTempRadSys->HydrRadSys(Item).NumOfSurfaces; ++SurfNum) {
                CheckSurfNum = state.dataLowTempRadSys->HydrRadSys(Item).SurfacePtr(SurfNum);
                if (CheckSurfNum == 0) {
                    continue;
                }
                if (AssignedAsRadiantSurface(CheckSurfNum)) {
                    ShowSevereError(
                        state, format("Surface {} is referenced by more than one radiant system--this is not allowed", Surface(CheckSurfNum).Name));
                    ErrorsFound = true;
                } else {
                    AssignedAsRadiantSurface(CheckSurfNum) = true;
                }
                // Also check the other side of interzone partitions
                if ((Surface(CheckSurfNum).ExtBoundCond > 0) && (Surface(CheckSurfNum).ExtBoundCond != CheckSurfNum)) {
                    if (AssignedAsRadiantSurface(Surface(CheckSurfNum).ExtBoundCond)) {
                        ShowSevereError(state,
                                        format("Interzone surface {} is referenced by more than one radiant system--this is not allowed",
                                               Surface(Surface(CheckSurfNum).ExtBoundCond).Name));
                        ErrorsFound = true;
                    } else {
                        AssignedAsRadiantSurface(Surface(CheckSurfNum).ExtBoundCond) = true;
                    }
                }
            }
        }

        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfCFloLowTempRadSys; ++Item) {
            for (SurfNum = 1; SurfNum <= state.dataLowTempRadSys->CFloRadSys(Item).NumOfSurfaces; ++SurfNum) {
                CheckSurfNum = state.dataLowTempRadSys->CFloRadSys(Item).SurfacePtr(SurfNum);
                if (CheckSurfNum == 0) {
                    continue;
                }
                if (AssignedAsRadiantSurface(CheckSurfNum)) {
                    ShowSevereError(
                        state, format("Surface {} is referenced by more than one radiant system--this is not allowed", Surface(CheckSurfNum).Name));
                    ErrorsFound = true;
                } else {
                    AssignedAsRadiantSurface(CheckSurfNum) = true;
                }
                // Also check the other side of interzone partitions
                if ((Surface(CheckSurfNum).ExtBoundCond > 0) && (Surface(CheckSurfNum).ExtBoundCond != CheckSurfNum)) {
                    if (AssignedAsRadiantSurface(Surface(CheckSurfNum).ExtBoundCond)) {
                        ShowSevereError(state,
                                        format("Interzone surface {} is referenced by more than one radiant system--this is not allowed",
                                               Surface(Surface(CheckSurfNum).ExtBoundCond).Name));
                        ErrorsFound = true;
                    } else {
                        AssignedAsRadiantSurface(Surface(CheckSurfNum).ExtBoundCond) = true;
                    }
                }
            }
        }

        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfElecLowTempRadSys; ++Item) {
            for (SurfNum = 1; SurfNum <= state.dataLowTempRadSys->ElecRadSys(Item).NumOfSurfaces; ++SurfNum) {
                CheckSurfNum = state.dataLowTempRadSys->ElecRadSys(Item).SurfacePtr(SurfNum);
                if (CheckSurfNum == 0) {
                    continue;
                }
                if (AssignedAsRadiantSurface(CheckSurfNum)) {
                    ShowSevereError(
                        state, format("Surface {} is referenced by more than one radiant system--this is not allowed", Surface(CheckSurfNum).Name));
                    ErrorsFound = true;
                } else {
                    AssignedAsRadiantSurface(CheckSurfNum) = true;
                }
                // Also check the other side of interzone partitions
                if ((Surface(CheckSurfNum).ExtBoundCond > 0) && (Surface(CheckSurfNum).ExtBoundCond != CheckSurfNum)) {
                    if (AssignedAsRadiantSurface(Surface(CheckSurfNum).ExtBoundCond)) {
                        ShowSevereError(state,
                                        format("Interzone surface {} is referenced by more than one radiant system--this is not allowed",
                                               Surface(Surface(CheckSurfNum).ExtBoundCond).Name));
                        ErrorsFound = true;
                    } else {
                        AssignedAsRadiantSurface(Surface(CheckSurfNum).ExtBoundCond) = true;
                    }
                }
            }
        }

        AssignedAsRadiantSurface.deallocate();
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input. Preceding conditions cause termination.", RoutineName));
        }

        // Set up the output variables for low temperature radiant systems
        // ZoneHVAC:LowTemperatureRadiant:VariableFlow (HydrRadSys)
        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfHydrLowTempRadSys; ++Item) {

            auto &thisHydrSys = state.dataLowTempRadSys->HydrRadSys(Item);

            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Rate",
                                Constant::Units::W,
                                thisHydrSys.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHydrSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Energy",
                                Constant::Units::J,
                                thisHydrSys.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHydrSys.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Fluid Energy",
                                Constant::Units::J,
                                thisHydrSys.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHydrSys.Name,
                                Constant::eResource::PlantLoopHeatingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Cooling Rate",
                                Constant::Units::W,
                                thisHydrSys.CoolPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHydrSys.Name);

            SetupOutputVariable(state,
                                "Zone Radiant HVAC Cooling Energy",
                                Constant::Units::J,
                                thisHydrSys.CoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHydrSys.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Cooling Fluid Energy",
                                Constant::Units::J,
                                thisHydrSys.CoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHydrSys.Name,
                                Constant::eResource::PlantLoopCoolingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisHydrSys.WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHydrSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Inlet Temperature",
                                Constant::Units::C,
                                thisHydrSys.WaterInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHydrSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Outlet Temperature",
                                Constant::Units::C,
                                thisHydrSys.WaterOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHydrSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Moisture Condensation Time",
                                Constant::Units::s,
                                thisHydrSys.CondCausedTimeOff,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisHydrSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Operation Mode",
                                Constant::Units::None,
                                thisHydrSys.opMode,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisHydrSys.Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSInternalVariable(state,
                                         "Hydronic Low Temp Radiant Design Water Volume Flow Rate for Heating",
                                         thisHydrSys.Name,
                                         "[m3/s]",
                                         thisHydrSys.WaterVolFlowMaxHeat);
                SetupEMSInternalVariable(state,
                                         "Hydronic Low Temp Radiant Design Water Volume Flow Rate for Cooling",
                                         thisHydrSys.Name,
                                         "[m3/s]",
                                         thisHydrSys.WaterVolFlowMaxCool);
                SetupEMSActuator(state,
                                 "Hydronic Low Temp Radiant",
                                 thisHydrSys.Name,
                                 "Water Mass Flow Rate",
                                 "[kg/s]",
                                 thisHydrSys.EMSOverrideOnWaterMdot,
                                 thisHydrSys.EMSWaterMdotOverrideValue);
            }
        }

        // Set up the output variables for low temperature radiant systems
        // ZoneHVAC:LowTemperatureRadiant:ConstantFlow (CFloRadSys)
        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfCFloLowTempRadSys; ++Item) {

            auto &thisCFloSys = state.dataLowTempRadSys->CFloRadSys(Item);

            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Rate",
                                Constant::Units::W,
                                thisCFloSys.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Energy",
                                Constant::Units::J,
                                thisCFloSys.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Fluid Heat Transfer Energy",
                                Constant::Units::J,
                                thisCFloSys.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name,
                                Constant::eResource::PlantLoopHeatingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Cooling Rate",
                                Constant::Units::W,
                                thisCFloSys.CoolPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Cooling Energy",
                                Constant::Units::J,
                                thisCFloSys.CoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Cooling Fluid Heat Transfer Energy",
                                Constant::Units::J,
                                thisCFloSys.CoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name,
                                Constant::eResource::PlantLoopCoolingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCFloSys.WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Injection Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCFloSys.WaterInjectionRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Recirculation Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCFloSys.WaterRecircRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Inlet Temperature",
                                Constant::Units::C,
                                thisCFloSys.WaterInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Outlet Temperature",
                                Constant::Units::C,
                                thisCFloSys.WaterOutletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Pump Inlet Temperature",
                                Constant::Units::C,
                                thisCFloSys.PumpInletTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Pump Electricity Rate",
                                Constant::Units::W,
                                thisCFloSys.PumpPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Pump Electricity Energy",
                                Constant::Units::J,
                                thisCFloSys.PumpEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::Pumps);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Pump Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCFloSys.PumpMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Pump Fluid Heat Gain Rate",
                                Constant::Units::W,
                                thisCFloSys.PumpHeattoFluid,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Pump Fluid Heat Gain Energy",
                                Constant::Units::J,
                                thisCFloSys.PumpHeattoFluidEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Moisture Condensation Time",
                                Constant::Units::s,
                                thisCFloSys.CondCausedTimeOff,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCFloSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Operation Mode",
                                Constant::Units::None,
                                thisCFloSys.opMode,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCFloSys.Name);
            if (state.dataLowTempRadSys->anyRadiantSystemUsingRunningMeanAverage) {
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC Running Mean Outdoor Dry-Bulb Temperature",
                                    Constant::Units::C,
                                    thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisCFloSys.Name);
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC Previous Day Running Mean Outdoor Dry-Bulb Temperature",
                                    Constant::Units::C,
                                    thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisCFloSys.Name);
                SetupOutputVariable(state,
                                    "Zone Radiant HVAC Previous Day Average Outdoor Dry-Bulb Temperature",
                                    Constant::Units::C,
                                    thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisCFloSys.Name);
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSInternalVariable(
                    state, "Constant Flow Low Temp Radiant Design Water Mass Flow Rate", thisCFloSys.Name, "[m3/s]", thisCFloSys.WaterVolFlowMax);
                SetupEMSActuator(state,
                                 "Constant Flow Low Temp Radiant",
                                 thisCFloSys.Name,
                                 "Water Mass Flow Rate",
                                 "[kg/s]",
                                 thisCFloSys.EMSOverrideOnWaterMdot,
                                 thisCFloSys.EMSWaterMdotOverrideValue);
            }
        }

        for (Item = 1; Item <= state.dataLowTempRadSys->NumOfElecLowTempRadSys; ++Item) {
            // Set up the output variables for low temperature radiant systems
            // ZoneHVAC:LowTemperatureRadiant:Electric (ElecRadSys)

            auto &thisElecSys = state.dataLowTempRadSys->ElecRadSys(Item);

            SetupOutputVariable(state,
                                "Zone Radiant HVAC Electricity Rate",
                                Constant::Units::W,
                                thisElecSys.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisElecSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Electricity Energy",
                                Constant::Units::J,
                                thisElecSys.ElecEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisElecSys.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Rate",
                                Constant::Units::W,
                                thisElecSys.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisElecSys.Name);
            SetupOutputVariable(state,
                                "Zone Radiant HVAC Heating Energy",
                                Constant::Units::J,
                                thisElecSys.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisElecSys.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatingCoils);
        }
    }

    void RadiantSystemBaseData::errorCheckZonesAndConstructions(EnergyPlusData &state, bool &errorsFound)
    {
        auto &Zone = state.dataHeatBal->Zone;
        auto &Surface = state.dataSurface->Surface;

        Real64 zoneMultipliers = 0.0;
        Real64 zoneMultipliersSurface = 0.0;
        Real64 zoneMultiplersTolerance = 0.001;
        for (int SurfNum = 1; SurfNum <= this->NumOfSurfaces; ++SurfNum) {

            if (this->SurfacePtr(SurfNum) == 0) {
                continue; // invalid surface -- detected earlier
            }

            if (state.dataGlobal->DisplayExtraWarnings) {
                // check zone numbers--ok if they are not the same
                // group warning issued earlier, show detailed warning here
                if (Surface(this->SurfacePtr(SurfNum)).Zone != this->ZonePtr) {
                    ShowWarningError(state,
                                     "A surface referenced in a Low Temperature Radiant System is not in same zone as the radiant system itself");
                    ShowContinueError(state, format("Surface = {}", Surface(this->SurfacePtr(SurfNum)).Name));
                    ShowContinueError(state,
                                      format("Surface in Zone = {}. Radiant System in Zone = {}",
                                             Zone(Surface(this->SurfacePtr(SurfNum)).Zone).Name,
                                             this->ZoneName));
                    ShowContinueError(state, format("Occurs in Low Temperature Radiant System = {}", this->Name));
                    ShowContinueError(state, "If this is intentionally a radiant system with surfaces in more than one thermal zone,");
                    ShowContinueError(state, "then ignore this warning message.  Otherwise, check the surfaces in this radiant system.");
                }
            }

            // check zone multipliers--these must be the same
            if (SurfNum == 1) {
                zoneMultipliers = double(Zone(this->ZonePtr).Multiplier) * double(Zone(this->ZonePtr).ListMultiplier);
            }
            zoneMultipliersSurface = double(Zone(Surface(this->SurfacePtr(SurfNum)).Zone).Multiplier) *
                                     double(Zone(Surface(this->SurfacePtr(SurfNum)).Zone).ListMultiplier);
            if (std::abs(zoneMultipliers - zoneMultipliersSurface) > zoneMultiplersTolerance) {
                ShowSevereError(state, "The zone multipliers are not the same for all surfaces contained in this radiant system");
                ShowContinueError(state, "This is not allowed and must be fixed for the simulation to run.");
                ShowContinueError(state, format("Occurs in Low Temperature Radiant System = {}", this->Name));
                errorsFound = true;
            }

            // make sure that this construction is defined with a source/sink--this must be the case or it can't serve as a radiant system surface
            if (!state.dataConstruction->Construct(Surface(this->SurfacePtr(SurfNum)).Construction).SourceSinkPresent) {
                ShowSevereError(state, "Construction referenced in Radiant System Surface does not have a source/sink present");
                ShowContinueError(state,
                                  format("Surface name= {}  Construction name = {}",
                                         Surface(this->SurfacePtr(SurfNum)).Name,
                                         state.dataConstruction->Construct(Surface(this->SurfacePtr(SurfNum)).Construction).Name));
                ShowContinueError(state, "Construction needs to be referenced by a \"ConstructionProperty:InternalHeatSource\" object.");
                errorsFound = true;
            }
        }
    }

    void InitLowTempRadiantSystem(EnergyPlusData &state,
                                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                  int const RadSysNum, // Index for the low temperature radiant system under consideration within the derived types
                                  SystemType const systemType, // Type of radiant system: hydronic, constant flow, or electric
                                  bool &InitErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000

        // Using/Aliasing

        using DataSizing::AutoSize;
        using DataZoneEquipment::CheckZoneEquipmentList;

        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::ScanPlantLoopsForObject;
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ZeroTol(0.0000001); // Smallest non-zero value allowed
        constexpr std::string_view RoutineName("InitLowTempRadiantSystem");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CurrentFlowSchedule; // Schedule value for flow fraction in a constant flow radiant system
        Real64 TotalEffic;          // Intermediate calculation variable for total pump efficiency
        Real64 mdot;                // local fluid mass flow rate
        Real64 rho;                 // local fluid density
        bool errFlag;

        InitErrorsFound = false;

        auto const &Surface = state.dataSurface->Surface;

        if (state.dataLowTempRadSys->MyOneTimeFlag) {
            state.dataLowTempRadSys->MyEnvrnFlagHydr.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSys);
            state.dataLowTempRadSys->MyEnvrnFlagCFlo.allocate(state.dataLowTempRadSys->NumOfCFloLowTempRadSys);
            state.dataLowTempRadSys->MyEnvrnFlagElec.allocate(state.dataLowTempRadSys->NumOfElecLowTempRadSys);
            state.dataLowTempRadSys->MyPlantScanFlagHydr.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSys);
            state.dataLowTempRadSys->MyPlantScanFlagCFlo.allocate(state.dataLowTempRadSys->NumOfCFloLowTempRadSys);
            state.dataLowTempRadSys->MyPlantScanFlagHydr = true;
            state.dataLowTempRadSys->MyPlantScanFlagCFlo = true;
            state.dataLowTempRadSys->MyEnvrnFlagHydr = true;
            state.dataLowTempRadSys->MyEnvrnFlagCFlo = true;
            state.dataLowTempRadSys->MyEnvrnFlagElec = true;
            state.dataLowTempRadSys->MyOneTimeFlag = false;
        }

        if (state.dataLowTempRadSys->FirstTimeInit) {

            for (auto &thisLTRSys : state.dataLowTempRadSys->HydrRadSys) {
                thisLTRSys.QRadSysSrcAvg.dimension(thisLTRSys.NumOfSurfaces, 0.0);
                thisLTRSys.LastQRadSysSrc.dimension(thisLTRSys.NumOfSurfaces, 0.0);
                thisLTRSys.LastSysTimeElapsed = 0.0;
                thisLTRSys.LastTimeStepSys = 0.0;
            }
            for (auto &thisCFLTRSys : state.dataLowTempRadSys->CFloRadSys) {
                thisCFLTRSys.QRadSysSrcAvg.dimension(thisCFLTRSys.NumOfSurfaces, 0.0);
                thisCFLTRSys.LastQRadSysSrc.dimension(thisCFLTRSys.NumOfSurfaces, 0.0);
                thisCFLTRSys.LastSysTimeElapsed = 0.0;
                thisCFLTRSys.LastTimeStepSys = 0.0;
            }
            for (auto &thisELTRSys : state.dataLowTempRadSys->ElecRadSys) {
                thisELTRSys.QRadSysSrcAvg.dimension(thisELTRSys.NumOfSurfaces, 0.0);
                thisELTRSys.LastQRadSysSrc.dimension(thisELTRSys.NumOfSurfaces, 0.0);
                thisELTRSys.LastSysTimeElapsed = 0.0;
                thisELTRSys.LastTimeStepSys = 0.0;
            }
            state.dataLowTempRadSys->MySizeFlagHydr.allocate(state.dataLowTempRadSys->NumOfHydrLowTempRadSys);
            state.dataLowTempRadSys->MySizeFlagCFlo.allocate(state.dataLowTempRadSys->NumOfCFloLowTempRadSys);
            state.dataLowTempRadSys->MySizeFlagElec.allocate(state.dataLowTempRadSys->NumOfElecLowTempRadSys);
            state.dataLowTempRadSys->MySizeFlagHydr = true;
            state.dataLowTempRadSys->MySizeFlagCFlo = true;
            state.dataLowTempRadSys->MySizeFlagElec = true;

            // Initialize total areas and ZeroLTRSource for all radiant systems
            for (auto &thisHRadSys : state.dataLowTempRadSys->HydrRadSys) {
                thisHRadSys.ZeroLTRSourceSumHATsurf = 0.0;
                thisHRadSys.TotalSurfaceArea = 0.0;
                for (int SurfNum = 1; SurfNum <= thisHRadSys.NumOfSurfaces; ++SurfNum) {
                    thisHRadSys.TotalSurfaceArea += Surface(thisHRadSys.SurfacePtr(SurfNum)).Area;
                }
            }
            for (auto &thisCFLRadSys : state.dataLowTempRadSys->CFloRadSys) {
                thisCFLRadSys.ZeroLTRSourceSumHATsurf = 0.0;
                thisCFLRadSys.TotalSurfaceArea = 0.0;
                for (int SurfNum = 1; SurfNum <= thisCFLRadSys.NumOfSurfaces; ++SurfNum) {
                    thisCFLRadSys.TotalSurfaceArea += Surface(thisCFLRadSys.SurfacePtr(SurfNum)).Area;
                }
            }
            for (auto &thisERadSys : state.dataLowTempRadSys->ElecRadSys) {
                thisERadSys.ZeroLTRSourceSumHATsurf = 0.0;
                thisERadSys.TotalSurfaceArea = 0.0;
                for (int SurfNum = 1; SurfNum <= thisERadSys.NumOfSurfaces; ++SurfNum) {
                    thisERadSys.TotalSurfaceArea += Surface(thisERadSys.SurfacePtr(SurfNum)).Area;
                }
            }

            Real64 MotorEffic(0.0);
            if (systemType == SystemType::ConstantFlow) {
                // Is there a reason why we were making a copy of this object?
                MotorEffic =
                    state.dataLowTempRadSys->CflowRadiantSysDesign(state.dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr).MotorEffic;
            }

            // Check pump parameters for constant flow hydronic radiant systems
            for (auto &thisCFLRadSys : state.dataLowTempRadSys->CFloRadSys) {
                // Calculate the efficiency for each pump: The calculation
                // is based on the PMPSIM code in the ASHRAE Secondary Toolkit
                if ((thisCFLRadSys.NomPowerUse > ZeroTol) && (MotorEffic > ZeroTol) && (thisCFLRadSys.WaterVolFlowMax != AutoSize)) {
                    TotalEffic = thisCFLRadSys.WaterVolFlowMax * thisCFLRadSys.NomPumpHead / thisCFLRadSys.NomPowerUse;
                    thisCFLRadSys.PumpEffic = TotalEffic / MotorEffic;
                    constexpr std::string_view fmt = "Check input.  Calc Pump Efficiency={:.5R}% {}, for pump in radiant system {}";
                    Real64 pumpEfficiency = thisCFLRadSys.PumpEffic * 100.0;
                    if (thisCFLRadSys.PumpEffic < 0.50) {
                        ShowWarningError(state, format(fmt, pumpEfficiency, "which is less than 50%", thisCFLRadSys.Name));
                    } else if ((thisCFLRadSys.PumpEffic > 0.95) && (thisCFLRadSys.PumpEffic <= 1.0)) {
                        ShowWarningError(state, format(fmt, pumpEfficiency, "is approaching 100%", thisCFLRadSys.Name));
                    } else if (thisCFLRadSys.PumpEffic > 1.0) {
                        ShowSevereError(state, format(fmt, pumpEfficiency, "which is bigger than 100%", thisCFLRadSys.Name));
                        InitErrorsFound = true;
                    }
                } else {
                    // Autosize is not an error but it does not need to check pump efficiency here
                    if (thisCFLRadSys.WaterVolFlowMax != AutoSize) {
                        ShowSevereError(state,
                                        format("Check input.  Pump nominal power and motor efficiency cannot be 0, for pump={}", thisCFLRadSys.Name));
                        InitErrorsFound = true;
                    }
                }
            }

            state.dataLowTempRadSys->FirstTimeInit = false;
        }

        if (systemType == SystemType::Hydronic) {
            if (state.dataLowTempRadSys->MyPlantScanFlagHydr(RadSysNum) && allocated(state.dataPlnt->PlantLoop)) {
                errFlag = false;
                if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode > 0) {
                    ScanPlantLoopsForObject(state,
                                            state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                            DataPlant::PlantEquipmentType::LowTempRadiant_VarFlow,
                                            state.dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitLowTempRadiantSystem: Program terminated due to previous condition(s).");
                    }
                }
                if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode > 0) {
                    ScanPlantLoopsForObject(state,
                                            state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                            DataPlant::PlantEquipmentType::LowTempRadiant_VarFlow,
                                            state.dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitLowTempRadiantSystem: Program terminated due to previous condition(s).");
                    }
                }
                state.dataLowTempRadSys->MyPlantScanFlagHydr(RadSysNum) = false;
            } else if (state.dataLowTempRadSys->MyPlantScanFlagHydr(RadSysNum) && !state.dataGlobal->AnyPlantInModel) {
                state.dataLowTempRadSys->MyPlantScanFlagHydr(RadSysNum) = false;
            }

        } else if (systemType == SystemType::ConstantFlow) {
            if (state.dataLowTempRadSys->MyPlantScanFlagCFlo(RadSysNum) && allocated(state.dataPlnt->PlantLoop)) {
                errFlag = false;
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode > 0) {
                    ScanPlantLoopsForObject(state,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                            DataPlant::PlantEquipmentType::LowTempRadiant_ConstFlow,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitLowTempRadiantSystem: Program terminated due to previous condition(s).");
                    }
                }
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode > 0) {
                    ScanPlantLoopsForObject(state,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                            DataPlant::PlantEquipmentType::LowTempRadiant_ConstFlow,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitLowTempRadiantSystem: Program terminated due to previous condition(s).");
                    }
                }
                state.dataLowTempRadSys->MyPlantScanFlagCFlo(RadSysNum) = false;
            } else if (state.dataLowTempRadSys->MyPlantScanFlagCFlo(RadSysNum) && !state.dataGlobal->AnyPlantInModel) {
                state.dataLowTempRadSys->MyPlantScanFlagCFlo(RadSysNum) = false;
            }
        }

        // need to check all units to see if they are on Zone Equipment List or issue warning
        if (!state.dataLowTempRadSys->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataLowTempRadSys->ZoneEquipmentListChecked = true;
            for (auto &thisRadSys : state.dataLowTempRadSys->RadSysTypes) {
                switch (thisRadSys.systemType) {
                case SystemType::Hydronic: {
                    if (CheckZoneEquipmentList(state, "ZoneHVAC:LowTemperatureRadiant:VariableFlow", thisRadSys.Name)) {
                        continue;
                    }
                    ShowSevereError(state,
                                    format("InitLowTempRadiantSystem: Unit=[ZoneHVAC:LowTemperatureRadiant:VariableFlow,{}] is not on any "
                                           "ZoneHVAC:EquipmentList.  It will not be simulated.",
                                           thisRadSys.Name));
                } break;
                case SystemType::ConstantFlow: {
                    if (CheckZoneEquipmentList(state, "ZoneHVAC:LowTemperatureRadiant:ConstantFlow", thisRadSys.Name)) {
                        continue;
                    }
                    ShowSevereError(state,
                                    format("InitLowTempRadiantSystem: Unit=[ZoneHVAC:LowTemperatureRadiant:ConstantFlow,{}] is not on any "
                                           "ZoneHVAC:EquipmentList.  It will not be simulated.",
                                           thisRadSys.Name));
                } break;
                case SystemType::Electric: {
                    if (CheckZoneEquipmentList(state, "ZoneHVAC:LowTemperatureRadiant:Electric", thisRadSys.Name)) {
                        continue;
                    }
                    ShowSevereError(state,
                                    format("InitLowTempRadiantSystem: Unit=[ZoneHVAC:LowTemperatureRadiant:Electric,{}] is not on any "
                                           "ZoneHVAC:EquipmentList.  It will not be simulated.",
                                           thisRadSys.Name));
                } break;
                default: { // Illegal system, but checked earlier
                } break;
                }
            }
        }

        if (!state.dataGlobal->SysSizingCalc && (systemType == SystemType::Hydronic)) {
            if (state.dataLowTempRadSys->MySizeFlagHydr(RadSysNum) && !state.dataLowTempRadSys->MyPlantScanFlagHydr(RadSysNum)) {
                // for each radiant system do the sizing once.
                SizeLowTempRadiantSystem(state, RadSysNum, systemType);
                state.dataLowTempRadSys->MySizeFlagHydr(RadSysNum) = false;

                Sched::Schedule *coldSetptSched = nullptr, *hotSetptSched = nullptr;
                if (systemType == SystemType::Hydronic) {
                    VarFlowRadDesignData variableFlowDesignDataObject{state.dataLowTempRadSys->HydronicRadiantSysDesign(
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).DesignObjectPtr)}; // Contains the data for variable flow hydronic systems;
                    coldSetptSched = variableFlowDesignDataObject.coolSetptSched;
                    hotSetptSched = variableFlowDesignDataObject.heatSetptSched;
                }

                // Can this system actually do cooling?
                if ((state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool > 0.0) &&
                    (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode > 0) &&
                    (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode > 0) && (coldSetptSched != nullptr)) {
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingSystem = true;
                }

                // Can this system actually do heating?
                if ((state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat > 0.0) &&
                    (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode > 0) &&
                    (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode > 0) && (hotSetptSched != nullptr)) {
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingSystem = true;
                }

                // set design mass flow rates
                if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode > 0) {
                    rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum)
                              .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterFlowMaxHeat =
                        rho * state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat;
                    InitComponentNodes(state,
                                       0.0,
                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterFlowMaxHeat,
                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode);
                }
                if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode > 0) {
                    rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum)
                              .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterFlowMaxCool =
                        rho * state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool;
                    InitComponentNodes(state,
                                       0.0,
                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterFlowMaxCool,
                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode);
                }
            }
        }

        if (!state.dataGlobal->SysSizingCalc && (systemType == SystemType::ConstantFlow)) {
            if (state.dataLowTempRadSys->MySizeFlagCFlo(RadSysNum) && !state.dataLowTempRadSys->MyPlantScanFlagCFlo(RadSysNum)) {
                // for each radiant system do the sizing once.
                SizeLowTempRadiantSystem(state, RadSysNum, systemType);

                // set design mass flow rates
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode > 0) {
                    rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum)
                              .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate =
                        rho * state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax;
                    InitComponentNodes(state,
                                       0.0,
                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate,
                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode,
                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode);
                }
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode > 0) {
                    rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum)
                              .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate =
                        rho * state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax;
                    InitComponentNodes(state,
                                       0.0,
                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate,
                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode,
                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode);
                }
                state.dataLowTempRadSys->MySizeFlagCFlo(RadSysNum) = false;
            }
        }

        if (!state.dataGlobal->SysSizingCalc && (systemType == SystemType::Electric)) {
            if (state.dataLowTempRadSys->MySizeFlagElec(RadSysNum)) {
                // for each radiant system do the sizing once.
                SizeLowTempRadiantSystem(state, RadSysNum, systemType);
                state.dataLowTempRadSys->MySizeFlagElec(RadSysNum) = false;
            }
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataLowTempRadSys->MyEnvrnFlagGeneral) {
            if (systemType == SystemType::Hydronic) {
                auto &thisLTR = state.dataLowTempRadSys->HydrRadSys(RadSysNum);
                thisLTR.ZeroLTRSourceSumHATsurf = 0.0;
                thisLTR.QRadSysSrcAvg = 0.0;
                thisLTR.LastQRadSysSrc = 0.0;
                thisLTR.LastSysTimeElapsed = 0.0;
                thisLTR.LastTimeStepSys = 0.0;
            } else if (systemType == SystemType::ConstantFlow) {
                auto &thisLTR = state.dataLowTempRadSys->CFloRadSys(RadSysNum);
                thisLTR.ZeroLTRSourceSumHATsurf = 0.0;
                thisLTR.QRadSysSrcAvg = 0.0;
                thisLTR.LastQRadSysSrc = 0.0;
                thisLTR.LastSysTimeElapsed = 0.0;
                thisLTR.LastTimeStepSys = 0.0;
            } else if (systemType == SystemType::Electric) {
                auto &thisLTR = state.dataLowTempRadSys->ElecRadSys(RadSysNum);
                thisLTR.ZeroLTRSourceSumHATsurf = 0.0;
                thisLTR.QRadSysSrcAvg = 0.0;
                thisLTR.LastQRadSysSrc = 0.0;
                thisLTR.LastSysTimeElapsed = 0.0;
                thisLTR.LastTimeStepSys = 0.0;
            }
            state.dataLowTempRadSys->MyEnvrnFlagGeneral = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataLowTempRadSys->MyEnvrnFlagGeneral = true;
        }

        // If we are at the beginning of a new environment OR the warmup period is done and the simulation is starting,
        // then the various changeover variables need to be reset so that we are starting from scratch.
        if ((state.dataGlobal->BeginEnvrnFlag && FirstHVACIteration) ||
            (!state.dataGlobal->WarmupFlag && state.dataGlobal->BeginDayFlag && FirstHVACIteration && state.dataGlobal->DayOfSim == 1)) {
            // Reset values related to changeover
            if (systemType == SystemType::Hydronic) {
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).lastOpMode = OpMode::None;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).lastDayOfSim = 0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).lastHourOfDay = 0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).lastTimeStep = 0;
            }
            if (systemType == SystemType::ConstantFlow) {
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).lastOpMode = OpMode::None;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).lastDayOfSim = 0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).lastHourOfDay = 0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).lastTimeStep = 0;
            }
        }

        if (systemType == SystemType::Hydronic) {
            if (state.dataGlobal->BeginEnvrnFlag && state.dataLowTempRadSys->MyEnvrnFlagHydr(RadSysNum)) {
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).HeatPower = 0.0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).HeatEnergy = 0.0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).CoolPower = 0.0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).CoolEnergy = 0.0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterInletTemp = 0.0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterOutletTemp = 0.0;
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterMassFlowRate = 0.0;

                if (!state.dataLowTempRadSys->MyPlantScanFlagHydr(RadSysNum)) {
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode > 0) {
                        InitComponentNodes(state,
                                           0.0,
                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterFlowMaxHeat,
                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode);
                    }
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode > 0) {
                        InitComponentNodes(state,
                                           0.0,
                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterFlowMaxCool,
                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode);
                    }
                }
                state.dataLowTempRadSys->MyEnvrnFlagHydr(RadSysNum) = false;
            }
        } // NumOfHydrLowTempRadSys > 0
        if (!state.dataGlobal->BeginEnvrnFlag && systemType == SystemType::Hydronic) {
            state.dataLowTempRadSys->MyEnvrnFlagHydr(RadSysNum) = true;
        }

        if (systemType == SystemType::ConstantFlow) {
            if (state.dataGlobal->BeginEnvrnFlag && state.dataLowTempRadSys->MyEnvrnFlagCFlo(RadSysNum)) {
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterInletTemp = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterOutletTemp = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).PumpInletTemp = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterInjectionRate = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterRecircRate = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).HeatPower = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).HeatEnergy = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).CoolPower = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).CoolEnergy = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).PumpPower = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).PumpMassFlowRate = 0.0;
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).PumpHeattoFluid = 0.0;

                if (!state.dataLowTempRadSys->MyPlantScanFlagCFlo(RadSysNum)) {
                    if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode > 0) {
                        InitComponentNodes(state,
                                           0.0,
                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate,
                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode,
                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode);
                    }
                    if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode > 0) {
                        InitComponentNodes(state,
                                           0.0,
                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate,
                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode,
                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode);
                    }
                }
                state.dataLowTempRadSys->MyEnvrnFlagCFlo(RadSysNum) = false;
            }

            if (state.dataLowTempRadSys->anyRadiantSystemUsingRunningMeanAverage) {
                if (state.dataGlobal->BeginDayFlag && state.dataLowTempRadSys->CFloRadSys(RadSysNum).setRunningMeanValuesAtBeginningOfDay) {
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).calculateRunningMeanAverageTemperature(state, RadSysNum);
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).setRunningMeanValuesAtBeginningOfDay = false; // only set these once per system
                } else if (!state.dataGlobal->BeginDayFlag && !state.dataLowTempRadSys->CFloRadSys(RadSysNum).setRunningMeanValuesAtBeginningOfDay) {
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).setRunningMeanValuesAtBeginningOfDay =
                        true; // reset so that the next time BeginDayFlag is true this can get set
                }
            }

        } // NumOfCFloLowTempRadSys > 0
        if (!state.dataGlobal->BeginEnvrnFlag && systemType == SystemType::ConstantFlow) {
            state.dataLowTempRadSys->MyEnvrnFlagCFlo(RadSysNum) = true;
        }

        if (systemType == SystemType::Electric) {
            if (state.dataGlobal->BeginEnvrnFlag && state.dataLowTempRadSys->MyEnvrnFlagElec(RadSysNum)) {
                state.dataLowTempRadSys->ElecRadSys(RadSysNum).HeatPower = 0.0;
                state.dataLowTempRadSys->ElecRadSys(RadSysNum).HeatEnergy = 0.0;
                state.dataLowTempRadSys->ElecRadSys(RadSysNum).ElecPower = 0.0;
                state.dataLowTempRadSys->ElecRadSys(RadSysNum).ElecEnergy = 0.0;
            }
            state.dataLowTempRadSys->MyEnvrnFlagElec(RadSysNum) = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag && systemType == SystemType::Electric) {
            state.dataLowTempRadSys->MyEnvrnFlagElec(RadSysNum) = true;
        }

        if (systemType == SystemType::ConstantFlow) {

            // Can this system actually do heating?
            if ((state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax > 0.0) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode > 0) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode > 0) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).hotWaterHiTempSched != nullptr) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).hotWaterLoTempSched != nullptr) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).hotCtrlHiTempSched != nullptr) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).hotCtrlLoTempSched != nullptr)) {
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = true;
            }

            // Can this system actually do cooling?
            if ((state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax > 0.0) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode > 0) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode > 0) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).coldWaterHiTempSched != nullptr) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).coldWaterLoTempSched != nullptr) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).coldCtrlHiTempSched != nullptr) &&
                (state.dataLowTempRadSys->CFloRadSys(RadSysNum).coldCtrlLoTempSched != nullptr)) {
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = true;
            }
        }

        if (state.dataGlobal->BeginTimeStepFlag && FirstHVACIteration) { // This is the first pass through in a particular time step

            switch (systemType) {
            case SystemType::Hydronic: {
                int ZoneNum = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr;
                auto &thisLTR = state.dataLowTempRadSys->HydrRadSys(RadSysNum);
                thisLTR.ZeroLTRSourceSumHATsurf =
                    state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state); // Set this to figure what part of the load the radiant system meets
                thisLTR.QRadSysSrcAvg = 0.0;                            // Initialize this variable to zero (radiant system defaults to off)
                thisLTR.LastQRadSysSrc = 0.0;     // At the start of a time step, reset to zero so average calculation can begin again
                thisLTR.LastSysTimeElapsed = 0.0; // At the start of a time step, reset to zero so average calculation can begin again
                thisLTR.LastTimeStepSys = 0.0; // At the start of a time step, reset to zero so average calculation can begin again                }
            } break;
            case SystemType::ConstantFlow: {
                int ZoneNum = state.dataLowTempRadSys->CFloRadSys(RadSysNum).ZonePtr;
                auto &thisLTR = state.dataLowTempRadSys->CFloRadSys(RadSysNum);
                thisLTR.ZeroLTRSourceSumHATsurf =
                    state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state); // Set this to figure what part of the load the radiant system meets
                thisLTR.QRadSysSrcAvg = 0.0;                            // Initialize this variable to zero (radiant system defaults to off)
                thisLTR.LastQRadSysSrc = 0.0;     // At the start of a time step, reset to zero so average calculation can begin again
                thisLTR.LastSysTimeElapsed = 0.0; // At the start of a time step, reset to zero so average calculation can begin again
                thisLTR.LastTimeStepSys = 0.0; // At the start of a time step, reset to zero so average calculation can begin again                }
            } break;
            case SystemType::Electric: {
                int ZoneNum = state.dataLowTempRadSys->ElecRadSys(RadSysNum).ZonePtr;
                auto &thisLTR = state.dataLowTempRadSys->ElecRadSys(RadSysNum);
                thisLTR.ZeroLTRSourceSumHATsurf =
                    state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state); // Set this to figure what part of the load the radiant system meets
                thisLTR.QRadSysSrcAvg = 0.0;                            // Initialize this variable to zero (radiant system defaults to off)
                thisLTR.LastQRadSysSrc = 0.0;     // At the start of a time step, reset to zero so average calculation can begin again
                thisLTR.LastSysTimeElapsed = 0.0; // At the start of a time step, reset to zero so average calculation can begin again
                thisLTR.LastTimeStepSys = 0.0; // At the start of a time step, reset to zero so average calculation can begin again                }
            } break;
            default: {
                ShowSevereError(state, "Radiant system entered without specification of type: electric, constant flow, or hydronic?");
                ShowContinueError(state, format("Occurs in Radiant System={}", state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                ShowFatalError(state, "Preceding condition causes termination.");
            } break;
            }

        } // ...for first pass through in a particular time step.

        switch (systemType) {

        case SystemType::Hydronic: {

            // Initialize the appropriate node data
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingSystem) {
                mdot = 0.0;
                SetComponentFlowRate(state,
                                     mdot,
                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode,
                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc);
            }
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingSystem) {
                mdot = 0.0;
                SetComponentFlowRate(state,
                                     mdot,
                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode,
                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc);
            }
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).opMode != OpMode::None && FirstHVACIteration) {
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).updateOperatingModeHistory(state);
            }

        } break;

        case SystemType::ConstantFlow: {
            state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 0.0;
            // Initialize the appropriate node data
            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem) {
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched != nullptr) {
                    CurrentFlowSchedule = state.dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched->getCurrentVal();
                } else {
                    CurrentFlowSchedule = 1.0; // Allow user to avoid putting in a schedule (defaults to constant flow at all times)
                }
                if (CurrentFlowSchedule > 1.0) {
                    CurrentFlowSchedule = 1.0; // Do not allow more flow than design maximum
                }
                if (CurrentFlowSchedule < 0.0) {
                    CurrentFlowSchedule = 0.0; // Do not allow negative flow
                }

                state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterMassFlowRate =
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate * CurrentFlowSchedule;

                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot) {
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterMassFlowRate =
                        state.dataLowTempRadSys->CFloRadSys(RadSysNum).EMSWaterMdotOverrideValue;
                }

                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode > 0) {
                    SetComponentFlowRate(state,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterMassFlowRate,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc);
                }
            }
            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem) {
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched != nullptr) {
                    CurrentFlowSchedule = state.dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched->getCurrentVal();
                } else {
                    CurrentFlowSchedule = 1.0; // Allow user to avoid putting in a schedule (defaults to constant flow at all times)
                }
                if (CurrentFlowSchedule > 1.0) {
                    CurrentFlowSchedule = 1.0; // Do not allow more flow than design maximum
                }
                if (CurrentFlowSchedule < 0.0) {
                    CurrentFlowSchedule = 0.0; // Do not allow negative flow
                }
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).ChWaterMassFlowRate =
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate * CurrentFlowSchedule;

                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot) {
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).ChWaterMassFlowRate =
                        state.dataLowTempRadSys->CFloRadSys(RadSysNum).EMSWaterMdotOverrideValue;
                }

                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode > 0) {
                    SetComponentFlowRate(state,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).ChWaterMassFlowRate,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode,
                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc);
                }
            }
            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).opMode != OpMode::None && FirstHVACIteration) {
                state.dataLowTempRadSys->CFloRadSys(RadSysNum).updateOperatingModeHistory(state);
            }

        } break;
        case SystemType::Electric: {
            state.dataLowTempRadSys->ElecRadSys(RadSysNum).opMode = OpMode::None;
        } break;
        default:
            break;
        }
    }

    void HydronicSystemBaseData::updateOperatingModeHistory(EnergyPlusData &state)
    {
        // Since this is only called when the operating mode is something other than "not operating",
        // the status from the previous system time step is what it did in the last or previous time step.
        // So, we can update the last status of the system using this information before resetting things
        // to "not operating".
        this->lastOpMode = this->opMode;

        if (state.dataGlobal->BeginDayFlag) {
            // The begin day flag is set which mean this is the first time step of the day.
            // This also means that the previous time step was the last time step of yesterday.
            // So, the day should be the previous day, the hour should be the last hour of the
            // day, and the time step should be the last time step.
            this->lastDayOfSim = state.dataGlobal->DayOfSim - 1;
            this->lastHourOfDay = Constant::iHoursInDay;
            this->lastTimeStep = state.dataGlobal->TimeStepsInHour;
        } else if (state.dataGlobal->BeginHourFlag) {
            // It's not the beginning of the day but it is the beginning of an hour other than
            // the first hour.  This means that the previous time step was the previous hour of
            // today in the last time step.  So, the day should be the current day, the hour should
            // be the previous hour, and the time step should be the last time step.
            this->lastDayOfSim = state.dataGlobal->DayOfSim;
            this->lastHourOfDay = state.dataGlobal->HourOfDay - 1;
            this->lastTimeStep = state.dataGlobal->TimeStepsInHour;
        } else if (state.dataGlobal->BeginTimeStepFlag) {
            // It's neither the beginning of the day nor the beginning of an hour but it is the start
            // of a time step other than the first time step in the hour.  So, the day should be the
            // current day, the hour should be the current hour, and the time step should be the
            // previous time step.
            this->lastDayOfSim = state.dataGlobal->DayOfSim;
            this->lastHourOfDay = state.dataGlobal->HourOfDay;
            this->lastTimeStep = state.dataGlobal->TimeStep - 1;
        } else {
            // It's not the beginning of the day, hour, or time step so the "last" value is simply the
            // same as the current value.  Note that these parameters only track down to the zone time
            // step level and will make decisions based on that.
            this->lastDayOfSim = state.dataGlobal->DayOfSim;
            this->lastHourOfDay = state.dataGlobal->HourOfDay;
            this->lastTimeStep = state.dataGlobal->TimeStep;
        }

        // Now go ahead and reset the operating mode (this will be set to something else if the system is running)
        this->opMode = OpMode::None;
    }

    void HydronicSystemBaseData::setOperatingModeBasedOnChangeoverDelay(EnergyPlusData &state)
    {
        if (this->lastOpMode == OpMode::None) {
            return; // this should only happen at the beginning of a simulation (at the start of warmup and the actual simulation)
        }
        // so let things proceed with whatever the system wants to do

        if (this->opMode == OpMode::None) {
            return; // always let it turn off
        }

        if (this->opMode == this->lastOpMode) {
            return; // always let it continue to operating in the same mode
        }

        if (this->changeoverDelaySched == nullptr) {
            return; // user not requesting any delays (no schedule entered) so let it do whatever is requested
        }

        Real64 currentChangeoverDelay = this->changeoverDelaySched->getCurrentVal();
        if (currentChangeoverDelay <= 0.0) {
            return; // delay is zero so let it do whatever it requested
        }

        // At this point, the radiant system is trying to switch modes from the previous time step, the user is requesting a delay in the changeover,
        // and the requested delay is greater than zero.  Calculate what the current time is in hours from the start of the simulation
        Real64 timeCurrent = 24.0 * float(state.dataGlobal->DayOfSim - 1) + float(state.dataGlobal->HourOfDay - 1) +
                             float(state.dataGlobal->TimeStep - 1) / float(state.dataGlobal->TimeStepsInHour);
        Real64 timeLast = 24.0 * float(this->lastDayOfSim - 1) + float(this->lastHourOfDay - 1) +
                          float(this->lastTimeStep - 1) / float(state.dataGlobal->TimeStepsInHour);
        Real64 actualTimeDifference = timeCurrent - timeLast;

        // If the time difference is not longer than the user delay, then the system should not switch modes and needs to be turned off.
        if (actualTimeDifference <= currentChangeoverDelay) {
            this->opMode = OpMode::None;
        }

        // Note: if the time difference is greater than the user delay request, then go ahead and keep the operating mode needed (don't do anything).
    }

    void SizeLowTempRadiantSystem(EnergyPlusData &state,
                                  int const RadSysNum, // Index for the low temperature radiant system under consideration within the derived types
                                  SystemType const systemType // Type of radiant system: hydronic, constant flow, or electric
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      August 2014 Bereket Nigusse, added scalable sizing
        //                      March 2014 Daeho Kang, add constant flow system autosizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing low temperature radiant components for which flow rates
        // and tube length or max electric power have not been specified in the input

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data. Maximum electric
        // power is set to the design heat load. Tube length is calculated by rule-of-thumb from
        // the surface area.

        // Using/Aliasing
        using namespace DataSizing;
        using HVAC::AutoCalculateSizing;
        using HVAC::CoolingCapacitySizing;
        using HVAC::HeatingCapacitySizing;

        using PlantUtilities::MyPlantSizingIndex;
        using PlantUtilities::RegisterPlantCompDesignFlow;

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr std::string_view RoutineName("SizeLowTempRadiantSystem");

        enum class OperatingMode
        {
            Invalid = -1,
            OFF,
            ClgHtg,
            ClgOnly,
            HtgOnly,
            Num
        };

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PltSizHeatNum(0);    // index of plant sizing object for 1st heating loop
        int PltSizCoolNum(0);    // index of plant sizing object for 1st cooling loop
        int SurfNum;             // surface index in radiant system data structure
        bool ErrorsFound(false); // If errors detected in input
        Real64 rho;
        Real64 Cp;
        bool IsAutoSize(false);              // Indicator to autosize
        Real64 WaterVolFlowMaxHeatDes(0.0);  // Design hot water flow for reporting
        Real64 WaterVolFlowMaxHeatUser(0.0); // User hard-sized hot water flow for
        Real64 WaterVolFlowMaxCoolDes(0.0);  // Design chilled water flow for reporting
        Real64 WaterVolFlowMaxCoolUser(0.0); // User hard-sized chilled water flow for reporting
        Real64 TubeLengthDes(0.0);           // Design tube length for reporting
        Real64 TubeLengthUser(0.0);          // User hard-sized tube length for reporting
        std::string CompName;                // component name
        std::string CompType;                // component type
        std::string SizingString;            // input field sizing description (e.g., Nominal Capacity)
        Real64 TempSize;                     // autosized value of coil input field
        int FieldNum;                        // IDD numeric field number where input field description is found
        bool PrintFlag;                      // TRUE when sizing information is reported in the eio file
        int CapSizingMethod;        // capacity sizing methods (HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity, and
                                    // FractionOfAutosizedHeatingCapacity )
        Real64 DesCoilLoad;         // design autosized or user specified capacity
        Real64 WaterVolFlowMaxDes;  // Design water volume flow rate for reporting
        Real64 WaterVolFlowMaxUser; // User hard-sized water volume flow rate for reporting

        DesCoilLoad = 0.0;
        state.dataSize->DataScalableCapSizingON = false;
        state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;

        auto const &Zone = state.dataHeatBal->Zone;

        if (systemType == SystemType::Electric) {

            if (state.dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower == AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

                CompType = "ZoneHVAC:LowTemperatureRadiant:Electric";
                CompName = state.dataLowTempRadSys->ElecRadSys(RadSysNum).Name;
                int SizingMethod = HeatingCapacitySizing;
                FieldNum = 1;
                PrintFlag = true;
                SizingString = state.dataLowTempRadSys->ElecRadSysNumericFields(RadSysNum).FieldNames(FieldNum) + " [W]";
                CapSizingMethod = state.dataLowTempRadSys->ElecRadSys(RadSysNum).HeatingCapMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;

                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (CapSizingMethod == HeatingDesignCapacity && state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity > 0.0) {
                        TempSize = state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity;
                        bool errorsFound = false;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                    } else if (CapSizingMethod == CapacityPerFloorArea) {
                        state.dataSize->DataScalableCapSizingON = true;
                        TempSize = state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity *
                                   Zone(state.dataLowTempRadSys->ElecRadSys(RadSysNum).ZonePtr).FloorArea;
                        bool errorsFound = false;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                        state.dataSize->DataScalableCapSizingON = false;
                        state.dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower = TempSize;
                    } else if (CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                        ShowSevereError(state,
                                        format("{}: auto-sizing cannot be done for {} = {}\".",
                                               RoutineName,
                                               CompType,
                                               state.dataLowTempRadSys->ElecRadSys(RadSysNum).Name));
                        ShowContinueError(state,
                                          "The \"SimulationControl\" object must have the field \"Do Zone Sizing Calculation\" set to Yes when the "
                                          "Heating Design Capacity Method = \"FractionOfAutosizedHeatingCapacity\".");
                        ErrorsFound = true;
                    }
                } else {
                    if (CapSizingMethod == HeatingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                        CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                        if (CapSizingMethod == HeatingDesignCapacity) {
                            if (state.dataSize->ZoneSizingRunDone) {
                                CheckZoneSizing(state, CompType, CompName);
                                state.dataSize->DataConstantUsedForSizing =
                                    state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                                state.dataSize->DataFractionUsedForSizing = 1.0;
                            }
                            if (state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity == AutoSize) {
                                TempSize = AutoSize;
                            } else {
                                TempSize = state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity;
                            }
                        } else if (CapSizingMethod == CapacityPerFloorArea) {
                            if (state.dataSize->ZoneSizingRunDone) {
                                CheckZoneSizing(state, CompType, CompName);
                                zoneEqSizing.HeatingCapacity = true;
                                zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                            }
                            TempSize = state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity *
                                       Zone(state.dataLowTempRadSys->ElecRadSys(RadSysNum).ZonePtr).FloorArea;
                            state.dataSize->DataScalableCapSizingON = true;

                        } else if (CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                            CheckZoneSizing(state, CompType, CompName);
                            zoneEqSizing.HeatingCapacity = true;
                            zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                            TempSize = zoneEqSizing.DesHeatingLoad * state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else {
                            TempSize = state.dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity;
                        }
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        state.dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataConstantUsedForSizing = 0.0;
                        state.dataSize->DataFractionUsedForSizing = 0.0;
                        state.dataSize->DataScalableCapSizingON = false;
                    }
                }
            }

        } else if (systemType == SystemType::Hydronic) {

            CompType = "ZoneHVAC:LowTemperatureRadiant:VariableFlow";
            CompName = state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name;

            IsAutoSize = false;
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity == AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

                int SizingMethod = HeatingCapacitySizing;
                FieldNum = 2;
                PrintFlag = true;
                SizingString = state.dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(FieldNum) + " [W]";
                CapSizingMethod = state.dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;

                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (CapSizingMethod == HeatingDesignCapacity && state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity > 0.0) {
                        TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity;
                        bool errorsFound = false;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                    } else if (CapSizingMethod == CapacityPerFloorArea) {
                        state.dataSize->DataScalableCapSizingON = true;
                        TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity *
                                   Zone(state.dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr).FloorArea;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataScalableCapSizingON = false;
                    } else if (CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                        if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat == AutoSize) {
                            ShowSevereError(state,
                                            format("{}: auto-sizing cannot be done for {} = {}\".",
                                                   RoutineName,
                                                   CompType,
                                                   state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                            ShowContinueError(state,
                                              "The \"SimulationControl\" object must have the field \"Do Zone Sizing Calculation\" set to Yes when "
                                              "the Heating Design Capacity Method = \"FractionOfAutosizedHeatingCapacity\".");
                            ErrorsFound = true;
                        }
                    }
                } else { // Autosize or hard-size with sizing run
                    if (CapSizingMethod == HeatingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                        CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                        if (CapSizingMethod == HeatingDesignCapacity) {
                            if (state.dataSize->ZoneSizingRunDone) {
                                CheckZoneSizing(state, CompType, CompName);
                                state.dataSize->DataConstantUsedForSizing =
                                    state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                                state.dataSize->DataFractionUsedForSizing = 1.0;
                            }
                            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity == AutoSize) {
                                TempSize = AutoSize;
                            } else {
                                TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity;
                            }
                        } else if (CapSizingMethod == CapacityPerFloorArea) {
                            if (state.dataSize->ZoneSizingRunDone) {
                                CheckZoneSizing(state, CompType, CompName);
                                zoneEqSizing.HeatingCapacity = true;
                                zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                            }
                            TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity *
                                       Zone(state.dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr).FloorArea;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else if (CapSizingMethod == FractionOfAutosizedHeatingCapacity) {
                            CheckZoneSizing(state, CompType, CompName);
                            zoneEqSizing.HeatingCapacity = true;
                            zoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                            TempSize = zoneEqSizing.DesHeatingLoad * state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else {
                            TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity;
                        }
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.overrideSizingString(SizingString);
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataConstantUsedForSizing = 0.0;
                        state.dataSize->DataFractionUsedForSizing = 0.0;
                        state.dataSize->DataScalableCapSizingON = false;
                    } else {
                        DesCoilLoad = 0.0;
                    }
                }
                // finally heating capacity is saved in this variable
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = DesCoilLoad;
            }

            IsAutoSize = false;
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat == AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                     "User-Specified Maximum Hot Water Flow [m3/s]",
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat);
                    }
                } else { // Autosize or hard-size with sizing run
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode > 0 &&
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode > 0) {
                        PltSizHeatNum = MyPlantSizingIndex(state,
                                                           CompType,
                                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode,
                                                           ErrorsFound);
                        if (PltSizHeatNum > 0) {
                            if (DesCoilLoad >= SmallLoad) {
                                rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                                Cp = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                                WaterVolFlowMaxHeatDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                            } else {
                                WaterVolFlowMaxHeatDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in ZoneHVAC:LowTemperatureRadiant:VariableFlow Object={}",
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                            ErrorsFound = true;
                        }
                    }

                    if (IsAutoSize) {
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat = WaterVolFlowMaxHeatDes;
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                     "Design Size Maximum Hot Water Flow [m3/s]",
                                                     WaterVolFlowMaxHeatDes);
                    } else { // hard-size with sizing data
                        if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat > 0.0 && WaterVolFlowMaxHeatDes > 0.0) {
                            WaterVolFlowMaxHeatUser = state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat;
                            BaseSizer::reportSizerOutput(state,
                                                         CompType,
                                                         state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                         "Design Size Maximum Hot Water Flow [m3/s]",
                                                         WaterVolFlowMaxHeatDes,
                                                         "User-Specified Maximum Hot Water Flow [m3/s]",
                                                         WaterVolFlowMaxHeatUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(WaterVolFlowMaxHeatDes - WaterVolFlowMaxHeatUser) / WaterVolFlowMaxHeatUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeLowTempRadiantSystem: Potential issue with equipment sizing for "
                                                       "ZoneHVAC:LowTemperatureRadiant:VariableFlow = \"{}\".",
                                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Hot Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxHeatUser));
                                    ShowContinueError(
                                        state, format("differs from Design Size Maximum Hot Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxHeatDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }

            IsAutoSize = false;
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity == AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

                int SizingMethod = CoolingCapacitySizing;
                FieldNum = 4;
                PrintFlag = true;
                SizingString = state.dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(FieldNum) + " [W]";
                CapSizingMethod = state.dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;

                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (CapSizingMethod == CoolingDesignCapacity && state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity > 0.0) {
                        TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity;
                        CoolingCapacitySizer sizerCoolingCapacity;
                        sizerCoolingCapacity.overrideSizingString(SizingString);
                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                    } else if (CapSizingMethod == CapacityPerFloorArea) {
                        state.dataSize->DataScalableCapSizingON = true;
                        TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity *
                                   Zone(state.dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr).FloorArea;
                        CoolingCapacitySizer sizerCoolingCapacity;
                        sizerCoolingCapacity.overrideSizingString(SizingString);
                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataScalableCapSizingON = false;
                    } else if (CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                        if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool == AutoSize) {
                            ShowSevereError(state,
                                            format("{}: auto-sizing cannot be done for {} = {}\".",
                                                   RoutineName,
                                                   CompType,
                                                   state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                            ShowContinueError(state,
                                              "The \"SimulationControl\" object must have the field \"Do Zone Sizing Calculation\" set to Yes when "
                                              "the Cooling Design Capacity Method = \"FractionOfAutosizedCoolingCapacity\".");
                            ErrorsFound = true;
                        }
                    }
                } else { // Autosize or hard-size with sizing run
                    if (CapSizingMethod == CoolingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                        CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                        if (CapSizingMethod == CoolingDesignCapacity) {
                            if (state.dataSize->ZoneSizingRunDone) {
                                CheckZoneSizing(state, CompType, CompName);
                                state.dataSize->DataConstantUsedForSizing =
                                    state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
                                state.dataSize->DataFractionUsedForSizing = 1.0;
                            }
                            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity == AutoSize) {
                                TempSize = AutoSize;
                            } else {
                                TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity;
                            }
                        } else if (CapSizingMethod == CapacityPerFloorArea) {
                            if (state.dataSize->ZoneSizingRunDone) {
                                CheckZoneSizing(state, CompType, CompName);
                                zoneEqSizing.CoolingCapacity = true;
                                zoneEqSizing.DesCoolingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
                            }
                            TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity *
                                       Zone(state.dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr).FloorArea;
                            state.dataSize->DataScalableCapSizingON = true;
                        } else if (CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                            CheckZoneSizing(state, CompType, CompName);
                            zoneEqSizing.CoolingCapacity = true;
                            zoneEqSizing.DesCoolingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
                            TempSize = zoneEqSizing.DesCoolingLoad * state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity;
                            state.dataSize->DataScalableCapSizingON = true;

                        } else {
                            TempSize = state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity;
                        }
                        CoolingCapacitySizer sizerCoolingCapacity;
                        sizerCoolingCapacity.overrideSizingString(SizingString);
                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataConstantUsedForSizing = 0.0;
                        state.dataSize->DataFractionUsedForSizing = 0.0;
                        state.dataSize->DataScalableCapSizingON = false;
                    } else {
                        DesCoilLoad = 0.0;
                    }
                }
                // finally cooling capacity is saved in this variable
                state.dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = DesCoilLoad;
            }

            IsAutoSize = false;
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool == AutoSize) {
                IsAutoSize = true;
            }
            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                     "User-Specified Maximum Cold Water Flow [m3/s]",
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool);
                    }
                } else { // Autosize or hard-size with sizing run
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode > 0 &&
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode > 0) {
                        PltSizCoolNum = MyPlantSizingIndex(state,
                                                           CompType,
                                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                                           state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode,
                                                           ErrorsFound);
                        if (PltSizCoolNum > 0) {
                            if (DesCoilLoad >= SmallLoad) {
                                rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                                Cp = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);
                                WaterVolFlowMaxCoolDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizCoolNum).DeltaT * Cp * rho);
                            } else {
                                WaterVolFlowMaxCoolDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in ZoneHVAC:LowTemperatureRadiant:VariableFlow Object={}",
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                            ErrorsFound = true;
                        }
                    }

                    if (IsAutoSize) {
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool = WaterVolFlowMaxCoolDes;
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                     "Design Size Maximum Cold Water Flow [m3/s]",
                                                     WaterVolFlowMaxCoolDes);
                    } else { // hard-size with sizing data
                        if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool > 0.0 && WaterVolFlowMaxCoolDes > 0.0) {
                            WaterVolFlowMaxCoolUser = state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool;
                            BaseSizer::reportSizerOutput(state,
                                                         CompType,
                                                         state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                         "Design Size Maximum Cold Water Flow [m3/s]",
                                                         WaterVolFlowMaxCoolDes,
                                                         "User-Specified Maximum Cold Water Flow [m3/s]",
                                                         WaterVolFlowMaxCoolUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(WaterVolFlowMaxCoolDes - WaterVolFlowMaxCoolUser) / WaterVolFlowMaxCoolUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeLowTempRadiantSystem: Potential issue with equipment sizing for "
                                                       "ZoneHVAC:LowTemperatureRadiant:VariableFlow = \"{}\".",
                                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Cool Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxCoolUser));
                                    ShowContinueError(
                                        state, format("differs from Design Size Maximum Cool Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxCoolDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }

            IsAutoSize = false;
            if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength == AutoSize) {
                IsAutoSize = true;
            }
            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                     "User-Specified Hydronic Tubing Length [m]",
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength);
                    }
                } else { // Autosize or hard-size with sizing run
                    // CheckZoneSizing is not required here because the tube length calculation is not dependent on zone sizing calculation results
                    TubeLengthDes = state.dataLowTempRadSys->HydrRadSys(RadSysNum).sizeRadiantSystemTubeLength(state);
                    if (IsAutoSize) {
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength = TubeLengthDes;
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                     "Design Size Hydronic Tubing Length [m]",
                                                     TubeLengthDes);
                    } else { // hard-size with sizing data
                        if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength > 0.0 && TubeLengthDes > 0.0) {
                            TubeLengthUser = state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength;
                            BaseSizer::reportSizerOutput(state,
                                                         CompType,
                                                         state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                                         "Design Size Hydronic Tubing Length [m]",
                                                         TubeLengthDes,
                                                         "User-Specified Hydronic Tubing Length [m]",
                                                         TubeLengthUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(TubeLengthDes - TubeLengthUser) / TubeLengthUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeLowTempRadiantSystem: Potential issue with equipment sizing for "
                                                       "ZoneHVAC:LowTemperatureRadiant:VariableFlow = \"{}\".",
                                                       state.dataLowTempRadSys->HydrRadSys(RadSysNum).Name));
                                    ShowContinueError(state, format("User-Specified Hydronic Tubing Length of {:.5R} [m]", TubeLengthUser));
                                    ShowContinueError(state, format("differs from Design Size Hydronic Tubing Length of {:.5R} [m]", TubeLengthDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }

            for (SurfNum = 1; SurfNum <= state.dataLowTempRadSys->HydrRadSys(RadSysNum).NumOfSurfaces; ++SurfNum) {
                if (state.dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircCalcMethod == CircuitCalc::CalculateFromLength) {
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircuits(SurfNum) =
                        (state.dataLowTempRadSys->HydrRadSys(RadSysNum).SurfaceFrac(SurfNum) *
                         state.dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength) /
                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).CircLength;
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircuits(SurfNum) =
                        max(state.dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircuits(SurfNum), 1.0);
                } else {
                    state.dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircuits(SurfNum) = 1.0;
                }
            }

            RegisterPlantCompDesignFlow(state,
                                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat);
            RegisterPlantCompDesignFlow(state,
                                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                        state.dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool);

        } else if (systemType == SystemType::ConstantFlow) {

            CompType = "ZoneHVAC:LowTemperatureRadiant:ConstantFlow";
            CompName = state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name;

            // Why is this not the same thing as the other OpMode enumeration? In the .hh file?
            OperatingMode OpMode; // System operating mode

            // Check which operating system it is
            int HeatNode = state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode;
            int CoolNode = state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode;
            if (HeatNode > 0 && CoolNode > 0) {
                OpMode = OperatingMode::ClgHtg;
            } else if (HeatNode > 0 && CoolNode <= 0) {
                OpMode = OperatingMode::HtgOnly;
            } else if (CoolNode > 0 && HeatNode <= 0) {
                OpMode = OperatingMode::ClgOnly;
            } else {
                OpMode = OperatingMode::OFF; // It shouldn't happen here
            }

            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax == AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                     "User-Specified Maximum Water Flow [m3/s]",
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax);
                    }
                } else { // Autosize or hard-size with sizing run
                    CheckZoneSizing(state, CompType, state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name);
                    // Estimate hot water and chilled water flows
                    // Index only if it provides heating to avoid severe error
                    if (OpMode == OperatingMode::ClgHtg || OpMode == OperatingMode::HtgOnly) {
                        PltSizHeatNum = MyPlantSizingIndex(state,
                                                           CompType,
                                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode,
                                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode,
                                                           ErrorsFound);
                    }
                    if (PltSizHeatNum > 0) {
                        if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad >= SmallLoad) {
                            rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum)
                                      .glycol->getDensity(state, Constant::HWInitConvTemp, "SizeLowTempRadiantSystem");
                            Cp = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum)
                                     .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, "SizeLowTempRadiantSystem");
                            WaterVolFlowMaxHeatDes = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad /
                                                     (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                        } else {
                            WaterVolFlowMaxHeatDes = 0.0;
                        }
                    } else {
                        if (OpMode == OperatingMode::ClgHtg || OpMode == OperatingMode::HtgOnly) {
                            ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in ZoneHVAC:LowTemperatureRadiant:ConstantFlow Object={}",
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name));
                            ErrorsFound = true;
                        }
                    }

                    // Index only if it provides cooling system to avoid severe error
                    if (OpMode == OperatingMode::ClgHtg || OpMode == OperatingMode::ClgOnly) {
                        PltSizCoolNum = MyPlantSizingIndex(state,
                                                           CompType,
                                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode,
                                                           state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode,
                                                           ErrorsFound);
                    }
                    if (PltSizCoolNum > 0) {
                        if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad >= SmallLoad) {
                            rho = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum)
                                      .glycol->getDensity(state, Constant::CWInitConvTemp, "SizeLowTempRadiantSystem");
                            Cp = state.dataPlnt->PlantLoop(state.dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum)
                                     .glycol->getSpecificHeat(state, Constant::CWInitConvTemp, "SizeLowTempRadiantSystem");
                            WaterVolFlowMaxCoolDes = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad /
                                                     (state.dataSize->PlantSizData(PltSizCoolNum).DeltaT * Cp * rho);
                        } else {
                            WaterVolFlowMaxCoolDes = 0.0;
                        }
                    } else {
                        if (OpMode == OperatingMode::ClgHtg || OpMode == OperatingMode::ClgOnly) {
                            ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in ZoneHVAC:LowTemperatureRadiant:ConstantFlow Object={}",
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name));
                            ErrorsFound = true;
                        }
                    }

                    // Determine maximum water flow rate depending upon system type
                    if (OpMode == OperatingMode::ClgHtg) {
                        WaterVolFlowMaxDes = std::max(WaterVolFlowMaxHeatDes, WaterVolFlowMaxCoolDes);
                    } else if (OpMode == OperatingMode::ClgOnly) {
                        WaterVolFlowMaxDes = WaterVolFlowMaxCoolDes;
                    } else if (OpMode == OperatingMode::HtgOnly) {
                        WaterVolFlowMaxDes = WaterVolFlowMaxHeatDes;
                    } else {
                        WaterVolFlowMaxDes = 0.0;
                    }

                    if (IsAutoSize) {
                        state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = WaterVolFlowMaxDes;
                        BaseSizer::reportSizerOutput(state,
                                                     CompType,
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                     "Design Size Maximum Water Flow [m3/s]",
                                                     WaterVolFlowMaxDes);
                    } else { // hard-size with sizing data
                        if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax > 0.0 && WaterVolFlowMaxDes > 0.0) {
                            WaterVolFlowMaxUser = state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax;
                            BaseSizer::reportSizerOutput(state,
                                                         CompType,
                                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                         "Design Size Maximum Water Flow [m3/s]",
                                                         WaterVolFlowMaxDes,
                                                         "User-Specified Maximum Water Flow [m3/s]",
                                                         WaterVolFlowMaxUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(WaterVolFlowMaxDes - WaterVolFlowMaxUser) / WaterVolFlowMaxUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeLowTempRadiantSystem: Potential issue with equipment sizing for "
                                                       "ZoneHVAC:LowTemperatureRadiant:ConstantFlow = \" {}\".",
                                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name));
                                    ShowContinueError(state, format("User-Specified Maximum Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxUser));
                                    ShowContinueError(state,
                                                      format("differs from Design Size Maximum Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }

            IsAutoSize = false;
            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength == AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                    if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ZoneHVAC:LowTemperatureRadiant:ConstantFlow",
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                     "User-Specified Hydronic Tubing Length [m]",
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength);
                    }
                } else { // Autosize or hard-size with sizing run
                    // CheckZoneSizing is not required here because the tube length calculation is not dependent on zone sizing calculation results
                    TubeLengthDes = state.dataLowTempRadSys->CFloRadSys(RadSysNum).sizeRadiantSystemTubeLength(state);
                    if (IsAutoSize) {
                        state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength = TubeLengthDes;
                        BaseSizer::reportSizerOutput(state,
                                                     "ZoneHVAC:LowTemperatureRadiant:ConstantFlow",
                                                     state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                     "Design Size Hydronic Tubing Length [m]",
                                                     TubeLengthDes);
                    } else { // hard-size with sizing data
                        if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength > 0.0 && TubeLengthDes > 0.0) {
                            TubeLengthUser = state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength;
                            BaseSizer::reportSizerOutput(state,
                                                         "ZoneHVAC:LowTemperatureRadiant:ConstantFlow",
                                                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name,
                                                         "Design Size Hydronic Tubing Length [m]",
                                                         TubeLengthDes,
                                                         "User-Specified Hydronic Tubing Length [m]",
                                                         TubeLengthUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(TubeLengthDes - TubeLengthUser) / TubeLengthUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeLowTempRadiantSystem: Potential issue with equipment sizing for "
                                                       "ZoneHVAC:LowTemperatureRadiant:ConstantFlow = \" {}\".",
                                                       state.dataLowTempRadSys->CFloRadSys(RadSysNum).Name));
                                    ShowContinueError(state, format("User-Specified Hydronic Tubing Length of {:.5R} [m]", TubeLengthUser));
                                    ShowContinueError(state, format("differs from Design Size Hydronic Tubing Length of {:.5R} [m]", TubeLengthDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }

            for (SurfNum = 1; SurfNum <= state.dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces; ++SurfNum) {
                if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircCalcMethod == CircuitCalc::CalculateFromLength) {
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircuits(SurfNum) =
                        (state.dataLowTempRadSys->CFloRadSys(RadSysNum).SurfaceFrac(SurfNum) *
                         state.dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength) /
                        state.dataLowTempRadSys->CFloRadSys(RadSysNum).CircLength;
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircuits(SurfNum) =
                        max(state.dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircuits(SurfNum), 1.0);
                } else {
                    state.dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircuits(SurfNum) = 1.0;
                }
            }
            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode > 0) {
                RegisterPlantCompDesignFlow(state,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax);
            }
            if (state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode > 0) {
                RegisterPlantCompDesignFlow(state,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode,
                                            state.dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    Real64 HydronicSystemBaseData::sizeRadiantSystemTubeLength(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   August 2017

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine figures out the tube length based on the spacing of tubes.
        // For single surface systems, this is fairly easy as there is only one spacing
        // to deal with.  For multi-surface systems, more work is necessary because each
        // surface could use a different spacing.

        // Return value
        Real64 sizeRadiantSystemTubeLength;

        Real64 tubeLength(0.0); // temporary holding place for the function calculation

        for (int surfNum = 1; surfNum <= this->NumOfSurfaces; ++surfNum) {
            auto &thisHydrSysSurf = state.dataSurface->Surface(this->SurfacePtr(surfNum));
            auto const &thisHydrSpacing = state.dataConstruction->Construct(thisHydrSysSurf.Construction).ThicknessPerpend;
            if ((thisHydrSpacing > 0.005) && (thisHydrSpacing < 0.5)) { // limit allowable spacing to between 1cm and 1m
                tubeLength += thisHydrSysSurf.Area / (2.0 * thisHydrSpacing);
            } else { // if not in allowable limit, default back to 0.15m (15cm or 6 inches)
                tubeLength += thisHydrSysSurf.Area / 0.15;
            }
        }

        sizeRadiantSystemTubeLength = tubeLength;
        return sizeRadiantSystemTubeLength;
    }

    void VariableFlowRadiantSystemData::calculateLowTemperatureRadiantSystem(EnergyPlusData &state,
                                                                             Real64 &LoadMet) // load met by the radiant system, in Watts
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does all of the stuff that is necessary to simulate
        // a low temperature hydronic radiant heating/cooling system.  Calls are
        // made to appropriate subroutines either in this module or outside of it.

        // METHODOLOGY EMPLOYED:
        // Follows the methods used by many other pieces of zone equipment.
        // Much like a water coil, a hydronic system will use the ControlCompOutput
        // routine to determine what fraction of capacity the unit should be
        // functioning at by controlling the flow rate of water to the element.

        // REFERENCES:
        // Other EnergyPlus modules
        // IBLAST-QTF research program, completed in January 1995 (unreleased)
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.
        // Seem, J.E. 1986. "Heat Transfer in Buildings", Ph.D. dissertation, University
        //   of Wisconsin-Madison.

        // Using/Aliasing
        using DataHeatBalance::ZoneData;
        using HVAC::SmallLoad;
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ActWaterFlow; // actual water flow for heating or cooling [kg/sec]
        Real64 ControlTemp;  // temperature of whatever is controlling the radiant system
        Real64 MassFlowFrac; // fraction of the maximum water flow rate as determined by the control algorithm
        Real64 MaxWaterFlow; // maximum water flow for heating or cooling [kg/sec]
        Real64 OffTempCool;  // temperature at which the flow rate throttles back to zero for cooling
        Real64 OffTempHeat;  // temperature at which the flow rate throttles back to zero for heating
        Real64 mdot;         // local temporary for fluid mass flow rate
        bool SysRunning;     // True when system is running

        VarFlowRadDesignData variableFlowDesignDataObject =                           // Is this intended to be a copy?
            state.dataLowTempRadSys->HydronicRadiantSysDesign(this->DesignObjectPtr); // Contains the data for variable flow hydronic systems

        MaxWaterFlow = 0.0;
        ActWaterFlow = 0.0;
        this->opMode = OpMode::None;
        SysRunning = true;

        if (this->availSched->getCurrentVal() <= 0) {

            // Unit is off or has no load upon it; set the flow rates to zero and then
            // simulate the components with the no flow conditions
            for (int SurfNum = 1; SurfNum <= this->NumOfSurfaces; ++SurfNum) {
                int SurfNum2 = this->SurfacePtr(SurfNum);
                auto &surface = state.dataSurface->Surface(SurfNum2);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                if (surface.ExtBoundCond > 0 && surface.ExtBoundCond != SurfNum2) {
                    state.dataHeatBalFanSys->QRadSysSource(surface.ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                }
            }
            if (this->HeatingSystem) {
                mdot = 0.0;
                SetComponentFlowRate(state, mdot, this->HotWaterInNode, this->HotWaterOutNode, this->HWPlantLoc);
            }
            if (this->CoolingSystem) {
                mdot = 0.0;
                SetComponentFlowRate(state, mdot, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
            }
        } else { // Unit might be on-->this section is intended to control the water mass flow rate being
            // sent to the radiant system

            ControlTemp = this->setRadiantSystemControlTemperature(state, variableFlowDesignDataObject.VarFlowControlType);

            if (variableFlowDesignDataObject.heatSetptSched != nullptr) {
                //                OffTempHeat = this->setOffTemperatureLowTemperatureRadiantSystem(state, this->HotSetptSchedPtr,
                //                this->HotThrottlRange);
                Real64 a;
                a = variableFlowDesignDataObject.HotThrottlRange; // Why are we doing this?
                OffTempHeat = this->setOffTemperatureLowTemperatureRadiantSystem(state,
                                                                                 variableFlowDesignDataObject.heatSetptSched,
                                                                                 variableFlowDesignDataObject.HotThrottlRange,
                                                                                 variableFlowDesignDataObject.VarFlowSetpointType);
            } else { // This system is not capable of heating, set OffTempHeat to something really low
                OffTempHeat = state.dataLowTempRadSys->LowTempHeating;
            }
            if (variableFlowDesignDataObject.coolSetptSched != nullptr) {
                OffTempCool = this->setOffTemperatureLowTemperatureRadiantSystem(state,
                                                                                 variableFlowDesignDataObject.coolSetptSched,
                                                                                 -variableFlowDesignDataObject.ColdThrottlRange,
                                                                                 variableFlowDesignDataObject.VarFlowSetpointType);
            } else { // This system is not capable of cooling, set OffTempCool to something really high
                OffTempCool = state.dataLowTempRadSys->HighTempCooling;
            }

            // Check for an illogical condition where a user enters controls that could
            // potentially be heating or cooling at a particular control temperature
            if (OffTempHeat > OffTempCool) {
                MassFlowFrac = 0.0;
                ShowSevereError(state, format("Overlapping heating and cooling control temps in radiant system: {}", this->Name));
                ShowFatalError(state, "Preceding condition causes termination.");

            } else { // Temperatures for heating and cooling do not overlap--calculate the mass flow fraction

                if (ControlTemp < OffTempHeat && this->HeatingSystem) { // Heating mode
                    this->opMode = OpMode::Heat;
                } else if (ControlTemp > OffTempCool && this->CoolingSystem) { // Cooling mode
                    this->opMode = OpMode::Cool;
                }

                this->setOperatingModeBasedOnChangeoverDelay(state);

                if (this->opMode == OpMode::Heat) {
                    MaxWaterFlow = this->WaterFlowMaxHeat;
                    MassFlowFrac = this->calculateOperationalFraction(OffTempHeat, ControlTemp, variableFlowDesignDataObject.HotThrottlRange);
                } else if (this->opMode == OpMode::Cool) {
                    MaxWaterFlow = this->WaterFlowMaxCool;
                    MassFlowFrac = this->calculateOperationalFraction(OffTempCool, ControlTemp, variableFlowDesignDataObject.ColdThrottlRange);
                } else {
                    MassFlowFrac = 0.0;
                }
            }

            // Calculate and limit the water flow rate
            ActWaterFlow = MassFlowFrac * MaxWaterFlow;
            if (ActWaterFlow < DataBranchAirLoopPlant::MassFlowTolerance) {
                ActWaterFlow = 0.0;
            }
            if (this->EMSOverrideOnWaterMdot) {
                ActWaterFlow = this->EMSWaterMdotOverrideValue;
            }

            if (this->opMode == OpMode::Heat) {
                if (this->HeatingSystem) {
                    SetComponentFlowRate(state, ActWaterFlow, this->HotWaterInNode, this->HotWaterOutNode, this->HWPlantLoc);
                } else { // not heating system
                    SysRunning = false;
                }
            } else if (this->opMode == OpMode::Cool) {
                if (this->CoolingSystem) {
                    SetComponentFlowRate(state, ActWaterFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                } else { // not cooling system
                    SysRunning = false;
                }
            }

            // Now simulate the system...
            if (((this->opMode == OpMode::Heat) || (this->opMode == OpMode::Cool)) && SysRunning) {
                this->calculateLowTemperatureRadiantSystemComponents(state, LoadMet, SystemType::Hydronic);
            }
        }
    }

    void VariableFlowRadiantSystemData::calculateLowTemperatureRadiantSystemComponents(
        EnergyPlusData &state,
        Real64 &LoadMet,
        SystemType const typeOfRadiantSystem) // Load met by the low temperature radiant system, in Watts
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000
        //       MODIFIED       Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves the radiant system based on how much water is (and
        // the conditions of the water) supplied to the radiant system.

        // METHODOLOGY EMPLOYED:
        // Use heat exchanger formulas to obtain the heat source/sink for the radiant
        // system based on the inlet conditions and flow rate of water.  Once that is
        // determined, recalculate the surface heat balances to reflect this heat
        // addition/subtraction.  The load met by the system is determined by the
        // difference between the convection from all surfaces in the zone when
        // there was no radiant system output and with a source/sink added.

        // REFERENCES:
        // IBLAST-QTF research program, completed in January 1995 (unreleased)
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.

        auto &Zone = state.dataHeatBal->Zone;

        // Using/Aliasing
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DewPointTemp;      // Dew-point temperature based on the zone air conditions
        Real64 EpsMdotCp;         // Epsilon (heat exchanger terminology) times water mass flow rate times water specific heat
        Real64 FullWaterMassFlow; // Original water mass flow rate before reducing the flow for condensation concerns
        Real64 LowestRadSurfTemp; // Lowest surface temperature of a radiant system (when condensation is a concern)
        Real64 PredictedCondTemp; // Temperature at which condensation is predicted (includes user parameter)
        Real64 ReductionFrac;     // Fraction that the flow should be reduced to avoid condensation
        Real64 SysWaterMassFlow;  // System level water mass flow rate (includes effect of zone multiplier)
        Real64 WaterMassFlow;     // Water mass flow rate in the radiant system, kg/s
        int WaterNodeIn;          // Node number of the water entering the radiant system
        Real64 WaterTempIn;       // Temperature of the water entering the radiant system, in C
        Real64 ZeroFlowSurfTemp;  // Temperature of radiant surface when flow is zero
        int ZoneNum;              // Zone pointer for this radiant system

        VarFlowRadDesignData variableFlowDesignDataObject =                           // Is this intended to be a deep copy?
            state.dataLowTempRadSys->HydronicRadiantSysDesign(this->DesignObjectPtr); // Contains the data for variable flow hydronic systems

        auto &Surface = state.dataSurface->Surface;

        Real64 Ca; // Coefficients to relate the inlet water temperature to the heat source
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
        // For more info on Ca through Cl, see comments below

        // First, apply heat exchanger logic to find the heat source/sink to the system.
        // This involves finding out the heat transfer characteristics of the hydronic
        // loop and then applying the equations derived on pp. 113-118 of the dissertation.

        // Set the conditions on the water side inlet
        switch (this->opMode) {
        case OpMode::Heat: {
            WaterNodeIn = this->HotWaterInNode;
        } break;
        case OpMode::Cool: {
            WaterNodeIn = this->ColdWaterInNode;
        } break;
        default: {
            WaterNodeIn = 0; // Suppress uninitialized warning
            ShowSevereError(state, "Illegal low temperature radiant system operating mode");
            ShowContinueError(state, format("Occurs in Radiant System={}", this->Name));
            ShowFatalError(state, "Preceding condition causes termination.");
        } break;
        }
        ZoneNum = this->ZonePtr;
        SysWaterMassFlow = state.dataLoopNodes->Node(WaterNodeIn).MassFlowRate;
        WaterMassFlow = state.dataLoopNodes->Node(WaterNodeIn).MassFlowRate / double(Zone(ZoneNum).Multiplier * Zone(ZoneNum).ListMultiplier);
        WaterTempIn = state.dataLoopNodes->Node(WaterNodeIn).Temp;

        if (WaterMassFlow <= 0.0) {
            // No flow or below minimum allowed so there is no heat source/sink
            // This is possible with a mismatch between system and plant operation
            // or a slight mismatch between zone and system controls.  This is not
            // necessarily a "problem" so this exception is necessary in the code.
            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                int SurfNum = this->SurfacePtr(RadSurfNum);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = 0.0;
                if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                    state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                }
            }

        } else {

            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {

                int SurfNum = this->SurfacePtr(RadSurfNum);
                // Determine the heat exchanger "effectiveness" term

                EpsMdotCp = calculateHXEffectivenessTerm(state,
                                                         SurfNum,
                                                         WaterTempIn,
                                                         WaterMassFlow,
                                                         this->SurfaceFrac(RadSurfNum),
                                                         this->NumCircuits(RadSurfNum),
                                                         this->DesignObjectPtr,
                                                         typeOfRadiantSystem);

                // Obtain the heat balance coefficients and calculate the intermediate coefficients
                // linking the inlet water temperature to the heat source/sink to the radiant system.
                // The coefficients are based on the following development...
                // The heat balance equations at the outside and inside surfaces are of the form:
                //   Tinside  = Ca + Cb*Toutside + Cc*q"
                //   Toutside = Cd + Ce*Tinside  + Cf*q"
                //   Tsource  = Cg + Ch*q"       + Ci*Tinside + Cj*Toutside
                // where:
                //   Tinside is the temperature at the inside surface
                //   Toutside is the temperature at the outside surface
                //   Tsource is the temperature within the radiant system at the location of the source/sink
                //   Ca is all of the other terms in the inside heat balance (solar, LW exchange, conduction history terms, etc.)
                //   Cb is the current cross CTF term
                //   Cc is the QTF inside term for the current heat source/sink
                //   Cd is all of the other terms in the outside heat balance (solar, LW exchange, conduction history terms, etc.)
                //   Ce is the current cross CTF term (should be equal to Cb)
                //   Cf is the QTF outside term for the current heat source/sink
                //   Cg is the summation of all temperature and source history terms at the source/sink location
                //   Ch is the QTF term at the source/sink location for the current heat source/sink
                //   Ci is the CTF inside term for the current inside surface temperature
                //   Cj is the CTF outside term for the current outside surface temperature
                // Note that it is necessary to not use "slow conduction" assumptions because the
                // source/sink has an impact on BOTH the inside and outside surface heat balances.
                // Hence the more general formulation.
                // The first two T equations above can be solved to remove the other surface temperature.
                // This results in the following equations:
                //   Tinside  = Ca + Cb*(Cd + Ce*Tinside + Cf*q") + Cc*q"   or...
                //   Tinside  = (Ca + Cb*Cd + (Cc+Cb*Cf)*q") / (1 - Ce*Cb)
                //   Toutside = Cd + Ce*(Ca + Cb*Toutside + Cc*q") + Cf*q"  or...
                //   Toutside = (Cd + Ce*Ca + (Cf+Ce*Cc)*q") / (1 - Ce*Cb)
                // Substituting the new equations for Tinside and Toutside as a function of C and q"
                // into the equation for Tsource...
                //   Tsource  = Cg + Ch*q" + Ci*((Ca + Cb*Cd + (Cc+Cb*Cf)*q") / (1 - Ce*Cb)) &
                //                         + Cj*((Cd + Ce*Ca + (Cf+Ce*Cc)*q") / (1 - Ce*Cb))
                // Or rearranging this to get Tsource as a function of q", we get...
                //   Tsource  =  Cg + ((Ci*(Ca + Cb*Cd) + Cj*(Cd + Ce*Ca))/(1-Ce*Cb)) &
                //             +(Ch + ((Ci*(Cc + Cb*Cf) + Cj*(Cf + Ce*Cc))/(1-Ce*Cb)))*q"
                // Or in a slightly simpler form...
                //   Tsource  = Ck + Cl*q"
                // where:
                //   Ck = Cg + ((Ci*(Ca + Cb*Cd) + Cj*(Cd + Ce*Ca))/(1-Ce*Cb))
                //   Cl = Ch + ((Ci*(Cc + Cb*Cf) + Cj*(Cf + Ce*Cc))/(1-Ce*Cb))
                // Note also that from heat exchanger "algebra", we have:
                //   q = epsilon*qmax    and    qmax = Mdot*Cp*(Twaterin-Tsource)
                // So...
                //   q" = q/Area = (epsilon*Mdot*Cp/Area)*(Twaterin-Tsource)
                // Or rearranging this equation:
                //   Tsource = -(q"*A/(epsilon*Mdot*Cp)) + Twaterin
                // Setting this equation equal to the other equation for Tsource a couple lines up
                // and rearranging to solve for q"...
                //   q" = (Twaterin - Ck) / (Cl + (A/(epsilon*Mdot*Cp))
                // or
                //   q  = (Twaterin - Ck) / ((Cl/A) + (1/epsilon*Mdot*Cp))
                // or
                //   q  = epsilon*Mdot*Cp*(Twaterin - Ck) / (1+(epsilon*Mdot*Cp*Cl/A))
                // which is the desired result, that is the heat source or sink to the radiant
                // system as a function of the water inlet temperature (flow rate is also in there
                // as well as all of the heat balance terms "hidden" in Ck and Cl).
                int ConstrNum = Surface(SurfNum).Construction;
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

                if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF) {

                    Ca = state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum);
                    Cb = state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum);
                    Cc = state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum);

                    Cd = state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum);
                    Ce = state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum);
                    Cf = state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum);

                    Cg = state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum);
                    Ch = thisConstruct.CTFTSourceQ[0];
                    Ci = thisConstruct.CTFTSourceIn[0];
                    Cj = thisConstruct.CTFTSourceOut[0];

                    Ck = Cg + ((Ci * (Ca + Cb * Cd) + Cj * (Cd + Ce * Ca)) / (1.0 - Ce * Cb));
                    Cl = Ch + ((Ci * (Cc + Cb * Cf) + Cj * (Cf + Ce * Cc)) / (1.0 - Ce * Cb));

                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) =
                        EpsMdotCp * (WaterTempIn - Ck) / (1.0 + (EpsMdotCp * Cl / Surface(SurfNum).Area));

                } else if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {

                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) = EpsMdotCp * (WaterTempIn - state.dataHeatBalFanSys->TCondFDSourceNode(SurfNum));
                }

                if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                    state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) =
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum); // Also set the other side of an interzone
                }
            }

            // "Temperature Comparison" Cut-off:
            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                // Check to see whether or not the system should really be running.  If
                // QRadSysSource is negative when we are in heating mode or QRadSysSource
                // is positive when we are in cooling mode, then the radiant system will
                // be doing the opposite of its intention.  In this case, the flow rate
                // is set to zero to avoid heating in cooling mode or cooling in heating
                // mode.
                int SurfNum = this->SurfacePtr(RadSurfNum);

                if (((this->opMode == OpMode::Heat) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) <= 0.0)) ||
                    ((this->opMode == OpMode::Cool) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) >= 0.0))) {
                    WaterMassFlow = 0.0;
                    if (this->opMode == OpMode::Heat) {
                        SetComponentFlowRate(state, WaterMassFlow, this->HotWaterInNode, this->HotWaterOutNode, this->HWPlantLoc);

                    } else if (this->opMode == OpMode::Cool) {
                        SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                    }
                    this->WaterMassFlowRate = WaterMassFlow;
                    this->opMode = OpMode::None;

                    for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                        int SurfNum2 = this->SurfacePtr(RadSurfNum2);
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                        if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                            state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                        }
                    }
                    break; // outer do loop
                }
            }

            // Condensation Cut-off:
            // Check to see whether there are any surface temperatures within the radiant system that have
            // dropped below the dew-point temperature.  If so, we need to shut off this radiant system.
            // A safety parameter is added (hardwired parameter) to avoid getting too close to condensation
            // conditions.
            this->CondCausedShutDown = false;
            DewPointTemp =
                PsyTdpFnWPb(state, state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRat, state.dataEnvrn->OutBaroPress);

            if ((this->opMode == OpMode::Cool) && (variableFlowDesignDataObject.condCtrlType == CondCtrlType::SimpleOff)) {

                for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                    if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) <
                        (DewPointTemp + variableFlowDesignDataObject.CondDewPtDeltaT)) {
                        // Condensation warning--must shut off radiant system
                        this->CondCausedShutDown = true;
                        WaterMassFlow = 0.0;
                        this->opMode = OpMode::None;
                        SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                        this->WaterMassFlowRate = WaterMassFlow;
                        for (int RadSurfNum3 = 1; RadSurfNum3 <= this->NumOfSurfaces; ++RadSurfNum3) {
                            int SurfNum2 = this->SurfacePtr(RadSurfNum3);
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                            if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) =
                                    0.0; // Also zero the other side of an interzone
                            }
                        }
                        // Produce a warning message so that user knows the system was shut-off due to potential for condensation
                        if (!state.dataGlobal->WarmupFlag) {
                            if (this->CondErrIndex == 0) { // allow errors up to number of radiant systems
                                ShowWarningMessage(state, format("{} [{}]", cHydronicSystem, this->Name));
                                ShowContinueError(state,
                                                  format("Surface [{}] temperature below dew-point temperature--potential for condensation exists",
                                                         Surface(this->SurfacePtr(RadSurfNum2)).Name));
                                ShowContinueError(state, "Flow to the radiant system will be shut-off to avoid condensation");
                                ShowContinueError(state,
                                                  format("Predicted radiant system surface temperature = {:.2R}",
                                                         state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2))));
                                ShowContinueError(state,
                                                  format("Zone dew-point temperature + safety delta T= {:.2R}",
                                                         DewPointTemp + variableFlowDesignDataObject.CondDewPtDeltaT));
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state,
                                                  format("Note that a {:.4R} C safety was chosen in the input for the shut-off criteria",
                                                         variableFlowDesignDataObject.CondDewPtDeltaT));
                                ShowContinueError(state, "Note also that this affects all surfaces that are part of this radiant system");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("{} [{}] condensation shut-off occurrence continues.", cHydronicSystem, this->Name),
                                                           this->CondErrIndex,
                                                           DewPointTemp,
                                                           DewPointTemp,
                                                           _,
                                                           "C",
                                                           "C");
                        }
                        break; // outer do loop
                    }
                }

            } else if ((this->opMode == OpMode::Cool) && (variableFlowDesignDataObject.condCtrlType == CondCtrlType::None)) {

                for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                    if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) < DewPointTemp) {
                        // Condensation occurring but user does not want to shut radiant system off ever
                        this->CondCausedShutDown = true;
                    }
                }

            } else if ((this->opMode == OpMode::Cool) && (variableFlowDesignDataObject.condCtrlType == CondCtrlType::VariedOff)) {

                LowestRadSurfTemp = 999.9;
                int CondSurfNum = 0;
                for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                    if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) <
                        (DewPointTemp + variableFlowDesignDataObject.CondDewPtDeltaT)) {
                        if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) < LowestRadSurfTemp) {
                            LowestRadSurfTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2));
                            CondSurfNum = RadSurfNum2;
                        }
                    }
                }

                if (CondSurfNum > 0) { // Condensation predicted so let's deal with it
                    // Process here is: turn everything off and see what the resulting surface temperature is for
                    // the surface that was causing the lowest temperature.  Then, interpolate to find the flow that
                    // would still allow the system to operate without producing condensation.  Rerun the heat balance
                    // and recheck for condensation.  If condensation still exists, shut everything down.  This avoids
                    // excessive iteration and still makes an attempt to vary the flow rate.
                    // First, shut everything off...
                    FullWaterMassFlow = WaterMassFlow;
                    WaterMassFlow = 0.0;
                    SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                    this->WaterMassFlowRate = WaterMassFlow;
                    for (int RadSurfNum3 = 1; RadSurfNum3 <= this->NumOfSurfaces; ++RadSurfNum3) {
                        int SurfNum2 = this->SurfacePtr(RadSurfNum3);
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                        if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                            state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                        }
                    }
                    // Redo the heat balances since we have changed the heat source (set it to zero)
                    HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                    HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);
                    // Now check all of the surface temperatures.  If any potentially have condensation, leave the system off.
                    for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                        if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) <
                            (DewPointTemp + variableFlowDesignDataObject.CondDewPtDeltaT)) {
                            this->CondCausedShutDown = true;
                        }
                    }
                    // If the system does not need to be shut down, then let's see if we can vary the flow based
                    // on the lowest temperature surface from before.  This will use interpolation to try a new
                    // flow rate.
                    if (!this->CondCausedShutDown) {
                        PredictedCondTemp = DewPointTemp + variableFlowDesignDataObject.CondDewPtDeltaT;
                        ZeroFlowSurfTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(CondSurfNum));
                        ReductionFrac = (ZeroFlowSurfTemp - PredictedCondTemp) / std::abs(ZeroFlowSurfTemp - LowestRadSurfTemp);
                        if (ReductionFrac < 0.0) {
                            ReductionFrac = 0.0; // Shouldn't happen as the above check should have screened this out
                        }
                        if (ReductionFrac > 1.0) {
                            ReductionFrac = 1.0; // Shouldn't happen either because condensation doesn't exist then
                        }
                        WaterMassFlow = ReductionFrac * FullWaterMassFlow;
                        SysWaterMassFlow = double(Zone(ZoneNum).Multiplier * Zone(ZoneNum).ListMultiplier) * WaterMassFlow;
                        // Got a new reduced flow rate that should work...reset loop variable and resimulate the system
                        SetComponentFlowRate(state, SysWaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                        this->WaterMassFlowRate = SysWaterMassFlow;

                        // Go through all of the surfaces again with the new flow rate...
                        for (int RadSurfNum3 = 1; RadSurfNum3 <= this->NumOfSurfaces; ++RadSurfNum3) {
                            int SurfNum = this->SurfacePtr(RadSurfNum3);
                            // Determine the heat exchanger "effectiveness" term

                            EpsMdotCp = calculateHXEffectivenessTerm(state,
                                                                     SurfNum,
                                                                     WaterTempIn,
                                                                     WaterMassFlow,
                                                                     this->SurfaceFrac(RadSurfNum3),
                                                                     this->NumCircuits(RadSurfNum3),
                                                                     this->DesignObjectPtr,
                                                                     typeOfRadiantSystem);

                            int ConstrNum = Surface(SurfNum).Construction;
                            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                            if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF) {
                                // For documentation on coefficients, see code earlier in this subroutine
                                Ca = state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum);
                                Cb = state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum);
                                Cc = state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum);
                                Cd = state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum);
                                Ce = state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum);
                                Cf = state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum);
                                Cg = state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum);
                                Ch = thisConstruct.CTFTSourceQ[0];
                                Ci = thisConstruct.CTFTSourceIn[0];
                                Cj = thisConstruct.CTFTSourceOut[0];
                                Ck = Cg + ((Ci * (Ca + Cb * Cd) + Cj * (Cd + Ce * Ca)) / (1.0 - Ce * Cb));
                                Cl = Ch + ((Ci * (Cc + Cb * Cf) + Cj * (Cf + Ce * Cc)) / (1.0 - Ce * Cb));
                                state.dataHeatBalFanSys->QRadSysSource(SurfNum) =
                                    EpsMdotCp * (WaterTempIn - Ck) / (1.0 + (EpsMdotCp * Cl / Surface(SurfNum).Area));
                            } else if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                                state.dataHeatBalFanSys->QRadSysSource(SurfNum) =
                                    EpsMdotCp * (WaterTempIn - state.dataHeatBalFanSys->TCondFDSourceNode(SurfNum));
                            }
                            if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                                state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) =
                                    state.dataHeatBalFanSys->QRadSysSource(SurfNum); // Also set the other side of an interzone
                            }
                        }

                        // Redo the heat balances since we have changed the heat source
                        HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                        HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

                        // Check for condensation one more time.  If no condensation, we are done.  If there is
                        // condensation, shut things down and be done.
                        for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                            if (this->CondCausedShutDown) {
                                break;
                            }
                            if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) < (PredictedCondTemp)) {
                                // Condensation still present--must shut off radiant system
                                this->CondCausedShutDown = true;
                                WaterMassFlow = 0.0;
                                this->opMode = OpMode::None;
                                SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                                this->WaterMassFlowRate = WaterMassFlow;
                                for (int RadSurfNum3 = 1; RadSurfNum3 <= this->NumOfSurfaces; ++RadSurfNum3) {
                                    int SurfNum2 = this->SurfacePtr(RadSurfNum3);
                                    state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                                    if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                        state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) =
                                            0.0; // Also zero the other side of an interzone
                                    }
                                }
                            }
                        }
                    }

                    if (this->CondCausedShutDown) {
                        // Produce a warning message so that user knows the system was shut-off due to potential for condensation
                        if (!state.dataGlobal->WarmupFlag) {
                            if (this->CondErrIndex == 0) { // allow errors up to number of radiant systems
                                ShowWarningMessage(state, format("{} [{}]", cHydronicSystem, this->Name));
                                ShowContinueError(state,
                                                  format("Surface [{}] temperature below dew-point temperature--potential for condensation exists",
                                                         Surface(this->SurfacePtr(CondSurfNum)).Name));
                                ShowContinueError(state, "Flow to the radiant system will be shut-off to avoid condensation");
                                ShowContinueError(state,
                                                  format("Predicted radiant system surface temperature = {:.2R}",
                                                         state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(CondSurfNum))));
                                ShowContinueError(state,
                                                  format("Zone dew-point temperature + safety delta T= {:.2R}",
                                                         DewPointTemp + variableFlowDesignDataObject.CondDewPtDeltaT));
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state,
                                                  format("Note that a {:.4R} C safety was chosen in the input for the shut-off criteria",
                                                         variableFlowDesignDataObject.CondDewPtDeltaT));
                                ShowContinueError(state, "Note also that this affects all surfaces that are part of this radiant system");
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("{} [{}] condensation shut-off occurrence continues.", cHydronicSystem, this->Name),
                                                           this->CondErrIndex,
                                                           DewPointTemp,
                                                           DewPointTemp,
                                                           _,
                                                           "C",
                                                           "C");
                        }
                    }
                } // Condensation Predicted in Variable Shut-Off Control Type
            } // In cooling mode and one of the condensation control types
        } // There was a non-zero flow

        // Now that we have the source/sink term, we must redo the heat balances to obtain
        // the new SumHATsurf value for the zone.  Note that the difference between the new
        // SumHATsurf and the value originally calculated by the heat balance with a zero
        // source for all radiant systems in the zone is the load met by the system (approximately).
        HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
        HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

        LoadMet = state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - this->ZeroLTRSourceSumHATsurf;
    }

    void ConstantFlowRadiantSystemData::calculateLowTemperatureRadiantSystem(EnergyPlusData &state,
                                                                             Real64 &LoadMet) // load met by the radiant system, in Watts
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   August 2003

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does all of the stuff that is necessary to simulate
        // a constant flow low temperature hydronic radiant heating/cooling system.
        // Calls are made to appropriate subroutines either in this module or
        // outside of it.

        // METHODOLOGY EMPLOYED:
        // Similar in many aspects to the hydronic (variable flow) radiant system
        // except that flow rate through the radiant system is constant (based on
        // the user schedule) and the inlet temperature is varied by injecting
        // more or less fluid from the main loop to achieve the desired inlet
        // temperature.

        // REFERENCES:
        // Other EnergyPlus modules
        // IBLAST-QTF research program, completed in January 1995 (unreleased)
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.
        // Seem, J.E. 1986. "Heat Transfer in Buildings", Ph.D. dissertation, University
        //   of Wisconsin-Madison.

        // Using/Aliasing
        using DataHeatBalance::ZoneData;
        using HVAC::SmallLoad;
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr LowCpFluidValue(100.0); // lowest allowed Cp fluid value (to avoid dividing by zero) [J/kg-K]
        constexpr std::string_view RoutineName("CalcLowTempCFloRadiantSystem");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CpFluid;         // Specific heat of the fluid in the radiant system
        Real64 InjectFlowRate;  // Calculated injection flow rate that will meet the inlet temperature requirement
        Real64 OffTempCool;     // temperature at which the cooling shuts down
        Real64 OffTempHeat;     // temperature at which the heating shuts down
        Real64 PumpPartLoadRat; // Pump part load ratio (based on user schedule, or 1.0 for no schedule)
        Real64 PumpTempRise;    // Temperature rise of the fluid as it passes through the pump
        Real64 RadInTemp;       // "Desired" radiant system water inlet temperature [Celsius]
        Real64 SetPointTemp;    // temperature that will be used to control the radiant system [Celsius]
        Real64 SetPointTempHi;  // Current high point in setpoint temperature range
        Real64 SetPointTempLo;  // Current low point in setpoint temperature range
        Real64 ShaftPower;      // Amount of power expended at the pump shaft
        bool SysRunning;        // TRUE when the system is running
        Real64 SysWaterInTemp;  // Fluid temperature supplied from the loop
        Real64 WaterTempHi;     // Current high point in water temperature range
        Real64 WaterTempLo;     // Current low point in water temperature range
        Real64 mdot;            // local temporary for water mass flow rate kg/s

        ConstantFlowRadDesignData ConstantFlowDesignDataObject =                   // Once again, is this supposed to be a deep copy?
            state.dataLowTempRadSys->CflowRadiantSysDesign(this->DesignObjectPtr); // Contains the data for variable flow hydronic systems

        // initialize local variables
        SysRunning = true; // default to running and turn off only if not running
        state.dataLowTempRadSys->VarOffCond = false;

        if (this->availSched->getCurrentVal() <= 0) {
            SysRunning = false;
        }

        if (SysRunning) { // Unit is probably on-->this section is intended to control the water
            // mass flow rate being sent to the radiant system

            // Set the current setpoint temperature (same procedure for either heating or cooling)

            SetPointTemp = this->setRadiantSystemControlTemperature(state, ConstantFlowDesignDataObject.ConstFlowControlType);

            // Avoid problems when there is no heating or cooling control because the system only cools or heats
            if (this->hotCtrlHiTempSched != nullptr) {
                OffTempHeat = this->hotCtrlHiTempSched->getCurrentVal();
            } else {
                OffTempHeat = state.dataLowTempRadSys->LowTempHeating;
            }
            if (this->coldCtrlLoTempSched != nullptr) {
                OffTempCool = this->coldCtrlLoTempSched->getCurrentVal();
            } else {
                OffTempCool = state.dataLowTempRadSys->HighTempCooling;
            }

            if (SetPointTemp < OffTempHeat && this->HeatingSystem) { // Heating mode
                this->opMode = OpMode::Heat;
            } else if (SetPointTemp > OffTempCool && this->CoolingSystem) { // Cooling mode
                this->opMode = OpMode::Cool;
            }

            this->setOperatingModeBasedOnChangeoverDelay(state);

            // Now actually decide what to do based on the setpoint temperature in relation to the control temperatures
            if (this->opMode == OpMode::Heat) { // HEATING MODE

                this->WaterMassFlowRate = this->HotWaterMassFlowRate;

                if (!this->HeatingSystem) {

                    SysRunning = false; // Can't heat unless it's a heating system

                } else { // It is a heating system so set all of the values for controls

                    SetPointTempHi = this->hotCtrlHiTempSched->getCurrentVal();
                    SetPointTempLo = this->hotCtrlLoTempSched->getCurrentVal();
                    if (SetPointTempHi < SetPointTempLo) {
                        ShowSevereError(state, format("Heating setpoint temperature mismatch in{}", this->Name));
                        ShowContinueError(state, "High setpoint temperature is less than low setpoint temperature--check your schedule input");
                        ShowFatalError(state, "Preceding condition causes termination.");
                    }

                    WaterTempHi = this->hotWaterHiTempSched->getCurrentVal();
                    WaterTempLo = this->hotWaterLoTempSched->getCurrentVal();
                    if (WaterTempHi < WaterTempLo) {
                        ShowSevereError(state, format("Heating water temperature mismatch in{}", this->Name));
                        ShowContinueError(state, "High water temperature is less than low water temperature--check your schedule input");
                        ShowFatalError(state, "Preceding condition causes termination.");
                    }

                    if (SetPointTemp >= SetPointTempHi) {
                        // System is above high heating setpoint so we should be able to turn the system off
                        RadInTemp = WaterTempLo;
                        SysRunning = false;
                    } else if (SetPointTemp <= SetPointTempLo) {
                        // System is running with its highest inlet temperature
                        RadInTemp = WaterTempHi;
                    } else {
                        // Interpolate to obtain the current radiant system inlet temperature
                        RadInTemp = WaterTempHi - (WaterTempHi - WaterTempLo) * (SetPointTemp - SetPointTempLo) / (SetPointTempHi - SetPointTempLo);
                    }
                }

            } else if (this->opMode == OpMode::Cool) { // COOLING MODE

                this->WaterMassFlowRate = this->ChWaterMassFlowRate;

                if (!this->CoolingSystem) {

                    SysRunning = false; // Can't cool unless it's a cooling system

                } else { // It is a cooling system so set all of the values for controls

                    SetPointTempHi = this->coldCtrlHiTempSched->getCurrentVal();
                    SetPointTempLo = this->coldCtrlLoTempSched->getCurrentVal();
                    if (SetPointTempHi < SetPointTempLo) {
                        ShowSevereError(state, format("Cooling setpoint temperature mismatch in{}", this->Name));
                        ShowContinueError(state, "High setpoint temperature is less than low setpoint temperature--check your schedule input");
                        ShowFatalError(state, "Preceding condition causes termination.");
                    }

                    WaterTempHi = this->coldWaterHiTempSched->getCurrentVal();
                    WaterTempLo = this->coldWaterLoTempSched->getCurrentVal();
                    if (WaterTempHi < WaterTempLo) {
                        ShowSevereError(state, format("Cooling water temperature mismatch in{}", this->Name));
                        ShowContinueError(state, "High water temperature is less than low water temperature--check your schedule input");
                        ShowFatalError(state, "Preceding condition causes termination.");
                    }

                    if (SetPointTemp <= SetPointTempLo) {
                        // System is below low cooling setpoint so we should be able to turn the system off
                        RadInTemp = WaterTempHi;
                        SysRunning = false;
                    } else if (SetPointTemp >= SetPointTempHi) {
                        // System is running with its lowest inlet temperature
                        RadInTemp = WaterTempLo;
                    } else {
                        // Interpolate to obtain the current radiant system inlet temperature
                        RadInTemp = WaterTempHi - (WaterTempHi - WaterTempLo) * (SetPointTemp - SetPointTempLo) / (SetPointTempHi - SetPointTempLo);
                    }
                }

            } else { // System is not running because the setpoint temperature is in the "deadband"

                RadInTemp = SetPointTemp;
                SysRunning = false;
            }
        }

        if (SysRunning) {
            CpFluid = this->water->getSpecificHeat(state, RadInTemp, RoutineName);
        }

        if ((!SysRunning) || (CpFluid < LowCpFluidValue)) {
            // Unit is off or has no load upon it OR CpFluid value is "zero" so
            // set the flow rates to zero and then simulate the components with
            // the no flow conditions
            this->opMode = OpMode::None;
            this->WaterMassFlowRate = 0.0;
            this->WaterInjectionRate = 0.0;
            this->WaterRecircRate = 0.0;
            this->HeatPower = 0.0;
            this->CoolPower = 0.0;
            this->PumpPower = 0.0;
            this->PumpMassFlowRate = 0.0;
            this->PumpHeattoFluid = 0.0;

            for (int SurfNum = 1; SurfNum <= this->NumOfSurfaces; ++SurfNum) {
                int SurfNum2 = this->SurfacePtr(SurfNum);
                auto &surface = state.dataSurface->Surface(SurfNum2);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                if (surface.ExtBoundCond > 0 && surface.ExtBoundCond != SurfNum2) {
                    state.dataHeatBalFanSys->QRadSysSource(surface.ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                }
            }

            // turn off flow requests made during init because it is not actually running
            if (this->CWPlantLoc.loopNum > 0) {
                mdot = 0.0;
                SetComponentFlowRate(state, mdot, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
            }
            if (this->HWPlantLoc.loopNum > 0) {
                mdot = 0.0;
                SetComponentFlowRate(state, mdot, this->HotWaterInNode, this->HotWaterOutNode, this->HWPlantLoc);
            }
        } else {            // (SysRunning) so simulate the system...
            bool Iteration; // FALSE when a normal solution, TRUE when it is a solution where we must also find the inlet temp
            int LoopInNode; // Node on the loop that is the inlet to the constant flow radiant system

            // Determine pump flow rate and pump heat addition
            this->PumpMassFlowRate = this->WaterMassFlowRate; // Set in InitLowTempRadiantSystem
            PumpPartLoadRat = (this->volFlowSched != nullptr) ? this->volFlowSched->getCurrentVal() : 1.0;

            this->PumpPower = PumpPartLoadRat * this->NomPowerUse;
            ShaftPower = this->PumpPower * ConstantFlowDesignDataObject.MotorEffic;
            // This adds the pump heat based on User input for the pump (same as in Pump module)
            // We assume that all of the heat ends up in the fluid eventually since this is a closed loop.
            this->PumpHeattoFluid = ShaftPower + ((this->PumpPower - ShaftPower) * ConstantFlowDesignDataObject.FracMotorLossToFluid);
            if (this->PumpMassFlowRate > 0.0) {
                PumpTempRise = this->PumpHeattoFluid / (this->PumpMassFlowRate * CpFluid);
            } else {
                PumpTempRise = 0.0;
            }

            state.dataLowTempRadSys->LoopReqTemp =
                RadInTemp - PumpTempRise; // Temperature required at the inlet of the pump to meet the temperature request

            if (this->opMode == OpMode::Heat) {

                // in heating mode so shut down cold water flow request
                if (this->CWPlantLoc.loopNum > 0) {
                    mdot = 0.0;
                    SetComponentFlowRate(state, mdot, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                }
                LoopInNode = this->HotWaterInNode;
                auto const &hotNode = state.dataLoopNodes->Node(LoopInNode);
                SysWaterInTemp = hotNode.Temp;
                Iteration = false;

                if ((SysWaterInTemp >= state.dataLowTempRadSys->LoopReqTemp) && (hotNode.MassFlowRateMaxAvail >= this->WaterMassFlowRate)) {
                    // Case 1: Adequate temperature and flow
                    // Best condition--loop inlet temperature greater than requested and we have enough flow.
                    // So, proceed assuming the RadInTemp requested by the controls and then figure out the
                    // mixing after the outlet radiant temperature is calculated.
                    this->WaterInletTemp = RadInTemp;
                    this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);

                    // We now have inlet and outlet temperatures--we still need to set the flow rates
                    if ((SysWaterInTemp - this->WaterOutletTemp) != 0.0) { // protect divide by zero
                        this->WaterInjectionRate =
                            (this->WaterMassFlowRate * (this->WaterInletTemp - this->WaterOutletTemp) / (SysWaterInTemp - this->WaterOutletTemp)) -
                            (this->PumpHeattoFluid / (CpFluid * (SysWaterInTemp - this->WaterOutletTemp)));
                    } else {
                        this->WaterInjectionRate = this->WaterMassFlowRate;
                    }
                    this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;

                } else if ((SysWaterInTemp < state.dataLowTempRadSys->LoopReqTemp) && (hotNode.MassFlowRateMaxAvail >= this->WaterMassFlowRate)) {
                    // Case 2: Adequate flow but temperature too low
                    // Only thing to do is to reset the inlet temperature and assume that the loop will supply
                    // the entire flow to the component (no recirculation but potentially some bypass for the
                    // overall loop).  There is no way we can meet the control temperature so don't even try.
                    this->WaterInletTemp = SysWaterInTemp + PumpTempRise;
                    this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);

                    // We now have inlet and outlet temperatures--we still need to set the flow rates
                    if ((SysWaterInTemp - this->WaterOutletTemp) != 0.0) { // protect divide by zero
                        this->WaterInjectionRate =
                            (this->WaterMassFlowRate * (this->WaterInletTemp - this->WaterOutletTemp) / (SysWaterInTemp - this->WaterOutletTemp)) -
                            (this->PumpHeattoFluid / (CpFluid * (SysWaterInTemp - this->WaterOutletTemp)));
                    } else {
                        this->WaterInjectionRate = this->WaterMassFlowRate;
                    }
                    if (this->WaterInjectionRate > this->WaterMassFlowRate) {
                        this->WaterInjectionRate = this->WaterMassFlowRate;
                    }
                    this->WaterRecircRate = 0.0; // by definition

                } else if ((SysWaterInTemp >= state.dataLowTempRadSys->LoopReqTemp) && (hotNode.MassFlowRateMaxAvail < this->WaterMassFlowRate)) {
                    // Case 3: Adequate temperature but loop flow is less than component flow
                    // This case might work out, but there is no guarantee that there is enough loop flow to
                    // mix with the recirculation flow and still provide a high enough temperature.  First
                    // step is to try the inlet temperature and flow rate as in Case 1.  If we can obtain
                    // the proper temperature inlet to the radiant system, then we are done.  If not, we
                    // have to repeat the solution for an unknown inlet temperature and a known recirculation
                    // rate.
                    this->WaterInletTemp = RadInTemp;
                    this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);

                    // Now see if we can really get that desired into temperature (RadInTemp) by solving
                    // for the flow that is injected from the loop.  A heat balance for the mixer that relates
                    // the important quantities is:
                    //   Mdotradsys*Cp*Tradsysin = Mdotloop*Cp*Tloop + (Mdotradsys-Mdotloop)*Cp*Tradsysout + PumpHeat
                    // or rearranging to get the injection flow (Mdotloop):
                    //   Mdotloop = Mdotcomp*(Tradsysin-Tradsysout)/(Tloop-Tradsysout) - PumpHeat/(Cp*(Tloop-Tradsysout))
                    // If Mdotloop from this equation is greater that the loop flow rate (Node%MassFlowRate),
                    // then we cannot meet the inlet temperature and we have to "iterate" through the
                    // alternate solution.
                    if ((SysWaterInTemp - this->WaterOutletTemp) != 0.0) { // protect divide by zero
                        InjectFlowRate =
                            (this->WaterMassFlowRate * (this->WaterInletTemp - this->WaterOutletTemp) / (SysWaterInTemp - this->WaterOutletTemp)) -
                            (this->PumpHeattoFluid / (CpFluid * (SysWaterInTemp - this->WaterOutletTemp)));
                    } else {
                        InjectFlowRate = this->WaterMassFlowRate;
                    }
                    if (InjectFlowRate > hotNode.MassFlowRateMaxAvail) {
                        // We didn't have enough flow from the loop to meet our inlet temperature request.
                        // So, set the injection rate to the loop flow and calculate the recirculation flow.
                        // Then, resimulate the radiant system using these values (it will obtain the actual
                        // inlet temperature that results from this).
                        this->WaterInjectionRate = hotNode.MassFlowRateMaxAvail;
                        this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;
                        this->WaterInletTemp = SysWaterInTemp + PumpTempRise;
                        Iteration = true;
                        this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);
                    } else {
                        this->WaterInjectionRate = InjectFlowRate;
                        this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;
                    }

                } else if ((SysWaterInTemp < state.dataLowTempRadSys->LoopReqTemp) && (hotNode.MassFlowRateMaxAvail < this->WaterMassFlowRate)) {
                    // Case 4: Temperature too low and loop flow is less than component flow
                    // Worst condition--can't meet the temperature request at all.  Only thing to do is to
                    // set the loop flow and recirculation rate (known) and solve for the inlet temperature
                    // using the "iteration" solution scheme from "Case 3B" above
                    this->WaterInjectionRate = hotNode.MassFlowRateMaxAvail;
                    this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;
                    this->WaterInletTemp = SysWaterInTemp + PumpTempRise;
                    Iteration = true;
                    this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);
                }

            } else if (this->opMode == OpMode::Cool) {

                // in cooling mode so shut down heating water flow request
                if (this->HWPlantLoc.loopNum > 0) {
                    mdot = 0.0;
                    SetComponentFlowRate(state, mdot, this->HotWaterInNode, this->HotWaterOutNode, this->HWPlantLoc);
                }
                LoopInNode = this->ColdWaterInNode;
                auto const &coldNode = state.dataLoopNodes->Node(LoopInNode);
                SysWaterInTemp = coldNode.Temp;
                state.dataLowTempRadSys->CFloCondIterNum = 1;
                while ((state.dataLowTempRadSys->CFloCondIterNum <= 1) ||
                       ((state.dataLowTempRadSys->CFloCondIterNum <= 2) && (ConstantFlowDesignDataObject.condCtrlType == CondCtrlType::VariedOff) &&
                        (state.dataLowTempRadSys->VarOffCond))) {
                    Iteration = false;

                    if ((SysWaterInTemp <= state.dataLowTempRadSys->LoopReqTemp) && (coldNode.MassFlowRateMaxAvail >= this->WaterMassFlowRate)) {
                        // Case 1: Adequate temperature and flow
                        // Best condition--loop inlet temperature lower than requested and we have enough flow.
                        // So, proceed assuming the RadInTemp requested by the controls and then figure out the
                        // mixing after the outlet radiant temperature is calculated.

                        // This condition can also happen when state.dataLowTempRadSys->LoopReqTemp has been reset  to dewpoint for condensation
                        // control
                        if (!state.dataLowTempRadSys->VarOffCond) {
                            this->WaterInletTemp = RadInTemp;
                        } else {
                            this->WaterInletTemp = state.dataLowTempRadSys->LoopReqTemp;
                        }
                        this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);

                        // We now have inlet and outlet temperatures--we still need to set the flow rates
                        if ((SysWaterInTemp - this->WaterOutletTemp) != 0.0) { // protect div by zero
                            this->WaterInjectionRate = (this->WaterMassFlowRate * (this->WaterInletTemp - this->WaterOutletTemp) /
                                                        (SysWaterInTemp - this->WaterOutletTemp)) -
                                                       (this->PumpHeattoFluid / (CpFluid * (SysWaterInTemp - this->WaterOutletTemp)));
                        } else {
                            this->WaterInjectionRate = this->WaterMassFlowRate;
                        }
                        this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;

                    } else if ((SysWaterInTemp > state.dataLowTempRadSys->LoopReqTemp) &&
                               (coldNode.MassFlowRateMaxAvail >= this->WaterMassFlowRate)) {
                        // Case 2: Adequate flow but temperature too high
                        // Only thing to do is to reset the inlet temperature and assume that the loop will supply
                        // the entire flow to the component (no recirculation but potentially some bypass for the
                        // overall loop).  There is no way we can meet the control temperature so don't even try.
                        this->WaterInletTemp = SysWaterInTemp + PumpTempRise;
                        this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);

                        // We now have inlet and outlet temperatures--we still need to set the flow rates
                        if ((SysWaterInTemp - this->WaterOutletTemp) != 0.0) { // protect div by zero
                            this->WaterInjectionRate = (this->WaterMassFlowRate * (this->WaterInletTemp - this->WaterOutletTemp) /
                                                        (SysWaterInTemp - this->WaterOutletTemp)) -
                                                       (this->PumpHeattoFluid / (CpFluid * (SysWaterInTemp - this->WaterOutletTemp)));
                        } else { // no temp change present, set injection rate to full flow
                            this->WaterInjectionRate = this->WaterMassFlowRate;
                        }
                        if (this->WaterInjectionRate > this->WaterMassFlowRate) {
                            this->WaterInjectionRate = this->WaterMassFlowRate;
                        }
                        this->WaterRecircRate = 0.0; // by definition

                    } else if ((SysWaterInTemp <= state.dataLowTempRadSys->LoopReqTemp) &&
                               (coldNode.MassFlowRateMaxAvail < this->WaterMassFlowRate)) {
                        // Case 3: Adequate temperature but loop flow is less than component flow
                        // This case might work out, but there is no guarantee that there is enough loop flow to
                        // mix with the recirculation flow and still provide a high enough temperature.  First
                        // step is to try the inlet temperature and flow rate as in Case 1.  If we can obtain
                        // the proper temperature inlet to the radiant system, then we are done.  If not, we
                        // have to repeat the solution for an unknown inlet temperature and a known recirculation
                        // rate.
                        // This condition might happen when state.dataLowTempRadSys->LoopReqTemp has been reset  to dewpoint for condensation control
                        if (!state.dataLowTempRadSys->VarOffCond) {
                            this->WaterInletTemp = RadInTemp;
                        } else {
                            this->WaterInletTemp = state.dataLowTempRadSys->LoopReqTemp;
                        }
                        this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);

                        // Now see if we can really get that desired into temperature (RadInTemp) by solving
                        // for the flow that is injected from the loop.  A heat balance for the mixer that relates
                        // the important quantities is:
                        //   Mdotradsys*Cp*Tradsysin = Mdotloop*Cp*Tloop + (Mdotradsys-Mdotloop)*Cp*Tradsysout + PumpHeat
                        // or rearranging to get the injection flow (Mdotloop):
                        //   Mdotloop = Mdotcomp*(Tradsysin-Tradsysout)/(Tloop-Tradsysout) - PumpHeat/(Cp*(Tloop-Tradsysout))
                        // If Mdotloop from this equation is greater that the loop flow rate (Node%MassFlowRate),
                        // then we cannot meet the inlet temperature and we have to "iterate" through the
                        // alternate solution.
                        if ((SysWaterInTemp - this->WaterOutletTemp) != 0.0) { // protect div by zero
                            InjectFlowRate = (this->WaterMassFlowRate * (this->WaterInletTemp - this->WaterOutletTemp) /
                                              (SysWaterInTemp - this->WaterOutletTemp)) -
                                             (this->PumpHeattoFluid / (CpFluid * (SysWaterInTemp - this->WaterOutletTemp)));
                        } else {
                            InjectFlowRate = this->WaterMassFlowRate;
                        }
                        if (InjectFlowRate > coldNode.MassFlowRateMaxAvail) {
                            // We didn't have enough flow from the loop to meet our inlet temperature request.
                            // So, set the injection rate to the loop flow and calculate the recirculation flow.
                            // Then, resimulate the radiant system using these values (it will obtain the actual
                            // inlet temperature that results from this).
                            this->WaterInjectionRate = coldNode.MassFlowRateMaxAvail;
                            this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;
                            this->WaterInletTemp = SysWaterInTemp + PumpTempRise;
                            Iteration = true;
                            this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);
                        } else {
                            this->WaterInjectionRate = InjectFlowRate;
                            this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;
                        }

                    } else if ((SysWaterInTemp > state.dataLowTempRadSys->LoopReqTemp) && (coldNode.MassFlowRateMaxAvail < this->WaterMassFlowRate)) {
                        // Case 4: Temperature too low and loop flow is less than component flow
                        // Worst condition--can't meet the temperature request at all.  Only thing to do is to
                        // set the loop flow and recirculation rate (known) and solve for the inlet temperature
                        // using the "iteration" solution scheme from "Case 3B" above
                        this->WaterInjectionRate = coldNode.MassFlowRateMaxAvail;
                        this->WaterRecircRate = this->WaterMassFlowRate - this->WaterInjectionRate;
                        this->WaterInletTemp = SysWaterInTemp + PumpTempRise;
                        Iteration = true;
                        this->calculateLowTemperatureRadiantSystemComponents(state, LoopInNode, Iteration, LoadMet, SystemType::ConstantFlow);
                    }

                    ++state.dataLowTempRadSys->CFloCondIterNum;
                }

            } // Operating mode (heating or cooling)

            // Case when system has been shut down because of condensation issues or other limitations:
            if (this->WaterMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
                this->WaterMassFlowRate = 0.0;
                this->WaterInjectionRate = 0.0;
                this->WaterRecircRate = 0.0;
                this->PumpMassFlowRate = 0.0;
                this->opMode = OpMode::None;
            }

            // There are some cases when the pump heat is actually enough to provide all the heating that the system needs.
            // In this case, the water injection flow rate will come back as a slightly negative number.  Reset it to zero
            // and just recirculate all the flow through the local loop.
            if (this->WaterInjectionRate < 0.0) {
                this->WaterInjectionRate = 0.0;
                this->WaterRecircRate = this->WaterMassFlowRate;
            }

            // Error check, just in case
            if (this->WaterRecircRate < 0.0) {
                ShowWarningError(state, "Flow mismatch in radiant system--result will be an energy imbalance--should not get this error");
                ShowContinueErrorTimeStamp(state, format("WaterRecircRate={:.2T}, in Radiant System={},", this->WaterRecircRate, this->Name));
                this->WaterRecircRate = 0.0;
                this->WaterInjectionRate = this->WaterMassFlowRate;
            }

        } // System running mode (yes or no)
    }

    void ConstantFlowRadiantSystemData::calculateLowTemperatureRadiantSystemComponents(
        EnergyPlusData &state,
        int const MainLoopNodeIn, // Node number on main loop of the inlet node to the radiant system
        bool const Iteration,     // FALSE for the regular solution, TRUE when we had to loop back
        Real64 &LoadMet,          // Load met by the low temperature radiant system, in Watts
        SystemType const typeOfRadiantSystem)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   August 2003
        //       MODIFIED       Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves the radiant system based on how much water is (and
        // the conditions of the water) supplied to the radiant system.  The purpose
        // of this subroutine is similar to CalcLowTempHydrRadSysComps except that
        // it solves this for a constant flow hydronic radiant system.

        // METHODOLOGY EMPLOYED:
        // Use heat exchanger formulas to obtain the heat source/sink for the radiant
        // system based on the inlet conditions and flow rate of water.  Once that is
        // determined, recalculate the surface heat balances to reflect this heat
        // addition/subtraction.  The load met by the system is determined by the
        // difference between the convection from all surfaces in the zone when
        // there was no radiant system output and with a source/sink added.

        // REFERENCES:
        // IBLAST-QTF research program, completed in January 1995 (unreleased)
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.

        auto &Zone = state.dataHeatBal->Zone;

        // Using/Aliasing
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr TempCheckLimit(0.1); // Maximum allowed temperature difference between outlet temperature calculations
        Real64 constexpr ZeroSystemResp(0.1); // Response below which the system response is really zero
        constexpr std::string_view RoutineName("CalcLowTempCFloRadSysComps");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 Cp;                    // Intermediate calculational variable for specific heat of water
        Real64 DewPointTemp;          // Dew-point temperature based on the zone air conditions
        Real64 EpsMdotCp;             // Epsilon (heat exchanger terminology) times water mass flow rate times water specific heat
        Real64 LoopTerm;              // Intermeidate calculation variable for determining the water inlet temperature
        Real64 Mdot;                  // Intermediate calculation variable for mass flow rate in a surface within the radiant system
        Real64 RecircTerm;            // Intermeidate calculation variable for determining the water inlet temperature
        Real64 SumFlowFracCkCm;       // Summation of surface flow fraction, Ck, and Cm product for each surface in the system
        Real64 SumFlowFracOneMinusCm; // Summation of surface flow fraction times (1-Cm) for each surface in the radiant system
        Real64 TotalRadSysPower;      // Total heat source/sink to radiant system
        Real64 TwiCoeff;              // Intermeidate calculation variable for determining the water inlet temperature
        Real64 WaterMassFlow;         // Water mass flow rate in the radiant system, kg/s
        Real64 WaterOutletTempCheck;  // Radiant system water outlet temperature (calculated from mixing all outlet streams together)
        Real64 WaterTempIn;           // Temperature of the water entering the radiant system, in C
        int ZoneNum;                  // number of zone being served
        Real64 ZoneMult;              // Zone multiplier for this system

        ConstantFlowRadDesignData ConstantFlowDesignDataObject =                   // I repeat myself
            state.dataLowTempRadSys->CflowRadiantSysDesign(this->DesignObjectPtr); // Contains the data for variable flow hydronic systems
        auto &Surface = state.dataSurface->Surface;

        Real64 Ca; // Coefficients to relate the inlet water temperature to the heat source
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
        // For more info on Ca through Cl, see comments below

        // First, apply heat exchanger logic to find the heat source/sink to the system.
        // This involves finding out the heat transfer characteristics of the hydronic
        // loop and then applying the equations derived on pp. 113-118 of the dissertation.
        if (state.dataLowTempRadSys->FirstTimeFlag) {
            state.dataLowTempRadSys->Ckj.allocate(state.dataLowTempRadSys->MaxCloNumOfSurfaces);
            state.dataLowTempRadSys->Cmj.allocate(state.dataLowTempRadSys->MaxCloNumOfSurfaces);
            state.dataLowTempRadSys->WaterTempOut.allocate(state.dataLowTempRadSys->MaxCloNumOfSurfaces);
            state.dataLowTempRadSys->FirstTimeFlag = false;
        }

        state.dataLowTempRadSys->Ckj = 0.0;
        state.dataLowTempRadSys->Cmj = 0.0;
        state.dataLowTempRadSys->WaterTempOut = this->WaterInletTemp;

        ZoneNum = this->ZonePtr;
        ZoneMult = double(Zone(ZoneNum).Multiplier * Zone(ZoneNum).ListMultiplier);
        WaterMassFlow = this->WaterMassFlowRate / ZoneMult;
        WaterTempIn = this->WaterInletTemp;

        if (WaterMassFlow <= 0.0) {
            // No flow or below minimum allowed so there is no heat source/sink
            // This is possible with a mismatch between system and plant operation
            // or a slight mismatch between zone and system controls.  This is not
            // necessarily a "problem" so this exception is necessary in the code.
            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                int SurfNum = this->SurfacePtr(RadSurfNum);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = 0.0;
                if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                    state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                }
            }

            this->WaterOutletTemp = this->WaterInletTemp;

        } else {

            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                int SurfNum = this->SurfacePtr(RadSurfNum);
                // Determine the heat exchanger "effectiveness" term

                EpsMdotCp = calculateHXEffectivenessTerm(state,
                                                         SurfNum,
                                                         WaterTempIn,
                                                         WaterMassFlow,
                                                         this->SurfaceFrac(RadSurfNum),
                                                         this->NumCircuits(RadSurfNum),
                                                         this->DesignObjectPtr,
                                                         typeOfRadiantSystem);

                // Obtain the heat balance coefficients and calculate the intermediate coefficients
                // linking the inlet water temperature to the heat source/sink to the radiant system.
                // The coefficients are based on the following development...
                // The heat balance equations at the outside and inside surfaces are of the form:
                //   Tinside  = Ca + Cb*Toutside + Cc*q"
                //   Toutside = Cd + Ce*Tinside  + Cf*q"
                //   Tsource  = Cg + Ch*q"       + Ci*Tinside + Cj*Toutside
                // where:
                //   Tinside is the temperature at the inside surface
                //   Toutside is the temperature at the outside surface
                //   Tsource is the temperature within the radiant system at the location of the source/sink
                //   Ca is all of the other terms in the inside heat balance (solar, LW exchange, conduction history terms, etc.)
                //   Cb is the current cross CTF term
                //   Cc is the QTF inside term for the current heat source/sink
                //   Cd is all of the other terms in the outside heat balance (solar, LW exchange, conduction history terms, etc.)
                //   Ce is the current cross CTF term (should be equal to Cb)
                //   Cf is the QTF outside term for the current heat source/sink
                //   Cg is the summation of all temperature and source history terms at the source/sink location
                //   Ch is the QTF term at the source/sink location for the current heat source/sink
                //   Ci is the CTF inside term for the current inside surface temperature
                //   Cj is the CTF outside term for the current outside surface temperature
                // Note that it is necessary to not use "slow conduction" assumptions because the
                // source/sink has an impact on BOTH the inside and outside surface heat balances.
                // Hence the more general formulation.
                // The first two T equations above can be solved to remove the other surface temperature.
                // This results in the following equations:
                //   Tinside  = Ca + Cb*(Cd + Ce*Tinside + Cf*q") + Cc*q"   or...
                //   Tinside  = (Ca + Cb*Cd + (Cc+Cb*Cf)*q") / (1 - Ce*Cb)
                //   Toutside = Cd + Ce*(Ca + Cb*Toutside + Cc*q") + Cf*q"  or...
                //   Toutside = (Cd + Ce*Ca + (Cf+Ce*Cc)*q") / (1 - Ce*Cb)
                // Substituting the new equations for Tinside and Toutside as a function of C and q"
                // into the equation for Tsource...
                //   Tsource  = Cg + Ch*q" + Ci*((Ca + Cb*Cd + (Cc+Cb*Cf)*q") / (1 - Ce*Cb)) &
                //                         + Cj*((Cd + Ce*Ca + (Cf+Ce*Cc)*q") / (1 - Ce*Cb))
                // Or rearranging this to get Tsource as a function of q", we get...
                //   Tsource  =  Cg + ((Ci*(Ca + Cb*Cd) + Cj*(Cd + Ce*Ca))/(1-Ce*Cb)) &
                //             +(Ch + ((Ci*(Cc + Cb*Cf) + Cj*(Cf + Ce*Cc))/(1-Ce*Cb)))*q"
                // Or in a slightly simpler form...
                //   Tsource  = Ck + Cl*q"
                // where:
                //   Ck = Cg + ((Ci*(Ca + Cb*Cd) + Cj*(Cd + Ce*Ca))/(1-Ce*Cb))
                //   Cl = Ch + ((Ci*(Cc + Cb*Cf) + Cj*(Cf + Ce*Cc))/(1-Ce*Cb))
                // Note also that from heat exchanger "algebra", we have:
                //   q = epsilon*qmax    and    qmax = Mdot*Cp*(Twaterin-Tsource)
                // So...
                //   q" = q/Area = (epsilon*Mdot*Cp/Area)*(Twaterin-Tsource)
                // Or rearranging this equation:
                //   Tsource = -(q"*A/(epsilon*Mdot*Cp)) + Twaterin
                // Setting this equation equal to the other equation for Tsource a couple lines up
                // and rearranging to solve for q"...
                //   q" = (Twaterin - Ck) / (Cl + (A/(epsilon*Mdot*Cp))
                // or
                //   q  = (Twaterin - Ck) / ((Cl/A) + (1/epsilon*Mdot*Cp))
                // or
                //   q  = epsilon*Mdot*Cp*(Twaterin - Ck) / (1+(epsilon*Mdot*Cp*Cl/A))
                // which is the desired result, that is the heat source or sink to the radiant
                // system as a function of the water inlet temperature (flow rate is also in there
                // as well as all of the heat balance terms "hidden" in Ck and Cl).

                int ConstrNum = Surface(SurfNum).Construction;
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

                Ca = state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum);
                Cb = state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum);
                Cc = state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum);

                Cd = state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum);
                Ce = state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum);
                Cf = state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum);

                Cg = state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum);
                Ch = thisConstruct.CTFTSourceQ[0];
                Ci = thisConstruct.CTFTSourceIn[0];
                Cj = thisConstruct.CTFTSourceOut[0];

                Ck = Cg + ((Ci * (Ca + Cb * Cd) + Cj * (Cd + Ce * Ca)) / (1.0 - Ce * Cb));
                Cl = Ch + ((Ci * (Cc + Cb * Cf) + Cj * (Cf + Ce * Cc)) / (1.0 - Ce * Cb));

                Mdot = WaterMassFlow * this->SurfaceFrac(RadSurfNum);
                Cp = this->water->getSpecificHeat(state, WaterTempIn, RoutineName);

                if (!Iteration) {

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF) {
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum) =
                            EpsMdotCp * (WaterTempIn - Ck) / (1.0 + (EpsMdotCp * Cl / Surface(SurfNum).Area));
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum) =
                            EpsMdotCp * (WaterTempIn - state.dataHeatBalFanSys->TCondFDSourceNode(SurfNum));
                    }

                    if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                        state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) =
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum); // Also set the other side of an interzone
                    }
                    state.dataLowTempRadSys->WaterTempOut(RadSurfNum) = WaterTempIn - (state.dataHeatBalFanSys->QRadSysSource(SurfNum) / (Mdot * Cp));
                } else { // (Iteration)
                    // In this case, we did not know the inlet temperature directly and have
                    // to figure it out as part of the solution.  Thus, we have to do a little
                    // more algebra.
                    // The last equation in the previous block was:
                    //   q = epsilon*Mdot*Cp*(Twaterin - Ck) / (1+(epsilon*Mdot*Cp*Cl/A))
                    // which combines with:
                    //   q = Mdot*Cp*(Twaterin - Twaterout,j)
                    // so that:
                    //   (Twaterin - Twaterout.j) = epsilon*(Twaterin - Ck) / (1+(epsilon*Mdot*Cp*Cl/A))
                    // Let:
                    //   Cm = epsilonj / (1+(epsilonj*Mdot,j*Cp*Cl,j/A))
                    // for each surface in the radiant system.  This results in:
                    //   (Twaterin - Twaterout,j) = Cm,j*(Twaterin - Ck,j)
                    // Or:
                    //   Twaterout,j = (1 - Cm,j)*Twaterin + Cm,j*Ck,j
                    // This holds for each surface that is part of the radiant system (j).  To get the
                    // overall outlet temperature, we have to do a mixing calculation after all of the
                    // surfaces have been simulated:
                    //   Twaterout = SUM(Fractionj*Twaterout,j)
                    // We also have to solve an energy balance at the mixing valve and add in pump heat.
                    // The energy balance at the mixing valve relates the loop inlet temperature (Tloopin)
                    // and the overall outlet temperature (Twaterout):
                    //   Tpumpin = (Mdotloop/Mdotradsys)*Tloopin + (Mdotrecirc/Mdotradsys)*Twaterout
                    // This can then be related to the inlet water temperature to the radiant system
                    // after pump heat has been taken into account:
                    //   Twaterin = (Mdotloop/Mdotradsys)*Tloopin + (Mdotrecirc/Mdotradsys)*Twaterout + PumpHeat/(Mdotradsys*Cp)
                    // Pluggin in the definition of Twaterout (sum equation above) and then the definition
                    // of each individual Twaterout,j equation (which is solely a function of Twaterin
                    // and coefficients), we can obtain an equation for Twaterin that consists of all
                    // known quantities.  This requires us to calculate Ck,j and Cm,j for all the radiant
                    // surfaces in the system first and then coming up with a calculation for Twaterin.
                    // After than, individual Twaterout,j can be calculated along with QRadSysSource.
                    state.dataLowTempRadSys->Ckj(RadSurfNum) = Ck;
                    state.dataLowTempRadSys->Cmj(RadSurfNum) = (EpsMdotCp / (Mdot * Cp)) / (1.0 + (EpsMdotCp * Cl / Surface(SurfNum).Area));

                    if (RadSurfNum == this->NumOfSurfaces) { // Last one so we can now do the other calculations
                        // Equation for Twaterin is:
                        //   Twaterin = (LoopTerm + RecircTerm)/(TwiCoeff)
                        // where:
                        //   LoopTerm   = (Mdotloop/Mdotradsys)*Tloopin + PumpHeat/(Mdotradsys*Cp)
                        //   RecircTerm = (Mdotrecirc/Mdotradsys)*SUM(FlowFracj*Ck,j*Cm,j)
                        //   TwiCoeff   = 1 - (Mdotrecirc/Mdotradsys)*SUM(FlowFracj*(1 - Cm,j))
                        SumFlowFracCkCm = 0.0;
                        SumFlowFracOneMinusCm = 0.0;
                        for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                            SumFlowFracCkCm += (this->SurfaceFrac(RadSurfNum2) * state.dataLowTempRadSys->Ckj(RadSurfNum) *
                                                state.dataLowTempRadSys->Cmj(RadSurfNum2));
                            SumFlowFracOneMinusCm += (this->SurfaceFrac(RadSurfNum2) * (1.0 - state.dataLowTempRadSys->Cmj(RadSurfNum2)));
                        }

                        LoopTerm = (this->WaterInjectionRate / this->WaterMassFlowRate) * state.dataLoopNodes->Node(MainLoopNodeIn).Temp +
                                   (this->PumpHeattoFluid / (this->WaterMassFlowRate * Cp));

                        RecircTerm = (this->WaterRecircRate / this->WaterMassFlowRate) * SumFlowFracCkCm;

                        TwiCoeff = 1.0 - (this->WaterRecircRate / this->WaterMassFlowRate) * SumFlowFracOneMinusCm;

                        WaterTempIn = (LoopTerm + RecircTerm) / (TwiCoeff);

                        this->WaterInletTemp = WaterTempIn;

                        for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                            state.dataLowTempRadSys->WaterTempOut(RadSurfNum2) =
                                WaterTempIn * (1.0 - state.dataLowTempRadSys->Cmj(RadSurfNum2)) +
                                (state.dataLowTempRadSys->Ckj(RadSurfNum2) * state.dataLowTempRadSys->Cmj(RadSurfNum2));
                            Mdot = WaterMassFlow * this->SurfaceFrac(RadSurfNum2);
                            int SurfNum2 = this->SurfacePtr(RadSurfNum2);
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum2) =
                                Mdot * Cp * (WaterTempIn - state.dataLowTempRadSys->WaterTempOut(RadSurfNum2));
                            if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) =
                                    state.dataHeatBalFanSys->QRadSysSource(SurfNum2); // Also set the other side of an interzone
                            }
                        }
                    }
                }
            }

            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                int SurfNum = this->SurfacePtr(RadSurfNum);
                // "Temperature Comparison" Cut-off:
                // Check to see whether or not the system should really be running.  If
                // QRadSysSource is negative when we are in heating mode or QRadSysSource
                // is positive when we are in cooling mode, then the radiant system will
                // be doing the opposite of its intention.  In this case, the flow rate
                // is set to zero to avoid heating in cooling mode or cooling in heating
                // mode.
                if (((this->opMode == OpMode::Heat) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) <= 0.0)) ||
                    ((this->opMode == OpMode::Cool) && (state.dataHeatBalFanSys->QRadSysSource(SurfNum) >= 0.0))) {
                    WaterMassFlow = 0.0;
                    if (this->opMode == OpMode::Heat) {
                        SetComponentFlowRate(state, WaterMassFlow, this->HotWaterInNode, this->HotWaterOutNode, this->HWPlantLoc);
                    } else if (this->opMode == OpMode::Cool) {
                        SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                    }
                    this->WaterMassFlowRate = WaterMassFlow;
                    this->opMode = OpMode::None;
                    for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                        int SurfNum2 = this->SurfacePtr(RadSurfNum2);
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                        if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                            state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                        }
                    }
                    break; // outer do loop
                }
            }
            // Condensation Cut-off:
            // Check to see whether there are any surface temperatures within the radiant system that have
            // dropped below the dew-point temperature.  If so, we need to shut off this radiant system.
            // A safety parameter is added (hardwired parameter) to avoid getting too close to condensation
            // conditions.
            this->CondCausedShutDown = false;
            DewPointTemp =
                PsyTdpFnWPb(state, state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->ZonePtr).airHumRat, state.dataEnvrn->OutBaroPress);

            if ((this->opMode == OpMode::Cool) && (ConstantFlowDesignDataObject.condCtrlType == CondCtrlType::SimpleOff)) {

                for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                    if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) <
                        (DewPointTemp + ConstantFlowDesignDataObject.CondDewPtDeltaT)) {
                        // Condensation warning--must shut off radiant system
                        this->CondCausedShutDown = true;
                        WaterMassFlow = 0.0;
                        this->opMode = OpMode::None;
                        SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                        this->WaterMassFlowRate = WaterMassFlow;
                        for (int RadSurfNum3 = 1; RadSurfNum3 <= this->NumOfSurfaces; ++RadSurfNum3) {
                            int SurfNum2 = this->SurfacePtr(RadSurfNum3);
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                            if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) =
                                    0.0; // Also zero the other side of an interzone
                            }
                        }
                        // Produce a warning message so that user knows the system was shut-off due to potential for condensation
                        if (!state.dataGlobal->WarmupFlag) {
                            if (this->CondErrIndex == 0) { // allow errors up to number of radiant systems
                                ShowWarningMessage(state, format("{} [{}]", cConstantFlowSystem, this->Name));
                                ShowContinueError(state,
                                                  format("Surface [{}] temperature below dew-point temperature--potential for condensation exists",
                                                         Surface(this->SurfacePtr(RadSurfNum2)).Name));
                                ShowContinueError(state, "Flow to the radiant system will be shut-off to avoid condensation");
                                ShowContinueError(state,
                                                  format("Predicted radiant system surface temperature = {:.2R}",
                                                         state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2))));
                                ShowContinueError(state,
                                                  format("Zone dew-point temperature + safety delta T= {:.2R}",
                                                         DewPointTemp + ConstantFlowDesignDataObject.CondDewPtDeltaT));
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state,
                                                  format("Note that a {:.4R} C safety was chosen in the input for the shut-off criteria",
                                                         ConstantFlowDesignDataObject.CondDewPtDeltaT));
                                ShowContinueError(state, "Note also that this affects all surfaces that are part of this radiant system");
                            }
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("{} [{}] condensation shut-off occurrence continues.", cConstantFlowSystem, this->Name),
                                this->CondErrIndex,
                                DewPointTemp,
                                DewPointTemp,
                                _,
                                "C",
                                "C");
                        }
                        break; // outer do loop
                    }
                }

            } else if ((this->opMode == OpMode::Cool) && (ConstantFlowDesignDataObject.condCtrlType == CondCtrlType::None)) {

                for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                    if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) < DewPointTemp) {
                        // Condensation occurring but user does not want to shut radiant system off ever
                        this->CondCausedShutDown = true;
                    }
                }

            } else if ((this->opMode == OpMode::Cool) && (ConstantFlowDesignDataObject.condCtrlType == CondCtrlType::VariedOff)) {

                for (int RadSurfNum2 = 1; RadSurfNum2 <= this->NumOfSurfaces; ++RadSurfNum2) {
                    if (state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2)) <
                        (DewPointTemp + ConstantFlowDesignDataObject.CondDewPtDeltaT)) {
                        state.dataLowTempRadSys->VarOffCond = true;
                        if (state.dataLowTempRadSys->CFloCondIterNum >= 2) {
                            // We have already iterated once so now we must shut off radiant system
                            this->CondCausedShutDown = true;
                            WaterMassFlow = 0.0;
                            this->opMode = OpMode::None;
                            SetComponentFlowRate(state, WaterMassFlow, this->ColdWaterInNode, this->ColdWaterOutNode, this->CWPlantLoc);
                            this->WaterMassFlowRate = WaterMassFlow;
                            for (int RadSurfNum3 = 1; RadSurfNum3 <= this->NumOfSurfaces; ++RadSurfNum3) {
                                int SurfNum2 = this->SurfacePtr(RadSurfNum3);
                                state.dataHeatBalFanSys->QRadSysSource(SurfNum2) = 0.0;
                                if (Surface(SurfNum2).ExtBoundCond > 0 && Surface(SurfNum2).ExtBoundCond != SurfNum2) {
                                    state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum2).ExtBoundCond) =
                                        0.0; // Also zero the other side of an interzone
                                }
                            }
                            // Produce a warning message so that user knows the system was shut-off due to potential for condensation
                            if (!state.dataGlobal->WarmupFlag) {
                                if (this->CondErrIndex == 0) { // allow errors up to number of radiant systems
                                    ShowWarningMessage(state, format("{} [{}]", cConstantFlowSystem, this->Name));
                                    ShowContinueError(
                                        state,
                                        format("Surface [{}] temperature below dew-point temperature--potential for condensation exists",
                                               Surface(this->SurfacePtr(RadSurfNum2)).Name));
                                    ShowContinueError(state, "Flow to the radiant system will be shut-off to avoid condensation");
                                    ShowContinueError(state,
                                                      format("Predicted radiant system surface temperature = {:.2R}",
                                                             state.dataHeatBalSurf->SurfInsideTempHist(1)(this->SurfacePtr(RadSurfNum2))));
                                    ShowContinueError(state,
                                                      format("Zone dew-point temperature + safety delta T= {:.2R}",
                                                             DewPointTemp + ConstantFlowDesignDataObject.CondDewPtDeltaT));
                                    ShowContinueErrorTimeStamp(state, "");
                                    ShowContinueError(state,
                                                      format("Note that a {:.4R} C safety was chosen in the input for the shut-off criteria",
                                                             ConstantFlowDesignDataObject.CondDewPtDeltaT));
                                    ShowContinueError(state, "Note also that this affects all surfaces that are part of this radiant system");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    format("{} [{}] condensation shut-off occurrence continues.", cConstantFlowSystem, this->Name),
                                    this->CondErrIndex,
                                    DewPointTemp,
                                    DewPointTemp,
                                    _,
                                    "C",
                                    "C");
                            }
                            break; // outer do loop
                        } else {   // (First iteration--reset loop required temperature and try again to avoid condensation)
                            state.dataLowTempRadSys->LoopReqTemp = DewPointTemp + ConstantFlowDesignDataObject.CondDewPtDeltaT;
                        }
                    }
                }
            }

            // Determine radiant system outlet temperature (two ways to calculate--use as a check)
            WaterOutletTempCheck = 0.0;
            TotalRadSysPower = 0.0;
            for (int RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                int SurfNum = this->SurfacePtr(RadSurfNum);
                TotalRadSysPower += state.dataHeatBalFanSys->QRadSysSource(SurfNum);
                WaterOutletTempCheck += (this->SurfaceFrac(RadSurfNum) * state.dataLowTempRadSys->WaterTempOut(RadSurfNum));
            }
            TotalRadSysPower *= ZoneMult;

            if (this->WaterMassFlowRate > 0.0) {
                Cp = this->water->getSpecificHeat(state, WaterTempIn, RoutineName);
                this->WaterOutletTemp = this->WaterInletTemp - (TotalRadSysPower / (this->WaterMassFlowRate * Cp));
                if ((std::abs(this->WaterOutletTemp - WaterOutletTempCheck) > TempCheckLimit) && (std::abs(TotalRadSysPower) > ZeroSystemResp)) {
                    // If the total system power is zero, that means we have shut down and the temperatures won't match because of that
                    ShowWarningError(state, "Radiant system water outlet temperature calculation mismatch--this should not happen");
                }
            } else {
                this->WaterOutletTemp = this->WaterInletTemp;
            }
        }

        // Now that we have the source/sink term(s), we must redo the heat balances to obtain
        // the new SumHATsurf value for the zone.  Note that the difference between the new
        // SumHATsurf and the value originally calculated by the heat balance with a zero
        // source for all radiant systems in the zone is the load met by the system (approximately).
        HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
        HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

        LoadMet = state.dataHeatBal->Zone(this->ZonePtr).sumHATsurf(state) - this->ZeroLTRSourceSumHATsurf;
    }
    // TODO Write unit tests for baseboard
    void ConstantFlowRadiantSystemData::calculateRunningMeanAverageTemperature(EnergyPlusData &state, int RadSysNum)
    {
        // This routine grabs the current weather data since it is currently available at this point in the simulation.  Note, however,
        // that the formula that calculates the running mean average (dry-bulb) temperature uses the values from "yesterday".  So, today's
        // values are calculated and then shifted at the beginning of the next day to the tomorrow variables.  It is these tomorrow variables
        // that are then used in the formula.  So, that is why some of the assignments are done in the order that they are in below.

        ConstantFlowRadDesignData constantFlowDesignDataObject{state.dataLowTempRadSys->CflowRadiantSysDesign(
            state.dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr)}; // Contains the data for constant flow hydronic systems

        if (state.dataGlobal->DayOfSim == 1 && state.dataGlobal->WarmupFlag) {
            // there is no "history" here--assume everything that came before was the same (this applies to design days also--weather is always the
            // same
            this->todayAverageOutdoorDryBulbTemperature = this->calculateCurrentDailyAverageODB(state);
            this->yesterdayAverageOutdoorDryBulbTemperature = this->todayAverageOutdoorDryBulbTemperature;
            this->todayRunningMeanOutdoorDryBulbTemperature = this->todayAverageOutdoorDryBulbTemperature;
            this->yesterdayRunningMeanOutdoorDryBulbTemperature = this->todayAverageOutdoorDryBulbTemperature;
        } else if (!state.dataGlobal->WarmupFlag && state.dataGlobal->NumOfDayInEnvrn > 1) {
            // This is an environment with more than one day (non-design day) so...
            // First update yesterday's information using what was previously calculated for "today"
            this->yesterdayAverageOutdoorDryBulbTemperature = this->todayAverageOutdoorDryBulbTemperature;
            this->yesterdayRunningMeanOutdoorDryBulbTemperature = this->todayRunningMeanOutdoorDryBulbTemperature;
            // Now update the running mean and average outdoor air temperatures
            this->todayRunningMeanOutdoorDryBulbTemperature =
                (1.0 - constantFlowDesignDataObject.runningMeanOutdoorAirTemperatureWeightingFactor) *
                    this->yesterdayAverageOutdoorDryBulbTemperature +
                constantFlowDesignDataObject.runningMeanOutdoorAirTemperatureWeightingFactor * this->yesterdayRunningMeanOutdoorDryBulbTemperature;
            this->todayAverageOutdoorDryBulbTemperature = this->calculateCurrentDailyAverageODB(state);
        }
    }

    Real64 ConstantFlowRadiantSystemData::calculateCurrentDailyAverageODB(EnergyPlusData &state)
    {
        Real64 sum = 0.0;
        for (int hourNumber = 1; hourNumber <= Constant::iHoursInDay; ++hourNumber) {
            for (int timeStepNumber = 1; timeStepNumber <= state.dataGlobal->TimeStepsInHour; ++timeStepNumber) {
                sum += state.dataWeather->wvarsHrTsToday(timeStepNumber, hourNumber).OutDryBulbTemp;
            }
        }
        return sum / (Constant::rHoursInDay * state.dataGlobal->TimeStepsInHour);
    }

    void ElectricRadiantSystemData::calculateLowTemperatureRadiantSystem(EnergyPlusData &state,
                                                                         Real64 &LoadMet) // load met by the radiant system, in Watts
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000
        //       MODIFIED       Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine does all of the stuff that is necessary to simulate
        // a low temperature electric radiant heating system.  Calls are made to
        // appropriate subroutines either in this module or outside of it.

        // METHODOLOGY EMPLOYED:
        // Follows the methods used by many other pieces of zone equipment except
        // that we are controlling the electrical input to the building element's
        // resistance heating wires.  Note that cooling is not allowed for such
        // a system.

        // REFERENCES:
        // Other EnergyPlus modules
        // IBLAST-QTF research program, completed in January 1995 (unreleased)
        // Strand, R.K. 1995. "Heat Source Transfer Functions and Their Application to
        //   Low Temperature Radiant Heating Systems", Ph.D. dissertation, University
        //   of Illinois at Urbana-Champaign, Department of Mechanical and Industrial
        //   Engineering.
        // Seem, J.E. 1986. "Heat Transfer in Buildings", Ph.D. dissertation, University
        //   of Wisconsin-Madison.

        // Using/Aliasing
        using DataHeatBalance::ZoneData;
        using HVAC::SmallLoad;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ControlTemp; // Temperature of the parameter that is controlling the radiant system
        Real64 HeatFrac;    // fraction of maximum electrical heat input to radiant system [dimensionless]
        Real64 OffTemp;     // Temperature above which the radiant system should be completely off [C]
        int RadSurfNum;     // number of surface that is the radiant system
        int SurfNum;        // intermediate variable for surface number in Surface derived type
        int ZoneNum;        // number of zone being served

        // initialize local variables
        ZoneNum = this->ZonePtr;
        HeatFrac = 0.0;
        auto &Surface = state.dataSurface->Surface;

        if (this->availSched->getCurrentVal() <= 0.0) {

            // Unit is off; set the heat source terms to zero
            for (RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                SurfNum = this->SurfacePtr(RadSurfNum);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = 0.0;
                if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                    state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                }
            }

        } else { // Unit might be on-->this section is intended to determine whether the controls say
            // that the unit should be on or not

            // Determine the current setpoint temperature and the temperature at which the unit should be completely off
            OffTemp = this->setOffTemperatureLowTemperatureRadiantSystem(state, this->setptSched, this->ThrottlRange, this->setpointType);

            // Determine the control temperature--what the setpoint/offtemp is being compared to for unit operation

            ControlTemp = this->setRadiantSystemControlTemperature(state, controlType);

            if (ControlTemp < OffTemp) { // HEATING MODE

                this->opMode = OpMode::Heat;

                HeatFrac = this->calculateOperationalFraction(OffTemp, ControlTemp, this->ThrottlRange);

                // Set the heat source for the low temperature electric radiant system
                for (RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                    SurfNum = this->SurfacePtr(RadSurfNum);
                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) = HeatFrac * this->MaxElecPower * this->SurfaceFrac(RadSurfNum);
                    if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                        state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) =
                            state.dataHeatBalFanSys->QRadSysSource(SurfNum); // Also set the other side of an interzone
                    }
                }

                // Now "simulate" the system by recalculating the heat balances
                HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);
                HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

                LoadMet = state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - this->ZeroLTRSourceSumHATsurf;

            } else { //  OFF or COOLING MODE (not allowed for an electric low temperature radiant system), turn it off

                for (RadSurfNum = 1; RadSurfNum <= this->NumOfSurfaces; ++RadSurfNum) {
                    SurfNum = this->SurfacePtr(RadSurfNum);
                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) = 0.0;
                    if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                        state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) = 0.0; // Also zero the other side of an interzone
                    }
                }
            }
        }
    }

    void RadiantSystemBaseData::updateLowTemperatureRadiantSystemSurfaces(EnergyPlusData &state)
    {

        // The purpose of this routine is to update the average heat source/sink for a particular system over the various system time
        // steps that make up the zone time step.  For hydronic systems, this routine must also set the outlet water conditions.
        // For the source/sink average update, if the system time step elapsed is still what it used to be, then either we are still
        // iterating orwe had to go back and shorten the time step.  As a result, we have to subtract out the previous value that we
        // added.  If the system time step elapsed is different, then we just need to add the new values to the running average.

        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepZone = state.dataGlobal->TimeStepZone;

        for (int radSurfNum = 1; radSurfNum <= this->NumOfSurfaces; ++radSurfNum) {

            int surfNum = this->SurfacePtr(radSurfNum);

            if (this->LastSysTimeElapsed == SysTimeElapsed) {
                // Still iterating or reducing system time step, so subtract old values which were
                // not valid
                this->QRadSysSrcAvg(radSurfNum) -= this->LastQRadSysSrc(radSurfNum) * this->LastTimeStepSys / TimeStepZone;
            }

            // Update the running average and the "last" values with the current values of the appropriate variables
            this->QRadSysSrcAvg(radSurfNum) += state.dataHeatBalFanSys->QRadSysSource(surfNum) * TimeStepSys / TimeStepZone;
            this->LastQRadSysSrc(radSurfNum) = state.dataHeatBalFanSys->QRadSysSource(surfNum);
        }
        this->LastSysTimeElapsed = SysTimeElapsed;
        this->LastTimeStepSys = TimeStepSys;
    }

    void VariableFlowRadiantSystemData::updateLowTemperatureRadiantSystem(EnergyPlusData &state)
    {

        // Using/Aliasing
        using PlantUtilities::SafeCopyPlantNode;
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr std::string_view RoutineName("UpdateVariableFlowSystem");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 cpWater;       // Specific heat of water
        int waterInletNode;   // Node number for the water side inlet of the radiant system
        Real64 waterMassFlow; // Flow rate of water in the radiant system
        int waterOutletNode;  // Node number for the water side outlet of the radiant system

        auto &Zone = state.dataHeatBal->Zone;
        auto &Node = state.dataLoopNodes->Node;

        // For a hydronic system, calculate the water side outlet conditions and set the
        // appropriate conditions on the correct HVAC node.

        // First sum up all of the heat sources/sinks associated with this system
        Real64 TotalHeatSource(0.0); // Total heat source or sink for a particular radiant system (sum of all surface source/sinks)
        for (int radSurfNum = 1; radSurfNum <= this->NumOfSurfaces; ++radSurfNum) {
            TotalHeatSource += state.dataHeatBalFanSys->QRadSysSource(this->SurfacePtr(radSurfNum));
        }
        TotalHeatSource *= double(Zone(this->ZonePtr).Multiplier * Zone(this->ZonePtr).ListMultiplier);

        // Update the heating side of things
        if (this->HeatingSystem) {

            waterInletNode = this->HotWaterInNode;
            waterOutletNode = this->HotWaterOutNode;
            waterMassFlow = Node(waterInletNode).MassFlowRate;

            cpWater = state.dataPlnt->PlantLoop(this->HWPlantLoc.loopNum).glycol->getSpecificHeat(state, Node(waterInletNode).Temp, RoutineName);

            if (this->opMode == OpMode::Heat) {
                if ((cpWater > 0.0) && (waterMassFlow > 0.0)) {
                    SafeCopyPlantNode(state, waterInletNode, waterOutletNode);
                    Node(waterOutletNode).Temp = Node(waterInletNode).Temp - TotalHeatSource / waterMassFlow / cpWater;
                } else {
                    SafeCopyPlantNode(state, waterInletNode, waterOutletNode);
                }

            } else { // OpMode::Cool or not on
                SafeCopyPlantNode(state, waterInletNode, waterOutletNode);
            }

            this->checkForOutOfRangeTemperatureResult(state, Node(waterOutletNode).Temp, Node(waterInletNode).Temp);
        }

        if (this->CoolingSystem) {

            waterInletNode = this->ColdWaterInNode;
            waterOutletNode = this->ColdWaterOutNode;
            waterMassFlow = Node(waterInletNode).MassFlowRate;

            cpWater = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getSpecificHeat(state, Node(waterInletNode).Temp, RoutineName);

            if (this->opMode == OpMode::Cool) {
                if ((cpWater > 0.0) && (waterMassFlow > 0.0)) {
                    SafeCopyPlantNode(state, waterInletNode, waterOutletNode);
                    Node(waterOutletNode).Temp = Node(waterInletNode).Temp - TotalHeatSource / waterMassFlow / cpWater;
                } else {
                    SafeCopyPlantNode(state, waterInletNode, waterOutletNode);
                }

            } else { // OpMode::Heat or not on
                SafeCopyPlantNode(state, waterInletNode, waterOutletNode);
            }

            this->checkForOutOfRangeTemperatureResult(state, Node(waterOutletNode).Temp, Node(waterInletNode).Temp);
        }
    }

    void ConstantFlowRadiantSystemData::updateLowTemperatureRadiantSystem(EnergyPlusData &state)
    {

        // Using/Aliasing
        using PlantUtilities::SafeCopyPlantNode;
        using PlantUtilities::SetComponentFlowRate;

        Real64 bypassMassFlow; // Local bypass for a constant flow radiant system (could have recirculation and/or bypass)
        int waterInletNode;    // Node number for the water side inlet of the radiant system
        int waterOutletNode;   // Node number for the water side outlet of the radiant system

        // For a constant flow system, calculate the water side outlet conditions
        // and set the appropriate conditions on the correct HVAC node.  This may
        // require mixing if the main system does not provide all of the flow that
        // the local radiant system circulates.

        // Update the heating side of things
        if (this->HeatingSystem) {

            waterInletNode = this->HotWaterInNode;
            waterOutletNode = this->HotWaterOutNode;
            SafeCopyPlantNode(state, waterInletNode, waterOutletNode);

            if (this->opMode == OpMode::Heat) {

                // Leave the inlet and outlet flow alone (if high enough) and perform a bypass if more flow than needed
                if (state.dataLoopNodes->Node(waterInletNode).MassFlowRate <= this->WaterInjectionRate) {
                    // Note that the water injection rate has already been restricted to the maximum available flow
                    state.dataLoopNodes->Node(waterOutletNode).Temp = this->WaterOutletTemp;
                } else {
                    // Loop is providing more flow than needed so perform a local bypass and
                    // mix the flows to obtain the proper outlet temperature.  In this case,
                    // the mass flow rates on the loop are left alone and the outlet temperature
                    // is calculated from a simple steady-steady, steady-flow energy balance.
                    bypassMassFlow = state.dataLoopNodes->Node(waterInletNode).MassFlowRate - this->WaterInjectionRate;
                    state.dataLoopNodes->Node(waterOutletNode).Temp =
                        ((bypassMassFlow * state.dataLoopNodes->Node(waterInletNode).Temp) + (this->WaterInjectionRate * this->WaterOutletTemp)) /
                        (state.dataLoopNodes->Node(waterOutletNode).MassFlowRate);
                }
            }
            this->checkForOutOfRangeTemperatureResult(
                state, state.dataLoopNodes->Node(waterOutletNode).Temp, state.dataLoopNodes->Node(waterInletNode).Temp);
        }

        if (this->CoolingSystem) {

            waterInletNode = this->ColdWaterInNode;
            waterOutletNode = this->ColdWaterOutNode;
            SafeCopyPlantNode(state, waterInletNode, waterOutletNode);

            if (this->opMode == OpMode::Cool) {

                if (state.dataLoopNodes->Node(waterInletNode).MassFlowRate <= this->WaterInjectionRate) {
                    // Note that the water injection rate has already been restricted to the maximum available flow

                    state.dataLoopNodes->Node(waterOutletNode).Temp = this->WaterOutletTemp;
                } else {
                    // Loop is providing more flow than needed so perform a local bypass and
                    // mix the flows to obtain the proper outlet temperature.  In this case,
                    // the mass flow rates on the loop are left alone and the outlet temperature
                    // is calculated from a simple steady-steady, steady-flow energy balance.
                    bypassMassFlow = state.dataLoopNodes->Node(waterInletNode).MassFlowRate - this->WaterInjectionRate;
                    state.dataLoopNodes->Node(waterOutletNode).Temp =
                        ((bypassMassFlow * state.dataLoopNodes->Node(waterInletNode).Temp) + (this->WaterInjectionRate * this->WaterOutletTemp)) /
                        (state.dataLoopNodes->Node(waterOutletNode).MassFlowRate);
                }

                this->checkForOutOfRangeTemperatureResult(
                    state, state.dataLoopNodes->Node(waterOutletNode).Temp, state.dataLoopNodes->Node(waterInletNode).Temp);
            }
        }
    }

    void ElectricRadiantSystemData::updateLowTemperatureRadiantSystem([[maybe_unused]] EnergyPlusData &state)
    { // Dummy routine: no updates are needed for electric radiant systems
    }

    void HydronicSystemBaseData::checkForOutOfRangeTemperatureResult(EnergyPlusData &state, Real64 const outletTemp, Real64 const inletTemp)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   March 2013

        // PURPOSE OF THIS SUBROUTINE:
        // check for crazy, out of range temperature results for fluid leaving radiant system

        // Using/Aliasing

        Real64 constexpr upperRangeLimit(500.0);  // high error trigger limit for when model is not working
        Real64 constexpr lowerRangeLimit(-300.0); // Low error trigger limit for when model is not working

        if (outletTemp < lowerRangeLimit) {
            state.dataLowTempRadSys->warnTooLow = true;
        }

        if (outletTemp > upperRangeLimit) {
            state.dataLowTempRadSys->warnTooHigh = true;
        }

        if (state.dataLowTempRadSys->warnTooLow || state.dataLowTempRadSys->warnTooHigh) {
            if (state.dataLowTempRadSys->warnTooLow) {
                if (this->OutRangeLoErrorCount == 0) {
                    ShowSevereMessage(state, "UpdateLowTempRadiantSystem: model result for fluid outlet temperature is not physical.");
                    ShowContinueError(state, format("Occurs for radiant system name = {}", this->Name));
                    ShowContinueError(state, format("Calculated radiant system outlet temperature = {:.3R} [C]", outletTemp));
                    ShowContinueError(state, format("Radiant system inlet temperature = {:.3R} [C]", inletTemp));
                    ShowContinueError(
                        state, "A possible cause is that the materials used in the internal source construction are not compatible with the model.");
                }
                ShowRecurringSevereErrorAtEnd(
                    state,
                    "UpdateLowTempRadiantSystem: Detected low out of range outlet temperature result for radiant system name =" + this->Name,
                    this->OutRangeLoErrorCount,
                    outletTemp,
                    outletTemp);
            }

            if (state.dataLowTempRadSys->warnTooHigh) {
                if (this->OutRangeHiErrorCount == 0) {
                    ShowSevereMessage(state, "UpdateLowTempRadiantSystem: model result for fluid outlet temperature is not physical.");
                    ShowContinueError(state, format("Occurs for radiant system name = {}", this->Name));
                    ShowContinueError(state, format("Calculated radiant system outlet temperature = {:.3R} [C]", outletTemp));
                    ShowContinueError(state, format("Radiant system inlet temperature = {:.3R} [C]", inletTemp));
                    ShowContinueError(
                        state, "A possible cause is that the materials used in the internal source construction are not compatible with the model.");
                }
                ShowRecurringSevereErrorAtEnd(
                    state,
                    "UpdateLowTempRadiantSystem: Detected high out of range outlet temperature result radiant system name =" + this->Name,
                    this->OutRangeHiErrorCount,
                    outletTemp,
                    outletTemp);
            }
        }
    }

    Real64 RadiantSystemBaseData::setRadiantSystemControlTemperature(EnergyPlusData &state, CtrlType TempControlType)
    {
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->ZonePtr);
        switch (TempControlType) {
        case CtrlType::MAT:
            return thisZoneHB.MAT;
        case CtrlType::MRT:
            return thisZoneHB.MRT;
        case CtrlType::Operative:
            return 0.5 * (thisZoneHB.MAT + thisZoneHB.MRT);
        case CtrlType::ODB:
            return state.dataHeatBal->Zone(this->ZonePtr).OutDryBulbTemp;
        case CtrlType::OWB:
            return state.dataHeatBal->Zone(this->ZonePtr).OutWetBulbTemp;
        case CtrlType::SurfFaceTemp:
            return state.dataHeatBalSurf->SurfTempIn(this->SurfacePtr(1)); // Grabs the inside face temperature of the first surface in the list
        case CtrlType::SurfIntTemp:
            return state.dataHeatBalSurf->SurfTempUserLoc(
                this->SurfacePtr(1)); // Grabs the temperature inside the slab at the location specified by the user
        case CtrlType::RunningMeanODB:
            return this->todayRunningMeanOutdoorDryBulbTemperature;
        default:
            ShowSevereError(state, format("Illegal control type in low temperature radiant system or it's design object: {}", this->Name));
            ShowFatalError(state, "Preceding condition causes termination.");
            return 0.0; // hush the compiler
        }
    }

    Real64
    RadiantSystemBaseData::calculateOperationalFraction(Real64 const offTemperature, Real64 const controlTemperature, Real64 const throttlingRange)
    {
        Real64 temperatureDifference = std::abs(offTemperature - controlTemperature);
        if (temperatureDifference <= 0.0) {
            return 0.0; // No temperature difference--turn things off (set to zero); technically shouldn't happen
        } else if (throttlingRange < 0.001) {
            return 1.0; // Throttling range is essentially zero and there is a temperature difference--turn it full on
        } else {
            // Temperature difference is non-zero and less than the throttling range--calculate the operation fraction, but limit to a maximum of 1.0
            return min(temperatureDifference / throttlingRange, 1.0);
        }
    }

    Real64 RadiantSystemBaseData::setOffTemperatureLowTemperatureRadiantSystem(EnergyPlusData &state,
                                                                               Sched::Schedule const *sched,
                                                                               const Real64 throttlingRange,
                                                                               SetpointType setpointType)
    {
        Real64 scheduleValue = sched->getCurrentVal();
        switch (setpointType) {
        case SetpointType::HalfFlowPower:
            return scheduleValue + 0.5 * throttlingRange;
        case SetpointType::ZeroFlowPower:
            return scheduleValue;
        default:
            ShowSevereError(state, format("Illegal setpoint type in low temperature radiant system: {}", this->Name));
            ShowFatalError(state, "Preceding condition causes termination.");
            return scheduleValue + 0.5 * throttlingRange; // hush the compiler
        }
    }

    Real64
    HydronicSystemBaseData::calculateHXEffectivenessTerm(EnergyPlusData &state,
                                                         int const SurfNum,          // Surface number for this particular part of the radiant system
                                                         Real64 const Temperature,   // Temperature of water entering the radiant system, in C
                                                         Real64 const WaterMassFlow, // Mass flow rate of water in the radiant system, in kg/s
                                                         Real64 const FlowFraction,  // Mass flow rate fraction for this surface in the radiant system
                                                         Real64 const NumCircs,      // Number of fluid circuits in this surface
                                                         int const DesignObjPtr,     // Design Object Pointer
                                                         SystemType const typeOfRadiantSystem)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   December 2000

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
        // Property data for water shown below as parameters taken from
        //   Incropera and DeWitt, Introduction to Heat Transfer, Table A.6.
        // Heat exchanger information also from Incropera and DeWitt.
        // Code based loosely on code from IBLAST program (research version)

        // Return value
        Real64 calculateHXEffectivenessTerm;

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MaxLaminarRe(2300.0); // Maximum Reynolds number for laminar flow
        int constexpr NumOfPropDivisions(13);
        Real64 constexpr MaxExpPower(50.0); // Maximum power after which EXP argument would be zero for DP variables
        Array1D<Real64> Temps(NumOfPropDivisions,
                              {1.85, 6.85, 11.85, 16.85, 21.85, 26.85, 31.85, 36.85, 41.85, 46.85, 51.85, 56.85, 61.85}); // Temperature, in C
        Array1D<Real64> Mu(NumOfPropDivisions,
                           {0.001652,
                            0.001422,
                            0.001225,
                            0.00108,
                            0.000959,
                            0.000855,
                            0.000769,
                            0.000695,
                            0.000631,
                            0.000577,
                            0.000528,
                            0.000489,
                            0.000453}); // Viscosity, in Ns/m2
        Array1D<Real64> Conductivity(
            NumOfPropDivisions, {0.574, 0.582, 0.590, 0.598, 0.606, 0.613, 0.620, 0.628, 0.634, 0.640, 0.645, 0.650, 0.656}); // Conductivity, in W/mK
        Array1D<Real64> Pr(NumOfPropDivisions,
                           {12.22, 10.26, 8.81, 7.56, 6.62, 5.83, 5.20, 4.62, 4.16, 3.77, 3.42, 3.15, 2.88}); // Prandtl number (dimensionless)
        constexpr std::string_view RoutineName("calculateHXEffectivenessTerm");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Index;
        Real64 InterpFrac;
        Real64 NuD;
        Real64 ReD;
        Real64 NTU;
        Real64 CpWater(0.0);
        Real64 Kactual;
        Real64 MUactual;
        Real64 PRactual;
        Real64 Eff; // HX effectiveness

        FluidToSlabHeatTransferType FluidToSlabHeatTransfer(FluidToSlabHeatTransferType::ConvectionOnly);
        Real64 TubeDiameterInner(0.0); // inside tube diameter for embedded tubing (meters)
        Real64 TubeDiameterOuter(0.0); // outside tube diameter for embedded tubing (meters)

        if (typeOfRadiantSystem == SystemType::Hydronic) {
            VarFlowRadDesignData variableFlowDesignDataObject{
                state.dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjPtr)}; // Contains the data for variable flow hydronic systems
            FluidToSlabHeatTransfer = variableFlowDesignDataObject.FluidToSlabHeatTransfer;
            TubeDiameterInner = variableFlowDesignDataObject.TubeDiameterInner;
            TubeDiameterOuter = variableFlowDesignDataObject.TubeDiameterOuter;
        }
        if (typeOfRadiantSystem == SystemType::ConstantFlow) {
            ConstantFlowRadDesignData constantFlowDesignDataObject{
                state.dataLowTempRadSys->CflowRadiantSysDesign(DesignObjPtr)}; // Contains the data for constant flow hydronic systems
            FluidToSlabHeatTransfer = constantFlowDesignDataObject.FluidToSlabHeatTransfer;
            TubeDiameterInner = constantFlowDesignDataObject.TubeDiameterInner;
            TubeDiameterOuter = constantFlowDesignDataObject.TubeDiameterOuter;
        }

        // First find out where we are in the range of temperatures
        Index = 1;
        while (Index <= NumOfPropDivisions) {
            if (Temperature < Temps(Index)) {
                break; // DO loop
            }
            ++Index;
        }

        // Initialize thermal properties of water
        if (Index == 1) {
            MUactual = Mu(Index);
            Kactual = Conductivity(Index);
            PRactual = Pr(Index);
        } else if (Index > NumOfPropDivisions) {
            Index = NumOfPropDivisions;
            MUactual = Mu(Index);
            Kactual = Conductivity(Index);
            PRactual = Pr(Index);
        } else {
            InterpFrac = (Temperature - Temps(Index - 1)) / (Temps(Index) - Temps(Index - 1));
            MUactual = Mu(Index - 1) + InterpFrac * (Mu(Index) - Mu(Index - 1));
            Kactual = Conductivity(Index - 1) + InterpFrac * (Conductivity(Index) - Conductivity(Index - 1));
            PRactual = Pr(Index - 1) + InterpFrac * (Pr(Index) - Pr(Index - 1));
        }
        // arguments are glycol name, temperature, and concentration
        switch (this->opMode) {
        case OpMode::Heat: {
            CpWater = state.dataPlnt->PlantLoop(this->HWPlantLoc.loopNum).glycol->getSpecificHeat(state, Temperature, RoutineName);
        } break;
        case OpMode::Cool: {
            CpWater = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getSpecificHeat(state, Temperature, RoutineName);
        } break;
        default: {
            assert(false);
        } break;
        }

        // Calculate NTU based on the heat transfer model

        if (FluidToSlabHeatTransfer == FluidToSlabHeatTransferType::ISOStandard) {

            Real64 U = this->calculateUFromISOStandard(state, SurfNum, WaterMassFlow * FlowFraction, typeOfRadiantSystem, DesignObjPtr);

            // Calculate the NTU parameter
            // NTU = UA/[(Mdot*Cp)min]
            // where: U = h (convection coefficient) and h = (k)(Nu)/D
            //        A = Constant::Pi()*D*TubeLength
            NTU = U * Constant::Pi * TubeDiameterOuter * this->TubeLength / (WaterMassFlow * CpWater); // FlowFraction cancels out here
        } else { // (this->FluidToSlabHeatTransfer == FluidToSlabHeatTransferTypes::ConvectionOnly)

            // Calculate the Reynold's number from RE=(4*Mdot)/(Pi*Mu*Diameter)
            ReD = 4.0 * WaterMassFlow * FlowFraction / (Constant::Pi * MUactual * TubeDiameterInner * NumCircs);

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
            NTU = Constant::Pi * Kactual * NuD * this->TubeLength / (WaterMassFlow * CpWater); // FlowFraction cancels out here
        }

        // Calculate Epsilon*MassFlowRate*Cp
        if (NTU > MaxExpPower) {
            Eff = 1.0;
            calculateHXEffectivenessTerm = FlowFraction * WaterMassFlow * CpWater;
        } else {
            Eff = 1.0 - std::exp(-NTU);
            calculateHXEffectivenessTerm = Eff * FlowFraction * WaterMassFlow * CpWater;
        }

        return calculateHXEffectivenessTerm;
    }

    Real64 HydronicSystemBaseData::calculateUFromISOStandard(EnergyPlusData &state,
                                                             int const SurfNum,
                                                             Real64 const WaterMassFlow,
                                                             SystemType typeOfRadiantSystem,
                                                             int const DesignObjPtr // Design Object Pointer
    )
    {
        // Calculates the U-value for a pipe embedded in a radiant system using the information
        // from ISO Standard 11855, Part 2 (2012): "Building environment design — Design, dimensioning,
        // installation and control of embedded radiant heating and cooling systems — Part 2:
        // Determination of the design heating and cooling capacity."  This looks exclusively at the heat transfer
        // between the fluid and the inner side of the pipe and heat conduction through the pipe.  The remainder
        // of the ISO calculation relates to the slab itself which is modeled using transient heat conduction here
        // in EnergyPlus.

        // Return value
        Real64 calculateUFromISOStandard;

        int constructionNumber = state.dataSurface->Surface(SurfNum).Construction;

        Real64 TubeDiameterOuter(0.0);
        Real64 TubeDiameterInner(0.0);
        Real64 TubeConductivity(0.0);

        if (typeOfRadiantSystem == SystemType::Hydronic) {
            VarFlowRadDesignData variableFlowDesignDataObject{
                state.dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjPtr)}; // Contains the data for variable flow hydronic systems
            TubeDiameterOuter = variableFlowDesignDataObject.TubeDiameterOuter;
            TubeDiameterInner = variableFlowDesignDataObject.TubeDiameterInner;
            TubeConductivity = variableFlowDesignDataObject.VarFlowTubeConductivity;
        }
        if (typeOfRadiantSystem == SystemType::ConstantFlow) {
            ConstantFlowRadDesignData constantFlowDesignDataObject{
                state.dataLowTempRadSys->CflowRadiantSysDesign(DesignObjPtr)}; // Contains the data for constant flow hydronic systems
            TubeDiameterOuter = constantFlowDesignDataObject.TubeDiameterOuter;
            TubeDiameterInner = constantFlowDesignDataObject.TubeDiameterInner;
            TubeConductivity = constantFlowDesignDataObject.ConstFlowTubeConductivity;
        }

        // Fluid resistance to heat transfer, assumes turbulent flow (Equation B5, p. 38 of ISO Standard 11855-2)
        Real64 distanceBetweenPipes = 2.0 * state.dataConstruction->Construct(constructionNumber).ThicknessPerpend;
        Real64 ratioDiameterToMassFlowLength = TubeDiameterInner / WaterMassFlow / this->TubeLength;
        Real64 rFluid = 0.125 / Constant::Pi * std::pow(distanceBetweenPipes, 0.13) * std::pow(ratioDiameterToMassFlowLength, 0.87);

        // Resistance to heat transfer (conduction through the piping material, Equation B6, p. 38 of ISO Standard 11855-2)
        Real64 rTube = 0.5 * distanceBetweenPipes * std::log(TubeDiameterOuter / TubeDiameterInner) / Constant::Pi / TubeConductivity;

        calculateUFromISOStandard = 1.0 / (rFluid + rTube);

        return calculateUFromISOStandard;
    }

    void UpdateRadSysSourceValAvg(EnergyPlusData &state,
                                  bool &LowTempRadSysOn) // .TRUE. if the radiant system has run this zone time step
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   November 2000

        // PURPOSE OF THIS SUBROUTINE:
        // To transfer the average value of the heat source/sink over the entire
        // zone time step back to the heat balance routines so that the heat
        // balance algorithms can simulate one last time with the average source
        // to maintain some reasonable amount of continuity and energy balance
        // in the temperature and flux histories.

        // METHODOLOGY EMPLOYED:
        // All of the record keeping for the average term is done in the Update
        // routine so the only other thing that this subroutine does is check to
        // see if the system was even on.  If any average term is non-zero, then
        // one or more of the radiant systems was running.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr CloseEnough(0.01); // Some arbitrarily small value to avoid zeros and numbers that are almost the same

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SurfNum; // DO loop counter for surface index

        LowTempRadSysOn = false;

        // If there are no radiant systems in this input file, just RETURN
        if (state.dataLowTempRadSys->TotalNumOfRadSystems == 0) {
            return;
        }

        // Now check to see if anything is running and transfer information from the "average" variables to
        // the array that will be used within the heat balance.
        state.dataHeatBalFanSys->QRadSysSource = 0.0; // Zero this out first
        for (int numRadSys = 1; numRadSys <= state.dataLowTempRadSys->NumOfHydrLowTempRadSys; ++numRadSys) {
            auto &thisLTR = state.dataLowTempRadSys->HydrRadSys(numRadSys);
            for (int numRadSurf = 1; numRadSurf <= thisLTR.NumOfSurfaces; ++numRadSurf) {
                if (thisLTR.QRadSysSrcAvg(numRadSurf) != 0.0) {
                    LowTempRadSysOn = true;
                }
                SurfNum = thisLTR.SurfacePtr(numRadSurf);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = thisLTR.QRadSysSrcAvg(numRadSurf);
            }
        }
        for (int numRadSys = 1; numRadSys <= state.dataLowTempRadSys->NumOfCFloLowTempRadSys; ++numRadSys) {
            auto &thisLTR = state.dataLowTempRadSys->CFloRadSys(numRadSys);
            for (int numRadSurf = 1; numRadSurf <= thisLTR.NumOfSurfaces; ++numRadSurf) {
                if (thisLTR.QRadSysSrcAvg(numRadSurf) != 0.0) {
                    LowTempRadSysOn = true;
                }
                SurfNum = thisLTR.SurfacePtr(numRadSurf);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = thisLTR.QRadSysSrcAvg(numRadSurf);
            }
        }
        for (int numRadSys = 1; numRadSys <= state.dataLowTempRadSys->NumOfElecLowTempRadSys; ++numRadSys) {
            auto &thisLTR = state.dataLowTempRadSys->ElecRadSys(numRadSys);
            for (int numRadSurf = 1; numRadSurf <= thisLTR.NumOfSurfaces; ++numRadSurf) {
                if (thisLTR.QRadSysSrcAvg(numRadSurf) != 0.0) {
                    LowTempRadSysOn = true;
                }
                SurfNum = thisLTR.SurfacePtr(numRadSurf);
                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = thisLTR.QRadSysSrcAvg(numRadSurf);
            }
        }

        auto const &Surface = state.dataSurface->Surface;

        // For interzone surfaces, QRadSysSource was only updated for the "active" side.  The
        // active side would have a non-zero value at this point.  If the numbers differ, then we have to manually update.
        for (SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (Surface(SurfNum).ExtBoundCond > 0 && Surface(SurfNum).ExtBoundCond != SurfNum) {
                if (std::abs(state.dataHeatBalFanSys->QRadSysSource(SurfNum) -
                             state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond)) > CloseEnough) { // numbers differ
                    if (std::abs(state.dataHeatBalFanSys->QRadSysSource(SurfNum)) >
                        std::abs(state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond))) {
                        state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond) = state.dataHeatBalFanSys->QRadSysSource(SurfNum);
                    } else {
                        state.dataHeatBalFanSys->QRadSysSource(SurfNum) = state.dataHeatBalFanSys->QRadSysSource(Surface(SurfNum).ExtBoundCond);
                    }
                }
            }
        }
    }

    void VariableFlowRadiantSystemData::reportLowTemperatureRadiantSystem([[maybe_unused]] EnergyPlusData &state)
    {

        auto &Zone = state.dataHeatBal->Zone;

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        Real64 totalRadSysPower(0.0); // Total source/sink power for the radiant system (sum of all surfaces of the system)

        for (int radSurfNum = 1; radSurfNum <= this->NumOfSurfaces; ++radSurfNum) {
            totalRadSysPower += state.dataHeatBalFanSys->QRadSysSource(this->SurfacePtr(radSurfNum));
        }

        totalRadSysPower *= double(Zone(this->ZonePtr).Multiplier * Zone(this->ZonePtr).ListMultiplier);

        this->HeatPower = 0.0;
        this->CoolPower = 0.0;

        if (this->opMode == OpMode::Heat) {
            this->WaterInletTemp = state.dataLoopNodes->Node(this->HotWaterInNode).Temp;
            this->WaterOutletTemp = state.dataLoopNodes->Node(this->HotWaterOutNode).Temp;
            this->WaterMassFlowRate = state.dataLoopNodes->Node(this->HotWaterInNode).MassFlowRate;
            this->HeatPower = totalRadSysPower;

        } else if (this->opMode == OpMode::Cool) {
            this->WaterInletTemp = state.dataLoopNodes->Node(this->ColdWaterInNode).Temp;
            this->WaterOutletTemp = state.dataLoopNodes->Node(this->ColdWaterOutNode).Temp;
            this->WaterMassFlowRate = state.dataLoopNodes->Node(this->ColdWaterInNode).MassFlowRate;
            this->CoolPower = -totalRadSysPower;

        } else { // Not Operating: Leave temperatures at previous values
            this->WaterMassFlowRate = 0.0;
            this->WaterOutletTemp = this->WaterInletTemp;
        }

        this->HeatEnergy = this->HeatPower * TimeStepSysSec;
        this->CoolEnergy = this->CoolPower * TimeStepSysSec;

        if (this->CondCausedShutDown) {
            this->CondCausedTimeOff = TimeStepSysSec;
        } else {
            this->CondCausedTimeOff = 0.0;
        }
    }

    void ConstantFlowRadiantSystemData::reportLowTemperatureRadiantSystem(EnergyPlusData &state)
    {

        auto &Zone = state.dataHeatBal->Zone;

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        constexpr std::string_view routineName("ReportConstantFlowSystem");
        Real64 cpFluid;               // Specific heat of the fluid in the radiant system
        Real64 totalRadSysPower(0.0); // Total source/sink power for the radiant system (sum of all surfaces of the system)

        for (int radSurfNum = 1; radSurfNum <= this->NumOfSurfaces; ++radSurfNum) {
            totalRadSysPower += state.dataHeatBalFanSys->QRadSysSource(this->SurfacePtr(radSurfNum));
        }

        totalRadSysPower *= double(Zone(this->ZonePtr).Multiplier * Zone(this->ZonePtr).ListMultiplier);

        this->HeatPower = 0.0;
        this->CoolPower = 0.0;

        // Note that temperatures have already been set as part of the simulation
        // step.  So, they do not need to be calculated here except for the pump
        // inlet temperature which was not calculated elsewhere.  If the system is
        // not operating, leave the temperatures with their previous values but
        // zero out the flow and power quantities (should have already been done
        // in another routine, but just in case...).

        if (this->opMode == OpMode::Heat) {
            cpFluid = state.dataPlnt->PlantLoop(this->HWPlantLoc.loopNum)
                          .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->HotWaterInNode).Temp, routineName);

            this->HeatPower = totalRadSysPower;
            if (this->PumpMassFlowRate > 0.0) {
                this->PumpInletTemp = this->WaterInletTemp - (this->PumpHeattoFluid / (this->PumpMassFlowRate * cpFluid));
            } else {
                this->PumpInletTemp = this->WaterInletTemp;
            }

        } else if (this->opMode == OpMode::Cool) {
            cpFluid = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum)
                          .glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->ColdWaterInNode).Temp, routineName);

            this->CoolPower = -totalRadSysPower;
            this->PumpInletTemp = this->WaterInletTemp - (this->PumpHeattoFluid / (this->PumpMassFlowRate * cpFluid));

        } else { // Not Operating
            this->WaterOutletTemp = this->WaterInletTemp;
            this->PumpInletTemp = this->WaterInletTemp;
            this->WaterMassFlowRate = 0.0;
            this->WaterInjectionRate = 0.0;
            this->WaterRecircRate = 0.0;
            this->HeatPower = 0.0;
            this->CoolPower = 0.0;
            this->PumpPower = 0.0;
            this->PumpMassFlowRate = 0.0;
            this->PumpHeattoFluid = 0.0;
        }

        this->HeatEnergy = this->HeatPower * TimeStepSysSec;
        this->CoolEnergy = this->CoolPower * TimeStepSysSec;
        this->PumpEnergy = this->PumpPower * TimeStepSysSec;
        this->PumpHeattoFluidEnergy = this->PumpHeattoFluid * TimeStepSysSec;

        if (this->CondCausedShutDown) {
            this->CondCausedTimeOff = TimeStepSysSec;
        } else {
            this->CondCausedTimeOff = 0.0;
        }
    }

    void ElectricRadiantSystemData::reportLowTemperatureRadiantSystem([[maybe_unused]] EnergyPlusData &state)
    {

        auto &Zone = state.dataHeatBal->Zone;
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        // Using/Aliasing
        Real64 totalRadSysPower(0.0); // Total source/sink power for the radiant system (sum of all surfaces of the system)

        for (int radSurfNum = 1; radSurfNum <= this->NumOfSurfaces; ++radSurfNum) {
            totalRadSysPower += state.dataHeatBalFanSys->QRadSysSource(this->SurfacePtr(radSurfNum));
        }

        totalRadSysPower *= double(Zone(this->ZonePtr).Multiplier * Zone(this->ZonePtr).ListMultiplier);

        this->ElecPower = totalRadSysPower;
        this->ElecEnergy = this->ElecPower * TimeStepSysSec;
        this->HeatPower = this->ElecPower;
        this->HeatEnergy = this->ElecEnergy;
    }

} // namespace LowTempRadiantSystem

} // namespace EnergyPlus
