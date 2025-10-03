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

// C++ headers
#include <ios>
#include <memory>
#include <sstream>
#include <stdexcept>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionConstants.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

constexpr std::array<int, (int)OutputProcessor::ReportFreq::Num> reportFreqInts = {
    -1, // EachCall
    0,  // TimeStep
    1,  // Hour
    2,  // Day
    3,  // Month
    4,  // Simulation
    5   // Year
};

const int SQLite::ReportNameId = 1;
const int SQLite::ReportForStringId = 2;
const int SQLite::TableNameId = 3;
const int SQLite::RowNameId = 4;
const int SQLite::ColumnNameId = 5;
const int SQLite::UnitsId = 6;

bool ParseSQLiteInput(EnergyPlusData &state, bool &writeOutputToSQLite, bool &writeTabularDataToSQLite)
{
    auto &ip = state.dataInputProcessing->inputProcessor;
    auto const instances = ip->epJSON.find("Output:SQLite");
    if (instances != ip->epJSON.end()) {

        auto find_input = [=, &state](nlohmann::json const &fields, std::string const &field_name) -> std::string {
            std::string input;
            auto found = fields.find(field_name);
            if (found != fields.end()) {
                input = found.value().get<std::string>();
            } else {
                state.dataInputProcessing->inputProcessor->getDefaultValue(state, "Output:SQLite", field_name, input);
            }
            return input;
        };

        // There can only be 1 "Output:SQLite"
        auto const instance = instances.value().begin();
        auto const &fields = instance.value();
        ip->markObjectAsUsed("Output:SQLite", instance.key());

        { // "option_type"
            std::string outputType = find_input(fields, "option_type");
            if ("SimpleAndTabular" == outputType) {
                writeTabularDataToSQLite = true;
                writeOutputToSQLite = true;
            } else if ("Simple" == outputType) {
                writeTabularDataToSQLite = false;
                writeOutputToSQLite = true;
            }
        }
        { // "unit_conversion_for_tabular_data"
            std::string tabularDataUnitConversion = find_input(fields, "unit_conversion_for_tabular_data");
            auto const &sql_ort = state.dataOutRptTab;

            if ("UseOutputControlTableStyles" == tabularDataUnitConversion) {
                // Jan 2021 Note: Since here we do not know weather sql_ort->unitsStyle has been processed or not,
                // the value "NotFound" is used for the option "UseOutputControlTableStyles" at this point;
                // This will be updated again and got concretely assigned first thing in OutputReportTabular::WriteTabularReports().
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::NotFound;
            } else if ("None" == tabularDataUnitConversion) {
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::None;
            } else if ("JtoKWH" == tabularDataUnitConversion) {
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoKWH;
            } else if ("JtoMJ" == tabularDataUnitConversion) {
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoMJ;
            } else if ("JtoGJ" == tabularDataUnitConversion) {
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::JtoGJ;
            } else if ("InchPound" == tabularDataUnitConversion) {
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPound;
            } else if ("InchPoundExceptElectricity" == tabularDataUnitConversion) {
                sql_ort->unitsStyle_SQLite = OutputReportTabular::UnitsStyle::InchPoundExceptElectricity;
            }
        }
        return true;
    }
    return false;
}

std::unique_ptr<SQLite> CreateSQLiteDatabase(EnergyPlusData &state)
{
    if (!state.files.outputControl.sqlite) {
        return nullptr;
    }
    try {
        bool writeOutputToSQLite = false;
        bool writeTabularDataToSQLite = false;
        bool parsedSQLite = ParseSQLiteInput(state, writeOutputToSQLite, writeTabularDataToSQLite);
        if (!parsedSQLite) {
            state.files.outputControl.sqlite = false;
            return nullptr;
        }
        auto errorStream = std::make_shared<std::ofstream>(state.dataStrGlobals->outputSqliteErrFilePath, std::ofstream::out | std::ofstream::trunc);
        return std::make_unique<SQLite>(errorStream,
                                        state.dataStrGlobals->outputSqlFilePath,
                                        state.dataStrGlobals->outputSqliteErrFilePath,
                                        writeOutputToSQLite,
                                        writeTabularDataToSQLite);
    } catch (const std::runtime_error &error) {
        ShowFatalError(state, error.what());
        return nullptr;
    }
}

void CreateSQLiteZoneExtendedOutput(EnergyPlusData &state)
{
    if (state.dataSQLiteProcedures->sqlite && state.dataSQLiteProcedures->sqlite->writeOutputToSQLite()) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            state.dataSQLiteProcedures->sqlite->addZoneData(zoneNum, state.dataHeatBal->Zone(zoneNum));
        }
        for (int listNum = 1; listNum <= state.dataHeatBal->NumOfZoneLists; ++listNum) {
            state.dataSQLiteProcedures->sqlite->addZoneListData(listNum, state.dataHeatBal->ZoneList(listNum));
        }
        for (int groupNum = 1; groupNum <= state.dataHeatBal->NumOfZoneGroups; ++groupNum) {
            state.dataSQLiteProcedures->sqlite->addZoneGroupData(groupNum, state.dataHeatBal->ZoneGroup(groupNum));
        }
        for (auto *sched : state.dataSched->schedules) {
            state.dataSQLiteProcedures->sqlite->addScheduleData(
                sched->Num,
                sched->Name,
                (sched->schedTypeNum == -1) ? "" : state.dataSched->scheduleTypes[sched->schedTypeNum]->Name,
                sched->getMinVal(state),
                sched->getMaxVal(state));
        }
        for (int surfaceNumber = 1; surfaceNumber <= state.dataSurface->TotSurfaces; ++surfaceNumber) {
            auto const &surface = state.dataSurface->Surface(surfaceNumber);
            state.dataSQLiteProcedures->sqlite->addSurfaceData(surfaceNumber, surface, DataSurfaces::cSurfaceClass(surface.Class));
        }
        for (int materialNum = 1; materialNum <= state.dataMaterial->materials.isize(); ++materialNum) {
            state.dataSQLiteProcedures->sqlite->addMaterialData(materialNum, state.dataMaterial->materials(materialNum));
        }
        for (int constructNum = 1; constructNum <= state.dataHeatBal->TotConstructs; ++constructNum) {
            auto const &construction = state.dataConstruction->Construct(constructNum);
            if (construction.TotGlassLayers == 0) {
                state.dataSQLiteProcedures->sqlite->addConstructionData(constructNum, construction, construction.UValue);
            } else {
                state.dataSQLiteProcedures->sqlite->addConstructionData(constructNum, construction, state.dataHeatBal->NominalU(constructNum));
            }
        }
        for (int lightNum = 1; lightNum <= state.dataHeatBal->TotLights; ++lightNum) {
            state.dataSQLiteProcedures->sqlite->addNominalLightingData(lightNum, state.dataHeatBal->Lights(lightNum));
        }
        for (int peopleNum = 1; peopleNum <= state.dataHeatBal->TotPeople; ++peopleNum) {
            state.dataSQLiteProcedures->sqlite->addNominalPeopleData(peopleNum, state.dataHeatBal->People(peopleNum));
        }
        for (int elecEquipNum = 1; elecEquipNum <= state.dataHeatBal->TotElecEquip; ++elecEquipNum) {
            state.dataSQLiteProcedures->sqlite->addNominalElectricEquipmentData(elecEquipNum, state.dataHeatBal->ZoneElectric(elecEquipNum));
        }
        for (int gasEquipNum = 1; gasEquipNum <= state.dataHeatBal->TotGasEquip; ++gasEquipNum) {
            state.dataSQLiteProcedures->sqlite->addNominalGasEquipmentData(gasEquipNum, state.dataHeatBal->ZoneGas(gasEquipNum));
        }
        for (int steamEquipNum = 1; steamEquipNum <= state.dataHeatBal->TotStmEquip; ++steamEquipNum) {
            state.dataSQLiteProcedures->sqlite->addNominalSteamEquipmentData(steamEquipNum, state.dataHeatBal->ZoneSteamEq(steamEquipNum));
        }
        for (int hWEquipNum = 1; hWEquipNum <= state.dataHeatBal->TotHWEquip; ++hWEquipNum) {
            state.dataSQLiteProcedures->sqlite->addNominalHotWaterEquipmentData(hWEquipNum, state.dataHeatBal->ZoneHWEq(hWEquipNum));
        }
        for (int otherEquipNum = 1; otherEquipNum <= state.dataHeatBal->TotOthEquip; ++otherEquipNum) {
            state.dataSQLiteProcedures->sqlite->addNominalOtherEquipmentData(otherEquipNum, state.dataHeatBal->ZoneOtherEq(otherEquipNum));
        }
        for (int bBHeatNum = 1; bBHeatNum <= state.dataHeatBal->TotBBHeat; ++bBHeatNum) {
            state.dataSQLiteProcedures->sqlite->addNominalBaseboardData(bBHeatNum, state.dataHeatBal->ZoneBBHeat(bBHeatNum));
        }
        for (int infilNum = 1; infilNum <= state.dataHeatBal->TotInfiltration; ++infilNum) {
            state.dataSQLiteProcedures->sqlite->addInfiltrationData(infilNum, state.dataHeatBal->Infiltration(infilNum));
        }
        for (int ventNum = 1; ventNum <= state.dataHeatBal->TotVentilation; ++ventNum) {
            state.dataSQLiteProcedures->sqlite->addVentilationData(ventNum, state.dataHeatBal->Ventilation(ventNum));
        }
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            state.dataSQLiteProcedures->sqlite->addRoomAirModelData(zoneNum, state.dataRoomAir->AirModel(zoneNum));
        }

        state.dataSQLiteProcedures->sqlite->createZoneExtendedOutput();
    }
}

SQLite::SQLite(std::shared_ptr<std::ostream> errorStream,
               fs::path const &dbName,
               fs::path const &errorFilePath,
               bool writeOutputToSQLite,
               bool writeTabularDataToSQLite)
    : SQLiteProcedures(errorStream, writeOutputToSQLite, dbName, errorFilePath), m_writeTabularDataToSQLite(writeTabularDataToSQLite),
      m_sqlDBTimeIndex(0), m_reportDataInsertStmt(nullptr), m_reportExtendedDataInsertStmt(nullptr), m_reportDictionaryInsertStmt(nullptr),
      m_timeIndexInsertStmt(nullptr), m_zoneInfoInsertStmt(nullptr), m_zoneInfoZoneListInsertStmt(nullptr), m_nominalLightingInsertStmt(nullptr),
      m_nominalElectricEquipmentInsertStmt(nullptr), m_nominalGasEquipmentInsertStmt(nullptr), m_nominalSteamEquipmentInsertStmt(nullptr),
      m_nominalHotWaterEquipmentInsertStmt(nullptr), m_nominalOtherEquipmentInsertStmt(nullptr), m_nominalBaseboardHeatInsertStmt(nullptr),
      m_surfaceInsertStmt(nullptr), m_constructionInsertStmt(nullptr), m_constructionLayerInsertStmt(nullptr), m_materialInsertStmt(nullptr),
      m_zoneListInsertStmt(nullptr), m_zoneGroupInsertStmt(nullptr), m_infiltrationInsertStmt(nullptr), m_ventilationInsertStmt(nullptr),
      m_nominalPeopleInsertStmt(nullptr), m_zoneSizingInsertStmt(nullptr), m_systemSizingInsertStmt(nullptr), m_componentSizingInsertStmt(nullptr),
      m_roomAirModelInsertStmt(nullptr), m_groundTemperatureInsertStmt(nullptr), m_weatherFileInsertStmt(nullptr), m_scheduleInsertStmt(nullptr),
      m_daylightMapTitleInsertStmt(nullptr), m_daylightMapHourlyTitleInsertStmt(nullptr), m_daylightMapHourlyDataInsertStmt(nullptr),
      m_environmentPeriodInsertStmt(nullptr), m_simulationsInsertStmt(nullptr), m_tabularDataInsertStmt(nullptr), m_stringsInsertStmt(nullptr),
      m_stringsLookUpStmt(nullptr), m_errorInsertStmt(nullptr), m_errorUpdateStmt(nullptr), m_simulationUpdateStmt(nullptr),
      m_simulationDataUpdateStmt(nullptr), m_rollbackToSavepointStmt(nullptr), m_createSavepointStmt(nullptr), m_releaseSavepointStmt(nullptr)
{
    if (m_writeOutputToSQLite) {
        sqliteExecuteCommand("PRAGMA locking_mode = EXCLUSIVE;");
        sqliteExecuteCommand("PRAGMA journal_mode = OFF;");
        sqliteExecuteCommand("PRAGMA synchronous = OFF;");
        sqliteExecuteCommand("PRAGMA encoding=\"UTF-8\";");

        // Turn this to ON for Foreign Key constraints.
        // This must be turned ON for every connection
        // Currently, inserting into daylighting tables does not work with this ON. The ZoneIndex referenced by DaylightMaps does not exist in
        // the database at the time data is inserted.
        sqliteExecuteCommand("PRAGMA foreign_keys = OFF;");

        initializeSimulationsTable();
        initializeEnvironmentPeriodsTable();
        initializeErrorsTable();
        initializeTimeIndicesTable();
        initializeZoneInfoTable();
        initializeZoneListTable();
        initializeZoneGroupTable();
        initializeZoneInfoZoneListTable();
        initializeSchedulesTable();
        initializeMaterialsTable();
        initializeConstructionsTables();
        initializeSurfacesTable();
        initializeReportDataDictionaryTable();
        initializeReportDataTables();
        initializeNominalPeopleTable();
        initializeNominalLightingTable();
        initializeNominalElectricEquipmentTable();
        initializeNominalGasEquipmentTable();
        initializeNominalSteamEquipmentTable();
        initializeNominalHotWaterEquipmentTable();
        initializeNominalOtherEquipmentTable();
        initializeNominalBaseboardHeatTable();
        initializeNominalInfiltrationTable();
        initializeNominalVentilationTable();
        initializeZoneSizingTable();
        initializeSystemSizingTable();
        initializeComponentSizingTable();
        initializeRoomAirModelTable();
        initializeDaylightMapTables();
        initializeViews();

        if (m_writeTabularDataToSQLite) {
            initializeTabularDataTable();
            initializeTabularDataView();
        }
    }
}

SQLite::~SQLite()
{
    sqlite3_finalize(m_reportDataInsertStmt);
    sqlite3_finalize(m_reportExtendedDataInsertStmt);
    sqlite3_finalize(m_reportDictionaryInsertStmt);
    sqlite3_finalize(m_timeIndexInsertStmt);
    sqlite3_finalize(m_zoneInfoInsertStmt);
    sqlite3_finalize(m_zoneListInsertStmt);
    sqlite3_finalize(m_zoneGroupInsertStmt);
    sqlite3_finalize(m_zoneInfoZoneListInsertStmt);
    sqlite3_finalize(m_nominalLightingInsertStmt);
    sqlite3_finalize(m_nominalElectricEquipmentInsertStmt);
    sqlite3_finalize(m_nominalGasEquipmentInsertStmt);
    sqlite3_finalize(m_nominalSteamEquipmentInsertStmt);
    sqlite3_finalize(m_nominalHotWaterEquipmentInsertStmt);
    sqlite3_finalize(m_nominalOtherEquipmentInsertStmt);
    sqlite3_finalize(m_nominalBaseboardHeatInsertStmt);
    sqlite3_finalize(m_surfaceInsertStmt);
    sqlite3_finalize(m_constructionInsertStmt);
    sqlite3_finalize(m_constructionLayerInsertStmt);
    sqlite3_finalize(m_materialInsertStmt);
    sqlite3_finalize(m_infiltrationInsertStmt);
    sqlite3_finalize(m_ventilationInsertStmt);
    sqlite3_finalize(m_nominalPeopleInsertStmt);
    sqlite3_finalize(m_zoneSizingInsertStmt);
    sqlite3_finalize(m_systemSizingInsertStmt);
    sqlite3_finalize(m_componentSizingInsertStmt);
    sqlite3_finalize(m_roomAirModelInsertStmt);
    sqlite3_finalize(m_groundTemperatureInsertStmt);
    sqlite3_finalize(m_weatherFileInsertStmt);
    sqlite3_finalize(m_scheduleInsertStmt);
    sqlite3_finalize(m_daylightMapTitleInsertStmt);
    sqlite3_finalize(m_daylightMapHourlyTitleInsertStmt);
    sqlite3_finalize(m_daylightMapHourlyDataInsertStmt);
    sqlite3_finalize(m_environmentPeriodInsertStmt);
    sqlite3_finalize(m_simulationsInsertStmt);
    sqlite3_finalize(m_tabularDataInsertStmt);
    sqlite3_finalize(m_stringsInsertStmt);
    sqlite3_finalize(m_stringsLookUpStmt);
    sqlite3_finalize(m_errorInsertStmt);
    sqlite3_finalize(m_errorUpdateStmt);
    sqlite3_finalize(m_simulationUpdateStmt);
    sqlite3_finalize(m_simulationDataUpdateStmt);
    sqlite3_finalize(m_rollbackToSavepointStmt);
    sqlite3_finalize(m_createSavepointStmt);
    sqlite3_finalize(m_releaseSavepointStmt);
}

