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

#ifndef DataGenerators_hh_INCLUDED
#define DataGenerators_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

namespace DataGenerators {

    // MODULE PARAMETER DEFINITIONS:
    enum class CurveMode
    {
        Invalid = -1,
        Normalized, // mode where efficiency curves are modifier curves
        Direct,     // mode where efficiency curves are direct
        Num
    };

    enum class SkinLoss
    {
        Invalid = -1,
        ConstantRate,
        UADT,              // UAdelta T mode for skin losses
        QuadraticFuelNdot, // Quadratic function of fuel flow for skin losses
        Num
    };

    enum class AirSupRateMode
    {
        Invalid = -1,
        QuadraticFuncofNdot,  // function of fuel rate mode for air flow
        ConstantStoicsAirRat, // Constant air ratio in stoics with fuel constituents
        QuadraticFuncofPel,   // function of electric power mode
        Num
    };

    enum class RecoverMode
    {
        Invalid = -1,
        NoRecoveryOnAirIntake,  // mode for controlling intake air heat recovery
        RecoverBurnInvertBatt,  // mode for controlling intake air heat recovery
        RecoverAuxiliaryBurner, // mode for controlling intake air heat recovery
        RecoverInverterBatt,    // mode for controlling intake air heat recovery
        RecoverInverter,        // mode for controlling intake air heat recovery
        RecoverBattery,         // mode for controlling intake air heat recovery
        Num
    };

    enum class ConstituentMode
    {
        Invalid = -1,
        RegularAir,
        UserDefinedConstituents,
        Num
    };

    enum class FuelTemperatureMode
    {
        Invalid = -1,
        FuelInTempFromNode,
        FuelInTempSchedule,
        Num
    };

    enum class WaterTempMode
    {
        Invalid = -1,
        Mains,
        AirNode,
        WaterNode,
        Schedule,
        Num
    };

    enum class InverterEfficiencyMode
    {
        Invalid = -1,
        Constant,
        Quadratic,
        Num
    };

    enum class ExhaustGasHX
    {
        Invalid = -1,
        FixedEffectiveness,   // exhaust gas HX modeling mode
        LMTDempiricalUAeff,   // exhaust gas HX modeling mode
        LMTDfundementalUAeff, // exhaust gas HX modeling mode
        Condensing,           // exhaust gas HX modeling mode
        Num
    };

    enum class ElectricalStorage
    {
        Invalid = -1,
        SimpleEffConstraints,         // electrical storage modeling mode
        LeadAcidBatterySaupe,         // electrical storage modeling mode
        LeadAcidBatterManwellMcGowan, // electrical storage modeling mode
        Num
    };

    enum class LossDestination
    {
        Invalid = -1,
        SurroundingZone,
        AirInletForFC,
        Num
    };

    enum class OperatingMode
    {
        Invalid = -1,
        Off,      // CHP operating mode OFF
        Standby,  // CHP operating mode Stand By
        WarmUp,   // CHP operating mode Warm Up or start up
        Normal,   // CHP operating mode Normal
        CoolDown, // CHP operating mode Cool down or shut down
        Num
    };

    enum class FuelMode
    {
        Invalid = -1,
        GaseousConstituents,
        GenericLiquid,
        Num
    };

    Real64 constexpr MinProductGasTemp(-100.0); // Minimum bound on search for product gas temps
    Real64 constexpr MaxProductGasTemp(2000.0); // Maximum bound on search for product gas temps

    enum class ThermodynamicMode
    {
        Invalid = -1,
        NISTShomate,
        NASAPolynomial,
        Num
    };

    Real64 constexpr RinKJperMolpK(0.0083145); // R is ideal gas constant (kJ/mol-K)
    Real64 constexpr InitHRTemp(50.0);         // Initialization temperature for heat recovery water
    Real64 constexpr ImBalanceTol(0.00001);    // used as fraction of electrical power at power module

