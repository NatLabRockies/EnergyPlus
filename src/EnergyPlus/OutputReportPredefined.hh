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

#ifndef OutputReportPredefined_hh_INCLUDED
#define OutputReportPredefined_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace OutputReportPredefined {

    // Using/Aliasing

    // Data
    // The following section initializes the predefined column heading variables
    // The variables get their value in AssignPredefined

    // Internal data structures to store information provided by calls
    int constexpr sizeIncrement(100);
    int constexpr recKindSurface(1);
    int constexpr recKindSubsurface(2);

    // Types

    struct reportNameType
    {
        // Members
        std::string name;
        std::string namewithspaces; // a "prettier version" than the key value
        std::string abrev;
        bool show;

        // Default Constructor
        reportNameType() : show(false)
        {
        }
    };

    struct SubTableType
    {
        // Members
        std::string name;
        int indexReportName;
        std::string footnote;

        // Default Constructor
        SubTableType() : indexReportName(0)
        {
        }
    };

    struct ColumnTagType
    {
        // Members
        std::string heading;
        int indexSubTable;

        // Default Constructor
        ColumnTagType() : indexSubTable(0)
        {
        }
    };

    struct TableEntryType
    {
        // Members
        std::string charEntry;
        std::string objectName;
        int indexColumn;
        int subTableIndex;
        int uniqueObjName;
        Real64 origRealEntry;
        int significantDigits;
        bool origEntryIsReal;

        // Default Constructor
        TableEntryType() : indexColumn(0), subTableIndex(0), uniqueObjName(0), origRealEntry(0.0), significantDigits(0), origEntryIsReal(false)
        {
        }
    };

    struct CompSizeTableEntryType
    {
        // Members
        std::string typeField;
        std::string nameField;
        std::string description;
        Real64 valField;
        bool active;
        bool written;

        // Default Constructor
        CompSizeTableEntryType() : valField(0.0), active(false), written(false)
        {
        }
    };

    struct ShadowRelateType
    {
        // Members
        int castSurf;
        int recSurf;
        int recKind;

        // Default Constructor
        ShadowRelateType() : castSurf(0), recSurf(0), recKind(0)
        {
        }
    };

    void SetPredefinedTables(EnergyPlusData &state);

    // Creates an entry for predefined tables when the entry
    // is a real variable. numSigDigits defaults to 2, and if supplied must be in [0-9]
    // Internally it uses a Fortran-Style write statement, meaning numbers are rounded rather than trimmed
    void PreDefTableEntry(EnergyPlusData &state,
                          int const columnIndex,
                          std::string_view objName,
                          Real64 const tableEntryReal,
                          ObjexxFCL::Optional_int_const numSigDigits = _);

    void PreDefTableEntry(EnergyPlusData &state, int const columnIndex, std::string_view objName, std::string_view tableEntryChar);

    void PreDefTableEntry(EnergyPlusData &state, int const columnIndex, std::string_view objName, int const tableEntryInt);

    std::string RetrievePreDefTableEntry(EnergyPlusData &state, int const columnIndex, std::string_view objName);

    void incrementTableEntry(EnergyPlusData &state);

    void AddCompSizeTableEntry(
        EnergyPlusData &state, std::string_view FieldType, std::string_view FieldName, std::string_view FieldDescription, Real64 const FieldValue);

    void AddShadowRelateTableEntry(EnergyPlusData &state, int const castingField, int const receivingField, int const receivingKind);

    int
    newPreDefReport(EnergyPlusData &state, std::string_view inReportName, std::string_view inReportAbrev, std::string_view inReportNamewithSpaces);

    int newPreDefSubTable(EnergyPlusData &state, int const reportIndex, std::string_view subTableName);

    void addFootNoteSubTable(EnergyPlusData &state, int const subTableIndex, std::string_view footnoteText);

    int newPreDefColumn(EnergyPlusData &state, int const subTableIndex, std::string_view columnHeading);

} // namespace OutputReportPredefined

struct OutputReportPredefinedData : BaseGlobalStruct
{

    // Climate Summary Report
    int pdrClim = 0;
    int pdstDesDay = 0;
    int pdchDDmaxDB = 0;
    int pdchDDrange = 0;
    int pdchDDhumid = 0;
    int pdchDDhumTyp = 0;
    int pdchDDwindSp = 0;
    int pdchDDwindDr = 0;
    int pdstMonthlyPrec = 0;
    int pdchMonthlyTotalPrecInWeather = 0;
    int pdchMonthlyTotalHrRain = 0;
    int pdchMonthlyTotalPrecInSitePrec = 0;
    int pdchMonthlyTotalIrrDep = 0;
    int pdchMonthlyTotalRainCol = 0;
    int pdstWthr = 0;
    int pdchWthrVal = 0;

    // HVAC Equipment Report
    int pdrEquip = 0;
    int pdstMech = 0;
    int pdchMechType = 0;
    int pdchMechNomCap = 0;
    int pdchMechNomEff = 0;
    int pdchMechRatCap = 0;
    int pdchMechRatEff = 0;
    int pdchMechIPLVSI = 0;
    int pdchMechIPLVIP = 0;

    // Fan subtable
    int pdstFan = 0;
    int pdchFanType = 0;
    int pdchFanTotEff = 0;
    int pdchFanDeltaP = 0;
    int pdchFanVolFlow = 0;
    int pdchFanMotorIn = 0;
    int pdchFanEnergyIndex = 0;
    int pdchFanEndUse = 0;
    int pdchFanPwr = 0;
    int pdchFanPwrPerFlow = 0;
    int pdchFanDesDay = 0;
    int pdchFanPkTime = 0;
    // Standard 229 Predef outputs for Fans
    int pdchFanPurpose = 0;
    int pdchFanAutosized = 0;
    int pdchFanMotorEff = 0;
    int pdchFanMotorHeatToZoneFrac = 0; // Motor Heat to Zone Fraction
    int pdchFanMotorHeatZone = 0;       // Motor Loss Zone Name
    int pdchFanAirLoopName = 0;

    // Pump subtable
    int pdstPump = 0;
    int pdchPumpType = 0;
    int pdchPumpControl = 0;
    int pdchPumpHead = 0;
    int pdchPumpFlow = 0;
    int pdchPumpPower = 0;
    int pdchPumpPwrPerFlow = 0;
    int pdchPumpEndUse = 0;
    int pdchMotEff = 0;
    // Standard 229 Predef outputs for Pumps
    int pdchPumpAutosized = 0;
    int pdchPumpPlantloopName = 0;
    int pdchPumpPlantloopBranchName = 0;

    // Cooling coil subtable
    int pdstCoolCoil = 0;
    int pdchCoolCoilType = 0;
    int pdchCoolCoilDesCap = 0;
    int pdchCoolCoilTotCap = 0;
    int pdchCoolCoilSensCap = 0;
    int pdchCoolCoilLatCap = 0;
    int pdchCoolCoilSHR = 0;
    int pdchCoolCoilNomEff = 0;
    int pdchCoolCoilUATotal = 0;
    int pdchCoolCoilArea = 0;

    // DX Cooling Coil subtable
    int pdstDXCoolCoil = 0;
    int pdchDXCoolCoilType = 0;           // DX cooling coil type
    int pdchDXCoolCoilNetCapSI = 0;       // Standard Rated (Net) Cooling Capacity [W]
    int pdchDXCoolCoilCOP = 0;            // EER/COP value in SI unit at AHRI std. 340/360 conditions [W/W]
    int pdchDXCoolCoilSEERUserIP = 0;     // SEER value in IP unit at AHRI std. 210/240 conditions and user PLF curve [Btu/W-hr]
    int pdchDXCoolCoilSEERStandardIP = 0; // SEER value in IP unit at AHRI std. 210/240 conditions and default PLF curve and C_D value [Btu/W-hr]
    int pdchDXCoolCoilEERIP = 0;          // EER value in IP unit at AHRI std. 340/360 conditions [Btu/W-h]
    int pdchDXCoolCoilIEERIP = 0;         // IEER value in IP unit at AHRI std. 340/360 conditions

    // DX Cooling Coil SEER2 subtable | AHRI std. 210/240 2023 conditions
    int pdstDXCoolCoil_2023 = 0;
    int pdchDXCoolCoilType_2023 = 0;        // DX cooling coil type
    int pdchDXCoolCoilNetCapSI_2023 = 0;    // Standard Rated (Net) Cooling Capacity [W]
    int pdchDXCoolCoilCOP_2023 = 0;         // EER/COP value in SI unit at AHRI std. 340/360 conditions [W/W]
    int pdchDXCoolCoilSEER2UserIP_2023 = 0; // SEER2 value in IP unit at AHRI std. 210/240 2023 conditions and user PLF curve [Btu/W-hr]
    int pdchDXCoolCoilSEER2StandardIP_2023 =
        0;                             // SEER2 value in IP unit at AHRI std. 210/240 2023 conditions and default PLF curve and C_D value [Btu/W-hr]
    int pdchDXCoolCoilEERIP_2023 = 0;  // EER value in IP unit at AHRI std. 340/360 conditions [Btu/W-h]
    int pdchDXCoolCoilIEERIP_2023 = 0; // IEER value in IP unit at AHRI std. 340/360 conditions

