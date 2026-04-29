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

#ifndef EcoRoofManager_hh_INCLUDED
#define EcoRoofManager_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace EcoRoofManager {

    // Functions

    void CalcEcoRoof(EnergyPlusData &state,
                     int const SurfNum,   // Indicator of Surface Number for the current surface
                     int const ConstrNum, // Indicator for construction index for the current surface
                     Real64 &TempExt      // Exterior temperature boundary condition
    );

    void initEcoRoofFirstTime(EnergyPlusData &state, int const SurfNum, int const ConstrNum);

    void initEcoRoof(EnergyPlusData &state, int const SurfNum, int const ConstrNum);

    void UpdateSoilProps(EnergyPlusData &state,
                         Real64 &Moisture,
                         Real64 &MeanRootMoisture,
                         Real64 const MoistureMax,
                         Real64 const MoistureResidual,
                         Real64 const SoilThickness,
                         Real64 const Vfluxf, // Water mass flux from vegetation [m/s]
                         Real64 const Vfluxg, // Water mass flux from soil surface [m/s]
                         int const ConstrNum, // Indicator for construction index for the current surface
                         Real64 &Alphag,
                         int const unit,    // unused1208
                         Real64 const Tg,   // unused1208
                         Real64 const Tf,   // unused1208
                         Real64 const Qsoil // unused1208
    );

    void CalculateEcoRoofSolar(EnergyPlusData &state,
                               Real64 &RS, // Solar radiation on a horizontal surface (roof)
                               Real64 &f1, // Solar term in Stomatal Resistance
                               int const SurfNum);

} // namespace EcoRoofManager

struct EcoRoofManagerData : BaseGlobalStruct
{

    Real64 CumRunoff; // Cumulative runoff, updated each time step (m) mult by roof area to get volume
    Real64 CumET;     // Cumulative evapotranspiration from soil and plants (m)
    Real64 CumPrecip;
    Real64 CumIrrigation;                             // Cumulative irrigation, updated each time step (m) mult by roof area to get volume
    std::array<Real64, 12> MonthlyIrrigation = {0.0}; // accumulator for monthly irrigation ActualAmount in mm
    Real64 CurrentRunoff;
    Real64 CurrentET;
    Real64 CurrentPrecipitation; // units of (m) per timestep
    Real64 CurrentIrrigation;    // units of (m) per timestep

    Real64 Tfold; // leaf temperature from the previous time step
    Real64 Tgold; // ground temperature from the previous time step
    bool EcoRoofbeginFlag = true;
    bool CalcEcoRoofMyEnvrnFlag = true;

