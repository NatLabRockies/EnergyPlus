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

#ifndef FuelCellElectricGenerator_hh_INCLUDED
#define FuelCellElectricGenerator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
namespace Curve {
    struct Curve;
}

namespace FuelCellElectricGenerator {

    enum class GasID
    // This enum starts at 1 because it has to match Gas Phase ThermoChemistry Structure Array in DataGenerators & GeneratorFuelSupply
    {
        Invalid = -1,
        CarbonDioxide = 1,
        Nitrogen,
        Oxygen,
        Water,
        Argon,
        Num
    };

    struct FCPowerModuleStruct
    {
        std::string Name;                      // name of this PowerModule data
        DataGenerators::CurveMode EffMode;     // mode for efficiency curves
        Curve::Curve *EffCurve = nullptr;      // pointer to curve for efficiency
        Real64 NomEff;                         // nominal efficiency
        Real64 NomPel;                         // nominal power rate at rating point
        int NumCyclesAtStart = 0.0;            // number of start stop cycles at beginning of simulation (user input)
        int NumCycles;                         // number of start stop cycles
        Real64 CyclingDegradRat;               // rate of degradation from cycles
        Real64 NumRunHours;                    // number of hours of operation
        Real64 OperateDegradRat;               // rate of degradation from run time (per hour)
        Real64 ThreshRunHours;                 // number of hours before degradation starts
        Real64 UpTranLimit;                    // power up transient limit
        Real64 DownTranLimit;                  // power down tran limit
        Real64 StartUpTime;                    // time for start up [hours]
        Real64 StartUpFuel;                    // fuel use during start up
        Real64 StartUpElectConsum;             // electricity used during start up
        Real64 StartUpElectProd;               // electricity produced during start up
        Real64 ShutDownTime;                   // time to shut down [hours]
        Real64 ShutDownFuel;                   // fuel consumed during shut down
        Real64 ShutDownElectConsum;            // Elect consumed during shut down
        Real64 ANC0;                           // Ancillary Loads constant term
        Real64 ANC1;                           // Ancillary Loads linear term
        DataGenerators::SkinLoss SkinLossMode; // how are skin losses determined
        std::string ZoneName;
        int ZoneID; // "pointer" to zone with component in it
        Real64 RadiativeFract;
        Real64 QdotSkin;
        Real64 UAskin;
        Curve::Curve *SkinLossCurve = nullptr;
        Curve::Curve *WaterSupplyCurve = nullptr; // pointer to curve for water use in reforming
        Real64 NdotDilutionAir;                   // user defined constant flow of dilution air (kmol/sec)
        Real64 StackHeatLossToDilution;           // (watts)
        std::string DilutionInletNodeName;        // dilution -> AirHR ?? added air heat recovery path
        int DilutionInletNode;                    // pointer to node for inlet
        std::string DilutionExhaustNodeName;
        int DilutionExhaustNode; // pointer to node getting exhaust
        Real64 PelMin;           // minimum operating point for FCPM electrical power Pel
        Real64 PelMax;           // maximum operating point for FCPM electrical power Pel
        // Calculated values and input from elsewhere
        Real64 Pel; // current DC electrical power produced
        Real64 PelLastTimeStep;
        Real64 Eel;                         // power module efficiency
        Real64 QdotStackCool;               // Heat removed by stack cooler
        Real64 FractionalDayofLastStartUp;  // fractional days into simulation
        Real64 FractionalDayofLastShutDown; // fractional Days into simulations
        bool HasBeenOn;
        bool DuringShutDown;
        bool DuringStartUp;
        Real64 NdotFuel;          // molar fuel use rate.  (kmol/sec)
        Real64 TotFuelInEnthalpy; // Enthalpy of fuel coming into FCPM (watts)
        Real64 NdotProdGas;       // (kmol/sec)
        Array1D<Real64> ConstitMolalFract;
        Array1D<GasID> GasLibID; // lookup ID in Gas Phase ThermoChemistry Structure Array
        Real64 TprodGasLeavingFCPM;
        Real64 NdotAir;          // molar air use rate    (kmol/sec)
        Real64 TotAirInEnthalpy; // Enthalpy of air coming nto FCPM energy balance (watts)
        Real64 NdotLiqwater;     // molar water use rate (kmol/sec)
        Real64 TwaterInlet;
        Real64 WaterInEnthalpy;       // Enthalpy of liquid water used for reforming (watts)
        Real64 DilutionAirInEnthalpy; // Enthalpy of Dilution air coming into FCPM (watts)
        Real64 DilutionAirOutEnthalpy;
        Real64 PelancillariesAC;   // ancillary power (watts)
        Real64 TotProdGasEnthalpy; // Enthalpy of product gases leaving FCPM   (watts)
        Real64 WaterOutEnthalpy;   // enthalpy of vapor from water used for reforming
        int SeqSubstitIter;
        int RegulaFalsiIter;

