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

// C++ Headers
#include <memory>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1.hh>

// EnergyPlus Headers
#include <EnergyPlus/CTElectricGenerator.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/FuelCellElectricGenerator.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/ICEngineElectricGenerator.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/MicroturbineElectricGenerator.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PVWatts.hh>
#include <EnergyPlus/Photovoltaics.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindTurbine.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#ifdef DEBUG_ARITHM_GCC_OR_CLANG
#    include <EnergyPlus/fenv_missing.h>
#endif

#ifdef DEBUG_ARITHM_MSVC
#    include <cfloat>
#endif

namespace EnergyPlus {

void createFacilityElectricPowerServiceObject(const EnergyPlusData &state)
{
    state.dataElectPwrSvcMgr->facilityElectricServiceObj = std::make_unique<ElectricPowerServiceManager>();
}

void initializeElectricPowerServiceZoneGains(const EnergyPlusData &state) // namespace routine for handling call from InternalHeatGains
{
    // internal zone gains need to be re initialized for begin new environment earlier than the main call into manage electric power service
    if (state.dataElectPwrSvcMgr->facilityElectricServiceObj->newEnvironmentInternalGainsFlag && state.dataGlobal->BeginEnvrnFlag) {
        state.dataElectPwrSvcMgr->facilityElectricServiceObj->reinitZoneGainsAtBeginEnvironment();
        state.dataElectPwrSvcMgr->facilityElectricServiceObj->newEnvironmentInternalGainsFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataElectPwrSvcMgr->facilityElectricServiceObj->newEnvironmentInternalGainsFlag = true;
    }
}

void ElectricPowerServiceManager::manageElectricPowerService(
    EnergyPlusData &state,
    bool const firstHVACIteration,
    bool &SimElecCircuits,      // simulation convergence flag
    bool const UpdateMetersOnly // if true then don't resimulate generators, just update meters.
)
{
    if (getInputFlag_) {
        getPowerManagerInput(state);
        getInputFlag_ = false;
    }

    if (state.dataGlobal->MetersHaveBeenInitialized && setupMeterIndexFlag_) {
        setupMeterIndices(state);
        setupMeterIndexFlag_ = false;
    }

    if (state.dataGlobal->BeginEnvrnFlag && newEnvironmentFlag_) {
        reinitAtBeginEnvironment();
        newEnvironmentFlag_ = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        newEnvironmentFlag_ = true;
    }

    // retrieve data from meters for demand and production
    totalBldgElecDemand_ =
        GetInstantMeterValue(state, elecFacilityMeterIndex_, OutputProcessor::TimeStepType::Zone) / state.dataGlobal->TimeStepZoneSec;
    totalHVACElecDemand_ =
        GetInstantMeterValue(state, elecFacilityMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    totalElectricDemand_ = totalBldgElecDemand_ + totalHVACElecDemand_;
    elecProducedPVRate_ =
        GetInstantMeterValue(state, elecProducedPVMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedWTRate_ =
        GetInstantMeterValue(state, elecProducedWTMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedStorageRate_ =
        GetInstantMeterValue(state, elecProducedStorageMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedCoGenRate_ =
        GetInstantMeterValue(state, elecProducedCoGenMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedPowerConversionRate_ = GetInstantMeterValue(state, elecProducedPowerConversionMeterIndex_, OutputProcessor::TimeStepType::System) /
                                       (state.dataHVACGlobal->TimeStepSysSec);

    wholeBldgRemainingLoad_ = totalElectricDemand_;

    if (UpdateMetersOnly) { // just update record keeping, don't resimulate load centers
        if (facilityPowerInTransformerPresent_) {
            facilityPowerInTransformerObj_->manageTransformers(state, 0.0);
        }

        updateWholeBuildingRecords(state);
        return;
    }

    for (auto &e : elecLoadCenterObjs) {
        e->manageElecLoadCenter(state, firstHVACIteration, wholeBldgRemainingLoad_);
    }

    updateWholeBuildingRecords(state);
    // The transformer call should be put outside of the "Load Center" loop because
    // 1) A transformer may be for utility, not for load center
    // 2) A tansformer may be shared by multiple load centers
    if (facilityPowerInTransformerPresent_) {
        facilityPowerInTransformerObj_->manageTransformers(state, 0.0);
    }

    updateWholeBuildingRecords(state);
    if (powerOutTransformerObj_ != nullptr) {
        powerOutTransformerObj_->manageTransformers(state, electSurplusRate_);
    }

    // Need to simulate through the Elec Manager at least twice to ensure that Heat Recovery information is included.
    // recheck this, may not be needed now that load centers are called more often.
    //  Does the IF condition also need to check if any thermal following strategies have been specified?
    //  That is, if only electrical following schemes, don't need to resimulate?
    if (firstHVACIteration) {
        SimElecCircuits = true;
    } else {
        SimElecCircuits = false;
    }
}

void ElectricPowerServiceManager::reinitZoneGainsAtBeginEnvironment()
{
    if (facilityPowerInTransformerPresent_) {
        facilityPowerInTransformerObj_->reinitZoneGainsAtBeginEnvironment();
    }
    if (powerOutTransformerObj_ != nullptr) {
        powerOutTransformerObj_->reinitZoneGainsAtBeginEnvironment();
    }
    if (numLoadCenters_ > 0) {
        for (auto &e : elecLoadCenterObjs) {
            e->reinitZoneGainsAtBeginEnvironment();
        }
    }
}

void ElectricPowerServiceManager::getPowerManagerInput(EnergyPlusData &state)
{
    static constexpr std::string_view routineName = "ElectricPowerServiceManager::getPowerManagerInput ";

    auto &s_ipsc = state.dataIPShortCut;

    numLoadCenters_ = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Distribution");

    if (numLoadCenters_ > 0) {
        for (int iLoadCenterNum = 1; iLoadCenterNum <= numLoadCenters_; ++iLoadCenterNum) {
            // call Electric Power Load Center constructor, in place
            elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(state, iLoadCenterNum));
        }
    } else {
        // issue #4639. see if there are any generators, inverters, converters, or storage devcies, that really need a ElectricLoadCenter:Distribution
        bool errorsFound(false);
        int numGenLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Generators");
        if (numGenLists > 0) {
            ShowSevereError(state, "ElectricLoadCenter:Generators input object requires an ElectricLoadCenterDistribution input object.");
            errorsFound = true;
        }
        int numInverters = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Inverter:Simple");
        numInverters += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Inverter:FunctionOfPower");
        numInverters += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Inverter:LookUpTable");
        if (numInverters > 0) {
            ShowSevereError(state, "ElectricLoadCenter:Inverter:* input objects require an ElectricLoadCenter:Distribution input object.");
            errorsFound = true;
        }
        int numStorage = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Storage:Simple");
        numStorage += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Storage:Battery");
        if (numStorage > 0) {
            ShowSevereError(state, "ElectricLoadCenter:Storage:* input objects require an ElectricLoadCenter:Distribution input object.");
            errorsFound = true;
        }
        int numGenerators = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:InternalCombustionEngine");
        numGenerators += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:CombustionTurbine");
        numGenerators += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:MicroCHP");
        numGenerators += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:FuelCell");
        numGenerators += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:Photovoltaic");
        numGenerators += state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Generator:WindTurbine");
        if (numGenerators > 0) {
            ShowSevereError(state, "Electric generator input objects require an ElectricLoadCenter:Distribution input object.");
            errorsFound = true;
        }

        if (errorsFound) {
            ShowFatalError(state, "Simulation halted because of missing input objects related to ElectricLoadCenter.");
        }

        // if user input did not include an Electric Load center, create a simple default one here for reporting purposes
        //   but only if there are any other electricity components set up (yet) for metering
        int anyElectricityPresent = GetMeterIndex(state, "ELECTRICITY:FACILITY");
        int anyPlantLoadProfilePresent = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "LoadProfile:Plant");
        if (anyElectricityPresent > -1 || anyPlantLoadProfilePresent > 0) {
            elecLoadCenterObjs.emplace_back(new ElectPowerLoadCenter(state, 0));
            numLoadCenters_ = 1;
        }
    }

    // see if there are any transformers of the type PowerInFromGrid
    numTransformers_ = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ElectricLoadCenter:Transformer");

    if (numTransformers_ > 0) {
        int numAlphas; // Number of elements in the alpha array
        int numNums;   // Number of elements in the numeric array
        int iOStat;    // IO Status when calling get input subroutine
        bool foundInFromGridTransformer = false;

        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Transformer";
        for (int loopTransformer = 1; loopTransformer <= numTransformers_; ++loopTransformer) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     loopTransformer,
                                                                     s_ipsc->cAlphaArgs,
                                                                     numAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     numNums,
                                                                     iOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            if (Util::SameString(s_ipsc->cAlphaArgs(3), "PowerInFromGrid")) {
                if (!foundInFromGridTransformer) {
                    foundInFromGridTransformer = true;
                    facilityPowerInTransformerName_ = s_ipsc->cAlphaArgs(1);
                    facilityPowerInTransformerPresent_ = true;
                } else {
                    // should only have one transformer in input that is PowerInFromGrid
                    ShowWarningError(state, format("{}{}=\"{}\", invalid entry.", routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3)));
                    ShowContinueError(state,
                                      "Only one transformer with Usage PowerInFromGrid can be used, first one in input file will be used and the "
                                      "simulation continues...");
                }
            } else if (Util::SameString(s_ipsc->cAlphaArgs(3), "PowerOutToGrid")) {
                if (powerOutTransformerObj_ == nullptr) {
                    ++numPowerOutTransformers_;
                    powerOutTransformerName_ = s_ipsc->cAlphaArgs(1);
                    powerOutTransformerObj_ = std::make_unique<ElectricTransformer>(state, powerOutTransformerName_);

                } else {
                    ShowWarningError(state,
                                     "Found more than one transformer set to PowerOutFromOnsiteGeneration, however only the first one will be used.");
                }
            }
        }
        if (foundInFromGridTransformer) {
            // call transformer constructor
            facilityPowerInTransformerObj_ = std::make_unique<ElectricTransformer>(state, facilityPowerInTransformerName_);
        }
    } // if transformers

    if (numLoadCenters_ > 0) {
        SetupOutputVariable(state,
                            "Facility Total Purchased Electricity Rate",
                            Constant::Units::W,
                            electPurchRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Facility Total Purchased Electricity Energy",
                            Constant::Units::J,
                            electricityPurch_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityPurchased,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Cogeneration);

        SetupOutputVariable(state,
                            "Facility Total Surplus Electricity Rate",
                            Constant::Units::W,
                            electSurplusRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Facility Total Surplus Electricity Energy",
                            Constant::Units::J,
                            electricitySurplus_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricitySurplusSold,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Cogeneration);

        SetupOutputVariable(state,
                            "Facility Net Purchased Electricity Rate",
                            Constant::Units::W,
                            electricityNetRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Facility Net Purchased Electricity Energy",
                            Constant::Units::J,
                            electricityNet_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityNet,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Cogeneration);

        SetupOutputVariable(state,
                            "Facility Total Building Electricity Demand Rate",
                            Constant::Units::W,
                            totalBldgElecDemand_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Facility Total HVAC Electricity Demand Rate",
                            Constant::Units::W,
                            totalHVACElecDemand_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Facility Total Electricity Demand Rate",
                            Constant::Units::W,
                            totalElectricDemand_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);

        SetupOutputVariable(state,
                            "Facility Total Produced Electricity Rate",
                            Constant::Units::W,
                            electProdRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Facility Total Produced Electricity Energy",
                            Constant::Units::J,
                            electricityProd_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);

        reportPVandWindCapacity(state);

        sumUpNumberOfStorageDevices();

        checkLoadCenters(state);
    }
}

void ElectricPowerServiceManager::setupMeterIndices(EnergyPlusData &state)
{
    elecFacilityMeterIndex_ = GetMeterIndex(state, "ELECTRICITY:FACILITY");
    elecProducedCoGenMeterIndex_ = GetMeterIndex(state, "COGENERATION:ELECTRICITYPRODUCED");
    elecProducedPVMeterIndex_ = GetMeterIndex(state, "PHOTOVOLTAIC:ELECTRICITYPRODUCED");
    elecProducedWTMeterIndex_ = GetMeterIndex(state, "WINDTURBINE:ELECTRICITYPRODUCED");
    elecProducedStorageMeterIndex_ = GetMeterIndex(state, "ELECTRICSTORAGE:ELECTRICITYPRODUCED");
    elecProducedPowerConversionMeterIndex_ = GetMeterIndex(state, "POWERCONVERSION:ELECTRICITYPRODUCED");

    if (numLoadCenters_ > 0) {
        for (auto &e : elecLoadCenterObjs) {
            e->setupLoadCenterMeterIndices(state);
        }
    }
    if (facilityPowerInTransformerPresent_) {
        facilityPowerInTransformerObj_->setupMeterIndices(state);
    }
}

void ElectricPowerServiceManager::reinitAtBeginEnvironment()
{
    wholeBldgRemainingLoad_ = 0.0;
    electricityProd_ = 0.0;
    electProdRate_ = 0.0;
    electricityPurch_ = 0.0;
    electPurchRate_ = 0.0;
    electSurplusRate_ = 0.0;
    electricitySurplus_ = 0.0;
    electricityNetRate_ = 0.0;
    electricityNet_ = 0.0;
    totalBldgElecDemand_ = 0.0;
    totalHVACElecDemand_ = 0.0;
    totalElectricDemand_ = 0.0;
    elecProducedPVRate_ = 0.0;
    elecProducedWTRate_ = 0.0;
    elecProducedStorageRate_ = 0.0;
    elecProducedCoGenRate_ = 0.0;

    if (numLoadCenters_ > 0) {
        for (auto &e : elecLoadCenterObjs) {
            e->reinitAtBeginEnvironment();
        }
    }
    if (facilityPowerInTransformerPresent_) {
        facilityPowerInTransformerObj_->reinitAtBeginEnvironment();
    }
    if (powerOutTransformerObj_ != nullptr) {
        powerOutTransformerObj_->reinitAtBeginEnvironment();
    }
}

void ElectricPowerServiceManager::verifyCustomMetersElecPowerMgr(EnergyPlusData &state)
{
    for (std::size_t loop = 0; loop < elecLoadCenterObjs.size(); ++loop) {
        elecLoadCenterObjs[loop]->setupLoadCenterMeterIndices(state);
    }
}

void ElectricPowerServiceManager::updateWholeBuildingRecords(EnergyPlusData &state)
{

    // main panel balancing.
    totalBldgElecDemand_ =
        GetInstantMeterValue(state, elecFacilityMeterIndex_, OutputProcessor::TimeStepType::Zone) / state.dataGlobal->TimeStepZoneSec;
    totalHVACElecDemand_ =
        GetInstantMeterValue(state, elecFacilityMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    totalElectricDemand_ = totalBldgElecDemand_ + totalHVACElecDemand_;
    elecProducedPVRate_ =
        GetInstantMeterValue(state, elecProducedPVMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedWTRate_ =
        GetInstantMeterValue(state, elecProducedWTMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedStorageRate_ =
        GetInstantMeterValue(state, elecProducedStorageMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedCoGenRate_ =
        GetInstantMeterValue(state, elecProducedCoGenMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
    elecProducedPowerConversionRate_ = GetInstantMeterValue(state, elecProducedPowerConversionMeterIndex_, OutputProcessor::TimeStepType::System) /
                                       (state.dataHVACGlobal->TimeStepSysSec);

    electProdRate_ = elecProducedCoGenRate_ + elecProducedPVRate_ + elecProducedWTRate_ + elecProducedStorageRate_ + elecProducedPowerConversionRate_;
    electricityProd_ = electProdRate_ * state.dataHVACGlobal->TimeStepSysSec; // whole building

    // Report the Total Electric Power Purchased [W], If negative then there is extra power to be sold or stored.
    electPurchRate_ = totalElectricDemand_ - electProdRate_;
    // Check this value against a tolerance to aid in reporting.
    if (std::abs(electPurchRate_) < 0.0001) {
        electPurchRate_ = 0.0;
    }
    if (electPurchRate_ < 0.0) {
        electPurchRate_ = 0.0; // don't want negative purchased...
    }

    // Report the Total Electric Energy Purchased [J]
    electricityPurch_ = electPurchRate_ * state.dataHVACGlobal->TimeStepSysSec;

    // report the total electric surplus....
    electSurplusRate_ = electProdRate_ - totalElectricDemand_;
    if (std::abs(electSurplusRate_) < 0.0001) {
        electSurplusRate_ = 0.0;
    }
    if (electSurplusRate_ < 0.0) {
        electSurplusRate_ = 0.0; // don't want negative surplus
    }

    electricitySurplus_ = electSurplusRate_ * state.dataHVACGlobal->TimeStepSysSec;

    // report the net electricity , + is purchased, - is surplus
    electricityNetRate_ = totalElectricDemand_ - electProdRate_;

    electricityNet_ = electricityNetRate_ * state.dataHVACGlobal->TimeStepSysSec;
}

void ElectricPowerServiceManager::reportPVandWindCapacity(EnergyPlusData &state)
{
    // LEED report
    pvTotalCapacity_ = 0.0;
    windTotalCapacity_ = 0.0;
    for (auto const &lc : elecLoadCenterObjs) {
        if (lc->numGenerators > 0) {
            for (auto const &g : lc->elecGenCntrlObj) {
                if ((g->generatorType == GeneratorType::PV) || (g->generatorType == GeneratorType::PVWatts)) {
                    pvTotalCapacity_ += g->maxPowerOut;
                }
                if (g->generatorType == GeneratorType::WindTurbine) {
                    windTotalCapacity_ += g->maxPowerOut;
                }
            }
        }
    }
    // put in total capacity for PV and Wind for LEED report
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedRenRatCap, "Photovoltaic", pvTotalCapacity_ / 1000, 2);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedRenRatCap, "Wind", windTotalCapacity_ / 1000, 2);

    // future work: this legacy approach is relying on the correct power output to have been placed in the Generator list.  There could be a
    // difference between this control input and the actual size of the systems as defined in the generator objects themselves.  This method should be
    // replaced with queries that check the capacity from the generator models.
}

void ElectricPowerServiceManager::sumUpNumberOfStorageDevices()
{
    numElecStorageDevices = 0;
    for (auto const &e : elecLoadCenterObjs) {
        if (e->storageObj != nullptr) {
            ++numElecStorageDevices;
        }
    }
}

void ElectricPowerServiceManager::checkLoadCenters(EnergyPlusData &state)
{

    // issue #5302, detect if storage used on more than one load center. This is really a kind of GlobalNames issue.
    // expanded to all devices on a load center
    bool errorsFound = false;

    // first fill in a vector of names
    std::vector<std::string> storageNames;
    std::vector<std::string> genListNames;
    std::vector<std::string> inverterNames;
    std::vector<std::string> converterNames;
    std::vector<std::string> transformerNames;
    for (auto &e : elecLoadCenterObjs) {
        if (e->storageObj != nullptr) {
            storageNames.emplace_back(e->storageObj->name());
        }
        if (!e->elecGenCntrlObj.empty()) {
            genListNames.emplace_back(e->generatorListName());
        }
        if (e->inverterObj != nullptr) {
            inverterNames.emplace_back(e->inverterObj->name());
        }
        if (e->converterObj != nullptr) {
            converterNames.emplace_back(e->converterObj->name());
        }
        if (e->transformerObj != nullptr) {
            transformerNames.emplace_back(e->transformerObj->name());
        }
    }

    // then check the vectors for duplicates.
    for (std::size_t i = 0; i < storageNames.size(); ++i) {
        for (std::size_t j = 0; j < storageNames.size(); ++j) {
            if (storageNames[i] == storageNames[j] && i != j) {
                ShowSevereError(state,
                                format("ElectricPowerServiceManager::checkLoadCenters, the electrical storage device named = {} is used in more than "
                                       "one ElectricLoadCenter:Distribution input object.",
                                       storageNames[i]));
                ShowContinueError(state, "Electric Load Centers cannot share the same storage device.");
                errorsFound = true;
                break;
            }
        }
        if (errorsFound) {
            break;
        }
    }

    for (std::size_t i = 0; i < genListNames.size(); ++i) {
        for (std::size_t j = 0; j < genListNames.size(); ++j) {
            if (genListNames[i] == genListNames[j] && i != j) {
                ShowSevereError(state,
                                format("ElectricPowerServiceManager::checkLoadCenters, the generator list named = {} is used in more than one "
                                       "ElectricLoadCenter:Distribution input object.",
                                       genListNames[i]));
                ShowContinueError(state, "Electric Load Centers cannot share the same generator list (ElectricLoadCenter:Generators).");
                errorsFound = true;
                break;
            }
        }
        if (errorsFound) {
            break;
        }
    }

    for (std::size_t i = 0; i < inverterNames.size(); ++i) {
        for (std::size_t j = 0; j < inverterNames.size(); ++j) {
            if (inverterNames[i] == inverterNames[j] && i != j) {
                ShowSevereError(state,
                                format("ElectricPowerServiceManager::checkLoadCenters, the inverter device named = {} is used in more than one "
                                       "ElectricLoadCenter:Distribution input object.",
                                       inverterNames[i]));
                ShowContinueError(state, "Electric Load Centers cannot share the same inverter device.");
                errorsFound = true;
                break;
            }
        }
        if (errorsFound) {
            break;
        }
    }

    for (std::size_t i = 0; i < converterNames.size(); ++i) {
        for (std::size_t j = 0; j < converterNames.size(); ++j) {
            if (converterNames[i] == converterNames[j] && i != j) {
                ShowSevereError(state,
                                format("ElectricPowerServiceManager::checkLoadCenters, the converter device named = {} is used in more than one "
                                       "ElectricLoadCenter:Distribution input object.",
                                       converterNames[i]));
                ShowContinueError(state, "Electric Load Centers cannot share the same converter device.");
                errorsFound = true;
                break;
            }
        }
        if (errorsFound) {
            break;
        }
    }

    for (std::size_t i = 0; i < transformerNames.size(); ++i) {
        for (std::size_t j = 0; j < transformerNames.size(); ++j) {
            if (transformerNames[i] == transformerNames[j] && i != j) {
                ShowSevereError(state,
                                format("ElectricPowerServiceManager::checkLoadCenters, the transformer device named = {} is used in more than one "
                                       "ElectricLoadCenter:Distribution input object.",
                                       transformerNames[i]));
                ShowContinueError(state, "Electric Load Centers cannot share the same transformer device.");
                errorsFound = true;
                break;
            }
        }
        if (errorsFound) {
            break;
        }
    }

    if (errorsFound) { // throw fatal, these errors could fatal out in internal gains with missleading data
        ShowFatalError(state, "ElectricPowerServiceManager::checkLoadCenters, preceding errors terminate program.");
    }
}

// TODO: Absolutely not. Constructors should not do this much work,
// because if this fails, then the constructor fails and who knows who
// will be referencing this object.
ElectPowerLoadCenter::ElectPowerLoadCenter(EnergyPlusData &state, int const objectNum)
    : numGenerators(0), bussType(ElectricBussType::Invalid), thermalProd(0.0), thermalProdRate(0.0), inverterPresent(false),
      subpanelFeedInRequest(0.0), subpanelFeedInRate(0.0), subpanelDrawRate(0.0), genElectricProd(0.0), genElectProdRate(0.0), storOpCVDrawRate(0.0),
      storOpCVFeedInRate(0.0), storOpCVChargeRate(0.0), storOpCVDischargeRate(0.0), storOpIsCharging(false), storOpIsDischarging(false),
      genOperationScheme_(GeneratorOpScheme::Invalid), demandMeterPtr_(0), generatorsPresent_(false), myCoGenSetupFlag_(true), demandLimit_(0.0),
      storagePresent_(false), transformerPresent_(false), totalPowerRequest_(0.0), totalThermalPowerRequest_(0.0),
      storageScheme_(StorageOpScheme::Invalid), trackStorageOpMeterIndex_(0), converterPresent_(false), maxStorageSOCFraction_(1.0),
      minStorageSOCFraction_(0.0), designStorageChargePower_(0.0), designStorageChargePowerWasSet_(false), designStorageDischargePower_(0.0),
      designStorageDischargePowerWasSet_(false), facilityDemandTarget_(0.0), eMSOverridePelFromStorage_(false), // if true, EMS calling for override
      eMSValuePelFromStorage_(0.0),      // value EMS is directing to use, power from storage [W]
      eMSOverridePelIntoStorage_(false), // if true, EMS calling for override
      eMSValuePelIntoStorage_(0.0)       // value EMS is directing to use, power into storage [W]
{

    static constexpr std::string_view routineName = "ElectPowerLoadCenter constructor ";
    int numAlphas; // Number of elements in the alpha array
    int numNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine

    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Distribution";
    bool errorsFound = false;
    if (objectNum > 0) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 objectNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 numAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 numNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        name_ = s_ipsc->cAlphaArgs(1);
        // how to verify names are unique across objects? add to GlobalNames?

        if (!s_ipsc->lAlphaFieldBlanks(2)) {
            generatorListName_ = s_ipsc->cAlphaArgs(2);
            // check that

            int testIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Generators", generatorListName_);
            if (testIndex == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                errorsFound = true;
            }
        }

        if (!s_ipsc->lAlphaFieldBlanks(3)) {
            // Load the Generator Control Operation Scheme
            genOperationScheme_ = static_cast<GeneratorOpScheme>(getEnumValue(generatorOpSchemeNamesUC, s_ipsc->cAlphaArgs(3)));
            if (genOperationScheme_ == GeneratorOpScheme::Invalid) {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                errorsFound = true;
            }
        }

        demandLimit_ = s_ipsc->rNumericArgs(1);

        if (genOperationScheme_ == GeneratorOpScheme::TrackSchedule) {
            if (s_ipsc->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(4));
                errorsFound = true;
            } else if ((trackSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                errorsFound = true;
            }
        }

        demandMeterName_ = Util::makeUPPER(s_ipsc->cAlphaArgs(5));
        // meters may not be "loaded" yet, defered check to later subroutine

        if (s_ipsc->cAlphaArgs(6).empty()) {
            bussType = ElectricBussType::ACBuss;
        } else if ((bussType = static_cast<ElectricBussType>(getEnumValue(electricBussTypeNamesUC, s_ipsc->cAlphaArgs(6)))) ==
                   ElectricBussType::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
            errorsFound = true;
        } else if (bussType == ElectricBussType::DCBussInverter) {
            inverterPresent = true;
        } else if (bussType == ElectricBussType::ACBussStorage) {
            storagePresent_ = true;
        } else if (bussType == ElectricBussType::DCBussInverterDCStorage || bussType == ElectricBussType::DCBussInverterACStorage) {
            inverterPresent = true;
            storagePresent_ = true;
        }

        if (inverterPresent) {
            if (!s_ipsc->lAlphaFieldBlanks(7)) {
                inverterName = s_ipsc->cAlphaArgs(7);
            } else {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                errorsFound = true;
            }
        }

        if (storagePresent_) {
            if (!s_ipsc->lAlphaFieldBlanks(8)) {
                storageName_ = s_ipsc->cAlphaArgs(8);
            } else {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                errorsFound = true;
            }
        }

        if (!s_ipsc->lAlphaFieldBlanks(9)) {
            // process transformer
            transformerName_ = s_ipsc->cAlphaArgs(9);
            // only transformers of use type powerFromLoadCenterToBldg are really held in a load center, The legacy applications for transformers are
            // held at the higher Electric service level
            transformerPresent_ = true;
        }

        // Begin new content for grid supply and more control over storage
        // user selected storage operation scheme
        if (s_ipsc->lAlphaFieldBlanks(10)) {
            storageScheme_ = StorageOpScheme::FacilityDemandStoreExcessOnSite;
        } else if ((storageScheme_ = static_cast<StorageOpScheme>(getEnumValue(storageOpSchemeNamesUC, s_ipsc->cAlphaArgs(10)))) ==
                   StorageOpScheme::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
            errorsFound = true;
        }

        if (!s_ipsc->lAlphaFieldBlanks(11)) {
            trackSorageOpMeterName_ = s_ipsc->cAlphaArgs(11);
        } else if (storageScheme_ == StorageOpScheme::MeterDemandStoreExcessOnSite) { // throw error
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(11), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
            errorsFound = true;
        }

        if (!s_ipsc->lAlphaFieldBlanks(12)) {
            converterName_ = s_ipsc->cAlphaArgs(12);
            converterPresent_ = true;
        } else if (storageScheme_ == StorageOpScheme::ChargeDischargeSchedules || storageScheme_ == StorageOpScheme::FacilityDemandLeveling) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(12), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
            errorsFound = true;
        }

        if (s_ipsc->lNumericFieldBlanks(2)) {
            maxStorageSOCFraction_ = 1.0;
        } else {
            maxStorageSOCFraction_ = s_ipsc->rNumericArgs(2);
        }
        if (s_ipsc->lNumericFieldBlanks(3)) {
            minStorageSOCFraction_ = 0.0;
        } else {
            minStorageSOCFraction_ = s_ipsc->rNumericArgs(3);
        }
        if (s_ipsc->lNumericFieldBlanks(4)) {
            designStorageChargePowerWasSet_ = false;
        } else {
            designStorageChargePowerWasSet_ = true;
            designStorageChargePower_ = s_ipsc->rNumericArgs(4);
        }
        if (s_ipsc->lNumericFieldBlanks(5)) {
            designStorageDischargePowerWasSet_ = false;
        } else {
            designStorageDischargePowerWasSet_ = true;
            designStorageDischargePower_ = s_ipsc->rNumericArgs(5);
        }

        if (!s_ipsc->lNumericFieldBlanks(6)) {
            facilityDemandTarget_ = s_ipsc->rNumericArgs(6);
        } else if (storageScheme_ == StorageOpScheme::FacilityDemandLeveling) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cNumericFieldNames(6), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
            errorsFound = true;
        }

        if (storageScheme_ == StorageOpScheme::ChargeDischargeSchedules) {
            if (s_ipsc->lAlphaFieldBlanks(13)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(13), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
                errorsFound = true;
            } else if ((storageChargeModSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(13))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(13), s_ipsc->cAlphaArgs(13));
                errorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(14)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(14), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
                errorsFound = true;
            } else if ((storageDischargeModSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(14))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(14), s_ipsc->cAlphaArgs(14));
                errorsFound = true;
            }
        }

        if (storageScheme_ == StorageOpScheme::FacilityDemandLeveling) {
            if (s_ipsc->lAlphaFieldBlanks(15)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(15), s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
                errorsFound = true;
            } else if ((facilityDemandTargetModSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(15))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(15), s_ipsc->cAlphaArgs(15));
                errorsFound = true;
            }
        }

    } else { // object num == 0
        // just construct an empty object and return
        return;
    }

    // now that we are done with processing get input for ElectricLoadCenter:Distribution we can call child input objects without IP shortcut problems
    s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Generators";
    int genListObjectNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, s_ipsc->cCurrentModuleObject, generatorListName_);
    if (genListObjectNum > 0) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 genListObjectNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 numAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 numNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        // Calculate the number of generators in list
        numGenerators = numNums / 2; // note IDD needs Min Fields = 6
        if (mod((numAlphas - 1 + numNums), 5) != 0) {
            ++numGenerators;
        }
        int alphaCount = 2;
        for (int genCount = 1; genCount <= numGenerators; ++genCount) {
            // call constructor in place
            generatorsPresent_ = true;
            elecGenCntrlObj.emplace_back(new GeneratorController(state,
                                                                 s_ipsc->cAlphaArgs(alphaCount),
                                                                 s_ipsc->cAlphaArgs(alphaCount + 1),
                                                                 s_ipsc->rNumericArgs(2 * genCount - 1),
                                                                 s_ipsc->cAlphaArgs(alphaCount + 2),
                                                                 s_ipsc->rNumericArgs(2 * genCount)));
            ++alphaCount;
            ++alphaCount;
            ++alphaCount;
        }

        // issue #5299 check for non-zero values in thermal electric ratio if gen op scheme is ThermalFollow*
        if (genOperationScheme_ == GeneratorOpScheme::ThermalFollow || genOperationScheme_ == GeneratorOpScheme::ThermalFollowLimitElectrical) {
            // check to make sure the user didn't input zeros for thermalToElectricControlRatio
            for (auto const &g : elecGenCntrlObj) {
                if (g->nominalThermElectRatio <= 0.0) {
                    ShowWarningError(state,
                                     format("Generator operation needs to be based on following thermal loads and needs values for Rated Thermal to "
                                            "Electrical Power Ratio in {} named {}",
                                            s_ipsc->cCurrentModuleObject,
                                            s_ipsc->cAlphaArgs(1)));
                }
            }
        }
    }

    if (!errorsFound && inverterPresent) {
        // call inverter constructor
        inverterObj = std::make_unique<DCtoACInverter>(state, inverterName);

        // Make sure only Generator::PVWatts are used with Inverter:PVWatts
        // Add up the total DC capacity and pass it to the inverter.
        if (inverterObj->modelType() == DCtoACInverter::InverterModelType::PVWatts) {
            Real64 totalDCCapacity = 0.0;
            for (const auto &generatorController : elecGenCntrlObj) {
                if (generatorController->generatorType != GeneratorType::PVWatts) {
                    errorsFound = true;
                    ShowSevereError(state, format("{}ElectricLoadCenter:Distribution=\"{}\",", routineName, name_));
                    ShowContinueError(state, "ElectricLoadCenter:Inverter:PVWatts can only be used with Generator:PVWatts");
                    ShowContinueError(state,
                                      format("\"{}\" is of type {}",
                                             generatorController->name,
                                             generatorTypeNames[static_cast<int>(generatorController->generatorType)]));
                } else {
                    totalDCCapacity += generatorController->pvwattsGenerator->getDCSystemCapacity();

                    // Pass the inverter properties to the PVWatts generator class
                    generatorController->pvwattsGenerator->setDCtoACRatio(inverterObj->pvWattsDCtoACSizeRatio());
                    generatorController->pvwattsGenerator->setInverterEfficiency(inverterObj->pvWattsInverterEfficiency());
                }
            }
            if (!errorsFound) {
                inverterObj->setPVWattsDCCapacity(state, totalDCCapacity);
            }
        }
    }

    if (!errorsFound && storagePresent_) {
        // call storage constructor
        storageObj = std::make_unique<ElectricStorage>(state, storageName_);
    }

    if (!errorsFound && transformerPresent_) {

        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Transformer";
        int transformerItemNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, s_ipsc->cCurrentModuleObject, transformerName_);
        if (transformerItemNum > 0) {
            int iOStat;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     transformerItemNum,
                                                                     s_ipsc->cAlphaArgs,
                                                                     numAlphas,
                                                                     s_ipsc->rNumericArgs,
                                                                     numNums,
                                                                     iOStat,
                                                                     s_ipsc->lNumericFieldBlanks,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);
            if (Util::SameString(s_ipsc->cAlphaArgs(3),
                                 "LoadCenterPowerConditioning")) { // this is the right kind of transformer
                transformerObj = std::make_unique<ElectricTransformer>(state, transformerName_);
            } else {
                ShowWarningError(
                    state,
                    format("Transformer named {} associated with the load center named {} should have {} set to LoadCenterPowerConditioning.",
                           transformerName_,
                           name_,
                           s_ipsc->cAlphaFieldNames(3)));
            }
        } else {
            ShowSevereError(state, format("Transformer named {}, was not found for the load center named {}", transformerName_, name_));
            errorsFound = true;
        }
    }

    if (!errorsFound && converterPresent_) {
        // call AC to DC converter constructor
        converterObj = std::make_unique<ACtoDCConverter>(state, converterName_);
    }

    // Setup general output variables for reporting in the electric load center
    SetupOutputVariable(state,
                        "Electric Load Center Produced Electricity Rate",
                        Constant::Units::W,
                        genElectProdRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name_);
    SetupOutputVariable(state,
                        "Electric Load Center Produced Electricity Energy",
                        Constant::Units::J,
                        genElectricProd,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        name_);
    SetupOutputVariable(state,
                        "Electric Load Center Supplied Electricity Rate",
                        Constant::Units::W,
                        subpanelFeedInRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name_);
    SetupOutputVariable(state,
                        "Electric Load Center Drawn Electricity Rate",
                        Constant::Units::W,
                        subpanelDrawRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name_);
    SetupOutputVariable(state,
                        "Electric Load Center Produced Thermal Rate",
                        Constant::Units::W,
                        thermalProdRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name_);
    SetupOutputVariable(state,
                        "Electric Load Center Produced Thermal Energy",
                        Constant::Units::J,
                        thermalProd,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        name_);
    SetupOutputVariable(state,
                        "Electric Load Center Requested Electricity Rate",
                        Constant::Units::W,
                        totalPowerRequest_,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name_);

    if (state.dataGlobal->AnyEnergyManagementSystemInModel && storagePresent_) {
        SetupEMSActuator(state, "Electrical Storage", name_, "Power Draw Rate", "[W]", eMSOverridePelFromStorage_, eMSValuePelFromStorage_);
        SetupEMSActuator(state, "Electrical Storage", name_, "Power Charge Rate", "[W]", eMSOverridePelIntoStorage_, eMSValuePelIntoStorage_);
    }

    if (errorsFound) {
        ShowFatalError(state, format("{}Preceding errors terminate program.", routineName));
    }
}