bool SQLite::writeOutputToSQLite() const
{
    return m_writeOutputToSQLite;
}

bool SQLite::writeTabularDataToSQLite() const
{
    return m_writeTabularDataToSQLite;
}

void SQLite::sqliteBegin()
{
    if (m_writeOutputToSQLite) {
        sqliteExecuteCommand("BEGIN;");
    }
}

void SQLite::sqliteCommit()
{
    if (m_writeOutputToSQLite) {
        sqliteExecuteCommand("COMMIT;");
    }
}

void SQLite::sqliteRollback()
{
    if (m_writeOutputToSQLite) {
        sqliteExecuteCommand("ROLLBACK;");
    }
}

void SQLite::sqliteRollbackToSavepoint(std::string_view savepoint_name)
{
    if (m_writeOutputToSQLite) {
        static constexpr std::string_view rollbackToSavepointSQL("ROLLBACK TO SAVEPOINT ?;");

        sqlitePrepareStatement(m_rollbackToSavepointStmt, rollbackToSavepointSQL);
        sqliteBindText(m_rollbackToSavepointStmt, 1, savepoint_name);
        sqliteStepCommand(m_rollbackToSavepointStmt);
        sqliteResetCommand(m_rollbackToSavepointStmt);
    }
}

void SQLite::sqliteReleaseSavepoint(std::string_view savepoint_name)
{
    if (m_writeOutputToSQLite) {
        static constexpr std::string_view releaseSavepointSQL("RELEASE SAVEPOINT ?;");

        sqlitePrepareStatement(m_releaseSavepointStmt, releaseSavepointSQL);
        sqliteBindText(m_releaseSavepointStmt, 1, savepoint_name);
        sqliteStepCommand(m_releaseSavepointStmt);
        sqliteResetCommand(m_releaseSavepointStmt);
    }
}

void SQLite::sqliteCreateSavepoint(std::string_view savepoint_name)
{
    if (m_writeOutputToSQLite) {
        static constexpr std::string_view createSavepointSQL("SAVEPOINT ?;");

        sqlitePrepareStatement(m_createSavepointStmt, createSavepointSQL);
        sqliteBindText(m_createSavepointStmt, 1, savepoint_name);
        sqliteStepCommand(m_createSavepointStmt);
        sqliteResetCommand(m_createSavepointStmt);
    }
}

bool SQLite::sqliteWithinTransaction()
{
    if (m_writeOutputToSQLite) {
        return SQLiteProcedures::sqliteWithinTransaction();
    }
    return false;
}

void SQLite::sqliteWriteMessage(std::string_view message)
{
    if (m_writeOutputToSQLite) {
        *m_errorStream << "SQLite3 message, " << message << std::endl;
    }
}

void SQLite::initializeReportDataDictionaryTable()
{
    constexpr std::string_view newTableSQL = "CREATE TABLE ReportDataDictionary("
                                             "ReportDataDictionaryIndex INTEGER PRIMARY KEY, "
                                             "IsMeter INTEGER, "
                                             "Type TEXT, "
                                             "IndexGroup TEXT, "
                                             "TimestepType TEXT, "
                                             "KeyValue TEXT, "
                                             "Name TEXT, "
                                             "ReportingFrequency TEXT, "
                                             "ScheduleName TEXT, "
                                             "Units TEXT);";

    sqliteExecuteCommand(newTableSQL);

    constexpr std::string_view preparedSQL = "INSERT INTO ReportDataDictionary ("
                                             "ReportDataDictionaryIndex, "
                                             "IsMeter, "
                                             "Type, "
                                             "IndexGroup, "
                                             "TimestepType, "
                                             "KeyValue, "
                                             "Name, "
                                             "ReportingFrequency, "
                                             "ScheduleName, "
                                             "Units) "
                                             "VALUES(?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_reportDictionaryInsertStmt, preparedSQL);
}

void SQLite::initializeReportDataTables()
{
    constexpr std::string_view reportDataTableSQL =
        "CREATE TABLE ReportData ("
        "ReportDataIndex INTEGER PRIMARY KEY, "
        "TimeIndex INTEGER, "
        "ReportDataDictionaryIndex INTEGER, "
        "Value REAL, "
        "FOREIGN KEY(TimeIndex) REFERENCES Time(TimeIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE "
        "FOREIGN KEY(ReportDataDictionaryIndex) REFERENCES ReportDataDictionary(ReportDataDictionaryIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE "
        ");";

    sqliteExecuteCommand(reportDataTableSQL);

    constexpr std::string_view reportDataInsertSQL = "INSERT INTO ReportData ("
                                                     "ReportDataIndex, "
                                                     "TimeIndex, "
                                                     "ReportDataDictionaryIndex, "
                                                     "Value) "
                                                     "VALUES(?,?,?,?);";

    sqlitePrepareStatement(m_reportDataInsertStmt, reportDataInsertSQL);

    constexpr std::string_view reportExtendedDataTableSQL = "CREATE TABLE ReportExtendedData ("
                                                            "ReportExtendedDataIndex INTEGER PRIMARY KEY, "
                                                            "ReportDataIndex INTEGER, "
                                                            "MaxValue REAL, "
                                                            "MaxMonth INTEGER, "
                                                            "MaxDay INTEGER, "
                                                            "MaxHour INTEGER, "
                                                            "MaxStartMinute INTEGER, "
                                                            "MaxMinute INTEGER, "
                                                            "MinValue REAL, "
                                                            "MinMonth INTEGER, "
                                                            "MinDay INTEGER, "
                                                            "MinHour INTEGER, "
                                                            "MinStartMinute INTEGER, "
                                                            "MinMinute INTEGER, "
                                                            "FOREIGN KEY(ReportDataIndex) REFERENCES ReportData(ReportDataIndex) "
                                                            "ON DELETE CASCADE ON UPDATE CASCADE "
                                                            ");";

    sqliteExecuteCommand(reportExtendedDataTableSQL);

    constexpr std::string_view reportExtendedDataInsertSQL = "INSERT INTO ReportExtendedData ("
                                                             "ReportExtendedDataIndex, "
                                                             "ReportDataIndex, "
                                                             "MaxValue, "
                                                             "MaxMonth, "
                                                             "MaxDay, "
                                                             "MaxHour, "
                                                             "MaxStartMinute, "
                                                             "MaxMinute, "
                                                             "MinValue, "
                                                             "MinMonth, "
                                                             "MinDay, "
                                                             "MinHour, "
                                                             "MinStartMinute, "
                                                             "MinMinute) "
                                                             "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_reportExtendedDataInsertStmt, reportExtendedDataInsertSQL);
}

void SQLite::initializeTimeIndicesTable()
{
    constexpr std::string_view timeTableSQL = "CREATE TABLE Time ("
                                              "TimeIndex INTEGER PRIMARY KEY, "
                                              "Year INTEGER, "
                                              "Month INTEGER, "
                                              "Day INTEGER, "
                                              "Hour INTEGER, "
                                              "Minute INTEGER, "
                                              "Dst INTEGER, "
                                              "Interval INTEGER, "
                                              "IntervalType INTEGER, "
                                              "SimulationDays INTEGER, "
                                              "DayType TEXT, "
                                              "EnvironmentPeriodIndex INTEGER, "
                                              "WarmupFlag INTEGER);";

    sqliteExecuteCommand(timeTableSQL);

    constexpr std::string_view timeIndexInsertSQL = "INSERT INTO Time ("
                                                    "TimeIndex, "
                                                    "Year, "
                                                    "Month, "
                                                    "Day, "
                                                    "Hour, "
                                                    "Minute, "
                                                    "DST, "
                                                    "Interval, "
                                                    "IntervalType, "
                                                    "SimulationDays, "
                                                    "DayType, "
                                                    "EnvironmentPeriodIndex, "
                                                    "WarmupFlag) "
                                                    "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_timeIndexInsertStmt, timeIndexInsertSQL);
}

void SQLite::initializeZoneInfoTable()
{
    constexpr std::string_view zonesTableSQL = "CREATE TABLE Zones ("
                                               "ZoneIndex INTEGER PRIMARY KEY, "
                                               "ZoneName TEXT, "
                                               "RelNorth REAL, "
                                               "OriginX REAL, "
                                               "OriginY REAL, "
                                               "OriginZ REAL, "
                                               "CentroidX REAL, "
                                               "CentroidY REAL, "
                                               "CentroidZ REAL, "
                                               "OfType INTEGER, "
                                               "Multiplier REAL, "
                                               "ListMultiplier REAL, "
                                               "MinimumX REAL, "
                                               "MaximumX REAL, "
                                               "MinimumY REAL, "
                                               "MaximumY REAL, "
                                               "MinimumZ REAL, "
                                               "MaximumZ REAL, "
                                               "CeilingHeight REAL, "
                                               "Volume REAL, "
                                               "InsideConvectionAlgo INTEGER, "
                                               "OutsideConvectionAlgo INTEGER, "
                                               "FloorArea REAL, "
                                               "ExtGrossWallArea REAL, "
                                               "ExtNetWallArea REAL, "
                                               "ExtWindowArea REAL, "
                                               "IsPartOfTotalArea INTEGER);";

    sqliteExecuteCommand(zonesTableSQL);

    constexpr std::string_view zoneInfoInsertSQL = "INSERT INTO Zones ("
                                                   "ZoneIndex, "
                                                   "ZoneName, "
                                                   "RelNorth, "
                                                   "OriginX, "
                                                   "OriginY, "

                                                   "OriginZ, "
                                                   "CentroidX, "
                                                   "CentroidY, "
                                                   "CentroidZ, "
                                                   "OfType, "

                                                   "Multiplier, "
                                                   "ListMultiplier, "
                                                   "MinimumX, "
                                                   "MaximumX, "
                                                   "MinimumY, "

                                                   "MaximumY, "
                                                   "MinimumZ, "
                                                   "MaximumZ, "
                                                   "CeilingHeight, "
                                                   "Volume, "

                                                   "InsideConvectionAlgo, "
                                                   "OutsideConvectionAlgo, "
                                                   "FloorArea, "
                                                   "ExtGrossWallArea, "
                                                   "ExtNetWallArea, "

                                                   "ExtWindowArea, "
                                                   "IsPartOfTotalArea) "
                                                   "VALUES (?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?,?,?,?, ?,?);";

    sqlitePrepareStatement(m_zoneInfoInsertStmt, zoneInfoInsertSQL);
}

void SQLite::initializeZoneInfoZoneListTable()
{
    constexpr std::string_view zoneInfoZoneListTableSQL = "CREATE TABLE ZoneInfoZoneLists ("
                                                          "ZoneListIndex INTEGER NOT NULL, "
                                                          "ZoneIndex INTEGER NOT NULL, "
                                                          "PRIMARY KEY(ZoneListIndex, ZoneIndex), "
                                                          "FOREIGN KEY(ZoneListIndex) REFERENCES ZoneLists(ZoneListIndex) "
                                                          "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                          "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                          "ON DELETE CASCADE ON UPDATE CASCADE "
                                                          ");";

    sqliteExecuteCommand(zoneInfoZoneListTableSQL);

    constexpr std::string_view zoneInfoZoneListInsertSQL = "INSERT INTO ZoneInfoZoneLists ("
                                                           "ZoneListIndex, "
                                                           "ZoneIndex) "
                                                           "VALUES (?,?);";

    sqlitePrepareStatement(m_zoneInfoZoneListInsertStmt, zoneInfoZoneListInsertSQL);
}

void SQLite::initializeNominalPeopleTable()
{
    constexpr std::string_view nominalPeopleTableSQL =
        "CREATE TABLE NominalPeople ( "
        "NominalPeopleIndex INTEGER PRIMARY KEY, ObjectName TEXT, ZoneIndex INTEGER,"
        "NumberOfPeople INTEGER, NumberOfPeopleScheduleIndex INTEGER, ActivityScheduleIndex INTEGER, FractionRadiant REAL, "
        "FractionConvected REAL, WorkEfficiencyScheduleIndex INTEGER, ClothingEfficiencyScheduleIndex INTEGER, "
        "AirVelocityScheduleIndex INTEGER, Fanger INTEGER, Pierce INTEGER, KSU INTEGER, "
        "MRTCalcType INTEGER, SurfaceIndex INTEGER, "
        "AngleFactorListName TEXT, AngleFactorList INTEGER, UserSpecifeidSensibleFraction REAL, Show55Warning INTEGER, "
        "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE, "
        "FOREIGN KEY(NumberOfPeopleScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE, "
        "FOREIGN KEY(ActivityScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE, "
        "FOREIGN KEY(WorkEfficiencyScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE, "
        "FOREIGN KEY(ClothingEfficiencyScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE, "
        "FOREIGN KEY(AirVelocityScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE, "
        "FOREIGN KEY(SurfaceIndex) REFERENCES Surfaces(SurfaceIndex) "
        "ON UPDATE CASCADE "
        ");";

    sqliteExecuteCommand(nominalPeopleTableSQL);

    constexpr std::string_view nominalPeopleInsertSQL = "INSERT INTO NominalPeople VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalPeopleInsertStmt, nominalPeopleInsertSQL);
}

void SQLite::initializeNominalLightingTable()
{
    constexpr std::string_view nominalLightingTableSQL =
        "CREATE TABLE NominalLighting ( "
        "NominalLightingIndex INTEGER PRIMARY KEY, ObjectName TEXT, "
        "ZoneIndex INTEGER, ScheduleIndex INTEGER, DesignLevel REAL, FractionReturnAir REAL, FractionRadiant REAL, "
        "FractionShortWave REAL, FractionReplaceable REAL, FractionConvected REAL, EndUseSubcategory TEXT, "
        "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE, "
        "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE "
        ");";

    sqliteExecuteCommand(nominalLightingTableSQL);

    constexpr std::string_view nominalLightingInsertSQL = "INSERT INTO NominalLighting VALUES(?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalLightingInsertStmt, nominalLightingInsertSQL);
}

void SQLite::initializeNominalElectricEquipmentTable()
{
    constexpr std::string_view nominalElectricEquipmentTableSQL = "CREATE TABLE NominalElectricEquipment ("
                                                                  "NominalElectricEquipmentIndex INTEGER PRIMARY KEY, "
                                                                  "ObjectName TEXT, "
                                                                  "ZoneIndex INTEGER, ScheduleIndex INTEGER, DesignLevel REAL, "
                                                                  "FractionLatent REAL, FractionRadiant REAL, FractionLost REAL, "
                                                                  "FractionConvected REAL, EndUseSubcategory TEXT, "
                                                                  "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                                  "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                                  "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
                                                                  "ON UPDATE CASCADE "
                                                                  ");";

    sqliteExecuteCommand(nominalElectricEquipmentTableSQL);

    constexpr std::string_view nominalElectricEquipmentInsertSQL = "INSERT INTO NominalElectricEquipment VALUES(?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalElectricEquipmentInsertStmt, nominalElectricEquipmentInsertSQL);
}

