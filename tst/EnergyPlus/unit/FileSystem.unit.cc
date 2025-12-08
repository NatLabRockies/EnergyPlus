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

// Google Test Headers
#include <gtest/gtest.h>

// C++ Headers
#include <fstream>
#include <iostream>

// EnergyPlus Headers
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/FileSystem.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST(FileSystem, movefile_test)
{
    // test moveFile function, specifically on Windows

    // create text file to be overridden by new file
    std::string filename = "FileSystemTest.idf";
    std::string line;
    std::stringstream buffer;
    std::ofstream ofs(filename);
    ofs << "Version," + DataStringGlobals::MatchVersion + ";" << std::endl;
    ofs.close();

    // read text file
    std::ifstream ifs(filename);
    buffer << ifs.rdbuf();
    // check that text file was created correctly
    EXPECT_EQ(buffer.str(), "Version," + DataStringGlobals::MatchVersion + ";\n");
    ifs.close();

    // create temporary text file to move to existing file created above
    std::string filename_temp = "FileSystemTest_temp.idf";
    std::ofstream ofs_temp(filename_temp);
    ofs_temp << "Version," + DataStringGlobals::MatchVersion + ";" << std::endl;
    ofs_temp.close();

    // move temporary text file to overwrite existing file
    EnergyPlus::FileSystem::moveFile(filename_temp, filename);
    // read original text file after using moveFile function
    std::stringstream buffer_new;
    std::ifstream ifs_new(filename);
    buffer_new << ifs_new.rdbuf();
    // check that original text file was overwritten by temporary text file
    EXPECT_EQ(buffer_new.str(), "Version," + DataStringGlobals::MatchVersion + ";\n");
    ifs_new.close();

    // remove files
    EnergyPlus::FileSystem::removeFile(filename);
    EnergyPlus::FileSystem::removeFile(filename_temp);
}

TEST(FileSystem, getAbsolutePath)
{
    std::string pathName = "FileSystemTest.idf";
    std::string absPathName = EnergyPlus::FileSystem::getAbsolutePath(pathName).string();
    EXPECT_TRUE(absPathName.find(pathName) != std::string::npos); // Check that the path name appears in the absolute path

    std::string currentDirWithSep = std::string(".") + EnergyPlus::DataStringGlobals::pathChar;
    pathName = currentDirWithSep + std::string("FileSystemTest.idf"); // e.g., "./FileSystemTest.idf"
    absPathName = EnergyPlus::FileSystem::getAbsolutePath(pathName).string();
    EXPECT_FALSE(absPathName.find(currentDirWithSep) != std::string::npos); // Make sure "./" doesn't appear in absolute path
}

TEST_F(EnergyPlusFixture, FileSystemGetFileType)
{
    fs::path filePath0("/eplus/myfile.idf");
    auto ext = EnergyPlus::FileSystem::getFileType(filePath0);
    EXPECT_ENUM_EQ(EnergyPlus::FileSystem::FileTypes::IDF, ext);

    fs::path filePath1("./schedulefileA.CSV");
    ext = EnergyPlus::FileSystem::getFileType(filePath1);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::CSV == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_flat_file_type(ext));

    fs::path filePath2("./schedulefileB.Csv");
    ext = EnergyPlus::FileSystem::getFileType(filePath2);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::CSV == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_flat_file_type(ext));

    fs::path filePath3("./schedulefileC.csv");
    ext = EnergyPlus::FileSystem::getFileType(filePath3);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::CSV == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_flat_file_type(ext));

    fs::path filePath4("./jsonfileA.JSON");
    ext = EnergyPlus::FileSystem::getFileType(filePath4);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::JSON == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_all_json_type(ext));

    fs::path filePath5("./jsonfileB.Json");
    ext = EnergyPlus::FileSystem::getFileType(filePath5);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::JSON == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_all_json_type(ext));

    fs::path filePath6("./jsonfileC.json");
    ext = EnergyPlus::FileSystem::getFileType(filePath6);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::JSON == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_all_json_type(ext));

    fs::path filePath7("./epjsonfileA.epJSON");
    ext = EnergyPlus::FileSystem::getFileType(filePath7);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::EpJSON == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_all_json_type(ext));

    fs::path filePath8("./epjsonfileB.EPJSON");
    ext = EnergyPlus::FileSystem::getFileType(filePath8);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::EpJSON == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_all_json_type(ext));

    fs::path filePath9("./epjsonfileC.epjson");
    ext = EnergyPlus::FileSystem::getFileType(filePath9);
    EXPECT_TRUE(EnergyPlus::FileSystem::FileTypes::EpJSON == ext);
    EXPECT_TRUE(EnergyPlus::FileSystem::is_all_json_type(ext));
}

