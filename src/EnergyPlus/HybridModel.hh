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

#ifndef HybridModel_hh_INCLUDED
#define HybridModel_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HybridModel {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS

    // MODULE VARIABLE TYPE DECLARATIONS:

    // SUBROUTINE SPECIFICATIONS:

    // Types

    struct HybridModelZone
    {
        // Members
        std::string Name;
        Sched::Schedule *measuredTempSched = nullptr;
        Sched::Schedule *measuredHumRatSched = nullptr;
        Sched::Schedule *measuredCO2ConcSched = nullptr;

        Sched::Schedule *peopleActivityLevelSched = nullptr;
        Sched::Schedule *peopleSensibleFracSched = nullptr;
        Sched::Schedule *peopleRadiantFracSched = nullptr;
        Sched::Schedule *peopleCO2GenRateSched = nullptr;

        Sched::Schedule *supplyAirTempSched = nullptr;
        Sched::Schedule *supplyAirMassFlowRateSched = nullptr;
        Sched::Schedule *supplyAirHumRatSched = nullptr;
        Sched::Schedule *supplyAirCO2ConcSched = nullptr;

        bool InternalThermalMassCalc_T = false;     // Calculate thermal mass flag with measured temperature
        bool InfiltrationCalc_T = false;            // Calculate air infiltration rate flag with measured temperature
        bool InfiltrationCalc_H = false;            // Calculate air infiltration rate flag with measured humidity ratio
        bool InfiltrationCalc_C = false;            // Calculate air infiltration rate flag with measured CO2 concentration
        bool PeopleCountCalc_T = false;             // Calculate zone people count flag with measured temperature
        bool PeopleCountCalc_H = false;             // Calculate zone people count flag with measured humidity ratio
        bool PeopleCountCalc_C = false;             // Calculate zone people count flag with measured CO2 concentration
        bool IncludeSystemSupplyParameters = false; // Flag to decide whether to include system supply terms

        int measuredTempStartMonth = 0;
        int measuredTempStartDate = 0;
        int measuredTempEndMonth = 0;
        int measuredTempEndDate = 0;
        int HybridStartDayOfYear = 0; // Hybrid model start date of year
        int HybridEndDayOfYear = 0;   // Hybrid model end date of year
    };

    // Object Data

    // Functions

    void GetHybridModelZone(EnergyPlusData &state);

} // namespace HybridModel

struct HybridModelData : BaseGlobalStruct
{

    bool FlagHybridModel = false;    // True if hybrid model is activated
    bool FlagHybridModel_TM = false; // User input IM option - True if hybrid model (thermal mass) is activated
    bool FlagHybridModel_AI = false; // User input IM option - True if hybrid model (air infiltration) is activated
    bool FlagHybridModel_PC = false; // User input IM option - True if hybrid model (people count) is activated

    int NumOfHybridModelZones = 0;   // Number of hybrid model zones in the model
    std::string CurrentModuleObject; // to assist in getting input

    Array1D<HybridModel::HybridModelZone> hybridModelZones;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->FlagHybridModel = false;
        this->FlagHybridModel_TM = false;
        this->FlagHybridModel_AI = false;
        this->FlagHybridModel_PC = false;
        this->NumOfHybridModelZones = 0;
        this->CurrentModuleObject.clear();
        this->hybridModelZones.deallocate();
    }
};

} // namespace EnergyPlus

#endif
