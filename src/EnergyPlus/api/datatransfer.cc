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

#include <cmath>

#include <ObjexxFCL/ArrayS.functions.hh>
#include <ObjexxFCL/time.hh>

#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/api/datatransfer.h>
#include <EnergyPlus/api/runtime.h>

using namespace EnergyPlus;

APIDataEntry *getAPIData(EnergyPlusState state, unsigned int *resultingSize)
{
    struct LocalAPIDataEntry
    {
        std::string what;
        std::string name;
        std::string type;
        std::string key;
        std::string unit;

        LocalAPIDataEntry(std::string _what, std::string _name, std::string _type, std::string _key, std::string _unit)
            : what(std::move(_what)), name(std::move(_name)), type(std::move(_type)), key(std::move(_key)), unit(std::move(_unit))
        {
        }
    };
    std::vector<LocalAPIDataEntry> localDataEntries;
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    for (auto const &availActuator : thisState->dataRuntimeLang->EMSActuatorAvailable) {
        if (availActuator.ComponentTypeName.empty() && availActuator.UniqueIDName.empty() && availActuator.ControlTypeName.empty()) {
            break;
        }
        localDataEntries.emplace_back(
            "Actuator", availActuator.ComponentTypeName, availActuator.ControlTypeName, availActuator.UniqueIDName, availActuator.Units);
    }
    for (auto const &availVariable : thisState->dataRuntimeLang->EMSInternalVarsAvailable) {
        if (availVariable.DataTypeName.empty() && availVariable.UniqueIDName.empty()) {
            break;
        }
        localDataEntries.emplace_back("InternalVariable", availVariable.DataTypeName, "", availVariable.UniqueIDName, availVariable.Units);
    }
    for (auto const &gVarName : thisState->dataPluginManager->globalVariableNames) {
        localDataEntries.emplace_back("PluginGlobalVariable", "", "", gVarName, "");
    }
    for (auto const &trend : thisState->dataPluginManager->trends) {
        localDataEntries.emplace_back("PluginTrendVariable,", "", "", trend.name, "");
    }
    for (auto const *meter : thisState->dataOutputProcessor->meters) {
        if (meter->Name.empty()) {
            break;
        }
        localDataEntries.emplace_back("OutputMeter", "", "", meter->Name, format("{}", EnergyPlus::Constant::unitNames[(int)meter->units]));
    }
    for (auto const *variable : thisState->dataOutputProcessor->outVars) {
        if (variable->varType != EnergyPlus::OutputProcessor::VariableType::Real) {
            continue;
        }
        if (variable->name.empty() && variable->keyUC.empty()) {
            break;
        }
        localDataEntries.emplace_back("OutputVariable",
                                      variable->name,
                                      "",
                                      variable->keyUC,
                                      variable->units == EnergyPlus::Constant::Units::customEMS
                                          ? variable->unitNameCustomEMS
                                          : format("{}", EnergyPlus::Constant::unitNames[(int)variable->units]));
    }
    *resultingSize = localDataEntries.size();
    auto *data = new APIDataEntry[*resultingSize];
    for (unsigned int i = 0; i < *resultingSize; i++) {
        data[i].what = new char[std::strlen(localDataEntries[i].what.c_str()) + 1];
        std::strcpy(data[i].what, localDataEntries[i].what.c_str());
        data[i].name = new char[std::strlen(localDataEntries[i].name.c_str()) + 1];
        std::strcpy(data[i].name, localDataEntries[i].name.c_str());
        data[i].key = new char[std::strlen(localDataEntries[i].key.c_str()) + 1];
        std::strcpy(data[i].key, localDataEntries[i].key.c_str());
        data[i].type = new char[std::strlen(localDataEntries[i].type.c_str()) + 1];
        std::strcpy(data[i].type, localDataEntries[i].type.c_str());
        data[i].unit = new char[std::strlen(localDataEntries[i].unit.c_str()) + 1];
        std::strcpy(data[i].unit, localDataEntries[i].unit.c_str());
    }
    return data;
}

void freeAPIData(const APIDataEntry *data, const unsigned int arraySize)
{
    for (unsigned int i = 0; i < arraySize; i++) {
        delete[] data[i].what;
        delete[] data[i].name;
        delete[] data[i].key;
        delete[] data[i].type;
        delete[] data[i].unit;
    }
    delete[] data;
}

