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

#ifndef SystemReports_hh_INCLUDED
#define SystemReports_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SystemReports {

    struct Energy
    {
        // Members
        Real64 TotDemand;
        Real64 Elec;
        Real64 Gas;
        Real64 Purch;
        Real64 Other;

        // Default Constructor
        Energy() : TotDemand(0.0), Elec(0.0), Gas(0.0), Purch(0.0), Other(0.0)
        {
        }
    };

    struct CoilType
    {
        // Members
        Energy DecreasedCC; // LoadMetByVent
        Energy DecreasedHC; // LoadMetByVent
        Energy IncreasedCC; // LoadIncreasedVent
        Energy IncreasedHC; // LoadAddedByVent
        Energy ReducedByCC; // LoadAddedByVent
        Energy ReducedByHC; // LoadAddedByVent

        // Default Constructor
        CoilType() = default;
    };

    struct SummarizeLoads
    {
        // Members
        CoilType Load;             // LoadMetByVent
        CoilType NoLoad;           // LoadMetByVentNoLoad
        CoilType ExcessLoad;       // LoadAddedByVentOvercool
        CoilType PotentialSavings; // LoadAddedByVentCoolLost
        CoilType PotentialCost;    // LoadAddedByVentHeatLost

        // Default Constructor
        SummarizeLoads() = default;
    };

    struct CompTypeError
    {
        // Members
        std::string CompType;
        int CompErrIndex;

        // Default Constructor
        CompTypeError() : CompErrIndex(0)
        {
        }
    };

    struct ZoneVentReportVariables
    {
        Real64 CoolingLoadMetByVent = 0.0;
        Real64 CoolingLoadAddedByVent = 0.0;
        Real64 OvercoolingByVent = 0.0;
        Real64 HeatingLoadMetByVent = 0.0;
        Real64 HeatingLoadAddedByVent = 0.0;
        Real64 OverheatingByVent = 0.0;
        Real64 NoLoadHeatingByVent = 0.0;
        Real64 NoLoadCoolingByVent = 0.0;

        Real64 OAMassFlow = 0.0;               // zone mech vent mass flow rate {kg/s}
        Real64 OAMass = 0.0;                   // zone mech vent total mass for time {kg}
        Real64 OAVolFlowStdRho = 0.0;          // zone mech vent volume flow rate at standard density {m3/s}
        Real64 OAVolStdRho = 0.0;              // zone mech vent total volume OA at standard density {m3}
        Real64 OAVolFlowCrntRho = 0.0;         // zone mech vent volume flow rate at current density {m3/s}
        Real64 OAVolCrntRho = 0.0;             // zone mech vent total volume OA at current density {m3}
        Real64 MechACH = 0.0;                  // zone mech vent air changes per hour {ACH}
        Real64 TargetVentilationFlowVoz = 0.0; // zone target ventilation ventilation flow based on 62.1 Voz-dyn {m3/s}
        Real64 TimeBelowVozDyn = 0.0;          // time [hrs] that mechanical+natural ventilation is < VozTarget - 1%
        Real64 TimeAtVozDyn = 0.0;             // time [hrs] that mechanical+natural ventilation is = VozTarget within 1% and > zero
        Real64 TimeAboveVozDyn = 0.0;          // time [hrs] that mechanical+natural ventilation is > VozTarget + 1%
        Real64 TimeVentUnocc = 0.0;            // time [hrs] that mechanical+natural ventilation is > zero during unoccupied
    };

    struct SysVentReportVariables
    {
        Real64 MechVentFlow = 0.0;             // air loop mechanical vent total volume OA at standard density {m3/s}
        Real64 NatVentFlow = 0.0;              // air loop natural vent total volume OA at standard density {m3/s}
        Real64 TargetVentilationFlowVoz = 0.0; // air loop target ventilation ventilation flow based on 62.1 Voz-dyn {m3/s}
        Real64 TimeBelowVozDyn = 0.0;          // time [hrs] that mechanical+natural ventilation is < VozTarget - 1%
        Real64 TimeAtVozDyn = 0.0;             // time [hrs] that mechanical+natural ventilation is = VozTarget within 1% and > zero
        Real64 TimeAboveVozDyn = 0.0;          // time [hrs] that mechanical+natural ventilation is > VozTarget + 1%
        Real64 TimeVentUnocc = 0.0;            // time [hrs] that mechanical+natural ventilation is > zero during unoccupied
        bool AnyZoneOccupied = false;          // true if any zone on system is occupied
    };
    struct SysLoadReportVariables
    {
        // SYSTEM LOADS REPORT
        Real64 TotHTNG = 0.0;
        Real64 TotCLNG = 0.0;

        // SYSTEM ENERGY USE REPORT
        Real64 TotH2OHOT = 0.0;
        Real64 TotH2OCOLD = 0.0;
        Real64 TotElec = 0.0;
        Real64 TotNaturalGas = 0.0;
        Real64 TotPropane = 0.0;
        Real64 TotSteam = 0.0;

        // SYSTEM COMPONENT LOADS REPORT
        Real64 HumidHTNG = 0.0;
        Real64 HumidElec = 0.0;
        Real64 HumidNaturalGas = 0.0;
        Real64 HumidPropane = 0.0;
        Real64 EvapCLNG = 0.0;
        Real64 EvapElec = 0.0;
        Real64 HeatExHTNG = 0.0;
        Real64 HeatExCLNG = 0.0;
        Real64 DesDehumidCLNG = 0.0;
        Real64 DesDehumidElec = 0.0;
        Real64 SolarCollectHeating = 0.0;
        Real64 SolarCollectCooling = 0.0;
        Real64 UserDefinedTerminalHeating = 0.0;
        Real64 UserDefinedTerminalCooling = 0.0;

        // SYSTEM COMPONENT ENERGY REPORT
        Real64 FANCompHTNG = 0.0;
        Real64 FANCompElec = 0.0;
        Real64 CCCompCLNG = 0.0;
        Real64 CCCompH2OCOLD = 0.0;
        Real64 CCCompElec = 0.0;
        Real64 HCCompH2OHOT = 0.0;
        Real64 HCCompElec = 0.0;
        Real64 HCCompElecRes = 0.0;
        Real64 HCCompHTNG = 0.0;
        Real64 HCCompNaturalGas = 0.0;
        Real64 HCCompPropane = 0.0;
        Real64 HCCompSteam = 0.0;
        Real64 DomesticH2O = 0.0;
    };

    struct SysPreDefRepType
    {
        Real64 MechVentTotal = 0.0;           // air loop mechanical vent total volume OA at standard density {m3}
        Real64 NatVentTotal = 0.0;            // air loop natural vent total volume OA at standard density {m3}
        Real64 TargetVentTotalVoz = 0.0;      // air loop target ventilation ventilation flow based on 62.1 Voz-dyn {m3}
        Real64 TimeBelowVozDynTotal = 0.0;    // time [hrs] that mechanical+natural ventilation is < VozTarget - 1%
        Real64 TimeAtVozDynTotal = 0.0;       // time [hrs] that mechanical+natural ventilation is = VozTarget within 1% and > zero
        Real64 TimeAboveVozDynTotal = 0.0;    // time [hrs] that mechanical+natural ventilation is > VozTarget + 1%
        Real64 MechVentTotalOcc = 0.0;        // air loop mechanical vent total volume OA at standard density {m3} during occupied
        Real64 NatVentTotalOcc = 0.0;         // air loop natural vent total volume OA at standard density {m3} during occupied
        Real64 TargetVentTotalVozOcc = 0.0;   // air loop target ventilation ventilation flow based on 62.1 Voz-dyn {m3} during occupied
        Real64 TimeBelowVozDynTotalOcc = 0.0; // time [hrs] that mechanical+natural ventilation is < VozTarget - 1% during occupied
        Real64 TimeAtVozDynTotalOcc = 0.0;    // time [hrs] that mechanical+natural ventilation is = VozTarget within 1% and > zero during occ
        Real64 TimeAboveVozDynTotalOcc = 0.0; // time [hrs] that mechanical+natural ventilation is > VozTarget + 1% during occupied
        Real64 TimeVentUnoccTotal = 0.0;      // time [hrs] that mechanical+natural ventilation is > zero during unoccupied
        Real64 TimeOccupiedTotal = 0.0;       // time [hrs] that any zone is occupied
        Real64 TimeFanContTotalOcc = 0.0;     // time [hrs] that fan is continuous when occupied
        Real64 TimeFanCycTotalOcc = 0.0;      // time [hrs] that fan is cycling when occupied
        Real64 TimeFanOffTotalOcc = 0.0;      // time [hrs] that fan is off when occupied
        Real64 TimeUnoccupiedTotal = 0.0;     // time [hrs] that all zones are unoccupied
        Real64 TimeFanContTotalUnocc = 0.0;   // time [hrs] that fan is continuous when unoccupied
        Real64 TimeFanCycTotalUnocc = 0.0;    // time [hrs] that fan is cycling when unoccupied
        Real64 TimeFanOffTotalUnocc = 0.0;    // time [hrs] that fan is off when unoccupied

        std::vector<Real64> TimeAtOALimit = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};    // time [hrs] at limit [n]
        std::vector<Real64> TimeAtOALimitOcc = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // time [hrs] at limit [n] during occupied
        std::vector<Real64> MechVentTotAtLimitOcc = {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // air loop mech vent total vol OA at limit [n] {m3} during occupied
    };

    // Functions

    void InitEnergyReports(EnergyPlusData &state);

    void FindFirstLastPtr(EnergyPlusData &state, int &LoopType, int &LoopNum, int &ArrayCount, int &LoopCount, bool &ConnectionFlag);

    void UpdateZoneCompPtrArray(EnergyPlusData &state,
                                int &Idx,
                                int const ListNum,
                                int const AirDistUnitNum,
                                int const PlantLoopType,
                                int const PlantLoop,
                                int const PlantBranch,
                                int const PlantComp);

    void UpdateZoneSubCompPtrArray(EnergyPlusData &state,
                                   int &Idx,
                                   int const ListNum,
                                   int const AirDistUnitNum,
                                   int const SubCompNum,
                                   int const PlantLoopType,
                                   int const PlantLoop,
                                   int const PlantBranch,
                                   int const PlantComp);

    void UpdateZoneSubSubCompPtrArray(EnergyPlusData &state,
                                      int &Idx,
                                      int const ListNum,
                                      int const AirDistUnitNum,
                                      int const SubCompNum,
                                      int const SubSubCompNum,
                                      int const PlantLoopType,
                                      int const PlantLoop,
                                      int const PlantBranch,
                                      int const PlantComp);

    void UpdateAirSysCompPtrArray(EnergyPlusData &state,
                                  int &Idx,
                                  int const AirLoopNum,
                                  int const BranchNum,
                                  int const CompNum,
                                  int const PlantLoopType,
                                  int const PlantLoop,
                                  int const PlantBranch,
                                  int const PlantComp);

    void UpdateAirSysSubCompPtrArray(EnergyPlusData &state,
                                     int &Idx,
                                     int const AirLoopNum,
                                     int const BranchNum,
                                     int const CompNum,
                                     int const SubCompNum,
                                     int const PlantLoopType,
                                     int const PlantLoop,
                                     int const PlantBranch,
                                     int const PlantComp);

    void UpdateAirSysSubSubCompPtrArray(EnergyPlusData &state,
                                        int &Idx,
                                        int const AirLoopNum,
                                        int const BranchNum,
                                        int const CompNum,
                                        int const SubCompNum,
                                        int const SubSubCompNum,
                                        int const PlantLoopType,
                                        int const PlantLoop,
                                        int const PlantBranch,
                                        int const PlantComp);

    void AllocateAndSetUpVentReports(EnergyPlusData &state);

    void CreateEnergyReportStructure(EnergyPlusData &state);

    // End Initialization Section of the Module
    //******************************************************************************

    // Beginning of Reporting subroutines for the SimAir Module
    // *****************************************************************************

    void ReportSystemEnergyUse(EnergyPlusData &state);

    void CalcSystemEnergyUse(EnergyPlusData &state,
                             bool const CompLoadFlag,
                             int const AirLoopNum,
                             std::string const &CompType,
                             Constant::eResource const EnergyType,
                             Real64 const CompLoad,
                             Real64 const CompEnergy);

    void ReportVentilationLoads(EnergyPlusData &state);

    void MatchPlantSys(EnergyPlusData &state,
                       int const AirLoopNum, // counter for zone air distribution inlets
                       int const BranchNum   // counter for zone air distribution inlets
    );

    void FindDemandSideMatch(EnergyPlusData &state,
                             std::string const &CompType, // Inlet node of the component to find the match of
                             std::string_view CompName,   // Outlet node of the component to find the match of
                             bool &MatchFound,            // Set to .TRUE. when a match is found
                             int &MatchLoopType,          // Loop number of the match
                             int &MatchLoop,              // Loop number of the match
                             int &MatchBranch,            // Branch number of the match
                             int &MatchComp               // Component number of the match
    );

    void ReportAirLoopConnections(EnergyPlusData &state);

    void reportAirLoopToplogy(EnergyPlusData &state);

    void fillAirloopToplogyComponentRow(EnergyPlusData &state,
                                        const std::string_view &loopName,
                                        const std::string_view &branchName,
                                        const HVAC::AirDuctType ductType,
                                        const std::string_view &compType,
                                        const std::string_view &compName,
                                        int &rowCounter);

    void reportZoneEquipmentToplogy(EnergyPlusData &state);

    void fillZoneEquipToplogyComponentRow(
        EnergyPlusData &state, const std::string_view &zoneName, const std::string_view &compType, const std::string_view &compName, int &rowCounter);

    void reportAirDistributionUnits(EnergyPlusData &state);

    //        End of Reporting subroutines for the SimAir Module
    // *****************************************************************************

    struct IdentifyLoop
    {
        // Members
        int LoopNum;
        int LoopType;

        // Default Constructor
        IdentifyLoop() : LoopNum(0), LoopType(0)
        {
        }
    };

} // namespace SystemReports