void SQLite::initializeNominalGasEquipmentTable()
{
    constexpr std::string_view nominalGasEquipmentTableSQL = "CREATE TABLE NominalGasEquipment( "
                                                             "NominalGasEquipmentIndex INTEGER PRIMARY KEY, ObjectName TEXT, "
                                                             "ZoneIndex INTEGER, ScheduleIndex INTEGER, "
                                                             "DesignLevel REAL, FractionLatent REAL, FractionRadiant REAL, FractionLost REAL, "
                                                             "FractionConvected REAL, EndUseSubcategory TEXT, "
                                                             "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                             "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                             "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
                                                             "ON UPDATE CASCADE "
                                                             ");";

    sqliteExecuteCommand(nominalGasEquipmentTableSQL);

    constexpr std::string_view nominalGasEquipmentInsertSQL = "INSERT INTO NominalGasEquipment VALUES(?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalGasEquipmentInsertStmt, nominalGasEquipmentInsertSQL);
}

void SQLite::initializeNominalSteamEquipmentTable()
{
    constexpr std::string_view nominalSteamEquipmentTableSQL = "CREATE TABLE NominalSteamEquipment( "
                                                               "NominalSteamEquipmentIndex INTEGER PRIMARY KEY, ObjectName TEXT, "
                                                               "ZoneIndex INTEGER, ScheduleIndex INTEGER, DesignLevel REAL, "
                                                               "FractionLatent REAL, FractionRadiant REAL, FractionLost REAL, "
                                                               "FractionConvected REAL, EndUseSubcategory TEXT, "
                                                               "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                               "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                               "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
                                                               "ON UPDATE CASCADE "
                                                               ");";

    sqliteExecuteCommand(nominalSteamEquipmentTableSQL);

    constexpr std::string_view nominalSteamEquipmentInsertSQL = "INSERT INTO NominalSteamEquipment VALUES(?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalSteamEquipmentInsertStmt, nominalSteamEquipmentInsertSQL);
}

void SQLite::initializeNominalHotWaterEquipmentTable()
{
    constexpr std::string_view nominalHotWaterEquipmentTableSQL =
        "CREATE TABLE NominalHotWaterEquipment("
        "NominalHotWaterEquipmentIndex INTEGER PRIMARY KEY, "
        "ObjectName TEXT, "
        "ZoneIndex INTEGER, SchedNo INTEGER, DesignLevel REAL, FractionLatent REAL, FractionRadiant REAL, FractionLost REAL, "
        "FractionConvected REAL, EndUseSubcategory TEXT, "
        "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE, "
        "FOREIGN KEY(SchedNo) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE "
        ");";

    sqliteExecuteCommand(nominalHotWaterEquipmentTableSQL);

    constexpr std::string_view nominalHotWaterEquipmentInsertSQL = "INSERT INTO NominalHotWaterEquipment VALUES(?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalHotWaterEquipmentInsertStmt, nominalHotWaterEquipmentInsertSQL);
}

void SQLite::initializeNominalOtherEquipmentTable()
{
    constexpr std::string_view nominalOtherEquipmentTableSQL = "CREATE TABLE NominalOtherEquipment( "
                                                               "NominalOtherEquipmentIndex INTEGER PRIMARY KEY, ObjectName TEXT, "
                                                               "ZoneIndex INTEGER, ScheduleIndex INTEGER, DesignLevel REAL, FractionLatent REAL, "
                                                               "FractionRadiant REAL, FractionLost REAL, "
                                                               "FractionConvected REAL, EndUseSubcategory TEXT, "
                                                               "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                               "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                               "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
                                                               "ON UPDATE CASCADE "
                                                               ");";

    sqliteExecuteCommand(nominalOtherEquipmentTableSQL);

    constexpr std::string_view nominalOtherEquipmentInsertSQL = "INSERT INTO NominalOtherEquipment VALUES(?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalOtherEquipmentInsertStmt, nominalOtherEquipmentInsertSQL);
}

void SQLite::initializeNominalBaseboardHeatTable()
{
    constexpr std::string_view nominalBaseboardHeatersTableSQL =
        "CREATE TABLE NominalBaseboardHeaters ( "
        "NominalBaseboardHeaterIndex INTEGER PRIMARY KEY, ObjectName TEXT, "
        "ZoneIndex INTEGER, ScheduleIndex INTEGER, CapatLowTemperature REAL, LowTemperature REAL, CapatHighTemperature REAL, "
        "HighTemperature REAL, FractionRadiant REAL, FractionConvected REAL, EndUseSubcategory TEXT, "
        "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE, "
        "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
        "ON UPDATE CASCADE "
        ");";

    sqliteExecuteCommand(nominalBaseboardHeatersTableSQL);

    constexpr std::string_view nominalBaseboardHeatInsertSQL = "INSERT INTO NominalBaseboardHeaters VALUES(?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_nominalBaseboardHeatInsertStmt, nominalBaseboardHeatInsertSQL);
}

void SQLite::initializeSurfacesTable()
{
    constexpr std::string_view surfacesTableSQL = "CREATE TABLE Surfaces ( "
                                                  "SurfaceIndex INTEGER PRIMARY KEY, SurfaceName TEXT, ConstructionIndex INTEGER, "
                                                  "ClassName TEXT, Area REAL, GrossArea REAL, Perimeter REAL, "
                                                  "Azimuth REAL, Height REAL, Reveal REAL, "
                                                  "Shape INTEGER, Sides INTEGER, Tilt REAL, Width REAL, HeatTransferSurf INTEGER, "
                                                  "BaseSurfaceIndex INTEGER, ZoneIndex INTEGER, ExtBoundCond INTEGER,  "
                                                  "ExtSolar INTEGER, ExtWind INTEGER, "
                                                  "FOREIGN KEY(ConstructionIndex) REFERENCES Constructions(ConstructionIndex) "
                                                  "ON UPDATE CASCADE, "
                                                  "FOREIGN KEY(BaseSurfaceIndex) REFERENCES Surfaces(SurfaceIndex) "
                                                  "ON UPDATE CASCADE, "
                                                  "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                  "ON DELETE CASCADE ON UPDATE CASCADE "
                                                  ");";

    sqliteExecuteCommand(surfacesTableSQL);

    constexpr std::string_view surfaceInsertSQL = "INSERT INTO Surfaces VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_surfaceInsertStmt, surfaceInsertSQL);
}

void SQLite::initializeConstructionsTables()
{
    constexpr std::string_view constructionsTableSQL =
        "CREATE TABLE Constructions ( "
        "ConstructionIndex INTEGER PRIMARY KEY, Name TEXT, TotalLayers INTEGER, "
        "TotalSolidLayers INTEGER, TotalGlassLayers INTEGER, InsideAbsorpVis REAL, OutsideAbsorpVis REAL, "
        "InsideAbsorpSolar REAL, OutsideAbsorpSolar REAL, InsideAbsorpThermal REAL, OutsideAbsorpThermal REAL, "
        "OutsideRoughness INTEGER, TypeIsWindow INTEGER, Uvalue REAL"
        ");";

    sqliteExecuteCommand(constructionsTableSQL);

    constexpr std::string_view constructionInsertSQL = "INSERT INTO Constructions VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_constructionInsertStmt, constructionInsertSQL);

    constexpr std::string_view constructionLayersTableSQL = "CREATE TABLE ConstructionLayers ( "
                                                            "ConstructionLayersIndex INTEGER PRIMARY KEY, "
                                                            "ConstructionIndex INTEGER, LayerIndex INTEGER, MaterialIndex INTEGER, "
                                                            "FOREIGN KEY(ConstructionIndex) REFERENCES Constructions(ConstructionIndex) "
                                                            "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                            "FOREIGN KEY(MaterialIndex) REFERENCES Materials(MaterialIndex) "
                                                            "ON UPDATE CASCADE "
                                                            ");";

    sqliteExecuteCommand(constructionLayersTableSQL);

    constexpr std::string_view constructionLayerInsertSQL =
        "INSERT INTO ConstructionLayers(ConstructionIndex, LayerIndex, MaterialIndex) VALUES(?,?,?);";

    sqlitePrepareStatement(m_constructionLayerInsertStmt, constructionLayerInsertSQL);
}

void SQLite::initializeMaterialsTable()
{
    constexpr std::string_view materialsTableSQL = "CREATE TABLE Materials ( "
                                                   "MaterialIndex INTEGER PRIMARY KEY, "
                                                   "Name TEXT, MaterialType INTEGER, Roughness INTEGER, "
                                                   "Conductivity REAL, Density REAL, IsoMoistCap REAL, Porosity REAL, Resistance REAL, "
                                                   "ROnly INTEGER, SpecHeat REAL, ThermGradCoef REAL, Thickness REAL, VaporDiffus REAL "
                                                   ");";

    sqliteExecuteCommand(materialsTableSQL);

    constexpr std::string_view materialInsertSQL = "INSERT INTO Materials VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_materialInsertStmt, materialInsertSQL);
}

void SQLite::initializeZoneListTable()
{
    constexpr std::string_view zoneListsTableSQL = "CREATE TABLE ZoneLists ( "
                                                   "ZoneListIndex INTEGER PRIMARY KEY, Name TEXT);";

    sqliteExecuteCommand(zoneListsTableSQL);

    constexpr std::string_view zoneListInsertSQL = "INSERT INTO ZoneLists VALUES(?,?);";

    sqlitePrepareStatement(m_zoneListInsertStmt, zoneListInsertSQL);
}

void SQLite::initializeZoneGroupTable()
{
    constexpr std::string_view zoneGroupsTableSQL = "CREATE TABLE ZoneGroups ( "
                                                    "ZoneGroupIndex INTEGER PRIMARY KEY, "
                                                    "ZoneGroupName TEXT, "
                                                    "ZoneListIndex INTEGER, "
                                                    "ZoneListMultiplier INTEGER, "
                                                    "FOREIGN KEY(ZoneListIndex) REFERENCES ZoneLists(ZoneListIndex) "
                                                    "ON UPDATE CASCADE "
                                                    ");";

    sqliteExecuteCommand(zoneGroupsTableSQL);

    constexpr std::string_view zoneGroupInsertSQL = "INSERT INTO ZoneGroups VALUES(?,?,?,?);";

    sqlitePrepareStatement(m_zoneGroupInsertStmt, zoneGroupInsertSQL);
}

void SQLite::initializeNominalInfiltrationTable()
{
    constexpr std::string_view nominalInfiltrationTableSQL = "CREATE TABLE NominalInfiltration ( "
                                                             "NominalInfiltrationIndex INTEGER PRIMARY KEY, "
                                                             "ObjectName TEXT, "
                                                             "ZoneIndex INTEGER, ScheduleIndex INTEGER, DesignLevel REAL, "
                                                             "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                             "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                             "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
                                                             "ON UPDATE CASCADE "
                                                             ");";

    sqliteExecuteCommand(nominalInfiltrationTableSQL);

    constexpr std::string_view infiltrationInsertSQL =
        "INSERT INTO NominalInfiltration (NominalInfiltrationIndex, ObjectName, ZoneIndex, ScheduleIndex, DesignLevel)"
        "VALUES (?,?,?,?,?);";

    sqlitePrepareStatement(m_infiltrationInsertStmt, infiltrationInsertSQL);
}

void SQLite::initializeNominalVentilationTable()
{
    constexpr std::string_view nominalVentilationTableSQL = "CREATE TABLE NominalVentilation ( "
                                                            "NominalVentilationIndex INTEGER PRIMARY KEY, "
                                                            "ObjectName TEXT, "
                                                            "ZoneIndex INTEGER, ScheduleIndex INTEGER, DesignLevel REAL, "
                                                            "FOREIGN KEY(ZoneIndex) REFERENCES Zones(ZoneIndex) "
                                                            "ON DELETE CASCADE ON UPDATE CASCADE, "
                                                            "FOREIGN KEY(ScheduleIndex) REFERENCES Schedules(ScheduleIndex) "
                                                            "ON UPDATE CASCADE "
                                                            ");";

    sqliteExecuteCommand(nominalVentilationTableSQL);

    constexpr std::string_view ventilationInsertSQL = "INSERT INTO NominalVentilation VALUES(?,?,?,?,?);";

    sqlitePrepareStatement(m_ventilationInsertStmt, ventilationInsertSQL);
}

void SQLite::initializeZoneSizingTable()
{
    constexpr std::string_view zoneSizesTableSQL =
        "CREATE TABLE ZoneSizes ( "
        "ZoneSizesIndex INTEGER PRIMARY KEY, ZoneName TEXT, LoadType TEXT, "
        "CalcDesLoad REAL, UserDesLoad REAL, CalcDesFlow REAL, UserDesFlow REAL, DesDayName TEXT, PeakHrMin TEXT, "
        "PeakTemp REAL, PeakHumRat REAL, CalcOutsideAirFlow REAL, DOASHeatAddRate REAL"
        ");";

    sqliteExecuteCommand(zoneSizesTableSQL);

    constexpr std::string_view zoneSizingInsertSQL = "INSERT INTO ZoneSizes VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_zoneSizingInsertStmt, zoneSizingInsertSQL);
}

void SQLite::initializeSystemSizingTable()
{
    constexpr std::string_view systemSizesTableSQL =
        "CREATE TABLE SystemSizes (SystemSizesIndex INTEGER PRIMARY KEY, SystemName TEXT, LoadType TEXT, PeakLoadType TEXT, "
        "UserDesCap REAL, CalcDesVolFlow REAL, UserDesVolFlow REAL, DesDayName TEXT, PeakHrMin TEXT);";

    sqliteExecuteCommand(systemSizesTableSQL);

    constexpr std::string_view systemSizingInsertSQL = "INSERT INTO SystemSizes VALUES(?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_systemSizingInsertStmt, systemSizingInsertSQL);
}

void SQLite::initializeComponentSizingTable()
{
    constexpr std::string_view componentSizesTableSQL = "CREATE TABLE ComponentSizes (ComponentSizesIndex INTEGER PRIMARY KEY, "
                                                        "CompType TEXT, CompName TEXT, Description TEXT, Value REAL, Units TEXT, StrValue TEXT);";

    sqliteExecuteCommand(componentSizesTableSQL);

    constexpr std::string_view componentSizingInsertSQL = "INSERT INTO ComponentSizes VALUES (?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_componentSizingInsertStmt, componentSizingInsertSQL);
}

void SQLite::initializeRoomAirModelTable()
{
    constexpr std::string_view roomAirModelsTableSQL =
        "CREATE TABLE RoomAirModels (ZoneIndex INTEGER PRIMARY KEY, AirModelName TEXT, AirModelType INTEGER, "
        "TempCoupleScheme INTEGER, SimAirModel INTEGER);";

    sqliteExecuteCommand(roomAirModelsTableSQL);

    constexpr std::string_view roomAirModelInsertSQL = "INSERT INTO RoomAirModels VALUES(?,?,?,?,?);";

    sqlitePrepareStatement(m_roomAirModelInsertStmt, roomAirModelInsertSQL);
}

void SQLite::initializeSchedulesTable()
{
    constexpr std::string_view scheduleTableSQL = "CREATE TABLE Schedules (ScheduleIndex INTEGER PRIMARY KEY, ScheduleName TEXT, "
                                                  "ScheduleType TEXT, ScheduleMinimum REAL, ScheduleMaximum REAL);";

    sqliteExecuteCommand(scheduleTableSQL);

    constexpr std::string_view scheduleInsertSQL = "INSERT INTO Schedules VALUES(?,?,?,?,?);";

    sqlitePrepareStatement(m_scheduleInsertStmt, scheduleInsertSQL);
}

