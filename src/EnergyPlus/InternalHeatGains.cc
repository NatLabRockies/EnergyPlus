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
#include <cmath>
#include <map>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/FuelCellElectricGenerator.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/PipeHeatTransfer.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WaterUse.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace InternalHeatGains {
    // Module containing the routines dealing with the internal heat gains

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000
    //       MODIFIED       Aug 2005, PGE (Added object names and report variables)
    //                      Feb 2006, PGE (Added end-use subcategories)
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Part of the heat balance modularization/re-engineering.  Purpose of this
    // module is to contain the internal heat gain routines in a single location.

    // METHODOLOGY EMPLOYED:
    // Routines are called as subroutines to supply the data-only module structures
    // with the proper values.

    // REFERENCES:
    // Legacy BLAST code

    // OTHER NOTES: none

    // Using/Aliasing
    using namespace DataEnvironment;
    using namespace DataHeatBalance;
    using namespace DataSurfaces;

    static constexpr std::array<DataHeatBalance::IntGainType, 1> IntGainTypesPeople = {DataHeatBalance::IntGainType::People};
    static constexpr std::array<DataHeatBalance::IntGainType, 1> IntGainTypesLight = {DataHeatBalance::IntGainType::Lights};
    static constexpr std::array<DataHeatBalance::IntGainType, 7> IntGainTypesEquip = {DataHeatBalance::IntGainType::ElectricEquipment,
                                                                                      DataHeatBalance::IntGainType::ElectricEquipmentITEAirCooled,
                                                                                      DataHeatBalance::IntGainType::GasEquipment,
                                                                                      DataHeatBalance::IntGainType::HotWaterEquipment,
                                                                                      DataHeatBalance::IntGainType::SteamEquipment,
                                                                                      DataHeatBalance::IntGainType::OtherEquipment,
                                                                                      DataHeatBalance::IntGainType::IndoorGreen};
    static constexpr std::array<DataHeatBalance::IntGainType, 10> IntGainTypesRefrig = {
        DataHeatBalance::IntGainType::RefrigerationCase,
        DataHeatBalance::IntGainType::RefrigerationCompressorRack,
        DataHeatBalance::IntGainType::RefrigerationSystemAirCooledCondenser,
        DataHeatBalance::IntGainType::RefrigerationSystemSuctionPipe,
        DataHeatBalance::IntGainType::RefrigerationSecondaryReceiver,
        DataHeatBalance::IntGainType::RefrigerationSecondaryPipe,
        DataHeatBalance::IntGainType::RefrigerationWalkIn,
        DataHeatBalance::IntGainType::RefrigerationTransSysAirCooledGasCooler,
        DataHeatBalance::IntGainType::RefrigerationTransSysSuctionPipeMT,
        DataHeatBalance::IntGainType::RefrigerationTransSysSuctionPipeLT};
    static constexpr std::array<DataHeatBalance::IntGainType, 3> IntGainTypesWaterUse = {DataHeatBalance::IntGainType::WaterUseEquipment,
                                                                                         DataHeatBalance::IntGainType::WaterHeaterMixed,
                                                                                         DataHeatBalance::IntGainType::WaterHeaterStratified};
    static constexpr std::array<DataHeatBalance::IntGainType, 20> IntGainTypesHvacLoss = {
        DataHeatBalance::IntGainType::ZoneBaseboardOutdoorTemperatureControlled,
        DataHeatBalance::IntGainType::ThermalStorageChilledWaterMixed,
        DataHeatBalance::IntGainType::ThermalStorageChilledWaterStratified,
        DataHeatBalance::IntGainType::PipeIndoor,
        DataHeatBalance::IntGainType::Pump_VarSpeed,
        DataHeatBalance::IntGainType::Pump_ConSpeed,
        DataHeatBalance::IntGainType::Pump_Cond,
        DataHeatBalance::IntGainType::PumpBank_VarSpeed,
        DataHeatBalance::IntGainType::PumpBank_ConSpeed,
        DataHeatBalance::IntGainType::PlantComponentUserDefined,
        DataHeatBalance::IntGainType::CoilUserDefined,
        DataHeatBalance::IntGainType::ZoneHVACForcedAirUserDefined,
        DataHeatBalance::IntGainType::AirTerminalUserDefined,
        DataHeatBalance::IntGainType::PackagedTESCoilTank,
        DataHeatBalance::IntGainType::FanSystemModel,
        DataHeatBalance::IntGainType::SecCoolingDXCoilSingleSpeed,
        DataHeatBalance::IntGainType::SecHeatingDXCoilSingleSpeed,
        DataHeatBalance::IntGainType::SecCoolingDXCoilTwoSpeed,
        DataHeatBalance::IntGainType::SecCoolingDXCoilMultiSpeed,
        DataHeatBalance::IntGainType::SecHeatingDXCoilMultiSpeed};
    static constexpr std::array<DataHeatBalance::IntGainType, 10> IntGainTypesPowerGen = {
        DataHeatBalance::IntGainType::GeneratorFuelCell,
        DataHeatBalance::IntGainType::GeneratorMicroCHP,
        DataHeatBalance::IntGainType::ElectricLoadCenterTransformer,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterSimple,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterFunctionOfPower,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterLookUpTable,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageLiIonNmcBattery,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageBattery,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageSimple,
        DataHeatBalance::IntGainType::ElectricLoadCenterConverter};
    // Explicitly list internal gains not gathered here
    [[maybe_unused]] static constexpr std::array<DataHeatBalance::IntGainType, 3> ExcludedIntGainTypes = {
        DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkCarbonDioxide,
        DataHeatBalance::IntGainType::DaylightingDeviceTubular,
        DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam};

    static constexpr std::array<std::string_view, static_cast<int>(DesignLevelMethod::Num)> DesignLevelMethodNamesUC = {
        "PEOPLE", "PEOPLE/AREA", "AREA/PERSON", "LIGHTINGLEVEL", "EQUIPMENTLEVEL", "WATTS/AREA", "WATTS/PERSON", "POWER/AREA", "POWER/PERSON"};

    void ManageInternalHeatGains(EnergyPlusData &state,
                                 ObjexxFCL::Optional_bool_const InitOnly) // when true, just calls the get input, if appropriate and returns.
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Mar 2004, FCW: move call to DayltgElecLightingControl from InitSurfaceHeatBalance
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This is the main driver subroutine for the internal heat gains.

        if (state.dataInternalHeatGains->GetInternalHeatGainsInputFlag) {
            GetInternalHeatGainsInput(state);
            state.dataInternalHeatGains->GetInternalHeatGainsInputFlag = false;
        }

        if (present(InitOnly)) {
            if (InitOnly) {
                return;
            }
        }

        InitInternalHeatGains(state);

        ReportInternalHeatGains(state);

        CheckReturnAirHeatGain(state);

        // for the load component report, gather the load components for each timestep but not when doing pulse
        if (state.dataGlobal->ZoneSizingCalc) {
            GatherComponentLoadsIntGain(state);
        }
    }

    void GetInternalHeatGainsInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997
        //       MODIFIED       September 1998, FW
        //                      May 2009, BG: added calls to setup for possible EMS override
        //       RE-ENGINEERED  August 2000, RKS

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the Internal Heat Gain Data for the Zones.
        // Sets up the various data that will be used later with the
        // schedulemanager to determine the actual values.

        // METHODOLOGY EMPLOYED:
        // The GetObjectItem routines are employed to retrieve the data.

        // REFERENCES:
        // IDD Objects:
        // People
        // Lights
        // ElectricEquipment
        // GasEquipment
        // SteamEquipment
        // HotWaterEquipment
        // OtherEquipment
        // ElectricEquipment:ITE:AirCooled
        // ZoneBaseboard:OutdoorTemperatureControlled

        // Using/Aliasing
        using namespace OutputReportPredefined;
        using namespace DataLoopNode;
        using Curve::GetCurveIndex;
        using NodeInputManager::GetOnlySingleNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetInternalHeatGains: ");
        static constexpr std::string_view routineName = "GetInternalHeatGains";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IOStat;

        // Formats
        static constexpr std::string_view Format_720(" Zone Internal Gains Nominal, {},{:.2R},{:.1R},");
        static constexpr std::string_view Format_722(" {} Internal Gains Nominal, {},{},{},{:.2R},{:.1R},");
        static constexpr std::string_view Format_723(
            "! <{} Internal Gains Nominal>,Name,Schedule Name,Zone Name,Zone Floor Area {{m2}},# Zone Occupants,{}");
        static constexpr std::string_view Format_724(" {}, {}\n");

        auto print_and_divide_if_greater_than_zero = [&](const Real64 numerator, const Real64 denominator) {
            if (denominator > 0.0) {
                print(state.files.eio, "{:.3R},", numerator / denominator);
            } else {
                print(state.files.eio, "N/A,");
            }
        };

        auto &ErrorsFound(state.dataInternalHeatGains->ErrorsFound);

        // TODO MJW: Punt for now, sometimes unit test need these to be allocated in AllocateZoneHeatBalArrays, but simulations need them here
        if (!state.dataHeatBal->ZoneIntGain.allocated()) {
            DataHeatBalance::AllocateIntGains(state);
        }
        state.dataHeatBal->ZoneRpt.allocate(state.dataGlobal->NumOfZones);
        state.dataHeatBal->spaceRpt.allocate(state.dataGlobal->numSpaces);
        state.dataHeatBal->ZoneIntEEuse.allocate(state.dataGlobal->NumOfZones);
        state.dataHeatBal->RefrigCaseCredit.allocate(state.dataGlobal->NumOfZones);

        Array1D_bool RepVarSet;
        RepVarSet.allocate(state.dataGlobal->NumOfZones);
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            RepVarSet(zoneNum) = true;
        }

        const std::string peopleModuleObject = "People";
        const std::string lightsModuleObject = "Lights";
        const std::string elecEqModuleObject = "ElectricEquipment";
        const std::string gasEqModuleObject = "GasEquipment";
        const std::string hwEqModuleObject = "HotWaterEquipment";
        const std::string stmEqModuleObject = "SteamEquipment";
        const std::string othEqModuleObject = "OtherEquipment";
        const std::string itEqModuleObject = "ElectricEquipment:ITE:AirCooled";
        const std::string bbModuleObject = "ZoneBaseboard:OutdoorTemperatureControlled";
        const std::string contamSSModuleObject = "ZoneContaminantSourceAndSink:CarbonDioxide";

        // Because there are occasions where getObjectItem will be called a second time within the routine (#9680)
        // We should use local arrays instead of state.dataIPShortCut
        int IHGNumAlphas = 0;
        int IHGNumNumbers = 0;
        Array1D<Real64> IHGNumbers;
        Array1D_string IHGAlphas;
        Array1D_bool IHGNumericFieldBlanks;
        Array1D_bool IHGAlphaFieldBlanks;
        Array1D_string IHGAlphaFieldNames;
        Array1D_string IHGNumericFieldNames;

        {
            int MaxAlphas = 0;
            int MaxNums = 0;
            int NumParams = 0;
            for (const auto &moduleName : {peopleModuleObject,
                                           lightsModuleObject,
                                           elecEqModuleObject,
                                           gasEqModuleObject,
                                           hwEqModuleObject,
                                           stmEqModuleObject,
                                           othEqModuleObject,
                                           itEqModuleObject,
                                           bbModuleObject,
                                           contamSSModuleObject}) {
                state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, moduleName, NumParams, IHGNumAlphas, IHGNumNumbers);
                MaxAlphas = std::max(MaxAlphas, IHGNumAlphas);
                MaxNums = std::max(MaxNums, IHGNumNumbers);
            }
            IHGAlphas.allocate(MaxAlphas);
            IHGAlphaFieldNames.allocate(MaxAlphas);
            IHGAlphaFieldBlanks.dimension(MaxAlphas, true);

            IHGNumbers.dimension(MaxNums, 0.0);
            IHGNumericFieldNames.allocate(MaxNums);
            IHGNumericFieldBlanks.dimension(MaxNums, true);
            IHGNumAlphas = 0;
            IHGNumNumbers = 0;
        }

        // PEOPLE: Includes both information related to the heat balance and thermal comfort
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> peopleObjects;
        int numPeopleStatements = 0;
        setupIHGZonesAndSpaces(state, peopleModuleObject, peopleObjects, numPeopleStatements, state.dataHeatBal->TotPeople, ErrorsFound);

        if (state.dataHeatBal->TotPeople > 0) {
            state.dataHeatBal->People.allocate(state.dataHeatBal->TotPeople);
            int peopleNum = 0;
            for (int peopleInputNum = 1; peopleInputNum <= numPeopleStatements; ++peopleInputNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         peopleModuleObject,
                                                                         peopleInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, peopleModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(3));
                if (IHGAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisPeopleInput = peopleObjects(peopleInputNum);
                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(4)));
                int fieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::People: {
                    fieldNum = 1;
                } break;
                case DesignLevelMethod::PeoplePerArea: {
                    fieldNum = 2;
                } break;
                case DesignLevelMethod::AreaPerPerson: {
                    fieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(fieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(fieldNum);
                std::string_view const levelField = IHGNumericFieldNames(fieldNum);

                // Create one People instance for every space associated with this People input object
                for (int Item1 = 1; Item1 <= thisPeopleInput.numOfSpaces; ++Item1) {
                    ++peopleNum;
                    auto &thisPeople = state.dataHeatBal->People(peopleNum);
                    int const spaceNum = thisPeopleInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisPeople.Name = thisPeopleInput.names(Item1);
                    thisPeople.spaceIndex = spaceNum;
                    thisPeople.ZonePtr = zoneNum;
                    thisPeople.sched = schedPtr;

                    // Number of people calculation method.
                    thisPeople.NumberOfPeople = setDesignLevel(
                        state, ErrorsFound, peopleModuleObject, thisPeopleInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Calculate nominal min/max people
                    thisPeople.NomMinNumberPeople = thisPeople.NumberOfPeople * thisPeople.sched->getMinVal(state);
                    thisPeople.NomMaxNumberPeople = thisPeople.NumberOfPeople * thisPeople.sched->getMaxVal(state);

                    if (zoneNum > 0) {
                        state.dataHeatBal->Zone(zoneNum).TotOccupants += thisPeople.NumberOfPeople;
                        // Note that min/max occupants are non-coincident
                        state.dataHeatBal->Zone(zoneNum).minOccupants += thisPeople.NomMinNumberPeople;
                        state.dataHeatBal->Zone(zoneNum).maxOccupants += thisPeople.NomMaxNumberPeople;
                    }

                    if (spaceNum > 0) {
                        state.dataHeatBal->space(spaceNum).TotOccupants += thisPeople.NumberOfPeople;
                        // Note that min/max occupants are non-coincident
                        state.dataHeatBal->space(spaceNum).minOccupants += thisPeople.NomMinNumberPeople;
                        state.dataHeatBal->space(spaceNum).maxOccupants += thisPeople.NomMaxNumberPeople;
                    }
                    thisPeople.FractionRadiant = IHGNumbers(4);
                    thisPeople.FractionConvected = 1.0 - thisPeople.FractionRadiant;
                    if (Item1 == 1) {
                        if (thisPeople.FractionConvected < 0.0) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", {} < 0.0, value ={:.2R}",
                                                   RoutineName,
                                                   peopleModuleObject,
                                                   IHGAlphas(1),
                                                   IHGNumericFieldNames(4),
                                                   IHGNumbers(4)));
                            ErrorsFound = true;
                        }
                    }

                    if (IHGNumNumbers >= 5 && !IHGNumericFieldBlanks(5)) {
                        thisPeople.UserSpecSensFrac = IHGNumbers(5);
                    } else {
                        thisPeople.UserSpecSensFrac = Constant::AutoCalculate;
                    }

                    if (IHGNumNumbers >= 6 && !IHGNumericFieldBlanks(6)) {
                        thisPeople.CO2RateFactor = IHGNumbers(6);
                    } else {
                        thisPeople.CO2RateFactor = 3.82e-8; // m3/s-W
                    }

                    if (IHGNumNumbers >= 7 && !IHGNumericFieldBlanks(7)) {
                        thisPeople.ColdStressTempThresh = IHGNumbers(7);
                    } else {
                        thisPeople.ColdStressTempThresh = 15.56; // degree C
                    }

                    if (IHGNumNumbers == 8 && !IHGNumericFieldBlanks(8)) {
                        thisPeople.HeatStressTempThresh = IHGNumbers(8);
                    } else {
                        thisPeople.HeatStressTempThresh = 30.0; // degree C
                    }

                    if (thisPeople.CO2RateFactor < 0.0) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", {} < 0.0, value ={:.2R}",
                                               RoutineName,
                                               peopleModuleObject,
                                               IHGAlphas(1),
                                               IHGNumericFieldNames(6),
                                               IHGNumbers(6)));
                        ErrorsFound = true;
                    }

                    thisPeople.activityLevelSched = Sched::GetSchedule(state, IHGAlphas(5));

                    if (Item1 == 1) {
                        if (IHGAlphaFieldBlanks(5)) {
                            ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(5));
                            ErrorsFound = true;
                        } else if (thisPeople.activityLevelSched == nullptr) {
                            ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(5), IHGAlphas(5));
                            ErrorsFound = true;
                        } else if (!thisPeople.activityLevelSched->checkMinVal(state, Clusive::In, 0.0)) {
                            Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(5), IHGAlphas(5), Clusive::In, 0.0);
                            ErrorsFound = true;
                        } else if (!thisPeople.activityLevelSched->checkMinMaxVals(state, Clusive::In, 70.0, Clusive::In, 1000.0)) {
                            Sched::ShowWarningBadMinMax(state,
                                                        eoh,
                                                        IHGAlphaFieldNames(5),
                                                        IHGAlphas(5),
                                                        Clusive::In,
                                                        70.0,
                                                        Clusive::In,
                                                        1000.0,
                                                        "Values fall outside of typical w/person range for thermal comfort reporting.");
                        }
                    }

                    // Following is an optional parameter (ASHRAE 55 warnings
                    if (IHGNumAlphas >= 6) {
                        if (BooleanSwitch bs = getYesNoValue(IHGAlphas(6)); bs != BooleanSwitch::Invalid) {
                            thisPeople.Show55Warning = static_cast<bool>(bs);
                        } else if (Item1 == 1) {
                            ShowSevereInvalidKey(state, eoh, IHGAlphaFieldNames(6), IHGAlphas(6));
                            ErrorsFound = true;
                        }
                    }

                    if (IHGNumAlphas > 6) { // Optional parameters present--thermal comfort data follows...
                        int lastOption = 0;
                        bool usingThermalComfort = false;
                        if (IHGNumAlphas > 20) {
                            lastOption = 20;
                        } else {
                            lastOption = IHGNumAlphas;
                        }

                        // check to see if the user has specified schedules for air velocity, clothing insulation, and/or work efficiency
                        // but have NOT made a selection for a thermal comfort model.  If so, then the schedules are reported as unused
                        // which could cause confusion.  The solution is for the user to either remove those schedules or pick a thermal
                        // comfort model.
                        int constexpr NumFirstTCModel = 14;
                        if (IHGNumAlphas < NumFirstTCModel) {
                            bool NoTCModelSelectedWithSchedules = false;
                            NoTCModelSelectedWithSchedules =
                                CheckThermalComfortSchedules(IHGAlphaFieldBlanks(9), IHGAlphaFieldBlanks(12), IHGAlphaFieldBlanks(13));
                            if (NoTCModelSelectedWithSchedules) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\" has comfort related schedules but no thermal comfort model selected.",
                                                        RoutineName,
                                                        peopleModuleObject,
                                                        IHGAlphas(1)));
                                ShowContinueError(state,
                                                  "If schedules are specified for air velocity, clothing insulation, and/or work efficiency but no "
                                                  "thermal comfort");
                                ShowContinueError(
                                    state, "thermal comfort model is selected, the schedules will be listed as unused schedules in the .err file.");
                                ShowContinueError(
                                    state,
                                    "To avoid these errors, select a valid thermal comfort model or eliminate these schedules in the PEOPLE input.");
                            }
                        }

                        for (int OptionNum = NumFirstTCModel; OptionNum <= lastOption; ++OptionNum) {

                            { // Why are we starting a nested scope immediately after opening up a scope?
                                std::string const &thermalComfortType = IHGAlphas(OptionNum);

                                if (thermalComfortType == "FANGER") {
                                    thisPeople.Fanger = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "PIERCE") {
                                    thisPeople.Pierce = true;
                                    state.dataHeatBal->AnyThermalComfortPierceModel = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "KSU") {
                                    thisPeople.KSU = true;
                                    state.dataHeatBal->AnyThermalComfortKSUModel = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "ADAPTIVEASH55") {
                                    thisPeople.AdaptiveASH55 = true;
                                    state.dataHeatBal->AdaptiveComfortRequested_ASH55 = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "ADAPTIVECEN15251") {
                                    thisPeople.AdaptiveCEN15251 = true;
                                    state.dataHeatBal->AdaptiveComfortRequested_CEN15251 = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "COOLINGEFFECTASH55") {
                                    thisPeople.CoolingEffectASH55 = true;
                                    state.dataHeatBal->AnyThermalComfortCoolingEffectModel = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "ANKLEDRAFTASH55") {
                                    thisPeople.AnkleDraftASH55 = true;
                                    state.dataHeatBal->AnyThermalComfortAnkleDraftModel = true;
                                    usingThermalComfort = true;

                                } else if (thermalComfortType == "") { // Blank input field--just ignore this

                                } else { // An invalid keyword was entered--warn but ignore
                                    if (Item1 == 1) {
                                        ShowWarningInvalidKey(state, eoh, IHGAlphaFieldNames(OptionNum), IHGAlphas(OptionNum), "");
                                        ShowContinueError(state,
                                                          "Valid Values are \"Fanger\", \"Pierce\", \"KSU\", \"AdaptiveASH55\", "
                                                          "\"AdaptiveCEN15251\", \"CoolingEffectASH55\", \"AnkleDraftASH55\"");
                                    }
                                }
                            }
                        }

                        if (usingThermalComfort) {

                            // Set the default value of MRTCalcType as 'EnclosureAveraged'
                            thisPeople.MRTCalcType = DataHeatBalance::CalcMRT::EnclosureAveraged;

                            bool ModelWithAdditionalInputs = thisPeople.Fanger || thisPeople.Pierce || thisPeople.KSU ||
                                                             thisPeople.CoolingEffectASH55 || thisPeople.AnkleDraftASH55;

                            // MRT Calculation Type and Surface Name
                            thisPeople.MRTCalcType = static_cast<CalcMRT>(getEnumValue(CalcMRTTypeNamesUC, IHGAlphas(7)));

                            switch (thisPeople.MRTCalcType) {
                            case DataHeatBalance::CalcMRT::EnclosureAveraged: {
                                // nothing to do here
                            } break;
                            case DataHeatBalance::CalcMRT::SurfaceWeighted: {
                                thisPeople.SurfacePtr = Util::FindItemInList(IHGAlphas(8), state.dataSurface->Surface);
                                if (thisPeople.SurfacePtr == 0 && ModelWithAdditionalInputs) {
                                    if (Item1 == 1) {
                                        ShowSevereError(state,
                                                        format("{}{}=\"{}\", {}={} invalid Surface Name={}",
                                                               RoutineName,
                                                               peopleModuleObject,
                                                               IHGAlphas(1),
                                                               IHGAlphaFieldNames(7),
                                                               IHGAlphas(7),
                                                               IHGAlphas(8)));
                                        ErrorsFound = true;
                                    }
                                } else {
                                    int const surfRadEnclNum = state.dataSurface->Surface(thisPeople.SurfacePtr).RadEnclIndex;
                                    int const thisPeopleRadEnclNum = state.dataHeatBal->space(thisPeople.spaceIndex).radiantEnclosureNum;
                                    if (surfRadEnclNum != thisPeopleRadEnclNum && ModelWithAdditionalInputs) {
                                        ShowSevereError(state,
                                                        format("{}{}=\"{}\", Surface referenced in {}={} in different enclosure.",
                                                               RoutineName,
                                                               peopleModuleObject,
                                                               IHGAlphas(1),
                                                               IHGAlphaFieldNames(7),
                                                               IHGAlphas(7)));
                                        ShowContinueError(state,
                                                          format("Surface is in Enclosure={} and {} is in Enclosure={}",
                                                                 state.dataViewFactor->EnclRadInfo(surfRadEnclNum).Name,
                                                                 peopleModuleObject,
                                                                 state.dataViewFactor->EnclRadInfo(thisPeopleRadEnclNum).Name));
                                        ErrorsFound = true;
                                    }
                                }

                            } break;
                            case DataHeatBalance::CalcMRT::AngleFactor: {
                                thisPeople.AngleFactorListName = IHGAlphas(8);

                            } break;
                            default: { // An invalid keyword was entered--warn but ignore
                                if (Item1 == 1 && ModelWithAdditionalInputs) {
                                    ShowWarningError(state,
                                                     format("{}{}=\"{}\", invalid {}={}",
                                                            RoutineName,
                                                            peopleModuleObject,
                                                            IHGAlphas(1),
                                                            IHGAlphaFieldNames(7),
                                                            IHGAlphas(7)));
                                    ShowContinueError(state, "...Valid values are \"EnclosureAveraged\", \"SurfaceWeighted\", \"AngleFactor\".");
                                }
                            } break;
                            } // switch (thisPeople.MRTCalcType)

                            if (!IHGAlphaFieldBlanks(9)) {
                                thisPeople.workEffSched = Sched::GetSchedule(state, IHGAlphas(9));
                            }

                            if (Item1 == 1) {
                                if (IHGAlphaFieldBlanks(9)) {
                                    if (ModelWithAdditionalInputs) {
                                        ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(9));
                                        ShowContinueError(state,
                                                          "It is required when Thermal Comfort Model Type is one of "
                                                          "\"Fanger\", \"Pierce\", \"KSU\", \"CoolingEffectASH55\" or \"AnkleDraftASH55\"");
                                        ErrorsFound = true;
                                    }
                                } else if (thisPeople.workEffSched == nullptr) {
                                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(9), IHGAlphas(9));
                                    ErrorsFound = true;
                                } else if (!thisPeople.workEffSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                                    Sched::ShowSevereBadMinMax(state, eoh, IHGAlphaFieldNames(9), IHGAlphas(9), Clusive::In, 0.0, Clusive::In, 1.0);
                                    ErrorsFound = true;
                                }
                            }

                            if (IHGAlphas(10).empty()) { // Using IHGAlphaFieldBlanks(10) doesn't work because this value is defaulted
                            } else if ((thisPeople.clothingType = static_cast<ClothingType>(getEnumValue(clothingTypeNamesUC, IHGAlphas(10)))) ==
                                       ClothingType::Invalid) {
                                ShowSevereInvalidKey(state, eoh, IHGAlphaFieldNames(10), IHGAlphas(10));
                                ErrorsFound = true;

                            } else {

                                switch (thisPeople.clothingType) {

                                case ClothingType::InsulationSchedule: {

                                    thisPeople.clothingSched = Sched::GetSchedule(state, IHGAlphas(12));
                                    if (Item1 == 1) {
                                        if (IHGAlphaFieldBlanks(12)) {
                                            if (ModelWithAdditionalInputs) {
                                                ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(12), IHGAlphaFieldNames(10), IHGAlphas(10));
                                                ErrorsFound = true;
                                            }
                                        } else if (thisPeople.clothingSched == nullptr) {
                                            if (ModelWithAdditionalInputs) {
                                                ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(12), IHGAlphas(12));
                                                ErrorsFound = true;
                                            }
                                        } else if (!thisPeople.clothingSched->checkMinVal(state, Clusive::In, 0.0)) {
                                            Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(12), IHGAlphas(12), Clusive::In, 0.0);
                                            ErrorsFound = true;
                                        } else if (!thisPeople.clothingSched->checkMaxVal(state, Clusive::In, 2.0)) {
                                            Sched::ShowWarningBadMax(state, eoh, IHGAlphaFieldNames(12), IHGAlphas(12), Clusive::In, 2.0, "");
                                        }
                                    }
                                } break;

                                case ClothingType::DynamicAshrae55: {
                                } break; // nothing extra to do, at least for now

                                case ClothingType::CalculationSchedule: {
                                    thisPeople.clothingMethodSched = Sched::GetSchedule(state, IHGAlphas(11));

                                    if (Item1 == 1) {
                                        if (thisPeople.clothingMethodSched == nullptr) {
                                            ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(11), IHGAlphas(11));
                                            ErrorsFound = true;
                                        }
                                    }

                                    if (thisPeople.clothingMethodSched->hasVal(state, 1)) {
                                        if ((thisPeople.clothingSched = Sched::GetSchedule(state, IHGAlphas(12))) == nullptr) {
                                            if (Item1 == 1) {
                                                ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(12), IHGAlphas(12));
                                                ErrorsFound = true;
                                            }
                                        }
                                    }
                                } break;

                                default: {
                                } break; // nothing to do for the other cases
                                } // switch (thisPeople.clothingType)
                            }

                            if (IHGAlphaFieldBlanks(13)) {
                            } else {
                                thisPeople.airVelocitySched = Sched::GetSchedule(state, IHGAlphas(13));
                            }

                            if (Item1 == 1) {
                                if (IHGAlphaFieldBlanks(13)) {
                                    if (ModelWithAdditionalInputs) {
                                        ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(13));
                                        ShowContinueError(state,
                                                          "Required when Thermal Comfort Model Type is one of "
                                                          "\"Fanger\", \"Pierce\", \"KSU\", \"CoolingEffectASH55\" or \"AnkleDraftASH55\"");
                                        ErrorsFound = true;
                                    }
                                } else if (thisPeople.airVelocitySched == nullptr) {
                                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(13), IHGAlphas(13));
                                    ErrorsFound = true;
                                } else if (!thisPeople.airVelocitySched->checkMinVal(state, Clusive::In, 0.0)) {
                                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(13), IHGAlphas(13), Clusive::In, 0.0);
                                    ErrorsFound = true;
                                }
                            }

                            if (IHGAlphas(21).empty()) { // Using IHGAlphaFieldBlanks(21) doesn't work because this field has a default
                            } else {
                                thisPeople.ankleAirVelocitySched = Sched::GetSchedule(state, IHGAlphas(21));
                            }

                            if (Item1 == 1) {
                                if (IHGAlphaFieldBlanks(21)) {
                                    if (thisPeople.AnkleDraftASH55) {
                                        ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(21), IHGAlphas(21));
                                        ShowContinueError(state,
                                                          "Required when Thermal Comfort Model Type is one of "
                                                          "\"Fanger\", \"Pierce\", \"KSU\", \"CoolingEffectASH55\" or \"AnkleDraftASH55\"");
                                        ErrorsFound = true;
                                    }
                                } else if (thisPeople.ankleAirVelocitySched == nullptr) {
                                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(21), IHGAlphas(21));
                                    ErrorsFound = true;
                                }
                            }
                        } // usingthermalcomfort block

                    } // ...end of thermal comfort data IF-THEN block  (IHGNumAlphass > 6)

                    if (thisPeople.ZonePtr <= 0) {
                        continue; // Error, will be caught and terminated later
                    }
                }
            }

            for (int peopleNum2 = 1; peopleNum2 <= state.dataHeatBal->TotPeople; ++peopleNum2) {
                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "People",
                                     state.dataHeatBal->People(peopleNum2).Name,
                                     "Number of People",
                                     "[each]",
                                     state.dataHeatBal->People(peopleNum2).EMSPeopleOn,
                                     state.dataHeatBal->People(peopleNum2).EMSNumberOfPeople);
                    SetupEMSInternalVariable(state,
                                             "People Count Design Level",
                                             state.dataHeatBal->People(peopleNum2).Name,
                                             "[each]",
                                             state.dataHeatBal->People(peopleNum2).NumberOfPeople);
                }

                // setup internal gains
                if (!ErrorsFound) {
                    SetupSpaceInternalGain(state,
                                           state.dataHeatBal->People(peopleNum2).spaceIndex,
                                           1.0,
                                           state.dataHeatBal->People(peopleNum2).Name,
                                           DataHeatBalance::IntGainType::People,
                                           &state.dataHeatBal->People(peopleNum2).ConGainRate,
                                           nullptr,
                                           &state.dataHeatBal->People(peopleNum2).RadGainRate,
                                           &state.dataHeatBal->People(peopleNum2).LatGainRate,
                                           nullptr,
                                           &state.dataHeatBal->People(peopleNum2).CO2GainRate);
                }
            }

            // transfer the nominal number of people in a zone to the tabular reporting
            for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
                if (state.dataHeatBal->Zone(Loop).TotOccupants > 0.0) {
                    if (state.dataHeatBal->Zone(Loop).FloorArea > 0.0 &&
                        state.dataHeatBal->Zone(Loop).FloorArea / state.dataHeatBal->Zone(Loop).TotOccupants < 0.1) {
                        ShowWarningError(
                            state, format("{}Zone=\"{}\" occupant density is extremely high.", RoutineName, state.dataHeatBal->Zone(Loop).Name));
                        if (state.dataHeatBal->Zone(Loop).FloorArea > 0.0) {
                            ShowContinueError(state,
                                              format("Occupant Density=[{:.0R}] person/m2.",
                                                     state.dataHeatBal->Zone(Loop).TotOccupants / state.dataHeatBal->Zone(Loop).FloorArea));
                        }
                        ShowContinueError(state,
                                          format("Occupant Density=[{:.3R}] m2/person. Problems in Temperature Out of Bounds may result.",
                                                 state.dataHeatBal->Zone(Loop).FloorArea / state.dataHeatBal->Zone(Loop).TotOccupants));
                    }
                    Real64 maxOccupLoad = 0.0;
                    int OptionNum = 0;
                    for (int Loop1 = 1; Loop1 <= state.dataHeatBal->TotPeople; ++Loop1) {
                        auto const &people = state.dataHeatBal->People(Loop1);
                        if (people.ZonePtr != Loop) {
                            continue;
                        }
                        if (maxOccupLoad < people.sched->getCurrentVal() * people.NumberOfPeople) {
                            maxOccupLoad = people.sched->getCurrentVal() * people.NumberOfPeople;
                            OptionNum = Loop1;
                        }
                    }
                    if (maxOccupLoad > state.dataHeatBal->Zone(Loop).TotOccupants) {
                        if (state.dataHeatBal->Zone(Loop).FloorArea > 0.0 && state.dataHeatBal->Zone(Loop).FloorArea / maxOccupLoad < 0.1) {
                            ShowWarningError(state,
                                             format("{}Zone=\"{}\" occupant density at a maximum schedule value is extremely high.",
                                                    RoutineName,
                                                    state.dataHeatBal->Zone(Loop).Name));
                            if (state.dataHeatBal->Zone(Loop).FloorArea > 0.0) {
                                ShowContinueError(
                                    state, format("Occupant Density=[{:.0R}] person/m2.", maxOccupLoad / state.dataHeatBal->Zone(Loop).FloorArea));
                            }
                            ShowContinueError(state,
                                              format("Occupant Density=[{:.3R}] m2/person. Problems in Temperature Out of Bounds may result.",
                                                     state.dataHeatBal->Zone(Loop).FloorArea / maxOccupLoad));
                            ShowContinueError(state,
                                              format("Check values in People={}, Number of People Schedule={}",
                                                     state.dataHeatBal->People(OptionNum).Name,
                                                     state.dataHeatBal->People(OptionNum).sched->getCurrentVal()));
                        }
                    }
                }

                if (state.dataHeatBal->Zone(Loop).isNominalControlled) { // conditioned zones only
                    if (state.dataHeatBal->Zone(Loop).TotOccupants > 0.0) {
                        state.dataHeatBal->Zone(Loop).isNominalOccupied = true;
                        PreDefTableEntry(state,
                                         state.dataOutRptPredefined->pdchOaoNomNumOcc1,
                                         state.dataHeatBal->Zone(Loop).Name,
                                         state.dataHeatBal->Zone(Loop).TotOccupants);
                        PreDefTableEntry(state,
                                         state.dataOutRptPredefined->pdchOaoNomNumOcc2,
                                         state.dataHeatBal->Zone(Loop).Name,
                                         state.dataHeatBal->Zone(Loop).TotOccupants);
                    }
                }
            }
        } // TotPeople > 0

        // Lights
        // Declared in state because the lights inputs are needed for demand manager
        int numLightsStatements = 0;
        Real64 sumArea = 0.0;  // sum of floor area for all lights objects
        Real64 sumPower = 0.0; // sum of power for all lights objects
        setupIHGZonesAndSpaces(
            state, lightsModuleObject, state.dataInternalHeatGains->lightsObjects, numLightsStatements, state.dataHeatBal->TotLights, ErrorsFound);

        if (state.dataHeatBal->TotLights > 0) {
            state.dataHeatBal->Lights.allocate(state.dataHeatBal->TotLights);
            bool CheckSharedExhaustFlag = false;
            int lightsNum = 0;
            for (int lightsInputNum = 1; lightsInputNum <= numLightsStatements; ++lightsInputNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         lightsModuleObject,
                                                                         lightsInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, lightsModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(3));
                if (IHGAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisLightsInput = state.dataInternalHeatGains->lightsObjects(lightsInputNum);

                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(4)));
                int fieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::LightingLevel: {
                    fieldNum = 1;
                } break;
                case DesignLevelMethod::WattsPerArea: {
                    fieldNum = 2;
                } break;
                case DesignLevelMethod::WattsPerPerson: {
                    fieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(fieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(fieldNum);
                std::string_view const levelField = IHGNumericFieldNames(fieldNum);

                // Create one Lights instance for every space associated with this Lights input object
                // Why? Why can't multple spaces share a single lights instance?
                // Answer: It followed the same pattern as when a ZoneList was used. It might/should be possible to refactor this.
                for (int Item1 = 1; Item1 <= thisLightsInput.numOfSpaces; ++Item1) {
                    ++lightsNum;
                    auto &thisLights = state.dataHeatBal->Lights(lightsNum);
                    int const spaceNum = thisLightsInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisLights.Name = thisLightsInput.names(Item1);
                    thisLights.spaceIndex = spaceNum;
                    thisLights.ZonePtr = zoneNum;
                    thisLights.sched = schedPtr;

                    // Lights Design Level calculation method.
                    thisLights.DesignLevel = setDesignLevel(
                        state, ErrorsFound, lightsModuleObject, thisLightsInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Calculate nominal min/max lighting level
                    thisLights.NomMinDesignLevel = thisLights.DesignLevel * thisLights.sched->getMinVal(state);
                    thisLights.NomMaxDesignLevel = thisLights.DesignLevel * thisLights.sched->getMaxVal(state);

                    thisLights.FractionReturnAir = IHGNumbers(4);
                    thisLights.FractionRadiant = IHGNumbers(5);
                    thisLights.FractionShortWave = IHGNumbers(6);
                    thisLights.FractionReplaceable = IHGNumbers(7);
                    thisLights.FractionReturnAirPlenTempCoeff1 = IHGNumbers(8);
                    thisLights.FractionReturnAirPlenTempCoeff2 = IHGNumbers(9);

                    thisLights.FractionConvected = 1.0 - (thisLights.FractionReturnAir + thisLights.FractionRadiant + thisLights.FractionShortWave);
                    if (std::abs(thisLights.FractionConvected) <= 0.001) {
                        thisLights.FractionConvected = 0.0;
                    }
                    if (thisLights.FractionConvected < 0.0) {
                        if (Item1 == 1) {
                            ShowSevereError(state, format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, lightsModuleObject, thisLights.Name));
                            ErrorsFound = true;
                        }
                    }

                    // Note: if FractionReturnAirIsCalculated = Yes and there is a return-air plenum:
                    // (1) The input values of FractionReturnAir, FractionRadiant and FractionShortWave, and the
                    // value of FractionConvected calculated from these are used in the zone sizing calculations;
                    // (2) in the regular calculation, FractionReturnAir is calculated each time step in
                    // Subr. InitInternalHeatGains as a function of the zone's return plenum air temperature
                    // using FractionReturnAirPlenTempCoeff1 and FractionReturnAirPlenTempCoeff2; then
                    // FractionRadiant and FractionConvected are adjusted from their input values such that
                    // FractionReturnAir + FractionRadiant + FractionShortWave + FractionConvected = 1.0, assuming
                    // FractionShortWave is constant and equal to its input value.

                    if (IHGNumAlphas > 4) {
                        thisLights.EndUseSubcategory = IHGAlphas(5);
                    } else {
                        thisLights.EndUseSubcategory = "General";
                    }

                    if (IHGAlphaFieldBlanks(6)) {
                        thisLights.FractionReturnAirIsCalculated = false;
                    } else if (IHGAlphas(6) != "YES" && IHGAlphas(6) != "NO") {
                        if (Item1 == 1) {
                            ShowWarningError(state,
                                             format("{}{}=\"{}\", invalid {}, value  ={}",
                                                    RoutineName,
                                                    lightsModuleObject,
                                                    thisLightsInput.Name,
                                                    IHGAlphaFieldNames(6),
                                                    IHGAlphas(6)));
                            ShowContinueError(state, ".. Return Air Fraction from Plenum will NOT be calculated.");
                        }
                        thisLights.FractionReturnAirIsCalculated = false;
                    } else {
                        thisLights.FractionReturnAirIsCalculated = (IHGAlphas(6) == "YES");
                    }

                    // Set return air node number
                    thisLights.ZoneReturnNum = 0;
                    thisLights.RetNodeName = "";
                    if (!IHGAlphaFieldBlanks(7)) {
                        if (thisLightsInput.ZoneListActive) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\": {} must be blank when using a ZoneList.",
                                                   RoutineName,
                                                   lightsModuleObject,
                                                   thisLightsInput.Name,
                                                   IHGAlphaFieldNames(7)));
                            ErrorsFound = true;
                        } else {
                            thisLights.RetNodeName = IHGAlphas(7);
                        }
                    }
                    if ((thisLights.FractionReturnAir > 0.0) && (thisLights.ZonePtr > 0)) {
                        thisLights.ZoneReturnNum = DataZoneEquipment::GetReturnNumForZone(state, thisLights.ZonePtr, thisLights.RetNodeName);
                    }

                    if ((thisLights.ZoneReturnNum == 0) && (thisLights.FractionReturnAir > 0.0) && (!IHGAlphaFieldBlanks(7))) {
                        ShowSevereError(
                            state,
                            format(
                                "{}{}=\"{}\", invalid {} ={}", RoutineName, lightsModuleObject, IHGAlphas(1), IHGAlphaFieldNames(7), IHGAlphas(7)));
                        ShowContinueError(state, "No matching Zone Return Air Node found.");
                        ErrorsFound = true;
                    }
                    // Set exhaust air node number
                    thisLights.ZoneExhaustNodeNum = 0;
                    if (!IHGAlphaFieldBlanks(8)) {
                        if (thisLightsInput.ZoneListActive) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\": {} must be blank when using a ZoneList.",
                                                   RoutineName,
                                                   lightsModuleObject,
                                                   thisLightsInput.Name,
                                                   IHGAlphaFieldNames(8)));
                            ErrorsFound = true;
                        } else {
                            bool exhaustNodeError = false;
                            thisLights.ZoneExhaustNodeNum = GetOnlySingleNode(state,
                                                                              IHGAlphas(8),
                                                                              exhaustNodeError,
                                                                              DataLoopNode::ConnectionObjectType::Lights,
                                                                              thisLights.Name,
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::ZoneExhaust,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              ObjectIsNotParent);
                            if (!exhaustNodeError) { // GetOnlySingleNode will throw error messages if this is a NodeList Name and for other issues
                                exhaustNodeError =
                                    DataZoneEquipment::VerifyLightsExhaustNodeForZone(state, thisLights.ZonePtr, thisLights.ZoneExhaustNodeNum);
                            }
                            if (exhaustNodeError) {
                                ShowSevereError(state,
                                                format("{}{}=\"{}\", invalid {} = {}",
                                                       RoutineName,
                                                       lightsModuleObject,
                                                       IHGAlphas(1),
                                                       IHGAlphaFieldNames(8),
                                                       IHGAlphas(8)));
                                ShowContinueError(state, "No matching Zone Exhaust Air Node found.");
                                ErrorsFound = true;
                            } else {
                                if (thisLights.ZoneReturnNum > 0) {
                                    state.dataZoneEquip->ZoneEquipConfig(thisLights.ZonePtr).ReturnNodeExhaustNodeNum(thisLights.ZoneReturnNum) =
                                        thisLights.ZoneExhaustNodeNum;
                                    CheckSharedExhaustFlag = true;
                                } else {
                                    ShowSevereError(state,
                                                    format("{}{}=\"{}\", {} ={} is not used",
                                                           RoutineName,
                                                           lightsModuleObject,
                                                           IHGAlphas(1),
                                                           IHGAlphaFieldNames(8),
                                                           IHGAlphas(8)));
                                    ShowContinueError(
                                        state, "No matching Zone Return Air Node found. The Exhaust Node requires Return Node to work together");
                                    ErrorsFound = true;
                                }
                            }
                        }

                        if (thisLights.ZonePtr <= 0) {
                            continue; // Error, will be caught and terminated later
                        }
                    }
                }
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                for (int lightsNum2 = 1; lightsNum2 <= state.dataHeatBal->TotLights; ++lightsNum2) {
                    SetupEMSActuator(state,
                                     "Lights",
                                     state.dataHeatBal->Lights(lightsNum2).Name,
                                     "Electricity Rate",
                                     "[W]",
                                     state.dataHeatBal->Lights(lightsNum2).EMSLightsOn,
                                     state.dataHeatBal->Lights(lightsNum2).EMSLightingPower);
                    SetupEMSInternalVariable(state,
                                             "Lighting Power Design Level",
                                             state.dataHeatBal->Lights(lightsNum2).Name,
                                             "[W]",
                                             state.dataHeatBal->Lights(lightsNum2).DesignLevel);
                } // EMS
            }
            for (int lightsNum2 = 1; lightsNum2 <= state.dataHeatBal->TotLights; ++lightsNum2) {
                int spaceNum = state.dataHeatBal->Lights(lightsNum2).spaceIndex;
                int zoneNum = state.dataHeatBal->Lights(lightsNum2).ZonePtr;
                // setup internal gains
                int returnNodeNum = 0;
                if ((state.dataHeatBal->Lights(lightsNum2).ZoneReturnNum > 0) &&
                    (state.dataHeatBal->Lights(lightsNum2).ZoneReturnNum <= state.dataZoneEquip->ZoneEquipConfig(zoneNum).NumReturnNodes)) {
                    returnNodeNum = state.dataZoneEquip->ZoneEquipConfig(zoneNum).ReturnNode(state.dataHeatBal->Lights(lightsNum2).ZoneReturnNum);
                }
                if (!ErrorsFound) {
                    SetupSpaceInternalGain(state,
                                           state.dataHeatBal->Lights(lightsNum2).spaceIndex,
                                           1.0,
                                           state.dataHeatBal->Lights(lightsNum2).Name,
                                           DataHeatBalance::IntGainType::Lights,
                                           &state.dataHeatBal->Lights(lightsNum2).ConGainRate,
                                           &state.dataHeatBal->Lights(lightsNum2).RetAirGainRate,
                                           &state.dataHeatBal->Lights(lightsNum2).RadGainRate,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           returnNodeNum);
                }

                if (state.dataHeatBal->Lights(lightsNum2).FractionReturnAir > 0) {
                    state.dataHeatBal->Zone(state.dataHeatBal->Lights(lightsNum2).ZonePtr).HasLtsRetAirGain = true;
                }
                // send values to predefined lighting summary report
                std::string liteName = state.dataHeatBal->Lights(lightsNum2).Name;
                Real64 mult = state.dataHeatBal->Zone(zoneNum).Multiplier * state.dataHeatBal->Zone(zoneNum).ListMultiplier;
                Real64 spaceArea = state.dataHeatBal->space(spaceNum).FloorArea;
                sumArea += spaceArea * mult;
                sumPower += state.dataHeatBal->Lights(lightsNum2).DesignLevel * mult;
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtZone, liteName, state.dataHeatBal->Zone(zoneNum).Name);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtSpace, liteName, state.dataHeatBal->space(spaceNum).Name);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtSpaceType, liteName, state.dataHeatBal->space(spaceNum).spaceType);
                if (spaceArea > 0.0) {
                    PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchInLtDens, liteName, state.dataHeatBal->Lights(lightsNum2).DesignLevel / spaceArea, 4);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtDens, liteName, DataPrecisionGlobals::constant_zero, 4);
                }
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtArea, liteName, spaceArea * mult);
                PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchInLtPower, liteName, state.dataHeatBal->Lights(lightsNum2).DesignLevel * mult);
                PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchInLtEndUse, liteName, state.dataHeatBal->Lights(lightsNum2).EndUseSubcategory);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtSchd, liteName, state.dataHeatBal->Lights(lightsNum2).sched->Name);
                PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchInLtRetAir, liteName, state.dataHeatBal->Lights(lightsNum2).FractionReturnAir, 4);
            } // Item1 - Number of Lights instances
            if (CheckSharedExhaustFlag) {
                DataZoneEquipment::CheckSharedExhaust(state);
                Array1D_bool ReturnNodeShared; // zone supply air inlet nodes
                ReturnNodeShared.allocate(state.dataHeatBal->TotLights);
                ReturnNodeShared = false;
                for (int Loop = 1; Loop <= state.dataHeatBal->TotLights; ++Loop) {
                    int ZoneNum = state.dataHeatBal->Lights(Loop).ZonePtr;
                    int ReturnNum = state.dataHeatBal->Lights(Loop).ZoneReturnNum;
                    int ExhaustNodeNum = state.dataHeatBal->Lights(Loop).ZoneExhaustNodeNum;
                    if (ReturnNum == 0 || ExhaustNodeNum == 0) {
                        continue;
                    }
                    for (int Loop1 = Loop + 1; Loop1 <= state.dataHeatBal->TotLights; ++Loop1) {
                        if (ZoneNum != state.dataHeatBal->Lights(Loop1).ZonePtr) {
                            continue;
                        }
                        if (ReturnNodeShared(Loop1)) {
                            continue;
                        }
                        if (ReturnNum == state.dataHeatBal->Lights(Loop1).ZoneReturnNum &&
                            ExhaustNodeNum != state.dataHeatBal->Lights(Loop1).ZoneExhaustNodeNum) {
                            ShowSevereError(state,
                                            format("{}{}: Duplicated Return Air Node = {} is found, ",
                                                   RoutineName,
                                                   lightsModuleObject,
                                                   state.dataHeatBal->Lights(Loop1).RetNodeName));
                            ShowContinueError(state,
                                              format(" in both Lights objects = {} and {}.",
                                                     state.dataHeatBal->Lights(Loop).Name,
                                                     state.dataHeatBal->Lights(Loop1).Name));
                            ErrorsFound = true;
                            ReturnNodeShared(Loop1) = true;
                        }
                    }
                }
                ReturnNodeShared.deallocate();
            }
        } // TotLights > 0 check
        // add total line to lighting summary table
        if (sumArea > 0.0) {
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtDens, "Interior Lighting Total", sumPower / sumArea,
                             4); // line 792
        } else {
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtDens, "Interior Lighting Total", DataPrecisionGlobals::constant_zero, 4);
        }
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtArea, "Interior Lighting Total", sumArea);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchInLtPower, "Interior Lighting Total", sumPower);

        // ElectricEquipment
        // Declared in state because the lights inputs are needed for demand manager
        int numZoneElectricStatements = 0;
        setupIHGZonesAndSpaces(state,
                               elecEqModuleObject,
                               state.dataInternalHeatGains->zoneElectricObjects,
                               numZoneElectricStatements,
                               state.dataHeatBal->TotElecEquip,
                               ErrorsFound);

        if (state.dataHeatBal->TotElecEquip > 0) {
            state.dataHeatBal->ZoneElectric.allocate(state.dataHeatBal->TotElecEquip);
            int elecEqNum = 0;
            for (int elecEqInputNum = 1; elecEqInputNum <= numZoneElectricStatements; ++elecEqInputNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         elecEqModuleObject,
                                                                         elecEqInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, elecEqModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(3));
                if (IHGAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisElecEqInput = state.dataInternalHeatGains->zoneElectricObjects(elecEqInputNum);
                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(4)));
                int fieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::EquipmentLevel: {
                    fieldNum = 1;
                } break;
                case DesignLevelMethod::WattsPerArea: {
                    fieldNum = 2;
                } break;
                case DesignLevelMethod::WattsPerPerson: {
                    fieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(fieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(fieldNum);
                std::string_view const levelField = IHGNumericFieldNames(fieldNum);

                for (int Item1 = 1; Item1 <= thisElecEqInput.numOfSpaces; ++Item1) {
                    ++elecEqNum;
                    auto &thisZoneElectric = state.dataHeatBal->ZoneElectric(elecEqNum);
                    int const spaceNum = thisElecEqInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneElectric.Name = thisElecEqInput.names(Item1);
                    thisZoneElectric.spaceIndex = spaceNum;
                    thisZoneElectric.ZonePtr = zoneNum;
                    thisZoneElectric.sched = schedPtr;

                    // Electric equipment design level calculation method.
                    thisZoneElectric.DesignLevel = setDesignLevel(
                        state, ErrorsFound, elecEqModuleObject, thisElecEqInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Calculate nominal min/max equipment level
                    thisZoneElectric.NomMinDesignLevel = thisZoneElectric.DesignLevel * thisZoneElectric.sched->getMinVal(state);
                    thisZoneElectric.NomMaxDesignLevel = thisZoneElectric.DesignLevel * thisZoneElectric.sched->getMaxVal(state);

                    thisZoneElectric.FractionLatent = IHGNumbers(4);
                    thisZoneElectric.FractionRadiant = IHGNumbers(5);
                    thisZoneElectric.FractionLost = IHGNumbers(6);
                    // FractionConvected is a calculated field
                    thisZoneElectric.FractionConvected =
                        1.0 - (thisZoneElectric.FractionLatent + thisZoneElectric.FractionRadiant + thisZoneElectric.FractionLost);
                    if (std::abs(thisZoneElectric.FractionConvected) <= 0.001) {
                        thisZoneElectric.FractionConvected = 0.0;
                    }
                    if (thisZoneElectric.FractionConvected < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, elecEqModuleObject, thisElecEqInput.Name));
                        ErrorsFound = true;
                    }

                    if (IHGNumAlphas > 4) {
                        thisZoneElectric.EndUseSubcategory = IHGAlphas(5);
                    } else {
                        thisZoneElectric.EndUseSubcategory = "General";
                    }
                    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(state,
                                         "ElectricEquipment",
                                         thisZoneElectric.Name,
                                         "Electricity Rate",
                                         "[W]",
                                         thisZoneElectric.EMSZoneEquipOverrideOn,
                                         thisZoneElectric.EMSEquipPower);
                        SetupEMSInternalVariable(
                            state, "Plug and Process Power Design Level", thisZoneElectric.Name, "[W]", thisZoneElectric.DesignLevel);
                    } // EMS
                    if (!ErrorsFound) {
                        SetupSpaceInternalGain(state,
                                               thisZoneElectric.spaceIndex,
                                               1.0,
                                               thisZoneElectric.Name,
                                               DataHeatBalance::IntGainType::ElectricEquipment,
                                               &thisZoneElectric.ConGainRate,
                                               nullptr,
                                               &thisZoneElectric.RadGainRate,
                                               &thisZoneElectric.LatGainRate);
                    }
                } // for elecEqInputNum.NumOfSpaces
            } // for elecEqInputNum
        } // TotElecEquip > 0

        // GasEquipment
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> zoneGasObjects;
        int numZoneGasStatements = 0;
        setupIHGZonesAndSpaces(state, gasEqModuleObject, zoneGasObjects, numZoneGasStatements, state.dataHeatBal->TotGasEquip, ErrorsFound);

        if (state.dataHeatBal->TotGasEquip > 0) {
            state.dataHeatBal->ZoneGas.allocate(state.dataHeatBal->TotGasEquip);
            int gasEqNum = 0;
            for (int gasEqInputNum = 1; gasEqInputNum <= numZoneGasStatements; ++gasEqInputNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         gasEqModuleObject,
                                                                         gasEqInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, gasEqModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(3));
                if (IHGAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisGasEqInput = zoneGasObjects(gasEqInputNum);
                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(4)));
                int fieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::EquipmentLevel: {
                    fieldNum = 1;
                } break;
                case DesignLevelMethod::WattsPerArea:
                case DesignLevelMethod::PowerPerArea: {
                    fieldNum = 2;
                } break;
                case DesignLevelMethod::WattsPerPerson:
                case DesignLevelMethod::PowerPerPerson: {
                    fieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(fieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(fieldNum);
                std::string_view const levelField = IHGNumericFieldNames(fieldNum);

                for (int Item1 = 1; Item1 <= thisGasEqInput.numOfSpaces; ++Item1) {
                    ++gasEqNum;
                    auto &thisZoneGas = state.dataHeatBal->ZoneGas(gasEqNum);
                    int const spaceNum = thisGasEqInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneGas.Name = thisGasEqInput.names(Item1);
                    thisZoneGas.spaceIndex = spaceNum;
                    thisZoneGas.ZonePtr = zoneNum;
                    thisZoneGas.sched = schedPtr;

                    // Gas equipment design level calculation method.
                    thisZoneGas.DesignLevel = setDesignLevel(
                        state, ErrorsFound, gasEqModuleObject, thisGasEqInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Calculate nominal min/max equipment level
                    thisZoneGas.NomMinDesignLevel = thisZoneGas.DesignLevel * thisZoneGas.sched->getMinVal(state);
                    thisZoneGas.NomMaxDesignLevel = thisZoneGas.DesignLevel * thisZoneGas.sched->getMaxVal(state);

                    thisZoneGas.FractionLatent = IHGNumbers(4);
                    thisZoneGas.FractionRadiant = IHGNumbers(5);
                    thisZoneGas.FractionLost = IHGNumbers(6);

                    if ((IHGNumNumbers == 7) || (!IHGNumericFieldBlanks(7))) {
                        thisZoneGas.CO2RateFactor = IHGNumbers(7);
                    }
                    if (thisZoneGas.CO2RateFactor < 0.0) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", {} < 0.0, value ={:.2R}",
                                               RoutineName,
                                               gasEqModuleObject,
                                               thisGasEqInput.Name,
                                               IHGNumericFieldNames(7),
                                               IHGNumbers(7)));
                        ErrorsFound = true;
                    }
                    if (thisZoneGas.CO2RateFactor > 4.0e-7) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", {} > 4.0E-7, value ={:.2R}",
                                               RoutineName,
                                               gasEqModuleObject,
                                               thisGasEqInput.Name,
                                               IHGNumericFieldNames(7),
                                               IHGNumbers(7)));
                        ErrorsFound = true;
                    }
                    // FractionConvected is a calculated field
                    thisZoneGas.FractionConvected = 1.0 - (thisZoneGas.FractionLatent + thisZoneGas.FractionRadiant + thisZoneGas.FractionLost);
                    if (std::abs(thisZoneGas.FractionConvected) <= 0.001) {
                        thisZoneGas.FractionConvected = 0.0;
                    }
                    if (thisZoneGas.FractionConvected < 0.0) {
                        if (Item1 == 1) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, gasEqModuleObject, thisGasEqInput.Name));
                            ErrorsFound = true;
                        }
                    }

                    if (IHGNumAlphas > 4) {
                        thisZoneGas.EndUseSubcategory = IHGAlphas(5);
                    } else {
                        thisZoneGas.EndUseSubcategory = "General";
                    }

                    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(state,
                                         "GasEquipment",
                                         thisZoneGas.Name,
                                         "NaturalGas Rate",
                                         "[W]",
                                         thisZoneGas.EMSZoneEquipOverrideOn,
                                         thisZoneGas.EMSEquipPower);
                        SetupEMSInternalVariable(state, "Gas Process Power Design Level", thisZoneGas.Name, "[W]", thisZoneGas.DesignLevel);
                    } // EMS

                    if (!ErrorsFound) {
                        SetupSpaceInternalGain(state,
                                               thisZoneGas.spaceIndex,
                                               1.0,
                                               thisZoneGas.Name,
                                               DataHeatBalance::IntGainType::GasEquipment,
                                               &thisZoneGas.ConGainRate,
                                               nullptr,
                                               &thisZoneGas.RadGainRate,
                                               &thisZoneGas.LatGainRate,
                                               nullptr,
                                               &thisZoneGas.CO2GainRate);
                    }

                } // for gasEqInputNum.NumOfSpaces
            } // for gasEqInputNum
        } // TotGasEquip > 0

        // HotWaterEquipment
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> hotWaterEqObjects;
        int numHotWaterEqStatements = 0;
        setupIHGZonesAndSpaces(state, hwEqModuleObject, hotWaterEqObjects, numHotWaterEqStatements, state.dataHeatBal->TotHWEquip, ErrorsFound);

        if (state.dataHeatBal->TotHWEquip > 0) {
            state.dataHeatBal->ZoneHWEq.allocate(state.dataHeatBal->TotHWEquip);
            int hwEqNum = 0;
            for (int hwEqInputNum = 1; hwEqInputNum <= numHotWaterEqStatements; ++hwEqInputNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         hwEqModuleObject,
                                                                         hwEqInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, hwEqModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(3));
                if (IHGAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisHWEqInput = hotWaterEqObjects(hwEqInputNum);
                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(4)));
                int fieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::EquipmentLevel: {
                    fieldNum = 1;
                } break;
                case DesignLevelMethod::WattsPerArea:
                case DesignLevelMethod::PowerPerArea: {
                    fieldNum = 2;
                } break;
                case DesignLevelMethod::WattsPerPerson:
                case DesignLevelMethod::PowerPerPerson: {
                    fieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(fieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(fieldNum);
                std::string_view const levelField = IHGNumericFieldNames(fieldNum);

                for (int Item1 = 1; Item1 <= thisHWEqInput.numOfSpaces; ++Item1) {
                    ++hwEqNum;
                    auto &thisZoneHWEq = state.dataHeatBal->ZoneHWEq(hwEqNum);
                    int const spaceNum = thisHWEqInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneHWEq.Name = thisHWEqInput.names(Item1);
                    thisZoneHWEq.spaceIndex = spaceNum;
                    thisZoneHWEq.ZonePtr = zoneNum;
                    thisZoneHWEq.sched = schedPtr;

                    // Hot Water equipment design level calculation method.
                    thisZoneHWEq.DesignLevel = setDesignLevel(
                        state, ErrorsFound, hwEqModuleObject, thisHWEqInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Calculate nominal min/max equipment level
                    thisZoneHWEq.NomMinDesignLevel = thisZoneHWEq.DesignLevel * thisZoneHWEq.sched->getMinVal(state);
                    thisZoneHWEq.NomMaxDesignLevel = thisZoneHWEq.DesignLevel * thisZoneHWEq.sched->getMaxVal(state);

                    thisZoneHWEq.FractionLatent = IHGNumbers(4);
                    thisZoneHWEq.FractionRadiant = IHGNumbers(5);
                    thisZoneHWEq.FractionLost = IHGNumbers(6);
                    // FractionConvected is a calculated field
                    thisZoneHWEq.FractionConvected = 1.0 - (thisZoneHWEq.FractionLatent + thisZoneHWEq.FractionRadiant + thisZoneHWEq.FractionLost);
                    if (std::abs(thisZoneHWEq.FractionConvected) <= 0.001) {
                        thisZoneHWEq.FractionConvected = 0.0;
                    }
                    if (thisZoneHWEq.FractionConvected < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, hwEqModuleObject, thisHWEqInput.Name));
                        ErrorsFound = true;
                    }

                    if (IHGNumAlphas > 4) {
                        thisZoneHWEq.EndUseSubcategory = IHGAlphas(5);
                    } else {
                        thisZoneHWEq.EndUseSubcategory = "General";
                    }

                    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(state,
                                         "HotWaterEquipment",
                                         thisZoneHWEq.Name,
                                         "District Heating Power Level",
                                         "[W]",
                                         thisZoneHWEq.EMSZoneEquipOverrideOn,
                                         thisZoneHWEq.EMSEquipPower);
                        SetupEMSInternalVariable(state, "Process District Heat Design Level", thisZoneHWEq.Name, "[W]", thisZoneHWEq.DesignLevel);
                    } // EMS

                    if (!ErrorsFound) {
                        SetupSpaceInternalGain(state,
                                               thisZoneHWEq.spaceIndex,
                                               1.0,
                                               thisZoneHWEq.Name,
                                               DataHeatBalance::IntGainType::HotWaterEquipment,
                                               &thisZoneHWEq.ConGainRate,
                                               nullptr,
                                               &thisZoneHWEq.RadGainRate,
                                               &thisZoneHWEq.LatGainRate);
                    }

                } // for hwEqInputNum.NumOfSpaces
            } // for hwEqInputNum
        } // TotHWEquip > 0

        // SteamEquipment
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> steamEqObjects;
        int numSteamEqStatements = 0;
        setupIHGZonesAndSpaces(state, stmEqModuleObject, steamEqObjects, numSteamEqStatements, state.dataHeatBal->TotStmEquip, ErrorsFound);

        if (state.dataHeatBal->TotStmEquip > 0) {
            state.dataHeatBal->ZoneSteamEq.allocate(state.dataHeatBal->TotStmEquip);
            int stmEqNum = 0;
            for (int stmEqInputNum = 1; stmEqInputNum <= numSteamEqStatements; ++stmEqInputNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         stmEqModuleObject,
                                                                         stmEqInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, stmEqModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(3));
                if (IHGAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisStmEqInput = steamEqObjects(stmEqInputNum);
                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(4)));
                int fieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::EquipmentLevel: {
                    fieldNum = 1;
                } break;
                case DesignLevelMethod::WattsPerArea:
                case DesignLevelMethod::PowerPerArea: {
                    fieldNum = 2;
                } break;
                case DesignLevelMethod::WattsPerPerson:
                case DesignLevelMethod::PowerPerPerson: {
                    fieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(fieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(fieldNum);
                std::string_view const levelField = IHGNumericFieldNames(fieldNum);

                for (int Item1 = 1; Item1 <= thisStmEqInput.numOfSpaces; ++Item1) {
                    ++stmEqNum;
                    auto &thisZoneStmEq = state.dataHeatBal->ZoneSteamEq(stmEqNum);
                    int const spaceNum = thisStmEqInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneStmEq.Name = thisStmEqInput.names(Item1);
                    thisZoneStmEq.spaceIndex = spaceNum;
                    thisZoneStmEq.ZonePtr = zoneNum;
                    thisZoneStmEq.sched = schedPtr;

                    // Steam equipment design level calculation method.
                    thisZoneStmEq.DesignLevel = setDesignLevel(
                        state, ErrorsFound, stmEqModuleObject, thisStmEqInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Calculate nominal min/max equipment level
                    thisZoneStmEq.NomMinDesignLevel = thisZoneStmEq.DesignLevel * thisZoneStmEq.sched->getMinVal(state);
                    thisZoneStmEq.NomMaxDesignLevel = thisZoneStmEq.DesignLevel * thisZoneStmEq.sched->getMaxVal(state);

                    thisZoneStmEq.FractionLatent = IHGNumbers(4);
                    thisZoneStmEq.FractionRadiant = IHGNumbers(5);
                    thisZoneStmEq.FractionLost = IHGNumbers(6);
                    // FractionConvected is a calculated field
                    thisZoneStmEq.FractionConvected =
                        1.0 - (thisZoneStmEq.FractionLatent + thisZoneStmEq.FractionRadiant + thisZoneStmEq.FractionLost);
                    if (std::abs(thisZoneStmEq.FractionConvected) <= 0.001) {
                        thisZoneStmEq.FractionConvected = 0.0;
                    }
                    if (thisZoneStmEq.FractionConvected < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, stmEqModuleObject, IHGAlphas(1)));
                        ErrorsFound = true;
                    }

                    if (IHGNumAlphas > 4) {
                        thisZoneStmEq.EndUseSubcategory = IHGAlphas(5);
                    } else {
                        thisZoneStmEq.EndUseSubcategory = "General";
                    }

                    if (thisZoneStmEq.ZonePtr <= 0) {
                        continue; // Error, will be caught and terminated later
                    }

                    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(state,
                                         "SteamEquipment",
                                         thisZoneStmEq.Name,
                                         "District Heating Power Level",
                                         "[W]",
                                         thisZoneStmEq.EMSZoneEquipOverrideOn,
                                         thisZoneStmEq.EMSEquipPower);
                        SetupEMSInternalVariable(
                            state, "Process Steam District Heat Design Level", thisZoneStmEq.Name, "[W]", thisZoneStmEq.DesignLevel);
                    } // EMS

                    if (!ErrorsFound) {
                        SetupSpaceInternalGain(state,
                                               thisZoneStmEq.spaceIndex,
                                               1.0,
                                               thisZoneStmEq.Name,
                                               DataHeatBalance::IntGainType::SteamEquipment,
                                               &thisZoneStmEq.ConGainRate,
                                               nullptr,
                                               &thisZoneStmEq.RadGainRate,
                                               &thisZoneStmEq.LatGainRate);
                    }

                } // for stmEqInputNum.NumOfSpaces
            } // for stmEqInputNum
        } // TotStmEquip > 0

        // OtherEquipment
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> otherEqObjects;
        int numOtherEqStatements = 0;
        setupIHGZonesAndSpaces(state, othEqModuleObject, otherEqObjects, numOtherEqStatements, state.dataHeatBal->TotOthEquip, ErrorsFound);

        if (state.dataHeatBal->TotOthEquip > 0) {
            state.dataHeatBal->ZoneOtherEq.allocate(state.dataHeatBal->TotOthEquip);
            int othEqNum = 0;
            for (int othEqInputNum = 1; othEqInputNum <= numOtherEqStatements; ++othEqInputNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         othEqModuleObject,
                                                                         othEqInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                // Note alpha field numbers are different for OtherEquipment
                ErrorObjectHeader eoh{routineName, othEqModuleObject, IHGAlphas(1)};
                Sched::Schedule *schedPtr = Sched::GetSchedule(state, IHGAlphas(4));
                if (IHGAlphaFieldBlanks(4)) {
                    ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(4));
                    ErrorsFound = true;
                } else if (schedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(4), IHGAlphas(4));
                    ErrorsFound = true;
                } else if (!schedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(4), IHGAlphas(4), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisOthEqInput = otherEqObjects(othEqInputNum);
                DesignLevelMethod const levelMethod = static_cast<DesignLevelMethod>(getEnumValue(DesignLevelMethodNamesUC, IHGAlphas(5)));
                int levelFieldNum = 1;
                switch (levelMethod) {
                case DesignLevelMethod::EquipmentLevel: {
                    levelFieldNum = 1;
                } break;
                case DesignLevelMethod::WattsPerArea:
                case DesignLevelMethod::PowerPerArea: {
                    levelFieldNum = 2;
                } break;
                case DesignLevelMethod::WattsPerPerson:
                case DesignLevelMethod::PowerPerPerson: {
                    levelFieldNum = 3;
                } break;
                default: {
                    assert(false);
                } break;
                }
                Real64 const levelValue = IHGNumbers(levelFieldNum);
                bool const levelBlank = IHGNumericFieldBlanks(levelFieldNum);
                std::string_view const levelField = IHGNumericFieldNames(levelFieldNum);

                for (int Item1 = 1; Item1 <= thisOthEqInput.numOfSpaces; ++Item1) {
                    ++othEqNum;
                    auto &thisZoneOthEq = state.dataHeatBal->ZoneOtherEq(othEqNum);
                    int const spaceNum = thisOthEqInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneOthEq.Name = thisOthEqInput.names(Item1);
                    thisZoneOthEq.spaceIndex = spaceNum;
                    thisZoneOthEq.ZonePtr = zoneNum;
                    thisZoneOthEq.sched = schedPtr;

                    if (IHGAlphas(2) == "NONE") {
                        thisZoneOthEq.OtherEquipFuelType = Constant::eFuel::None;
                    } else {
                        thisZoneOthEq.OtherEquipFuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, IHGAlphas(2)));
                        if (thisZoneOthEq.OtherEquipFuelType == Constant::eFuel::Invalid ||
                            thisZoneOthEq.OtherEquipFuelType == Constant::eFuel::Water) {
                            ShowSevereError(state,
                                            format("{}{}: invalid {} entered={} for {}={}",
                                                   RoutineName,
                                                   othEqModuleObject,
                                                   IHGAlphaFieldNames(2),
                                                   IHGAlphas(2),
                                                   IHGAlphaFieldNames(1),
                                                   thisOthEqInput.Name));
                            ErrorsFound = true;
                        }

                        // Build list of fuel types used in each zone and space (excluding Water)

                        bool found = false;
                        for (Constant::eFuel fuelType : state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNums) {
                            if (thisZoneOthEq.OtherEquipFuelType == fuelType) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNums.emplace_back(thisZoneOthEq.OtherEquipFuelType);
                            // state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNames.emplace_back(FuelTypeString);
                        }
                        found = false;
                        for (Constant::eFuel fuelType : state.dataHeatBal->space(spaceNum).otherEquipFuelTypeNums) {
                            if (thisZoneOthEq.OtherEquipFuelType == fuelType) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            state.dataHeatBal->space(spaceNum).otherEquipFuelTypeNums.emplace_back(thisZoneOthEq.OtherEquipFuelType);
                            // state.dataHeatBal->space(spaceNum).otherEquipFuelTypeNames.emplace_back(FuelTypeString);
                        }
                    }

                    // equipment design level calculation method.
                    thisZoneOthEq.DesignLevel = setDesignLevel(
                        state, ErrorsFound, othEqModuleObject, thisOthEqInput, levelMethod, zoneNum, spaceNum, levelValue, levelBlank, levelField);

                    // Throw an error if the design level is negative and we have a fuel type
                    if (thisZoneOthEq.DesignLevel < 0.0 && thisZoneOthEq.OtherEquipFuelType != Constant::eFuel::Invalid &&
                        thisZoneOthEq.OtherEquipFuelType != Constant::eFuel::None) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", {} is not allowed to be negative",
                                               RoutineName,
                                               othEqModuleObject,
                                               thisOthEqInput.Name,
                                               IHGNumericFieldNames(levelFieldNum)));
                        ShowContinueError(
                            state, format("... when a fuel type of {} is specified.", Constant::eFuelNames[(int)thisZoneOthEq.OtherEquipFuelType]));
                        ErrorsFound = true;
                    }

                    // Calculate nominal min/max equipment level
                    thisZoneOthEq.NomMinDesignLevel = thisZoneOthEq.DesignLevel * thisZoneOthEq.sched->getMinVal(state);
                    thisZoneOthEq.NomMaxDesignLevel = thisZoneOthEq.DesignLevel * thisZoneOthEq.sched->getMaxVal(state);

                    thisZoneOthEq.FractionLatent = IHGNumbers(4);
                    thisZoneOthEq.FractionRadiant = IHGNumbers(5);
                    thisZoneOthEq.FractionLost = IHGNumbers(6);

                    if ((IHGNumNumbers == 7) || (!IHGNumericFieldBlanks(7))) {
                        thisZoneOthEq.CO2RateFactor = IHGNumbers(7);
                    }
                    if (thisZoneOthEq.CO2RateFactor < 0.0) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", {} < 0.0, value ={:.2R}",
                                               RoutineName,
                                               othEqModuleObject,
                                               thisOthEqInput.Name,
                                               IHGNumericFieldNames(7),
                                               IHGNumbers(7)));
                        ErrorsFound = true;
                    }
                    if (thisZoneOthEq.CO2RateFactor > 4.0e-7) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", {} > 4.0E-7, value ={:.2R}",
                                               RoutineName,
                                               othEqModuleObject,
                                               thisOthEqInput.Name,
                                               IHGNumericFieldNames(7),
                                               IHGNumbers(7)));
                        ErrorsFound = true;
                    }

                    // FractionConvected is a calculated field
                    thisZoneOthEq.FractionConvected =
                        1.0 - (thisZoneOthEq.FractionLatent + thisZoneOthEq.FractionRadiant + thisZoneOthEq.FractionLost);
                    if (std::abs(thisZoneOthEq.FractionConvected) <= 0.001) {
                        thisZoneOthEq.FractionConvected = 0.0;
                    }
                    if (thisZoneOthEq.FractionConvected < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, othEqModuleObject, thisOthEqInput.Name));
                        ErrorsFound = true;
                    }

                    if (IHGNumAlphas > 5) {
                        thisZoneOthEq.EndUseSubcategory = IHGAlphas(6);
                    } else {
                        thisZoneOthEq.EndUseSubcategory = "General";
                    }

                    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(state,
                                         "OtherEquipment",
                                         thisZoneOthEq.Name,
                                         "Power Level",
                                         "[W]",
                                         thisZoneOthEq.EMSZoneEquipOverrideOn,
                                         thisZoneOthEq.EMSEquipPower);
                        SetupEMSInternalVariable(state, "Other Equipment Design Level", thisZoneOthEq.Name, "[W]", thisZoneOthEq.DesignLevel);
                    } // EMS

                    if (!ErrorsFound) {
                        SetupSpaceInternalGain(state,
                                               thisZoneOthEq.spaceIndex,
                                               1.0,
                                               thisZoneOthEq.Name,
                                               DataHeatBalance::IntGainType::OtherEquipment,
                                               &thisZoneOthEq.ConGainRate,
                                               nullptr,
                                               &thisZoneOthEq.RadGainRate,
                                               &thisZoneOthEq.LatGainRate);
                    }

                } // for othEqInputNum.NumOfSpaces
            } // for othEqInputNum
        } // TotOtherEquip > 0

        // ElectricEquipment:ITE:AirCooled
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> iTEqObjects;
        int numZoneITEqStatements = 0;
        // Note that this object type does not support ZoneList due to node names in input fields
        bool zoneListNotAllowed = true;
        setupIHGZonesAndSpaces(
            state, itEqModuleObject, iTEqObjects, numZoneITEqStatements, state.dataHeatBal->TotITEquip, ErrorsFound, zoneListNotAllowed);

        if (state.dataHeatBal->TotITEquip > 0) {
            state.dataHeatBal->ZoneITEq.allocate(state.dataHeatBal->TotITEquip);
            int itEqNum = 0;
            for (int itEqInputNum = 1; itEqInputNum <= numZoneITEqStatements; ++itEqInputNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         itEqModuleObject,
                                                                         itEqInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, itEqModuleObject, IHGAlphas(1)};
                Sched::Schedule *opSchedPtr = Sched::GetSchedule(state, IHGAlphas(5));
                if (IHGAlphaFieldBlanks(5)) {
                    opSchedPtr = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but default is constant-1.0
                } else if (opSchedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(5), IHGAlphas(5));
                    ErrorsFound = true;
                } else if (!opSchedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(5), IHGAlphas(5), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                Sched::Schedule *cpuSchedPtr = Sched::GetSchedule(state, IHGAlphas(6));
                if (IHGAlphaFieldBlanks(6)) {
                    cpuSchedPtr = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but default is constant-1.0
                } else if (cpuSchedPtr == nullptr) {
                    ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(6), IHGAlphas(6));
                    ErrorsFound = true;
                } else if (!cpuSchedPtr->checkMinVal(state, Clusive::In, 0.0)) {
                    Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(6), IHGAlphas(6), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                auto &thisITEqInput = iTEqObjects(itEqInputNum);
                for (int Item1 = 1; Item1 <= thisITEqInput.numOfSpaces; ++Item1) {
                    ++itEqNum;
                    auto &thisZoneITEq = state.dataHeatBal->ZoneITEq(itEqNum);
                    int const spaceNum = thisITEqInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneITEq.Name = thisITEqInput.names(Item1);
                    thisZoneITEq.spaceIndex = spaceNum;
                    thisZoneITEq.ZonePtr = zoneNum;
                    thisZoneITEq.operSched = opSchedPtr;
                    thisZoneITEq.cpuLoadSched = cpuSchedPtr;

                    // IT equipment design level calculation method.
                    if (IHGAlphaFieldBlanks(3)) {
                        thisZoneITEq.FlowControlWithApproachTemps = false;
                    } else {
                        if (Util::SameString(IHGAlphas(3), "FlowFromSystem")) {
                            thisZoneITEq.FlowControlWithApproachTemps = false;
                        } else if (Util::SameString(IHGAlphas(3), "FlowControlWithApproachTemperatures")) {
                            thisZoneITEq.FlowControlWithApproachTemps = true;
                            state.dataHeatBal->Zone(thisZoneITEq.ZonePtr).HasAdjustedReturnTempByITE = true;
                            state.dataHeatBal->Zone(thisZoneITEq.ZonePtr).NoHeatToReturnAir = false;
                        } else {
                            ShowSevereError(
                                state,
                                format("{}{}=\"{}\": invalid calculation method: {}", RoutineName, itEqModuleObject, IHGAlphas(1), IHGAlphas(3)));
                            ErrorsFound = true;
                        }
                    }

                    {
                        std::string const &equipmentLevel = IHGAlphas(4);
                        if (equipmentLevel == "WATTS/UNIT") {
                            Real64 spaceFrac = 1.0;
                            if (thisITEqInput.numOfSpaces > 1) {
                                Real64 const zoneArea = state.dataHeatBal->Zone(zoneNum).FloorArea;
                                if (zoneArea > 0.0) {
                                    spaceFrac = state.dataHeatBal->space(spaceNum).FloorArea / zoneArea;
                                } else {
                                    ShowSevereError(
                                        state,
                                        format("{}Zone floor area is zero when allocating ElectricEquipment:ITE:AirCooled loads to Spaces.",
                                               RoutineName));
                                    ShowContinueError(state,
                                                      format("Occurs for ElectricEquipment:ITE:AirCooled object ={} in Zone={}",
                                                             thisITEqInput.Name,
                                                             state.dataHeatBal->Zone(zoneNum).Name));
                                    ErrorsFound = true;
                                }
                            }
                            thisZoneITEq.DesignTotalPower = IHGNumbers(1) * IHGNumbers(2) * spaceFrac;
                            if (IHGNumericFieldBlanks(1)) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", specifies {}, but that field is blank.  0 IT Equipment will result.",
                                                        RoutineName,
                                                        itEqModuleObject,
                                                        IHGAlphas(1),
                                                        IHGNumericFieldNames(1)));
                            }
                            if (IHGNumericFieldBlanks(2)) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", specifies {}, but that field is blank.  0 IT Equipment will result.",
                                                        RoutineName,
                                                        itEqModuleObject,
                                                        IHGAlphas(1),
                                                        IHGNumericFieldNames(2)));
                            }

                        } else if (equipmentLevel == "WATTS/AREA") {
                            if (thisZoneITEq.ZonePtr != 0) {
                                if (IHGNumbers(3) >= 0.0) {
                                    if (spaceNum > 0) {
                                        thisZoneITEq.DesignTotalPower = IHGNumbers(3) * state.dataHeatBal->space(spaceNum).FloorArea;
                                        if ((state.dataHeatBal->space(spaceNum).FloorArea <= 0.0) &&
                                            !state.dataHeatBal->space(spaceNum).isRemainderSpace) {
                                            ShowWarningError(
                                                state,
                                                format("{}{}=\"{}\", specifies {}, but Space Floor Area = 0.  0 IT Equipment will result.",
                                                       RoutineName,
                                                       itEqModuleObject,
                                                       IHGAlphas(1),
                                                       IHGNumericFieldNames(3)));
                                        }
                                    } else {
                                        ShowSevereError(state,
                                                        format("{}{}=\"{}\", invalid {}, value  [<0.0]={:.3R}",
                                                               RoutineName,
                                                               itEqModuleObject,
                                                               IHGAlphas(1),
                                                               IHGNumericFieldNames(3),
                                                               IHGNumbers(3)));
                                        ErrorsFound = true;
                                    }
                                }
                                if (IHGNumericFieldBlanks(3)) {
                                    ShowWarningError(state,
                                                     format("{}{}=\"{}\", specifies {}, but that field is blank.  0 IT Equipment will result.",
                                                            RoutineName,
                                                            itEqModuleObject,
                                                            IHGAlphas(1),
                                                            IHGNumericFieldNames(3)));
                                }

                            } else {
                                ShowSevereError(state,
                                                format("{}{}=\"{}\", invalid {}, value  ={}",
                                                       RoutineName,
                                                       itEqModuleObject,
                                                       IHGAlphas(1),
                                                       IHGAlphaFieldNames(4),
                                                       IHGAlphas(4)));
                                ShowContinueError(state, "...Valid values are \"Watts/Unit\" or \"Watts/Area\".");
                                ErrorsFound = true;
                            }
                        }

                        // Calculate nominal min/max equipment level
                        thisZoneITEq.NomMinDesignLevel = thisZoneITEq.DesignTotalPower * thisZoneITEq.cpuLoadSched->getMinVal(state);
                        thisZoneITEq.NomMaxDesignLevel = thisZoneITEq.DesignTotalPower * thisZoneITEq.cpuLoadSched->getMaxVal(state);

                        thisZoneITEq.DesignFanPowerFrac = IHGNumbers(4);
                        thisZoneITEq.DesignFanPower = thisZoneITEq.DesignFanPowerFrac * thisZoneITEq.DesignTotalPower;
                        thisZoneITEq.DesignCPUPower = (1.0 - thisZoneITEq.DesignFanPowerFrac) * thisZoneITEq.DesignTotalPower;
                        thisZoneITEq.DesignAirVolFlowRate = IHGNumbers(5) * thisZoneITEq.DesignTotalPower;
                        thisZoneITEq.DesignTAirIn = IHGNumbers(6);
                        thisZoneITEq.DesignRecircFrac = IHGNumbers(7);
                        thisZoneITEq.DesignUPSEfficiency = IHGNumbers(8);
                        thisZoneITEq.UPSLossToZoneFrac = IHGNumbers(9);
                        thisZoneITEq.SupplyApproachTemp = IHGNumbers(10);
                        thisZoneITEq.ReturnApproachTemp = IHGNumbers(11);

                        bool hasSupplyApproachTemp = !IHGNumericFieldBlanks(10);
                        bool hasReturnApproachTemp = !IHGNumericFieldBlanks(11);

                        // Performance curves
                        thisZoneITEq.CPUPowerFLTCurve = GetCurveIndex(state, IHGAlphas(7));
                        if (thisZoneITEq.CPUPowerFLTCurve == 0) {
                            ShowSevereError(state, format("{}{} \"{}\"", RoutineName, itEqModuleObject, IHGAlphas(1)));
                            ShowContinueError(state, format("Invalid {}={}", IHGAlphaFieldNames(7), IHGAlphas(7)));
                            ErrorsFound = true;
                        }

                        thisZoneITEq.AirFlowFLTCurve = GetCurveIndex(state, IHGAlphas(8));
                        if (thisZoneITEq.AirFlowFLTCurve == 0) {
                            ShowSevereError(state, format("{}{} \"{}\"", RoutineName, itEqModuleObject, IHGAlphas(1)));
                            ShowContinueError(state, format("Invalid {}={}", IHGAlphaFieldNames(8), IHGAlphas(8)));
                            ErrorsFound = true;
                        }

                        thisZoneITEq.FanPowerFFCurve = GetCurveIndex(state, IHGAlphas(9));
                        if (thisZoneITEq.FanPowerFFCurve == 0) {
                            ShowSevereError(state, format("{}{} \"{}\"", RoutineName, itEqModuleObject, IHGAlphas(1)));
                            ShowContinueError(state, format("Invalid {}={}", IHGAlphaFieldNames(9), IHGAlphas(9)));
                            ErrorsFound = true;
                        }

                        if (!IHGAlphaFieldBlanks(15)) {
                            // If this field isn't blank, it must point to a valid curve
                            thisZoneITEq.RecircFLTCurve = GetCurveIndex(state, IHGAlphas(15));
                            if (thisZoneITEq.RecircFLTCurve == 0) {
                                ShowSevereError(state, format("{}{} \"{}\"", RoutineName, itEqModuleObject, IHGAlphas(1)));
                                ShowContinueError(state, format("Invalid {}={}", IHGAlphaFieldNames(15), IHGAlphas(15)));
                                ErrorsFound = true;
                            }
                        } else {
                            // If this curve is left blank, then the curve is assumed to always equal 1.0.
                            thisZoneITEq.RecircFLTCurve = 0;
                        }

                        if (!IHGAlphaFieldBlanks(16)) {
                            // If this field isn't blank, it must point to a valid curve
                            thisZoneITEq.UPSEfficFPLRCurve = GetCurveIndex(state, IHGAlphas(16));
                            if (thisZoneITEq.UPSEfficFPLRCurve == 0) {
                                ShowSevereError(state, format("{}{} \"{}\"", RoutineName, itEqModuleObject, IHGAlphas(1)));
                                ShowContinueError(state, format("Invalid {}={}", IHGAlphaFieldNames(16), IHGAlphas(16)));
                                ErrorsFound = true;
                            }
                        } else {
                            // If this curve is left blank, then the curve is assumed to always equal 1.0.
                            thisZoneITEq.UPSEfficFPLRCurve = 0;
                        }

                        // Environmental class
                        thisZoneITEq.Class = static_cast<ITEClass>(getEnumValue(ITEClassNamesUC, Util::makeUPPER(IHGAlphas(10))));
                        ErrorsFound = ErrorsFound || (thisZoneITEq.Class == ITEClass::Invalid);

                        // Air and supply inlet connections
                        thisZoneITEq.AirConnectionType =
                            static_cast<ITEInletConnection>(getEnumValue(ITEInletConnectionNamesUC, Util::makeUPPER(IHGAlphas(11))));
                        if (thisZoneITEq.AirConnectionType == ITEInletConnection::RoomAirModel) {
                            // ZoneITEq(Loop).AirConnectionType = ITEInletConnection::RoomAirModel;
                            ShowWarningError(state,
                                             format("{}{}=\"{}Air Inlet Connection Type = RoomAirModel is not implemented yet, using ZoneAirNode",
                                                    RoutineName,
                                                    itEqModuleObject,
                                                    IHGAlphas(1)));
                            thisZoneITEq.AirConnectionType = ITEInletConnection::ZoneAirNode;
                        }
                        ErrorsFound = ErrorsFound || (thisZoneITEq.AirConnectionType == ITEInletConnection::Invalid);

                        if (IHGAlphaFieldBlanks(14)) {
                            if (thisZoneITEq.AirConnectionType == ITEInletConnection::AdjustedSupply) {
                                ShowSevereError(state, format("{}{}: {}", RoutineName, itEqModuleObject, IHGAlphas(1)));
                                ShowContinueError(state,
                                                  format("For {}= AdjustedSupply, {} is required, but this field is blank.",
                                                         IHGAlphaFieldNames(11),
                                                         IHGAlphaFieldNames(14)));
                                ErrorsFound = true;
                            } else if (thisZoneITEq.FlowControlWithApproachTemps) {
                                ShowSevereError(state, format("{}{}: {}", RoutineName, itEqModuleObject, IHGAlphas(1)));
                                ShowContinueError(state,
                                                  format("For {}= FlowControlWithApproachTemperatures, {} is required, but this field is blank.",
                                                         IHGAlphaFieldNames(3),
                                                         IHGAlphaFieldNames(14)));
                                ErrorsFound = true;
                            }
                        } else {
                            thisZoneITEq.SupplyAirNodeNum = GetOnlySingleNode(state,
                                                                              IHGAlphas(14),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::ElectricEquipmentITEAirCooled,
                                                                              IHGAlphas(1),
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::Sensor,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              ObjectIsNotParent);
                        }

                        // check supply air node for matches with zone equipment supply air node
                        int zoneEqIndex = DataZoneEquipment::GetControlledZoneIndex(state, state.dataHeatBal->Zone(thisZoneITEq.ZonePtr).Name);
                        if (zoneEqIndex > 0) { // zoneEqIndex could be zero in the case of an uncontrolled zone
                            auto itStart = state.dataZoneEquip->ZoneEquipConfig(zoneEqIndex).InletNode.begin();
                            auto itEnd = state.dataZoneEquip->ZoneEquipConfig(zoneEqIndex).InletNode.end();
                            int key = thisZoneITEq.SupplyAirNodeNum;
                            thisZoneITEq.inControlledZone = true;
                            bool supplyNodeFound = false;
                            if (std::find(itStart, itEnd, key) != itEnd) {
                                supplyNodeFound = true;
                            }

                            if (thisZoneITEq.AirConnectionType == ITEInletConnection::AdjustedSupply && !supplyNodeFound) {
                                // supply air node must match zone equipment supply air node for these conditions
                                ShowSevereError(state, format("{}: ElectricEquipment:ITE:AirCooled {}", RoutineName, thisZoneITEq.Name));
                                ShowContinueError(state, "Air Inlet Connection Type = AdjustedSupply but no Supply Air Node is specified.");
                                ErrorsFound = true;
                            } else if (thisZoneITEq.FlowControlWithApproachTemps && !supplyNodeFound) {
                                // supply air node must match zone equipment supply air node for these conditions
                                ShowSevereError(state, format("{}: ElectricEquipment:ITE:AirCooled {}", RoutineName, thisZoneITEq.Name));
                                ShowContinueError(state, "Air Inlet Connection Type = AdjustedSupply but no Supply Air Node is specified.");
                                ErrorsFound = true;
                            } else if (thisZoneITEq.SupplyAirNodeNum != 0 && !supplyNodeFound) {
                                // the given supply air node does not match any zone equipment supply air nodes
                                ShowWarningError(
                                    state,
                                    format("{}name: '{}. Supply Air Node Name '{}' does not match any ZoneHVAC:EquipmentConnections objects.",
                                           itEqModuleObject,
                                           IHGAlphas(1),
                                           IHGAlphas(14)));
                            }
                        } // end of if block for zoneEqIndex > 0

                        // End-Use subcategories
                        if (IHGNumAlphas > 16) {
                            thisZoneITEq.EndUseSubcategoryCPU = IHGAlphas(17);
                        } else {
                            thisZoneITEq.EndUseSubcategoryCPU = "ITE-CPU";
                        }

                        if (IHGNumAlphas > 17) {
                            thisZoneITEq.EndUseSubcategoryFan = IHGAlphas(18);
                        } else {
                            thisZoneITEq.EndUseSubcategoryFan = "ITE-Fans";
                        }
                        if (thisZoneITEq.ZonePtr <= 0) {
                            continue; // Error, will be caught and terminated later
                        }

                        if (IHGNumAlphas > 18) {
                            thisZoneITEq.EndUseSubcategoryUPS = IHGAlphas(19);
                        } else {
                            thisZoneITEq.EndUseSubcategoryUPS = "ITE-UPS";
                        }
                        if (thisZoneITEq.FlowControlWithApproachTemps) {
                            if (IHGAlphaFieldBlanks(20)) {
                                if (!hasSupplyApproachTemp) {
                                    ShowSevereCustom(
                                        state,
                                        eoh,
                                        format("For {}= FlowControlWithApproachTemperatures, either {} or {} is required, but both are left blank.",
                                               IHGAlphaFieldNames(3),
                                               IHGNumericFieldNames(10),
                                               IHGAlphaFieldNames(20)));
                                    ErrorsFound = true;
                                }
                            } else if ((thisZoneITEq.supplyApproachTempSched = Sched::GetSchedule(state, IHGAlphas(20))) == nullptr) {
                                ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(20), IHGAlphas(20));
                                ErrorsFound = true;
                            }

                            if (IHGAlphaFieldBlanks(21)) {
                                if (!hasReturnApproachTemp) {
                                    ShowSevereCustom(
                                        state,
                                        eoh,
                                        format("For {}= FlowControlWithApproachTemperatures, either {} or {} is required, but both are left blank.",
                                               IHGAlphaFieldNames(3),
                                               IHGNumericFieldNames(11),
                                               IHGAlphaFieldNames(21)));
                                    ErrorsFound = true;
                                }
                            } else if ((thisZoneITEq.returnApproachTempSched = Sched::GetSchedule(state, IHGAlphas(21))) == nullptr) {
                                ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(20), IHGAlphas(20));
                                ErrorsFound = true;
                            }
                        }

                        if (thisZoneITEq.FlowControlWithApproachTemps) {
                            Real64 TAirInSizing = 0.0;
                            // Set the TAirInSizing to the maximum setpoint value to do sizing based on the maximum fan and cpu power of the ite
                            // object
                            SetPointManager::GetSetPointManagerInputs(state);
                            for (auto *spm : state.dataSetPointManager->spms) {
                                if (spm->type != SetPointManager::SPMType::SZCooling) {
                                    continue;
                                }
                                auto const *spmSZC = dynamic_cast<SetPointManager::SPMSingleZoneTemp *>(spm);
                                assert(spmSZC != nullptr);
                                if (spmSZC->ctrlZoneNum == zoneNum) {
                                    TAirInSizing = spmSZC->maxSetTemp;
                                }
                            }

                            thisZoneITEq.SizingTAirIn = max(TAirInSizing, thisZoneITEq.DesignTAirIn);
                        }

                        // MJW - EMS Not in place yet
                        // if ( AnyEnergyManagementSystemInModel ) {
                        // SetupEMSActuator( "ElectricEquipment", ZoneITEq( Loop ).Name, "Electric Power Level", "[W]", ZoneITEq( Loop
                        // ).EMSZoneEquipOverrideOn, ZoneITEq( Loop ).EMSEquipPower ); SetupEMSInternalVariable( "Plug and Process Power Design
                        // Level", ZoneITEq( Loop ).Name, "[W]", ZoneITEq( Loop ).DesignTotalPower ); } // EMS

                        if (!ErrorsFound) {
                            SetupSpaceInternalGain(state,
                                                   thisZoneITEq.spaceIndex,
                                                   1.0,
                                                   thisZoneITEq.Name,
                                                   DataHeatBalance::IntGainType::ElectricEquipmentITEAirCooled,
                                                   &thisZoneITEq.PowerRpt[(int)PERptVars::ConGainToZone]);
                        }
                    }
                } // for itEqInputNum.NumOfSpaces
            } // for itEqInputNum
            for (int Loop = 1; Loop <= state.dataHeatBal->TotITEquip; ++Loop) {
                if (state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(Loop).ZonePtr).HasAdjustedReturnTempByITE &&
                    (!state.dataHeatBal->ZoneITEq(Loop).FlowControlWithApproachTemps)) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\": invalid calculation method {} for Zone: {}",
                                           RoutineName,
                                           itEqModuleObject,
                                           IHGAlphas(1),
                                           IHGAlphas(3),
                                           IHGAlphas(2)));
                    ShowContinueError(state, "...Multiple flow control methods apply to one zone. ");
                    ErrorsFound = true;
                }
            }
        } // TotITEquip > 0

        // ZoneBaseboard:OutdoorTemperatureControlled
        EPVector<InternalHeatGains::GlobalInternalGainMiscObject> zoneBBHeatObjects;
        int numZoneBBHeatStatements = 0;
        setupIHGZonesAndSpaces(state, bbModuleObject, zoneBBHeatObjects, numZoneBBHeatStatements, state.dataHeatBal->TotBBHeat, ErrorsFound);

        if (state.dataHeatBal->TotBBHeat > 0) {
            state.dataHeatBal->ZoneBBHeat.allocate(state.dataHeatBal->TotBBHeat);
            int bbHeatNum = 0;
            for (int bbHeatInputNum = 1; bbHeatInputNum <= numZoneBBHeatStatements; ++bbHeatInputNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         bbModuleObject,
                                                                         bbHeatInputNum,
                                                                         IHGAlphas,
                                                                         IHGNumAlphas,
                                                                         IHGNumbers,
                                                                         IHGNumNumbers,
                                                                         IOStat,
                                                                         IHGNumericFieldBlanks,
                                                                         IHGAlphaFieldBlanks,
                                                                         IHGAlphaFieldNames,
                                                                         IHGNumericFieldNames);

                ErrorObjectHeader eoh{routineName, bbModuleObject, IHGAlphas(1)};

                auto &thisBBHeatInput = zoneBBHeatObjects(bbHeatInputNum);
                for (int Item1 = 1; Item1 <= thisBBHeatInput.numOfSpaces; ++Item1) {
                    ++bbHeatNum;
                    auto &thisZoneBBHeat = state.dataHeatBal->ZoneBBHeat(bbHeatNum);
                    int const spaceNum = thisBBHeatInput.spaceNums(Item1);
                    int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                    thisZoneBBHeat.Name = thisBBHeatInput.names(Item1);
                    thisZoneBBHeat.spaceIndex = spaceNum;
                    thisZoneBBHeat.ZonePtr = zoneNum;

                    if (IHGAlphaFieldBlanks(3)) {
                        ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                        ErrorsFound = true;
                    } else if ((thisZoneBBHeat.sched = Sched::GetSchedule(state, IHGAlphas(3))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                        ErrorsFound = true;
                    } else if (!thisZoneBBHeat.sched->checkMinVal(state, Clusive::In, 0.0)) {
                        Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                        ErrorsFound = true;
                    }

                    if (IHGNumAlphas > 3) {
                        thisZoneBBHeat.EndUseSubcategory = IHGAlphas(4);
                    } else {
                        thisZoneBBHeat.EndUseSubcategory = "General";
                    }

                    Real64 spaceFrac = 1.0;
                    if (thisBBHeatInput.numOfSpaces > 1) {
                        Real64 const zoneArea = state.dataHeatBal->Zone(zoneNum).FloorArea;
                        if (zoneArea > 0.0) {
                            spaceFrac = state.dataHeatBal->space(spaceNum).FloorArea / zoneArea;
                        } else {
                            ShowSevereError(
                                state,
                                format("{}Zone floor area is zero when allocating ZoneBaseboard:OutdoorTemperatureControlled loads to Spaces.",
                                       RoutineName));
                            ShowContinueError(state,
                                              format("Occurs for ZoneBaseboard:OutdoorTemperatureControlled object ={} in Zone={}",
                                                     thisBBHeatInput.Name,
                                                     state.dataHeatBal->Zone(zoneNum).Name));
                            ErrorsFound = true;
                        }
                    }
                    thisZoneBBHeat.CapatLowTemperature = IHGNumbers(1) * spaceFrac;
                    thisZoneBBHeat.LowTemperature = IHGNumbers(2);
                    thisZoneBBHeat.CapatHighTemperature = IHGNumbers(3) * spaceFrac;
                    thisZoneBBHeat.HighTemperature = IHGNumbers(4);
                    thisZoneBBHeat.FractionRadiant = IHGNumbers(5);
                    thisZoneBBHeat.FractionConvected = 1.0 - thisZoneBBHeat.FractionRadiant;
                    if (thisZoneBBHeat.FractionConvected < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\", Sum of Fractions > 1.0", RoutineName, bbModuleObject, thisBBHeatInput.Name));
                        ErrorsFound = true;
                    }

                    if (thisZoneBBHeat.ZonePtr <= 0) {
                        continue; // Error, will be caught and terminated later
                    }

                    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        SetupEMSActuator(state,
                                         "ZoneBaseboard:OutdoorTemperatureControlled",
                                         thisZoneBBHeat.Name,
                                         "Power Level",
                                         "[W]",
                                         thisZoneBBHeat.EMSZoneBaseboardOverrideOn,
                                         thisZoneBBHeat.EMSZoneBaseboardPower);
                        SetupEMSInternalVariable(state,
                                                 "Simple Zone Baseboard Capacity At Low Temperature",
                                                 thisZoneBBHeat.Name,
                                                 "[W]",
                                                 thisZoneBBHeat.CapatLowTemperature);
                        SetupEMSInternalVariable(state,
                                                 "Simple Zone Baseboard Capacity At High Temperature",
                                                 thisZoneBBHeat.Name,
                                                 "[W]",
                                                 thisZoneBBHeat.CapatHighTemperature);
                    } // EMS

                    SetupSpaceInternalGain(state,
                                           thisZoneBBHeat.spaceIndex,
                                           1.0,
                                           thisZoneBBHeat.Name,
                                           DataHeatBalance::IntGainType::ZoneBaseboardOutdoorTemperatureControlled,
                                           &thisZoneBBHeat.ConGainRate,
                                           nullptr,
                                           &thisZoneBBHeat.RadGainRate);
                } // for bbHeatInputNum.NumOfSpaces
            } // for bbHeatInputNum
        } // TotBBHeat > 0

        state.dataHeatBal->TotCO2Gen = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, contamSSModuleObject);
        state.dataHeatBal->ZoneCO2Gen.allocate(state.dataHeatBal->TotCO2Gen);

        for (int Loop = 1; Loop <= state.dataHeatBal->TotCO2Gen; ++Loop) {
            IHGAlphas = "";
            IHGNumbers = 0.0;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     contamSSModuleObject,
                                                                     Loop,
                                                                     IHGAlphas,
                                                                     IHGNumAlphas,
                                                                     IHGNumbers,
                                                                     IHGNumNumbers,
                                                                     IOStat,
                                                                     IHGNumericFieldBlanks,
                                                                     IHGAlphaFieldBlanks,
                                                                     IHGAlphaFieldNames,
                                                                     IHGNumericFieldNames);

            ErrorObjectHeader eoh{routineName, contamSSModuleObject, IHGAlphas(1)};
            Util::IsNameEmpty(state, IHGAlphas(1), contamSSModuleObject, ErrorsFound);

            state.dataHeatBal->ZoneCO2Gen(Loop).Name = IHGAlphas(1);

            state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr = Util::FindItemInList(IHGAlphas(2), state.dataHeatBal->Zone);
            if (state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr == 0) {
                ShowSevereError(
                    state,
                    format(
                        "{}{}=\"{}\", invalid {} entered={}", RoutineName, contamSSModuleObject, IHGAlphas(1), IHGAlphaFieldNames(2), IHGAlphas(2)));
                ErrorsFound = true;
            }

            if (IHGAlphaFieldBlanks(3)) {
                ShowSevereEmptyField(state, eoh, IHGAlphaFieldNames(3));
                ErrorsFound = true;
            } else if ((state.dataHeatBal->ZoneCO2Gen(Loop).sched = Sched::GetSchedule(state, IHGAlphas(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3));
                ErrorsFound = true;
            } else if (!state.dataHeatBal->ZoneCO2Gen(Loop).sched->checkMinVal(state, Clusive::In, 0.0)) {
                Sched::ShowSevereBadMin(state, eoh, IHGAlphaFieldNames(3), IHGAlphas(3), Clusive::In, 0.0);
                ErrorsFound = true;
            }

            state.dataHeatBal->ZoneCO2Gen(Loop).CO2DesignRate = IHGNumbers(1);

            if (state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr <= 0) {
                continue; // Error, will be caught and terminated later
            }

            // Object report variables
            SetupOutputVariable(state,
                                "Contaminant Source or Sink CO2 Gain Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneCO2Gen(Loop).CO2GainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneCO2Gen(Loop).Name);

            // Zone total report variables
            if (RepVarSet(state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr)) {
                RepVarSet(state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr) = false;

                SetupOutputVariable(state,
                                    "Zone Contaminant Source or Sink CO2 Gain Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataHeatBal->ZoneRpt(state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr).CO2Rate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr).Name);
            }

            SetupZoneInternalGain(state,
                                  state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr,
                                  state.dataHeatBal->ZoneCO2Gen(Loop).Name,
                                  DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkCarbonDioxide,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  &state.dataHeatBal->ZoneCO2Gen(Loop).CO2GainRate);
        }

        RepVarSet.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in Getting Internal Gains Input, Program Stopped", RoutineName));
        }
        setupIHGOutputs(state);

        static constexpr std::string_view Format_721(
            "! <Zone Internal Gains Nominal>,Zone Name, Floor Area {{m2}},# Occupants,Area per Occupant "
            "{{m2/person}},Occupant per Area {{person/m2}},Interior Lighting {{W/m2}},Electric Load {{W/m2}},Gas Load {{W/m2}},Other "
            "Load {{W/m2}},Hot Water Eq {{W/m2}},Steam Equipment {{W/m2}},Sum Loads per Area {{W/m2}},Outdoor Controlled Baseboard "
            "Heat\n");

        print(state.files.eio, Format_721);

        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            auto &zone = state.dataHeatBal->Zone(Loop);

            Real64 LightTot = 0.0;
            Real64 ElecTot = 0.0;
            Real64 GasTot = 0.0;
            Real64 OthTot = 0.0;
            Real64 HWETot = 0.0;
            Real64 StmTot = 0.0;
            std::string BBHeatInd = "No"; // Yes if BBHeat in zone, no if not.

            for (auto const &lights : state.dataHeatBal->Lights) {
                if (lights.ZonePtr == Loop) {
                    LightTot += lights.DesignLevel;
                }
            }
            for (auto const &elecEq : state.dataHeatBal->ZoneElectric) {
                if (elecEq.ZonePtr == Loop) {
                    ElecTot += elecEq.DesignLevel;
                }
            }
            for (auto const &itEq : state.dataHeatBal->ZoneITEq) {
                if (itEq.ZonePtr == Loop) {
                    ElecTot += itEq.DesignTotalPower; // Should this not be itTot?
                }
            }
            for (auto const &gasEq : state.dataHeatBal->ZoneGas) {
                if (gasEq.ZonePtr == Loop) {
                    GasTot += gasEq.DesignLevel;
                }
            }
            for (auto const &otherEq : state.dataHeatBal->ZoneOtherEq) {
                if (otherEq.ZonePtr == Loop) {
                    OthTot += otherEq.DesignLevel;
                }
            }
            for (auto const &steamEq : state.dataHeatBal->ZoneSteamEq) {
                if (steamEq.ZonePtr == Loop) {
                    StmTot += steamEq.DesignLevel;
                }
            }
            for (auto const &hotWaterEq : state.dataHeatBal->ZoneHWEq) {
                if (hotWaterEq.ZonePtr == Loop) {
                    HWETot += hotWaterEq.DesignLevel;
                }
            }
            for (auto const &bbHeat : state.dataHeatBal->ZoneBBHeat) {
                if (bbHeat.ZonePtr == Loop) {
                    BBHeatInd = "Yes";
                }
            }

            zone.InternalHeatGains = LightTot + ElecTot + GasTot + OthTot + HWETot + StmTot;
            if (zone.FloorArea > 0.0) {
                print(state.files.eio, Format_720, zone.Name, zone.FloorArea, zone.TotOccupants);
                print_and_divide_if_greater_than_zero(zone.FloorArea, zone.TotOccupants);
                print(state.files.eio, "{:.3R},", zone.TotOccupants / zone.FloorArea);
                print(state.files.eio, "{:.3R},", LightTot / zone.FloorArea);
                print(state.files.eio, "{:.3R},", ElecTot / zone.FloorArea);
                print(state.files.eio, "{:.3R},", GasTot / zone.FloorArea);
                print(state.files.eio, "{:.3R},", OthTot / zone.FloorArea);
                print(state.files.eio, "{:.3R},", HWETot / zone.FloorArea);
                print(state.files.eio, "{:.3R},", StmTot / zone.FloorArea);
                print(state.files.eio, "{:.3R},{}\n", zone.InternalHeatGains / zone.FloorArea, BBHeatInd);
            } else {
                print(state.files.eio, Format_720, zone.Name, zone.FloorArea, zone.TotOccupants);
                print(state.files.eio, "0.0,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,{}\n", BBHeatInd);
            }
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotPeople; ++Loop) {
            auto &people = state.dataHeatBal->People(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "People",
                      "Number of People {},People/Floor Area {person/m2},Floor Area per person {m2/person},Fraction Radiant,Fraction "
                      "Convected,Sensible Fraction Calculation,Activity level,ASHRAE 55 Warnings,Carbon Dioxide Generation Rate,"
                      "Minimum Number of People for All Day Types,Maximum Number of People for All Day Types,"
                      "Minimum Number of People for Weekdays, Maximum Number of People for Weekdays, "
                      "Minimum Number of People for Weekends/Holidays, Maximum Number of People for Weekends /Holidays,"
                      "Minimum Number of People for Summer Design Days, Maximum Number of People for Summer Design Days,"
                      "Minimum Number of People for Winter Design Days, Maximum Number of People for Winter Design Days");
                if (people.Fanger || people.Pierce || people.KSU || people.CoolingEffectASH55 || people.AnkleDraftASH55) {
                    print(state.files.eio,
                          ",MRT Calculation Type,Work Efficiency, Clothing Insulation Calculation Method,Clothing "
                          "Insulation Calculation Method Schedule,Clothing,Air Velocity,Fanger Calculation,Pierce "
                          "Calculation,KSU Calculation,Cooling Effect Calculation,Ankle Draft Calculation\n");
                } else {
                    print(state.files.eio, "\n");
                }
            }

            if (people.ZonePtr == 0) {
                print(state.files.eio, Format_724, "People-Illegal Zone specified", people.Name);
                continue;
            }

            auto const &zone = state.dataHeatBal->Zone(people.ZonePtr);

            print(state.files.eio, Format_722, "People", people.Name, people.sched->Name, zone.Name, zone.FloorArea, zone.TotOccupants);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople);

            print_and_divide_if_greater_than_zero(people.NumberOfPeople, zone.FloorArea);

            if (people.NumberOfPeople > 0.0) {
                print_and_divide_if_greater_than_zero(zone.FloorArea, people.NumberOfPeople);
            } else {
                print(state.files.eio, "N/A,");
            }

            print(state.files.eio, "{:.3R},", people.FractionRadiant);
            print(state.files.eio, "{:.3R},", people.FractionConvected);
            if (people.UserSpecSensFrac == Constant::AutoCalculate) {
                print(state.files.eio, "AutoCalculate,");
            } else {
                print(state.files.eio, "{:.3R},", people.UserSpecSensFrac);
            }
            print(state.files.eio, "{},", people.activityLevelSched->Name);

            print(state.files.eio, "{},", yesNoNames[(int)people.Show55Warning]);
            print(state.files.eio, "{:.4R},", people.CO2RateFactor);
            print(state.files.eio, "{:.1R},", people.NomMinNumberPeople);
            print(state.files.eio, "{:.1R},", people.NomMaxNumberPeople);

            Real64 SchMin, SchMax;

            // weekdays
            std::tie(SchMin, SchMax) = people.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMin);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = people.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMin);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = people.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMin);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = people.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMin);
            print(state.files.eio, "{:.1R},", people.NumberOfPeople * SchMax);

            if (people.Fanger || people.Pierce || people.KSU || people.CoolingEffectASH55 || people.AnkleDraftASH55) {

                if (people.MRTCalcType == DataHeatBalance::CalcMRT::EnclosureAveraged) {
                    print(state.files.eio, "Zone Averaged,");
                } else if (people.MRTCalcType == DataHeatBalance::CalcMRT::SurfaceWeighted) {
                    print(state.files.eio, "Surface Weighted,");
                } else if (people.MRTCalcType == DataHeatBalance::CalcMRT::AngleFactor) {
                    print(state.files.eio, "Angle Factor,");
                } else {
                    print(state.files.eio, "N/A,");
                }
                print(state.files.eio, "{},", people.workEffSched ? people.workEffSched->Name : "");

                print(state.files.eio, clothingTypeEIOStrings[(int)people.clothingType]);

                if (people.clothingType == ClothingType::CalculationSchedule) {
                    print(state.files.eio, "{},", people.clothingMethodSched->Name);
                } else {
                    print(state.files.eio, "N/A,");
                }

                print(state.files.eio, "{},", people.clothingSched ? people.clothingSched->Name : "");
                print(state.files.eio, "{},", people.airVelocitySched ? people.airVelocitySched->Name : "");

                print(state.files.eio, "{},", yesNoNames[(int)people.Fanger]);
                print(state.files.eio, "{},", yesNoNames[(int)people.Pierce]);
                print(state.files.eio, "{},", yesNoNames[(int)people.KSU]);
                print(state.files.eio, "{},", yesNoNames[(int)people.CoolingEffectASH55]);
                print(state.files.eio, "{}", yesNoNames[(int)people.AnkleDraftASH55]);
            }
            print(state.files.eio, "\n");
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotLights; ++Loop) {
            auto &lights = state.dataHeatBal->Lights(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "Lights",
                      "Lighting Level {W},Lights/Floor Area {W/m2},Lights per person {W/person},Fraction Return "
                      "Air,Fraction Radiant,Fraction Short Wave,Fraction Convected,Fraction Replaceable,End-Use, "
                      "Minimum Lighting Level for All Day Types {W},Maximum Lighting Level for All Day Types {W},"
                      "Minimum Lighting Level for Weekdays {W}, Maximum Lighting Level for Weekdays {W},"
                      "Minimum Lighting Level for Weekends/Holidays {W}, Maximum Lighting Level for Weekends /Holidays {W},"
                      "Minimum Lighting Level for Summer Design Days {W}, Maximum Lighting Level for Summer Design Days {W},"
                      "Minimum Lighting Level for Winter Design Days {W}, Maximum Lighting Level for Winter Design Days {W}\n");
            }

            if (lights.ZonePtr == 0) {
                print(state.files.eio, "Lights-Illegal Zone specified", lights.Name);
                continue;
            }

            auto const &zone = state.dataHeatBal->Zone(lights.ZonePtr);

            print(state.files.eio, Format_722, "Lights", lights.Name, lights.sched->Name, zone.Name, zone.FloorArea, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", lights.DesignLevel);

            print_and_divide_if_greater_than_zero(lights.DesignLevel, zone.FloorArea);
            print_and_divide_if_greater_than_zero(lights.DesignLevel, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", lights.FractionReturnAir);
            print(state.files.eio, "{:.3R},", lights.FractionRadiant);
            print(state.files.eio, "{:.3R},", lights.FractionShortWave);
            print(state.files.eio, "{:.3R},", lights.FractionConvected);
            print(state.files.eio, "{:.3R},", lights.FractionReplaceable);
            print(state.files.eio, "{},", lights.EndUseSubcategory);
            print(state.files.eio, "{:.3R},", lights.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", lights.NomMaxDesignLevel);

            auto &light = state.dataHeatBal->Lights(Loop);

            Real64 SchMin, SchMax;
            // weekdays
            std::tie(SchMin, SchMax) = light.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", light.DesignLevel * SchMin);
            print(state.files.eio, "{:.1R},", light.DesignLevel * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = light.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", light.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", light.DesignLevel * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = light.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", light.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", light.DesignLevel * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = light.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", light.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R}\n", light.DesignLevel * SchMax);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotElecEquip; ++Loop) {
            auto &elecEq = state.dataHeatBal->ZoneElectric(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "ElectricEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,"
                      "Minimum Equipment Level for All Day Types {W},Maximum Equipment Level for All Day Types {W},"
                      "Minimum Equipment Level for Weekdays {W}, Maximum Equipment Level for Weekdays {W},"
                      "Minimum Equipment Level for Weekends/Holidays {W}, Maximum Equipment Level for Weekends /Holidays {W},"
                      "Minimum Equipment Level for Summer Design Days {W}, Maximum Equipment Level for Summer Design Days {W},"
                      "Minimum Equipment Level for Winter Design Days {W}, Maximum Equipment Level for Winter Design Days {W}\n");
            }

            if (elecEq.ZonePtr == 0) {
                print(state.files.eio, Format_724, "Electric Equipment-Illegal Zone specified", elecEq.Name);
                continue;
            }

            auto &zone = state.dataHeatBal->Zone(elecEq.ZonePtr);

            print(state.files.eio, Format_722, "ElectricEquipment", elecEq.Name, elecEq.sched->Name, zone.Name, zone.FloorArea, zone.TotOccupants);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel);

            print_and_divide_if_greater_than_zero(elecEq.DesignLevel, zone.FloorArea);
            print_and_divide_if_greater_than_zero(elecEq.DesignLevel, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", elecEq.FractionLatent);
            print(state.files.eio, "{:.3R},", elecEq.FractionRadiant);
            print(state.files.eio, "{:.3R},", elecEq.FractionLost);
            print(state.files.eio, "{:.3R},", elecEq.FractionConvected);
            print(state.files.eio, "{},", elecEq.EndUseSubcategory);
            print(state.files.eio, "{:.3R},", elecEq.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", elecEq.NomMaxDesignLevel);

            Real64 SchMin, SchMax;

            // weekdays
            std::tie(SchMin, SchMax) = elecEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = elecEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = elecEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = elecEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", elecEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R}\n", elecEq.DesignLevel * SchMax);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotGasEquip; ++Loop) {
            auto &gasEq = state.dataHeatBal->ZoneGas(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "GasEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,"
                      "Minimum Equipment Level for All Day Types {W},Maximum Equipment Level for All Day Types {W},"
                      "Minimum Equipment Level for Weekdays {W}, Maximum Equipment Level for Weekdays {W},"
                      "Minimum Equipment Level for Weekends/Holidays {W}, Maximum Equipment Level for Weekends /Holidays {W},"
                      "Minimum Equipment Level for Summer Design Days {W}, Maximum Equipment Level for Summer Design Days {W},"
                      "Minimum Equipment Level for Winter Design Days {W}, Maximum Equipment Level for Winter Design Days {W}\n");
            }

            if (gasEq.ZonePtr == 0) {
                print(state.files.eio, Format_724, "Gas Equipment-Illegal Zone specified", gasEq.Name);
                continue;
            }

            auto &zone = state.dataHeatBal->Zone(gasEq.ZonePtr);

            print(state.files.eio, Format_722, "GasEquipment", gasEq.Name, gasEq.sched->Name, zone.Name, zone.FloorArea, zone.TotOccupants);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel);

            print_and_divide_if_greater_than_zero(gasEq.DesignLevel, zone.FloorArea);
            print_and_divide_if_greater_than_zero(gasEq.DesignLevel, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", gasEq.FractionLatent);
            print(state.files.eio, "{:.3R},", gasEq.FractionRadiant);
            print(state.files.eio, "{:.3R},", gasEq.FractionLost);
            print(state.files.eio, "{:.3R},", gasEq.FractionConvected);
            print(state.files.eio, "{},", gasEq.EndUseSubcategory);
            print(state.files.eio, "{:.3R},", gasEq.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", gasEq.NomMaxDesignLevel);

            Real64 SchMin, SchMax;
            // weekdays
            std::tie(SchMin, SchMax) = gasEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = gasEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = gasEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = gasEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", gasEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R}\n", gasEq.DesignLevel * SchMax);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotHWEquip; ++Loop) {
            auto &hotWaterEq = state.dataHeatBal->ZoneHWEq(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "HotWaterEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,"
                      "Minimum Equipment Level for All Day Types {W},Maximum Equipment Level for All Day Types {W},"
                      "Minimum Equipment Level for Weekdays {W}, Maximum Equipment Level for Weekdays {W},"
                      "Minimum Equipment Level for Weekends/Holidays {W}, Maximum Equipment Level for Weekends /Holidays {W},"
                      "Minimum Equipment Level for Summer Design Days {W}, Maximum Equipment Level for Summer Design Days {W},"
                      "Minimum Equipment Level for Winter Design Days {W}, Maximum Equipment Level for Winter Design Days {W}\n");
            }

            if (hotWaterEq.ZonePtr == 0) {
                print(state.files.eio, Format_724, "Hot Water Equipment-Illegal Zone specified", hotWaterEq.Name);
                continue;
            }

            auto const &zone = state.dataHeatBal->Zone(hotWaterEq.ZonePtr);

            print(state.files.eio,
                  Format_722,
                  "HotWaterEquipment",
                  hotWaterEq.Name,
                  hotWaterEq.sched->Name,
                  zone.Name,
                  zone.FloorArea,
                  zone.TotOccupants);

            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel);

            print_and_divide_if_greater_than_zero(hotWaterEq.DesignLevel, zone.FloorArea);
            print_and_divide_if_greater_than_zero(hotWaterEq.DesignLevel, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", hotWaterEq.FractionLatent);
            print(state.files.eio, "{:.3R},", hotWaterEq.FractionRadiant);
            print(state.files.eio, "{:.3R},", hotWaterEq.FractionLost);
            print(state.files.eio, "{:.3R},", hotWaterEq.FractionConvected);
            print(state.files.eio, "{},", hotWaterEq.EndUseSubcategory);
            print(state.files.eio, "{:.3R},", hotWaterEq.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", hotWaterEq.NomMaxDesignLevel);

            Real64 SchMin, SchMax;
            // weekdays
            std::tie(SchMin, SchMax) = hotWaterEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = hotWaterEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = hotWaterEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = hotWaterEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", hotWaterEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R}\n", hotWaterEq.DesignLevel * SchMax);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotStmEquip; ++Loop) {
            auto &steamEq = state.dataHeatBal->ZoneSteamEq(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "SteamEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,End-Use SubCategory,"
                      "Minimum Equipment Level for All Day Types {W},Maximum Equipment Level for All Day Types {W},"
                      "Minimum Equipment Level for Weekdays {W}, Maximum Equipment Level for Weekdays {W},"
                      "Minimum Equipment Level for Weekends/Holidays {W}, Maximum Equipment Level for Weekends /Holidays {W},"
                      "Minimum Equipment Level for Summer Design Days {W}, Maximum Equipment Level for Summer Design Days {W},"
                      "Minimum Equipment Level for Winter Design Days {W}, Maximum Equipment Level for Winter Design Days {W}\n");
            }

            if (steamEq.ZonePtr == 0) {
                print(state.files.eio, Format_724, "Steam Equipment-Illegal Zone specified", steamEq.Name);
                continue;
            }

            auto &zone = state.dataHeatBal->Zone(steamEq.ZonePtr);

            print(state.files.eio, Format_722, "SteamEquipment", steamEq.Name, steamEq.sched->Name, zone.Name, zone.FloorArea, zone.TotOccupants);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel);

            print_and_divide_if_greater_than_zero(steamEq.DesignLevel, zone.FloorArea);
            print_and_divide_if_greater_than_zero(steamEq.DesignLevel, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", steamEq.FractionLatent);
            print(state.files.eio, "{:.3R},", steamEq.FractionRadiant);
            print(state.files.eio, "{:.3R},", steamEq.FractionLost);
            print(state.files.eio, "{:.3R},", steamEq.FractionConvected);
            print(state.files.eio, "{},", steamEq.EndUseSubcategory);
            print(state.files.eio, "{:.3R},", steamEq.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", steamEq.NomMaxDesignLevel);

            Real64 SchMin, SchMax;
            // weekdays
            std::tie(SchMin, SchMax) = steamEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = steamEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = steamEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = steamEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", steamEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R}\n", steamEq.DesignLevel * SchMax);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotOthEquip; ++Loop) {
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "OtherEquipment",
                      "Equipment Level {W},Equipment/Floor Area {W/m2},Equipment per person {W/person},Fraction Latent,Fraction Radiant,Fraction "
                      "Lost,Fraction Convected,"
                      "Minimum Equipment Level for All Day Types {W},Maximum Equipment Level for All Day Types {W},"
                      "Minimum Equipment Level for Weekdays {W}, Maximum Equipment Level for Weekdays {W},"
                      "Minimum Equipment Level for Weekends/Holidays {W}, Maximum Equipment Level for Weekends /Holidays {W},"
                      "Minimum Equipment Level for Summer Design Days {W}, Maximum Equipment Level for Summer Design Days {W},"
                      "Minimum Equipment Level for Winter Design Days {W}, Maximum Equipment Level for Winter Design Days {W}\n");
            }

            auto &otherEq = state.dataHeatBal->ZoneOtherEq(Loop);

            if (otherEq.ZonePtr == 0) {
                print(state.files.eio, Format_724, "Other Equipment-Illegal Zone specified", otherEq.Name);
                continue;
            }

            auto const &zone = state.dataHeatBal->Zone(otherEq.ZonePtr);

            print(state.files.eio, Format_722, "OtherEquipment", otherEq.Name, otherEq.sched->Name, zone.Name, zone.FloorArea, zone.TotOccupants);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel);

            print_and_divide_if_greater_than_zero(otherEq.DesignLevel, zone.FloorArea);
            print_and_divide_if_greater_than_zero(otherEq.DesignLevel, zone.TotOccupants);

            print(state.files.eio, "{:.3R},", otherEq.FractionLatent);
            print(state.files.eio, "{:.3R},", otherEq.FractionRadiant);
            print(state.files.eio, "{:.3R},", otherEq.FractionLost);
            print(state.files.eio, "{:.3R},", otherEq.FractionConvected);
            print(state.files.eio, "{:.3R},", otherEq.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", otherEq.NomMaxDesignLevel);

            Real64 SchMin, SchMax;

            // weekdays
            std::tie(SchMin, SchMax) = otherEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = otherEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = otherEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = otherEq.sched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", otherEq.DesignLevel * SchMin);
            print(state.files.eio, "{:.3R}\n", otherEq.DesignLevel * SchMax);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotITEquip; ++Loop) {
            auto &itEq = state.dataHeatBal->ZoneITEq(Loop);

            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "ElectricEquipment:ITE:AirCooled",
                      "Equipment Level {W},"
                      "Equipment/Floor Area {W/m2},Equipment per person {W/person},"
                      "Fraction Convected,CPU End-Use SubCategory,Fan End-Use SubCategory,UPS End-Use SubCategory,"
                      "Minimum Equipment Level for All Day Types {W},Maximum Equipment Level for All Day Types {W},"
                      "Minimum Equipment Level for Weekdays {W}, Maximum Equipment Level for Weekdays {W},"
                      "Minimum Equipment Level for Weekends/Holidays {W}, Maximum Equipment Level for Weekends /Holidays {W},"
                      "Minimum Equipment Level for Summer Design Days {W}, Maximum Equipment Level for Summer Design Days {W},"
                      "Minimum Equipment Level for Winter Design Days {W}, Maximum Equipment Level for Winter Design Days {W},"
                      "Design Air Volume Flow Rate {m3/s}\n");
            }

            if (itEq.ZonePtr == 0) {
                print(state.files.eio, Format_724, "ElectricEquipment:ITE:AirCooled-Illegal Zone specified", itEq.Name);
                continue;
            }

            auto const &zone = state.dataHeatBal->Zone(itEq.ZonePtr);
            print(state.files.eio,
                  Format_722,
                  "ElectricEquipment:ITE:AirCooled",
                  itEq.Name,
                  itEq.operSched->Name,
                  zone.Name,
                  zone.FloorArea,
                  zone.TotOccupants);

            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower);

            print_and_divide_if_greater_than_zero(itEq.DesignTotalPower, zone.FloorArea);
            print_and_divide_if_greater_than_zero(itEq.DesignTotalPower, zone.TotOccupants);

            // ElectricEquipment:ITE:AirCooled is 100% convective
            print(state.files.eio, "1.0,");

            print(state.files.eio, "{},", itEq.EndUseSubcategoryCPU);
            print(state.files.eio, "{},", itEq.EndUseSubcategoryFan);
            print(state.files.eio, "{},", itEq.EndUseSubcategoryUPS);
            print(state.files.eio, "{:.3R},", itEq.NomMinDesignLevel);
            print(state.files.eio, "{:.3R},", itEq.NomMaxDesignLevel);

            Real64 SchMin, SchMax;
            // weekdays
            std::tie(SchMin, SchMax) = itEq.operSched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::Weekday);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMin);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMax);

            // weekends/holidays
            std::tie(SchMin, SchMax) = itEq.operSched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WeekEndHoliday);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMin);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMax);

            // summer design days
            std::tie(SchMin, SchMax) = itEq.operSched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::SummerDesignDay);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMin);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMax);

            // winter design days
            std::tie(SchMin, SchMax) = itEq.operSched->getMinMaxValsByDayType(state, Sched::DayTypeGroup::WinterDesignDay);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMin);
            print(state.files.eio, "{:.3R},", itEq.DesignTotalPower * SchMax);

            print(state.files.eio, "{:.10R}\n", itEq.DesignAirVolFlowRate);
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotBBHeat; ++Loop) {
            auto &bbHeat = state.dataHeatBal->ZoneBBHeat(Loop);
            if (Loop == 1) {
                print(state.files.eio,
                      Format_723,
                      "Outdoor Controlled Baseboard Heat",
                      "Capacity at Low Temperature {W},Low Temperature {C},Capacity at High Temperature "
                      "{W},High Temperature {C},Fraction Radiant,Fraction Convected,End-Use Subcategory\n");
            }

            if (bbHeat.ZonePtr == 0) {
                print(state.files.eio, Format_724, "Outdoor Controlled Baseboard Heat-Illegal Zone specified", bbHeat.Name);
                continue;
            }

            auto const &zone = state.dataHeatBal->Zone(bbHeat.ZonePtr);

            print(state.files.eio,
                  Format_722,
                  "Outdoor Controlled Baseboard Heat",
                  bbHeat.Name,
                  bbHeat.sched->Name,
                  zone.Name,
                  zone.FloorArea,
                  zone.TotOccupants);

            print(state.files.eio, "{:.3R},", bbHeat.CapatLowTemperature);
            print(state.files.eio, "{:.3R},", bbHeat.LowTemperature);
            print(state.files.eio, "{:.3R},", bbHeat.CapatHighTemperature);
            print(state.files.eio, "{:.3R},", bbHeat.HighTemperature);
            print(state.files.eio, "{:.3R},", bbHeat.FractionRadiant);
            print(state.files.eio, "{:.3R},", bbHeat.FractionConvected);
            print(state.files.eio, "{}\n", bbHeat.EndUseSubcategory);
        }
    }

    void setupIHGZonesAndSpaces(EnergyPlusData &state,
                                const std::string &objectType,
                                EPVector<InternalHeatGains::GlobalInternalGainMiscObject> &inputObjects,
                                int &numInputObjects,
                                int &numGainInstances,
                                bool &errors,
                                const bool zoneListNotAllowed)
    {
        // This function pre-processes the input objects for objectType and determines the ultimate number
        // of simulation instances for each input object after expansion for SpaceList, Zone, or ZoneList.
        // inputObjects is allocated here and filled with data for further input processing.

        constexpr std::string_view routineName = "setupIHGZonesAndSpaces: ";

        auto &ip = state.dataInputProcessing->inputProcessor;
        auto const instances = ip->epJSON.find(objectType);
        if (instances != ip->epJSON.end()) {
            bool localErrFlag = false;
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, objectType);
            auto &instancesValue = instances.value();
            numInputObjects = int(instancesValue.size());
            inputObjects.allocate(numInputObjects);

            numGainInstances = 0;
            int counter = 0;
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &objectFields = instance.value();
                std::string const &thisObjectName = Util::makeUPPER(instance.key());
                ip->markObjectAsUsed(objectType, instance.key());

                // For incoming idf, maintain object order
                ++counter;
                int objNum = ip->getIDFObjNum(state, objectType, counter);
                inputObjects(objNum).Name = thisObjectName;
                std::string areaFieldName;
                if (zoneListNotAllowed) {
                    areaFieldName = "zone_or_space_name";
                } else {
                    areaFieldName = "zone_or_zonelist_or_space_or_spacelist_name";
                }
                std::string areaName = ip->getAlphaFieldValue(objectFields, objectSchemaProps, areaFieldName);

                int zoneNum = Util::FindItemInList(areaName, state.dataHeatBal->Zone);
                if (zoneNum > 0) {
                    inputObjects(objNum).spaceStartPtr = numGainInstances + 1;
                    int numSpaces = state.dataHeatBal->Zone(zoneNum).numSpaces;
                    numGainInstances += numSpaces;
                    inputObjects(objNum).numOfSpaces = numSpaces;
                    inputObjects(objNum).ZoneListActive = false;
                    if (numSpaces == 1) {
                        inputObjects(objNum).spaceNums.emplace_back(state.dataHeatBal->Zone(zoneNum).spaceIndexes(1));
                        inputObjects(objNum).names.emplace_back(inputObjects(objNum).Name);
                    } else {
                        for (int const spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                            inputObjects(objNum).spaceNums.emplace_back(spaceNum);
                            inputObjects(objNum).names.emplace_back(state.dataHeatBal->space(spaceNum).Name + ' ' + inputObjects(objNum).Name);
                        }
                    }
                    continue;
                }
                int spaceNum = Util::FindItemInList(areaName, state.dataHeatBal->space);
                if (spaceNum > 0) {
                    inputObjects(objNum).spaceStartPtr = numGainInstances + 1;
                    ++numGainInstances;
                    inputObjects(objNum).numOfSpaces = 1;
                    inputObjects(objNum).spaceListActive = false;
                    inputObjects(objNum).spaceOrSpaceListPtr = spaceNum;
                    inputObjects(objNum).spaceNums.emplace_back(spaceNum);
                    inputObjects(objNum).names.emplace_back(inputObjects(objNum).Name);
                    continue;
                }
                int zoneListNum = Util::FindItemInList(areaName, state.dataHeatBal->ZoneList);
                if (zoneListNum > 0) {
                    if (zoneListNotAllowed) {
                        ShowSevereError(
                            state, format("{}=\"{}\" ZoneList Name=\"{}\" not allowed for {}.", objectType, thisObjectName, areaName, objectType));
                        errors = true;
                        localErrFlag = true;
                    } else {

                        inputObjects(objNum).spaceStartPtr = numGainInstances + 1;
                        int numSpaces = 0;
                        for (int const listZoneIdx : state.dataHeatBal->ZoneList(zoneListNum).Zone) {
                            numSpaces += state.dataHeatBal->Zone(listZoneIdx).numSpaces;
                            for (int const spaceNum2 : state.dataHeatBal->Zone(listZoneIdx).spaceIndexes) {
                                inputObjects(objNum).spaceNums.emplace_back(spaceNum2);
                                inputObjects(objNum).names.emplace_back(state.dataHeatBal->space(spaceNum2).Name + ' ' + inputObjects(objNum).Name);
                            }
                        }
                        numGainInstances += numSpaces;
                        inputObjects(objNum).numOfSpaces = numSpaces;
                        inputObjects(objNum).ZoneListActive = true;
                    }
                    continue;
                }
                int spaceListNum = Util::FindItemInList(areaName, state.dataHeatBal->spaceList);
                if (spaceListNum > 0) {
                    if (zoneListNotAllowed) {
                        ShowSevereError(
                            state, format("{}=\"{}\" SpaceList Name=\"{}\" not allowed for {}.", objectType, thisObjectName, areaName, objectType));
                        errors = true;
                        localErrFlag = true;
                    } else {
                        inputObjects(objNum).spaceStartPtr = numGainInstances + 1;
                        int numSpaces = state.dataHeatBal->spaceList(spaceListNum).numListSpaces;
                        numGainInstances += numSpaces;
                        inputObjects(objNum).numOfSpaces = numSpaces;
                        inputObjects(objNum).spaceListActive = true;
                        inputObjects(objNum).spaceOrSpaceListPtr = spaceListNum;
                        for (int const spaceNum2 : state.dataHeatBal->spaceList(spaceListNum).spaces) {
                            inputObjects(objNum).spaceNums.emplace_back(spaceNum2);
                            inputObjects(objNum).names.emplace_back(state.dataHeatBal->space(spaceNum2).Name + ' ' + inputObjects(objNum).Name);
                        }
                    }
                    continue;
                }
                ShowSevereError(state, format("{}=\"{}\" invalid {}=\"{}\" not found.", objectType, thisObjectName, areaFieldName, areaName));
                errors = true;
                localErrFlag = true;
            }
            if (localErrFlag) {
                ShowSevereError(state, format("{}Errors with invalid names in {} objects.", routineName, objectType));
                ShowContinueError(state, "...These will not be read in.  Other errors may occur.");
                numGainInstances = 0;
            }
        }
    }

    Real64 setDesignLevel(EnergyPlusData &state,
                          bool &ErrorsFound,
                          std::string_view const objectType,
                          InternalHeatGains::GlobalInternalGainMiscObject const &inputObject,
                          DesignLevelMethod const method,
                          int const zoneNum,
                          int const spaceNum,
                          Real64 const inputValue,
                          bool const inputBlank,
                          std::string_view const fieldName)
    {
        static constexpr std::string_view RoutineName("GetInternalHeatGains: "); // Use this for now to avoid error diffs

        Real64 designLevel = 0.0; // return value

        // Check input value
        if (inputBlank) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", specifies {}, but that field is blank.  0 {} will result.",
                                    RoutineName,
                                    objectType,
                                    inputObject.Name,
                                    fieldName,
                                    objectType));
            return designLevel;
        }

        switch (method) {
        case DesignLevelMethod::People:
        case DesignLevelMethod::LightingLevel:
        case DesignLevelMethod::EquipmentLevel: {
            // No check
        } break;
        case DesignLevelMethod::PeoplePerArea:
        case DesignLevelMethod::WattsPerArea:
        case DesignLevelMethod::PowerPerArea: {
            if (inputValue < 0.0) {
                ShowSevereError(
                    state, format("{}{}=\"{}\", invalid {}, value  [<0.0]={:.3R}", RoutineName, objectType, inputObject.Name, fieldName, inputValue));
                ErrorsFound = true;
            }
        } break;
        case DesignLevelMethod::AreaPerPerson:
        case DesignLevelMethod::WattsPerPerson:
        case DesignLevelMethod::PowerPerPerson: {
            if (inputValue <= 0.0) {
                ShowSevereError(
                    state,
                    format("{}{}=\"{}\", invalid {}, value  [<=0.0]={:.3R}", RoutineName, objectType, inputObject.Name, fieldName, inputValue));
                ErrorsFound = true;
            }
        } break;
        default: {
            assert(false);
        } break;
        }
        if (ErrorsFound) {
            return designLevel;
        }

        switch (method) {
        case DesignLevelMethod::People:
        case DesignLevelMethod::LightingLevel:
        case DesignLevelMethod::EquipmentLevel: {
            // Set space load fraction
            Real64 spaceFrac = 1.0;
            if (inputObject.numOfSpaces > 1) {
                auto const &zone = state.dataHeatBal->Zone(zoneNum);
                if (inputObject.spaceListActive) {
                    spaceFrac = 1.0; // apply the full amount to every space in the SpaceList
                } else if (zone.numSpaces == 1) {
                    spaceFrac = 1.0; // apply the full amount to the space in the zone
                } else {
                    if (zone.FloorArea > 0.0) {
                        spaceFrac = state.dataHeatBal->space(spaceNum).FloorArea / zone.FloorArea;
                    } else {
                        ShowSevereError(state, format("{}Zone floor area is zero when allocating {} loads to Spaces.", RoutineName, objectType));
                        ShowContinueError(state, format("Occurs for {} object ={} in Zone={}", objectType, inputObject.Name, zone.Name));
                        ErrorsFound = true;
                    }
                }
            }
            designLevel = inputValue * spaceFrac;
        } break;
        case DesignLevelMethod::PeoplePerArea:
        case DesignLevelMethod::WattsPerArea:
        case DesignLevelMethod::PowerPerArea: {
            if (spaceNum != 0) {
                auto const &space = state.dataHeatBal->space(spaceNum);
                designLevel = inputValue * space.FloorArea;
                if ((space.FloorArea <= 0.0) && !space.isRemainderSpace) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", specifies {}, but Space Floor Area = 0.  0 {} will result.",
                                            RoutineName,
                                            objectType,
                                            inputObject.Name,
                                            fieldName,
                                            objectType));
                }
            }
        } break;
        case DesignLevelMethod::AreaPerPerson: {
            if (spaceNum != 0) {
                auto const &space = state.dataHeatBal->space(spaceNum);
                designLevel = space.FloorArea / inputValue;
                if ((space.FloorArea <= 0.0) && !space.isRemainderSpace) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", specifies {}, but Space Floor Area = 0.  0 {} will result.",
                                            RoutineName,
                                            objectType,
                                            inputObject.Name,
                                            fieldName,
                                            objectType));
                }
            }
        } break;
        case DesignLevelMethod::WattsPerPerson:
        case DesignLevelMethod::PowerPerPerson: {
            if (spaceNum != 0) {
                auto const &space = state.dataHeatBal->space(spaceNum);
                designLevel = inputValue * space.TotOccupants;
                if (space.TotOccupants <= 0.0) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", specifies {}, but Total Occupants = 0.  0 {} will result.",
                                            RoutineName,
                                            objectType,
                                            inputObject.Name,
                                            fieldName,
                                            objectType));
                }
            }
        } break;
        default: {
            assert(false);
        } break;
        }

        return designLevel;
    }

    void setupIHGOutputs(EnergyPlusData &state)
    {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            // Overall Zone Variables
            SetupOutputVariable(state,
                                "Zone Total Internal Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotRadiantGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotRadiantGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Visible Radiation Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotVisHeatGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Visible Radiation Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotVisHeatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotConvectiveGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotConvectiveGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotLatentGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotLatentGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotTotalHeatGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(zoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Total Internal Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneRpt(zoneNum).TotTotalHeatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(zoneNum).Name);
        }

        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            // Overall Space Variables
            SetupOutputVariable(state,
                                "Space Total Internal Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->spaceRpt(spaceNum).TotRadiantGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->spaceRpt(spaceNum).TotRadiantGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Visible Radiation Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->spaceRpt(spaceNum).TotVisHeatGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Visible Radiation Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->spaceRpt(spaceNum).TotVisHeatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->spaceRpt(spaceNum).TotConvectiveGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->spaceRpt(spaceNum).TotConvectiveGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->spaceRpt(spaceNum).TotLatentGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->spaceRpt(spaceNum).TotLatentGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->spaceRpt(spaceNum).TotTotalHeatGain,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->space(spaceNum).Name);
            SetupOutputVariable(state,
                                "Space Total Internal Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->spaceRpt(spaceNum).TotTotalHeatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->space(spaceNum).Name);
        }

        // Add zone and space outputs only where the particular type of equipment is actually present
        Array1D_bool addZoneOutputs;
        addZoneOutputs.dimension(state.dataGlobal->NumOfZones, false);
        Array1D_bool addSpaceOutputs;
        addSpaceOutputs.dimension(state.dataGlobal->numSpaces, false);

        for (int peopleNum = 1; peopleNum <= state.dataHeatBal->TotPeople; ++peopleNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->People(peopleNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->People(peopleNum).spaceIndex) = true;
            // Object report variables
            SetupOutputVariable(state,
                                "People Occupant Count",
                                Constant::Units::None,
                                state.dataHeatBal->People(peopleNum).NumOcc,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->People(peopleNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->People(peopleNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->People(peopleNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->People(peopleNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Sensible Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->People(peopleNum).SenGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Sensible Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->People(peopleNum).SenGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->People(peopleNum).LatGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->People(peopleNum).LatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->People(peopleNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->People(peopleNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Air Temperature",
                                Constant::Units::C,
                                state.dataHeatBal->People(peopleNum).TemperatureInZone,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
            SetupOutputVariable(state,
                                "People Air Relative Humidity",
                                Constant::Units::Perc,
                                state.dataHeatBal->People(peopleNum).RelativeHumidityInZone,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->People(peopleNum).Name);
        }

        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                // Zone total report variables
                SetupOutputVariable(state,
                                    "Zone People Occupant Count",
                                    Constant::Units::None,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleNumOcc,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Sensible Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleSenGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Sensible Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleSenGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone People Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).PeopleTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                SetupOutputVariable(state,
                                    "Space People Occupant Count",
                                    Constant::Units::None,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleNumOcc,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Sensible Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleSenGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Sensible Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleSenGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space People Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).PeopleTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }

        for (int lightsNum = 1; lightsNum <= state.dataHeatBal->TotLights; ++lightsNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->Lights(lightsNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->Lights(lightsNum).spaceIndex) = true;
            // Object report variables
            SetupOutputVariable(state,
                                "Lights Electricity Rate",
                                Constant::Units::W,
                                state.dataHeatBal->Lights(lightsNum).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Lights(lightsNum).Name);

            SetupOutputVariable(state,
                                "Lights Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Lights(lightsNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->Lights(lightsNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Visible Radiation Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Lights(lightsNum).VisGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Lights(lightsNum).Name);

            SetupOutputVariable(state,
                                "Lights Visible Radiation Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->Lights(lightsNum).VisGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Lights(lightsNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->Lights(lightsNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Return Air Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Lights(lightsNum).RetAirGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Return Air Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->Lights(lightsNum).RetAirGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Lights(lightsNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->Lights(lightsNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Lights(lightsNum).Name);
            SetupOutputVariable(state,
                                "Lights Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Lights(lightsNum).Consumption,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Lights(lightsNum).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorLights,
                                state.dataHeatBal->Lights(lightsNum).EndUseSubcategory,
                                state.dataHeatBal->Zone(state.dataHeatBal->Lights(lightsNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->Lights(lightsNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->Lights(lightsNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->Lights(lightsNum).spaceIndex).spaceType);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                SetupOutputVariable(state,
                                    "Zone Lights Electricity Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Electricity Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsElecConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Visible Radiation Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsVisGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Visible Radiation Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsVisGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Return Air Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsRetAirGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Return Air Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsRetAirGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Lights Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).LtsTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                SetupOutputVariable(state,
                                    "Space Lights Electricity Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Electricity Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsElecConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Visible Radiation Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsVisGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Visible Radiation Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsVisGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Return Air Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsRetAirGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Return Air Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsRetAirGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Lights Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).LtsTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }
        for (int elecEqNum = 1; elecEqNum <= state.dataHeatBal->TotElecEquip; ++elecEqNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->ZoneElectric(elecEqNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->ZoneElectric(elecEqNum).spaceIndex) = true;
            // Object report variables
            SetupOutputVariable(state,
                                "Electric Equipment Electricity Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Consumption,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneElectric(elecEqNum).EndUseSubcategory,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneElectric(elecEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneElectric(elecEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneElectric(elecEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneElectric(elecEqNum).spaceIndex).spaceType);

            SetupOutputVariable(state,
                                "Electric Equipment Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneElectric(elecEqNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneElectric(elecEqNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneElectric(elecEqNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneElectric(elecEqNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneElectric(elecEqNum).LatGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneElectric(elecEqNum).LatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Lost Heat Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneElectric(elecEqNum).LostEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Lost Heat Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneElectric(elecEqNum).LostRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneElectric(elecEqNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
            SetupOutputVariable(state,
                                "Electric Equipment Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneElectric(elecEqNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneElectric(elecEqNum).Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Electricity Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Electricity Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);

                SetupOutputVariable(state,
                                    "Zone Electric Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Electric Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ElecTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                SetupOutputVariable(state,
                                    "Space Electric Equipment Electricity Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Electricity Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);

                SetupOutputVariable(state,
                                    "Space Electric Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Electric Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).ElecTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }
        // Object report variables
        for (int gasEqNum = 1; gasEqNum <= state.dataHeatBal->TotGasEquip; ++gasEqNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->ZoneGas(gasEqNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->ZoneGas(gasEqNum).spaceIndex) = true;
            SetupOutputVariable(state,
                                "Gas Equipment NaturalGas Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGas(gasEqNum).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment NaturalGas Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGas(gasEqNum).Consumption,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name,
                                Constant::eResource::NaturalGas,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneGas(gasEqNum).EndUseSubcategory,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneGas(gasEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneGas(gasEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneGas(gasEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneGas(gasEqNum).spaceIndex).spaceType);

            SetupOutputVariable(state,
                                "Gas Equipment Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGas(gasEqNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGas(gasEqNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGas(gasEqNum).LatGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Lost Heat Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGas(gasEqNum).LostEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGas(gasEqNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGas(gasEqNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGas(gasEqNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGas(gasEqNum).LatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Lost Heat Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGas(gasEqNum).LostRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
            SetupOutputVariable(state,
                                "Gas Equipment Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGas(gasEqNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneGas(gasEqNum).Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {

                SetupOutputVariable(state,
                                    "Zone Gas Equipment NaturalGas Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment NaturalGas Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);

                SetupOutputVariable(state,
                                    "Zone Gas Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Gas Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).GasTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {

                SetupOutputVariable(state,
                                    "Space Gas Equipment NaturalGas Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment NaturalGas Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);

                SetupOutputVariable(state,
                                    "Space Gas Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Gas Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).GasTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }

        // Object report variables
        for (int hwEqNum = 1; hwEqNum <= state.dataHeatBal->TotHWEquip; ++hwEqNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->ZoneHWEq(hwEqNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->ZoneHWEq(hwEqNum).spaceIndex) = true;
            SetupOutputVariable(state,
                                "Hot Water Equipment District Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment District Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Consumption,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name,
                                Constant::eResource::DistrictHeatingWater,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).EndUseSubcategory,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneHWEq(hwEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneHWEq(hwEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneHWEq(hwEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneHWEq(hwEqNum).spaceIndex).spaceType);

            SetupOutputVariable(state,
                                "Hot Water Equipment Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).LatGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).LatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Lost Heat Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).LostEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Lost Heat Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).LostRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
            SetupOutputVariable(state,
                                "Hot Water Equipment Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneHWEq(hwEqNum).Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment District Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment District Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);

                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Hot Water Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).HWTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment District Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment District Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);

                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Hot Water Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).HWTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }

        // Object report variables
        for (int stmEqNum = 1; stmEqNum <= state.dataHeatBal->TotStmEquip; ++stmEqNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->ZoneSteamEq(stmEqNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->ZoneSteamEq(stmEqNum).spaceIndex) = true;
            SetupOutputVariable(state,
                                "Steam Equipment District Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment District Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Consumption,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name,
                                Constant::eResource::DistrictHeatingSteam,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).EndUseSubcategory,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneSteamEq(stmEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneSteamEq(stmEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneSteamEq(stmEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneSteamEq(stmEqNum).spaceIndex).spaceType);

            SetupOutputVariable(state,
                                "Steam Equipment Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Latent Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).LatGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Latent Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).LatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Lost Heat Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).LostEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Lost Heat Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).LostRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
            SetupOutputVariable(state,
                                "Steam Equipment Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneSteamEq(stmEqNum).Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                SetupOutputVariable(state,
                                    "Zone Steam Equipment District Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment District Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);

                SetupOutputVariable(state,
                                    "Zone Steam Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Steam Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).SteamTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                SetupOutputVariable(state,
                                    "Space Steam Equipment District Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment District Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamConsump,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);

                SetupOutputVariable(state,
                                    "Space Steam Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Steam Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).SteamTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }

        // Object report variables
        for (int othEqNum = 1; othEqNum <= state.dataHeatBal->TotOthEquip; ++othEqNum) {
            // Set flags for zone and space total report variables
            auto &zoneOtherEq = state.dataHeatBal->ZoneOtherEq(othEqNum);

            addZoneOutputs(zoneOtherEq.ZonePtr) = true;
            addSpaceOutputs(zoneOtherEq.spaceIndex) = true;
            if (zoneOtherEq.OtherEquipFuelType != Constant::eFuel::Invalid && zoneOtherEq.OtherEquipFuelType != Constant::eFuel::None) {
                SetupOutputVariable(state,
                                    format("Other Equipment {} Rate", Constant::eFuelNames[(int)zoneOtherEq.OtherEquipFuelType]),
                                    Constant::Units::W,
                                    zoneOtherEq.Power,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    zoneOtherEq.Name);
                SetupOutputVariable(state,
                                    format("Other Equipment {} Energy", Constant::eFuelNames[(int)zoneOtherEq.OtherEquipFuelType]),
                                    Constant::Units::J,
                                    zoneOtherEq.Consumption,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    zoneOtherEq.Name,
                                    Constant::eFuel2eResource[(int)zoneOtherEq.OtherEquipFuelType],
                                    OutputProcessor::Group::Building,
                                    OutputProcessor::EndUseCat::InteriorEquipment,
                                    zoneOtherEq.EndUseSubcategory,
                                    state.dataHeatBal->Zone(zoneOtherEq.ZonePtr).Name,
                                    state.dataHeatBal->Zone(zoneOtherEq.ZonePtr).Multiplier,
                                    state.dataHeatBal->Zone(zoneOtherEq.ZonePtr).ListMultiplier,
                                    state.dataHeatBal->space(zoneOtherEq.spaceIndex).spaceType);
            }

            SetupOutputVariable(state,
                                "Other Equipment Radiant Heating Energy",
                                Constant::Units::J,
                                zoneOtherEq.RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Radiant Heating Rate",
                                Constant::Units::W,
                                zoneOtherEq.RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Convective Heating Energy",
                                Constant::Units::J,
                                zoneOtherEq.ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Convective Heating Rate",
                                Constant::Units::W,
                                zoneOtherEq.ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Latent Gain Energy",
                                Constant::Units::J,
                                zoneOtherEq.LatGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Latent Gain Rate",
                                Constant::Units::W,
                                zoneOtherEq.LatGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Lost Heat Energy",
                                Constant::Units::J,
                                zoneOtherEq.LostEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Lost Heat Rate",
                                Constant::Units::W,
                                zoneOtherEq.LostRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Total Heating Energy",
                                Constant::Units::J,
                                zoneOtherEq.TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                zoneOtherEq.Name);
            SetupOutputVariable(state,
                                "Other Equipment Total Heating Rate",
                                Constant::Units::W,
                                zoneOtherEq.TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                zoneOtherEq.Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                for (size_t i = 0; i < state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNums.size(); ++i) {
                    Constant::eFuel fuelType = state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNums[i];
                    if (fuelType == Constant::eFuel::Invalid || fuelType == Constant::eFuel::None) {
                        continue;
                    }

                    std::string_view fuelName = Constant::eFuelNames[(int)state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNums[i]];

                    SetupOutputVariable(state,
                                        format("Zone Other Equipment {} Rate", fuelName),
                                        Constant::Units::W,
                                        state.dataHeatBal->ZoneRpt(zoneNum).OtherPower[(int)fuelType],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(zoneNum).Name);
                    SetupOutputVariable(state,
                                        format("Zone Other Equipment {} Energy", fuelName),
                                        Constant::Units::J,
                                        state.dataHeatBal->ZoneRpt(zoneNum).OtherConsump[(int)fuelType],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(zoneNum).Name);
                }

                SetupOutputVariable(state,
                                    "Zone Other Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Other Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).OtherTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                for (size_t i = 0; i < state.dataHeatBal->space(spaceNum).otherEquipFuelTypeNums.size(); ++i) {
                    Constant::eFuel fuelType = state.dataHeatBal->space(spaceNum).otherEquipFuelTypeNums[i];
                    if (fuelType == Constant::eFuel::Invalid || fuelType == Constant::eFuel::None) {
                        continue;
                    }

                    SetupOutputVariable(state,
                                        format("Space Other Equipment {} Rate", Constant::eFuelNames[(int)fuelType]),
                                        Constant::Units::W,
                                        state.dataHeatBal->spaceRpt(spaceNum).OtherPower[(int)fuelType],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->space(spaceNum).Name);
                    SetupOutputVariable(state,
                                        format("Space Other Equipment {} Energy", Constant::eFuelNames[(int)fuelType]),
                                        Constant::Units::J,
                                        state.dataHeatBal->spaceRpt(spaceNum).OtherConsump[(int)fuelType],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->space(spaceNum).Name);
                }

                SetupOutputVariable(state,
                                    "Space Other Equipment Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Latent Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherLatGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Latent Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherLatGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Lost Heat Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherLost,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Lost Heat Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherLostRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Other Equipment Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).OtherTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }
        // Object report variables
        for (int itEqNum = 1; itEqNum <= state.dataHeatBal->TotITEquip; ++itEqNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->ZoneITEq(itEqNum).spaceIndex) = true;

            constexpr std::array<std::string_view, (int)PERptVars::Num> PowerOutputVariableStrings = {
                "ITE CPU Electricity Rate",
                "ITE Fan Electricity Rate",
                "ITE UPS Electricity Rate",
                "ITE CPU Electricity Rate at Design Inlet Conditions",
                "ITE Fan Electricity Rate at Design Inlet Conditions",
                "ITE UPS Heat Gain to Zone Rate",
                "ITE Total Heat Gain to Zone Rate"};

            for (int i = 0; i < (int)PERptVars::Num; ++i) {
                SetupOutputVariable(state,
                                    PowerOutputVariableStrings[i],
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneITEq(itEqNum).PowerRpt[i],
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->ZoneITEq(itEqNum).Name);
            }

            SetupOutputVariable(state,
                                "ITE CPU Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::CPU],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneITEq(itEqNum).EndUseSubcategoryCPU,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneITEq(itEqNum).spaceIndex).spaceType);

            SetupOutputVariable(state,
                                "ITE Fan Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::Fan],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneITEq(itEqNum).EndUseSubcategoryFan,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneITEq(itEqNum).spaceIndex).spaceType);
            SetupOutputVariable(state,
                                "ITE UPS Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::UPS],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneITEq(itEqNum).EndUseSubcategoryUPS,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(itEqNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneITEq(itEqNum).spaceIndex).spaceType);
            SetupOutputVariable(state,
                                "ITE CPU Electricity Energy at Design Inlet Conditions",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::CPUAtDesign],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Fan Electricity Energy at Design Inlet Conditions",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::FanAtDesign],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE UPS Heat Gain to Zone Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::UPSGainToZone],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Total Heat Gain to Zone Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneITEq(itEqNum).EnergyRpt[(int)PERptVars::ConGainToZone],
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);

            SetupOutputVariable(state,
                                "ITE Standard Density Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirVolFlowStdDensity,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Current Density Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirVolFlowCurDensity,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirMassFlow,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dry-Bulb Temperature",
                                Constant::Units::C,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirInletDryBulbT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dewpoint Temperature",
                                Constant::Units::C,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirInletDewpointT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Relative Humidity",
                                Constant::Units::Perc,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirInletRelHum,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Outlet Dry-Bulb Temperature",
                                Constant::Units::C,
                                state.dataHeatBal->ZoneITEq(itEqNum).AirOutletDryBulbT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            if (state.dataHeatBal->ZoneITEq(itEqNum).SupplyAirNodeNum != 0) {
                SetupOutputVariable(state,
                                    "ITE Supply Heat Index",
                                    Constant::Units::None,
                                    state.dataHeatBal->ZoneITEq(itEqNum).SHI,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->ZoneITEq(itEqNum).Name);
            }
            SetupOutputVariable(state,
                                "ITE Air Inlet Operating Range Exceeded Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeOutOfOperRange,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dry-Bulb Temperature Above Operating Range Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeAboveDryBulbT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dry-Bulb Temperature Below Operating Range Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeBelowDryBulbT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dewpoint Temperature Above Operating Range Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeAboveDewpointT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dewpoint Temperature Below Operating Range Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeBelowDewpointT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Relative Humidity Above Operating Range Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeAboveRH,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Relative Humidity Below Operating Range Time",
                                Constant::Units::hr,
                                state.dataHeatBal->ZoneITEq(itEqNum).TimeBelowRH,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dry-Bulb Temperature Difference Above Operating Range",
                                Constant::Units::deltaC,
                                state.dataHeatBal->ZoneITEq(itEqNum).DryBulbTAboveDeltaT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dry-Bulb Temperature Difference Below Operating Range",
                                Constant::Units::deltaC,
                                state.dataHeatBal->ZoneITEq(itEqNum).DryBulbTBelowDeltaT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dewpoint Temperature Difference Above Operating Range",
                                Constant::Units::deltaC,
                                state.dataHeatBal->ZoneITEq(itEqNum).DewpointTAboveDeltaT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Dewpoint Temperature Difference Below Operating Range",
                                Constant::Units::deltaC,
                                state.dataHeatBal->ZoneITEq(itEqNum).DewpointTBelowDeltaT,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Relative Humidity Difference Above Operating Range",
                                Constant::Units::Perc,
                                state.dataHeatBal->ZoneITEq(itEqNum).RHAboveDeltaRH,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
            SetupOutputVariable(state,
                                "ITE Air Inlet Relative Humidity Difference Below Operating Range",
                                Constant::Units::Perc,
                                state.dataHeatBal->ZoneITEq(itEqNum).RHBelowDeltaRH,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneITEq(itEqNum).Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {

                constexpr std::array<std::string_view, (int)PERptVars::Num> PowerOutputVariableStrings = {
                    "Zone ITE CPU Electricity Rate",
                    "Zone ITE Fan Electricity Rate",
                    "Zone ITE UPS Electricity Rate",
                    "Zone ITE CPU Electricity Rate at Design Inlet Conditions",
                    "Zone ITE Fan Electricity Rate at Design Inlet Conditions",
                    "Zone ITE UPS Heat Gain to Zone Rate",
                    "Zone ITE Total Heat Gain to Zone Rate"};

                for (int i = 0; i < (int)PERptVars::Num; ++i) {
                    SetupOutputVariable(state,
                                        PowerOutputVariableStrings[i],
                                        Constant::Units::W,
                                        state.dataHeatBal->ZoneRpt(zoneNum).PowerRpt[i],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(zoneNum).Name);
                }

                SetupOutputVariable(state,
                                    "Zone ITE Adjusted Return Air Temperature",
                                    Constant::Units::C,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEAdjReturnTemp,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);

                constexpr std::array<std::string_view, (int)PERptVars::Num> EnergyOutputVariableStrings = {
                    "Zone ITE CPU Electricity Energy",
                    "Zone ITE Fan Electricity Energy",
                    "Zone ITE UPS Electricity Energy",
                    "Zone ITE CPU Electricity Energy at Design Inlet Conditions",
                    "Zone ITE Fan Electricity Energy at Design Inlet Conditions",
                    "Zone ITE UPS Heat Gain to Zone Energy",
                    "Zone ITE Total Heat Gain to Zone Energy"};

                for (int i = 0; i < (int)PERptVars::Num; ++i) {
                    SetupOutputVariable(state,
                                        EnergyOutputVariableStrings[i],
                                        Constant::Units::J,
                                        state.dataHeatBal->ZoneRpt(zoneNum).EnergyRpt[i],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(zoneNum).Name);
                }

                SetupOutputVariable(state,
                                    "Zone ITE Standard Density Air Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqAirVolFlowStdDensity,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqAirMassFlow,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Average Supply Heat Index",
                                    Constant::Units::None,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqSHI,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Operating Range Exceeded Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeOutOfOperRange,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Dry-Bulb Temperature Above Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeAboveDryBulbT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Dry-Bulb Temperature Below Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeBelowDryBulbT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Dewpoint Temperature Above Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeAboveDewpointT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Dewpoint Temperature Below Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeBelowDewpointT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Relative Humidity Above Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeAboveRH,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone ITE Any Air Inlet Relative Humidity Below Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->ZoneRpt(zoneNum).ITEqTimeBelowRH,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                constexpr std::array<std::string_view, (int)PERptVars::Num> PowerOutputVariableStrings = {
                    "Space ITE CPU Electricity Rate",
                    "Space ITE Fan Electricity Rate",
                    "Space ITE UPS Electricity Rate",
                    "Space ITE CPU Electricity Rate at Design Inlet Conditions",
                    "Space ITE Fan Electricity Rate at Design Inlet Conditions",
                    "Space ITE UPS Heat Gain to Zone Rate",
                    "Space ITE Total Heat Gain to Zone Rate"};

                for (int i = 0; i < (int)PERptVars::Num; ++i) {
                    SetupOutputVariable(state,
                                        PowerOutputVariableStrings[i],
                                        Constant::Units::W,
                                        state.dataHeatBal->spaceRpt(spaceNum).PowerRpt[i],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->space(spaceNum).Name);
                }

                // Not applicable for space until space has it's own air temperatures
                // Setup Output Variable(state,
                //                    "Space ITE Adjusted Return Air Temperature",
                //                    Constant::Units::W,
                //                    state.dataHeatBal->spaceRpt(spaceNum).ITEAdjReturnTemp,
                //                    OutputProcessor::TimeStepType::Zone,
                //                    OutputProcessor::StoreType::Average,
                //                    state.dataHeatBal->space(spaceNum).Name);

                constexpr std::array<std::string_view, (int)PERptVars::Num> EnergyOutputVariableStrings = {
                    "Space ITE CPU Electricity Energy",
                    "Space ITE Fan Electricity Energy",
                    "Space ITE UPS Electricity Energy",
                    "Space ITE CPU Electricity Energy at Design Inlet Conditions",
                    "Space ITE Fan Electricity Energy at Design Inlet Conditions",
                    "Space ITE UPS Heat Gain to Zone Energy",
                    "Space ITE Total Heat Gain to Zone Energy"};

                for (int i = 0; i < (int)PERptVars::Num; ++i) {
                    SetupOutputVariable(state,
                                        EnergyOutputVariableStrings[i],
                                        Constant::Units::J,
                                        state.dataHeatBal->spaceRpt(spaceNum).EnergyRpt[i],
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->space(spaceNum).Name);
                }

                SetupOutputVariable(state,
                                    "Space ITE Standard Density Air Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqAirVolFlowStdDensity,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqAirMassFlow,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Average Supply Heat Index",
                                    Constant::Units::None,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqSHI,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Operating Range Exceeded Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Dry-Bulb Temperature Above Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Dry-Bulb Temperature Below Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDryBulbT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Dewpoint Temperature Above Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDewpointT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Dewpoint Temperature Below Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDewpointT,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Relative Humidity Above Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveRH,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space ITE Any Air Inlet Relative Humidity Below Operating Range Time",
                                    Constant::Units::hr,
                                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowRH,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }

        // Object report variables
        for (int bbHeatNum = 1; bbHeatNum <= state.dataHeatBal->TotBBHeat; ++bbHeatNum) {
            // Set flags for zone and space total report variables
            addZoneOutputs(state.dataHeatBal->ZoneBBHeat(bbHeatNum).ZonePtr) = true;
            addSpaceOutputs(state.dataHeatBal->ZoneBBHeat(bbHeatNum).spaceIndex) = true;
            SetupOutputVariable(state,
                                "Baseboard Electricity Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Power,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
            SetupOutputVariable(state,
                                "Baseboard Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Consumption,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::InteriorEquipment,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).EndUseSubcategory,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneBBHeat(bbHeatNum).ZonePtr).Name,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneBBHeat(bbHeatNum).ZonePtr).Multiplier,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneBBHeat(bbHeatNum).ZonePtr).ListMultiplier,
                                state.dataHeatBal->space(state.dataHeatBal->ZoneBBHeat(bbHeatNum).spaceIndex).spaceType);

            SetupOutputVariable(state,
                                "Baseboard Radiant Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).RadGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
            SetupOutputVariable(state,
                                "Baseboard Radiant Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).RadGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
            SetupOutputVariable(state,
                                "Baseboard Convective Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).ConGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
            SetupOutputVariable(state,
                                "Baseboard Convective Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).ConGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).TotGainEnergy,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).TotGainRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->ZoneBBHeat(bbHeatNum).Name);
        }

        // Zone total report variables
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            if (addZoneOutputs(zoneNum)) {
                SetupOutputVariable(state,
                                    "Zone Baseboard Electricity Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Baseboard Electricity Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatElecCons,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);

                SetupOutputVariable(state,
                                    "Zone Baseboard Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Baseboard Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Baseboard Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Baseboard Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Baseboard Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(zoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Baseboard Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->ZoneRpt(zoneNum).BaseHeatTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(zoneNum).Name);
            }
            // Reset zone output flag
            addZoneOutputs(zoneNum) = false;
        }

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            if (addSpaceOutputs(spaceNum)) {
                SetupOutputVariable(state,
                                    "Space Baseboard Electricity Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatPower,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Baseboard Electricity Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatElecCons,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);

                SetupOutputVariable(state,
                                    "Space Baseboard Radiant Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatRadGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Baseboard Radiant Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatRadGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Baseboard Convective Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatConGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Baseboard Convective Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatConGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Baseboard Total Heating Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatTotGain,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->space(spaceNum).Name);
                SetupOutputVariable(state,
                                    "Space Baseboard Total Heating Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->spaceRpt(spaceNum).BaseHeatTotGainRate,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->space(spaceNum).Name);
            }
            // Reset space output flag
            addSpaceOutputs(spaceNum) = false;
        }
    }

    void InitInternalHeatGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997
        //       MODIFIED       November 1998, FW: add adjustment to elec lights for dayltg controls
        //                      August 2003, FCW: add optional calculation of light-to-return fraction
        //                       as a function of return plenum air temperature.
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the zone internal heat gains
        // that are independent of the zone air temperature.

        // Using/Aliasing
        using Dayltg::FigureTDDZoneGains;
        using FuelCellElectricGenerator::FigureFuelCellZoneGains;
        using MicroCHPElectricGenerator::FigureMicroCHPZoneGains;
        using OutputReportTabular::AllocateLoadComponentArrays;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using RefrigeratedCase::FigureRefrigerationZoneGains;
        using WaterThermalTanks::CalcWaterThermalTankZoneGains;
        using WaterUse::CalcWaterUseZoneGains;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::array<Real64, 9> C = {
            6.4611027, 0.946892, 0.0000255737, 7.139322, -0.0627909, 0.0000589271, -0.198550, 0.000940018, -0.00000149532};
        static ZoneCatEUseData const zeroZoneCatEUse; // For initialization

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ActivityLevel_WperPerson; // Units on Activity Level (Schedule)
        Real64 NumberOccupants;          // Number of occupants
        Real64 Q;                        // , QR
        Real64 TotalPeopleGain;          // Total heat gain from people (intermediate calculational variable)
        Real64 SensiblePeopleGain;       // Sensible heat gain from people (intermediate calculational variable)
        Real64 FractionConvected;        // For general lighting, fraction of heat from lights convected to zone air
        Real64 FractionReturnAir;        // For general lighting, fraction of heat from lights convected to zone's return air
        Real64 FractionRadiant;          // For general lighting, fraction of heat from lights to zone that is long wave
        Real64 ReturnPlenumTemp;         // Air temperature of a zone's return air plenum (C)
        Real64 pulseMultipler;           // use to create a pulse for the load component report computations

        //  REAL(r64), ALLOCATABLE, SAVE, DIMENSION(:) :: QSA

        //  IF (.NOT. ALLOCATED(QSA)) ALLOCATE(QSA(NumOfZones))

        //  Zero out time step variables
        for (auto &e : state.dataHeatBal->spaceIntGain) {
            e.NOFOCC = 0.0;
            e.QLTSW = 0.0;
        }

        state.dataHeatBal->ZoneIntEEuse = zeroZoneCatEUse; // Set all member arrays to zeros

        for (auto &e : state.dataHeatBal->ZoneRpt) {
            e.CO2Rate = 0.0;
        }

        for (auto &e : state.dataHeatBal->spaceRpt) {
            // People
            e.PeopleRadGain = 0.0;
            e.PeopleConGain = 0.0;
            e.PeopleSenGain = 0.0;
            e.PeopleNumOcc = 0.0;
            e.PeopleLatGain = 0.0;
            e.PeopleTotGain = 0.0;
            e.PeopleRadGainRate = 0.0;
            e.PeopleConGainRate = 0.0;
            e.PeopleSenGainRate = 0.0;
            e.PeopleLatGainRate = 0.0;
            e.PeopleTotGainRate = 0.0;
            // Lights
            e.LtsPower = 0.0;
            e.LtsElecConsump = 0.0;
            e.LtsRadGain = 0.0;
            e.LtsVisGain = 0.0;
            e.LtsConGain = 0.0;
            e.LtsRetAirGain = 0.0;
            e.LtsTotGain = 0.0;
            e.LtsRadGainRate = 0.0;
            e.LtsVisGainRate = 0.0;
            e.LtsConGainRate = 0.0;
            e.LtsRetAirGainRate = 0.0;
            e.LtsTotGainRate = 0.0;
            // Baseboard Heat
            e.BaseHeatPower = 0.0;
            e.BaseHeatElecCons = 0.0;
            e.BaseHeatRadGain = 0.0;
            e.BaseHeatConGain = 0.0;
            e.BaseHeatTotGain = 0.0;
            e.BaseHeatRadGainRate = 0.0;
            e.BaseHeatConGainRate = 0.0;
            e.BaseHeatTotGainRate = 0.0;
            // Electric Equipment
            e.ElecPower = 0.0;
            e.ElecConsump = 0.0;
            e.ElecRadGain = 0.0;
            e.ElecConGain = 0.0;
            e.ElecLatGain = 0.0;
            e.ElecLost = 0.0;
            e.ElecTotGain = 0.0;
            e.ElecRadGainRate = 0.0;
            e.ElecConGainRate = 0.0;
            e.ElecLatGainRate = 0.0;
            e.ElecLostRate = 0.0;
            e.ElecTotGainRate = 0.0;
            // Gas Equipment
            e.GasPower = 0.0;
            e.GasConsump = 0.0;
            e.GasRadGain = 0.0;
            e.GasConGain = 0.0;
            e.GasLatGain = 0.0;
            e.GasLost = 0.0;
            e.GasTotGain = 0.0;
            e.GasRadGainRate = 0.0;
            e.GasConGainRate = 0.0;
            e.GasLatGainRate = 0.0;
            e.GasLostRate = 0.0;
            e.GasTotGainRate = 0.0;
            // Hot Water Equipment
            e.HWPower = 0.0;
            e.HWConsump = 0.0;
            e.HWRadGain = 0.0;
            e.HWConGain = 0.0;
            e.HWLatGain = 0.0;
            e.HWLost = 0.0;
            e.HWTotGain = 0.0;
            e.HWRadGainRate = 0.0;
            e.HWConGainRate = 0.0;
            e.HWLatGainRate = 0.0;
            e.HWLostRate = 0.0;
            e.HWTotGainRate = 0.0;
            // Steam Equipment
            e.SteamPower = 0.0;
            e.SteamConsump = 0.0;
            e.SteamRadGain = 0.0;
            e.SteamConGain = 0.0;
            e.SteamLatGain = 0.0;
            e.SteamLost = 0.0;
            e.SteamTotGain = 0.0;
            e.SteamRadGainRate = 0.0;
            e.SteamConGainRate = 0.0;
            e.SteamLatGainRate = 0.0;
            e.SteamLostRate = 0.0;
            e.SteamTotGainRate = 0.0;
            // Other Equipment
            e.OtherRadGain = 0.0;
            e.OtherConGain = 0.0;
            e.OtherLatGain = 0.0;
            e.OtherLost = 0.0;
            e.OtherTotGain = 0.0;
            e.OtherRadGainRate = 0.0;
            e.OtherConGainRate = 0.0;
            e.OtherLatGainRate = 0.0;
            e.OtherLostRate = 0.0;
            e.OtherTotGainRate = 0.0;
            // Overall Zone Variables
            e.TotRadiantGain = 0.0;
            e.TotVisHeatGain = 0.0;
            e.TotConvectiveGain = 0.0;
            e.TotLatentGain = 0.0;
            e.TotTotalHeatGain = 0.0;
            e.TotRadiantGainRate = 0.0;
            e.TotVisHeatGainRate = 0.0;
            e.TotConvectiveGainRate = 0.0;
            e.TotLatentGainRate = 0.0;
            e.TotTotalHeatGainRate = 0.0;
            // Contaminant
            e.CO2Rate = 0.0;
            e.GCRate = 0.0;
            for (int i = 0; i < (int)Constant::eFuel::Num; ++i) {
                e.OtherPower[i] = 0.0;
                e.OtherConsump[i] = 0.0;
            }
        }

        for (auto &e : state.dataHeatBal->ZonePreDefRep) {
            e.NumOcc = 0.0;
        }

        //  QSA = 0.0

        // Process Internal Heat Gains, People done below
        // Occupant Stuff
        //   METHOD:
        //       The function is based on a curve fit to data presented in
        //       Table 48 'Heat Gain From People' of Chapter 1 of the 'Carrier
        //       Handbook of Air Conditioning System Design', 1965.  Values of
        //       Sensible gain were obtained from the table at average adjusted
        //       metabolic rates 350, 400, 450, 500, 750, 850, 1000, and
        //       1450 Btu/hr each at temperatures 82, 80, 78, 75, and 70F.
        //       Sensible gains of 0.0 at 96F and equal to the metabolic rate
        //       at 30F were assumed in order to give reasonable values beyond
        //       The reported temperature range.
        for (int Loop = 1; Loop <= state.dataHeatBal->TotPeople; ++Loop) {
            auto &thisPeople = state.dataHeatBal->People(Loop);
            int NZ = state.dataHeatBal->People(Loop).ZonePtr;
            int spaceNum = thisPeople.spaceIndex;
            auto const &thisSpaceHB = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum);
            NumberOccupants = thisPeople.NumberOfPeople * thisPeople.sched->getCurrentVal();

            if (thisPeople.EMSPeopleOn) {
                NumberOccupants = thisPeople.EMSNumberOfPeople;
            }

            TotalPeopleGain = 0.0;
            SensiblePeopleGain = 0.0;

            auto &thisZoneRep = state.dataHeatBal->ZonePreDefRep(NZ);
            if (NumberOccupants > 0.0) {
                ActivityLevel_WperPerson = thisPeople.activityLevelSched->getCurrentVal();
                TotalPeopleGain = NumberOccupants * ActivityLevel_WperPerson;
                // if the user did not specify a sensible fraction, calculate the sensible heat gain
                if (thisPeople.UserSpecSensFrac == Constant::AutoCalculate) {
                    Real64 airTemp = thisSpaceHB.MAT;
                    if (state.dataRoomAir->anyNonMixingRoomAirModel) {
                        if (state.dataRoomAir->IsZoneDispVent3Node(NZ) || state.dataRoomAir->IsZoneUFAD(NZ)) {
                            airTemp = state.dataRoomAir->TCMF(NZ);
                        }
                    }
                    SensiblePeopleGain =
                        NumberOccupants * (C[0] + ActivityLevel_WperPerson * (C[1] + ActivityLevel_WperPerson * C[2]) +
                                           airTemp * ((C[3] + ActivityLevel_WperPerson * (C[4] + ActivityLevel_WperPerson * C[5])) +
                                                      airTemp * (C[6] + ActivityLevel_WperPerson * (C[7] + ActivityLevel_WperPerson * C[8]))));
                } else { // if the user did specify a sensible fraction, use it
                    SensiblePeopleGain = TotalPeopleGain * thisPeople.UserSpecSensFrac;
                }

                if (SensiblePeopleGain > TotalPeopleGain) {
                    SensiblePeopleGain = TotalPeopleGain;
                }
                if (SensiblePeopleGain < 0.0) {
                    SensiblePeopleGain = 0.0;
                }

                // For predefined tabular reports related to outside air ventilation
                thisZoneRep.isOccupied = true; // set flag to occupied to be used in tabular reporting for ventilation
                thisZoneRep.NumOcc += NumberOccupants;
                thisZoneRep.NumOccAccum += NumberOccupants * state.dataGlobal->TimeStepZone;
                thisZoneRep.NumOccAccumTime += state.dataGlobal->TimeStepZone;
            } else {
                state.dataHeatBal->ZonePreDefRep(NZ).isOccupied = false; // set flag to occupied to be used in tabular reporting for ventilation
            }

            thisPeople.NumOcc = NumberOccupants;
            thisPeople.RadGainRate = SensiblePeopleGain * thisPeople.FractionRadiant;
            thisPeople.ConGainRate = SensiblePeopleGain * thisPeople.FractionConvected;
            thisPeople.SenGainRate = SensiblePeopleGain;
            thisPeople.LatGainRate = TotalPeopleGain - SensiblePeopleGain;
            thisPeople.TotGainRate = TotalPeopleGain;
            thisPeople.CO2GainRate = TotalPeopleGain * thisPeople.CO2RateFactor;

            auto &thisSpaceIntGain = state.dataHeatBal->spaceIntGain(spaceNum);
            thisSpaceIntGain.NOFOCC += thisPeople.NumOcc;
            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(spaceNum);
            thisSpaceRpt.PeopleRadGainRate += thisPeople.RadGainRate;
            thisSpaceRpt.PeopleConGainRate += thisPeople.ConGainRate;
            thisSpaceRpt.PeopleSenGainRate += thisPeople.SenGainRate;
            thisSpaceRpt.PeopleLatGainRate += thisPeople.LatGainRate;
            thisSpaceRpt.PeopleTotGainRate += thisPeople.TotGainRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotLights; ++Loop) {
            auto &thisLights = state.dataHeatBal->Lights(Loop);
            int NZ = thisLights.ZonePtr;
            int spaceNum = thisLights.spaceIndex;
            Q = thisLights.DesignLevel * thisLights.sched->getCurrentVal();

            if (state.dataDayltg->ZoneDaylight(NZ).totRefPts > 0) {
                if (thisLights.FractionReplaceable > 0.0) { // FractionReplaceable can only be 0 or 1 for these models
                    Q *= state.dataDayltg->spacePowerReductionFactor(spaceNum);
                }
            }

            // Reduce lighting power due to demand limiting
            if (thisLights.ManageDemand && (Q > thisLights.DemandLimit)) {
                Q = thisLights.DemandLimit;
            }

            // Set Q to EMS override if being called for by EMs
            if (thisLights.EMSLightsOn) {
                Q = thisLights.EMSLightingPower;
            }

            FractionConvected = thisLights.FractionConvected;
            FractionReturnAir = thisLights.FractionReturnAir;
            FractionRadiant = thisLights.FractionRadiant;
            if (thisLights.FractionReturnAirIsCalculated && !state.dataGlobal->ZoneSizingCalc && state.dataGlobal->SimTimeSteps > 1) {
                // Calculate FractionReturnAir based on conditions in the zone's return air plenum, if there is one.
                if (state.dataHeatBal->Zone(NZ).IsControlled) {
                    int retNum = thisLights.ZoneReturnNum;
                    int ReturnZonePlenumCondNum = state.dataZoneEquip->ZoneEquipConfig(NZ).ReturnNodePlenumNum(retNum);
                    if (ReturnZonePlenumCondNum > 0) {
                        ReturnPlenumTemp = state.dataZonePlenum->ZoneRetPlenCond(ReturnZonePlenumCondNum).ZoneTemp;
                        FractionReturnAir =
                            thisLights.FractionReturnAirPlenTempCoeff1 - thisLights.FractionReturnAirPlenTempCoeff2 * ReturnPlenumTemp;
                        FractionReturnAir = max(0.0, min(1.0, FractionReturnAir));
                        if (FractionReturnAir >= (1.0 - thisLights.FractionShortWave)) {
                            FractionReturnAir = 1.0 - thisLights.FractionShortWave;
                            FractionRadiant = 0.0;
                            FractionConvected = 0.0;
                        } else {
                            FractionRadiant = ((1.0 - FractionReturnAir - thisLights.FractionShortWave) /
                                               (thisLights.FractionRadiant + thisLights.FractionConvected)) *
                                              thisLights.FractionRadiant;
                            FractionConvected = 1.0 - (FractionReturnAir + FractionRadiant + thisLights.FractionShortWave);
                        }
                    }
                }
            }

            thisLights.Power = Q;
            thisLights.RadGainRate = Q * FractionRadiant;
            thisLights.VisGainRate = Q * thisLights.FractionShortWave;
            thisLights.ConGainRate = Q * FractionConvected;
            thisLights.RetAirGainRate = Q * FractionReturnAir;
            thisLights.TotGainRate = Q;

            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(spaceNum);
            thisSpaceRpt.LtsPower += thisLights.Power;
            thisSpaceRpt.LtsRadGainRate += thisLights.RadGainRate;
            thisSpaceRpt.LtsVisGainRate += thisLights.VisGainRate;
            state.dataHeatBal->spaceIntGain(spaceNum).QLTSW += thisLights.VisGainRate;
            thisSpaceRpt.LtsConGainRate += thisLights.ConGainRate;
            thisSpaceRpt.LtsRetAirGainRate += thisLights.RetAirGainRate;
            thisSpaceRpt.LtsTotGainRate += thisLights.TotGainRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotElecEquip; ++Loop) {
            auto &thisElecEq = state.dataHeatBal->ZoneElectric(Loop);
            Q = thisElecEq.DesignLevel * thisElecEq.sched->getCurrentVal();

            // Reduce equipment power due to demand limiting
            if (thisElecEq.ManageDemand && (Q > thisElecEq.DemandLimit)) {
                Q = thisElecEq.DemandLimit;
            }

            // Set Q to EMS override if being called for by EMs
            if (thisElecEq.EMSZoneEquipOverrideOn) {
                Q = thisElecEq.EMSEquipPower;
            }

            thisElecEq.Power = Q;
            thisElecEq.RadGainRate = Q * thisElecEq.FractionRadiant;
            thisElecEq.ConGainRate = Q * thisElecEq.FractionConvected;
            thisElecEq.LatGainRate = Q * thisElecEq.FractionLatent;
            thisElecEq.LostRate = Q * thisElecEq.FractionLost;
            thisElecEq.TotGainRate = Q - thisElecEq.LostRate;

            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(thisElecEq.spaceIndex);
            thisSpaceRpt.ElecPower += thisElecEq.Power;
            thisSpaceRpt.ElecRadGainRate += thisElecEq.RadGainRate;
            thisSpaceRpt.ElecConGainRate += thisElecEq.ConGainRate;
            thisSpaceRpt.ElecLatGainRate += thisElecEq.LatGainRate;
            thisSpaceRpt.ElecLostRate += thisElecEq.LostRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotGasEquip; ++Loop) {
            auto &thisGasEq = state.dataHeatBal->ZoneGas(Loop);
            Q = thisGasEq.DesignLevel * thisGasEq.sched->getCurrentVal();

            // Set Q to EMS override if being called for by EMs
            if (thisGasEq.EMSZoneEquipOverrideOn) {
                Q = thisGasEq.EMSEquipPower;
            }

            thisGasEq.Power = Q;
            thisGasEq.RadGainRate = Q * thisGasEq.FractionRadiant;
            thisGasEq.ConGainRate = Q * thisGasEq.FractionConvected;
            thisGasEq.LatGainRate = Q * thisGasEq.FractionLatent;
            thisGasEq.LostRate = Q * thisGasEq.FractionLost;
            thisGasEq.TotGainRate = Q - thisGasEq.LostRate;
            thisGasEq.CO2GainRate = Q * thisGasEq.CO2RateFactor;

            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(thisGasEq.spaceIndex);
            thisSpaceRpt.GasPower += thisGasEq.Power;
            thisSpaceRpt.GasRadGainRate += thisGasEq.RadGainRate;
            thisSpaceRpt.GasConGainRate += thisGasEq.ConGainRate;
            thisSpaceRpt.GasLatGainRate += thisGasEq.LatGainRate;
            thisSpaceRpt.GasLostRate += thisGasEq.LostRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotOthEquip; ++Loop) {
            auto &thisOtherEq = state.dataHeatBal->ZoneOtherEq(Loop);
            Q = thisOtherEq.DesignLevel * thisOtherEq.sched->getCurrentVal();

            // Set Q to EMS override if being called for by EMs
            if (thisOtherEq.EMSZoneEquipOverrideOn) {
                Q = thisOtherEq.EMSEquipPower;
            }

            thisOtherEq.Power = Q;
            thisOtherEq.RadGainRate = Q * thisOtherEq.FractionRadiant;
            thisOtherEq.ConGainRate = Q * thisOtherEq.FractionConvected;
            thisOtherEq.LatGainRate = Q * thisOtherEq.FractionLatent;
            thisOtherEq.LostRate = Q * thisOtherEq.FractionLost;
            thisOtherEq.TotGainRate = Q - thisOtherEq.LostRate;

            int fuelType = (int)thisOtherEq.OtherEquipFuelType;
            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(thisOtherEq.spaceIndex);
            thisSpaceRpt.OtherPower[fuelType] += thisOtherEq.Power;
            thisSpaceRpt.OtherTotGainRate += thisOtherEq.TotGainRate;
            thisSpaceRpt.OtherRadGainRate += thisOtherEq.RadGainRate;
            thisSpaceRpt.OtherConGainRate += thisOtherEq.ConGainRate;
            thisSpaceRpt.OtherLatGainRate += thisOtherEq.LatGainRate;
            thisSpaceRpt.OtherLostRate += thisOtherEq.LostRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotHWEquip; ++Loop) {
            auto &thisHWEq = state.dataHeatBal->ZoneHWEq(Loop);
            Q = thisHWEq.DesignLevel * thisHWEq.sched->getCurrentVal();

            // Set Q to EMS override if being called for by EMs
            if (thisHWEq.EMSZoneEquipOverrideOn) {
                Q = thisHWEq.EMSEquipPower;
            }

            thisHWEq.Power = Q;
            thisHWEq.RadGainRate = Q * thisHWEq.FractionRadiant;
            thisHWEq.ConGainRate = Q * thisHWEq.FractionConvected;
            thisHWEq.LatGainRate = Q * thisHWEq.FractionLatent;
            thisHWEq.LostRate = Q * thisHWEq.FractionLost;
            thisHWEq.TotGainRate = Q - thisHWEq.LostRate;

            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(thisHWEq.spaceIndex);
            thisSpaceRpt.HWPower += thisHWEq.Power;
            thisSpaceRpt.HWRadGainRate += thisHWEq.RadGainRate;
            thisSpaceRpt.HWConGainRate += thisHWEq.ConGainRate;
            thisSpaceRpt.HWLatGainRate += thisHWEq.LatGainRate;
            thisSpaceRpt.HWLostRate += thisHWEq.LostRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotStmEquip; ++Loop) {
            auto &thisSteamEq = state.dataHeatBal->ZoneSteamEq(Loop);
            Q = thisSteamEq.DesignLevel * thisSteamEq.sched->getCurrentVal();

            // Set Q to EMS override if being called for by EMs
            if (thisSteamEq.EMSZoneEquipOverrideOn) {
                Q = thisSteamEq.EMSEquipPower;
            }

            thisSteamEq.Power = Q;
            thisSteamEq.RadGainRate = Q * thisSteamEq.FractionRadiant;
            thisSteamEq.ConGainRate = Q * thisSteamEq.FractionConvected;
            thisSteamEq.LatGainRate = Q * thisSteamEq.FractionLatent;
            thisSteamEq.LostRate = Q * thisSteamEq.FractionLost;
            thisSteamEq.TotGainRate = Q - thisSteamEq.LostRate;

            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(thisSteamEq.spaceIndex);
            thisSpaceRpt.SteamPower += thisSteamEq.Power;
            thisSpaceRpt.SteamRadGainRate += thisSteamEq.RadGainRate;
            thisSpaceRpt.SteamConGainRate += thisSteamEq.ConGainRate;
            thisSpaceRpt.SteamLatGainRate += thisSteamEq.LatGainRate;
            thisSpaceRpt.SteamLostRate += thisSteamEq.LostRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotBBHeat; ++Loop) {
            auto &thisBBHeat = state.dataHeatBal->ZoneBBHeat(Loop);
            int NZ = thisBBHeat.ZonePtr;
            if (state.dataHeatBal->Zone(NZ).OutDryBulbTemp >= thisBBHeat.HighTemperature) {
                Q = 0.0;
            } else if (state.dataHeatBal->Zone(NZ).OutDryBulbTemp > thisBBHeat.LowTemperature) {
                Q = (state.dataHeatBal->Zone(NZ).OutDryBulbTemp - thisBBHeat.LowTemperature) *
                        (thisBBHeat.CapatHighTemperature - thisBBHeat.CapatLowTemperature) /
                        (thisBBHeat.HighTemperature - thisBBHeat.LowTemperature) +
                    thisBBHeat.CapatLowTemperature;
            } else {
                Q = thisBBHeat.CapatLowTemperature;
            }
            Q *= thisBBHeat.sched->getCurrentVal();

            // set with EMS value if being called for.
            if (thisBBHeat.EMSZoneBaseboardOverrideOn) {
                Q = thisBBHeat.EMSZoneBaseboardPower;
            }

            thisBBHeat.Power = Q;
            thisBBHeat.RadGainRate = Q * thisBBHeat.FractionRadiant;
            thisBBHeat.ConGainRate = Q * thisBBHeat.FractionConvected;
            thisBBHeat.TotGainRate = Q;

            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(thisBBHeat.spaceIndex);
            thisSpaceRpt.BaseHeatPower += thisBBHeat.Power;
            thisSpaceRpt.BaseHeatRadGainRate += thisBBHeat.RadGainRate;
            thisSpaceRpt.BaseHeatConGainRate += thisBBHeat.ConGainRate;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotCO2Gen; ++Loop) {
            int NZ = state.dataHeatBal->ZoneCO2Gen(Loop).ZonePtr;
            state.dataHeatBal->ZoneCO2Gen(Loop).CO2GainRate =
                state.dataHeatBal->ZoneCO2Gen(Loop).CO2DesignRate * state.dataHeatBal->ZoneCO2Gen(Loop).sched->getCurrentVal();
            state.dataHeatBal->ZoneRpt(NZ).CO2Rate += state.dataHeatBal->ZoneCO2Gen(Loop).CO2GainRate;
        }

        if (state.dataHeatBal->TotITEquip > 0) {
            CalcZoneITEq(state);
        }

        CalcWaterThermalTankZoneGains(state);
        PipeHeatTransfer::PipeHTData::CalcZonePipesHeatGain(state);
        CalcWaterUseZoneGains(state);
        FigureFuelCellZoneGains(state);
        FigureMicroCHPZoneGains(state);
        initializeElectricPowerServiceZoneGains(state);
        FigureTDDZoneGains(state);
        FigureRefrigerationZoneGains(state);

        // store pointer values to hold generic internal gain values constant for entire timestep
        UpdateInternalGainValues(state);

        for (int NZ = 1; NZ <= state.dataGlobal->NumOfZones; ++NZ) {
            InternalHeatGains::SumAllInternalLatentGains(state, NZ); // Sets zone and space latent gains
            // Added for hybrid model
            if (state.dataHybridModel->FlagHybridModel_PC) {
                InternalHeatGains::SumAllInternalLatentGainsExceptPeople(state, NZ); // Also sets space gains
            }
        }

        // QL is per radiant enclosure (one or more spaces if grouped by air boundaries)
        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
            auto &thisEnclosure(state.dataViewFactor->EnclRadInfo(enclosureNum));
            thisEnclosure.radQThermalRad = 0.0;
            for (int const spaceNum : thisEnclosure.spaceNums) {
                Real64 spaceQL = SumAllSpaceInternalRadiationGains(state, spaceNum);
                thisEnclosure.radQThermalRad += spaceQL;
            }
        }

        pulseMultipler = 0.01; // the W/sqft pulse for the zone
        if (state.dataGlobal->CompLoadReportIsReq) {
            AllocateLoadComponentArrays(state);
        }
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) { // Loop through all surfaces...
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurf = thisSpace.HTSurfaceFirst;
                int const lastSurf = thisSpace.HTSurfaceLast;
                if (firstSurf <= 0) {
                    continue;
                }
                for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                    auto const &thisEnclosure = state.dataViewFactor->EnclRadInfo(state.dataSurface->Surface(SurfNum).RadEnclIndex);

                    if (!state.dataGlobal->doLoadComponentPulseNow) {
                        state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) =
                            thisEnclosure.radQThermalRad * thisEnclosure.radThermAbsMult * state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                    } else {
                        // radiant value prior to adjustment for pulse for load component report
                        Real64 const curQL = thisEnclosure.radQThermalRad;
                        // for the loads component report during the special sizing run increase the radiant portion
                        // a small amount to create a "pulse" of heat that is used for the delayed loads
                        // radiant value including adjustment for pulse for load component report
                        Real64 const adjQL = curQL + thisEnclosure.FloorArea * pulseMultipler;
                        // ITABSF is the Inside Thermal Absorptance
                        // EnclRadThermAbsMult is a multiplier for each zone
                        // SurfQdotRadIntGainsInPerArea is the thermal radiation absorbed on inside surfaces
                        state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) =
                            adjQL * thisEnclosure.radThermAbsMult * state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                        // store the magnitude and time of the pulse
                        state.dataOutRptTab->radiantPulseTimestep(state.dataSize->CurOverallSimDay, zoneNum) =
                            (state.dataGlobal->HourOfDay - 1) * state.dataGlobal->TimeStepsInHour + state.dataGlobal->TimeStep;
                        state.dataOutRptTab->radiantPulseReceived(state.dataSize->CurOverallSimDay, SurfNum) =
                            (adjQL - curQL) * thisEnclosure.radThermAbsMult * state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) *
                            state.dataSurface->Surface(SurfNum).Area;
                    }
                }
            }
        }
    }

    void CheckReturnAirHeatGain(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Xuan Luo
        //       DATE WRITTEN   Jan 2018

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine currently creates the values for standard "zone loads" reporting
        // from the heat balance module.
        for (auto const &zone : state.dataHeatBal->Zone) {
            if (zone.HasAdjustedReturnTempByITE && zone.HasLtsRetAirGain) {
                ShowFatalError(state,
                               "Return air heat gains from lights are not allowed when Air Flow Calculation Method = "
                               "FlowControlWithApproachTemperatures in zones with ITE objects.");
            }
            if (zone.HasAdjustedReturnTempByITE && zone.HasAirFlowWindowReturn) {
                ShowFatalError(state,
                               "Return air heat gains from windows are not allowed when Air Flow Calculation Method = "
                               "FlowControlWithApproachTemperatures in zones with ITE objects.");
            }
        }
    }

    void CalcZoneITEq(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         M.J. Witte
        //       DATE WRITTEN   October 2014

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the gains and other results for ElectricEquipment:ITE:AirCooled.
        // This broken into a separate subroutine, because the calculations are more detailed than the other
        // types of internal gains.

        using namespace Psychrometrics;
        using Curve::CurveValue;
        using HVAC::SmallAirVolFlow;
        using HVAC::SmallTempDiff;

        // Operating Limits for environmental class: None, A1, A2, A3, A4, B, C, H1
        // From ASHRAE 2021 Thermal Guidelines environmental classes for Air-Cooled ITE
        static constexpr std::array<Real64, static_cast<int>(ITEClass::Num)> DBMin = {
            -99.0, 15.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0}; // Minimum dry-bulb temperature [C]
        static constexpr std::array<Real64, static_cast<int>(ITEClass::Num)> DBMax = {
            99.0, 32.0, 35.0, 40.0, 45.0, 35.0, 40.0, 25.0}; // Maximum dry-bulb temperature [C]
        static constexpr std::array<Real64, static_cast<int>(ITEClass::Num)> DPMin = {
            -99.0, -12.0, -12.0, -12.0, -12.0, -99.0, -99.0, -12.0}; // Minimum dewpoint temperature [C]
        static constexpr std::array<Real64, static_cast<int>(ITEClass::Num)> DPMax = {
            99.0, 17.0, 21.0, 24.0, 24.0, 28.0, 28.0, 17.0}; // Maximum dewpoint temperature [C]
        static constexpr std::array<Real64, static_cast<int>(ITEClass::Num)> RHMin = {
            0.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0, 8.0}; // Minimum relative humidity [%]
        static constexpr std::array<Real64, static_cast<int>(ITEClass::Num)> RHMax = {
            99.0, 80.0, 80.0, 85.0, 90.0, 80.0, 80.0, 80.0}; // Maximum relative humidity [%]

        static constexpr std::string_view RoutineName("CalcZoneITEq");
        Real64 OperSchedFrac;             // Operating schedule fraction
        Real64 CPULoadSchedFrac;          // CPU loading schedule fraction
        ITEInletConnection AirConnection; // Air connection type
        Real64 TSupply(0.0);              // Supply air temperature [C]
        Real64 WSupply;                   // Supply air humidity ratio [kgWater/kgDryAir]
        Real64 RecircFrac;                // Recirculation fraction - current
        Real64 TRecirc;                   // Recirculation air temperature [C]
        Real64 WRecirc;                   // Recirculation air humidity ratio [kgWater/kgDryAir]
        Real64 TAirIn;                    // Entering air dry-bulb temperature [C]
        Real64 TAirInDesign;              // Design entering air dry-bulb temperature [C]
        Real64 WAirIn;                    // Entering air humidity ratio [kgWater/kgDryAir]
        Real64 TDPAirIn;                  // Entering air dewpoint temperature [C]
        Real64 RHAirIn;                   // Entering air relative humidity [%]
        Real64 SupplyHeatIndex;           // Supply heat index
        Real64 TAirOut;                   // Leaving air temperature [C]
        Real64 AirVolFlowFrac;            // Air volume flow fraction
        Real64 AirVolFlowFracDesignT;     // Air volume flow fraction at design entering air temperature
        Real64 AirVolFlowRate;            // Air volume flow rate at current density [m3/s]
        Real64 AirMassFlowRate;           // Air mass flow rate [kg/s]
        Real64 CPUPower;                  // CPU power input [W]
        Real64 FanPower;                  // Fan power input [W]
        Real64 UPSPower;                  // UPS new power input (losses) [W]
        Real64 UPSPartLoadRatio;          // UPS part load ratio (current total power input / design total power input)
        Real64 UPSHeatGain;               // UPS convective heat gain to zone [W]

        std::map<int, std::vector<int>> ZoneITEMap;

        //  Zero out time step variables
        // Object report variables
        for (int Loop = 1; Loop <= state.dataHeatBal->TotITEquip; ++Loop) {

            for (int i = 0; i < (int)PERptVars::Num; ++i) {
                state.dataHeatBal->ZoneITEq(Loop).PowerRpt[i] = 0.0;
                state.dataHeatBal->ZoneITEq(Loop).EnergyRpt[i] = 0.0;
            }

            state.dataHeatBal->ZoneITEq(Loop).AirVolFlowStdDensity = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).AirVolFlowCurDensity = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).AirMassFlow = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).AirInletDryBulbT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).AirInletDewpointT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).AirInletRelHum = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).AirOutletDryBulbT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).SHI = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeAboveDryBulbT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeBelowDryBulbT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeAboveDewpointT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeBelowDewpointT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeAboveRH = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).TimeBelowRH = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).DryBulbTAboveDeltaT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).DryBulbTBelowDeltaT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).DewpointTAboveDeltaT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).DewpointTBelowDeltaT = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).RHAboveDeltaRH = 0.0;
            state.dataHeatBal->ZoneITEq(Loop).RHBelowDeltaRH = 0.0;
        } // ZoneITEq init loop

        // Zone total report variables
        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {

            for (int i = 0; i < (int)PERptVars::Num; ++i) {
                state.dataHeatBal->ZoneRpt(Loop).PowerRpt[i] = 0.0;
                state.dataHeatBal->ZoneRpt(Loop).EnergyRpt[i] = 0.0;
            }

            state.dataHeatBal->ZoneRpt(Loop).ITEAdjReturnTemp = 0.0;

            state.dataHeatBal->ZoneRpt(Loop).ITEqAirVolFlowStdDensity = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqAirMassFlow = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqSHI = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeOutOfOperRange = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeAboveDryBulbT = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeBelowDryBulbT = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeAboveDewpointT = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeBelowDewpointT = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeAboveRH = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).ITEqTimeBelowRH = 0.0;

            state.dataHeatBal->ZoneRpt(Loop).SumTinMinusTSup = 0.0;
            state.dataHeatBal->ZoneRpt(Loop).SumToutMinusTSup = 0.0;
        } // Zone init loop

        // Space total report variables
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {

            for (int i = 0; i < (int)PERptVars::Num; ++i) {
                state.dataHeatBal->spaceRpt(spaceNum).PowerRpt[i] = 0.0;
                state.dataHeatBal->spaceRpt(spaceNum).EnergyRpt[i] = 0.0;
            }

            state.dataHeatBal->spaceRpt(spaceNum).ITEAdjReturnTemp = 0.0;

            state.dataHeatBal->spaceRpt(spaceNum).ITEqAirVolFlowStdDensity = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqAirMassFlow = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqSHI = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDryBulbT = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDewpointT = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDewpointT = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveRH = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowRH = 0.0;

            state.dataHeatBal->spaceRpt(spaceNum).SumTinMinusTSup = 0.0;
            state.dataHeatBal->spaceRpt(spaceNum).SumToutMinusTSup = 0.0;
        } // Space init spaceNum

        for (int Loop = 1; Loop <= state.dataHeatBal->TotITEquip; ++Loop) {
            // Get schedules
            int NZ = state.dataHeatBal->ZoneITEq(Loop).ZonePtr;
            auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(NZ);
            int spaceNum = state.dataHeatBal->ZoneITEq(Loop).spaceIndex;
            OperSchedFrac = state.dataHeatBal->ZoneITEq(Loop).operSched->getCurrentVal();
            CPULoadSchedFrac = state.dataHeatBal->ZoneITEq(Loop).cpuLoadSched->getCurrentVal();

            // Determine inlet air temperature and humidity
            AirConnection = state.dataHeatBal->ZoneITEq(Loop).AirConnectionType;
            RecircFrac = 0.0;
            int SupplyNodeNum = state.dataHeatBal->ZoneITEq(Loop).SupplyAirNodeNum;
            if (state.dataHeatBal->ZoneITEq(Loop).FlowControlWithApproachTemps) {
                TSupply = state.dataLoopNodes->Node(SupplyNodeNum).Temp;
                WSupply = state.dataLoopNodes->Node(SupplyNodeNum).HumRat;
                if (state.dataHeatBal->ZoneITEq(Loop).supplyApproachTempSched != nullptr) {
                    TAirIn = TSupply + state.dataHeatBal->ZoneITEq(Loop).supplyApproachTempSched->getCurrentVal();
                } else {
                    TAirIn = TSupply + state.dataHeatBal->ZoneITEq(Loop).SupplyApproachTemp;
                }
                WAirIn = state.dataLoopNodes->Node(SupplyNodeNum).HumRat;
            } else {
                if (AirConnection == ITEInletConnection::AdjustedSupply) {
                    TSupply = state.dataLoopNodes->Node(SupplyNodeNum).Temp;
                    WSupply = state.dataLoopNodes->Node(SupplyNodeNum).HumRat;
                    if (state.dataHeatBal->ZoneITEq(Loop).RecircFLTCurve != 0) {
                        RecircFrac = state.dataHeatBal->ZoneITEq(Loop).DesignRecircFrac *
                                     CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).RecircFLTCurve, CPULoadSchedFrac, TSupply);
                    } else {
                        RecircFrac = state.dataHeatBal->ZoneITEq(Loop).DesignRecircFrac;
                    }
                    TRecirc = thisZoneHB.MAT;
                    WRecirc = thisZoneHB.airHumRat;
                    TAirIn = TRecirc * RecircFrac + TSupply * (1.0 - RecircFrac);
                    WAirIn = WRecirc * RecircFrac + WSupply * (1.0 - RecircFrac);
                } else if (AirConnection == ITEInletConnection::RoomAirModel) {
                    // Room air model option: TAirIn=TAirZone, according to EngineeringRef 17.1.4
                    TAirIn = thisZoneHB.MAT;
                    TSupply = TAirIn;
                    WAirIn = thisZoneHB.airHumRat;
                } else {
                    // TAirIn = TRoomAirNodeIn, according to EngineeringRef 17.1.4
                    if (state.dataHeatBal->ZoneITEq(Loop).inControlledZone) {
                        int ZoneAirInletNode = state.dataZoneEquip->ZoneEquipConfig(NZ).InletNode(1);
                        TSupply = state.dataLoopNodes->Node(ZoneAirInletNode).Temp;
                    } else {
                        TSupply = thisZoneHB.MAT;
                    }
                    TAirIn = thisZoneHB.MAT;
                    WAirIn = thisZoneHB.airHumRat;
                }
            }
            TDPAirIn = PsyTdpFnWPb(state, WAirIn, state.dataEnvrn->StdBaroPress, RoutineName);
            RHAirIn = 100.0 * PsyRhFnTdbWPb(state, TAirIn, WAirIn, state.dataEnvrn->StdBaroPress, RoutineName); // RHAirIn is %

            // Calculate power input and airflow
            TAirInDesign = state.dataHeatBal->ZoneITEq(Loop).DesignTAirIn;

            if (state.dataGlobal->DoingSizing && state.dataHeatBal->ZoneITEq(Loop).FlowControlWithApproachTemps) {

                TAirInDesign = state.dataHeatBal->ZoneITEq(Loop).SizingTAirIn;
                if (state.dataHeatBal->ZoneITEq(Loop).supplyApproachTempSched != nullptr) {
                    TAirInDesign = TAirInDesign + state.dataHeatBal->ZoneITEq(Loop).supplyApproachTempSched->getCurrentVal();
                } else {
                    TAirInDesign = TAirInDesign + state.dataHeatBal->ZoneITEq(Loop).SupplyApproachTemp;
                }
                OperSchedFrac = state.dataHeatBal->ZoneITEq(Loop).operSched->getCurrentVal();
                CPULoadSchedFrac = state.dataHeatBal->ZoneITEq(Loop).cpuLoadSched->getCurrentVal();
            }

            CPUPower = max(state.dataHeatBal->ZoneITEq(Loop).DesignCPUPower * OperSchedFrac *
                               CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).CPUPowerFLTCurve, CPULoadSchedFrac, TAirIn),
                           0.0);
            state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::CPUAtDesign] =
                max(state.dataHeatBal->ZoneITEq(Loop).DesignCPUPower * OperSchedFrac *
                        CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).CPUPowerFLTCurve, CPULoadSchedFrac, TAirInDesign),
                    0.0);

            AirVolFlowFrac = max(CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).AirFlowFLTCurve, CPULoadSchedFrac, TAirIn), 0.0);
            AirVolFlowRate = state.dataHeatBal->ZoneITEq(Loop).DesignAirVolFlowRate * OperSchedFrac * AirVolFlowFrac;
            if (AirVolFlowRate < SmallAirVolFlow) {
                AirVolFlowRate = 0.0;
            }
            AirVolFlowFracDesignT = max(CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).AirFlowFLTCurve, CPULoadSchedFrac, TAirInDesign), 0.0);

            FanPower = max(state.dataHeatBal->ZoneITEq(Loop).DesignFanPower * OperSchedFrac *
                               CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).FanPowerFFCurve, AirVolFlowFrac),
                           0.0);
            state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::FanAtDesign] =
                max(state.dataHeatBal->ZoneITEq(Loop).DesignFanPower * OperSchedFrac *
                        CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).FanPowerFFCurve, AirVolFlowFracDesignT),
                    0.0);

            // Calculate UPS net power input (power in less power to ITEquip) and UPS heat gain to zone
            if (state.dataHeatBal->ZoneITEq(Loop).DesignTotalPower > 0.0) {
                UPSPartLoadRatio = (CPUPower + FanPower) / state.dataHeatBal->ZoneITEq(Loop).DesignTotalPower;
            } else {
                UPSPartLoadRatio = 0.0;
            }
            if (state.dataHeatBal->ZoneITEq(Loop).UPSEfficFPLRCurve != 0) {
                UPSPower =
                    (CPUPower + FanPower) * max((1.0 - state.dataHeatBal->ZoneITEq(Loop).DesignUPSEfficiency *
                                                           CurveValue(state, state.dataHeatBal->ZoneITEq(Loop).UPSEfficFPLRCurve, UPSPartLoadRatio)),
                                                0.0);
            } else {
                UPSPower = (CPUPower + FanPower) * max((1.0 - state.dataHeatBal->ZoneITEq(Loop).DesignUPSEfficiency), 0.0);
            }
            UPSHeatGain = UPSPower * state.dataHeatBal->ZoneITEq(Loop).UPSLossToZoneFrac;

            // Calculate air outlet conditions and convective heat gain to zone

            AirMassFlowRate = AirVolFlowRate * PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, TAirIn, WAirIn, RoutineName);
            if (AirMassFlowRate > 0.0) {
                TAirOut = TAirIn + (CPUPower + FanPower) / AirMassFlowRate / PsyCpAirFnW(WAirIn);
            } else {
                TAirOut = TAirIn;
            }

            if (std::abs(TAirOut - TSupply) < SmallTempDiff) {
                TAirOut = TSupply;
            }

            if ((SupplyNodeNum != 0) && (TAirOut != TSupply)) {
                SupplyHeatIndex = (TAirIn - TSupply) / (TAirOut - TSupply);
            } else {
                SupplyHeatIndex = 0.0;
            }

            if (AirConnection == ITEInletConnection::AdjustedSupply || AirConnection == ITEInletConnection::ZoneAirNode) {
                // If not a room air model, then all ITEquip power input is a convective heat gain to the zone heat balance, plus UPS heat gain
                state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::ConGainToZone] = CPUPower + FanPower + UPSHeatGain;
            } else if (AirConnection == ITEInletConnection::RoomAirModel) {
                // Room air model option not implemented yet - set room air model outlet node conditions here
                // If a room air model, then the only convective heat gain to the zone heat balance is the UPS heat gain
                state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::ConGainToZone] = UPSHeatGain;
            }
            if (state.dataHeatBal->Zone(state.dataHeatBal->ZoneITEq(Loop).ZonePtr).HasAdjustedReturnTempByITE) {
                ZoneITEMap[state.dataHeatBal->ZoneITEq(Loop).ZonePtr].push_back(Loop);
            }
            if (state.dataGlobal->DoingSizing && state.dataHeatBal->ZoneITEq(Loop).FlowControlWithApproachTemps) {
                if (state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::FanAtDesign] +
                        state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::CPUAtDesign] >
                    state.dataHeatBal->ZoneITEq(Loop).DesignTotalPower) {
                    state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::ConGainToZone] =
                        state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::FanAtDesign] +
                        state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::CPUAtDesign];
                }
            }
            // Object report variables
            state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::CPU] = CPUPower;
            state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::Fan] = FanPower;
            state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::UPS] = UPSPower;
            // ZoneITEq( Loop ).CPUPowerAtDesign = set above
            // ZoneITEq( Loop ).FanPowerAtDesign = set above
            state.dataHeatBal->ZoneITEq(Loop).PowerRpt[(int)PERptVars::UPSGainToZone] = UPSHeatGain; // UPSGainRateToZone = UPSHeatGain;
            // ZoneITEq( Loop ).ConGainRateToZone = set above

            for (int i = 0; i < (int)PERptVars::Num; ++i) {
                state.dataHeatBal->ZoneRpt(NZ).PowerRpt[i] += state.dataHeatBal->ZoneITEq(Loop).PowerRpt[i];
                state.dataHeatBal->spaceRpt(spaceNum).PowerRpt[i] += state.dataHeatBal->ZoneITEq(Loop).PowerRpt[i];
                state.dataHeatBal->ZoneITEq(Loop).EnergyRpt[i] = state.dataHeatBal->ZoneITEq(Loop).PowerRpt[i] * state.dataGlobal->TimeStepZoneSec;
                state.dataHeatBal->ZoneRpt(NZ).EnergyRpt[i] += state.dataHeatBal->ZoneITEq(Loop).EnergyRpt[i];
                state.dataHeatBal->spaceRpt(spaceNum).EnergyRpt[i] += state.dataHeatBal->ZoneITEq(Loop).EnergyRpt[i];
            }

            state.dataHeatBal->ZoneITEq(Loop).AirVolFlowStdDensity = AirMassFlowRate / state.dataEnvrn->StdRhoAir;
            state.dataHeatBal->ZoneITEq(Loop).AirVolFlowCurDensity = AirVolFlowRate;
            state.dataHeatBal->ZoneITEq(Loop).AirMassFlow = AirMassFlowRate;
            state.dataHeatBal->ZoneITEq(Loop).AirInletDryBulbT = TAirIn;
            state.dataHeatBal->ZoneITEq(Loop).AirInletDewpointT = TDPAirIn;
            state.dataHeatBal->ZoneITEq(Loop).AirInletRelHum = RHAirIn;
            state.dataHeatBal->ZoneITEq(Loop).AirOutletDryBulbT = TAirOut;
            state.dataHeatBal->ZoneITEq(Loop).SHI = SupplyHeatIndex;

            state.dataHeatBal->ZoneRpt(NZ).ITEqAirVolFlowStdDensity += state.dataHeatBal->ZoneITEq(Loop).AirVolFlowStdDensity;
            state.dataHeatBal->ZoneRpt(NZ).ITEqAirMassFlow += state.dataHeatBal->ZoneITEq(Loop).AirMassFlow;
            state.dataHeatBal->ZoneRpt(NZ).SumTinMinusTSup += (TAirIn - TSupply) * AirVolFlowRate;
            state.dataHeatBal->ZoneRpt(NZ).SumToutMinusTSup += (TAirOut - TSupply) * AirVolFlowRate;

            state.dataHeatBal->spaceRpt(spaceNum).ITEqAirVolFlowStdDensity += state.dataHeatBal->ZoneITEq(Loop).AirVolFlowStdDensity;
            state.dataHeatBal->spaceRpt(spaceNum).ITEqAirMassFlow += state.dataHeatBal->ZoneITEq(Loop).AirMassFlow;
            state.dataHeatBal->spaceRpt(spaceNum).SumTinMinusTSup += (TAirIn - TSupply) * AirVolFlowRate;
            state.dataHeatBal->spaceRpt(spaceNum).SumToutMinusTSup += (TAirOut - TSupply) * AirVolFlowRate;

            // Check environmental class operating range limits (defined as parameters in this subroutine)
            // Index for environmental class (None=0, A1=1, A2=2, A3=3, A4=4, B=5, C=6, H1=7)
            int EnvClass = static_cast<int>(state.dataHeatBal->ZoneITEq(Loop).Class);
            if (EnvClass > 0) {
                if (TAirIn > DBMax[EnvClass]) {
                    state.dataHeatBal->ZoneITEq(Loop).TimeAboveDryBulbT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).DryBulbTAboveDeltaT = TAirIn - DBMax[EnvClass];
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDryBulbT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDryBulbT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                }
                if (TAirIn < DBMin[EnvClass]) {
                    state.dataHeatBal->ZoneITEq(Loop).TimeBelowDryBulbT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).DryBulbTBelowDeltaT = TAirIn - DBMin[EnvClass];
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowDryBulbT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDryBulbT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                }
                if (TDPAirIn > DPMax[EnvClass]) {
                    state.dataHeatBal->ZoneITEq(Loop).TimeAboveDewpointT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).DewpointTAboveDeltaT = TDPAirIn - DPMax[EnvClass];
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveDewpointT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveDewpointT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                }
                if (TDPAirIn < DPMin[EnvClass]) {
                    state.dataHeatBal->ZoneITEq(Loop).TimeBelowDewpointT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).DewpointTBelowDeltaT = TDPAirIn - DPMin[EnvClass];
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowDewpointT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowDewpointT = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                }
                if (RHAirIn > RHMax[EnvClass]) {
                    state.dataHeatBal->ZoneITEq(Loop).TimeAboveRH = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).RHAboveDeltaRH = RHAirIn - RHMax[EnvClass];
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeAboveRH = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeAboveRH = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                }
                if (RHAirIn < RHMin[EnvClass]) {
                    state.dataHeatBal->ZoneITEq(Loop).TimeBelowRH = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).TimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneITEq(Loop).RHBelowDeltaRH = RHAirIn - RHMin[EnvClass];
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeBelowRH = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->ZoneRpt(NZ).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeBelowRH = state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->spaceRpt(spaceNum).ITEqTimeOutOfOperRange = state.dataGlobal->TimeStepZone;
                }
            }

        } // ZoneITEq calc loop

        // Zone and space-level sensible heat index
        for (int Loop = 1; Loop <= state.dataHeatBal->TotITEquip; ++Loop) {
            int ZN = state.dataHeatBal->ZoneITEq(Loop).ZonePtr;
            int spaceNum = state.dataHeatBal->ZoneITEq(Loop).spaceIndex;
            if (state.dataHeatBal->ZoneRpt(ZN).SumToutMinusTSup != 0.0) {
                state.dataHeatBal->ZoneRpt(ZN).ITEqSHI =
                    state.dataHeatBal->ZoneRpt(ZN).SumTinMinusTSup / state.dataHeatBal->ZoneRpt(ZN).SumToutMinusTSup;
            }
            if (state.dataHeatBal->spaceRpt(spaceNum).SumToutMinusTSup != 0.0) {
                state.dataHeatBal->spaceRpt(spaceNum).ITEqSHI =
                    state.dataHeatBal->spaceRpt(spaceNum).SumTinMinusTSup / state.dataHeatBal->spaceRpt(spaceNum).SumToutMinusTSup;
            }
        }

        std::map<int, std::vector<int>>::iterator it = ZoneITEMap.begin();
        Real64 totalGain;
        Real64 totalRate;
        Real64 TAirReturn;
        while (it != ZoneITEMap.end()) {
            if (state.dataHeatBal->Zone(it->first).HasAdjustedReturnTempByITE) {
                totalGain = 0;
                totalRate = 0;
                for (int i : it->second) {
                    if (state.dataHeatBal->ZoneITEq(i).returnApproachTempSched != nullptr) {
                        TAirReturn = state.dataHeatBal->ZoneITEq(i).AirOutletDryBulbT +
                                     state.dataHeatBal->ZoneITEq(i).returnApproachTempSched->getCurrentVal();
                    } else {
                        TAirReturn = state.dataHeatBal->ZoneITEq(i).AirOutletDryBulbT + state.dataHeatBal->ZoneITEq(i).ReturnApproachTemp;
                    }
                    totalRate += state.dataHeatBal->ZoneITEq(i).AirMassFlow;
                    totalGain += state.dataHeatBal->ZoneITEq(i).AirMassFlow * TAirReturn;
                }
                if (totalRate != 0) {
                    state.dataHeatBal->Zone(it->first).AdjustedReturnTempByITE = totalGain / totalRate;
                    state.dataHeatBal->ZoneRpt(it->first).ITEAdjReturnTemp = state.dataHeatBal->Zone(it->first).AdjustedReturnTempByITE;
                }
            }
            ++it;
        }

    } // End CalcZoneITEq

    void ReportInternalHeatGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   June 1997
        //       MODIFIED       July 1997 RKS
        //       RE-ENGINEERED  December 1998 LKL

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine currently creates the values for standard "zone loads" reporting
        // from the heat balance module.

        // METHODOLOGY EMPLOYED:
        // The reporting methodology is described in the OutputDataStructure.doc
        // as the "modified modular" format.

        // REFERENCES:
        // OutputDataStructure.doc (EnergyPlus documentation)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static constexpr std::array<DataHeatBalance::IntGainType, 8> TradIntGainTypes = {DataHeatBalance::IntGainType::People,
                                                                                         DataHeatBalance::IntGainType::Lights,
                                                                                         DataHeatBalance::IntGainType::ElectricEquipment,
                                                                                         DataHeatBalance::IntGainType::ElectricEquipmentITEAirCooled,
                                                                                         DataHeatBalance::IntGainType::GasEquipment,
                                                                                         DataHeatBalance::IntGainType::HotWaterEquipment,
                                                                                         DataHeatBalance::IntGainType::SteamEquipment,
                                                                                         DataHeatBalance::IntGainType::OtherEquipment};

        for (int Loop = 1; Loop <= state.dataHeatBal->TotPeople; ++Loop) {
            auto &thisPeople = state.dataHeatBal->People(Loop);
            thisPeople.RadGainEnergy = thisPeople.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisPeople.ConGainEnergy = thisPeople.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisPeople.SenGainEnergy = thisPeople.SenGainRate * state.dataGlobal->TimeStepZoneSec;
            thisPeople.LatGainEnergy = thisPeople.LatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisPeople.TotGainEnergy = thisPeople.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotLights; ++Loop) {
            auto &thisLights = state.dataHeatBal->Lights(Loop);
            thisLights.Consumption = thisLights.Power * state.dataGlobal->TimeStepZoneSec;
            thisLights.RadGainEnergy = thisLights.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisLights.VisGainEnergy = thisLights.VisGainRate * state.dataGlobal->TimeStepZoneSec;
            thisLights.ConGainEnergy = thisLights.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisLights.RetAirGainEnergy = thisLights.RetAirGainRate * state.dataGlobal->TimeStepZoneSec;
            thisLights.TotGainEnergy = thisLights.TotGainRate * state.dataGlobal->TimeStepZoneSec;
            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataGlobal->DoOutputReporting && state.dataOutRptTab->WriteTabularFiles &&
                    (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather)) { // for weather simulations only
                    // for tabular report, accumulate the total electricity used for each Light object
                    thisLights.SumConsumption += thisLights.Consumption;
                    // for tabular report, accumulate the time when each Light has consumption (using a very small threshold instead of zero)
                    if (thisLights.Power > 0.01 * thisLights.DesignLevel) {
                        thisLights.SumTimeNotZeroCons += state.dataGlobal->TimeStepZone;
                    }
                }
            }
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotElecEquip; ++Loop) {
            auto &thisElecEquip = state.dataHeatBal->ZoneElectric(Loop);
            thisElecEquip.Consumption = thisElecEquip.Power * state.dataGlobal->TimeStepZoneSec;
            thisElecEquip.RadGainEnergy = thisElecEquip.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisElecEquip.ConGainEnergy = thisElecEquip.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisElecEquip.LatGainEnergy = thisElecEquip.LatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisElecEquip.LostEnergy = thisElecEquip.LostRate * state.dataGlobal->TimeStepZoneSec;
            thisElecEquip.TotGainEnergy = thisElecEquip.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotGasEquip; ++Loop) {
            auto &thisGasEquip = state.dataHeatBal->ZoneGas(Loop);
            thisGasEquip.Consumption = thisGasEquip.Power * state.dataGlobal->TimeStepZoneSec;
            thisGasEquip.RadGainEnergy = thisGasEquip.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisGasEquip.ConGainEnergy = thisGasEquip.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisGasEquip.LatGainEnergy = thisGasEquip.LatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisGasEquip.LostEnergy = thisGasEquip.LostRate * state.dataGlobal->TimeStepZoneSec;
            thisGasEquip.TotGainEnergy = thisGasEquip.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotOthEquip; ++Loop) {
            auto &thisOtherEquip = state.dataHeatBal->ZoneOtherEq(Loop);
            thisOtherEquip.Consumption = thisOtherEquip.Power * state.dataGlobal->TimeStepZoneSec;
            thisOtherEquip.RadGainEnergy = thisOtherEquip.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisOtherEquip.ConGainEnergy = thisOtherEquip.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisOtherEquip.LatGainEnergy = thisOtherEquip.LatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisOtherEquip.LostEnergy = thisOtherEquip.LostRate * state.dataGlobal->TimeStepZoneSec;
            thisOtherEquip.TotGainEnergy = thisOtherEquip.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotHWEquip; ++Loop) {
            auto &thisHWEquip = state.dataHeatBal->ZoneHWEq(Loop);
            thisHWEquip.Consumption = thisHWEquip.Power * state.dataGlobal->TimeStepZoneSec;
            thisHWEquip.RadGainEnergy = thisHWEquip.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisHWEquip.ConGainEnergy = thisHWEquip.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisHWEquip.LatGainEnergy = thisHWEquip.LatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisHWEquip.LostEnergy = thisHWEquip.LostRate * state.dataGlobal->TimeStepZoneSec;
            thisHWEquip.TotGainEnergy = thisHWEquip.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotStmEquip; ++Loop) {
            auto &thisSteamEquip = state.dataHeatBal->ZoneSteamEq(Loop);
            thisSteamEquip.Consumption = thisSteamEquip.Power * state.dataGlobal->TimeStepZoneSec;
            thisSteamEquip.RadGainEnergy = thisSteamEquip.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSteamEquip.ConGainEnergy = thisSteamEquip.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSteamEquip.LatGainEnergy = thisSteamEquip.LatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSteamEquip.LostEnergy = thisSteamEquip.LostRate * state.dataGlobal->TimeStepZoneSec;
            thisSteamEquip.TotGainEnergy = thisSteamEquip.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotBBHeat; ++Loop) {
            auto &thisBBHeat = state.dataHeatBal->ZoneBBHeat(Loop);
            thisBBHeat.Consumption = thisBBHeat.Power * state.dataGlobal->TimeStepZoneSec;
            thisBBHeat.RadGainEnergy = thisBBHeat.RadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisBBHeat.ConGainEnergy = thisBBHeat.ConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisBBHeat.TotGainEnergy = thisBBHeat.TotGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        // Zero zone-level values
        for (auto &e : state.dataHeatBal->ZoneIntGain) {
            e.NOFOCC = 0.0;
            e.QLTSW = 0.0;
        }

        for (auto &e : state.dataHeatBal->ZoneRpt) {
            // People
            e.PeopleRadGain = 0.0;
            e.PeopleConGain = 0.0;
            e.PeopleSenGain = 0.0;
            e.PeopleNumOcc = 0.0;
            e.PeopleLatGain = 0.0;
            e.PeopleTotGain = 0.0;
            e.PeopleRadGainRate = 0.0;
            e.PeopleConGainRate = 0.0;
            e.PeopleSenGainRate = 0.0;
            e.PeopleLatGainRate = 0.0;
            e.PeopleTotGainRate = 0.0;
            // Lights
            e.LtsPower = 0.0;
            e.LtsElecConsump = 0.0;
            e.LtsRadGain = 0.0;
            e.LtsVisGain = 0.0;
            e.LtsConGain = 0.0;
            e.LtsRetAirGain = 0.0;
            e.LtsTotGain = 0.0;
            e.LtsRadGainRate = 0.0;
            e.LtsVisGainRate = 0.0;
            e.LtsConGainRate = 0.0;
            e.LtsRetAirGainRate = 0.0;
            e.LtsTotGainRate = 0.0;
            // Baseboard Heat
            e.BaseHeatPower = 0.0;
            e.BaseHeatElecCons = 0.0;
            e.BaseHeatRadGain = 0.0;
            e.BaseHeatConGain = 0.0;
            e.BaseHeatTotGain = 0.0;
            e.BaseHeatRadGainRate = 0.0;
            e.BaseHeatConGainRate = 0.0;
            e.BaseHeatTotGainRate = 0.0;
            // Electric Equipment
            e.ElecPower = 0.0;
            e.ElecConsump = 0.0;
            e.ElecRadGain = 0.0;
            e.ElecConGain = 0.0;
            e.ElecLatGain = 0.0;
            e.ElecLost = 0.0;
            e.ElecTotGain = 0.0;
            e.ElecRadGainRate = 0.0;
            e.ElecConGainRate = 0.0;
            e.ElecLatGainRate = 0.0;
            e.ElecLostRate = 0.0;
            e.ElecTotGainRate = 0.0;
            // Gas Equipment
            e.GasPower = 0.0;
            e.GasConsump = 0.0;
            e.GasRadGain = 0.0;
            e.GasConGain = 0.0;
            e.GasLatGain = 0.0;
            e.GasLost = 0.0;
            e.GasTotGain = 0.0;
            e.GasRadGainRate = 0.0;
            e.GasConGainRate = 0.0;
            e.GasLatGainRate = 0.0;
            e.GasLostRate = 0.0;
            e.GasTotGainRate = 0.0;
            // Hot Water Equipment
            e.HWPower = 0.0;
            e.HWConsump = 0.0;
            e.HWRadGain = 0.0;
            e.HWConGain = 0.0;
            e.HWLatGain = 0.0;
            e.HWLost = 0.0;
            e.HWTotGain = 0.0;
            e.HWRadGainRate = 0.0;
            e.HWConGainRate = 0.0;
            e.HWLatGainRate = 0.0;
            e.HWLostRate = 0.0;
            e.HWTotGainRate = 0.0;
            // Steam Equipment
            e.SteamPower = 0.0;
            e.SteamConsump = 0.0;
            e.SteamRadGain = 0.0;
            e.SteamConGain = 0.0;
            e.SteamLatGain = 0.0;
            e.SteamLost = 0.0;
            e.SteamTotGain = 0.0;
            e.SteamRadGainRate = 0.0;
            e.SteamConGainRate = 0.0;
            e.SteamLatGainRate = 0.0;
            e.SteamLostRate = 0.0;
            e.SteamTotGainRate = 0.0;
            // Other Equipment
            e.OtherRadGain = 0.0;
            e.OtherConGain = 0.0;
            e.OtherLatGain = 0.0;
            e.OtherLost = 0.0;
            e.OtherTotGain = 0.0;
            e.OtherRadGainRate = 0.0;
            e.OtherConGainRate = 0.0;
            e.OtherLatGainRate = 0.0;
            e.OtherLostRate = 0.0;
            e.OtherTotGainRate = 0.0;
            // Overall Zone Variables
            e.TotRadiantGain = 0.0;
            e.TotVisHeatGain = 0.0;
            e.TotConvectiveGain = 0.0;
            e.TotLatentGain = 0.0;
            e.TotTotalHeatGain = 0.0;
            e.TotRadiantGainRate = 0.0;
            e.TotVisHeatGainRate = 0.0;
            e.TotConvectiveGainRate = 0.0;
            e.TotLatentGainRate = 0.0;
            e.TotTotalHeatGainRate = 0.0;
            // Contaminant
            // e.CO2Rate = 0.0; - cleared and accumulated in InitInternalHeatGains
            e.GCRate = 0.0;
            for (int i = 0; i < (int)Constant::eFuel::Num; ++i) {
                e.OtherPower[i] = 0.0;
                e.OtherConsump[i] = 0.0;
            }
        }

        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            auto &thisSpaceRpt = state.dataHeatBal->spaceRpt(spaceNum);
            auto &thisSpaceIntGain = state.dataHeatBal->spaceIntGain(spaceNum);
            int zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
            auto &thisZoneRpt = state.dataHeatBal->ZoneRpt(zoneNum);
            auto &thisZoneIntGain = state.dataHeatBal->ZoneIntGain(zoneNum);
            // People
            thisSpaceRpt.PeopleNumOcc = thisSpaceIntGain.NOFOCC;
            thisSpaceRpt.PeopleRadGain = thisSpaceRpt.PeopleRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.PeopleConGain = thisSpaceRpt.PeopleConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.PeopleSenGain = thisSpaceRpt.PeopleSenGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.PeopleLatGain = thisSpaceRpt.PeopleLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.PeopleTotGain = thisSpaceRpt.PeopleTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneIntGain.NOFOCC += thisSpaceIntGain.NOFOCC;
            thisZoneRpt.PeopleRadGainRate += thisSpaceRpt.PeopleRadGainRate;
            thisZoneRpt.PeopleConGainRate += thisSpaceRpt.PeopleConGainRate;
            thisZoneRpt.PeopleSenGainRate += thisSpaceRpt.PeopleSenGainRate;
            thisZoneRpt.PeopleLatGainRate += thisSpaceRpt.PeopleLatGainRate;
            thisZoneRpt.PeopleTotGainRate += thisSpaceRpt.PeopleTotGainRate;

            // General Lights
            thisSpaceRpt.LtsElecConsump = thisSpaceRpt.LtsPower * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.LtsRetAirGain = thisSpaceRpt.LtsRetAirGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.LtsRadGain = thisSpaceRpt.LtsRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.LtsTotGain = thisSpaceRpt.LtsTotGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.LtsConGain = thisSpaceRpt.LtsConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.LtsVisGain = thisSpaceRpt.LtsVisGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.LtsPower += thisSpaceRpt.LtsPower;
            thisZoneRpt.LtsRetAirGainRate += thisSpaceRpt.LtsRetAirGainRate;
            thisZoneRpt.LtsRadGainRate += thisSpaceRpt.LtsRadGainRate;
            thisZoneRpt.LtsTotGainRate += thisSpaceRpt.LtsTotGainRate;
            thisZoneRpt.LtsConGainRate += thisSpaceRpt.LtsConGainRate;
            thisZoneRpt.LtsVisGainRate += thisSpaceRpt.LtsVisGainRate;
            thisZoneIntGain.QLTSW += thisSpaceIntGain.QLTSW;

            // Electric Equipment
            thisSpaceRpt.ElecConsump = thisSpaceRpt.ElecPower * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.ElecConGain = thisSpaceRpt.ElecConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.ElecRadGain = thisSpaceRpt.ElecRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.ElecLatGain = thisSpaceRpt.ElecLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.ElecLost = thisSpaceRpt.ElecLostRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.ElecTotGainRate = thisSpaceRpt.ElecConGainRate + thisSpaceRpt.ElecRadGainRate + thisSpaceRpt.ElecLatGainRate;
            thisSpaceRpt.ElecTotGain = thisSpaceRpt.ElecTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.ElecPower += thisSpaceRpt.ElecPower;
            thisZoneRpt.ElecConGainRate += thisSpaceRpt.ElecConGainRate;
            thisZoneRpt.ElecRadGainRate += thisSpaceRpt.ElecRadGainRate;
            thisZoneRpt.ElecLatGainRate += thisSpaceRpt.ElecLatGainRate;
            thisZoneRpt.ElecLostRate += thisSpaceRpt.ElecLostRate;
            thisZoneRpt.ElecTotGainRate += thisSpaceRpt.ElecTotGainRate;

            // Gas Equipment
            thisSpaceRpt.GasConsump = thisSpaceRpt.GasPower * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.GasConGain = thisSpaceRpt.GasConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.GasRadGain = thisSpaceRpt.GasRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.GasLatGain = thisSpaceRpt.GasLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.GasLost = thisSpaceRpt.GasLostRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.GasTotGainRate = thisSpaceRpt.GasConGainRate + thisSpaceRpt.GasRadGainRate + thisSpaceRpt.GasLatGainRate;
            thisSpaceRpt.GasTotGain = thisSpaceRpt.GasTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.GasPower += thisSpaceRpt.GasPower;
            thisZoneRpt.GasConGainRate += thisSpaceRpt.GasConGainRate;
            thisZoneRpt.GasRadGainRate += thisSpaceRpt.GasRadGainRate;
            thisZoneRpt.GasLatGainRate += thisSpaceRpt.GasLatGainRate;
            thisZoneRpt.GasLostRate += thisSpaceRpt.GasLostRate;
            thisZoneRpt.GasTotGainRate += thisSpaceRpt.GasTotGainRate;

            // Hot Water Equipment
            thisSpaceRpt.HWConsump = thisSpaceRpt.HWPower * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.HWConGain = thisSpaceRpt.HWConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.HWRadGain = thisSpaceRpt.HWRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.HWLatGain = thisSpaceRpt.HWLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.HWLost = thisSpaceRpt.HWLostRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.HWTotGainRate = thisSpaceRpt.HWConGainRate + thisSpaceRpt.HWRadGainRate + thisSpaceRpt.HWLatGainRate;
            thisSpaceRpt.HWTotGain = thisSpaceRpt.HWTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.HWPower += thisSpaceRpt.HWPower;
            thisZoneRpt.HWConGainRate += thisSpaceRpt.HWConGainRate;
            thisZoneRpt.HWRadGainRate += thisSpaceRpt.HWRadGainRate;
            thisZoneRpt.HWLatGainRate += thisSpaceRpt.HWLatGainRate;
            thisZoneRpt.HWLostRate += thisSpaceRpt.HWLostRate;
            thisZoneRpt.HWTotGainRate += thisSpaceRpt.HWTotGainRate;

            // Steam Equipment
            thisSpaceRpt.SteamConsump = thisSpaceRpt.SteamPower * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.SteamConGain = thisSpaceRpt.SteamConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.SteamRadGain = thisSpaceRpt.SteamRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.SteamLatGain = thisSpaceRpt.SteamLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.SteamLost = thisSpaceRpt.SteamLostRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.SteamTotGainRate = thisSpaceRpt.SteamConGainRate + thisSpaceRpt.SteamRadGainRate + thisSpaceRpt.SteamLatGainRate;
            thisSpaceRpt.SteamTotGain = thisSpaceRpt.SteamTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.SteamPower += thisSpaceRpt.SteamPower;
            thisZoneRpt.SteamConGainRate += thisSpaceRpt.SteamConGainRate;
            thisZoneRpt.SteamRadGainRate += thisSpaceRpt.SteamRadGainRate;
            thisZoneRpt.SteamLatGainRate += thisSpaceRpt.SteamLatGainRate;
            thisZoneRpt.SteamLostRate += thisSpaceRpt.SteamLostRate;
            thisZoneRpt.SteamTotGainRate += thisSpaceRpt.SteamTotGainRate;

            // Other Equipment
            thisSpaceRpt.OtherConGain = thisSpaceRpt.OtherConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.OtherRadGain = thisSpaceRpt.OtherRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.OtherLatGain = thisSpaceRpt.OtherLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.OtherLost = thisSpaceRpt.OtherLostRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.OtherTotGainRate = thisSpaceRpt.OtherConGainRate + thisSpaceRpt.OtherRadGainRate + thisSpaceRpt.OtherLatGainRate;
            thisSpaceRpt.OtherTotGain = thisSpaceRpt.OtherTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.OtherConGainRate += thisSpaceRpt.OtherConGainRate;
            thisZoneRpt.OtherRadGainRate += thisSpaceRpt.OtherRadGainRate;
            thisZoneRpt.OtherLatGainRate += thisSpaceRpt.OtherLatGainRate;
            thisZoneRpt.OtherLostRate += thisSpaceRpt.OtherLostRate;
            thisZoneRpt.OtherTotGainRate += thisSpaceRpt.OtherTotGainRate;

            for (Constant::eFuel fuelTypeNum : state.dataHeatBal->space(spaceNum).otherEquipFuelTypeNums) {
                int fuelIdx = (int)fuelTypeNum;
                thisSpaceRpt.OtherConsump[fuelIdx] = thisSpaceRpt.OtherPower[fuelIdx] * state.dataGlobal->TimeStepZoneSec;
                thisZoneRpt.OtherPower[fuelIdx] += thisSpaceRpt.OtherPower[fuelIdx];
            }

            // Baseboard Heat
            thisSpaceRpt.BaseHeatElecCons = thisSpaceRpt.BaseHeatPower * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.BaseHeatConGain = thisSpaceRpt.BaseHeatConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.BaseHeatRadGain = thisSpaceRpt.BaseHeatRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisSpaceRpt.BaseHeatTotGainRate = thisSpaceRpt.BaseHeatConGainRate + thisSpaceRpt.BaseHeatRadGainRate;
            thisSpaceRpt.BaseHeatTotGain = thisSpaceRpt.BaseHeatTotGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.BaseHeatPower += thisSpaceRpt.BaseHeatPower;
            thisZoneRpt.BaseHeatConGainRate += thisSpaceRpt.BaseHeatConGainRate;
            thisZoneRpt.BaseHeatRadGainRate += thisSpaceRpt.BaseHeatRadGainRate;
            thisZoneRpt.BaseHeatTotGainRate += thisSpaceRpt.BaseHeatTotGainRate;

            // Overall Space Variables

            // these overalls include component gains from devices like water heater, water use, and generators
            //   working vars QFCConv QGenConv QFCRad QGenRad  WaterUseLatentGain WaterThermalTankGain WaterUseSensibleGain

            thisSpaceRpt.TotVisHeatGain = thisSpaceRpt.LtsVisGain;
            thisSpaceRpt.TotVisHeatGainRate = thisSpaceRpt.LtsVisGainRate;

            thisSpaceRpt.TotRadiantGainRate = SumInternalRadiationGainsByTypes(state, zoneNum, TradIntGainTypes, spaceNum);
            thisSpaceRpt.TotRadiantGain = thisSpaceRpt.TotRadiantGainRate * state.dataGlobal->TimeStepZoneSec;

            thisSpaceRpt.TotConvectiveGainRate = SumInternalConvectionGainsByTypes(state, zoneNum, TradIntGainTypes, spaceNum);
            thisSpaceRpt.TotConvectiveGain = thisSpaceRpt.TotConvectiveGainRate * state.dataGlobal->TimeStepZoneSec;

            thisSpaceRpt.TotLatentGainRate = SumInternalLatentGainsByTypes(state, zoneNum, TradIntGainTypes, spaceNum);
            thisSpaceRpt.TotLatentGain = thisSpaceRpt.TotLatentGainRate * state.dataGlobal->TimeStepZoneSec;

            thisSpaceRpt.TotTotalHeatGainRate = thisSpaceRpt.TotLatentGainRate + thisSpaceRpt.TotRadiantGainRate +
                                                thisSpaceRpt.TotConvectiveGainRate + thisSpaceRpt.TotVisHeatGainRate;
            thisSpaceRpt.TotTotalHeatGain = thisSpaceRpt.TotTotalHeatGainRate * state.dataGlobal->TimeStepZoneSec;
        }

        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            auto &thisZoneRpt = state.dataHeatBal->ZoneRpt(zoneNum);
            auto &thisZoneIntGain = state.dataHeatBal->ZoneIntGain(zoneNum);

            // People
            thisZoneRpt.PeopleNumOcc = thisZoneIntGain.NOFOCC;
            thisZoneRpt.PeopleRadGain = thisZoneRpt.PeopleRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.PeopleConGain = thisZoneRpt.PeopleConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.PeopleSenGain = thisZoneRpt.PeopleSenGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.PeopleLatGain = thisZoneRpt.PeopleLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.PeopleTotGain = thisZoneRpt.PeopleTotGainRate * state.dataGlobal->TimeStepZoneSec;

            // General Lights
            thisZoneRpt.LtsRetAirGain = thisZoneRpt.LtsRetAirGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.LtsRadGain = thisZoneRpt.LtsRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.LtsTotGain = thisZoneRpt.LtsTotGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.LtsConGain = thisZoneRpt.LtsConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.LtsVisGain = thisZoneRpt.LtsVisGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.LtsElecConsump = thisZoneRpt.LtsPower * state.dataGlobal->TimeStepZoneSec;

            // Electric Equipment
            thisZoneRpt.ElecConGain = thisZoneRpt.ElecConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.ElecRadGain = thisZoneRpt.ElecRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.ElecLatGain = thisZoneRpt.ElecLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.ElecLost = thisZoneRpt.ElecLostRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.ElecConsump = thisZoneRpt.ElecPower * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.ElecTotGain = thisZoneRpt.ElecTotGainRate * state.dataGlobal->TimeStepZoneSec;

            // Gas Equipment
            thisZoneRpt.GasConGain = thisZoneRpt.GasConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.GasRadGain = thisZoneRpt.GasRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.GasLatGain = thisZoneRpt.GasLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.GasLost = thisZoneRpt.GasLostRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.GasConsump = thisZoneRpt.GasPower * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.GasTotGain = thisZoneRpt.GasTotGainRate * state.dataGlobal->TimeStepZoneSec;

            // Hot Water Equipment
            thisZoneRpt.HWConGain = thisZoneRpt.HWConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.HWRadGain = thisZoneRpt.HWRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.HWLatGain = thisZoneRpt.HWLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.HWLost = thisZoneRpt.HWLostRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.HWConsump = thisZoneRpt.HWPower * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.HWTotGain = thisZoneRpt.HWTotGainRate * state.dataGlobal->TimeStepZoneSec;

            // Steam Equipment
            thisZoneRpt.SteamConGain = thisZoneRpt.SteamConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.SteamRadGain = thisZoneRpt.SteamRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.SteamLatGain = thisZoneRpt.SteamLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.SteamLost = thisZoneRpt.SteamLostRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.SteamConsump = thisZoneRpt.SteamPower * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.SteamTotGain = thisZoneRpt.SteamTotGainRate * state.dataGlobal->TimeStepZoneSec;

            // Other Equipment
            thisZoneRpt.OtherConGain = thisZoneRpt.OtherConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.OtherRadGain = thisZoneRpt.OtherRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.OtherLatGain = thisZoneRpt.OtherLatGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.OtherLost = thisZoneRpt.OtherLostRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.OtherTotGain = thisZoneRpt.OtherTotGainRate * state.dataGlobal->TimeStepZoneSec;
            for (Constant::eFuel fuelTypeNum : state.dataHeatBal->Zone(zoneNum).otherEquipFuelTypeNums) {
                int fuelIdx = (int)fuelTypeNum;
                thisZoneRpt.OtherConsump[fuelIdx] = thisZoneRpt.OtherPower[fuelIdx] * state.dataGlobal->TimeStepZoneSec;
            }

            // Baseboard Heat
            thisZoneRpt.BaseHeatConGain = thisZoneRpt.BaseHeatConGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.BaseHeatRadGain = thisZoneRpt.BaseHeatRadGainRate * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.BaseHeatElecCons = thisZoneRpt.BaseHeatPower * state.dataGlobal->TimeStepZoneSec;
            thisZoneRpt.BaseHeatTotGain = thisZoneRpt.BaseHeatTotGainRate * state.dataGlobal->TimeStepZoneSec;

            // Overall Zone Variables

            // these overalls include component gains from devices like water heater, water use, and generators
            //   working vars QFCConv QGenConv QFCRad QGenRad  WaterUseLatentGain WaterThermalTankGain WaterUseSensibleGain

            thisZoneRpt.TotVisHeatGain = thisZoneRpt.LtsVisGain;
            thisZoneRpt.TotVisHeatGainRate = thisZoneRpt.LtsVisGainRate;

            thisZoneRpt.TotRadiantGainRate = SumInternalRadiationGainsByTypes(state, zoneNum, TradIntGainTypes);
            thisZoneRpt.TotRadiantGain = thisZoneRpt.TotRadiantGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.TotConvectiveGainRate = SumInternalConvectionGainsByTypes(state, zoneNum, TradIntGainTypes);
            thisZoneRpt.TotConvectiveGain = thisZoneRpt.TotConvectiveGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.TotLatentGainRate = SumInternalLatentGainsByTypes(state, zoneNum, TradIntGainTypes);
            thisZoneRpt.TotLatentGain = thisZoneRpt.TotLatentGainRate * state.dataGlobal->TimeStepZoneSec;

            thisZoneRpt.TotTotalHeatGainRate =
                thisZoneRpt.TotLatentGainRate + thisZoneRpt.TotRadiantGainRate + thisZoneRpt.TotConvectiveGainRate + thisZoneRpt.TotVisHeatGainRate;
            thisZoneRpt.TotTotalHeatGain = thisZoneRpt.TotTotalHeatGainRate * state.dataGlobal->TimeStepZoneSec;
        }
    }

    Real64 GetDesignLightingLevelForZone(EnergyPlusData &state, int const WhichZone) // name of zone
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2007; January 2008 - moved to InternalGains
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This routine sums the Lighting Level for a zone.
        // Will issue a severe error for illegal zone.
        // Must be called after InternalHeatGains get input.

        // Using/Aliasing
        using namespace DataHeatBalance;
        // Return value
        Real64 DesignLightingLevelSum; // Sum of design lighting level for this zone

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int Loop;

        if (state.dataInternalHeatGains->GetInternalHeatGainsInputFlag) {
            ShowFatalError(state, "GetDesignLightingLevelForZone: Function called prior to Getting Lights Input.");
        }

        DesignLightingLevelSum = 0.0;

        for (Loop = 1; Loop <= state.dataHeatBal->TotLights; ++Loop) {
            if (state.dataHeatBal->Lights(Loop).ZonePtr == WhichZone) {
                DesignLightingLevelSum += state.dataHeatBal->Lights(Loop).DesignLevel;
            }
        }

        return DesignLightingLevelSum;
    }

    bool CheckThermalComfortSchedules(bool const WorkEffSch, // Blank work efficiency schedule = true
                                      bool const CloInsSch,  // Blank clothing insulation schedule = true
                                      bool const AirVeloSch) // Blank air velocity schedule = true
    {
        bool TCSchedsPresent = false;

        if (!WorkEffSch || !CloInsSch || !AirVeloSch) {
            TCSchedsPresent = true;
        }

        return TCSchedsPresent;
    }

    void CheckLightsReplaceableMinMaxForZone(EnergyPlusData &state, int const WhichZone) // Zone Number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Daylighting is not available unless Lights (replaceable) is 0.0 or 1.0.  No dimming will be done
        // unless the lights replaceable fraction is 1.0.  This is documented in the InputOutputReference but
        // not warned about.  Also, this will sum the Zone Design Lighting level, in case the calling routine
        // would like to have an error if the lights is zero and daylighting is requested.

        // METHODOLOGY EMPLOYED:
        // Traverse the LIGHTS structure and get fraction replaceable - min/max as well as lighting
        // level for a zone.

        // Using/Aliasing
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        Real64 LightsRepMin; // Minimum Lighting replacement fraction for any lights statement for this zone
        Real64 LightsRepMax; // Maximum Lighting replacement fraction for any lights statement for this zone
        int NumLights;       // Number of Lights statement for that zone.

        if (state.dataInternalHeatGains->GetInternalHeatGainsInputFlag) {
            ShowFatalError(state, "CheckLightsReplaceableMinMaxForZone: Function called prior to Getting Lights Input.");
        }

        LightsRepMin = 99999.0;
        LightsRepMax = -99999.0;
        NumLights = 0;

        for (Loop = 1; Loop <= state.dataHeatBal->TotLights; ++Loop) {
            if (state.dataHeatBal->Lights(Loop).ZonePtr != WhichZone) {
                continue;
            }
            LightsRepMin = min(LightsRepMin, state.dataHeatBal->Lights(Loop).FractionReplaceable);
            LightsRepMax = max(LightsRepMax, state.dataHeatBal->Lights(Loop).FractionReplaceable);
            ++NumLights;
            if ((state.dataDayltg->ZoneDaylight(WhichZone).totRefPts > 0) &&
                (state.dataHeatBal->Lights(Loop).FractionReplaceable > 0.0 && state.dataHeatBal->Lights(Loop).FractionReplaceable < 1.0)) {
                ShowWarningError(state, "CheckLightsReplaceableMinMaxForZone: Fraction Replaceable must be 0.0 or 1.0 if used with daylighting.");
                ShowContinueError(state,
                                  format("..Lights=\"{}\", Fraction Replaceable will be reset to 1.0 to allow dimming controls",
                                         state.dataHeatBal->Lights(Loop).Name));
                ShowContinueError(state, format("..in Zone={}", state.dataHeatBal->Zone(WhichZone).Name));
                state.dataHeatBal->Lights(Loop).FractionReplaceable = 1.0;
            }
        }

        if (state.dataDayltg->ZoneDaylight(WhichZone).totRefPts > 0) {
            if (LightsRepMax == 0.0) {
                ShowWarningError(state,
                                 format("CheckLightsReplaceable: Zone \"{}\" has Daylighting:Controls.", state.dataHeatBal->Zone(WhichZone).Name));
                ShowContinueError(state, "but all of the LIGHTS object in that zone have zero Fraction Replaceable.");
                ShowContinueError(state, "The daylighting controls will have no effect.");
            }
            if (NumLights == 0) {
                ShowWarningError(state,
                                 format("CheckLightsReplaceable: Zone \"{}\" has Daylighting:Controls.", state.dataHeatBal->Zone(WhichZone).Name));
                ShowContinueError(state, "but there are no LIGHTS objects in that zone.");
                ShowContinueError(state, "The daylighting controls will have no effect.");
            }
        }
    }

    void UpdateInternalGainValues(EnergyPlusData &state, bool const SuppressRadiationUpdate, bool const SumLatentGains)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011
        bool DoRadiationUpdate{!SuppressRadiationUpdate};

        // store pointer values to hold generic internal gain values constant for entire timestep
        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
            auto &thisIntGain = state.dataHeatBal->spaceIntGainDevices(spaceNum);
            for (int Loop = 1; Loop <= thisIntGain.numberOfDevices; ++Loop) {
                thisIntGain.device(Loop).ConvectGainRate = *thisIntGain.device(Loop).PtrConvectGainRate * thisIntGain.device(Loop).spaceGainFrac;
                thisIntGain.device(Loop).ReturnAirConvGainRate =
                    *thisIntGain.device(Loop).PtrReturnAirConvGainRate * thisIntGain.device(Loop).spaceGainFrac;
                if (DoRadiationUpdate) {
                    thisIntGain.device(Loop).RadiantGainRate = *thisIntGain.device(Loop).PtrRadiantGainRate * thisIntGain.device(Loop).spaceGainFrac;
                }
                thisIntGain.device(Loop).LatentGainRate = *thisIntGain.device(Loop).PtrLatentGainRate * thisIntGain.device(Loop).spaceGainFrac;
                thisIntGain.device(Loop).ReturnAirLatentGainRate =
                    *thisIntGain.device(Loop).PtrReturnAirLatentGainRate * thisIntGain.device(Loop).spaceGainFrac;
                thisIntGain.device(Loop).CarbonDioxideGainRate =
                    *thisIntGain.device(Loop).PtrCarbonDioxideGainRate * thisIntGain.device(Loop).spaceGainFrac;
                thisIntGain.device(Loop).GenericContamGainRate =
                    *thisIntGain.device(Loop).PtrGenericContamGainRate * thisIntGain.device(Loop).spaceGainFrac;
            }
        }
        if (SumLatentGains) {
            for (int NZ = 1; NZ <= state.dataGlobal->NumOfZones; ++NZ) {
                InternalHeatGains::SumAllInternalLatentGains(state, NZ);
                // Added for the hybrid model
                if (state.dataHybridModel->FlagHybridModel_PC) {
                    InternalHeatGains::SumAllInternalLatentGainsExceptPeople(state, NZ);
                }
            }
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation && allocated(state.dataContaminantBalance->ZoneGCGain)) {
            for (int NZ = 1; NZ <= state.dataGlobal->NumOfZones; ++NZ) {
                state.dataContaminantBalance->ZoneGCGain(NZ) = InternalHeatGains::SumAllInternalGenericContamGains(state, NZ);
                state.dataHeatBal->ZoneRpt(NZ).GCRate = state.dataContaminantBalance->ZoneGCGain(NZ);
            }
        }
    }

    Real64 zoneSumAllInternalConvectionGains(EnergyPlusData &state,
                                             int const zoneNum // zone index pointer to sum gains for
    )
    {
        Real64 zoneSumConvGainRate(0.0);
        // worker routine for summing all the internal gain types

        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }
            zoneSumConvGainRate += InternalHeatGains::spaceSumAllInternalConvectionGains(state, spaceNum);
        }

        return zoneSumConvGainRate;
    }

    Real64 spaceSumAllInternalConvectionGains(EnergyPlusData &state,
                                              int const spaceNum // space index pointer to sum gains for
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        Real64 spaceSumConvGainRate(0.0);

        for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
            spaceSumConvGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ConvectGainRate;
        }
        return spaceSumConvGainRate;
    }

    // For HybridModel
    Real64 SumAllInternalConvectionGainsExceptPeople(EnergyPlusData &state, int const ZoneNum)
    {
        // Return value
        Real64 SumConvGainRateExceptPeople = 0.0;

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType != DataHeatBalance::IntGainType::People) {
                    SumConvGainRateExceptPeople += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ConvectGainRate;
                }
            }
        }

        return SumConvGainRateExceptPeople;
    }

    Real64 SumInternalConvectionGainsByTypes(
        EnergyPlusData &state,
        int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
        gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
        int const spaceIndex)                                      // space index pointer, sum gains only for this space
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011cl

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // Return value
        Real64 SumConvGainRate = 0.0;

        int NumberOfTypes = GainTypeARR.size();

        // TODO MJW: This could be refactored to avoid duplicate code, but for now . . . .
        if (spaceIndex > 0) {
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceIndex).numberOfDevices; ++DeviceNum) {
                for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                    if (state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                        SumConvGainRate += state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).ConvectGainRate;
                    }
                }
            }
        } else {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                    continue;
                }
                for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                    for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                        if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                            SumConvGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ConvectGainRate;
                        }
                    }
                }
            }
        }

        return SumConvGainRate;
    }

    Real64 zoneSumAllReturnAirConvectionGains(EnergyPlusData &state,
                                              int const zoneNum,      // zone index pointer  to sum gains for
                                              int const returnNodeNum // return air node number
    )
    {
        Real64 zoneSumReturnAirGainRate = 0.0;
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }
            zoneSumReturnAirGainRate += InternalHeatGains::spaceSumAllReturnAirConvectionGains(state, spaceNum, returnNodeNum);
        }

        return zoneSumReturnAirGainRate;
    }

    Real64 spaceSumAllReturnAirConvectionGains(EnergyPlusData &state,
                                               int const spaceNum,     // space index pointer to sum gains for
                                               int const returnNodeNum // return air node number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        Real64 spaceSumReturnAirGainRate = 0.0;

        for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
            // If ReturnNodeNum is zero, sum for entire zone, otherwise sum only for specified ReturnNodeNum
            if ((returnNodeNum == 0) || (returnNodeNum == state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ReturnAirNodeNum)) {
                spaceSumReturnAirGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ReturnAirConvGainRate;
            }
        }

        return spaceSumReturnAirGainRate;
    }

    Real64 SumReturnAirConvectionGainsByTypes(
        EnergyPlusData &state,
        int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
        gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of integer valued gain types
        int const spaceIndex                                       // space index pointer, sum gains only for this space
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // Return value
        Real64 SumReturnAirGainRate(0.0);

        int NumberOfTypes = GainTypeARR.size();

        // TODO MJW: This could be refactored to avoid duplicate code, but for now . . . .
        if (spaceIndex > 0) {
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceIndex).numberOfDevices; ++DeviceNum) {
                for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                    if (state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                        SumReturnAirGainRate += state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).ReturnAirConvGainRate;
                    }
                }
            }
        } else {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                    continue;
                }

                for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                    for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {

                        if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                            SumReturnAirGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ReturnAirConvGainRate;
                        }
                    }
                }
            }
        }

        return SumReturnAirGainRate;
    }

    Real64 SumAllSpaceInternalRadiationGains(EnergyPlusData &state,
                                             int const spaceNum // space index pointer for which space to sum gains for
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // Return value
        Real64 sumRadGainRate(0.0);

        if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
            sumRadGainRate = 0.0;
            return sumRadGainRate;
        }

        for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
            sumRadGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).RadiantGainRate;
        }

        return sumRadGainRate;
    }

    Real64
    SumInternalRadiationGainsByTypes(EnergyPlusData &state,
                                     int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
                                     gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
                                     int const spaceIndex)                                      // space index pointer, sum gains only for this space
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // Return value
        Real64 SumRadiationGainRate(0.0);

        int NumberOfTypes = GainTypeARR.size();

        // TODO MJW: This could be refactored to avoid duplicate code, but for now . . . .
        if (spaceIndex > 0) {
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceIndex).numberOfDevices; ++DeviceNum) {
                for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                    if (state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                        SumRadiationGainRate += state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).RadiantGainRate;
                    }
                }
            }
        } else {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                    continue;
                }
                for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                    for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                        if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                            SumRadiationGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).RadiantGainRate;
                        }
                    }
                }
            }
        }

        return SumRadiationGainRate;
    }

    Real64 SumEnclosureInternalRadiationGainsByTypes(
        EnergyPlusData &state,
        int const enclosureNum,                                    // enclosure to sum gains for
        gsl::span<const DataHeatBalance::IntGainType> GainTypeARR) // variable length 1-d array of enum valued gain types
    {
        // Return value
        Real64 SumRadiationGainRate(0.0);

        int NumberOfTypes = GainTypeARR.size();

        for (int spaceNum : state.dataViewFactor->EnclRadInfo(enclosureNum).spaceNums) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                    if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                        SumRadiationGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).RadiantGainRate;
                    }
                }
            }
        }

        return SumRadiationGainRate;
    }

    void SumAllInternalLatentGains(EnergyPlusData &state,
                                   int const ZoneNum // zone index pointer for which zone to sum gains for
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        Real64 zoneLatentGainRate = 0.0;

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).latentGain = 0.0;
                continue;
            }

            Real64 spaceLatentGainRate = 0.0;
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                spaceLatentGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).LatentGainRate;
            }
            state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).latentGain = spaceLatentGainRate;
            zoneLatentGainRate += spaceLatentGainRate;
        }

        state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).latentGain = zoneLatentGainRate;
    }

    // Added for hybrid model -- calculate the latent gain from all sources except for people
    void SumAllInternalLatentGainsExceptPeople(EnergyPlusData &state,
                                               int const ZoneNum // zone index pointer for which zone to sum gains for
    )
    {
        Real64 zoneLatentGainRateExceptPeople(0.0);

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }

            Real64 spaceLatentGainRateExceptPeople = 0.0;
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType != DataHeatBalance::IntGainType::People) {
                    spaceLatentGainRateExceptPeople += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).LatentGainRate;
                }
            }
            state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).latentGainExceptPeople = spaceLatentGainRateExceptPeople;
            zoneLatentGainRateExceptPeople += spaceLatentGainRateExceptPeople;
        }

        state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).latentGainExceptPeople = zoneLatentGainRateExceptPeople;
    }

    Real64
    SumInternalLatentGainsByTypes(EnergyPlusData &state,
                                  int const ZoneNum,                                         // zone index pointer for which zone to sum gains for
                                  gsl::span<const DataHeatBalance::IntGainType> GainTypeARR, // variable length 1-d array of enum valued gain types
                                  int const spaceIndex)                                      // space index pointer, sum gains only for this space
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // Return value
        Real64 SumLatentGainRate(0.0);

        int NumberOfTypes = GainTypeARR.size();

        // TODO MJW: This could be refactored to avoid duplicate code, but for now . . . .
        if (spaceIndex > 0) {
            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceIndex).numberOfDevices; ++DeviceNum) {
                for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                    if (state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                        SumLatentGainRate += state.dataHeatBal->spaceIntGainDevices(spaceIndex).device(DeviceNum).LatentGainRate;
                    }
                }
            }
        } else {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                    continue;
                }
                for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                    for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {
                        if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                            SumLatentGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).LatentGainRate;
                        }
                    }
                }
            }
        }

        return SumLatentGainRate;
    }

    Real64 SumAllReturnAirLatentGains(EnergyPlusData &state,
                                      int const ZoneNum,      // zone index pointer for which zone to sum gains for
                                      int const ReturnNodeNum // return air node number
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Nov. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        Real64 SumRetAirLatentGainRate(0.0);

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }

            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                // If ReturnNodeNum is zero, sum for entire zone, otherwise sum only for specified ReturnNodeNum
                if ((ReturnNodeNum == 0) || (ReturnNodeNum == state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ReturnAirNodeNum)) {
                    SumRetAirLatentGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).ReturnAirLatentGainRate;
                }
            }
        }

        return SumRetAirLatentGainRate;
    }

    Real64 SumAllInternalCO2Gains(EnergyPlusData &state,
                                  int const ZoneNum // zone index pointer for which zone to sum gains for
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types

        // Return value
        Real64 SumCO2GainRate(0.0);

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }

            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                SumCO2GainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CarbonDioxideGainRate;
            }
        }

        return SumCO2GainRate;
    }

    // Added for hybrid model -- function for calculating CO2 gains except people
    Real64 SumAllInternalCO2GainsExceptPeople(EnergyPlusData &state,
                                              int const ZoneNum // zone index pointer for which zone to sum gains for
    )
    {
        // Return value
        Real64 SumCO2GainRateExceptPeople(0.0);

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }

            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType != DataHeatBalance::IntGainType::People) {
                    SumCO2GainRateExceptPeople += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CarbonDioxideGainRate;
                }
            }
        }

        return SumCO2GainRateExceptPeople;
    }

    Real64
    SumInternalCO2GainsByTypes(EnergyPlusData &state,
                               int const ZoneNum,                                        // zone index pointer for which zone to sum gains for
                               gsl::span<const DataHeatBalance::IntGainType> GainTypeARR // variable length 1-d array of integer valued gain types
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec. 2011

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gain types

        // Return value
        Real64 SumCO2GainRate(0.0);

        int NumberOfTypes = GainTypeARR.size();

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }

            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                for (int TypeNum = 0; TypeNum < NumberOfTypes; ++TypeNum) {

                    if (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == GainTypeARR[TypeNum]) {
                        SumCO2GainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CarbonDioxideGainRate;
                    }
                }
            }
        }

        return SumCO2GainRate;
    }

    Real64 SumAllInternalGenericContamGains(EnergyPlusData &state,
                                            int const ZoneNum // zone index pointer for which zone to sum gains for
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         L. Gu
        //       DATE WRITTEN   Feb. 2012

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing all the internal gain types based on the existing subroutine SumAllInternalCO2Gains

        // Return value
        Real64 SumGCGainRate(0.0);

        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
                continue;
            }

            for (int DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
                SumGCGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).GenericContamGainRate;
            }
        }

        return SumGCGainRate;
    }

    void GatherComponentLoadsIntGain(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   September 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Gather values during sizing used for loads component report.

        // METHODOLOGY EMPLOYED:
        //   Save sequence of values for report during sizing.

        // Using/Aliasing
        using namespace DataHeatBalance;

        if (state.dataGlobal->CompLoadReportIsReq && !state.dataGlobal->isPulseZoneSizing) {
            int TimeStepInDay = (state.dataGlobal->HourOfDay - 1) * state.dataGlobal->TimeStepsInHour + state.dataGlobal->TimeStep;
            for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
                auto &znCLDayTS = state.dataOutRptTab->znCompLoads[state.dataSize->CurOverallSimDay - 1].ts[TimeStepInDay - 1].spacezone[iZone - 1];
                gatherCompLoadIntGain2(state, znCLDayTS, iZone);
            }
            for (int iEncl = 1; iEncl <= state.dataViewFactor->NumOfRadiantEnclosures; ++iEncl) {
                auto &enclCLDayTS = state.dataOutRptTab->enclCompLoads[state.dataSize->CurOverallSimDay - 1].ts[TimeStepInDay - 1].encl[iEncl - 1];
                enclCLDayTS.peopleRadSeq = SumEnclosureInternalRadiationGainsByTypes(state, iEncl, IntGainTypesPeople);
                enclCLDayTS.lightLWRadSeq = SumEnclosureInternalRadiationGainsByTypes(state, iEncl, IntGainTypesLight);
                enclCLDayTS.equipRadSeq = SumEnclosureInternalRadiationGainsByTypes(state, iEncl, IntGainTypesEquip);
                enclCLDayTS.hvacLossRadSeq = SumEnclosureInternalRadiationGainsByTypes(state, iEncl, IntGainTypesHvacLoss);
                enclCLDayTS.powerGenRadSeq = SumEnclosureInternalRadiationGainsByTypes(state, iEncl, IntGainTypesPowerGen);
            }
            if (state.dataHeatBal->doSpaceHeatBalanceSizing) {
                for (int iSpace = 1; iSpace <= state.dataGlobal->NumOfZones; ++iSpace) {
                    auto &spCLDayTS =
                        state.dataOutRptTab->spCompLoads[state.dataSize->CurOverallSimDay - 1].ts[TimeStepInDay - 1].spacezone[iSpace - 1];
                    gatherCompLoadIntGain2(state, spCLDayTS, state.dataHeatBal->space(iSpace).zoneNum, iSpace);
                }
            }
        }
    }

    void
    gatherCompLoadIntGain2(EnergyPlusData &state, OutputReportTabular::compLoadsSpaceZone &szCompLoadDayTS, int const zoneNum, int const spaceNum)
    {
        // Make sure all types of internal gains have been gathered
        assert((int)(size(IntGainTypesPeople) + size(IntGainTypesLight) + size(IntGainTypesEquip) + size(IntGainTypesRefrig) +
                     size(IntGainTypesWaterUse) + size(IntGainTypesHvacLoss) + size(IntGainTypesPowerGen) + size(ExcludedIntGainTypes)) ==
               (int)DataHeatBalance::IntGainType::Num);

        szCompLoadDayTS.peopleInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesPeople, spaceNum);
        szCompLoadDayTS.peopleLatentSeq = SumInternalLatentGainsByTypes(state, zoneNum, IntGainTypesPeople, spaceNum);

        szCompLoadDayTS.lightInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesLight, spaceNum);
        szCompLoadDayTS.lightRetAirSeq = SumReturnAirConvectionGainsByTypes(state, zoneNum, IntGainTypesLight, spaceNum);

        szCompLoadDayTS.equipInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesEquip, spaceNum);
        szCompLoadDayTS.equipLatentSeq = SumInternalLatentGainsByTypes(state, zoneNum, IntGainTypesEquip, spaceNum);

        szCompLoadDayTS.refrigInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesRefrig, spaceNum);
        szCompLoadDayTS.refrigRetAirSeq = SumReturnAirConvectionGainsByTypes(state, zoneNum, IntGainTypesRefrig, spaceNum);
        szCompLoadDayTS.refrigLatentSeq = SumInternalLatentGainsByTypes(state, zoneNum, IntGainTypesRefrig, spaceNum);

        szCompLoadDayTS.waterUseInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesWaterUse, spaceNum);
        szCompLoadDayTS.waterUseLatentSeq = SumInternalLatentGainsByTypes(state, zoneNum, IntGainTypesWaterUse, spaceNum);

        szCompLoadDayTS.hvacLossInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesHvacLoss, spaceNum);

        szCompLoadDayTS.powerGenInstantSeq = SumInternalConvectionGainsByTypes(state, zoneNum, IntGainTypesPowerGen, spaceNum);
    }

    int GetInternalGainDeviceIndex(EnergyPlusData &state,
                                   int const spaceNum,                             // space index pointer for which space to sum gains for
                                   DataHeatBalance::IntGainType const intGainType, // space internal gain type enum
                                   std::string_view const intGainName)             // Internal gain name
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012

        // PURPOSE OF THIS SUBROUTINE:
        // utility to retrieve index pointer to a specific internal gain
        // the subroutine returns the index of matched internal gain device or -1 if no match found.

        int DeviceNum;
        int DeviceIndex;
        if (state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices == 0) {
            DeviceIndex = -1;
            return DeviceIndex;
        }
        for (DeviceNum = 1; DeviceNum <= state.dataHeatBal->spaceIntGainDevices(spaceNum).numberOfDevices; ++DeviceNum) {
            if ((Util::SameString(state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompObjectName, intGainName.data())) &&
                (state.dataHeatBal->spaceIntGainDevices(spaceNum).device(DeviceNum).CompType == intGainType)) {
                DeviceIndex = DeviceNum;
                break;
            } else {
                DeviceIndex = -1;
            }
        }
        return DeviceIndex;
    }

    Real64 SumInternalConvectionGainsByIndices(
        EnergyPlusData &state,
        int const numGains,                // number of device gains to sum
        const Array1D_int &deviceSpaceARR, // variable length 1-d array of integer space index pointers to include in summation
        const Array1D_int &deviceIndexARR, // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &fractionARR // array of fractional multipliers to apply to devices
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gains by index

        // Return value
        Real64 sumConvGainRate(0.0);

        assert(numGains <= isize(deviceSpaceARR));
        assert(numGains <= isize(deviceIndexARR));
        assert(numGains <= isize(fractionARR));

        for (int loop = 1; loop <= numGains; ++loop) {
            int spaceNum = deviceSpaceARR(loop);
            int deviceNum = deviceIndexARR(loop);
            Real64 deviceFraction = fractionARR(loop);
            sumConvGainRate += state.dataHeatBal->spaceIntGainDevices(spaceNum).device(deviceNum).ConvectGainRate * deviceFraction;
        }
        return sumConvGainRate;
    }

    Real64 SumInternalLatentGainsByIndices(
        EnergyPlusData &state,
        int const numGains,                // number of device gains to sum
        const Array1D_int &deviceSpaceARR, // variable length 1-d array of integer space index pointers to include in summation
        const Array1D_int &deviceIndexARR, // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &fractionARR // array of fractional multipliers to apply to devices
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gains by index

        // Return value
        Real64 sumLatentGainRate(0.0);

        assert(numGains <= isize(deviceSpaceARR));
        assert(numGains <= isize(deviceIndexARR));
        assert(numGains <= isize(fractionARR));

        for (int loop = 1; loop <= numGains; ++loop) {
            int spaceNum = deviceSpaceARR(loop);
            int deviceNum = deviceIndexARR(loop);
            Real64 deviceFraction = fractionARR(loop);
            sumLatentGainRate =
                sumLatentGainRate + state.dataHeatBal->spaceIntGainDevices(spaceNum).device(deviceNum).LatentGainRate * deviceFraction;
        }
        return sumLatentGainRate;
    }

    Real64 SumReturnAirConvectionGainsByIndices(
        EnergyPlusData &state,
        int const numGains,                // number of device gains to sum
        const Array1D_int &deviceSpaceARR, // variable length 1-d array of integer space index pointers to include in summation
        const Array1D_int &deviceIndexARR, // variable length 1-d array of integer device index pointers to include in summation
        const Array1D<Real64> &fractionARR // array of fractional multipliers to apply to devices
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012

        // PURPOSE OF THIS SUBROUTINE:
        // worker routine for summing a subset of the internal gains by index

        // Return value
        Real64 sumReturnAirGainRate(0.0);

        assert(numGains <= isize(deviceSpaceARR));
        assert(numGains <= isize(deviceIndexARR));
        assert(numGains <= isize(fractionARR));

        for (int loop = 1; loop <= numGains; ++loop) {
            int spaceNum = deviceSpaceARR(loop);
            int deviceNum = deviceIndexARR(loop);
            Real64 deviceFraction = fractionARR(loop);
            sumReturnAirGainRate =
                sumReturnAirGainRate + state.dataHeatBal->spaceIntGainDevices(spaceNum).device(deviceNum).ReturnAirConvGainRate * deviceFraction;
        }
        return sumReturnAirGainRate;
    }
} // namespace InternalHeatGains

} // namespace EnergyPlus
