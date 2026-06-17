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

#ifndef SizingManager_hh_INCLUDED
#define SizingManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SizingManager {

    struct ZoneListData
    {
        // Members
        std::string Name;
        int NumOfZones;
        Array1D_int Zones;

        // Default Constructor
        ZoneListData() : NumOfZones(0)
        {
        }
    };

    void ManageSizing(EnergyPlusData &state);

    bool CalcdoLoadComponentPulseNow(
        EnergyPlusData const &state, bool isPulseZoneSizing, bool WarmupFlag, int HourOfDay, int TimeStep, Constant::KindOfSim KindOfSim);

    void ManageSystemSizingAdjustments(EnergyPlusData &state);

    void ManageSystemVentilationAdjustments(EnergyPlusData &state);

    void DetermineSystemPopulationDiversity(EnergyPlusData &state);

    void GetOARequirements(EnergyPlusData &state);

    void ProcessInputOARequirements(EnergyPlusData &state,
                                    std::string const &cCurrentModuleObject,
                                    int OAIndex,
                                    Array1D_string const &cAlphaArgs,
                                    int const NumAlphas,
                                    Array1D<Real64> const &rNumericArgs,
                                    int const NumNumbers,
                                    Array1D_bool const &lAlphaFieldBlanks,
                                    Array1D_string const &cAlphaFieldNames,
                                    bool &ErrorsFound // If errors found in input
    );

    void GetZoneAirDistribution(EnergyPlusData &state);

    void GetZoneHVACSizing(EnergyPlusData &state);

    void GetAirTerminalSizing(EnergyPlusData &state);

    void GetSizingParams(EnergyPlusData &state);

    void GetZoneSizingInput(EnergyPlusData &state);

    void ReportTemperatureInputError(EnergyPlusData &state,
                                     std::string cObjectName,
                                     int const paramNum,
                                     Real64 comparisonTemperature,
                                     bool const shouldFlagSevere,
                                     bool &ErrorsFound);

    void GetZoneAndZoneListNames(
        EnergyPlusData &state, bool &ErrorsFound, int &NumZones, Array1D_string &ZoneNames, int &NumZoneLists, Array1D<ZoneListData> &ZoneListNames);

    void GetSystemSizingInput(EnergyPlusData &state);

    void GetPlantSizingInput(EnergyPlusData &state);

    void SetupZoneSizing(EnergyPlusData &state, bool &ErrorsFound);

    void reportZoneSizing(EnergyPlusData &state,
                          DataHeatBalance::ZoneSpaceData const &zoneOrSpace,
                          DataSizing::ZoneSizingData const &zsFinalSizing,
                          DataSizing::ZoneSizingData const &zsCalcFinalSizing,
                          Array2D<DataSizing::ZoneSizingData> const &zsCalcSizing,
                          Array2D<DataSizing::ZoneSizingData> const &zSizing,
                          Real64 const zoneMult,
                          bool const isSpace);

    void reportZoneSizingEio(EnergyPlusData &state,
                             std::string const &ZoneName,   // the name of the zone
                             std::string const &LoadType,   // the description of the input variable
                             Real64 CalcDesLoad,            // the value from the sizing calculation [W]
                             Real64 UserDesLoad,            // the value from the sizing calculation modified by user input [W]
                             Real64 CalcDesFlow,            // calculated design air flow rate [m3/s]
                             Real64 UserDesFlow,            // user input or modified design air flow rate [m3/s]
                             std::string const &DesDayName, // the name of the design day that produced the peak
                             std::string const &PeakHrMin,  // time stamp of the peak
                             Real64 PeakTemp,               // temperature at peak [C]
                             Real64 PeakHumRat,             // humidity ratio at peak [kg water/kg dry air]
                             Real64 FloorArea,              // zone floor area [m2]
                             Real64 TotOccs,                // design number of occupants for the zone
                             Real64 MinOAVolFlow,           // zone design minimum outside air flow rate [m3/s]
                             Real64 DOASHeatAddRate,        // zone design heat addition rate from the DOAS [W]
                             bool const isSpace);

    void ReportSysSizing(EnergyPlusData &state,
                         std::string const &SysName,    // the name of the zone
                         std::string_view LoadType,     // either "Cooling" or "Heating"
                         std::string_view PeakLoadType, // either "Sensible" or "Total"
                         Real64 UserDesCap,             // User  Design Capacity
                         Real64 CalcDesVolFlow,         // Calculated  Design Air Flow Rate
                         Real64 UserDesVolFlow,         // User Design Air Flow Rate
                         std::string const &DesDayName, // the name of the design day that produced the peak
                         std::string const &DesDayDate, // the date that produced the peak
                         int TimeStepIndex              // time step of the peak
    );

    std::string TimeIndexToHrMinString(EnergyPlusData const &state, int timeIndex);

    void UpdateFacilitySizing(EnergyPlusData &state, Constant::CallIndicator CallIndicator);

    void UpdateTermUnitFinalZoneSizing(EnergyPlusData &state);

} // namespace SizingManager

struct SizingManagerData : BaseGlobalStruct
{

    // MODULE VARIABLE DECLARATIONS:
    int NumAirLoops = 0;
    bool ReportZoneSizingMyOneTimeFlag = true;
    bool ReportSpaceSizingMyOneTimeFlag = true;
    bool ReportSysSizingMyOneTimeFlag = true;
    bool runZeroingOnce = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumAirLoops = 0;
        ReportZoneSizingMyOneTimeFlag = true;
        ReportSpaceSizingMyOneTimeFlag = true;
        ReportSysSizingMyOneTimeFlag = true;
        runZeroingOnce = true;
    }
};

} // namespace EnergyPlus

#endif