void SQLite::initializeDaylightMapTables()
{
    constexpr std::string_view daylightMapsTableSQL = "CREATE TABLE DaylightMaps ( "
                                                      "MapNumber INTEGER PRIMARY KEY, MapName TEXT, "
                                                      "Environment TEXT, Zone INTEGER, ReferencePts TEXT, Z REAL, "
                                                      "FOREIGN KEY(Zone) REFERENCES Zones(ZoneIndex) "
                                                      "ON DELETE CASCADE ON UPDATE CASCADE "
                                                      ");";

    sqliteExecuteCommand(daylightMapsTableSQL);

    constexpr std::string_view daylightMapTitleInsertSQL = "INSERT INTO DaylightMaps VALUES(?,?,?,?,?,?);";

    sqlitePrepareStatement(m_daylightMapTitleInsertStmt, daylightMapTitleInsertSQL);

    constexpr std::string_view daylightMapHourlyReportsTableSQL = "CREATE TABLE DaylightMapHourlyReports ( "
                                                                  "HourlyReportIndex INTEGER PRIMARY KEY, "
                                                                  "MapNumber INTEGER, Year INTEGER, Month INTEGER, DayOfMonth INTEGER, Hour INTEGER, "
                                                                  "FOREIGN KEY(MapNumber) REFERENCES DaylightMaps(MapNumber) "
                                                                  "ON DELETE CASCADE ON UPDATE CASCADE "
                                                                  ");";

    sqliteExecuteCommand(daylightMapHourlyReportsTableSQL);

    constexpr std::string_view daylightMapHourlyTitleInsertSQL = "INSERT INTO DaylightMapHourlyReports VALUES(?,?,?,?,?,?);";

    sqlitePrepareStatement(m_daylightMapHourlyTitleInsertStmt, daylightMapHourlyTitleInsertSQL);

    constexpr std::string_view daylightMapHourlyDataTableSQL =
        "CREATE TABLE DaylightMapHourlyData ( "
        "HourlyDataIndex INTEGER PRIMARY KEY, HourlyReportIndex INTEGER, "
        "X REAL, Y REAL, Illuminance REAL, "
        "FOREIGN KEY(HourlyReportIndex) REFERENCES DaylightMapHourlyReports(HourlyReportIndex) "
        "ON DELETE CASCADE ON UPDATE CASCADE "
        ");";

    sqliteExecuteCommand(daylightMapHourlyDataTableSQL);

    constexpr std::string_view daylightMapHourlyDataInsertSQL = "INSERT INTO DaylightMapHourlyData VALUES(?,?,?,?,?);";

    sqlitePrepareStatement(m_daylightMapHourlyDataInsertStmt, daylightMapHourlyDataInsertSQL);
}

void SQLite::initializeViews()
{
    constexpr std::string_view reportVariableWithTimeViewSQL =
        "CREATE VIEW ReportVariableWithTime AS "
        "SELECT rd.ReportDataIndex, rd.TimeIndex, rd.ReportDataDictionaryIndex, red.ReportExtendedDataIndex, rd.Value, "
        "t.Month, t.Day, t.Hour, t.Minute, t.Dst, t.Interval, t.IntervalType, t.SimulationDays, t.DayType, t.EnvironmentPeriodIndex, t.WarmupFlag, "
        "rdd.IsMeter, rdd.Type, rdd.IndexGroup, rdd.TimestepType, rdd.KeyValue, rdd.Name, rdd.ReportingFrequency, rdd.ScheduleName, rdd.Units, "
        "red.MaxValue, red.MaxMonth, red.MaxDay, red.MaxStartMinute, red.MaxMinute, red.MinValue, red.MinMonth, red.MinDay, red.MinStartMinute, "
        "red.MinMinute "
        "FROM ReportData As rd "
        "INNER JOIN ReportDataDictionary As rdd "
        "ON rd.ReportDataDictionaryIndex = rdd.ReportDataDictionaryIndex "
        "LEFT OUTER JOIN ReportExtendedData As red "
        "ON rd.ReportDataIndex = red.ReportDataIndex "
        "INNER JOIN Time As t "
        "ON rd.TimeIndex = t.TimeIndex;";

    sqliteExecuteCommand(reportVariableWithTimeViewSQL);

    constexpr std::string_view reportVariableDataViewSQL =
        "CREATE VIEW ReportVariableData AS "
        "SELECT rd.ReportDataIndex As rowid, rd.TimeIndex, rd.ReportDataDictionaryIndex As ReportVariableDataDictionaryIndex, "
        "rd.Value As VariableValue, red.ReportExtendedDataIndex As ReportVariableExtendedDataIndex "
        "FROM ReportData As rd "
        "LEFT OUTER JOIN ReportExtendedData As red "
        "ON rd.ReportDataIndex = red.ReportDataIndex;";

    sqliteExecuteCommand(reportVariableDataViewSQL);

    constexpr std::string_view reportVariableDataDictionaryViewSQL =
        "CREATE VIEW ReportVariableDataDictionary AS "
        "SELECT rdd.ReportDataDictionaryIndex As ReportVariableDataDictionaryIndex, rdd.Type As VariableType, rdd.IndexGroup, rdd.TimestepType, "
        "rdd.KeyValue, rdd.Name As VariableName, rdd.ReportingFrequency, rdd.ScheduleName, rdd.Units As VariableUnits "
        "FROM ReportDataDictionary As rdd;";

    sqliteExecuteCommand(reportVariableDataDictionaryViewSQL);

    constexpr std::string_view reportVariableExtendedDataViewSQL =
        "CREATE VIEW ReportVariableExtendedData AS "
        "SELECT red.ReportExtendedDataIndex As ReportVariableExtendedDataIndex, red.MaxValue, red.MaxMonth, red.MaxDay, "
        "red.MaxStartMinute, red.MaxMinute, red.MinValue, red.MinMonth, red.MinDay, red.MinStartMinute, red.MinMinute "
        "FROM ReportExtendedData As red;";

    sqliteExecuteCommand(reportVariableExtendedDataViewSQL);

    constexpr std::string_view reportMeterDataViewSQL =
        "CREATE VIEW ReportMeterData AS "
        "SELECT rd.ReportDataIndex As rowid, rd.TimeIndex, rd.ReportDataDictionaryIndex As ReportMeterDataDictionaryIndex, "
        "rd.Value As VariableValue, red.ReportExtendedDataIndex As ReportVariableExtendedDataIndex "
        "FROM ReportData As rd "
        "LEFT OUTER JOIN ReportExtendedData As red "
        "ON rd.ReportDataIndex = red.ReportDataIndex "
        "INNER JOIN ReportDataDictionary As rdd "
        "ON rd.ReportDataDictionaryIndex = rdd.ReportDataDictionaryIndex "
        "WHERE rdd.IsMeter = 1;";

    sqliteExecuteCommand(reportMeterDataViewSQL);

    constexpr std::string_view reportMeterDataDictionaryViewSQL =
        "CREATE VIEW ReportMeterDataDictionary AS "
        "SELECT rdd.ReportDataDictionaryIndex As ReportMeterDataDictionaryIndex, rdd.Type As VariableType, rdd.IndexGroup, rdd.TimestepType, "
        "rdd.KeyValue, rdd.Name As VariableName, rdd.ReportingFrequency, rdd.ScheduleName, rdd.Units As VariableUnits "
        "FROM ReportDataDictionary As rdd "
        "WHERE rdd.IsMeter = 1;";

    sqliteExecuteCommand(reportMeterDataDictionaryViewSQL);

    constexpr std::string_view reportMeterExtendedDataViewSQL =
        "CREATE VIEW ReportMeterExtendedData AS "
        "SELECT red.ReportExtendedDataIndex As ReportMeterExtendedDataIndex, red.MaxValue, red.MaxMonth, red.MaxDay, "
        "red.MaxStartMinute, red.MaxMinute, red.MinValue, red.MinMonth, red.MinDay, red.MinStartMinute, red.MinMinute "
        "FROM ReportExtendedData As red "
        "LEFT OUTER JOIN ReportData As rd "
        "ON rd.ReportDataIndex = red.ReportDataIndex "
        "INNER JOIN ReportDataDictionary As rdd "
        "ON rd.ReportDataDictionaryIndex = rdd.ReportDataDictionaryIndex "
        "WHERE rdd.IsMeter = 1;";

    sqliteExecuteCommand(reportMeterExtendedDataViewSQL);
}

void SQLite::initializeSimulationsTable()
{
    constexpr std::string_view simulationsTableSQL = "CREATE TABLE Simulations (SimulationIndex INTEGER PRIMARY KEY, "
                                                     "EnergyPlusVersion TEXT, TimeStamp TEXT, NumTimestepsPerHour INTEGER, Completed BOOL, "
                                                     "CompletedSuccessfully BOOL);";

    sqliteExecuteCommand(simulationsTableSQL);

    constexpr std::string_view simulationsInsertSQL =
        "INSERT INTO Simulations(SimulationIndex, EnergyPlusVersion, TimeStamp, Completed, CompletedSuccessfully) "
        "VALUES(?,?,?,'FALSE','FALSE');";

    sqlitePrepareStatement(m_simulationsInsertStmt, simulationsInsertSQL);

    constexpr std::string_view simulationUpdateSQL = "UPDATE Simulations SET "
                                                     "Completed = ?, CompletedSuccessfully = ? "
                                                     "WHERE SimulationIndex = ?";

    sqlitePrepareStatement(m_simulationUpdateStmt, simulationUpdateSQL);

    constexpr std::string_view simulationDataUpdateSQL = "UPDATE Simulations SET "
                                                         "NumTimestepsPerHour = ? "
                                                         "WHERE SimulationIndex = ?";

    sqlitePrepareStatement(m_simulationDataUpdateStmt, simulationDataUpdateSQL);
}

void SQLite::initializeErrorsTable()
{
    constexpr std::string_view errorsTableSQL = "CREATE TABLE Errors ( "
                                                "ErrorIndex INTEGER PRIMARY KEY, SimulationIndex INTEGER, "
                                                "ErrorType INTEGER, ErrorMessage TEXT, Count INTEGER, "
                                                "FOREIGN KEY(SimulationIndex) REFERENCES Simulations(SimulationIndex) "
                                                "ON DELETE CASCADE ON UPDATE CASCADE "
                                                ");";

    sqliteExecuteCommand(errorsTableSQL);

    constexpr std::string_view errorInsertSQL = "INSERT INTO Errors VALUES(?,?,?,?,?);";

    sqlitePrepareStatement(m_errorInsertStmt, errorInsertSQL);

    constexpr std::string_view errorUpdateSQL =
        "UPDATE Errors SET "
        "ErrorMessage = ErrorMessage || ? WHERE ErrorIndex = (SELECT ErrorIndex FROM Errors ORDER BY ErrorIndex DESC LIMIT 1)";

    sqlitePrepareStatement(m_errorUpdateStmt, errorUpdateSQL);
}

void SQLite::initializeEnvironmentPeriodsTable()
{
    constexpr std::string_view environmentPeriodsTableSQL = "CREATE TABLE EnvironmentPeriods ( "
                                                            "EnvironmentPeriodIndex INTEGER PRIMARY KEY, "
                                                            "SimulationIndex INTEGER, EnvironmentName TEXT, EnvironmentType INTEGER, "
                                                            "FOREIGN KEY(SimulationIndex) REFERENCES Simulations(SimulationIndex) "
                                                            "ON DELETE CASCADE ON UPDATE CASCADE "
                                                            ");";

    sqliteExecuteCommand(environmentPeriodsTableSQL);

    constexpr std::string_view environmentPeriodInsertSQL = "INSERT INTO EnvironmentPeriods VALUES(?,?,?,?);";

    sqlitePrepareStatement(m_environmentPeriodInsertStmt, environmentPeriodInsertSQL);
}

void SQLite::initializeTabularDataTable()
{
    constexpr std::string_view sql = "CREATE TABLE StringTypes ( "
                                     "StringTypeIndex INTEGER PRIMARY KEY, "
                                     "Value TEXT"
                                     ");";

    sqliteExecuteCommand(sql);

    sqliteExecuteCommand(format("INSERT INTO StringTypes VALUES({},'ReportName');", ReportNameId));
    sqliteExecuteCommand(format("INSERT INTO StringTypes VALUES({},'ReportForString');", ReportForStringId));
    sqliteExecuteCommand(format("INSERT INTO StringTypes VALUES({},'TableName');", TableNameId));
    sqliteExecuteCommand(format("INSERT INTO StringTypes VALUES({},'RowName');", RowNameId));
    sqliteExecuteCommand(format("INSERT INTO StringTypes VALUES({},'ColumnName');", ColumnNameId));
    sqliteExecuteCommand(format("INSERT INTO StringTypes VALUES({},'Units');", UnitsId));

    constexpr std::string_view sql2 = "CREATE TABLE Strings ( "
                                      "StringIndex INTEGER PRIMARY KEY, "
                                      "StringTypeIndex INTEGER, "
                                      "Value TEXT, "
                                      "UNIQUE(StringTypeIndex, Value), "
                                      "FOREIGN KEY(StringTypeIndex) REFERENCES StringTypes(StringTypeIndex) "
                                      "ON UPDATE CASCADE "
                                      ");";

    sqliteExecuteCommand(sql2);

    constexpr std::string_view sql3 = "INSERT INTO Strings (StringIndex,StringTypeIndex,Value) VALUES(?,?,?);";

    sqlitePrepareStatement(m_stringsInsertStmt, sql3);

    constexpr std::string_view sql4 = "SELECT StringIndex FROM Strings WHERE StringTypeIndex=? AND Value=?;";

    sqlitePrepareStatement(m_stringsLookUpStmt, sql4);

    constexpr std::string_view sql5 = "CREATE TABLE TabularData ( "
                                      "TabularDataIndex INTEGER PRIMARY KEY, "
                                      "ReportNameIndex INTEGER, "
                                      "ReportForStringIndex INTEGER, "
                                      "TableNameIndex INTEGER, "
                                      "RowNameIndex INTEGER, "
                                      "ColumnNameIndex INTEGER, "
                                      "UnitsIndex INTEGER, "
                                      "SimulationIndex INTEGER, "
                                      "RowId INTEGER, "
                                      "ColumnId INTEGER, "
                                      "Value TEXT, "
                                      "FOREIGN KEY(ReportNameIndex) REFERENCES Strings(StringIndex) "
                                      "ON UPDATE CASCADE "
                                      "FOREIGN KEY(ReportForStringIndex) REFERENCES Strings(StringIndex) "
                                      "ON UPDATE CASCADE "
                                      "FOREIGN KEY(TableNameIndex) REFERENCES Strings(StringIndex) "
                                      "ON UPDATE CASCADE "
                                      "FOREIGN KEY(RowNameIndex) REFERENCES Strings(StringIndex) "
                                      "ON UPDATE CASCADE "
                                      "FOREIGN KEY(ColumnNameIndex) REFERENCES Strings(StringIndex) "
                                      "ON UPDATE CASCADE "
                                      "FOREIGN KEY(UnitsIndex) REFERENCES Strings(StringIndex) "
                                      "ON UPDATE CASCADE "
                                      "FOREIGN KEY(SimulationIndex) REFERENCES Simulations(SimulationIndex) "
                                      "ON DELETE CASCADE ON UPDATE CASCADE "
                                      ");";

    sqliteExecuteCommand(sql5);

    constexpr std::string_view sql6 = "INSERT INTO TabularData VALUES(?,?,?,?,?,?,?,?,?,?,?);";

    sqlitePrepareStatement(m_tabularDataInsertStmt, sql6);
}