char *listAllAPIDataCSV(EnergyPlusState state)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    std::string output = "**ACTUATORS**\n";
    for (auto const &availActuator : thisState->dataRuntimeLang->EMSActuatorAvailable) {
        if (availActuator.ComponentTypeName.empty() && availActuator.UniqueIDName.empty() && availActuator.ControlTypeName.empty()) {
            break;
        }
        output.append("Actuator,");
        output.append(availActuator.ComponentTypeName).append(",");
        output.append(availActuator.ControlTypeName).append(",");
        output.append(availActuator.UniqueIDName).append(",");
        output.append(availActuator.Units).append("\n");
    }
    output.append("**INTERNAL_VARIABLES**\n");
    for (auto const &availVariable : thisState->dataRuntimeLang->EMSInternalVarsAvailable) {
        if (availVariable.DataTypeName.empty() && availVariable.UniqueIDName.empty()) {
            break;
        }
        output.append("InternalVariable,");
        output.append(availVariable.DataTypeName).append(",");
        output.append(availVariable.UniqueIDName).append(",");
        output.append(availVariable.Units).append("\n");
    }
    output.append("**PLUGIN_GLOBAL_VARIABLES**\n");
    for (auto const &gVarName : thisState->dataPluginManager->globalVariableNames) {
        output.append("PluginGlobalVariable,");
        output.append(gVarName).append("\n");
    }
    output.append("**TRENDS**\n");
    for (auto const &trend : thisState->dataPluginManager->trends) {
        output.append("PluginTrendVariable,");
        output.append(trend.name).append("\n");
    }
    output.append("**METERS**\n");
    for (auto const *meter : thisState->dataOutputProcessor->meters) {
        if (meter->Name.empty()) {
            break;
        }
        output.append("OutputMeter").append(","); // This multiple append thing is not good
        output.append(meter->Name).append(",");
        output.append(format("{}\n", EnergyPlus::Constant::unitNames[(int)meter->units]));
    }
    output.append("**VARIABLES**\n");
    for (auto const *variable : thisState->dataOutputProcessor->outVars) {
        if (variable->varType != EnergyPlus::OutputProcessor::VariableType::Real) {
            continue;
        }
        if (variable->name.empty() && variable->keyUC.empty()) {
            break;
        }
        output.append("OutputVariable,");
        output.append(variable->name).append(",");
        output.append(variable->keyUC).append(",");
        output.append(format("{}\n",
                             variable->units == EnergyPlus::Constant::Units::customEMS ? variable->unitNameCustomEMS
                                                                                       : EnergyPlus::Constant::unitNames[(int)variable->units]));
    }
    // note that we cannot just return a c_str to the local string, as the string will be destructed upon leaving
    // this function, and undefined behavior will occur.
    // instead make a deep copy, and the user must manage the new char * pointer
    // strcpy copies including the null-terminator, strlen doesn't include it
    char *p = new char[std::strlen(output.c_str()) + 1];
    std::strcpy(p, output.c_str());
    return p;
}

int apiDataFullyReady(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (thisState->dataPluginManager->fullyReady) {
        return 1;
    }
    return 0;
}

int apiErrorFlag(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (thisState->dataPluginManager->apiErrorFlag) {
        return 1;
    }
    return 0;
}

void resetErrorFlag(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    thisState->dataPluginManager->apiErrorFlag = false;
}

char *inputFilePath(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    std::string const path_utf8 = EnergyPlus::FileSystem::toGenericString(thisState->dataStrGlobals->inputFilePath);
    char *p = new char[std::strlen(path_utf8.c_str()) + 1];
    std::strcpy(p, path_utf8.c_str());
    return p;
}

char *epwFilePath(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    std::string const path_utf8 = EnergyPlus::FileSystem::toGenericString(thisState->files.inputWeatherFilePath.filePath);
    char *p = new char[std::strlen(path_utf8.c_str()) + 1];
    std::strcpy(p, path_utf8.c_str());
    return p;
}

char **getObjectNames(EnergyPlusState state, const char *objectType, unsigned int *resultingSize)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    auto &epjson = thisState->dataInputProcessing->inputProcessor->epJSON;
    const auto instances = epjson.find(objectType);
    if (instances == epjson.end()) {
        *resultingSize = 0;
        return nullptr;
    }
    auto &instancesValue = instances.value();
    *resultingSize = instancesValue.size();
    char **data = new char *[*resultingSize];
    unsigned int i = -1;
    for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
        i++;
        std::string s = std::string(instance.key());
        data[i] = new char[std::strlen(instance.key().data()) + 1];
        std::strcpy(data[i], instance.key().data());
    }
    return data;
}