void ElectPowerLoadCenter::manageElecLoadCenter(EnergyPlusData &state, bool const firstHVACIteration, Real64 &remainingWholePowerDemand)
{
    //
    subpanelFeedInRequest = remainingWholePowerDemand;

    if (generatorsPresent_) {
        dispatchGenerators(state, firstHVACIteration, remainingWholePowerDemand);

    } // if generators present
    updateLoadCenterGeneratorRecords(state);
    if (bussType == ElectricBussType::DCBussInverter || bussType == ElectricBussType::DCBussInverterACStorage) {
        inverterObj->simulate(state, genElectProdRate);
    }

    if (storagePresent_) {
        storageObj->timeCheckAndUpdate(state);
        dispatchStorage(state, subpanelFeedInRequest);
    }

    if (bussType == ElectricBussType::DCBussInverterDCStorage) {
        if (inverterObj != nullptr) {
            inverterObj->simulate(state, storOpCVFeedInRate);
        }
    }

    if (converterObj != nullptr) {
        converterObj->simulate(state, storOpCVDrawRate);
    }

    if (transformerObj != nullptr) {
        if (storOpCVFeedInRate > 0.0) {
            transformerObj->manageTransformers(state, storOpCVFeedInRate);
        } else if (storOpCVDrawRate > 0.0) {
            transformerObj->manageTransformers(state, subpanelDrawRate);
        }
    }
    updateLoadCenterGeneratorRecords(state);
}

void ElectPowerLoadCenter::dispatchGenerators(EnergyPlusData &state,
                                              bool const firstHVACIteration,
                                              Real64 &remainingWholePowerDemand // power request in, remaining unmet request out
)
{

    // This funciton checks generator operation scheme and assigns requests to power generators
    // the generators are called to simulate from here and passed some control data
    // the actual production from each generator is recorded and accounting tracks how much of the load is met

    // If a generator is needed in the simulation for a small load and it is less than the minimum part load ratio
    // the generator will operate at the minimum part load ratio and the excess will either reduce demand or
    // be available for storage or sell back to the power company.

    // Both the Demand Limit and Track Electrical schemes will sequentially load the available generators.  All demand
    Real64 loadCenterElectricLoad = 0.0;
    Real64 remainingLoad = 0.0;
    Real64 customMeterDemand = 0.0;

    switch (genOperationScheme_) {

    case GeneratorOpScheme::BaseLoad: {

        loadCenterElectricLoad = remainingWholePowerDemand;

        for (auto &g : elecGenCntrlObj) {

            if (g->availSched->getCurrentVal() > 0.0) {
                // Set the Operation Flag
                g->onThisTimestep = true;
                // Set the electric generator load request
                g->powerRequestThisTimestep = g->maxPowerOut;
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;
            }

            // now handle EMS override
            if (g->eMSRequestOn) {
                g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                if (g->powerRequestThisTimestep > 0.0) {
                    g->onThisTimestep = true;
                } else {
                    g->onThisTimestep = false;
                }
            }

            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            totalPowerRequest_ += g->powerRequestThisTimestep;
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining load
        }
        break;
    }
    case GeneratorOpScheme::DemandLimit: {
        // The Demand Limit scheme tries to have the generators meet all of the demand above the purchased Electric
        //  limit set by the user.
        remainingLoad = remainingWholePowerDemand - demandLimit_;
        loadCenterElectricLoad = remainingLoad;

        for (auto &g : elecGenCntrlObj) {

            if (g->availSched->getCurrentVal() > 0.0 && remainingLoad > 0.0) {
                // Set the Operation Flag
                g->onThisTimestep = true;

                // Set the electric generator load
                g->powerRequestThisTimestep = min(g->maxPowerOut, remainingLoad);

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            }

            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            if (g->eMSRequestOn) {
                totalPowerRequest_ += max(g->eMSPowerRequest, 0.0);
            } else {
                if (g->powerRequestThisTimestep > 0.0) {
                    totalPowerRequest_ += g->maxPowerOut;
                    totalPowerRequest_ = min(loadCenterElectricLoad, totalPowerRequest_);
                }
            }
            remainingLoad -= g->electProdRate;             // Update remaining load to be met by this load center
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining load
        }
        break;
    }
    case GeneratorOpScheme::TrackElectrical: {
        // The Track Electrical scheme tries to have the generators meet all of the electrical demand for the building.
        remainingLoad = remainingWholePowerDemand;
        loadCenterElectricLoad = remainingLoad;

        for (auto &g : elecGenCntrlObj) {

            if (g->availSched->getCurrentVal() > 0.0 && remainingLoad > 0.0) {
                // Set the Operation Flag
                g->onThisTimestep = true;

                // Set the electric generator load
                g->powerRequestThisTimestep = min(g->maxPowerOut, remainingLoad);

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;
                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            }

            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            if (g->eMSRequestOn) {
                totalPowerRequest_ += max(g->eMSPowerRequest, 0.0);
            } else {
                if (g->powerRequestThisTimestep > 0.0) {
                    totalPowerRequest_ += g->maxPowerOut;
                    totalPowerRequest_ = min(loadCenterElectricLoad, totalPowerRequest_);
                }
            }
            remainingLoad -= g->electProdRate;             // Update remaining load to be met by this load center
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining load
        }
        break;
    }
    case GeneratorOpScheme::TrackSchedule: {
        // The Track Schedule scheme tries to have the generators meet the electrical demand determined from a schedule.
        //  Code is very similar to 'Track Electrical' except for initial RemainingLoad is replaced by SchedElecDemand
        //  and PV production is ignored.
        remainingLoad = trackSched_->getCurrentVal();
        loadCenterElectricLoad = remainingLoad;

        for (auto &g : elecGenCntrlObj) {

            if (g->availSched->getCurrentVal() > 0.0 && remainingLoad > 0.0) {
                // Set the Operation Flag
                g->onThisTimestep = true;

                // Set the electric generator load
                g->powerRequestThisTimestep = min(g->maxPowerOut, remainingLoad);

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            }

            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            if (g->eMSRequestOn) {
                totalPowerRequest_ += max(g->eMSPowerRequest, 0.0);
            } else {
                if (g->powerRequestThisTimestep > 0.0) {
                    totalPowerRequest_ += g->maxPowerOut;
                    totalPowerRequest_ = min(loadCenterElectricLoad, totalPowerRequest_);
                }
            }
            remainingLoad -= g->electProdRate;             // Update remaining load to be met by this load center
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining load
        }
        break;
    }
    case GeneratorOpScheme::TrackMeter: {
        // The TRACK CUSTOM METER scheme tries to have the generators meet all of the
        //   electrical demand from a meter, it can also be a user-defined Custom Meter
        //   and PV is ignored.
        customMeterDemand =
            GetInstantMeterValue(state, demandMeterPtr_, OutputProcessor::TimeStepType::Zone) / state.dataGlobal->TimeStepZoneSec +
            GetInstantMeterValue(state, demandMeterPtr_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);

        remainingLoad = customMeterDemand;
        loadCenterElectricLoad = remainingLoad;

        for (auto &g : elecGenCntrlObj) {
            if (g->availSched->getCurrentVal() > 0.0 && remainingLoad > 0.0) {
                // Set the Operation Flag
                g->onThisTimestep = true;
                // Set the electric generator load
                g->powerRequestThisTimestep = min(g->maxPowerOut, remainingLoad);

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;

                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            }

            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            if (g->eMSRequestOn) {
                totalPowerRequest_ += max(g->eMSPowerRequest, 0.0);
            } else {
                if (g->powerRequestThisTimestep > 0.0) {
                    totalPowerRequest_ += g->maxPowerOut;
                    totalPowerRequest_ = min(loadCenterElectricLoad, totalPowerRequest_);
                }
            }
            remainingLoad -= g->electProdRate;             // Update remaining load to be met by this load center
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining load
        } // end for
        break;
    }
    case GeneratorOpScheme::ThermalFollow: {
        // Turn thermal load into an electrical load for cogenerators controlled to follow heat loads
        Real64 remainingThermalLoad = calcLoadCenterThermalLoad(state);
        Real64 loadCenterThermalLoad = remainingThermalLoad;
        for (auto &g : elecGenCntrlObj) {

            if (g->availSched->getCurrentVal() > 0.0 && remainingThermalLoad > 0.0) {

                if (g->nominalThermElectRatio > 0.0) {
                    remainingLoad = remainingThermalLoad / g->nominalThermElectRatio;
                    g->powerRequestThisTimestep = min(g->maxPowerOut, remainingLoad);
                    g->onThisTimestep = true;
                    // now handle EMS override
                    if (g->eMSRequestOn) {
                        g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                        if (g->powerRequestThisTimestep > 0.0) {
                            g->onThisTimestep = true;
                        } else {
                            g->onThisTimestep = false;
                        }
                    }
                }
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;
                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            }

            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            if (g->eMSRequestOn) {
                totalThermalPowerRequest_ += max(g->eMSPowerRequest, 0.0) * g->nominalThermElectRatio;
                totalPowerRequest_ += max(g->eMSPowerRequest, 0.0);
            } else {
                if (totalThermalPowerRequest_ < loadCenterThermalLoad && g->powerRequestThisTimestep > 0.0) {
                    Real64 excessThermalPowerRequest = totalThermalPowerRequest_ + g->maxPowerOut * g->nominalThermElectRatio - loadCenterThermalLoad;
                    if (excessThermalPowerRequest < 0.0) {
                        totalThermalPowerRequest_ += g->maxPowerOut * g->nominalThermElectRatio;
                        totalPowerRequest_ += g->maxPowerOut;
                    } else {
                        totalThermalPowerRequest_ = loadCenterThermalLoad;
                        if (g->nominalThermElectRatio > 0.0) {
                            totalPowerRequest_ += g->maxPowerOut - (excessThermalPowerRequest / g->nominalThermElectRatio);
                        }
                    }
                }
            }
            remainingThermalLoad -= g->thermProdRate; // Update remaining load to be met
            // by this load center
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining load
        }
        break;
    }
    case GeneratorOpScheme::ThermalFollowLimitElectrical: {
        //  Turn a thermal load into an electrical load for cogenerators controlled to follow heat loads.
        //  Add intitialization of RemainingThermalLoad as in the ThermalFollow operating scheme above.
        Real64 remainingThermalLoad = calcLoadCenterThermalLoad(state);
        // Total current electrical demand for the building is a secondary limit.
        remainingLoad = remainingWholePowerDemand;
        loadCenterElectricLoad = remainingWholePowerDemand;
        Real64 loadCenterThermalLoad = remainingThermalLoad;
        for (auto &g : elecGenCntrlObj) {
            if ((g->availSched->getCurrentVal() > 0.0) && (remainingThermalLoad > 0.0) && (remainingLoad > 0.0)) {
                if (g->nominalThermElectRatio > 0.0) {
                    remainingLoad = min(remainingWholePowerDemand, remainingThermalLoad / g->nominalThermElectRatio);
                    g->powerRequestThisTimestep = min(g->maxPowerOut, remainingLoad);
                    g->onThisTimestep = true;
                    // now handle EMS override
                    if (g->eMSRequestOn) {
                        g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                        if (g->powerRequestThisTimestep > 0.0) {
                            g->onThisTimestep = true;
                        } else {
                            g->onThisTimestep = false;
                        }
                    }
                }
            } else {
                g->onThisTimestep = false;
                g->powerRequestThisTimestep = 0.0;
                // now handle EMS override
                if (g->eMSRequestOn) {
                    g->powerRequestThisTimestep = max(g->eMSPowerRequest, 0.0);
                    if (g->powerRequestThisTimestep > 0.0) {
                        g->onThisTimestep = true;
                    } else {
                        g->onThisTimestep = false;
                    }
                }
            }
            // Get generator's actual electrical and thermal power outputs
            g->simGeneratorGetPowerOutput(
                state, g->onThisTimestep, g->powerRequestThisTimestep, firstHVACIteration, g->electProdRate, g->thermProdRate);

            if (g->eMSRequestOn) {
                totalThermalPowerRequest_ += max(g->eMSPowerRequest, 0.0) * g->nominalThermElectRatio;
                totalPowerRequest_ += (max(g->eMSPowerRequest, 0.0));
            } else {
                if (totalThermalPowerRequest_ < loadCenterThermalLoad && g->powerRequestThisTimestep > 0.0) {
                    Real64 excessThermalPowerRequest = totalThermalPowerRequest_ + g->maxPowerOut * g->nominalThermElectRatio - loadCenterThermalLoad;
                    if (excessThermalPowerRequest < 0.0) {
                        totalThermalPowerRequest_ += g->maxPowerOut * g->nominalThermElectRatio;
                        totalPowerRequest_ += g->maxPowerOut;
                    } else {
                        totalThermalPowerRequest_ = loadCenterThermalLoad;
                        if (g->nominalThermElectRatio > 0.0) {
                            totalPowerRequest_ += g->maxPowerOut - (excessThermalPowerRequest / g->nominalThermElectRatio);
                        }
                    }
                    totalPowerRequest_ = min(loadCenterElectricLoad, totalPowerRequest_);
                }
            }
            remainingThermalLoad -= g->thermProdRate; // Update remaining thermal load to
            // be met by this load center
            remainingWholePowerDemand -= g->electProdRate; // Update whole building remaining
                                                           // electric load
        }
        break;
    }
    case GeneratorOpScheme::Invalid: {
        // do nothing
        break;
    }
    default:
        assert(false);
    } // end switch

    // sum up generator production
    genElectProdRate = 0.0;
    genElectricProd = 0.0;
    for (auto const &g : elecGenCntrlObj) {
        genElectProdRate += g->electProdRate;
        g->electricityProd = g->electProdRate * (state.dataHVACGlobal->TimeStepSysSec);
        genElectricProd += g->electricityProd;
    }
}

