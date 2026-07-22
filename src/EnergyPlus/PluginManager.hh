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

#ifndef EPLUS_PLUGIN_MANAGER_HH
#define EPLUS_PLUGIN_MANAGER_HH

// C++ Headers
#include <queue>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EnergyPlus.hh>

#if LINK_WITH_PYTHON
#    ifndef PyObject_HEAD
struct _object;
using PyObject = _object;
#    endif
#endif

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PluginManagement {

    void registerNewCallback(const EnergyPlusData &state, EMSManager::EMSCallFrom iCalledFrom, const std::function<void(void *)> &f);
    void registerUserDefinedCallback(const EnergyPlusData &state, const std::function<void(void *)> &f, const std::string &programNameInInputFile);

    void runAnyRegisteredCallbacks(EnergyPlusData &state, EMSManager::EMSCallFrom iCalledFrom, bool &anyRan);
    void onBeginEnvironment(const EnergyPlusData &state);
    std::string pythonStringForUsage(const EnergyPlusData &state);

    void clear_state();

    struct PluginInstance
    {
        PluginInstance(const fs::path &_modulePath, const std::string &_className, std::string emsName, bool runPluginDuringWarmup);

        // members
        fs::path modulePath;
        std::string className;
        std::string emsAlias;
        bool runDuringWarmup;
        std::string stringIdentifier; // for diagnostic reporting

        // setup/shutdown should only be called once construction is completely done, i.e., setup() should only be called once the vector holding all
        // the instances is done for the day, and shutdown should only be called when you are ready to destruct all the instances.  The things that
        // happen inside setup() and shutdown() are related to un-managed memory, and it's tricky to manage inside existing constructor/move
        // operations, so they are split out into these explicitly called methods.
        void setup(EnergyPlusData &state);
        void shutdown() const;

        // methods
        bool run(EnergyPlusData &state, EMSManager::EMSCallFrom iCallingPoint) const; // calls main() on this plugin instance

        // plugin calling point hooks
        const char *sHookBeginNewEnvironment = "on_begin_new_environment";
        const char *sHookBeginZoneTimestepBeforeSetCurrentWeather = "on_begin_zone_timestep_before_set_current_weather";
        const char *sHookAfterNewEnvironmentWarmUpIsComplete = "on_after_new_environment_warmup_is_complete";
        const char *sHookBeginZoneTimestepBeforeInitHeatBalance = "on_begin_zone_timestep_before_init_heat_balance";
        const char *sHookBeginZoneTimestepAfterInitHeatBalance = "on_begin_zone_timestep_after_init_heat_balance";
        const char *sHookBeginTimestepBeforePredictor = "on_begin_timestep_before_predictor";
        const char *sHookAfterPredictorBeforeHVACManagers = "on_after_predictor_before_hvac_managers";
        const char *sHookAfterPredictorAfterHVACManagers = "on_after_predictor_after_hvac_managers";
        const char *sHookInsideHVACSystemIterationLoop = "on_inside_hvac_system_iteration_loop";
        const char *sHookEndOfZoneTimestepBeforeZoneReporting = "on_end_of_zone_timestep_before_zone_reporting";
        const char *sHookEndOfZoneTimestepAfterZoneReporting = "on_end_of_zone_timestep_after_zone_reporting";
        const char *sHookEndOfSystemTimestepBeforeHVACReporting = "on_end_of_system_timestep_before_hvac_reporting";
        const char *sHookEndOfSystemTimestepAfterHVACReporting = "on_end_of_system_timestep_after_hvac_reporting";
        const char *sHookEndOfZoneSizing = "on_end_of_zone_sizing";
        const char *sHookEndOfSystemSizing = "on_end_of_system_sizing";
        const char *sHookAfterComponentInputReadIn = "on_end_of_component_input_read_in";
        const char *sHookUserDefinedComponentModel = "on_user_defined_component_model";
        const char *sHookUnitarySystemSizing = "on_unitary_system_sizing";
        bool bHasBeginNewEnvironment = false;
        bool bHasBeginZoneTimestepBeforeSetCurrentWeather = false;
        bool bHasAfterNewEnvironmentWarmUpIsComplete = false;
        bool bHasBeginZoneTimestepBeforeInitHeatBalance = false;
        bool bHasBeginZoneTimestepAfterInitHeatBalance = false;
        bool bHasBeginTimestepBeforePredictor = false;
        bool bHasAfterPredictorBeforeHVACManagers = false;
        bool bHasAfterPredictorAfterHVACManagers = false;
        bool bHasInsideHVACSystemIterationLoop = false;
        bool bHasEndOfZoneTimestepBeforeZoneReporting = false;
        bool bHasEndOfZoneTimestepAfterZoneReporting = false;
        bool bHasEndOfSystemTimestepBeforeHVACReporting = false;
        bool bHasEndOfSystemTimestepAfterHVACReporting = false;
        bool bHasEndOfZoneSizing = false;
        bool bHasEndOfSystemSizing = false;
        bool bHasAfterComponentInputReadIn = false;
        bool bHasUserDefinedComponentModel = false;
        bool bHasUnitarySystemSizing = false;
#if LINK_WITH_PYTHON
        PyObject *pModule = nullptr;        // reference to module
        PyObject *pClassInstance = nullptr; // reference to instantiated class -- *don't decref until the end of the simulation*
        // precalculated function names as PyObjects
        PyObject *pBeginNewEnvironment = nullptr;
        PyObject *pBeginZoneTimestepBeforeSetCurrentWeather = nullptr;
        PyObject *pAfterNewEnvironmentWarmUpIsComplete = nullptr;
        PyObject *pBeginZoneTimestepBeforeInitHeatBalance = nullptr;
        PyObject *pBeginZoneTimestepAfterInitHeatBalance = nullptr;
        PyObject *pBeginTimestepBeforePredictor = nullptr;
        PyObject *pAfterPredictorBeforeHVACManagers = nullptr;
        PyObject *pAfterPredictorAfterHVACManagers = nullptr;
        PyObject *pInsideHVACSystemIterationLoop = nullptr;
        PyObject *pEndOfZoneTimestepBeforeZoneReporting = nullptr;
        PyObject *pEndOfZoneTimestepAfterZoneReporting = nullptr;
        PyObject *pEndOfSystemTimestepBeforeHVACReporting = nullptr;
        PyObject *pEndOfSystemTimestepAfterHVACReporting = nullptr;
        PyObject *pEndOfZoneSizing = nullptr;
        PyObject *pEndOfSystemSizing = nullptr;
        PyObject *pAfterComponentInputReadIn = nullptr;
        PyObject *pUserDefinedComponentModel = nullptr;
        PyObject *pUnitarySystemSizing = nullptr;
#endif
    };

    // TODO: Make this use PythonEngine so we don't duplicate code
    class PluginManager
    {
    public:
        explicit PluginManager(EnergyPlusData &state);
        ~PluginManager();

        static int numActiveCallbacks(const EnergyPlusData &state);
        static void setupOutputVariables(EnergyPlusData &state);

        int maxGlobalVariableIndex = -1;
        void addGlobalVariable(const EnergyPlusData &state, const std::string &name);
        static int getGlobalVariableHandle(EnergyPlusData &state, const std::string &name, bool suppress_warning = false);
        static Real64 getGlobalVariableValue(EnergyPlusData &state, int handle);
        static void setGlobalVariableValue(EnergyPlusData &state, int handle, Real64 value);

        int maxTrendVariableIndex = -1;
        static int getTrendVariableHandle(const EnergyPlusData &state, const std::string &name);
        static Real64 getTrendVariableValue(const EnergyPlusData &state, int handle, int timeIndex);
        static size_t getTrendVariableHistorySize(const EnergyPlusData &state, int handle);
        static Real64 getTrendVariableAverage(const EnergyPlusData &state, int handle, int count);
        static Real64 getTrendVariableMin(const EnergyPlusData &state, int handle, int count);
        static Real64 getTrendVariableMax(const EnergyPlusData &state, int handle, int count);
        static Real64 getTrendVariableSum(const EnergyPlusData &state, int handle, int count);
        static Real64 getTrendVariableDirection(const EnergyPlusData &state, int handle, int count);

        static void updatePluginValues(EnergyPlusData &state);

        static int getLocationOfUserDefinedPlugin(const EnergyPlusData &state, std::string const &_programName);
        static int getUserDefinedCallbackIndex(const EnergyPlusData &state, const std::string &callbackProgramName);
        static void runSingleUserDefinedPlugin(EnergyPlusData &state, int index);
        static void runSingleUserDefinedCallback(EnergyPlusData &state, int index);
        static bool anyUnexpectedPluginObjects(EnergyPlusData &state);

        bool eplusRunningViaPythonAPI = false;

        // For debugging purposes / issuing better error messages
        static std::vector<std::string> currentPythonPath();
    };

    struct PluginTrendVariable
    {
        std::string name;
        int numValues;
        std::deque<Real64> values;
        std::deque<Real64> times;
        int indexOfPluginVariable;
        PluginTrendVariable(const EnergyPlusData &state, std::string _name, int _numValues, int _indexOfPluginVariable);
        void reset()
        {
            this->values.clear();
            for (int i = 1; i <= this->numValues; i++) {
                this->values.push_back(0);
            }
        }
    };

} // namespace PluginManagement