struct SystemReportsData : BaseGlobalStruct
{

    Real64 AnyZoneTimeBelowVozDyn = 0.0;    // time [hrs] that any zone mechanical+natural ventilation is < VozTarget - 1%
    Real64 AllZonesTimeAtVozDyn = 0.0;      // time [hrs] that all zones mechanical+natural ventilation is = VozTarget within 1% and > zero
    Real64 AnyZoneTimeAboveVozDyn = 0.0;    // time [hrs] that any zone mechanical+natural ventilation is > VozTarget + 1%
    Real64 AnyZoneTimeVentUnocc = 0.0;      // time [hrs] that any zone mechanical+natural ventilation is > zero during unoccupied
    Real64 AnyZoneTimeBelowVozDynOcc = 0.0; // time [hrs] that any zone mechanical+natural ventilation is < VozTarget - 1% during occupied
    Real64 AllZonesTimeAtVozDynOcc = 0.0;   // time [hrs] that all zones mech+nat vent is = VozTarget within 1% and > zero during occupied
    Real64 AnyZoneTimeAboveVozDynOcc = 0.0; // time [hrs] that any zone mechanical+natural ventilation is > VozTarget + 1% during occupied

    bool AirLoopLoadsReportEnabled = true;
    bool VentLoadsReportEnabled = true;
    bool VentEnergyReportEnabled = false;
    bool VentReportStructureCreated = false;
    int TotalLoopConnects = 0; // Total number of loop connections
    int MaxLoopArraySize = 100;
    int MaxCompArraySize = 500;