void ElectPowerLoadCenter::dispatchStorage(EnergyPlusData &state,
                                           Real64 const originalFeedInRequest // whole building remaining electric demand for this load center
)
{

    // 1. resolve generator power rate into storage operation control volume, by buss type
    switch (bussType) {
    case ElectricBussType::Invalid:
    case ElectricBussType::ACBuss:
    case ElectricBussType::DCBussInverter: {
        // do nothing, no storage to manage
        break;
    }
    case ElectricBussType::ACBussStorage: {
        storOpCVGenRate = genElectProdRate;
        break;
    }
    case ElectricBussType::DCBussInverterDCStorage: {
        storOpCVGenRate = genElectProdRate;
        break;
    }
    case ElectricBussType::DCBussInverterACStorage: {
        // TODO call inverter model here?
        storOpCVGenRate = inverterObj->aCPowerOut();
        break;
    }
    default:
        assert(false);
    } // end switch buss type

    // 2.  determine subpanel feed in and draw requests based on storage operation control scheme
    Real64 subpanelFeedInRequest = 0.0;
    Real64 subpanelDrawRequest = 0.0;
    switch (storageScheme_) {
    case StorageOpScheme::Invalid: {
        // do nothing
        break;
    }
    case StorageOpScheme::FacilityDemandStoreExcessOnSite: {
        subpanelFeedInRequest = originalFeedInRequest; // legacy behavior, storage dispatched to meet building load
        subpanelDrawRequest = 0.0;
        break;
    }
    case StorageOpScheme::MeterDemandStoreExcessOnSite: {
        // Get meter rate
        subpanelFeedInRequest =
            GetInstantMeterValue(state, trackStorageOpMeterIndex_, OutputProcessor::TimeStepType::Zone) / state.dataGlobal->TimeStepZoneSec +
            GetInstantMeterValue(state, trackStorageOpMeterIndex_, OutputProcessor::TimeStepType::System) / (state.dataHVACGlobal->TimeStepSysSec);
        subpanelDrawRequest = 0.0;
        break;
    }
    case StorageOpScheme::ChargeDischargeSchedules: {
        // do not need to deal with subpanel rates here, charge or discharge is known from schedules and filled in below
        break;
    }
    case StorageOpScheme::FacilityDemandLeveling: {
        Real64 demandTarget = facilityDemandTarget_ * facilityDemandTargetModSched_->getCurrentVal();
        // compare target to
        Real64 deltaLoad = originalFeedInRequest - demandTarget;
        if (deltaLoad >= 0.0) {
            // subpanel should feed main panel
            subpanelFeedInRequest = deltaLoad;
            subpanelDrawRequest = 0.0;
        } else {
            // subpanel should draw from main panel
            subpanelFeedInRequest = 0.0;
            subpanelDrawRequest = std::abs(deltaLoad);
        }
        break;
    }
    default:
        assert(false);
    }

    // 3. adjust feed in and draw rates from subpanel to storage operation control volume
    Real64 adjustedFeedInRequest = 0.0; // account for any inverter or transformer losses
    Real64 adjustedDrawRequest = 0.0;   // account for any converer or transformer losses

    switch (bussType) {
    case ElectricBussType::Invalid:
    case ElectricBussType::ACBuss:
    case ElectricBussType::DCBussInverter: {
        // do nothing, no storage to manage
        break;
    }
    case ElectricBussType::ACBussStorage:
    case ElectricBussType::DCBussInverterACStorage: {
        if (transformerObj == nullptr) {
            adjustedFeedInRequest = subpanelFeedInRequest;
            adjustedDrawRequest = subpanelDrawRequest;
        } else {
            adjustedFeedInRequest = subpanelFeedInRequest + transformerObj->getLossRateForOutputPower(state, subpanelFeedInRequest);
            adjustedDrawRequest = subpanelDrawRequest - transformerObj->getLossRateForInputPower(state, subpanelDrawRequest);
        }
        break;
    }
    case ElectricBussType::DCBussInverterDCStorage: {
        // can we get updated power conditioning losses here?
        if (transformerObj == nullptr) {
            adjustedFeedInRequest = subpanelFeedInRequest + inverterObj->getLossRateForOutputPower(state, subpanelFeedInRequest);
            if (converterObj == nullptr) { // some operation schemes will never need a converter
                adjustedDrawRequest = subpanelDrawRequest;
            } else {
                adjustedDrawRequest = subpanelDrawRequest - converterObj->getLossRateForInputPower(state, subpanelDrawRequest);
            }
        } else {
            adjustedFeedInRequest = subpanelFeedInRequest + inverterObj->getLossRateForOutputPower(state, subpanelFeedInRequest) +
                                    transformerObj->getLossRateForOutputPower(state, subpanelFeedInRequest);
            if (converterObj == nullptr) {
                adjustedDrawRequest = subpanelDrawRequest - transformerObj->getLossRateForInputPower(state, subpanelDrawRequest);
            } else {
                adjustedDrawRequest = subpanelDrawRequest - converterObj->getLossRateForInputPower(state, subpanelDrawRequest) -
                                      transformerObj->getLossRateForInputPower(state, subpanelDrawRequest);
            }
        }
        break;
    }
    default:
        assert(false);
    } // end switch buss type

    switch (storageScheme_) {
    case StorageOpScheme::Invalid: {
        // do nothing
        break;
    }
    case StorageOpScheme::FacilityDemandStoreExcessOnSite: // these are both the same because adjusted feed in request has already accounted for the
                                                           // difference
    case StorageOpScheme::MeterDemandStoreExcessOnSite: {
        // this is the legacy behavior but with more limits from storage control operation information

        // no draws from main panel
        storOpCVDrawRate = 0.0;

        if (storOpCVGenRate < adjustedFeedInRequest) {
            // draw from storage
            storOpCVDischargeRate = adjustedFeedInRequest - storOpCVGenRate;
            storOpCVChargeRate = 0.0;
            storOpIsDischarging = true;
            storOpIsCharging = false;

        } else if (storOpCVGenRate > adjustedFeedInRequest) {
            // add to storage
            storOpCVDischargeRate = 0.0;
            storOpCVChargeRate = storOpCVGenRate - adjustedFeedInRequest;
            storOpIsCharging = true;
            storOpIsDischarging = false;

        } else if (storOpCVGenRate == adjustedFeedInRequest) {
            // do nothing
            storOpCVDischargeRate = 0.0;
            storOpCVChargeRate = 0.0;
            storOpIsCharging = false;
            storOpIsDischarging = false;
        }
        break;
    }

    case StorageOpScheme::ChargeDischargeSchedules: {
        storOpCVChargeRate = designStorageChargePower_ * storageChargeModSched_->getCurrentVal();
        storOpCVDischargeRate = designStorageDischargePower_ * storageDischargeModSched_->getCurrentVal();
        Real64 genAndStorSum = storOpCVGenRate + storOpCVDischargeRate - storOpCVChargeRate;
        if (genAndStorSum >= 0.0) { // power to feed toward main panel
            storOpCVDrawRate = 0.0;
            storOpCVFeedInRate = genAndStorSum;
        } else { // shortfall, will need to draw from main panel (e.g. for grid charging)
            storOpCVFeedInRate = 0.0;
            storOpCVDrawRate = std::abs(genAndStorSum);
        }
        if (storOpCVChargeRate > 0.0) {
            storOpIsCharging = true;
        } else {
            storOpIsCharging = false;
        }
        if (storOpCVDischargeRate > 0.0) {
            storOpIsDischarging = true;
        } else {
            storOpIsDischarging = false;
        }
        break;
    }
    case StorageOpScheme::FacilityDemandLeveling: {

        if (adjustedDrawRequest > 0.0) { // the only reason to draw instead of feed is to charge storage
            storOpCVFeedInRate = 0.0;
            storOpCVDrawRate = adjustedDrawRequest;
            storOpCVChargeRate = storOpCVDrawRate + storOpCVGenRate;
            storOpCVDischargeRate = 0.0;
            storOpIsCharging = true;
            storOpIsDischarging = false;
        }
        if (adjustedFeedInRequest > 0.0) {
            storOpCVDrawRate = 0.0;
            storOpCVFeedInRate = adjustedFeedInRequest;
            if (storOpCVGenRate < adjustedFeedInRequest) {
                // draw from storage
                storOpCVDischargeRate = adjustedFeedInRequest - storOpCVGenRate;
                storOpCVChargeRate = 0.0;
                storOpIsDischarging = true;
                storOpIsCharging = false;

            } else if (storOpCVGenRate > adjustedFeedInRequest) {
                // add to storage
                storOpCVDischargeRate = 0.0;
                storOpCVChargeRate = storOpCVGenRate - adjustedFeedInRequest;
                storOpIsCharging = true;
                storOpIsDischarging = false;

            } else if (storOpCVGenRate == adjustedFeedInRequest) {
                // do nothing
                storOpCVDischargeRate = 0.0;
                storOpCVChargeRate = 0.0;
                storOpIsCharging = false;
                storOpIsDischarging = false;
            }
        }
        break;
    }
    default:
        assert(true);
    }

    // handle EMS overrides
    if (eMSOverridePelFromStorage_ || eMSOverridePelIntoStorage_) {
        if (eMSOverridePelFromStorage_ && !eMSOverridePelIntoStorage_) {
            // EMS is calling for specific discharge rate
            storOpCVDischargeRate = max(eMSValuePelFromStorage_, 0.0);
            storOpCVChargeRate = 0.0;
            storOpIsDischarging = true;
            storOpIsCharging = false;
        } else if (!eMSOverridePelFromStorage_ && eMSOverridePelIntoStorage_) {
            // EMS is calling for specific charge rate
            storOpCVChargeRate = max(eMSValuePelIntoStorage_, 0.0);
            storOpCVDischargeRate = 0.0;
            storOpIsDischarging = false;
            storOpIsCharging = true;
        } else if (eMSOverridePelFromStorage_ && eMSOverridePelIntoStorage_) {
            // EMS is asking to override both
            if (eMSValuePelIntoStorage_ > eMSValuePelFromStorage_) {
                storOpCVChargeRate = eMSValuePelIntoStorage_ - eMSValuePelFromStorage_;
                storOpCVDischargeRate = 0.0;
                storOpIsDischarging = false;
                storOpIsCharging = true;
            } else if (eMSValuePelIntoStorage_ < eMSValuePelFromStorage_) {
                storOpCVDischargeRate = eMSValuePelFromStorage_ - eMSValuePelIntoStorage_;
                storOpCVChargeRate = 0.0;
                storOpIsDischarging = true;
                storOpIsCharging = false;
            } else { // they equal just hold
                storOpCVDischargeRate = 0.0;
                storOpCVChargeRate = 0.0;
                storOpIsDischarging = false;
                storOpIsCharging = false;
            }
        }
    }

    // check against the controller limits
    if (designStorageChargePowerWasSet_) {
        storOpCVChargeRate = min(storOpCVChargeRate, designStorageChargePower_);
    }
    if (designStorageDischargePowerWasSet_) {
        storOpCVDischargeRate = min(storOpCVDischargeRate, designStorageDischargePower_);
    }

    // dispatch final request to storage device, calculate, update, and report storage device, passing what controller wants for SOC limits

    storageObj->simulate(
        state, storOpCVChargeRate, storOpCVDischargeRate, storOpIsCharging, storOpIsDischarging, maxStorageSOCFraction_, minStorageSOCFraction_);

    // rebalance with final charge and discharge rates
    Real64 genAndStorSum = storOpCVGenRate + storOpCVDischargeRate - storOpCVChargeRate;
    if (genAndStorSum >= 0.0) { // power to feed toward main panel
        storOpCVDrawRate = 0.0;
        storOpCVFeedInRate = genAndStorSum;
    } else { // shortfall, will need to draw from main panel (e.g. for grid charging)
        storOpCVFeedInRate = 0.0;
        storOpCVDrawRate = std::abs(genAndStorSum);
    }
}

void ElectPowerLoadCenter::setupLoadCenterMeterIndices(EnergyPlusData &state)
{
    demandMeterPtr_ = EnergyPlus::GetMeterIndex(state, demandMeterName_);
    if ((demandMeterPtr_ == 0) && (genOperationScheme_ == GeneratorOpScheme::TrackMeter)) { // throw error
        ShowFatalError(
            state,
            format("ElectPowerLoadCenter::setupLoadCenterMeterIndices  Did not find Meter named: {} in ElectricLoadCenter:Distribution named {}",
                   demandMeterName_,
                   name_));
    }

    if (storageScheme_ == StorageOpScheme::MeterDemandStoreExcessOnSite) {
        trackStorageOpMeterIndex_ = EnergyPlus::GetMeterIndex(state, trackSorageOpMeterName_);
        if (trackStorageOpMeterIndex_ == 0) { //
            ShowFatalError(
                state,
                format("ElectPowerLoadCenter::setupLoadCenterMeterIndices  Did not find Meter named: {} in ElectricLoadCenter:Distribution named {}",
                       trackSorageOpMeterName_,
                       name_));
        }
    }
}

void ElectPowerLoadCenter::reinitAtBeginEnvironment()
{
    genElectricProd = 0.0;
    genElectProdRate = 0.0;
    thermalProd = 0.0;
    thermalProdRate = 0.0;
    totalPowerRequest_ = 0.0;
    totalThermalPowerRequest_ = 0.0;
    subpanelFeedInRate = 0.0;
    subpanelDrawRate = 0.0;
    storOpCVDrawRate = 0.0;
    storOpCVFeedInRate = 0.0;
    storOpCVChargeRate = 0.0;
    storOpCVDischargeRate = 0.0;

    if (generatorsPresent_ && numGenerators > 0) {
        for (auto &g : elecGenCntrlObj) {
            g->reinitAtBeginEnvironment();
        }
    }

    if (transformerObj != nullptr) {
        transformerObj->reinitAtBeginEnvironment();
    }

    if (storageObj != nullptr) {
        storageObj->reinitAtBeginEnvironment();
    }

    if (inverterObj != nullptr) {
        inverterObj->reinitAtBeginEnvironment();
    }

    if (converterObj != nullptr) {
        converterObj->reinitAtBeginEnvironment();
    }
}

void ElectPowerLoadCenter::reinitZoneGainsAtBeginEnvironment()
{
    if (transformerObj != nullptr) {
        transformerObj->reinitZoneGainsAtBeginEnvironment();
    }

    if (storageObj != nullptr) {
        storageObj->reinitZoneGainsAtBeginEnvironment();
    }

    if (inverterObj != nullptr) {
        inverterObj->reinitZoneGainsAtBeginEnvironment();
    }

    if (converterObj != nullptr) {
        converterObj->reinitZoneGainsAtBeginEnvironment();
    }
}

std::string const &ElectPowerLoadCenter::generatorListName() const
{
    return generatorListName_;
}

void ElectPowerLoadCenter::updateLoadCenterGeneratorRecords(EnergyPlusData &state)
{

    switch (bussType) {
    case ElectricBussType::ACBuss: {
        genElectProdRate = 0.0;
        genElectricProd = 0.0;
        for (auto const &gc : elecGenCntrlObj) {
            genElectProdRate += gc->electProdRate;
            genElectricProd += gc->electricityProd;
        }
        // no inverter, no storage, so generator production equals subpanel feed in
        subpanelFeedInRate = genElectProdRate;
        if (transformerObj != nullptr) {
            subpanelFeedInRate -= transformerObj->getLossRateForInputPower(state, genElectProdRate);
        }
        subpanelDrawRate = 0.0;

        break;
    }
    case ElectricBussType::ACBussStorage: {
        genElectProdRate = 0.0;
        genElectricProd = 0.0;
        for (auto const &gc : elecGenCntrlObj) {
            genElectProdRate += gc->electProdRate;
            genElectricProd += gc->electricityProd;
        }
        if (storageObj != nullptr) {
            subpanelFeedInRate = genElectProdRate + storOpCVDischargeRate - storOpCVChargeRate;
        } else {
            subpanelFeedInRate = genElectProdRate;
        }
        if (transformerObj != nullptr) {
            subpanelFeedInRate -= transformerObj->getLossRateForInputPower(state, subpanelFeedInRate);
        }
        subpanelDrawRate = 0.0;
        break;
    }
    case ElectricBussType::DCBussInverter: {
        genElectProdRate = 0.0;
        genElectricProd = 0.0;
        for (auto const &gc : elecGenCntrlObj) {
            genElectProdRate += gc->electProdRate;
            genElectricProd += gc->electricityProd;
        }

        if (inverterObj != nullptr) {
            subpanelFeedInRate = inverterObj->aCPowerOut();
        }
        if (transformerObj != nullptr) {
            subpanelFeedInRate -= transformerObj->getLossRateForInputPower(state, subpanelFeedInRate);
        }
        subpanelDrawRate = 0.0;
        break;
    }

    case ElectricBussType::DCBussInverterDCStorage: {
        genElectProdRate = 0.0;
        genElectricProd = 0.0;
        for (auto const &gc : elecGenCntrlObj) {
            genElectProdRate += gc->electProdRate;
            genElectricProd += gc->electricityProd;
        }
        if (inverterObj != nullptr) {
            subpanelFeedInRate = inverterObj->aCPowerOut();
        }

        if (converterObj != nullptr) {
            subpanelDrawRate = converterObj->aCPowerIn();
        }
        if (transformerObj != nullptr) {
            subpanelFeedInRate -= transformerObj->getLossRateForInputPower(state, subpanelFeedInRate);
            subpanelDrawRate += transformerObj->getLossRateForOutputPower(state, subpanelDrawRate);
        }
        break;
    }
    case ElectricBussType::DCBussInverterACStorage: {
        genElectProdRate = 0.0;
        genElectricProd = 0.0;
        for (auto const &gc : elecGenCntrlObj) {
            genElectProdRate += gc->electProdRate;
            genElectricProd += gc->electricityProd;
        }
        if (inverterObj != nullptr && storagePresent_) {
            subpanelFeedInRate = inverterObj->aCPowerOut() + storOpCVDischargeRate - storOpCVChargeRate;
        }

        subpanelDrawRate = storOpCVDrawRate; // no converter for AC storage
        if (transformerObj != nullptr) {
            subpanelFeedInRate -= transformerObj->getLossRateForInputPower(state, subpanelFeedInRate);
            subpanelDrawRate += transformerObj->getLossRateForOutputPower(state, subpanelDrawRate);
        }
        break;
    }
    case ElectricBussType::Invalid: {
        // do nothing
        break;
    }
    default:
        assert(false);
    } // end switch
    thermalProdRate = 0.0;
    thermalProd = 0.0;
    for (auto const &gc : elecGenCntrlObj) {
        thermalProdRate += gc->thermProdRate;
        thermalProd += gc->thermalProd;
    }
}

Real64 ElectPowerLoadCenter::calcLoadCenterThermalLoad(EnergyPlusData &state)
{
    if (myCoGenSetupFlag_) {
        for (auto &g : elecGenCntrlObj) {
            bool plantNotFound = false;
            PlantUtilities::ScanPlantLoopsForObject(state, g->compPlantName, g->compPlantType, g->cogenLocation, plantNotFound, _, _, _, _, _);
            if (!plantNotFound) {
                g->plantInfoFound = true;
            }
        }
        myCoGenSetupFlag_ = false;
    } // cogen setup

    // sum up "MyLoad" for all generators on this load center from plant structure
    Real64 thermalLoad = 0.0;
    for (auto &g : elecGenCntrlObj) {
        if (g->plantInfoFound) {
            thermalLoad += state.dataPlnt->PlantLoop(g->cogenLocation.loopNum)
                               .LoopSide(g->cogenLocation.loopSideNum)
                               .Branch(g->cogenLocation.branchNum)
                               .Comp(g->cogenLocation.compNum)
                               .MyLoad;
        }
    }
    return thermalLoad;
}

