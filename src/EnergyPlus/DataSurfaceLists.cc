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
#include <format>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataSurfaceLists {

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   September 2008
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This data-only module contains type definitions and variables
// associated with Radiant System Surface Groups.

void GetSurfaceListsInputs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   September 2008

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the surface lists for the Radiant System Surface Groups input.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view CurrentModuleObject1("ZoneHVAC:LowTemperatureRadiant:SurfaceGroup");
    constexpr std::string_view CurrentModuleObject2("ZoneHVAC:VentilatedSlab:SlabGroup");
    Real64 constexpr FlowFractionTolerance(0.0001); // Smallest deviation from unity for the sum of all fractions
    Real64 constexpr SurfListMinFlowFrac(0.001);    // Minimum allowed flow fraction (to avoid divide by zero)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NameConflict;         // Used to see if a surface name matches the name of a surface list (not allowed)
    Real64 SumOfAllFractions; // Summation of all of the fractions for splitting flow (must sum to 1)
    bool ErrorsFound;

    // Obtain all of the user data related to surface lists.  Need to get
    // this before getting the radiant system or ventilated slab data.

    auto &SurfList(state.dataSurfLists->SurfList);
    auto &SlabList(state.dataSurfLists->SlabList);

    ErrorsFound = false;

    auto *inputProcessor = state.dataInputProcessing->inputProcessor.get();

    // Update Num in state and make local convenience copy
    int NumOfSurfaceLists = state.dataSurfLists->NumOfSurfaceLists = inputProcessor->getNumObjectsFound(state, CurrentModuleObject1);
    int NumOfSurfListVentSlab = state.dataSurfLists->NumOfSurfListVentSlab = inputProcessor->getNumObjectsFound(state, CurrentModuleObject2);

    SurfList.allocate(NumOfSurfaceLists);
    SlabList.allocate(NumOfSurfListVentSlab);

    if (NumOfSurfaceLists > 0) {
        auto const &surfaceGroupSchemaProps = inputProcessor->getObjectSchemaProps(state, std::string(CurrentModuleObject1));
        auto const &surfaceFractionSchemaProps = surfaceGroupSchemaProps.at("surface_fractions").at("items").at("properties");
        auto const surfaceGroupObjects = inputProcessor->epJSON.find(std::string(CurrentModuleObject1));
        static constexpr std::string_view surfaceNameFieldName = "Surface Name";
        if (surfaceGroupObjects != inputProcessor->epJSON.end()) {
            int Item = 0;
            for (auto const &surfaceGroupInstance : surfaceGroupObjects.value().items()) {
                auto const &surfaceGroupFields = surfaceGroupInstance.value();
                auto const surfaceGroupName = Util::makeUPPER(surfaceGroupInstance.key());
                auto const surfaceFractionsField = surfaceGroupFields.find("surface_fractions");

                inputProcessor->markObjectAsUsed(std::string(CurrentModuleObject1), surfaceGroupInstance.key());

                ++Item;
                SurfList(Item).Name = surfaceGroupName;
                SurfList(Item).NumOfSurfaces =
                    (surfaceFractionsField != surfaceGroupFields.end()) ? static_cast<int>(surfaceFractionsField->size()) : 0;

                NameConflict = Util::FindItemInList(SurfList(Item).Name, state.dataSurface->Surface);
                if (NameConflict > 0) { // A surface list has the same name as a surface--not allowed
                    ShowSevereError(state,
                                    std::format("{}{}",
                                                CurrentModuleObject1,
                                                " = " + SurfList(Item).Name + " has the same name as a surface; this is not allowed."));
                    ErrorsFound = true;
                }

                if (SurfList(Item).NumOfSurfaces < 1) {
                    ShowSevereError(state,
                                    std::format("{}{}", CurrentModuleObject1, " = " + SurfList(Item).Name + " does not have any surfaces listed."));
                    ErrorsFound = true;
                } else {
                    SurfList(Item).SurfName.allocate(SurfList(Item).NumOfSurfaces);
                    SurfList(Item).SurfPtr.allocate(SurfList(Item).NumOfSurfaces);
                    SurfList(Item).SurfFlowFrac.allocate(SurfList(Item).NumOfSurfaces);
                }

                SumOfAllFractions = 0.0;
                bool showSameZoneWarning = true;
                int ZoneForSurface = 0; // Zone number that first surface is attached to
                for (int SurfNum = 1; SurfNum <= SurfList(Item).NumOfSurfaces; ++SurfNum) {
                    auto const &surfaceFraction = (*surfaceFractionsField)[SurfNum - 1];
                    SurfList(Item).SurfName(SurfNum) =
                        Util::makeUPPER(inputProcessor->getAlphaFieldValue(surfaceFraction, surfaceFractionSchemaProps, "surface_name"));
                    SurfList(Item).SurfPtr(SurfNum) = Util::FindItemInList(SurfList(Item).SurfName(SurfNum), state.dataSurface->Surface);
                    if (SurfList(Item).SurfPtr(SurfNum) == 0) {
                        ShowSevereError(
                            state,
                            std::format(
                                "{} in {} statement not found = {}", surfaceNameFieldName, CurrentModuleObject1, SurfList(Item).SurfName(SurfNum)));
                        ErrorsFound = true;
                    } else { // Make sure that all of the surfaces are located in the same zone
                        state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(SurfList(Item).SurfPtr(SurfNum)) = true;
                        if (SurfNum == 1) {
                            ZoneForSurface = state.dataSurface->Surface(SurfList(Item).SurfPtr(SurfNum)).Zone;
                        }
                        if (SurfNum > 1) {
                            if (ZoneForSurface != state.dataSurface->Surface(SurfList(Item).SurfPtr(SurfNum)).Zone && showSameZoneWarning) {
                                ShowWarningError(state,
                                                 std::format("Not all surfaces in same zone for {} = {}", CurrentModuleObject1, SurfList(Item).Name));
                                if (!state.dataGlobal->DisplayExtraWarnings) {
                                    ShowContinueError(state,
                                                      "If this is intentionally a radiant system with surfaces in more than one thermal zone,");
                                    ShowContinueError(
                                        state, "then ignore this warning message.  Use Output:Diagnostics,DisplayExtraWarnings for more details.");
                                }
                                showSameZoneWarning = false;
                            }
                        }
                    }
                    SurfList(Item).SurfFlowFrac(SurfNum) =
                        inputProcessor->getRealFieldValue(surfaceFraction, surfaceFractionSchemaProps, "flow_fraction_for_surface");
                    if (SurfList(Item).SurfFlowFrac(SurfNum) < SurfListMinFlowFrac) {
                        ShowSevereError(state,
                                        std::format("The Flow Fraction for Surface {} in Surface Group {} is too low",
                                                    SurfList(Item).SurfName(SurfNum),
                                                    SurfList(Item).Name));
                        ShowContinueError(state,
                                          std::format("Flow fraction of {:.6f} is less than minimum criteria = {:.6f}",
                                                      SurfList(Item).SurfFlowFrac(SurfNum),
                                                      SurfListMinFlowFrac));
                        ShowContinueError(state,
                                          "Zero or extremely low flow fractions are not allowed. Remove this surface from the surface group or "
                                          "combine small surfaces together.");
                        ErrorsFound = true;
                    }
                    SumOfAllFractions += SurfList(Item).SurfFlowFrac(SurfNum);
                }

                if (std::abs(SumOfAllFractions - 1.0) > FlowFractionTolerance) {
                    ShowSevereError(state,
                                    std::format("{}{}", CurrentModuleObject1, " flow fractions do not add up to unity for " + SurfList(Item).Name));
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowSevereError(state, std::format("{}{}", CurrentModuleObject1, " errors found getting input. Program will terminate."));
        }
    }

    if (NumOfSurfListVentSlab > 0) {
        auto const &slabGroupSchemaProps = inputProcessor->getObjectSchemaProps(state, std::string(CurrentModuleObject2));
        auto const &slabGroupDataSchemaProps = slabGroupSchemaProps.at("data").at("items").at("properties");
        auto const slabGroupObjects = inputProcessor->epJSON.find(std::string(CurrentModuleObject2));
        static constexpr std::string_view zoneNameFieldName = "Zone Name";
        static constexpr std::string_view slabSurfaceNameFieldName = "Surface Name";

        if (slabGroupObjects != inputProcessor->epJSON.end()) {
            int Item = 0;
            for (auto const &slabGroupInstance : slabGroupObjects.value().items()) {
                auto const &slabGroupFields = slabGroupInstance.value();
                auto const slabGroupName = Util::makeUPPER(slabGroupInstance.key());
                auto const slabGroupDataField = slabGroupFields.find("data");

                inputProcessor->markObjectAsUsed(std::string(CurrentModuleObject2), slabGroupInstance.key());

                ++Item;
                SlabList(Item).Name = slabGroupName;
                SlabList(Item).NumOfSurfaces = (slabGroupDataField != slabGroupFields.end()) ? static_cast<int>(slabGroupDataField->size()) : 0;

                NameConflict = Util::FindItemInList(SlabList(Item).Name, state.dataSurface->Surface);
                if (NameConflict > 0) { // A surface list has the same name as a surface--not allowed
                    ShowSevereError(state,
                                    std::format("{}{}",
                                                CurrentModuleObject2,
                                                " = " + SlabList(Item).Name + " has the same name as a slab; this is not allowed."));
                    ErrorsFound = true;
                }

                if (SlabList(Item).NumOfSurfaces < 1) {
                    ShowSevereError(state,
                                    std::format("{}{}", CurrentModuleObject2, " = " + SlabList(Item).Name + " does not have any slabs listed."));
                    ErrorsFound = true;
                } else {
                    SlabList(Item).ZoneName.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).ZonePtr.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).SurfName.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).SurfPtr.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).CoreDiameter.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).CoreLength.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).CoreNumbers.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).SlabInNodeName.allocate(SlabList(Item).NumOfSurfaces);
                    SlabList(Item).SlabOutNodeName.allocate(SlabList(Item).NumOfSurfaces);
                }

                for (int SurfNum = 1; SurfNum <= SlabList(Item).NumOfSurfaces; ++SurfNum) {
                    auto const &slabGroupData = (*slabGroupDataField)[SurfNum - 1];
                    SlabList(Item).ZoneName(SurfNum) =
                        Util::makeUPPER(inputProcessor->getAlphaFieldValue(slabGroupData, slabGroupDataSchemaProps, "zone_name"));
                    SlabList(Item).ZonePtr(SurfNum) = Util::FindItemInList(SlabList(Item).ZoneName(SurfNum), state.dataHeatBal->Zone);
                    if (SlabList(Item).ZonePtr(SurfNum) == 0) {
                        ShowSevereError(
                            state,
                            std::format("{} in {} Zone not found = {}", zoneNameFieldName, CurrentModuleObject2, SlabList(Item).ZoneName(SurfNum)));
                        ErrorsFound = true;
                    }

                    SlabList(Item).SurfName(SurfNum) =
                        Util::makeUPPER(inputProcessor->getAlphaFieldValue(slabGroupData, slabGroupDataSchemaProps, "surface_name"));
                    SlabList(Item).SurfPtr(SurfNum) = Util::FindItemInList(SlabList(Item).SurfName(SurfNum), state.dataSurface->Surface);
                    if (SlabList(Item).SurfPtr(SurfNum) == 0) {
                        ShowSevereError(state,
                                        std::format("{} in {} statement not found = {}",
                                                    slabSurfaceNameFieldName,
                                                    CurrentModuleObject2,
                                                    SlabList(Item).SurfName(SurfNum)));
                        ErrorsFound = true;
                    }
                    for (int SrfList = 1; SrfList <= NumOfSurfaceLists; ++SrfList) {
                        NameConflict =
                            Util::FindItemInList(SlabList(Item).SurfName(SurfNum), SurfList(SrfList).SurfName, SurfList(SrfList).NumOfSurfaces);
                        if (NameConflict > 0) { // A slab list includes a surface on a surface list--not allowed
                            ShowSevereError(
                                state, std::format("{}{}", CurrentModuleObject2, "=\"" + SlabList(Item).Name + "\", invalid surface specified."));
                            ShowContinueError(state, std::format("Surface=\"{}\" is also on a Surface List.", SlabList(Item).SurfName(SurfNum)));
                            ShowContinueError(
                                state, std::format("{}{}", CurrentModuleObject1, "=\"" + SurfList(SrfList).Name + "\" has this surface also."));
                            ShowContinueError(state, "A surface cannot be on both lists. The models cannot operate correctly.");
                            ErrorsFound = true;
                        }
                    }
                    state.dataSurface->SurfIsRadSurfOrVentSlabOrPool(SlabList(Item).SurfPtr(SurfNum)) = true;

                    SlabList(Item).CoreDiameter(SurfNum) =
                        inputProcessor->getRealFieldValue(slabGroupData, slabGroupDataSchemaProps, "core_diameter_for_surface");
                    SlabList(Item).CoreLength(SurfNum) =
                        inputProcessor->getRealFieldValue(slabGroupData, slabGroupDataSchemaProps, "core_length_for_surface");
                    SlabList(Item).CoreNumbers(SurfNum) =
                        inputProcessor->getRealFieldValue(slabGroupData, slabGroupDataSchemaProps, "core_numbers_for_surface");
                    SlabList(Item).SlabInNodeName(SurfNum) = Util::makeUPPER(
                        inputProcessor->getAlphaFieldValue(slabGroupData, slabGroupDataSchemaProps, "slab_inlet_node_name_for_surface"));
                    SlabList(Item).SlabOutNodeName(SurfNum) = Util::makeUPPER(
                        inputProcessor->getAlphaFieldValue(slabGroupData, slabGroupDataSchemaProps, "slab_outlet_node_name_for_surface"));
                }
            }
        }

        if (ErrorsFound) {
            ShowSevereError(state, std::format("{}{}", CurrentModuleObject2, " errors found getting input. Program will terminate."));
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "GetSurfaceListsInputs: Program terminates due to preceding conditions.");
    }
}

int GetNumberOfSurfaceLists(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   September 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Acts as a target for outside routines to make sure data is gotten before using.

    if (!state.dataSurfLists->SurfaceListInputsFilled) {
        GetSurfaceListsInputs(state);
        state.dataSurfLists->SurfaceListInputsFilled = true;
    }

    return state.dataSurfLists->NumOfSurfaceLists;
}

int GetNumberOfSurfListVentSlab(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   September 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Acts as a target for outside routines to make sure data is gotten before using.

    // Return value
    int NumberOfSurfListVentSlab;

    if (!state.dataSurfLists->SurfaceListInputsFilled) {
        GetSurfaceListsInputs(state);
        state.dataSurfLists->SurfaceListInputsFilled = true;
    }

    NumberOfSurfListVentSlab = state.dataSurfLists->NumOfSurfListVentSlab;

    return NumberOfSurfListVentSlab;
}

} // namespace EnergyPlus::DataSurfaceLists