    // DX Cooling Coil subtable per ANSI/ASHRAE Std 127 for Tests A, B, C and D
    int pdstDXCoolCoil2 = 0;
    int pdchDXCoolCoilType2 = 0;      // DX cooling coil type
    int pdchDXCoolCoilNetCapSIA = 0;  // Standard Rated (Net) Cooling Capacity [W], Test A
    int pdchDXCoolCoilElecPowerA = 0; // Standard Rated Electric Power [W], Test A
    int pdchDXCoolCoilNetCapSIB = 0;  // Standard Rated (Net) Cooling Capacity [W], Test B
    int pdchDXCoolCoilElecPowerB = 0; // Standard Rated Electric Power [W], Test B
    int pdchDXCoolCoilNetCapSIC = 0;  // Standard Rated (Net) Cooling Capacity [W], Test C
    int pdchDXCoolCoilElecPowerC = 0; // Standard Rated Electric Power [W], Test C
    int pdchDXCoolCoilNetCapSID = 0;  // Standard Rated (Net) Cooling Capacity [W], Test D
    int pdchDXCoolCoilElecPowerD = 0; // Standard Rated Electric Power [W], Test D

    // Water-to-Air HP report
    int pdstWAHP = 0;
    int pdchWAHPType = 0;
    int pdchWAHPRatedCapAtRatedCdts = 0;
    int pdchWAHPRatedSensCapAtRatedCdts = 0;
    int pdchWAHPRatedPowerAtRatedCdts = 0;
    int pdchWAHPRatedCOPAtRatedCdts = 0;
    int pdchWAHPRatedAirDBT = 0;
    int pdchWAHPRatedAirWBT = 0;
    int pdchWAHPRatedWtrT = 0;
    int pdchWAHPDD = 0;

    // VAV DX Cooling Ratings Details
    int pdstVAVDXCoolCoil = 0; // details for Packaged VAV rating under AHRI 340/360
    int pdchVAVDXCoolCoilType = 0;
    int pdchVAVDXFanName = 0;
    int pdchVAVDXCoolCoilNetCapSI = 0;
    int pdchVAVDXCoolCoilCOP = 0;
    int pdchVAVDXCoolCoilIEERIP = 0;
    int pdchVAVDXCoolCoilEERIP = 0;
    int pdchVAVDXCoolCoilMdotA = 0;
    int pdchVAVDXCoolCoilCOP_B = 0;
    int pdchVAVDXCoolCoilEER_B_IP = 0;
    int pdchVAVDXCoolCoilMdotB = 0;
    int pdchVAVDXCoolCoilCOP_C = 0;
    int pdchVAVDXCoolCoilEER_C_IP = 0;
    int pdchVAVDXCoolCoilMdotC = 0;
    int pdchVAVDXCoolCoilCOP_D = 0;
    int pdchVAVDXCoolCoilEER_D_IP = 0;
    int pdchVAVDXCoolCoilMdotD = 0;

    // DX Heating Coil subtable
    int pdstDXHeatCoil = 0;
    int pdchDXHeatCoilType = 0; // DX Heating coil type
    int pdchDXHeatCoilHighCap = 0;
    int pdchDXHeatCoilLowCap = 0;
    int pdchDXHeatCoilHSPFSI = 0;    // HSPF value in SI unit at AHRI std. 340/360 conditions [W/W]
    int pdchDXHeatCoilHSPFIP = 0;    // HSPF value in IP unit at AHRI std. 340/360 conditions [Btu/W-hr]
    int pdchDXHeatCoilRegionNum = 0; // Region number for which HSPF is calculated
    // Standard 229 Predef outputs for DX Heating Coils
    int pdchDXHeatCoilMinOADBTforCompOp = 0;
    int pdchDXHeatCoilSuppHiT = 0; // Supplemental Heat High Shutoff Temperature
    int pdchDXHeatCoilAirloopName = 0;

    // DX Heating Coil subtable| AHRI std. 210/240 2023 conditions
    int pdstDXHeatCoil_2023 = 0;
    int pdchDXHeatCoilType_2023 = 0; // DX Heating coil type
    int pdchDXHeatCoilHighCap_2023 = 0;
    int pdchDXHeatCoilLowCap_2023 = 0;
    int pdchDXHeatCoilHSPF2SI_2023 = 0;   // HSPF2 value in SI unit at AHRI std. 340/360 conditions [W/W]
    int pdchDXHeatCoilHSPF2IP_2023 = 0;   // HSPF2 value in IP unit at AHRI std. 340/360 conditions [Btu/W-hr]
    int pdchDXHeatCoilRegionNum_2023 = 0; // Region number for which HSPF is calculated
    int pdchDXHeatCoilMinOADBTforCompOp_2023 = 0;
    int pdchDXHeatCoilAirloopName_2023 = 0;

    // Heating Coil subtable
    int pdstHeatCoil = 0;
    int pdchHeatCoilType = 0;
    int pdchHeatCoilDesCap = 0;
    int pdchHeatCoilNomCap = 0;
    int pdchHeatCoilNomEff = 0;
    // Standard 229 Predef outputs for Heating Coils
    int pdchHeatCoilUsedAsSupHeat = 0;
    int pdchHeatCoilAirloopName = 0;
    int pdchHeatCoilPlantloopName = 0;

    // SWH subtable
    int pdstSWH = 0;
    int pdchSWHType = 0;
    int pdchSWHVol = 0;
    int pdchSWHHeatIn = 0;
    int pdchSWHThEff = 0;
    int pdchSWHRecEff = 0;
    int pdchSWHEnFac = 0;

    // Std 229 Chiller Table in Equipment Summary
    int pdstChiller = 0;
    int pdchChillerType = 0;                        // Type
    int pdchChillerRefCap = 0;                      // Reference Capacity[W]
    int pdchChillerRefEff = 0;                      // Reference Efficiency [W/W]
    int pdchChillerRatedCap = 0;                    // Rated Capacity [W]
    int pdchChillerRatedEff = 0;                    // Rated Efficiency [W/W]
    int pdchChillerIPLVinSI = 0;                    // IPLV in SI Units [W/W]
    int pdchChillerIPLVinIP = 0;                    // IPLV in IP Units [Btu/W-h]
    int pdchChillerPlantloopName = 0;               // Plantloop name
    int pdchChillerPlantloopBranchName = 0;         // Plantloop branch name
    int pdchChillerCondLoopName = 0;                // Condenser loop name
    int pdchChillerCondLoopBranchName = 0;          // Condenser loop branch name
    int pdchChillerMinPLR = 0;                      // Minimum part load ratio
    int pdchChillerFuelType = 0;                    // Fuel type
    int pdchChillerRatedEntCondTemp = 0;            // Rated entering condenser temperature
    int pdchChillerRatedLevEvapTemp = 0;            // Rated leaving evaporator temperature
    int pdchChillerRefEntCondTemp = 0;              // Reference entering condenser temperature
    int pdchChillerRefLevEvapTemp = 0;              // Reference leaving evaporator temperature
    int pdchChillerDesSizeRefCHWFlowRate = 0;       // Design Size Reference Chilled Water Flow Rate
    int pdchChillerDesSizeRefCondFluidFlowRate = 0; // Design Size Reference Condenser Fluid Flow Rate
    int pdchChillerHeatRecPlantloopName = 0;        // Heat recovery Plantloop name
    int pdchChillerHeatRecPlantloopBranchName = 0;  // Heat recovery Plantloop branch name
    int pdchChillerRecRelCapFrac = 0;               // Recovery Relative Capacity Fraction

    // Std 229 Boiler Table in Equipment Summary
    int pdstBoiler = 0;
    int pdchBoilerType = 0;                // Type
    int pdchBoilerRefCap = 0;              // Reference Capacity [W]
    int pdchBoilerRefEff = 0;              // Reference Efficiency [W/W]
    int pdchBoilerRatedCap = 0;            // Rated Capacity [W]
    int pdchBoilerRatedEff = 0;            // Rated Efficiency [W/W]
    int pdchBoilerPlantloopName = 0;       // Plantloop name
    int pdchBoilerPlantloopBranchName = 0; // Plantloop branch name
    int pdchBoilerMinPLR = 0;              // Minimum part load ratio
    int pdchBoilerFuelType = 0;            // Fuel type
    int pdchBoilerParaElecLoad = 0;        // Parasitic electric load