void SQLite::initializeTabularDataView()
{
    constexpr std::string_view sql = "CREATE VIEW TabularDataWithStrings AS SELECT "
                                     "td.TabularDataIndex, "
                                     "td.Value As Value, "
                                     "reportn.Value As ReportName, "
                                     "fs.Value As ReportForString, "
                                     "tn.Value As TableName, "
                                     "rn.Value As RowName, "
                                     "cn.Value As ColumnName, "
                                     "u.Value As Units "
                                     "FROM TabularData As td "
                                     "INNER JOIN Strings As reportn ON reportn.StringIndex=td.ReportNameIndex "
                                     "INNER JOIN Strings As fs ON fs.StringIndex=td.ReportForStringIndex "
                                     "INNER JOIN Strings As tn ON tn.StringIndex=td.TableNameIndex "
                                     "INNER JOIN Strings As rn ON rn.StringIndex=td.RowNameIndex "
                                     "INNER JOIN Strings As cn ON cn.StringIndex=td.ColumnNameIndex "
                                     "INNER JOIN Strings As u ON u.StringIndex=td.UnitsIndex;";

    sqliteExecuteCommand(sql);
}

void SQLite::initializeIndexes()
{
    if (m_writeOutputToSQLite) {
        sqliteExecuteCommand("CREATE INDEX rddMTR ON ReportDataDictionary (IsMeter);");
        sqliteExecuteCommand("CREATE INDEX redRD ON ReportExtendedData (ReportDataIndex);");

        // These following indexes could potentially be used by sqlite, but for a narrow range of queries
        // There are no built in views that use these indexes in their queries.
        // sqliteExecuteCommand("CREATE INDEX dmhdHRI ON DaylightMapHourlyData (HourlyReportIndex);");
        // sqliteExecuteCommand("CREATE INDEX dmhrMNI ON DaylightMapHourlyReports (MapNumber);");

        // This following index is used by sqlite, but doesn't seem to increase performance in my testing
        // sqliteExecuteCommand("CREATE INDEX tdI ON TabularData (ReportNameIndex, ReportForStringIndex, TableNameIndex, RowNameIndex,
        // ColumnNameIndex, UnitsIndex, Value);");
    }
}

void SQLite::adjustReportingHourAndMinutes(int &hour, int &minutes)
{
    switch (minutes) {
    case 60:
        minutes = 0;
        break;
    default:
        --hour;
    }
}

void SQLite::parseUnitsAndDescription(std::string_view combinedString, std::string &units, std::string &description)
{
    std::size_t leftPos = combinedString.find("[");
    std::size_t rightPos = combinedString.find("]");

    if ((leftPos < rightPos) && (leftPos != std::string::npos) && (rightPos != std::string::npos)) {
        units = combinedString.substr(leftPos + 1, rightPos - leftPos - 1);
        description = combinedString.substr(0, leftPos - 1);
    } else {
        units = "";
        description = combinedString;
    }
}

int SQLite::logicalToInteger(const bool value)
{
    return value ? 1 : 0;
}

void SQLite::createSQLiteReportDictionaryRecord(int const reportVariableReportID,
                                                OutputProcessor::StoreType const storeType,
                                                std::string_view indexGroup,
                                                std::string_view keyedValueString,
                                                std::string_view const variableName,
                                                OutputProcessor::TimeStepType timeStepType,
                                                std::string_view units,
                                                OutputProcessor::ReportFreq const reportFreq,
                                                bool isMeter,
                                                std::string_view const scheduleName)
{
    static constexpr std::array<std::string_view, (int)OutputProcessor::ReportFreq::Num> reportFreqStrings = {
        "HVAC System Timestep", "Zone Timestep", "Hourly", "Daily", "Monthly", "Run Period", "Annual"};

    static constexpr std::array<std::string_view, (int)OutputProcessor::StoreType::Num> storeTypeStrings = {// "Dummy",
                                                                                                            "Avg",
                                                                                                            "Sum"};

    static constexpr std::array<std::string_view, (int)OutputProcessor::TimeStepType::Num> timeStepTypeStrings = {// "Dummy",
                                                                                                                  "Zone",
                                                                                                                  "HVAC System"};

    if (m_writeOutputToSQLite) {
        sqliteBindInteger(m_reportDictionaryInsertStmt, 1, reportVariableReportID);
        sqliteBindLogical(m_reportDictionaryInsertStmt, 2, isMeter);
        sqliteBindText(
            m_reportDictionaryInsertStmt, 3, (storeType == OutputProcessor::StoreType::Invalid) ? "Unknown!!!" : storeTypeStrings[(int)storeType]);
        sqliteBindText(m_reportDictionaryInsertStmt, 4, indexGroup);
        sqliteBindText(m_reportDictionaryInsertStmt,
                       5,
                       (timeStepType == OutputProcessor::TimeStepType::Invalid) ? "Unknown!!!" : timeStepTypeStrings[(int)timeStepType]);
        sqliteBindText(m_reportDictionaryInsertStmt, 6, keyedValueString);
        sqliteBindText(m_reportDictionaryInsertStmt, 7, variableName);
        sqliteBindText(m_reportDictionaryInsertStmt,
                       8,
                       (reportFreq == OutputProcessor::ReportFreq::Invalid) ? "Unknown!!!" : reportFreqStrings[(int)reportFreq]);

        if (!scheduleName.empty()) {
            sqliteBindText(m_reportDictionaryInsertStmt, 9, scheduleName);
        } else {
            sqliteBindNULL(m_reportDictionaryInsertStmt, 9);
        }

        sqliteBindText(m_reportDictionaryInsertStmt, 10, units);

        sqliteStepCommand(m_reportDictionaryInsertStmt);
        sqliteResetCommand(m_reportDictionaryInsertStmt);
    }
}

void SQLite::createSQLiteReportDataRecord(int const recordIndex,
                                          Real64 const value,
                                          OutputProcessor::ReportFreq const reportFreq,
                                          Real64 const minValue,
                                          int const minValueDate,
                                          Real64 const maxValue,
                                          int const maxValueDate,
                                          int const minutesPerTimeStep)
{

    if (!m_writeOutputToSQLite) {
        return;
    }

    ++m_dataIndex;

    sqliteBindInteger(m_reportDataInsertStmt, 1, m_dataIndex);
    sqliteBindForeignKey(m_reportDataInsertStmt, 2, m_sqlDBTimeIndex);
    sqliteBindForeignKey(m_reportDataInsertStmt, 3, recordIndex);
    sqliteBindDouble(m_reportDataInsertStmt, 4, value);

    sqliteStepCommand(m_reportDataInsertStmt);
    sqliteResetCommand(m_reportDataInsertStmt);

    if (minValueDate != -1 && maxValueDate != -1) {
        int minMonth;
        int minDay;
        int minHour;
        int minMinute;
        int maxMonth;
        int maxDay;
        int maxHour;
        int maxMinute;

        General::DecodeMonDayHrMin(minValueDate, minMonth, minDay, minHour, minMinute);
        General::DecodeMonDayHrMin(maxValueDate, maxMonth, maxDay, maxHour, maxMinute);

        adjustReportingHourAndMinutes(minHour, minMinute);
        adjustReportingHourAndMinutes(maxHour, maxMinute);

        ++m_extendedDataIndex;

        if (minutesPerTimeStep != -1) { // This is for data created by a 'Report Meter' statement
            switch (reportFreq) {
            case OutputProcessor::ReportFreq::Hour:
            case OutputProcessor::ReportFreq::Day:
            case OutputProcessor::ReportFreq::Month:
            case OutputProcessor::ReportFreq::Simulation:
            case OutputProcessor::ReportFreq::Year: {
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 1, m_extendedDataIndex);
                sqliteBindForeignKey(m_reportExtendedDataInsertStmt, 2, m_dataIndex);

                sqliteBindDouble(m_reportExtendedDataInsertStmt, 3, maxValue);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 4, maxMonth);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 5, maxDay);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 6, maxHour);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 7, maxMinute - minutesPerTimeStep + 1);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 8, maxMinute);

                sqliteBindDouble(m_reportExtendedDataInsertStmt, 9, minValue);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 10, minMonth);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 11, minDay);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 12, minHour);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 13, minMinute - minutesPerTimeStep + 1);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 14, minMinute);

                sqliteStepCommand(m_reportExtendedDataInsertStmt);
                sqliteResetCommand(m_reportExtendedDataInsertStmt);
            } break;

            case OutputProcessor::ReportFreq::TimeStep:
            case OutputProcessor::ReportFreq::EachCall: {
                --m_extendedDataIndex; // Reset the data index to account for the error
            } break;

            default: {
                --m_extendedDataIndex; // Reset the data index to account for the error
                std::stringstream ss;
                ss << "Illegal reportingInterval passed to CreateSQLiteMeterRecord: " << (int)reportFreq;
                sqliteWriteMessage(ss.str());
            } break;
            } // switch (reportFreq)

        } else { // This is for data created by a 'Report Variable' statement
            switch (reportFreq) {
            case OutputProcessor::ReportFreq::Day:
            case OutputProcessor::ReportFreq::Month:
            case OutputProcessor::ReportFreq::Simulation:
            case OutputProcessor::ReportFreq::Year: {
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 1, m_extendedDataIndex);
                sqliteBindForeignKey(m_reportExtendedDataInsertStmt, 2, m_dataIndex);

                sqliteBindDouble(m_reportExtendedDataInsertStmt, 3, maxValue);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 4, maxMonth);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 5, maxDay);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 6, maxHour);
                sqliteBindNULL(m_reportExtendedDataInsertStmt, 7);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 8, maxMinute);

                sqliteBindDouble(m_reportExtendedDataInsertStmt, 9, minValue);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 10, minMonth);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 11, minDay);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 12, minHour);
                sqliteBindNULL(m_reportExtendedDataInsertStmt, 13);
                sqliteBindInteger(m_reportExtendedDataInsertStmt, 14, minMinute);

                sqliteStepCommand(m_reportExtendedDataInsertStmt);
                sqliteResetCommand(m_reportExtendedDataInsertStmt);
            } break;

            case OutputProcessor::ReportFreq::TimeStep:
            case OutputProcessor::ReportFreq::EachCall:
            case OutputProcessor::ReportFreq::Hour: {
                --m_extendedDataIndex; // Reset the data index to account for the error
            } break;
            default: {
                --m_extendedDataIndex; // Reset the data index to account for the error
                std::stringstream ss;
                ss << "Illegal reportingInterval passed to CreateSQLiteMeterRecord: " << (int)reportFreq;
                sqliteWriteMessage(ss.str());
            } break;
            } // switch (reportFreq)
        } // if (minutesPerTimeStep != -1)
    } // if (minDataValue != 0)
} // SQLite::createSQLiteReportDataRecord()

void SQLite::createSQLiteTimeIndexRecord(OutputProcessor::ReportFreq const reportFreq,
                                         [[maybe_unused]] int const recordIndex,
                                         int const cumlativeSimulationDays,
                                         int const curEnvirNum,
                                         int const simulationYear,
                                         bool const curYearIsLeapYear,
                                         int const month,
                                         int const dayOfMonth,
                                         int const hour,
                                         Real64 const endMinute,
                                         Real64 const startMinute,
                                         int const dst,
                                         std::string_view const dayType,
                                         bool const warmupFlag)
{
    if (m_writeOutputToSQLite) {
        int intervalInMinutes = 60;

        static std::vector<int> lastDayOfMonth = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (curYearIsLeapYear) {
            lastDayOfMonth[1] = 29;
        }

        switch (reportFreq) {
        case OutputProcessor::ReportFreq::EachCall:
        case OutputProcessor::ReportFreq::TimeStep: {
            if (month == -1 || dayOfMonth == -1 || hour == -1 || endMinute == -1.0 || startMinute == -1.0 || dst == -1 || dayType == "") {
                sqliteWriteMessage("Empty month, dayOfMonth, hour, endMinute, startMinute, dst, or dayType passed to CreateSQLiteTimeIndexRecord");
                break;
            }
            ++m_sqlDBTimeIndex;

            int intEndMinute = static_cast<int>(endMinute + 0.5);
            int intStartMinute = static_cast<int>(startMinute + 0.5);
            int t_hour = hour;
            intervalInMinutes = intEndMinute - intStartMinute;
            adjustReportingHourAndMinutes(t_hour, intEndMinute);

            sqliteBindInteger(m_timeIndexInsertStmt, 1, m_sqlDBTimeIndex);
            sqliteBindInteger(m_timeIndexInsertStmt, 2, simulationYear);
            sqliteBindInteger(m_timeIndexInsertStmt, 3, month);
            sqliteBindInteger(m_timeIndexInsertStmt, 4, dayOfMonth);
            sqliteBindInteger(m_timeIndexInsertStmt, 5, t_hour);
            sqliteBindInteger(m_timeIndexInsertStmt, 6, intEndMinute);
            sqliteBindInteger(m_timeIndexInsertStmt, 7, dst);
            sqliteBindInteger(m_timeIndexInsertStmt, 8, intervalInMinutes);
            sqliteBindInteger(m_timeIndexInsertStmt, 9, reportFreqInts[(int)reportFreq]);
            sqliteBindInteger(m_timeIndexInsertStmt, 10, cumlativeSimulationDays);
            sqliteBindText(m_timeIndexInsertStmt, 11, dayType);
            sqliteBindInteger(m_timeIndexInsertStmt, 12, curEnvirNum);
            sqliteBindLogical(m_timeIndexInsertStmt, 13, warmupFlag);

            sqliteStepCommand(m_timeIndexInsertStmt);
            sqliteResetCommand(m_timeIndexInsertStmt);

            break;
        }
        case OutputProcessor::ReportFreq::Hour: {
            if (month == -1 || dayOfMonth == -1 || hour == -1 || dst == -1 || dayType == "") {
                sqliteWriteMessage("Empty month, dayOfMonth, hour, dst, or dayType passed to CreateSQLiteTimeIndexRecord");
                break;
            }
            ++m_sqlDBTimeIndex;

            sqliteBindInteger(m_timeIndexInsertStmt, 1, m_sqlDBTimeIndex);
            sqliteBindInteger(m_timeIndexInsertStmt, 2, simulationYear);
            sqliteBindInteger(m_timeIndexInsertStmt, 3, month);
            sqliteBindInteger(m_timeIndexInsertStmt, 4, dayOfMonth);
            sqliteBindInteger(m_timeIndexInsertStmt, 5, hour);
            sqliteBindInteger(m_timeIndexInsertStmt, 6, 0);
            sqliteBindInteger(m_timeIndexInsertStmt, 7, dst);
            sqliteBindInteger(m_timeIndexInsertStmt, 8, intervalInMinutes);
            sqliteBindInteger(m_timeIndexInsertStmt, 9, reportFreqInts[(int)reportFreq]);
            sqliteBindInteger(m_timeIndexInsertStmt, 10, cumlativeSimulationDays);
            sqliteBindText(m_timeIndexInsertStmt, 11, dayType);
            sqliteBindInteger(m_timeIndexInsertStmt, 12, curEnvirNum);

            sqliteStepCommand(m_timeIndexInsertStmt);
            sqliteResetCommand(m_timeIndexInsertStmt);

            break;
        }
        case OutputProcessor::ReportFreq::Day: {
            if (month == -1 || dayOfMonth == -1 || dst == -1 || dayType == "") {
                sqliteWriteMessage("Empty month, dayOfMonth, dst, or dayType passed to CreateSQLiteTimeIndexRecord");
                break;
            }
            ++m_sqlDBTimeIndex;

            intervalInMinutes = 60 * 24;
            sqliteBindInteger(m_timeIndexInsertStmt, 1, m_sqlDBTimeIndex);
            sqliteBindInteger(m_timeIndexInsertStmt, 2, simulationYear);
            sqliteBindInteger(m_timeIndexInsertStmt, 3, month);
            sqliteBindInteger(m_timeIndexInsertStmt, 4, dayOfMonth);
            sqliteBindInteger(m_timeIndexInsertStmt, 5, 24);
            sqliteBindInteger(m_timeIndexInsertStmt, 6, 0);
            sqliteBindInteger(m_timeIndexInsertStmt, 7, dst);
            sqliteBindInteger(m_timeIndexInsertStmt, 8, intervalInMinutes);
            sqliteBindInteger(m_timeIndexInsertStmt, 9, reportFreqInts[(int)reportFreq]);
            sqliteBindInteger(m_timeIndexInsertStmt, 10, cumlativeSimulationDays);
            sqliteBindText(m_timeIndexInsertStmt, 11, dayType);
            sqliteBindInteger(m_timeIndexInsertStmt, 12, curEnvirNum);

            sqliteStepCommand(m_timeIndexInsertStmt);
            sqliteResetCommand(m_timeIndexInsertStmt);

            break;
        }
        case OutputProcessor::ReportFreq::Month: {
            if (month == -1) {
                sqliteWriteMessage("Empty month passed to CreateSQLiteTimeIndexRecord");
                break;
            }
            ++m_sqlDBTimeIndex;

            intervalInMinutes = 60 * 24 * lastDayOfMonth[month - 1];
            sqliteBindInteger(m_timeIndexInsertStmt, 1, m_sqlDBTimeIndex);
            sqliteBindInteger(m_timeIndexInsertStmt, 2, simulationYear);
            sqliteBindInteger(m_timeIndexInsertStmt, 3, month);
            sqliteBindInteger(m_timeIndexInsertStmt, 4, lastDayOfMonth[month - 1]);
            sqliteBindInteger(m_timeIndexInsertStmt, 5, 24);
            sqliteBindInteger(m_timeIndexInsertStmt, 6, 0);
            sqliteBindNULL(m_timeIndexInsertStmt, 7);
            sqliteBindInteger(m_timeIndexInsertStmt, 8, intervalInMinutes);
            sqliteBindInteger(m_timeIndexInsertStmt, 9, reportFreqInts[(int)reportFreq]);
            sqliteBindInteger(m_timeIndexInsertStmt, 10, cumlativeSimulationDays);
            sqliteBindNULL(m_timeIndexInsertStmt, 11);
            sqliteBindInteger(m_timeIndexInsertStmt, 12, curEnvirNum);

            sqliteStepCommand(m_timeIndexInsertStmt);
            sqliteResetCommand(m_timeIndexInsertStmt);

            break;
        }
        case OutputProcessor::ReportFreq::Simulation: {
            ++m_sqlDBTimeIndex;

            intervalInMinutes = 60 * 24 * cumlativeSimulationDays;
            sqliteBindInteger(m_timeIndexInsertStmt, 1, m_sqlDBTimeIndex);
            sqliteBindNULL(m_timeIndexInsertStmt, 2);
            sqliteBindNULL(m_timeIndexInsertStmt, 3);
            sqliteBindNULL(m_timeIndexInsertStmt, 4);
            sqliteBindNULL(m_timeIndexInsertStmt, 5);
            sqliteBindNULL(m_timeIndexInsertStmt, 6);
            sqliteBindNULL(m_timeIndexInsertStmt, 7);
            sqliteBindInteger(m_timeIndexInsertStmt, 8, intervalInMinutes);
            sqliteBindInteger(m_timeIndexInsertStmt, 9, reportFreqInts[(int)reportFreq]);
            sqliteBindInteger(m_timeIndexInsertStmt, 10, cumlativeSimulationDays);
            sqliteBindNULL(m_timeIndexInsertStmt, 11);
            sqliteBindInteger(m_timeIndexInsertStmt, 12, curEnvirNum);

            sqliteStepCommand(m_timeIndexInsertStmt);
            sqliteResetCommand(m_timeIndexInsertStmt);

            break;
        }
        default: {
            std::stringstream ss;
            ss << "Illegal reportingInterval passed to CreateSQLiteTimeIndexRecord: " << (int)reportFreq;
            sqliteWriteMessage(ss.str());
        }
        }
    }
} // SQLite::createSQLiteTimeIndexRecord()

