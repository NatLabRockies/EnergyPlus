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

#ifndef CTElectricGenerator_hh_INCLUDED
#define CTElectricGenerator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Curve {
    struct Curve;
}

namespace CTElectricGenerator {

    struct CTGeneratorData : PlantComponent
    {
        // Members
        std::string Name;                                   // user identifier
        std::string TypeOf = "Generator:CombustionTurbine"; // Type of Generator
        GeneratorType CompType_Num = GeneratorType::CombTurbine;
        Constant::eFuel FuelType;                      // Type of Fuel - DIESEL, GASOLINE, GAS
        Real64 RatedPowerOutput = 0.0;                 // W - design nominal capacity of Generator
        int ElectricCircuitNode = 0;                   // Electric Circuit Node
        Real64 MinPartLoadRat = 0.0;                   // (CT MIN) min allowed operating frac full load
        Real64 MaxPartLoadRat = 0.0;                   // (CT MAX) max allowed operating frac full load
        Real64 OptPartLoadRat = 0.0;                   // (CT BEST) optimal operating frac full load
        Real64 FuelEnergyUseRate = 0.0;                // (EFUEL) rate of Fuel Energy Required to run COMBUSTION turbine (W)
        Real64 FuelEnergy = 0.0;                       // Amount of Fuel Energy Required to run COMBUSTION turbine (J)
        Curve::Curve *PLBasedFuelInputCurve = nullptr; // (FUL1GC) Curve for Part Load Ratio Based Fuel Input
        // Coefficients Poly Fit
        Curve::Curve *TempBasedFuelInputCurve = nullptr; // (FUL2GC) Curve for Ambient Temperature Based Fuel Input
        // Coeff Poly Fit
        Real64 ExhaustFlow = 0.0;                        // (FEX) Exhaust Gas Flow Rate cubic meters per second???
        Curve::Curve *ExhaustFlowCurve = nullptr;        // (FEXGC) Curve for Exhaust Gas Flow Rate Input Coef Poly Fit
        Real64 ExhaustTemp = 0.0;                        // (TEX) Exhaust Gas Temperature in C
        Curve::Curve *PLBasedExhaustTempCurve = nullptr; // (TEX1GC) Curve for Part Load Ratio Based Exhaust Temp Input
        // Coeffs Poly Fit
        Curve::Curve *TempBasedExhaustTempCurve = nullptr; // (TEX2GC) Curve for Ambient Temperature Based Exhaust Gas Temp to
        // Fuel Energy Input Coeffs Poly Fit
        Real64 QLubeOilRecovered = 0.0;                 // (ELUBE) Recovered Lube Oil Energy (W)
        Real64 QExhaustRecovered = 0.0;                 // (EEX) Recovered Exhaust heat  (W)
        Real64 QTotalHeatRecovered = 0.0;               // total heat recovered (W)
        Real64 LubeOilEnergyRec = 0.0;                  // Recovered Lube Oil Energy (J)
        Real64 ExhaustEnergyRec = 0.0;                  // Recovered Exhaust heat  (J)
        Real64 TotalHeatEnergyRec = 0.0;                // total heat recovered (J)
        Curve::Curve *QLubeOilRecoveredCurve = nullptr; // (ELUBEGC) Curve for Recoverable Lube Oil heat Input Coef Poly Fit
        Real64 UA = 0.0;                                // (UACGC) exhaust gas Heat Exchanger UA
        std::array<Real64, 2> UACoef = {0.0};           // Heat Exchanger UA  Coeffs Poly Fit
        Real64 MaxExhaustperCTPower = 0.0;              // MAX EXHAUST FLOW PER W POWER OUTPUT COEFF
        Real64 DesignHeatRecVolFlowRate = 0.0;          // m3/s, Design Water mass flow rate through heat recovery loop
        Real64 DesignHeatRecMassFlowRate = 0.0;         // kg/s, Design Water mass flow rate through heat recovery loop
        Real64 DesignMinExitGasTemp = 0.0;              // Steam Saturation Temperature (C)
        Real64 DesignAirInletTemp = 0.0;                // Design Turbine Air Inlet Temperature (C)
        Real64 ExhaustStackTemp = 0.0;                  // turbine exhaust gas temp (C)
        bool HeatRecActive = false;                     // true when design max flow rate > 0
        int HeatRecInletNodeNum = 0;                    // Node number on the heat recovery inlet side of the condenser
        int HeatRecOutletNodeNum = 0;                   // Node number on the heat recovery outlet side of the condenser
        Real64 HeatRecInletTemp = 0.0;                  // Inlet Temperature of the heat recovery fluid
        Real64 HeatRecOutletTemp = 0.0;                 // Outlet Temperature of the heat recovery fluid
        Real64 HeatRecMdot = 0.0;                       // reporting: Heat Recovery Loop Mass flow rate
        PlantLocation HRPlantLoc;                       // cooling water plant loop component index, for heat recovery
        Real64 FuelMdot = 0.0;                          // reporting: Fuel Amount used (kg/s)
        Real64 FuelHeatingValue = 0.0;                  // Heating Value for Fuel in (kJ/kg)
        Real64 ElecPowerGenerated = 0.0;                // reporting: power generated (W)
        Real64 ElecEnergyGenerated = 0.0;               // reporting: power generated (W)
        Real64 HeatRecMaxTemp = 0.0;                    // Max Temp that can be produced in heat recovery
        int OAInletNode = 0;                            // optional inlet node index pointer for outdoor air for combustion
        bool MyEnvrnFlag = true;
        bool MyPlantScanFlag = true;
        bool MySizeAndNodeInitFlag = true;
        bool CheckEquipName = true;
        bool MyFlag = true;

        // Default Constructor
        CTGeneratorData() : HRPlantLoc{}
        {
        }

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        void setupOutputVars(EnergyPlusData &state);

        void InitCTGenerators(EnergyPlusData &state, bool RunFlag, bool FirstHVACIteration);

        void CalcCTGeneratorModel(EnergyPlusData &state, bool RunFlag, Real64 MyLoad, bool FirstHVACIteration);

        static CTGeneratorData *factory(EnergyPlusData &state, std::string const &objectName);

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetCTGeneratorInput(EnergyPlusData &state);

} // namespace CTElectricGenerator

struct CTElectricGeneratorData : BaseGlobalStruct
{
    bool getCTInputFlag = true;
    Array1D<CTElectricGenerator::CTGeneratorData> CTGenerator;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) CTElectricGeneratorData();
    }
};

} // namespace EnergyPlus

#endif