    // Std 229 cooling towers and fluid coolers Table in Equipment Summary
    int pdstCTFC = 0;
    int pdchCTFCType = 0;               // Type
    int pdchCTFCCondLoopName = 0;       // Condenser loop name
    int pdchCTFCCondLoopBranchName = 0; // Condenser loop branch name
    int pdchCTFCFluidType = 0;          // Fluid type
    int pdchCTFCRange = 0;              // Range
    int pdchCTFCApproach = 0;           // Approach
    int pdchCTFCDesFanPwr = 0;          // Design Fan Power
    int pdchCTFCDesInletAirWBT = 0;     // Design inlet air wet-bulb temperature
    int pdchCTFCDesWaterFlowRate = 0;   // Design Water Flow Rate
    int pdchCTFCLevWaterSPTemp = 0;     // Leaving water setpoint temperature

    // Std 229 Plantloop and CondenserLoop Table in Equipment Summary
    int pdstPLCL = 0;
    int pdchPLCLType = 0;            // type (PlantLoop or CondenserLoop)
    int pdchPLCLProvHeat = 0;        // provides heating
    int pdchPLCLProvCool = 0;        // provides cooling
    int pdchPLCLMaxLoopFlowRate = 0; // Maximum Loop Flow Rate
    int pdchPLCLMinLoopFlowRate = 0; // Minimum Loop Flow Rate

    // Std 229 Air Terminal Table in Equipment Summary
    int pdstAirTerm = 0;
    int pdchAirTermZoneName = 0;       // Zone Name
    int pdchAirTermMinFlow = 0;        // Minimum Flow
    int pdchAirTermMinOutdoorFlow = 0; // Minimum Outdoor Flow
    int pdchAirTermSupCoolingSP = 0;   // Supply cooling setpoint
    int pdchAirTermSupHeatingSP = 0;   // Supply heating setpoint
    int pdchAirTermHeatingCap = 0;     // Heating capacity
    int pdchAirTermCoolingCap = 0;     // Cooling capacity
    int pdchAirTermTypeInp = 0;        // Type of Input Object
    int pdchAirTermHeatCoilType = 0;   // Heat/Reheat Coil Object Type
    int pdchAirTermCoolCoilType = 0;   // Chilled Water Coil Object Type
    int pdchAirTermFanType = 0;        // Fan Object Type
    int pdchAirTermFanName = 0;        // Fan Name
    int pdchAirTermPrimFlow = 0;       // Primary Air Flow Rate
    int pdchAirTermSecdFlow = 0;       // Secondary Air Flow Rate
    int pdchAirTermMinFlowSch = 0;     // Minimum Flow Schedule Name
    int pdchAirTermMaxFlowReh = 0;     // Maximum Flow During Reheat
    int pdchAirTermMinOAflowSch = 0;   // Minimum Outdoor Flow Schedule Name
    int pdchAirTermTempCntl = 0;       // Temperature Control

    // Std 229 Air Heat Recovery
    int pdstAirHR = 0;
    int pdchAirHRInputObjType = 0;              // input object type
    int pdchAirHRPlateOrRotary = 0;             // plate/rotary
    int pdchAirHRSenEffAt100PerHeatAirFlow = 0; // Sensible Effectiveness at 100% Heating Air Flow
    int pdchAirHRSenEffAt100PerCoolAirFlow = 0; // Sensible Effectiveness at 100% Cooling Air Flow
    int pdchAirHRLatEffAt100PerHeatAirFlow = 0; // Latent Effectiveness at 100% Heating Air Flow
    int pdchAirHRLatEffAt100PerCoolAirFlow = 0; // Latent Effectiveness at 100% Cooling Air Flow
    int pdchAirHRExhaustAirflow = 0;            // Exhaust (Secondary) airflow
    int pdchAirHRSupplyAirflow = 0;             // Supply (Outdoor) airflow

    // Envelope Report
    int pdrEnvelope = 0;
    int pdstOpaque = 0;
    int pdchOpCons = 0;
    int pdchOpZone = 0;
    int pdchOpSpace = 0;
    int pdchOpRefl = 0;
    int pdchOpUfactFilm = 0;
    int pdchOpUfactNoFilm = 0;
    int pdchOpGrArea = 0;
    int pdchOpNetArea = 0;
    int pdchOpAzimuth = 0;
    int pdchOpTilt = 0;
    int pdchOpDir = 0;
    int pdstIntOpaque = 0;
    int pdchIntOpCons = 0;
    int pdchIntOpZone = 0;
    int pdchIntOpSpace = 0;
    int pdchIntOpAdjSurf = 0;
    int pdchIntOpRefl = 0;
    int pdchIntOpUfactFilm = 0;
    int pdchIntOpUfactNoFilm = 0;
    int pdchIntOpGrArea = 0;
    int pdchIntOpNetArea = 0;
    int pdchIntOpAzimuth = 0;
    int pdchIntOpTilt = 0;
    int pdchIntOpDir = 0;
    int pdstFen = 0;
    int pdchFenCons = 0;
    int pdchFenZone = 0;
    int pdchFenSpace = 0;
    int pdchFenFrameDivName = 0;
    int pdchFenAreaOf1 = 0;
    int pdchFenGlassAreaOf1 = 0;
    int pdchFenFrameAreaOf1 = 0;
    int pdchFenDividerAreaOf1 = 0;
    int pdchFenArea = 0;
    int pdchFenUfact = 0;
    int pdchFenSHGC = 0;
    int pdchFenVisTr = 0;
    int pdchFenAssemNfrcType = 0;
    int pdchFenAssemUfact = 0;
    int pdchFenAssemSHGC = 0;
    int pdchFenAssemVisTr = 0;
    int pdchFenFrameConductance = 0;
    int pdchFenDividerConductance = 0;
    int pdchFenSwitchable = 0;
    int pdchFenParent = 0;
    int pdchFenAzimuth = 0;
    int pdchFenTilt = 0;
    int pdchFenDir = 0;

    int pdstFenShd = 0;
    int pdchFenShdFrameDiv = 0;
    int pdchFenShdUfact = 0;
    int pdchFenShdSHGC = 0;
    int pdchFenShdVisTr = 0;
    int pdchFenShdAssemNfrcType = 0;
    int pdchFenShdAssemUfact = 0;
    int pdchFenShdAssemSHGC = 0;
    int pdchFenShdAssemVisTr = 0;

    int pdstDoor = 0;
    int pdchDrCons = 0;
    int pdchDrZone = 0;
    int pdchDrSpace = 0;
    int pdchDrUfactFilm = 0;
    int pdchDrUfactNoFilm = 0;
    int pdchDrGrArea = 0;
    int pdchDrParent = 0;
    int pdstIntDoor = 0;
    int pdchIntDrCons = 0;
    int pdchIntDrZone = 0;
    int pdchIntDrSpace = 0;
    int pdchIntDrUfactFilm = 0;
    int pdchIntDrUfactNoFilm = 0;
    int pdchIntDrGrArea = 0;
    int pdchIntDrParent = 0;
    int pdstIntFen = 0;
    int pdchIntFenCons = 0;
    int pdchIntFenZone = 0;
    int pdchIntFenSpace = 0;
    int pdchIntFenAreaOf1 = 0;
    int pdchIntFenArea = 0;
    int pdchIntFenUfact = 0;
    int pdchIntFenSHGC = 0;
    int pdchIntFenVisTr = 0;
    int pdchIntFenParent = 0;

    int pdstOpqConsLayers = 0;
    std::vector<int> pdchOpqConsLayCol;

    // Shading Report
    int pdrShading = 0;
    int pdstSunlitFrac = 0;
    int pdchSlfMar21_9 = 0;
    int pdchSlfMar21_12 = 0;
    int pdchSlfMar21_15 = 0;
    int pdchSlfJun21_9 = 0;
    int pdchSlfJun21_12 = 0;
    int pdchSlfJun21_15 = 0;
    int pdchSlfDec21_9 = 0;
    int pdchSlfDec21_12 = 0;
    int pdchSlfDec21_15 = 0;
    int pdstWindowControl = 0;
    int pdchWscName = 0;
    int pdchWscShading = 0;
    int pdchWscShadCons = 0;
    int pdchWscControl = 0;
    int pdchWscGlare = 0;

    // Lighting Report
    int pdrLighting = 0;
    int pdstInLite = 0;
    int pdchInLtZone = 0;
    int pdchInLtSpace = 0;
    int pdchInLtSpaceType = 0;
    int pdchInLtDens = 0;
    int pdchInLtArea = 0;
    int pdchInLtPower = 0;
    int pdchInLtEndUse = 0;
    int pdchInLtSchd = 0;
    int pdchInLtAvgHrSchd = 0;
    int pdchInLtAvgHrOper = 0;
    int pdchInLtFullLoadHrs = 0;
    int pdchInLtRetAir = 0;
    int pdchInLtCond = 0;
    int pdchInLtConsump = 0;
    int pdstExtLite = 0;
    int pdchExLtPower = 0;
    int pdchExLtClock = 0;
    int pdchExLtSchd = 0;
    int pdchExLtAvgHrSchd = 0;
    int pdchExLtAvgHrOper = 0;
    int pdchExLtFullLoadHrs = 0;
    int pdchExLtConsump = 0;
    int pdstDaylight = 0;
    int pdchDyLtZone = 0;
    int pdchDyLtCtrlName = 0;
    int pdchDyLtKind = 0;
    int pdchDyLtCtrlType = 0;
    int pdchDyLtFrac = 0;
    int pdchDyLtWInst = 0;
    int pdchDyLtWCtrl = 0;