        // Default Constructor
        FCPowerModuleStruct()
            : EffMode(DataGenerators::CurveMode::Invalid), NomEff(0.0), NomPel(0.0), NumCycles(0), CyclingDegradRat(0.0), NumRunHours(0.0),
              OperateDegradRat(0.0), ThreshRunHours(0.0), UpTranLimit(0.0), DownTranLimit(0.0), StartUpTime(0.0), StartUpFuel(0.0),
              StartUpElectConsum(0.0), StartUpElectProd(0.0), ShutDownTime(0.0), ShutDownFuel(0.0), ShutDownElectConsum(0.0), ANC0(0.0), ANC1(0.0),
              SkinLossMode(DataGenerators::SkinLoss::Invalid), ZoneID(0), RadiativeFract(0.0), QdotSkin(0.0), UAskin(0.0), NdotDilutionAir(0.0),
              StackHeatLossToDilution(0.0), DilutionInletNode(0), DilutionExhaustNode(0), PelMin(0.0), PelMax(0.0), Pel(0.0), PelLastTimeStep(0.0),
              Eel(0.0), QdotStackCool(0.0), FractionalDayofLastStartUp(0.0), FractionalDayofLastShutDown(0.0), HasBeenOn(true), DuringShutDown(false),
              DuringStartUp(false), NdotFuel(0.0), TotFuelInEnthalpy(0.0), NdotProdGas(0.0), ConstitMolalFract(14, 0.0), GasLibID(14, GasID::Invalid),
              TprodGasLeavingFCPM(0.0), NdotAir(0.0), TotAirInEnthalpy(0.0), NdotLiqwater(0.0), TwaterInlet(0.0), WaterInEnthalpy(0.0),
              DilutionAirInEnthalpy(0.0), DilutionAirOutEnthalpy(0.0), PelancillariesAC(0.0), TotProdGasEnthalpy(0.0), WaterOutEnthalpy(0.0),
              SeqSubstitIter(0), RegulaFalsiIter(0)
        {
        }
    };

    struct FCAirSupplyDataStruct
    {
        std::string Name;                              // name of this
        std::string NodeName;                          // Air supply node name
        int SupNodeNum;                                // Air supply node ID
        Curve::Curve *BlowerPowerCurve = nullptr;      // "pointer" to blower power quadratic
        Real64 BlowerHeatLossFactor;                   // alpha for blower heat loss fraction
        DataGenerators::AirSupRateMode AirSupRateMode; // control for modeling method used to deterime supply air flow rate
        Real64 Stoics;                                 // excess air ratio
        Curve::Curve *AirFuncPelCurve = nullptr;       // "pointer" to curve for air as function of power
        Real64 AirTempCoeff;                           // coeff a3 in equ 16.
        Curve::Curve *AirFuncNdotCurve = nullptr;      // "pointer" to curve for air as function of fuel flow rate
        DataGenerators::RecoverMode IntakeRecoveryMode;
        DataGenerators::ConstituentMode ConstituentMode; // how are air data input
        int NumConstituents;
        Array1D_string ConstitName;
        Array1D<Real64> ConstitMolalFract;
        // Calculated values and input from elsewhere
        Array1D<GasID> GasLibID; // lookup ID in Gas Phase ThermoChemistry Structure Array
        Real64 O2fraction;
        Real64 TairIntoBlower;  // temperature entering blower
        Real64 TairIntoFCPM;    // temperature leaving blower and entering FCPM
        Real64 PairCompEl;      // power drawn by compressor
        Real64 QskinLoss;       // pumping losses for zone
        Real64 QintakeRecovery; // heat recovered on intake air by accessories