void SQLite::createYearlyTimeIndexRecord(int const simulationYear, int const curEnvirNum)
{
    if (m_writeOutputToSQLite) {

        ++m_sqlDBTimeIndex;

        sqliteBindInteger(m_timeIndexInsertStmt, 1, m_sqlDBTimeIndex);
        sqliteBindInteger(m_timeIndexInsertStmt, 2, simulationYear);
        sqliteBindNULL(m_timeIndexInsertStmt, 3);
        sqliteBindNULL(m_timeIndexInsertStmt, 4);
        sqliteBindNULL(m_timeIndexInsertStmt, 5);
        sqliteBindNULL(m_timeIndexInsertStmt, 6);
        sqliteBindNULL(m_timeIndexInsertStmt, 7);
        sqliteBindNULL(m_timeIndexInsertStmt, 8);
        sqliteBindInteger(m_timeIndexInsertStmt, 9, reportFreqInts[(int)OutputProcessor::ReportFreq::Year]);
        sqliteBindNULL(m_timeIndexInsertStmt, 10);
        sqliteBindNULL(m_timeIndexInsertStmt, 11);
        sqliteBindInteger(m_timeIndexInsertStmt, 12, curEnvirNum);

        sqliteStepCommand(m_timeIndexInsertStmt);
        sqliteResetCommand(m_timeIndexInsertStmt);
    }
}

void SQLite::addSQLiteZoneSizingRecord(std::string_view zoneName,   // the name of the zone
                                       std::string_view loadType,   // the description of the input variable
                                       Real64 const calcDesLoad,    // the value from the sizing calculation [W]
                                       Real64 const userDesLoad,    // the value from the sizing calculation modified by user input [W]
                                       Real64 const calcDesFlow,    // calculated design air flow rate [m3/s]
                                       Real64 const userDesFlow,    // user input or modified design air flow rate [m3/s]
                                       std::string_view desDayName, // the name of the design day that produced the peak
                                       std::string_view peakHrMin,  // time stamp of the peak
                                       Real64 const peakTemp,       // temperature at peak [C]
                                       Real64 const peakHumRat,     // humidity ratio at peak [kg water/kg dry air]
                                       Real64 const minOAVolFlow,   // zone design minimum outside air flow rate [m3/s]
                                       Real64 const DOASHeatAddRate // zone design heat addition rate from the DOAS [W]
)
{
    if (m_writeOutputToSQLite) {
        ++m_zoneSizingIndex;
        sqliteBindInteger(m_zoneSizingInsertStmt, 1, m_zoneSizingIndex);
        sqliteBindText(m_zoneSizingInsertStmt, 2, zoneName);
        sqliteBindText(m_zoneSizingInsertStmt, 3, loadType);

        sqliteBindDouble(m_zoneSizingInsertStmt, 4, calcDesLoad);
        sqliteBindDouble(m_zoneSizingInsertStmt, 5, userDesLoad);
        sqliteBindDouble(m_zoneSizingInsertStmt, 6, calcDesFlow);
        sqliteBindDouble(m_zoneSizingInsertStmt, 7, userDesFlow);

        sqliteBindText(m_zoneSizingInsertStmt, 8, desDayName);
        sqliteBindText(m_zoneSizingInsertStmt, 9, peakHrMin);

        sqliteBindDouble(m_zoneSizingInsertStmt, 10, peakTemp);
        sqliteBindDouble(m_zoneSizingInsertStmt, 11, peakHumRat);
        sqliteBindDouble(m_zoneSizingInsertStmt, 12, minOAVolFlow);
        sqliteBindDouble(m_zoneSizingInsertStmt, 13, DOASHeatAddRate);

        sqliteStepCommand(m_zoneSizingInsertStmt);
        sqliteResetCommand(m_zoneSizingInsertStmt);
    }
}

void SQLite::addSQLiteSystemSizingRecord(std::string_view SysName,      // the name of the system
                                         std::string_view LoadType,     // either "Cooling" or "Heating"
                                         std::string_view PeakLoadType, // either "Sensible" or "Total"
                                         Real64 const UserDesCap,       // User  Design Capacity
                                         Real64 const CalcDesVolFlow,   // Calculated Cooling Design Air Flow Rate
                                         Real64 const UserDesVolFlow,   // User Cooling Design Air Flow Rate
                                         std::string_view DesDayName,   // the name of the design day that produced the peak
                                         std::string_view PeakHrMin     // time stamp of the peak
)
{
    if (m_writeOutputToSQLite) {
        ++m_systemSizingIndex;
        sqliteBindInteger(m_systemSizingInsertStmt, 1, m_systemSizingIndex);
        sqliteBindText(m_systemSizingInsertStmt, 2, SysName);
        sqliteBindText(m_systemSizingInsertStmt, 3, LoadType);
        sqliteBindText(m_systemSizingInsertStmt, 4, PeakLoadType);

        sqliteBindDouble(m_systemSizingInsertStmt, 5, UserDesCap);
        sqliteBindDouble(m_systemSizingInsertStmt, 6, CalcDesVolFlow);
        sqliteBindDouble(m_systemSizingInsertStmt, 7, UserDesVolFlow);
        sqliteBindText(m_systemSizingInsertStmt, 8, DesDayName);
        sqliteBindText(m_systemSizingInsertStmt, 9, PeakHrMin);

        sqliteStepCommand(m_systemSizingInsertStmt);
        sqliteResetCommand(m_systemSizingInsertStmt);
    }
}

void SQLite::addSQLiteComponentSizingRecord(std::string_view compType, // the type of the component
                                            std::string_view compName, // the name of the component
                                            std::string_view varDesc,  // the description of the input variable
                                            Real64 const varValue      // the value from the sizing calculation
)
{
    if (m_writeOutputToSQLite) {
        ++m_componentSizingIndex;

        std::string description;
        std::string units;

        parseUnitsAndDescription(varDesc, units, description);

        sqliteBindInteger(m_componentSizingInsertStmt, 1, m_componentSizingIndex);
        sqliteBindText(m_componentSizingInsertStmt, 2, compType);
        sqliteBindText(m_componentSizingInsertStmt, 3, compName);
        sqliteBindText(m_componentSizingInsertStmt, 4, description);
        sqliteBindDouble(m_componentSizingInsertStmt, 5, varValue);
        sqliteBindText(m_componentSizingInsertStmt, 6, units);

        sqliteStepCommand(m_componentSizingInsertStmt);
        sqliteResetCommand(m_componentSizingInsertStmt);
    }
}

void SQLite::addSQLiteComponentSizingStrRecord(std::string_view compType, // the type of the component
                                               std::string_view compName, // the name of the component
                                               std::string_view varDesc,  // the description of the input variable
                                               std::string_view varValue  // the value from the sizing calculation
)
{
    if (m_writeOutputToSQLite) {
        ++m_componentSizingIndex;
        sqliteBindInteger(m_componentSizingInsertStmt, 1, m_componentSizingIndex);
        sqliteBindText(m_componentSizingInsertStmt, 2, compType);
        sqliteBindText(m_componentSizingInsertStmt, 3, compName);
        sqliteBindText(m_componentSizingInsertStmt, 4, varDesc);
        sqliteBindText(m_componentSizingInsertStmt, 7, varValue);

        sqliteStepCommand(m_componentSizingInsertStmt);
        sqliteResetCommand(m_componentSizingInsertStmt);
    }
}

void SQLite::createSQLiteDaylightMapTitle(
    int const mapNum, std::string_view mapName, std::string_view environmentName, int const zone, std::string_view refPts, Real64 const zCoord)
{
    if (m_writeOutputToSQLite) {
        // for some reason it is adding extra mapNumbers that are getting UNIQUE constraint ignored.
        // Might need to look into it, basically I think something is getting double inserted (12/06/14)
        sqliteBindInteger(m_daylightMapTitleInsertStmt, 1, mapNum);
        sqliteBindText(m_daylightMapTitleInsertStmt, 2, mapName);
        sqliteBindText(m_daylightMapTitleInsertStmt, 3, environmentName);
        sqliteBindForeignKey(m_daylightMapTitleInsertStmt, 4, zone);
        sqliteBindText(m_daylightMapTitleInsertStmt, 5, refPts);
        sqliteBindDouble(m_daylightMapTitleInsertStmt, 6, zCoord);

        sqliteStepCommand(m_daylightMapTitleInsertStmt);
        sqliteResetCommand(m_daylightMapTitleInsertStmt);
    }
}

void SQLite::createSQLiteDaylightMap(int const mapNum,
                                     int const year,
                                     int const month,
                                     int const dayOfMonth,
                                     int const hourOfDay,
                                     int const nX,
                                     Array1D<Real64> const &x,
                                     int const nY,
                                     Array1D<Real64> const &y,
                                     Array2<Real64> const &illuminance)
{
    if (m_writeOutputToSQLite) {
        ++m_hourlyReportIndex;
        int b = 0;
        sqliteBindInteger(m_daylightMapHourlyTitleInsertStmt, ++b, m_hourlyReportIndex);
        sqliteBindForeignKey(m_daylightMapHourlyTitleInsertStmt, ++b, mapNum);
        sqliteBindForeignKey(m_daylightMapHourlyTitleInsertStmt, ++b, year);
        sqliteBindInteger(m_daylightMapHourlyTitleInsertStmt, ++b, month);
        sqliteBindInteger(m_daylightMapHourlyTitleInsertStmt, ++b, dayOfMonth);
        sqliteBindInteger(m_daylightMapHourlyTitleInsertStmt, ++b, hourOfDay);

        sqliteStepCommand(m_daylightMapHourlyTitleInsertStmt);
        sqliteResetCommand(m_daylightMapHourlyTitleInsertStmt);

        for (int yIndex = 1; yIndex <= nY; ++yIndex) {
            for (int xIndex = 1; xIndex <= nX; ++xIndex) {
                ++m_hourlyDataIndex;
                sqliteBindInteger(m_daylightMapHourlyDataInsertStmt, 1, m_hourlyDataIndex);
                sqliteBindForeignKey(m_daylightMapHourlyDataInsertStmt, 2, m_hourlyReportIndex);
                sqliteBindDouble(m_daylightMapHourlyDataInsertStmt, 3, x(xIndex));
                sqliteBindDouble(m_daylightMapHourlyDataInsertStmt, 4, y(yIndex));
                sqliteBindDouble(m_daylightMapHourlyDataInsertStmt, 5, illuminance(xIndex, yIndex));

                sqliteStepCommand(m_daylightMapHourlyDataInsertStmt);
                sqliteResetCommand(m_daylightMapHourlyDataInsertStmt);
            }
        }
    }
}

void SQLite::createSQLiteTabularDataRecords(Array2D_string const &body, // html table row, html table column
                                            Array1D_string const &rowLabels,
                                            Array1D_string const &columnLabels,
                                            std::string_view reportName,
                                            std::string_view reportForString,
                                            std::string_view tableName)
{
    if (m_writeTabularDataToSQLite) {
        size_t sizeColumnLabels = columnLabels.size();
        size_t sizeRowLabels = rowLabels.size();

        int const reportNameIndex = createSQLiteStringTableRecord(reportName, ReportNameId);
        int const reportForStringIndex = createSQLiteStringTableRecord(reportForString, ReportForStringId);
        int const tableNameIndex = createSQLiteStringTableRecord(tableName, TableNameId);
        int unitsIndex;

        for (size_t iCol = 0, k = body.index(1, 1); iCol < sizeColumnLabels; ++iCol) {
            std::string colUnits;
            std::string colDescription;
            parseUnitsAndDescription(columnLabels[iCol], colUnits, colDescription);

            int const columnLabelIndex = createSQLiteStringTableRecord(colDescription, ColumnNameId);

            if (!colUnits.empty()) {
                unitsIndex = createSQLiteStringTableRecord(colUnits, UnitsId);
            }

            for (size_t iRow = 0; iRow < sizeRowLabels; ++iRow) {
                ++m_tabularDataIndex;
                std::string rowUnits;
                std::string rowDescription;
                parseUnitsAndDescription(rowLabels[iRow], rowUnits, rowDescription);

                int const rowLabelIndex = createSQLiteStringTableRecord(rowDescription, RowNameId);

                if (colUnits.empty()) {
                    unitsIndex = createSQLiteStringTableRecord(rowUnits, UnitsId);
                }

                sqliteBindInteger(m_tabularDataInsertStmt, 1, m_tabularDataIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 2, reportNameIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 3, reportForStringIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 4, tableNameIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 5, rowLabelIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 6, columnLabelIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 7, unitsIndex);
                sqliteBindForeignKey(m_tabularDataInsertStmt, 8, 1);
                sqliteBindInteger(m_tabularDataInsertStmt, 9, iRow);
                sqliteBindInteger(m_tabularDataInsertStmt, 10, iCol);
                sqliteBindText(m_tabularDataInsertStmt, 11, body[k]);

                sqliteStepCommand(m_tabularDataInsertStmt);
                sqliteResetCommand(m_tabularDataInsertStmt);

                ++k;
            }
        }
    }
}