    // Sizing Report
    int pdrSizing = 0;
    int pdstSpaceClSize = 0;
    int pdchSpClCalcDesLd = 0;
    int pdchSpClUserDesLd = 0;
    int pdchSpClUserDesLdPerArea = 0;
    int pdchSpClCalcDesAirFlow = 0;
    int pdchSpClUserDesAirFlow = 0;
    int pdchSpClDesDay = 0;
    int pdchSpClPkTime = 0;
    int pdchSpClPkTstatTemp = 0;
    int pdchSpClPkIndTemp = 0;
    int pdchSpClPkIndHum = 0;
    int pdchSpClPkOATemp = 0;
    int pdchSpClPkOAHum = 0;
    int pdchSpClPkOAMinFlow = 0;
    int pdchSpClPkDOASHeatGain = 0;
    int pdstZoneClSize = 0;
    int pdchZnClCalcDesLd = 0;
    int pdchZnClUserDesLd = 0;
    int pdchZnClUserDesLdPerArea = 0;
    int pdchZnClCalcDesAirFlow = 0;
    int pdchZnClUserDesAirFlow = 0;
    int pdchZnClDesDay = 0;
    int pdchZnClPkTime = 0;
    int pdchZnClPkTstatTemp = 0;
    int pdchZnClPkIndTemp = 0;
    int pdchZnClPkIndHum = 0;
    int pdchZnClPkOATemp = 0;
    int pdchZnClPkOAHum = 0;
    int pdchZnClPkOAMinFlow = 0;
    int pdchZnClPkDOASHeatGain = 0;
    int pdstSpaceHtSize = 0;
    int pdchSpHtCalcDesLd = 0;
    int pdchSpHtUserDesLd = 0;
    int pdchSpHtUserDesLdPerArea = 0;
    int pdchSpHtCalcDesAirFlow = 0;
    int pdchSpHtUserDesAirFlow = 0;
    int pdchSpHtDesDay = 0;
    int pdchSpHtPkTime = 0;
    int pdchSpHtPkTstatTemp = 0;
    int pdchSpHtPkIndTemp = 0;
    int pdchSpHtPkIndHum = 0;
    int pdchSpHtPkOATemp = 0;
    int pdchSpHtPkOAHum = 0;
    int pdchSpHtPkOAMinFlow = 0;
    int pdchSpHtPkDOASHeatGain = 0;
    int pdstZoneHtSize = 0;
    int pdchZnHtCalcDesLd = 0;
    int pdchZnHtUserDesLd = 0;
    int pdchZnHtUserDesLdPerArea = 0;
    int pdchZnHtCalcDesAirFlow = 0;
    int pdchZnHtUserDesAirFlow = 0;
    int pdchZnHtDesDay = 0;
    int pdchZnHtPkTime = 0;
    int pdchZnHtPkTstatTemp = 0;
    int pdchZnHtPkIndTemp = 0;
    int pdchZnHtPkIndHum = 0;
    int pdchZnHtPkOATemp = 0;
    int pdchZnHtPkOAHum = 0;
    int pdchZnHtPkOAMinFlow = 0;
    int pdchZnHtPkDOASHeatGain = 0;
    int pdstSystemSize = 0;
    int pdchSysSizCalcClAir = 0;
    int pdchSysSizUserClAir = 0;
    int pdchSysSizCalcHtAir = 0;
    int pdchSysSizUserHtAir = 0;
    int pdchSysSizAdjustedClAir = 0;
    int pdchSysSizAdjustedHtAir = 0;
    int pdchSysSizAdjustedMainAir = 0;
    int pdchSysSizCalcHeatFlowRatio = 0;
    int pdchSysSizUserHeatFlowRatio = 0;
    int pdstPlantSize = 0;
    int pdchPlantSizCalcVdot = 0;
    int pdchPlantSizMeasVdot = 0;
    int pdchPlantSizPrevVdot = 0;
    int pdchPlantSizCoincYesNo = 0;
    int pdchPlantSizDesDay = 0;
    int pdchPlantSizPkTimeHour = 0;
    int pdchPlantSizPkTimeDayOfSim = 0;
    int pdchPlantSizPkTimeMin = 0;

    // Coil Selection Table
    int pdrCoilSizingDetailsTable = 0;
    int pdstCoilSummaryCoilSelection = 0;
    int pdchCoilLocation = 0;
    int pdchCoilHVACType = 0;
    int pdchCoilHVACName = 0;
    int pdchCoilZoneName = 0;
    int pdchCoilName = 0;
    int pdchCoilType = 0;
    int pdchSysSizingMethCoinc = 0;
    int pdchSysSizingMethCap = 0;
    int pdchSysSizingMethAir = 0;

    int pdchCoilIsCapAutosized = 0;
    int pdchCoilIsAirFlowAutosized = 0;
    int pdchCoilIsWaterFlowAutosized = 0;
    int pdchCoilIsOATreated = 0;
    int pdchCoilFinalTotalCap = 0;
    int pdchCoilFinalSensCap = 0;
    int pdchCoilFinalAirVolFlowRate = 0;
    int pdchCoilFinalPlantVolFlowRate = 0;

    int pdchCoilDDnameSensIdealPeak = 0;
    int pdchCoilDateTimeSensIdealPeak = 0;
    int pdchCoilDDnameTotIdealPeak = 0;
    int pdchCoilDateTimeTotIdealPeak = 0;
    int pdchCoilDDnameAirFlowIdealPeak = 0;
    int pdchCoilDateTimeAirFlowIdealPeak = 0;
    int pdchCoilPeakLoadTypeToSizeOn = 0;
    int pdchCoilTotalCapIdealPeak = 0;
    int pdchCoilSensCapIdealPeak = 0;
    int pdchCoilAirMassFlowIdealPeak = 0;
    int pdchCoilAirVolumeFlowIdealPeak = 0;
    int pdchCoilEntDryBulbIdealPeak = 0;
    int pdchCoilEntWetBulbIdealPeak = 0;
    int pdchCoilEntHumRatIdealPeak = 0;
    int pdchCoilEntEnthalpyIdealPeak = 0;
    int pdchCoilLvgDryBulbIdealPeak = 0;
    int pdchCoilLvgWetBulbIdealPeak = 0;
    int pdchCoilLvgHumRatIdealPeak = 0;
    int pdchCoilLvgEnthalpyIdealPeak = 0;
    int pdchCoilWaterMassFlowIdealPeak = 0;
    int pdchCoilEntWaterTempIdealPeak = 0;
    int pdchCoilLvgWaterTempIdealPeak = 0;
    int pdchCoilWaterDeltaTempIdealPeak = 0;
    int pdchCoilRatedTotalCap = 0;
    int pdchCoilRatedSensCap = 0;
    int pdchCoilOffRatingCapacityModifierIdealPeak = 0;
    int pdchCoilOffRatingCapacityModifierSimPeak = 0;
    int pdchCoilRatedAirMass = 0;
    int pdchCoilRatedEntDryBulb = 0;
    int pdchCoilRatedEntWetBulb = 0;
    int pdchCoilRatedEntHumRat = 0;
    int pdchCoilRatedEntEnthalpy = 0;
    int pdchCoilRatedLvgDryBulb = 0;
    int pdchCoilRatedLvgWetBulb = 0;
    int pdchCoilRatedLvgHumRat = 0;
    int pdchCoilRatedLvgEnthalpy = 0;
    int pdchCoilRatedWaterMass = 0;
    int pdchCoilRatedEntWaterTemp = 0;
    int pdchCoilRatedLvgWaterTemp = 0;
    int pdchCoilRatedWaterDeltaTemp = 0;

    int pdchFanAssociatedWithCoilName = 0;
    int pdchFanAssociatedWithCoilType = 0;
    int pdchFanAssociatedVdotSize = 0;
    int pdchFanAssociatedMdotSize = 0;

    int pdchFanHeatGainIdealPeak = 0;
    int pdchCoilNetTotalCapacityIdealPeak = 0;
    int pdchCoilPlantLoopName = 0;

    int pdchPlantMassFlowMaximum = 0;
    int pdchPlantRetTempDesign = 0;
    int pdchPlantSupTempDesign = 0;
    int pdchPlantDeltaTempDesign = 0;
    int pdchPlantCapacity = 0;
    int pdchCoilCapPrcntPlantCapacity = 0;
    int pdchCoilFlowPrcntPlantFlow = 0;
    int pdchOADryBulbIdealPeak = 0;
    int pdchOAHumRatIdealPeak = 0;
    int pdchOAWetBulbatIdealPeak = 0;
    int pdchOAVolFlowIdealPeak = 0;
    int pdchOAFlowPrcntIdealPeak = 0;
    int pdchOABarometricPressureIdealPeak = 0;

