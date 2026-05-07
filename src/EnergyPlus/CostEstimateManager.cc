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
#include <algorithm>
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CostEstimateManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
// #include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataPhotovoltaics.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/PlantChillers.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace CostEstimateManager {

    // Module containing the routines dealing with the Cost Estimation capability of EnergyPlus

    // MODULE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   April-May 2004
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // produce a construction cost estimate report based on
    // input and certain building calculations by EnergyPlus

    // METHODOLOGY EMPLOYED:
    // Routine gets called once, Just before tabular reports.
    // Cost Estimate objects are child objects that will inherit from
    // other input objects.
    // Uses a Line Item metaphor where each Cost Estimate object is a line
    // Create report using utility subroutines taken from OutputReportTabular (by J.Glazer)

    // Using/Aliasing

    constexpr std::array<std::string_view, static_cast<int>(ParentObject::Num)> ParentObjectNamesUC{"GENERAL",
                                                                                                    "CONSTRUCTION",
                                                                                                    "COIL:DX",
                                                                                                    "COIL:COOLING:DX",
                                                                                                    "COIL:COOLING:DX:SINGLESPEED",
                                                                                                    "COIL:HEATING:FUEL",
                                                                                                    "CHILLER:ELECTRIC",
                                                                                                    "DAYLIGHTING:CONTROLS",
                                                                                                    "SHADING:ZONE:DETAILED",
                                                                                                    "LIGHTS",
                                                                                                    "GENERATOR:PHOTOVOLTAIC"};

    void SimCostEstimate(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         BGriffith
        //       DATE WRITTEN   April 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Entry point; manage calls to other subroutines

        if (state.dataCostEstimateManager->GetCostInput) {
            GetCostEstimateInput(state);
            state.dataCostEstimateManager->GetCostInput = false;
        }

        // Need to add check Costs before this will work properly

        if (state.dataGlobal->KickOffSimulation) {
            return;
        }

        if (state.dataCostEstimateManager->DoCostEstimate) {
            CalcCostEstimate(state);
        }
    }

    void GetCostEstimateInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         BGriffith
        //       DATE WRITTEN   April 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Get Cost Estimation object input.

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item; // Item to be "gotten"
        int NumCostAdjust;
        int NumRefAdjust;
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine

        int NumLineItems = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ComponentCost:LineItem");

        if (NumLineItems == 0) {
            state.dataCostEstimateManager->DoCostEstimate = false;
            return;
        }
        state.dataCostEstimateManager->DoCostEstimate = true;
        //    WriteTabularFiles = .TRUE.

        if (!allocated(state.dataCostEstimateManager->CostLineItem)) {
            state.dataCostEstimateManager->CostLineItem.allocate(NumLineItems);
        }
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "ComponentCost:LineItem";

        for (Item = 1; Item <= NumLineItems; ++Item) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Item,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus);
            state.dataCostEstimateManager->CostLineItem(Item).LineName = state.dataIPShortCut->cAlphaArgs(1);
            state.dataCostEstimateManager->CostLineItem(Item).ParentObjType =
                static_cast<ParentObject>(getEnumValue(ParentObjectNamesUC, state.dataIPShortCut->cAlphaArgs(3)));
            state.dataCostEstimateManager->CostLineItem(Item).ParentObjName = state.dataIPShortCut->cAlphaArgs(4);
            state.dataCostEstimateManager->CostLineItem(Item).PerEach = state.dataIPShortCut->rNumericArgs(1);
            state.dataCostEstimateManager->CostLineItem(Item).PerSquareMeter = state.dataIPShortCut->rNumericArgs(2);
            state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap = state.dataIPShortCut->rNumericArgs(3);
            state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP = state.dataIPShortCut->rNumericArgs(4);
            state.dataCostEstimateManager->CostLineItem(Item).PerCubicMeter = state.dataIPShortCut->rNumericArgs(5);
            state.dataCostEstimateManager->CostLineItem(Item).PerCubMeterPerSec = state.dataIPShortCut->rNumericArgs(6);
            state.dataCostEstimateManager->CostLineItem(Item).PerUAinWattperDelK = state.dataIPShortCut->rNumericArgs(7);
            state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataIPShortCut->rNumericArgs(8);
        }

        // most input error checking to be performed later within Case construct in Calc routine.

        cCurrentModuleObject = "ComponentCost:Adjustments";
        NumCostAdjust = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (NumCostAdjust == 1) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     1,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus);
            state.dataCostEstimateManager->CurntBldg.MiscCostperSqMeter = state.dataIPShortCut->rNumericArgs(1);
            state.dataCostEstimateManager->CurntBldg.DesignFeeFrac = state.dataIPShortCut->rNumericArgs(2);
            state.dataCostEstimateManager->CurntBldg.ContractorFeeFrac = state.dataIPShortCut->rNumericArgs(3);
            state.dataCostEstimateManager->CurntBldg.ContingencyFrac = state.dataIPShortCut->rNumericArgs(4);
            state.dataCostEstimateManager->CurntBldg.BondCostFrac = state.dataIPShortCut->rNumericArgs(5);
            state.dataCostEstimateManager->CurntBldg.CommissioningFrac = state.dataIPShortCut->rNumericArgs(6);
            state.dataCostEstimateManager->CurntBldg.RegionalModifier = state.dataIPShortCut->rNumericArgs(7);

        } else if (NumCostAdjust > 1) {
            ShowSevereError(state, EnergyPlus::format("{}: Only one instance of this object is allowed.", cCurrentModuleObject));
            ErrorsFound = true;
        }

        cCurrentModuleObject = "ComponentCost:Reference";
        NumRefAdjust = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (NumRefAdjust == 1) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     1,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus);
            state.dataCostEstimateManager->RefrncBldg.LineItemTot = state.dataIPShortCut->rNumericArgs(1);
            state.dataCostEstimateManager->RefrncBldg.MiscCostperSqMeter = state.dataIPShortCut->rNumericArgs(2);
            state.dataCostEstimateManager->RefrncBldg.DesignFeeFrac = state.dataIPShortCut->rNumericArgs(3);
            state.dataCostEstimateManager->RefrncBldg.ContractorFeeFrac = state.dataIPShortCut->rNumericArgs(4);
            state.dataCostEstimateManager->RefrncBldg.ContingencyFrac = state.dataIPShortCut->rNumericArgs(5);
            state.dataCostEstimateManager->RefrncBldg.BondCostFrac = state.dataIPShortCut->rNumericArgs(6);
            state.dataCostEstimateManager->RefrncBldg.CommissioningFrac = state.dataIPShortCut->rNumericArgs(7);
            state.dataCostEstimateManager->RefrncBldg.RegionalModifier = state.dataIPShortCut->rNumericArgs(8);

        } else if (NumRefAdjust > 1) {
            ShowSevereError(state, EnergyPlus::format("{} : Only one instance of this object is allowed.", cCurrentModuleObject));
            ErrorsFound = true;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in processing cost estimate input");
        }

        CheckCostEstimateInput(state, ErrorsFound);

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in processing cost estimate input");
        }
    }

    void CheckCostEstimateInput(EnergyPlusData &state, bool &ErrorsFound) // Set to true if errors in input, fatal at end of routine
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         BGriffith
        //       DATE WRITTEN   April 2004
        //       MODIFIED       February 2005, M. J. Witte
        //                        Add subscript to DX coil variables due to new multimode DX coil
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the Cost Estimate based on inputs.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item;            // do-loop counter for line items
        int ThisConstructID; // hold result of FindItem searching for Construct name
        int ThisSurfID;      // hold result from findItem
        int ThisZoneID;      // hold result from findItem

        std::string ThisConstructStr;
        auto &Zone(state.dataHeatBal->Zone);

        int thisCoil; // index of named coil in its derived type
        int thisChil;
        int thisPV;

        // Setup working data structure for line items
        for (Item = 1; Item <= (int)state.dataCostEstimateManager->CostLineItem.size(); ++Item) { // Loop thru cost line items

            state.dataCostEstimateManager->CostLineItem(Item).LineNumber = Item;

            switch (state.dataCostEstimateManager->CostLineItem(Item).ParentObjType) {
            case ParentObject::General: {
            } break;
            case ParentObject::Construction: {

                // test input for problems
                //  is PerSquareMeter non-zero? if it is are other cost per values set?
                //   issue warning that 'Cost Estimate requested for Constructions with zero cost per unit area
                if (state.dataCostEstimateManager->CostLineItem(Item).PerSquareMeter == 0) {
                    ShowSevereError(
                        state,
                        EnergyPlus::format("ComponentCost:LineItem: \"{}\" Construction object needs non-zero construction costs per square meter",
                                           state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }

                ThisConstructStr = state.dataCostEstimateManager->CostLineItem(Item).ParentObjName;
                ThisConstructID = Util::FindItem(ThisConstructStr, state.dataConstruction->Construct);
                if (ThisConstructID == 0) { // do any surfaces have the specified construction? If not issue warning.
                    ShowWarningError(
                        state,
                        EnergyPlus::format("ComponentCost:LineItem: \"{}\" Construction=\"{}\", no surfaces have the Construction specified",
                                           state.dataCostEstimateManager->CostLineItem(Item).LineName,
                                           state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                    ShowContinueError(state, "No costs will be calculated for this Construction.");
                    //        ErrorsFound = .TRUE.
                    continue;
                }
            } break;
            case ParentObject::CoilDX:
            case ParentObject::CoilCoolingDX:
            case ParentObject::CoilCoolingDXSingleSpeed: {
                // test if too many pricing methods are set in user input
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0)) {
                    ShowSevereError(
                        state,
                        EnergyPlus::format("ComponentCost:LineItem: \"{}\", {}, too many pricing methods specified",
                                           state.dataCostEstimateManager->CostLineItem(Item).LineName,
                                           ParentObjectNamesUC[static_cast<int>(state.dataCostEstimateManager->CostLineItem(Item).ParentObjType)]));
                    ErrorsFound = true;
                }
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0)) {
                    ShowSevereError(
                        state,
                        EnergyPlus::format("ComponentCost:LineItem: \"{}\", {}, too many pricing methods specified",
                                           state.dataCostEstimateManager->CostLineItem(Item).LineName,
                                           ParentObjectNamesUC[static_cast<int>(state.dataCostEstimateManager->CostLineItem(Item).ParentObjType)]));
                    ErrorsFound = true;
                }
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0)) {
                    ShowSevereError(
                        state,
                        EnergyPlus::format("ComponentCost:LineItem: \"{}\", {}, too many pricing methods specified",
                                           state.dataCostEstimateManager->CostLineItem(Item).LineName,
                                           ParentObjectNamesUC[static_cast<int>(state.dataCostEstimateManager->CostLineItem(Item).ParentObjType)]));
                    ErrorsFound = true;
                }
                //  check for wildcard * in object name..
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == "*") { // wildcard, apply to all such components

                } else if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ShowSevereError(
                        state,
                        EnergyPlus::format("ComponentCost:LineItem: \"{}\", {}, too many pricing methods specified",
                                           state.dataCostEstimateManager->CostLineItem(Item).LineName,
                                           ParentObjectNamesUC[static_cast<int>(state.dataCostEstimateManager->CostLineItem(Item).ParentObjType)]));
                    ErrorsFound = true;

                } else { // assume name is probably useful
                    bool coilFound = false;
                    auto &parentObjName = state.dataCostEstimateManager->CostLineItem(Item).ParentObjName;
                    if ((state.dataCostEstimateManager->CostLineItem(Item).ParentObjType == ParentObject::CoilDX) ||
                        (state.dataCostEstimateManager->CostLineItem(Item).ParentObjType == ParentObject::CoilCoolingDXSingleSpeed)) {
                        if (Util::FindItem(parentObjName, state.dataDXCoils->DXCoil) > 0) {
                            coilFound = true;
                        }
                    } else if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjType == ParentObject::CoilCoolingDX) {
                        if (CoilCoolingDX::factory(state, parentObjName) != -1) {
                            coilFound = true;
                        }
                    }
                    if (!coilFound) {
                        ShowWarningError(state,
                                         EnergyPlus::format(
                                             "ComponentCost:LineItem: \"{}\", {}, invalid coil specified",
                                             state.dataCostEstimateManager->CostLineItem(Item).LineName,
                                             ParentObjectNamesUC[static_cast<int>(state.dataCostEstimateManager->CostLineItem(Item).ParentObjType)]));
                        ShowContinueError(state,
                                          EnergyPlus::format("Coil Specified=\"{}\", calculations will not be completed for this item.",
                                                             state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                    }
                }
            } break;
            case ParentObject::CoilHeatingFuel: {
                // test if too many pricing methods are set in user input
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0)) {
                    ShowSevereError(state,
                                    EnergyPlus::format("ComponentCost:LineItem: \"{}\", Coil:Heating:Fuel, too many pricing methods specified",
                                                       state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0)) {
                    ShowSevereError(state,
                                    EnergyPlus::format("ComponentCost:LineItem: \"{}\", Coil:Heating:Fuel, too many pricing methods specified",
                                                       state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0)) {
                    ShowSevereError(state,
                                    EnergyPlus::format("ComponentCost:LineItem: \"{}\", Coil:Heating:Fuel, too many pricing methods specified",
                                                       state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
                //  check for wildcard * in object name..
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == "*") { // wildcard, apply to all such components

                } else if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ShowSevereError(state,
                                    EnergyPlus::format("ComponentCost:LineItem: \"{}\", Coil:Heating:Fuel, need to specify a Reference Object Name",
                                                       state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;

                } else { // assume name is probably useful
                    thisCoil = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataHeatingCoils->HeatingCoil);
                    if (thisCoil == 0) {
                        ShowWarningError(state,
                                         EnergyPlus::format("ComponentCost:LineItem: \"{}\", Coil:Heating:Fuel, invalid coil specified",
                                                            state.dataCostEstimateManager->CostLineItem(Item).LineName));
                        ShowContinueError(state,
                                          EnergyPlus::format("Coil Specified=\"{}\", calculations will not be completed for this item.",
                                                             state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                    }
                }
            } break;
            case ParentObject::ChillerElectric: {
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ShowSevereError(state,
                                    EnergyPlus::format("ComponentCost:LineItem: \"{}\", Chiller:Electric, need to specify a Reference Object Name",
                                                       state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
                thisChil = 0;
                int chillNum = 0;
                for (auto const &ch : state.dataPlantChillers->ElectricChiller) {
                    chillNum++;
                    if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == ch.Name) {
                        thisChil = chillNum;
                    }
                }
                if (thisChil == 0) {
                    ShowWarningError(state,
                                     EnergyPlus::format("ComponentCost:LineItem: \"{}\", Chiller:Electric, invalid chiller specified.",
                                                        state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ShowContinueError(state,
                                      std::format("Chiller Specified=\"{}\", calculations will not be completed for this item.",
                                                  state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                }
            } break;
            case ParentObject::DaylightingControls: {
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == "*") { // wildcard, apply to all such components
                } else if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ShowSevereError(state,
                                    std::format("ComponentCost:LineItem: \"{}\", Daylighting:Controls, need to specify a Reference Object Name",
                                                state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                } else {
                    ThisZoneID = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, Zone);
                    if (ThisZoneID > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataDayltg->ZoneDaylight(ThisZoneID).totRefPts;
                    } else {
                        ShowSevereError(state,
                                        std::format("ComponentCost:LineItem: \"{}\", Daylighting:Controls, need to specify a valid zone name",
                                                    state.dataCostEstimateManager->CostLineItem(Item).LineName));
                        ShowContinueError(state,
                                          std::format("Zone specified=\"{}\".", state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                        ErrorsFound = true;
                    }
                }
            } break;
            case ParentObject::ShadingZoneDetailed: {
                if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ThisSurfID = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataSurface->Surface);
                    if (ThisSurfID > 0) {
                        ThisZoneID = Util::FindItem(state.dataSurface->Surface(ThisSurfID).ZoneName, Zone);
                        if (ThisZoneID == 0) {
                            ShowSevereError(state,
                                            std::format("ComponentCost:LineItem: \"{}\", Shading:Zone:Detailed, need to specify a valid zone name",
                                                        state.dataCostEstimateManager->CostLineItem(Item).LineName));
                            ShowContinueError(state, std::format("Zone specified=\"{}\".", state.dataSurface->Surface(ThisSurfID).ZoneName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state,
                                        std::format("ComponentCost:LineItem: \"{}\", Shading:Zone:Detailed, need to specify a valid surface name",
                                                    state.dataCostEstimateManager->CostLineItem(Item).LineName));
                        ShowContinueError(state,
                                          std::format("Surface specified=\"{}\".", state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state,
                                    std::format("ComponentCost:LineItem: \"{}\", Shading:Zone:Detailed, specify a Reference Object Name",
                                                state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
            } break;
            case ParentObject::Lights: {
                if ((state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) &&
                    (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0)) {
                    ShowSevereError(state,
                                    std::format("ComponentCost:LineItem: \"{}\", Lights, too many pricing methods specified",
                                                state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap != 0.0) {
                    if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                        ThisZoneID = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, Zone);
                        if (ThisZoneID == 0) {
                            ShowSevereError(state,
                                            std::format("ComponentCost:LineItem: \"{}\", Lights, need to specify a valid zone name",
                                                        state.dataCostEstimateManager->CostLineItem(Item).LineName));
                            ShowContinueError(state,
                                              std::format("Zone specified=\"{}\".", state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state,
                                        std::format("ComponentCost:LineItem: \"{}\", Lights, need to specify a Reference Object Name",
                                                    state.dataCostEstimateManager->CostLineItem(Item).LineName));
                        ErrorsFound = true;
                    }
                }
            } break;
            case ParentObject::GeneratorPhotovoltaic: {
                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap != 0.0) {
                    if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                        thisPV = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataPhotovoltaic->PVarray);
                        if (thisPV > 0) {
                            if (state.dataPhotovoltaic->PVarray(thisPV).PVModelType != DataPhotovoltaics::PVModel::Simple) {
                                ShowSevereError(state,
                                                std::format("ComponentCost:LineItem: \"{}\", Generator:Photovoltaic, only available for model type "
                                                            "PhotovoltaicPerformance:Simple",
                                                            state.dataCostEstimateManager->CostLineItem(Item).LineName));
                                ErrorsFound = true;
                            }
                        } else {
                            ShowSevereError(state,
                                            std::format("ComponentCost:LineItem: \"{}\", Generator:Photovoltaic, need to specify a valid PV array",
                                                        state.dataCostEstimateManager->CostLineItem(Item).LineName));
                            ShowContinueError(
                                state, std::format("PV Array specified=\"{}\".", state.dataCostEstimateManager->CostLineItem(Item).ParentObjName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state,
                                        std::format("ComponentCost:LineItem: \"{}\", Generator:Photovoltaic, need to specify a Reference Object Name",
                                                    state.dataCostEstimateManager->CostLineItem(Item).LineName));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state,
                                    std::format("ComponentCost:LineItem: \"{}\", Generator:Photovoltaic, need to specify a per-kilowatt cost ",
                                                state.dataCostEstimateManager->CostLineItem(Item).LineName));
                    ErrorsFound = true;
                }
            } break;
            default: {
                ShowWarningError(state,
                                 std::format("ComponentCost:LineItem: \"{}\", invalid cost item -- not included in cost estimate.",
                                             state.dataCostEstimateManager->CostLineItem(Item).LineName));
                ShowContinueError(
                    state,
                    std::format(
                        "... invalid object type={}",
                        EnergyPlus::format(ParentObjectNamesUC[static_cast<int>(state.dataCostEstimateManager->CostLineItem(Item).ParentObjType)])));
            } break;
            }
        }
    }

    void CalcCostEstimate(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         BGriffith
        //       DATE WRITTEN   April 2004
        //       MODIFIED       February 2005, M. J. Witte
        //                        Add subscript to DX coil variables due to new multimode DX coil
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the Cost Estimate based on inputs.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item;            // do-loop counter for line items
        int ThisConstructID; // hold result of FindItem searching for Construct name
        int ThisSurfID;      // hold result from findItem
        int ThisZoneID;      // hold result from findItem

        auto &Zone(state.dataHeatBal->Zone);
        std::string ThisConstructStr;

        Array1D_bool uniqueSurfMask;
        Array1D<Real64> SurfMultipleARR;
        int surf;     // do-loop counter for checking for surfaces for uniqueness
        int thisCoil; // index of named coil in its derived type
        bool WildcardObjNames;
        int thisChil;
        int thisPV;
        Real64 Multipliers;

        // Setup working data structure for line items
        for (Item = 1; Item <= (int)state.dataCostEstimateManager->CostLineItem.size(); ++Item) { // Loop thru cost line items

            state.dataCostEstimateManager->CostLineItem(Item).LineNumber = Item;

            switch (state.dataCostEstimateManager->CostLineItem(Item).ParentObjType) {
            case ParentObject::General: {
                state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                    state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
            } break;
            case ParentObject::Construction: {
                ThisConstructStr = state.dataCostEstimateManager->CostLineItem(Item).ParentObjName;
                ThisConstructID = Util::FindItem(ThisConstructStr, state.dataConstruction->Construct);
                // need to determine unique surfaces... some surfaces are shared by zones and hence doubled
                uniqueSurfMask.dimension(state.dataSurface->TotSurfaces, true); // init to true and change duplicates to false
                SurfMultipleARR.dimension(state.dataSurface->TotSurfaces, 1.0);
                for (surf = 1; surf <= state.dataSurface->TotSurfaces; ++surf) {
                    if (state.dataSurface->Surface(surf).ExtBoundCond >= 1) {
                        if (state.dataSurface->Surface(surf).ExtBoundCond < surf) { // already cycled through
                            uniqueSurfMask(surf) = false;
                        }
                    }
                    if (state.dataSurface->Surface(surf).Construction == 0) { // throw out others for now
                        uniqueSurfMask(surf) = false;
                    }
                    if (state.dataSurface->Surface(surf).Zone > 0) {
                        SurfMultipleARR(surf) =
                            Zone(state.dataSurface->Surface(surf).Zone).Multiplier * Zone(state.dataSurface->Surface(surf).Zone).ListMultiplier;
                    }
                }
                // determine which surfaces have the construction type  and if any are duplicates..
                Real64 Qty(0.0);
                for (int i = 1; i <= state.dataSurface->TotSurfaces; ++i) {
                    auto const &s(state.dataSurface->Surface(i));
                    if (uniqueSurfMask(i) && (s.Construction == ThisConstructID)) {
                        Qty += s.Area * SurfMultipleARR(i);
                    }
                }
                state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty;
                state.dataCostEstimateManager->CostLineItem(Item).Units = "m2";
                state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerSquareMeter;
                state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                    state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;

                uniqueSurfMask.deallocate();
                SurfMultipleARR.deallocate();
            } break;
            case ParentObject::CoilDX:
            case ParentObject::CoilCoolingDXSingleSpeed: {
                WildcardObjNames = false;
                thisCoil = 0;
                //  check for wildcard * in object name..
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == "*") { // wildcard, apply to all such components
                    WildcardObjNames = true;
                } else if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    thisCoil = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataDXCoils->DXCoil);
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) {
                    if (WildcardObjNames) {
                        Real64 Qty(0.0);
                        for (auto const &e : state.dataDXCoils->DXCoil) {
                            Qty += e.RatedTotCap(1);
                        }
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot cool cap.)";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                    if (thisCoil > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataDXCoils->DXCoil(thisCoil).RatedTotCap(1) / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot cool cap.)";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0) {
                    if (WildcardObjNames) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = double(state.dataDXCoils->NumDXCoils);
                    }
                    if (thisCoil > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = 1.0;
                    }
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0) {
                    if (WildcardObjNames) {
                        Real64 Qty(0.0);
                        for (auto const &e : state.dataDXCoils->DXCoil) {
                            int maxSpeed = e.RatedCOP.size();
                            Qty += e.RatedCOP(maxSpeed) * e.RatedTotCap(maxSpeed);
                        }
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*COP (total, rated) ";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                    if (thisCoil > 0) {
                        int maxSpeed = state.dataDXCoils->DXCoil(thisCoil).RatedCOP.size();
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataDXCoils->DXCoil(thisCoil).RatedCOP(maxSpeed) *
                                                                                state.dataDXCoils->DXCoil(thisCoil).RatedTotCap(maxSpeed) / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*COP (total, rated) ";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                }
            } break;
            case ParentObject::CoilCoolingDX: {
                WildcardObjNames = false;
                auto &parentObjName = state.dataCostEstimateManager->CostLineItem(Item).ParentObjName;
                bool coilFound = false;
                //  check for wildcard * in object name..
                if (parentObjName == "*") { // wildcard, apply to all such components
                    WildcardObjNames = true;
                } else if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    // Purposefully not calling the factory here
                    // Input validation happens before we get to this point
                    // The factory throws a severe error when the coil is not found
                    // Finding the coil like this here to protects against another SevereError being thrown out of context
                    auto &v = state.dataCoilCoolingDX->coilCoolingDXs;
                    auto isInCoils = [&parentObjName](const CoilCoolingDX &coil) { return coil.name == parentObjName; };
                    auto it = std::find_if(v.begin(), v.end(), isInCoils);
                    if (it != v.end()) {
                        thisCoil = std::distance(v.begin(), it);
                        coilFound = true;
                    }
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) {
                    if (WildcardObjNames) {
                        Real64 Qty(0.0);
                        for (auto const &e : state.dataCoilCoolingDX->coilCoolingDXs) {
                            Qty += e.performance->ratedGrossTotalCap();
                        }
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot cool cap.)";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                    if (coilFound) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty =
                            state.dataCoilCoolingDX->coilCoolingDXs[thisCoil].performance->ratedGrossTotalCap() / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot cool cap.)";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0) {
                    if (WildcardObjNames) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = double(state.dataCoilCoolingDX->coilCoolingDXs.size());
                    }
                    if (coilFound) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = 1.0;
                    }
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0) {
                    if (WildcardObjNames) {
                        Real64 Qty(0.0);
                        for (auto const &e : state.dataCoilCoolingDX->coilCoolingDXs) {
                            Real64 COP = e.performance->grossRatedCoolingCOPAtMaxSpeed(state);
                            Qty += COP * e.performance->ratedGrossTotalCap();
                        }
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*COP (total, rated) ";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                    if (coilFound) {
                        Real64 COP = state.dataCoilCoolingDX->coilCoolingDXs[thisCoil].performance->grossRatedCoolingCOPAtMaxSpeed(state);
                        state.dataCostEstimateManager->CostLineItem(Item).Qty =
                            COP * state.dataCoilCoolingDX->coilCoolingDXs[thisCoil].performance->ratedGrossTotalCap() / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*COP (total, rated) ";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                }
            } break;
            case ParentObject::CoilHeatingFuel: {
                WildcardObjNames = false;
                thisCoil = 0;
                //  check for wildcard * in object name..
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == "*") { // wildcard, apply to all such components
                    WildcardObjNames = true;
                } else if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    thisCoil = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataHeatingCoils->HeatingCoil);
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0) {
                    if (WildcardObjNames) {
                        Real64 Qty(0.0);
                        for (auto const &e : state.dataHeatingCoils->HeatingCoil) {
                            if (e.coilType == HVAC::CoilType::HeatingDXSingleSpeed) {
                                Qty += e.NominalCapacity;
                            }
                        }
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot heat cap.)";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                    if (thisCoil > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty =
                            state.dataHeatingCoils->HeatingCoil(thisCoil).NominalCapacity / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot heat cap.)";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0) {
                    if (WildcardObjNames) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataHeatingCoils->NumHeatingCoils;
                    }
                    if (thisCoil > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = 1.0;
                    }
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0) {
                    if (WildcardObjNames) {
                        Real64 Qty(0.0);
                        for (auto const &e : state.dataHeatingCoils->HeatingCoil) {
                            if (e.coilType == HVAC::CoilType::HeatingDXSingleSpeed) {
                                Qty += e.Efficiency * e.NominalCapacity;
                            }
                        }
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = Qty / 1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*Eff (total, rated) ";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                    if (thisCoil > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataHeatingCoils->HeatingCoil(thisCoil).Efficiency *
                                                                                state.dataHeatingCoils->HeatingCoil(thisCoil).NominalCapacity /
                                                                                1000.0;
                        state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*Eff (total, rated) ";
                        state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                        state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                            state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                    }
                }
            } break;
            case ParentObject::ChillerElectric: {
                thisChil = 0;
                int chillNum = 0;
                for (auto const &ch : state.dataPlantChillers->ElectricChiller) {
                    chillNum++;
                    if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == ch.Name) {
                        thisChil = chillNum;
                    }
                }
                if ((thisChil > 0) && (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap > 0.0)) {
                    state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataPlantChillers->ElectricChiller(thisChil).NomCap / 1000.0;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (tot cool cap.)";
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                }
                if ((thisChil > 0) && (state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP > 0.0)) {
                    state.dataCostEstimateManager->CostLineItem(Item).Qty =
                        state.dataPlantChillers->ElectricChiller(thisChil).COP * state.dataPlantChillers->ElectricChiller(thisChil).NomCap / 1000.0;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "kW*COP (total, rated) ";
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerKWCapPerCOP;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                }
                if ((thisChil > 0) && (state.dataCostEstimateManager->CostLineItem(Item).PerEach > 0.0)) {
                    state.dataCostEstimateManager->CostLineItem(Item).Qty = 1.0;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                }
            } break;
            case ParentObject::DaylightingControls: {
                if (state.dataCostEstimateManager->CostLineItem(Item).ParentObjName == "*") { // wildcard, apply to all such components
                    state.dataCostEstimateManager->CostLineItem(Item).Qty = sum(state.dataDayltg->ZoneDaylight, &Dayltg::ZoneDaylightCalc::totRefPts);
                } else if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ThisZoneID = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, Zone);
                    if (ThisZoneID > 0) {
                        state.dataCostEstimateManager->CostLineItem(Item).Qty = state.dataDayltg->ZoneDaylight(ThisZoneID).totRefPts;
                    }
                }

                state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                    state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
            } break;
            case ParentObject::ShadingZoneDetailed: {
                if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                    ThisSurfID = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataSurface->Surface);
                    if (ThisSurfID > 0) {
                        ThisZoneID = Util::FindItem(state.dataSurface->Surface(ThisSurfID).ZoneName, Zone);
                        if (ThisZoneID > 0) {
                            state.dataCostEstimateManager->CostLineItem(Item).Qty =
                                state.dataSurface->Surface(ThisSurfID).Area * Zone(ThisZoneID).Multiplier * Zone(ThisZoneID).ListMultiplier;
                            state.dataCostEstimateManager->CostLineItem(Item).Units = "m2";
                            state.dataCostEstimateManager->CostLineItem(Item).ValuePer =
                                state.dataCostEstimateManager->CostLineItem(Item).PerSquareMeter;
                            state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                                state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                        }
                    }
                }
            } break;
            case ParentObject::Lights: {
                if (state.dataCostEstimateManager->CostLineItem(Item).PerEach != 0.0) {
                    state.dataCostEstimateManager->CostLineItem(Item).Qty = 1.0;
                    state.dataCostEstimateManager->CostLineItem(Item).Units = "Ea.";
                    state.dataCostEstimateManager->CostLineItem(Item).ValuePer = state.dataCostEstimateManager->CostLineItem(Item).PerEach;
                    state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                        state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                }

                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap != 0.0) {
                    if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                        ThisZoneID = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, Zone);
                        if (ThisZoneID > 0) {
                            Real64 Qty(0.0);
                            for (auto const &e : state.dataHeatBal->Lights) {
                                if (e.ZonePtr == ThisZoneID) {
                                    Qty += e.DesignLevel;
                                }
                            }
                            state.dataCostEstimateManager->CostLineItem(Item).Qty =
                                (Zone(ThisZoneID).Multiplier * Zone(ThisZoneID).ListMultiplier / 1000.0) *
                                Qty; // this handles more than one light object per zone.
                            state.dataCostEstimateManager->CostLineItem(Item).Units = "kW";
                            state.dataCostEstimateManager->CostLineItem(Item).ValuePer =
                                state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                            state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                                state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                        }
                    }
                }
            } break;
            case ParentObject::GeneratorPhotovoltaic: {
                if (state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap != 0.0) {
                    if (!state.dataCostEstimateManager->CostLineItem(Item).ParentObjName.empty()) {
                        thisPV = Util::FindItem(state.dataCostEstimateManager->CostLineItem(Item).ParentObjName, state.dataPhotovoltaic->PVarray);
                        if (thisPV > 0) {
                            ThisZoneID =
                                Util::FindItem(state.dataSurface->Surface(state.dataPhotovoltaic->PVarray(thisPV).SurfacePtr).ZoneName, Zone);
                            if (ThisZoneID == 0) {
                                Multipliers = 1.0;
                            } else {
                                Multipliers = Zone(ThisZoneID).Multiplier * Zone(ThisZoneID).ListMultiplier;
                            }
                            if (state.dataPhotovoltaic->PVarray(thisPV).PVModelType == DataPhotovoltaics::PVModel::Simple) {
                                state.dataCostEstimateManager->CostLineItem(Item).Qty =
                                    1000.0 * state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.AreaCol *
                                    state.dataPhotovoltaic->PVarray(thisPV).SimplePVModule.PVEfficiency * Multipliers / 1000.0;
                            }
                            state.dataCostEstimateManager->CostLineItem(Item).Units = "kW (rated)";
                            state.dataCostEstimateManager->CostLineItem(Item).ValuePer =
                                state.dataCostEstimateManager->CostLineItem(Item).PerKiloWattCap;
                            state.dataCostEstimateManager->CostLineItem(Item).LineSubTotal =
                                state.dataCostEstimateManager->CostLineItem(Item).Qty * state.dataCostEstimateManager->CostLineItem(Item).ValuePer;
                        }
                    }
                }
            } break;
            default:
                break;
            }
        }

        // now sum up the line items, result for the current building

        state.dataCostEstimateManager->CurntBldg.LineItemTot = sum(state.dataCostEstimateManager->CostLineItem, &CostLineItemStruct::LineSubTotal);
    }

} // namespace CostEstimateManager

} // namespace EnergyPlus
