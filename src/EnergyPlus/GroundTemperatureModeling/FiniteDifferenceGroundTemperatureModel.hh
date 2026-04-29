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

#ifndef FiniteDifferenceGroundTemperatureModel_hh_INCLUDED
#define FiniteDifferenceGroundTemperatureModel_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace GroundTemp {

    // Derived class for Finite-Difference Model
    class FiniteDiffGroundTempsModel : public BaseGroundTempsModel
    {

        static int constexpr maxYearsToIterate = 10;

        Real64 rhoCp_soil_liq_1 = 0.0;
        Real64 rhoCP_soil_liq = 0.0;
        Real64 rhoCP_soil_transient = 0.0;
        Real64 rhoCP_soil_ice = 0.0;

    public:
        Real64 baseConductivity = 0.0;
        Real64 baseDensity = 0.0;
        Real64 baseSpecificHeat = 0.0;
        int totalNumCells = 0;
        Real64 timeStepInSeconds = 0.0;
        Real64 evapotransCoeff = 0.0;
        Real64 saturatedWaterContent = 0.0;
        Real64 waterContent = 0.0;
        Real64 annualAveAirTemp = 0.0;
        Real64 minDailyAirTemp = 100.0;  // Set hi. Will be reset later
        Real64 maxDailyAirTemp = -100.0; // Set low. Will be reset later
        Real64 dayOfMinDailyAirTemp = 1;
        Real64 depth = 0.0;
        Real64 simTimeInDays = 0.0;

        struct instanceOfCellData
        {

            struct properties
            {
                Real64 conductivity = 0.0;
                Real64 density = 0.0;
                Real64 specificHeat = 0.0;
                Real64 diffusivity = 0.0;
                Real64 rhoCp = 0.0;
            };

            properties props;

            int index = 0;
            Real64 thickness = 0.0;
            Real64 minZValue = 0.0;
            Real64 maxZValue = 0.0;
            Real64 temperature = 0.0;
            Real64 temperature_prevIteration = 0.0;
            Real64 temperature_prevTimeStep = 0.0;
            Real64 temperature_finalConvergence = 0.0;
            Real64 beta = 0.0;
            Real64 volume = 0.0;
            Real64 conductionArea = 1.0; // Assumes 1 m2
        };

        Array1D<instanceOfCellData> cellArray;

        struct instanceOfWeatherData
        {
            Real64 dryBulbTemp = 0.0;
            Real64 relativeHumidity = 0.0;
            Real64 windSpeed = 0.0;
            Real64 horizontalRadiation = 0.0;
            Real64 airDensity = 0.0;
        };

        Array1D<instanceOfWeatherData> weatherDataArray;

        static FiniteDiffGroundTempsModel *FiniteDiffGTMFactory(EnergyPlusData &state, const std::string &objectName);

        void getWeatherData(EnergyPlusData &state);

        void initAndSim(EnergyPlusData &state);

        void developMesh();

        void performSimulation(EnergyPlusData &state);

        void updateSurfaceCellTemperature(const EnergyPlusData &state);

        void updateGeneralDomainCellTemperature(int cell);

        void updateBottomCellTemperature();

        void initDomain(EnergyPlusData &state);

        bool checkFinalTemperatureConvergence(const EnergyPlusData &state);

        bool checkIterationTemperatureConvergence();

        void updateIterationTemperatures();

        void updateTimeStepTemperatures(const EnergyPlusData &state);

        void doStartOfTimeStepInits();

        Real64 getGroundTemp(EnergyPlusData &state) override;

        Real64 getGroundTempAtTimeInSeconds(EnergyPlusData &state, Real64 depth, Real64 timeInSecondsOfSim) override;

        Real64 getGroundTempAtTimeInMonths(EnergyPlusData &state, Real64 depth, int monthOfSim) override;

        void evaluateSoilRhoCpInit();

        void evaluateSoilRhoCpCell(int cell);

        static Real64 interpolate(Real64 x, Real64 x_hi, Real64 x_low, Real64 y_hi, Real64 y_low);

        Array2D<Real64> groundTemps;

        Array1D<Real64> cellDepths;

        enum surfaceTypes
        {
            surfaceCoverType_bareSoil = 1,
            surfaceCoverType_shortGrass = 2,
            surfaceCoverType_longGrass = 3
        };
    };

} // namespace GroundTemp
} // namespace EnergyPlus

#endif