    int pdchAirSysRADryBulbIdealPeak = 0;
    int pdchAirSysRAHumRatIdealPeak = 0;
    int pdchZoneAirDryBulbIdealPeak = 0;
    int pdchZoneAirHumRatIdealPeak = 0;
    int pdchZoneAirRelHumIdealPeak = 0;
    int pdchCoilUA = 0;
    int pdchZoneSensibleLoadIdealPeak = 0;
    int pdchZoneLatentLoadIdealPeak = 0;
    int pdchCoilIdealSizCapOverSimPeak = 0;
    int pdchCoilIdealSizCapUnderSimPeak = 0;
    int pdchReheatCoilMultiplier = 0;
    int pdchFlowCapRatioLowCapIncreaseRatio = 0;
    int pdchFlowCapRatioHiCapDecreaseRatio = 0;
    int pdchPlantFluidSpecificHeat = 0;
    int pdchPlantFluidDensity = 0;
    int pdchMoistAirSpecificHeat = 0;
    int pdchDryAirSpecificHeat = 0;
    int pdchStandRhoAir = 0;

    // Std 229 Coil Connections
    int pdstCoilConnections = 0;
    int pdchCoilName_CCs = 0;
    int pdchCoilType_CCs = 0;
    int pdchCoilLoc_CCs = 0;
    int pdchCoilHVACType_CCs = 0;
    int pdchCoilHVACName_CCs = 0;
    int pdchCoilZoneNames_CCs = 0;
    int pdchCoilSupFanName_CCs = 0;
    int pdchCoilSupFanType_CCs = 0;
    int pdchCoilPlantName_CCs = 0;     // Plant Name for Coil*
    int pdchCoilAirloopName_CCs = 0;   // Airloop Name
    int pdchCoilPlantloopName_CCs = 0; // Plant Loop Name

    // Coil Sizing Summary (subset of Coil Selection Table)
    int pdst2CoilSummaryCoilSelection = 0;
    int pdch2CoilName = 0;
    int pdch2CoilType = 0;
    int pdch2CoilHVACType = 0;
    int pdch2CoilHVACName = 0;

    int pdch2CoilFinalTotalCap = 0;
    int pdch2CoilFinalSensCap = 0;
    int pdch2CoilFinalAirVolFlowRate = 0;
    int pdch2CoilFinalPlantVolFlowRate = 0;
    int pdch2CoilUA = 0;

    int pdch2CoilDDnameSensIdealPeak = 0;
    int pdch2CoilDateTimeSensIdealPeak = 0;
    int pdch2CoilDDnameAirFlowIdealPeak = 0;
    int pdch2CoilDateTimeAirFlowIdealPeak = 0;
    int pdch2CoilTotalCapIdealPeak = 0;
    int pdch2CoilSensCapIdealPeak = 0;
    int pdch2CoilAirVolumeFlowIdealPeak = 0;
    int pdch2CoilEntDryBulbIdealPeak = 0;
    int pdch2CoilEntWetBulbIdealPeak = 0;
    int pdch2CoilEntHumRatIdealPeak = 0;
    int pdch2CoilLvgDryBulbIdealPeak = 0;
    int pdch2CoilLvgWetBulbIdealPeak = 0;
    int pdch2CoilLvgHumRatIdealPeak = 0;
    int pdch2CoilRatedTotalCap = 0;
    int pdch2CoilRatedSensCap = 0;

    int pdch2OADryBulbIdealPeak = 0;
    int pdch2OAHumRatIdealPeak = 0;
    int pdch2OAWetBulbatIdealPeak = 0;
    int pdch2OAFlowPrcntIdealPeak = 0;
    int pdch2ZoneAirDryBulbIdealPeak = 0;
    int pdch2ZoneAirHumRatIdealPeak = 0;
    int pdch2ZoneAirRelHumIdealPeak = 0;
    int pdch2ZoneSensibleLoadIdealPeak = 0;
    int pdch2ZoneLatentLoadIdealPeak = 0;

    // System summary
    int pdrSystem = 0;
    int pdstEconomizer = 0;
    int pdchEcoKind = 0;
    int pdchEcoMinOA = 0;
    int pdchEcoMaxOA = 0;
    int pdchEcoRetTemp = 0;
    int pdchEcoRetEnth = 0;
    int pdchEcoOATempLim = 0;
    int pdchEcoOAEnthLim = 0;
    int pdstDemCntlVent = 0;
    int pdchDCVventMechName = 0;
    int pdchDCVperPerson = 0;
    int pdchDCVperArea = 0;
    int pdchDCVperZone = 0;
    int pdchDCVperACH = 0;
    int pdchDCVMethod = 0;
    int pdchDCVOASchName = 0;

    // added for new DCV
    int pdchDCVZoneADEffCooling = 0;
    int pdchDCVZoneADEffHeating = 0;
    int pdchDCVZoneADEffSchName = 0;
    int pdchDCVType = 0;

    int pdstSimpleComfort = 0;
    int pdchSCwinterClothes = 0;
    int pdchSCsummerClothes = 0;
    int pdchSCeitherClothes = 0;
    int pdstUnmetLoads = 0;
    int pdchULnotMetHeat = 0;
    int pdchULnotMetCool = 0;
    int pdchULnotMetHeatOcc = 0;
    int pdchULnotMetCoolOcc = 0;

    int pdstStatSchd = 0;
    int pdchStatName = 0;
    int pdchStatCtrlTypeSchd = 0;
    int pdchStatSchdType1 = 0;
    int pdchStatSchdTypeName1 = 0;
    int pdchStatSchdHeatName = 0;
    int pdchStatSchdCoolName = 0;

    // HVAC Topology
    int pdrTopology = 0;

    int pdstTopAirLoop = 0;
    int pdchTopAirLoopName = 0;
    int pdchTopAirSplitName = 0;
    int pdchTopAirBranchName = 0;
    int pdchTopAirSupplyBranchType = 0;
    int pdchTopAirCompType = 0;
    int pdchTopAirCompName = 0;
    int pdchTopAirSubCompType = 0;
    int pdchTopAirSubCompName = 0;
    int pdchTopAirSubSubCompType = 0;
    int pdchTopAirSubSubCompName = 0;
    int pdchTopAirDownSplitMixName = 0;
    int pdchTopAirMixName = 0;

    int pdstTopAirDemand = 0;
    int pdchTopAirDemandName = 0;
    int pdchTopAirSupplyBranchName = 0;
    int pdchTopAirSupplyDuctType = 0;
    int pdchTopAirSupplyPCompType = 0;
    int pdchTopAirSupplyPCompName = 0;
    int pdchTopAirTermUnitType = 0;
    int pdchTopAirTermUnitName = 0;
    int pdchTopAirZoneName = 0;
    int pdchTopAirReturnPCompType = 0;
    int pdchTopAirReturnPCompName = 0;

    int pdstTopZnEqp = 0;
    int pdchTopZnEqpName = 0;
    int pdchTopZnEqpCompType = 0;
    int pdchTopZnEqpCompName = 0;
    int pdchTopZnEqpSubCompType = 0;
    int pdchTopZnEqpSubCompName = 0;
    int pdchTopZnEqpSubSubCompType = 0;
    int pdchTopZnEqpSubSubCompName = 0;

    int pdstTopPlantLoop2 = 0;
    int pdchTopPlantLoopType2 = 0;
    int pdchTopPlantLoopName2 = 0;
    int pdchTopPlantSide2 = 0;
    int pdchTopPlantSplitName2 = 0;
    int pdchTopPlantBranchName2 = 0;
    int pdchTopPlantCompType2 = 0;
    int pdchTopPlantCompName2 = 0;
    int pdchTopPlantMixName2 = 0;

    // Outdoor Air Report
    int pdrOutsideAir = 0;
    int pdstOAavgOcc = 0;
    int pdchOaoAvgNumOcc1 = 0;
    int pdchOaoNomNumOcc1 = 0;
    int pdchOaoZoneVol1 = 0;
    int pdchOaoAvgMechVent = 0;
    int pdchOaoAvgInfil = 0;
    int pdchOaoAvgAFNInfil = 0;
    int pdchOaoAvgSimpVent = 0;
    int pdchOaoAvgTotVent = 0;
    int pdstOAminOcc = 0;
    int pdchOaoAvgNumOcc2 = 0;
    int pdchOaoNomNumOcc2 = 0;
    int pdchOaoZoneVol2 = 0;
    int pdchOaoMinMechVent = 0;
    int pdchOaoMinInfil = 0;
    int pdchOaoMinAFNInfil = 0;
    int pdchOaoMinSimpVent = 0;
    int pdchOaoMinTotVent = 0;

    // Outdoor Air Details Report
    int pdrOutsideAirDetails = 0;

