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

#ifndef SQLiteFixture_hh_INCLUDED
#define SQLiteFixture_hh_INCLUDED

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/SQLiteProcedures.hh>

namespace EnergyPlus {

class SQLiteFixture : public EnergyPlusFixture
{

protected:
    //    static void SetUpTestCase()
    //    {
    //        EnergyPlusFixture::SetUpTestCase();
    //    }
    static void TearDownTestCase()
    {
    }

    std::shared_ptr<std::ostringstream> ss;

    virtual void SetUp()
    {
        EnergyPlusFixture::SetUp(); // Sets up the base fixture first.
        ss = std::make_shared<std::ostringstream>();
        // Make sure CreateSQLiteDatabase() is not called anywhere during the test in EP code
        // By default, set the SQLite location to memory
        std::string sql_filepath(":memory:");
        // Alternatively, when debugging a test, change outpath to the location on disk so you can query the database manually after test ran
        // std::string sql_filepath("/path/on/disk/to/eplusout.sql");
        ASSERT_NO_THROW(state->dataSQLiteProcedures->sqlite = std::make_unique<SQLite>(ss, sql_filepath, "std::ostringstream", true, true));
        ASSERT_TRUE(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite());
        ASSERT_TRUE(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite());
        state->dataSQLiteProcedures->sqlite->sqliteExecuteCommand("PRAGMA foreign_keys = ON;");
        EXPECT_EQ("SQLite3 message, std::ostringstream open for processing!\n", ss->str());
        ss->str(std::string());
    }

    virtual void TearDown()
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }

    void resetDatabase()
    {
        ss = std::make_shared<std::ostringstream>();
        ASSERT_NO_THROW(state->dataSQLiteProcedures->sqlite = std::make_unique<SQLite>(ss, ":memory:", "std::ostringstream", true, true));
        ASSERT_TRUE(state->dataSQLiteProcedures->sqlite->writeOutputToSQLite());
        ASSERT_TRUE(state->dataSQLiteProcedures->sqlite->writeTabularDataToSQLite());
        state->dataSQLiteProcedures->sqlite->sqliteExecuteCommand("PRAGMA foreign_keys = ON;");
        EXPECT_EQ("SQLite3 message, std::ostringstream open for processing!\n", ss->str());
        ss->str(std::string());
    }

#ifdef GET_OUT
    std::string storageType(const int storageTypeIndex)
    {
        return state->dataSQLiteProcedures->sqlite->storageType(storageTypeIndex);
    }

    std::string timestepTypeName(const int timestepType)
    {
        return state->dataSQLiteProcedures->sqlite->timestepTypeName(timestepType);
    }

    std::string reportingFreqName(const int reportingFreqIndex)
    {
        return state->dataSQLiteProcedures->sqlite->reportingFreqName(reportingFreqIndex);
    }
#endif // GET_OUT
    std::string columnText(const unsigned char *column)
    {
        return std::string(reinterpret_cast<const char *>(column));
    }

    int logicalToInteger(const bool value)
    {
        return state->dataSQLiteProcedures->sqlite->logicalToInteger(value);
    }

    void adjustReportingHourAndMinutes(int &hour, int &minutes)
    {
        state->dataSQLiteProcedures->sqlite->adjustReportingHourAndMinutes(hour, minutes);
    }

    bool indexExists(const std::string &indexName)
    {
        sqlite3_stmt *sqlStmtPtr;
        std::string sql("pragma index_info(" + indexName + ");");
        bool success = false;
        int rc = sqlite3_prepare_v2(state->dataSQLiteProcedures->sqlite->m_db.get(), sql.c_str(), -1, &sqlStmtPtr, nullptr);
        if (SQLITE_OK != rc) {
            sqlite3_finalize(sqlStmtPtr);
            return success;
        }
        if (SQLITE_ROW == sqlite3_step(sqlStmtPtr)) {
            success = true;
        }
        sqlite3_finalize(sqlStmtPtr);
        return success;
    }

    int columnCount(const std::string &tableName)
    {
        sqlite3_stmt *sqlStmtPtr;
        std::string sql("pragma table_info(" + tableName + ");");
        int rc = sqlite3_prepare_v2(state->dataSQLiteProcedures->sqlite->m_db.get(), sql.c_str(), -1, &sqlStmtPtr, nullptr);
        if (SQLITE_OK != rc) {
            sqlite3_finalize(sqlStmtPtr);
            return -1;
        }
        int rowCount = 0;
        while (SQLITE_ROW == sqlite3_step(sqlStmtPtr)) {
            ++rowCount;
        }
        sqlite3_finalize(sqlStmtPtr);
        return rowCount;
    }

    std::vector<std::vector<std::string>> queryResult(const std::string &statement, const std::string &tableName)
    {
        std::vector<std::vector<std::string>> queryVector;

        int rowCount = columnCount(tableName);
        if (rowCount < 1) {
            return queryVector;
        }

        sqlite3_stmt *sqlStmtPtr;

        sqlite3_prepare_v2(state->dataSQLiteProcedures->sqlite->m_db.get(), statement.c_str(), -1, &sqlStmtPtr, nullptr);
        while (SQLITE_ROW == sqlite3_step(sqlStmtPtr)) {
            std::vector<std::string> valueVector;
            for (int i = 0; i < rowCount; ++i) {
                auto sqlite_value = sqlite3_column_text(sqlStmtPtr, i);
                if (nullptr == sqlite_value) {
                    valueVector.push_back("");
                } else {
                    valueVector.push_back(columnText(sqlite_value));
                }
            }
            queryVector.push_back(valueVector);
        }
        sqlite3_finalize(sqlStmtPtr);
        return queryVector;
    }

    // Helper method that will return the first double it finds, or -10000.0 if not found (abritrarily chosen value)
    Real64 execAndReturnFirstDouble(const std::string &statement)
    {

        Real64 result(-10000.0);

        sqlite3_stmt *sqlStmtPtr;

        int code = sqlite3_prepare_v2(state->dataSQLiteProcedures->sqlite->m_db.get(), statement.c_str(), -1, &sqlStmtPtr, nullptr);

        code = sqlite3_step(sqlStmtPtr);
        if (code == SQLITE_ROW) {
            result = sqlite3_column_double(sqlStmtPtr, 0);
        }

        // must finalize to prevent memory leaks
        sqlite3_finalize(sqlStmtPtr);

        return result;
    }
};

} // namespace EnergyPlus

#endif
