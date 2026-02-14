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

// PCM Thermal Storage Module - PCMThermalStorage.cc

// C++ Headers
#include <algorithm>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PCMThermalStorage.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {
namespace PCMStorage {

    PlantComponent *PCMStorageData::factory(EnergyPlusData &state, std::string const &objectName)
    {
        static bool getPCMInputFlag = true;
        if (getPCMInputFlag) {
            PCMStorage::GetPCMStorageInput(state);
            getPCMInputFlag = false;
        }

        auto &pcm = PCMStorageData::instance();
        if (pcm.Name == objectName) {
            return static_cast<PlantComponent *>(&pcm);
        }

        ShowFatalError(state, "PCMStorage factory: No PCM storage found with name: " + objectName);
        return nullptr;
    }

    void SimulatePCMStorage(EnergyPlusData &state, PlantLocation const &plantLoc, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
    {
        // Silence only the truly unused parameters
        (void)FirstHVACIteration;
        (void)CurLoad;
        (void)RunFlag;
        auto &PCM = PCMStorageData::instance();

        if (!PCM.Initialized) {
            PCM.Init(state);
        }

        PCM.Calculate(state, plantLoc);
    }

    void
    PCMStorageData::simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
    {
        SimulatePCMStorage(state, calledFromLocation, FirstHVACIteration, CurLoad, RunFlag);
    }

    void PCMStorageData::oneTimeInit(EnergyPlusData &state)
    {
        this->Init(state);
    }

    void PCMStorageData::Init(EnergyPlusData &state)
    {
        // Run once to find this component in the plant loops
        if (this->MyPlantScanFlag) {
            bool errFlag = false;

            PlantUtilities::ScanPlantLoopsForObject(
                state, this->Name, DataPlant::PlantEquipmentType::TS_PCM, this->usePlantLoc, errFlag, _, _, _, UseSideInletNode, _, _);

            PlantUtilities::ScanPlantLoopsForObject(
                state, this->Name, DataPlant::PlantEquipmentType::TS_PCM, this->sourcePlantLoc, errFlag, _, _, _, PlantSideInletNode, _, _);

            if (errFlag) {
                ShowFatalError(state, "PCMStorageData::Init: Error scanning plant loops for PCM tank named: " + this->Name);
            }

            EnergyPlus::PCMStorage::RegisterPCMStorageOutputVariables(state);
            this->MyPlantScanFlag = false;
        }
        Real64 temp = state.dataLoopNodes->Node(this->UseSideInletNode).Temp;

        // get the water fluid property object
        EnergyPlus::Fluid::GlycolProps *rhoUseProps = this->usePlantLoc.loop->glycol;
        double rhoUse = rhoUseProps->getDensity(state, temp, "PCMStorageData::Calculate");

        EnergyPlus::Fluid::GlycolProps *rhoPlantProps = this->sourcePlantLoc.loop->glycol;
        double rhoPlant = rhoPlantProps->getDensity(state, temp, "PCMStorageData::Calculate");

        // At the beginning of each new environment (e.g. design day) perform one-time initializations.
        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {
            // Determine design flow rates when the user has requested autosizing (indicated by a value <= 0).
            // If the design flow rates are already positive the code will use them as-is.  Otherwise it will
            // attempt to infer a reasonable flow from the connected plant loop or use a small default value.
            if (this->UseSideDesignFlowRate <= 0.0) {
                // Try to use the current inlet mass flow rate from the plant loop as a starting point.
                Real64 initMassFlow = 0.0;
                if (this->UseSideInletNode > 0) {
                    initMassFlow = state.dataLoopNodes->Node(this->UseSideInletNode).MassFlowRate;
                }
                if (initMassFlow > 0.0) {
                    this->UseSideDesignFlowRate = initMassFlow / rhoUse;
                } else if (this->PlantSideDesignFlowRate > 0.0) {
                    this->UseSideDesignFlowRate = this->PlantSideDesignFlowRate;
                } else {
                    // Fall back to a nominal small flow of 1.0e-4 m3/s if no information is available.
                    this->UseSideDesignFlowRate = 1.0e-4;
                }
            }
            if (this->PlantSideDesignFlowRate <= 0.0) {
                Real64 initMassFlow = 0.0;
                if (this->PlantSideInletNode > 0) {
                    initMassFlow = state.dataLoopNodes->Node(this->PlantSideInletNode).MassFlowRate;
                }
                if (initMassFlow > 0.0) {
                    this->PlantSideDesignFlowRate = initMassFlow / rhoPlant;
                } else {
                    // If nothing else is known use the use side design flow rate as the plant side design flow rate.
                    this->PlantSideDesignFlowRate = this->UseSideDesignFlowRate;
                }
            }
            // Convert design volumetric flow rates (m3/s) to mass flow rates (kg/s).
            this->UseSideMassFlowRate = this->UseSideDesignFlowRate * rhoUse;
            this->PlantSideMassFlowRate = this->PlantSideDesignFlowRate * rhoPlant;

            // If the tank capacity has been set to zero or negative (autosize request), estimate a capacity.
            // The estimate here is based on storing one hour of flow on the larger of the use side or plant side.
            // This provides a reasonable latent mass storage size without relying on external sizing objects.
            if (this->TankCapacity <= 0.0) {
                Real64 designMassFlow = std::max(this->UseSideMassFlowRate, this->PlantSideMassFlowRate);
                // Store one hour (3600 s) of flow.
                this->TankCapacity = designMassFlow * 3600.0;
            }

            // Initialize the component nodes with the calculated maximum mass flow limits.
            PlantUtilities::InitComponentNodes(state, 0.0, this->UseSideMassFlowRate, this->UseSideInletNode, this->UseSideOutletNode);
            PlantUtilities::InitComponentNodes(state, 0.0, this->PlantSideMassFlowRate, this->PlantSideInletNode, this->PlantSideOutletNode);

            // Set the component flow priorities so that the plant loop will attempt to deliver the required flow.
            for (int compNum = 1; compNum <= state.dataPlnt->PlantLoop(this->usePlantLoc.loopNum)
                                                 .LoopSide(this->usePlantLoc.loopSideNum)
                                                 .Branch(this->usePlantLoc.branchNum)
                                                 .TotalComponents;
                 ++compNum) {
                state.dataPlnt->PlantLoop(this->usePlantLoc.loopNum)
                    .LoopSide(this->usePlantLoc.loopSideNum)
                    .Branch(this->usePlantLoc.branchNum)
                    .Comp(compNum)
                    .FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
            }
            for (int compNum = 1; compNum <= state.dataPlnt->PlantLoop(this->sourcePlantLoc.loopNum)
                                                 .LoopSide(this->sourcePlantLoc.loopSideNum)
                                                 .Branch(this->sourcePlantLoc.branchNum)
                                                 .TotalComponents;
                 ++compNum) {
                state.dataPlnt->PlantLoop(this->sourcePlantLoc.loopNum)
                    .LoopSide(this->sourcePlantLoc.loopSideNum)
                    .Branch(this->sourcePlantLoc.branchNum)
                    .Comp(compNum)
                    .FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
            }

            // Reset state variables for the start of the new environment.
            this->EnergyStored = this->TankCapacity * this->LatentHeat;
            this->PercentCapacity = 100.0;

            // Flag that the initialization for this environment is complete.
            this->MyEnvrnFlag = false;
        }

        // When BeginEnvrnFlag is reset at the end of a sizing period, allow the initialization to occur again for the next environment.
        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        // Real64 avail = this->AvailabilitySchedule->getCurrentVal();
        //  Establish component flow requests on each side of the storage tank.  Use the
        //  design mass flow rates computed above and apply availability scheduling.  Note
        //  that SetComponentFlowRate expects a mass flow (kg/s), not a volumetric flow.  The
        //  previous implementation passed the design volumetric flow rate directly which
        //  resulted in very small mass flows and in some cases zero flow on the use side.  Here
        //  we convert to mass flow and schedule it off when the availability schedule is zero.
    }

    void PCMStorageData::Calculate(EnergyPlusData &state, PlantLocation const &plantLoc)
    {
        // plantLoc is passed in but not referenced in the body → silence here
        (void)plantLoc;
        auto &useInlet = state.dataLoopNodes->Node(UseSideInletNode);
        auto &useOutlet = state.dataLoopNodes->Node(UseSideOutletNode);
        auto &plantInlet = state.dataLoopNodes->Node(PlantSideInletNode);
        auto &plantOutlet = state.dataLoopNodes->Node(PlantSideOutletNode);

        // Real64 avail = this->AvailabilitySchedule->getCurrentVal();
        Real64 dt_seconds = state.dataHVACGlobal->TimeStepSys * 3600.0;

        Real64 temp = state.dataLoopNodes->Node(this->UseSideInletNode).Temp;

        EnergyPlus::Fluid::GlycolProps *cpUseProps = this->usePlantLoc.loop->glycol;
        Real64 CpWaterUse = cpUseProps->getSpecificHeat(state, temp, "PCMStorageData::Calculate"); // J/kg-C

        EnergyPlus::Fluid::GlycolProps *cpPlantProps = this->sourcePlantLoc.loop->glycol;
        Real64 CpWaterPlant = cpPlantProps->getSpecificHeat(state, temp, "PCMStorageData::Calculate"); // J/kg-C
        // Real64 massFlowUse = useInlet.MassFlowRate;
        // Real64 massFlowPlant = plantInlet.MassFlowRate;

        Real64 plantOutletTemp = plantInlet.Temp - (Effectiveness * (plantInlet.Temp - FreezingTemp)); // Calculate Plant Outlet Temperature
        Real64 useOutletTemp = useInlet.Temp + (Effectiveness * (MeltingTemp - useInlet.Temp));        // Calculate Use Outlet Temperature

        // Real64 deltaTUse = useInlet.Temp - useOutletTemp;       // Heat to Water Heater
        // Real64 deltaTPlant = plantInlet.Temp - plantOutletTemp; // Heat to PCM Tank
        PlantUtilities::SafeCopyPlantNode(state, this->UseSideInletNode, this->UseSideOutletNode);
        PlantUtilities::SafeCopyPlantNode(state, this->PlantSideInletNode, this->PlantSideOutletNode);

        if (this->AvailabilitySchedule->getCurrentVal() <= 0.0) {
            useInlet.MassFlowRate = 0.0;
            useOutlet.MassFlowRate = 0.0;
            plantInlet.MassFlowRate = 0.0;
            plantOutlet.MassFlowRate = 0.0;
            return;
        }

        // Real64 useheatTransfer = massFlowUse * CpWater * deltaTUse;       // Heat to Water Heater
        // Real64 plantheatTransfer = massFlowPlant * CpWater * deltaTPlant; // Heat to PCM Tank

        // Calculate tank temperature from stored energy
        if (this->PCMmat != nullptr) {
            Real64 targetEnthalpy = EnergyStored / TankCapacity;
            Real64 Tlow = this->PCMmat->peakTempMelting - 30.0;
            Real64 Thigh = this->PCMmat->peakTempMelting + 30.0;
            Real64 approxTemp = this->PCMmat->peakTempMelting;

            for (int i = 0; i < 25; ++i) {
                Real64 Tmid = 0.5 * (Tlow + Thigh);
                Real64 hMid = this->PCMmat->getEnthalpy(
                    Tmid, this->PCMmat->peakTempMelting, this->PCMmat->deltaTempMeltingLow, this->PCMmat->deltaTempMeltingHigh);
                if (std::abs(hMid - targetEnthalpy) < 0.1) {
                    approxTemp = Tmid;
                    break;
                }
                if (hMid > targetEnthalpy) {
                    Thigh = Tmid;
                } else {
                    Tlow = Tmid;
                }
            }

            this->PCM_TankTemp = approxTemp;
        }

        // Determine operation mode.  The sign of the heat transfer on each side dictates whether the unit should
        // charge or discharge.  In the original implementation a second condition on the tank temperature
        // prevented charging unless the PCM temperature was below the melting point, which caused the plant
        // side flow to be ignored even when the PCM should have been storing energy.  Here we rely solely
        // on the direction of heat transfer to set the mode.
        // SOC as fraction 0–1
        Real64 soc = EnergyStored / (TankCapacity * LatentHeat);
        soc = std::clamp(soc, 0.0, 1.0);

        // Hysteresis thresholds (tune as needed)
        constexpr Real64 cutInSOC = 0.40;  // start charging when SOC <= 40%
        constexpr Real64 cutOutSOC = 0.95; // stop charging when SOC >= 95%

        // Persistent mode
        static bool chargingMode = false;
        if (soc <= cutInSOC) {
            chargingMode = true;
        }
        if (soc >= cutOutSOC) {
            chargingMode = false;
        }

        // Decide flows: charge => plant flows only; discharge => use flows only
        Real64 mUseReq = 0.0;
        Real64 mPlantReq = 0.0;

        if (chargingMode) {
            mPlantReq = this->PlantSideMassFlowRate;
        } else {
            mUseReq = this->UseSideMassFlowRate;
        }

        // Issue flow requests (mass flow, kg/s)
        PlantUtilities::SetComponentFlowRate(state, mUseReq, this->UseSideInletNode, this->UseSideOutletNode, this->usePlantLoc);
        PlantUtilities::SetComponentFlowRate(state, mPlantReq, this->PlantSideInletNode, this->PlantSideOutletNode, this->sourcePlantLoc);

        // Update node temps for the side that actually flowed
        if (mUseReq > 0.0) {
            useOutlet.Temp = useOutletTemp;
        } else if (mPlantReq > 0.0) {
            plantOutlet.Temp = std::min(plantOutletTemp, state.dataPlnt->PlantLoop[this->sourcePlantLoc.loopNum].MaxTemp);
        }

        // Recompute heat-transfer rates using the requested flows (W)
        // (deltaTUse/deltaTPlant were computed above; keep your existing outlet temp calcs)
        Real64 useheatTransfer_req = mUseReq * CpWaterUse * (useInlet.Temp - useOutletTemp);
        Real64 plantheatTransfer_req = mPlantReq * CpWaterPlant * (plantInlet.Temp - plantOutletTemp);

        // Only one side should contribute per timestep by construction; but compute net formally:
        Real64 netPowerW = plantheatTransfer_req + useheatTransfer_req - HeatLossRate;

        // Update stored energy (J)
        EnergyStored += netPowerW * dt_seconds;

        // Persist rates for reporting
        this->useheatTransfer = useheatTransfer_req;
        this->plantheatTransfer = plantheatTransfer_req;

        // Enforce bounds and update percent capacity (0–100)
        EnergyStored = std::clamp(EnergyStored, 0.0, TankCapacity * LatentHeat);
        PercentCapacity = 100.0 * EnergyStored / (TankCapacity * LatentHeat);
    }

    void RegisterPCMStorageOutputVariables(EnergyPlusData &state)
    {
        EnergyPlus::PCMStorage::PCMStorageData &PCM = EnergyPlus::PCMStorage::PCMStorageData::instance();

        SetupOutputVariable(state,
                            "Thermal Energy Storage Percent Capacity",
                            Constant::Units::None,
                            PCM.PercentCapacity,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Heat Loss Rate",
                            Constant::Units::W,
                            PCM.HeatLossRate,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Energy Stored",
                            Constant::Units::J,
                            PCM.EnergyStored,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Tank Temperature",
                            Constant::Units::C,
                            PCM.PCM_TankTemp,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Use Side Heat Transfer Rate",
                            Constant::Units::W,
                            PCM.useheatTransfer,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Plant Side Heat Transfer Rate",
                            Constant::Units::W,
                            PCM.plantheatTransfer,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Use Side Inlet Temperature",
                            Constant::Units::C,
                            state.dataLoopNodes->Node(PCM.UseSideInletNode).Temp,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Use Side Outlet Temperature",
                            Constant::Units::C,
                            state.dataLoopNodes->Node(PCM.UseSideOutletNode).Temp,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Use Side Mass Flow Rate",
                            Constant::Units::kg_s,
                            state.dataLoopNodes->Node(PCM.UseSideInletNode).MassFlowRate,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Plant Side Inlet Temperature",
                            Constant::Units::C,
                            state.dataLoopNodes->Node(PCM.PlantSideInletNode).Temp,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Plant Side Outlet Temperature",
                            Constant::Units::C,
                            state.dataLoopNodes->Node(PCM.PlantSideOutletNode).Temp,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);

        SetupOutputVariable(state,
                            "Thermal Energy Storage Plant Side Mass Flow Rate",
                            Constant::Units::kg_s,
                            state.dataLoopNodes->Node(PCM.PlantSideInletNode).MassFlowRate,
                            EnergyPlus::OutputProcessor::TimeStepType::System,
                            EnergyPlus::OutputProcessor::StoreType::Average,
                            PCM.Name);
    }

    EnergyPlus::PCMStorage::PCMStorageData &EnergyPlus::PCMStorage::PCMStorageData::instance()
    {
        static EnergyPlus::PCMStorage::PCMStorageData inst;
        return inst;
    }

    void GetPCMStorageInput(EnergyPlusData &state)
    {
        constexpr std::string_view RoutineName = "GetPCMStorageInput";

        auto &PCM = EnergyPlus::PCMStorage::PCMStorageData::instance();
        bool ErrorsFound = false;

        int NumPCMObjs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ThermalStorage:PCM");
        if (NumPCMObjs != 1) {
            ShowSevereError(state, "Exactly one ThermalStorage:PCM object is required.");
            ErrorsFound = true;
            return;
        }

        state.dataIPShortCut->cCurrentModuleObject = "ThermalStorage:PCM";

        int NumAlphas;
        int NumNums;
        int IOStat;

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 state.dataIPShortCut->cCurrentModuleObject,
                                                                 1,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);

        PCM.Name = state.dataIPShortCut->cAlphaArgs(1);
        PCM.AvailabilityScheduleName = state.dataIPShortCut->cAlphaArgs(2);

        ErrorObjectHeader eoh{RoutineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(2)};
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            PCM.AvailabilitySchedule = Sched::GetScheduleAlwaysOn(state);
        } else if ((PCM.AvailabilitySchedule = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        int matNum = Material::GetMaterialNum(state, state.dataIPShortCut->cAlphaArgs(7));
        if (matNum == 0) {
            ShowSevereError(
                state, format("{}: Invalid PCM material name: {}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(7)));
            ErrorsFound = true;
        } else {
            // Obtains conduction FD related parameters from input file
            std::string savedModuleObj = state.dataIPShortCut->cCurrentModuleObject;

            // --- Restore the module name for proper error reporting
            state.dataIPShortCut->cCurrentModuleObject = savedModuleObj;

            auto *mat = state.dataMaterial->materials(matNum);

            if (!mat->hasPCM) {
                ShowSevereError(state,
                                format("{}: Material {} is not a phase change material.", state.dataIPShortCut->cCurrentModuleObject, mat->Name));
                ErrorsFound = true;
            } else {
                PCM.PCMMaterialNum = matNum;
                PCM.PCMmat = dynamic_cast<Material::MaterialPhaseChange *>(mat);

                // Read the numeric fields for tank capacity, heat loss rate and design flow rates.  Allow the
                // fields for tank capacity and flow rates to be autosized by accepting values less than or
                // equal to zero.  When these inputs are autosized the actual values will be determined during
                // the Init() routine based on the connected plant loop flow rates.  Only the heat loss rate
                // is strictly required to be non-negative.
                PCM.TankCapacity = state.dataIPShortCut->rNumericArgs(1);
                PCM.HeatLossRate = state.dataIPShortCut->rNumericArgs(2);
                PCM.UseSideDesignFlowRate = state.dataIPShortCut->rNumericArgs(3);
                PCM.PlantSideDesignFlowRate = state.dataIPShortCut->rNumericArgs(4);

                // Report warnings instead of fatal errors for design flows <= 0.0 indicating an autosize request.
                if (PCM.UseSideDesignFlowRate < 0.0) {
                    ShowWarningError(state,
                                     format("{}={}, Use Side Design Flow Rate was entered as {:.6R}.  This will be autosized during initialization.",
                                            state.dataIPShortCut->cCurrentModuleObject,
                                            PCM.Name,
                                            PCM.UseSideDesignFlowRate));
                }
                if (PCM.PlantSideDesignFlowRate < 0.0) {
                    ShowWarningError(
                        state,
                        format("{}={}, Plant Side Design Flow Rate was entered as {:.6R}.  This will be autosized during initialization.",
                               state.dataIPShortCut->cCurrentModuleObject,
                               PCM.Name,
                               PCM.PlantSideDesignFlowRate));
                }

                PCM.MeltingTemp = PCM.PCMmat->peakTempMelting;
                PCM.FreezingTemp = PCM.PCMmat->peakTempFreezing;
                PCM.LatentHeat = PCM.PCMmat->totalLatentHeat;
                PCM.SpecificHeat = (PCM.PCMmat->specificHeatSolid + PCM.PCMmat->specificHeatLiquid) / 2.0;
            }
        }

        PCM.PlantSideInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                     state.dataIPShortCut->cAlphaArgs(3),
                                                                     ErrorsFound,
                                                                     DataLoopNode::ConnectionObjectType::ThermalStoragePCM,
                                                                     PCM.Name,
                                                                     DataLoopNode::NodeFluidType::Water,
                                                                     DataLoopNode::ConnectionType::Inlet,
                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                     DataLoopNode::ObjectIsNotParent);

        PCM.PlantSideOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                      state.dataIPShortCut->cAlphaArgs(4),
                                                                      ErrorsFound,
                                                                      DataLoopNode::ConnectionObjectType::ThermalStoragePCM,
                                                                      PCM.Name,
                                                                      DataLoopNode::NodeFluidType::Water,
                                                                      DataLoopNode::ConnectionType::Outlet,
                                                                      NodeInputManager::CompFluidStream::Primary,
                                                                      DataLoopNode::ObjectIsNotParent);

        PCM.UseSideInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                   state.dataIPShortCut->cAlphaArgs(5),
                                                                   ErrorsFound,
                                                                   DataLoopNode::ConnectionObjectType::ThermalStoragePCM,
                                                                   PCM.Name,
                                                                   DataLoopNode::NodeFluidType::Water,
                                                                   DataLoopNode::ConnectionType::Inlet,
                                                                   NodeInputManager::CompFluidStream::Secondary,
                                                                   DataLoopNode::ObjectIsNotParent);

        PCM.UseSideOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                    state.dataIPShortCut->cAlphaArgs(6),
                                                                    ErrorsFound,
                                                                    DataLoopNode::ConnectionObjectType::ThermalStoragePCM,
                                                                    PCM.Name,
                                                                    DataLoopNode::NodeFluidType::Water,
                                                                    DataLoopNode::ConnectionType::Outlet,
                                                                    NodeInputManager::CompFluidStream::Secondary,
                                                                    DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           PCM.Name,
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           state.dataIPShortCut->cAlphaArgs(4),
                                           "PCM Storage Plant Side");

        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           PCM.Name,
                                           state.dataIPShortCut->cAlphaArgs(5),
                                           state.dataIPShortCut->cAlphaArgs(6),
                                           "PCM Storage Use Side");

        // Allow the tank capacity to be autosized by entering a value <= 0.  A negative or zero
        // entry signals that the capacity should be determined automatically during initialization.
        // Issue a warning to let the user know that autosizing will occur but do not halt execution.
        if (PCM.TankCapacity <= 0.0) {
            ShowWarningError(state,
                             format("{}={}, Tank Capacity was entered as {:.6R} and will be autosized during initialization.",
                                    state.dataIPShortCut->cCurrentModuleObject,
                                    PCM.Name,
                                    PCM.TankCapacity));
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
        }
    }
} // namespace PCMStorage
} // namespace EnergyPlus
