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

#ifndef OutputReportTabular_hh_INCLUDED
#define OutputReportTabular_hh_INCLUDED

// C++ Headers
#include <fstream>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array2S.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace OutputReportTabular {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS:

    enum class AggType
    {
        Invalid = -1,
        SumOrAvg,
        Maximum,
        Minimum,
        ValueWhenMaxMin,
        HoursZero,
        HoursNonZero,
        HoursPositive,
        HoursNonPositive,
        HoursNegative,
        HoursNonNegative,
        SumOrAverageHoursShown,
        MaximumDuringHoursShown,
        MinimumDuringHoursShown,
        Num
    };

    enum class TableStyle
    {
        Invalid = -1,
        Comma,
        Tab,
        Fixed,
        HTML,
        XML,
        Num
    };

    enum class UnitsStyle
    {
        Invalid = -1,
        None,
        JtoKWH,
        JtoMJ,
        JtoGJ,
        InchPound,
        InchPoundExceptElectricity,
        NotFound,
        Num
    };
    constexpr std::array<std::string_view, static_cast<int>(UnitsStyle::Num) - 1> UnitsStyleNamesUC{
        "NONE", "JTOKWH", "JTOMJ", "JTOGJ", "INCHPOUND", "INCHPOUNDEXCEPTELECTRICITY"};

    enum class EndUseSubTableType
    {
        Invalid = -1,
        BySubCategory,
        BySpaceType,
        Num
    };

    enum LoadCompCol // These correspond to the columns in the load component table
    {
        // Invalid is not necessary since this is not specified by the user in the IDF
        SensInst = 1,
        SensDelay,
        SensRA,
        Latent,
        Total,
        Perc,
        Area,
        PerArea
    };

    enum LoadCompRow
    {
        // Invalid is not necessary since this is not specified by the user in the IDF
        // internal gains
        People = 1,
        Lights,
        Equip,
        Refrig,
        WaterUse,
        HvacLoss,
        PowerGen,

        // misc
        DOAS,
        Infil,
        ZoneVent,
        IntZonMix,

        // opaque surfaces
        Roof,
        IntZonCeil,
        OtherRoof,
        ExtWall,
        IntZonWall,
        GrdWall,
        OtherWall,
        ExtFlr,
        IntZonFlr,
        GrdFlr,
        OtherFlr,

        // subsurfaces
        FeneCond,
        FeneSolr,
        OpqDoor,

        // total
        GrdTot
    };

    // BEPS Report Related Variables
    // From Report:Table:Predefined - BEPS
    constexpr int numResourceTypes(14);
    constexpr int numSourceTypes(12);

    constexpr const char *validChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_:.");

    enum class OutputType
    {
        Invalid = -1,
        Space,
        Zone,
        AirLoop,
        Facility,
        Num
    };

    // MODULE VARIABLE DECLARATIONS:

    // The Binned table type is different and only references one variable and its structure is very
    // different from the others so it is has its own type.

    constexpr int numNamedMonthly(63);
    // These reports are detailed/named in routine InitializePredefinedMonthlyTitles

    // Allow up to five output files to be created
    constexpr int maxNumStyles(5);

    // LineTypes for reading the stat file
    enum class StatLineType
    {
        Invalid = -1,
        Initialized, // used as a dummy placeholder
        StatisticsLine,
        LocationLine,
        LatLongLine,
        ElevationLine,
        StdPressureLine,
        DataSourceLine,
        WMOStationLine,
        DesignConditionsLine,
        HeatingConditionsLine,
        CoolingConditionsLine,
        StdHDDLine,
        StdCDDLine,
        MaxDryBulbLine,
        MinDryBulbLine,
        MaxDewPointLine,
        MinDewPointLine,
        WithHDDLine,
        WithCDDLine,
        MaxHourlyPrec,
        MonthlyPrec,
        KoppenLine,
        KoppenDes1Line,
        KoppenDes2Line,
        AshStdLine,
        AshStdDes1Line,
        AshStdDes2Line,
        AshStdDes3Line,
        Num
    };

    static constexpr std::array<DataHeatBalance::IntGainType, 1> IntGainTypesTubularGCLS = {DataHeatBalance::IntGainType::DaylightingDeviceTubular};

    // Types

    struct OutputTableBinnedType
    {
        // Members
        std::string keyValue;       // the key value (usually an asterisk to indicate all variables
        std::string varOrMeter;     // the name of the variable or meter
        Real64 intervalStart = 0.0; // The lowest value for the intervals being binned into.
        Real64 intervalSize = 0.0;  // The size of the bins starting with Interval start.
        int intervalCount = 0;      // The number of bins used. The number of hours below the start of
        // the lowest bin and above the value of the last bin are also shown.
        int resIndex = 0; // result index - pointer to BinResults array
        int numTables = 0;
        OutputProcessor::VariableType typeOfVar = OutputProcessor::VariableType::Invalid; // Was NotFound
        OutputProcessor::StoreType avgSum = OutputProcessor::StoreType::Average;          // Variable  is Averaged=1 or Summed=2
        OutputProcessor::TimeStepType stepType = OutputProcessor::TimeStepType::Zone;     // Variable time step is Zone=1 or HVAC=2
        Constant::Units units = Constant::Units::Invalid;                                 // the units enumeration
        Sched::Schedule *sched = nullptr;                                                 // index to the schedule specified - if no schedule use zero
    };

    struct BinResultsType
    {
        // Members
        Array1D<Real64> mnth; // monthly bins
        Array1D<Real64> hrly; // hourly bins

        // Default Constructor
        BinResultsType() : mnth(12, 0.0), hrly(24, 0.0)
        {
        }
    };

    struct BinObjVarIDType
    {
        // Members
        std::string namesOfObj; // name of the object
        int varMeterNum;        // variable or meter number

        // Default Constructor
        BinObjVarIDType() : varMeterNum(0)
        {
        }
    };

    struct BinStatisticsType
    {
        // Members
        Real64 sum;     // sum of the variable
        Real64 sum2;    // sum of the variable squared
        int n;          // number of items in sum
        Real64 minimum; // minimum value
        Real64 maximum; // maximum value

        // Default Constructor
        BinStatisticsType() : sum(0.0), sum2(0.0), n(0), minimum(0.0), maximum(0.0)
        {
        }
    };

    struct NamedMonthlyType
    {
        // Members
        std::string title; // report title
        bool show;         // if report should be shown

        // Default Constructor
        NamedMonthlyType() : show(false)
        {
        }
    };

    struct MonthlyInputType
    {
        // Members
        std::string name;      // identifier
        int numFieldSet = 0;   // number of monthly field sets
        int firstFieldSet = 0; // pointer to the first field set
        int numTables = 0;     // number of tables
        int firstTable = 0;    // pointer to the first table
        int showDigits = 0;    // the number of digits to be shown
        bool isNamedMonthly = false;
    };

    struct MonthlyFieldSetInputType
    {
        // Members
        std::string variMeter;      // the name of the variable or meter
        std::string colHead;        // the column header to use instead of the variable name (only for predefined)
        AggType aggregate;          // the type of aggregation for the variable (see aggType parameters)
        Constant::Units varUnits;   // Units enumeration
        std::string variMeterUpper; // the name of the variable or meter uppercased
        OutputProcessor::VariableType typeOfVar;
        int keyCount;                              // noel
        OutputProcessor::StoreType varAvgSum;      // Variable  is Averaged=1 or Summed=2
        OutputProcessor::TimeStepType varStepType; // Variable time step is Zone=1 or HVAC=2
        Array1D_string NamesOfKeys;                // keyNames !noel
        Array1D_int IndexesForKeyVar;              // keyVarIndexes !noel

        // Default Constructor
        MonthlyFieldSetInputType()
            : aggregate(AggType::Invalid), varUnits(Constant::Units::None), typeOfVar(OutputProcessor::VariableType::Invalid), keyCount(0),
              varAvgSum(OutputProcessor::StoreType::Average), varStepType(OutputProcessor::TimeStepType::Zone)
        {
        }
    };

    struct MonthlyTablesType
    {
        // Members
        std::string keyValue; // the key value - the object names that result in the variable
        int firstColumn;      // pointer to the monthly column array for the first item
        int numColumns;       // number of columns for the table

        // Default Constructor
        MonthlyTablesType() : firstColumn(0), numColumns(0)
        {
        }
    };

    struct MonthlyColumnsType
    {
        // Members
        std::string varName;                     // name of variable
        std::string colHead;                     // column header (not used for user defined monthly)
        int varNum;                              // variable or meter number
        OutputProcessor::VariableType typeOfVar; // 0=not found, 1=integer, 2=real, 3=meter
        OutputProcessor::StoreType avgSum;       // Variable  is Averaged=1 or Summed=2
        OutputProcessor::TimeStepType stepType;  // Variable time step is Zone=1 or HVAC=2
        Constant::Units units;                   // the units string, may be blank
        AggType aggType;                         // index to the type of aggregation (see list of parameters)
        Array1D<Real64> reslt;                   // monthly results
        Array1D<Real64> duration;                // the time during which results are summed for use in averages
        Array1D_int timeStamp;                   // encoded timestamp of max or min
        Real64 aggForStep;                       // holds the aggregation for the HVAC time steps when smaller than
        // the zone timestep

        // Default Constructor
        MonthlyColumnsType()
            : varNum(0), typeOfVar(OutputProcessor::VariableType::Invalid), avgSum(OutputProcessor::StoreType::Average),
              stepType(OutputProcessor::TimeStepType::Zone), units(Constant::Units::None), aggType(AggType::Invalid), reslt(12, 0.0),
              duration(12, 0.0), timeStamp(12, 0), aggForStep(0.0)
        {
        }
    };

    struct TOCEntriesType
    {
        // Members
        std::string reportName;  // the name of the individual report
        std::string sectionName; // the name of the section containing individual reports
        bool isWritten;          // flag if the entry has been written to TOC

        // Default Constructor
        TOCEntriesType() : isWritten(false)
        {
        }
    };

    struct UnitConvType
    {
        // Members
        std::string siName; // the name abbreviation or symbol of the SI units
        std::string ipName; // the name abbreviation or symbol of the IP units
        Real64 mult;        // the multiplier used to convert from SI to IP in IP = (SI * mult) + offset
        Real64 offset;      // the offset used to convert from SI to IP in IP = (SI * mult) + offset
        std::string hint;   // the string used when multiple SI units match
        bool several;       // several different options for the SI unit to be converted into IP
        bool is_default;    // if part of a set of "several" this should be used as default

        // Default Constructor
        UnitConvType() : mult(1.0), offset(0.0), several(false), is_default(false)
        {
        }
    };

    struct CompLoadTablesType
    {
        // members
        int desDayNum = 0;              // design day number
        int timeStepMax = 0;            // times step of the day that the maximum occurs
        Array2D<Real64> cells;          // main component table results (column, row)
        Array2D_bool cellUsed;          // flag if the cell is used for the table of results (column, row)
        std::string peakDateHrMin = ""; // string containing peak timestamp
        Real64 outsideDryBulb = 0.0;    // outside dry bulb temperature at peak
        Real64 outsideWetBulb = 0.0;    // outside wet bulb temperature at peak
        Real64 outsideHumRatio = 0.0;   // outside humidity ratio at peak
        Real64 zoneDryBulb = 0.0;       // zone dry bulb temperature at peak
        Real64 zoneRelHum = 0.0;        // zone relative humidity at peak
        Real64 zoneHumRatio = 0.0;      // zone humidity ratio at peak

        Real64 supAirTemp = 0.0;     // supply air temperature
        Real64 mixAirTemp = 0.0;     // mixed air temperature
        Real64 mainFanAirFlow = 0.0; // main fan air flow
        Real64 outsideAirFlow = 0.0; // outside air flow
        Real64 designPeakLoad = 0.0; // design peak load
        Real64 diffDesignPeak = 0.0; // difference between Design and Peak Load

        Real64 peakDesSensLoad = 0.0;    // peak design sensible load
        Real64 estInstDelSensLoad = 0.0; // estimated instant plus delayed sensible load
        Real64 diffPeakEst = 0.0;        // difference between the peak design sensible load and the estimated instant plus delayed sensible load
        Array1D_int zoneIndices;         // the zone numbers covered by the report

        Real64 outsideAirRatio = 0.0;   // outside Air
        Real64 floorArea = 0.0;         // floor area
        Real64 airflowPerFlrArea = 0.0; // airflow per floor area
        Real64 airflowPerTotCap = 0.0;  // airflow per total capacity
        Real64 areaPerTotCap = 0.0;     // area per total capacity
        Real64 totCapPerArea = 0.0;     // total capacity per area
        Real64 chlPumpPerFlow = 0.0;    // chiller pump power per flow
        Real64 cndPumpPerFlow = 0.0;    // condenser pump power per flow
        Real64 numPeople = 0.0;         // number of people
    };

    struct ZompComponentAreasType
    {
        // members
        Real64 floor;
        Real64 roof;
        Real64 ceiling;
        Real64 extWall;
        Real64 intZoneWall;
        Real64 grndCntWall;
        Real64 extFloor;
        Real64 intZoneFloor;
        Real64 grndCntFloor;
        Real64 fenestration;
        Real64 door;

        // default constructor
        ZompComponentAreasType()
            : floor(0.), roof(0.), ceiling(0.), extWall(0.), intZoneWall(0.), grndCntWall(0.), extFloor(0.), intZoneFloor(0.), grndCntFloor(0.),
              fenestration(0.), door(0.)
        {
        }
    };

    struct compLoadsSurface
    {
        Real64 loadConvectedNormal = 0.0;
        Real64 loadConvectedWithPulse = 0.0;
        Real64 netSurfRadSeq = 0.0;
        Real64 ITABSFseq = 0.0;     // used for determining the radiant fraction on each surface
        Real64 TMULTseq = 0.0;      // used for determining the radiant fraction on each surface
        Real64 lightSWRadSeq = 0.0; // short wave visible radiation
        Real64 feneSolarRadSeq = 0.0;
    };

    struct compLoadsTimeStepSurfaces
    {
        std::vector<compLoadsSurface> surf;
    };

    struct componentLoadsSurf
    {
        std::vector<compLoadsTimeStepSurfaces> ts;
    };

    struct compLoadsSpaceZone
    {
        Real64 peopleInstantSeq = 0.0;
        Real64 peopleLatentSeq = 0.0;

        Real64 lightInstantSeq = 0.0;
        Real64 lightRetAirSeq = 0.0;

        Real64 equipInstantSeq = 0.0;
        Real64 equipLatentSeq = 0.0;

        Real64 refrigInstantSeq = 0.0;
        Real64 refrigRetAirSeq = 0.0;
        Real64 refrigLatentSeq = 0.0;

        Real64 waterUseInstantSeq = 0.0;
        Real64 waterUseLatentSeq = 0.0;

        Real64 hvacLossInstantSeq = 0.0;

        Real64 powerGenInstantSeq = 0.0;
        Real64 powerGenRadSeq = 0.0;
        Real64 infilInstantSeq = 0.0;
        Real64 infilLatentSeq = 0.0;

        Real64 zoneVentInstantSeq = 0.0;
        Real64 zoneVentLatentSeq = 0.0;

        Real64 interZoneMixInstantSeq = 0.0;
        Real64 interZoneMixLatentSeq = 0.0;

        Real64 feneCondInstantSeq = 0.0;
        bool adjFenDone = false;
    };
    struct compLoadsTimeStepSpZn
    {
        std::vector<compLoadsSpaceZone> spacezone;
    };
    struct componentLoadsSpZn
    {
        std::vector<compLoadsTimeStepSpZn> ts;
    };

    struct compLoadsEnclosure
    {
        Real64 peopleRadSeq = 0.0;
        Real64 lightLWRadSeq = 0.0; // long wave thermal radiation
        Real64 equipRadSeq = 0.0;
        Real64 hvacLossRadSeq = 0.0;
        Real64 powerGenRadSeq = 0.0;
    };
    struct compLoadsTimeStepEncl
    {
        std::vector<compLoadsEnclosure> encl;
    };
    struct componentLoadsEncl
    {
        std::vector<compLoadsTimeStepEncl> ts;
    };
    struct tabularReportStyle
    {
        UnitsStyle unitsStyle = UnitsStyle::None;
        bool formatReals = true;
        bool produceTabular = true;
        bool produceSQLite = true;
        bool produceJSON = true;
    };

    // Functions

    std::ofstream &open_tbl_stream(EnergyPlusData &state, int const iStyle, fs::path const &filePath, bool output_to_file = true);

    void UpdateTabularReports(EnergyPlusData &state, OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    //======================================================================================================================
    //======================================================================================================================

    //    GET INPUT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void GetInputTabularMonthly(EnergyPlusData &state);

    int AddMonthlyReport(EnergyPlusData &state, std::string const &inReportName, int const inNumDigitsShown, bool isNamedMonthly = false);

    void AddMonthlyFieldSetInput(
        EnergyPlusData &state, int const inMonthReport, std::string const &inVariMeter, std::string const &inColHead, AggType const inAggregate);

    void InitializeTabularMonthly(EnergyPlusData &state);

    bool isInvalidAggregationOrder(EnergyPlusData &state);

    void GetInputTabularTimeBins(EnergyPlusData &state);

    bool warningAboutKeyNotFound(EnergyPlusData &state, int foundIndex, int inObjIndex, std::string const &moduleName);

    void GetInputTabularStyle(EnergyPlusData &state);

    UnitsStyle SetUnitsStyleFromString(std::string const &unitStringIn);

    void GetInputOutputTableSummaryReports(EnergyPlusData &state);

    bool isCompLoadRepReq(EnergyPlusData &state);

    bool hasSizingPeriodsDays(EnergyPlusData &state);

    void InitializePredefinedMonthlyTitles(EnergyPlusData &state);

    void CreatePredefinedMonthlyReports(EnergyPlusData &state);

    void GetInputFuelAndPollutionFactors(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    OTHER INITIALIZATION ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void OpenOutputTabularFile(EnergyPlusData &state);

    void CloseOutputTabularFile(EnergyPlusData &state);

    void WriteTableOfContents(EnergyPlusData &state);

    void AddTOCReportPeriod(const int nReportPeriods,
                            const std::string &kw,
                            const Array1D<Weather::ReportPeriodData> &ReportPeriodInputData,
                            std::ostream &tbl_stream);

    //======================================================================================================================
    //======================================================================================================================

    //    GATHER DATA EACH TIME STEP ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void GatherBinResultsForTimestep(EnergyPlusData &state, OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    void GatherMonthlyResultsForTimestep(EnergyPlusData &state,
                                         OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    void GatherBEPSResultsForTimestep(EnergyPlusData &state,
                                      OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    void GatherSourceEnergyEndUseResultsForTimestep(EnergyPlusData &state,
                                                    OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    void GatherPeakDemandForTimestep(EnergyPlusData &state, OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    void GatherHeatGainReport(EnergyPlusData &state, OutputProcessor::TimeStepType t_timeStepType); // What kind of data to update (Zone, HVAC)

    void GatherHeatEmissionReport(EnergyPlusData &state, OutputProcessor::TimeStepType t_timeStepType);

    void CalcHeatEmissionReport(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    WRITE OUTPUT FILE ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void WriteTabularReports(EnergyPlusData &state);

    void parseStatLine(const std::string &lineIn,
                       StatLineType &lineType,
                       bool &desConditionlinepassed,
                       bool &heatingDesignlinepassed,
                       bool &coolingDesignlinepassed,
                       bool isKoppen,
                       bool &insideLiquidPrecipitation);

    void FillWeatherPredefinedEntries(EnergyPlusData &state);

    std::string GetColumnUsingTabs(std::string const &inString, // Input String
                                   int const colNum             // Column number
    );

    void FillRemainingPredefinedEntries(EnergyPlusData &state);

    void WriteMonthlyTables(EnergyPlusData &state);

    void WriteTimeBinTables(EnergyPlusData &state);

    void WriteBEPSTable(EnergyPlusData &state);

    void writeBEPSEndUseBySubCatOrSpaceType(EnergyPlusData &state,
                                            EndUseSubTableType tableType,
                                            Array2D<Real64> &endUseSubOther,
                                            Array2D<Real64> &collapsedEndUse,
                                            Array3D<Real64> &collapsedEndUseSub,
                                            Array1D_bool &needOtherRow,
                                            const tabularReportStyle &currentStyle);

    std::string ResourceWarningMessage(std::string const &resource);

    Real64 WaterConversionFunct(Real64 WaterTotal, Real64 ConversionFactor);

    void WriteSourceEnergyEndUseSummary(EnergyPlusData &state);

    void WriteDemandEndUseSummary(EnergyPlusData &state);

    void WriteCompCostTable(EnergyPlusData &state);

    void writeRowReportPeriodInputVeri(const std::string &reportType,
                                       Array2D_string &tableBody,
                                       const int rowid,
                                       const int periodIdx,
                                       const Array1D<Weather::ReportPeriodData> &ReportPeriodInputData);

    void WriteVeriSumTable(EnergyPlusData &state);

    void writeVeriSumSpaceTables(EnergyPlusData &state, const tabularReportStyle &style);

    void WriteAdaptiveComfortTable(EnergyPlusData &state);

    std::string formatReportPeriodTimestamp(const int year, const int month, const int day, const int hour);

    void WriteReportHeaderReportingPeriod(EnergyPlusData &state,
                                          const std::string &reportKeyWord,
                                          const int periodIdx,
                                          const Array1D<Weather::ReportPeriodData> &ReportPeriodInputData);

    void WriteReportPeriodTimeConsumption(EnergyPlusData &state);

    void WriteThermalResilienceTablesRepPeriod(EnergyPlusData &state, int const periodIdx);

    void WriteThermalResilienceTables(EnergyPlusData &state);

    void WriteCO2ResilienceTables(EnergyPlusData &state);

    void WriteCO2ResilienceTablesRepPeriod(EnergyPlusData &state, const int periodIdx);

    void WriteVisualResilienceTables(EnergyPlusData &state);

    void WriteVisualResilienceTablesRepPeriod(EnergyPlusData &state, const int periodIdx);

    template <int columnNum>
    void WriteResilienceBinsTable(EnergyPlusData &state,
                                  std::array<int, columnNum> const &columnHead,
                                  const std::array<Real64, columnNum> DataHeatBalance::ZoneResilience::*memberPtr,
                                  Real64 const unitConvMultiplier = 1.0);

    template <int columnNum>
    void WriteResilienceBinsTableNonPreDefUseZoneData(EnergyPlusData &state,
                                                      const std::string &tableName,
                                                      Array1D_string const &columnHead,
                                                      Array1D_int &columnWidth,
                                                      const std::array<Real64, columnNum> DataHeatBalance::ZoneResilience::*memberPtr,
                                                      Array1D_string &rowHead,
                                                      Array2D_string &tableBody,
                                                      tabularReportStyle const &style,
                                                      Real64 const unitConvMultiplier = 1.0);

    void WriteResilienceBinsTableReportingPeriod(EnergyPlusData &state,
                                                 const std::string &tableType,
                                                 int const columnNum,
                                                 int const periodIdx,
                                                 const std::string &periodTitle,
                                                 const std::string &tableName,
                                                 Array1D_string const &columnHead,
                                                 Array1D_int &columnWidth,
                                                 Array2D<std::vector<Real64>> const &ZoneBins,
                                                 Array1D_string &rowHead,
                                                 Array2D_string &tableBody,
                                                 tabularReportStyle const &style,
                                                 Real64 const unitConvMultiplier = 1.0);

    void WriteSETHoursTableNonPreDefUseZoneData(EnergyPlusData &state,
                                                int const columnNum,
                                                const std::string &tableName,
                                                Array1D_string const &columnHead,
                                                Array1D_int &columnWidth,
                                                const std::array<Real64, 5> DataHeatBalance::ZoneResilience::*memberPtr,
                                                Array1D_string &rowHead,
                                                Array2D_string &tableBody,
                                                tabularReportStyle const &style,
                                                Real64 const unitConvMultiplier = 1.0);

    void WriteSETHoursTableReportingPeriod(EnergyPlusData &state,
                                           int const columnNum,
                                           int const periodIdx,
                                           const std::string &periodTitle,
                                           const std::string &tableName,
                                           Array1D_string const &columnHead,
                                           Array1D_int &columnWidth,
                                           Array2D<std::vector<Real64>> const &ZoneBins,
                                           Array1D_string &rowHead,
                                           Array2D_string &tableBody,
                                           tabularReportStyle const &style,
                                           Real64 const unitConvMultiplier = 1.0);

    // return the table entry of the rowIndex-th row and columnIndex-th col
    std::string RetrieveEntryFromTableBody(Array2D_string const &tableBody, int const rowIndex, int const columnIndex);

    void WriteHourOfSafetyTable(EnergyPlusData &state,
                                int const columnNum,
                                std::vector<int> const &columnHead,
                                Array1D<std::vector<Real64>> const &ZoneBins,
                                int const dateColIdx,
                                tabularReportStyle const &style);

    void WriteHourOfSafetyTableNonPreDefUseZoneData(EnergyPlusData &state,
                                                    int const columnNum,
                                                    const std::string &tableName,
                                                    Array1D_string const &columnHead,
                                                    Array1D_int &columnWidth,
                                                    const std::array<Real64, 5> DataHeatBalance::ZoneResilience::*memberPtr,
                                                    Array1D_string &rowHead,
                                                    Array2D_string &tableBody,
                                                    int const dateColIdx,
                                                    tabularReportStyle const &style);

    void WriteHourOfSafetyTableReportingPeriod(EnergyPlusData &state,
                                               int const columnNum,
                                               int const periodIdx,
                                               const std::string &periodTitle,
                                               const std::string &tableName,
                                               Array1D_string const &columnHead,
                                               Array1D_int &columnWidth,
                                               Array2D<std::vector<Real64>> const &ZoneBins,
                                               Array1D_string &rowHead,
                                               Array2D_string &tableBody,
                                               int const dateColIdx,
                                               tabularReportStyle const &style);

    void WriteHeatEmissionTable(EnergyPlusData &state);

    void WritePredefinedTables(EnergyPlusData &state);

    void WriteComponentSizing(EnergyPlusData &state);

    void WriteSurfaceShadowing(EnergyPlusData &state);

    void WriteEioTables(EnergyPlusData &state);

    int unitsFromHeading(EnergyPlusData &state, std::string &heading);

    int unitsFromHeading(EnergyPlusData &state, std::string &heading, UnitsStyle unitsStyle_para);

    std::vector<std::string> splitCommaString(std::string const &inputString);

    void AddTOCLoadComponentTableSummaries(EnergyPlusData &state);

    void AllocateLoadComponentArrays(EnergyPlusData &state);

    void DeallocateLoadComponentArrays(EnergyPlusData const &state);

    void ComputeLoadComponentDecayCurve(EnergyPlusData &state);

    void GatherComponentLoadsSurface(EnergyPlusData &state);

    void GatherComponentLoadsHVAC(EnergyPlusData &state);

    void gatherSpaceZoneCompLoadsHVAC(OutputReportTabular::compLoadsSpaceZone &compLoadDayTS,
                                      DataHeatBalance::AirReportVars const &szAirRpt,
                                      Real64 const timeStepSysSec);

    void WriteLoadComponentSummaryTables(EnergyPlusData &state);

    void computeSpaceZoneCompLoads(EnergyPlusData &state,
                                   DataSizing::ZoneSizingData const &calcFinalSizing,
                                   CompLoadTablesType &coolCompLoadTables,
                                   CompLoadTablesType &heatCompLoadTables,
                                   Array1D<Real64> &peopleDelaySeq,
                                   Array1D<Real64> &equipDelaySeq,
                                   Array1D<Real64> &hvacLossDelaySeq,
                                   Array1D<Real64> &powerGenDelaySeq,
                                   Array1D<Real64> &lightDelaySeq,
                                   Array1D<Real64> &feneSolarDelaySeq,
                                   std::vector<OutputReportTabular::componentLoadsSpZn> &szCompLoadLoc,
                                   Array2D<Real64> &surfDelaySeq,
                                   ZompComponentAreasType &componentAreas,
                                   int const iZone,
                                   int const iSpace = 0);

    void GetDelaySequences(EnergyPlusData &state,
                           int desDaySelected,
                           bool isCooling,
                           int zoneIndex,
                           Array1D<Real64> &peopleDelaySeq,
                           Array1D<Real64> &equipDelaySeq,
                           Array1D<Real64> &hvacLossDelaySeq,
                           Array1D<Real64> &powerGenDelaySeq,
                           Array1D<Real64> &lightDelaySeq,
                           Array1D<Real64> &feneSolarDelaySeq,
                           std::vector<OutputReportTabular::componentLoadsSpZn> &szCompLoadLoc,
                           Array2D<Real64> &surfDelaySeq,
                           int const iSpace = 0);

    void ComputeTableBodyUsingMovingAvg(EnergyPlusData &state,
                                        Array2D<Real64> &resultCells,
                                        Array2D_bool &resultCellsUsed,
                                        int desDaySelected,
                                        int timeOfMax,
                                        int zoneIndex,
                                        Array1D<Real64> const &peopleDelaySeq,
                                        Array1D<Real64> const &equipDelaySeq,
                                        Array1D<Real64> const &hvacLossDelaySeq,
                                        Array1D<Real64> const &powerGenDelaySeq,
                                        Array1D<Real64> const &lightDelaySeq,
                                        Array1D<Real64> const &feneSolarDelaySeq,
                                        std::vector<OutputReportTabular::componentLoadsSpZn> &szCompLoadLoc,
                                        Array2D<Real64> const &surfDelaySeq,
                                        int const iSpace = 0);

    void CollectPeakZoneConditions(
        EnergyPlusData &state, CompLoadTablesType &compLoad, int desDaySelected, int timeOfMax, int zoneIndex, bool isCooling, int spaceIndex = 0);

    void ComputeEngineeringChecks(CompLoadTablesType &compLoad);

    void GetZoneComponentAreas(EnergyPlusData &state, Array1D<ZompComponentAreasType> &znAreas, Array1D<ZompComponentAreasType> &spAreas);

    void addSurfaceArea(DataSurfaces::SurfaceData const &curSurface, Array1D<ZompComponentAreasType> &areas, bool isZone);

    void AddAreaColumnForZone(ZompComponentAreasType const &compAreas, CompLoadTablesType &compLoadTotal);

    void CombineLoadCompResults(CompLoadTablesType &compLoadTotal, CompLoadTablesType const &compLoadPartial, Real64 multiplier);

    void AddTotalRowsForLoadSummary(CompLoadTablesType &compLoadTotal);

    void ComputePeakDifference(CompLoadTablesType &compLoad);

    void LoadSummaryUnitConversion(EnergyPlusData &state, CompLoadTablesType &compLoadTotal);

    void LoadSummaryUnitConversion(EnergyPlusData &state, CompLoadTablesType &compLoadTotal, UnitsStyle unitsStyle_para);

    void CreateListOfZonesForAirLoop(EnergyPlusData const &state, CompLoadTablesType &compLoad, Array1D_int const &zoneToAirLoop, int curAirLoop);

    void OutputCompLoadSummary(EnergyPlusData &state,
                               EnergyPlus::OutputReportTabular::OutputType kind,
                               CompLoadTablesType const &compLoadCool,
                               CompLoadTablesType const &compLoadHeat,
                               int zoneOrAirLoopIndex,
                               tabularReportStyle const &style);

    void WriteReportHeaders(EnergyPlusData &state,
                            std::string const &reportName,
                            std::string const &objectName,
                            OutputProcessor::StoreType const averageOrSum);

    void WriteSubtitle(EnergyPlusData &state, std::string const &subtitle);

    void WriteTextLine(EnergyPlusData &state, std::string const &lineOfText, bool const useBold = false);

    void WriteTable(EnergyPlusData &state,
                    Array2S_string const body, // row,column
                    const Array1D_string &rowLabels,
                    const Array1D_string &columnLabels,
                    Array1D_int &widthColumn,
                    bool transposeXML = false,
                    std::string_view const footnoteText = {});

    void setTabularReportStyles(EnergyPlusData &state);

    std::string MakeAnchorName(std::string const &reportString, std::string const &objectString);

    std::string InsertCurrencySymbol(EnergyPlusData &state,
                                     std::string const &inString, // Input String
                                     bool const isHTML            // True if an HTML string
    );

    std::string ConvertToElementTag(std::string const &inString); // Input String

    std::string ConvertUnicodeToUTF8(unsigned long const codepoint);

    std::string ConvertToEscaped(std::string const &inString, // Input String
                                 bool isXML = true);          // isXML if false assumes HTML and will not convert quotes and apostrophes, for HTML4

    void DetermineBuildingFloorArea(EnergyPlusData &state);

    // Tables with Subcategories in particular have a blank for rowHead for display in the HTML output.
    // This routine will fill up the blanks for output to Sql in particular
    void FillRowHead(Array1D_string &rowHead);

    //======================================================================================================================
    //======================================================================================================================

    //    ROUTINES TO RESET GATHERED VALUES TO ZERO

    //======================================================================================================================
    //======================================================================================================================

    void ResetTabularReports(EnergyPlusData &state);

    void ResetMonthlyGathering(EnergyPlusData &state);

    void ResetBinGathering(EnergyPlusData const &state);

    void ResetBEPSGathering(EnergyPlusData const &state);

    void ResetSourceEnergyEndUseGathering(EnergyPlusData const &state);

    void ResetPeakDemandGathering(EnergyPlusData const &state);

    void ResetHeatGainGathering(EnergyPlusData &state);

    void ResetRemainingPredefinedEntries(EnergyPlusData &state);

    void ResetAdaptiveComfort(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    ROUTINES RELATED TO IF VALUE IS IN A RANGE

    //======================================================================================================================
    //======================================================================================================================

    bool isInTriangle(
        Real64 const qx, Real64 const qy, Real64 const x1, Real64 const y1, Real64 const x2, Real64 const y2, Real64 const x3, Real64 const y3);

    bool isInQuadrilateral(Real64 const qx,
                           Real64 const qy,
                           Real64 const ax,
                           Real64 const ay,
                           Real64 const bx,
                           Real64 const by,
                           Real64 const cx,
                           Real64 const cy,
                           Real64 const dx,
                           Real64 const dy);

    //======================================================================================================================
    //======================================================================================================================

    //    SUPPORT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    std::string RealToStr(bool const formatReals, Real64 const RealIn, int const numDigits);

    Real64 StrToReal(std::string_view stringIn);

    std::string DateToString(int const codedDate); // word containing encoded month, day, hour, minute

    bool isNumber(std::string const &s);

    int digitsAferDecimal(std::string const &s);

    std::string stringJoinDelimiter(const std::vector<std::string> &in_strings, const std::string &delimiter);

    void AddTOCEntry(EnergyPlusData &state, std::string const &nameSection, std::string const &nameReport);

    void SetupUnitConversions(EnergyPlusData &state);

    std::string GetUnitSubString(std::string const &inString); // Input String

    void LookupSItoIP(EnergyPlusData &state, std::string const &stringInWithSI, int &unitConvIndex, std::string &stringOutWithIP);

    void LookupJtokWH(EnergyPlusData &state, std::string const &stringInWithJ, int &unitConvIndex, std::string &stringOutWithKWH);

    Real64 ConvertIP(EnergyPlusData &state, int const unitConvIndex, Real64 const SIvalue);

    Real64 ConvertIPdelta(EnergyPlusData &state, int const unitConvIndex, Real64 const SIvalue);

    void GetUnitConversion(EnergyPlusData &state, int const unitConvIndex, Real64 &multiplier, Real64 &offset, std::string &IPunit);

    Real64 getSpecificUnitMultiplier(EnergyPlusData &state, std::string const &SIunit, std::string const &IPunit);

    Real64 getSpecificUnitDivider(EnergyPlusData &state, std::string const &SIunit, std::string const &IPunit);

    Real64 getSpecificUnitIndex(EnergyPlusData &state, std::string const &SIunit, std::string const &IPunit);

} // namespace OutputReportTabular

struct OutputReportTabularData : BaseGlobalStruct
{

    OutputReportTabular::UnitsStyle unitsStyle_Tabular = OutputReportTabular::UnitsStyle::None;
    OutputReportTabular::UnitsStyle unitsStyle_SQLite = OutputReportTabular::UnitsStyle::NotFound;
    OutputReportTabular::UnitsStyle unitsStyle_JSON = OutputReportTabular::UnitsStyle::NotFound;

    bool ip() const
    {
        return this->unitsStyle_Tabular == OutputReportTabular::UnitsStyle::InchPound ||
               this->unitsStyle_Tabular == OutputReportTabular::UnitsStyle::InchPoundExceptElectricity;
    }
    int OutputTableBinnedCount = 0;
    int BinResultsTableCount = 0;
    int BinResultsIntervalCount = 0;
    int MonthlyInputCount = 0;
    int sizeMonthlyInput = 0;
    int MonthlyFieldSetInputCount = 0;
    int sizeMonthlyFieldSetInput = 0;
    int MonthlyTablesCount = 0;
    int MonthlyColumnsCount = 0;
    Array1D_bool IsMonthGathered = Array1D_bool(12, false); // shown as true for any month used
    int TOCEntriesCount = 0;
    int TOCEntriesSize = 0;
    int UnitConvSize = 0;
    bool WriteTabularFiles = false;
    bool GetInput = true;

    // From Report:Table:Style
    int numStyles = 0;
    std::ofstream csv_stream; // CSV table stream
    std::ofstream tab_stream; // Tab table stream
    std::ofstream fix_stream; // Fixed table stream
    std::ofstream htm_stream; // HTML table stream
    std::ofstream xml_stream; // XML table stream
    Array1D<std::ofstream *> TabularOutputFile = Array1D<std::ofstream *>(
        OutputReportTabular::maxNumStyles, {&csv_stream, &tab_stream, &fix_stream, &htm_stream, &xml_stream}); // Table stream array
    Array1D_string del = Array1D_string(OutputReportTabular::maxNumStyles);                                    // the delimiter to use
    Array1D<OutputReportTabular::TableStyle> TableStyle = Array1D<OutputReportTabular::TableStyle>(
        OutputReportTabular::maxNumStyles, OutputReportTabular::TableStyle::Invalid); // see list of parameters

    Real64 timeInYear = 0.0;
    int defaultSigDigits = 2;
    bool formatReals_Tabular = true;
    bool formatReals_JSON = true;
    bool formatReals_SQLite = true;
    std::vector<OutputReportTabular::tabularReportStyle> tabularReportPasses; // unique combinations of units and real formatting

    // Flags for predefined tabular reports
    bool displayTabularBEPS = false;
    bool displayLEEDSummary = false;
    bool displayTabularCompCosts = false;
    bool displayTabularVeriSum = false;
    bool displayComponentSizing = false;
    bool displaySurfaceShadowing = false;
    bool displayDemandEndUse = false;
    bool displayAdaptiveComfort = false;
    bool displaySourceEnergyEndUseSummary = false;
    bool displayZoneComponentLoadSummary = false;
    bool displayAirLoopComponentLoadSummary = false;
    bool displayFacilityComponentLoadSummary = false;
    bool displayLifeCycleCostReport = false;
    bool displayTariffReport = false;
    bool displayEconomicResultSummary = false;
    bool displayHeatEmissionsSummary = false;
    bool displayEioSummary = false;
    bool displayThermalResilienceSummary = false;
    bool displayCO2ResilienceSummary = false;
    bool displayVisualResilienceSummary = false;
    bool displayThermalResilienceSummaryExplicitly = false;
    bool displayCO2ResilienceSummaryExplicitly = false;
    bool displayVisualResilienceSummaryExplicitly = false;
    // BEPS Report Related Variables
    // From Report:Table:Predefined - BEPS
    // arrays that hold the meter numbers that are initialized at get input

    Array1D_int meterNumTotalsBEPS = Array1D_int(OutputReportTabular::numResourceTypes, -1);
    Array1D_int meterNumTotalsSource = Array1D_int(OutputReportTabular::numSourceTypes, -1);
    Array1D_bool fuelfactorsused = Array1D_bool(OutputReportTabular::numSourceTypes, false);
    Array1D_bool ffUsed = Array1D_bool(OutputReportTabular::numResourceTypes, false);
    Array1D<Real64> SourceFactors = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
    Array1D_bool ffSchedUsed = Array1D_bool(OutputReportTabular::numResourceTypes, false);
    Array1D<Sched::Schedule *> ffScheds = Array1D<Sched::Schedule *>(OutputReportTabular::numResourceTypes, nullptr);
    Array2D_int meterNumEndUseBEPS = Array2D_int(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), -1);
    Array3D_int meterNumEndUseSubBEPS;
    Array3D_int meterNumEndUseSpTypeBEPS;
    // arrays that hold the names of the resource and end uses
    Array1D_string resourceTypeNames = Array1D_string(OutputReportTabular::numResourceTypes);
    Array1D_string sourceTypeNames = Array1D_string(OutputReportTabular::numSourceTypes);
    Array1D_string endUseNames = Array1D_string(static_cast<int>(Constant::EndUse::Num));
    // arrays that hold the actual values for the year
    Array1D<Real64> gatherTotalsBEPS = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
    Array1D<Real64> gatherTotalsBySourceBEPS = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
    Array1D<Real64> gatherTotalsSource = Array1D<Real64>(OutputReportTabular::numSourceTypes, 0.0);
    Array1D<Real64> gatherTotalsBySource = Array1D<Real64>(OutputReportTabular::numSourceTypes, 0.0);
    Array2D<Real64> gatherEndUseBEPS = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
    Array2D<Real64> gatherEndUseBySourceBEPS = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
    Array3D<Real64> gatherEndUseSubBEPS;
    Array3D<Real64> gatherEndUseSpTypeBEPS;
    Array1D_bool needOtherRowLEED45 = Array1D_bool(static_cast<int>(Constant::EndUse::Num));
    Array1D_bool needOtherRowEndUse = Array1D_bool(static_cast<int>(Constant::EndUse::Num));

    // arrays the hold the demand values
    Array1D<Real64> gatherDemandTotal = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
    Array2D<Real64> gatherDemandEndUse = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
    Array2D<Real64> gatherDemandIndEndUse = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
    Array3D<Real64> gatherDemandEndUseSub;
    Array3D<Real64> gatherDemandIndEndUseSub;
    Array1D_int gatherDemandTimeStamp = Array1D_int(OutputReportTabular::numResourceTypes, 0);

    // to keep track of hours for the BEPS report gathering
    Real64 gatherElapsedTimeBEPS = 0.0;
    // for normalization of results
    Real64 buildingGrossFloorArea = 0.0;
    Real64 buildingConditionedFloorArea = 0.0;
    // keep track if schedules are used in fuel factors
    bool fuelFactorSchedulesUsed = false;
    // for electric load components on BEPS report
    int meterNumPowerFuelFireGen = 0;
    Real64 gatherPowerFuelFireGen = 0.0;
    int meterNumPowerPV = 0;
    Real64 gatherPowerPV = 0.0;
    int meterNumPowerWind = 0;
    Real64 gatherPowerWind = 0.0;
    Real64 OverallNetEnergyFromStorage = 0.0;
    int meterNumPowerHTGeothermal = 0;
    Real64 gatherPowerHTGeothermal = 0.0;
    int meterNumElecProduced = 0;
    Real64 gatherElecProduced = 0.0;
    int meterNumElecPurchased = 0;
    Real64 gatherElecPurchased = 0.0;
    int meterNumElecSurplusSold = 0;
    Real64 gatherElecSurplusSold = 0.0;
    int meterNumElecStorage = 0;
    Real64 gatherElecStorage = 0.0;
    int meterNumPowerConversion = 0;
    Real64 gatherPowerConversion = 0.0;
    // for on site thermal source components on BEPS report
    int meterNumWaterHeatRecovery = 0;
    Real64 gatherWaterHeatRecovery = 0.0;
    int meterNumAirHeatRecoveryCool = 0;
    Real64 gatherAirHeatRecoveryCool = 0.0;
    int meterNumAirHeatRecoveryHeat = 0;
    Real64 gatherAirHeatRecoveryHeat = 0.0;
    int meterNumHeatHTGeothermal = 0;
    Real64 gatherHeatHTGeothermal = 0.0;
    int meterNumHeatSolarWater = 0;
    Real64 gatherHeatSolarWater = 0.0;
    int meterNumHeatSolarAir = 0;
    Real64 gatherHeatSolarAir = 0.0;
    // for on site water components on BEPS report
    int meterNumRainWater = 0;
    Real64 gatherRainWater = 0.0;
    int meterNumCondensate = 0;
    Real64 gatherCondensate = 0.0;
    int meterNumGroundwater = 0;
    Real64 gatherWellwater = 0.0;
    int meterNumMains = 0;
    Real64 gatherMains = 0.0;
    int meterNumWaterEndUseTotal = 0;
    Real64 gatherWaterEndUseTotal = 0.0;
    // for source energy conversion factors on BEPS report
    Real64 sourceFactorElectric = 0.0;
    Real64 sourceFactorNaturalGas = 0.0;
    Real64 efficiencyDistrictCooling = 0.0;
    Real64 efficiencyDistrictHeatingWater = 0.0;
    Real64 sourceFactorDistrictHeatingSteam = 0.0;
    Real64 sourceFactorGasoline = 0.0;
    Real64 sourceFactorDiesel = 0.0;
    Real64 sourceFactorCoal = 0.0;
    Real64 sourceFactorFuelOil1 = 0.0;
    Real64 sourceFactorFuelOil2 = 0.0;
    Real64 sourceFactorPropane = 0.0;
    Real64 sourceFactorOtherFuel1 = 0.0;
    Real64 sourceFactorOtherFuel2 = 0.0;

    Array1D_int td = Array1D_int(8);
    //(1)   Current year
    //(2)   Current month
    //(3)   Current day
    //(4)   Time difference with respect to UTC in minutes (0-59)
    //(5)   Hour of the day (0-23)
    //(6)   Minutes (0-59)
    //(7)   Seconds (0-59)
    //(8)   Milliseconds (0-999)

    // Design day name storage
    Array1D_string DesignDayName;
    int DesignDayCount = 0;

    // arrays related to pulse and load component reporting
    Array2D_int radiantPulseTimestep;
    Array2D<Real64> radiantPulseReceived;
    Array2D<Real64> decayCurveCool;
    Array2D<Real64> decayCurveHeat;

    std::vector<OutputReportTabular::componentLoadsSurf> surfCompLoads; // Surface component loads by day, timestep, then surface
    std::vector<OutputReportTabular::componentLoadsSpZn> znCompLoads;   // Zone component loads by day, timestep, then zone
    std::vector<OutputReportTabular::componentLoadsSpZn> spCompLoads;   // Space component loads by day, timestep, then space
    std::vector<OutputReportTabular::componentLoadsEncl> enclCompLoads; // Enclosure component loads by day, timestep, then enclsoure

    int maxUniqueKeyCount = 0;

    // for the XML report must keep track fo the active sub-table name and report set by other routines
    std::string activeSubTableName;
    std::string activeReportNameNoSpace;
    std::string activeReportName;
    std::string activeForName;
    std::string prevReportName;

    // Object Data
    Array1D<OutputReportTabular::OutputTableBinnedType> OutputTableBinned;
    Array2D<OutputReportTabular::BinResultsType> BinResults;      // table number, number of intervals
    Array1D<OutputReportTabular::BinResultsType> BinResultsBelow; // time below the lowest defined bin
    Array1D<OutputReportTabular::BinResultsType> BinResultsAbove; // time above the highest defined bin
    Array1D<OutputReportTabular::BinObjVarIDType> BinObjVarID;
    Array1D<OutputReportTabular::BinStatisticsType> BinStatistics;
    Array1D<OutputReportTabular::NamedMonthlyType> namedMonthly; // for predefined monthly report titles
    Array1D<OutputReportTabular::MonthlyFieldSetInputType> MonthlyFieldSetInput;
    Array1D<OutputReportTabular::MonthlyInputType> MonthlyInput;
    Array1D<OutputReportTabular::MonthlyTablesType> MonthlyTables;
    Array1D<OutputReportTabular::MonthlyColumnsType> MonthlyColumns;
    Array1D<OutputReportTabular::TOCEntriesType> TOCEntries;
    Array1D<OutputReportTabular::UnitConvType> UnitConv;

    bool GatherMonthlyResultsForTimestepRunOnce = true;
    bool UpdateTabularReportsGetInput = true;
    bool GatherHeatGainReportfirstTime = true;
    bool AllocateLoadComponentArraysDoAllocate = true;
    bool initAdjFenDone = false;
    int numPeopleAdaptive = 0;

    Real64 BigNum = 0.0;
    int ErrCount1 = 0;
    Array1D<OutputProcessor::VariableType> MonthlyColumnsTypeOfVar;
    Array1D<OutputProcessor::TimeStepType> MonthlyColumnsStepType;
    Array1D<OutputReportTabular::AggType> MonthlyColumnsAggType;
    Array1D_int MonthlyColumnsVarNum;
    Array1D_int MonthlyTablesNumColumns;
    int curFirstColumn = 0;
    int iZoneGHGR = 0;
    int iRadiantGHGR = 0;
    int iunitGHGR = 0;
    int curZoneGHGR = 0;
    Real64 eqpSensGHGR = 0.0;
    Real64 totalGHGR = 0.0; // the following arrays store the radiant total for each timestep
    Array1D<Real64> radiantHeat;
    Array1D<Real64> radiantCool;
    Array1D<Real64> ATUHeat;
    Array1D<Real64> ATUCool;
    int timestepTimeStampGHGR = 0;
    Real64 bldgHtPk = 0.0;
    Real64 bldgClPk = 0.0;
    Real64 timeStepRatio = 0.0;
    Real64 totalVolume = 0.0;
    int numUncondZones = 0;
    int numCondZones = 0;
    Real64 HrsPerWeek = 0.0; // sensible heat gain report totals
    Real64 totalZoneEqHt = 0.0;
    Real64 totalZoneEqCl = 0.0;
    Real64 totalHvacATUHt = 0.0;
    Real64 totalHvacATUCl = 0.0;
    Real64 totalSurfHt = 0.0;
    Real64 totalSurfCl = 0.0;
    Real64 totalPeoplAdd = 0.0;
    Real64 totalLiteAdd = 0.0;
    Real64 totalEquipAdd = 0.0;
    Real64 totalWindAdd = 0.0;
    Real64 totalIzaAdd = 0.0;
    Real64 totalInfilAdd = 0.0;
    Real64 totalOtherAdd = 0.0;
    Real64 totalEquipRem = 0.0;
    Real64 totalWindRem = 0.0;
    Real64 totalIzaRem = 0.0;
    Real64 totalInfilRem = 0.0;
    Real64 totalOtherRem = 0.0;
    Real64 curConversionOffset = 0.0;
    Real64 leedSiteIntLite = 0.0;
    Real64 leedSiteSpHeat = 0.0;
    Real64 leedSiteSpCool = 0.0;
    Real64 leedSiteFanInt = 0.0;
    Real64 leedSiteSrvWatr = 0.0;
    Real64 leedSiteRecept = 0.0;
    Real64 leedSiteTotal = 0.0;
    Real64 m2_unitConv = 0.0;
    int unitConvIndexWCCT = 0;
    int grandTotal = 1;
    int condTotal = 2;
    int uncondTotal = 3;
    int notpartTotal = 4;
    int unitConvIndexWVST = 0;
    Real64 m_unitConv = 0.0;
    Real64 m2_unitConvWVST = 0.0;
    Real64 m3_unitConv = 0.0;
    Real64 Wm2_unitConv = 0.0;
    Array1D<Real64> zstArea = Array1D<Real64>(4);
    Array1D<Real64> zstVolume = Array1D<Real64>(4);
    Array1D<Real64> zstWallArea = Array1D<Real64>(4);
    Array1D<Real64> zstUndWallArea = Array1D<Real64>(4);
    Array1D<Real64> zstWindowArea = Array1D<Real64>(4);
    Array1D<Real64> zstOpeningArea = Array1D<Real64>(4);
    Array1D<Real64> zstLight = Array1D<Real64>(4);
    Array1D<Real64> zstPeople = Array1D<Real64>(4);
    Array1D<Real64> zstPlug = Array1D<Real64>(4);
    int indexUnitConvWCS = 0;
    Real64 curValueSIWCS = 0.0;
    Real64 curValueWCS = 0.0;
    Real64 BigNumRMG = 0.0;
    int foundGsui = 0;
    int iUnitGsui = 0;
    int foundGsum = 0;
    int iUnitGsum = 0;
    std::string footnote;
    std::string m_unitName;
    std::string m2_unitName;
    std::string m3_unitName;
    std::string Wm2_unitName;
    std::string curColHeadWithSI;
    std::string curColHead;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->unitsStyle_Tabular = OutputReportTabular::UnitsStyle::None;
        this->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::NotFound;
        this->OutputTableBinnedCount = 0;
        this->BinResultsTableCount = 0;
        this->BinResultsIntervalCount = 0;
        this->MonthlyInputCount = 0;
        this->sizeMonthlyInput = 0;
        this->MonthlyFieldSetInputCount = 0;
        this->sizeMonthlyFieldSetInput = 0;
        this->MonthlyTablesCount = 0;
        this->MonthlyColumnsCount = 0;
        this->IsMonthGathered = Array1D_bool(12, false);
        this->TOCEntriesCount = 0;
        this->TOCEntriesSize = 0;
        this->UnitConvSize = 0;
        this->WriteTabularFiles = false;
        this->GetInput = true;
        this->numStyles = 0;
        this->TabularOutputFile = Array1D<std::ofstream *>(
            OutputReportTabular::maxNumStyles, {&this->csv_stream, &this->tab_stream, &this->fix_stream, &this->htm_stream, &this->xml_stream});
        this->del = Array1D_string(OutputReportTabular::maxNumStyles);
        this->TableStyle = Array1D<OutputReportTabular::TableStyle>(OutputReportTabular::maxNumStyles, OutputReportTabular::TableStyle::Invalid);
        this->timeInYear = 0.0;
        this->displayTabularBEPS = false;
        this->displayLEEDSummary = false;
        this->displayTabularCompCosts = false;
        this->displayTabularVeriSum = false;
        this->displayComponentSizing = false;
        this->displaySurfaceShadowing = false;
        this->displayDemandEndUse = false;
        this->displayAdaptiveComfort = false;
        this->displaySourceEnergyEndUseSummary = false;
        this->displayZoneComponentLoadSummary = false;
        this->displayAirLoopComponentLoadSummary = false;
        this->displayFacilityComponentLoadSummary = false;
        this->displayLifeCycleCostReport = false;
        this->displayTariffReport = false;
        this->displayEconomicResultSummary = false;
        this->displayHeatEmissionsSummary = false;
        this->displayEioSummary = false;
        this->displayThermalResilienceSummary = false;
        this->displayCO2ResilienceSummary = false;
        this->displayVisualResilienceSummary = false;
        this->displayThermalResilienceSummaryExplicitly = false;
        this->displayCO2ResilienceSummaryExplicitly = false;
        this->displayVisualResilienceSummaryExplicitly = false;
        this->meterNumTotalsBEPS = Array1D_int(OutputReportTabular::numResourceTypes, 0);
        this->meterNumTotalsSource = Array1D_int(OutputReportTabular::numSourceTypes, 0);
        this->fuelfactorsused = Array1D_bool(OutputReportTabular::numSourceTypes, false);
        this->ffUsed = Array1D_bool(OutputReportTabular::numResourceTypes, false);
        this->SourceFactors = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
        this->ffSchedUsed = Array1D_bool(OutputReportTabular::numResourceTypes, false);
        this->ffScheds = Array1D<Sched::Schedule *>(OutputReportTabular::numResourceTypes, nullptr);
        this->meterNumEndUseBEPS = Array2D_int(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0);
        this->meterNumEndUseSubBEPS.deallocate();
        this->meterNumEndUseSpTypeBEPS.deallocate();
        this->resourceTypeNames = Array1D_string(OutputReportTabular::numResourceTypes);
        this->sourceTypeNames = Array1D_string(OutputReportTabular::numSourceTypes);
        this->endUseNames = Array1D_string(static_cast<int>(Constant::EndUse::Num));
        this->gatherTotalsBEPS = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
        this->gatherTotalsBySourceBEPS = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
        this->gatherTotalsSource = Array1D<Real64>(OutputReportTabular::numSourceTypes, 0.0);
        this->gatherTotalsBySource = Array1D<Real64>(OutputReportTabular::numSourceTypes, 0.0);
        this->gatherEndUseBEPS = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
        this->gatherEndUseBySourceBEPS = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
        this->gatherEndUseSubBEPS.deallocate();
        this->gatherEndUseSpTypeBEPS.deallocate();
        this->needOtherRowLEED45 = Array1D_bool(static_cast<int>(Constant::EndUse::Num));
        this->needOtherRowEndUse = Array1D_bool(static_cast<int>(Constant::EndUse::Num));
        this->gatherDemandTotal = Array1D<Real64>(OutputReportTabular::numResourceTypes, 0.0);
        this->gatherDemandEndUse = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
        this->gatherDemandIndEndUse = Array2D<Real64>(OutputReportTabular::numResourceTypes, static_cast<int>(Constant::EndUse::Num), 0.0);
        this->gatherDemandEndUseSub.deallocate();
        this->gatherDemandIndEndUseSub.deallocate();
        this->gatherDemandTimeStamp = Array1D_int(OutputReportTabular::numResourceTypes, 0);
        this->gatherElapsedTimeBEPS = 0.0;
        this->buildingGrossFloorArea = 0.0;
        this->buildingConditionedFloorArea = 0.0;
        this->fuelFactorSchedulesUsed = false;
        this->meterNumPowerFuelFireGen = 0;
        this->gatherPowerFuelFireGen = 0.0;
        this->meterNumPowerPV = 0;
        this->gatherPowerPV = 0.0;
        this->meterNumPowerWind = 0;
        this->gatherPowerWind = 0.0;
        this->OverallNetEnergyFromStorage = 0.0;
        this->meterNumPowerHTGeothermal = 0;
        this->gatherPowerHTGeothermal = 0.0;
        this->meterNumElecProduced = 0;
        this->gatherElecProduced = 0.0;
        this->meterNumElecPurchased = 0;
        this->gatherElecPurchased = 0.0;
        this->meterNumElecSurplusSold = 0;
        this->gatherElecSurplusSold = 0.0;
        this->meterNumElecStorage = 0;
        this->gatherElecStorage = 0.0;
        this->meterNumPowerConversion = 0;
        this->gatherPowerConversion = 0.0;
        this->meterNumWaterHeatRecovery = 0;
        this->gatherWaterHeatRecovery = 0.0;
        this->meterNumAirHeatRecoveryCool = 0;
        this->gatherAirHeatRecoveryCool = 0.0;
        this->meterNumAirHeatRecoveryHeat = 0;
        this->gatherAirHeatRecoveryHeat = 0.0;
        this->meterNumHeatHTGeothermal = 0;
        this->gatherHeatHTGeothermal = 0.0;
        this->meterNumHeatSolarWater = 0;
        this->gatherHeatSolarWater = 0.0;
        this->meterNumHeatSolarAir = 0;
        this->gatherHeatSolarAir = 0.0;
        this->meterNumRainWater = 0;
        this->gatherRainWater = 0.0;
        this->meterNumCondensate = 0;
        this->gatherCondensate = 0.0;
        this->meterNumGroundwater = 0;
        this->gatherWellwater = 0.0;
        this->meterNumMains = 0;
        this->gatherMains = 0.0;
        this->meterNumWaterEndUseTotal = 0;
        this->gatherWaterEndUseTotal = 0.0;
        this->sourceFactorElectric = 0.0;
        this->sourceFactorNaturalGas = 0.0;
        this->efficiencyDistrictCooling = 0.0;
        this->efficiencyDistrictHeatingWater = 0.0;
        this->sourceFactorDistrictHeatingSteam = 0.0;
        this->sourceFactorGasoline = 0.0;
        this->sourceFactorDiesel = 0.0;
        this->sourceFactorCoal = 0.0;
        this->sourceFactorFuelOil1 = 0.0;
        this->sourceFactorFuelOil2 = 0.0;
        this->sourceFactorPropane = 0.0;
        this->sourceFactorOtherFuel1 = 0.0;
        this->sourceFactorOtherFuel2 = 0.0;
        this->td = Array1D_int(8);
        this->DesignDayName.deallocate();
        this->DesignDayCount = 0;
        this->radiantPulseTimestep.deallocate();
        this->radiantPulseReceived.deallocate();
        this->decayCurveCool.deallocate();
        this->decayCurveHeat.deallocate();
        this->surfCompLoads.clear();
        this->znCompLoads.clear();
        this->spCompLoads.clear();
        this->enclCompLoads.clear();

        this->maxUniqueKeyCount = 0;
        this->activeSubTableName.clear();
        this->activeReportNameNoSpace.clear();
        this->activeReportName.clear();
        this->activeForName.clear();
        this->prevReportName.clear();
        this->OutputTableBinned.deallocate();
        this->BinResults.deallocate();
        this->BinResultsBelow.deallocate();
        this->BinResultsAbove.deallocate();
        this->BinObjVarID.deallocate();
        this->BinStatistics.deallocate();
        this->namedMonthly.deallocate();
        this->MonthlyFieldSetInput.deallocate();
        this->MonthlyInput.deallocate();
        this->MonthlyTables.deallocate();
        this->MonthlyColumns.deallocate();
        this->TOCEntries.deallocate();
        this->UnitConv.deallocate();
        this->GatherMonthlyResultsForTimestepRunOnce = true;
        this->UpdateTabularReportsGetInput = true;
        this->GatherHeatGainReportfirstTime = true;
        this->AllocateLoadComponentArraysDoAllocate = true;
        this->initAdjFenDone = false;
        this->numPeopleAdaptive = 0;

        this->BigNum = 0.0;
        this->ErrCount1 = 0;
        this->MonthlyColumnsTypeOfVar.clear();
        this->MonthlyColumnsStepType.clear();
        this->MonthlyColumnsAggType.clear();
        this->MonthlyColumnsVarNum.clear();
        this->MonthlyTablesNumColumns.clear();
        this->curFirstColumn = 0;
        this->iZoneGHGR = 0;
        this->iRadiantGHGR = 0;
        this->iunitGHGR = 0;
        this->curZoneGHGR = 0;
        this->eqpSensGHGR = 0.0;
        this->totalGHGR = 0.0;
        // the following arrays store the radiant total for each timestep
        this->radiantHeat.clear();
        this->radiantCool.clear();
        this->ATUHeat.clear();
        this->ATUCool.clear();
        this->timestepTimeStampGHGR = 0;
        this->bldgHtPk = 0.0;
        this->bldgClPk = 0.0;
        this->timeStepRatio = 0.0;
        this->totalVolume = 0.0;
        this->numUncondZones = 0;
        this->numCondZones = 0;
        this->HrsPerWeek = 0.0;
        // sensible heat gain report totals
        this->totalZoneEqHt = 0.0;
        this->totalZoneEqCl = 0.0;
        this->totalHvacATUHt = 0.0;
        this->totalHvacATUCl = 0.0;
        this->totalSurfHt = 0.0;
        this->totalSurfCl = 0.0;
        this->totalPeoplAdd = 0.0;
        this->totalLiteAdd = 0.0;
        this->totalEquipAdd = 0.0;
        this->totalWindAdd = 0.0;
        this->totalIzaAdd = 0.0;
        this->totalInfilAdd = 0.0;
        this->totalOtherAdd = 0.0;
        this->totalEquipRem = 0.0;
        this->totalWindRem = 0.0;
        this->totalIzaRem = 0.0;
        this->totalInfilRem = 0.0;
        this->totalOtherRem = 0.0;
        this->curConversionOffset = 0.0;
        this->leedSiteIntLite = 0.0;
        this->leedSiteSpHeat = 0.0;
        this->leedSiteSpCool = 0.0;
        this->leedSiteFanInt = 0.0;
        this->leedSiteSrvWatr = 0.0;
        this->leedSiteRecept = 0.0;
        this->leedSiteTotal = 0.0;
        this->m2_unitConv = 0.0;
        this->unitConvIndexWCCT = 0;
        this->grandTotal = 1;
        this->condTotal = 2;
        this->uncondTotal = 3;
        this->notpartTotal = 4;
        this->unitConvIndexWVST = 0;
        this->m_unitConv = 0.0;
        this->m2_unitConvWVST = 0.0;
        this->m3_unitConv = 0.0;
        this->Wm2_unitConv = 0.0;
        this->zstArea = Array1D<Real64>(4);
        this->zstVolume = Array1D<Real64>(4);
        this->zstWallArea = Array1D<Real64>(4);
        this->zstUndWallArea = Array1D<Real64>(4);
        this->zstWindowArea = Array1D<Real64>(4);
        this->zstOpeningArea = Array1D<Real64>(4);
        this->zstLight = Array1D<Real64>(4);
        this->zstPeople = Array1D<Real64>(4);
        this->zstPlug = Array1D<Real64>(4);
        this->indexUnitConvWCS = 0;
        this->curValueSIWCS = 0.0;
        this->curValueWCS = 0.0;
        this->BigNumRMG = 0.0;
        this->foundGsui = 0;
        this->iUnitGsui = 0;
        this->foundGsum = 0;
        this->iUnitGsum = 0;
        this->footnote.clear();
        this->m_unitName.clear();
        this->m2_unitName.clear();
        this->m3_unitName.clear();
        this->Wm2_unitName.clear();
        this->curColHeadWithSI.clear();
        this->curColHead.clear();
    }
};

} // namespace EnergyPlus

#endif