    int pdstOAmechVentParByZone = 0;
    int pdchOaMvAirLpNm = 0;
    int pdchOaMvAvgNumOcc = 0;
    int pdchOaMvNomNumOcc = 0;
    int pdchOaMvZoneVol = 0;
    int pdchOaMvZoneArea = 0;
    int pdchOaMvDesZnOa = 0;
    int pdchOaMvMinDynTrgVent = 0;

    int pdstOAtotAirByZone = 0;
    int pdchOaTaBzMechVent = 0;
    int pdchOaTaBzNatVent = 0;
    int pdchOaTaBzTotVent = 0;
    int pdchOaTaBzInfil = 0;
    int pdchOaTaBzTotVentInfil = 0;
    int pdchOaTaBzDynTrgVent = 0;
    int pdchOaTaBzTmBelow = 0;
    int pdchOaTaBzTmAt = 0;
    int pdchOaTaBzTmAbove = 0;
    int pdchOaTaBzTmAboveUnocc = 0;

    int pdstOAavgOccByZone = 0;
    int pdchOaOccBzMechVent = 0;
    int pdchOaOccBzNatVent = 0;
    int pdchOaOccBzTotVent = 0;
    int pdchOaOccBzInfil = 0;
    int pdchOaOccBzTotVentInfil = 0;
    int pdchOaOccBzDynTrgVent = 0;
    int pdchOaOccBzTmBelow = 0;
    int pdchOaOccBzTmAt = 0;
    int pdchOaOccBzTmAbove = 0;

    int pdstOAtotAirByLoop = 0;
    int pdchOaTaAlMechVent = 0;
    int pdchOaTaAlNatVent = 0;
    int pdchOaTaAlTotVent = 0;
    int pdchOaTaAlSumDynTrgVent = 0;
    int pdchOaTaAlTmBelow = 0;
    int pdchOaTaAlTmAt = 0;
    int pdchOaTaAlTmAbove = 0;
    int pdchOaTaAlTmAboveUnocc = 0;

    int pdstOAavgOccByLoop = 0;
    int pdchOaOccAlMechVent = 0;
    int pdchOaOccAlNatVent = 0;
    int pdchOaOccAlTotVent = 0;
    int pdchOaOccAlSumDynTrgVent = 0;
    int pdchOaOccAlTmBelow = 0;
    int pdchOaOccAlTmAt = 0;
    int pdchOaOccAlTmAbove = 0;

    int pdstOAtimeFactorsDurOcc = 0;
    int pdchOaTmFctNoLimit = 0;
    int pdchOaTmFctHiHumid = 0;
    int pdchOaTmFctNiteVent = 0;
    int pdchOaTmFctEcono = 0;
    int pdchOaTmFctDCV = 0;
    int pdchOaTmFctExhaust = 0;
    int pdchOaTmFctMixedLimit = 0;
    int pdchOaTmFctLimit = 0;
    int pdchOaTmFctDemand = 0;
    int pdchOaTmFctEMS = 0;

    int pdstOAavgFactorsDurOcc = 0;
    int pdchOaAvFctNoLimit = 0;
    int pdchOaAvFctHiHumid = 0;
    int pdchOaAvFctNiteVent = 0;
    int pdchOaAvFctEcono = 0;
    int pdchOaAvFctDCV = 0;
    int pdchOaAvFctExhaust = 0;
    int pdchOaAvFctMixedLimit = 0;
    int pdchOaAvFctLimit = 0;
    int pdchOaAvFctDemand = 0;
    int pdchOaAvFctEMS = 0;

    // Object Count Report
    int pdrObjCnt = 0;
    int pdstSurfCnt = 0;
    int pdchSurfCntTot = 0;
    int pdchSurfCntExt = 0;
    int pdstHVACcnt = 0;
    int pdchHVACcntVal = 0;
    int pdstFieldCnt = 0;
    int pdchFieldCntVal = 0;

    // Energy Meters Report
    int pdrEnergyMeters = 0;

    int pdstEMelecvalues = 0;
    int pdchEMelecannual = 0;
    int pdchEMelecminvalue = 0;
    int pdchEMelecminvaluetime = 0;
    int pdchEMelecmaxvalue = 0;
    int pdchEMelecmaxvaluetime = 0;

    int pdstEMgasvalues = 0;
    int pdchEMgasannual = 0;
    int pdchEMgasminvalue = 0;
    int pdchEMgasminvaluetime = 0;
    int pdchEMgasmaxvalue = 0;
    int pdchEMgasmaxvaluetime = 0;

    int pdstEMcoolvalues = 0;
    int pdchEMcoolannual = 0;
    int pdchEMcoolminvalue = 0;
    int pdchEMcoolminvaluetime = 0;
    int pdchEMcoolmaxvalue = 0;
    int pdchEMcoolmaxvaluetime = 0;

    int pdstEMwatervalues = 0;
    int pdchEMwaterannual = 0;
    int pdchEMwaterminvalue = 0;
    int pdchEMwaterminvaluetime = 0;
    int pdchEMwatermaxvalue = 0;
    int pdchEMwatermaxvaluetime = 0;

    int pdstEMotherJvalues = 0;
    int pdchEMotherJannual = 0;
    int pdchEMotherJminvalue = 0;
    int pdchEMotherJminvaluetime = 0;
    int pdchEMotherJmaxvalue = 0;
    int pdchEMotherJmaxvaluetime = 0;

    int pdstEMotherKGvalues = 0;
    int pdchEMotherKGannual = 0;
    int pdchEMotherKGminvalue = 0;
    int pdchEMotherKGminvaluetime = 0;
    int pdchEMotherKGmaxvalue = 0;
    int pdchEMotherKGmaxvaluetime = 0;

    int pdstEMotherM3values = 0;
    int pdchEMotherM3annual = 0;
    int pdchEMotherM3minvalue = 0;
    int pdchEMotherM3minvaluetime = 0;
    int pdchEMotherM3maxvalue = 0;
    int pdchEMotherM3maxvaluetime = 0;

    int pdstEMotherLvalues = 0;
    int pdchEMotherLannual = 0;
    int pdchEMotherLminvalue = 0;
    int pdchEMotherLminvaluetime = 0;
    int pdchEMotherLmaxvalue = 0;
    int pdchEMotherLmaxvaluetime = 0;

    // Sensible Heat Gas Component Report
    int pdrSensibleGain = 0;
    // annual
    int pdstSHGSannual = 0;
    int pdchSHGSAnZoneEqHt = 0;
    int pdchSHGSAnZoneEqCl = 0;
    int pdchSHGSAnHvacATUHt = 0;
    int pdchSHGSAnHvacATUCl = 0;
    int pdchSHGSAnSurfHt = 0;
    int pdchSHGSAnSurfCl = 0;
    int pdchSHGSAnPeoplAdd = 0;
    int pdchSHGSAnLiteAdd = 0;
    int pdchSHGSAnEquipAdd = 0;
    int pdchSHGSAnWindAdd = 0;
    int pdchSHGSAnIzaAdd = 0;
    int pdchSHGSAnInfilAdd = 0;
    int pdchSHGSAnOtherAdd = 0;
    int pdchSHGSAnEquipRem = 0;
    int pdchSHGSAnWindRem = 0;
    int pdchSHGSAnIzaRem = 0;
    int pdchSHGSAnInfilRem = 0;
    int pdchSHGSAnOtherRem = 0;
    // peak cooling
    int pdstSHGSpkCl = 0;
    int pdchSHGSClTimePeak = 0;
    int pdchSHGSClHvacHt = 0;
    int pdchSHGSClHvacCl = 0;
    int pdchSHGSClHvacATUHt = 0;
    int pdchSHGSClHvacATUCl = 0;
    int pdchSHGSClSurfHt = 0;
    int pdchSHGSClSurfCl = 0;
    int pdchSHGSClPeoplAdd = 0;
    int pdchSHGSClLiteAdd = 0;
    int pdchSHGSClEquipAdd = 0;
    int pdchSHGSClWindAdd = 0;
    int pdchSHGSClIzaAdd = 0;
    int pdchSHGSClInfilAdd = 0;
    int pdchSHGSClOtherAdd = 0;
    int pdchSHGSClEquipRem = 0;
    int pdchSHGSClWindRem = 0;
    int pdchSHGSClIzaRem = 0;
    int pdchSHGSClInfilRem = 0;
    int pdchSHGSClOtherRem = 0;
    // peak heating
    int pdstSHGSpkHt = 0;
    int pdchSHGSHtTimePeak = 0;
    int pdchSHGSHtHvacHt = 0;
    int pdchSHGSHtHvacCl = 0;
    int pdchSHGSHtHvacATUHt = 0;
    int pdchSHGSHtHvacATUCl = 0;
    int pdchSHGSHtSurfHt = 0;
    int pdchSHGSHtSurfCl = 0;
    int pdchSHGSHtPeoplAdd = 0;
    int pdchSHGSHtLiteAdd = 0;
    int pdchSHGSHtEquipAdd = 0;
    int pdchSHGSHtWindAdd = 0;
    int pdchSHGSHtIzaAdd = 0;
    int pdchSHGSHtInfilAdd = 0;
    int pdchSHGSHtOtherAdd = 0;
    int pdchSHGSHtEquipRem = 0;
    int pdchSHGSHtWindRem = 0;
    int pdchSHGSHtIzaRem = 0;
    int pdchSHGSHtInfilRem = 0;
    int pdchSHGSHtOtherRem = 0;
    // Standard62Report
    int pdrStd62 = 0;
    int pdstS62sysVentReqCool = 0;
    int pdchS62svrClSumVpz = 0;
    int pdchS62svrClPs = 0;
    int pdchS62svrClSumPz = 0;
    int pdchS62svrClD = 0;
    int pdchS62svrClDorg = 0;
    int pdchS62svrClVou = 0;
    int pdchS62svrClVps = 0;
    int pdchS62svrClXs = 0;
    int pdchS62svrClEv = 0;
    int pdchS62svrClEvMthd = 0;
    int pdchS62svrClVot = 0;
    int pdchS62svrClPercOA = 0;
    int pdchS62svrClEnvironmentOfPs = 0;
    int pdchS62svrClTimeOfPs = 0;

