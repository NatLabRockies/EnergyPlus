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

#ifndef OutsideEnergySources_hh_INCLUDED
#define OutsideEnergySources_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace OutsideEnergySources {

    struct OutsideEnergySourceSpecs : public PlantComponent
    {

        // Members
        std::string Name;                                                                 // user identifier
        Real64 NomCap = 0.0;                                                              // design nominal capacity of district service
        bool NomCapWasAutoSized = false;                                                  // ture if Nominal Capacity was autosize on input
        Sched::Schedule *capFractionSched = nullptr;                                      // capacity modifier schedule number
        int InletNodeNum = 0;                                                             // Node number on the inlet side of the plant
        int OutletNodeNum = 0;                                                            // Node number on the inlet side of the plant
        Real64 EnergyTransfer = 0.0;                                                      // cooling energy provided in time step
        Real64 EnergyRate = 0.0;                                                          // cooling power
        DataPlant::PlantEquipmentType EnergyType{DataPlant::PlantEquipmentType::Invalid}; // flag for district heating OR cooling
        // loop topology variables
        PlantLocation plantLoc{};
        // flags
        bool BeginEnvrnInitFlag = true;
        bool CheckEquipName = true;
        Real64 MassFlowRate = 0.0;
        Real64 InletTemp = 0.0;
        Real64 OutletTemp = 0.0;
        Real64 OutletSteamQuality = 0.0;

        OutsideEnergySourceSpecs() = default;

        virtual ~OutsideEnergySourceSpecs() = default;

        static PlantComponent *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string_view objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void initialize(EnergyPlusData &state, Real64 curLoad);

        void calculate(EnergyPlusData &state, bool runFlag, Real64 curLoad);

        void size(EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    void GetOutsideEnergySourcesInput(EnergyPlusData &state);

    void InitSimVars(int EnergySourceNum, Real64 MyLoad);

} // namespace OutsideEnergySources

struct OutsideEnergySourcesData : BaseGlobalStruct
{

    int NumDistrictUnits = 0;
    bool SimOutsideEnergyGetInputFlag = true;
    Array1D<OutsideEnergySources::OutsideEnergySourceSpecs> EnergySource;
    std::unordered_map<std::string, std::string> EnergySourceUniqueNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumDistrictUnits = 0;
        this->SimOutsideEnergyGetInputFlag = true;
        this->EnergySource.deallocate();
        this->EnergySourceUniqueNames.clear();
    }
};

} // namespace EnergyPlus

#endif