void freeObjectNames(char **objectNames, unsigned int arraySize)
{
    // as of right now we don't actually need to free the underlying strings, they exist in the epJSON instance, so just delete our array of pointers
    (void)arraySize;
    // no op to avoid compiler warning that this variable is unused, in the future, this may be needed so keeping it in the API now
    delete[] objectNames;
}

int getNumNodesInCondFDSurfaceLayer(EnergyPlusState state, const char *surfName, const char *matName)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    std::string UCsurfName = EnergyPlus::Util::makeUPPER(surfName);
    std::string UCmatName = EnergyPlus::Util::makeUPPER(matName);
    return EnergyPlus::HeatBalFiniteDiffManager::numNodesInMaterialLayer(*thisState, UCsurfName, UCmatName);
}

void requestVariable(EnergyPlusState state, const char *type, const char *key)
{
    // allow specifying a request for an output variable, so that E+ does not have to keep all of them in memory
    // should be called before energyplus is run!
    // note that the variable request array is cleared during clear_state, so if you run multiple E+ runs, these must be requested again each time.
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    EnergyPlus::OutputProcessor::APIOutputVariableRequest request;
    request.varName = type;
    request.varKey = key;
    thisState->dataOutputProcessor->apiVarRequests.push_back(request);
}

int getVariableHandle(EnergyPlusState state, const char *type, const char *key)
{
    // Variables are accessed through a single integer ID, but there are multiple internal types: real and integer.
    // I am going to make the integer handle span all both types, by carefully defining the handle.
    // basically, the handles are contiguous, with:
    //  - index 1 being the first real variable handle
    //  - index N being the highest real variable handle
    //  - index N+1 being the first integer variable handle
    //  - index N+M being the highest integer variable handle
    // In this function, it is as simple as looping over both types and continuing to increment
    // the handle carefully.  In the getValue function it is just a matter of checking array sizes.
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    std::string const typeUC = EnergyPlus::Util::makeUPPER(type);
    std::string const keyUC = EnergyPlus::Util::makeUPPER(key);
    for (int i = 0; i < thisState->dataOutputProcessor->outVars.size(); i++) {
        auto const *var = thisState->dataOutputProcessor->outVars[i];
        if (typeUC == var->nameUC && keyUC == var->keyUC) {
            return i;
        }
    }
    return -1; // return -1 if it wasn't found
}

Real64 getVariableValue(EnergyPlusState state, const int handle)
{
    // this function works in conjunction with the plan set up in getVariableHandle
    // basically, the handles are contiguous, with:
    //  - index 1 being the first real variable handle
    //  - index N being the highest real variable handle
    //  - index N+1 being the first integer variable handle
    //  - index N+M being the highest integer variable handle
    // note that this function will return -1 if it cannot
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle >= 0 && handle < thisState->dataOutputProcessor->outVars.size()) {
        auto const *thisOutputVar = thisState->dataOutputProcessor->outVars[handle];
        if (thisOutputVar->varType == EnergyPlus::OutputProcessor::VariableType::Real) {
            return *(dynamic_cast<EnergyPlus::OutputProcessor::OutVarReal const *>(thisOutputVar))->Which;
        }
        if (thisOutputVar->varType == EnergyPlus::OutputProcessor::VariableType::Integer) {
            return (Real64) * (dynamic_cast<EnergyPlus::OutputProcessor::OutVarInt const *>(thisOutputVar))->Which;
        }
        if (thisState->dataGlobal->errorCallback) {
            std::cout << "ERROR: Variable at handle has type other than Real or Integer, returning zero but caller should take note and likely abort."
                      << std::endl;
        } else {
            // must be running from python plugin, need to fatal out once the plugin is done
            // throw an error, set the fatal flag, and then return zero
            ShowSevereError(*thisState, fmt::format("Data Exchange API: Error in getVariableValue; received handle: {}", handle));
            ShowContinueError(*thisState,
                              "The getVariableValue function will return 0 for now to allow the plugin to finish, then EnergyPlus will abort");
        }
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (thisState->dataGlobal->errorCallback) {
        std::cout << "ERROR: Variable handle out of range in getVariableValue, returning zero but caller should take note and likely abort."
                  << std::endl;
    } else {
        // must be running from python plugin, need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return zero
        ShowSevereError(*thisState, fmt::format("Data Exchange API: Index error in getVariableValue; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getVariableValue function will return 0 for now to allow the plugin to finish, then EnergyPlus will abort");
    }
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

int getMeterHandle(EnergyPlusState state, const char *meterName)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    std::string const meterNameUC = EnergyPlus::Util::makeUPPER(meterName);
    const int i = EnergyPlus::GetMeterIndex(*thisState, meterNameUC);
    if (i == 0) {
        // inside E+, zero is meaningful, but through the API, I want to use negative one as a signal of a bad lookup
        return -1;
    }
    return i;
}

Real64 getMeterValue(EnergyPlusState state, int handle)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle >= 0 && handle < thisState->dataOutputProcessor->meters.size()) {
        return EnergyPlus::GetCurrentMeterValue(*thisState, handle);
    }
    if (thisState->dataGlobal->errorCallback) {
        std::cout << "ERROR: Meter handle out of range in getMeterValue, returning zero but caller should take note and likely abort." << std::endl;
    } else {
        // must be running from python plugin, need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return zero
        ShowSevereError(*thisState, fmt::format("Data Exchange API: Index error in getMeterValue; received handle: {}", handle));
        ShowContinueError(*thisState, "The getMeterValue function will return 0 for now to allow the plugin to finish, then EnergyPlus will abort");
    }
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

