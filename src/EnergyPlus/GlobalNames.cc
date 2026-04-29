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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::GlobalNames {

// Module containing the routines dealing with matching and assuring that
// various component types are unique by name (e.g. Chillers).

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   October 2005
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This module allows for verification of uniqueness (by name) across
// certain component names (esp. Chillers, Boilers)

void IntraObjUniquenessCheck(EnergyPlusData &state,
                             std::string const &NameToVerify,
                             std::string_view const CurrentModuleObject,
                             std::string_view FieldName,
                             std::unordered_set<std::string> &UniqueStrings,
                             bool &ErrorsFound)
{
    if (NameToVerify.empty()) {
        ShowSevereError(state, EnergyPlus::format("E+ object type {} cannot have a blank {} field", CurrentModuleObject, FieldName));
        ErrorsFound = true;
        // NameToVerify = "xxxxx";
        return;
    }

    auto const find_string = UniqueStrings.find(NameToVerify);
    if (find_string == UniqueStrings.end()) {
        UniqueStrings.emplace(NameToVerify);
    } else {
        ErrorsFound = true;
        ShowSevereError(state, EnergyPlus::format("{} has a duplicate field {}", CurrentModuleObject, NameToVerify));
    }
}

bool VerifyUniqueInterObjectName(EnergyPlusData &state,
                                 std::unordered_map<std::string, std::string> &names,
                                 std::string const &object_name,
                                 std::string_view object_type,
                                 std::string_view field_name,
                                 bool &ErrorsFound)
{
    if (object_name.empty()) {
        ShowSevereError(state, EnergyPlus::format("E+ object type {} cannot have blank {} field", object_name, field_name));
        ErrorsFound = true;
        // object_name = "xxxxx";
        return true;
    }
    auto const names_iter = names.find(object_name);
    if (names_iter == names.end()) {
        names.emplace(object_name, object_type);
    } else {
        ErrorsFound = true;
        ShowSevereError(
            state, EnergyPlus::format("{} with object type {} duplicates a name in object type {}", object_name, object_type, names_iter->second));
        return true;
    }
    return false;
}

bool VerifyUniqueInterObjectName(EnergyPlusData &state,
                                 std::unordered_map<std::string, std::string> &names,
                                 std::string const &object_name,
                                 std::string const &object_type,
                                 bool &ErrorsFound)
{
    if (object_name.empty()) {
        ShowSevereError(state, EnergyPlus::format("E+ object type {} has a blank field", object_name));
        ErrorsFound = true;
        // object_name = "xxxxx";
        return true;
    }
    auto const names_iter = names.find(object_name);
    if (names_iter == names.end()) {
        names.emplace(object_name, object_type);
    } else {
        ErrorsFound = true;
        ShowSevereError(
            state, EnergyPlus::format("{} with object type {} duplicates a name in object type {}", object_name, object_type, names_iter->second));
        return true;
    }
    return false;
}

void VerifyUniqueChillerName(
    EnergyPlusData &state, std::string_view TypeToVerify, std::string const &NameToVerify, bool &ErrorsFound, std::string const &StringToDisplay)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine verifies that a new name will be unique in the list of
    // chillers.  If not found in the list, it is added before returning.

    auto const iter = state.dataGlobalNames->ChillerNames.find(NameToVerify);
    if (iter != state.dataGlobalNames->ChillerNames.end()) {
        ShowSevereError(state, EnergyPlus::format("{}, duplicate name={}, Chiller Type=\"{}\".", StringToDisplay, NameToVerify, iter->second));
        ShowContinueError(state, EnergyPlus::format("...Current entry is Chiller Type=\"{}\".", TypeToVerify));
        ErrorsFound = true;
    } else {
        state.dataGlobalNames->ChillerNames.emplace(NameToVerify, Util::makeUPPER(TypeToVerify));
        state.dataGlobalNames->NumChillers = static_cast<int>(state.dataGlobalNames->ChillerNames.size());
    }
}

