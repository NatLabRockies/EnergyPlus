// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
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

#ifndef DataSystemVariables_hh_INCLUDED
#define DataSystemVariables_hh_INCLUDED

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Timer.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataSystemVariables {

    int constexpr iUnicode_end = 0; // endline value when Unicode file

    // Shading methods
    enum class ShadingMethod
    {
        Invalid = -1,
        PolygonClipping,
        PixelCounting,
        Scheduled,
        Imported,
        Num
    };

    // Functions

    // Helper to try and locate a file in common folders if it's not found directly (such as when passed as a filename only). Looks in current
    // working folder, programs folder, etc.
    // Returns an empty path if not found.
    [[nodiscard]] fs::path CheckForActualFilePath(EnergyPlusData &state,
                                                  fs::path const &originalInputFilePath, // path (or filename only) as input for object
                                                  const std::string &contextString = std::string());

    void processEnvironmentVariables(EnergyPlusData &state);

} // namespace DataSystemVariables

struct SystemVarsData : BaseGlobalStruct
{
    bool firstTime = true;

    DataSystemVariables::ShadingMethod shadingMethod = DataSystemVariables::ShadingMethod::PolygonClipping; // defines the shading method used

    bool DDOnly = false;        // TRUE if design days (sizingperiod:*) only are to be run.
    bool ReverseDD = false;     // TRUE if reverse design days (reordering sizingperiod:*) are to be run.
    bool FullAnnualRun = false; // TRUE if full annual simulation is to be run.
    bool DeveloperFlag = false; // TRUE if developer flag is turned on. (turns on more displays to console)
    bool TimingFlag = false;    // TRUE if timing flag is turned on. (turns on more timing displays to console)

    bool SutherlandHodgman = true;                 // TRUE if SutherlandHodgman algorithm for polygon clipping is to be used.
    bool SlaterBarsky = false;                     // TRUE if SlaterBarsky algorithm for polygon clipping is to be used for vertical polygons.
    bool DetailedSkyDiffuseAlgorithm = false;      // use detailed diffuse shading algorithm for sky (shading transmittance varies)
    bool DetailedSolarTimestepIntegration = false; // when true, use detailed timestep integration for all solar,shading, etc.

    bool ReportExtShadingSunlitFrac = false; // when true, the sunlit fraction for all surfaces are exported as a csv format output
    bool DisableGroupSelfShading = false;    // when true, defined shadowing surfaces group is ignored when calculating sunlit fraction
    bool DisableAllSelfShading = false;      // when true, all external shadowing surfaces is ignored when calculating sunlit fraction
    bool DisableSelfShadingWithinGroup = false;
    bool DisableSelfShadingBetweenGroup = false;

    int shadingGroupsNum = 0;                 // number of shading groups
    Array1D_string shadingGroupZoneListNames; // array of zone names in user input

    bool TrackAirLoopEnvFlag = false; // If TRUE generates a file with runtime statistics for each HVAC
    //  controller on each air loop
    bool TraceAirLoopEnvFlag = false; // If TRUE generates a trace file with the converged solutions of all
    // HVAC controllers on each air loop at each call to SimAirLoop()
    bool TraceHVACControllerEnvFlag = false; // If TRUE generates a trace file for each individual HVAC
    // controller with all controller iterations

    bool ReportDuringWarmup = false;                      // True when the report outputs even during warmup
    bool ReportDuringHVACSizingSimulation = false;        // true when reporting outputs during HVAC sizing Simulation
    bool ReportDetailedWarmupConvergence = false;         // True when the detailed warmup convergence is requested
    bool UpdateDataDuringWarmupExternalInterface = false; // variable sets in the external interface.

    // This update the value during the warmup added for FMI
    Timer runtimeTimer;
    std::string MinReportFrequency; // String for minimum reporting frequency
    bool SortedIDD = true;          // after processing, use sorted IDD to obtain Defs, etc.
    bool lMinimalShadowing = false; // TRUE if Minimal is to override Solar Distribution flag
    fs::path envinputpath1;
    fs::path envinputpath2;
    bool TestAllPaths = false;
    int iEnvSetThreads = 0;
    bool lEnvSetThreadsInput = false;
    int iepEnvSetThreads = 0;
    bool lepSetThreadsInput = false;
    int iIDFSetThreads = 0;
    bool lIDFSetThreadsInput = false;
    int inumActiveSims = 1;
    bool lnumActiveSims = false;
    int MaxNumberOfThreads = 1;
    int NumberIntRadThreads = 1;
    int iNominalTotSurfaces = 0;
    bool Threading = false;
    bool ciForceTimeStep = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        shadingMethod = DataSystemVariables::ShadingMethod::PolygonClipping;
        DDOnly = false;
        ReverseDD = false;
        FullAnnualRun = false;
        DeveloperFlag = false;
        TimingFlag = false;

        firstTime = true;

        SutherlandHodgman = true;
        SlaterBarsky = false;
        DetailedSkyDiffuseAlgorithm = false;
        DetailedSolarTimestepIntegration = false;

        ReportExtShadingSunlitFrac = false;
        DisableGroupSelfShading = false;
        DisableAllSelfShading = false;

        TrackAirLoopEnvFlag = false;
        TraceAirLoopEnvFlag = false;
        TraceHVACControllerEnvFlag = false;

        ReportDuringWarmup = false;
        ReportDuringHVACSizingSimulation = false;
        ReportDetailedWarmupConvergence = false;
        UpdateDataDuringWarmupExternalInterface = false;

        runtimeTimer = Timer{};
        SortedIDD = true;
        lMinimalShadowing = false;
        TestAllPaths = false;
        iEnvSetThreads = 0;
        lEnvSetThreadsInput = false;
        iepEnvSetThreads = 0;
        lepSetThreadsInput = false;
        iIDFSetThreads = 0;
        lIDFSetThreadsInput = false;
        inumActiveSims = 1;
        lnumActiveSims = false;
        MaxNumberOfThreads = 1;
        NumberIntRadThreads = 1;
        iNominalTotSurfaces = 0;
        Threading = false;
    }
};

} // namespace EnergyPlus

#endif