// TODO: Constructors should not do this much work
GeneratorController::GeneratorController(EnergyPlusData &state,
                                         std::string const &objectName,
                                         std::string const &objectType,
                                         Real64 ratedElecPowerOutput,
                                         std::string const &availSchedName,
                                         Real64 thermalToElectRatio)
    : generatorType(GeneratorType::Invalid), compPlantType(DataPlant::PlantEquipmentType::Invalid), generatorIndex(0), maxPowerOut(0.0),
      powerRequestThisTimestep(0.0), onThisTimestep(false), eMSPowerRequest(0.0), eMSRequestOn(false), plantInfoFound(false),
      cogenLocation(PlantLocation(0, DataPlant::LoopSideLocation::Invalid, 0, 0)), nominalThermElectRatio(0.0), dCElectricityProd(0.0),
      dCElectProdRate(0.0), electricityProd(0.0), electProdRate(0.0), thermalProd(0.0), thermProdRate(0.0), pvwattsGenerator(nullptr),
      errCountNegElectProd_(0)
{

    static constexpr std::string_view routineName = "GeneratorController constructor ";

    auto &s_ipsc = state.dataIPShortCut;

    ErrorObjectHeader eoh{routineName, objectType, objectName};

    name = objectName;

    generatorType = static_cast<GeneratorType>(getEnumValue(generatorTypeNamesUC, Util::makeUPPER(objectType)));
    switch (generatorType) {
    case GeneratorType::ICEngine: {
        compPlantType = DataPlant::PlantEquipmentType::Generator_ICEngine;
        compPlantName = name;
        break;
    }
    case GeneratorType::CombTurbine: {
        compPlantType = DataPlant::PlantEquipmentType::Generator_CTurbine;
        compPlantName = name;
        break;
    }
    case GeneratorType::Microturbine: {
        compPlantType = DataPlant::PlantEquipmentType::Generator_MicroTurbine;
        compPlantName = name;
        break;
    }
    case GeneratorType::PV: {
        compPlantType = DataPlant::PlantEquipmentType::PVTSolarCollectorFlatPlate;
        compPlantName = name;
        break;
    }
    case GeneratorType::PVWatts: {
        compPlantType = DataPlant::PlantEquipmentType::Invalid;

        int ObjNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Generator:PVWatts", Util::makeUPPER(objectName));
        assert(ObjNum >= 0);
        if (ObjNum == 0) {
            ShowFatalError(state, format("Cannot find Generator:PVWatts {}", objectName));
        }
        pvwattsGenerator = PVWatts::PVWattsGenerator::createFromIdfObj(state, ObjNum);
        pvwattsGenerator->setupOutputVariables(state);
        break;
    }
    case GeneratorType::FuelCell: {
        // fuel cell has two possible plant component types, stack cooler and exhaust gas HX.
        // exhaust gas HX is required and it assumed that it has more thermal capacity and is used for control
        compPlantType = DataPlant::PlantEquipmentType::Generator_FCExhaust;
        // and the name of plant component is not the same as the generator because of child object references, so fetch that name
        auto *thisFC = FuelCellElectricGenerator::FCDataStruct::factory(state, name);
        compPlantName = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFC)->ExhaustHX.Name;
        break;
    }
    case GeneratorType::MicroCHP: {
        compPlantType = DataPlant::PlantEquipmentType::Generator_MicroCHP;
        compPlantName = name;
        break;
    }
    case GeneratorType::WindTurbine: {
        compPlantType = DataPlant::PlantEquipmentType::Invalid;
        break;
    }
    default: {
        ShowSevereError(state, format("{}{} invalid entry.", routineName, s_ipsc->cCurrentModuleObject));
        ShowContinueError(state, format("Invalid {} associated with generator = {}", objectType, objectName));
        break;
    }
    }

    if (availSchedName.empty()) {
        availSched = Sched::GetScheduleAlwaysOn(state);
    } else if ((availSched = Sched::GetSchedule(state, availSchedName)) == nullptr) {
        ShowSevereItemNotFound(state, eoh, "Availability Schedule Name", availSchedName);
    } else if (generatorType == GeneratorType::PVWatts) {
        ShowWarningCustom(state, eoh, "Availability Schedule will be ignored (runs all the time).");
    } else if (generatorType == GeneratorType::PV) {
        // It should only warn if Performance type is SimplePV (DataPhotovoltaics::iSimplePVModel).
        // Except you need GetPVInput to have run already etc
        // Note: you can't use s_ipsc->cAlphaArgs etc or it'll override what will still need to be processed in
        // ElectPowerLoadCenter::ElectPowerLoadCenter after this function is called
        int PVNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, objectType, Util::makeUPPER(objectName));
        int NumAlphas; // Number of PV Array parameter alpha names being passed
        int NumNums;   // Number of PV Array numeric parameters are being passed
        int IOStat;
        Array1D_string Alphas(5);   // Alpha items for object
        Array1D<Real64> Numbers(2); // Numeric items for object
        state.dataInputProcessing->inputProcessor->getObjectItem(state, objectType, PVNum, Alphas, NumAlphas, Numbers, NumNums, IOStat);
        if (Util::SameString(Alphas(3), "PhotovoltaicPerformance:Simple")) {
            ShowWarningCustom(state, eoh, "Availability Schedule will be ignored (runs all the time).");
            ShowContinueError(state, "To limit this Generator:Photovoltaic's output, please use the Inverter's availability schedule instead.");
        }
    }

    maxPowerOut = ratedElecPowerOutput, nominalThermElectRatio = thermalToElectRatio;

    SetupOutputVariable(state,
                        "Generator Requested Electricity Rate",
                        Constant::Units::W,
                        powerRequestThisTimestep,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        objectName);
    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSInternalVariable(state, "Generator Nominal Maximum Power", objectName, "[W]", maxPowerOut);
        SetupEMSInternalVariable(state, "Generator Nominal Thermal To Electric Ratio", objectName, "[ratio]", nominalThermElectRatio);
        SetupEMSActuator(state, "On-Site Generator Control", objectName, "Requested Power", "[W]", eMSRequestOn, eMSPowerRequest);
    }
}

void GeneratorController::reinitAtBeginEnvironment()
{
    onThisTimestep = false;
    dCElectricityProd = 0.0;
    dCElectProdRate = 0.0;
    electricityProd = 0.0;
    electProdRate = 0.0;
    thermalProd = 0.0;
    thermProdRate = 0.0;
}