void VerifyUniqueBaseboardName(EnergyPlusData &state,
                               std::string_view const &TypeToVerify,
                               std::string const &NameToVerify,
                               bool &ErrorsFound,
                               std::string const &StringToDisplay)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine verifies that a new name will be unique in the list of
    // Baseboards.  If not found in the list, it is added before returning.

    auto const iter = state.dataGlobalNames->BaseboardNames.find(NameToVerify);
    if (iter != state.dataGlobalNames->BaseboardNames.end()) {
        ShowSevereError(state, EnergyPlus::format("{}, duplicate name={}, Baseboard Type=\"{}\".", StringToDisplay, NameToVerify, iter->second));
        ShowContinueError(state, EnergyPlus::format("...Current entry is Baseboard Type=\"{}\".", TypeToVerify));
        ErrorsFound = true;
    } else {
        state.dataGlobalNames->BaseboardNames.emplace(NameToVerify, Util::makeUPPER(TypeToVerify));
        state.dataGlobalNames->NumBaseboards = static_cast<int>(state.dataGlobalNames->BaseboardNames.size());
    }
}

void VerifyUniqueBoilerName(
    EnergyPlusData &state, std::string const &TypeToVerify, std::string const &NameToVerify, bool &ErrorsFound, std::string const &StringToDisplay)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine verifies that a new name will be unique in the list of
    // Boilers.  If not found in the list, it is added before returning.

    auto const iter = state.dataGlobalNames->BoilerNames.find(NameToVerify);
    if (iter != state.dataGlobalNames->BoilerNames.end()) {
        ShowSevereError(state, EnergyPlus::format("{}, duplicate name={}, Boiler Type=\"{}\".", StringToDisplay, NameToVerify, iter->second));
        ShowContinueError(state, EnergyPlus::format("...Current entry is Boiler Type=\"{}\".", TypeToVerify));
        ErrorsFound = true;
    } else {
        state.dataGlobalNames->BoilerNames.emplace(NameToVerify, Util::makeUPPER(TypeToVerify));
        state.dataGlobalNames->NumBoilers = static_cast<int>(state.dataGlobalNames->BoilerNames.size());
    }
}

void VerifyUniqueCoilName(
    EnergyPlusData &state, std::string const &TypeToVerify, std::string &NameToVerify, bool &ErrorsFound, std::string const &StringToDisplay)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine verifies that a new name will be unique in the list of
    // Coils.  If not found in the list, it is added before returning.

    if (NameToVerify.empty()) {
        ShowSevereError(state, EnergyPlus::format("\"{}\" cannot have a blank field", TypeToVerify));
        ErrorsFound = true;
        NameToVerify = "xxxxx";
        return;
    }

    auto const iter = state.dataGlobalNames->CoilNames.find(NameToVerify);
    if (iter != state.dataGlobalNames->CoilNames.end()) {
        ShowSevereError(state, EnergyPlus::format("{}, duplicate name={}, Coil Type=\"{}\".", StringToDisplay, NameToVerify, iter->second));
        ShowContinueError(state, EnergyPlus::format("...Current entry is Coil Type=\"{}\".", TypeToVerify));
        ErrorsFound = true;
    } else {
        state.dataGlobalNames->CoilNames.emplace(NameToVerify, Util::makeUPPER(TypeToVerify));
        state.dataGlobalNames->NumCoils = static_cast<int>(state.dataGlobalNames->CoilNames.size());
    }
}

void VerifyUniqueADUName(
    EnergyPlusData &state, std::string const &TypeToVerify, std::string const &NameToVerify, bool &ErrorsFound, std::string const &StringToDisplay)
{
    auto const iter = state.dataGlobalNames->aDUNames.find(NameToVerify);
    if (iter != state.dataGlobalNames->aDUNames.end()) {
        ShowSevereError(state, EnergyPlus::format("{}, duplicate name={}, ADU Type=\"{}\".", StringToDisplay, NameToVerify, iter->second));
        ShowContinueError(state, EnergyPlus::format("...Current entry is Air Distribution Unit Type=\"{}\".", TypeToVerify));
        ErrorsFound = true;
    } else {
        state.dataGlobalNames->aDUNames.emplace(NameToVerify, Util::makeUPPER(TypeToVerify));
        state.dataGlobalNames->numAirDistUnits = static_cast<int>(state.dataGlobalNames->aDUNames.size());
    }
}

} // namespace EnergyPlus::GlobalNames