        // Default Constructor
        FCAirSupplyDataStruct()
            : SupNodeNum(0), BlowerHeatLossFactor(0.0), AirSupRateMode(DataGenerators::AirSupRateMode::Invalid), Stoics(0.0), AirTempCoeff(0.0),
              IntakeRecoveryMode(DataGenerators::RecoverMode::Invalid), ConstituentMode(DataGenerators::ConstituentMode::Invalid), NumConstituents(0),
              ConstitName(14), ConstitMolalFract(14, 0.0), GasLibID(14, GasID::Invalid), O2fraction(0.0), TairIntoBlower(0.0), TairIntoFCPM(0.0),
              PairCompEl(0.0), QskinLoss(0.0), QintakeRecovery(0.0)
        {
        }
    };

    struct FCWaterSupplyDataStruct
    {
        std::string Name;                                                                     // name of this water supply module
        DataGenerators::WaterTempMode waterTempMode = DataGenerators::WaterTempMode::Invalid; // temperature of water inlet determination
        std::string NodeName;                                                                 // node name for temperature at input
        int NodeNum;                                                                          // node number for temperature at input
        Sched::Schedule *sched = nullptr;                                                     // water temperature at input
        Curve::Curve *WaterSupRateCurve = nullptr; // "pointer" to water flow rate curve as a function of fuel rate
        Curve::Curve *PmpPowerCurve = nullptr;     // "pointer to Pump power curve as a function of water flow Rate
        Real64 PmpPowerLossFactor;                 // Pump heat loss factor
        // calculated data
        bool IsModeled;
        Real64 TwaterIntoCompress; // inlet Water Temperature
        Real64 TwaterIntoFCPM;     // pumped water temp
        Real64 PwaterCompEl;       // water pump power
        Real64 QskinLoss;          // pumping losses for zone

        // Default Constructor
        FCWaterSupplyDataStruct()
            : NodeNum(0), PmpPowerLossFactor(0.0), IsModeled(true), TwaterIntoCompress(0.0), TwaterIntoFCPM(0.0), PwaterCompEl(0.0), QskinLoss(0.0)
        {
        }
    };

    struct FCAuxilHeatDataStruct
    {
        std::string Name; // name of this auxiliary heating module
        std::string ZoneName;
        int ZoneID;
        Real64 UASkin; // for skin losses to zone
        Real64 ExcessAirRAT;
        Real64 ANC0;
        Real64 ANC1;
        DataGenerators::LossDestination SkinLossDestination; // control mode for where lost heat goes
        Real64 MaxPowerW;
        Real64 MinPowerW;
        Real64 MaxPowerkmolperSec;
        Real64 MinPowerkmolperSec;
        // calculated and from elsewhere
        int NumConstituents;
        Real64 TauxMix;
        Real64 NdotAuxMix;
        Array1D<Real64> ConstitMolalFract;
        Array1D<GasID> GasLibID; // lookup ID in Gas Phase ThermoChemistry Structure Array
        Real64 QskinLoss;        // Heat lost to room
        Real64 QairIntake;       // heat into intake air

        // Default Constructor
        FCAuxilHeatDataStruct()
            : ZoneID(0), UASkin(0.0), ExcessAirRAT(0.0), ANC0(0.0), ANC1(0.0), SkinLossDestination(DataGenerators::LossDestination::Invalid),
              MaxPowerW(0.0), MinPowerW(0.0), MaxPowerkmolperSec(0.0), MinPowerkmolperSec(0.0), NumConstituents(0), TauxMix(0.0), NdotAuxMix(0.0),
              ConstitMolalFract(14, 0.0), GasLibID(14, GasID::Invalid), QskinLoss(0.0), QairIntake(0.0)
        {
        }
    };

