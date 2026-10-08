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

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConstructionAssignmentSet.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace ConstructionAssignments {

    int ConstructionAssignmentSetData::getConstructionAssignment(DataSurfaces::SurfaceData const &surface) const
    {
        int extBoundCond = surface.ExtBoundCond;

        // Base opaque surfaces
        if (surface.Class == DataSurfaces::SurfaceClass::Wall || surface.Class == DataSurfaces::SurfaceClass::Floor ||
            surface.Class == DataSurfaces::SurfaceClass::Roof) {

            SurfaceConstructionAssignmentsData const *surfConstrs = nullptr;
            if (extBoundCond == DataSurfaces::ExternalEnvironment || extBoundCond == DataSurfaces::OtherSideCoefNoCalcExt ||
                extBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
                surfConstrs = exteriorSurfConstructions.get();
            } else if (extBoundCond == DataSurfaces::Ground || extBoundCond == DataSurfaces::GroundFCfactorMethod ||
                       extBoundCond == DataSurfaces::KivaFoundation) {
                surfConstrs = groundContactSurfConstructions.get();
            } else if (surface.ExtBoundCondName == surface.Name) {
                // Adiabatic: surface points to itself.
                // ExtBoundCondName == Name is preserved through BC reconciliation.
                return adiabaticSurfConstrNum;
            } else {
                surfConstrs = interiorSurfConstructions.get();
            }

            if (surfConstrs == nullptr) {
                return 0;
            }
            if (surface.Class == DataSurfaces::SurfaceClass::Floor) {
                return surfConstrs->floorConstrNum;
            }
            if (surface.Class == DataSurfaces::SurfaceClass::Roof) {
                return surfConstrs->roofCeilingConstrNum;
            }
            return surfConstrs->wallConstrNum;
        }

        // Interior mass
        if (surface.Class == DataSurfaces::SurfaceClass::IntMass) {
            return interiorPartitionConstrNum;
        }

        // Subsurfaces — extBoundCond is inherited from base surface (see GetHTSubSurfaceData)
        SubSurfaceConstructionAssignmentsData const *subSurfConstrs = nullptr;
        if (extBoundCond == DataSurfaces::ExternalEnvironment || extBoundCond == DataSurfaces::OtherSideCoefNoCalcExt ||
            extBoundCond == DataSurfaces::OtherSideCoefCalcExt) {
            subSurfConstrs = exteriorSubSurfConstructions.get();
        } else {
            subSurfConstrs = interiorSubSurfConstructions.get();
        }

        if (subSurfConstrs == nullptr) {
            return 0;
        }
        switch (surface.OriginalClass) {
        case DataSurfaces::SurfaceClass::Window:
        case DataSurfaces::SurfaceClass::FixedWindow:
            return subSurfConstrs->fixedWindowConstrNum;
        case DataSurfaces::SurfaceClass::OperableWindow:
            return subSurfConstrs->operableWindowConstrNum;
        case DataSurfaces::SurfaceClass::Skylight:
            return subSurfConstrs->skylightConstrNum;
        case DataSurfaces::SurfaceClass::Door:
            return subSurfConstrs->doorConstrNum;
        case DataSurfaces::SurfaceClass::GlassDoor:
            return subSurfConstrs->glassDoorConstrNum;
        case DataSurfaces::SurfaceClass::OverheadDoor:
            return subSurfConstrs->overheadDoorConstrNum;
        case DataSurfaces::SurfaceClass::TDD_Dome:
            return subSurfConstrs->tddDomeConstrNum;
        case DataSurfaces::SurfaceClass::TDD_Diffuser:
            return subSurfConstrs->tddDiffuserConstrNum;
        default:
            return 0;
        }
    }

    ConstructionWithSearchDistance constructionWithSearchDistance(EnergyPlusData &state, DataSurfaces::SurfaceData const &surface)
    {

        if (surface.Construction > 0) {
            return {.searchDistance = SearchDistanceType::Explicit, .constructionNum = surface.Construction};
        }
        auto &s_dc = state.dataConstructionAssignments;

        int spaceNum = surface.spaceNum;
        if (spaceNum > 0) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const cIdx = thisSpace.constructionAssignmentSetIndex;
            if (cIdx >= 0) {
                auto const &dcs = s_dc->constructionAssignmentSets[cIdx];
                int constrNum = dcs.getConstructionAssignment(surface);
                if (constrNum > 0) {
                    return {.searchDistance = SearchDistanceType::Space, .constructionNum = constrNum};
                }
            }
        }

        {
            int const cIdx = s_dc->buildingConstructionAssignmentSetIndex;
            if (cIdx >= 0) {
                auto const &dcs = s_dc->constructionAssignmentSets[cIdx];
                int constrNum = dcs.getConstructionAssignment(surface);
                if (constrNum > 0) {
                    return {.searchDistance = SearchDistanceType::Building, .constructionNum = constrNum};
                }
            }
        }

        return {.searchDistance = SearchDistanceType::Invalid, .constructionNum = 0};
    }

    ConstructionWithSearchDistance
    resolveConstructionWithSearchDistance(EnergyPlusData &state, DataSurfaces::SurfaceData const &surface, bool &ErrorsFound)
    {

        ConstructionWithSearchDistance thisCWSD = constructionWithSearchDistance(state, surface);

        // If no adjacent surface, return
        if (surface.ExtBoundCond <= 0) {
            return thisCWSD;
        }

        auto const &otherSurface = state.dataSurface->Surface(surface.ExtBoundCond);
        ConstructionWithSearchDistance adjacentCWSD = constructionWithSearchDistance(state, otherSurface);

        // This surface is deferring to the adjacent surface's resolved construction: it needs the
        // reverse (outside-to-inside) layer order, not the identical construction number, since
        // this surface is the other side of the same physical assembly.
        auto reversedAdjacent = [&]() -> ConstructionWithSearchDistance {
            int const reversedConstrNum = DataHeatBalance::AssignReverseConstructionNumber(state, adjacentCWSD.constructionNum, ErrorsFound);
            return {.searchDistance = adjacentCWSD.searchDistance, .constructionNum = reversedConstrNum};
        };

        bool const thisFound = thisCWSD.searchDistance != SearchDistanceType::Invalid;
        bool const adjacentFound = adjacentCWSD.searchDistance != SearchDistanceType::Invalid;

        if (thisFound && !adjacentFound) {
            // return this construction
            return thisCWSD;
        }

        if (!thisFound && adjacentFound) {
            // return the reverse of the adjacent construction
            return reversedAdjacent();
        }

        if (!thisFound && !adjacentFound) {
            // no constructions, nothing to be done
            return ConstructionWithSearchDistance{};
        }

        // both surfaces return a construction

        if (thisCWSD.constructionNum == adjacentCWSD.constructionNum) {
            // both surfaces have same construction
            return thisCWSD;
        }

        // Different constructions, but already mirror-images of each other (e.g. a Floor/RoofCeiling
        // pair modeled with dedicated, intentionally-reversed constructions): each side's own
        // independently resolved construction is already correct, regardless of search distance -
        // don't search for/create a reverse of whichever side "wins" below.
        {
            const auto &thisConstruct = state.dataConstruction->Construct(thisCWSD.constructionNum);
            const auto &adjacentConstruct = state.dataConstruction->Construct(adjacentCWSD.constructionNum);
            bool is_reversed_equal_layers = true;
            if (thisConstruct.TotLayers != adjacentConstruct.TotLayers) {
                is_reversed_equal_layers = false;
            } else {
                int adjacentLayerNum = 0;
                for (int thisLayerNum = thisConstruct.TotLayers; thisLayerNum >= 1; --thisLayerNum) {
                    ++adjacentLayerNum;
                    if (thisConstruct.LayerPoint(thisLayerNum) != adjacentConstruct.LayerPoint(adjacentLayerNum)) {
                        is_reversed_equal_layers = false;
                        break;
                    }
                }
            }

            if (is_reversed_equal_layers) {
                // these constructions are reverse equal
                return thisCWSD;
            }
        }

        // both surfaces return a construction, they are not the same, and are not reverse-equal

        if (thisCWSD.searchDistance < adjacentCWSD.searchDistance) {
            // lower search distance to construction
            return thisCWSD;
        }

        if (thisCWSD.searchDistance > adjacentCWSD.searchDistance) {
            // lower search distance to adjacent construction: use its reverse
            return reversedAdjacent();
        }

        // both surfaces return a construction, they are not the same, they are not reverse-equal,
        // and both have same search distance: give up for now
        ShowWarningError(
            state,
            std::format(
                R"(resolveConstructionWithSearchDistance: Surface="{}" and adjacent surface="{}" have different constructions with the same search distance. Using construction="{}".)",
                surface.Name,
                otherSurface.Name,
                state.dataConstruction->Construct(thisCWSD.constructionNum).Name));
        return thisCWSD;
    }

    void GetConstructionAssignmentSetData(EnergyPlusData &state, bool &ErrorsFound)
    {
        static constexpr std::string_view routineName = "GetConstructionAssignmentSetData: ";
        auto &ip = state.dataInputProcessing->inputProcessor;
        auto &s_dc = state.dataConstructionAssignments;

        // Returns the construction index for a named construction, or 0 if blank.
        // Errors if the name is non-blank but not found.
        auto getConstrNum = [&state, &ErrorsFound, &ip](std::string_view objType,
                                                        std::string_view objName,
                                                        nlohmann::json const &fields,
                                                        nlohmann::json const &objectSchemaProps,
                                                        const char *fieldName) -> int {
            std::string constrName = Util::makeUPPER(ip->getAlphaFieldValue(fields, objectSchemaProps, fieldName));
            if (constrName.empty()) {
                return 0;
            }

            int num = Util::FindItemInList(constrName, state.dataConstruction->Construct, state.dataHeatBal->TotConstructs);
            if (num == 0) {
                ShowSevereError(state, std::format(R"({}{}="{}", invalid {}="{}" not found.)", routineName, objType, objName, fieldName, constrName));
                ErrorsFound = true;
            }
            return num;
        };

        // Parse SurfaceConstructionAssignments
        {
            const std::string cCurrentModuleObject = "SurfaceConstructionAssignments";
            auto const instances = ip->epJSON.find(cCurrentModuleObject);
            if (instances != ip->epJSON.end()) {

                s_dc->surfaceConstructionAssignments.reserve(instances.value().size());

                auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
                for (auto instance = instances.value().begin(); instance != instances.value().end(); ++instance) {
                    ip->markObjectAsUsed(cCurrentModuleObject, instance.key());
                    auto const &fields = instance.value();
                    auto &data = s_dc->surfaceConstructionAssignments.emplace_back(std::make_shared<SurfaceConstructionAssignmentsData>());
                    data->Name = Util::makeUPPER(instance.key());

                    auto lookUpConstrNum = [&](const char *fieldName) -> int {
                        const int constrNum = getConstrNum(cCurrentModuleObject, data->Name, fields, objectSchemaProps, fieldName);
                        if (constrNum > 0) {
                            auto const &thisContruct = state.dataConstruction->Construct(constrNum);
                            if (thisContruct.TypeIsWindow) {
                                ShowSevereError(state,
                                                std::format(R"({}{}="{}", invalid {}="{}" - has Window material.)",
                                                            routineName,
                                                            cCurrentModuleObject,
                                                            data->Name,
                                                            fieldName,
                                                            thisContruct.Name));
                                ErrorsFound = true;
                            }
                        }
                        return constrNum;
                    };
                    data->floorConstrNum = lookUpConstrNum("floor_construction_name");
                    data->wallConstrNum = lookUpConstrNum("wall_construction_name");
                    data->roofCeilingConstrNum = lookUpConstrNum("roof_ceiling_construction_name");
                }
            }
        }

        // Parse SubSurfaceConstructionAssignments
        {
            const std::string cCurrentModuleObject = "SubSurfaceConstructionAssignments";
            auto const instances = ip->epJSON.find(cCurrentModuleObject);
            if (instances != ip->epJSON.end()) {

                s_dc->subSurfaceConstructionAssignments.reserve(instances.value().size());

                auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
                for (auto instance = instances.value().begin(); instance != instances.value().end(); ++instance) {
                    ip->markObjectAsUsed(cCurrentModuleObject, instance.key());
                    auto const &fields = instance.value();
                    auto &data = s_dc->subSurfaceConstructionAssignments.emplace_back(std::make_shared<SubSurfaceConstructionAssignmentsData>());
                    data->Name = Util::makeUPPER(instance.key());

                    auto lookUpConstrNum = [&](const char *fieldName, bool isGlazed) -> int {
                        const int constrNum = getConstrNum(cCurrentModuleObject, data->Name, fields, objectSchemaProps, fieldName);
                        if (constrNum > 0) {
                            auto const &thisContruct = state.dataConstruction->Construct(constrNum);
                            if (isGlazed) {
                                if (!thisContruct.TypeIsWindow && !thisContruct.TypeIsAirBoundary) {
                                    ShowSevereError(
                                        state,
                                        std::format(
                                            R"({}{}="{}", invalid {}="{}" - has an opaque surface construction; it should have a window construction.)",
                                            routineName,
                                            cCurrentModuleObject,
                                            data->Name,
                                            fieldName,
                                            thisContruct.Name));
                                    ErrorsFound = true;
                                }
                                if (thisContruct.SourceSinkPresent) {
                                    // Windows are not allowed to have embedded sources/sinks
                                    ShowSevereError(state,
                                                    std::format(R"({}{}="{}", invalid {}="{}" - has embedded source/sink.)",
                                                                routineName,
                                                                cCurrentModuleObject,
                                                                data->Name,
                                                                fieldName,
                                                                thisContruct.Name));
                                    ErrorsFound = true;
                                }

                            } else {
                                if (thisContruct.TypeIsWindow) {
                                    ShowSevereError(state,
                                                    std::format(R"({}{}="{}", invalid {}="{}" - has Window material.)",
                                                                routineName,
                                                                cCurrentModuleObject,
                                                                data->Name,
                                                                fieldName,
                                                                thisContruct.Name));
                                    ErrorsFound = true;
                                }
                            }
                        }
                        return constrNum;
                    };
                    data->fixedWindowConstrNum = lookUpConstrNum("fixed_window_construction_name", true);
                    data->operableWindowConstrNum = lookUpConstrNum("operable_window_construction_name", true);
                    data->doorConstrNum = lookUpConstrNum("door_construction_name", false);
                    data->glassDoorConstrNum = lookUpConstrNum("glass_door_construction_name", true);
                    data->overheadDoorConstrNum = lookUpConstrNum("overhead_door_construction_name", false);
                    data->skylightConstrNum = lookUpConstrNum("skylight_construction_name", true);
                    data->tddDomeConstrNum = lookUpConstrNum("tubular_daylight_dome_construction_name", true);
                    data->tddDiffuserConstrNum = lookUpConstrNum("tubular_daylight_diffuser_construction_name", true);
                }
            }
        }

        // Parse ConstructionAssignmentSet (references the above, so must come last)
        {
            const std::string cCurrentModuleObject = "ConstructionAssignmentSet";
            auto const instances = ip->epJSON.find(cCurrentModuleObject);
            if (instances != ip->epJSON.end()) {

                s_dc->constructionAssignmentSets.reserve(instances.value().size());

                auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
                for (auto instance = instances.value().begin(); instance != instances.value().end(); ++instance) {
                    ip->markObjectAsUsed(cCurrentModuleObject, instance.key());
                    auto const &fields = instance.value();

                    ConstructionAssignmentSetData &dcs = s_dc->constructionAssignmentSets.emplace_back();
                    dcs.Name = Util::makeUPPER(instance.key());

                    auto getUC = [&](const char *key) { return Util::makeUPPER(ip->getAlphaFieldValue(fields, objectSchemaProps, key)); };

                    auto findSurfConstr = [&](const char *key) -> std::shared_ptr<SurfaceConstructionAssignmentsData> {
                        std::string name = getUC(key);
                        if (name.empty()) {
                            return nullptr;
                        }
                        auto it = std::find_if(s_dc->surfaceConstructionAssignments.begin(),
                                               s_dc->surfaceConstructionAssignments.end(),
                                               [&name](const std::shared_ptr<SurfaceConstructionAssignmentsData> &d) { return d->Name == name; });
                        if (it == s_dc->surfaceConstructionAssignments.end()) {
                            ShowSevereError(
                                state,
                                std::format(R"({}{}="{}", invalid {}="{}" not found.)", routineName, cCurrentModuleObject, dcs.Name, key, name));
                            ErrorsFound = true;
                            return nullptr;
                        }
                        return *it;
                    };

                    auto findSubSurfConstr = [&](const char *key) -> std::shared_ptr<SubSurfaceConstructionAssignmentsData> {
                        std::string name = getUC(key);
                        if (name.empty()) {
                            return nullptr;
                        }
                        auto it = std::find_if(s_dc->subSurfaceConstructionAssignments.begin(),
                                               s_dc->subSurfaceConstructionAssignments.end(),
                                               [&name](const std::shared_ptr<SubSurfaceConstructionAssignmentsData> &d) { return d->Name == name; });
                        if (it == s_dc->subSurfaceConstructionAssignments.end()) {
                            ShowSevereError(
                                state,
                                std::format(R"({}{}="{}", invalid {}="{}" not found.)", routineName, cCurrentModuleObject, dcs.Name, key, name));
                            ErrorsFound = true;
                            return nullptr;
                        }
                        return *it;
                    };

                    dcs.exteriorSurfConstructions = findSurfConstr("exterior_surface_construction_assignments_name");
                    dcs.interiorSurfConstructions = findSurfConstr("interior_surface_construction_assignments_name");
                    dcs.groundContactSurfConstructions = findSurfConstr("ground_contact_surface_construction_assignments_name");
                    dcs.exteriorSubSurfConstructions = findSubSurfConstr("exterior_subsurface_construction_assignments_name");
                    dcs.interiorSubSurfConstructions = findSubSurfConstr("interior_subsurface_construction_assignments_name");

                    auto lookUpConstrNum = [&](const char *fieldName) -> int {
                        const int constrNum = getConstrNum(cCurrentModuleObject, dcs.Name, fields, objectSchemaProps, fieldName);
                        if (constrNum > 0) {
                            auto const &thisContruct = state.dataConstruction->Construct(constrNum);
                            if (thisContruct.TypeIsWindow) {
                                ShowSevereError(state,
                                                std::format(R"({}{}="{}", invalid {}="{}" - has Window material.)",
                                                            routineName,
                                                            cCurrentModuleObject,
                                                            dcs.Name,
                                                            fieldName,
                                                            thisContruct.Name));
                                ErrorsFound = true;
                            }
                        }
                        return constrNum;
                    };
                    dcs.interiorPartitionConstrNum = lookUpConstrNum("interior_partition_construction_name");
                    dcs.adiabaticSurfConstrNum = lookUpConstrNum("adiabatic_surface_construction_name");
                }
            }
        }
        // Resolve Building-level construction assignment set (name was stored before CAS data was loaded)
        if (!s_dc->buildingConstructionAssignmentSetName.empty()) {
            auto it = std::find_if(s_dc->constructionAssignmentSets.begin(),
                                   s_dc->constructionAssignmentSets.end(),
                                   [&](const ConstructionAssignmentSetData &d) { return d.Name == s_dc->buildingConstructionAssignmentSetName; });
            if (it == s_dc->constructionAssignmentSets.end()) {
                ShowSevereError(state,
                                std::format(R"({}Building: invalid Construction Assignment Set Name="{}" not found.)",
                                            routineName,
                                            s_dc->buildingConstructionAssignmentSetName));
                ErrorsFound = true;
            } else {
                s_dc->buildingConstructionAssignmentSetIndex = static_cast<int>(std::distance(s_dc->constructionAssignmentSets.begin(), it));
            }
        }
    }

} // namespace ConstructionAssignments
} // namespace EnergyPlus