    int pdstS62sysVentReqHeat = 0;
    int pdchS62svrHtSumVpz = 0;
    int pdchS62svrHtPs = 0;
    int pdchS62svrHtSumPz = 0;
    int pdchS62svrHtD = 0;
    int pdchS62svrHtDorg = 0;
    int pdchS62svrHtVou = 0;
    int pdchS62svrHtVps = 0;
    int pdchS62svrHtXs = 0;
    int pdchS62svrHtEv = 0;
    int pdchS62svrHtEvMthd = 0;
    int pdchS62svrHtVot = 0;
    int pdchS62svrHtPercOA = 0;
    int pdchS62svrHtEnvironmentOfPs = 0;
    int pdchS62svrHtTimeOfPs = 0;

    int pdstS62znVentPar = 0;
    int pdchS62zvpAlN = 0;
    int pdchS62zvpRp = 0;
    int pdchS62zvpPz = 0;
    int pdchS62zvpRa = 0;
    int pdchS62zvpAz = 0;
    int pdchS62zvpVbz = 0;
    int pdchS62zvpClEz = 0;
    int pdchS62zvpClVoz = 0;
    int pdchS62zvpHtEz = 0;
    int pdchS62zvpHtVoz = 0;

    int pdstS62sysVentPar = 0;
    int pdchS62svpRp = 0;
    int pdchS62svpPz = 0;
    int pdchS62svpRa = 0;
    int pdchS62svpAz = 0;
    int pdchS62svpVbz = 0;
    int pdchS62svpClVoz = 0;
    int pdchS62svpHtVoz = 0;

    int pdstS62znCoolDes = 0;
    int pdchS62zcdAlN = 0;
    int pdchS62zcdBox = 0;
    int pdchS62zcdVpz = 0;
    int pdchS62zcdVps = 0;
    int pdchS62zcdVsec = 0;
    int pdchS62zcdVdz = 0;
    int pdchS62zcdVpzmin = 0;
    int pdchS62zcdVpzminSPSize = 0;
    int pdchS62zcdVozclg = 0;
    int pdchS62zcdZpz = 0;
    int pdchS62zcdEp = 0;
    int pdchS62zcdEr = 0;
    int pdchS62zcdFa = 0;
    int pdchS62zcdFb = 0;
    int pdchS62zcdFc = 0;
    int pdchS62zcdEvz = 0;

    int pdstS62sysCoolDes = 0;
    int pdchS62scdVpz = 0;
    int pdchS62scdVps = 0;
    int pdchS62scdVsec = 0;
    int pdchS62scdVdz = 0;
    int pdchS62scdVpzmin = 0;
    int pdchS62scdVozclg = 0;
    int pdchS62scdEvz = 0;

    int pdstS62znHeatDes = 0;
    int pdchS62zhdAlN = 0;
    int pdchS62zhdBox = 0;
    int pdchS62zhdVpz = 0;
    int pdchS62zhdVps = 0;
    int pdchS62zhdVsec = 0;
    int pdchS62zhdVdz = 0;
    int pdchS62zhdVpzmin = 0;
    int pdchS62zhdVpzminSPSize = 0;
    int pdchS62zhdVozhtg = 0;
    int pdchS62zhdZpz = 0;
    int pdchS62zhdEp = 0;
    int pdchS62zhdEr = 0;
    int pdchS62zhdFa = 0;
    int pdchS62zhdFb = 0;
    int pdchS62zhdFc = 0;
    int pdchS62zhdEvz = 0;

    int pdstS62sysHeatDes = 0;
    int pdchS62shdVpz = 0;
    int pdchS62shdVps = 0;
    int pdchS62shdVsec = 0;
    int pdchS62shdVdz = 0;
    int pdchS62shdVpzmin = 0;
    int pdchS62shdVozhtg = 0;
    int pdchS62shdEvz = 0;

    //  LEED Summary
    int pdrLeed = 0;
    int pdstLeedGenInfo = 0;
    int pdchLeedGenData = 0;

    int pdstLeedSpaceUsageType = 0;
    int pdchLeedSutName = 0;
    int pdchLeedSutSpArea = 0;
    int pdchLeedSutOcArea = 0;
    int pdchLeedSutUnArea = 0;
    int pdchLeedSutHrsWeek = 0;

    int pdstLeedAdvsMsg = 0;
    int pdchLeedAmData = 0;

    int pdstLeedEneTypSum = 0;
    int pdchLeedEtsType = 0;
    int pdchLeedEtsRtNm = 0;
    int pdchLeedEtsVirt = 0;
    int pdchLeedEtsEneUnt = 0;
    int pdchLeedEtsDemUnt = 0;

    int pdstLeedPerf = 0;
    int pdchLeedPerfRot = 0;
    int pdchLeedPerfElEneUse = 0;
    int pdchLeedPerfElDem = 0;
    int pdchLeedPerfGasEneUse = 0;
    int pdchLeedPerfGasDem = 0;
    int pdchLeedPerfGasolineEneUse = 0;
    int pdchLeedPerfGasolineDem = 0;
    int pdchLeedPerfDieselEneUse = 0;
    int pdchLeedPerfDieselDem = 0;
    int pdchLeedPerfCoalEneUse = 0;
    int pdchLeedPerfCoalDem = 0;
    int pdchLeedPerfFuelOil1EneUse = 0;
    int pdchLeedPerfFuelOil1Dem = 0;
    int pdchLeedPerfFuelOil2EneUse = 0;
    int pdchLeedPerfFuelOil2Dem = 0;
    int pdchLeedPerfPropaneEneUse = 0;
    int pdchLeedPerfPropaneDem = 0;
    int pdchLeedPerfOtherFuel1EneUse = 0;
    int pdchLeedPerfOtherFuel1Dem = 0;
    int pdchLeedPerfOtherFuel2EneUse = 0;
    int pdchLeedPerfOtherFuel2Dem = 0;
    int pdchLeedPerfDisClEneUse = 0;
    int pdchLeedPerfDisClDem = 0;
    int pdchLeedPerfDisHtWtrEneUse = 0;
    int pdchLeedPerfDisHtWtrDem = 0;
    int pdchLeedPerfDisHtStEneUse = 0;
    int pdchLeedPerfDisHtStDem = 0;

    int pdstLeedEneUseSum = 0;
    int pdchLeedEusUnt = 0;
    int pdchLeedEusProc = 0;
    int pdchLeedEusTotal = 0;

    int pdstLeedEneCostSum = 0;
    int pdchLeedEcUnt = 0;
    int pdchLeedEcsProc = 0;
    int pdchLeedEcsTotal = 0;
    Real64 LEEDelecCostTotal = 0;
    Real64 LEEDgasCostTotal = 0;
    Real64 LEEDothrCostTotal = 0;

    int pdstLeedRenewSum = 0;
    int pdchLeedRenRatCap = 0;
    int pdchLeedRenAnGen = 0;

    int pdstLeedEneUseIntEl = 0;
    int pdchLeedEuiElec = 0;
    int pdstLeedEneUseIntNatG = 0;
    int pdchLeedEuiNatG = 0;
    int pdstLeedEneUseIntOthr = 0;
    int pdchLeedEuiOthr = 0;

    int pdstLeedEneUsePerc = 0;
    int pdchLeedEupPerc = 0;

    int pdstLeedEqFlLdHrs = 0;
    int pdchLeedEflhEflh = 0;
    int pdchLeedEflhNonZerHrs = 0;

    int pdstLeedSchedSetPts = 0;
    int pdChLeedSchStPtFirstObjUsed = 0;
    int pdChLeedSchStPtMonthUsed = 0;
    int pdchLeedSchStPt11amWednesday = 0;
    int pdchLeedSchStPt11amWedCnt = 0;
    int pdchLeedSchStPt11pmWednesday = 0;
    int pdchLeedSchStPt11pmWedCnt = 0;