void GeneratorController::simGeneratorGetPowerOutput(EnergyPlusData &state,
                                                     bool const runFlag,
                                                     Real64 const myElecLoadRequest,
                                                     bool const FirstHVACIteration, // Unused 2010 JANUARY
                                                     Real64 &electricPowerOutput,   // Actual generator electric power output
                                                     Real64 &thermalPowerOutput     // Actual generator thermal power output
)
{
    // Select and call models and also collect results for load center power conditioning and reporting
    switch (generatorType) {
    case GeneratorType::ICEngine: {

        auto *thisICE = ICEngineElectricGenerator::ICEngineGeneratorSpecs::factory(state, name);

        // dummy vars
        PlantLocation L(0, DataPlant::LoopSideLocation::Invalid, 0, 0);
        Real64 tempLoad = myElecLoadRequest;

        // simulate
        dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->InitICEngineGenerators(state, runFlag, FirstHVACIteration);
        dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->CalcICEngineGeneratorModel(state, runFlag, tempLoad);
        dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->update(state);
        electProdRate = dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->ElecPowerGenerated;
        electricityProd = dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->ElecEnergyGenerated;
        thermProdRate = dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->QTotalHeatRecovered;
        thermalProd = dynamic_cast<ICEngineElectricGenerator::ICEngineGeneratorSpecs *>(thisICE)->TotalHeatEnergyRec;
        electricPowerOutput = electProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::CombTurbine: {

        auto *thisCTE = CTElectricGenerator::CTGeneratorData::factory(state, name);
        // dummy vars
        PlantLocation L(0, DataPlant::LoopSideLocation::Invalid, 0, 0);
        Real64 tempLoad = myElecLoadRequest;

        // simulate
        thisCTE->simulate(state, L, FirstHVACIteration, tempLoad, runFlag);
        dynamic_cast<CTElectricGenerator::CTGeneratorData *>(thisCTE)->InitCTGenerators(state, runFlag, FirstHVACIteration);
        dynamic_cast<CTElectricGenerator::CTGeneratorData *>(thisCTE)->CalcCTGeneratorModel(state, runFlag, tempLoad, FirstHVACIteration);
        electProdRate = dynamic_cast<CTElectricGenerator::CTGeneratorData *>(thisCTE)->ElecPowerGenerated;
        electricityProd = dynamic_cast<CTElectricGenerator::CTGeneratorData *>(thisCTE)->ElecEnergyGenerated;
        thermProdRate = dynamic_cast<CTElectricGenerator::CTGeneratorData *>(thisCTE)->QTotalHeatRecovered;
        thermalProd = dynamic_cast<CTElectricGenerator::CTGeneratorData *>(thisCTE)->TotalHeatEnergyRec;
        electricPowerOutput = electProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::PV: {
        Photovoltaics::SimPVGenerator(state, GeneratorType::PV, name, generatorIndex, runFlag, myElecLoadRequest);
        Photovoltaics::GetPVGeneratorResults(
            state, GeneratorType::PV, generatorIndex, dCElectProdRate, dCElectricityProd, thermProdRate, thermalProd);
        electricPowerOutput = dCElectProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::PVWatts: {
        pvwattsGenerator->calc(state);
        pvwattsGenerator->getResults(dCElectProdRate, dCElectricityProd, thermProdRate, thermalProd);
        electricPowerOutput = dCElectProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::FuelCell: {
        auto *thisFC = FuelCellElectricGenerator::FCDataStruct::factory(state, name);
        dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFC)->SimFuelCellGenerator(state, runFlag, myElecLoadRequest, FirstHVACIteration);
        electProdRate = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFC)->Report.ACPowerGen;
        electricityProd = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFC)->Report.ACEnergyGen;
        thermProdRate = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFC)->Report.qHX;
        thermalProd = dynamic_cast<FuelCellElectricGenerator::FCDataStruct *>(thisFC)->Report.HXenergy;
        electricPowerOutput = electProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::MicroCHP: {
        auto *thisMCHP = MicroCHPElectricGenerator::MicroCHPDataStruct::factory(state, name);

        // simulate
        dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->InitMicroCHPNoNormalizeGenerators(state);

        if (!state.dataPlnt->PlantFirstSizeCompleted) {
            break;
        }

        dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->CalcMicroCHPNoNormalizeGeneratorModel(
            state, runFlag, false, myElecLoadRequest, DataPrecisionGlobals::constant_zero);
        dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->CalcUpdateHeatRecovery(state);
        dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->UpdateMicroCHPGeneratorRecords(state);

        electProdRate = dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->A42Model.ACPowerGen;
        electricityProd = dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->A42Model.ACEnergyGen;
        thermProdRate = dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->A42Model.QdotHR;
        thermalProd = dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(thisMCHP)->A42Model.TotalHeatEnergyRec;
        electricPowerOutput = electProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::Microturbine: {
        auto *thisMTG = MicroturbineElectricGenerator::MTGeneratorSpecs::factory(state, name);

        // dummy vars
        PlantLocation L(0, DataPlant::LoopSideLocation::Invalid, 0, 0);
        Real64 tempLoad = myElecLoadRequest;

        // simulate
        dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->InitMTGenerators(state, runFlag, tempLoad, FirstHVACIteration);
        dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->CalcMTGeneratorModel(state, runFlag, tempLoad);
        dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->UpdateMTGeneratorRecords(state);
        electProdRate = dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->ElecPowerGenerated;
        electricityProd = dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->EnergyGen;
        thermProdRate = dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->QHeatRecovered;
        thermalProd = dynamic_cast<MicroturbineElectricGenerator::MTGeneratorSpecs *>(thisMTG)->ExhaustEnergyRec;
        electricPowerOutput = electProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::WindTurbine: {
        WindTurbine::SimWindTurbine(state, GeneratorType::WindTurbine, name, generatorIndex, runFlag, myElecLoadRequest);
        WindTurbine::GetWTGeneratorResults(
            state, GeneratorType::WindTurbine, generatorIndex, electProdRate, electricityProd, thermProdRate, thermalProd);
        electricPowerOutput = electProdRate;
        thermalPowerOutput = thermProdRate;
        break;
    }
    case GeneratorType::Invalid:
    case GeneratorType::Num: {
        // do nothing
        break;
    }
    } // end switch

    // check if generator production has gone wrong and is negative, reset to zero and warn
    if (electricPowerOutput < 0.0) {
        if (errCountNegElectProd_ == 0) {
            ShowWarningMessage(
                state,
                format("{} named {} is producing negative electric power, check generator inputs.", generatorTypeNames[(int)generatorType], name));
            ShowContinueError(state, format("Electric power production rate ={:.4R}", electricPowerOutput));
            ShowContinueError(state, "The power will be set to zero, and the simulation continues... ");
        }
        ShowRecurringWarningErrorAtEnd(state,
                                       format("{} named {} is producing negative electric power ", generatorTypeNames[(int)generatorType], name),
                                       errCountNegElectProd_,
                                       electricPowerOutput,
                                       electricPowerOutput);
        electricPowerOutput = 0.0;
    }
}

DCtoACInverter::DCtoACInverter(EnergyPlusData &state, std::string const &objectName)
    : aCPowerOut_(0.0), aCEnergyOut_(0.0), efficiency_(0.0), dCPowerIn_(0.0), dCEnergyIn_(0.0), conversionLossPower_(0.0), conversionLossEnergy_(0.0),
      conversionLossEnergyDecrement_(0.0), thermLossRate_(0.0), thermLossEnergy_(0.0), qdotConvZone_(0.0), qdotRadZone_(0.0), ancillACuseRate_(0.0),
      ancillACuseEnergy_(0.0), modelType_(InverterModelType::Invalid), heatLossesDestination_(ThermalLossDestination::Invalid), zoneNum_(0),
      zoneRadFract_(0.0), nominalVoltage_(0.0), nomVoltEfficiencyARR_(6, 0.0), ratedPower_(0.0), minPower_(0.0), maxPower_(0.0), minEfficiency_(0.0),
      maxEfficiency_(0.0), standbyPower_(0.0)
{
    // initialize
    nomVoltEfficiencyARR_.resize(6, 0.0);

    static constexpr std::string_view routineName = "DCtoACInverter constructor ";

    auto &s_ipsc = state.dataIPShortCut;

    bool errorsFound = false;
    // if/when add object class name to input object this can be simplified. for now search all possible types
    bool foundInverter = false;
    int testInvertIndex = 0;
    int invertIDFObjectNum = 0;

    testInvertIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Inverter:LookUpTable", objectName);
    if (testInvertIndex > 0) {
        foundInverter = true;
        invertIDFObjectNum = testInvertIndex;
        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Inverter:LookUpTable";
        modelType_ = InverterModelType::CECLookUpTableModel;
    }
    testInvertIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Inverter:FunctionOfPower", objectName);
    if (testInvertIndex > 0) {
        foundInverter = true;
        invertIDFObjectNum = testInvertIndex;
        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Inverter:FunctionOfPower";
        modelType_ = InverterModelType::CurveFuncOfPower;
    }
    testInvertIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Inverter:Simple", objectName);
    if (testInvertIndex > 0) {
        foundInverter = true;
        invertIDFObjectNum = testInvertIndex;
        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Inverter:Simple";
        modelType_ = InverterModelType::SimpleConstantEff;
    }
    testInvertIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Inverter:PVWatts", objectName);
    if (testInvertIndex > 0) {
        foundInverter = true;
        invertIDFObjectNum = testInvertIndex;
        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Inverter:PVWatts";
        modelType_ = InverterModelType::PVWatts;
    }

    if (foundInverter) {
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 invertIDFObjectNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
        name_ = s_ipsc->cAlphaArgs(1);
        // how to verify names are unique across objects? add to GlobalNames?

        if (modelType_ == InverterModelType::PVWatts) {
            availSched_ = Sched::GetScheduleAlwaysOn(state);
            zoneNum_ = 0;
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
            zoneRadFract_ = 0;
        } else if (s_ipsc->lAlphaFieldBlanks(2)) {
            availSched_ = Sched::GetScheduleAlwaysOn(state);
        } else {
            if ((availSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                errorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                heatLossesDestination_ = ThermalLossDestination::LostToOutside;
            } else if ((zoneNum_ = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->Zone)) == 0) {
                heatLossesDestination_ = ThermalLossDestination::LostToOutside;
                ShowWarningItemNotFound(
                    state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), "Inverter heat losses will not be added to a zone");
                // continue with simulation but inverter losses not sent to a zone.
            } else {
                heatLossesDestination_ = ThermalLossDestination::ZoneGains;
            }
            zoneRadFract_ = s_ipsc->rNumericArgs(1);
        }

        // now the input objects differ depending on class type
        switch (modelType_) {
        case InverterModelType::CECLookUpTableModel: {
            ratedPower_ = s_ipsc->rNumericArgs(2);
            standbyPower_ = s_ipsc->rNumericArgs(3);

            nominalVoltage_ = s_ipsc->rNumericArgs(4);
            nomVoltEfficiencyARR_[0] = s_ipsc->rNumericArgs(5);
            nomVoltEfficiencyARR_[1] = s_ipsc->rNumericArgs(6);
            nomVoltEfficiencyARR_[2] = s_ipsc->rNumericArgs(7);
            nomVoltEfficiencyARR_[3] = s_ipsc->rNumericArgs(8);
            nomVoltEfficiencyARR_[4] = s_ipsc->rNumericArgs(9);
            nomVoltEfficiencyARR_[5] = s_ipsc->rNumericArgs(10);
            break;
        }
        case InverterModelType::CurveFuncOfPower: {
            if (s_ipsc->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(4));
                errorsFound = true;
            } else if ((effCurve_ = Curve::GetCurve(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                errorsFound = true;
            }

            ratedPower_ = s_ipsc->rNumericArgs(2);
            minEfficiency_ = s_ipsc->rNumericArgs(3);
            maxEfficiency_ = s_ipsc->rNumericArgs(4);
            minPower_ = s_ipsc->rNumericArgs(5);
            maxPower_ = s_ipsc->rNumericArgs(6);
            standbyPower_ = s_ipsc->rNumericArgs(7);
            break;
        }
        case InverterModelType::SimpleConstantEff: {
            efficiency_ = s_ipsc->rNumericArgs(2);
            break;
        }
        case InverterModelType::Invalid: {
            // do nothing
            break;
        }
        case InverterModelType::PVWatts: {
            pvWattsDCtoACSizeRatio_ = s_ipsc->rNumericArgs(1);
            pvWattsInverterEfficiency_ = s_ipsc->rNumericArgs(2);
            break;
        }
        default:
            assert(false);
        } // end switch modelType

        SetupOutputVariable(state,
                            "Inverter DC to AC Efficiency",
                            Constant::Units::None,
                            efficiency_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Inverter DC Input Electricity Rate",
                            Constant::Units::W,
                            dCPowerIn_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Inverter DC Input Electricity Energy",
                            Constant::Units::J,
                            dCEnergyIn_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Inverter AC Output Electricity Rate",
                            Constant::Units::W,
                            aCPowerOut_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Inverter AC Output Electricity Energy",
                            Constant::Units::J,
                            aCEnergyOut_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Inverter Conversion Loss Power",
                            Constant::Units::W,
                            conversionLossPower_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Inverter Conversion Loss Energy",
                            Constant::Units::J,
                            conversionLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Inverter Conversion Loss Decrement Energy",
                            Constant::Units::J,
                            conversionLossEnergyDecrement_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityProduced,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::PowerConversion);
        SetupOutputVariable(state,
                            "Inverter Thermal Loss Rate",
                            Constant::Units::W,
                            thermLossRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Inverter Thermal Loss Energy",
                            Constant::Units::J,
                            thermLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Inverter Ancillary AC Electricity Rate",
                            Constant::Units::W,
                            ancillACuseRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Inverter Ancillary AC Electricity Energy",
                            Constant::Units::J,
                            ancillACuseEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::Plant, // called cogeneration for end use table
                            OutputProcessor::EndUseCat::Cogeneration,
                            "DCtoACInverter Ancillary");
        if (zoneNum_ > 0) {
            switch (modelType_) {
            case InverterModelType::SimpleConstantEff: {
                SetupZoneInternalGain(
                    state, zoneNum_, name_, DataHeatBalance::IntGainType::ElectricLoadCenterInverterSimple, &qdotConvZone_, nullptr, &qdotRadZone_);
                break;
            }
            case InverterModelType::CurveFuncOfPower: {
                SetupZoneInternalGain(state,
                                      zoneNum_,
                                      name_,
                                      DataHeatBalance::IntGainType::ElectricLoadCenterInverterFunctionOfPower,
                                      &qdotConvZone_,
                                      nullptr,
                                      &qdotRadZone_);
                break;
            }
            case InverterModelType::CECLookUpTableModel: {
                SetupZoneInternalGain(state,
                                      zoneNum_,
                                      name_,
                                      DataHeatBalance::IntGainType::ElectricLoadCenterInverterLookUpTable,
                                      &qdotConvZone_,
                                      nullptr,
                                      &qdotRadZone_);
                break;
            }
            case InverterModelType::Invalid: {
                // do nothing
                break;
            }
            case InverterModelType::PVWatts: {
                break;
            }
            default:
                assert(false);
            } // end switch modelType
        }
    } else {
        ShowSevereError(state, format("{} did not find inverter name = {}", routineName, objectName));
        errorsFound = true;
    }

    if (errorsFound) {
        ShowFatalError(state, format("{}Preceding errors terminate program.", routineName));
    }
}

void DCtoACInverter::reinitAtBeginEnvironment()
{
    ancillACuseRate_ = 0.0;
    ancillACuseEnergy_ = 0.0;
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

void DCtoACInverter::reinitZoneGainsAtBeginEnvironment()
{
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

void DCtoACInverter::setPVWattsDCCapacity(EnergyPlusData &state, const Real64 dcCapacity)
{
    if (modelType_ != InverterModelType::PVWatts) {
        ShowFatalError(state, "Setting the DC Capacity for the inverter only works with PVWatts Inverters.");
    }
    ratedPower_ = dcCapacity / pvWattsDCtoACSizeRatio_;
}

Real64 DCtoACInverter::pvWattsDCCapacity()
{
    return ratedPower_ * pvWattsDCtoACSizeRatio_;
}

Real64 DCtoACInverter::pvWattsInverterEfficiency()
{
    return pvWattsInverterEfficiency_;
}

Real64 DCtoACInverter::pvWattsDCtoACSizeRatio()
{
    return pvWattsDCtoACSizeRatio_;
}

Real64 DCtoACInverter::aCPowerOut() const
{
    return aCPowerOut_;
}

DCtoACInverter::InverterModelType DCtoACInverter::modelType() const
{
    return modelType_;
}

std::string const &DCtoACInverter::name() const
{
    return name_;
}

Real64 DCtoACInverter::getLossRateForOutputPower(EnergyPlusData &state, Real64 const powerOutOfInverter)
{

    // need to invert, find a dCPowerIn that produces the desired AC power out
    // use last efficiency for initial guess
    if (efficiency_ > 0.0) {
        dCPowerIn_ = powerOutOfInverter / efficiency_;
    } else {
        dCPowerIn_ = powerOutOfInverter;
        calcEfficiency(state);
        dCPowerIn_ = powerOutOfInverter / efficiency_;
    }

    calcEfficiency(state);
    // one more update is close enough.
    if (efficiency_ > 0.0) {
        dCPowerIn_ = powerOutOfInverter / efficiency_;
    }
    calcEfficiency(state);
    return (1.0 - efficiency_) * dCPowerIn_;
}

void DCtoACInverter::calcEfficiency(EnergyPlusData &state)
{
    switch (modelType_) {
    case InverterModelType::CECLookUpTableModel: {
        // we don't model voltage, so use nominal voltage
        Real64 normalizedPower = dCPowerIn_ / ratedPower_;

        // get efficiency
        if (normalizedPower <= 0.1) {
            // extrapolate or fix at 10% value? fix it for now
            efficiency_ = nomVoltEfficiencyARR_[0];
        } else if ((normalizedPower > 0.1) && (normalizedPower < 0.20)) {
            efficiency_ = nomVoltEfficiencyARR_[0] + ((normalizedPower - 0.1) / (0.2 - 0.1)) * (nomVoltEfficiencyARR_[1] - nomVoltEfficiencyARR_[0]);
        } else if (normalizedPower == 0.2) {
            efficiency_ = nomVoltEfficiencyARR_[1];
        } else if ((normalizedPower > 0.2) && (normalizedPower < 0.30)) {
            efficiency_ = nomVoltEfficiencyARR_[1] + ((normalizedPower - 0.2) / (0.3 - 0.2)) * (nomVoltEfficiencyARR_[2] - nomVoltEfficiencyARR_[1]);
        } else if (normalizedPower == 0.3) {
            efficiency_ = nomVoltEfficiencyARR_[2];
        } else if ((normalizedPower > 0.3) && (normalizedPower < 0.50)) {
            efficiency_ = nomVoltEfficiencyARR_[2] + ((normalizedPower - 0.3) / (0.5 - 0.3)) * (nomVoltEfficiencyARR_[3] - nomVoltEfficiencyARR_[2]);
        } else if (normalizedPower == 0.5) {
            efficiency_ = nomVoltEfficiencyARR_[3];
        } else if ((normalizedPower > 0.5) && (normalizedPower < 0.75)) {
            efficiency_ = nomVoltEfficiencyARR_[3] + ((normalizedPower - 0.5) / (0.75 - 0.5)) * (nomVoltEfficiencyARR_[4] - nomVoltEfficiencyARR_[3]);
        } else if (normalizedPower == 0.75) {
            efficiency_ = nomVoltEfficiencyARR_[4];
        } else if ((normalizedPower > 0.75) && (normalizedPower < 1.0)) {
            efficiency_ =
                nomVoltEfficiencyARR_[4] + ((normalizedPower - 0.75) / (1.0 - 0.75)) * (nomVoltEfficiencyARR_[5] - nomVoltEfficiencyARR_[4]);
        } else if (normalizedPower >= 1.0) {
            efficiency_ = nomVoltEfficiencyARR_[5];
        } else {
            assert(false);
        }

        efficiency_ = max(efficiency_, 0.0);
        efficiency_ = min(efficiency_, 1.0);
    } break;

    case InverterModelType::CurveFuncOfPower: {
        Real64 normalizedPower = dCPowerIn_ / ratedPower_;
        efficiency_ = effCurve_->value(state, normalizedPower);
        efficiency_ = max(efficiency_, minEfficiency_);
        efficiency_ = min(efficiency_, maxEfficiency_);
    } break;

    case InverterModelType::PVWatts: {
        // This code is lifted from ssc cmod_pvwatts5.cpp:powerout() method.
        // It was easier to do this calculation here because we have a many to one relationship between inverter
        // and generator whereas theirs is one to one.
        Real64 constexpr etaref = 0.9637;
        Real64 constexpr A = -0.0162;
        Real64 constexpr B = -0.0059;
        Real64 constexpr C = 0.9858;
        Real64 const pdc0 = ratedPower_ / pvWattsInverterEfficiency_;
        Real64 const plr = dCPowerIn_ / pdc0;
        Real64 ac = 0;

        if (plr > 0) {
            // normal operation
            Real64 eta = (A * plr + B / plr + C) * pvWattsInverterEfficiency_ / etaref;
            ac = dCPowerIn_ * eta;
            if (ac > ratedPower_) {
                // clipping
                ac = ratedPower_;
            }
            // make sure no negative AC values (no parasitic nighttime losses calculated)
            if (ac < 0) {
                ac = 0;
            }
            efficiency_ = ac / dCPowerIn_;
        } else {
            efficiency_ = 1.0; // Set to a non-zero reasonable value (to avoid divide by zero error)
        }
        break;
    }
    case InverterModelType::SimpleConstantEff:
    case InverterModelType::Invalid: {
        // do nothing
        break;
    }
    default:
        assert(false);
    } // end switch
}

void DCtoACInverter::simulate(EnergyPlusData &state, Real64 const powerIntoInverter)
{
    dCPowerIn_ = powerIntoInverter;
    dCEnergyIn_ = dCPowerIn_ * (state.dataHVACGlobal->TimeStepSysSec);
    // check availability schedule
    if (availSched_->getCurrentVal() > 0.0) {

        // now calculate Inverter based on model type
        calcEfficiency(state);
        aCPowerOut_ = efficiency_ * dCPowerIn_;
        aCEnergyOut_ = aCPowerOut_ * (state.dataHVACGlobal->TimeStepSysSec);

        if (aCPowerOut_ == 0.0) {
            ancillACuseEnergy_ = standbyPower_ * (state.dataHVACGlobal->TimeStepSysSec);
            ancillACuseRate_ = standbyPower_;
        } else {
            ancillACuseRate_ = 0.0;
            ancillACuseEnergy_ = 0.0;
        }
    } else { // not available per schedule, inverter is dead.
        //  assume thermal shunt for DC in, but no standby electricity
        aCPowerOut_ = 0.0;
        aCEnergyOut_ = 0.0;
        ancillACuseRate_ = 0.0;
        ancillACuseEnergy_ = 0.0;
    }
    // update report variables
    conversionLossPower_ = dCPowerIn_ - aCPowerOut_;
    conversionLossEnergy_ = conversionLossPower_ * (state.dataHVACGlobal->TimeStepSysSec);
    conversionLossEnergyDecrement_ = -1.0 * conversionLossEnergy_;
    thermLossRate_ = dCPowerIn_ - aCPowerOut_ + ancillACuseRate_;
    thermLossEnergy_ = thermLossRate_ * (state.dataHVACGlobal->TimeStepSysSec);
    qdotConvZone_ = thermLossRate_ * (1.0 - zoneRadFract_);
    qdotRadZone_ = thermLossRate_ * zoneRadFract_;
}

ACtoDCConverter::ACtoDCConverter(EnergyPlusData &state, std::string const &objectName)
    : efficiency_(0.0), aCPowerIn_(0.0), aCEnergyIn_(0.0), dCPowerOut_(0.0), dCEnergyOut_(0.0), conversionLossPower_(0.0), conversionLossEnergy_(0.0),
      conversionLossEnergyDecrement_(0.0), thermLossRate_(0.0), thermLossEnergy_(0.0), qdotConvZone_(0.0), qdotRadZone_(0.0), ancillACuseRate_(0.0),
      ancillACuseEnergy_(0.0), modelType_(ConverterModelType::Invalid), heatLossesDestination_(ThermalLossDestination::Invalid), zoneNum_(0),
      zoneRadFract_(0.0), // radiative fraction for thermal losses to zone
      standbyPower_(0.0), maxPower_(0.0)
{

    static constexpr std::string_view routineName = "ACtoDCConverter constructor ";

    auto &s_ipsc = state.dataIPShortCut;

    bool errorsFound = false;
    // if/when add object class name to input object this can be simplified. for now search all possible types

    int testConvertIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Storage:Converter", objectName);

    if (testConvertIndex > 0) {
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Storage:Converter";

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 testConvertIndex,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        name_ = s_ipsc->cAlphaArgs(1);
        // need a new general approach for verify names are unique across objects,  next gen GlobalNames

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            availSched_ = Sched::GetScheduleAlwaysOn(state);
        } else if ((availSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            errorsFound = true;
        }

        modelType_ = static_cast<ConverterModelType>(getEnumValue(converterModelTypeNamesUC, s_ipsc->cAlphaArgs(3)));
        if (modelType_ == ConverterModelType::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
            errorsFound = true;
        }

        switch (modelType_) {
        case ConverterModelType::SimpleConstantEff: {
            efficiency_ = s_ipsc->rNumericArgs(1);
        } break;

        case ConverterModelType::CurveFuncOfPower: {
            maxPower_ = s_ipsc->rNumericArgs(2);

            if (s_ipsc->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(4));
                errorsFound = true;
            } else if ((effCurve_ = Curve::GetCurve(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                errorsFound = true;
            }
        } break;

        case ConverterModelType::Invalid: {
            // do nothing
        } break;

        default:
            assert(false);
        } // end switch

        standbyPower_ = s_ipsc->rNumericArgs(3);

        if (s_ipsc->lAlphaFieldBlanks(5)) {
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
        } else if ((zoneNum_ = Util::FindItemInList(s_ipsc->cAlphaArgs(5), state.dataHeatBal->Zone)) == 0) {
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
            ShowWarningItemNotFound(
                state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), "Inverter heat losses will not be added to a zone");
            // continue with simulation but inverter losses not sent to a zone.
        } else {
            heatLossesDestination_ = ThermalLossDestination::ZoneGains;
        }

        zoneRadFract_ = s_ipsc->rNumericArgs(4);

        SetupOutputVariable(state,
                            "Converter AC to DC Efficiency",
                            Constant::Units::None,
                            efficiency_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Converter AC Input Electricity Rate",
                            Constant::Units::W,
                            aCPowerIn_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Converter AC Input Electricity Energy",
                            Constant::Units::J,
                            aCEnergyIn_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Converter DC Output Electricity Rate",
                            Constant::Units::W,
                            dCPowerOut_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Converter DC Output Electricity Energy",
                            Constant::Units::J,
                            dCEnergyOut_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Converter Electricity Loss Rate",
                            Constant::Units::W,
                            conversionLossPower_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Converter Electricity Loss Energy",
                            Constant::Units::J,
                            conversionLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Converter Electricity Loss Decrement Energy",
                            Constant::Units::J,
                            conversionLossEnergyDecrement_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityProduced,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::PowerConversion);
        SetupOutputVariable(state,
                            "Converter Thermal Loss Rate",
                            Constant::Units::W,
                            thermLossRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Converter Thermal Loss Energy",
                            Constant::Units::J,
                            thermLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Converter Ancillary AC Electricity Rate",
                            Constant::Units::W,
                            ancillACuseRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Converter Ancillary AC Electricity Energy",
                            Constant::Units::J,
                            ancillACuseEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::Plant, // called cogeneration for end use table
                            OutputProcessor::EndUseCat::Cogeneration,
                            "ACtoDCConverter Ancillary");
        if (zoneNum_ > 0) {
            SetupZoneInternalGain(
                state, zoneNum_, name_, DataHeatBalance::IntGainType::ElectricLoadCenterConverter, &qdotConvZone_, nullptr, &qdotRadZone_);
        }
    } else {
        ShowSevereError(state, format("{} did not find power converter name = {}", routineName, objectName));
        errorsFound = true;
    }

    if (errorsFound) {
        ShowFatalError(state, format("{}Preceding errors terminate program.", routineName));
    }
}

void ACtoDCConverter::reinitAtBeginEnvironment()
{
    ancillACuseRate_ = 0.0;
    ancillACuseEnergy_ = 0.0;
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

void ACtoDCConverter::reinitZoneGainsAtBeginEnvironment()
{
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

Real64 ACtoDCConverter::aCPowerIn() const
{
    return aCPowerIn_;
}

Real64 ACtoDCConverter::getLossRateForInputPower(EnergyPlusData &state, Real64 const powerIntoConverter)
{
    aCPowerIn_ = powerIntoConverter;
    calcEfficiency(state);
    return (1.0 - efficiency_) * aCPowerIn_;
}

std::string const &ACtoDCConverter::name() const
{
    return name_;
}

void ACtoDCConverter::calcEfficiency(EnergyPlusData &state)
{
    switch (modelType_) {
    case ConverterModelType::Invalid:
    case ConverterModelType::SimpleConstantEff: {
        break;
    }
    case ConverterModelType::CurveFuncOfPower: {
        Real64 normalizedPower = aCPowerIn_ / maxPower_;
        efficiency_ = effCurve_->value(state, normalizedPower);
        break;
    }
    default:
        assert(false);
    } // end switch
}

void ACtoDCConverter::simulate(EnergyPlusData &state, Real64 const powerOutFromConverter)
{
    // need to invert, find an aCPowerIn that produces the desired DC power out

    // use last efficiency for initial guess
    if (availSched_->getCurrentVal() > 0.0) {

        aCPowerIn_ = powerOutFromConverter / efficiency_;
        calcEfficiency(state), aCPowerIn_ = powerOutFromConverter / efficiency_;
        calcEfficiency(state),

            dCPowerOut_ = aCPowerIn_ * efficiency_;

        if (dCPowerOut_ == 0.0) {
            ancillACuseEnergy_ = standbyPower_ * (state.dataHVACGlobal->TimeStepSysSec);
            ancillACuseRate_ = standbyPower_;
        } else {
            ancillACuseRate_ = 0.0;
            ancillACuseEnergy_ = 0.0;
        }

    } else { // not available
        aCPowerIn_ = 0.0;
        dCPowerOut_ = 0.0;
        ancillACuseRate_ = 0.0;
        ancillACuseEnergy_ = 0.0;
    }

    // update and report
    aCEnergyIn_ = aCPowerIn_ * (state.dataHVACGlobal->TimeStepSysSec);
    dCEnergyOut_ = dCPowerOut_ * (state.dataHVACGlobal->TimeStepSysSec);
    conversionLossPower_ = aCPowerIn_ - dCPowerOut_;
    conversionLossEnergy_ = conversionLossPower_ * (state.dataHVACGlobal->TimeStepSysSec);
    conversionLossEnergyDecrement_ = -1.0 * conversionLossEnergy_;
    thermLossRate_ = aCPowerIn_ - dCPowerOut_ + ancillACuseRate_;
    thermLossEnergy_ = thermLossRate_ * (state.dataHVACGlobal->TimeStepSysSec);
    qdotConvZone_ = thermLossRate_ * (1.0 - zoneRadFract_);
    qdotRadZone_ = thermLossRate_ * zoneRadFract_;
}

ElectricStorage::ElectricStorage( // main constructor
    EnergyPlusData &state,
    std::string const &objectName)
    : storedPower_(0.0), storedEnergy_(0.0), drawnPower_(0.0), drawnEnergy_(0.0), decrementedEnergyStored_(0.0), maxRainflowArrayBounds_(100),
      myWarmUpFlag_(false), storageModelMode_(StorageModelType::Invalid), heatLossesDestination_(ThermalLossDestination::Invalid), zoneNum_(0),
      zoneRadFract_(0.0), startingEnergyStored_(0.0), energeticEfficCharge_(0.0), energeticEfficDischarge_(0.0), maxPowerDraw_(0.0),
      maxPowerStore_(0.0), maxEnergyCapacity_(0.0), parallelNum_(0), seriesNum_(0), numBattery_(0), cycleBinNum_(0), startingSOC_(0.0),
      maxAhCapacity_(0.0), availableFrac_(0.0), chargeConversionRate_(0.0), chargedOCV_(0.0), dischargedOCV_(0.0), internalR_(0.0),
      maxDischargeI_(0.0), cutoffV_(0.0), maxChargeRate_(0.0), lifeCalculation_(false), liIon_dcToDcChargingEff_(0.0), liIon_mass_(0.0),
      liIon_surfaceArea_(0.0), liIon_Cp_(0.0), liIon_heatTransferCoef_(0.0), liIon_Vfull_(0.0), liIon_Vexp_(0.0), liIon_Vnom_(0.0),
      liIon_Vnom_default_(0.0), liIon_Qfull_(0.0), liIon_Qexp_(0.0), liIon_Qnom_(0.0), liIon_C_rate_(0.0), thisTimeStepStateOfCharge_(0.0),
      lastTimeStepStateOfCharge_(0.0), pelNeedFromStorage_(0.0), pelFromStorage_(0.0), pelIntoStorage_(0.0), qdotConvZone_(0.0), qdotRadZone_(0.0),
      timeElapsed_(0.0), thisTimeStepAvailable_(0.0), thisTimeStepBound_(0.0), lastTimeStepAvailable_(0.0), lastTimeStepBound_(0.0),
      lastTwoTimeStepAvailable_(0.0), lastTwoTimeStepBound_(0.0), count0_(0), electEnergyinStorage_(0.0), thermLossRate_(0.0), thermLossEnergy_(0.0),
      storageMode_(0), absoluteSOC_(0.0), fractionSOC_(0.0), batteryCurrent_(0.0), batteryVoltage_(0.0), batteryDamage_(0.0), batteryTemperature_(0.0)
{

    static constexpr std::string_view routineName = "ElectricStorage constructor ";

    auto &s_ipsc = state.dataIPShortCut;

    bool errorsFound = false;
    // if/when add object class name to input object this can be simplified. for now search all possible types
    bool foundStorage = false;
    int storageIDFObjectNum = 0;

    const std::array<std::pair<std::string, StorageModelType>, 3> storageTypes{
        {{"ElectricLoadCenter:Storage:Simple", StorageModelType::SimpleBucketStorage},
         {"ElectricLoadCenter:Storage:Battery", StorageModelType::KIBaMBattery},
         {"ElectricLoadCenter:Storage:LiIonNMCBattery", StorageModelType::LiIonNmcBattery}}};

    for (auto &item : storageTypes) {
        int testStorageIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, item.first, objectName);
        if (testStorageIndex > 0) {
            foundStorage = true;
            storageIDFObjectNum = testStorageIndex;
            s_ipsc->cCurrentModuleObject = item.first;
            storageModelMode_ = item.second;
            break;
        }
    }

    if (foundStorage) {
        int numAlphas; // Number of elements in the alpha array
        int numNums;   // Number of elements in the numeric array
        int iOStat;    // IO Status when calling get input subroutine
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 storageIDFObjectNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 numAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 numNums,
                                                                 iOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        name_ = s_ipsc->cAlphaArgs(1);
        // how to verify names are unique across objects? add to GlobalNames?

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            availSched_ = Sched::GetScheduleAlwaysOn(state);
        } else if ((availSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            errorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(3)) {
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
        } else if ((zoneNum_ = Util::FindItemInList(s_ipsc->cAlphaArgs(3), state.dataHeatBal->Zone)) == 0) {
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
            ShowWarningItemNotFound(
                state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), "Storage heat losses will not be added to a zone");
            // continue with simulation but storage losses not sent to a zone.
        } else {
            heatLossesDestination_ = ThermalLossDestination::ZoneGains;
        }

        zoneRadFract_ = s_ipsc->rNumericArgs(1);

        switch (storageModelMode_) {

        case StorageModelType::SimpleBucketStorage: {
            energeticEfficCharge_ = checkUserEfficiencyInput(state, s_ipsc->rNumericArgs(2), true, name_, errorsFound);
            energeticEfficDischarge_ = checkUserEfficiencyInput(state, s_ipsc->rNumericArgs(3), false, name_, errorsFound);
            maxEnergyCapacity_ = s_ipsc->rNumericArgs(4);
            maxPowerDraw_ = s_ipsc->rNumericArgs(5);
            maxPowerStore_ = s_ipsc->rNumericArgs(6);
            startingEnergyStored_ = s_ipsc->rNumericArgs(7);
            SetupOutputVariable(state,
                                "Electric Storage Simple Charge State",
                                Constant::Units::J,
                                electEnergyinStorage_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_); // issue #4921
            break;
        }

        case StorageModelType::KIBaMBattery: {
            if (s_ipsc->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(4));
                errorsFound = true;
            } else if ((chargeCurve_ = Curve::GetCurve(state, s_ipsc->cAlphaArgs(4))) == nullptr) { // voltage calculation for charging
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                errorsFound = true;
            } else if (chargeCurve_->numDims != 1) {
                Curve::ShowSevereCurveDims(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4), "1", chargeCurve_->numDims);
                errorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(5)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(5));
                errorsFound = true;
            } else if ((dischargeCurve_ = Curve::GetCurve(state, s_ipsc->cAlphaArgs(5))) == nullptr) { // voltage calculation for discharging
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                errorsFound = true;
            } else if (dischargeCurve_->numDims != 1) {
                Curve::ShowSevereCurveDims(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), "1", dischargeCurve_->numDims);
                errorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(6)) {
                lifeCalculation_ = false;
            } else if (BooleanSwitch bs = getYesNoValue(s_ipsc->cAlphaArgs(6)); bs != BooleanSwitch::Invalid) {
                lifeCalculation_ = static_cast<bool>(bs);
            } else {
                ShowWarningInvalidBool(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6), "No");
                lifeCalculation_ = false;
            }

            if (lifeCalculation_) {
                if (s_ipsc->lAlphaFieldBlanks(7)) {
                    ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                    errorsFound = true;
                } else if ((lifeCurve_ = Curve::GetCurve(state, s_ipsc->cAlphaArgs(7))) == nullptr) { // Battery life calculation
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
                    errorsFound = true;
                } else if (lifeCurve_->numDims != 1) {
                    Curve::ShowSevereCurveDims(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7), "1", lifeCurve_->numDims);
                    errorsFound = true;
                }

                cycleBinNum_ = s_ipsc->rNumericArgs(14);

                if (!errorsFound) { // life cycle calculation for this battery, allocate arrays for degradation calculation
                                    // std::vector is zero base instead of 1, so first index is now 0.
                    b10_.resize(maxRainflowArrayBounds_ + 1, 0.0);
                    x0_.resize(maxRainflowArrayBounds_ + 1, 0);
                    nmb0_.resize(cycleBinNum_, 0.0);
                    oneNmb0_.resize(cycleBinNum_, 0.0);
                }
            }

            parallelNum_ = s_ipsc->rNumericArgs(2);
            seriesNum_ = state.dataIPShortCut->rNumericArgs(3);
            numBattery_ = parallelNum_ * seriesNum_;
            maxAhCapacity_ = state.dataIPShortCut->rNumericArgs(4);
            startingSOC_ = state.dataIPShortCut->rNumericArgs(5);
            availableFrac_ = state.dataIPShortCut->rNumericArgs(6);
            chargeConversionRate_ = state.dataIPShortCut->rNumericArgs(7);
            chargedOCV_ = state.dataIPShortCut->rNumericArgs(8);
            dischargedOCV_ = state.dataIPShortCut->rNumericArgs(9);
            internalR_ = state.dataIPShortCut->rNumericArgs(10);
            maxDischargeI_ = state.dataIPShortCut->rNumericArgs(11);
            cutoffV_ = state.dataIPShortCut->rNumericArgs(12);
            maxChargeRate_ = state.dataIPShortCut->rNumericArgs(13);

            // Check charging and discharging curves to make sure charging curve always gives a higher voltage (#8817)
            if (!errorsFound && chargeCurve_ != nullptr && dischargeCurve_ != nullptr) {
                checkChargeDischargeVoltageCurves(state, name_, chargedOCV_, dischargedOCV_, chargeCurve_, dischargeCurve_);
            }

            break;
        }
        case StorageModelType::LiIonNmcBattery: {
            lifeCalculation_ = (Util::SameString(state.dataIPShortCut->cAlphaArgs(4), "KandlerSmith") || state.dataIPShortCut->lAlphaFieldBlanks(4));
            seriesNum_ = static_cast<int>(state.dataIPShortCut->rNumericArgs(2));
            parallelNum_ = static_cast<int>(state.dataIPShortCut->rNumericArgs(3));
            startingSOC_ = state.dataIPShortCut->lNumericFieldBlanks(4) ? 0.5 : state.dataIPShortCut->rNumericArgs(4);
            liIon_dcToDcChargingEff_ = state.dataIPShortCut->lNumericFieldBlanks(5) ? 0.95 : state.dataIPShortCut->rNumericArgs(5);
            liIon_mass_ = state.dataIPShortCut->rNumericArgs(6);
            liIon_surfaceArea_ = state.dataIPShortCut->rNumericArgs(7);
            liIon_Cp_ = state.dataIPShortCut->lNumericFieldBlanks(8) ? 1500.0 : state.dataIPShortCut->rNumericArgs(8);
            liIon_heatTransferCoef_ = state.dataIPShortCut->lNumericFieldBlanks(9) ? 7.5 : state.dataIPShortCut->rNumericArgs(9);
            liIon_Vfull_ = state.dataIPShortCut->lNumericFieldBlanks(10) ? 4.2 : state.dataIPShortCut->rNumericArgs(10);
            liIon_Vexp_ = state.dataIPShortCut->lNumericFieldBlanks(11) ? 3.53 : state.dataIPShortCut->rNumericArgs(11);
            liIon_Vnom_ = state.dataIPShortCut->lNumericFieldBlanks(12) ? 3.342 : state.dataIPShortCut->rNumericArgs(12);
            if (liIon_Vfull_ < liIon_Vexp_ || liIon_Vexp_ < liIon_Vnom_) {
                ShowSevereError(
                    state,
                    format(
                        "{}{}=\"{}\", invalid entry.", routineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("{} must be greater than {},",
                                         state.dataIPShortCut->cNumericFieldNames(10),
                                         state.dataIPShortCut->cNumericFieldNames(11)));
                ShowContinueError(state, format("which must be greater than {}.", state.dataIPShortCut->cNumericFieldNames(12)));
                for (int i = 10; i <= 12; ++i) {
                    ShowContinueError(state,
                                      format("{} = {:.3R}", state.dataIPShortCut->cNumericFieldNames(i), state.dataIPShortCut->rNumericArgs(i)));
                }
                errorsFound = true;
            }
            liIon_Vnom_default_ = state.dataIPShortCut->lNumericFieldBlanks(13) ? 3.342 : state.dataIPShortCut->rNumericArgs(13);
            liIon_Qfull_ = state.dataIPShortCut->lNumericFieldBlanks(14) ? 3.2 : state.dataIPShortCut->rNumericArgs(14);
            liIon_Qexp_ =
                state.dataIPShortCut->lNumericFieldBlanks(15) ? 0.8075 * liIon_Qfull_ : state.dataIPShortCut->rNumericArgs(15) * liIon_Qfull_;
            liIon_Qnom_ =
                state.dataIPShortCut->lNumericFieldBlanks(16) ? 0.976875 * liIon_Qfull_ : state.dataIPShortCut->rNumericArgs(16) * liIon_Qfull_;
            if (liIon_Qexp_ >= liIon_Qnom_) {
                ShowSevereError(
                    state,
                    format(
                        "{}{}=\"{}\", invalid entry.", routineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("{} must be greater than {}.",
                                         state.dataIPShortCut->cNumericFieldNames(16),
                                         state.dataIPShortCut->cNumericFieldNames(15)));
                for (int i = 15; i <= 16; ++i) {
                    ShowContinueError(state,
                                      format("{} = {:.3R}", state.dataIPShortCut->cNumericFieldNames(i), state.dataIPShortCut->rNumericArgs(i)));
                }
                errorsFound = true;
            }
            liIon_C_rate_ = state.dataIPShortCut->lNumericFieldBlanks(17) ? 1.0 : state.dataIPShortCut->rNumericArgs(17);
            internalR_ = state.dataIPShortCut->lNumericFieldBlanks(18) ? 0.09 : state.dataIPShortCut->rNumericArgs(18);

            maxAhCapacity_ = liIon_Qfull_ * parallelNum_;

            if (!errorsFound) {
                // Set the Lifetime model in SSC
                // I'm using a raw pointer here because the the battery_t constructor expects it.
                // The pointer is then passed into the battery_t where it is converted into a unique_ptr and persists along with that object.
                // Therefore I am not deleting this pointer here because that will be handled by the battery_t class.
                lifetime_t *battLifetime;
                if (lifeCalculation_) {
                    battLifetime = new lifetime_nmc_t(state.dataHVACGlobal->TimeStepSys);
                } else {
                    // This sets a lifetime model where the capacity is always 100%.
                    std::vector<double> tblVals{{20, 0, 100, 20, 5000, 100, 20, 10000, 100, 80, 0, 100, 80, 1000, 100, 80, 2000, 100}};
                    util::matrix_t<double> battLifetimeMatrix(6, 3, &tblVals);
                    battLifetime = new lifetime_calendar_cycle_t(battLifetimeMatrix, state.dataHVACGlobal->TimeStepSys);
                }

                // Create the SSC battery object
                ssc_battery_ = std::unique_ptr<battery_t>(
                    new battery_t(state.dataHVACGlobal->TimeStepSys,
                                  battery_params::CHEM::LITHIUM_ION,
                                  new capacity_lithium_ion_t(maxAhCapacity_, // Capacity of the whole battery
                                                             startingSOC_ * 100.0,
                                                             100.0, // Reset later
                                                             0.0,   // Reset later
                                                             state.dataHVACGlobal->TimeStepSys),
                                  new voltage_dynamic_t(seriesNum_,
                                                        parallelNum_,
                                                        liIon_Vnom_default_,
                                                        liIon_Vfull_,
                                                        liIon_Vexp_,
                                                        liIon_Vnom_,
                                                        liIon_Qfull_, // Capacity of one cell
                                                        liIon_Qexp_,
                                                        liIon_Qnom_,
                                                        liIon_C_rate_,
                                                        internalR_,
                                                        state.dataHVACGlobal->TimeStepSys),
                                  battLifetime,
                                  new thermal_t(state.dataHVACGlobal->TimeStepSys,
                                                liIon_mass_,
                                                liIon_surfaceArea_,
                                                internalR_ * seriesNum_ / parallelNum_, // Electric resistance of the whole battery
                                                liIon_Cp_,
                                                liIon_heatTransferCoef_,
                                                20.0 // Picking a temperature for now, will reset before each run.
                                                ),
                                  nullptr));
                ssc_lastBatteryState_ = std::make_unique<battery_state>(ssc_battery_->get_state());
                ssc_lastBatteryTimeStep_ = ssc_battery_->get_params().dt_hr;
                ssc_initBatteryState_ = std::make_unique<battery_state>(ssc_battery_->get_state());
            }

            break;
        }
        case StorageModelType::Invalid: {
            // do nothing
            break;
        }
        default:
            assert(false);
        } // switch storage model type

        if (storageModelMode_ == StorageModelType::KIBaMBattery || storageModelMode_ == StorageModelType::LiIonNmcBattery) {
            SetupOutputVariable(state,
                                "Electric Storage Operating Mode Index",
                                Constant::Units::None,
                                storageMode_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_);
            SetupOutputVariable(state,
                                "Electric Storage Battery Charge State",
                                Constant::Units::Ah,
                                absoluteSOC_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_); // issue #4921
            SetupOutputVariable(state,
                                "Electric Storage Charge Fraction",
                                Constant::Units::None,
                                fractionSOC_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_);
            SetupOutputVariable(state,
                                "Electric Storage Total Current",
                                Constant::Units::A,
                                batteryCurrent_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_);
            SetupOutputVariable(state,
                                "Electric Storage Total Voltage",
                                Constant::Units::V,
                                batteryVoltage_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_);

            if (lifeCalculation_) {
                SetupOutputVariable(state,
                                    "Electric Storage Degradation Fraction",
                                    Constant::Units::None,
                                    batteryDamage_,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    name_);
            }
        }

        SetupOutputVariable(state,
                            "Electric Storage Charge Power",
                            Constant::Units::W,
                            storedPower_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Electric Storage Charge Energy",
                            Constant::Units::J,
                            storedEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Electric Storage Production Decrement Energy",
                            Constant::Units::J,
                            decrementedEnergyStored_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityProduced,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::ElectricStorage);
        SetupOutputVariable(state,
                            "Electric Storage Discharge Power",
                            Constant::Units::W,
                            drawnPower_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Electric Storage Discharge Energy",
                            Constant::Units::J,
                            drawnEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_,
                            Constant::eResource::ElectricityProduced,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::ElectricStorage);
        SetupOutputVariable(state,
                            "Electric Storage Thermal Loss Rate",
                            Constant::Units::W,
                            thermLossRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Electric Storage Thermal Loss Energy",
                            Constant::Units::J,
                            thermLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            if (storageModelMode_ == StorageModelType::SimpleBucketStorage) {
                SetupEMSInternalVariable(state, "Electrical Storage Simple Maximum Capacity", name_, "[J]", maxEnergyCapacity_);
            } else if (storageModelMode_ == StorageModelType::KIBaMBattery) {
                SetupEMSInternalVariable(state, "Electrical Storage Battery Maximum Capacity", name_, "[Ah]", maxAhCapacity_);
            }
        }
        if (storageModelMode_ == StorageModelType::LiIonNmcBattery) {
            SetupOutputVariable(state,
                                "Electric Storage Battery Temperature",
                                Constant::Units::C,
                                batteryTemperature_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                name_);
        }

        if (zoneNum_ > 0) {
            switch (storageModelMode_) {
            case StorageModelType::SimpleBucketStorage: {
                SetupZoneInternalGain(
                    state, zoneNum_, name_, DataHeatBalance::IntGainType::ElectricLoadCenterStorageSimple, &qdotConvZone_, nullptr, &qdotRadZone_);
                break;
            }
            case StorageModelType::KIBaMBattery: {
                SetupZoneInternalGain(
                    state, zoneNum_, name_, DataHeatBalance::IntGainType::ElectricLoadCenterStorageBattery, &qdotConvZone_, nullptr, &qdotRadZone_);
                break;
            }
            case StorageModelType::LiIonNmcBattery: {
                SetupZoneInternalGain(state,
                                      zoneNum_,
                                      name_,
                                      DataHeatBalance::IntGainType::ElectricLoadCenterStorageLiIonNmcBattery,
                                      &qdotConvZone_,
                                      nullptr,
                                      &qdotRadZone_);
                break;
            }
            case StorageModelType::Invalid: {
                // do nothing
                break;
            }
            default:
                assert(false);
            } // switch storage model type
        }
    } else { // storage not found
        ShowSevereError(state, format("{} did not find storage name = {}", routineName, objectName));
        errorsFound = true;
    }
    if (errorsFound) {
        ShowFatalError(state, format("{}Preceding errors terminate program.", routineName));
    }
}

Real64 checkUserEfficiencyInput(EnergyPlusData &state, Real64 userInputValue, bool isCharging, std::string const &deviceName, bool &errorsFound)
{
    Real64 constexpr minChargeEfficiency = 0.001;
    Real64 constexpr minDischargeEfficiency = 0.001;

    // Fix for Defect #8867.  Do not allow either efficiency to be zero as it will lead to a divide by zero (NaN).
    if (isCharging) {
        if (userInputValue < minChargeEfficiency) {
            ShowSevereError(state,
                            format("ElectricStorage charge efficiency was too low.  This occurred for electric storage unit named {}", deviceName));
            ShowContinueError(state, "Please check your input value  for this electric storage unit and fix the charge efficiency.");
            errorsFound = true;
            return minChargeEfficiency;
        } else {
            return userInputValue;
        }
    } else { // discharging
        if (userInputValue < minDischargeEfficiency) {
            ShowSevereError(
                state, format("ElectricStorage discharge efficiency was too low.  This occurred for electric storage unit named {}", deviceName));
            ShowContinueError(state, "Please check your input value  for this electric storage unit and fix the discharge efficiency.");
            errorsFound = true;
            return minDischargeEfficiency;
        } else {
            return userInputValue;
        }
    }
}

void checkChargeDischargeVoltageCurves(
    EnergyPlusData &state, std::string_view nameBatt, Real64 const E0c, Real64 const E0d, Curve::Curve *chargeCurve, Curve::Curve *dischargeCurve)
{
    int constexpr numChecks = 50;  // number of divisions for checking the functions from 0 to 1 for fraction charged/discharged
    int constexpr numReports = 10; // number of divisions for reporting
    bool gotErrs = false;

    // Fix for Defect #8817.  When the charging curve results in a lower voltage than the discharging curve, the battery
    // will give the appearance that the energy in Joules being removed from the battery exceeds what was stored.  This
    // checks to make sure that voltage for charging is always higher than discharging at the same fraction charged.
    for (int loop = 1; loop <= numChecks + 1; ++loop) {
        Real64 xfc = float(loop - 1) / float(numChecks);                   // = q0/qmax
        Real64 xfd = 1.0 - xfc;                                            // = (qmax-q0)/qmax = 1 - xfc
        Real64 chargeVoltage = E0d + chargeCurve->value(state, xfc);       // E0d+Ac*xfc+Cc*xfc/(Dc-xfc)
        Real64 dischargeVoltage = E0c + dischargeCurve->value(state, xfd); // E0c+Ad*xfd+Cd*xfd/(Dd-xfd)
        if (dischargeVoltage > chargeVoltage) {
            gotErrs = true;
            break;
        }
    }
    if (gotErrs) {
        ShowWarningMessage(state, format("Kinetic Battery Model: {} has a charging/discharging voltage curve conflict.", nameBatt));
        ShowContinueError(state,
                          "Discharging voltage is higher than charging voltage which may potentially lead to an imbalance in the stored energy.");
        ShowContinueError(state, "Check the charging and discharging curves to make sure that the charging voltage is greater than discharging.");
        ShowContinueError(state, "Also check the charging and discharging energy outputs to find any discrepancies.");
        for (int loop = 1; loop <= numReports + 1; ++loop) {
            Real64 xfc = float(loop - 1) / float(numReports);                  // = q0/qmax
            Real64 xfd = 1.0 - xfc;                                            // = (qmax-q0)/qmax = 1 - xfc
            Real64 chargeVoltage = E0d + chargeCurve->value(state, xfc);       // E0d+Ac*xfc+Cc*xfc/(Dc-xfc)
            Real64 dischargeVoltage = E0c + dischargeCurve->value(state, xfd); // E0c+Ad*xfd+Cd*xfd/(Dd-xfd)
            ShowContinueError(
                state,
                format(
                    "Charged fraction = {:.1R}, Charging voltage = {:.3R} V, Discharging voltage = {:.3R} V", xfc, chargeVoltage, dischargeVoltage));
        }
    }
}

void ElectricStorage::reinitAtBeginEnvironment()
{
    pelNeedFromStorage_ = 0.0;
    pelFromStorage_ = 0.0;
    pelIntoStorage_ = 0.0;
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
    timeElapsed_ = 0.0;
    electEnergyinStorage_ = 0.0;
    storedPower_ = 0.0;
    storedEnergy_ = 0.0;
    decrementedEnergyStored_ = 0.0;
    drawnPower_ = 0.0;
    drawnEnergy_ = 0.0;
    thermLossRate_ = 0.0;
    thermLossEnergy_ = 0.0;
    lastTimeStepStateOfCharge_ = startingEnergyStored_;
    thisTimeStepStateOfCharge_ = startingEnergyStored_;

    if (storageModelMode_ == StorageModelType::KIBaMBattery) {
        Real64 initialCharge = maxAhCapacity_ * startingSOC_;
        lastTwoTimeStepAvailable_ = initialCharge * availableFrac_;
        lastTwoTimeStepBound_ = initialCharge * (1.0 - availableFrac_);
        lastTimeStepAvailable_ = initialCharge * availableFrac_;
        lastTimeStepBound_ = initialCharge * (1.0 - availableFrac_);
        thisTimeStepAvailable_ = initialCharge * availableFrac_;
        thisTimeStepBound_ = initialCharge * (1.0 - availableFrac_);
        if (lifeCalculation_) {
            count0_ = 1;            // Index 0 is for initial SOC, so new input starts from index 1.
            b10_[0] = startingSOC_; // the initial fractional SOC is stored as the reference
            x0_[0] = 0.0;
            for (int loop = 1; loop < maxRainflowArrayBounds_ + 1; ++loop) {
                b10_[loop] = 0.0;
                x0_[loop] = 0.0;
            }
            for (int loop = 0; loop < cycleBinNum_; ++loop) {
                oneNmb0_[loop] = 0.0;
                nmb0_[loop] = 0.0;
            }
            batteryDamage_ = 0.0;
        }
    } else if (storageModelMode_ == StorageModelType::LiIonNmcBattery) {
        // Copy the initial battery state to the last battery state
        *ssc_lastBatteryState_ = *ssc_initBatteryState_;
        ssc_lastBatteryTimeStep_ = ssc_initBatteryTimeStep_;
        ssc_battery_->set_state(*ssc_lastBatteryState_, ssc_initBatteryTimeStep_);
    }
    myWarmUpFlag_ = true;
}

void ElectricStorage::reinitZoneGainsAtBeginEnvironment()
{
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

void ElectricStorage::reinitAtEndWarmup()
{
    // need to reset initial state of charge at beginning of environment but after warm up is complete
    lastTimeStepStateOfCharge_ = startingEnergyStored_;
    thisTimeStepStateOfCharge_ = startingEnergyStored_;
    if (storageModelMode_ == StorageModelType::KIBaMBattery) {
        Real64 initialCharge = maxAhCapacity_ * startingSOC_;
        lastTwoTimeStepAvailable_ = initialCharge * availableFrac_;
        lastTwoTimeStepBound_ = initialCharge * (1.0 - availableFrac_);
        lastTimeStepAvailable_ = initialCharge * availableFrac_;
        lastTimeStepBound_ = initialCharge * (1.0 - availableFrac_);
        thisTimeStepAvailable_ = initialCharge * availableFrac_;
        thisTimeStepBound_ = initialCharge * (1.0 - availableFrac_);
        if (lifeCalculation_) {
            count0_ = 1;            // Index 0 is for initial SOC, so new input starts from index 1.
            b10_[0] = startingSOC_; // the initial fractional SOC is stored as the reference
            x0_[0] = 0.0;
            for (int loop = 1; loop < maxRainflowArrayBounds_ + 1; ++loop) {
                b10_[loop] = 0.0;
                x0_[loop] = 0.0;
            }
            for (int loop = 0; loop < cycleBinNum_; ++loop) {
                oneNmb0_[loop] = 0.0;
                nmb0_[loop] = 0.0;
            }
            batteryDamage_ = 0.0;
        }
    } else if (storageModelMode_ == StorageModelType::LiIonNmcBattery) {
        // Copy the initial battery state to the last battery state
        *ssc_lastBatteryState_ = *ssc_initBatteryState_;
        ssc_lastBatteryTimeStep_ = ssc_initBatteryTimeStep_;
        ssc_battery_->set_state(*ssc_lastBatteryState_, ssc_lastBatteryTimeStep_);
    }
    myWarmUpFlag_ = false;
}

void ElectricStorage::timeCheckAndUpdate(EnergyPlusData &state)
{

    if (myWarmUpFlag_ && !state.dataGlobal->WarmupFlag) {
        reinitAtEndWarmup();
    }

    Real64 timeElapsedLoc =
        state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;
    if (timeElapsed_ != timeElapsedLoc) { // time changed, update last with "current" result from previous time
        if (storageModelMode_ == StorageModelType::KIBaMBattery && lifeCalculation_) {
            //    At this point, the current values, last time step values and last two time step values have not been updated, hence:
            //    "ThisTimeStep*" actually points to the previous one time step
            //    "LastTimeStep*" actually points to the previous two time steps
            //    "LastTwoTimeStep" actually points to the previous three time steps

            //      Calculate the fractional SOC change between the "current" time step and the "previous one" time step
            Real64 deltaSOC1 = thisTimeStepAvailable_ + thisTimeStepBound_ - lastTimeStepAvailable_ - lastTimeStepBound_;
            deltaSOC1 /= maxAhCapacity_;

            //      Calculate the fractional SOC change between the "previous one" time step and the "previous two" time steps
            Real64 deltaSOC2 = lastTimeStepAvailable_ + lastTimeStepBound_ - lastTwoTimeStepAvailable_ - lastTwoTimeStepBound_;
            deltaSOC2 /= maxAhCapacity_;

            //     DeltaSOC2 = 0 may occur at the begining of each simulation environment.
            //     DeltaSOC1 * DeltaSOC2 means that the SOC from "LastTimeStep" is a peak or valley. Only peak or valley needs
            //     to call the rain flow algorithm
            if ((deltaSOC2 == 0) || ((deltaSOC1 * deltaSOC2) < 0)) {
                //     Because we cannot determine whehter "ThisTimeStep" is a peak or valley (next time step is unknown yet), we
                //     use the "LastTimeStep" value for battery life calculation.
                Real64 input0 = (lastTimeStepAvailable_ + lastTimeStepBound_) / maxAhCapacity_;
                b10_[count0_] = input0;

                //        The array size needs to be increased when count = MaxRainflowArrayBounds. Please note that (MaxRainflowArrayBounds +1)
                //        is the index used in the subroutine RainFlow. So we cannot reallocate array size until count = MaxRainflowArrayBounds +1.

                int constexpr maxRainflowArrayInc_ = 100;

                if (count0_ == maxRainflowArrayBounds_) {
                    b10_.resize(maxRainflowArrayBounds_ + 1 + maxRainflowArrayInc_, 0.0);
                    x0_.resize(maxRainflowArrayBounds_ + 1 + maxRainflowArrayInc_, 0.0);
                    maxRainflowArrayBounds_ += maxRainflowArrayInc_;
                }

                rainflow(cycleBinNum_, input0, b10_, x0_, count0_, nmb0_, oneNmb0_);

                batteryDamage_ = 0.0;

                for (int binNum = 0; binNum < cycleBinNum_; ++binNum) {
                    //       Battery damage is calculated by accumulating the impact from each cycle.
                    batteryDamage_ += oneNmb0_[binNum] / lifeCurve_->value(state, (double(binNum) / double(cycleBinNum_)));
                }
            }
        } else if (storageModelMode_ == StorageModelType::LiIonNmcBattery) {
            *ssc_lastBatteryState_ = ssc_battery_->get_state();
            ssc_lastBatteryTimeStep_ = ssc_battery_->get_params().dt_hr;
        }

        lastTimeStepStateOfCharge_ = thisTimeStepStateOfCharge_;
        lastTwoTimeStepAvailable_ = lastTimeStepAvailable_;
        lastTwoTimeStepBound_ = lastTimeStepBound_;
        lastTimeStepAvailable_ = thisTimeStepAvailable_;
        lastTimeStepBound_ = thisTimeStepBound_;
        timeElapsed_ = timeElapsedLoc;

    } // end if time changed
}

void ElectricStorage::simulate(EnergyPlusData &state,
                               Real64 &powerCharge,
                               Real64 &powerDischarge,
                               bool &charging,
                               bool &discharging,
                               Real64 const controlSOCMaxFracLimit,
                               Real64 const controlSOCMinFracLimit)
{
    // pass thru to constrain function depending on storage model type
    if (availSched_->getCurrentVal() == 0.0) { // storage not available
        discharging = false;
        powerDischarge = 0.0;
        charging = false;
        powerCharge = 0.0;
    }

    if (storageModelMode_ == StorageModelType::SimpleBucketStorage) {
        simulateSimpleBucketModel(state, powerCharge, powerDischarge, charging, discharging, controlSOCMaxFracLimit, controlSOCMinFracLimit);
    } else if (storageModelMode_ == StorageModelType::KIBaMBattery) {
        simulateKineticBatteryModel(state, powerCharge, powerDischarge, charging, discharging, controlSOCMaxFracLimit, controlSOCMinFracLimit);
    } else if (storageModelMode_ == StorageModelType::LiIonNmcBattery) {
        simulateLiIonNmcBatteryModel(state, powerCharge, powerDischarge, charging, discharging, controlSOCMaxFracLimit, controlSOCMinFracLimit);
    }
}

std::string const &ElectricStorage::name() const
{
    return name_;
}

void ElectricStorage::simulateSimpleBucketModel(EnergyPlusData &state,
                                                Real64 &powerCharge,
                                                Real64 &powerDischarge,
                                                bool &charging,
                                                bool &discharging,
                                                Real64 const controlSOCMaxFracLimit,
                                                Real64 const controlSOCMinFracLimit)
{

    // given arguments for how the storage operation would like to run storage charge or discharge
    // apply model constraints and adjust arguments accordingly

    if (charging) {

        if (lastTimeStepStateOfCharge_ >= (maxEnergyCapacity_ * controlSOCMaxFracLimit)) {
            // storage full!  no more allowed!
            powerCharge = 0.0;
            charging = false;
        }
        if (powerCharge > maxPowerStore_) {
            powerCharge = maxPowerStore_;
        }

        // now check to see if charge would exceed capacity, and modify to just fill physical storage cap
        if ((lastTimeStepStateOfCharge_ + powerCharge * state.dataHVACGlobal->TimeStepSysSec * energeticEfficCharge_) >=
            (maxEnergyCapacity_ * controlSOCMaxFracLimit)) {
            powerCharge = ((maxEnergyCapacity_ * controlSOCMaxFracLimit) - lastTimeStepStateOfCharge_) /
                          (state.dataHVACGlobal->TimeStepSysSec * energeticEfficCharge_);
        }
    } // charging

    if (discharging) {

        if (lastTimeStepStateOfCharge_ <= (maxEnergyCapacity_ * controlSOCMinFracLimit)) {
            // storage empty  no more allowed!
            powerDischarge = 0.0;
            discharging = false;
        }
        if (powerDischarge > maxPowerDraw_) {
            powerDischarge = maxPowerDraw_;
        }
        // now check if will empty this timestep, power draw is amplified by energetic effic
        if ((lastTimeStepStateOfCharge_ - powerDischarge * state.dataHVACGlobal->TimeStepSysSec / energeticEfficDischarge_) <=
            (maxEnergyCapacity_ * controlSOCMinFracLimit)) {
            powerDischarge = (lastTimeStepStateOfCharge_ - (maxEnergyCapacity_ * controlSOCMinFracLimit)) * energeticEfficDischarge_ /
                             (state.dataHVACGlobal->TimeStepSysSec);
        }
    }

    if ((!charging) && (!discharging)) {
        thisTimeStepStateOfCharge_ = lastTimeStepStateOfCharge_;
        pelIntoStorage_ = 0.0;
        pelFromStorage_ = 0.0;
    }
    if (charging) {
        pelIntoStorage_ = powerCharge;
        pelFromStorage_ = 0.0;
        thisTimeStepStateOfCharge_ = lastTimeStepStateOfCharge_ + powerCharge * state.dataHVACGlobal->TimeStepSysSec * energeticEfficCharge_;
    }
    if (discharging) {
        pelIntoStorage_ = 0.0;
        pelFromStorage_ = powerDischarge;
        thisTimeStepStateOfCharge_ = lastTimeStepStateOfCharge_ - powerDischarge * state.dataHVACGlobal->TimeStepSysSec / energeticEfficDischarge_;
        thisTimeStepStateOfCharge_ = max(thisTimeStepStateOfCharge_, 0.0);
    }

    // updates and reports
    electEnergyinStorage_ = thisTimeStepStateOfCharge_; //[J]
    storedPower_ = pelIntoStorage_;
    storedEnergy_ = pelIntoStorage_ * state.dataHVACGlobal->TimeStepSysSec;
    decrementedEnergyStored_ = -1.0 * storedEnergy_;
    drawnPower_ = pelFromStorage_;
    drawnEnergy_ = pelFromStorage_ * state.dataHVACGlobal->TimeStepSysSec;
    thermLossRate_ = max(storedPower_ * (1.0 - energeticEfficCharge_), drawnPower_ * (1.0 - energeticEfficDischarge_));
    thermLossEnergy_ = thermLossRate_ * state.dataHVACGlobal->TimeStepSysSec;

    if (zoneNum_ > 0) { // set values for zone heat gains
        qdotConvZone_ = (1.0 - zoneRadFract_) * thermLossRate_;
        qdotRadZone_ = (zoneRadFract_)*thermLossRate_;
    }
}

void ElectricStorage::simulateKineticBatteryModel(EnergyPlusData &state,
                                                  Real64 &powerCharge,
                                                  Real64 &powerDischarge,
                                                  bool &charging,
                                                  bool &discharging,
                                                  Real64 const controlSOCMaxFracLimit,
                                                  Real64 const controlSOCMinFracLimit)
{

    // initialize locals
    Real64 I0 = 0.0;
    Real64 Volt = 0.0;

    Real64 T0 = 0.0;
    Real64 qmaxf = 0.0;
    Real64 Ef = 0.0;
    Real64 q0 = 0.0;

    Real64 qmax = maxAhCapacity_;
    Real64 E0c = chargedOCV_;
    Real64 E0d = dischargedOCV_;
    Real64 k = chargeConversionRate_;
    Real64 c = availableFrac_;

    if (charging) {

        //*************************************************
        // The sign of power and current is negative in charging
        //*************************************************
        Real64 Pw = -powerCharge / numBattery_;
        q0 = lastTimeStepAvailable_ + lastTimeStepBound_;
        if (q0 > qmax * controlSOCMaxFracLimit) {
            // stop charging with controller signal for max state of charge
            Pw = 0.0;
            powerCharge = 0.0;
            charging = false;
            storageMode_ = 0;
            storedPower_ = 0.0;
            storedEnergy_ = 0.0;
            decrementedEnergyStored_ = 0.0;
            drawnPower_ = 0.0;
            drawnEnergy_ = 0.0;
            return;
        }

        I0 = 1.0;                                                                                       // Initial assumption
        T0 = std::abs(qmax / I0);                                                                       // Initial Assumption
        qmaxf = qmax * k * c * T0 / (1.0 - std::exp(-k * T0) + c * (k * T0 - 1.0 + std::exp(-k * T0))); // Initial calculation of a function qmax(I)
        Real64 Xf = q0 / qmaxf;
        Ef = E0d + chargeCurve_->value(state, Xf); // E0d+Ac*Xf+Cc*Xf/(Dc-Xf) (use curve)
        Volt = Ef - I0 * internalR_;
        Real64 Inew = 0.0;
        if (Volt != 0.0) {
            Inew = Pw / Volt;
        }
        Real64 Tnew = 0.0;
        if (Inew != 0.0) {
            Tnew = qmaxf / std::abs(Inew);
        }
        Real64 error = 1.0;

        while (error > 0.0001) { // Iteration process to get converged current(I)
            I0 = Inew;
            T0 = Tnew;
            qmaxf = qmax * k * c * T0 / (1.0 - std::exp(-k * T0) + c * (k * T0 - 1.0 + std::exp(-k * T0)));
            Xf = q0 / qmaxf;
            Ef = E0d + chargeCurve_->value(state, Xf); // E0d+Ac*Xf+Cc*Xf/(Dc-Xf) (use curve)
            Volt = Ef - I0 * internalR_;
            Inew = Pw / Volt;
            Tnew = std::abs(qmaxf / Inew); // ***Always positive here
            error = std::abs(Inew - I0);
        }

        Real64 dividend = -k * c * qmax + k * lastTimeStepAvailable_ * std::exp(-k * state.dataHVACGlobal->TimeStepSys) +
                          q0 * k * c * (1.0 - std::exp(-k * state.dataHVACGlobal->TimeStepSys));
        Real64 divisor = 1.0 - std::exp(-k * state.dataHVACGlobal->TimeStepSys) +
                         c * (k * state.dataHVACGlobal->TimeStepSys - 1 + std::exp(-k * state.dataHVACGlobal->TimeStepSys));
        Real64 Imax = dividend / divisor;
        // Below: This is the limit of charging current from Charge Rate Limit (input)
        Imax = max(Imax, -(qmax - q0) * maxChargeRate_);

        if (std::abs(I0) <= std::abs(Imax)) {
            I0 = Pw / Volt;
            // Pactual = I0 * Volt;
        } else {
            I0 = Imax;
            qmaxf = 80.0; // Initial assumption to solve the equation using iterative method
            error = 10.0; // Initial assumption ...
            while (error > 0.001) {
                // *** I0(current) should be positive for this calculation
                Real64 RHS = (qmax * k * c * qmaxf / std::abs(I0)) /
                             (1.0 - std::exp(-k * qmaxf / std::abs(I0)) + c * (k * qmaxf / std::abs(I0) - 1.0 + std::exp(-k * qmaxf / std::abs(I0))));
                error = std::abs(qmaxf - RHS);
                qmaxf = RHS;
            }
        }
    }

    if (discharging) {
        //**********************************************
        // The sign of power and current is positive in discharging
        //**********************************************

        Real64 Pw = powerDischarge / numBattery_;
        q0 = lastTimeStepAvailable_ + lastTimeStepBound_;

        if (q0 < qmax * controlSOCMinFracLimit) {
            // stop discharging with controller signal for min state of charge
            Pw = 0.0;
            discharging = false;
            powerDischarge = 0.0;
            storageMode_ = 0;
            storedPower_ = 0.0;
            storedEnergy_ = 0.0;
            decrementedEnergyStored_ = 0.0;
            drawnPower_ = 0.0;
            drawnEnergy_ = 0.0;
            return;
        }

        bool const ok = determineCurrentForBatteryDischarge(state, I0, T0, Volt, Pw, q0, dischargeCurve_, k, c, qmax, E0c, internalR_);
        if (!ok) {
            ShowFatalError(state,
                           format("ElectricLoadCenter:Storage:Battery named=\"{}\". Battery discharge current could not be estimated due to "
                                  "iteration limit reached. ",
                                  name_));
            // issue #5301, need more diagnostics for this.
        }

        Real64 dividend = k * lastTimeStepAvailable_ * std::exp(-k * state.dataHVACGlobal->TimeStepSys) +
                          q0 * k * c * (1.0 - std::exp(-k * state.dataHVACGlobal->TimeStepSys));
        Real64 divisor = 1.0 - std::exp(-k * state.dataHVACGlobal->TimeStepSys) +
                         c * (k * state.dataHVACGlobal->TimeStepSys - 1.0 + std::exp(-k * state.dataHVACGlobal->TimeStepSys));
        Real64 Imax = dividend / divisor;
        Imax = min(Imax, maxDischargeI_);
        if (std::abs(I0) <= Imax) {
            I0 = Pw / Volt;
            // Pactual = I0 * Volt;
        } else {
            I0 = Imax;
            qmaxf = 10.0;        // Initial assumption to solve the equation using iterative method
            Real64 error = 10.0; // Initial assumption ...
            while (error > 0.001) {
                Real64 RHS = (qmax * k * c * qmaxf / I0) / (1.0 - std::exp(-k * qmaxf / I0) + c * (k * qmaxf / I0 - 1 + std::exp(-k * qmaxf / I0)));
                error = std::abs(qmaxf - RHS);
                qmaxf = RHS;
            }
            Real64 Xf = (qmax - q0) / qmaxf;
            Ef = E0c + dischargeCurve_->value(state, Xf);
            Volt = Ef - I0 * internalR_;
        }
        if (Volt < cutoffV_) {
            I0 = 0.0;
        }
    } // if discharging

    if ((!charging) && (!discharging)) {
        thisTimeStepAvailable_ = lastTimeStepAvailable_;
        thisTimeStepBound_ = lastTimeStepBound_;
        I0 = 0.0;
        Volt = 0.0;
        q0 = lastTimeStepAvailable_ + lastTimeStepBound_;
    } else {
        Real64 newAvailable = lastTimeStepAvailable_ * std::exp(-k * state.dataHVACGlobal->TimeStepSys) +
                              (q0 * k * c - I0) * (1.0 - std::exp(-k * state.dataHVACGlobal->TimeStepSys)) / k -
                              I0 * c * (k * state.dataHVACGlobal->TimeStepSys - 1.0 + std::exp(-k * state.dataHVACGlobal->TimeStepSys)) / k;
        Real64 newBound = lastTimeStepBound_ * std::exp(-k * state.dataHVACGlobal->TimeStepSys) +
                          q0 * (1.0 - c) * (1.0 - std::exp(-k * state.dataHVACGlobal->TimeStepSys)) -
                          I0 * (1.0 - c) * (k * state.dataHVACGlobal->TimeStepSys - 1.0 + std::exp(-k * state.dataHVACGlobal->TimeStepSys)) / k;
        thisTimeStepAvailable_ = max(0.0, newAvailable);
        thisTimeStepBound_ = max(0.0, newBound);
    }

    // Pactual = I0 * Volt;
    Real64 TotalSOC = thisTimeStepAvailable_ + thisTimeStepBound_;

    // output1
    if (TotalSOC > q0) {
        storageMode_ = 2;
        storedPower_ = -1.0 * Volt * I0 * numBattery_; // Issue #5303, fix sign issue
        storedEnergy_ = -1.0 * Volt * I0 * numBattery_ * state.dataHVACGlobal->TimeStepSysSec;
        decrementedEnergyStored_ = -1.0 * storedEnergy_;
        drawnPower_ = 0.0;
        drawnEnergy_ = 0.0;

    } else if (TotalSOC < q0) {
        storageMode_ = 1;
        storedPower_ = 0.0;
        storedEnergy_ = 0.0;
        decrementedEnergyStored_ = 0.0;
        drawnPower_ = Volt * I0 * numBattery_;
        drawnEnergy_ = Volt * I0 * numBattery_ * state.dataHVACGlobal->TimeStepSysSec;

    } else {
        storageMode_ = 0;
        storedPower_ = 0.0;
        storedEnergy_ = 0.0;
        decrementedEnergyStored_ = 0.0;
        drawnPower_ = 0.0;
        drawnEnergy_ = 0.0;
    }

    absoluteSOC_ = TotalSOC * numBattery_;
    fractionSOC_ = TotalSOC / qmax;
    batteryCurrent_ = I0 * parallelNum_;
    batteryVoltage_ = Volt * seriesNum_;
    thermLossRate_ = internalR_ * pow_2(I0) * numBattery_;
    thermLossEnergy_ = internalR_ * pow_2(I0) * state.dataHVACGlobal->TimeStepSysSec * numBattery_;

    if (zoneNum_ > 0) { // set values for zone heat gains
        qdotConvZone_ = ((1.0 - zoneRadFract_) * thermLossRate_) * numBattery_;
        qdotRadZone_ = ((zoneRadFract_)*thermLossRate_) * numBattery_;
    }

    powerCharge = storedPower_;
    powerDischarge = drawnPower_;
}

void ElectricStorage::simulateLiIonNmcBatteryModel(EnergyPlusData &state,
                                                   Real64 &powerCharge,
                                                   Real64 &powerDischarge,
                                                   bool &charging,
                                                   bool &discharging,
                                                   Real64 const controlSOCMaxFracLimit,
                                                   Real64 const controlSOCMinFracLimit)
{

// Disable floating point exceptions around SSC battery calculations, which uses quiet_NaN in particular
#ifdef DEBUG_ARITHM_GCC_OR_CLANG
    int old_excepts = fegetexcept();
    fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

#ifdef DEBUG_ARITHM_MSVC
    unsigned int ori_fpcntrl = 0;
    _controlfp_s(&ori_fpcntrl, 0, 0);

    // Temporarily disable traps, as MSVC does it reversed compared to GCC,
    // you need to turn ON the bits for the exceptions you want to mask
    unsigned int fpcntrl = ori_fpcntrl | (_EM_ZERODIVIDE | _EM_INVALID | _EM_OVERFLOW);
    _controlfp_s(&fpcntrl, fpcntrl, _MCW_EM);
#endif

    // Copy the battery state from the end of last timestep
    battery_state battState = *ssc_lastBatteryState_;
    ssc_battery_->set_state(battState, ssc_lastBatteryTimeStep_);
    if (std::lround(ssc_battery_->get_params().dt_hr * 60.0) != std::lround(state.dataHVACGlobal->TimeStepSys * 60.0)) {
        ssc_battery_->ChangeTimestep(state.dataHVACGlobal->TimeStepSys);
    }

    // Set the temperature the battery sees
    if (zoneNum_ > 0) {
        // If in a zone, use the zone temperature
        battState.thermal->T_room = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum_).ZT;
    } else {
        // If outside, use outdoor temperature
        battState.thermal->T_room = state.dataEnvrn->OutDryBulbTemp;
    }

    // Set the SOC limits
    ssc_battery_->changeSOCLimits(controlSOCMinFracLimit * 100.0, controlSOCMaxFracLimit * 100.0);

    // Set the current timestep length

    // Run the battery
    // SAM uses negative values for charging, positive for discharging
    // E+ power/energy outputs are positive
    double power{0.0}; // Using double instead of Real64 because SSC is expecting a double
    if (charging) {
        power = -powerCharge;
    } else if (discharging) {
        power = powerDischarge;
    }
    power *= 0.001; // Convert to kW
    ssc_battery_->runPower(power);

    // Store outputs
    const battery_state &battState2{ssc_battery_->get_state()};
    if (battState2.P < 0.0) { // negative for charging
        storageMode_ = 2;
        powerCharge = fabs(battState2.P) * 1000.0; // kW -> W
        powerDischarge = 0.0;
        charging = true;
        discharging = false;
    } else if (battState2.P > 0.0) { // positive for discharging
        storageMode_ = 1;
        powerCharge = 0.0;
        powerDischarge = fabs(battState2.P) * 1000.0; // kW -> W
        charging = false;
        discharging = true;
    } else {
        storageMode_ = 0;
        powerCharge = 0.0;
        powerDischarge = 0.0;
        charging = false;
        discharging = false;
    }
    absoluteSOC_ = ssc_battery_->charge_total();
    fractionSOC_ = ssc_battery_->SOC() * 0.01; // % -> fraction
    batteryCurrent_ = ssc_battery_->I();
    batteryVoltage_ = ssc_battery_->V();
    batteryDamage_ = 1.0 - (ssc_battery_->charge_maximum_lifetime() / maxAhCapacity_);
    storedPower_ = powerCharge;
    storedEnergy_ = storedPower_ * state.dataHVACGlobal->TimeStepSysSec;
    drawnPower_ = powerDischarge;
    drawnEnergy_ = drawnPower_ * state.dataHVACGlobal->TimeStepSysSec;
    decrementedEnergyStored_ = -storedEnergy_;
    thermLossRate_ = battState2.thermal->heat_dissipated * 1000.0; // kW -> W
    thermLossEnergy_ = thermLossRate_ * state.dataHVACGlobal->TimeStepSysSec;
    batteryTemperature_ = battState2.thermal->T_batt;

    // Zone Heat Gains
    if (zoneNum_ > 0) { // set values for zone heat gains
        qdotConvZone_ = (1.0 - zoneRadFract_) * thermLossRate_;
        qdotRadZone_ = (zoneRadFract_)*thermLossRate_;
    }

#ifdef DEBUG_ARITHM_GCC_OR_CLANG
    feenableexcept(old_excepts);
#endif

#ifdef DEBUG_ARITHM_MSVC
    _controlfp_s(nullptr, ori_fpcntrl, _MCW_EM);
#endif
}

Real64 ElectricStorage::drawnPower() const
{
    return drawnPower_;
}

Real64 ElectricStorage::storedPower() const
{
    return storedPower_;
}

Real64 ElectricStorage::drawnEnergy() const
{
    return drawnEnergy_;
}

Real64 ElectricStorage::storedEnergy() const
{
    return storedEnergy_;
}

Real64 ElectricStorage::stateOfChargeFraction() const
{
    return fractionSOC_;
}

Real64 ElectricStorage::batteryTemperature() const
{
    assert(storageModelMode_ == StorageModelType::LiIonNmcBattery);
    return batteryTemperature_;
}

bool ElectricStorage::determineCurrentForBatteryDischarge(EnergyPlusData &state,
                                                          Real64 &curI0,
                                                          Real64 &curT0,
                                                          Real64 &curVolt,
                                                          Real64 const Pw, // Power withdraw from each module,
                                                          Real64 const q0, // available charge last timestep, sum of available and bound
                                                          Curve::Curve *curve,
                                                          Real64 const k,
                                                          Real64 const c,
                                                          Real64 const qmax,
                                                          Real64 const E0c,
                                                          Real64 const InternalR)
{
    curI0 = 10.0;         // Initial assumption
    curT0 = qmax / curI0; // Initial Assumption
    Real64 qmaxf = qmax * k * c * curT0 /
                   (1.0 - std::exp(-k * curT0) + c * (k * curT0 - 1.0 + std::exp(-k * curT0))); // Initial calculation of a function qmax(I)
    Real64 Xf = (qmax - q0) / qmaxf;
    Real64 Ef = E0c + curve->value(state, Xf); // E0d+Ac*Xf+Cc*X/(Dc-Xf)
    curVolt = Ef - curI0 * InternalR;
    Real64 Inew = Pw / curVolt;
    Real64 Tnew = qmaxf / Inew;
    Real64 error = 1.0;
    int countForIteration = 0;
    bool exceedIterationLimit = false;

    while (error > 0.0001) { // Iteration process to get converged current(I)
        curI0 = Inew;
        curT0 = Tnew;
        qmaxf = qmax * k * c * curT0 / (1.0 - std::exp(-k * curT0) + c * (k * curT0 - 1.0 + std::exp(-k * curT0)));
        // add div by zero protection #5301
        if (qmaxf != 0.0) {
            Xf = (qmax - q0) / qmaxf;
        } else {
            Xf = 1.0;
        }

        Ef = E0c + curve->value(state, Xf); // E0c+Ad*Xf+Cd*X/(Dd-Xf)
        curVolt = Ef - curI0 * InternalR;
        // add div by zero protection #5301
        if (curVolt != 0.0) {
            Inew = Pw / curVolt;
        } else {
            Inew = 1.0;
        }

        // add div by zero protection #5301
        if (Inew != 0.0) {
            Tnew = qmaxf / Inew;
        } else {
            Tnew = 1.0;
        }

        error = std::abs(Inew - curI0);
        ++countForIteration;
        if (countForIteration > 1000) {
            exceedIterationLimit = true;
            // Issue #5301 need more diagnostics for this case
            ShowWarningError(
                state, "ElectricStorage::determineCurrentForBatteryDischarge, iteration limit exceeded, failed to solve for discharge current.");
            ShowContinueError(state, format("Last timestep charge available, q0 = {:.5R}", q0));
            ShowContinueError(state, format("New Current, Inew = {:.5R} [Amps]", Inew));
            ShowContinueError(state, format("Power discharge per module cell, Pw = {:.5R} ", Pw));
            ShowContinueError(
                state, format("Charge Conversion Rate, [1/h] change rate from bound charge energy to available charge, parameter k = {:.5R}", k));
            ShowContinueError(state, format("parameter c = {:.5R}", c));
            ShowContinueError(state, format("parameter qmax = {:.5R}", qmax));
            ShowContinueError(state, format("Fully charged open circuit voltage, parameter E0c  = {:.5R}", E0c));
            ShowContinueError(state, format("parameter InternalR = {:.5R}", InternalR));
            if (qmaxf == 0.0) {
                ShowContinueError(state, "qmaxf was zero, would have divided by zero.");
            }
            if (Inew == 0.0) {
                ShowContinueError(state, "Inew was zero, would have divided by zero. ");
            }
            if (curVolt == 0.0) {
                ShowContinueError(state, "curVolt was zero, would have divided by zero. ");
            }

            ShowContinueErrorTimeStamp(state, "ElectricStorage::determineCurrentForBatteryDischarge ");
            break;
        }
    }
    return (!exceedIterationLimit);
}

void ElectricStorage::rainflow(int const numbin,           // numbin = constant value
                               Real64 const input,         // input = input value from other object (battery model)
                               std::vector<Real64> &B1,    // stores values of points, calculated here - stored for next timestep
                               std::vector<Real64> &X,     // stores values of two data point difference, calculated here - stored for next timestep
                               int &count,                 // calculated here - stored for next timestep in main loop
                               std::vector<Real64> &Nmb,   // calculated here - stored for next timestep in main loop
                               std::vector<Real64> &OneNmb // calculated here - stored for next timestep in main loop
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Y. KyungTae & W. Wang
    //       DATE WRITTEN   July-August, 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Rainflow cycle counting for battery life calculation

    // METHODOLOGY EMPLOYED:
    // <description>

    // REFERENCES:
    // Ariduru S. 2004. Fatigue life calculation by rainflow cycle counting method.
    //                  Master Thesis, Middle East Technical University.

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // Array B1 stores the value of points
    // Array X stores the value of two data points' difference.

    int num;

    X[count] = input - B1[count - 1]; // calculate the difference between two data (current and previous)

    // Get rid of the data if it is not peak nor valley
    // The value of count means the number of peak or valley points added to the arrary B10/B1, not including the
    // first point B10(0)/B1(0). Therefore, even if count =2, B1(count-2) is still valid.
    if (count >= 3) {
        //  The following check on peak or valley may be not necessary in most times because the same check is made in the
        //  upper-level subroutine. However, it does not hurt to leave it here.
        if (X[count] * X[count - 1] >= 0) {
            X[count - 1] = B1[count] - B1[count - 2];
            shift(B1, count - 1, count, B1); // Get rid of (count-1) row in B1
            shift(X, count, count, X);
            --count; // If the value keep increasing or decreasing, get rid of the middle point.
        } // Only valley and peak will be stored in the matrix, B1

        if ((count == 3) && (std::abs(X[2]) <= std::abs(X[3]))) {
            //  This means the starting point is included in X(2), a half cycle is counted according to the rain flow
            //  algorithm specified in the reference (Ariduru S. 2004)
            num = nint((std::abs(X[2]) * numbin * 10 + 5) / 10); // Count half cycle
            Nmb[num] += 0.5;
            // B1 = eoshift( B1, 1 ); // Once counting a half cycle, get rid of the value.
            B1.erase(B1.begin());
            B1.push_back(0.0);
            // X = eoshift( X, 1 );
            X.erase(X.begin());
            X.push_back(0.0);
            --count; // The number of matrix, B1 and X1 decrease.
        }
    } // Counting cyle end
    //*** Note: The value of "count" changes in the upper "IF LOOP"

    if (count >= 4) { // count 1 cycle
        while (std::abs(X[count]) > std::abs(X[count - 1])) {
            //  This means that the starting point is not included in X(count-1). a cycle is counted according to the rain flow
            //  algorithm specified in the reference (Ariduru S. 2004)
            num = nint((std::abs(X[count - 1]) * numbin * 10 + 5) / 10);
            ++Nmb[num];

            //     X(count-2) = ABS(X(count))-ABS(X(count-1))+ABS(X(count-2))
            X[count - 2] = B1[count] - B1[count - 3]; // Updating X needs to be done before shift operation below

            shift(B1, count - 1, count, B1); // Get rid of two data points one by one
            shift(B1, count - 2, count, B1); // Delete one point

            shift(X, count, count, X);     // Get rid of two data points one by one
            shift(X, count - 1, count, X); // Delete one point

            count -= 2; // If one cycle is counted, two data points are deleted.
            if (count < 4) {
                break; // When only three data points exists, one cycle cannot be counted.
            }
        }
    }

    ++count;

    // Check the rest of the half cycles every time step
    OneNmb = Nmb; // Array Nmb (Bins) will be used for the next time step later.
                  // OneNmb is used to show the current output only.
}

void ElectricStorage::shift(std::vector<Real64> &A, int const m, int const n, std::vector<Real64> &B)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Y. KyungTae & W. Wang
    //       DATE WRITTEN   July-August, 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Utility subroutine for rainflow cycle counting

    int ShiftNum; // Loop variable

    for (ShiftNum = 1; ShiftNum <= m - 1; ++ShiftNum) {
        B[ShiftNum] = A[ShiftNum];
    }

    for (ShiftNum = m; ShiftNum <= n; ++ShiftNum) {
        B[ShiftNum] = A[ShiftNum + 1];
    }
}

