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

#ifndef MicroCHPElectricGenerator_hh_INCLUDED
#define MicroCHPElectricGenerator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace MicroCHPElectricGenerator {

    struct MicroCHPParamsNonNormalized
    {
        std::string Name;                        // name of this PowerModule data
        Real64 MaxElecPower;                     // net electric power [W]
        Real64 MinElecPower;                     // net electric power [W]
        Real64 MinWaterMdot;                     // minimum cooling water flow [kg/s]
        Real64 MaxWaterTemp;                     // limit temp for inlet cooling water [C]
        Curve::Curve *ElecEffCurve = nullptr;    // index for TriQuadratic for electrical efficiency
        Curve::Curve *ThermalEffCurve = nullptr; // index for TriQuadric for thermal efficiency
        bool InternalFlowControl;                // Plant or Internal Flow rate control?
        bool PlantFlowControl;                   // default is plant control
        Curve::Curve *WaterFlowCurve = nullptr;  // index for BiQuadratic for water flow rate internal control
        Curve::Curve *AirFlowCurve = nullptr;    // index for Quadratic for generator air flow
        Real64 DeltaPelMax;                      // max rate of change in net electric power [W/s}
        Real64 DeltaFuelMdotMax;                 // Maximum Rate of change in fuel flow rate [kmol/s2]
        Real64 UAhx;                             // heat exchanger UA [W/K]
        Real64 UAskin;                           // skin loss UA [W/K]
        Real64 RadiativeFraction;                // skin loss fraction to radiant energy []
        Real64 MCeng;                            // aggregated thermal mass of engine [J/K]
        Real64 MCcw;                             // aggregated thermal mass of heat recovery [J/k]
        Real64 Pstandby;                         // standby power [w]
        bool WarmUpByTimeDelay;                  // Warm up mode control
        bool WarmUpByEngineTemp;                 // Warm up mode control
        Real64 kf;                               // coefficient k_f for warmup fuel flow rate
        Real64 TnomEngOp;                        // nominal engine operating temperature [C]
        Real64 kp;                               // coefficient k_p for warmup power
        Real64 Rfuelwarmup;                      // Warm Up Fuel Flow Rate Limit Ratio
        Real64 WarmUpDelay;                      // time for warm up delay [s]
        Real64 PcoolDown;                        // power during cool down
        Real64 CoolDownDelay;                    // time for cool down delay [s]
        bool MandatoryFullCoolDown;
        bool WarmRestartOkay;
        // calculated and from elsewhere
        Real64 TimeElapsed; // Fraction of the current hour that has elapsed (h)
        // Saved in order to identify the beginning of a new system time
        DataGenerators::OperatingMode OpMode;
        Real64 OffModeTime;      // amount of time generator spent in Off mode
        Real64 StandyByModeTime; // amount of time generator spent in standby mode
        Real64 WarmUpModeTime;   // amount of time generator spent in warm up mode
        Real64 NormalModeTime;   // amount of time generator spent in normal mode
        Real64 CoolDownModeTime; // amount of time generator spent in Cool down mode
        Real64 TengLast;         // last timestep's value for engine temperature
        Real64 TempCWOutLast;    // last timestep's value for cooling water outlet temperature
        Real64 Pnet;
        Real64 ElecEff;
        Real64 Qgross;
        Real64 ThermEff;
        Real64 Qgenss;
        Real64 NdotFuel;
        Real64 MdotFuel;
        Real64 Teng;
        Real64 TcwIn;
        Real64 TcwOut;
        Real64 MdotAir;
        Real64 QdotSkin; // rate of heat loss to zone
        Real64 QdotConvZone;
        Real64 QdotRadZone;
        Real64 ACPowerGen;           // reporting: power (W)
        Real64 ACEnergyGen;          // reporting: energy (J)
        Real64 QdotHX;               // reporting: rate of heat exchange from engine to coolant (W)
        Real64 QdotHR;               // reporting: rate of heat recovered (W)
        Real64 TotalHeatEnergyRec;   // reporting: total heat recovered (J)
        Real64 FuelEnergyLHV;        // reporting: Fuel Energy used in Lower Heating Value(J)
        Real64 FuelEnergyUseRateLHV; // reporting: Fuel Energy used in Lower Heating Value(W)
        Real64 FuelEnergyHHV;        // reporting: Fuel Energy used in Higher Heating Value(J)
        Real64 FuelEnergyUseRateHHV; // reporting: Fuel Energy used in Higher Heating Value(W)
        Real64 HeatRecInletTemp;     // reporting: Heat Recovery Loop Inlet Temperature (C)
        Real64 HeatRecOutletTemp;    // reporting: Heat Recovery Loop Outlet Temperature (C)
        Real64 FuelCompressPower;    // electrical power used by fuel supply compressor [W]
        Real64 FuelCompressEnergy;   // electrical energy used by fuel supply compressor [J]
        Real64 FuelCompressSkinLoss; // heat rate of losses.by fuel supply compressor [W]
        Real64 SkinLossPower;        // heat loss to surrounding zone [W]
        Real64 SkinLossEnergy;       // heat loss to surround zone [J]
        Real64 SkinLossConvect;      // convective heat loss to zone [W]
        Real64 SkinLossRadiat;       // radiative heat loss to zone [W]

        // Default Constructor
        MicroCHPParamsNonNormalized()
            : MaxElecPower(0.0), MinElecPower(0.0), MinWaterMdot(0.0), MaxWaterTemp(0.0), InternalFlowControl(false), PlantFlowControl(true),
              DeltaPelMax(0.0), DeltaFuelMdotMax(0.0), UAhx(0.0), UAskin(0.0), RadiativeFraction(0.0), MCeng(0.0), MCcw(0.0), Pstandby(0.0),
              WarmUpByTimeDelay(false), WarmUpByEngineTemp(true), kf(0.0), TnomEngOp(0.0), kp(0.0), Rfuelwarmup(0.0), WarmUpDelay(0.0),
              PcoolDown(0.0), CoolDownDelay(0.0), MandatoryFullCoolDown(false), WarmRestartOkay(true), TimeElapsed(0.0),
              OpMode(DataGenerators::OperatingMode::Invalid), OffModeTime(0.0), StandyByModeTime(0.0), WarmUpModeTime(0.0), NormalModeTime(0.0),
              CoolDownModeTime(0.0), TengLast(20.0), TempCWOutLast(20.0), Pnet(0.0), ElecEff(0.0), Qgross(0.0), ThermEff(0.0), Qgenss(0.0),
              NdotFuel(0.0), MdotFuel(0.0), Teng(20.0), TcwIn(20.0), TcwOut(20.0), MdotAir(0.0), QdotSkin(0.0), QdotConvZone(0.0), QdotRadZone(0.0),
              ACPowerGen(0.0), ACEnergyGen(0.0), QdotHX(0.0), QdotHR(0.0), TotalHeatEnergyRec(0.0), FuelEnergyLHV(0.0), FuelEnergyUseRateLHV(0.0),
              FuelEnergyHHV(0.0), FuelEnergyUseRateHHV(0.0), HeatRecInletTemp(0.0), HeatRecOutletTemp(0.0), FuelCompressPower(0.0),
              FuelCompressEnergy(0.0), FuelCompressSkinLoss(0.0), SkinLossPower(0.0), SkinLossEnergy(0.0), SkinLossConvect(0.0), SkinLossRadiat(0.0)
        {
        }
    };

    struct MicroCHPDataStruct : PlantComponent
    {
        std::string Name;                     // name of this Micro CHP Generator
        std::string ParamObjName;             // name of parameter object
        MicroCHPParamsNonNormalized A42Model; // Nested parameter data structure
        Real64 NomEff;                        // nominal efficiency
        std::string ZoneName;
        int ZoneID;
        std::string PlantInletNodeName;
        int PlantInletNodeID;
        std::string PlantOutletNodeName;
        int PlantOutletNodeID;
        Real64 PlantMassFlowRate;              // only if internal control
        Real64 PlantMassFlowRateMax;           // hardware limit for node%massflowrateMax
        bool PlantMassFlowRateMaxWasAutoSized; // true if mass flow rate was autosized on input
        std::string AirInletNodeName;
        int AirInletNodeID;
        std::string AirOutletNodeName;
        int AirOutletNodeID;
        int FuelSupplyID;                      // index for fuel supply data structure
        int DynamicsControlID;                 // index in GeneratorDynamics data where control issues are handled
        Sched::Schedule *availSched = nullptr; // index for availability schedule
        PlantLocation CWPlantLoc;              // cooling water plant loop component index
        bool CheckEquipName;
        bool MySizeFlag;
        bool MyEnvrnFlag;
        bool MyPlantScanFlag;
        bool myFlag;

        // Default Constructor
        MicroCHPDataStruct()
            : NomEff(0.0), ZoneID(0), PlantInletNodeID(0), PlantOutletNodeID(0), PlantMassFlowRate(0.0), PlantMassFlowRateMax(0.0),
              PlantMassFlowRateMaxWasAutoSized(false), AirInletNodeID(0), AirOutletNodeID(0), FuelSupplyID(0), DynamicsControlID(0), CWPlantLoc{},
              CheckEquipName(true), MySizeFlag(true), MyEnvrnFlag(true), MyPlantScanFlag(true), myFlag(true)
        {
        }

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 Real64 &MaxLoad,
                                 Real64 &MinLoad,
                                 Real64 &OptLoad) override;

        void onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void setupOutputVars(EnergyPlusData &state);

        void InitMicroCHPNoNormalizeGenerators(EnergyPlusData &state);

        void CalcUpdateHeatRecovery(EnergyPlusData &state) const;

        void CalcMicroCHPNoNormalizeGeneratorModel(EnergyPlusData &state,
                                                   bool RunFlagElectCenter, // TRUE when Generator operating
                                                   bool RunFlagPlant,
                                                   Real64 MyElectricLoad, // Generator demand
                                                   Real64 MyThermalLoad);

        void UpdateMicroCHPGeneratorRecords(EnergyPlusData &state);

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetMicroCHPGeneratorInput(EnergyPlusData &state);

    Real64 FuncDetermineEngineTemp(Real64 TcwOut,   // hot water leaving temp
                                   Real64 MCeng,    // Fictitious mass and heat capacity of engine
                                   Real64 UAHX,     // Heat exchanger UA
                                   Real64 UAskin,   // Skin losses UA
                                   Real64 Troom,    // surrounding zone temperature C
                                   Real64 Qgenss,   // steady state generator heat generation
                                   Real64 TengLast, // engine temp at previous time step
                                   Real64 time      // elapsed time since previous evaluation
    );

    Real64 FuncDetermineCoolantWaterExitTemp(Real64 TcwIn,      // hot water inlet temp
                                             Real64 MCcw,       // Fictitious mass and heat capacity of coolant hx
                                             Real64 UAHX,       // Heat exchanger UA
                                             Real64 MdotCpcw,   // mass flow and specific heat of coolant water
                                             Real64 Teng,       // engine mass temperature C
                                             Real64 TcwoutLast, // coolant water leaving temp at previous time step
                                             Real64 time        // elapsed time since previous evaluation
    );

    bool CheckMicroCHPThermalBalance(Real64 NomHeatGen, // nominal heat generation rate for scaling
                                     Real64 TcwIn,      // hot water inlet temp
                                     Real64 TcwOut,     // hot water leaving temp
                                     Real64 Teng,       // engine mass temperature C
                                     Real64 Troom,      // surrounding zone temperature C
                                     Real64 UAHX,       // Heat exchanger UA
                                     Real64 UAskin,     // Skin losses UA
                                     Real64 Qgenss,     // steady state generator heat generation
                                     Real64 MCeng,      // Fictitious mass and heat capacity of engine
                                     Real64 MCcw,       // Fictitious mass and heat capacity of coolant hx
                                     Real64 MdotCpcw    // mass flow and specific heat of coolant water
    );

    void FigureMicroCHPZoneGains(EnergyPlusData &state);

} // namespace MicroCHPElectricGenerator

struct MicroCHPElectricGeneratorData : BaseGlobalStruct
{

    int NumMicroCHPs = 0;
    int NumMicroCHPParams = 0;
    EPVector<MicroCHPElectricGenerator::MicroCHPDataStruct> MicroCHP;
    EPVector<MicroCHPElectricGenerator::MicroCHPParamsNonNormalized> MicroCHPParamInput;
    bool getMicroCHPInputFlag = true;
    bool MyOneTimeFlag = true;
    bool MyEnvrnFlag = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumMicroCHPs = 0;
        this->NumMicroCHPParams = 0;
        this->getMicroCHPInputFlag = true;
        this->MicroCHP.deallocate();
        this->MicroCHPParamInput.deallocate();
        this->MyOneTimeFlag = true;
        this->MyEnvrnFlag = true;
    }
};

} // namespace EnergyPlus

#endif
