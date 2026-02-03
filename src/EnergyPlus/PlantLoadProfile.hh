// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef PlantLoadProfile_hh_INCLUDED
#define PlantLoadProfile_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantLoadProfile {

    enum class PlantLoopFluidType
    {
        Invalid = -1,
        Water,
        Steam,
        Num
    };

    struct PlantProfileData : public PlantComponent
    {
        virtual ~PlantProfileData() = default;

        // Members
        std::string Name;                                           // Name of Plant Load Profile object
        DataPlant::PlantEquipmentType Type;                         // Plant Side Connection: 'Type' assigned in DataPlant
        PlantLocation plantLoc;                                     // plant loop component location object
        PlantLoopFluidType FluidType = PlantLoopFluidType::Invalid; // plant loop fluid type: water or steam
        bool Init;                                                  // Flag for initialization:  TRUE means do the init
        bool InitSizing;                                            // Flag for initialization of plant sizing
        int InletNode;
        Real64 InletTemp; // Inlet temperature (C)
        int OutletNode;
        Real64 OutletTemp;                            // Outlet temperature (C)
        Sched::Schedule *loadSched = nullptr;         // load schedule
        bool EMSOverridePower;                        // if true, then EMS is calling to override power level
        Real64 EMSPowerValue;                         // value EMS is directing to use for power [W]
        Real64 PeakVolFlowRate;                       // Peak volumetric flow rate, also water consumption rate (m3/s)
        Sched::Schedule *flowRateFracSched = nullptr; // flow rate fraction schedule
        Real64 VolFlowRate;                           // Volumetric flow rate (m3/s)
        Real64 MassFlowRate;                          // Mass flow rate (kg/s)
        Real64 DegOfSubcooling = 0.0;                 // Degree of subcooling in steam outlet
        Real64 LoopSubcoolReturn = 0.0;               // Loop subcooling for steam return
        bool EMSOverrideMassFlow;
        Real64 EMSMassFlowValue;
        // Report variables
        Real64 Power;         // Power required to meet the load (W)
        Real64 Energy;        // Energy required to meet the load (J)
        Real64 HeatingEnergy; // Heating Energy required to meet the load (J)
        Real64 CoolingEnergy; // Cooling Energy required to meet the load (J)

        // Default Constructor
        PlantProfileData()
            : Type(DataPlant::PlantEquipmentType::Invalid), plantLoc{}, Init(true), InitSizing(true), InletNode(0), InletTemp(0.0), OutletNode(0),
              OutletTemp(0.0), EMSOverridePower(false), EMSPowerValue(0.0), PeakVolFlowRate(0.0), VolFlowRate(0.0), MassFlowRate(0.0),
              EMSOverrideMassFlow(false), EMSMassFlowValue(0.0), Power(0.0), Energy(0.0), HeatingEnergy(0.0), CoolingEnergy(0.0)
        {
        }

        // Functions
        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void InitPlantProfile(EnergyPlusData &state);

        void UpdatePlantProfile(EnergyPlusData &state) const;

        void ReportPlantProfile(EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;

        void getCurrentPower(EnergyPlusData &state, Real64 &power) override;
    };

    // This could be static inside the class
    void GetPlantProfileInput(EnergyPlusData &state);

} // namespace PlantLoadProfile

struct PlantLoadProfileData : BaseGlobalStruct
{

    bool GetPlantLoadProfileInputFlag = true;
    int NumOfPlantProfile = 0;
    Array1D<PlantLoadProfile::PlantProfileData> PlantProfile;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetPlantLoadProfileInputFlag = true;
        this->NumOfPlantProfile = 0;
        this->PlantProfile.deallocate();
    }
};

} // namespace EnergyPlus

#endif