struct PluginManagerData : BaseGlobalStruct
{
    std::map<EMSManager::EMSCallFrom, std::vector<std::function<void(void *)>>> callbacks;
    std::vector<std::string> userDefinedCallbackNames;
    std::vector<std::function<void(void *)>> userDefinedCallbacks;
    std::unique_ptr<PluginManagement::PluginManager> pluginManager;
    std::vector<PluginManagement::PluginTrendVariable> trends;
    std::vector<PluginManagement::PluginInstance> plugins;

    std::vector<std::string> globalVariableNames;
    std::vector<Real64> globalVariableValues;
    bool fullyReady = false;
    bool apiErrorFlag = false;
    std::vector<std::string> const objectsToFind = {
        "PythonPlugin:OutputVariable", "PythonPlugin:SearchPaths", "PythonPlugin:Instance", "PythonPlugin:Variables", "PythonPlugin:TrendVariable"};

    bool eplusRunningViaPythonAPI = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        callbacks.clear();
        userDefinedCallbackNames.clear();
        userDefinedCallbacks.clear();
#if LINK_WITH_PYTHON
        for (auto &plugin : plugins) {
            plugin.shutdown(); // clear unmanaged memory first
        }
        trends.clear();
        globalVariableNames.clear();
        globalVariableValues.clear();
        plugins.clear();
        fullyReady = false;
        apiErrorFlag = false;
        auto *p = pluginManager.release();
        delete p;
#endif
    }
};

} // namespace EnergyPlus

#endif // EPLUS_PLUGIN_MANAGER_HH