    EPVector<SystemReports::SummarizeLoads> Vent;
    EPVector<SystemReports::ZoneVentReportVariables> ZoneVentRepVars;
    EPVector<SystemReports::SysVentReportVariables> SysVentRepVars;
    EPVector<SystemReports::SysLoadReportVariables> SysLoadRepVars;
    EPVector<SystemReports::SysPreDefRepType> SysPreDefRep;

    bool OneTimeFlag_FindFirstLastPtr = true;
    bool OneTimeFlag_InitEnergyReports = true;
    bool OneTimeFlag_UpdateZoneCompPtrArray = true;
    int ArrayLimit_UpdateZoneCompPtrArray = 100;
    int ArrayCounter_UpdateZoneCompPtrArray = 1;
    bool OneTimeFlag_UpdateZoneSubCompPtrArray = true;
    int ArrayLimit_UpdateZoneSubCompPtrArray = 100;
    int ArrayCounter_UpdateZoneSubCompPtrArray = 1;
    bool OneTimeFlag_UpdateZoneSubSubCompPtrArray = true;
    int ArrayLimit_UpdateZoneSubSubCompPtrArray = 100;
    int ArrayCounter_UpdateZoneSubSubCompPtrArray = 1;
    bool OneTimeFlag_UpdateAirSysCompPtrArray = true;
    int ArrayLimit_UpdateAirSysCompPtrArray = 100;
    int ArrayCounter_UpdateAirSysCompPtrArray = 1;
    bool OneTimeFlag_UpdateAirSysSubCompPtrArray = true;
    int ArrayLimit_UpdateAirSysSubCompPtrArray = 100;
    int ArrayCounter_UpdateAirSysSubCompPtrArray = 1;
    bool OneTimeFlag_UpdateAirSysSubSubCompPtrArray = true;
    int ArrayLimit_UpdateAirSysSubSubCompPtrArray = 100;
    int ArrayCounter_UpdateAirSysSubSubCompPtrArray = 1;
    int NumCompTypes = 0;
    Array1D<SystemReports::CompTypeError> CompTypeErrors = Array1D<SystemReports::CompTypeError>(100);
    Array1D<SystemReports::IdentifyLoop> LoopStack;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->AnyZoneTimeBelowVozDyn = 0.0;
        this->AllZonesTimeAtVozDyn = 0.0;
        this->AnyZoneTimeAboveVozDyn = 0.0;
        this->AnyZoneTimeVentUnocc = 0.0;
        this->AnyZoneTimeBelowVozDynOcc = 0.0;
        this->AllZonesTimeAtVozDynOcc = 0.0;
        this->AnyZoneTimeAboveVozDynOcc = 0.0;
        this->AirLoopLoadsReportEnabled = true;
        this->VentLoadsReportEnabled = true;
        this->VentEnergyReportEnabled = false;
        this->VentReportStructureCreated = false;
        this->TotalLoopConnects = 0;
        this->MaxLoopArraySize = 100;
        this->MaxCompArraySize = 500;
        this->Vent.deallocate();
        this->ZoneVentRepVars.deallocate();
        this->SysVentRepVars.deallocate();
        this->SysLoadRepVars.deallocate();
        this->SysPreDefRep.deallocate();
        this->OneTimeFlag_FindFirstLastPtr = true;
        this->OneTimeFlag_InitEnergyReports = true;
        this->OneTimeFlag_UpdateZoneCompPtrArray = true;
        this->ArrayLimit_UpdateZoneCompPtrArray = 100;
        this->ArrayCounter_UpdateZoneCompPtrArray = 1;
        this->OneTimeFlag_UpdateZoneSubCompPtrArray = true;
        this->ArrayLimit_UpdateZoneSubCompPtrArray = 100;
        this->ArrayCounter_UpdateZoneSubCompPtrArray = 1;
        this->OneTimeFlag_UpdateZoneSubSubCompPtrArray = true;
        this->ArrayLimit_UpdateZoneSubSubCompPtrArray = 100;
        this->ArrayCounter_UpdateZoneSubSubCompPtrArray = 1;
        this->OneTimeFlag_UpdateAirSysCompPtrArray = true;
        this->ArrayLimit_UpdateAirSysCompPtrArray = 100;
        this->ArrayCounter_UpdateAirSysCompPtrArray = 1;
        this->OneTimeFlag_UpdateAirSysSubCompPtrArray = true;
        this->ArrayLimit_UpdateAirSysSubCompPtrArray = 100;
        this->ArrayCounter_UpdateAirSysSubCompPtrArray = 1;
        this->OneTimeFlag_UpdateAirSysSubSubCompPtrArray = true;
        this->ArrayLimit_UpdateAirSysSubSubCompPtrArray = 100;
        this->ArrayCounter_UpdateAirSysSubSubCompPtrArray = 1;
        this->NumCompTypes = 0;
        this->CompTypeErrors = Array1D<SystemReports::CompTypeError>(100);
    }
};

} // namespace EnergyPlus

#endif