int SQLite::createSQLiteStringTableRecord(std::string_view stringValue, int const stringType)
{
    int rowId = -1;
    if (m_writeOutputToSQLite) {

        auto ret = m_tabularStrings.emplace(make_pair(stringValue, stringType), 0);

        if (!ret.second) {
            rowId = ret.first->second;
        } else {
            sqliteBindInteger(m_stringsInsertStmt, 1, m_stringIndex);
            sqliteBindForeignKey(m_stringsInsertStmt, 2, stringType);
            sqliteBindText(m_stringsInsertStmt, 3, stringValue);

            int errorcode = sqliteStepCommand(m_stringsInsertStmt);
            sqliteResetCommand(m_stringsInsertStmt);

            if (errorcode != SQLITE_CONSTRAINT) {
                rowId = m_stringIndex++;
            } else {
                sqliteBindInteger(m_stringsLookUpStmt, 1, stringType);
                sqliteBindText(m_stringsLookUpStmt, 2, stringValue);
                sqliteStepCommand(m_stringsLookUpStmt);
                rowId = sqlite3_column_int(m_stringsLookUpStmt, 0);
                sqliteResetCommand(m_stringsLookUpStmt);
            }
            ret.first->second = rowId;
        }
    }
    return rowId;
}

void SQLite::createSQLiteSimulationsRecord(int const id, std::string_view verString, std::string_view currentDateTime)
{
    if (m_writeOutputToSQLite) {
        sqliteBindInteger(m_simulationsInsertStmt, 1, id);
        sqliteBindText(m_simulationsInsertStmt, 2, verString);
        sqliteBindText(m_simulationsInsertStmt, 3, currentDateTime);

        sqliteStepCommand(m_simulationsInsertStmt);
        sqliteResetCommand(m_simulationsInsertStmt);
    }
}

void SQLite::createSQLiteErrorRecord(int const simulationIndex, int const errorType, std::string_view errorMessage, int const cnt)
{
    if (m_writeOutputToSQLite) {
        ++m_errorIndex;

        sqliteBindInteger(m_errorInsertStmt, 1, m_errorIndex);
        sqliteBindForeignKey(m_errorInsertStmt, 2, simulationIndex);
        sqliteBindInteger(m_errorInsertStmt, 3, errorType);
        sqliteBindText(m_errorInsertStmt, 4, errorMessage);
        sqliteBindInteger(m_errorInsertStmt, 5, cnt);

        sqliteStepCommand(m_errorInsertStmt);
        sqliteResetCommand(m_errorInsertStmt);
    }
}

void SQLite::updateSQLiteErrorRecord(std::string const &errorMessage)
{
    if (m_writeOutputToSQLite) {
        sqliteBindText(m_errorUpdateStmt, 1, "  " + errorMessage);

        sqliteStepCommand(m_errorUpdateStmt);
        sqliteResetCommand(m_errorUpdateStmt);
    }
}

void SQLite::updateSQLiteSimulationRecord(int const id, int const numOfTimeStepInHour)
{
    if (m_writeOutputToSQLite) {
        sqliteBindInteger(m_simulationDataUpdateStmt, 1, numOfTimeStepInHour);
        sqliteBindForeignKey(m_simulationDataUpdateStmt, 2, id);

        sqliteStepCommand(m_simulationDataUpdateStmt);
        sqliteResetCommand(m_simulationDataUpdateStmt);
    }
}

void SQLite::updateSQLiteSimulationRecord(bool const completed, bool const completedSuccessfully, int const id)
{
    if (m_writeOutputToSQLite) {
        sqliteBindLogical(m_simulationUpdateStmt, 1, completed);
        sqliteBindLogical(m_simulationUpdateStmt, 2, completedSuccessfully);
        sqliteBindForeignKey(m_simulationUpdateStmt, 3, id); // seems to always be 1, SimulationManager::ManageSimulation()

        sqliteStepCommand(m_simulationUpdateStmt);
        sqliteResetCommand(m_simulationUpdateStmt);
    }
}

void SQLite::createZoneExtendedOutput()
{
    if (m_writeOutputToSQLite) {
        for (auto const &zone : zones) {
            zone->insertIntoSQLite(m_zoneInfoInsertStmt);
        }
        for (auto const &zoneList : zoneLists) {
            zoneList->insertIntoSQLite(m_zoneListInsertStmt, m_zoneInfoZoneListInsertStmt);
        }
        for (auto const &zoneGroup : zoneGroups) {
            zoneGroup->insertIntoSQLite(m_zoneGroupInsertStmt);
        }
        for (auto const &schedule : schedules) {
            schedule->insertIntoSQLite(m_scheduleInsertStmt);
        }
        for (auto const &material : materials) {
            material->insertIntoSQLite(m_materialInsertStmt);
        }
        for (auto const &construction : constructions) {
            construction->insertIntoSQLite(m_constructionInsertStmt, m_constructionLayerInsertStmt);
        }
        for (auto const &surface : surfaces) {
            surface->insertIntoSQLite(m_surfaceInsertStmt);
        }
        for (auto const &nominalLighting : nominalLightings) {
            nominalLighting->insertIntoSQLite(m_nominalLightingInsertStmt);
        }
        for (auto const &nominalPeople : nominalPeoples) {
            nominalPeople->insertIntoSQLite(m_nominalPeopleInsertStmt);
        }
        for (auto const &nominalElectricEquipment : nominalElectricEquipments) {
            nominalElectricEquipment->insertIntoSQLite(m_nominalElectricEquipmentInsertStmt);
        }
        for (auto const &nominalGasEquipment : nominalGasEquipments) {
            nominalGasEquipment->insertIntoSQLite(m_nominalGasEquipmentInsertStmt);
        }
        for (auto const &nominalSteamEquipment : nominalSteamEquipments) {
            nominalSteamEquipment->insertIntoSQLite(m_nominalSteamEquipmentInsertStmt);
        }
        for (auto const &nominalHotWaterEquipment : nominalHotWaterEquipments) {
            nominalHotWaterEquipment->insertIntoSQLite(m_nominalHotWaterEquipmentInsertStmt);
        }
        for (auto const &nominalOtherEquipment : nominalOtherEquipments) {
            nominalOtherEquipment->insertIntoSQLite(m_nominalOtherEquipmentInsertStmt);
        }
        for (auto const &nominalBaseboardHeat : nominalBaseboardHeats) {
            nominalBaseboardHeat->insertIntoSQLite(m_nominalBaseboardHeatInsertStmt);
        }
        for (auto const &infiltration : infiltrations) {
            infiltration->insertIntoSQLite(m_infiltrationInsertStmt);
        }
        for (auto const &ventilation : ventilations) {
            ventilation->insertIntoSQLite(m_ventilationInsertStmt);
        }
        for (auto const &roomAirModel : roomAirModels) {
            roomAirModel->insertIntoSQLite(m_roomAirModelInsertStmt);
        }
    }
}

void SQLite::createSQLiteEnvironmentPeriodRecord(const int curEnvirNum,
                                                 std::string_view environmentName,
                                                 const Constant::KindOfSim kindOfSim,
                                                 const int simulationIndex)
{
    if (m_writeOutputToSQLite) {
        sqliteBindInteger(m_environmentPeriodInsertStmt, 1, curEnvirNum);
        sqliteBindForeignKey(m_environmentPeriodInsertStmt, 2, simulationIndex);
        sqliteBindText(m_environmentPeriodInsertStmt, 3, environmentName);
        sqliteBindInteger(m_environmentPeriodInsertStmt, 4, static_cast<int>(kindOfSim));

        sqliteStepCommand(m_environmentPeriodInsertStmt);
        sqliteResetCommand(m_environmentPeriodInsertStmt);
    }
}

void SQLite::addScheduleData(int const number, std::string_view name, std::string_view type, double const minValue, double const maxValue)
{
    schedules.push_back(std::make_unique<Schedule>(m_errorStream, m_db, number, name, type, minValue, maxValue));
}

void SQLite::addZoneData(int const number, DataHeatBalance::ZoneData const &zoneData)
{
    zones.push_back(std::make_unique<Zone>(m_errorStream, m_db, number, zoneData));
}

void SQLite::addZoneListData(int const number, DataHeatBalance::ZoneListData const &zoneListData)
{
    zoneLists.push_back(std::make_unique<ZoneList>(m_errorStream, m_db, number, zoneListData));
}

void SQLite::addSurfaceData(int const number, DataSurfaces::SurfaceData const &surfaceData, std::string_view surfaceClass)
{
    surfaces.push_back(std::make_unique<Surface>(m_errorStream, m_db, number, surfaceData, surfaceClass));
}

void SQLite::addZoneGroupData(int const number, DataHeatBalance::ZoneGroupData const &zoneGroupData)
{
    zoneGroups.push_back(std::make_unique<ZoneGroup>(m_errorStream, m_db, number, zoneGroupData));
}

void SQLite::addMaterialData(int const number, EnergyPlus::Material::MaterialBase const *materialData)
{
    materials.push_back(std::make_unique<Material>(m_errorStream, m_db, number, materialData));
}
void SQLite::addConstructionData(int const number,
                                 EnergyPlus::Construction::ConstructionProps const &constructionData,
                                 double const &constructionUValue)
{
    constructions.push_back(std::make_unique<Construction>(m_errorStream, m_db, number, constructionData, constructionUValue));
}
void SQLite::addNominalLightingData(int const number, DataHeatBalance::LightsData const &nominalLightingData)
{
    nominalLightings.push_back(std::make_unique<NominalLighting>(m_errorStream, m_db, number, nominalLightingData));
}
void SQLite::addNominalPeopleData(int const number, DataHeatBalance::PeopleData const &nominalPeopleData)
{
    nominalPeoples.push_back(std::make_unique<NominalPeople>(m_errorStream, m_db, number, nominalPeopleData));
}
void SQLite::addNominalElectricEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalElectricEquipmentData)
{
    nominalElectricEquipments.push_back(std::make_unique<NominalElectricEquipment>(m_errorStream, m_db, number, nominalElectricEquipmentData));
}
void SQLite::addNominalGasEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalGasEquipmentData)
{
    nominalGasEquipments.push_back(std::make_unique<NominalGasEquipment>(m_errorStream, m_db, number, nominalGasEquipmentData));
}
void SQLite::addNominalSteamEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalSteamEquipmentData)
{
    nominalSteamEquipments.push_back(std::make_unique<NominalSteamEquipment>(m_errorStream, m_db, number, nominalSteamEquipmentData));
}
void SQLite::addNominalHotWaterEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalHotWaterEquipmentData)
{
    nominalHotWaterEquipments.push_back(std::make_unique<NominalHotWaterEquipment>(m_errorStream, m_db, number, nominalHotWaterEquipmentData));
}
void SQLite::addNominalOtherEquipmentData(int const number, DataHeatBalance::ZoneEquipData const &nominalOtherEquipmentData)
{
    nominalOtherEquipments.push_back(std::make_unique<NominalOtherEquipment>(m_errorStream, m_db, number, nominalOtherEquipmentData));
}
void SQLite::addNominalBaseboardData(int const number, DataHeatBalance::BBHeatData const &nominalBaseboardData)
{
    nominalBaseboardHeats.push_back(std::make_unique<NominalBaseboardHeat>(m_errorStream, m_db, number, nominalBaseboardData));
}
void SQLite::addInfiltrationData(int const number, DataHeatBalance::InfiltrationData const &infiltrationData)
{
    infiltrations.push_back(std::make_unique<Infiltration>(m_errorStream, m_db, number, infiltrationData));
}
void SQLite::addVentilationData(int const number, DataHeatBalance::VentilationData const &ventilationData)
{
    ventilations.push_back(std::make_unique<Ventilation>(m_errorStream, m_db, number, ventilationData));
}
void SQLite::addRoomAirModelData(int const number, RoomAir::AirModelData const &roomAirModelData)
{
    roomAirModels.push_back(std::make_unique<RoomAirModel>(m_errorStream, m_db, number, roomAirModelData));
}

bool SQLite::ZoneGroup::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zoneList);
    sqliteBindInteger(insertStmt, 4, multiplier);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::Material::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    double isoMoistCap = 0.0;
    double thermGradCoef = 0.0;
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindInteger(insertStmt, 3, static_cast<int>(group));
    sqliteBindInteger(insertStmt, 4, static_cast<int>(roughness));
    sqliteBindDouble(insertStmt, 5, conductivity);
    sqliteBindDouble(insertStmt, 6, density);
    sqliteBindDouble(insertStmt, 7, isoMoistCap);
    sqliteBindDouble(insertStmt, 8, porosity);
    sqliteBindDouble(insertStmt, 9, resistance);
    sqliteBindLogical(insertStmt, 10, rOnly);
    sqliteBindDouble(insertStmt, 11, specHeat);
    sqliteBindDouble(insertStmt, 12, thermGradCoef);
    sqliteBindDouble(insertStmt, 13, thickness);
    sqliteBindDouble(insertStmt, 14, vaporDiffus);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::Construction::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindInteger(insertStmt, 3, totLayers);
    sqliteBindInteger(insertStmt, 4, totSolidLayers);
    sqliteBindInteger(insertStmt, 5, totGlassLayers);
    sqliteBindDouble(insertStmt, 6, insideAbsorpVis);
    sqliteBindDouble(insertStmt, 7, outsideAbsorpVis);
    sqliteBindDouble(insertStmt, 8, insideAbsorpSolar);
    sqliteBindDouble(insertStmt, 9, outsideAbsorpSolar);
    sqliteBindDouble(insertStmt, 10, insideAbsorpThermal);
    sqliteBindDouble(insertStmt, 11, outsideAbsorpThermal);
    sqliteBindInteger(insertStmt, 12, static_cast<int>(outsideRoughness));
    sqliteBindLogical(insertStmt, 13, typeIsWindow);
    sqliteBindDouble(insertStmt, 14, uValue);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::Construction::insertIntoSQLite(sqlite3_stmt *insertStmt, sqlite3_stmt *subInsertStmt)
{
    bool constructionInsertValid = insertIntoSQLite(insertStmt);
    if (!constructionInsertValid) {
        return false;
    }

    bool valid = true;
    for (auto const &constructionLayer : constructionLayers) {
        bool validInsert = constructionLayer->insertIntoSQLite(subInsertStmt);
        if (valid && !validInsert) {
            valid = false;
        }
    }
    return valid;
}
bool SQLite::Construction::ConstructionLayer::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindForeignKey(insertStmt, 1, constructNumber);
    sqliteBindInteger(insertStmt, 2, layerNumber);
    sqliteBindForeignKey(insertStmt, 3, layerPoint);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalLighting::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);
    sqliteBindDouble(insertStmt, 6, fractionReturnAir);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionShortWave);
    sqliteBindDouble(insertStmt, 9, fractionReplaceable);
    sqliteBindDouble(insertStmt, 10, fractionConvected);
    sqliteBindText(insertStmt, 11, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalPeople::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindDouble(insertStmt, 4, numberOfPeople);
    sqliteBindForeignKey(insertStmt, 5, numberOfPeopleSched ? numberOfPeopleSched->Num : -1);
    sqliteBindForeignKey(insertStmt, 6, activityLevelSched ? activityLevelSched->Num : -1);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionConvected);
    sqliteBindForeignKey(insertStmt, 9, workEffSched ? workEffSched->Num : -1);
    sqliteBindForeignKey(insertStmt, 10, clothingSched ? clothingSched->Num : -1);
    sqliteBindForeignKey(insertStmt, 11, airVelocitySched ? airVelocitySched->Num : -1);
    sqliteBindLogical(insertStmt, 12, fanger);
    sqliteBindLogical(insertStmt, 13, pierce);
    sqliteBindLogical(insertStmt, 14, ksu);
    sqliteBindInteger(insertStmt, 15, static_cast<int>(mrtCalcType));
    sqliteBindForeignKey(insertStmt, 16, surfacePtr);
    sqliteBindText(insertStmt, 17, angleFactorListName);
    sqliteBindInteger(insertStmt, 18, angleFactorListPtr);
    sqliteBindDouble(insertStmt, 19, userSpecSensFrac);
    sqliteBindLogical(insertStmt, 20, show55Warning);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalElectricEquipment::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);
    sqliteBindDouble(insertStmt, 6, fractionLatent);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionLost);
    sqliteBindDouble(insertStmt, 9, fractionConvected);
    sqliteBindText(insertStmt, 10, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalGasEquipment::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);
    sqliteBindDouble(insertStmt, 6, fractionLatent);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionLost);
    sqliteBindDouble(insertStmt, 9, fractionConvected);
    sqliteBindText(insertStmt, 10, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalSteamEquipment::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);
    sqliteBindDouble(insertStmt, 6, fractionLatent);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionLost);
    sqliteBindDouble(insertStmt, 9, fractionConvected);
    sqliteBindText(insertStmt, 10, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalHotWaterEquipment::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);
    sqliteBindDouble(insertStmt, 6, fractionLatent);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionLost);
    sqliteBindDouble(insertStmt, 9, fractionConvected);
    sqliteBindText(insertStmt, 10, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalOtherEquipment::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);
    sqliteBindDouble(insertStmt, 6, fractionLatent);
    sqliteBindDouble(insertStmt, 7, fractionRadiant);
    sqliteBindDouble(insertStmt, 8, fractionLost);
    sqliteBindDouble(insertStmt, 9, fractionConvected);
    sqliteBindText(insertStmt, 10, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::NominalBaseboardHeat::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, capatLowTemperature);
    sqliteBindDouble(insertStmt, 6, lowTemperature);
    sqliteBindDouble(insertStmt, 7, capatHighTemperature);
    sqliteBindDouble(insertStmt, 8, highTemperature);
    sqliteBindDouble(insertStmt, 9, fractionRadiant);
    sqliteBindDouble(insertStmt, 10, fractionConvected);
    sqliteBindText(insertStmt, 11, endUseSubcategory);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::Infiltration::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::Ventilation::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, zonePtr);
    sqliteBindForeignKey(insertStmt, 4, sched->Num);
    sqliteBindDouble(insertStmt, 5, designLevel);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}