    struct FCExhaustHXDataStruct
    {
        std::string Name;                         // name of this exhaust gas heat recovery
        std::string WaterInNodeName;              // HR Water Inlet Node
        int WaterInNode;                          // HR Water Outlet Node ID
        std::string WaterOutNodeName;             // HR water outlet Node name
        int WaterOutNode;                         // HR Water outlet Node ID
        Real64 WaterVolumeFlowMax;                // HR water flow rate max avail
        std::string ExhaustOutNodeName;           // air node for exhaust flow
        int ExhaustOutNode;                       // Exhaust Air node ID
        DataGenerators::ExhaustGasHX HXmodelMode; // Heat Exchanger Calculation Method
        Real64 HXEffect;                          // Heat Exchanger Effectiveness (method 1)
        Real64 hxs0;                              // (method 2)
        Real64 hxs1;                              // (method 2)
        Real64 hxs2;                              // (method 2)
        Real64 hxs3;                              // (method 2)
        Real64 hxs4;                              // (method 2)
        Real64 h0gas;                             // (method 3)
        Real64 NdotGasRef;                        // (method 3)
        Real64 nCoeff;                            // (method 3)
        Real64 AreaGas;                           // (method 3)
        Real64 h0Water;                           // (method 3)
        Real64 NdotWaterRef;                      // (method 3)
        Real64 mCoeff;                            // (method 3)
        Real64 AreaWater;                         // (method 3)
        Real64 Fadjust;                           // (method 3)
        Real64 l1Coeff;                           // (method 4)
        Real64 l2Coeff;                           // (method 4)
        Real64 CondensationThresholdTemp;         // (method 4) [degrees C]
        // calculated
        Real64 qHX;                     // heat flow from gas stream to water
        Real64 THXexh;                  // temperature of exhaust gases leaving heat exchanger.
        Real64 WaterMassFlowRateDesign; // Design level of water flow rate
        Real64 WaterMassFlowRate;       // water flow rate in plant loop
        Real64 WaterInletTemp;
        Real64 WaterVaporFractExh; // water vapor fraction in exhaust gas stream.
        Real64 CondensateRate;     // water condensation rate.
        Array1D<Real64> ConstitMolalFract;
        Array1D<GasID> GasLibID; // lookup ID in Gas Phase ThermoChemistry Structure Array
        Real64 NdotHXleaving;
        Real64 WaterOutletTemp;
        Real64 WaterOutletEnthalpy;

        // Default Constructor
        FCExhaustHXDataStruct()
            : WaterInNode(0), WaterOutNode(0), WaterVolumeFlowMax(0.0), ExhaustOutNode(0), HXmodelMode(DataGenerators::ExhaustGasHX::Invalid),
              HXEffect(0.0), hxs0(0.0), hxs1(0.0), hxs2(0.0), hxs3(0.0), hxs4(0.0), h0gas(0.0), NdotGasRef(0.0), nCoeff(0.0), AreaGas(0.0),
              h0Water(0.0), NdotWaterRef(0.0), mCoeff(0.0), AreaWater(0.0), Fadjust(0.0), l1Coeff(0.0), l2Coeff(0.0), CondensationThresholdTemp(0.0),
              qHX(0.0), THXexh(0.0), WaterMassFlowRateDesign(0.0), WaterMassFlowRate(0.0), WaterInletTemp(0.0), WaterVaporFractExh(0.0),
              CondensateRate(0.0), ConstitMolalFract(14, 0.0), GasLibID(14, GasID::Invalid), NdotHXleaving(0.0), WaterOutletTemp(0.0),
              WaterOutletEnthalpy(0.0)
        {
        }
    };

    struct BatteryDichargeDataStruct
    {
        std::string Name; // name of this battery data set
        Real64 NumInSeries;
        Real64 NumInParallel;
        Real64 NominalVoltage;
        Real64 LowVoltsDischarged; // not used
        int NumTablePairs;
        Array1D<Real64> DischargeCurrent; // amps
        Array1D<Real64> DischargeTime;    // hours
        // calculated variables
        Real64 k;    // parameter in Manwell McGowan model
        Real64 c;    // parameter in Manwell McGowan model
        Real64 qmax; // parameter in Manwell McGowan model

        // Default Constructor
        BatteryDichargeDataStruct()
            : NumInSeries(0.0), NumInParallel(0.0), NominalVoltage(0.0), LowVoltsDischarged(0.0), NumTablePairs(0), k(0.0), c(0.0), qmax(0.0)
        {
        }
    };

    struct FCElecStorageDataStruct
    {
        std::string Name; // name of this electrical storage module
        DataGenerators::ElectricalStorage StorageModelMode;
        Real64 StartingEnergyStored; // joules inside
        Real64 EnergeticEfficCharge; // for
        Real64 EnergeticEfficDischarge;
        Real64 MaxPowerDraw;  // for simple bucket method 0
        Real64 MaxPowerStore; // for simple bucket method 0
        Real64 NominalVoltage;
        Real64 NominalEnergyCapacity; // [J]
        // calculated and from elsewhere vars
        Real64 ThisTimeStepStateOfCharge; // [J]
        Real64 LastTimeStepStateOfCharge; // [J]
        Real64 PelNeedFromStorage;
        Real64 IdesiredDischargeCurrent;
        Real64 PelFromStorage; // power
        Real64 IfromStorage;   // current this timestepm
        Real64 PelIntoStorage;
        Real64 QairIntake; // heat into intake air
        // nested structures
        BatteryDichargeDataStruct Battery;

