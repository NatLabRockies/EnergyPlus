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

#ifndef ICEngineElectricGenerator_hh_INCLUDED
#define ICEngineElectricGenerator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Curve {
    struct Curve;
}

namespace ICEngineElectricGenerator {

    Real64 constexpr ReferenceTemp(25.0); // Reference temperature by which lower heating
    // value is reported.  This should be subtracted
    // off of when calculated exhaust energies.

    struct ICEngineGeneratorSpecs : PlantComponent
    {
        // Members
        std::string Name;   // user identifier
        std::string TypeOf; // Type of Generator
        GeneratorType CompType_Num;
        Constant::eFuel FuelType;                      // Type of Fuel - DIESEL, GASOLINE, GAS
        Real64 RatedPowerOutput;                       // W - design nominal capacity of Generator
        int ElectricCircuitNode;                       // Electric Circuit Node
        Real64 MinPartLoadRat;                         // (IC ENGINE MIN) min allowed operating frac full load
        Real64 MaxPartLoadRat;                         // (IC ENGINE MAX) max allowed operating frac full load
        Real64 OptPartLoadRat;                         // (IC ENGINE BEST) optimal operating frac full load
        Real64 ElecOutputFuelRat;                      // (RELDC) Ratio of Generator output to Fuel Energy Input
        Curve::Curve *ElecOutputFuelCurve = nullptr;   // Curve for generator output to Fuel Energy Input Coeff Poly Fit
        Real64 RecJacHeattoFuelRat;                    // (RJACDC) Ratio of Recoverable Jacket Heat to Fuel Energy Input
        Curve::Curve *RecJacHeattoFuelCurve = nullptr; // Curve for Ratio of Recoverable Jacket Heat to
        // Fuel Energy Input Coeff Poly Fit
        Real64 RecLubeHeattoFuelRat;                    // (RLUBDC) Ratio of Recoverable Lube Oil Heat to Fuel Energy Input
        Curve::Curve *RecLubeHeattoFuelCurve = nullptr; // Curve for Ratio of Recoverable Lube Oil Heat to
        // Fuel Energy Input Coef Poly Fit
        Real64 TotExhausttoFuelRat;                    // (REXDC) Total Exhaust heat Input to Fuel Energy Input
        Curve::Curve *TotExhausttoFuelCurve = nullptr; // Curve for Total Exhaust heat Input to Fuel Energy Input
        // Coeffs Poly Fit
        Real64 ExhaustTemp;                       // (TEXDC) Exhaust Gas Temp to Fuel Energy Input
        Curve::Curve *ExhaustTempCurve = nullptr; // Curve for Exhaust Gas Temp to Fuel Energy Input Coeffs Poly Fit
        int ErrExhaustTempIndex;                  // error index for temp curve
        Real64 UA;                                // (UACDC) exhaust gas Heat Exchanger UA to Capacity
        Array1D<Real64> UACoef;                   // Heat Exchanger UA Coeffs Poly Fit
        Real64 MaxExhaustperPowerOutput;          // MAX EXHAUST FLOW PER W DSL POWER OUTPUT COEFF
        Real64 DesignMinExitGasTemp;              // Steam Saturation Temperature
        Real64 FuelHeatingValue;                  // Heating Value of Fuel in kJ/kg
        Real64 DesignHeatRecVolFlowRate;          // m3/s, Design Water mass flow rate through heat recovery loop
        Real64 DesignHeatRecMassFlowRate;         // kg/s, Design Water mass flow rate through heat recovery loop
        bool HeatRecActive;                       // True if Heat Rec Design Vol Flow Rate > 0
        int HeatRecInletNodeNum;                  // Node number on the heat recovery inlet side of the condenser
        int HeatRecOutletNodeNum;                 // Node number on the heat recovery outlet side of the condenser
        Real64 HeatRecInletTemp;                  // Inlet Temperature of the heat recovery fluid
        Real64 HeatRecOutletTemp;                 // Outlet Temperature of the heat recovery fluid
        Real64 HeatRecMdotDesign;                 // reporting: Heat Recovery Loop Mass flow rate
        Real64 HeatRecMdotActual;
        Real64 QTotalHeatRecovered; // total heat recovered (W)
        Real64 QJacketRecovered;    // heat recovered from jacket (W)
        Real64 QLubeOilRecovered;   // heat recovered from lube (W)
        Real64 QExhaustRecovered;   // exhaust gas heat recovered (W)
        Real64 FuelEnergyUseRate;   // Fuel Energy used (W)
        Real64 TotalHeatEnergyRec;  // total heat recovered (J)
        Real64 JacketEnergyRec;     // heat recovered from jacket (J)
        Real64 LubeOilEnergyRec;    // heat recovered from lube (J)
        Real64 ExhaustEnergyRec;    // exhaust gas heat recovered (J)
        Real64 FuelEnergy;          // Fuel Energy used (J)
        Real64 FuelMdot;            // Fuel Amount used (Kg/s)
        Real64 ExhaustStackTemp;    // Exhaust Stack Temperature (C)
        Real64 ElecPowerGenerated;  // Electric Power Generated (W)
        Real64 ElecEnergyGenerated; // Amount of Electric Energy Generated (J)
        Real64 HeatRecMaxTemp;      // Max Temp that can be produced in heat recovery
        PlantLocation HRPlantLoc;   // cooling water plant loop component index, for heat recovery
        bool MyEnvrnFlag;
        bool MyPlantScanFlag;
        bool MySizeAndNodeInitFlag;
        bool CheckEquipName;
        bool myFlag;

