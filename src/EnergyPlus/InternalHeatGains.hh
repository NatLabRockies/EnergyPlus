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

#ifndef InternalHeatGains_hh_INCLUDED
#define InternalHeatGains_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <GSL/span.h>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace InternalHeatGains {

    enum class DesignLevelMethod
    {
        Invalid = -1,
        People,
        PeoplePerArea,
        AreaPerPerson,
        LightingLevel,
        EquipmentLevel,
        WattsPerArea,
        WattsPerPerson,
        PowerPerArea,
        PowerPerPerson,
        Num
    };

    struct GlobalInternalGainMiscObject
    {
        // Members
        std::string Name;
        bool ZoneListActive = false;
        int spaceOrSpaceListPtr = 0;
        int numOfSpaces = 0;
        int spaceStartPtr = 0;
        bool spaceListActive = false;
        EPVector<int> spaceNums;     // Indexes to spaces associated with this input object
        EPVector<std::string> names; // Names for each instance created from this input object
    };

    void ManageInternalHeatGains(EnergyPlusData &state,
                                 ObjexxFCL::Optional_bool_const InitOnly = _); // when true, just calls the get input, if appropriate and returns.

    void GetInternalHeatGainsInput(EnergyPlusData &state);

    void setupIHGZonesAndSpaces(EnergyPlusData &state,
                                const std::string &objectType,
                                EPVector<InternalHeatGains::GlobalInternalGainMiscObject> &inputObjects,
                                int &numInputObjects,
                                int &numGainInstances,
                                bool &errors,
                                const bool zoneListNotAllowed = false);

    Real64 setDesignLevel(EnergyPlusData &state,
                          bool &ErrorsFound,
                          std::string_view const objectType,
                          InternalHeatGains::GlobalInternalGainMiscObject const &inputObject,
                          DesignLevelMethod const method,
                          int const zoneNum,
                          int const spaceNum,
                          Real64 const inputValue,
                          bool const inputBlank,
                          std::string_view const fieldName);

    void setupIHGOutputs(EnergyPlusData &state);

    void InitInternalHeatGains(EnergyPlusData &state);

    void CheckReturnAirHeatGain(EnergyPlusData &state);

    void CalcZoneITEq(EnergyPlusData &state);

    void ReportInternalHeatGains(EnergyPlusData &state);

    Real64 GetDesignLightingLevelForZone(EnergyPlusData &state, int const WhichZone); // name of zone

    bool CheckThermalComfortSchedules(bool const WorkEffSch,  // Blank work efficiency schedule = true
                                      bool const CloInsSch,   // Blank clothing insulation schedule = true
                                      bool const AirVeloSch); // Blank air velocity schedule = true

    void CheckLightsReplaceableMinMaxForZone(EnergyPlusData &state, int const WhichZone); // Zone Number

    void UpdateInternalGainValues(EnergyPlusData &state, bool const SuppressRadiationUpdate = false, bool const SumLatentGains = false);

    Real64 zoneSumAllInternalConvectionGains(EnergyPlusData &state,
                                             int const zoneNum); // zone index pointer to sum gains for

    Real64 spaceSumAllInternalConvectionGains(EnergyPlusData &state,
                                              int const spaceNum); // space index pointer to sum gains for

    Real64 SumAllInternalConvectionGainsExceptPeople(EnergyPlusData &state,
                                                     int const ZoneNum); // zone index pointer for which zone to sum gains for

    Real64 SumInternalConvectionGainsByTypes(
        EnergyPlusData &state,
        int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
        gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
        int const spaceIndex = 0);                                 // space index pointer, sum gains only for this space

    int GetInternalGainDeviceIndex(EnergyPlusData &state,
                                   int const ZoneNum,                              // zone index pointer for which zone to sum gains for
                                   DataHeatBalance::IntGainType const IntGainType, // zone internal gain type enum
                                   std::string_view const IntGainName);            // Internal gain name

    Real64 SumInternalConvectionGainsByIndices(
        EnergyPlusData &state,
        int const NumGains,                // number of device gains to sum
        const Array1D_int &DeviceSpaceARR, // variable length 1-d array of integer space index pointers to include in summation
        const Array1D_int &DeviceIndexARR, // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &FractionARR // array of fractional multipliers to apply to devices
    );

    Real64 SumInternalLatentGainsByIndices(
        EnergyPlusData &state,
        int const NumGains,                // number of device gains to sum
        const Array1D_int &DeviceSpaceARR, // variable length 1-d array of integer space index pointers to include in summation
        const Array1D_int &DeviceIndexARR, // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &FractionARR // array of fractional multipliers to apply to devices
    );

    Real64 SumReturnAirConvectionGainsByIndices(
        EnergyPlusData &state,
        int const NumGains,                // number of device gains to sum
        const Array1D_int &DeviceSpaceARR, // variable length 1-d array of integer space index pointers to include in summation
        const Array1D_int &DeviceIndexARR, // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &FractionARR // array of fractional multipliers to apply to devices
    );

    Real64 zoneSumAllReturnAirConvectionGains(EnergyPlusData &state,
                                              int const zoneNum,      // zone index pointer to sum gains for
                                              int const returnNodeNum // return air node number
    );

    Real64 spaceSumAllReturnAirConvectionGains(EnergyPlusData &state,
                                               int const spaceNum,     // space index pointer to sum gains for
                                               int const returnNodeNum // return air node number
    );

    Real64 SumReturnAirConvectionGainsByTypes(
        EnergyPlusData &state,
        int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
        gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
        int const spaceIndex = 0);                                 // space index pointer, sum gains only for this space

    Real64 SumAllSpaceInternalRadiationGains(EnergyPlusData &state,
                                             int const SpaceNum // space index pointer for which space to sum gains for
    );

    Real64
    SumInternalRadiationGainsByTypes(EnergyPlusData &state,
                                     int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
                                     gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
                                     int const spaceIndex = 0);                                 // space index pointer, sum gains only for this space

    Real64 SumEnclosureInternalRadiationGainsByTypes(
        EnergyPlusData &state,
        int const enclosureNum,                                     // enclosure to sum gains for
        gsl::span<const DataHeatBalance::IntGainType> GainTypeARR); // variable length 1-d array of enum valued gain types

    void SumAllInternalLatentGains(EnergyPlusData &state,
                                   int const ZoneNum // zone index pointer for which zone to sum gains for
    );

    // Added for hybrid model -- calculate the latent gain from all sources except for people
    void SumAllInternalLatentGainsExceptPeople(EnergyPlusData &state,
                                               int const ZoneNum // zone index pointer for which zone to sum gains for
    );

    Real64
    SumInternalLatentGainsByTypes(EnergyPlusData &state,
                                  int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
                                  gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
                                  int const spaceIndex = 0);                                 // space index pointer, sum gains only for this space

    Real64 SumAllReturnAirLatentGains(EnergyPlusData &state,
                                      int const ZoneNum,      // zone index pointer for which zone to sum gains for
                                      int const ReturnNodeNum // return air node number
    );

    Real64 SumAllInternalCO2Gains(EnergyPlusData &state,
                                  int const ZoneNum // zone index pointer for which zone to sum gains for
    );

    // Added for hybrid model -- Overload function for calculating CO2 gains except people
    Real64 SumAllInternalCO2GainsExceptPeople(EnergyPlusData &state,
                                              int const ZoneNum // zone index pointer for which zone to sum gains for
    );

    Real64
    SumInternalCO2GainsByTypes(EnergyPlusData &state,
                               int const ZoneNum,                                        // zone index pointer for which zone to sum gains for
                               gsl::span<const DataHeatBalance::IntGainType> GainTypeARR // variable length 1-d array of integer valued gain types
    );

    Real64 SumAllInternalGenericContamGains(EnergyPlusData &state,
                                            int const ZoneNum // zone index pointer for which zone to sum gains for
    );

    void GatherComponentLoadsIntGain(EnergyPlusData &state);

    void gatherCompLoadIntGain2(EnergyPlusData &state,
                                OutputReportTabular::compLoadsSpaceZone &szCompLoadDayTS,
                                int const zoneNum,
                                int const spaceNum = 0);

} // namespace InternalHeatGains

struct InternalHeatGainsData : BaseGlobalStruct
{

    bool GetInternalHeatGainsInputFlag = true; // Controls the GET routine calling (limited to first time)
    bool ErrorsFound = false;                  // if errors were found in the input

    // Declared here because they are needed later for the demand manager, other types of internal gain inputs are local
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> lightsObjects;
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> zoneElectricObjects;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetInternalHeatGainsInputFlag = true;
        this->ErrorsFound = false;
    }
};

} // namespace EnergyPlus

#endif
