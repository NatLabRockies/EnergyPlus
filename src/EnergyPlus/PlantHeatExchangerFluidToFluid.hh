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

#ifndef PlantHeatExchangerFluidToFluid_hh_INCLUDED
#define PlantHeatExchangerFluidToFluid_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantHeatExchangerFluidToFluid {

    enum class FluidHXType
    {
        Invalid = -1,
        CrossFlowBothUnMixed,
        CrossFlowBothMixed,
        CrossFlowSupplyLoopMixedDemandLoopUnMixed,
        CrossFlowSupplyLoopUnMixedDemandLoopMixed,
        CounterFlow,
        ParallelFlow,
        Ideal,
        Num
    };

    enum class ControlType
    {
        Invalid = -1,
        UncontrolledOn,
        OperationSchemeModulated,
        OperationSchemeOnOff,
        HeatingSetPointModulated,
        HeatingSetPointOnOff,
        CoolingSetPointModulated,
        CoolingSetPointOnOff,
        DualDeadBandSetPointModulated,
        DualDeadBandSetPointOnOff,
        CoolingDifferentialOnOff,
        CoolingSetPointOnOffWithComponentOverride,
        TrackComponentOnOff,
        Num
    };

    enum class CtrlTempType
    {
        Invalid = -1,
        WetBulbTemperature,
        DryBulbTemperature,
        LoopTemperature,
        Num
    };

    enum class HXAction
    {
        Invalid = -1,
        HeatingSupplySideLoop,
        CoolingSupplySideLoop,
        Num
    };

    struct PlantConnectionStruct : PlantLocation
    {
        // Members
        int inletNodeNum;                      // plant loop inlet node index
        int outletNodeNum;                     // plant loop outlet node index
        Real64 MassFlowRateMin;                // minimum (hardware) flow rate for component [kg/s]
        Real64 MassFlowRateMax;                // maximum (hardware) flow rate for component [kg/s]
        Real64 DesignVolumeFlowRate;           // design flow rate [m3/s]
        bool DesignVolumeFlowRateWasAutoSized; // true if design flow rate was autosize on input
        Real64 MyLoad;                         // current load request of supply equip for op scheme control[W]
        Real64 MinLoad;                        // reports back size for load dispatch routines [W]
        Real64 MaxLoad;                        // reports back size for load dispatch [W]
        Real64 OptLoad;                        // reports back size for load dispatch [W]
        Real64 InletTemp;                      // current inlet fluid temperature [C]
        Real64 InletMassFlowRate;              // current inlet mass flow rate [kg/s]
        Real64 OutletTemp;                     // component outlet temperature [C]

        // Default Constructor
        PlantConnectionStruct()
            : inletNodeNum(0), outletNodeNum(0), MassFlowRateMin(0.0), MassFlowRateMax(0.0), DesignVolumeFlowRate(0.0),
              DesignVolumeFlowRateWasAutoSized(false), MyLoad(0.0), MinLoad(0.0), MaxLoad(0.0), OptLoad(0.0), InletTemp(0.0), InletMassFlowRate(0.0),
              OutletTemp(0.0)
        {
        }
    };

    struct PlantLocatorStruct : PlantLocation
    {
        // Members
        int inletNodeNum; // plant loop inlet node index

        // Default Constructor
        PlantLocatorStruct() : inletNodeNum(0)
        {
        }
    };

    struct HeatExchangerStruct : PlantComponent
    {
        // Members
        std::string Name;
        Sched::Schedule *availSched = nullptr;
        FluidHXType HeatExchangeModelType;
        Real64 UA;
        bool UAWasAutoSized; // true is UA was autosized on input
        ControlType controlMode;
        int SetPointNodeNum;
        Real64 TempControlTol;
        CtrlTempType ControlSignalTemp;
        Real64 MinOperationTemp;
        Real64 MaxOperationTemp;
        PlantConnectionStruct DemandSideLoop; // plant connections and data for the side of HX connected to demand side
        PlantConnectionStruct SupplySideLoop;
        OutputProcessor::EndUseCat HeatTransferMeteringEndUse = OutputProcessor::EndUseCat::Invalid;
        std::string ComponentUserName; // user name for control-associated  component
        DataPlant::PlantEquipmentType ComponentType;
        PlantLocatorStruct OtherCompSupplySideLoop;
        PlantLocatorStruct OtherCompDemandSideLoop;
        Real64 SizingFactor;
        Real64 HeatTransferRate;
        Real64 HeatTransferEnergy;
        Real64 Effectiveness;
        Real64 OperationStatus;
        int DmdSideModulatSolvNoConvergeErrorCount;
        int DmdSideModulatSolvNoConvergeErrorIndex;
        int DmdSideModulatSolvFailErrorCount;
        int DmdSideModulatSolvFailErrorIndex;
        bool MyOneTimeFlag;
        bool MyFlag;
        bool MyEnvrnFlag;

        // Default Constructor
        HeatExchangerStruct()
            : HeatExchangeModelType(FluidHXType::Invalid), UA(0.0), UAWasAutoSized(false), controlMode(ControlType::Invalid), SetPointNodeNum(0),
              TempControlTol(0.0), ControlSignalTemp(CtrlTempType::Invalid), MinOperationTemp(-99999.0), MaxOperationTemp(99999.0),
              ComponentType(DataPlant::PlantEquipmentType::Invalid), SizingFactor(1.0), HeatTransferRate(0.0), HeatTransferEnergy(0.0),
              Effectiveness(0.0), OperationStatus(0.0), DmdSideModulatSolvNoConvergeErrorCount(0), DmdSideModulatSolvNoConvergeErrorIndex(0),
              DmdSideModulatSolvFailErrorCount(0), DmdSideModulatSolvFailErrorIndex(0), MyOneTimeFlag(true), MyFlag(true), MyEnvrnFlag(true)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void setupOutputVars(EnergyPlusData &state);

        void initialize(EnergyPlusData &state);

        void size(EnergyPlusData &state);

        void calculate(EnergyPlusData &state, Real64 SupSideMdot, Real64 DmdSideMdot);

        void control(EnergyPlusData &state, Real64 MyLoad, bool FirstHVACIteration);

        void findDemandSideLoopFlow(EnergyPlusData &state, Real64 TargetSupplySideLoopLeavingTemp, HXAction HXActionMode);

        void oneTimeInit(EnergyPlusData &state) override;

        void updateCompFlowData(EnergyPlusData &state);

        bool hasSupplySideTES(EnergyPlusData &state);
    };

    void GetFluidHeatExchangerInput(EnergyPlusData &state);

} // namespace PlantHeatExchangerFluidToFluid

struct PlantHeatExchangerFluidToFluidData : BaseGlobalStruct
{

    int NumberOfPlantFluidHXs = 0;
    bool GetInput = true;
    EPVector<PlantHeatExchangerFluidToFluid::HeatExchangerStruct> FluidHX;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumberOfPlantFluidHXs = 0;
        this->GetInput = true;
        this->FluidHX.deallocate();
    }
};

} // namespace EnergyPlus

#endif
