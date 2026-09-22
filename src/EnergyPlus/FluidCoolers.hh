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

#ifndef FluidCoolers_hh_INCLUDED
#define FluidCoolers_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

namespace FluidCoolers {

    enum class PerfInputMethod
    {
        Invalid = -1,
        NOMINAL_CAPACITY,
        U_FACTOR,
        Num
    };

    struct FluidCoolerspecs : PlantComponent
    {
        // Members
        std::string Name; // User identifier
        DataPlant::PlantEquipmentType FluidCoolerType;
        PerfInputMethod PerformanceInputMethod_Num;
        bool Available;                               // need an array of logicals--load identifiers of available equipment
        bool ON;                                      // Simulate the machine at it's operating part load ratio
        Real64 DesignWaterFlowRate;                   // Design water flow rate through the fluid cooler [m3/s]
        bool DesignWaterFlowRateWasAutoSized;         // true if design water rate was autosize on input
        Real64 DesWaterMassFlowRate;                  // Design water flow rate through the fluid cooler [kg/s]
        Real64 HighSpeedAirFlowRate;                  // Air flow rate through the fluid cooler at high speed [m3/s]
        bool HighSpeedAirFlowRateWasAutoSized;        // true if high speed air rate was autosize on input
        Real64 HighSpeedFanPower;                     // Fan power at high fan speed [W]
        bool HighSpeedFanPowerWasAutoSized;           // true if high fan power was autosize on input
        Real64 HighSpeedFluidCoolerUA;                // UA of fluid cooler at high fan speed [W/C]
        bool HighSpeedFluidCoolerUAWasAutoSized;      // true if high speed UA was autosized on input
        Real64 LowSpeedAirFlowRate;                   // Air flow rate through fluid cooler at low speed [m3/s]
        bool LowSpeedAirFlowRateWasAutoSized;         // true if low speed air rate was autosize on input
        Real64 LowSpeedAirFlowRateSizingFactor;       // sizing factor for low speed air flow rate []
        Real64 LowSpeedFanPower;                      // Fan power at low fan speed [W]
        bool LowSpeedFanPowerWasAutoSized;            // true if low speed fan power set to autosize on input
        Real64 LowSpeedFanPowerSizingFactor;          // sizing factor for low speed fan power []
        Real64 LowSpeedFluidCoolerUA;                 // UA of fluid cooler at low fan speed [W/C]
        bool LowSpeedFluidCoolerUAWasAutoSized;       // true if low speed UA set to autosize on input
        Real64 LowSpeedFluidCoolerUASizingFactor;     // sizing factor for low speed UA []
        Real64 DesignEnteringWaterTemp;               // Entering water temperature at design conditions
        Real64 DesignLeavingWaterTemp;                // Entering water temperature at design conditions
        Real64 DesignEnteringAirTemp;                 // Entering water temperature at design conditions
        Real64 DesignEnteringAirWetBulbTemp;          // Entering water temperature at design condition
        Real64 FluidCoolerMassFlowRateMultiplier;     // Maximum fluid cooler flow rate is this multiplier * design flow rate
        Real64 FluidCoolerNominalCapacity;            // Nominal capacity of the fluid cooler [W] at high speed
        Real64 FluidCoolerLowSpeedNomCap;             // Nominal capacity of the fluid cooler [W] at low speed
        bool FluidCoolerLowSpeedNomCapWasAutoSized;   // true if previous was set to autosize on input
        Real64 FluidCoolerLowSpeedNomCapSizingFactor; // sizing factor for low speed capacity []
        int WaterInletNodeNum;                        // Node number on the water inlet side of the fluid cooler
        int WaterOutletNodeNum;                       // Node number on the water outlet side of the fluid cooler
        int OutdoorAirInletNodeNum;                   // Node number of outdoor air inlet for the fluid cooler
        int HighMassFlowErrorCount;                   // Counter when mass flow rate is > Design*FluidCoolerMassFlowRateMultiplier
        int HighMassFlowErrorIndex;                   // Index for high mass flow recurring error message
        int OutletWaterTempErrorCount;                // Counter when outlet water temperature is < minimum allowed temperature
        int OutletWaterTempErrorIndex;                // Index for outlet water temperature recurring error message
        int SmallWaterMassFlowErrorCount;             // Counter when water mass flow rate is very small
        int SmallWaterMassFlowErrorIndex;             // Index for very small water mass flow rate recurring error message
        int WMFRLessThanMinAvailErrCount;             // Counter when water mass flow rate is less than minimum available
        int WMFRLessThanMinAvailErrIndex;             // Index for water mass flow rate less than minavail recurring message
        int WMFRGreaterThanMaxAvailErrCount;          // Counter when water mass flow rate is greater than minimum available
        int WMFRGreaterThanMaxAvailErrIndex;          // Index for water mass flow rate > minavail recurring message
        PlantLocation plantLoc;                       // loop topology variables
        // bool oneTimeInitFlag; // Defined in PlantComponent.hh
        bool beginEnvrnInit;

        // Report vars
        Real64 InletWaterTemp;    // Fluid cooler inlet water temperature (C)
        Real64 OutletWaterTemp;   // Fluid cooler outlet water temperature (C)
        Real64 WaterMassFlowRate; // Fluid cooler water mass flow rate (m3/s)
        Real64 Qactual;           // Fluid cooler heat rejection rate (W)
        Real64 FanPower;          // Fluid cooler fan power (W)
        Real64 FanEnergy;         // Fluid cooler fan energy consumption (J)