bool SQLite::RoomAirModel::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, airModelName);
    sqliteBindInteger(insertStmt, 3, static_cast<int>(airModel));
    sqliteBindInteger(insertStmt, 4, static_cast<int>(tempCoupleScheme));
    sqliteBindLogical(insertStmt, 5, simAirModel);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}

bool SQLite::Surface::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindForeignKey(insertStmt, 3, construction);
    sqliteBindText(insertStmt, 4, surfaceClass);
    sqliteBindDouble(insertStmt, 5, area);
    sqliteBindDouble(insertStmt, 6, grossArea);
    sqliteBindDouble(insertStmt, 7, perimeter);
    sqliteBindDouble(insertStmt, 8, azimuth);
    sqliteBindDouble(insertStmt, 9, height);
    sqliteBindDouble(insertStmt, 10, reveal);
    sqliteBindInteger(insertStmt, 11, static_cast<int>(shape));
    sqliteBindInteger(insertStmt, 12, sides);
    sqliteBindDouble(insertStmt, 13, tilt);
    sqliteBindDouble(insertStmt, 14, width);
    sqliteBindLogical(insertStmt, 15, heatTransSurf);
    sqliteBindForeignKey(insertStmt, 16, baseSurf);
    sqliteBindForeignKey(insertStmt, 17, zone);
    sqliteBindInteger(insertStmt, 18, extBoundCond);
    sqliteBindLogical(insertStmt, 19, extSolar);
    sqliteBindLogical(insertStmt, 20, extWind);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}

bool SQLite::ZoneList::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}

bool SQLite::ZoneList::insertIntoSQLite(sqlite3_stmt *insertStmt, sqlite3_stmt *subInsertStmt)
{
    bool zoneListInsertValid = insertIntoSQLite(insertStmt);
    if (!zoneListInsertValid) {
        return false;
    }
    bool valid = true;
    for (size_t i = 1; i <= zones.size(); ++i) {
        sqliteBindForeignKey(subInsertStmt, 1, number);
        sqliteBindForeignKey(subInsertStmt, 2, zones(i));
        int rc = sqliteStepCommand(subInsertStmt);
        bool validInsert = sqliteStepValidity(rc);
        sqliteResetCommand(subInsertStmt);
        if (valid && !validInsert) {
            valid = false;
        }
    }
    return valid;
}

bool SQLite::Schedule::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindText(insertStmt, 3, type);
    sqliteBindDouble(insertStmt, 4, minValue);
    sqliteBindDouble(insertStmt, 5, maxValue);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}

bool SQLite::Zone::insertIntoSQLite(sqlite3_stmt *insertStmt)
{
    sqliteBindInteger(insertStmt, 1, number);
    sqliteBindText(insertStmt, 2, name);
    sqliteBindDouble(insertStmt, 3, relNorth);
    sqliteBindDouble(insertStmt, 4, originX);
    sqliteBindDouble(insertStmt, 5, originY);
    sqliteBindDouble(insertStmt, 6, originZ);
    sqliteBindDouble(insertStmt, 7, centroidX);
    sqliteBindDouble(insertStmt, 8, centroidY);
    sqliteBindDouble(insertStmt, 9, centroidZ);
    sqliteBindInteger(insertStmt, 10, ofType);
    sqliteBindInteger(insertStmt, 11, multiplier);
    sqliteBindInteger(insertStmt, 12, listMultiplier);
    sqliteBindDouble(insertStmt, 13, minimumX);
    sqliteBindDouble(insertStmt, 14, maximumX);
    sqliteBindDouble(insertStmt, 15, minimumY);
    sqliteBindDouble(insertStmt, 16, maximumY);
    sqliteBindDouble(insertStmt, 17, minimumZ);
    sqliteBindDouble(insertStmt, 18, maximumZ);
    sqliteBindDouble(insertStmt, 19, ceilingHeight);
    sqliteBindDouble(insertStmt, 20, volume);
    sqliteBindInteger(insertStmt, 21, Convect::HcIntReportVals[static_cast<int>(insideConvectionAlgo)]);
    sqliteBindInteger(insertStmt, 22, Convect::HcExtReportVals[static_cast<int>(outsideConvectionAlgo)]);
    sqliteBindDouble(insertStmt, 23, floorArea);
    sqliteBindDouble(insertStmt, 24, extGrossWallArea);
    sqliteBindDouble(insertStmt, 25, extNetWallArea);
    sqliteBindDouble(insertStmt, 26, extWindowArea);
    sqliteBindLogical(insertStmt, 27, isPartOfTotalArea);

    int rc = sqliteStepCommand(insertStmt);
    bool validInsert = sqliteStepValidity(rc);
    sqliteResetCommand(insertStmt);
    return validInsert;
}

SQLite::SQLiteData::SQLiteData(std::shared_ptr<std::ostream> const &errorStream, std::shared_ptr<sqlite3> const &db)
    : SQLiteProcedures(errorStream, db)
{
}

SQLiteProcedures::SQLiteProcedures(std::shared_ptr<std::ostream> const &errorStream, std::shared_ptr<sqlite3> const &db)
    : m_writeOutputToSQLite(true), m_errorStream(errorStream), m_db(db)
{
}

SQLiteProcedures::SQLiteProcedures(std::shared_ptr<std::ostream> const &errorStream,
                                   bool writeOutputToSQLite,
                                   fs::path const &dbName,
                                   fs::path const &errorFilePath)
    : m_writeOutputToSQLite(writeOutputToSQLite), m_errorStream(errorStream)
{
    sqlite3 *m_connection = nullptr;
    if (m_writeOutputToSQLite) {
        int rc;
        bool ok = true;

        std::string const dbName_utf8 = FileSystem::toGenericString(dbName);

        // Test if we can write to the sqlite error file
        //  Does there need to be a separate sqlite.err file at all?  Consider using eplusout.err
        if (m_errorStream) {
            *m_errorStream << "SQLite3 message, " << FileSystem::toGenericString(errorFilePath) << " open for processing!" << std::endl;
        } else {
            ok = false;
        }

        // Test if we can create a new file named dbName
        if (ok && dbName != ":memory:") {
            std::ofstream test(dbName, std::ofstream::out | std::ofstream::trunc);
            if (test.is_open()) {
                test.close();
            } else {
                ok = false;
            }
        }

        // Test if we can write to the database
        // If we can't then there are probably locks on the database
        if (ok) {
            // sqlite3_open_v2 could return SQLITE_BUSY at this point. If so, do not proceed to sqlite3_exec.
            rc = sqlite3_open_v2(dbName_utf8.c_str(), &m_connection, SQLITE_OPEN_READWRITE, nullptr);
            if (rc) {
                *m_errorStream << "SQLite3 message, can't get exclusive lock to open database: " << sqlite3_errmsg(m_connection) << std::endl;
                ok = false;
            }
        }

        if (ok) {
            char *zErrMsg = nullptr;
            // Set journal_mode OFF to avoid creating the file dbName + "-journal" (when dbName is a regular file)
            rc = sqlite3_exec(m_connection, "PRAGMA journal_mode = OFF;", nullptr, 0, &zErrMsg);
            if (!rc) {
                rc = sqlite3_exec(m_connection, "CREATE TABLE Test(x INTEGER PRIMARY KEY)", nullptr, 0, &zErrMsg);
            }
            sqlite3_close(m_connection);
            if (rc) {
                *m_errorStream << "SQLite3 message, can't get exclusive lock to edit database: " << zErrMsg << std::endl;
                ok = false;
            } else {
                if (dbName != ":memory:") {
                    // Remove test db
                    // rc = remove(dbName_utf8.c_str());
                    if (fs::is_regular_file(dbName)) {
                        std::error_code ec;
                        if (!fs::remove(dbName, ec)) {
                            // File operation failed. SQLite connection is not in an error state.
                            *m_errorStream << "SQLite3 message, can't remove old database. code=" << ec.value() << ", error: " << ec.message()
                                           << std::endl;
                            ok = false;
                        }
                    }
                }
            }
            sqlite3_free(zErrMsg);
        }

        if (ok) {
            // Now open the output db for the duration of the simulation
            rc = sqlite3_open_v2(dbName_utf8.c_str(), &m_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
            m_db = std::shared_ptr<sqlite3>(m_connection, sqlite3_close);
            if (rc) {
                *m_errorStream << "SQLite3 message, can't open new database: " << sqlite3_errmsg(m_connection) << std::endl;
                ok = false;
            }
        }

        if (!ok) {
            throw std::runtime_error("The SQLite database failed to open.");
        }
    }
}

int SQLiteProcedures::sqliteExecuteCommand(std::string_view commandBuffer)
{
    char *zErrMsg = 0;

    int rc = sqlite3_exec(m_db.get(), commandBuffer.data(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        *m_errorStream << zErrMsg;
    }
    sqlite3_free(zErrMsg);

    return rc;
}

int SQLiteProcedures::sqlitePrepareStatement(sqlite3_stmt *&stmt, std::string_view stmtBuffer)
{
    int rc = sqlite3_prepare_v2(m_db.get(), stmtBuffer.data(), stmtBuffer.size(), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        *m_errorStream << "SQLite3 message, sqlite3_prepare_v2 message:\n"
                       << stmtBuffer << "\n"
                       << sqlite3_errstr(rc) << "\n"
                       << sqlite3_errmsg(m_db.get()) << std::endl;
    }

    return rc;
}

int SQLiteProcedures::sqliteBindText(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, std::string_view textBuffer)
{
    int rc = sqlite3_bind_text(stmt, stmtInsertLocationIndex, textBuffer.data(), textBuffer.size(), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        *m_errorStream << "SQLite3 message, sqlite3_bind_text failed:\n"
                       << textBuffer << "\n"
                       << sqlite3_errstr(rc) << "\n"
                       << sqlite3_errmsg(m_db.get()) << std::endl;
    }

    return rc;
}

int SQLiteProcedures::sqliteBindInteger(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const int intToInsert)
{
    int rc = sqlite3_bind_int(stmt, stmtInsertLocationIndex, intToInsert);
    if (rc != SQLITE_OK) {
        *m_errorStream << "SQLite3 message, sqlite3_bind_int failed: " << intToInsert << std::endl;
    }

    return rc;
}

int SQLiteProcedures::sqliteBindDouble(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const double doubleToInsert)
{
    int rc = sqlite3_bind_double(stmt, stmtInsertLocationIndex, doubleToInsert);
    if (rc != SQLITE_OK) {
        *m_errorStream << "SQLite3 message, sqlite3_bind_double failed: " << doubleToInsert << std::endl;
    }

    return rc;
}

int SQLiteProcedures::sqliteBindNULL(sqlite3_stmt *stmt, const int stmtInsertLocationIndex)
{
    int rc = sqlite3_bind_null(stmt, stmtInsertLocationIndex);
    if (rc != SQLITE_OK) {
        *m_errorStream << "SQLite3 message, sqlite3_bind_null failed" << std::endl;
    }

    return rc;
}

int SQLiteProcedures::sqliteBindForeignKey(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const int intToInsert)
{
    int rc = -1;
    if (intToInsert > 0) {
        rc = sqlite3_bind_int(stmt, stmtInsertLocationIndex, intToInsert);
    } else {
        rc = sqlite3_bind_null(stmt, stmtInsertLocationIndex);
    }
    if (rc != SQLITE_OK) {
        *m_errorStream << "SQLite3 message, sqliteBindForeignKey failed: " << intToInsert << std::endl;
    }

    return rc;
}

int SQLiteProcedures::sqliteBindLogical(sqlite3_stmt *stmt, const int stmtInsertLocationIndex, const bool valueToInsert)
{
    return sqliteBindInteger(stmt, stmtInsertLocationIndex, valueToInsert ? 1 : 0);
}

bool SQLiteProcedures::sqliteStepValidity(int const rc)
{
    bool isValid = false;
    switch (rc) {
    case SQLITE_DONE:
    case SQLITE_OK:
    case SQLITE_ROW:
        isValid = true;
        break;
    default:
        break;
    }
    return isValid;
}

int SQLiteProcedures::sqliteStepCommand(sqlite3_stmt *stmt)
{
    int rc = sqlite3_step(stmt);
    switch (rc) {
    case SQLITE_DONE:
    case SQLITE_OK:
    case SQLITE_ROW:
        break;
    default:
        *m_errorStream << "SQLite3 message, sqlite3_step message: " << sqlite3_errmsg(m_db.get()) << std::endl;
        break;
    }

    return rc;
}

int SQLiteProcedures::sqliteResetCommand(sqlite3_stmt *stmt)
{
    return sqlite3_reset(stmt);
}

bool SQLiteProcedures::sqliteWithinTransaction()
{
    return (sqlite3_get_autocommit(m_db.get()) == 0);
}

// int SQLiteProcedures::sqliteClearBindings(sqlite3_stmt * stmt)
// {
//     return sqlite3_clear_bindings(stmt);
// }

// int SQLiteProcedures::sqliteFinalizeCommand(sqlite3_stmt * stmt)
// {
//     return sqlite3_finalize(stmt);
// }

} // namespace EnergyPlus
