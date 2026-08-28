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

#ifndef ConstructionAssignmentSet_hh_INCLUDED
#define ConstructionAssignmentSet_hh_INCLUDED

// C++ Headers
#include <array>
#include <memory>
#include <string_view>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;
namespace DataSurfaces {
    struct SurfaceData;
}

namespace ConstructionAssignments {

    enum class SearchDistanceType
    {
        Invalid = -1,
        Explicit = 0,
        Space,
        // SpaceType
        // BuildingStory
        Building,
        // BuildingSpaceType,
        Num,
    };

    // Display strings for the EnvelopeSummary report's "Construction Assignment Source" column.
    constexpr std::array<std::string_view, static_cast<int>(SearchDistanceType::Num)> SearchDistanceTypeNames = {
        "Explicit",
        "Space",
        "Building",
    };

    struct ConstructionWithSearchDistance
    {
        SearchDistanceType searchDistance = SearchDistanceType::Invalid;
        int constructionNum = 0;
    };

    void GetConstructionAssignmentSetData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    // For a given surface, this function will call constructionWithSearchDistance for the surface and its adjacent surface (if it exists) and
    // resolve which construction to return based on the search distance for both. When the adjacent surface's construction wins, the returned
    // construction is its reverse (outside-to-inside) layer order, since this surface is the other side of the same assembly.
    ConstructionWithSearchDistance
    resolveConstructionWithSearchDistance(EnergyPlusData &state, DataSurfaces::SurfaceData const &surface, bool &ErrorsFound);

    // This is a helper only, use resolveConstructionWithSearchDistance instead for public usage
    // This function looks up a construction for the surface, checking surface, space, and building-level construction assignment sets.
    ConstructionWithSearchDistance constructionWithSearchDistance(EnergyPlusData &state, DataSurfaces::SurfaceData const &surface);

    struct SubSurfaceConstructionAssignmentsData
    {
        std::string Name;
        int fixedWindowConstrNum = 0;
        int operableWindowConstrNum = 0;
        int doorConstrNum = 0;
        int glassDoorConstrNum = 0;
        int overheadDoorConstrNum = 0;
        int skylightConstrNum = 0;
        int tddDomeConstrNum = 0;
        int tddDiffuserConstrNum = 0;
    };

    struct SurfaceConstructionAssignmentsData
    {
        std::string Name;
        int floorConstrNum = 0;
        int wallConstrNum = 0;
        int roofCeilingConstrNum = 0;
    };

    struct ConstructionAssignmentSetData
    {
        std::string Name;
        // Surfaces Constructions
        std::shared_ptr<SurfaceConstructionAssignmentsData> exteriorSurfConstructions;
        std::shared_ptr<SurfaceConstructionAssignmentsData> interiorSurfConstructions;
        std::shared_ptr<SurfaceConstructionAssignmentsData> groundContactSurfConstructions;
        // SubSurfaces Constructions
        std::shared_ptr<SubSurfaceConstructionAssignmentsData> exteriorSubSurfConstructions;
        std::shared_ptr<SubSurfaceConstructionAssignmentsData> interiorSubSurfConstructions;
        // Direct Constructions indices
        int interiorPartitionConstrNum = 0;
        int adiabaticSurfConstrNum = 0;

        int getConstructionAssignment(DataSurfaces::SurfaceData const &surface) const;
    };
} // namespace ConstructionAssignments

struct ConstructionAssignmentsData : BaseGlobalStruct
{

    std::vector<std::shared_ptr<ConstructionAssignments::SurfaceConstructionAssignmentsData>> surfaceConstructionAssignments;
    std::vector<std::shared_ptr<ConstructionAssignments::SubSurfaceConstructionAssignmentsData>> subSurfaceConstructionAssignments;

    std::vector<ConstructionAssignments::ConstructionAssignmentSetData> constructionAssignmentSets;

    std::string buildingConstructionAssignmentSetName; // Raw name from Building object, set before CAS data is loaded
    int buildingConstructionAssignmentSetIndex = -1;   // 0-based index into constructionAssignmentSets, or -1 if none

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        constructionAssignmentSets.clear();
        surfaceConstructionAssignments.clear();
        subSurfaceConstructionAssignments.clear();
        buildingConstructionAssignmentSetName.clear();
        buildingConstructionAssignmentSetIndex = -1;
    }
};

} // namespace EnergyPlus

#endif
