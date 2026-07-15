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

// C++ Headers
#include <format>

// EnergyPlus Headers
#include <EnergyPlus/Boilers.hh>
#include <EnergyPlus/ChillerAbsorption.hh>
#include <EnergyPlus/ChillerElectricEIR.hh>
#include <EnergyPlus/ChillerIndirectAbsorption.hh>
#include <EnergyPlus/ChillerReformulatedEIR.hh>
#include <EnergyPlus/CondenserLoopTowers.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/PlantChillers.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace FaultsManager {

    // MODULE INFORMATION:
    //       AUTHOR         Tianzhen Hong, LBNL
    //       DATE WRITTEN   August 2013
    //       MODIFIED       Sep. 2013, Xiufeng Pang (XP), LBNL. Added Fouling Coil fault
    //                      Feb. 2015, Rongpeng Zhang, LBNL. Added Thermostat/Humidistat Offset faults
    //                      Apr. 2015, Rongpeng Zhang, LBNL. Added Fouling Air Filter fault
    //                      May. 2016, Rongpeng Zhang, LBNL. Added Chiller/Condenser Supply Water Temperature Sensor fault
    //                      Jun. 2016, Rongpeng Zhang, LBNL. Added Tower Scaling fault
    //                      Jul. 2016, Rongpeng Zhang, LBNL. Added Coil Supply Air Temperature Sensor fault
    //                      Oct. 2016, Rongpeng Zhang, LBNL. Added Fouling Boiler fault
    //                      Nov. 2016, Rongpeng Zhang, LBNL. Added Fouling Chiller fault
    //                      Jan. 2017, Rongpeng Zhang, LBNL. Added Fouling Evaporative Cooler fault

    // PURPOSE OF THIS MODULE:
    // This module manages operational faults of buildings and systems.

    // METHODOLOGY EMPLOYED:
    //  Various methods are employed depending types of faults

    namespace {
        // These were static variables within different functions. They were pulled out into the namespace
        // to facilitate easier unit testing of those functions.
        // These are purposefully not in the header file as an extern variable. No one outside of this module should
        // use these. They are cleared by clear_state() for use by unit tests, but normal simulations should be unaffected.
        // This is purposefully in an anonymous namespace so nothing outside this implementation file can use it.

    } // namespace

    // FaultTypeEnum

    // Types of faults under Group Operational Faults in IDD
    //  1. Temperature sensor offset (FY14)
    //  2. Humidity sensor offset (FY14)
    //  3. Enthalpy sensor offset (FY14)
    //  4. Fouling coils (FY14)
    //  5. Thermostat offset (FY15)
    //  6. Humidistat offset (FY15)
    //  7. Fouling air filter (FY15)
    //  8. Chiller Supply Water Temperature Sensor Offset (FY16)
    //  9. Condenser Supply Water Temperature Sensor Offset (FY16)
    //  10. Cooling Tower Scaling (FY16)
    //  11. Coil Supply Air Temperature Sensor Offset (FY16)
    // coming ...
    //  Fouling: chillers, boilers, cooling towers
    //  Damper leakage: return air, outdoor air
    //  Blockage: pipe
    //  Meter: air flow, water flow
    //  CO2 sensor
    //  Pressure sensor offset
    //  more

    Array1D_string const cFaults({0, 15},
                                 {"FaultModel:TemperatureSensorOffset:OutdoorAir",
                                  "FaultModel:HumiditySensorOffset:OutdoorAir",
                                  "FaultModel:EnthalpySensorOffset:OutdoorAir",
                                  "FaultModel:TemperatureSensorOffset:ReturnAir",
                                  "FaultModel:EnthalpySensorOffset:ReturnAir",
                                  "FaultModel:Fouling:Coil",
                                  "FaultModel:ThermostatOffset",
                                  "FaultModel:HumidistatOffset",
                                  "FaultModel:Fouling:AirFilter",
                                  "FaultModel:TemperatureSensorOffset:ChillerSupplyWater",
                                  "FaultModel:TemperatureSensorOffset:CondenserSupplyWater",
                                  "FaultModel:Fouling:CoolingTower",
                                  "FaultModel:TemperatureSensorOffset:CoilSupplyAir",
                                  "FaultModel:Fouling:Boiler",
                                  "FaultModel:Fouling:Chiller",
                                  "FaultModel:Fouling:EvaporativeCooler"});
    //      'FaultModel:PressureSensorOffset:OutdoorAir   ', &
    //      'FaultModel:TemperatureSensorOffset:SupplyAir ', &
    //      'FaultModel:TemperatureSensorOffset:ZoneAir   ', &
    //      'FaultModel:Blockage:Branch                   ', &
    //      'FaultModel:Fouling:Chiller                   ', &
    //      'FaultModel:Fouling:Boiler                    ', &
    //      'FaultModel:DamperLeakage:ReturnAir           ', &
    //      'FaultModel:DamperLeakage:OutdoorAir          ' /)

    enum class ChillerType
    {
        Invalid = -1,
        ChillerElectric,
        ChillerElectricEIR,
        ChillerElectricReformulatedEIR,
        ChillerConstantCOP,
        ChillerEngineDriven,
        ChillerCombustionTurbine,
        ChillerAbsorption,
        ChillerAbsorptionIndirect,
        Num
    };

    enum class CoilType
    {
        Invalid = -1,
        CoilHeatingElectric,
        CoilHeatingFuel,
        CoilHeatingDesuperheater,
        CoilHeatingSteam,
        CoilHeatingWater,
        CoilCoolingWater,
        CoilCoolingWaterDetailedgeometry,
        CoilSystemCoolingDX,
        CoilSystemHeatingDX,
        AirLoopHVACUnitarySystem,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(ChillerType::Num)> ChillerTypeNamesUC{"CHILLER:ELECTRIC",
                                                                                                  "CHILLER:ELECTRIC:EIR",
                                                                                                  "CHILLER:ELECTRIC:REFORMULATEDEIR",
                                                                                                  "CHILLER:CONSTANTCOP",
                                                                                                  "CHILLER:ENGINEDRIVEN",
                                                                                                  "CHILLER:COMBUSTIONTURBINE",
                                                                                                  "CHILLER:ABSORPTION",
                                                                                                  "CHILLER:ABSORPTION:INDIRECT"};

    constexpr std::array<std::string_view, static_cast<int>(CoilType::Num)> CoilTypeNamesUC{"COIL:HEATING:ELECTRIC",
                                                                                            "COIL:HEATING:FUEL",
                                                                                            "COIL:HEATING:DESUPERHEATER",
                                                                                            "COIL:HEATING:STEAM",
                                                                                            "COIL:HEATING:WATER",
                                                                                            "COIL:COOLING:WATER",
                                                                                            "COIL:COOLING:WATER:DETAILEDGEOMETRY",
                                                                                            "COILSYSTEM:COOLING:DX",
                                                                                            "COILSYSTEM:HEATING:DX",
                                                                                            "AIRLOOPHVAC:UNITARYSYSTEM"};

    constexpr std::array<std::string_view, static_cast<int>(FouledCoil::Num)> FouledCoilNamesUC{"FOULEDUARATED", "FOULINGFACTOR"};

    void CheckAndReadFaults(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Tianzhen Hong, LBNL
        //       DATE WRITTEN   August 2013
        //       MODIFIED       Sep. 2013, Xiufeng Pang (XP), LBNL. Added Fouling Coil fault
        //                      Feb. 2015, Rongpeng Zhang, LBNL. Added Thermostat/Humidistat Offset faults
        //                      Apr. 2015, Rongpeng Zhang, LBNL. Added Fouling Air Filter fault
        //                      May. 2016, Rongpeng Zhang, LBNL. Added Chiller/Condenser Supply Water Temperature Sensor fault
        //                      Jun. 2016, Rongpeng Zhang, LBNL. Added Tower Scaling fault
        //                      Jul. 2016, Rongpeng Zhang, LBNL. Added Coil Supply Air Temperature Sensor fault
        //                      Oct. 2016, Rongpeng Zhang, LBNL. Added Fouling Boiler fault
        //                      Nov. 2016, Rongpeng Zhang, LBNL. Added Fouling Chiller fault
        //                      Jan. 2017, Rongpeng Zhang, LBNL. Added Fouling Evaporative Cooler fault

        // PURPOSE OF THIS SUBROUTINE:
        //  1. Determine if any operational faults are present in a model and set flags
        //  2. Read faults input

        // METHODOLOGY EMPLOYED:
        // Get number of faults-related input objects and assign faults input to data structure
        static constexpr std::string_view routineName = "CheckAndReadFaults";
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;  // Number of Alphas for each GetobjectItem call
        int NumNumbers; // Number of Numbers for each GetobjectItem call
        int IOStatus;
        Array1D_string cAlphaArgs(10); // Alpha input items for object
        Array1D_bool lAlphaFieldBlanks(10, false);
        Array1D_bool lNumericFieldBlanks(10, false);
        Array1D_string cAlphaFieldNames(10);
        Array1D_string cNumericFieldNames(10);
        Array1D<Real64> rNumericArgs(10); // Numeric input items for object
        std::string cFaultCurrentObject;

        if (state.dataFaultsMgr->RunFaultMgrOnceFlag) {
            return;
        }

        // check number of faults
        state.dataFaultsMgr->NumFaults = 0;
        state.dataFaultsMgr->NumFaultyEconomizer = 0;
        for (int NumFaultsTemp = 0, i = 0; i <= 15; ++i) {
            NumFaultsTemp = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cFaults(i));
            state.dataFaultsMgr->NumFaults += NumFaultsTemp;

            if (i <= 4) {
                // 1st-5th fault: economizer sensor offset
                state.dataFaultsMgr->NumFaultyEconomizer += NumFaultsTemp;
            } else if (i == 5) {
                // 6th fault: Coil fouling
                state.dataFaultsMgr->NumFouledCoil = NumFaultsTemp;
            } else if (i == 6) {
                // 7th fault: Faulty thermostat
                state.dataFaultsMgr->NumFaultyThermostat = NumFaultsTemp;
            } else if (i == 7) {
                // 8th fault: Faulty humidistat
                state.dataFaultsMgr->NumFaultyHumidistat = NumFaultsTemp;
            } else if (i == 8) {
                // 9th fault: Fouled air filter
                state.dataFaultsMgr->NumFaultyAirFilter = NumFaultsTemp;
            } else if (i == 9) {
                // 10th fault: Faulty Chillers Supply Water Temperature Sensor
                state.dataFaultsMgr->NumFaultyChillerSWTSensor = NumFaultsTemp;
            } else if (i == 10) {
                // 11th fault: Faulty Condenser Supply Water Temperature Sensor
                state.dataFaultsMgr->NumFaultyCondenserSWTSensor = NumFaultsTemp;
            } else if (i == 11) {
                // 12th fault: Faulty Towers with Scaling
                state.dataFaultsMgr->NumFaultyTowerFouling = NumFaultsTemp;
            } else if (i == 12) {
                // 13th fault: Faulty Coil Supply Air Temperature Sensor
                state.dataFaultsMgr->NumFaultyCoilSATSensor = NumFaultsTemp;
            } else if (i == 13) {
                // 14th fault: Faulty Boiler with Fouling
                state.dataFaultsMgr->NumFaultyBoilerFouling = NumFaultsTemp;
            } else if (i == 14) {
                // 15th fault: Faulty Chiller with Fouling
                state.dataFaultsMgr->NumFaultyChillerFouling = NumFaultsTemp;
            } else if (i == 15) {
                // 16th fault: Faulty Evaporative Cooler with Fouling
                state.dataFaultsMgr->NumFaultyEvapCoolerFouling = NumFaultsTemp;
            }
        }

        if (state.dataFaultsMgr->NumFaults > 0) {
            state.dataFaultsMgr->AnyFaultsInModel = true;
        } else {
            state.dataFaultsMgr->AnyFaultsInModel = false;
        }

        if (!state.dataFaultsMgr->AnyFaultsInModel) {
            state.dataFaultsMgr->RunFaultMgrOnceFlag = true;
            return;
        }

        // allocate fault array
        if (state.dataFaultsMgr->NumFaultyEconomizer > 0) {
            state.dataFaultsMgr->FaultsEconomizer.allocate(state.dataFaultsMgr->NumFaultyEconomizer);
        }
        if (state.dataFaultsMgr->NumFouledCoil > 0) {
            state.dataFaultsMgr->FouledCoils.allocate(state.dataFaultsMgr->NumFouledCoil);
        }
        if (state.dataFaultsMgr->NumFaultyThermostat > 0) {
            state.dataFaultsMgr->FaultsThermostatOffset.allocate(state.dataFaultsMgr->NumFaultyThermostat);
        }
        if (state.dataFaultsMgr->NumFaultyHumidistat > 0) {
            state.dataFaultsMgr->FaultsHumidistatOffset.allocate(state.dataFaultsMgr->NumFaultyHumidistat);
        }
        if (state.dataFaultsMgr->NumFaultyAirFilter > 0) {
            state.dataFaultsMgr->FaultsFouledAirFilters.allocate(state.dataFaultsMgr->NumFaultyAirFilter);
        }
        if (state.dataFaultsMgr->NumFaultyChillerSWTSensor > 0) {
            state.dataFaultsMgr->FaultsChillerSWTSensor.allocate(state.dataFaultsMgr->NumFaultyChillerSWTSensor);
        }
        if (state.dataFaultsMgr->NumFaultyCondenserSWTSensor > 0) {
            state.dataFaultsMgr->FaultsCondenserSWTSensor.allocate(state.dataFaultsMgr->NumFaultyCondenserSWTSensor);
        }
        if (state.dataFaultsMgr->NumFaultyTowerFouling > 0) {
            state.dataFaultsMgr->FaultsTowerFouling.allocate(state.dataFaultsMgr->NumFaultyTowerFouling);
        }
        if (state.dataFaultsMgr->NumFaultyCoilSATSensor > 0) {
            state.dataFaultsMgr->FaultsCoilSATSensor.allocate(state.dataFaultsMgr->NumFaultyCoilSATSensor);
        }
        if (state.dataFaultsMgr->NumFaultyBoilerFouling > 0) {
            state.dataFaultsMgr->FaultsBoilerFouling.allocate(state.dataFaultsMgr->NumFaultyBoilerFouling);
        }
        if (state.dataFaultsMgr->NumFaultyChillerFouling > 0) {
            state.dataFaultsMgr->FaultsChillerFouling.allocate(state.dataFaultsMgr->NumFaultyChillerFouling);
        }
        if (state.dataFaultsMgr->NumFaultyEvapCoolerFouling > 0) {
            state.dataFaultsMgr->FaultsEvapCoolerFouling.allocate(state.dataFaultsMgr->NumFaultyEvapCoolerFouling);
        }

        // read faults input of Evaporative Cooler Fouling
        for (int jFault_EvapCoolerFouling = 1; jFault_EvapCoolerFouling <= state.dataFaultsMgr->NumFaultyEvapCoolerFouling;
             ++jFault_EvapCoolerFouling) {
            auto &faultsECFouling = state.dataFaultsMgr->FaultsEvapCoolerFouling(jFault_EvapCoolerFouling);

            cFaultCurrentObject = cFaults(15); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_EvapCoolerFouling,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsECFouling.type = FaultType::Fouling_EvapCooler;
            faultsECFouling.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsECFouling.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsECFouling.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsECFouling.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsECFouling.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // CapReductionFactor - degree of fault
            faultsECFouling.FoulingFactor = rNumericArgs(1);

            // Evaporative cooler type
            faultsECFouling.EvapCoolerType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Evaporative cooler name
            faultsECFouling.EvapCoolerName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Evaporative cooler check
            if (Util::SameString(faultsECFouling.EvapCoolerType, "EvaporativeCooler:Indirect:WetCoil")) {
                // Read in evaporative cooler is not done yet
                if (state.dataEvapCoolers->GetInputEvapComponentsFlag) {
                    EvaporativeCoolers::GetEvapInput(state);
                    state.dataEvapCoolers->GetInputEvapComponentsFlag = false;
                }

                // Check whether the evaporative cooler  name and type match each other;
                int EvapCoolerNum =
                    Util::FindItemInList(faultsECFouling.EvapCoolerName, state.dataEvapCoolers->EvapCond, &EvaporativeCoolers::EvapConditions::Name);
                if (EvapCoolerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the boiler with the fault model
                    state.dataEvapCoolers->EvapCond(EvapCoolerNum).FaultyEvapCoolerFoulingFlag = true;
                    state.dataEvapCoolers->EvapCond(EvapCoolerNum).FaultyEvapCoolerFoulingIndex = jFault_EvapCoolerFouling;
                }
            }
        }

        // read faults input of Chiller Fouling
        for (int jFault_ChillerFouling = 1; jFault_ChillerFouling <= state.dataFaultsMgr->NumFaultyChillerFouling; ++jFault_ChillerFouling) {
            auto &faultsChillerFouling = state.dataFaultsMgr->FaultsChillerFouling(jFault_ChillerFouling);

            cFaultCurrentObject = cFaults(14); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_ChillerFouling,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsChillerFouling.type = FaultType::Fouling_Chiller;
            faultsChillerFouling.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsChillerFouling.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsChillerFouling.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsChillerFouling.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsChillerFouling.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // CapReductionFactor - degree of fault
            faultsChillerFouling.FoulingFactor = rNumericArgs(1);

            // Chiller type
            faultsChillerFouling.ChillerType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Chiller name
            faultsChillerFouling.ChillerName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Chiller check
            int ChillerNum;
            ChillerType ChillerTypeCheck =
                static_cast<ChillerType>(getEnumValue(ChillerTypeNamesUC, Util::makeUPPER(faultsChillerFouling.ChillerType)));
            switch (ChillerTypeCheck) {
            case ChillerType::ChillerElectric: {
                // Check whether the chiller name and chiller type match each other
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->ElectricChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerFouling.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {

                    if (state.dataPlantChillers->ElectricChiller(ChillerNum).CondenserType != DataPlant::CondenserType::WaterCooled) {
                        // The fault model is only applicable to the chillers with water based condensers
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified chiller is not water cooled. The chiller fouling "
                                        "fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));

                    } else {
                        // Link the chiller with the fault model
                        state.dataPlantChillers->ElectricChiller(ChillerNum).FaultyChillerFoulingFlag = true;
                        state.dataPlantChillers->ElectricChiller(ChillerNum).FaultyChillerFoulingIndex = jFault_ChillerFouling;
                    }
                }
            } break;
            case ChillerType::ChillerElectricEIR: {
                // Read in chiller if not done yet
                if (state.dataChillerElectricEIR->getInputFlag) {
                    ChillerElectricEIR::GetElectricEIRChillerInput(state);
                    state.dataChillerElectricEIR->getInputFlag = false;
                }

                // Check whether the chiller name and chiller type match each other
                ChillerNum = Util::FindItemInList(faultsChillerFouling.ChillerName, state.dataChillerElectricEIR->ElectricEIRChiller);
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {

                    if (state.dataChillerElectricEIR->ElectricEIRChiller(ChillerNum).CondenserType != DataPlant::CondenserType::WaterCooled) {
                        // The fault model is only applicable to the chillers with water based condensers
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified chiller is not water cooled. The chiller fouling "
                                        "fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));

                    } else {
                        // Link the chiller with the fault model
                        state.dataChillerElectricEIR->ElectricEIRChiller(ChillerNum).FaultyChillerFoulingFlag = true;
                        state.dataChillerElectricEIR->ElectricEIRChiller(ChillerNum).FaultyChillerFoulingIndex = jFault_ChillerFouling;
                    }
                }
            } break;
            case ChillerType::ChillerElectricReformulatedEIR: {
                // Read in chiller if not done yet
                if (state.dataChillerReformulatedEIR->GetInputREIR) {
                    ChillerReformulatedEIR::GetElecReformEIRChillerInput(state);
                    state.dataChillerReformulatedEIR->GetInputREIR = false;
                }

                // Check whether the chiller name and chiller type match each other
                ChillerNum = Util::FindItemInList(faultsChillerFouling.ChillerName, state.dataChillerReformulatedEIR->ElecReformEIRChiller);
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {

                    if (state.dataChillerReformulatedEIR->ElecReformEIRChiller(ChillerNum).CondenserType != DataPlant::CondenserType::WaterCooled) {
                        // The fault model is only applicable to the chillers with water based condensers
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified chiller is not water cooled. The chiller fouling "
                                        "fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));

                    } else {
                        // Link the chiller with the fault model
                        state.dataChillerReformulatedEIR->ElecReformEIRChiller(ChillerNum).FaultyChillerFoulingFlag = true;
                        state.dataChillerReformulatedEIR->ElecReformEIRChiller(ChillerNum).FaultyChillerFoulingIndex = jFault_ChillerFouling;
                    }
                }
            } break;
            case ChillerType::ChillerConstantCOP: {
                // Check whether the chiller name and chiller type match each other
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->ConstCOPChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerFouling.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {

                    if (state.dataPlantChillers->ConstCOPChiller(ChillerNum).CondenserType != DataPlant::CondenserType::WaterCooled) {
                        // The fault model is only applicable to the chillers with water based condensers
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified chiller is not water cooled. The chiller fouling "
                                        "fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));

                    } else {
                        // Link the chiller with the fault model
                        state.dataPlantChillers->ConstCOPChiller(ChillerNum).FaultyChillerFoulingFlag = true;
                        state.dataPlantChillers->ConstCOPChiller(ChillerNum).FaultyChillerFoulingIndex = jFault_ChillerFouling;
                    }
                }
            } break;
            case ChillerType::ChillerEngineDriven: {
                // Check whether the chiller name and chiller type match each other
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->EngineDrivenChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerFouling.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {

                    if (state.dataPlantChillers->EngineDrivenChiller(ChillerNum).CondenserType != DataPlant::CondenserType::WaterCooled) {
                        // The fault model is only applicable to the chillers with water based condensers
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified chiller is not water cooled. The chiller fouling "
                                        "fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));

                    } else {
                        // Link the fault model with the water cooled chiller
                        state.dataPlantChillers->EngineDrivenChiller(ChillerNum).FaultyChillerFoulingFlag = true;
                        state.dataPlantChillers->EngineDrivenChiller(ChillerNum).FaultyChillerFoulingIndex = jFault_ChillerFouling;
                    }
                }
            } break;
            case ChillerType::ChillerCombustionTurbine: {
                // Check whether the chiller name and chiller type match each other
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->GTChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerFouling.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    if (state.dataPlantChillers->GTChiller(ChillerNum).CondenserType != DataPlant::CondenserType::WaterCooled) {
                        // The fault model is only applicable to the chillers with water based condensers
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified chiller is not water cooled. The chiller fouling "
                                        "fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));

                    } else {
                        // Link the fault model with the water cooled chiller
                        state.dataPlantChillers->GTChiller(ChillerNum).FaultyChillerFoulingFlag = true;
                        state.dataPlantChillers->GTChiller(ChillerNum).FaultyChillerFoulingIndex = jFault_ChillerFouling;
                    }
                }
            } break;
            default:
                break;
            }
        }

        // read faults input of Boiler Fouling
        for (int jFault_BoilerFouling = 1; jFault_BoilerFouling <= state.dataFaultsMgr->NumFaultyBoilerFouling; ++jFault_BoilerFouling) {
            auto &faultsBoilerFouling = state.dataFaultsMgr->FaultsBoilerFouling(jFault_BoilerFouling);

            cFaultCurrentObject = cFaults(13); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_BoilerFouling,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsBoilerFouling.type = FaultType::Fouling_Boiler;
            faultsBoilerFouling.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsBoilerFouling.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsBoilerFouling.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsBoilerFouling.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsBoilerFouling.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // CapReductionFactor - degree of fault
            faultsBoilerFouling.FoulingFactor = rNumericArgs(1);

            // Boiler type
            faultsBoilerFouling.BoilerType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Boiler name
            faultsBoilerFouling.BoilerName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Boiler check and link
            {
                if (state.dataBoilers->getBoilerInputFlag) {
                    Boilers::GetBoilerInput(state);
                    state.dataBoilers->getBoilerInputFlag = false;
                }
                // Check the boiler name and boiler type
                auto boiler_it = std::find_if(state.dataBoilers->Boiler.begin(), state.dataBoilers->Boiler.end(), [&](const auto &b) {
                    return b.Name == faultsBoilerFouling.BoilerName;
                });
                if (boiler_it == state.dataBoilers->Boiler.end()) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the boiler with the fault model
                    boiler_it->FaultyBoilerFoulingFlag = true;
                    boiler_it->FaultyBoilerFoulingIndex = jFault_BoilerFouling;
                }
            }
        }

        // read faults input of Coil SAT Sensor Offset
        for (int jFault_CoilSAT = 1; jFault_CoilSAT <= state.dataFaultsMgr->NumFaultyCoilSATSensor; ++jFault_CoilSAT) {
            auto &faultsCoilSATFouling = state.dataFaultsMgr->FaultsCoilSATSensor(jFault_CoilSAT);

            cFaultCurrentObject = cFaults(12); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_CoilSAT,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsCoilSATFouling.type = FaultType::TemperatureSensorOffset_CoilSupplyAir;
            faultsCoilSATFouling.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsCoilSATFouling.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsCoilSATFouling.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsCoilSATFouling.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsCoilSATFouling.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // offset - degree of fault
            faultsCoilSATFouling.Offset = rNumericArgs(1);

            // Coil type
            faultsCoilSATFouling.CoilType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Coil name
            faultsCoilSATFouling.CoilName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Coil check and link
            CoilType CoilTypeCheck = static_cast<CoilType>(getEnumValue(CoilTypeNamesUC, Util::makeUPPER(faultsCoilSATFouling.CoilType)));
            switch (CoilTypeCheck) {
            case CoilType::CoilHeatingElectric:
            case CoilType::CoilHeatingFuel:
            case CoilType::CoilHeatingDesuperheater: {
                // Read in coil input if not done yet
                if (state.dataHeatingCoils->GetCoilsInputFlag) {
                    HeatingCoils::GetHeatingCoilInput(state);
                    state.dataHeatingCoils->GetCoilsInputFlag = false;
                }
                // Check the coil name and coil type
                int CoilNum = Util::FindItemInList(faultsCoilSATFouling.CoilName, state.dataHeatingCoils->HeatingCoil);
                if (CoilNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the coil with the fault model
                    state.dataHeatingCoils->HeatingCoil(CoilNum).FaultyCoilSATFlag = true;
                    state.dataHeatingCoils->HeatingCoil(CoilNum).FaultyCoilSATIndex = jFault_CoilSAT;
                }
            } break;
            case CoilType::CoilHeatingSteam: {
                // Read in coil input if not done yet
                if (state.dataSteamCoils->GetSteamCoilsInputFlag) {
                    SteamCoils::GetSteamCoilInput(state);
                    state.dataSteamCoils->GetSteamCoilsInputFlag = false;
                }
                // Check the coil name and coil type
                int CoilNum = Util::FindItemInList(faultsCoilSATFouling.CoilName, state.dataSteamCoils->SteamCoil);
                if (CoilNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {

                    if (state.dataSteamCoils->SteamCoil(CoilNum).TypeOfCoil != SteamCoils::CoilControlType::TemperatureSetPoint) {
                        // The fault model is only applicable to the coils controlled on leaving air temperature
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\". The specified coil is not controlled on leaving air temperature. "
                                        "The coil SAT sensor fault model will not be applied.",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(5),
                                        cAlphaArgs(5)));
                    } else {
                        // Link the fault model with the coil that is controlled on leaving air temperature
                        state.dataSteamCoils->SteamCoil(CoilNum).FaultyCoilSATFlag = true;
                        state.dataSteamCoils->SteamCoil(CoilNum).FaultyCoilSATIndex = jFault_CoilSAT;
                    }
                }
            } break;
            case CoilType::CoilHeatingWater:
            case CoilType::CoilCoolingWater:
            case CoilType::CoilCoolingWaterDetailedgeometry: {
                // Read in coil input if not done yet
                if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
                    WaterCoils::GetWaterCoilInput(state);
                    state.dataWaterCoils->GetWaterCoilsInputFlag = false;
                }
                // Check the coil name and coil type
                int CoilNum = Util::FindItemInList(faultsCoilSATFouling.CoilName, state.dataWaterCoils->WaterCoil);
                if (CoilNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                }

                // Read in Water Coil Controller Name
                faultsCoilSATFouling.WaterCoilControllerName = cAlphaArgs(6);
                if (lAlphaFieldBlanks(6)) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(6), cAlphaArgs(6)));
                    state.dataFaultsMgr->ErrorsFound = true;
                }
                // Read in controller input if not done yet
                if (state.dataHVACControllers->GetControllerInputFlag) {
                    HVACControllers::GetControllerInput(state);
                    state.dataHVACControllers->GetControllerInputFlag = false;
                }
                // Check the controller name
                int ControlNum = Util::FindItemInList(faultsCoilSATFouling.WaterCoilControllerName,
                                                      state.dataHVACControllers->ControllerProps,
                                                      &HVACControllers::ControllerPropsType::ControllerName);
                if (ControlNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(6), cAlphaArgs(6)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the controller with the fault model
                    state.dataHVACControllers->ControllerProps(ControlNum).FaultyCoilSATFlag = true;
                    state.dataHVACControllers->ControllerProps(ControlNum).FaultyCoilSATIndex = jFault_CoilSAT;

                    // Check whether the controller match the coil
                    if (state.dataHVACControllers->ControllerProps(ControlNum).SensedNode !=
                        state.dataWaterCoils->WaterCoil(CoilNum).AirOutletNodeNum) {
                        ShowSevereError(state,
                                        std::format("{} = \"{}\" invalid {} = \"{}\" does not match {} = \"{}",
                                                    cFaultCurrentObject,
                                                    cAlphaArgs(1),
                                                    cAlphaFieldNames(6),
                                                    cAlphaArgs(6),
                                                    cAlphaFieldNames(5),
                                                    cAlphaArgs(5)));
                        state.dataFaultsMgr->ErrorsFound = true;
                    }
                }
            } break;
            case CoilType::CoilSystemCoolingDX: {
                // see case CoilCheck::AirLoopHVACUnitarySystem: below
                // UnitarySystem connects a different way. Make sure this works by testing a CoilSystem model.
                // Read in DXCoolingSystem input if not done yet
                // if (state.dataHVACDXSys->GetInputFlag) {
                //    HVACDXSystem::GetDXCoolingSystemInput(state);
                //    state.dataHVACDXSys->GetInputFlag = false;
                //}

                //// Check the coil name and coil type
                // int CoilSysNum = Util::FindItemInList(faultsCoilSATFouling.CoilName,
                //                                                 state.dataHVACDXSys->DXCoolingSystem);
                // if (CoilSysNum <= 0) {
                //    ShowSevereError(state, std::format("{}{} = \"{}\" invalid {} = \"{}{}\" not found.", //, cFaultCurrentObject, cAlphaArgs(1),
                //    cAlphaFieldNames(5), //, cAlphaArgs(5))); state.dataFaultsMgr->ErrorsFound = true;
                //} else {
                //    // Link the coil system with the fault model
                //    state.dataHVACDXSys->DXCoolingSystem(CoilSysNum).FaultyCoilSATFlag = true;
                //    state.dataHVACDXSys->DXCoolingSystem(CoilSysNum).FaultyCoilSATIndex = jFault_CoilSAT;
                //}
            } break;
            case CoilType::CoilSystemHeatingDX: {
                // Read in DXCoolingSystem input if not done yet
                if (state.dataHVACDXHeatPumpSys->GetInputFlag) {
                    HVACDXHeatPumpSystem::GetDXHeatPumpSystemInput(state);
                    state.dataHVACDXHeatPumpSys->GetInputFlag = false;
                }

                // Check the coil name and coil type
                int CoilSysNum = Util::FindItemInList(faultsCoilSATFouling.CoilName, state.dataHVACDXHeatPumpSys->DXHeatPumpSystem);
                if (CoilSysNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the coil system with the fault model
                    state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(CoilSysNum).FaultyCoilSATFlag = true;
                    state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(CoilSysNum).FaultyCoilSATIndex = jFault_CoilSAT;
                }
            } break;
            case CoilType::AirLoopHVACUnitarySystem: {
                // UnitarySystem model connects to FaultManager via function call to FaultsManager::SetFaultyCoilSATSensor
            } break;
            default:
                break;
            }
        } // End read faults input of Fault_type 113

        // read faults input of Cooling tower scaling
        for (int jFault_TowerFouling = 1; jFault_TowerFouling <= state.dataFaultsMgr->NumFaultyTowerFouling; ++jFault_TowerFouling) {
            auto &faultsTowerFouling = state.dataFaultsMgr->FaultsTowerFouling(jFault_TowerFouling);

            cFaultCurrentObject = cFaults(11); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_TowerFouling,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsTowerFouling.type = FaultType::Fouling_Tower;
            faultsTowerFouling.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsTowerFouling.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsTowerFouling.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsTowerFouling.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsTowerFouling.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // UAReductionFactor - degree of fault
            faultsTowerFouling.UAReductionFactor = rNumericArgs(1);

            // Cooling tower type
            faultsTowerFouling.TowerType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Cooling tower name
            faultsTowerFouling.TowerName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Tower check and link
            {
                // Read in tower input if not done yet
                if (state.dataCondenserLoopTowers->GetInput) {
                    CondenserLoopTowers::GetTowerInput(state);
                    state.dataCondenserLoopTowers->GetInput = false;
                }
                // Check the tower name and tower type
                int TowerNum = Util::FindItemInList(faultsTowerFouling.TowerName, state.dataCondenserLoopTowers->towers);
                if (TowerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the tower with the fault model
                    state.dataCondenserLoopTowers->towers(TowerNum).FaultyTowerFoulingFlag = true;
                    state.dataCondenserLoopTowers->towers(TowerNum).FaultyTowerFoulingIndex = jFault_TowerFouling;

                    // Check the faulty tower type
                    if (!Util::SameString(DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataCondenserLoopTowers->towers(TowerNum).TowerType)],
                                          faultsTowerFouling.TowerType)) {
                        ShowWarningError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\" not match the type of {}. Tower type in the fault model is updated. ",
                                        cFaultCurrentObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(4),
                                        cAlphaArgs(4),
                                        cAlphaFieldNames(5)));
                        faultsTowerFouling.TowerType =
                            DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataCondenserLoopTowers->towers(TowerNum).TowerType)];
                    }

                    // Check the tower model
                    // Performance Input Method should be UFactorTimesAreaAndDesignWaterFlowRate to apply the fault model
                    if (state.dataCondenserLoopTowers->towers(TowerNum).PerformanceInputMethod_Num != CondenserLoopTowers::PIM::UFactor) {
                        ShowWarningError(
                            state,
                            std::format(
                                "{} = \"{}\" invalid {} = \"{}. Tower Performance Input Method is not UFactorTimesAreaAndDesignWaterFlowRate. The "
                                "tower fouling fault model will not be applied to the tower. ",
                                cFaultCurrentObject,
                                cAlphaArgs(1),
                                cAlphaFieldNames(5),
                                cAlphaFieldNames(5)));
                        state.dataCondenserLoopTowers->towers(TowerNum).FaultyTowerFoulingFlag = false;
                    }
                }
            }
        }

        // read faults input of Condenser SWT Sensor Offset
        for (int jFault_CondenserSWT = 1; jFault_CondenserSWT <= state.dataFaultsMgr->NumFaultyCondenserSWTSensor; ++jFault_CondenserSWT) {
            auto &faultsCondSWTFouling = state.dataFaultsMgr->FaultsCondenserSWTSensor(jFault_CondenserSWT);

            cFaultCurrentObject = cFaults(10); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_CondenserSWT,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsCondSWTFouling.type = FaultType::TemperatureSensorOffset_CondenserSupplyWater;
            faultsCondSWTFouling.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsCondSWTFouling.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsCondSWTFouling.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsCondSWTFouling.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsCondSWTFouling.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // offset - degree of fault
            faultsCondSWTFouling.Offset = rNumericArgs(1);

            // Cooling tower type
            faultsCondSWTFouling.TowerType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Cooling tower name
            faultsCondSWTFouling.TowerName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Tower check and link
            {
                // Read in tower input if not done yet
                if (state.dataCondenserLoopTowers->GetInput) {
                    CondenserLoopTowers::GetTowerInput(state);
                    state.dataCondenserLoopTowers->GetInput = false;
                }
                // Check the tower name and tower type
                int TowerNum = Util::FindItemInList(faultsCondSWTFouling.TowerName, state.dataCondenserLoopTowers->towers);
                if (TowerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the tower with the fault model
                    state.dataCondenserLoopTowers->towers(TowerNum).FaultyCondenserSWTFlag = true;
                    state.dataCondenserLoopTowers->towers(TowerNum).FaultyCondenserSWTIndex = jFault_CondenserSWT;

                    // Check the faulty tower type
                    if (!Util::SameString(DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataCondenserLoopTowers->towers(TowerNum).TowerType)],
                                          faultsCondSWTFouling.TowerType)) {
                        ShowWarningError(state,
                                         std::format("{} = \"{}\" invalid {} = \"{}\" not match the type of {}. Tower type is updated. ",
                                                     cFaultCurrentObject,
                                                     cAlphaArgs(1),
                                                     cAlphaFieldNames(4),
                                                     cAlphaArgs(4),
                                                     cAlphaFieldNames(5)));
                        faultsCondSWTFouling.TowerType =
                            DataPlant::PlantEquipTypeNames[static_cast<int>(state.dataCondenserLoopTowers->towers(TowerNum).TowerType)];
                    }
                }
            }
        }

        // read faults input of Chiller SWT Sensor Offset
        for (int jFault_ChillerSWT = 1; jFault_ChillerSWT <= state.dataFaultsMgr->NumFaultyChillerSWTSensor; ++jFault_ChillerSWT) {
            auto &faultsChillerSWT = state.dataFaultsMgr->FaultsChillerSWTSensor(jFault_ChillerSWT);

            cFaultCurrentObject = cFaults(9); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_ChillerSWT,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsChillerSWT.type = FaultType::TemperatureSensorOffset_ChillerSupplyWater;
            faultsChillerSWT.Name = cAlphaArgs(1);

            // Fault availability schedule
            if (lAlphaFieldBlanks(2)) {
                faultsChillerSWT.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsChillerSWT.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fault severity schedule
            if (lAlphaFieldBlanks(3)) {
                faultsChillerSWT.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsChillerSWT.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // offset - degree of fault
            faultsChillerSWT.Offset = rNumericArgs(1);

            // Chiller type
            faultsChillerSWT.ChillerType = cAlphaArgs(4);
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Chiller name
            faultsChillerSWT.ChillerName = cAlphaArgs(5);
            if (lAlphaFieldBlanks(5)) {
                ShowSevereError(
                    state,
                    std::format("{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Chiller check
            int ChillerNum;
            ChillerType ChillerTypeCheck = static_cast<ChillerType>(getEnumValue(ChillerTypeNamesUC, Util::makeUPPER(faultsChillerSWT.ChillerType)));
            switch (ChillerTypeCheck) {
            case ChillerType::ChillerElectric: {
                // Check whether the chiller name and chiller type match each other
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->ElectricChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerSWT.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataPlantChillers->ElectricChiller(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataPlantChillers->ElectricChiller(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerElectricEIR: {
                // Read in chiller if not done yet
                if (state.dataChillerElectricEIR->getInputFlag) {
                    ChillerElectricEIR::GetElectricEIRChillerInput(state);
                    state.dataChillerElectricEIR->getInputFlag = false;
                }
                // Check whether the chiller name and chiller type match each other
                ChillerNum = Util::FindItemInList(faultsChillerSWT.ChillerName, state.dataChillerElectricEIR->ElectricEIRChiller);
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataChillerElectricEIR->ElectricEIRChiller(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataChillerElectricEIR->ElectricEIRChiller(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerElectricReformulatedEIR: {
                // Read in chiller if not done yet
                if (state.dataChillerReformulatedEIR->GetInputREIR) {
                    ChillerReformulatedEIR::GetElecReformEIRChillerInput(state);
                    state.dataChillerReformulatedEIR->GetInputREIR = false;
                }
                // Check whether the chiller name and chiller type match each other
                ChillerNum = Util::FindItemInList(faultsChillerSWT.ChillerName, state.dataChillerReformulatedEIR->ElecReformEIRChiller);
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataChillerReformulatedEIR->ElecReformEIRChiller(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataChillerReformulatedEIR->ElecReformEIRChiller(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerEngineDriven: {
                // Check whether the chiller name and chiller type match each other
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->EngineDrivenChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerSWT.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataPlantChillers->EngineDrivenChiller(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataPlantChillers->EngineDrivenChiller(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerCombustionTurbine: {
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->GTChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerSWT.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataPlantChillers->GTChiller(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataPlantChillers->GTChiller(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerConstantCOP: {
                ChillerNum = 0;
                int thisChil = 0;
                for (auto const &ch : state.dataPlantChillers->ConstCOPChiller) {
                    thisChil++;
                    if (ch.Name == faultsChillerSWT.ChillerName) {
                        ChillerNum = thisChil;
                    }
                }
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataPlantChillers->ConstCOPChiller(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataPlantChillers->ConstCOPChiller(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerAbsorption: {
                // Read in chiller if not done yet
                if (state.dataChillerAbsorber->getInput) {
                    ChillerAbsorption::GetBLASTAbsorberInput(state);
                    state.dataChillerAbsorber->getInput = false;
                }
                // Check whether the chiller name and chiller type match each other
                ChillerNum = Util::FindItemInList(faultsChillerSWT.ChillerName, state.dataChillerAbsorber->absorptionChillers);
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Link the chiller with the fault model
                    state.dataChillerAbsorber->absorptionChillers(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataChillerAbsorber->absorptionChillers(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            case ChillerType::ChillerAbsorptionIndirect: {
                // Read in chiller if not done yet
                if (state.dataChillerIndirectAbsorption->GetInput) {
                    ChillerIndirectAbsorption::GetIndirectAbsorberInput(state);
                    state.dataChillerIndirectAbsorption->GetInput = false;
                }
                // Check whether the chiller name and chiller type match each other
                ChillerNum = Util::FindItemInList(faultsChillerSWT.ChillerName, state.dataChillerIndirectAbsorption->IndirectAbsorber);
                if (ChillerNum <= 0) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" not found.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    state.dataChillerIndirectAbsorption->IndirectAbsorber(ChillerNum).FaultyChillerSWTFlag = true;
                    state.dataChillerIndirectAbsorption->IndirectAbsorber(ChillerNum).FaultyChillerSWTIndex = jFault_ChillerSWT;
                }
            } break;
            default:
                break;
            }
        }

        // read faults input of Fouled Air Filters
        for (int jFault_AirFilter = 1; jFault_AirFilter <= state.dataFaultsMgr->NumFaultyAirFilter; ++jFault_AirFilter) {
            auto &faultsAirFilter = state.dataFaultsMgr->FaultsFouledAirFilters(jFault_AirFilter);

            // Read in fan if not done yet
            if (state.dataFans->GetFanInputFlag) {
                Fans::GetFanInput(state);
            }

            cFaultCurrentObject = cFaults(8); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_AirFilter,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsAirFilter.type = FaultType::Fouling_AirFilter;
            faultsAirFilter.Name = cAlphaArgs(1);

            // Information of the fan associated with the fouling air filter
            faultsAirFilter.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, cAlphaArgs(2)));
            if (faultsAirFilter.fanType == HVAC::FanType::SystemModel) {
                ShowSevereError(state, "Fault:AirFilter cannot be applied to a Fan:SystemModel object");
                state.dataFaultsMgr->ErrorsFound = true;
            }

            faultsAirFilter.fanName = cAlphaArgs(3);

            // Check whether the specified fan exists in the fan list
            if ((faultsAirFilter.fanNum = Fans::GetFanIndex(state, cAlphaArgs(3))) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            } else {
                auto *fanComp = dynamic_cast<Fans::FanComponent *>(state.dataFans->fans(faultsAirFilter.fanNum));
                assert(fanComp != nullptr);

                fanComp->faultyFilterFlag = true;
                fanComp->faultyFilterIndex = jFault_AirFilter;
            }

            // Fault availability schedule
            if (lAlphaFieldBlanks(4)) {
                faultsAirFilter.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsAirFilter.availSched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fan pressure increase fraction schedule
            if (lAlphaFieldBlanks(5)) {
                faultsAirFilter.pressFracSched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsAirFilter.pressFracSched = Sched::GetSchedule(state, cAlphaArgs(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(5), cAlphaArgs(5));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Fan curve describing the relationship between fan pressure rise and air flow rate

            if ((faultsAirFilter.fanCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(6))) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Checking  whether the specified fan curve covers the **design** operational point of the fan cannot be done here
            // as the fan might be autosized and is not sized yet, so we call it in Fan::SizeFan instead

            // In the fan object, calculate by each time-step: 1) pressure increase value; 2) air flow rate decrease value.
        }

        // read faults input of HumidistatOffset
        for (int jFault_Humidistat = 1; jFault_Humidistat <= state.dataFaultsMgr->NumFaultyHumidistat; ++jFault_Humidistat) {
            auto &faultsHStat = state.dataFaultsMgr->FaultsHumidistatOffset(jFault_Humidistat);

            cFaultCurrentObject = cFaults(7); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_Humidistat,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsHStat.type = FaultType::HumidistatOffset;
            faultsHStat.Name = cAlphaArgs(1);
            faultsHStat.FaultyHumidistatName = cAlphaArgs(2);
            faultsHStat.FaultyHumidistatType = cAlphaArgs(3);

            if (Util::SameString(faultsHStat.FaultyHumidistatType, "ThermostatOffsetDependent")) {
                // For Humidistat Offset Type: ThermostatOffsetDependent

                // Related Thermostat Offset Fault Name is required for Humidistat Offset Type: ThermostatOffsetDependent
                if (lAlphaFieldBlanks(6)) {
                    ShowSevereError(state,
                                    std::format("{} = \"{}\": {} cannot be blank for Humidistat Offset Type = \"ThermostatOffsetDependent\".",
                                                cFaultCurrentObject,
                                                cAlphaArgs(1),
                                                cAlphaFieldNames(6)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    faultsHStat.FaultyThermostatName = cAlphaArgs(6);
                }

            } else {
                // For Humidistat Offset Type: ThermostatOffsetIndependent

                // Availability schedule
                if (lAlphaFieldBlanks(4)) {
                    faultsHStat.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
                } else if ((faultsHStat.availSched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                    state.dataFaultsMgr->ErrorsFound = true;
                }

                // Severity schedule
                if (lAlphaFieldBlanks(5)) {
                    faultsHStat.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
                } else if ((faultsHStat.severitySched = Sched::GetSchedule(state, cAlphaArgs(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(5), cAlphaArgs(5));
                    state.dataFaultsMgr->ErrorsFound = true;
                }

                // Reference offset value is required for Humidistat Offset Type: ThermostatOffsetIndependent
                if (lAlphaFieldBlanks(1)) {
                    ShowSevereError(state,
                                    std::format("{} = \"{}\": {} cannot be blank for Humidistat Offset Type = \"ThermostatOffsetIndependent\".",
                                                cFaultCurrentObject,
                                                cAlphaArgs(1),
                                                cNumericFieldNames(1)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    faultsHStat.Offset = rNumericArgs(1);
                }
            }
        }

        // read faults input of ThermostatOffset
        for (int jFault_Thermostat = 1; jFault_Thermostat <= state.dataFaultsMgr->NumFaultyThermostat; ++jFault_Thermostat) {
            auto &faultsTStat = state.dataFaultsMgr->FaultsThermostatOffset(jFault_Thermostat);

            cFaultCurrentObject = cFaults(6); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_Thermostat,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsTStat.type = FaultType::ThermostatOffset;
            faultsTStat.Name = cAlphaArgs(1);
            faultsTStat.FaultyThermostatName = cAlphaArgs(2);

            // Availability schedule
            if (lAlphaFieldBlanks(3)) {
                faultsTStat.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsTStat.availSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Severity schedule
            if (lAlphaFieldBlanks(4)) {
                faultsTStat.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsTStat.severitySched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Reference offset value is required
            if (lAlphaFieldBlanks(1)) {
                ShowSevereError(state, std::format("{} = \"{}\" cannot be blank.", cFaultCurrentObject, cNumericFieldNames(1)));
                state.dataFaultsMgr->ErrorsFound = true;
            } else {
                faultsTStat.Offset = rNumericArgs(1);
            }
        }

        // read faults input of Fouling_Coil
        for (int jFault_FoulingCoil = 1; jFault_FoulingCoil <= state.dataFaultsMgr->NumFouledCoil; ++jFault_FoulingCoil) {
            auto &faultsFoulCoil = state.dataFaultsMgr->FouledCoils(jFault_FoulingCoil);

            cFaultCurrentObject = cFaults(5); // fault object string
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cFaultCurrentObject,
                                                                     jFault_FoulingCoil,
                                                                     cAlphaArgs,
                                                                     NumAlphas,
                                                                     rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};
            faultsFoulCoil.type = FaultType::Fouling_Coil;
            faultsFoulCoil.Name = cAlphaArgs(1);
            faultsFoulCoil.FouledCoilName = cAlphaArgs(2);

            // Availability schedule
            if (lAlphaFieldBlanks(3)) {
                faultsFoulCoil.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
            } else if ((faultsFoulCoil.availSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            // Severity schedule
            if (lAlphaFieldBlanks(4)) {
                faultsFoulCoil.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
            } else if ((faultsFoulCoil.severitySched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                state.dataFaultsMgr->ErrorsFound = true;
            }

            faultsFoulCoil.FoulingInputMethod = static_cast<FouledCoil>(getEnumValue(FouledCoilNamesUC, Util::makeUPPER(cAlphaArgs(5))));
            if (faultsFoulCoil.FoulingInputMethod == FouledCoil::Invalid) {
                faultsFoulCoil.FoulingInputMethod = FouledCoil::UARated;
            }

            faultsFoulCoil.UAFouled = rNumericArgs(1);
            faultsFoulCoil.Rfw = rNumericArgs(2);
            faultsFoulCoil.Rfa = rNumericArgs(3);
            faultsFoulCoil.Aout = rNumericArgs(4);
            faultsFoulCoil.Aratio = rNumericArgs(5);

            // Coil check and link
            {
                // Obtains and Allocates WaterCoil related parameters from input file
                if (state.dataWaterCoils->GetWaterCoilsInputFlag) {
                    WaterCoils::GetWaterCoilInput(state);
                    state.dataWaterCoils->GetWaterCoilsInputFlag = false;
                }

                // Check the coil name and type
                int CoilNum = Util::FindItemInList(faultsFoulCoil.FouledCoilName, state.dataWaterCoils->WaterCoil);
                if (CoilNum <= 0) {
                    ShowSevereError(state,
                                    std::format("{} = \"{}\". Referenced Coil named \"{}\" was not found.",
                                                cFaultCurrentObject,
                                                cAlphaArgs(1),
                                                faultsFoulCoil.FouledCoilName));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    // Coil is found: check if the right type
                    if ((state.dataWaterCoils->WaterCoil(CoilNum).WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) ||
                        (state.dataWaterCoils->WaterCoil(CoilNum).WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling)) {
                        // Link the Coil with the fault model
                        state.dataWaterCoils->WaterCoil(CoilNum).FaultyCoilFoulingFlag = true;
                        state.dataWaterCoils->WaterCoil(CoilNum).FaultyCoilFoulingIndex = jFault_FoulingCoil;

                        faultsFoulCoil.FouledCoilType = state.dataWaterCoils->WaterCoil(CoilNum).WaterCoilType;
                        faultsFoulCoil.FouledCoilNum = CoilNum;

                        SetupOutputVariable(state,
                                            "Coil Fouling Factor",
                                            Constant::Units::K_W,
                                            state.dataWaterCoils->WaterCoil(CoilNum).FaultyCoilFoulingFactor,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            state.dataWaterCoils->WaterCoil(CoilNum).Name);

                        // Coil:Cooling:Water doesn't report UA because it's not variable,
                        // but here, it's useful since we do change it via fouling, so report it
                        if (state.dataWaterCoils->WaterCoil(CoilNum).WaterCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) {
                            SetupOutputVariable(state,
                                                "Cooling Coil Total U Factor Times Area Value",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).UACoilTotal,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);

                            SetupOutputVariable(state,
                                                "Cooling Coil External U Factor Times Area Value",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).UACoilExternal,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);

                            SetupOutputVariable(state,
                                                "Cooling Coil Internal U Factor Times Area Value",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).UACoilInternal,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);

                            SetupOutputVariable(state,
                                                "Cooling Coil Total U Factor Times Area Value Before Fouling",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).OriginalUACoilVariable,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);

                            SetupOutputVariable(state,
                                                "Cooling Coil External U Factor Times Area Value Before Fouling",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).OriginalUACoilExternal,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);

                            SetupOutputVariable(state,
                                                "Cooling Coil Internal U Factor Times Area Value Before Fouling",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).OriginalUACoilInternal,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);

                        } else {
                            SetupOutputVariable(state,
                                                "Heating Coil U Factor Times Area Value Before Fouling",
                                                Constant::Units::W_K,
                                                state.dataWaterCoils->WaterCoil(CoilNum).OriginalUACoilVariable,
                                                OutputProcessor::TimeStepType::System,
                                                OutputProcessor::StoreType::Average,
                                                state.dataWaterCoils->WaterCoil(CoilNum).Name);
                        }
                    } else {
                        ShowSevereError(
                            state,
                            std::format("{} = \"{}\" invalid {} = \"{}\".", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(
                            state, R"(Coil was found but it is not one of the supported types ("Coil:Cooling:Water" or "Coil:Heating:Water").)");
                        state.dataFaultsMgr->ErrorsFound = true;
                    }
                }
            }
        }

        // read faults input: Fault_type 0 to 4, which are related with economizer sensors
        for (int j = 0, i = 0; i <= 4; ++i) {
            cFaultCurrentObject = cFaults(i); // fault object string
            int NumFaultsTemp = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cFaultCurrentObject);

            for (int jj = 1; jj <= NumFaultsTemp; ++jj) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cFaultCurrentObject,
                                                                         jj,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cFaultCurrentObject, cAlphaArgs(1)};

                ++j;
                auto &fault = state.dataFaultsMgr->FaultsEconomizer(j);
                fault.type = static_cast<FaultType>(i);

                fault.Name = cAlphaArgs(1);

                // check availability schedule
                if (lAlphaFieldBlanks(2)) {
                    fault.availSched = Sched::GetScheduleAlwaysOn(state); // returns schedule value of 1
                } else if ((fault.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    state.dataFaultsMgr->ErrorsFound = true;
                }

                // check severity schedule
                if (lAlphaFieldBlanks(3)) {
                    fault.severitySched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
                } else if ((fault.severitySched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    state.dataFaultsMgr->ErrorsFound = true;
                }

                fault.ControllerType = cAlphaArgs(4);
                // check controller type
                if (lAlphaFieldBlanks(4)) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(4), cAlphaArgs(4)));
                    state.dataFaultsMgr->ErrorsFound = true;
                } else {
                    if (Util::makeUPPER(cAlphaArgs(4)) == "CONTROLLER:OUTDOORAIR") {
                        fault.ControllerTypeEnum = iController_AirEconomizer;

                        // CASE ...

                    } else {
                    }
                }

                state.dataFaultsMgr->FaultsEconomizer(j).ControllerName = cAlphaArgs(5);
                // check controller name
                if (lAlphaFieldBlanks(5)) {
                    ShowSevereError(
                        state,
                        std::format(
                            "{} = \"{}\" invalid {} = \"{}\" blank.", cFaultCurrentObject, cAlphaArgs(1), cAlphaFieldNames(5), cAlphaArgs(5)));
                    state.dataFaultsMgr->ErrorsFound = true;
                }

                // offset - degree of fault
                state.dataFaultsMgr->FaultsEconomizer(j).Offset = rNumericArgs(1);
            }
        }

        state.dataFaultsMgr->RunFaultMgrOnceFlag = true;

        if (state.dataFaultsMgr->ErrorsFound) {
            ShowFatalError(state, "CheckAndReadFaults: Errors found in getting FaultModel input data. Preceding condition(s) cause termination.");
        }
    }

    Real64 FaultProperties::CalFaultOffsetAct([[maybe_unused]] EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Jun. 2016

        // PURPOSE OF THIS SUBROUTINE:
        //       To calculate the dynamic fault offset based on the fault availability schedule and severity schedule.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FaultFac(0.0); // fault modification factor

        // Check fault availability schedules
        if (this->availSched->getCurrentVal() > 0.0) {

            // Check fault severity schedules
            FaultFac = (this->severitySched != nullptr) ? this->severitySched->getCurrentVal() : 1.0;
        }

        return FaultFac * this->Offset;
    }

    Real64 FaultPropertiesFouling::CalFoulingFactor([[maybe_unused]] EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Nov. 2016

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the dynamic Nominal Capacity or Efficiency Reduction due to fouling, based on the fault availability schedule and severity
        // schedule. The factor is the ratio between the nominal capacity or efficiency at fouling case and that at fault free case

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FaultFac(0.0); // fault modification factor
        Real64 foulingFactorLocal(
            1.0); // Actual Nominal Fouling Factor, ratio between the nominal capacity or efficiency at fouling case and that at fault free case

        // Check fault availability schedules
        if (this->availSched->getCurrentVal() > 0.0) {

            // Check fault severity schedules
            FaultFac = (this->severitySched != nullptr) ? this->severitySched->getCurrentVal() : 1.0;
        }

        // The more severe the fouling fault is (i.e., larger FaultFac), the less the FoulingFactor is
        if (FaultFac > 0.0) {
            foulingFactorLocal = min(this->FoulingFactor / FaultFac, 1.0);
        }

        return foulingFactorLocal;
    }

    Real64 FaultPropertiesTowerFouling::CalFaultyTowerFoulingFactor([[maybe_unused]] EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Jul. 2016

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the dynamic tower fouling factor based on the fault availability schedule and severity schedule.
        // Fouling factor is the ratio between the UA value at fouling case and that at fault free case

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FaultFac(0.0);             // fault modification factor
        Real64 UAReductionFactorAct(1.0); // actual UA Reduction Factor, ratio between the UA value at fouling case and that at fault free case

        // Check fault availability schedules
        if (this->availSched->getCurrentVal() > 0.0) {

            // Check fault severity schedules
            FaultFac = (this->severitySched != nullptr) ? this->severitySched->getCurrentVal() : 1.0;
        }

        // The more severe the fouling fault is (i.e., larger FaultFac), the less the UAReductionFactor is
        if (FaultFac > 0.0) {
            UAReductionFactorAct = min(this->UAReductionFactor / FaultFac, 1.0);
        }

        return UAReductionFactorAct;
    }

    Real64 FaultPropertiesFoulingCoil::FaultFraction([[maybe_unused]] EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Julien Marrec, EffiBEM
        //       DATE WRITTEN   Feb. 2020

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the Fault Fraction based on Availability and Severity Schedules

        // Check fault availability schedules
        if (this->availSched->getCurrentVal() > 0.0) {

            // Check fault severity schedules (Ptr initialized to -1, so would return a FaultFrac of 1 if not set)
            return this->severitySched->getCurrentVal();
        }

        return 0.0;
    }

    void FaultPropertiesChillerSWT::CalFaultChillerSWT(bool FlagVariableFlow, // True if chiller is variable flow and false if it is constant flow
                                                       Real64 FaultyChillerSWTOffset, // Faulty chiller SWT sensor offset
                                                       Real64 Cp,                     // Local fluid specific heat
                                                       Real64 EvapInletTemp,          // Chiller evaporator inlet water temperature
                                                       Real64 &EvapOutletTemp,        // Chiller evaporator outlet water temperature
                                                       Real64 &EvapMassFlowRate,      // Chiller mass flow rate
                                                       Real64 &QEvaporator            // Chiller evaporator heat transfer rate
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Jun. 2016

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the mass flow rate and supply water temperature of a chiller with faulty SWT sensor.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // Variables for fault free cases
        Real64 EvapOutletTemp_ff = EvapOutletTemp;     // Chiller supply water temperature, fault free [C]
        Real64 EvapMassFlowRate_ff = EvapMassFlowRate; // Chiller mass flow rate, fault free [kg/s]
        Real64 QEvaporator_ff = QEvaporator;           // Chiller evaporator heat transfer rate, fault free [W]

        // Variables for faulty cases
        Real64 EvapOutletTemp_f;                         // Chiller supply water temperature, faulty case [C]
        Real64 EvapMassFlowRate_f = EvapMassFlowRate_ff; // Chiller mass flow rate, faulty case [kg/s]
        Real64 QEvaporator_f;                            // Chiller evaporator heat transfer rate, faulty case [W]

        if (!FlagVariableFlow) {
            // Chillers with ConstantFlow mode

            EvapOutletTemp_f = EvapOutletTemp_ff - FaultyChillerSWTOffset;

            if ((EvapInletTemp > EvapOutletTemp_f) && (EvapMassFlowRate_ff > 0)) {
                QEvaporator_f = EvapMassFlowRate_ff * Cp * (EvapInletTemp - EvapOutletTemp_f);
            } else {
                EvapMassFlowRate_f = 0.0;
                QEvaporator_f = 0.0;
            }

        } else {
            // Chillers with LeavingSetpointModulated mode

            EvapOutletTemp_f = EvapOutletTemp_ff - FaultyChillerSWTOffset;

            if ((EvapInletTemp > EvapOutletTemp_f) && (Cp > 0) && (EvapMassFlowRate_ff > 0)) {
                EvapMassFlowRate_f = QEvaporator_ff / Cp / (EvapInletTemp - EvapOutletTemp_ff);
                QEvaporator_f = EvapMassFlowRate_f * Cp * (EvapInletTemp - EvapOutletTemp_f);
            } else {
                EvapMassFlowRate_f = 0.0;
                QEvaporator_f = 0.0;
            }
        }

        // Return variables
        EvapOutletTemp = EvapOutletTemp_f;
        EvapMassFlowRate = EvapMassFlowRate_f;
        QEvaporator = QEvaporator_f;
    }

    bool FaultPropertiesAirFilter::CheckFaultyAirFilterFanCurve(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rongpeng Zhang
        //       DATE WRITTEN   Apr. 2015

        // PURPOSE OF THIS SUBROUTINE:
        // To check whether the fan curve specified in the FaultModel:Fouling:AirFilter object
        // covers the rated operational point of the corresponding fan
        // Return true if the curve covers the fan rated operational point
        auto *fan = state.dataFans->fans(this->fanNum);
        Real64 deltaPressCal = Curve::CurveValue(state, this->fanCurveNum, fan->maxAirFlowRate);
        return ((deltaPressCal > 0.95 * fan->deltaPress) && (deltaPressCal < 1.05 * fan->deltaPress));
    }

    void SetFaultyCoilSATSensor(
        EnergyPlusData &state, std::string const &CompType, std::string_view CompName, bool &FaultyCoilSATFlag, int &FaultyCoilSATIndex)
    {

        FaultyCoilSATFlag = false;
        FaultyCoilSATIndex = 0;
        if (state.dataFaultsMgr->NumFaultyCoilSATSensor == 0) {
            return;
        }
        for (int jFault_CoilSAT = 1; jFault_CoilSAT <= state.dataFaultsMgr->NumFaultyCoilSATSensor; ++jFault_CoilSAT) {
            if (Util::SameString(state.dataFaultsMgr->FaultsCoilSATSensor(jFault_CoilSAT).CoilType, CompType) &&
                Util::SameString(state.dataFaultsMgr->FaultsCoilSATSensor(jFault_CoilSAT).CoilName, CompName)) {
                FaultyCoilSATFlag = true;
                FaultyCoilSATIndex = jFault_CoilSAT;
                break;
            }
        }
    }

} // namespace FaultsManager

} // namespace EnergyPlus