        // Default Constructor
        FCElecStorageDataStruct()
            : StorageModelMode(DataGenerators::ElectricalStorage::Invalid), StartingEnergyStored(0.0), EnergeticEfficCharge(0.0),
              EnergeticEfficDischarge(0.0), MaxPowerDraw(0.0), MaxPowerStore(0.0), NominalVoltage(0.0), NominalEnergyCapacity(0.0),
              ThisTimeStepStateOfCharge(0.0), LastTimeStepStateOfCharge(0.0), PelNeedFromStorage(0.0), IdesiredDischargeCurrent(0.0),
              PelFromStorage(0.0), IfromStorage(0.0), PelIntoStorage(0.0), QairIntake(0.0)
        {
        }
    };

    struct FCInverterDataStruct
    {
        std::string Name;                               // name of this inverter
        DataGenerators::InverterEfficiencyMode EffMode; // efficiency calculation mode
        Real64 ConstEff;
        Curve::Curve *EffQuadraticCurve = nullptr;
        // calculated and from elsewhere
        Real64 PCUlosses;
        Real64 QairIntake;

        // Default Constructor
        FCInverterDataStruct() : EffMode(DataGenerators::InverterEfficiencyMode::Invalid), ConstEff(0.0), PCUlosses(0.0), QairIntake(0.0)
        {
        }
    };

    struct FCReportDataStruct
    {
        // Members
        Real64 ACPowerGen;           // reporting: power (W)
        Real64 ACEnergyGen;          // reporting: energy (J)
        Real64 QdotExhaust;          // reporting: exhaust gas heat recovered (W)
        Real64 TotalHeatEnergyRec;   // reporting: total heat recovered (J)
        Real64 ExhaustEnergyRec;     // reporting: exhaust gas heat recovered (J)
        Real64 FuelEnergyLHV;        // reporting: Fuel Energy used in Lower Heating Value(J)
        Real64 FuelEnergyUseRateLHV; // reporting: Fuel Energy used in Lower Heating Value(W)
        Real64 FuelEnergyHHV;        // reporting: Fuel Energy used in Lower Heating Value(J)
        Real64 FuelEnergyUseRateHHV; // reporting: Fuel Energy used in Lower Heating Value(W)
        Real64 FuelRateMdot;         // (Kg/s)
        Real64 HeatRecInletTemp;     // reporting: Heat Recovery Loop Inlet Temperature (C)
        Real64 HeatRecOutletTemp;    // reporting: Heat Recovery Loop Outlet Temperature (C)
        Real64 HeatRecMdot;          // reporting: Heat Recovery Loop Mass flow rate (kg/s)
        // air supply and blower
        Real64 TairInlet;        // State point 1
        Real64 TairIntoFCPM;     // Temperature at State point 4
        Real64 NdotAir;          // air flow in kmol/sec
        Real64 TotAirInEnthalpy; // Enthalpy at State point 4
        Real64 BlowerPower;      // electrical power used by air supply blower
        Real64 BlowerEnergy;     // electrical energy used by air supply blower
        Real64 BlowerSkinLoss;   // heat rate of losses by blower
        // fuel supply and compressor
        Real64 TfuelInlet;           // State point 2 [C]
        Real64 TfuelIntoFCPM;        // state point 5 [C]
        Real64 NdotFuel;             // fuel flow in [kmol/sec]
        Real64 TotFuelInEnthalpy;    // state point 5 [W]
        Real64 FuelCompressPower;    // electrical power used by fuel supply compressor [W]
        Real64 FuelCompressEnergy;   // electrical energy used by fuel supply compressor [J]
        Real64 FuelCompressSkinLoss; // heat rate of losses.by fuel supply compressor [W]
        // reformer water supply
        Real64 TwaterInlet;           // State point 3
        Real64 TwaterIntoFCPM;        // State point 6
        Real64 NdotWater;             // water flow in kmol/sec (reformer water)
        Real64 WaterPumpPower;        // electrical power used by water pump [W]
        Real64 WaterPumpEnergy;       // electrical energy used by water pump [J]
        Real64 WaterIntoFCPMEnthalpy; // state point 6
        // product (exhaust) gas leaving power module
        Real64 TprodGas;      // State point 7 Product Gas temperature
        Real64 EnthalProdGas; // state point 7 product gas enthalpy
        Real64 NdotProdGas;   // point 7 flow rate [kmol/sec]
        Real64 NdotProdAr;    // argon flow rate at point 7
        Real64 NdotProdCO2;   // carbon dioxide flow rate at point 7
        Real64 NdotProdH2O;   // water vapor flow rate at point 7
        Real64 NdotProdN2;    // nitrogen flow rate at point 7
        Real64 NdotProdO2;    // oxygen flow rate at point 7
        // heat exchanger for water to exhaust heat recovery
        Real64 qHX;                // heat flow from gas stream to water [W]
        Real64 HXenergy;           // energy from gas stream to water [J]
        Real64 THXexh;             // temperature of exhaust gases leaving heat exchanger.
        Real64 WaterVaporFractExh; // water vapor fraction in exhaust gas stream
        // relative to water vapor entering HX  (NdotH2O/Ndoaux-mix)
        Real64 CondensateRate;     // water condensation rate [kmol/s]
        int SeqSubstIterations;    // number of iterations in SOFC loop
        int RegulaFalsiIterations; // number of iterations in Tproduct gas solving
        Real64 ACancillariesPower;
        Real64 ACancillariesEnergy;
        Real64 PCUlosses;            // power conditioning Unit losses
        Real64 DCPowerGen;           // Pel, Power module power level [W]
        Real64 DCPowerEff;           // Eel, power module efficiency []
        Real64 ElectEnergyinStorage; // State of charge in Electrical Storage [J]
        Real64 StoredPower;          // Power added to Electrical Storage [W]
        Real64 StoredEnergy;         // energy added to Electrical STorage [J]
        Real64 DrawnPower;           // Power drawn from Electrical STorage [W]
        Real64 DrawnEnergy;          // Energy drawn from Electrical STorage [J]
        Real64 SkinLossPower;        // heat loss to surrounding zone [W]
        Real64 SkinLossEnergy;       // heat loss to surround zone [J]
        Real64 SkinLossConvect;      // convective heat loss to zone [W]
        Real64 SkinLossRadiat;       // radiative heat loss to zone [W}
        Real64 ElectEfficiency;
        Real64 ThermalEfficiency;
        Real64 OverallEfficiency;
        Real64 ExergyEfficiency;
        int NumCycles;       // Number of start-stop cycles
        Real64 FCPMSkinLoss; // Power module skin losses [W]

