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

#ifndef SQLiteProcedures_hh_INCLUDED
#define SQLiteProcedures_hh_INCLUDED

// C++ Headers
#include <fstream>
#include <iosfwd>
#include <map>
#include <memory>
#include <sqlite3.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>

namespace EnergyPlus {

// Forward
struct EnergyPlusData;

class SQLiteProcedures
{
protected:
    SQLiteProcedures(std::shared_ptr<std::ostream> const &errorStream, std::shared_ptr<sqlite3> const &db);
    SQLiteProcedures(std::shared_ptr<std::ostream> const &errorStream,
                     bool writeOutputToSQLite,
                     fs::path const &dbName,
                     fs::path const &errorFilePath);

    int sqliteExecuteCommand(std::string_view commandBuffer);
    int sqlitePrepareStatement(sqlite3_stmt *&stmt, std::string_view stmtBuffer);

    int sqliteBindText(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, std::string_view textBuffer);
    int sqliteBindInteger(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const int intToInsert);
    int sqliteBindDouble(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const double doubleToInsert);
    int sqliteBindNULL(sqlite3_stmt *stmt, const int stmtInsertLocationIndex);
    int sqliteBindLogical(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const bool valueToInsert);

    // This assumes a Foreign Key must be greater than 0 to be a valid Foreign Key, otherwise it sets the field to NULL.
    int sqliteBindForeignKey(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const int intToInsert);

    bool sqliteStepValidity(int const rc);
    int sqliteStepCommand(sqlite3_stmt *stmt);
    int sqliteResetCommand(sqlite3_stmt *stmt);
    bool sqliteWithinTransaction();
    // int sqliteClearBindings(sqlite3_stmt * stmt);
    // int sqliteFinalizeCommand(sqlite3_stmt * stmt);

    bool m_writeOutputToSQLite;
    std::shared_ptr<std::ostream> m_errorStream;
    std::shared_ptr<sqlite3> m_db;
};

class SQLite : SQLiteProcedures
{
public:
    // Friend SQLiteFixture which is the gtest fixture class for testing SQLite
    // This allows for testing of private methods in SQLite
    friend class SQLiteFixture;