        // Inlet conds
        Real64 WaterTemp;
        Real64 AirTemp;
        Real64 AirHumRat;
        Real64 AirPress;
        Real64 AirWetBulb;

        // additional stuff
        int indexInArray;

        // Default Constructor
        FluidCoolerspecs()
            : FluidCoolerType(DataPlant::PlantEquipmentType::Invalid), PerformanceInputMethod_Num(PerfInputMethod::NOMINAL_CAPACITY), Available(true),
              ON(true), DesignWaterFlowRate(0.0), DesignWaterFlowRateWasAutoSized(false), DesWaterMassFlowRate(0.0), HighSpeedAirFlowRate(0.0),
              HighSpeedAirFlowRateWasAutoSized(false), HighSpeedFanPower(0.0), HighSpeedFanPowerWasAutoSized(false), HighSpeedFluidCoolerUA(0.0),
              HighSpeedFluidCoolerUAWasAutoSized(false), LowSpeedAirFlowRate(0.0), LowSpeedAirFlowRateWasAutoSized(false),
              LowSpeedAirFlowRateSizingFactor(0.0), LowSpeedFanPower(0.0), LowSpeedFanPowerWasAutoSized(false), LowSpeedFanPowerSizingFactor(0.0),
              LowSpeedFluidCoolerUA(0.0), LowSpeedFluidCoolerUAWasAutoSized(false), LowSpeedFluidCoolerUASizingFactor(0.0),
              DesignEnteringWaterTemp(0.0), DesignLeavingWaterTemp(0.0), DesignEnteringAirTemp(0.0), DesignEnteringAirWetBulbTemp(0.0),
              FluidCoolerMassFlowRateMultiplier(0.0), FluidCoolerNominalCapacity(0.0), FluidCoolerLowSpeedNomCap(0.0),
              FluidCoolerLowSpeedNomCapWasAutoSized(false), FluidCoolerLowSpeedNomCapSizingFactor(0.0), WaterInletNodeNum(0), WaterOutletNodeNum(0),
              OutdoorAirInletNodeNum(0), HighMassFlowErrorCount(0), HighMassFlowErrorIndex(0), OutletWaterTempErrorCount(0),
              OutletWaterTempErrorIndex(0), SmallWaterMassFlowErrorCount(0), SmallWaterMassFlowErrorIndex(0), WMFRLessThanMinAvailErrCount(0),
              WMFRLessThanMinAvailErrIndex(0), WMFRGreaterThanMaxAvailErrCount(0), WMFRGreaterThanMaxAvailErrIndex(0), plantLoc{},
              beginEnvrnInit(true), InletWaterTemp(0.0), OutletWaterTemp(0.0), WaterMassFlowRate(0.0), Qactual(0.0), FanPower(0.0), FanEnergy(0.0),
              WaterTemp(0.0), AirTemp(0.0), AirHumRat(0.0), AirPress(0.0), AirWetBulb(0.0), indexInArray(0)
        {
        }

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;

        void initEachEnvironment(EnergyPlusData &state);

        void initialize(EnergyPlusData &state);

        void setupOutputVars(EnergyPlusData &state);

        void size(EnergyPlusData &state);

        void update(EnergyPlusData &state);

        void report(EnergyPlusData &state, bool RunFlag);

        bool validateSingleSpeedInputs(EnergyPlusData &state,
                                       std::string const &cCurrentModuleObject,
                                       Array1D<std::string> const &AlphArray,
                                       Array1D<std::string> const &cNumericFieldNames,
                                       Array1D<std::string> const &cAlphaFieldNames);

        bool validateTwoSpeedInputs(EnergyPlusData &state,
                                    std::string const &cCurrentModuleObject,
                                    Array1D<std::string> const &AlphArray,
                                    Array1D<std::string> const &cNumericFieldNames,
                                    Array1D<std::string> const &cAlphaFieldNames);

        void calcSingleSpeed(EnergyPlusData &state);

        void calcTwoSpeed(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 [[maybe_unused]] Real64 &MaxLoad,
                                 [[maybe_unused]] Real64 &MinLoad,
                                 [[maybe_unused]] Real64 &OptLoad) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        static FluidCoolerspecs *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType typeOf, std::string const &objectName);
    };

    void GetFluidCoolerInput(EnergyPlusData &state);

    void CalcFluidCoolerOutlet(
        EnergyPlusData &state, int FluidCoolerNum, Real64 _WaterMassFlowRate, Real64 AirFlowRate, Real64 UAdesign, Real64 &_OutletWaterTemp);

} // namespace FluidCoolers

struct FluidCoolersData : BaseGlobalStruct
{

    bool GetFluidCoolerInputFlag = true;
    int NumSimpleFluidCoolers = 0;
    Array1D<FluidCoolers::FluidCoolerspecs> SimpleFluidCooler;
    std::unordered_map<std::string, std::string> UniqueSimpleFluidCoolerNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetFluidCoolerInputFlag = true;
        this->NumSimpleFluidCoolers = 0;
        this->SimpleFluidCooler.deallocate();
        this->UniqueSimpleFluidCoolerNames.clear();
    }
};

} // namespace EnergyPlus

#endif