        // Default Constructor
        FCReportDataStruct()
            : ACPowerGen(0.0), ACEnergyGen(0.0), QdotExhaust(0.0), TotalHeatEnergyRec(0.0), ExhaustEnergyRec(0.0), FuelEnergyLHV(0.0),
              FuelEnergyUseRateLHV(0.0), FuelEnergyHHV(0.0), FuelEnergyUseRateHHV(0.0), FuelRateMdot(0.0), HeatRecInletTemp(0.0),
              HeatRecOutletTemp(0.0), HeatRecMdot(0.0), TairInlet(0.0), TairIntoFCPM(0.0), NdotAir(0.0), TotAirInEnthalpy(0.0), BlowerPower(0.0),
              BlowerEnergy(0.0), BlowerSkinLoss(0.0), TfuelInlet(0.0), TfuelIntoFCPM(0.0), NdotFuel(0.0), TotFuelInEnthalpy(0.0),
              FuelCompressPower(0.0), FuelCompressEnergy(0.0), FuelCompressSkinLoss(0.0), TwaterInlet(0.0), TwaterIntoFCPM(0.0), NdotWater(0.0),
              WaterPumpPower(0.0), WaterPumpEnergy(0.0), WaterIntoFCPMEnthalpy(0.0), TprodGas(0.0), EnthalProdGas(0.0), NdotProdGas(0.0),
              NdotProdAr(0.0), NdotProdCO2(0.0), NdotProdH2O(0.0), NdotProdN2(0.0), NdotProdO2(0.0), qHX(0.0), HXenergy(0.0), THXexh(0.0),
              WaterVaporFractExh(0.0), CondensateRate(0.0), SeqSubstIterations(0), RegulaFalsiIterations(0), ACancillariesPower(0.0),
              ACancillariesEnergy(0.0), PCUlosses(0.0), DCPowerGen(0.0), DCPowerEff(0.0), ElectEnergyinStorage(0.0), StoredPower(0.0),
              StoredEnergy(0.0), DrawnPower(0.0), DrawnEnergy(0.0), SkinLossPower(0.0), SkinLossEnergy(0.0), SkinLossConvect(0.0),
              SkinLossRadiat(0.0), ElectEfficiency(0.0), ThermalEfficiency(0.0), OverallEfficiency(0.0), ExergyEfficiency(0.0), NumCycles(0),
              FCPMSkinLoss(0.0)
        {
        }
    };