int getActuatorHandle(EnergyPlusState state, const char *componentType, const char *controlType, const char *uniqueKey)
{
    int handle = 0;
    std::string const typeUC = EnergyPlus::Util::makeUPPER(componentType);
    std::string const keyUC = EnergyPlus::Util::makeUPPER(uniqueKey);
    std::string const controlUC = EnergyPlus::Util::makeUPPER(controlType);
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    for (int ActuatorLoop = 1; ActuatorLoop <= thisState->dataRuntimeLang->numEMSActuatorsAvailable; ++ActuatorLoop) {
        auto &availActuator = thisState->dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop);
        handle++;
        std::string const actuatorTypeUC = EnergyPlus::Util::makeUPPER(availActuator.ComponentTypeName);
        std::string const actuatorIDUC = EnergyPlus::Util::makeUPPER(availActuator.UniqueIDName);
        std::string const actuatorControlUC = EnergyPlus::Util::makeUPPER(availActuator.ControlTypeName);
        if (typeUC == actuatorTypeUC && keyUC == actuatorIDUC && controlUC == actuatorControlUC) {
            if (availActuator.handleCount > 0) {
                // If the handle is already used by an IDF EnergyManagementSystem:Actuator, we should warn the user
                bool foundActuator = false;
                for (auto const &usedActuator : thisState->dataRuntimeLang->EMSActuatorUsed) {
                    if (usedActuator.ActuatorVariableNum == handle) {
                        ShowWarningError(
                            *thisState,
                            "Data Exchange API: An EnergyManagementSystem:Actuator seems to be already defined in the EnergyPlus File and named '" +
                                usedActuator.Name + "'.");
                        ShowContinueError(
                            *thisState, fmt::format("Occurred for componentType='{}', controlType='{}', uniqueKey='{}'.", typeUC, controlUC, keyUC));
                        ShowContinueError(*thisState,
                                          fmt::format("The getActuatorHandle function will still return the handle (= {}) but caller "
                                                      "should take note that there is a risk of overwritting.",
                                                      handle));
                        foundActuator = true;
                        break;
                    }
                }
                if (!foundActuator) {
                    ShowWarningError(*thisState, "Data Exchange API: You seem to already have tried to get an Actuator Handle on this one.");
                    ShowContinueError(*thisState,
                                      fmt::format("Occurred for componentType='{}', controlType='{}', uniqueKey='{}'.", typeUC, controlUC, keyUC));
                    ShowContinueError(*thisState,
                                      fmt::format("The getActuatorHandle function will still return the handle (= {}) but caller should "
                                                  "take note that there is a risk of overwritting.",
                                                  handle));
                }
            }
            ++availActuator.handleCount;

            return handle;
        }
    }
    return -1;
}

void resetActuator(EnergyPlusState state, int handle)
{
    // resets the actuator so that E+ will use the internally calculated value again
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle >= 1 && handle <= thisState->dataRuntimeLang->numEMSActuatorsAvailable) {
        const auto &theActuator(thisState->dataRuntimeLang->EMSActuatorAvailable(handle));
        *theActuator.Actuated = false;
    } else {
        if (thisState->dataGlobal->errorCallback) {
            std::cout << "ERROR: Actuator handle out of range in resetActuator, returning but caller should take note and likely abort." << std::endl;
        } else {
            // must be running from python plugin, need to fatal out once the plugin is done
            // throw an error, set the fatal flag, and then return
            ShowSevereError(*thisState, fmt::format("Data Exchange API: index error in resetActuator; received handle: {}", handle));
            ShowContinueError(*thisState, "The resetActuator function will return to allow the plugin to finish, then EnergyPlus will abort");
        }
        thisState->dataPluginManager->apiErrorFlag = true;
    }
}

