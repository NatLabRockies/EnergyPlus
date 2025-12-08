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

#ifndef ChillerElectricASHRAE205_hh_INCLUDED
#define ChillerElectricASHRAE205_hh_INCLUDED

#include <ObjexxFCL/Array1D.hh>

#include "EnergyPlus/ChillerElectricEIR.hh"
#include "rs0001.h"
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace ChillerElectricASHRAE205 {

    enum class AmbientTempIndicator
    {
        Invalid = -1,
        Schedule,   // ambient temperature around tank (or HPWH inlet air) is scheduled
        TempZone,   // tank is located in a zone or HPWH inlet air is zone air only
        OutsideAir, // tank is located outdoors or HPWH inlet air is outdoor air only
        ZoneAndOA,  // applicable to HPWH only, inlet air is mixture of OA and zone air
        Num,
    };

    void getChillerASHRAE205Input(EnergyPlusData &state);

    struct ASHRAE205ChillerSpecs : ChillerElectricEIR::ElectricEIRChillerSpecs
    {
        static constexpr std::string_view ObjectType{"Chiller:Electric:ASHRAE205"};
        std::shared_ptr<tk205::rs0001_ns::RS0001> Representation; // ASHRAE205 representation instance
        std::pair<EnergyPlusData *, std::string> LoggerContext;

        Btwxt::InterpolationMethod InterpolationType{Btwxt::InterpolationMethod::linear};
        int MinSequenceNumber{1};
        int MaxSequenceNumber{1};

        int OilCoolerInletNode{0};
        int OilCoolerOutletNode{0};
        Real64 OilCoolerVolFlowRate{0};
        Real64 OilCoolerMassFlowRate{0};
        PlantLocation OCPlantLoc{0, DataPlant::LoopSideLocation::Invalid, 0, 0};
        int AuxiliaryHeatInletNode{0};
        int AuxiliaryHeatOutletNode{0};
        Real64 AuxiliaryVolFlowRate{0};
        Real64 AuxiliaryMassFlowRate{0};
        PlantLocation AHPlantLoc{0, DataPlant::LoopSideLocation::Invalid, 0, 0};
        Real64 QOilCooler{0};
        Real64 QAuxiliary{0};
        Real64 OilCoolerEnergy{0};
        Real64 AuxiliaryEnergy{0};

        AmbientTempIndicator AmbientTempType{AmbientTempIndicator::Invalid};
        Sched::Schedule *ambientTempSched = nullptr;
        int AmbientTempZone{0};           // Number of ambient zone around tank
        int AmbientTempOutsideAirNode{0}; // Number of outside air node
        Real64 AmbientTemp{0};
        Real64 AmbientZoneGain{0};       // Internal gain to zone from losses (W)
        Real64 AmbientZoneGainEnergy{0}; // Internal gain to zone from losses (J)
        std::string EndUseSubcategory;   // identifier use for the end use subcategory

        // Default Constructor
        ASHRAE205ChillerSpecs() = default;

        void setOutputVariables(EnergyPlusData &state);

        static ASHRAE205ChillerSpecs *factory(EnergyPlusData &state, std::string const &chillerName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad) override;

        void size(EnergyPlusData &state) override;

        void findEvaporatorMassFlowRate(EnergyPlusData &state, Real64 &load, Real64 Cp);

        void calculate(EnergyPlusData &state,
                       Real64 &MyLoad, // operating load
                       bool RunFlag    // TRUE when chiller operating
                       ) override;

        void update(EnergyPlusData &state,
                    Real64 MyLoad, // current load
                    bool RunFlag   // TRUE if chiller operating
                    ) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

} // namespace ChillerElectricASHRAE205

struct ChillerElectricASHRAE205Data : BaseGlobalStruct
{
    bool getInputFlag = true;
    Array1D<ChillerElectricASHRAE205::ASHRAE205ChillerSpecs> Electric205Chiller;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ChillerElectricASHRAE205Data();
    }
};

} // namespace EnergyPlus
#endif // ChillerElectricASHRAE205_hh_INCLUDED