    struct GeneratorFuelSupplyDataStruct
    {
        // Members
        // user input data
        std::string Name;                                                                                // name of this fuel supply module
        DataGenerators::FuelTemperatureMode FuelTempMode = DataGenerators::FuelTemperatureMode::Invalid; // temperature of fuel node
        DataGenerators::FuelMode FuelTypeMode = DataGenerators::FuelMode::Invalid;                       // type of fuel, gasous or liquid
        std::string NodeName;                                                                            // node name for temperature at input
        int NodeNum = 0;                                                                                 // node number for temperature at input
        Sched::Schedule *sched = nullptr;                                                                // fuel temperature at input
        int CompPowerCurveID = 0;                                                                        // "pointer" to compressor power cubic curve
        Real64 CompPowerLossFactor = 0.0;
        int NumConstituents = 0; // number of constituents in fue supply
        Array1D_string ConstitName;
        Array1D<Real64> ConstitMolalFract;
        // calculated data (except some for generic liquid)
        Array1D_int GasLibID;              // lookup ID in Gas Phase ThermoChemistry Structure Array
        Real64 LHV = 0.0;                  // lower heating value of gaseous fuel (kJ/mol)
        Real64 LHVJperkg = 0.0;            // lower heating value of gaseous fuel (J/kg)
        Real64 LHVliquid = 0.0;            // userdefined lhv for generic liquid (J/kg)
        Real64 HHV = 0.0;                  // higher heating value of fuel (J/kg)
        Real64 MW = 0.0;                   // molecular weight g/mol
        Real64 eCO2 = 0.0;                 // mass flow based CO2 emissions factor for complete combustion (-)
        Real64 KmolPerSecToKgPerSec = 0.0; // conversion from moles to kilograms for this fuel. (
        Real64 StoicOxygenRate = 0.0;
        Real64 TfuelIntoCompress = 0.0; // inlet fuel temperature
        Real64 TfuelIntoFCPM = 0.0;     // compressed fuel temp
        Real64 PfuelCompEl = 0.0;       // fuel compressor power
        Real64 QskinLoss = 0.0;         // pumping losses for zone
        Real64 CO2ProductGasCoef = 0.0; // molar multiplier for stoic products of this fuel
        Real64 H2OProductGasCoef = 0.0; // molar multiplier for stoic products of this fuel

        // Default Constructor
        GeneratorFuelSupplyDataStruct() : ConstitName(14), ConstitMolalFract(14, 0.0), GasLibID(14, 0)
        {
        }
    };

    struct GasPropertyDataStruct
    {
        // Members
        std::string ConstituentName;
        std::string ConstituentFormula;
        Real64 StdRefMolarEnthOfForm = 0.0;
        DataGenerators::ThermodynamicMode ThermoMode = DataGenerators::ThermodynamicMode::Invalid; // method of calculation for thermodynamics
        Real64 ShomateA = 0.0;
        Real64 ShomateB = 0.0;
        Real64 ShomateC = 0.0;
        Real64 ShomateD = 0.0;
        Real64 ShomateE = 0.0;
        Real64 ShomateF = 0.0;
        Real64 ShomateG = 0.0;
        Real64 ShomateH = 0.0;
        Real64 NumCarbons = 0.0;
        Real64 NumHydrogens = 0.0;
        Real64 NumOxygens = 0.0;
        Real64 MolecularWeight = 0.0;
        Real64 NASA_A1 = 0.0;
        Real64 NASA_A2 = 0.0;
        Real64 NASA_A3 = 0.0;
        Real64 NASA_A4 = 0.0;
        Real64 NASA_A5 = 0.0;
        Real64 NASA_A6 = 0.0;
        Real64 NASA_A7 = 0.0;
    };

