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

#ifndef SolarCollectors_hh_INCLUDED
#define SolarCollectors_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SolarCollectors {

    enum struct TestTypeEnum
    {
        INVALID = -1,
        INLET,
        AVERAGE,
        OUTLET,
        NUM
    };
    constexpr std::array<std::string_view, static_cast<int>(TestTypeEnum::NUM)> testTypesUC = {"INLET", "AVERAGE", "OUTLET"};

    struct ParametersData
    {
        std::string Name;                                    // Name of solar collector parameters
        Real64 Area = 0.0;                                   // Gross area of collector (m2)
        Real64 TestMassFlowRate = 0.0;                       // Test volumetric flow rate (m3/s)
        TestTypeEnum TestType = TestTypeEnum::INLET;         // Test correlation type (INLET | AVERAGE | OUTLET)
        Real64 eff0 = 0.0;                                   // Coefficient 1 of efficiency equation (Y-intercept)
        Real64 eff1 = 0.0;                                   // Coefficient 2 of efficiency equation (1st order)
        Real64 eff2 = 0.0;                                   // Coefficient 3 of efficiency equation (2nd order)
        Real64 iam1 = 0.0;                                   // Coefficient 2 of incident angle modifier (1st order)
        Real64 iam2 = 0.0;                                   // Coefficient 3 of incident angle modifier (2nd order)
        Real64 Volume = 0.0;                                 // collector water net volume (m3)
        Real64 SideHeight = 0.0;                             // collector side height (m)
        Real64 ThermalMass = 0.0;                            // thermal mass of the absorber plate (J/m2C)
        Real64 ULossSide = 0.0;                              // heat loss conductance for collector side (W/m2C)
        Real64 ULossBottom = 0.0;                            // heat loss conductance for collector bottom (W/m2C)
        Real64 AspectRatio = 0.0;                            // collector aspect ratio (dimensionless)
        int NumOfCovers = 0;                                 // number of transparent collector covers
        Real64 CoverSpacing = 0.0;                           // collector cover spacings (m)
        std::array<Real64, 2> RefractiveIndex = {0.0};       // refractive index of inner and outer covers (dimensionless)
        std::array<Real64, 2> ExtCoefTimesThickness = {0.0}; // extinction coefficient times thickness of covers (dimensionless)
        std::array<Real64, 2> EmissOfCover = {0.0};          // emissivity of inner and outer covers (dimensionless)
        Real64 EmissOfAbsPlate = 0.0;                        // emissivity Of absorber plate (dimensionless)
        Real64 AbsorOfAbsPlate = 0.0;                        // absorptance of the absorber plate (dimensionless)

        Real64 IAM(EnergyPlusData &state, Real64 IncidentAngle);
    };

    struct CollectorData : PlantComponent
    {
        std::string Name;                                                            // Name of solar collector
        std::string BCType;                                                          // Boundary condition Type
        std::string OSCMName;                                                        // OtherSideConditionsModel
        int VentCavIndex = 0;                                                        // index of ventilated cavity object
        DataPlant::PlantEquipmentType Type = DataPlant::PlantEquipmentType::Invalid; // Plant Side Connection: 'Type' assigned in DataPlant
        PlantLocation plantLoc;                                                      // Water plant loop component location object
        bool Init = true;                                                            // Flag for initialization:  TRUE means do the init
        bool InitSizing = true;                                                      // Flag for initialization of plant sizing
        int Parameters = 0;                                                          // Parameters object number
        int Surface = 0;                                                             // Surface object number
        int InletNode = 0;                                                           // Inlet node
        Real64 InletTemp = 0.0;                                                      // Inlet temperature from plant (C)
        int OutletNode = 0;                                                          // Outlet node
        Real64 OutletTemp = 0.0;      // Outlet temperature or stagnation temperature in the collector (C)
        Real64 MassFlowRate = 0.0;    // Mass flow rate through the collector (kg/s)
        Real64 MassFlowRateMax = 0.0; // Maximum mass flow rate through the collector (kg/s)
        Real64 VolFlowRateMax = 0.0;  // Maximum volumetric flow rate through the collector (m3/s)
        int ErrIndex = 0;             // Error index for recurring error
        int IterErrIndex = 0;         // Error index for recurring error (iteration - did not converge)
        // Report variables
        Real64 IncidentAngleModifier = 0.0; // Net incident angle modifier
        Real64 Efficiency = 0.0;            // Thermal efficiency of solar energy conversion
        Real64 Power = 0.0;                 // Heat gain or loss to collector fluid (W)
        Real64 HeatGain = 0.0;              // Heat gain to collector fluid (W)
        Real64 HeatLoss = 0.0;              // Heat loss from collector fluid (W)
        Real64 Energy = 0.0;                // Energy gained (or lost) to collector fluid (J)
        // Report variables
        Real64 HeatRate = 0.0;           // Collector useful Heat gain rate [W]
        Real64 HeatEnergy = 0.0;         // Collector useful Heat gain energy [J]
        Real64 StoredHeatRate = 0.0;     // net heat gain or loss rate of the collector fluid [W]
        Real64 StoredHeatEnergy = 0.0;   // net heat gain or loss energy of the collector fluid [J]
        Real64 HeatGainRate = 0.0;       // Collector useful Heat gain rate [W]
        Real64 SkinHeatLossRate = 0.0;   // collector skin heat loss rate [W]
        Real64 CollHeatLossEnergy = 0.0; // collector skin heat loss energy[J]
        Real64 TauAlpha = 0.0;           // Transmittance-absorptance product total radiation
        Real64 UTopLoss = 0.0;           // Over all top loss coefficient [W/m2.C]
        Real64 TempOfWater = 0.0;        // average temperature of the collector water [C]
        Real64 TempOfAbsPlate = 0.0;     // average temperature of the abs plate [C]
        Real64 TempOfInnerCover = 0.0;   // temperature of the collector inner cover [C]
        Real64 TempOfOuterCover = 0.0;   // temperature of the collector inner cover [C]
        // Data from elsewhere and calculated
        Real64 TauAlphaSkyDiffuse = 0.0;                   // Transmittance-absorptance product sky diffuse radiation
        Real64 TauAlphaGndDiffuse = 0.0;                   // Transmittance-absorptance product grn diffuse radiation
        Real64 TauAlphaBeam = 0.0;                         // Transmittance-absorptance product beam radiation
        std::array<Real64, 2> CoversAbsSkyDiffuse = {0.0}; // sky diffuse solar absorptance of cover
        std::array<Real64, 2> CoversAbsGndDiffuse = {0.0}; // ground diffuse solar absorptance of cover
        std::array<Real64, 2> CoverAbs = {0.0};            // solar rad weighted covers absorptance
        Real64 TimeElapsed = 0.0;                          // Fraction of the current hour that has elapsed (h)
        // Saved in order to identify the beginning of a new system time
        Real64 UbLoss = 0.0;                 // Over all bottom loss coefficient [W/m2C]
        Real64 UsLoss = 0.0;                 // Over all side loss coefficient [W/m2C]
        Real64 AreaRatio = 0.0;              // Side area to collector area ratio [-]
        Real64 RefDiffInnerCover = 0.0;      // diffuse reflectance of the inner cover (cover 1) from bottom
        Real64 SavedTempOfWater = 0.0;       // water temp carried from time step to time step [C]
        Real64 SavedTempOfAbsPlate = 0.0;    // abs plate temp carried from time step to time step [C]
        Real64 SavedTempOfInnerCover = 0.0;  // inner cover temp carried from time step to time step [C]
        Real64 SavedTempOfOuterCover = 0.0;  // outer cover temp carried from time step to time step [C]
        Real64 SavedTempCollectorOSCM = 0.0; // Temperature of collector back from OSCM at previous time step [C]
        Real64 Length = 0.0;                 // characteristic length of the abs plate
        Real64 TiltR2V = 0.0;                // collector tilt angle from the vertical [degree]
        Real64 Tilt = 0.0;                   // collector tilt angle from the horizontal [degree]
        Real64 CosTilt = 0.0;                // cosine of collector tilt angle [-]
        Real64 SinTilt = 0.0;                // sine of 1.8 times collector tilt angle [-]
        Real64 SideArea = 0.0;               // weighted collector side area (m2)
        Real64 Area = 0.0;                   // collector area (m2)
        Real64 Volume = 0.0;                 // collector net volume (m3)
        bool OSCM_ON = false;                // Boundary condition is OSCM
        bool InitICS = false;                // used to initialize ICS variables only
        bool SetLoopIndexFlag = true;
        bool SetDiffRadFlag = true;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void setupOutputVars(EnergyPlusData &state);

        void initialize(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void CalcTransRefAbsOfCover(EnergyPlusData &state,
                                    Real64 IncidentAngle,                         // Angle of incidence (radians)
                                    Real64 &TransSys,                             // cover system solar transmittance
                                    Real64 &ReflSys,                              // cover system solar reflectance
                                    Real64 &AbsCover1,                            // Inner cover solar absorptance
                                    Real64 &AbsCover2,                            // Outer cover solar absorptance
                                    ObjexxFCL::Optional_bool_const InOUTFlag = _, // flag for calc. diffuse solar refl of cover from inside out
                                    ObjexxFCL::Optional<Real64> RefSysDiffuse = _ // cover system solar reflectance from inner to outer cover
        ) const;

        void CalcSolarCollector(EnergyPlusData &state);

        void CalcICSSolarCollector(EnergyPlusData &state);

        void CalcTransAbsorProduct(EnergyPlusData &state, Real64 IncidAngle);

        void CalcHeatTransCoeffAndCoverTemp(EnergyPlusData &state);

        static void ICSCollectorAnalyticalSolution(EnergyPlusData &state,
                                                   Real64 SecInTimeStep,     // seconds in a time step
                                                   Real64 a1,                // coefficient of ODE for Tp
                                                   Real64 a2,                // coefficient of ODE for Tp
                                                   Real64 a3,                // coefficient of ODE for Tp
                                                   Real64 b1,                // coefficient of ODE for TW
                                                   Real64 b2,                // coefficient of ODE for TW
                                                   Real64 b3,                // coefficient of ODE for TW
                                                   Real64 TempAbsPlateOld,   // absorber plate temperature at previous time step [C]
                                                   Real64 TempWaterOld,      // collector water temperature at previous time step [C]
                                                   Real64 &TempAbsPlate,     // absorber plate temperature at current time step [C]
                                                   Real64 &TempWater,        // collector water temperature at current time step [C]
                                                   bool AbsorberPlateHasMass // flag for absorber thermal mass
        );

        static Real64 CalcConvCoeffBetweenPlates(Real64 TempSurf1, // temperature of surface 1
                                                 Real64 TempSurf2, // temperature of surface 1
                                                 Real64 AirGap,    // characteristic length [m]
                                                 Real64 CosTilt,   // cosine of surface tilt angle relative to the horizontal
                                                 Real64 SinTilt    // sine of surface tilt angle relative to the horizontal
        );

        static Real64 CalcConvCoeffAbsPlateAndWater(EnergyPlusData &state,
                                                    Real64 TAbsorber, // temperature of absorber plate [C]
                                                    Real64 TWater,    // temperature of water [C]
                                                    Real64 Lc,        // characteristic length [m]
                                                    Real64 TiltR2V    // collector tilt angle relative to the vertical [degree]
        );

        static void GetExtVentedCavityIndex(EnergyPlusData &state, int SurfacePtr, int &VentCavIndex);

        void update(EnergyPlusData &state);

        void report(EnergyPlusData &state);

        void oneTimeInit_new(EnergyPlusData &state) override;

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetSolarCollectorInput(EnergyPlusData &state);

} // namespace SolarCollectors

struct SolarCollectorsData : BaseGlobalStruct
{
    int NumOfCollectors = 0;
    int NumOfParameters = 0;
    bool GetInputFlag = true;
    EPVector<SolarCollectors::ParametersData> Parameters;
    EPVector<SolarCollectors::CollectorData> Collector;
    std::unordered_map<std::string, std::string> UniqueParametersNames;
    std::unordered_map<std::string, std::string> UniqueCollectorNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) SolarCollectorsData();
    }
};

} // namespace EnergyPlus

#endif