void setActuatorValue(EnergyPlusState state, const int handle, const Real64 value)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle >= 1 && handle <= thisState->dataRuntimeLang->numEMSActuatorsAvailable) {
        auto &theActuator(thisState->dataRuntimeLang->EMSActuatorAvailable(handle));
        if (theActuator.RealValue) {
            *theActuator.RealValue = value;
        } else if (theActuator.IntValue) {
            *theActuator.IntValue = static_cast<int>(std::lround(value));
        } else {
            // follow protocol from EMS manager, where 1.0 is true, 0.0 is false, and anything else is also false
            *theActuator.LogValue = value > 0.99999 && value < 1.00001;
            // allow small tolerance while passing between languages and types
        }
        *theActuator.Actuated = true;
    } else {
        if (thisState->dataGlobal->errorCallback) {
            std::cout << "ERROR: Actuator handle out of range in setActuatorValue, returning but caller should take note and likely abort."
                      << std::endl;
        } else {
            // must be running from python plugin, need to fatal out once the plugin is done
            // throw an error, set the fatal flag, and then return
            ShowSevereError(*thisState, fmt::format("Data Exchange API: index error in setActuatorValue; received handle: {}", handle));
            ShowContinueError(*thisState, "The setActuatorValue function will return to allow the plugin to finish, then EnergyPlus will abort");
        }
        thisState->dataPluginManager->apiErrorFlag = true;
    }
}