    void addScheduleData(int const number, std::string_view name, std::string_view type, double const minValue, double const maxValue);
    void addZoneData(int const number, DataHeatBalance::ZoneData const &zoneData);
    void addZoneListData(int const number, DataHeatBalance::ZoneListData const &zoneListData);
    void addSurfaceData(int const number, DataSurfaces::SurfaceData const &surfaceData, std::string_view surfaceClass);
    void addZoneGroupData(int const number, DataHeatBalance::ZoneGroupData const &zoneGroupData);
    void addMaterialData(int const number, Material::MaterialBase const *materialData);
    void addConstructionData(int const number, Construction::ConstructionProps const &constructionData, double const &constructionUValue);
    void addNominalLightingData(int const number, DataHeatBalance::LightsData const &nominalLightingData);
    void addNominalPeopleData(int const number, DataHeatBalance::PeopleData const &nominalPeopleData);
    void addNominalElectricEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalElectricEquipmentData);
    void addNominalGasEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalGasEquipmentData);
    void addNominalSteamEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalSteamEquipmentData);
    void addNominalHotWaterEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalHotWaterEquipmentData);
    void addNominalOtherEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalOtherEquipmentData);
    void addNominalBaseboardData(int const number, DataHeatBalance::BBHeatData const &nominalBaseboardData);
    void addInfiltrationData(int const number, DataHeatBalance::InfiltrationData const &infiltrationData);
    void addVentilationData(int const number, DataHeatBalance::VentilationData const &ventilationData);
    void addRoomAirModelData(int const number, RoomAir::AirModelData const &roomAirModelData);

    // Open the DB and prepare for writing data
    // Create all of the tables on construction
    SQLite(std::shared_ptr<std::ostream> errorStream,
           fs::path const &dbName,
           fs::path const &errorFilePath,
           bool writeOutputToSQLite = false,
           bool writeTabularDataToSQLite = false);

    // Close database and free prepared statements
    virtual ~SQLite();

    bool writeOutputToSQLite() const;
    bool writeTabularDataToSQLite() const;

    // Begin a transaction
    void sqliteBegin();

    // Commit a transaction
    void sqliteCommit();

    // Rollback a transaction (cancel)
    void sqliteRollback();

    // Rollback to a named savepoint (nested transaction)
    void sqliteRollbackToSavepoint(std::string_view savepoint_name);

    // Release a named savepoint (nested transaction)
    void sqliteReleaseSavepoint(std::string_view savepoint_name);

    // create a named savepoint (nested transaction)
    void sqliteCreateSavepoint(std::string_view savepoint_name);

    // Within a current transaction
    bool sqliteWithinTransaction();

    void createSQLiteReportDictionaryRecord(int const reportVariableReportID,
                                            OutputProcessor::StoreType const storeType,
                                            std::string_view indexGroup,
                                            std::string_view keyedValueString,
                                            std::string_view const variableName,
                                            OutputProcessor::TimeStepType const timeStepType,
                                            std::string_view units,
                                            OutputProcessor::ReportFreq const reportFreq,
                                            bool isMeter,
                                            std::string_view const ScheduleName = {});

    void createSQLiteReportDataRecord(int const recordIndex,
                                      Real64 const value,
                                      OutputProcessor::ReportFreq const reportFreq = OutputProcessor::ReportFreq::Invalid,
                                      Real64 const minValue = 0.0,
                                      int const minValueDate = -1,
                                      Real64 const maxValue = 0.0,
                                      int const maxValueDate = -1,
                                      int const minutesPerTimeStep = -1);

    void createSQLiteTimeIndexRecord(OutputProcessor::ReportFreq const reportFreq,
                                     int const recordIndex,
                                     int const CumlativeSimulationDays,
                                     int const curEnvirNum,
                                     int const simulationYear,
                                     bool const curYearIsLeapYear,
                                     int const Month = -1,
                                     int const DayOfMonth = -1,
                                     int const Hour = -1,
                                     Real64 const EndMinute = -1.0,
                                     Real64 const StartMinute = -1.0,
                                     int const DST = -1,
                                     std::string_view const DayType = "",
                                     bool const warmupFlag = false);

    void createYearlyTimeIndexRecord(int const simulationYear, int const curEnvirNum);

    void addSQLiteZoneSizingRecord(std::string_view ZoneName,   // the name of the zone
                                   std::string_view LoadType,   // the description of the input variable
                                   Real64 const CalcDesLoad,    // the value from the sizing calculation [W]
                                   Real64 const UserDesLoad,    // the value from the sizing calculation modified by user input [W]
                                   Real64 const CalcDesFlow,    // calculated design air flow rate [m3/s]
                                   Real64 const UserDesFlow,    // user input or modified design air flow rate [m3/s]
                                   std::string_view DesDayName, // the name of the design day that produced the peak
                                   std::string_view PeakHrMin,  // time stamp of the peak
                                   Real64 const PeakTemp,       // temperature at peak [C]
                                   Real64 const PeakHumRat,     // humidity ratio at peak [kg water/kg dry air]
                                   Real64 const MinOAVolFlow,   // zone design minimum outside air flow rate [m3/s]
                                   Real64 const DOASHeatAddRate // zone design heat addition rate from the DOAS [W]
    );

    void addSQLiteSystemSizingRecord(std::string_view SysName,      // the name of the system
                                     std::string_view LoadType,     // either "Cooling" or "Heating"
                                     std::string_view PeakLoadType, // either "Sensible" or "Total"
                                     Real64 const UserDesCap,       // User  Design Capacity
                                     Real64 const CalcDesVolFlow,   // Calculated Cooling Design Air Flow Rate
                                     Real64 const UserDesVolFlow,   // User Cooling Design Air Flow Rate
                                     std::string_view DesDayName,   // the name of the design day that produced the peak
                                     std::string_view PeakHrMin     // time stamp of the peak
    );

    void addSQLiteComponentSizingRecord(std::string_view CompType, // the type of the component
                                        std::string_view CompName, // the name of the component
                                        std::string_view VarDesc,  // the description of the input variable
                                        Real64 const VarValue      // the value from the sizing calculation
    );

    void addSQLiteComponentSizingStrRecord(std::string_view CompType, // the type of the component
                                           std::string_view CompName, // the name of the component
                                           std::string_view VarDesc,  // the description of the input variable
                                           std::string_view VarValue  // the sizing option or result
    );

    void createSQLiteDaylightMapTitle(
        int const mapNum, std::string_view mapName, std::string_view environmentName, int const zone, std::string_view refPts, Real64 const zCoord);

    void createSQLiteDaylightMap(int const mapNum,
                                 int const year,
                                 int const month,
                                 int const dayOfMonth,
                                 int const hourOfDay,
                                 int const nX,
                                 Array1D<Real64> const &x,
                                 int const nY,
                                 Array1D<Real64> const &y,
                                 Array2<Real64> const &illuminance);

    void createSQLiteTabularDataRecords(Array2D_string const &body, // row,column
                                        Array1D_string const &rowLabels,
                                        Array1D_string const &columnLabels,
                                        std::string_view ReportName,
                                        std::string_view ReportForString,
                                        std::string_view TableName);

    void createSQLiteSimulationsRecord(int const ID, std::string_view verString, std::string_view currentDateTime);

    void createSQLiteErrorRecord(int const simulationIndex, int const errorType, std::string_view errorMessage, int const cnt);

    void updateSQLiteErrorRecord(std::string const &errorMessage);

    void updateSQLiteSimulationRecord(bool const completed, bool const completedSuccessfully, int const id = 1);

    void updateSQLiteSimulationRecord(int const id, int const numOfTimeStepInHour);

    void createSQLiteEnvironmentPeriodRecord(const int curEnvirNum,
                                             std::string_view environmentName,
                                             const Constant::KindOfSim kindOfSim,
                                             const int simulationIndex = 1);

    void sqliteWriteMessage(std::string_view message);

    void createZoneExtendedOutput();

    void initializeIndexes();

