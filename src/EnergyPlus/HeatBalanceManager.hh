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

#ifndef HeatBalanceManager_hh_INCLUDED
#define HeatBalanceManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/SurfaceOctree.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatBalanceManager {

    struct WarmupConvergence
    {
        // Members
        Array1D_int PassFlag; // one flag (1=Fail), (2=Pass) for each of the 4 conditions of convergence from
        // warmup (PassFlag(1)=Max Temp, PassFlag(2)=Min Temp, PassFlag(3)=Max Heat Load
        // PassFlag(4)=Max Cool Load)
        // Following are stored test values for temperature and loads convergence
        Real64 TestMaxTempValue;     // Max Temperature convergence value=ABS(MaxTempPrevDay(ZoneNum)-MaxTempZone(ZoneNum))
        Real64 TestMinTempValue;     // Min Temperature convergence value=ABS(MinTempPrevDay(ZoneNum)-MinTempZone(ZoneNum))
        Real64 TestMaxHeatLoadValue; // Max Heat Load convergence value=
        //  ABS((MaxHeatLoadZone(ZoneNum)-MaxHeatLoadPrevDay(ZoneNum))/MaxHeatLoadZone(ZoneNum))
        Real64 TestMaxCoolLoadValue; // Max Cool Load convergence value=
        //  ABS((MaxCoolLoadZone(ZoneNum)-MaxCoolLoadPrevDay(ZoneNum))/MaxCoolLoadZone(ZoneNum))

        // Default Constructor
        WarmupConvergence() : PassFlag(4, 2), TestMaxTempValue(0.0), TestMinTempValue(0.0), TestMaxHeatLoadValue(0.0), TestMaxCoolLoadValue(0.0)
        {
        }
    };

    void ManageHeatBalance(EnergyPlusData &state);

    void GetHeatBalanceInput(EnergyPlusData &state);

    void CheckUsedConstructions(EnergyPlusData &state, [[maybe_unused]] bool &ErrorsFound);

    bool CheckValidSimulationObjects(EnergyPlusData &state);

    void SetPreConstructionInputParameters(EnergyPlusData &state);

    void GetProjectControlData(EnergyPlusData &state, bool &ErrorsFound); // Set to true if errors detected during getting data

    void GetSiteAtmosphereData(EnergyPlusData &state, bool &ErrorsFound);

    void GetConstructData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void GetBuildingData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void GetZoneData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void GetSpaceData(EnergyPlusData &state, bool &ErrorsFound);

    int GetGeneralSpaceTypeNum(EnergyPlusData &state);

    void GetZoneLocalEnvData(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void getZoneMRTCalculationData(EnergyPlusData &state);

    void ProcessZoneData(EnergyPlusData &state,
                         std::string const &cCurrentModuleObject,
                         int const ZoneLoop,
                         Array1D_string const &cAlphaArgs,
                         int const NumAlphas,
                         Array1D<Real64> const &rNumericArgs,
                         int const NumNumbers,
                         Array1D_bool const &lNumericFieldBlanks, // Unused
                         Array1D_bool const &lAlphaFieldBlanks,
                         Array1D_string const &cAlphaFieldNames,
                         Array1D_string const &cNumericFieldNames, // Unused
                         bool &ErrorsFound                         // If errors found in input
    );

    void InitHeatBalance(EnergyPlusData &state);

    void AllocateZoneHeatBalArrays(EnergyPlusData &state);

    void AllocateHeatBalArrays(EnergyPlusData &state);

    void RecKeepHeatBalance(EnergyPlusData &state);

    void CheckWarmupConvergence(EnergyPlusData &state);

    void ReportWarmupConvergence(EnergyPlusData &state);

    void UpdateWindowFaceTempsNonBSDFWin(EnergyPlusData &state);

    void ReportHeatBalance(EnergyPlusData &state);

    void OpenShadingFile(EnergyPlusData &state);

    void GetFrameAndDividerData(EnergyPlusData &state); // set to true if errors found in input

    void SearchWindow5DataFile(EnergyPlusData &state,
                               fs::path const &DesiredFilePath,            // File path (or just name) that contains the Window5 constructions.
                               std::string const &DesiredConstructionName, // Name that will be searched for in the Window5 data file
                               bool &ConstructionFound,                    // True if DesiredConstructionName is in the Window5 data file
                               bool &EOFonFile,                            // True if EOF during file read
                               bool &ErrorsFound                           // True if there is a problem with the entry requested from the data file
    );

    void SetStormWindowControl(EnergyPlusData &state);

    void CreateFCfactorConstructions(EnergyPlusData &state,
                                     int &ConstrNum,   // Counter for Constructions
                                     bool &ErrorsFound // If errors found in input
    );

    void CreateAirBoundaryConstructions(EnergyPlusData &state,
                                        int &ConstrNum,   // Counter for Constructions
                                        bool &ErrorsFound // If errors found in input
    );

    void GetIncidentSolarMultiplier(EnergyPlusData &state, bool &ErrorsFound);

    void GetScheduledSurfaceGains(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void CheckScheduledSurfaceGains(EnergyPlusData &state, int const ZoneNum); // Zone number for which error check will be performed

    void CreateTCConstructions(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void SetupComplexFenestrationStateInput(EnergyPlusData &state,
                                            int &ConstrNum, // num of construction items thus far
                                            bool &ErrorsFound);

    void InitConductionTransferFunctions(EnergyPlusData &state);

} // namespace HeatBalanceManager

struct HeatBalanceMgrData : BaseGlobalStruct
{

    bool ManageHeatBalanceGetInputFlag = true;
    bool DoReport = false;
    bool ChangeSet = true; // Toggle for checking storm windows
    bool FirstWarmupWrite = true;
    bool WarmupConvergenceWarning = false;
    bool SizingWarmupConvergenceWarning = false;
    bool ReportWarmupConvergenceFirstWarmupWrite = true;

    std::string CurrentModuleObject; // to assist in getting input
    std::unordered_map<std::string, std::string> UniqueConstructNames;

    // Real Variables for the Heat Balance Simulation
    // Variables used to determine warmup convergence
    Array1D<Real64> MaxCoolLoadPrevDay; // Max cooling load from the previous day
    Array1D<Real64> MaxCoolLoadZone;    // Maximum zone cooling load from the current day
    Array1D<Real64> MaxHeatLoadPrevDay; // Max heating load from the previous day
    Array1D<Real64> MaxHeatLoadZone;    // Maximum zone heating load from the current day
    Array1D<Real64> MaxTempPrevDay;     // Max temperature from the previous day
    Array1D<Real64> MaxTempZone;        // Maximum zone temperature from the current day
    Array1D<Real64> MinTempPrevDay;     // Min temperature from the previous day
    Array1D<Real64> MinTempZone;        // Minimum zone temperature from the current day

    // Variables used to report difference in temperature and load from the last two warmup days
    Array1D<Real64> WarmupTempDiff;     // Temperature difference between the last two warmup days
    Array1D<Real64> WarmupLoadDiff;     // Zone load differences between the last two warmup days
    Array1D<Real64> TempZoneSecPrevDay; // Zone air temperature from the second last warmup day
    Array1D<Real64> LoadZoneSecPrevDay; // Zone load from the second last warmup day
    Array1D<Real64> TempZonePrevDay;    // Zone air temperature from the previous day
    Array1D<Real64> LoadZonePrevDay;    // Zone load from the previuos day
    Array1D<Real64> TempZone;           // Zone air temperature from the current warmup day
    Array1D<Real64> LoadZone;           // Zone load from the current warmup day

    Array2D<Real64> TempZoneRpt;       // Zone air temperature to report (average over all warmup days)
    Array1D<Real64> TempZoneRptStdDev; // Zone air temperature to report (std dev over all warmup days)
    Array2D<Real64> LoadZoneRpt;       // Zone load to report (average over all warmup days)
    Array1D<Real64> LoadZoneRptStdDev; // Zone load to report (std dev over all warmup days)
    Array2D<Real64> MaxLoadZoneRpt;    // Maximum zone load for reporting calcs

    int CountWarmupDayPoints; // Count of warmup timesteps (to achieve warmup)

    Array1D<HeatBalanceManager::WarmupConvergence> WarmupConvergenceValues;
    SurfaceOctreeCube surfaceOctree;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {

        ManageHeatBalanceGetInputFlag = true;
        UniqueConstructNames.clear();
        DoReport = false;
        ChangeSet = true;
        FirstWarmupWrite = true;
        WarmupConvergenceWarning = false;
        SizingWarmupConvergenceWarning = false;
        ReportWarmupConvergenceFirstWarmupWrite = true;

        CurrentModuleObject = std::string();
        MaxCoolLoadPrevDay.clear();
        MaxCoolLoadZone.clear();
        MaxHeatLoadPrevDay.clear();
        MaxHeatLoadZone.clear();
        MaxTempPrevDay.clear();
        MaxTempZone.clear();
        MinTempPrevDay.clear();
        MinTempZone.clear();
        WarmupTempDiff.clear();
        WarmupLoadDiff.clear();
        TempZoneSecPrevDay.clear();
        LoadZoneSecPrevDay.clear();
        TempZonePrevDay.clear();
        LoadZonePrevDay.clear();
        TempZone.clear();
        LoadZone.clear();
        TempZoneRpt.clear();
        TempZoneRptStdDev.clear();
        LoadZoneRpt.clear();
        LoadZoneRptStdDev.clear();
        MaxLoadZoneRpt.clear();

        CountWarmupDayPoints = int();

        WarmupConvergenceValues.clear();
        surfaceOctree = SurfaceOctreeCube();
    }
};

} // namespace EnergyPlus

#endif
