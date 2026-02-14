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

#ifndef DataErrorTracking_hh_INCLUDED
#define DataErrorTracking_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataErrorTracking {

    int constexpr SearchCounts(20);

    std::array<const char *, 21> constexpr MessageSearch({"",
                                                          "InterZone Surface Areas",
                                                          "CAUTION -- Interzone",
                                                          "Node Connection Error",
                                                          "InterZone Surface Azimu",
                                                          "InterZone Surface Tilts",
                                                          "Suspected non-planar",
                                                          "Deprecated",
                                                          "Floor Tilt=",
                                                          "Roof/Ceiling Tilt=",
                                                          "View factors not",
                                                          "Unbalanced exhaust air",
                                                          "Loads Initialization",
                                                          "CalcDaylightMapPoints:",
                                                          "Zone Air Heat Balance",
                                                          "occupant density is ext",
                                                          "Temperature (low) out o",
                                                          "Temperature (high) out",
                                                          "nominally unused",
                                                          "InfraredTransparent",
                                                          "No reporting elements"});
    std::array<const char *, 21> constexpr Summaries({"",
                                                      "InterZone Surface Areas -- mismatch",
                                                      "Interzone surfaces - different zones",
                                                      "Node Connection Errors",
                                                      "InterZone Surface Azimuths -- mismatch",
                                                      "InterZone Surface Tilts -- mismatch",
                                                      "Likely non-planar surfaces",
                                                      "Deprecated Features or Key Values",
                                                      "Incorrect Floor Tilt",
                                                      "Incorrect Roof/Ceiling Tilt",
                                                      "Incomplete View factors",
                                                      "Unbalanced exhaust air flow",
                                                      "Loads Initialization did not Converge",
                                                      "CalcDaylightMapPoints: Window",
                                                      "Zone Air Heat Balance Warnings",
                                                      "Occupant density is extremely high",
                                                      "Temperature (low) out of bounds",
                                                      "Temperature (high) out of bounds",
                                                      "Nominally Unused Constructions",
                                                      "Material:InfraredTransparent usage",
                                                      "No Reporting Elements requested"});

    // in below -- simple line end <CR>.  End of Whole message <CRE>
    auto constexpr MoreDetails_0("");
    auto constexpr MoreDetails_1(
        "Area mismatch errors happen when the interzone surface in zone A is<CR>not the same size as it's companion in zone B.<CRE>"); // InterZone
    // Surface
    // Areas --
    // mismatch
    auto constexpr MoreDetails_2(""); // Interzone surfaces - different zones
    auto constexpr MoreDetails_3(
        "Node connection errors are often caused by spelling mistakes in a node name field.<CR>To track down the problem, search the "
        "idf file for each node name listed to see if it<CR>occurs in the expected input fields.<CRE>"); // Node Connection Errors
    auto constexpr MoreDetails_4("The azimuths (outward facing angle) of two interzone surfaces should not be the same.<CR>Normally, the absolute "
                                 "difference between the two azimuths will be 180 degrees.<CR>You can turn on the report: "
                                 "Output:Surfaces:List,Details; to inspect your surfaces.<CRE>"); // InterZone Surface Azimuths -- mismatch
    auto constexpr MoreDetails_5("");                                                             // InterZone Surface Tilts -- mismatch
    auto constexpr MoreDetails_6("EnergyPlus Surfaces should be planar. If the error indicates a small increment for the<CR>out of planar bounds, "
                                 "then the calculations are likely okay though you should try to fix<CR>the problem. If a greater increment, the "
                                 "calculations will likely be incorrect.<CRE>"); // Likely non-planar surfaces
    auto constexpr MoreDetails_7(
        "A deprecated feature warning/severe error indicates that you are using a feature which will be<CR>removed in a future "
        "release. The new feature is likely included in the EnergyPlus version you are<CR>using.  Consider switching now to avoid "
        "future problems.<CR>A deprecated key value message indicates you are using an out-dated key value in your input "
        "file.<CR>While EnergyPlus may continue to accept these values, some other input file readers may not.<CR>Consider changing to "
        "values that are included as valid in the Energy+.idd for these objects.<CRE>"); // Deprecated Features or Key Values
    auto constexpr MoreDetails_8("Floors are usually flat and \"tilted\" 180 degrees.  If you get this error message,<CR>it's likely that you "
                                 "need to reverse the vertices of the surface to remove the error.<CR>EnergyPlus will attempt to fix the vertices "
                                 "for the running simulation.<CR>You can turn on the report: Output:Surfaces:List,Details; to inspect your "
                                 "surfaces.<CRE>"); // Incorrect Floor Tilt
    auto constexpr MoreDetails_9("Flat roofs/ceilings are \"tilted\" 0 degrees. Pitched roofs should be \"near\" 0 degrees.<CR>If you get this "
                                 "error message, it's likely that you need to reverse the vertices of<CR>the surface to remove the error. "
                                 "EnergyPlus will attempt to fix the vertices for the<CR>running simulation. You can turn on the report: "
                                 "Output:Surfaces:List,Details;<CR>to inspect your surfaces.<CRE>"); // Incorrect Roof/Ceiling Tilt
    auto constexpr MoreDetails_10(
        "Incomplete view factors can result from incorrect floor specifications (such as tilting 0<CR>instead of 180) or not enough "
        "surfaces in a zone to make an enclosure.  The error message<CR>also shows an enforced reciprocity value.  You can decide if "
        "you need to make geometry<CR>changes based on that value.<CRE>"); // Incomplete View factors
    auto constexpr MoreDetails_11("Unbalanced exhaust air flow errors can occur when exhaust fans are running but there is no<CR>supply air. Turn "
                                  "off exhaust fans when the system is not running may help resolve the problem.<CR>Time shown is first "
                                  "occurrence of error.<CRE>"); // Unbalanced exhaust air flow
    auto constexpr MoreDetails_12(
        "1) very high thermal mass such as very thick concrete (solution: increase max number of warmup<CR>   days in the BUILDING "
        "object);<CR>2) moderate mass and inadequate space conditioning such that the building keeps getting warmer<CR>   and warmer "
        "on successive days (solution: add HVAC, check building thermal properties,<CR>   check if infiltration is included, make "
        "sure HVAC properly controlled);<CR>3) a soil layer modeled below the concrete slab - (solution remove this layer and read "
        "about<CR>   ground temperatures in the Auxiliary Programs document).<CR>4) unreasonable (too small) limits in the BUILDING "
        "object for temperature (.4 default) or<CR>   loads tolerances (.04 default)<CRE>"); // Loads Initialization did not Converge
    auto constexpr MoreDetails_13(
        "Window is too close to map points for accurate calculation.  Suggested change is to create<CR>Output:IlluminanceMap "
        "coordinates (x,y,z) that are more \"inside\" the zone<CRE>"); // CalcDaylightMapPoints: Window
    auto constexpr MoreDetails_14("Zone Air Heat Balance out of Balance warnings are currently used by developers.<CR>Users can safely ignore "
                                  "these warnings.<CRE>"); // Zone Air Heat Balance Warnings
    auto constexpr MoreDetails_15("The occupant density warning is provided to alert you to potential conditions that can cause<CR>problems with "
                                  "the heat balance calculations. Too high a density could be cause for severe<CR>temperature out of bounds "
                                  "errors in a zone leading to program termination.<CRE>"); // Occupant density is extremely high
    auto constexpr MoreDetails_16(
        "A temperature out of bounds problem can be caused by several things. The user should check:<CR>1) the weather environment "
        "(including the horizontal IR from sky)<CR>2) the level of internal gains with respect to the zone<CR>3) the thermal "
        "properties of their materials.  And other things.<CR>A common cause is a building with no or little thermal mass - all "
        "materials with Material:NoMass definitions.<CRE>"); // Temperature (low) out of bounds AND Temperature (high) out of bounds
    auto constexpr MoreDetails_18(
        "The nominally unused constructions warning is provided to alert you to potential conditions that can cause<CR>extra time "
        "during simulation. Each construction is calculated by the algorithm indicated in the HeatBalanceAlgorithm<CR>object. You may "
        "remove the constructions indicated (when you use the DisplayExtraWarnings option).<CRE>"); // Nominally unused constructions
    auto constexpr MoreDetails_19(
        "Using Material:InfraredTransparent materials in constructions are correctly used in interzone surface<CR>constructions. "
        "Warnings are given if they are used in other kinds of surfaces.<CR>They CANNOT currently be used with "
        "ConductionFiniteDifference algorithms.<CRE>"); // InfraredTransparent constructions in non-interzone surfaces
    auto constexpr MoreDetails_20(
        "No Reporting elements have been requested. You will see no output values from your run.<CR>Add Output:Variable, "
        "Output:Meter, Output:Table:SummaryReports, Output:Table:Monthly, Output:Table:TimeBins<CR>objects to your input file to "
        "receive output values from the simulation.<CRE>"); // No reporting elements requested

    std::array<const char *, 21> constexpr MoreDetails({MoreDetails_0,  MoreDetails_1,  MoreDetails_2,  MoreDetails_3,  MoreDetails_4,
                                                        MoreDetails_5,  MoreDetails_6,  MoreDetails_7,  MoreDetails_8,  MoreDetails_9,
                                                        MoreDetails_10, MoreDetails_11, MoreDetails_12, MoreDetails_13, MoreDetails_14,
                                                        MoreDetails_15, MoreDetails_16, MoreDetails_16, MoreDetails_18, MoreDetails_19,
                                                        MoreDetails_20}); // Details 16 applies to both temperature out of bounds |

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
    Array1D_int MatchCounts;
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

    ErrorTrackingData()
    {
        MatchCounts = Array1D_int(DataErrorTracking::SearchCounts, 0);
    }

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