private:
    int createSQLiteStringTableRecord(std::string_view stringValue, int const stringType);

    static void adjustReportingHourAndMinutes(int &hour, int &minutes);
    // Given combinedString, parse out units and description.
    // Example: Given combinedString "Total Energy [GJ]", return "Total Energy"
    // in description and "GJ" in units.
    static void parseUnitsAndDescription(std::string_view combinedString, std::string &units, std::string &description);

    static int logicalToInteger(const bool value);

    void initializeReportDataDictionaryTable();
    void initializeReportDataTables();
    void initializeTimeIndicesTable();
    void initializeZoneInfoTable();
    void initializeZoneInfoZoneListTable();
    void initializeNominalPeopleTable();
    void initializeNominalLightingTable();
    void initializeNominalElectricEquipmentTable();
    void initializeNominalGasEquipmentTable();
    void initializeNominalSteamEquipmentTable();
    void initializeNominalHotWaterEquipmentTable();
    void initializeNominalOtherEquipmentTable();
    void initializeNominalBaseboardHeatTable();
    void initializeSurfacesTable();
    void initializeConstructionsTables();
    void initializeMaterialsTable();
    void initializeZoneListTable();
    void initializeZoneGroupTable();
    void initializeNominalInfiltrationTable();
    void initializeNominalVentilationTable();
    void initializeZoneSizingTable();
    void initializeSystemSizingTable();
    void initializeComponentSizingTable();
    void initializeRoomAirModelTable();
    void initializeSchedulesTable();
    void initializeDaylightMapTables();
    void initializeViews();
    void initializeSimulationsTable();
    void initializeEnvironmentPeriodsTable();
    void initializeErrorsTable();
    void initializeTabularDataTable();
    void initializeTabularDataView();

    bool m_writeTabularDataToSQLite;
    int m_sqlDBTimeIndex;

    int m_hourlyReportIndex = 0;
    int m_hourlyDataIndex = 0;
    int m_tabularDataIndex = 0;
    int m_stringIndex = 1;
    std::map<std::pair<std::string, int>, int> m_tabularStrings;
    int m_errorIndex = 0;
    int m_dataIndex = 0;
    int m_extendedDataIndex = 0;
    int m_zoneSizingIndex = 0;
    int m_systemSizingIndex = 0;
    int m_componentSizingIndex = 0;

    sqlite3_stmt *m_reportDataInsertStmt;
    sqlite3_stmt *m_reportExtendedDataInsertStmt;
    sqlite3_stmt *m_reportDictionaryInsertStmt;
    sqlite3_stmt *m_timeIndexInsertStmt;
    sqlite3_stmt *m_zoneInfoInsertStmt;
    sqlite3_stmt *m_zoneInfoZoneListInsertStmt;
    sqlite3_stmt *m_nominalLightingInsertStmt;
    sqlite3_stmt *m_nominalElectricEquipmentInsertStmt;
    sqlite3_stmt *m_nominalGasEquipmentInsertStmt;
    sqlite3_stmt *m_nominalSteamEquipmentInsertStmt;
    sqlite3_stmt *m_nominalHotWaterEquipmentInsertStmt;
    sqlite3_stmt *m_nominalOtherEquipmentInsertStmt;
    sqlite3_stmt *m_nominalBaseboardHeatInsertStmt;
    sqlite3_stmt *m_surfaceInsertStmt;
    sqlite3_stmt *m_constructionInsertStmt;
    sqlite3_stmt *m_constructionLayerInsertStmt;
    sqlite3_stmt *m_materialInsertStmt;
    sqlite3_stmt *m_zoneListInsertStmt;
    sqlite3_stmt *m_zoneGroupInsertStmt;
    sqlite3_stmt *m_infiltrationInsertStmt;
    sqlite3_stmt *m_ventilationInsertStmt;
    sqlite3_stmt *m_nominalPeopleInsertStmt;
    sqlite3_stmt *m_zoneSizingInsertStmt;
    sqlite3_stmt *m_systemSizingInsertStmt;
    sqlite3_stmt *m_componentSizingInsertStmt;
    sqlite3_stmt *m_roomAirModelInsertStmt;
    sqlite3_stmt *m_groundTemperatureInsertStmt;
    sqlite3_stmt *m_weatherFileInsertStmt;
    sqlite3_stmt *m_scheduleInsertStmt;
    sqlite3_stmt *m_daylightMapTitleInsertStmt;
    sqlite3_stmt *m_daylightMapHourlyTitleInsertStmt;
    sqlite3_stmt *m_daylightMapHourlyDataInsertStmt;
    sqlite3_stmt *m_environmentPeriodInsertStmt;
    sqlite3_stmt *m_simulationsInsertStmt;
    sqlite3_stmt *m_tabularDataInsertStmt;
    sqlite3_stmt *m_stringsInsertStmt;
    sqlite3_stmt *m_stringsLookUpStmt;
    sqlite3_stmt *m_errorInsertStmt;
    sqlite3_stmt *m_errorUpdateStmt;
    sqlite3_stmt *m_simulationUpdateStmt;
    sqlite3_stmt *m_simulationDataUpdateStmt;
    sqlite3_stmt *m_rollbackToSavepointStmt;
    sqlite3_stmt *m_createSavepointStmt;
    sqlite3_stmt *m_releaseSavepointStmt;

    static const int LocalReportEach;     //  Write out each time UpdatedataandLocalReport is called
    static const int LocalReportTimeStep; //  Write out at 'EndTimeStepFlag'
    static const int LocalReportHourly;   //  Write out at 'EndHourFlag'
    static const int LocalReportDaily;    //  Write out at 'EndDayFlag'
    static const int LocalReportMonthly;  //  Write out at end of month (must be determined)
    static const int LocalReportSim;      //  Write out once per environment 'EndEnvrnFlag'
    static const int LocalReportYearly;   //  Write out once per year
    static const int ReportNameId;
    static const int ReportForStringId;
    static const int TableNameId;
    static const int RowNameId;
    static const int ColumnNameId;
    static const int UnitsId;

    class SQLiteData : public SQLiteProcedures
    {
    protected:
        SQLiteData(std::shared_ptr<std::ostream> const &errorStream, std::shared_ptr<sqlite3> const &db);
        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) = 0;
    };

    class Schedule : SQLiteData
    {
    public:
        Schedule(std::shared_ptr<std::ostream> const &errorStream,
                 std::shared_ptr<sqlite3> const &db,
                 int const scheduleNumber,
                 std::string_view scheduleName,
                 std::string_view scheduleType,
                 double const scheduleMinValue,
                 double const scheduleMaxValue)
            : SQLiteData(errorStream, db), number(scheduleNumber), name(scheduleName), type(scheduleType), minValue(scheduleMinValue),
              maxValue(scheduleMaxValue)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const name;
        std::string const type;
        double const minValue;
        double const maxValue;
    };

    class Surface : SQLiteData
    {
    public:
        Surface(std::shared_ptr<std::ostream> const &errorStream,
                std::shared_ptr<sqlite3> const &db,
                int const surfaceNumber,
                DataSurfaces::SurfaceData const &surfaceData,
                std::string_view surfaceClass)
            : SQLiteData(errorStream, db), number(surfaceNumber), name(surfaceData.Name), construction(surfaceData.Construction),
              surfaceClass(surfaceClass), area(surfaceData.Area), grossArea(surfaceData.GrossArea), perimeter(surfaceData.Perimeter),
              azimuth(surfaceData.Azimuth), height(surfaceData.Height), reveal(surfaceData.Reveal), shape(surfaceData.Shape),
              sides(surfaceData.Sides), tilt(surfaceData.Tilt), width(surfaceData.Width), heatTransSurf(surfaceData.HeatTransSurf),
              baseSurf(surfaceData.BaseSurf), zone(surfaceData.Zone), extBoundCond(surfaceData.ExtBoundCond), extSolar(surfaceData.ExtSolar),
              extWind(surfaceData.ExtWind)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &construction;
        std::string const surfaceClass;
        double const &area;
        double const &grossArea;
        double const &perimeter;
        double const &azimuth;
        double const &height;
        double const &reveal;
        DataSurfaces::SurfaceShape const &shape;
        int const &sides;
        double const &tilt;
        double const &width;
        bool const &heatTransSurf;
        int const &baseSurf;
        int const &zone;
        int const &extBoundCond;
        bool const &extSolar;
        bool const &extWind;
    };

    class Zone : SQLiteData
    {
    public:
        Zone(std::shared_ptr<std::ostream> const &errorStream,
             std::shared_ptr<sqlite3> const &db,
             int const zoneNumber,
             DataHeatBalance::ZoneData const &zoneData)
            : SQLiteData(errorStream, db), number(zoneNumber), name(zoneData.Name), relNorth(zoneData.RelNorth), originX(zoneData.OriginX),
              originY(zoneData.OriginY), originZ(zoneData.OriginZ), centroidX(zoneData.Centroid.x), centroidY(zoneData.Centroid.y),
              centroidZ(zoneData.Centroid.z), ofType(zoneData.OfType), multiplier(zoneData.Multiplier), listMultiplier(zoneData.ListMultiplier),
              minimumX(zoneData.MinimumX), maximumX(zoneData.MaximumX), minimumY(zoneData.MinimumY), maximumY(zoneData.MaximumY),
              minimumZ(zoneData.MinimumZ), maximumZ(zoneData.MaximumZ), ceilingHeight(zoneData.CeilingHeight), volume(zoneData.Volume),
              insideConvectionAlgo(zoneData.IntConvAlgo), outsideConvectionAlgo(zoneData.ExtConvAlgo), floorArea(zoneData.FloorArea),
              extGrossWallArea(zoneData.ExtGrossWallArea), extNetWallArea(zoneData.ExtNetWallArea), extWindowArea(zoneData.ExtWindowArea),
              isPartOfTotalArea(zoneData.isPartOfTotalArea)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        double const &relNorth;
        double const &originX;
        double const &originY;
        double const &originZ;
        double const &centroidX;
        double const &centroidY;
        double const &centroidZ;
        int const &ofType;
        int const &multiplier;
        int const &listMultiplier;
        double const &minimumX;
        double const &maximumX;
        double const &minimumY;
        double const &maximumY;
        double const &minimumZ;
        double const &maximumZ;
        double const &ceilingHeight;
        double const &volume;
        Convect::HcInt const &insideConvectionAlgo;
        Convect::HcExt const &outsideConvectionAlgo;
        double const &floorArea;
        double const &extGrossWallArea;
        double const &extNetWallArea;
        double const &extWindowArea;
        bool const &isPartOfTotalArea;
    };

    class ZoneList : SQLiteData
    {
    public:
        ZoneList(std::shared_ptr<std::ostream> const &errorStream,
                 std::shared_ptr<sqlite3> const &db,
                 int const zoneListNumber,
                 DataHeatBalance::ZoneListData const &zoneListData)
            : SQLiteData(errorStream, db), number(zoneListNumber), name(zoneListData.Name), zones(zoneListData.Zone)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;
        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt, sqlite3_stmt *subInsertStmt);

    private:
        int const number;
        std::string const &name;
        Array1D_int const &zones;
    };

    class ZoneGroup : SQLiteData
    {
    public:
        ZoneGroup(std::shared_ptr<std::ostream> const &errorStream,
                  std::shared_ptr<sqlite3> const &db,
                  int const zoneGroupNumber,
                  DataHeatBalance::ZoneGroupData const &zoneGroupData)
            : SQLiteData(errorStream, db), number(zoneGroupNumber), name(zoneGroupData.Name), zoneList(zoneGroupData.ZoneList),
              multiplier(zoneGroupData.Multiplier)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zoneList;
        int const &multiplier;
    };

    class Material : SQLiteData
    {
    public:
        Material(std::shared_ptr<std::ostream> const &errorStream,
                 std::shared_ptr<sqlite3> const &db,
                 int const materialNumber,
                 EnergyPlus::Material::MaterialBase const *materialData)
            : SQLiteData(errorStream, db), number(materialNumber), name(materialData->Name), group(materialData->group),
              roughness(materialData->Roughness), conductivity(materialData->Conductivity), density(materialData->Density),
              // isoMoistCap(materialData->IsoMoistCap),
              porosity(materialData->Porosity), resistance(materialData->Resistance), rOnly(materialData->ROnly), specHeat(materialData->SpecHeat),
              // thermGradCoef(materialData->ThermGradCoef),
              thickness(materialData->Thickness), vaporDiffus(materialData->VaporDiffus)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        EnergyPlus::Material::Group const &group;
        EnergyPlus::Material::SurfaceRoughness const &roughness;
        double const &conductivity;
        double const &density;
        // double const &isoMoistCap;
        double const &porosity;
        double const &resistance;
        bool const &rOnly;
        double const &specHeat;
        // double const &thermGradCoef;
        double const &thickness;
        double const &vaporDiffus;
    };

    class Construction : SQLiteData
    {
    public:
        Construction(std::shared_ptr<std::ostream> const &errorStream,
                     std::shared_ptr<sqlite3> const &db,
                     int const constructionNumber,
                     EnergyPlus::Construction::ConstructionProps const &constructionData,
                     double const &constructionUValue)
            : SQLiteData(errorStream, db), number(constructionNumber), name(constructionData.Name), totLayers(constructionData.TotLayers),
              totSolidLayers(constructionData.TotSolidLayers), totGlassLayers(constructionData.TotGlassLayers),
              insideAbsorpVis(constructionData.InsideAbsorpVis), outsideAbsorpVis(constructionData.OutsideAbsorpVis),
              insideAbsorpSolar(constructionData.InsideAbsorpSolar), outsideAbsorpSolar(constructionData.OutsideAbsorpSolar),
              insideAbsorpThermal(constructionData.InsideAbsorpThermal), outsideAbsorpThermal(constructionData.OutsideAbsorpThermal),
              outsideRoughness(constructionData.OutsideRoughness), typeIsWindow(constructionData.TypeIsWindow), uValue(constructionUValue)
        {
            for (int layerNum = 1; layerNum <= constructionData.TotLayers; ++layerNum) {
                constructionLayers.push_back(std::unique_ptr<Construction::ConstructionLayer>(
                    new ConstructionLayer(m_errorStream, m_db, number, layerNum, constructionData.LayerPoint(layerNum))));
            }
        }

        // only inserts construction
        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;
        // inserts construction and construction layers
        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt, sqlite3_stmt *subInsertStmt);

    private:
        int const number;
        std::string const &name;
        int const &totLayers;
        int const &totSolidLayers;
        int const &totGlassLayers;
        double const &insideAbsorpVis;
        double const &outsideAbsorpVis;
        double const &insideAbsorpSolar;
        double const &outsideAbsorpSolar;
        double const &insideAbsorpThermal;
        double const &outsideAbsorpThermal;
        EnergyPlus::Material::SurfaceRoughness const &outsideRoughness;
        bool const &typeIsWindow;
        double const &uValue;

        class ConstructionLayer : SQLiteData
        {
        public:
            ConstructionLayer(std::shared_ptr<std::ostream> const &errorStream,
                              std::shared_ptr<sqlite3> const &db,
                              int const &constructNumber,
                              int const layerNumber,
                              int const &layerPoint)
                : SQLiteData(errorStream, db), constructNumber(constructNumber), layerNumber(layerNumber), layerPoint(layerPoint)
            {
            }

            virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

        private:
            int const &constructNumber;
            int const layerNumber;
            int const &layerPoint;
        };

        std::vector<std::unique_ptr<Construction::ConstructionLayer>> constructionLayers;
    };

    class NominalLighting : SQLiteData
    {
    public:
        NominalLighting(std::shared_ptr<std::ostream> const &errorStream,
                        std::shared_ptr<sqlite3> const &db,
                        int const nominalLightingNumber,
                        DataHeatBalance::LightsData const &nominalLightingData)
            : SQLiteData(errorStream, db), number(nominalLightingNumber), name(nominalLightingData.Name), zonePtr(nominalLightingData.ZonePtr),
              sched(nominalLightingData.sched), designLevel(nominalLightingData.DesignLevel),
              fractionReturnAir(nominalLightingData.FractionReturnAir), fractionRadiant(nominalLightingData.FractionRadiant),
              fractionShortWave(nominalLightingData.FractionShortWave), fractionReplaceable(nominalLightingData.FractionReplaceable),
              fractionConvected(nominalLightingData.FractionConvected), endUseSubcategory(nominalLightingData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
        double const &fractionReturnAir;
        double const &fractionRadiant;
        double const &fractionShortWave;
        double const &fractionReplaceable;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class NominalPeople : SQLiteData
    {
    public:
        NominalPeople(std::shared_ptr<std::ostream> const &errorStream,
                      std::shared_ptr<sqlite3> const &db,
                      int const nominalPeopleNumber,
                      DataHeatBalance::PeopleData const &nominalPeopleData)
            : SQLiteData(errorStream, db), number(nominalPeopleNumber), name(nominalPeopleData.Name), zonePtr(nominalPeopleData.ZonePtr),
              numberOfPeople(nominalPeopleData.NumberOfPeople), numberOfPeopleSched(nominalPeopleData.sched),
              activityLevelSched(nominalPeopleData.activityLevelSched), fractionRadiant(nominalPeopleData.FractionRadiant),
              fractionConvected(nominalPeopleData.FractionConvected), workEffSched(nominalPeopleData.workEffSched),
              clothingSched(nominalPeopleData.clothingSched), airVelocitySched(nominalPeopleData.airVelocitySched), fanger(nominalPeopleData.Fanger),
              pierce(nominalPeopleData.Pierce), ksu(nominalPeopleData.KSU), mrtCalcType(nominalPeopleData.MRTCalcType),
              surfacePtr(nominalPeopleData.SurfacePtr), angleFactorListName(nominalPeopleData.AngleFactorListName),
              angleFactorListPtr(nominalPeopleData.AngleFactorListPtr), userSpecSensFrac(nominalPeopleData.UserSpecSensFrac),
              show55Warning(nominalPeopleData.Show55Warning)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        double const &numberOfPeople;
        Sched::Schedule const *numberOfPeopleSched;
        Sched::Schedule const *activityLevelSched;
        double const &fractionRadiant;
        double const &fractionConvected;
        Sched::Schedule const *workEffSched;
        Sched::Schedule const *clothingSched;
        Sched::Schedule const *airVelocitySched;
        bool const &fanger;
        bool const &pierce;
        bool const &ksu;
        DataHeatBalance::CalcMRT const &mrtCalcType;
        int const &surfacePtr;
        std::string const &angleFactorListName;
        int const &angleFactorListPtr;
        double const &userSpecSensFrac;
        bool const &show55Warning;
    };

    class NominalElectricEquipment : SQLiteData
    {
    public:
        NominalElectricEquipment(std::shared_ptr<std::ostream> const &errorStream,
                                 std::shared_ptr<sqlite3> const &db,
                                 int const nominalElectricEquipmentNumber,
                                 DataHeatBalance::ZoneEquipData const &nominalElectricEquipmentData)
            : SQLiteData(errorStream, db), number(nominalElectricEquipmentNumber), name(nominalElectricEquipmentData.Name),
              zonePtr(nominalElectricEquipmentData.ZonePtr), sched(nominalElectricEquipmentData.sched),
              designLevel(nominalElectricEquipmentData.DesignLevel), fractionLatent(nominalElectricEquipmentData.FractionLatent),
              fractionRadiant(nominalElectricEquipmentData.FractionRadiant), fractionLost(nominalElectricEquipmentData.FractionLost),
              fractionConvected(nominalElectricEquipmentData.FractionConvected), endUseSubcategory(nominalElectricEquipmentData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
        double const &fractionLatent;
        double const &fractionRadiant;
        double const &fractionLost;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class NominalGasEquipment : SQLiteData
    {
    public:
        NominalGasEquipment(std::shared_ptr<std::ostream> const &errorStream,
                            std::shared_ptr<sqlite3> const &db,
                            int const nominalGasEquipmentNumber,
                            DataHeatBalance::ZoneEquipData const &nominalGasEquipmentData)
            : SQLiteData(errorStream, db), number(nominalGasEquipmentNumber), name(nominalGasEquipmentData.Name),
              zonePtr(nominalGasEquipmentData.ZonePtr), sched(nominalGasEquipmentData.sched), designLevel(nominalGasEquipmentData.DesignLevel),
              fractionLatent(nominalGasEquipmentData.FractionLatent), fractionRadiant(nominalGasEquipmentData.FractionRadiant),
              fractionLost(nominalGasEquipmentData.FractionLost), fractionConvected(nominalGasEquipmentData.FractionConvected),
              endUseSubcategory(nominalGasEquipmentData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
        double const &fractionLatent;
        double const &fractionRadiant;
        double const &fractionLost;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class NominalSteamEquipment : SQLiteData
    {
    public:
        NominalSteamEquipment(std::shared_ptr<std::ostream> const &errorStream,
                              std::shared_ptr<sqlite3> const &db,
                              int const nominalSteamEquipmentNumber,
                              DataHeatBalance::ZoneEquipData const &nominalSteamEquipmentData)
            : SQLiteData(errorStream, db), number(nominalSteamEquipmentNumber), name(nominalSteamEquipmentData.Name),
              zonePtr(nominalSteamEquipmentData.ZonePtr), sched(nominalSteamEquipmentData.sched), designLevel(nominalSteamEquipmentData.DesignLevel),
              fractionLatent(nominalSteamEquipmentData.FractionLatent), fractionRadiant(nominalSteamEquipmentData.FractionRadiant),
              fractionLost(nominalSteamEquipmentData.FractionLost), fractionConvected(nominalSteamEquipmentData.FractionConvected),
              endUseSubcategory(nominalSteamEquipmentData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
        double const &fractionLatent;
        double const &fractionRadiant;
        double const &fractionLost;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class NominalHotWaterEquipment : SQLiteData
    {
    public:
        NominalHotWaterEquipment(std::shared_ptr<std::ostream> const &errorStream,
                                 std::shared_ptr<sqlite3> const &db,
                                 int const nominalHotWaterEquipmentNumber,
                                 DataHeatBalance::ZoneEquipData const &nominalHotWaterEquipmentData)
            : SQLiteData(errorStream, db), number(nominalHotWaterEquipmentNumber), name(nominalHotWaterEquipmentData.Name),
              zonePtr(nominalHotWaterEquipmentData.ZonePtr), sched(nominalHotWaterEquipmentData.sched),
              designLevel(nominalHotWaterEquipmentData.DesignLevel), fractionLatent(nominalHotWaterEquipmentData.FractionLatent),
              fractionRadiant(nominalHotWaterEquipmentData.FractionRadiant), fractionLost(nominalHotWaterEquipmentData.FractionLost),
              fractionConvected(nominalHotWaterEquipmentData.FractionConvected), endUseSubcategory(nominalHotWaterEquipmentData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
        double const &fractionLatent;
        double const &fractionRadiant;
        double const &fractionLost;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class NominalOtherEquipment : SQLiteData
    {
    public:
        NominalOtherEquipment(std::shared_ptr<std::ostream> const &errorStream,
                              std::shared_ptr<sqlite3> const &db,
                              int const nominalOtherEquipmentNumber,
                              DataHeatBalance::ZoneEquipData const &nominalOtherEquipmentData)
            : SQLiteData(errorStream, db), number(nominalOtherEquipmentNumber), name(nominalOtherEquipmentData.Name),
              zonePtr(nominalOtherEquipmentData.ZonePtr), sched(nominalOtherEquipmentData.sched), designLevel(nominalOtherEquipmentData.DesignLevel),
              fractionLatent(nominalOtherEquipmentData.FractionLatent), fractionRadiant(nominalOtherEquipmentData.FractionRadiant),
              fractionLost(nominalOtherEquipmentData.FractionLost), fractionConvected(nominalOtherEquipmentData.FractionConvected),
              endUseSubcategory(nominalOtherEquipmentData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
        double const &fractionLatent;
        double const &fractionRadiant;
        double const &fractionLost;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class NominalBaseboardHeat : SQLiteData
    {
    public:
        NominalBaseboardHeat(std::shared_ptr<std::ostream> const &errorStream,
                             std::shared_ptr<sqlite3> const &db,
                             int const nominalBaseboardHeatNumber,
                             DataHeatBalance::BBHeatData const &nominalBaseboardHeatData)
            : SQLiteData(errorStream, db), number(nominalBaseboardHeatNumber), name(nominalBaseboardHeatData.Name),
              zonePtr(nominalBaseboardHeatData.ZonePtr), sched(nominalBaseboardHeatData.sched),
              capatLowTemperature(nominalBaseboardHeatData.CapatLowTemperature), lowTemperature(nominalBaseboardHeatData.LowTemperature),
              capatHighTemperature(nominalBaseboardHeatData.CapatHighTemperature), highTemperature(nominalBaseboardHeatData.HighTemperature),
              fractionRadiant(nominalBaseboardHeatData.FractionRadiant), fractionConvected(nominalBaseboardHeatData.FractionConvected),
              endUseSubcategory(nominalBaseboardHeatData.EndUseSubcategory)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &capatLowTemperature;
        double const &lowTemperature;
        double const &capatHighTemperature;
        double const &highTemperature;
        double const &fractionRadiant;
        double const &fractionConvected;
        std::string const &endUseSubcategory;
    };

    class Infiltration : SQLiteData
    {
    public:
        Infiltration(std::shared_ptr<std::ostream> const &errorStream,
                     std::shared_ptr<sqlite3> const &db,
                     int const infiltrationNumber,
                     DataHeatBalance::InfiltrationData const &infiltrationData)
            : SQLiteData(errorStream, db), number(infiltrationNumber), name(infiltrationData.Name), zonePtr(infiltrationData.ZonePtr),
              sched(infiltrationData.sched), designLevel(infiltrationData.DesignLevel)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
    };

    class Ventilation : SQLiteData
    {
    public:
        Ventilation(std::shared_ptr<std::ostream> const &errorStream,
                    std::shared_ptr<sqlite3> const &db,
                    int const ventilationNumber,
                    DataHeatBalance::VentilationData const &ventilationData)
            : SQLiteData(errorStream, db), number(ventilationNumber), name(ventilationData.Name), zonePtr(ventilationData.ZonePtr),
              sched(ventilationData.availSched), designLevel(ventilationData.DesignLevel)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &name;
        int const &zonePtr;
        Sched::Schedule const *sched;
        double const &designLevel;
    };

    class RoomAirModel : SQLiteData
    {
    public:
        RoomAirModel(std::shared_ptr<std::ostream> const &errorStream,
                     std::shared_ptr<sqlite3> const &db,
                     int const roomAirModelNumber,
                     RoomAir::AirModelData const &roomAirModelData)
            : SQLiteData(errorStream, db), number(roomAirModelNumber), airModelName(roomAirModelData.Name), airModel(roomAirModelData.AirModel),
              tempCoupleScheme(roomAirModelData.TempCoupleScheme), simAirModel(roomAirModelData.SimAirModel)
        {
        }

        virtual bool insertIntoSQLite(sqlite3_stmt *insertStmt) override;

    private:
        int const number;
        std::string const &airModelName;
        RoomAir::RoomAirModel const &airModel;
        RoomAir::CouplingScheme const &tempCoupleScheme;
        bool const &simAirModel;
    };

    std::vector<std::unique_ptr<SQLite::Zone>> zones;
    std::vector<std::unique_ptr<SQLite::ZoneList>> zoneLists;
    std::vector<std::unique_ptr<SQLite::ZoneGroup>> zoneGroups;
    std::vector<std::unique_ptr<SQLite::Schedule>> schedules;
    std::vector<std::unique_ptr<SQLite::Surface>> surfaces;
    std::vector<std::unique_ptr<SQLite::Material>> materials;
    std::vector<std::unique_ptr<SQLite::Construction>> constructions;
    std::vector<std::unique_ptr<SQLite::NominalLighting>> nominalLightings;
    std::vector<std::unique_ptr<SQLite::NominalPeople>> nominalPeoples;
    std::vector<std::unique_ptr<SQLite::NominalElectricEquipment>> nominalElectricEquipments;
    std::vector<std::unique_ptr<SQLite::NominalGasEquipment>> nominalGasEquipments;
    std::vector<std::unique_ptr<SQLite::NominalSteamEquipment>> nominalSteamEquipments;
    std::vector<std::unique_ptr<SQLite::NominalHotWaterEquipment>> nominalHotWaterEquipments;
    std::vector<std::unique_ptr<SQLite::NominalOtherEquipment>> nominalOtherEquipments;
    std::vector<std::unique_ptr<SQLite::NominalBaseboardHeat>> nominalBaseboardHeats;
    std::vector<std::unique_ptr<SQLite::Infiltration>> infiltrations;
    std::vector<std::unique_ptr<SQLite::Ventilation>> ventilations;
    std::vector<std::unique_ptr<SQLite::RoomAirModel>> roomAirModels;
};

bool ParseSQLiteInput(EnergyPlusData &state, bool &writeOutputToSQLite, bool &writeTabularDataToSQLite);

std::unique_ptr<SQLite> CreateSQLiteDatabase(EnergyPlusData &state);

void CreateSQLiteZoneExtendedOutput(EnergyPlusData &state);

struct SQLiteProceduresData : BaseGlobalStruct
{
    std::unique_ptr<SQLite> sqlite;
    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        sqlite.reset(); // probably not necessary, as it is recreated in ManageSimulation, but it should be fine to delete it here
    }
};

} // namespace EnergyPlus

#endif