    struct FCStackCoolerDataStruct
    {
        std::string Name;             // name of this stack cooler module
        std::string WaterInNodeName;  // HR Water Inlet Node
        int WaterInNode;              // HR Water Outlet Node ID
        std::string WaterOutNodeName; // HR water outlet Node name
        int WaterOutNode;             // HR Water outlet Node ID
        Real64 TstackNom;             // nominal fuel cell stack temperature
        Real64 TstackActual;          // actual fuel cell stack temperature
        Real64 r0;                    // stack cooling power coefficient r0
        Real64 r1;                    // stack cooling power coefficient r1
        Real64 r2;                    // stack cooling power coefficient r2
        Real64 r3;                    // stack cooling power coefficient r3
        Real64 MdotStackCoolant;      // stack coolant flow rate kg/s
        Real64 UAs_cool;              // stack heat transfer coef
        Real64 Fs_cogen;
        Real64 As_cogen;
        Real64 MdotCogenNom;
        Real64 hCogenNom;
        Real64 ns;
        Real64 PstackPumpEl;
        Real64 PmpPowerLossFactor;
        Real64 f0;
        Real64 f1;
        Real64 f2;
        // calculated and from elsewhere
        bool StackCoolerPresent; // control modeling
        Real64 qs_cool;
        Real64 qs_air;

        // Default Constructor
        FCStackCoolerDataStruct()
            : WaterInNode(0), WaterOutNode(0), TstackNom(0.0), TstackActual(0.0), r0(0.0), r1(0.0), r2(0.0), r3(0.0), MdotStackCoolant(0.0),
              UAs_cool(0.0), Fs_cogen(0.0), As_cogen(0.0), MdotCogenNom(0.0), hCogenNom(0.0), ns(0.0), PstackPumpEl(0.0), PmpPowerLossFactor(0.0),
              f0(0.0), f1(0.0), f2(0.0), StackCoolerPresent(false), qs_cool(0.0), qs_air(0.0)
        {
        }
    };

    struct FCDataStruct : PlantComponent
    {
        // Members
        // from input data and nested types for subsystems
        DataPlant::PlantEquipmentType Type;
        std::string Name;                    // user identifier
        std::string NameFCPM;                // name of FC Power Module
        FCPowerModuleStruct FCPM;            // data for Power Module
        std::string NameFCAirSup;            // name of air supply module for fuel cell
        FCAirSupplyDataStruct AirSup;        // data for air supply module
        std::string NameFCFuelSup;           // name of fuel supply module
        int FuelSupNum;                      // index for fuel supply module structure
        std::string NameFCWaterSup;          // name of water supply module
        FCWaterSupplyDataStruct WaterSup;    // data for water supply module
        std::string NameFCAuxilHeat;         // name of auxiliary heating module
        FCAuxilHeatDataStruct AuxilHeat;     // data for auxiliary heating module
        std::string NameExhaustHX;           // name of Exhaust HX module
        FCExhaustHXDataStruct ExhaustHX;     // data for Exhaust heat exchanger module
        std::string NameElecStorage;         // name of Battery module
        FCElecStorageDataStruct ElecStorage; // data for Battery module
        std::string NameInverter;            // name of Inverter Module
        FCInverterDataStruct Inverter;       // data for Inverter module
        std::string NameStackCooler;         // name of Inverter Module
        FCStackCoolerDataStruct StackCooler; // data for Inverter module
        PlantLocation CWPlantLoc;            // cooling water plant loop component index
        FCReportDataStruct Report;           // data for reporting as E+ output variables
        // calculated whole-system level variables
        Real64 ACPowerGen; // Net output from SOFC unit
        Real64 QconvZone;  // convective heat lost to surrounding zone
        Real64 QradZone;   // radiative heat lost to surrounding zone
        int DynamicsControlID;
        Real64 TimeElapsed; // used to track when timestep has changed
        bool MyEnvrnFlag_Init;
        bool MyWarmupFlag_Init;
        bool MyPlantScanFlag_Init;