Real64 getActuatorValue(EnergyPlusState state, const int handle)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle >= 1 && handle <= thisState->dataRuntimeLang->numEMSActuatorsAvailable) {
        const auto &theActuator(thisState->dataRuntimeLang->EMSActuatorAvailable(handle));
        if (theActuator.RealValue) {
            return *theActuator.RealValue;
        }
        if (theActuator.IntValue) {
            return static_cast<float>(*theActuator.IntValue);
        }
        // follow protocol from EMS manager, where 1.0 is true, 0.0 is false, and anything else is also false
        if (*theActuator.LogValue) {
            return 1;
        }
        return 0;
    }
    if (thisState->dataGlobal->errorCallback) {
        std::cout << "ERROR: Actuator handle out of range in getActuatorValue, returning zero but caller should take note and likely abort."
                  << std::endl;
    } else {
        // must be running from python plugin, need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return 0
        ShowSevereError(*thisState, fmt::format("Data Exchange API: index error in getActuatorValue; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getActuatorValue function will return 0 for now to allow the plugin to finish, then EnergyPlus will abort");
    }
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

int getInternalVariableHandle(EnergyPlusState state, const char *type, const char *key)
{
    int handle = 0;
    std::string const typeUC = EnergyPlus::Util::makeUPPER(type);
    std::string const keyUC = EnergyPlus::Util::makeUPPER(key);
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    for (auto const &availVariable : thisState->dataRuntimeLang->EMSInternalVarsAvailable) {
        // TODO: this should stop at numEMSInternalVarsAvailable
        handle++;
        std::string const variableTypeUC = EnergyPlus::Util::makeUPPER(availVariable.DataTypeName);
        std::string const variableIDUC = EnergyPlus::Util::makeUPPER(availVariable.UniqueIDName);
        if (typeUC == variableTypeUC && keyUC == variableIDUC) {
            return handle;
        }
    }
    return -1;
}

Real64 getInternalVariableValue(EnergyPlusState state, int handle)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle >= 1 && handle <= (int)thisState->dataRuntimeLang->numEMSInternalVarsAvailable) {
        auto const &thisVar = thisState->dataRuntimeLang->EMSInternalVarsAvailable(handle);
        if (thisVar.PntrVarTypeUsed == EnergyPlus::DataRuntimeLanguage::PtrDataType::Real) {
            return *thisVar.RealValue;
        }
        if (thisVar.PntrVarTypeUsed == EnergyPlus::DataRuntimeLanguage::PtrDataType::Integer) {
            return (Real64)(*thisVar.IntValue);
        }
        // Doesn't look like this struct actually has a logical member type, so uh, throw here?
        std::cout << "ERROR: Invalid internal variable type here, developer issue., returning zero but caller should take note and likely abort."
                  << std::endl;
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (thisState->dataGlobal->errorCallback) {
        std::cout << "ERROR: Internal variable handle out of range in getInternalVariableValue, returning zero but caller should take note and "
                     "likely abort."
                  << std::endl;
    } else {
        // must be running from python plugin, need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return 0
        ShowSevereError(*thisState, fmt::format("Data Exchange API: index error in getInternalVariableValue; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getInternalVariableValue function will return 0 for now to allow the plugin to finish, then EnergyPlus will abort");
    }
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

int getEMSGlobalVariableHandle(EnergyPlusState state, const char *name)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    int index = 0;
    for (auto const &erlVar : thisState->dataRuntimeLang->ErlVariable) {
        index++;
        // only respond if we are outside of the built-in EMS var range
        if (index < thisState->dataRuntimeLang->emsVarBuiltInStart || index > thisState->dataRuntimeLang->emsVarBuiltInEnd) {
            if (EnergyPlus::Util::SameString(name, erlVar.Name)) {
                return index;
            }
        }
    }
    return 0;
}

Real64 getEMSGlobalVariableValue(EnergyPlusState state, int handle)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    auto const &erl = thisState->dataRuntimeLang;
    bool const insideBuiltInRange = handle >= erl->emsVarBuiltInStart && handle <= erl->emsVarBuiltInEnd;
    if (insideBuiltInRange || handle > thisState->dataRuntimeLang->NumErlVariables) {
        // need to fatal out once the process is done
        // throw an error, set the fatal flag, and then return 0
        EnergyPlus::ShowSevereError(
            *thisState, fmt::format("Data Exchange API: Problem -- index error in getEMSGlobalVariableValue; received handle: {}", handle));
        EnergyPlus::ShowContinueError(
            *thisState, "The getEMSGlobalVariableValue function will return 0 for now to allow the process to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return erl->ErlVariable(handle).Value.Number;
}

void setEMSGlobalVariableValue(EnergyPlusState state, int handle, Real64 value)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    auto const &erl = thisState->dataRuntimeLang;
    bool const insideBuiltInRange = handle >= erl->emsVarBuiltInStart && handle <= erl->emsVarBuiltInEnd;
    if (insideBuiltInRange || handle > erl->NumErlVariables) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        EnergyPlus::ShowSevereError(
            *thisState, fmt::format("Data Exchange API: Problem -- index error in setEMSGlobalVariableValue; received handle: {}", handle));
        EnergyPlus::ShowContinueError(*thisState,
                                      "The setEMSGlobalVariableValue function will return to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
    }
    erl->ErlVariable(handle).Value.Number = value;
}

int getPluginGlobalVariableHandle(EnergyPlusState state, const char *name)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataPluginManager->pluginManager->getGlobalVariableHandle(*thisState, name);
}

Real64 getPluginGlobalVariableValue(EnergyPlusState state, int handle)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxGlobalVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return 0
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginGlobalVariableValue; received handle: {}", handle));
        ShowContinueError(
            *thisState, "The getPluginGlobalVariableValue function will return 0 for now to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return thisState->dataPluginManager->pluginManager->getGlobalVariableValue(*thisState, handle);
}

void setPluginGlobalVariableValue(EnergyPlusState state, int handle, Real64 value)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxGlobalVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in setPluginGlobalVariableValue; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginGlobalVariableValue function will return to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
    }
    thisState->dataPluginManager->pluginManager->setGlobalVariableValue(*thisState, handle, value);
}

int getPluginTrendVariableHandle(EnergyPlusState state, const char *name)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableHandle(*thisState, name);
}

Real64 getPluginTrendVariableValue(EnergyPlusState state, int handle, int timeIndex)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxTrendVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginTrendVariableValue; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableValue function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (timeIndex < 1 || timeIndex > EnergyPlus::PluginManagement::PluginManager::getTrendVariableHistorySize(*thisState, handle)) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(
            *thisState,
            fmt::format("Data Exchange API: Problem -- trend history count argument out of range in getPluginTrendVariableValue; received value: {}",
                        timeIndex));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableValue function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableValue(*thisState, handle, timeIndex);
}

Real64 getPluginTrendVariableAverage(EnergyPlusState state, int handle, int count)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxTrendVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginTrendVariableAverage; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableAverage function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (count < 2 || count > EnergyPlus::PluginManagement::PluginManager::getTrendVariableHistorySize(*thisState, handle)) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(
            *thisState,
            fmt::format(
                "Data Exchange API: Problem -- trend history count argument out of range in getPluginTrendVariableAverage; received value: {}",
                count));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableAverage function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableAverage(*thisState, handle, count);
}