    // static variables extracted from functions
    int FirstEcoSurf = 0;             // Indicates lowest numbered surface that is an ecoroof, used to determine WHEN to updatesoilProps...
    bool QuickConductionSurf = false; // indicator for quick conduction surface
    Real64 LAI = 0.2;                 // Leaf area index
    Real64 epsilonf = 0.95;           // Leaf Emisivity
    Real64 epsilong = 0.95;           // Soil Emisivity
    Real64 Alphag = 0.3;              // Ground Albedo
    Real64 Alphaf = 0.2;              // Leaf Albedo (reflectivity to solar radiation)
    Real64 e0 = 2.0;                  // Windless lower limit of exchange coefficient (from FASST docs)
    Real64 RH = 50.0;                 // Relative humidity (%)
    Real64 Pa = 101325.0;             // Atmospheric Pressure (PA)
    Real64 Tg = 10.0;                 // Ground Surface temperature C ***** FROM PREVIOUS TIME STEP
    Real64 Tf = 10.0;                 // Leaf temperature C ***** FROM PREVIOUS TIME STEP
    Real64 Zf = 0.2;                  // Height of plants (m)
    // DJS Oct 2007 release - note I got rid of the initialization of moisture and meanrootmoisture here as these
    // values are now set at beginning of each new DD and each new warm-up loop.
    Real64 Moisture = 0.0; // m^3/m^3.The moisture content in the soil is the value provided by a user
    Real64 MoistureResidual =
        0.05;                      // m^3/m^3. Residual & maximum water contents are unique to each material. See Frankenstein et al (2004b) for data.
    Real64 MoistureMax = 0.5;      // Maximum volumetric moisture content (porosity) m^3/m^3
    Real64 MeanRootMoisture = 0.0; // Mean value of root moisture m^3/m^3
    Real64 SoilThickness = 0.2;    // Soil thickness (m)
    Real64 StomatalResistanceMin = 0.0; // s/m . ! Minimum stomatal resistance is unique for each veg. type.
    Real64 f3 = 1.0;                    // As the value of gd for tall grass is 0, then f3 = 1
    // ECMWF 2002 CY25R1 report has gd=0.0 for all veg except trees where gd=0.03.
    Real64 Zog = 0.001;     // Ground roughness length scale (m)
    Real64 Za = 2.0;        // Instrument height where atmospheric wind speed is measured (m)
    Real64 Lf = 0.0;        // latent heat flux
    Real64 Vfluxf = 0.0;    // Water evapotr. rate associated with latent heat from vegetation [m/s]
    Real64 Qsoil = 0.0;     // heat flux from the soil layer
    Real64 sheatf = 0.0;    // sensible heat flux coeff for foliage (W/m^2K)
    Real64 sensiblef = 0.0; // sensible heat transfer TO foliage (W/m^2) DJS Jan 2011
    Real64 sheatg = 0.0;    // intermediate calculation variable - sensible flux coef (W/m^2K for ground)
    Real64 sensibleg = 0.0; // sensible heat flux TO ground (w/m^2) DJS Jan 2011
    Real64 Lg = 0.0;        // latent heat flux from ground surface
    Real64 Vfluxg = 0.0;    // Water evapotr. rate associated with latent heat from ground surface [m/s]
    Real64 TopDepth = 0.0;  // Thickness of "near-surface" soil layer
    Real64 RootDepth = 0.0; // Thickness of "root zone" soil layer //Autodesk Was used uninitialized
    // Note TopDepth+RootDepth = thickness of ecoroof soil layer
    Real64 TimeStepZoneSec = 0.0;               // Seconds per TimeStep
    Real64 DryCond = 0.0;                       // Dry soil value of conductivity
    Real64 DryDens = 0.0;                       // Dry soil value of density
    Real64 DryAbsorp = 0.0;                     // Dry soil value of solar absorptance (1-albedo)
    Real64 DrySpecHeat = 0.0;                   // Dry soil value of specific heat
    bool UpdatebeginFlag = true;                // one time flag
    Real64 CapillaryPotentialTop = -3.8997;     // This variable keeps track of the capillary potential of the soil in both layers and time (m)
    Real64 CapillaryPotentialRoot = -3.8997;    // This variable keeps track of the capillary potential of the soil in both layers and time (m)
    Real64 SoilHydroConductivityTop = 8.72e-6;  // This is the soil water conductivity in the soil (m/s)
    Real64 SoilHydroConductivityRoot = 8.72e-6; // This is the soil water conductivity in the soil (m/s)
    Real64 SoilConductivityAveTop = 8.72e-6;    // This is the average soil water conductivity (m/s)
    Real64 SoilConductivityAveRoot = 8.72e-6;
    Real64 RelativeSoilSaturationTop =
        0.0; // Relative Soil Saturation (soil moisture-residual soil moisture)/(saturation soil moisture-residual soil moisture)
    Real64 RelativeSoilSaturationRoot = 0.0;
    Real64 TestMoisture = 0.15; // This makes sure that the moisture cannot change by too much in each step
    int ErrIndex = 0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->EcoRoofbeginFlag = true;
        this->CalcEcoRoofMyEnvrnFlag = true;
        this->FirstEcoSurf = 0;
        this->QuickConductionSurf = false;
        this->LAI = 0.2;
        this->epsilonf = 0.95;
        this->epsilong = 0.95;
        this->Alphag = 0.3;
        this->Alphaf = 0.2;
        this->e0 = 2.0;
        this->RH = 50.0;
        this->Pa = 101325.0;
        this->Tg = 10.0;
        this->Tf = 10.0;
        this->Zf = 0.2;
        this->Moisture = 0.0;
        this->MoistureResidual = 0.05;
        this->MoistureMax = 0.5;
        this->MeanRootMoisture = 0.0;
        this->SoilThickness = 0.2;
        this->StomatalResistanceMin = 0.0;
        this->f3 = 1.0;
        this->Zog = 0.001;
        this->Za = 2.0;
        this->Lf = 0.0;
        this->Vfluxf = 0.0;
        this->Qsoil = 0.0;
        this->sheatf = 0.0;
        this->sensiblef = 0.0;
        this->sheatg = 0.0;
        this->sensibleg = 0.0;
        this->Lg = 0.0;
        this->Vfluxg = 0.0;
        this->TopDepth = 0.0;
        this->RootDepth = 0.0;
        // Note TopDepth+RootDepth = thickness of e
        this->TimeStepZoneSec = 0.0;
        this->DryCond = 0.0;
        this->DryDens = 0.0;
        this->DryAbsorp = 0.0;
        this->DrySpecHeat = 0.0;
        this->UpdatebeginFlag = true;
        this->CapillaryPotentialTop = -3.8997;
        this->CapillaryPotentialRoot = -3.8997;
        this->SoilHydroConductivityTop = 8.72e-6;
        this->SoilHydroConductivityRoot = 8.72e-6;
        this->SoilConductivityAveTop = 8.72e-6;
        this->SoilConductivityAveRoot = 8.72e-6;
        this->RelativeSoilSaturationTop = 0.0;
        this->RelativeSoilSaturationRoot = 0.0;
        this->TestMoisture = 0.15;
        this->ErrIndex = 0;
    }
};

} // namespace EnergyPlus

#endif
