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

#ifndef DualDuct_hh_INCLUDED
#define DualDuct_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DualDuct {

    enum class DualDuctDamper
    {
        Invalid = -1,
        ConstantVolume,
        VariableVolume,
        OutdoorAir,
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(DualDuctDamper::Num)> dualDuctDamperNames = {
        "ConstantVolume", "VariableVolume", "OutdoorAir"};

    enum class PerPersonMode
    {
        Invalid = -1,
        ModeNotSet,
        DCVByCurrentLevel,
        ByDesignLevel,
        Num
    };

    struct DualDuctAirTerminalFlowConditions
    {
        Real64 AirMassFlowRate = 0.0;         // MassFlow through the Damper being Simulated [kg/Sec]
        Real64 AirMassFlowRateMaxAvail = 0.0; // MassFlow through the Damper being Simulated [kg/Sec]
        Real64 AirMassFlowRateMinAvail = 0.0; // MassFlow through the Damper being Simulated [kg/Sec]
        Real64 AirMassFlowRateMax = 0.0;      // Max Mass Flow Rate or the Design Mass Flow Rate
        Real64 AirTemp = 0.0;
        Real64 AirHumRat = 0.0;
        Real64 AirEnthalpy = 0.0;
        Real64 AirMassFlowRateHist1 = 0.0; // flow history back 1 iteration kg/s
        Real64 AirMassFlowRateHist2 = 0.0; // flow history back 2 iteration kg/s
        Real64 AirMassFlowRateHist3 = 0.0; // flow history back 3 iteration kg/s
        Real64 AirMassFlowDiffMag = 0.0;   // flow difference scale, kg/s
    };

    struct DualDuctAirTerminal
    {
        std::string Name;                                    // Name of the Damper
        DualDuctDamper DamperType = DualDuctDamper::Invalid; // Type of Damper ie. VAV, Mixing, Inducing, etc.
        Sched::Schedule *availSched = nullptr; // Pointer to the correct schedule // brazenly assume that this is an availability schedule
        Real64 MaxAirVolFlowRate = 0.0;        // Max Specified Volume Flow Rate of Damper [m3/sec]
        Real64 MaxAirMassFlowRate = 0.0;       // Max Specified MAss Flow Rate of Damper [kg/s]
        int HotAirInletNodeNum = 0;
        int ColdAirInletNodeNum = 0;
        int OutletNodeNum = 0;
        Real64 ZoneMinAirFracDes = 0.0; // Fraction of supply air used as design minimum flow
        Real64 ZoneMinAirFrac = 0.0;    // Fraction of supply air used as current minimum flow
        Real64 ColdAirDamperPosition = 0.0;
        Real64 HotAirDamperPosition = 0.0;
        int OAInletNodeNum = 0;                                    // Alternate Node for VAV:OutdoorAir for Outdoor Air
        int RecircAirInletNodeNum = 0;                             // Alternate Node for VAV:OutdoorAir for Recirc Air
        bool RecircIsUsed = true;                                  // if true. then not using recirc duct, which is okay
        Real64 DesignOAFlowRate = 0.0;                             // Terminal Outdoor Air Design Flow Rate for VAV:OutdoorAir, m3/s
        Real64 DesignRecircFlowRate = 0.0;                         // Terminal Recirc Air Design Flow Rate for VAV:OutdoorAir, m3/s
        Real64 RecircAirDamperPosition = 0.0;                      // Alternate Damper Pos Output for VAV:OutdoorAir for Recirc Air
        Real64 OADamperPosition = 0.0;                             // Alternate Damper Pos Output for VAV:OutdoorAir for Recirc Air
        Real64 OAFraction = 0.0;                                   // Outdoor Air Fraction for VAV:OutdoorAir
        int ADUNum = 0;                                            // index of corresponding air distribution unit ZoneHVAC:AirdistributionUnit
        int CtrlZoneNum = 0;                                       // Pointer to CtrlZone data structure
        int CtrlZoneInNodeIndex = 0;                               // which controlled zone inlet node number corresponds with this unit
        Real64 OutdoorAirFlowRate = 0.0;                           // report variable for TU outdoor air flow rate
        bool NoOAFlowInputFromUser = true;                         // avoids OA calculation if no input specified by user
        int OARequirementsPtr = 0;                                 // - Index to DesignSpecification:OutdoorAir object
        PerPersonMode OAPerPersonMode = PerPersonMode::ModeNotSet; // mode for how per person rates are determined, DCV or design.
        int AirLoopNum = 0;                                        // index to airloop that this terminal unit is connected to
        Sched::Schedule *zoneTurndownMinAirFracSched = nullptr;    // pointer to the schedule for turndown minimum airflow fraction
        Real64 ZoneTurndownMinAirFrac = 1.0; // turndown minimum airflow fraction value, multiplier of zone design minimum air flow
        bool MyEnvrnFlag = true;             // environment flag
        bool MySizeFlag = true;              // sizing flag
        bool MyAirLoopFlag = true;           // airloop flag
        bool CheckEquipName = true;
        DualDuctAirTerminalFlowConditions dd_airterminalHotAirInlet;
        DualDuctAirTerminalFlowConditions dd_airterminalColdAirInlet;
        DualDuctAirTerminalFlowConditions dd_airterminalOutlet;
        DualDuctAirTerminalFlowConditions dd_airterminalOAInlet;
        DualDuctAirTerminalFlowConditions dd_airterminalRecircAirInlet;

        void InitDualDuct(EnergyPlusData &state, bool FirstHVACIteration);

        void SizeDualDuct(EnergyPlusData &state);

        void SimDualDuctConstVol(EnergyPlusData &state, int ZoneNum, int ZoneNodeNum);

        void SimDualDuctVarVol(EnergyPlusData &state, int ZoneNum, int ZoneNodeNum);

        void SimDualDuctVAVOutdoorAir(EnergyPlusData &state, int ZoneNum, int ZoneNodeNum);

        void CalcOAMassFlow(EnergyPlusData &state,
                            Real64 &SAMassFlow,   // outside air based on optional user input
                            Real64 &AirLoopOAFrac // outside air based on optional user input
        );

        void CalcOAOnlyMassFlow(EnergyPlusData &state,
                                Real64 &OAMassFlow,                          // outside air flow from user input kg/s
                                ObjexxFCL::Optional<Real64> MaxOAVolFlow = _ // design level for outside air m3/s
        );

        void CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state);

        void UpdateDualDuct(EnergyPlusData &state);

        void reportTerminalUnit(EnergyPlusData &state);
    };

    void SimulateDualDuct(EnergyPlusData &state, std::string_view CompName, bool FirstHVACIteration, int ZoneNum, int ZoneNodeNum, int &CompIndex);

    void GetDualDuctInput(EnergyPlusData &state);

    void ReportDualDuctConnections(EnergyPlusData &state);

    void GetDualDuctOutdoorAirRecircUse(EnergyPlusData &state, std::string const &CompTypeName, std::string_view CompName, bool &RecircIsUsed);

} // namespace DualDuct

struct DualDuctData : BaseGlobalStruct
{
    int NumDDAirTerminal = 0; // The Number of Dampers found in the Input //Autodesk Poss used uninitialized in ReportDualDuctConnections
    int NumDualDuctVarVolOA = 0;
    bool GetDualDuctInputFlag = true; // Flag set to make sure you get input once
    Array1D<DualDuct::DualDuctAirTerminal> dd_airterminal;
    std::unordered_map<std::string, std::string> UniqueDualDuctAirTerminalNames;
    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items
    bool GetDualDuctOutdoorAirRecircUseFirstTimeOnly = true;
    Array1D_bool RecircIsUsedARR;
    Array1D_string DamperNamesARR;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) DualDuctData();
    }
};

} // namespace EnergyPlus

#endif
