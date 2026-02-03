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

#ifndef HeatBalanceKivaManager_hh_INCLUDED
#define HeatBalanceKivaManager_hh_INCLUDED

// Kiva Headers
#include <libkiva/Aggregator.hpp>
#include <libkiva/Ground.hpp>
#include <libkiva/Instance.hpp>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatBalanceKivaManager {

    constexpr int KIVAZONE_UNCONTROLLED = 0;
    constexpr int KIVAZONE_TEMPCONTROL = 1;
    constexpr int KIVAZONE_COMFORTCONTROL = 2;
    constexpr int KIVAZONE_STAGEDCONTROL = 3;

    class KivaWeatherData
    {
    public:
        int intervalsPerHour;

        Real64 annualAverageDrybulbTemp;
        std::vector<Real64> dryBulb;
        std::vector<Real64> windSpeed;
        std::vector<Real64> skyEmissivity;
    };

    class FoundationKiva
    {
    public:
        Kiva::Foundation foundation;
        Kiva::InputBlock intHIns;
        Kiva::InputBlock intVIns;
        Kiva::InputBlock extHIns;
        Kiva::InputBlock extVIns;
        Kiva::InputBlock footing;
        std::string name;
        std::vector<int> surfaces;
        int wallConstructionIndex = 0;
        Real64 assumedIndoorTemperature;
    };

    class KivaInstanceMap
    {
    public:
        KivaInstanceMap(EnergyPlusData &state,
                        Kiva::Foundation &foundation,
                        int floorSurface,
                        std::vector<int> wallSurfaces,
                        int zoneNum,
                        Real64 zoneAssumedTemperature,
                        Real64 floorWeight,
                        int constructionNum,
                        class KivaManager *kmPtr = nullptr);
        Kiva::Instance instance;
        int floorSurface;
        std::vector<int> wallSurfaces;
        int zoneNum;
        int zoneControlType; // Uncontrolled=0, Temperature=1, Operative=2, Comfort=3, HumidityAndTemperature=4
        int zoneControlNum;
        Real64 zoneAssumedTemperature;
        void initGround(EnergyPlusData &state, const KivaWeatherData &kivaWeather);
        int getAccDate(EnergyPlusData &state, const int numAccelaratedTimesteps, const int acceleratedTimestep);
        void
        setInitialBoundaryConditions(EnergyPlusData &state, const KivaWeatherData &kivaWeather, const int date, const int hour, const int timestep);
        void setBoundaryConditions(EnergyPlusData &state);
        Real64 floorWeight;
        int constructionNum = 0;
        class KivaManager *kmPtr;

#ifdef GROUND_PLOT
        void plotDomain(EnergyPlusData &state);
        Kiva::SnapshotSettings ss;
        Kiva::GroundPlot gp;
        std::string debugDir;
        std::size_t plotNum;
#endif
    };

    class KivaManager
    {
    public:
        KivaManager();
        virtual ~KivaManager();
        void readWeatherData(EnergyPlusData &state);
        bool setupKivaInstances(EnergyPlusData &state);
        Real64 getDeepGroundDepth(Kiva::Foundation fnd);
        void initKivaInstances(EnergyPlusData &state);
        void calcKivaInstances(EnergyPlusData &state);
        void defineDefaultFoundation(EnergyPlusData &state);
        void addDefaultFoundation();
        int findFoundation(std::string const &name);
        void calcKivaSurfaceResults(EnergyPlusData &state);

        KivaWeatherData kivaWeather;
        FoundationKiva defaultFoundation;
        std::vector<FoundationKiva> foundationInputs;
        std::vector<KivaInstanceMap> kivaInstances;

        struct ConvectionAlgorithms
        {
            Kiva::ConvectionAlgorithm in;
            Kiva::ConvectionAlgorithm out;
            Kiva::ForcedConvectionTerm f; // Separated out for faster execution
        };

        std::map<int, ConvectionAlgorithms> surfaceConvMap;
        std::map<int, Kiva::Aggregator> surfaceMap;
        Real64 timestep;

        struct Settings
        {
            Settings();

            Real64 soilK;
            Real64 soilRho;
            Real64 soilCp;
            Real64 groundSolarAbs;
            Real64 groundThermalAbs;
            Real64 groundRoughness;
            Real64 farFieldWidth;

            enum DGType
            {
                ZERO_FLUX,
                GROUNDWATER,
                AUTO
            };

            DGType deepGroundBoundary;
            Real64 deepGroundDepth;
            bool autocalculateDeepGroundDepth;
            Real64 minCellDim;
            Real64 maxGrowthCoeff;

            enum TSType
            {
                HOURLY,
                TIMESTEP
            };

            TSType timestepType;
        };

        struct WallGroup
        {
            WallGroup();
            WallGroup(Real64 exposedPerimeter, std::vector<int> wallIDs);
            Real64 exposedPerimeter;
            std::vector<int> wallIDs;
        };

        Settings settings;
        bool defaultAdded;
        int defaultIndex;
    };

} // namespace HeatBalanceKivaManager

} // namespace EnergyPlus

#endif // HeatBalanceKivaManager_hh_INCLUDED