// constructor
ElectricTransformer::ElectricTransformer(EnergyPlusData &state, std::string const &objectName)
    : myOneTimeFlag_(true), usageMode_(TransformerUse::Invalid), heatLossesDestination_(ThermalLossDestination::Invalid), zoneNum_(0),
      zoneRadFrac_(0.0), ratedCapacity_(0.0), factorTempCoeff_(0.0), tempRise_(0.0), eddyFrac_(0.0),
      performanceInputMode_(TransformerPerformanceInput::Invalid), ratedEfficiency_(0.0), ratedPUL_(0.0), ratedTemp_(0.0), maxPUL_(0.0),
      considerLosses_(true), ratedNL_(0.0), ratedLL_(0.0), overloadErrorIndex_(0), efficiency_(0.0), powerIn_(0.0), energyIn_(0.0), powerOut_(0.0),
      energyOut_(0.0), noLoadLossRate_(0.0), noLoadLossEnergy_(0.0), loadLossRate_(0.0), loadLossEnergy_(0.0), thermalLossRate_(0.0),
      thermalLossEnergy_(0.0), elecUseMeteredUtilityLosses_(0.0), powerConversionMeteredLosses_(0.0), qdotConvZone_(0.0), qdotRadZone_(0.0)
{
    static constexpr std::string_view routineName = "ElectricTransformer constructor ";
    auto &s_ipsc = state.dataIPShortCut;

    bool errorsFound = false;
    int transformerIDFObjectNum = 0;
    s_ipsc->cCurrentModuleObject = "ElectricLoadCenter:Transformer";

    transformerIDFObjectNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ElectricLoadCenter:Transformer", objectName);
    if (transformerIDFObjectNum > 0) {
        int numAlphas; // Number of elements in the alpha array
        int numNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 transformerIDFObjectNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 numAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 numNums,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
        name_ = s_ipsc->cAlphaArgs(1);
        // how to verify names are unique across objects? add to GlobalNames?
        if (s_ipsc->lAlphaFieldBlanks(2)) {
            availSched_ = Sched::GetScheduleAlwaysOn(state);
        } else if ((availSched_ = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            errorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(3)) {
            usageMode_ = TransformerUse::PowerInFromGrid; // default
        } else if ((usageMode_ = static_cast<TransformerUse>(getEnumValue(transformerUseNamesUC, s_ipsc->cAlphaArgs(3)))) ==
                   TransformerUse::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
            errorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(4)) {
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
        } else if ((zoneNum_ = Util::FindItemInList(s_ipsc->cAlphaArgs(4), state.dataHeatBal->Zone)) == 0) {
            heatLossesDestination_ = ThermalLossDestination::LostToOutside;
            ShowWarningItemNotFound(
                state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4), "Transformer heat losses will not be added to a zone");
            // continue with simulation but storage losses not sent to a zone.
        } else {
            heatLossesDestination_ = ThermalLossDestination::ZoneGains;
        }

        zoneRadFrac_ = s_ipsc->rNumericArgs(1);
        ratedCapacity_ = s_ipsc->rNumericArgs(2);
        // unused phase_ = s_ipsc->rNumericArgs(3);

        if (Util::SameString(s_ipsc->cAlphaArgs(5), "Copper")) {
            factorTempCoeff_ = 234.5;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(5), "Aluminum")) {
            factorTempCoeff_ = 225.0;
        } else {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
            errorsFound = true;
        }
        tempRise_ = s_ipsc->rNumericArgs(4);
        eddyFrac_ = s_ipsc->rNumericArgs(5);

        performanceInputMode_ = static_cast<TransformerPerformanceInput>(getEnumValue(transformerPerformanceInputNamesUC, s_ipsc->cAlphaArgs(6)));
        if (performanceInputMode_ == TransformerPerformanceInput::Invalid) {
            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
            errorsFound = true;
        }

        if (ratedCapacity_ == 0) {
            if (performanceInputMode_ == TransformerPerformanceInput::LossesMethod) {
                ShowWarningError(state, format("{}{}=\"{}\".", routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Specified {} = {}", s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6)));
                ShowContinueError(state, format("Specified {} = {:.1R}", s_ipsc->cNumericFieldNames(2), ratedCapacity_));
                ShowContinueError(state, "Transformer load and no load losses cannot be calculated with 0.0 rated capacity.");
                ShowContinueError(state, "Simulation continues but transformer losses will be set to zero.");
            }
        }
        ratedNL_ = s_ipsc->rNumericArgs(6);
        ratedLL_ = s_ipsc->rNumericArgs(7);
        ratedEfficiency_ = s_ipsc->rNumericArgs(8);
        ratedPUL_ = s_ipsc->rNumericArgs(9);
        ratedTemp_ = s_ipsc->rNumericArgs(10);
        maxPUL_ = s_ipsc->rNumericArgs(11);
        // Check the input for MaxPUL if the performance input method is EfficiencyMethod
        if (performanceInputMode_ == TransformerPerformanceInput::EfficiencyMethod) {
            if (s_ipsc->lNumericFieldBlanks(11)) {
                maxPUL_ = ratedPUL_;
            } else if (maxPUL_ <= 0 || maxPUL_ > 1) {
                ShowSevereError(state, format("{}{}=\"{}\", invalid entry.", routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, format("Invalid {}=[{:.3R}].", s_ipsc->cNumericFieldNames(11), s_ipsc->rNumericArgs(11)));
                ShowContinueError(state, "Entered value must be > 0 and <= 1.");
                errorsFound = true;
            }
        }

        BooleanSwitch bs = getYesNoValue(s_ipsc->cAlphaArgs(7));
        if (bs != BooleanSwitch::Invalid) {
            considerLosses_ = static_cast<bool>(bs);
        } else if (usageMode_ == TransformerUse::PowerInFromGrid) {
            ShowSevereInvalidBool(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
            errorsFound = true;
        }

        int numAlphaBeforeMeter = 7;
        int numWiredMeters = numAlphas - numAlphaBeforeMeter;

        if (usageMode_ == TransformerUse::PowerInFromGrid) {

            // Provide warning if no meter is wired to a transformer used to get power from the grid
            if (numWiredMeters <= 0) {
                ShowWarningError(state, format("{}ElectricLoadCenter:Transformer=\"{}\":", routineName, name_));
                ShowContinueError(state, "ISOLATED Transformer: No meter wired to a transformer used to input power from grid");
            }

            wiredMeterNames_.resize(numWiredMeters, "");
            wiredMeterPtrs_.resize(numWiredMeters, 0);
            specialMeter_.resize(numWiredMeters, false);

            // Meter check deferred because they may have not been "loaded" yet,
            for (int loopCount = 0; loopCount < numWiredMeters; ++loopCount) {
                wiredMeterNames_[loopCount] = Util::makeUPPER(s_ipsc->cAlphaArgs(loopCount + numAlphaBeforeMeter + 1));
                // Assign SpecialMeter as TRUE if the meter name is Electricity:Facility or Electricity:HVAC
                if (Util::SameString(wiredMeterNames_[loopCount], "Electricity:Facility") ||
                    Util::SameString(wiredMeterNames_[loopCount], "Electricity:HVAC")) {
                    specialMeter_[loopCount] = true;
                } else {
                    specialMeter_[loopCount] = false;
                }
            }
        }
        SetupOutputVariable(state,
                            "Transformer Efficiency",
                            Constant::Units::None,
                            efficiency_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Input Electricity Rate",
                            Constant::Units::W,
                            powerIn_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Input Electricity Energy",
                            Constant::Units::J,
                            energyIn_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Output Electricity Rate",
                            Constant::Units::W,
                            powerOut_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Output Electricity Energy",
                            Constant::Units::J,
                            energyOut_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Transformer No Load Loss Rate",
                            Constant::Units::W,
                            noLoadLossRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Transformer No Load Loss Energy",
                            Constant::Units::J,
                            noLoadLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Load Loss Rate",
                            Constant::Units::W,
                            loadLossRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Load Loss Energy",
                            Constant::Units::J,
                            loadLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Thermal Loss Rate",
                            Constant::Units::W,
                            thermalLossRate_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name_);
        SetupOutputVariable(state,
                            "Transformer Thermal Loss Energy",
                            Constant::Units::J,
                            thermalLossEnergy_,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            name_);
        if (usageMode_ == TransformerUse::PowerInFromGrid) { // power losses metered as an end use exterior equipment
            SetupOutputVariable(state,
                                "Transformer Distribution Electricity Loss Energy",
                                Constant::Units::J,
                                elecUseMeteredUtilityLosses_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                name_,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC, // Is this correct?
                                OutputProcessor::EndUseCat::ExteriorEquipment,
                                "Transformer");
        }
        if (usageMode_ == TransformerUse::PowerOutFromBldgToGrid) {
            SetupOutputVariable(state,
                                "Transformer Cogeneration Electricity Loss Energy",
                                Constant::Units::J,
                                powerConversionMeteredLosses_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                name_,
                                Constant::eResource::ElectricityProduced,
                                OutputProcessor::Group::HVAC, // Is this correct?
                                OutputProcessor::EndUseCat::PowerConversion);
        }
        if (usageMode_ == TransformerUse::PowerBetweenLoadCenterAndBldg) {
            SetupOutputVariable(state,
                                "Transformer Conversion Electricity Loss Energy",
                                Constant::Units::J,
                                powerConversionMeteredLosses_,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                name_,
                                Constant::eResource::ElectricityProduced,
                                OutputProcessor::Group::HVAC, // Is this correct?
                                OutputProcessor::EndUseCat::PowerConversion);
        }

        if (zoneNum_ > 0) {
            SetupZoneInternalGain(
                state, zoneNum_, name_, DataHeatBalance::IntGainType::ElectricLoadCenterTransformer, &qdotConvZone_, nullptr, &qdotRadZone_);
        }

    } else {
        ShowSevereError(state, format("{} did not find transformer name = {}", routineName, objectName));
        errorsFound = true;
    }

    if (errorsFound) {
        ShowFatalError(state, format("{}Preceding errors terminate program.", routineName));
    }
}

Real64 ElectricTransformer::getLossRateForOutputPower(EnergyPlusData &state, Real64 const powerOutOfTransformer)
{
    manageTransformers(state, powerOutOfTransformer);
    return totalLossRate_;
}

Real64 ElectricTransformer::getLossRateForInputPower(EnergyPlusData &state, Real64 const powerIntoTransformer)
{
    manageTransformers(state, powerIntoTransformer);
    return totalLossRate_;
}

void ElectricTransformer::manageTransformers(EnergyPlusData &state, Real64 const surplusPowerOutFromLoadCenters)
{
    Real64 constexpr ambTempRef = 20.0; // reference ambient temperature (C)
    if (myOneTimeFlag_) {
        // calculate rated no load losses and rated load losses if the performance input method is based on
        // nominal efficiency. This calculation is done only once

        if (performanceInputMode_ == TransformerPerformanceInput::EfficiencyMethod) {

            Real64 resRef = factorTempCoeff_ + tempRise_ + ambTempRef;
            Real64 resSpecified = factorTempCoeff_ + ratedTemp_;
            Real64 resRatio = resSpecified / resRef;
            Real64 factorTempCorr = (1.0 - eddyFrac_) * resRatio + eddyFrac_ * (1.0 / resRatio);
            Real64 numerator = ratedCapacity_ * ratedPUL_ * (1.0 - ratedEfficiency_);
            Real64 denominator = ratedEfficiency_ * (1.0 + pow_2(ratedPUL_ / maxPUL_));

            ratedNL_ = numerator / denominator;
            ratedLL_ = ratedNL_ / (factorTempCorr * pow_2(maxPUL_));
        }
        myOneTimeFlag_ = false;
    }

    Real64 elecLoad = 0.0;     // transformer load which may be power in or out depending on the usage mode
    Real64 pastElecLoad = 0.0; // transformer load at the previous timestep
    switch (usageMode_) {
    case TransformerUse::PowerInFromGrid: {
        for (std::size_t meterNum = 0; meterNum < wiredMeterPtrs_.size(); ++meterNum) {

            if (state.dataGlobal->MetersHaveBeenInitialized) {

                elecLoad +=
                    GetInstantMeterValue(state, wiredMeterPtrs_[meterNum], OutputProcessor::TimeStepType::Zone) / state.dataGlobal->TimeStepZoneSec +
                    GetInstantMeterValue(state, wiredMeterPtrs_[meterNum], OutputProcessor::TimeStepType::System) /
                        (state.dataHVACGlobal->TimeStepSysSec);
                // PastElecLoad store the metered value in the previous time step. This value will be used to check whether
                // a transformer is overloaded or not.
                pastElecLoad += GetCurrentMeterValue(state, wiredMeterPtrs_[meterNum]) / state.dataGlobal->TimeStepZoneSec;
            } else {
                elecLoad = 0.0;
                pastElecLoad = 0.0;
            }

            // Because transformer loss has been accounted for by Electricity:Facility and Electricity:HVAC, the transformer
            // loss needs to be deducted from the metered value. Otherwise, double counting (circular relationship) occurs.
            if (specialMeter_[meterNum]) {
                elecLoad = elecLoad - loadLossRate_ - noLoadLossRate_;

                if (elecLoad < 0) {
                    elecLoad = 0.0; // Essential check.
                }
            }
        }

        powerOut_ = elecLoad; // the metered value is transformer's output in PowerInFromGrid mode
        break;
    }
    case TransformerUse::PowerOutFromBldgToGrid: {
        powerIn_ = surplusPowerOutFromLoadCenters;
        elecLoad = surplusPowerOutFromLoadCenters; // TODO this is input but should be output with the losses, but we don't have them yet.
        break;
    }
    case TransformerUse::PowerBetweenLoadCenterAndBldg: {
        // TODO, new configuration for transformer, really part of the specific load center and connects it to the main building bus
        powerIn_ = surplusPowerOutFromLoadCenters;
        elecLoad = surplusPowerOutFromLoadCenters;
        break;
    }
    case TransformerUse::Invalid: {
        // do nothing
        break;
    }
    default:
        assert(false);
    } // switch usage mode

    // check availability schedule
    if (ratedCapacity_ > 0.0 && availSched_->getCurrentVal() > 0.0) {

        Real64 pUL = elecLoad / ratedCapacity_;

        if (pUL > 1.0) {
            pUL = 1.0;
        }

        // Originally, PUL was used to check whether a transformer is overloaded (PUL > 1.0 or not). However, it was
        // found that ElecLoad obtained from GetInstantMeterVlaue() might refer to intermideiate values before
        // convergence. The intermediate values may issue false warning. This the reason why PastElecLoad obtained
        // by GetCurrentMeterValue() is used here to check overload issue.
        if ((pastElecLoad / ratedCapacity_) > 1.0) {
            if (overloadErrorIndex_ == 0) {
                ShowSevereError(state, "Transformer Overloaded");
                ShowContinueError(state, format("Entered in ElectricLoadCenter:Transformer ={}", name_));
            }
            ShowRecurringSevereErrorAtEnd(state, "Transformer Overloaded: Entered in ElectricLoadCenter:Transformer =" + name_, overloadErrorIndex_);
        }

        Real64 tempChange = std::pow(pUL, 1.6) * tempRise_;
        Real64 ambTemp = 20.0;
        if (heatLossesDestination_ == ThermalLossDestination::ZoneGains) {

            ambTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum_).MAT;
        } else {
            ambTemp = 20.0;
        }

        Real64 resRef = factorTempCoeff_ + tempRise_ + ambTempRef;
        Real64 resSpecified = factorTempCoeff_ + tempChange + ambTemp;
        Real64 resRatio = resSpecified / resRef;
        Real64 factorTempCorr = (1.0 - eddyFrac_) * resRatio + eddyFrac_ * (1.0 / resRatio);

        loadLossRate_ = ratedLL_ * pow_2(pUL) * factorTempCorr;
        noLoadLossRate_ = ratedNL_;
    } else { // Transformer is not available.
        loadLossRate_ = 0.0;
        noLoadLossRate_ = 0.0;
    }

    totalLossRate_ = loadLossRate_ + noLoadLossRate_;

    switch (usageMode_) {
    case TransformerUse::PowerInFromGrid: {
        powerIn_ = elecLoad + totalLossRate_;

        // Transformer losses are wired to the meter via the variable "%ElecUseUtility" only if transformer losses
        // are considered in utility cost. If transformer losses are not considered in utility cost, 0 is assigned
        // to the variable "%ElecUseUtility".
        if (considerLosses_) {
            elecUseMeteredUtilityLosses_ = totalLossRate_ * state.dataHVACGlobal->TimeStepSysSec;
        } else {
            elecUseMeteredUtilityLosses_ = 0.0;
        }

        // Transformer has two modes.If it works in one mode, the variable for meter output in the other mode
        // is assigned 0
        // unused totalLossEnergy_ = totalLossRate_ * state.dataHVACGlobal->TimeStepSysSec;

        break;
    }

    case TransformerUse::PowerOutFromBldgToGrid:
    case TransformerUse::PowerBetweenLoadCenterAndBldg: {
        powerOut_ = elecLoad - totalLossRate_;

        if (powerOut_ < 0) {
            powerOut_ = 0.0;
        }

        powerConversionMeteredLosses_ = -1.0 * totalLossRate_ * state.dataHVACGlobal->TimeStepSysSec;

        // Transformer has two modes.If it works in one mode, the variable for meter output in the other mode
        // is assigned 0
        elecUseMeteredUtilityLosses_ = 0.0;
        break;
    }

    case TransformerUse::Invalid: {
        // do nothing
        assert(false);
    }
    default:
        assert(false);
    } // switch

    if (powerIn_ <= 0) {
        efficiency_ = 1.0; // Set to something reasonable to avoid a divide by zero error
    } else {
        efficiency_ = powerOut_ / powerIn_;
    }
    noLoadLossEnergy_ = noLoadLossRate_ * state.dataHVACGlobal->TimeStepSysSec;
    loadLossEnergy_ = loadLossRate_ * state.dataHVACGlobal->TimeStepSysSec;

    energyIn_ = powerIn_ * state.dataHVACGlobal->TimeStepSysSec;
    energyOut_ = powerOut_ * state.dataHVACGlobal->TimeStepSysSec;

    //   Thermal loss rate may not be equal to Total loss rate. This is the case when surplus power is less than the
    //    calculated total loss rate for a cogeneration transformer. That is why "PowerIn - PowerOut" is used below.
    thermalLossRate_ = powerIn_ - powerOut_;
    thermalLossEnergy_ = thermalLossRate_ * state.dataHVACGlobal->TimeStepSysSec;

    if (zoneNum_ > 0) { // set values for zone heat gains
        qdotConvZone_ = (1.0 - zoneRadFrac_) * thermalLossRate_;
        qdotRadZone_ = (zoneRadFrac_)*thermalLossRate_;
    }
}

void ElectricTransformer::setupMeterIndices(EnergyPlusData &state)
{
    if (usageMode_ == TransformerUse::PowerInFromGrid) {
        for (std::size_t meterNum = 0; meterNum < wiredMeterNames_.size(); ++meterNum) {

            wiredMeterPtrs_[meterNum] = GetMeterIndex(state, wiredMeterNames_[meterNum]);

            // Check whether the meter is an electricity meter
            // Index function is used here because some resource types are not Electricity but strings containing
            // Electricity such as ElectricityPurchased and ElectricityProduced.
            // It is not proper to have this check in GetInput routine because the meter index may have not been defined
            auto *meter = state.dataOutputProcessor->meters[wiredMeterPtrs_[meterNum]];
            if (meter->resource != Constant::eResource::Electricity && meter->resource != Constant::eResource::ElectricityPurchased &&
                meter->resource != Constant::eResource::ElectricitySurplusSold && meter->resource != Constant::eResource::ElectricityProduced &&
                meter->resource != Constant::eResource::ElectricityNet) {
                ShowFatalError(state, format("Non-electricity meter used for {}", name_));
            }
        }
    }
}

void ElectricTransformer::reinitAtBeginEnvironment()
{
    efficiency_ = 0.0;
    powerIn_ = 0.0;
    energyIn_ = 0.0;
    powerOut_ = 0.0;
    energyOut_ = 0.0;
    noLoadLossRate_ = 0.0;
    noLoadLossEnergy_ = 0.0;
    loadLossRate_ = 0.0;
    loadLossEnergy_ = 0.0;
    thermalLossRate_ = 0.0;
    thermalLossEnergy_ = 0.0;
    elecUseMeteredUtilityLosses_ = 0.0;
    powerConversionMeteredLosses_ = 0.0;
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

void ElectricTransformer::reinitZoneGainsAtBeginEnvironment()
{
    qdotConvZone_ = 0.0;
    qdotRadZone_ = 0.0;
}

std::string const &ElectricTransformer::name() const
{
    return name_;
}

} // namespace EnergyPlus