Real64 getPluginTrendVariableMin(EnergyPlusState state, int handle, int count)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxTrendVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginTrendVariableMin; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableMin function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (count < 2 || count > EnergyPlus::PluginManagement::PluginManager::getTrendVariableHistorySize(*thisState, handle)) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(
            *thisState,
            fmt::format("Data Exchange API: Problem -- trend history count argument out of range in getPluginTrendVariableMin; received value: {}",
                        count));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableMin function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableMin(*thisState, handle, count);
}

Real64 getPluginTrendVariableMax(EnergyPlusState state, int handle, int count)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxTrendVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginTrendVariableMax; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableMax function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (count < 2 || count > EnergyPlus::PluginManagement::PluginManager::getTrendVariableHistorySize(*thisState, handle)) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(
            *thisState,
            fmt::format("Data Exchange API: Problem -- trend history count argument out of range in getPluginTrendVariableMax; received value: {}",
                        count));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableMax function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableMax(*thisState, handle, count);
}

Real64 getPluginTrendVariableSum(EnergyPlusState state, int handle, int count)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxTrendVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginTrendVariableSum; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableSum function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (count < 2 || count > EnergyPlus::PluginManagement::PluginManager::getTrendVariableHistorySize(*thisState, handle)) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(
            *thisState,
            fmt::format("Data Exchange API: Problem -- trend history count argument out of range in getPluginTrendVariableSum; received value: {}",
                        count));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableSum function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableSum(*thisState, handle, count);
}

Real64 getPluginTrendVariableDirection(EnergyPlusState state, int handle, int count)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (handle < 0 || handle > thisState->dataPluginManager->pluginManager->maxTrendVariableIndex) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(*thisState,
                        fmt::format("Data Exchange API: Problem -- index error in getPluginTrendVariableDirection; received handle: {}", handle));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableDirection function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    if (count < 2 || count > EnergyPlus::PluginManagement::PluginManager::getTrendVariableHistorySize(*thisState, handle)) {
        // need to fatal out once the plugin is done
        // throw an error, set the fatal flag, and then return
        ShowSevereError(
            *thisState,
            fmt::format(
                "Data Exchange API: Problem -- trend history count argument out of range in getPluginTrendVariableDirection; received value: {}",
                count));
        ShowContinueError(*thisState,
                          "The getPluginTrendVariableDirection function will return 0 to allow the plugin to finish, then EnergyPlus will abort");
        thisState->dataPluginManager->apiErrorFlag = true;
        return 0;
    }
    return EnergyPlus::PluginManagement::PluginManager::getTrendVariableDirection(*thisState, handle, count);
}

int year(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->Year;
}

int calendarYear(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataGlobal->CalendarYear;
}

int month(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->Month;
}

int dayOfMonth(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->DayOfMonth;
}

int dayOfWeek(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->DayOfWeek;
}

int dayOfYear(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->DayOfYear;
}

int daylightSavingsTimeIndicator(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->DSTIndicator;
}

int hour(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataGlobal->HourOfDay - 1;
    // no, just stay on 0..23+ DSTadjust ! offset by 1 and daylight savings time
}

Real64 currentTime(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    if (thisState->dataHVACGlobal->TimeStepSys < thisState->dataGlobal->TimeStepZone) {
        // CurrentTime is for end of zone timestep, need to move back to beginning of current zone timestep, then add on system time elapsed already
        // plus current system timestep
        return thisState->dataGlobal->CurrentTime - thisState->dataGlobal->TimeStepZone + thisState->dataHVACGlobal->SysTimeElapsed +
               thisState->dataHVACGlobal->TimeStepSys;
    }
    return thisState->dataGlobal->CurrentTime;
}

int minutes(EnergyPlusState state)
{
    // the -1 is to push us to the right minute, but this should be handled cautiously because if we are inside the HVAC iteration loop,
    // currentTime() returns a floating point fractional hour, so truncation could put this a few seconds from the expected minute.
    const Real64 currentTimeVal = currentTime(state);
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const Real64 fractionalHoursIntoTheDay = currentTimeVal - static_cast<double>(thisState->dataGlobal->HourOfDay - 1);
    const Real64 fractionalMinutesIntoTheDay = std::round(fractionalHoursIntoTheDay * 60.0);
    return static_cast<int>(fractionalMinutesIntoTheDay);
}

