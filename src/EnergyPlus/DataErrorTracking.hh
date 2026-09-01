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

#ifndef DataErrorTracking_hh_INCLUDED
#define DataErrorTracking_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataErrorTracking {

    // See also in UtilityRoutines.cc: struct ErrorSummaryInfo and
    // constexpr std::array<ErrorSummaryInfo, static_cast<size_t>(ErrorSummaryType::Num)> ErrorSummaries
    enum class ErrorSummaryType
    {
        InterZoneSurfaceAreaMismatch = 0,  // "InterZone Surface Areas"
        InterZoneSurfaceDifferentZones,    // "CAUTION -- Interzone"
        NodeConnectionErrors,              // "Node Connection Error"
        InterZoneSurfaceAzimuthMismatch,   // "InterZone Surface Azimu"
        InterZoneSurfaceTiltMismatch,      // "InterZone Surface Tilts"
        NonPlanarSurfaces,                 // "Suspected non-planar"
        DeprecatedFeaturesOrKeyValues,     // "Deprecated"
        IncorrectFloorTilt,                // "Floor Tilt="
        IncorrectRoofCeilingTilt,          // "Roof/Ceiling Tilt="
        IncompleteViewFactors,             // "View factors not"
        UnbalancedExhaustAirFlow,          // "Unbalanced exhaust air"
        LoadsInitializationDidNotConverge, // "Loads Initialization"
        DaylightMapPointsNearWindow,       // "CalcDaylightMapPoints:"
        ZoneAirHeatBalanceWarnings,        // "Zone Air Heat Balance"
        OccupantDensityExtremelyHigh,      // "occupant density is ext"
        TemperatureLowOutOfBounds,         // "Temperature (low) out o"
        TemperatureHighOutOfBounds,        // "Temperature (high) out"
        NominallyUnusedConstructions,      // "nominally unused"
        InfraredTransparentUsage,          // "InfraredTransparent"
        NoReportingElementsRequested,      // "No reporting elements"
        Num
    };

    struct RecurringErrorData
    {
        // Members
        std::string Message;    // Message to be written to "error file" at end of simulation
        int Count = 0;          // Count of total times this recurring error message has been called
        int WarmupCount = 0;    // Count of times this recurring error message has been called during warmup
        int SizingCount = 0;    // Count of times this recurring error message has been called during sizing
        Real64 MaxValue = 0.0;  // Max of the values passed for this recurring error message
        Real64 MinValue = 0.0;  // Min of the values passed for this recurring error message
        Real64 SumValue = 0.0;  // Sum of the values passed for this recurring error message
        std::string MaxUnits;   // units for Max values
        std::string MinUnits;   // units for Min values
        std::string SumUnits;   // units for Sum values
        bool ReportMax = false; // Flag to report max value
        bool ReportMin = false; // Flag to report min value
        bool ReportSum = false; // Flag to report sum value
    };

} // namespace DataErrorTracking

struct ErrorTrackingData : BaseGlobalStruct
{
    Array1D<DataErrorTracking::RecurringErrorData> RecurringErrors;
    std::array<int, static_cast<size_t>(DataErrorTracking::ErrorSummaryType::Num)> ErrorSummaryCount{};
    bool AbortProcessing = false;            // Flag used to if currently in "abort processing"
    int NumRecurringErrors = 0;              // Number of stored recurring error messages
    int TotalSevereErrors = 0;               // Counter
    int TotalWarningErrors = 0;              // Counter
    int TotalSevereErrorsDuringWarmup = 0;   // Counter
    int TotalWarningErrorsDuringWarmup = 0;  // Counter
    int TotalSevereErrorsDuringSizing = 0;   // Counter
    int TotalWarningErrorsDuringSizing = 0;  // Counter
    int TotalMultipliedWindows = 0;          // Counter
    int TotalCoincidentVertices = 0;         // Counter
    int TotalDegenerateSurfaces = 0;         // Counter
    int TotalReceivingNonConvexSurfaces = 0; // Counter
    int TotalCastingNonConvexSurfaces = 0;   // Counter
    int TotalRoomAirPatternTooLow = 0;       // Counter
    int TotalRoomAirPatternTooHigh = 0;      // Counter
    bool AskForConnectionsReport = false;    // Flag used to tell when connections should be reported
    bool AskForSurfacesReport = false;       // Flag used to tell when surfaces should be reported
    bool AskForPlantCheckOnAbort = false;    // flag used to tell if plant structure can be checked
    bool ExitDuringSimulations = false;      // flag used to tell if program is in simulation mode when fatal occurs
    std::string LastSevereError;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ErrorTrackingData();
    }
};

} // namespace EnergyPlus

#endif