TEST(FileSystem, Others)
{
    {
        std::string pathName = "folder/FileSystemTest.txt.idf";
        EXPECT_EQ("idf", EnergyPlus::FileSystem::getFileExtension(pathName));
        EXPECT_EQ("folder/FileSystemTest.txt", EnergyPlus::FileSystem::removeFileExtension(pathName));
        EXPECT_EQ(fs::path("folder"), EnergyPlus::FileSystem::getParentDirectoryPath(pathName));
    }
    {
        std::string pathName = "folder/FileSystemTest.txt";
        EXPECT_EQ("txt", EnergyPlus::FileSystem::getFileExtension(pathName));
        EXPECT_EQ("folder/FileSystemTest", EnergyPlus::FileSystem::removeFileExtension(pathName));
        EXPECT_EQ(fs::path("folder"), EnergyPlus::FileSystem::getParentDirectoryPath(pathName));
    }
    {
        std::string pathName = "folder/FileSystemTest";
        EXPECT_EQ("", EnergyPlus::FileSystem::getFileExtension(pathName));
        EXPECT_EQ("folder/FileSystemTest", EnergyPlus::FileSystem::removeFileExtension(pathName));
        EXPECT_EQ(fs::path("folder"), EnergyPlus::FileSystem::getParentDirectoryPath(pathName));
    }
}

TEST(FileSystem, getProgramPath)
{
    fs::path programPath = EnergyPlus::FileSystem::getProgramPath();
    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(programPath));
    EXPECT_TRUE(programPath.string().find("energyplus_tests") != std::string::npos);
    fs::path expectedPath{"energyplus_tests"};
    expectedPath.replace_extension(EnergyPlus::FileSystem::exeExtension);
    EXPECT_EQ(expectedPath, EnergyPlus::FileSystem::getFileName(programPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists(EnergyPlus::FileSystem::getParentDirectoryPath(programPath)));
}

TEST(FileSystem, getParentDirectoryPath)
{
    EXPECT_EQ(fs::path{"a/b"}, EnergyPlus::FileSystem::getParentDirectoryPath("a/b/c"));
    EXPECT_EQ(fs::path{"a/b"}, EnergyPlus::FileSystem::getParentDirectoryPath("a/b/c/"));
    EXPECT_EQ(fs::path{"./"}, EnergyPlus::FileSystem::getParentDirectoryPath("a.idf"));
}