int numTimeStepsInHour([[maybe_unused]] EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataGlobal->TimeStepsInHour;
}

int zoneTimeStepNum([[maybe_unused]] EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataGlobal->TimeStep;
}

int holidayIndex(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->HolidayIndex;
}

int sunIsUp(EnergyPlusState state)
{
    // maintain response convention from previous (EMS) implementation
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return (int)thisState->dataEnvrn->SunIsUp;
}

int isRaining(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return (int)thisState->dataEnvrn->IsRain;
}

int warmupFlag(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return (int)thisState->dataGlobal->WarmupFlag;
}

Real64 zoneTimeStep(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataGlobal->TimeStepZone;
}

Real64 systemTimeStep(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataHVACGlobal->TimeStepSys;
}

int currentEnvironmentNum(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return thisState->dataEnvrn->CurEnvirNum;
}

int kindOfSim(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return static_cast<int>(thisState->dataGlobal->KindOfSim);
}

int getConstructionHandle(EnergyPlusState state, const char *constructionName)
{
    int handle = 0;
    std::string const nameUC = EnergyPlus::Util::makeUPPER(constructionName);
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    for (auto const &construct : thisState->dataConstruction->Construct) {
        handle++;
        if (nameUC == EnergyPlus::Util::makeUPPER(construct.Name)) {
            return handle;
        }
    }
    return -1; // return -1 if it wasn't found
}

int actualTime(EnergyPlusState)
{
    const std::string datestring;
    Array1D_int datevalues(8);
    ObjexxFCL::date_and_time(datestring, _, _, datevalues);
    return sum(datevalues({5, 8}));
}

int actualDateTime(EnergyPlusState)
{
    const std::string datestring;
    const Array1D_int datevalues(8);
    ObjexxFCL::date_and_time(datestring, _, _, datevalues);
    return sum(datevalues);
}

int todayWeatherIsRainAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);

    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return (int)thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).IsRain;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

int todayWeatherIsSnowAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);

    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return (int)thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).IsSnow;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

Real64 todayWeatherOutDryBulbAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);

    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).OutDryBulbTemp;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherOutDewPointAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).OutDewPointTemp;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherOutBarometricPressureAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).OutBaroPress;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherOutRelativeHumidityAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).OutRelHum;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherWindSpeedAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).WindSpeed;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherWindDirectionAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).WindDir;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherSkyTemperatureAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).SkyTemp;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherHorizontalIRSkyAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).HorizIRSky;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherBeamSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).BeamSolarRad;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherDiffuseSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).DifSolarRad;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherAlbedoAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).Albedo;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 todayWeatherLiquidPrecipitationAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsToday(timeStepNum, iHour).LiquidPrecip;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

int tomorrowWeatherIsRainAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return (int)thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).IsRain;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

int tomorrowWeatherIsSnowAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return (int)thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).IsSnow;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0;
}

Real64 tomorrowWeatherOutDryBulbAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).OutDryBulbTemp;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherOutDewPointAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).OutDewPointTemp;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherOutBarometricPressureAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).OutBaroPress;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherOutRelativeHumidityAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).OutRelHum;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherWindSpeedAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).WindSpeed;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherWindDirectionAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).WindDir;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherSkyTemperatureAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).SkyTemp;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherHorizontalIRSkyAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).HorizIRSky;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherBeamSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).BeamSolarRad;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherDiffuseSolarRadiationAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).DifSolarRad;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherAlbedoAtTime(EnergyPlusState state, int hour, int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).Albedo;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 tomorrowWeatherLiquidPrecipitationAtTime(EnergyPlusState state, const int hour, const int timeStepNum)
{
    auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    const int iHour = hour + 1;
    if ((iHour > 0) && (iHour <= EnergyPlus::Constant::iHoursInDay) && (timeStepNum > 0) && (timeStepNum <= thisState->dataGlobal->TimeStepsInHour)) {
        return thisState->dataWeather->wvarsHrTsTomorrow(timeStepNum, iHour).LiquidPrecip;
    }
    ShowSevereError(*thisState, "Invalid return from weather lookup, check hour and time step argument values are in range.");
    thisState->dataPluginManager->apiErrorFlag = true;
    return 0.0;
}

Real64 currentSimTime(EnergyPlusState state)
{
    const auto *thisState = static_cast<EnergyPlus::EnergyPlusData *>(state);
    return (thisState->dataGlobal->DayOfSim - 1) * EnergyPlus::Constant::iHoursInDay + currentTime(state);
}
