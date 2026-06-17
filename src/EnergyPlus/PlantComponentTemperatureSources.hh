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

#ifndef PlantComponentTemperatureSources_hh_INCLUDED
#define PlantComponentTemperatureSources_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantComponentTemperatureSources {

    // MODULE PARAMETER DEFINITIONS:
    enum class TempSpecType
    {
        Invalid = -1,
        Constant,
        Schedule,
        Num
    };

    struct WaterSourceSpecs : PlantComponent
    {
        // Members
        std::string Name;                       // user identifier
        int InletNodeNum;                       // Node number on the inlet side of the plant
        int OutletNodeNum;                      // Node number on the outlet side of the plant
        Real64 DesVolFlowRate;                  // m**3/s - design nominal volumetric flow rate
        bool DesVolFlowRateWasAutoSized;        // true if design flow rate was autosized on input
        Real64 MassFlowRateMax;                 // kg/s - design mass flow rate
        bool EMSOverrideOnMassFlowRateMax;      // if true EMS is calling to override maximum mass flow
        Real64 EMSOverrideValueMassFlowRateMax; // value to use if EMS is overriding max mass flow
        Real64 MassFlowRate;
        TempSpecType tempSpecType; // temperature specification type
        Sched::Schedule *tempSpecSched = nullptr;
        Real64 BoundaryTemp;
        Real64 OutletTemp; // may be different if the flow is off
        Real64 InletTemp;
        Real64 HeatRate;
        Real64 HeatEnergy;
        PlantLocation plantLoc;
        Real64 SizFac; // sizing factor
        bool CheckEquipName;
        bool MyFlag;
        bool MyEnvironFlag;
        bool IsThisSized; // TRUE if sizing is done

        // Default Constructor
        WaterSourceSpecs()
            : InletNodeNum(0), OutletNodeNum(0), DesVolFlowRate(0.0), DesVolFlowRateWasAutoSized(false), MassFlowRateMax(0.0),
              EMSOverrideOnMassFlowRateMax(false), EMSOverrideValueMassFlowRateMax(0.0), MassFlowRate(0.0), tempSpecType(TempSpecType::Invalid),
              BoundaryTemp(0.0), OutletTemp(0.0), InletTemp(0.0), HeatRate(0.0), HeatEnergy(0.0), plantLoc{}, SizFac(0.0), CheckEquipName(true),
              MyFlag(true), MyEnvironFlag(true), IsThisSized(false)
        {
        }

        // Destructor
        ~WaterSourceSpecs() = default;

        void initialize(EnergyPlusData &state, Real64 &MyLoad);

        void setupOutputVars(EnergyPlusData &state);

        void autosize(EnergyPlusData &state);

        void calculate(EnergyPlusData &state);

        void update(EnergyPlusData &state);

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 Real64 &MaxLoad,
                                 Real64 &MinLoad,
                                 Real64 &OptLoad) override;

        void getSizingFactor(Real64 &_SizFac) override;

        void onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetWaterSourceInput(EnergyPlusData &state);

} // namespace PlantComponentTemperatureSources

struct PlantCompTempSrcData : BaseGlobalStruct
{

    int NumSources = 0;
    bool getWaterSourceInput = true;
    EPVector<PlantComponentTemperatureSources::WaterSourceSpecs> WaterSource;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumSources = 0;
        this->getWaterSourceInput = true;
        this->WaterSource.deallocate();
    }
};

} // namespace EnergyPlus

#endif