        int SolverErr_Type1_Iter;      // Iteration counter for Fuel Cell solver root finding Type 1 error warning messages
        int SolverErr_Type1_IterIndex; // Index for Fuel Cell solver root finding Type 1 error warning messages
        int SolverErr_Type2_Iter;      // Iteration counter for Fuel Cell solver root finding Type 2 error warning messages
        int SolverErr_Type2_IterIndex; // Index for Fuel Cell solver root finding Type 2 error warning messages

        // Default Constructor
        FCDataStruct()
            : Type(DataPlant::PlantEquipmentType::Invalid), FuelSupNum(0), CWPlantLoc{}, ACPowerGen(0.0), QconvZone(0.0), QradZone(0.0),
              DynamicsControlID(0), TimeElapsed(0.0), MyEnvrnFlag_Init(true), MyWarmupFlag_Init(false), MyPlantScanFlag_Init(true),
              SolverErr_Type1_Iter(0), SolverErr_Type1_IterIndex(0), SolverErr_Type2_Iter(0), SolverErr_Type2_IterIndex(0)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        static PlantComponent *factory_exhaust(EnergyPlusData &state, std::string const &objectName);

        void initialize(EnergyPlusData &state);

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void setupOutputVars(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void FigureAirHeatCap(EnergyPlusData &state, Real64 FluidTemp, Real64 &Cp);

        void FigureAirEnthalpy(EnergyPlusData &state, Real64 FluidTemp, Real64 &Hair);

        void FigureFuelHeatCap(EnergyPlusData &state, Real64 FluidTemp, Real64 &Cp) const;

        void FigureFuelEnthalpy(EnergyPlusData &state, Real64 FluidTemp, Real64 &Hfuel) const;

        void FigureProductGasesEnthalpy(EnergyPlusData &state, Real64 FluidTemp, Real64 &HProdGases);

        void FigureAuxilHeatGasHeatCap(EnergyPlusData &state, Real64 FluidTemp, Real64 &Cp);

        void FigureACAncillaries(EnergyPlusData &state, Real64 &PacAncill);

        void FigurePowerConditioningLosses(EnergyPlusData &state, Real64 Pdemand, Real64 &PpcuLosses) const;

        void FigureTransientConstraints(EnergyPlusData &state,
                                        Real64 &Pel,       // DC power control setting for power module
                                        bool &Constrained, // true if transient constraints kick in
                                        Real64 &PelDiff    // if constrained then this is the difference, positive
        );

        static void FigureGaseousWaterEnthalpy(Real64 FluidTemp, Real64 &HGasWater);

        static void FigureLiquidWaterEnthalpy(Real64 FluidTemp, Real64 &HLiqWater);

        static void FigureLiquidWaterHeatCap(Real64 FluidTemp, Real64 &Cp);

        void CalcFuelCellAuxHeater();

        void CalcFuelCellGenHeatRecovery(EnergyPlusData &state);

        void CalcFuelCellGeneratorModel(EnergyPlusData &state, bool RunFlag, Real64 MyLoad, bool FirstHVACIteration);

        void CalcUpdateHeatRecovery(EnergyPlusData &state, bool FirstHVACIteration) const;

        void ManageElectStorInteractions(EnergyPlusData &state,
                                         Real64 Pdemand,
                                         Real64 PpcuLosses,
                                         bool &Constrained, // TODO: This one is never used anywhere in the code
                                         Real64 &Pstorage,
                                         Real64 &PgridOverage // electricity that can't be stored and needs to go out
        );

        void SimFuelCellGenerator(EnergyPlusData &state,
                                  bool RunFlag,  // simulate Generator when TRUE
                                  Real64 MyLoad, // demand on electric generator
                                  bool FirstHVACIteration);

        void UpdateFuelCellGeneratorRecords(EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    void getFuelCellInput(EnergyPlusData &state);

    void FigureFuelCellZoneGains(EnergyPlusData &state);

} // namespace FuelCellElectricGenerator

struct FuelCellElectricGeneratorData : BaseGlobalStruct
{

    int NumFuelCellGenerators = 0;
    bool getFuelCellInputFlag = true;
    bool MyEnvrnFlag = true;
    Array1D<FuelCellElectricGenerator::FCDataStruct> FuelCell;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumFuelCellGenerators = 0;
        this->getFuelCellInputFlag = true;
        this->MyEnvrnFlag = true;
        this->FuelCell.deallocate();
    }
};

} // namespace EnergyPlus

#endif