    int pdrThermalResilience = 0;
    int pdstHIHours = 0;
    int pdchHIHourSafe = 0;
    int pdchHIHourCaution = 0;
    int pdchHIHourExtremeCaution = 0;
    int pdchHIHourDanger = 0;
    int pdchHIHourExtremeDanger = 0;
    int pdstHIOccuHours = 0;
    int pdchHIOccuHourSafe = 0;
    int pdchHIOccuHourCaution = 0;
    int pdchHIOccuHourExtremeCaution = 0;
    int pdchHIOccuHourDanger = 0;
    int pdchHIOccuHourExtremeDanger = 0;
    int pdstHIOccupiedHours = 0;
    int pdchHIOccupiedHourSafe = 0;
    int pdchHIOccupiedHourCaution = 0;
    int pdchHIOccupiedHourExtremeCaution = 0;
    int pdchHIOccupiedHourDanger = 0;
    int pdchHIOccupiedHourExtremeDanger = 0;
    int pdstHumidexHours = 0;
    int pdchHumidexHourLittle = 0;
    int pdchHumidexHourSome = 0;
    int pdchHumidexHourGreat = 0;
    int pdchHumidexHourDanger = 0;
    int pdchHumidexHourStroke = 0;
    int pdstHumidexOccuHours = 0;
    int pdchHumidexOccuHourLittle = 0;
    int pdchHumidexOccuHourSome = 0;
    int pdchHumidexOccuHourGreat = 0;
    int pdchHumidexOccuHourDanger = 0;
    int pdchHumidexOccuHourStroke = 0;
    int pdstHumidexOccupiedHours = 0;
    int pdchHumidexOccupiedHourLittle = 0;
    int pdchHumidexOccupiedHourSome = 0;
    int pdchHumidexOccupiedHourGreat = 0;
    int pdchHumidexOccupiedHourDanger = 0;
    int pdchHumidexOccupiedHourStroke = 0;

    int pdstHeatingSETHours = 0;
    int pdchHeatingSETHours = 0;
    int pdchHeatingSETOccuHours = 0;
    int pdchHeatingSETOccupiedHours = 0;
    int pdchHeatingSETUnmetDuration = 0;
    int pdchHeatingSETUnmetTime = 0;
    int pdstCoolingSETHours = 0;
    int pdchCoolingSETHours = 0;
    int pdchCoolingSETOccuHours = 0;
    int pdchCoolingSETOccupiedHours = 0;
    int pdchCoolingSETUnmetDuration = 0;
    int pdchCoolingSETUnmetTime = 0;

    int pdstHourOfSafetyColdEvent = 0;
    int pdchColdHourOfSafety = 0;
    int pdchColdSafetyEndTime = 0;
    int pdchColdSafeTempExceedHour = 0;
    int pdchColdSafeTempExceedOccHour = 0;
    int pdchColdSafeTempExceedOccupiedHour = 0;
    int pdstHourOfSafetyHeatEvent = 0;
    int pdchHeatHourOfSafety = 0;
    int pdchHeatSafetyEndTime = 0;
    int pdchHeatSafeTempExceedHour = 0;
    int pdchHeatSafeTempExceedOccHour = 0;
    int pdchHeatSafeTempExceedOccupiedHour = 0;

    int pdstUnmetDegreeHour = 0;
    int pdchCoolingUnmetDegreeHour = 0;
    int pdchCoolingUnmetDegreeOccHour = 0;
    int pdchCoolingUnmetDegreeOccupiedHour = 0;
    int pdchHeatingUnmetDegreeHour = 0;
    int pdchHeatingUnmetDegreeOccHour = 0;
    int pdchHeatingUnmetDegreeOccupiedHour = 0;

    int pdstDiscomfortWtExceedOccuHour = 0;
    int pdstDiscomfortWtExceedOccupiedHour = 0;
    int pdchVeryColdExceedOccuHour = 0;
    int pdchCoolExceedOccuHour = 0;
    int pdchWarmExceedOccuHour = 0;
    int pdchVeryHotExceedOccuHour = 0;
    int pdchVeryColdExceedOccupiedHour = 0;
    int pdchCoolExceedOccupiedHour = 0;
    int pdchWarmExceedOccupiedHour = 0;
    int pdchVeryHotExceedOccupiedHour = 0;

    int pdrCO2Resilience = 0;
    int pdstCO2Hours = 0;
    int pdchCO2HourSafe = 0;
    int pdchCO2HourCaution = 0;
    int pdchCO2HourHazard = 0;
    int pdstCO2OccuHours = 0;
    int pdchCO2OccuHourSafe = 0;
    int pdchCO2OccuHourCaution = 0;
    int pdchCO2OccuHourHazard = 0;
    int pdstCO2OccupiedHours = 0;
    int pdchCO2OccupiedHourSafe = 0;
    int pdchCO2OccupiedHourCaution = 0;
    int pdchCO2OccupiedHourHazard = 0;

    int pdrVisualResilience = 0;
    int pdstIllumHours = 0;
    int pdchIllumHourDark = 0;
    int pdchIllumHourDim = 0;
    int pdchIllumHourAdequate = 0;
    int pdchIllumHourBright = 0;
    int pdstIllumOccuHours = 0;
    int pdchIllumOccuHourDark = 0;
    int pdchIllumOccuHourDim = 0;
    int pdchIllumOccuHourAdequate = 0;
    int pdchIllumOccuHourBright = 0;
    int pdstIllumOccupiedHours = 0;
    int pdchIllumOccupiedHourDark = 0;
    int pdchIllumOccupiedHourDim = 0;
    int pdchIllumOccupiedHourAdequate = 0;
    int pdchIllumOccupiedHourBright = 0;

    int sizeReportName = 0;
    int numReportName = 0;

    int sizeSubTable = 0;
    int numSubTable = 0;

    int sizeColumnTag = 0;
    int numColumnTag = 0;

    int sizeTableEntry = 0;
    int numTableEntry = 0;

    int sizeCompSizeTableEntry = 0;
    int numCompSizeTableEntry = 0;

    int sizeShadowRelate = 0;
    int numShadowRelate = 0;

    // Totals for ABUPS report
    Real64 TotalNotMetHeatingOccupiedForABUPS = 0.0;
    Real64 TotalNotMetCoolingOccupiedForABUPS = 0.0;
    Real64 TotalNotMetOccupiedForABUPS = 0.0;
    Real64 TotalTimeNotSimpleASH55EitherForABUPS = 0.0;

    // Totals for OA details report
    Real64 TotalVozMax = 0.0;                     // total Voz (max of heating and cooling)
    Real64 TotalAnyZoneBelowVozDynForOA = 0.0;    // total time any zone below target Voz-Dyn
    Real64 TotalAllZonesAtVozDynForOA = 0.0;      // total time all zones at target Voz-Dyn
    Real64 TotalAnyZoneAboveVozDynForOA = 0.0;    // total time any zone above target Voz-Dyn
    Real64 TotalAnyZoneBelowVozDynOccForOA = 0.0; // total time any zone below target Voz-Dyn
    Real64 TotalAllZonesAtVozDynOccForOA = 0.0;   // total time all zones at target Voz-Dyn
    Real64 TotalAnyZoneAboveVozDynOccForOA = 0.0; // total time any zone above target Voz-Dyn
    Real64 TotalAnyZoneVentUnoccForOA = 0.0;      // total time any zone has ventilation when unoccupied

    Real64 TotalAnyAirLoopBelowVozDynForOA = 0.0;    // total time any air loop below target Voz-Dyn
    Real64 TotalAllAirLoopsAtVozDynForOA = 0.0;      // total time all air loops at target Voz-Dyn
    Real64 TotalAnyAirLoopAboveVozDynForOA = 0.0;    // total time any air loop above target Voz-Dyn
    Real64 TotalAnyAirLoopBelowVozDynOccForOA = 0.0; // total time any air loop below target Voz-Dyn
    Real64 TotalAllAirLoopsAtVozDynOccForOA = 0.0;   // total time all air loops at target Voz-Dyn
    Real64 TotalAnyAirLoopAboveVozDynOccForOA = 0.0; // total time any air loop above target Voz-Dyn
    Real64 TotalAnyAirLoopVentUnoccForOA = 0.0;      // total time any air loop has ventilation when unoccupied

    Array1D<OutputReportPredefined::reportNameType> reportName;
    Array1D<OutputReportPredefined::SubTableType> subTable;
    Array1D<OutputReportPredefined::ColumnTagType> columnTag;
    Array1D<OutputReportPredefined::TableEntryType> tableEntry;
    Array1D<OutputReportPredefined::CompSizeTableEntryType> CompSizeTableEntry;
    Array1D<OutputReportPredefined::ShadowRelateType> ShadowRelate;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) OutputReportPredefinedData();
    }
};

} // namespace EnergyPlus

#endif