        // Default Constructor
        ICEngineGeneratorSpecs()
            : TypeOf("Generator:InternalCombustionEngine"), CompType_Num(GeneratorType::ICEngine), RatedPowerOutput(0.0), ElectricCircuitNode(0),
              MinPartLoadRat(0.0), MaxPartLoadRat(0.0), OptPartLoadRat(0.0), ElecOutputFuelRat(0.0), RecJacHeattoFuelRat(0.0),
              RecLubeHeattoFuelRat(0.0), TotExhausttoFuelRat(0.0), ExhaustTemp(0.0), ErrExhaustTempIndex(0), UA(0.0), UACoef(2, 0.0),
              MaxExhaustperPowerOutput(0.0), DesignMinExitGasTemp(0.0), FuelHeatingValue(0.0), DesignHeatRecVolFlowRate(0.0),
              DesignHeatRecMassFlowRate(0.0), HeatRecActive(false), HeatRecInletNodeNum(0), HeatRecOutletNodeNum(0), HeatRecInletTemp(0.0),
              HeatRecOutletTemp(0.0), HeatRecMdotDesign(0.0), HeatRecMdotActual(0.0), QTotalHeatRecovered(0.0), QJacketRecovered(0.0),
              QLubeOilRecovered(0.0), QExhaustRecovered(0.0), FuelEnergyUseRate(0.0), TotalHeatEnergyRec(0.0), JacketEnergyRec(0.0),
              LubeOilEnergyRec(0.0), ExhaustEnergyRec(0.0), FuelEnergy(0.0), FuelMdot(0.0), ExhaustStackTemp(0.0), ElecPowerGenerated(0.0),
              ElecEnergyGenerated(0.0), HeatRecMaxTemp(0.0), HRPlantLoc{}, MyEnvrnFlag(true), MyPlantScanFlag(true), MySizeAndNodeInitFlag(true),
              CheckEquipName(true), myFlag(true)
        {
        }

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void InitICEngineGenerators(EnergyPlusData &state, bool RunFlag, bool FirstHVACIteration);

        void CalcICEngineGeneratorModel(EnergyPlusData &state, bool RunFlag, Real64 MyLoad);

        void CalcICEngineGenHeatRecovery(EnergyPlusData &state, Real64 EnergyRecovered, Real64 HeatRecMdot, Real64 &HRecRatio);

        void update(EnergyPlusData &state);

        void setupOutputVars(EnergyPlusData &state);

        void getDesignCapacities(EnergyPlusData &state,
                                 [[maybe_unused]] const PlantLocation &calledFromLocation,
                                 Real64 &MaxLoad,
                                 Real64 &MinLoad,
                                 Real64 &OptLoad) override;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetICEngineGeneratorInput(EnergyPlusData &state);

} // namespace ICEngineElectricGenerator

struct ICEngineElectricGeneratorData : BaseGlobalStruct
{

    int NumICEngineGenerators = 0;                                                // number of IC ENGINE Generators specified in input
    bool getICEInput = true;                                                      // When TRUE, calls subroutine to read input file.
    Array1D<ICEngineElectricGenerator::ICEngineGeneratorSpecs> ICEngineGenerator; // dimension to number of machines

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->getICEInput = true;
        this->NumICEngineGenerators = 0;
        this->ICEngineGenerator.deallocate();
    }
};

} // namespace EnergyPlus

#endif