TEST(FileSystem, make_and_remove_Directory)
{
    fs::remove_all("sandboxA");

    fs::path dirPath("sandboxA/a");
    fs::path rootPath = "sandboxA";
    EXPECT_EQ(rootPath, EnergyPlus::FileSystem::getParentDirectoryPath(dirPath));

    EXPECT_FALSE(EnergyPlus::FileSystem::pathExists(rootPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(rootPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::directoryExists(rootPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::pathExists(dirPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(dirPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::directoryExists(dirPath));

    // This used to fail, because it can't make intermediate directories... which I think is a weird unwanted limitation
    // eg: energyplus -d out/a/b/c/ should be possible. It would create out, out/a, out/a/b/ and out/a/b/c/ as needed
    EnergyPlus::FileSystem::makeDirectory(dirPath);

    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(rootPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(rootPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists(rootPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(dirPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(dirPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists(dirPath));

    fs::path filePath("sandboxA/a/file.txt.idf");
    std::ofstream(filePath).put('a'); // create regular file

    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(rootPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(rootPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists(rootPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(dirPath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(dirPath));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists(dirPath));

    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(filePath));
    EXPECT_TRUE(EnergyPlus::FileSystem::fileExists(filePath));
    EXPECT_FALSE(EnergyPlus::FileSystem::directoryExists((filePath)));

    fs::remove_all("sandbox");
}

TEST(FileSystem, Elaborate)
{
    EnergyPlus::FileSystem::makeDirectory("sandboxB");
    std::string pathName("sandboxB/file1.txt.idf");
    std::ofstream(pathName).put('a'); // create regular file
    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists(pathName));
    EXPECT_TRUE(EnergyPlus::FileSystem::fileExists(pathName));
    EXPECT_TRUE(EnergyPlus::FileSystem::pathExists("sandboxB"));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists("sandboxB"));
    EXPECT_TRUE(EnergyPlus::FileSystem::directoryExists("sandboxB/"));
    EXPECT_GT(EnergyPlus::FileSystem::getAbsolutePath(pathName).string().size(), pathName.size());
    // fs::equivalent throws when it doesn't exist (because it checks for file status)
    EXPECT_TRUE(
        fs::equivalent(fs::path("sandboxB/"), EnergyPlus::FileSystem::getParentDirectoryPath(EnergyPlus::FileSystem::getAbsolutePath(pathName))));
    EXPECT_TRUE(
        fs::equivalent(fs::path("sandboxB"), EnergyPlus::FileSystem::getParentDirectoryPath(EnergyPlus::FileSystem::getAbsolutePath(pathName))));
    EXPECT_TRUE(fs::equivalent(fs::path("sandboxB"), EnergyPlus::FileSystem::getAbsolutePath("./sandboxB")));
    EXPECT_TRUE(fs::equivalent(fs::path("sandboxB"), EnergyPlus::FileSystem::getAbsolutePath("./sandboxB/../sandboxB")));
    EnergyPlus::FileSystem::removeFile(pathName);
    EXPECT_FALSE(EnergyPlus::FileSystem::pathExists(pathName));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(pathName));

    fs::remove_all("sandboxB");
}

// Windows support for symlink isn't great, you'd need admin privileges
#ifndef _WIN32
TEST(FileSystem, getAbsolutePath_WithSymlink)
{
    fs::remove_all("sandboxSymlink");

    fs::path productsDir = "sandboxSymlink/Products";
    fs::create_directories(productsDir);

    fs::path exeName = "energyplus-9.5.0";
    fs::path exePath = productsDir / exeName;
    fs::path symlinkPath = productsDir / "energyplus";

    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(exePath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(symlinkPath));

    std::ofstream(exePath).put('a');
    EXPECT_TRUE(EnergyPlus::FileSystem::fileExists(exePath));
    EXPECT_FALSE(EnergyPlus::FileSystem::fileExists(symlinkPath));

    fs::create_symlink("energyplus-9.5.0", symlinkPath);
    EXPECT_TRUE(EnergyPlus::FileSystem::fileExists(exePath));
    EXPECT_TRUE(EnergyPlus::FileSystem::fileExists(symlinkPath));
    EXPECT_FALSE(fs::is_symlink(exePath));
    EXPECT_TRUE(fs::is_symlink(symlinkPath));
    EXPECT_EQ(exeName, fs::read_symlink(symlinkPath));

    // Now, we check that we can actually resolve it correctly
    EXPECT_EQ(EnergyPlus::FileSystem::getAbsolutePath(exePath), EnergyPlus::FileSystem::getAbsolutePath(symlinkPath));

    fs::remove_all("sandboxSymlink");
}
#endif