    struct GeneratorDynamicsManagerStruct
    {
        // Members
        // user input data
        std::string Name;
        Real64 PelMin = 0.0;              // minimum operating point for electrical power Pel
        Real64 PelMax = 0.0;              // maximum operating point for electrical power Pel
        Real64 UpTranLimit = 0.0;         // power up transient limit W/s
        Real64 DownTranLimit = 0.0;       // power down tran limit  W/s
        Real64 UpTranLimitFuel = 0.0;     // fuel up transient limit kg/s
        Real64 DownTranLimitFuel = 0.0;   // fuel down transient limit kg/s
        bool WarmUpByTimeDelay = false;   // Warm up mode control
        bool WarmUpByEngineTemp = true;   // Warm up mode control
        Real64 StartUpTimeDelay = 0.0;    // time for start up [hours]
        Real64 WarmUpDelay = 0.0;         // time for warm up delay [s]
        Real64 StartUpFuel = 0.0;         // fuel use during start up
        Real64 StartUpElectConsum = 0.0;  // electricity used during start up
        Real64 StartUpElectProd = 0.0;    // electricity produced during start up
        Real64 ShutDownFuel = 0.0;        // fuel consumed during shut down
        Real64 ShutDownElectConsum = 0.0; // Elect consumed during shut down
        Real64 PcoolDown = 0.0;           // power during cool down
        Real64 CoolDownDelay = 0.0;       // time for cool down delay [hours]
        int NumCyclesInit = 0;            // number of start stop cycles at beginning
        Real64 NumRunHoursInit = 0.0;     // number of hours of operation beginning
        Real64 Pstandby = 0.0;            // standby power [w]
        Real64 MCeng = 0.0;               // aggregated thermal mass of engine [  ]
        Real64 MCcw = 0.0;                // aggregated thermal mass of heat recovery [   ]
        Real64 kf = 0.0;                  // coefficient k_f for warmup fuel flow rate
        Real64 TnomEngOp = 0.0;           // nominal engine operating temperature [C]
        Real64 kp = 0.0;                  // coefficient k_p for warmup power
        bool MandatoryFullCoolDown = false;
        bool WarmRestartOkay = true;
        Sched::Schedule *availSched = nullptr;
        // Calculated values and input from elsewhere
        DataGenerators::OperatingMode CurrentOpMode = DataGenerators::OperatingMode::Off; // current operating mode, uses params like OpModeNormal
        DataGenerators::OperatingMode LastOpMode = DataGenerators::OperatingMode::Off;
        Real64 FractionalDayofLastShutDown = 0.0;
        Real64 FractionalDayofLastStartUp = 0.0;
        bool HasBeenOn = false;
        bool DuringStartUp = false;
        bool DuringShutDown = false;
        Real64 FuelMdotLastTimestep = 0.0;
        Real64 PelLastTimeStep = 0.0;
        int NumCycles = 0;
        Real64 PLRforSubtimestepStartUp = 0.0;
        Real64 PLRforSubtimestepShutDown = 0.0; // part load for not in shut down, shut down part is (1 - PLR)
        Real64 ElectEffNom = 0.0;               // efficiency to use for control decisions
        Real64 ThermEffNom = 0.0;               // thermal efficiency to use fo control decisions
        Real64 QdotHXMax = 0.0;                 // Thermal power max
        Real64 QdotHXMin = 0.0;                 // thermal power min
        Real64 QdotHXOpt = 0.0;                 // thermal power nominal/optimal
    };

} // namespace DataGenerators

struct GeneratorsData : BaseGlobalStruct
{
    Array1D<DataGenerators::GasPropertyDataStruct> GasPhaseThermoChemistryData;
    Array1D<DataGenerators::GeneratorFuelSupplyDataStruct> FuelSupply; // fuel supply (reused across various)
    Array1D<DataGenerators::GeneratorDynamicsManagerStruct> GeneratorDynamics;

    int InletCWnode = 0; // cooling water inlet node ID
    bool InternalFlowControl = false;
    Real64 TcwIn = 0.0;          // inlet cooling water temperature (C)
    Real64 TrialMdotcw = 0.0;    // test or estimate of what the plant flows are going to be (kg/s)
    Real64 LimitMinMdotcw = 0.0; // lower limit for cooling water flow for generatior operation (kg/s)

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) GeneratorsData();
    }
};

} // namespace EnergyPlus

#endif
