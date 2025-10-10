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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACManager.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::HeatBalanceAirManager {
// Module containing the air heat balance simulation routines
// calculation (initialization) routines

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen
//       DATE WRITTEN   February 1998
//       MODIFIED       May-July 2000 Joe Huang for Comis Link

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage the air simluation heat balance on the building.

// REFERENCES:
// The heat balance method is outlined in the "Tarp Alogorithms Manual"
// The methods are also summarized in many BSO Theses and papers.

// OTHER NOTES:
// This module was created from IBLAST subroutines

enum class AirflowSpec
{
    Invalid = -1,
    FlowPerZone,
    FlowPerArea,
    FlowPerExteriorArea,
    FlowPerExteriorWallArea,
    FlowPerPerson,
    AirChanges,
    Num
};
constexpr std::array<std::string_view, static_cast<int>(AirflowSpec::Num)> airflowSpecNamesUC = {
    "FLOW/ZONE", "FLOW/AREA", "FLOW/EXTERIORAREA", "FLOW/EXTERIORWALLAREA", "FLOW/PERSON", "AIRCHANGES/HOUR"};

constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::VentilationType::Num)> ventilationTypeNamesUC = {
    "NATURAL", "INTAKE", "EXHAUST", "BALANCED"};

constexpr std::array<std::string_view, static_cast<int>(DataHeatBalance::InfVentDensityBasis::Num)> infVentDensityBasisNamesUC = {
    "OUTDOOR", "STANDARD", "INDOOR"};

constexpr std::array<std::string_view, static_cast<int>(RoomAir::RoomAirModel::Num)> roomAirModelNamesUC = {"USERDEFINED",
                                                                                                            "MIXING",
                                                                                                            "ONENODEDISPLACEMENTVENTILATION",
                                                                                                            "THREENODEDISPLACEMENTVENTILATION",
                                                                                                            "CROSSVENTILATION",
                                                                                                            "UNDERFLOORAIRDISTRIBUTIONINTERIOR",
                                                                                                            "UNDERFLOORAIRDISTRIBUTIONEXTERIOR",
                                                                                                            "AIRFLOWNETWORK"};

constexpr std::array<std::string_view, static_cast<int>(RoomAir::CouplingScheme::Num)> couplingSchemeNamesUC = {"DIRECT", "INDIRECT"};

void ManageAirHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages the heat air balance method of calculating
    // building thermal loads.  It is called from the HeatBalanceManager
    // at the time step level.  This driver manages the calls to all of
    // the other drivers and simulation algorithms.

    // Obtains and Allocates heat balance related parameters from input file
    if (state.dataHeatBalAirMgr->ManageAirHeatBalanceGetInputFlag) {
        GetAirHeatBalanceInput(state);
        state.dataHeatBalAirMgr->ManageAirHeatBalanceGetInputFlag = false;
    }

    InitAirHeatBalance(state); // Initialize all heat balance related parameters

    // Solve the zone heat balance 'Detailed' solution
    // Call the air surface heat balances
    CalcHeatBalanceAir(state);

    ReportZoneMeanAirTemp(state);
}

void GetAirHeatBalanceInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the main routine to call other input routines

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound = false;

    GetAirFlowFlag(state, ErrorsFound);

    SetZoneMassConservationFlag(state);

    // get input parameters for modeling of room air flow
    GetRoomAirModelParameters(state, ErrorsFound);

    if (ErrorsFound) {
        ShowFatalError(state, "GetAirHeatBalanceInput: Errors found in getting Air inputs");
    }
}

void GetAirFlowFlag(EnergyPlusData &state, bool &ErrorsFound) // Set to true if errors found
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Garrett Westmacott
    //       DATE WRITTEN   February 2000
    //       MODIFIED       Oct 2003, FCW: Change "Infiltration-Air Change Rate" from Sum to State

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calls the routine to get simple air flow input data.

    // METHODOLOGY EMPLOYED:
    // Modelled after 'Modual Example' in Guide for Module Developers

    state.dataHeatBal->AirFlowFlag = true; // UseSimpleAirFlow;

    GetSimpleAirModelInputs(state, ErrorsFound);
    if (state.dataHeatBal->TotInfiltration + state.dataHeatBal->TotVentilation + state.dataHeatBal->TotMixing + state.dataHeatBal->TotCrossMixing +
            state.dataHeatBal->TotRefDoorMixing >
        0) {
        static constexpr std::string_view Format_720("! <AirFlow Model>, Simple\n AirFlow Model, {}\n");
        print(state.files.eio, Format_720, "Simple");
    }
}

void SetZoneMassConservationFlag(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION :
    // AUTHOR         Bereket Nigusse, FSEC
    // DATE WRITTEN   February 2014

    // PURPOSE OF THIS SUBROUTINE :
    // This subroutine sets the zone mass conservation flag to true.

    if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance &&
        state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) {
        for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
            state.dataHeatBalFanSys->ZoneMassBalanceFlag(state.dataHeatBal->Mixing(Loop).ZonePtr) = true;
            state.dataHeatBalFanSys->ZoneMassBalanceFlag(state.dataHeatBal->Mixing(Loop).FromZone) = true;
        }
    }
}

void GetSimpleAirModelInputs(EnergyPlusData &state, bool &ErrorsFound) // IF errors found in input
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2000
    //       MODIFIED       Oct 2003,FCW: change "Infiltration-Air Change Rate" from Sum to State
    //       MODIFIED       Jan 2008,LG: Allow multiple infiltration and ventilation objects per zone
    //                      May 2009, BG: added calls to setup for possible EMS override
    //                      August 2011, TKS: added refrigeration door mixing

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the input for the "simple" air flow model.

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr VentilTempLimit = 100.0;                                   // degrees Celsius
    Real64 constexpr MixingTempLimit = 100.0;                                   // degrees Celsius
    Real64 constexpr VentilWSLimit = 40.0;                                      // m/s
    static constexpr std::string_view RoutineName("GetSimpleAirModelInputs: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetSimpleAirModelInputs";
    // Refrigeration Door Mixing Protection types, factors used to moderate mixing flow.
    Real64 constexpr RefDoorNone = 0.0;
    Real64 constexpr RefDoorAirCurtain = 0.5;
    Real64 constexpr RefDoorStripCurtain = 0.9;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlpha;  // Number of Alphas for each GetobjectItem call
    int NumNumber; // Number of Numbers for each GetobjectItem call
    int NumArgs;
    int IOStat;
    Array1D_string cAlphaFieldNames;
    Array1D_string cNumericFieldNames;
    Array1D_bool lNumericFieldBlanks;
    Array1D_bool lAlphaFieldBlanks;
    Array1D_string cAlphaArgs;
    Array1D<Real64> rNumericArgs;

    Array1D_bool RepVarSet;

    std::string StringOut;
    std::string NameThisObject;
    Array1D<Real64> TotInfilVentFlow;
    Array1D<Real64> TotMixingFlow;
    Array1D<Real64> ZoneMixingNum;
    int ConnectionNumber;
    int ZoneNumA;
    int ZoneNumB;

    // Formats
    static constexpr std::string_view Format_720(" {} Airflow Stats Nominal, {},{},{},{:.2R},{:.1R},");
    static constexpr std::string_view Format_721(
        "! <{} Airflow Stats Nominal>,Name,Schedule Name,Zone Name, Zone Floor Area {{m2}}, # Zone Occupants,{}\n");
    static constexpr std::string_view Format_722(" {}, {}\n");

    RepVarSet.dimension(state.dataGlobal->NumOfZones, true);

    // Following used for reporting
    state.dataHeatBal->ZnAirRpt.allocate(state.dataGlobal->NumOfZones);
    if (state.dataHeatBal->doSpaceHeatBalanceSizing || state.dataHeatBal->doSpaceHeatBalanceSimulation) {
        state.dataHeatBal->spaceAirRpt.allocate(state.dataGlobal->numSpaces);
    }

    for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
        std::string const &name = state.dataHeatBal->Zone(Loop).Name;
        auto &thisZnAirRpt = state.dataHeatBal->ZnAirRpt(Loop);
        thisZnAirRpt.setUpOutputVars(state, DataStringGlobals::zonePrefix, name);
        if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            for (int spaceNum : state.dataHeatBal->Zone(Loop).spaceIndexes) {
                state.dataHeatBal->spaceAirRpt(spaceNum).setUpOutputVars(
                    state, DataStringGlobals::spacePrefix, state.dataHeatBal->space(spaceNum).Name);
            }
        }

        // CurrentModuleObject='Zone'
        if (state.dataGlobal->DisplayAdvancedReportVariables) {
            SetupOutputVariable(state,
                                "Zone Phase Change Material Melting Enthalpy",
                                Constant::Units::J_kg,
                                thisZnAirRpt.SumEnthalpyM,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                name);
            SetupOutputVariable(state,
                                "Zone Phase Change Material Freezing Enthalpy",
                                Constant::Units::J_kg,
                                thisZnAirRpt.SumEnthalpyH,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                name);
        }

        SetupOutputVariable(state,
                            "Zone Exfiltration Heat Transfer Rate",
                            Constant::Units::W,
                            thisZnAirRpt.ExfilTotalLoss,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
        SetupOutputVariable(state,
                            "Zone Exfiltration Sensible Heat Transfer Rate",
                            Constant::Units::W,
                            thisZnAirRpt.ExfilSensiLoss,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
        SetupOutputVariable(state,
                            "Zone Exfiltration Latent Heat Transfer Rate",
                            Constant::Units::W,
                            thisZnAirRpt.ExfilLatentLoss,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
        SetupOutputVariable(state,
                            "Zone Exhaust Air Heat Transfer Rate",
                            Constant::Units::W,
                            thisZnAirRpt.ExhTotalLoss,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
        SetupOutputVariable(state,
                            "Zone Exhaust Air Sensible Heat Transfer Rate",
                            Constant::Units::W,
                            thisZnAirRpt.ExhSensiLoss,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
        SetupOutputVariable(state,
                            "Zone Exhaust Air Latent Heat Transfer Rate",
                            Constant::Units::W,
                            thisZnAirRpt.ExhLatentLoss,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
    }

    SetupOutputVariable(state,
                        "Site Total Zone Exfiltration Heat Loss",
                        Constant::Units::J,
                        state.dataHeatBal->ZoneTotalExfiltrationHeatLoss,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Environment");
    SetupOutputVariable(state,
                        "Site Total Zone Exhaust Air Heat Loss",
                        Constant::Units::J,
                        state.dataHeatBal->ZoneTotalExhaustHeatLoss,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        "Environment");

    std::string cCurrentModuleObject = "ZoneAirBalance:OutdoorAir";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    int maxAlpha = NumAlpha;
    int maxNumber = NumNumber;
    cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneMixing";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneCrossMixing";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneRefrigerationDoorMixing";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);

    cAlphaArgs.allocate(maxAlpha);
    cAlphaFieldNames.allocate(maxAlpha);
    cNumericFieldNames.allocate(maxNumber);
    rNumericArgs.dimension(maxNumber, 0.0);
    lAlphaFieldBlanks.dimension(maxAlpha, true);
    lNumericFieldBlanks.dimension(maxNumber, true);

    cCurrentModuleObject = "ZoneAirBalance:OutdoorAir";
    state.dataHeatBal->TotZoneAirBalance = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

    state.dataHeatBal->ZoneAirBalance.allocate(state.dataHeatBal->TotZoneAirBalance);

    for (int Loop = 1; Loop <= state.dataHeatBal->TotZoneAirBalance; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 Loop,
                                                                 cAlphaArgs,
                                                                 NumAlpha,
                                                                 rNumericArgs,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
        bool IsNotOK = false;
        auto &thisZoneAirBalance = state.dataHeatBal->ZoneAirBalance(Loop);
        thisZoneAirBalance.Name = cAlphaArgs(1);
        thisZoneAirBalance.ZoneName = cAlphaArgs(2);
        thisZoneAirBalance.ZonePtr = Util::FindItemInList(cAlphaArgs(2), state.dataHeatBal->Zone);
        if (thisZoneAirBalance.ZonePtr == 0) {
            ShowSevereError(state,
                            format(R"({}{}="{}", invalid (not found) {}="{}".)",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   cAlphaArgs(1),
                                   cAlphaFieldNames(2),
                                   cAlphaArgs(2)));
            ErrorsFound = true;
        } else {
            state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).zoneOABalanceIndex = Loop;
        }
        GlobalNames::IntraObjUniquenessCheck(
            state, cAlphaArgs(2), cCurrentModuleObject, cAlphaFieldNames(2), state.dataHeatBalAirMgr->UniqueZoneNames, IsNotOK);
        if (IsNotOK) {
            ShowSevereError(state,
                            format(R"({}{}="{}", a duplicated object {}="{}" is found.)",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   cAlphaArgs(1),
                                   cAlphaFieldNames(2),
                                   cAlphaArgs(2)));
            ShowContinueError(state, format("A zone can only have one {} object.", cCurrentModuleObject));
            ErrorsFound = true;
        }

        {
            thisZoneAirBalance.BalanceMethod = static_cast<DataHeatBalance::AirBalance>(
                getEnumValue(DataHeatBalance::AirBalanceTypeNamesUC,
                             Util::makeUPPER(cAlphaArgs(3)))); // Air balance method type character input-->convert to enum
            if (thisZoneAirBalance.BalanceMethod == DataHeatBalance::AirBalance::Invalid) {
                thisZoneAirBalance.BalanceMethod = DataHeatBalance::AirBalance::None;
                ShowWarningError(state,
                                 format("{}{} = {} not valid choice for {}={}",
                                        RoutineName,
                                        cAlphaFieldNames(3),
                                        cAlphaArgs(3),
                                        cCurrentModuleObject,
                                        cAlphaArgs(1)));
                ShowContinueError(state, "The default choice \"NONE\" is assigned");
            }
        }

        thisZoneAirBalance.InducedAirRate = rNumericArgs(1);
        if (rNumericArgs(1) < 0.0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid Induced Outdoor Air Due to Duct Leakage Unbalance specification [<0.0]={:.3R}",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   cAlphaArgs(1),
                                   rNumericArgs(1)));
            ErrorsFound = true;
        }

        if (lAlphaFieldBlanks(4)) {
            ShowSevereEmptyField(state, eoh, cAlphaFieldNames(4));
        } else if ((thisZoneAirBalance.inducedAirSched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
            ErrorsFound = true;
        } else if (!thisZoneAirBalance.inducedAirSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4), Clusive::In, 0.0, Clusive::In, 1.0);
            ErrorsFound = true;
        }

        // Check whether this zone is also controlled by hybrid ventilation object with ventilation control option or not
        bool ControlFlag = Avail::GetHybridVentilationControlStatus(state, thisZoneAirBalance.ZonePtr);
        if (ControlFlag && thisZoneAirBalance.BalanceMethod == DataHeatBalance::AirBalance::Quadrature) {
            thisZoneAirBalance.BalanceMethod = DataHeatBalance::AirBalance::None;
            ShowWarningError(
                state,
                format("{} = {}: This Zone ({}) is controlled by AvailabilityManager:HybridVentilation with Simple Airflow Control Type option.",
                       cCurrentModuleObject,
                       thisZoneAirBalance.Name,
                       cAlphaArgs(2)));
            ShowContinueError(state,
                              "Air balance method type QUADRATURE and Simple Airflow Control Type cannot co-exist. The NONE method is assigned");
        }

        if (thisZoneAirBalance.BalanceMethod == DataHeatBalance::AirBalance::Quadrature) {
            state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).zoneOAQuadratureSum = true;
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Sensible Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceHeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Sensible Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceHeatGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Latent Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceLatentLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Latent Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceLatentGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Total Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceTotalLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Total Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceTotalGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Current Density Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceVdotCurDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Standard Density Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceVdotStdDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Current Density Volume",
                                Constant::Units::m3,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceVolumeCurDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Standard Density Volume",
                                Constant::Units::m3,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceVolumeStdDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Mass",
                                Constant::Units::kg,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceMdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Changes per Hour",
                                Constant::Units::ach,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceAirChangeRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Fan Electricity Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnAirRpt(thisZoneAirBalance.ZonePtr).OABalanceFanElec,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Building,
                                OutputProcessor::EndUseCat::Fans,
                                "Ventilation (simple)",
                                state.dataHeatBal->Zone(thisZoneAirBalance.ZonePtr).Name);
        }
    }

    // Set up and process ZoneInfiltration:* inputs

    cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
    int numDesignFlowInfiltrationObjects = 0;
    int totDesignFlowInfiltration = 0; // Total ZoneInfiltration:DesignFlowRate instances after expansion to spaces
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> infiltrationDesignFlowRateObjects;
    InternalHeatGains::setupIHGZonesAndSpaces(
        state, cCurrentModuleObject, infiltrationDesignFlowRateObjects, numDesignFlowInfiltrationObjects, totDesignFlowInfiltration, ErrorsFound);

    cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
    int numLeakageAreaInfiltrationObjects = 0;
    int totLeakageAreaInfiltration = 0; // Total ZoneInfiltration:EffectiveLeakageArea instances after expansion to spaces
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> infiltrationLeakageAreaObjects;
    bool const zoneListNotAllowed = true;
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              infiltrationLeakageAreaObjects,
                                              numLeakageAreaInfiltrationObjects,
                                              totLeakageAreaInfiltration,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
    int numFlowCoefficientInfiltrationObjects = 0;
    int totFlowCoefficientInfiltration = 0; // Total ZoneInfiltration:FlowCoefficient instances after expansion to spaces
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> infiltrationFlowCoefficientObjects;
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              infiltrationFlowCoefficientObjects,
                                              numFlowCoefficientInfiltrationObjects,
                                              totFlowCoefficientInfiltration,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    state.dataHeatBal->TotInfiltration = totDesignFlowInfiltration + totLeakageAreaInfiltration + totFlowCoefficientInfiltration;
    state.dataHeatBal->Infiltration.allocate(state.dataHeatBal->TotInfiltration);
    state.dataHeatBalAirMgr->UniqueInfiltrationNames.reserve(static_cast<unsigned>(state.dataHeatBal->TotInfiltration));

    int infiltrationNum = 0;
    if (totDesignFlowInfiltration > 0) {
        cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
        for (int infilInputNum = 1; infilInputNum <= numDesignFlowInfiltrationObjects; ++infilInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     infilInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
            // Create one Infiltration instance for every space associated with this input object
            auto &thisInfiltrationInput = infiltrationDesignFlowRateObjects(infilInputNum);
            for (int Item1 = 1; Item1 <= thisInfiltrationInput.numOfSpaces; ++Item1) {
                ++infiltrationNum;
                auto &thisInfiltration = state.dataHeatBal->Infiltration(infiltrationNum);
                thisInfiltration.Name = thisInfiltrationInput.names(Item1);
                thisInfiltration.spaceIndex = thisInfiltrationInput.spaceNums(Item1);
                auto &thisSpace = state.dataHeatBal->space(thisInfiltration.spaceIndex);
                thisInfiltration.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                thisInfiltration.ModelType = DataHeatBalance::InfiltrationModelType::DesignFlowRate;

                if (lAlphaFieldBlanks(3)) {
                    thisInfiltration.sched =
                        Sched::GetScheduleAlwaysOn(state); // This is not an availability schedule per se, but does default to Constant-1.0
                } else if ((thisInfiltration.sched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    if (Item1 == 1) { // avoid repeated error messages from the same input object
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                        ErrorsFound = true;
                    }
                }

                // Set space flow fractions
                // Infiltration equipment design level calculation method.
                AirflowSpec flow = static_cast<AirflowSpec>(getEnumValue(airflowSpecNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
                switch (flow) {
                case AirflowSpec::FlowPerZone:
                    if (lNumericFieldBlanks(1)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(1)));
                    } else {
                        Real64 spaceFrac = 1.0;
                        if (!thisInfiltrationInput.spaceListActive && (thisInfiltrationInput.numOfSpaces > 1)) {
                            Real64 const zoneVolume = thisZone.Volume;
                            if (zoneVolume > 0.0) {
                                spaceFrac = thisSpace.Volume / zoneVolume;
                            } else {
                                ShowSevereError(state, format("{}Zone volume is zero when allocating Infiltration to Spaces.", RoutineName));
                                ShowContinueError(
                                    state,
                                    format("Occurs for {}=\"{}\" in Zone=\"{}\".", cCurrentModuleObject, thisInfiltrationInput.Name, thisZone.Name));
                                ErrorsFound = true;
                            }
                        }

                        thisInfiltration.DesignLevel = rNumericArgs(1) * spaceFrac;
                    }
                    break;

                case AirflowSpec::FlowPerArea:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(2) * thisSpace.FloorArea;
                            if (thisInfiltration.ZonePtr > 0) {
                                if (thisSpace.FloorArea <= 0.0) {
                                    ShowWarningError(state,
                                                     format("{}{}=\"{}\", {} specifies {}, but Space Floor Area = 0.  0 Infiltration will result.",
                                                            RoutineName,
                                                            cCurrentModuleObject,
                                                            thisInfiltration.Name,
                                                            cAlphaFieldNames(4),
                                                            cNumericFieldNames(2)));
                                }
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/area specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(2)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(2)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(2)));
                    }
                    break;

                case AirflowSpec::FlowPerExteriorArea:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(3) * thisSpace.ExteriorTotalSurfArea;
                            if (thisSpace.ExteriorTotalSurfArea <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Exterior Surface Area = 0.  0 Infiltration will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisInfiltration.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{} = \"{}\", invalid flow/exteriorarea specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpec::FlowPerExteriorWallArea:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(3) * thisSpace.ExtGrossWallArea;
                            if (thisSpace.ExtGrossWallArea <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Exterior Wall Area = 0.  0 Infiltration will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisInfiltration.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{} = \"{}\", invalid flow/exteriorwallarea specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpec::AirChanges:
                    if (thisInfiltration.spaceIndex != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(4) * thisSpace.Volume / Constant::rSecsInHour;
                            if (thisSpace.Volume <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Volume = 0.  0 Infiltration will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisInfiltration.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(4)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}In {} = \"{}\", invalid ACH (air changes per hour) specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(4)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(4)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltrationInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(4)));
                    }
                    break;

                default:
                    if (Item1 == 1) {
                        ShowSevereError(
                            state,
                            format("{}{}=\"{}\", invalid calculation method={}", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4)));
                        ErrorsFound = true;
                    }
                }

                thisInfiltration.ConstantTermCoef = !lNumericFieldBlanks(5) ? rNumericArgs(5) : 1.0;
                thisInfiltration.TemperatureTermCoef = !lNumericFieldBlanks(6) ? rNumericArgs(6) : 0.0;
                thisInfiltration.VelocityTermCoef = !lNumericFieldBlanks(7) ? rNumericArgs(7) : 0.0;
                thisInfiltration.VelocitySQTermCoef = !lNumericFieldBlanks(8) ? rNumericArgs(8) : 0.0;

                if (thisInfiltration.ConstantTermCoef == 0.0 && thisInfiltration.TemperatureTermCoef == 0.0 &&
                    thisInfiltration.VelocityTermCoef == 0.0 && thisInfiltration.VelocitySQTermCoef == 0.0) {
                    if (Item1 == 1) {
                        ShowWarningError(
                            state,
                            format(
                                R"({}{}="{}", in {}="{}".)", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "Infiltration Coefficients are all zero.  No Infiltration will be reported.");
                    }
                }
                thisInfiltration.densityBasis = static_cast<DataHeatBalance::InfVentDensityBasis>(
                    getEnumValue(infVentDensityBasisNamesUC, cAlphaArgs(5))); // NOLINT(modernize-use-auto)
            }
        }
    }

    if (totLeakageAreaInfiltration > 0) {
        cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
        for (int infilInputNum = 1; infilInputNum <= numLeakageAreaInfiltrationObjects; ++infilInputNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     infilInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            // Create one Infiltration instance for every space associated with this input object
            auto &thisInfiltrationInput = infiltrationLeakageAreaObjects(infilInputNum);
            for (int Item1 = 1; Item1 <= thisInfiltrationInput.numOfSpaces; ++Item1) {
                ++infiltrationNum;
                auto &thisInfiltration = state.dataHeatBal->Infiltration(infiltrationNum);
                thisInfiltration.Name = thisInfiltrationInput.names(Item1);
                thisInfiltration.spaceIndex = thisInfiltrationInput.spaceNums(Item1);
                auto &thisSpace = state.dataHeatBal->space(thisInfiltration.spaceIndex);
                thisInfiltration.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                thisInfiltration.ModelType = DataHeatBalance::InfiltrationModelType::ShermanGrimsrud;

                if (lAlphaFieldBlanks(3)) {
                    thisInfiltration.sched = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule but defaults to constant 1.0
                } else if ((thisInfiltration.sched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }
                thisInfiltration.BasicStackCoefficient = rNumericArgs(2);
                thisInfiltration.BasicWindCoefficient = rNumericArgs(3);

                if (lNumericFieldBlanks(1)) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", field {} is blank.  0 Infiltration will result.",
                                            RoutineName,
                                            cCurrentModuleObject,
                                            thisInfiltrationInput.Name,
                                            cNumericFieldNames(1)));
                } else {
                    Real64 spaceFrac = 1.0;
                    if (!thisInfiltrationInput.spaceListActive && (thisInfiltrationInput.numOfSpaces > 1)) {
                        Real64 const zoneExteriorTotalSurfArea = thisZone.ExteriorTotalSurfArea;
                        if (zoneExteriorTotalSurfArea > 0.0) {
                            spaceFrac = thisSpace.ExteriorTotalSurfArea / zoneExteriorTotalSurfArea;
                        } else {
                            ShowSevereError(state,
                                            format("{}Zone exterior surface area is zero when allocating Infiltration to Spaces.", RoutineName));
                            ShowContinueError(
                                state,
                                format("Occurs for {}=\"{}\" in Zone=\"{}\".", cCurrentModuleObject, thisInfiltrationInput.Name, thisZone.Name));
                            ErrorsFound = true;
                        }
                    }

                    thisInfiltration.LeakageArea = rNumericArgs(1) * spaceFrac;
                }
                // check if space has exterior surfaces
                if (thisInfiltration.spaceIndex > 0) {
                    if (thisSpace.ExteriorTotalSurfArea <= 0.0) {
                        ShowWarningError(state,
                                         format(R"({}{}="{}", Space="{}" does not have surfaces exposed to outdoors.)",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltrationInput.Name,
                                                thisSpace.Name));
                        ShowContinueError(state, "Infiltration model is appropriate for exterior spaces not interior spaces, simulation continues.");
                    }
                }
            }
        }
    }

    if (totFlowCoefficientInfiltration > 0) {
        cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
        for (int infilInputNum = 1; infilInputNum <= numFlowCoefficientInfiltrationObjects; ++infilInputNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     infilInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            // Create one Infiltration instance for every space associated with this input object
            auto &thisInfiltrationInput = infiltrationFlowCoefficientObjects(infilInputNum);
            for (int Item1 = 1; Item1 <= thisInfiltrationInput.numOfSpaces; ++Item1) {
                ++infiltrationNum;
                auto &thisInfiltration = state.dataHeatBal->Infiltration(infiltrationNum);
                thisInfiltration.Name = thisInfiltrationInput.names(Item1);
                thisInfiltration.spaceIndex = thisInfiltrationInput.spaceNums(Item1);
                auto &thisSpace = state.dataHeatBal->space(thisInfiltration.spaceIndex);
                thisInfiltration.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                thisInfiltration.ModelType = DataHeatBalance::InfiltrationModelType::AIM2;

                if (lAlphaFieldBlanks(3)) {
                    thisInfiltration.sched = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
                } else if ((thisInfiltration.sched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }
                thisInfiltration.AIM2StackCoefficient = rNumericArgs(2);
                thisInfiltration.PressureExponent = rNumericArgs(3);
                thisInfiltration.AIM2WindCoefficient = rNumericArgs(4);
                thisInfiltration.ShelterFactor = rNumericArgs(5);

                if (lNumericFieldBlanks(1)) {
                    ShowWarningError(state,
                                     format("{}{}=\"{}\", field {} is blank.  0 Infiltration will result.",
                                            RoutineName,
                                            cCurrentModuleObject,
                                            thisInfiltrationInput.Name,
                                            cNumericFieldNames(1)));
                } else {
                    Real64 spaceFrac = 1.0;
                    if (!thisInfiltrationInput.spaceListActive && (thisInfiltrationInput.numOfSpaces > 1)) {
                        Real64 const zoneExteriorTotalSurfArea = thisZone.ExteriorTotalSurfArea;
                        if (zoneExteriorTotalSurfArea > 0.0) {
                            spaceFrac = thisSpace.ExteriorTotalSurfArea / zoneExteriorTotalSurfArea;
                        } else {
                            ShowSevereError(state,
                                            format("{}Zone exterior surface area is zero when allocating Infiltration to Spaces.", RoutineName));
                            ShowContinueError(
                                state,
                                format("Occurs for {}=\"{}\" in Zone=\"{}\".", cCurrentModuleObject, thisInfiltrationInput.Name, thisZone.Name));
                            ErrorsFound = true;
                        }
                    }

                    thisInfiltration.FlowCoefficient = rNumericArgs(1) * spaceFrac;
                    // check if space has exterior surfaces
                    if (thisInfiltration.spaceIndex > 0) {
                        if (thisSpace.ExteriorTotalSurfArea <= 0.0) {
                            ShowWarningError(state,
                                             format(R"({}{}="{}", Space="{}" does not have surfaces exposed to outdoors.)",
                                                    RoutineName,
                                                    cCurrentModuleObject,
                                                    thisInfiltrationInput.Name,
                                                    thisSpace.Name));
                            ShowContinueError(state,
                                              "Infiltration model is appropriate for exterior spaces not interior spaces, simulation continues.");
                        }
                    }
                }
            }
        }
    }

    // setup zone-level infiltration reports
    for (int Loop = 1; Loop <= state.dataHeatBal->TotInfiltration; ++Loop) {
        if (state.dataHeatBal->Infiltration(Loop).ZonePtr > 0 &&
            !state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).zoneOAQuadratureSum) {
            // Object report variables
            SetupOutputVariable(state,
                                "Infiltration Sensible Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Infiltration(Loop).InfilHeatLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Sensible Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Infiltration(Loop).InfilHeatGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Latent Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Infiltration(Loop).InfilLatentLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Latent Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Infiltration(Loop).InfilLatentGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Total Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Infiltration(Loop).InfilTotalLoss,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Total Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->Infiltration(Loop).InfilTotalGain,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Current Density Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->Infiltration(Loop).InfilVdotCurDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Standard Density Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->Infiltration(Loop).InfilVdotStdDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Outdoor Density Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->Infiltration(Loop).InfilVdotOutDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Current Density Volume",
                                Constant::Units::m3,
                                state.dataHeatBal->Infiltration(Loop).InfilVolumeCurDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Standard Density Volume",
                                Constant::Units::m3,
                                state.dataHeatBal->Infiltration(Loop).InfilVolumeStdDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Mass",
                                Constant::Units::kg,
                                state.dataHeatBal->Infiltration(Loop).InfilMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->Infiltration(Loop).InfilMdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Current Density Air Change Rate",
                                Constant::Units::ach,
                                state.dataHeatBal->Infiltration(Loop).InfilAirChangeRateCurDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Standard Density Air Change Rate",
                                Constant::Units::ach,
                                state.dataHeatBal->Infiltration(Loop).InfilAirChangeRateStdDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Outdoor Density Air Change Rate",
                                Constant::Units::ach,
                                state.dataHeatBal->Infiltration(Loop).InfilAirChangeRateOutDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);

            if (RepVarSet(state.dataHeatBal->Infiltration(Loop).ZonePtr)) {
                RepVarSet(state.dataHeatBal->Infiltration(Loop).ZonePtr) = false;
                SetupOutputVariable(state,
                                    "Zone Infiltration Sensible Heat Loss Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilHeatLoss,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Sensible Heat Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilHeatGain,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Latent Heat Loss Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilLatentLoss,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Latent Heat Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilLatentGain,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Total Heat Loss Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilTotalLoss,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Total Heat Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilTotalGain,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Current Density Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVdotCurDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Standard Density Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVdotStdDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Outdoor Density Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVdotOutDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Current Density Volume",
                                    Constant::Units::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVolumeCurDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Standard Density Volume",
                                    Constant::Units::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVolumeStdDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Mass",
                                    Constant::Units::kg,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilMass,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilMdot,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Current Density Air Change Rate",
                                    Constant::Units::ach,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilAirChangeRateCurDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Standard Density Air Change Rate",
                                    Constant::Units::ach,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilAirChangeRateStdDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Outdoor Density Air Change Rate",
                                    Constant::Units::ach,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilAirChangeRateOutDensity,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
            }
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "Zone Infiltration",
                             state.dataHeatBal->Infiltration(Loop).Name,
                             "Air Exchange Flow Rate",
                             "[m3/s]",
                             state.dataHeatBal->Infiltration(Loop).EMSOverrideOn,
                             state.dataHeatBal->Infiltration(Loop).EMSAirFlowRateValue);
        }
    }

    // Set up and process ZoneVentilation:* inputs

    RepVarSet = true;

    cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
    int numDesignFlowVentilationObjects = 0;
    int totDesignFlowVentilation = 0; // Total ZoneVentilation:DesignFlowRate instances after expansion to spaces
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> ventilationDesignFlowRateObjects;
    InternalHeatGains::setupIHGZonesAndSpaces(
        state, cCurrentModuleObject, ventilationDesignFlowRateObjects, numDesignFlowVentilationObjects, totDesignFlowVentilation, ErrorsFound);

    cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
    int numWindStackVentilationObjects = 0;
    int totWindStackVentilation = 0; // Total ZoneVentilation:WindandStackOpenArea instances after expansion to spaces
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> ventilationWindStackObjects;
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              ventilationWindStackObjects,
                                              numWindStackVentilationObjects,
                                              totWindStackVentilation,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    state.dataHeatBal->TotVentilation = totDesignFlowVentilation + totWindStackVentilation;
    state.dataHeatBal->Ventilation.allocate(state.dataHeatBal->TotVentilation);

    int ventilationNum = 0;
    if (numDesignFlowVentilationObjects > 0) {
        cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
        for (int ventInputNum = 1; ventInputNum <= numDesignFlowVentilationObjects; ++ventInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ventInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
            auto &thisVentilationInput = ventilationDesignFlowRateObjects(ventInputNum);
            for (int Item1 = 1; Item1 <= thisVentilationInput.numOfSpaces; ++Item1) {
                ++ventilationNum;
                auto &thisVentilation = state.dataHeatBal->Ventilation(ventilationNum);
                thisVentilation.Name = thisVentilationInput.names(Item1);
                thisVentilation.spaceIndex = thisVentilationInput.spaceNums(Item1);
                auto &thisSpace = state.dataHeatBal->space(thisVentilation.spaceIndex);
                thisVentilation.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                thisVentilation.ModelType = DataHeatBalance::VentilationModelType::DesignFlowRate;
                if (lAlphaFieldBlanks(3)) {
                    thisVentilation.availSched = Sched::GetScheduleAlwaysOn(state); // Defaults to constant-1.0
                } else if ((thisVentilation.availSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    if (Item1 == 1) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                        ErrorsFound = true;
                    }
                }

                // Ventilation equipment design level calculation method
                AirflowSpec flow = static_cast<AirflowSpec>(getEnumValue(airflowSpecNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
                switch (flow) {
                case AirflowSpec::FlowPerZone:
                    thisVentilation.DesignLevel = rNumericArgs(1);
                    if (lNumericFieldBlanks(1)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Ventilation will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisVentilation.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(1)));
                    }
                    break;

                case AirflowSpec::FlowPerArea:
                    if (thisVentilation.spaceIndex != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            thisVentilation.DesignLevel = rNumericArgs(2) * thisSpace.FloorArea;
                            if (thisSpace.FloorArea <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Floor Area = 0.  0 Ventilation will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisVentilation.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(2)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/area specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisVentilation.Name,
                                                   rNumericArgs(2)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(2)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Ventilation will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisVentilation.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(2)));
                    }
                    break;

                case AirflowSpec::FlowPerPerson:
                    if (thisVentilation.spaceIndex != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisVentilation.DesignLevel = rNumericArgs(3) * thisSpace.TotOccupants;
                            if (thisSpace.TotOccupants <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Zone Total Occupants = 0.  0 Ventilation will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisVentilation.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisVentilation.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {}specifies {}, but that field is blank.  0 Ventilation will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisVentilation.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpec::AirChanges:
                    if (thisVentilation.spaceIndex != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            thisVentilation.DesignLevel = rNumericArgs(4) * thisSpace.Volume / Constant::rSecsInHour;
                            if (thisSpace.Volume <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Volume = 0.  0 Ventilation will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisVentilation.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(4)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid ACH (air changes per hour) specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisVentilation.Name,
                                                   rNumericArgs(5)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(4)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Ventilation will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisVentilation.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(4)));
                    }
                    break;

                default:
                    if (Item1 == 1) {
                        ShowSevereError(
                            state,
                            format("{}{}=\"{}\", invalid calculation method={}", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4)));
                        ErrorsFound = true;
                    }
                }

                if (cAlphaArgs(5).empty()) {
                    thisVentilation.FanType = DataHeatBalance::VentilationType::Natural;
                } else {
                    thisVentilation.FanType = static_cast<DataHeatBalance::VentilationType>(getEnumValue(ventilationTypeNamesUC, cAlphaArgs(5)));
                    if (thisVentilation.FanType == DataHeatBalance::VentilationType::Invalid) {
                        ShowSevereError(state,
                                        format(R"({}{}="{}". invalid {}="{}".)",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               thisVentilation.Name,
                                               cAlphaFieldNames(5),
                                               cAlphaArgs(5)));
                        ErrorsFound = true;
                    }
                }

                thisVentilation.FanPressure = rNumericArgs(5);
                if (thisVentilation.FanPressure < 0.0) {
                    if (Item1 == 1) {
                        ShowSevereError(
                            state,
                            format("{}{}=\"{}\", {} must be >=0", RoutineName, cCurrentModuleObject, thisVentilation.Name, cNumericFieldNames(5)));
                        ErrorsFound = true;
                    }
                }

                thisVentilation.FanEfficiency = rNumericArgs(6);
                if ((thisVentilation.FanEfficiency <= 0.0) || (thisVentilation.FanEfficiency > 1.0)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\",{} must be in range >0 and <= 1",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               thisVentilation.Name,
                                               cNumericFieldNames(6)));
                        ErrorsFound = true;
                    }
                }

                // Override any user input for cases where natural ventilation is being used
                if (thisVentilation.FanType == DataHeatBalance::VentilationType::Natural) {
                    thisVentilation.FanPressure = 0.0;
                    thisVentilation.FanEfficiency = 1.0;
                }

                thisVentilation.ConstantTermCoef = !lNumericFieldBlanks(7) ? rNumericArgs(7) : 1.0;
                thisVentilation.TemperatureTermCoef = !lNumericFieldBlanks(8) ? rNumericArgs(8) : 0.0;
                thisVentilation.VelocityTermCoef = !lNumericFieldBlanks(9) ? rNumericArgs(9) : 0.0;
                thisVentilation.VelocitySQTermCoef = !lNumericFieldBlanks(10) ? rNumericArgs(10) : 0.0;

                if (thisVentilation.ConstantTermCoef == 0.0 && thisVentilation.TemperatureTermCoef == 0.0 &&
                    thisVentilation.VelocityTermCoef == 0.0 && thisVentilation.VelocitySQTermCoef == 0.0) {
                    if (Item1 == 1) {
                        ShowWarningError(
                            state,
                            format(
                                "{}{}=\"{}\", in {}=\"{}\".", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "Ventilation Coefficients are all zero.  No Ventilation will be reported.");
                    }
                }

                if (!lNumericFieldBlanks(11)) {
                    thisVentilation.MinIndoorTemperature = rNumericArgs(11);
                } else {
                    thisVentilation.MinIndoorTemperature = -VentilTempLimit;
                }

                //    Ventilation(Loop)%MinIndoorTemperature = rNumericArgs(11)
                if ((thisVentilation.MinIndoorTemperature < -VentilTempLimit) || (thisVentilation.MinIndoorTemperature > VentilTempLimit)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\" must have {} between -100C and 100C.",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               cNumericFieldNames(11)));
                        ShowContinueError(state, format("...value entered=[{:.2R}].", rNumericArgs(11)));
                        ErrorsFound = true;
                    }
                }

                if (!lAlphaFieldBlanks(6)) {
                    thisVentilation.minIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(6));
                }

                if (Item1 == 1) { // Item check prevents this error from printing multiple times
                    if (lAlphaFieldBlanks(6)) {
                        if (lNumericFieldBlanks(11)) {
                            // ShowWarningEmptyField(state, eoh, cNumericFieldNames(11), format("The default value will be used ({:.1R})",
                            // -VentilTempLimit));
                        }
                    } else if (thisVentilation.minIndoorTempSched == nullptr) {
                        ShowWarningItemNotFound(state,
                                                eoh,
                                                cAlphaFieldNames(6),
                                                cAlphaArgs(6),
                                                format("The default value will be used ({:.1R})", thisVentilation.MinIndoorTemperature));
                    } else if (!thisVentilation.minIndoorTempSched->checkMinMaxVals(
                                   state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(6), cAlphaArgs(6), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(11)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(6), cAlphaFieldNames(11), cAlphaFieldNames(6)));
                    }
                }

                thisVentilation.MaxIndoorTemperature = !lNumericFieldBlanks(12) ? rNumericArgs(12) : VentilTempLimit;
                if ((thisVentilation.MaxIndoorTemperature < -VentilTempLimit) || (thisVentilation.MaxIndoorTemperature > VentilTempLimit)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        format("{}{} = {} must have a maximum indoor temperature between -100C and 100C",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1)));
                        ErrorsFound = true;
                    }
                }

                if (!lAlphaFieldBlanks(7)) {
                    thisVentilation.maxIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(7));
                }

                if (Item1 == 1) { // Item check prevents this error from printing multiple times
                    if (lAlphaFieldBlanks(7)) {
                        if (lNumericFieldBlanks(12)) {
                            // ShowWarningEmptyField(state, eoh, cNumericFieldNames(12), format("The default value will be used ({:.1R})",
                            // VentilTempLimit));
                        }
                    } else if (thisVentilation.maxIndoorTempSched == nullptr) {
                        ShowWarningItemNotFound(state,
                                                eoh,
                                                cAlphaFieldNames(7),
                                                cAlphaArgs(7),
                                                format("The default value will be used ({:.1R})", thisVentilation.MaxIndoorTemperature));
                    } else if (!thisVentilation.maxIndoorTempSched->checkMinMaxVals(
                                   state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(7), cAlphaArgs(7), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(12)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(7), cAlphaFieldNames(12), cAlphaFieldNames(7)));
                    }
                }

                thisVentilation.DelTemperature = !lNumericFieldBlanks(13) ? rNumericArgs(13) : -VentilTempLimit;
                //    Ventilation(Loop)%DelTemperature = rNumericArgs(13)  !  3/12/03  Negative del temp now allowed COP

                if (!lAlphaFieldBlanks(8)) {
                    thisVentilation.deltaTempSched = Sched::GetSchedule(state, cAlphaArgs(8));
                }

                if (Item1 == 1) { // Item check prevents this error from printing multiple times
                    if (lAlphaFieldBlanks(8)) {
                        if (lNumericFieldBlanks(13)) {
                            // ShowWarningEmptyField(state, eoh, cNumericFieldNames(13), format("The default value will be used ({:.1R})",
                            // VentilTempLimit));
                        }
                    } else if (thisVentilation.deltaTempSched == nullptr) {
                        ShowWarningItemNotFound(state,
                                                eoh,
                                                cAlphaFieldNames(8),
                                                cAlphaArgs(8),
                                                format("The default value will be used ({:.1R})", thisVentilation.DelTemperature));
                    } else if (!thisVentilation.deltaTempSched->checkMinVal(state, Clusive::In, -VentilTempLimit)) {
                        Sched::ShowSevereBadMin(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8), Clusive::In, -100);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(13)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(8), cAlphaFieldNames(13), cAlphaFieldNames(8)));
                    }
                }

                thisVentilation.MinOutdoorTemperature = !lNumericFieldBlanks(14) ? rNumericArgs(14) : -VentilTempLimit;
                if ((thisVentilation.MinOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MinOutdoorTemperature > VentilTempLimit)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        format("{}{} statement = {} must have {} between -100C and 100C",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               cNumericFieldNames(14)));
                        ErrorsFound = true;
                    }
                }

                if (!lAlphaFieldBlanks(9)) {
                    thisVentilation.minOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(9));
                }

                if (Item1 == 1) { // Item check prevents this error from printing multiple times
                    if (lAlphaFieldBlanks(9)) {
                        if (lNumericFieldBlanks(14)) {
                            // ShowWarningEmptyField(state, eoh, cNumericFieldNames(14), format("The default value will be used ({:.1R})",
                            // VentilTempLimit));
                        }
                    } else if (thisVentilation.minOutdoorTempSched == nullptr) {
                        ShowWarningItemNotFound(state,
                                                eoh,
                                                cAlphaFieldNames(9),
                                                cAlphaArgs(9),
                                                format("The default value will be used ({:.1R})", thisVentilation.MinOutdoorTemperature));
                    } else if (!thisVentilation.minOutdoorTempSched->checkMinMaxVals(
                                   state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(9), cAlphaArgs(9), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(14)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(9), cNumericFieldNames(14), cAlphaFieldNames(9)));
                    }
                }

                thisVentilation.MaxOutdoorTemperature = !lNumericFieldBlanks(15) ? rNumericArgs(15) : VentilTempLimit;
                if (Item1 == 1) {
                    if ((thisVentilation.MaxOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MaxOutdoorTemperature > VentilTempLimit)) {
                        ShowSevereError(state,
                                        format("{}{} statement = {} must have a {} between -100C and 100C",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               cNumericFieldNames(15)));
                        ErrorsFound = true;
                    }
                }

                if (!lAlphaFieldBlanks(10)) {
                    thisVentilation.maxOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(10));
                }

                if (Item1 == 1) { // Item check prevents this error from printing multiple times
                    if (lAlphaFieldBlanks(10)) {
                        if (lNumericFieldBlanks(15)) {
                            // ShowWarningEmptyField(state, eoh, cNumericFieldNames(15), format("The default value will be used ({:.1R})",
                            // VentilTempLimit));
                        }
                    } else if (thisVentilation.maxOutdoorTempSched == nullptr) {
                        ShowWarningItemNotFound(state,
                                                eoh,
                                                cAlphaFieldNames(10),
                                                cAlphaArgs(10),
                                                format("The default value will be used ({:.1R})", thisVentilation.MaxOutdoorTemperature));
                    } else if (!thisVentilation.maxOutdoorTempSched->checkMinMaxVals(
                                   state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(10), cAlphaArgs(10), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(15)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(10), cNumericFieldNames(15), cAlphaFieldNames(15)));
                    }
                }

                thisVentilation.MaxWindSpeed = !lNumericFieldBlanks(16) ? rNumericArgs(16) : VentilWSLimit;
                if (Item1 == 1) {
                    if ((thisVentilation.MaxWindSpeed < -VentilWSLimit) || (thisVentilation.MaxWindSpeed > VentilWSLimit)) {
                        ShowSevereError(state,
                                        format("{}{} statement = {} must have a maximum wind speed between -40 m/s and 40 m/s",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1)));
                        ErrorsFound = true;
                    }
                }

                thisVentilation.densityBasis = static_cast<DataHeatBalance::InfVentDensityBasis>(
                    getEnumValue(infVentDensityBasisNamesUC, cAlphaArgs(11))); // NOLINT(modernize-use-auto)

                // Report variables should be added for individual VENTILATION objects, in addition to zone totals below

                if (thisVentilation.ZonePtr > 0) {
                    if (RepVarSet(thisVentilation.ZonePtr) && !thisZone.zoneOAQuadratureSum) {
                        RepVarSet(thisVentilation.ZonePtr) = false;
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Outdoor Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotOutDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume",
                                            Constant::Units::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume",
                                            Constant::Units::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass",
                                            Constant::Units::kg,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMass,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMdot,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRateCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRateStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Outdoor Density Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRateOutDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Fan Electricity Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilFanElec,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name,
                                            Constant::eResource::Electricity,
                                            OutputProcessor::Group::Building,
                                            OutputProcessor::EndUseCat::Fans,
                                            "Ventilation (simple)",
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Air Inlet Temperature",
                                            Constant::Units::C,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirTemp,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                    }
                }

                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "Zone Ventilation",
                                     thisVentilation.Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     thisVentilation.EMSSimpleVentOn,
                                     thisVentilation.EMSimpleVentFlowRate);
                }
            }
        }
    }

    if (numWindStackVentilationObjects > 0) {
        cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
        for (int ventInputNum = 1; ventInputNum <= numWindStackVentilationObjects; ++ventInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ventInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            auto &thisVentilationInput = ventilationWindStackObjects(ventInputNum);
            for (int Item1 = 1; Item1 <= thisVentilationInput.numOfSpaces; ++Item1) {
                ++ventilationNum;
                auto &thisVentilation = state.dataHeatBal->Ventilation(ventilationNum);
                thisVentilation.Name = thisVentilationInput.names(Item1);
                thisVentilation.spaceIndex = thisVentilationInput.spaceNums(Item1);
                auto &thisSpace = state.dataHeatBal->space(thisVentilation.spaceIndex);
                thisVentilation.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                thisVentilation.ModelType = DataHeatBalance::VentilationModelType::WindAndStack;

                thisVentilation.availSched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0

                thisVentilation.OpenArea = rNumericArgs(1);
                if (thisVentilation.OpenArea < 0.0) {
                    ShowSevereError(
                        state, format("{}{}=\"{}\", {} must be positive.", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cNumericFieldNames(1)));
                    ErrorsFound = true;
                }

                if (lAlphaFieldBlanks(3)) {
                    thisVentilation.openAreaFracSched =
                        Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
                } else if ((thisVentilation.openAreaFracSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                thisVentilation.OpenEff = rNumericArgs(2);
                if (thisVentilation.OpenEff != Constant::AutoCalculate && (thisVentilation.OpenEff < 0.0 || thisVentilation.OpenEff > 1.0)) {
                    ShowSevereError(
                        state,
                        format("{}{}=\"{}\", {} must be between 0 and 1.", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cNumericFieldNames(2)));
                    ErrorsFound = true;
                }

                thisVentilation.EffAngle = rNumericArgs(3);
                if (thisVentilation.EffAngle < 0.0 || thisVentilation.EffAngle >= 360.0) {
                    ShowSevereError(
                        state,
                        format(
                            "{}{}=\"{}\", {} must be between 0 and 360.", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cNumericFieldNames(3)));
                    ErrorsFound = true;
                }

                thisVentilation.DH = rNumericArgs(4);
                if (thisVentilation.DH < 0.0) {
                    ShowSevereError(
                        state, format("{}{}=\"{}\", {} must be positive.", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cNumericFieldNames(4)));
                    ErrorsFound = true;
                }

                thisVentilation.DiscCoef = rNumericArgs(5);
                if (thisVentilation.DiscCoef != Constant::AutoCalculate && (thisVentilation.DiscCoef < 0.0 || thisVentilation.DiscCoef > 1.0)) {
                    ShowSevereError(
                        state,
                        format("{}{}=\"{}\", {} must be between 0 and 1.", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cNumericFieldNames(5)));
                    ErrorsFound = true;
                }

                if (!lNumericFieldBlanks(6)) {
                    thisVentilation.MinIndoorTemperature = rNumericArgs(6);
                } else {
                    thisVentilation.MinIndoorTemperature = -VentilTempLimit;
                }
                if ((thisVentilation.MinIndoorTemperature < -VentilTempLimit) || (thisVentilation.MinIndoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    format("{}{} statement = {} must have {} between -100C and 100C",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           cNumericFieldNames(6)));
                    ErrorsFound = true;
                }

                if (!lAlphaFieldBlanks(4)) {
                    thisVentilation.minIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(4));
                }

                if (lAlphaFieldBlanks(4)) {
                    if (lNumericFieldBlanks(6)) {
                        // ShowWarningEmptyField(state, eoh, cAlphaFieldNames(12), format("The default value will be used ({:.1R})",
                        // VentilTempLimit));
                    }
                } else if (thisVentilation.minIndoorTempSched == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            cAlphaFieldNames(4),
                                            cAlphaArgs(4),
                                            format("The default value will be used ({:.1R})", thisVentilation.MinIndoorTemperature));
                } else if (!thisVentilation.minIndoorTempSched->checkMinMaxVals(state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                    Sched::ShowSevereBadMinMax(
                        state, eoh, cAlphaFieldNames(4), cAlphaArgs(4), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                    ErrorsFound = true;
                } else if (!lNumericFieldBlanks(6)) {
                    ShowWarningCustom(
                        state,
                        eoh,
                        format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(4), cNumericFieldNames(6), cAlphaFieldNames(4)));
                }

                // Max indoor temperature
                if (!lNumericFieldBlanks(7)) {
                    thisVentilation.MaxIndoorTemperature = rNumericArgs(7);
                } else {
                    thisVentilation.MaxIndoorTemperature = VentilTempLimit;
                }
                if ((thisVentilation.MaxIndoorTemperature < -VentilTempLimit) || (thisVentilation.MaxIndoorTemperature > VentilTempLimit)) {
                    ShowSevereCustom(state, eoh, format("{} must be between {}C and {}C", cNumericFieldNames(7), -VentilTempLimit, VentilTempLimit));
                    ErrorsFound = true;
                }

                if (!lAlphaFieldBlanks(5)) {
                    thisVentilation.maxIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(5));
                }

                if (lAlphaFieldBlanks(5)) {
                    if (lNumericFieldBlanks(7)) {
                        // ShowWarningEmptyField(state, eoh, cNumericFieldNames(7), format("The default value will be used ({:.1R})",
                        // VentilTempLimit));
                    }
                } else if (thisVentilation.maxIndoorTempSched == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            cAlphaFieldNames(5),
                                            cAlphaArgs(5),
                                            format("The default value will be used ({:.1R})", thisVentilation.MaxIndoorTemperature));
                } else if (!thisVentilation.maxIndoorTempSched->checkMinMaxVals(state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                    Sched::ShowSevereBadMinMax(
                        state, eoh, cAlphaFieldNames(5), cAlphaArgs(5), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                    ErrorsFound = true;
                } else if (!lNumericFieldBlanks(7)) {
                    ShowWarningCustom(
                        state,
                        eoh,
                        format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(5), cNumericFieldNames(7), cAlphaFieldNames(5)));
                }

                if (!lNumericFieldBlanks(8)) {
                    thisVentilation.DelTemperature = rNumericArgs(8);
                } else {
                    thisVentilation.DelTemperature = -VentilTempLimit;
                }

                if (!lAlphaFieldBlanks(6)) {
                    thisVentilation.deltaTempSched = Sched::GetSchedule(state, cAlphaArgs(6));
                }

                if (lAlphaFieldBlanks(6)) {
                    if (lNumericFieldBlanks(8)) {
                        // ShowWarningEmptyField(state, eoh, cNumericFieldNames(8), format("The default value will be used ({:.1R})",
                        // VentilTempLimit));
                    }
                } else if (thisVentilation.deltaTempSched == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            cAlphaFieldNames(6),
                                            cAlphaArgs(6),
                                            format("The default value will be used ({:.1R})", thisVentilation.DelTemperature));
                } else if (!thisVentilation.deltaTempSched->checkMinVal(state, Clusive::In, -VentilTempLimit)) {
                    Sched::ShowSevereBadMin(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6), Clusive::In, -VentilTempLimit);
                    ErrorsFound = true;
                } else if (!lNumericFieldBlanks(8)) {
                    ShowWarningCustom(
                        state,
                        eoh,
                        format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(6), cNumericFieldNames(8), cAlphaFieldNames(6)));
                }

                // Min outdoor temp
                thisVentilation.MinOutdoorTemperature = !lNumericFieldBlanks(9) ? rNumericArgs(9) : -VentilTempLimit;
                if ((thisVentilation.MinOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MinOutdoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    format("{}{} statement = {} must have {} between -100C and 100C",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           cNumericFieldNames(9)));
                    ErrorsFound = true;
                }

                if (!lAlphaFieldBlanks(7)) {
                    thisVentilation.minOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(7));
                }

                if (lAlphaFieldBlanks(7)) {
                    if (lNumericFieldBlanks(9)) {
                        // ShowWarningEmptyField(state, eoh, cNumericFieldNames(9), format("The default value will be used ({:.1R})",
                        // VentilTempLimit));
                    }
                } else if (thisVentilation.minOutdoorTempSched == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            cAlphaFieldNames(7),
                                            cAlphaArgs(7),
                                            format("The default value will be used ({:.1R})", thisVentilation.MinOutdoorTemperature));
                } else if (!thisVentilation.minOutdoorTempSched->checkMinMaxVals(
                               state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                    Sched::ShowSevereBadMinMax(
                        state, eoh, cAlphaFieldNames(7), cAlphaArgs(7), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                    ErrorsFound = true;
                } else if (!lNumericFieldBlanks(9)) {
                    ShowWarningCustom(
                        state,
                        eoh,
                        format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(7), cNumericFieldNames(9), cAlphaFieldNames(7)));
                }

                // Max outdoor temp
                thisVentilation.MaxOutdoorTemperature = !lNumericFieldBlanks(10) ? rNumericArgs(10) : VentilTempLimit;
                if ((thisVentilation.MaxOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MaxOutdoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    format("{}{} statement = {} must have a {} between -100C and 100C",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           cNumericFieldNames(10)));
                    ErrorsFound = true;
                }

                if (!lAlphaFieldBlanks(8)) {
                    thisVentilation.maxOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(8));
                }

                if (lAlphaFieldBlanks(8)) {
                    if (lNumericFieldBlanks(10)) {
                        // ShowWarningEmptyField(state, eoh, cNumericFieldNames(10), format("The default value will be used ({:.1R})",
                        // VentilTempLimit));
                    }
                } else if (thisVentilation.maxOutdoorTempSched == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            cAlphaFieldNames(8),
                                            cAlphaArgs(8),
                                            format("The default value will be used ({:.1R})", thisVentilation.MaxOutdoorTemperature));
                } else if (!thisVentilation.maxOutdoorTempSched->checkMinMaxVals(
                               state, Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit)) {
                    Sched::ShowSevereBadMinMax(
                        state, eoh, cAlphaFieldNames(8), cAlphaArgs(8), Clusive::In, -VentilTempLimit, Clusive::In, VentilTempLimit);
                    ErrorsFound = true;
                } else if (!lNumericFieldBlanks(10)) {
                    ShowWarningCustom(
                        state,
                        eoh,
                        format("Both {} and {} provided, {} will be used.", cAlphaFieldNames(8), cNumericFieldNames(10), cAlphaFieldNames(8)));
                }

                // Max wind speed
                thisVentilation.MaxWindSpeed = !lNumericFieldBlanks(11) ? rNumericArgs(11) : VentilWSLimit;
                if ((thisVentilation.MaxWindSpeed < -VentilWSLimit) || (thisVentilation.MaxWindSpeed > VentilWSLimit)) {
                    ShowSevereError(state,
                                    format("{}{} statement = {} must have a maximum wind speed between 0 m/s and 40 m/s",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1)));
                    ErrorsFound = true;
                }

                // Report variables should be added for individual VENTILATION objects, in addition to zone totals below

                if (thisVentilation.ZonePtr > 0) {
                    if (RepVarSet(thisVentilation.ZonePtr) && !thisZone.zoneOAQuadratureSum) {
                        RepVarSet(thisVentilation.ZonePtr) = false;
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Outdoor Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotOutDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume",
                                            Constant::Units::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume",
                                            Constant::Units::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass",
                                            Constant::Units::kg,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMass,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMdot,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRateCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRateStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Outdoor Density Air Change Rate",
                                            Constant::Units::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRateOutDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Fan Electricity Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilFanElec,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            thisZone.Name,
                                            Constant::eResource::Electricity,
                                            OutputProcessor::Group::Building,
                                            OutputProcessor::EndUseCat::Fans,
                                            "Ventilation (simple)",
                                            thisZone.Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Air Inlet Temperature",
                                            Constant::Units::C,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirTemp,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            thisZone.Name);
                    }
                }

                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "Zone Ventilation",
                                     thisVentilation.Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     thisVentilation.EMSSimpleVentOn,
                                     thisVentilation.EMSimpleVentFlowRate);
                }
            }
        }
    }

    // Set up and process ZoneMixing and ZoneCrossMixing inputs

    RepVarSet = true;

    cCurrentModuleObject = "ZoneMixing";
    int numZoneMixingInputObjects = 0;
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> zoneMixingInputObjects;
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              zoneMixingInputObjects,
                                              numZoneMixingInputObjects,
                                              state.dataHeatBal->TotMixing,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    if (state.dataHeatBal->TotMixing > 0) {
        cCurrentModuleObject = "ZoneMixing";
        state.dataHeatBal->Mixing.allocate(state.dataHeatBal->TotMixing);
        int mixingNum = 0;
        for (int mixingInputNum = 1; mixingInputNum <= numZoneMixingInputObjects; ++mixingInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     mixingInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
            // Create one Mixing instance for every space associated with this input object
            auto const &thisMixingInput = zoneMixingInputObjects(mixingInputNum);
            for (int Item1 = 1; Item1 <= thisMixingInput.numOfSpaces; ++Item1) {
                ++mixingNum;
                auto &thisMixing = state.dataHeatBal->Mixing(mixingNum);
                thisMixing.Name = thisMixingInput.names(Item1);
                thisMixing.spaceIndex = thisMixingInput.spaceNums(Item1);
                auto const &thisSpace = state.dataHeatBal->space(thisMixing.spaceIndex);
                thisMixing.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                if (lAlphaFieldBlanks(3)) {
                    thisMixing.sched = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
                } else if ((thisMixing.sched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                // Mixing equipment design level calculation method
                AirflowSpec flow = static_cast<AirflowSpec>(getEnumValue(airflowSpecNamesUC, cAlphaArgs(4)));
                switch (flow) {
                case AirflowSpec::FlowPerZone:
                    thisMixing.DesignLevel = rNumericArgs(1);
                    if (lNumericFieldBlanks(1)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(1)));
                    } else {
                        Real64 spaceFrac = 1.0;
                        if (!thisMixingInput.spaceListActive && (thisMixingInput.numOfSpaces > 1)) {
                            Real64 const zoneVolume = thisZone.Volume;
                            if (zoneVolume > 0.0) {
                                spaceFrac = thisSpace.Volume / zoneVolume;
                            } else {
                                ShowSevereError(state, format("{}Zone volume is zero when allocating Mixing to Spaces.", RoutineName));
                                ShowContinueError(
                                    state, format("Occurs for {}=\"{}\" in Zone=\"{}\".", cCurrentModuleObject, thisMixingInput.Name, thisZone.Name));
                                ErrorsFound = true;
                            }
                        }

                        thisMixing.DesignLevel = rNumericArgs(1) * spaceFrac;
                    }
                    break;

                case AirflowSpec::FlowPerArea:
                    if (thisMixing.spaceIndex != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            thisMixing.DesignLevel = rNumericArgs(2) * thisSpace.FloorArea;
                            if (thisMixing.spaceIndex > 0) {
                                if (thisZone.FloorArea <= 0.0) {
                                    ShowWarningError(state,
                                                     format("{}{}=\"{}\", {} specifies {}, but Space Floor Area = 0.  0 Mixing will result.",
                                                            RoutineName,
                                                            cCurrentModuleObject,
                                                            thisMixingInput.Name,
                                                            cAlphaFieldNames(4),
                                                            cNumericFieldNames(2)));
                                }
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/area specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisMixingInput.Name,
                                                   rNumericArgs(2)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(2)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(2)));
                    }
                    break;

                case AirflowSpec::FlowPerPerson:
                    if (thisMixing.spaceIndex != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisMixing.DesignLevel = rNumericArgs(3) * thisSpace.TotOccupants;
                            if (thisSpace.TotOccupants <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Total Occupants = 0.  0 Mixing will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisMixingInput.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisMixingInput.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpec::AirChanges:
                    if (thisMixing.spaceIndex != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            thisMixing.DesignLevel = rNumericArgs(4) * thisSpace.Volume / Constant::rSecsInHour;
                            if (thisSpace.Volume <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Volume = 0.  0 Mixing will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisMixingInput.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(4)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid ACH (air changes per hour) specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisMixingInput.Name,
                                                   rNumericArgs(4)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(4)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(4)));
                    }
                    break;

                default:
                    ShowSevereError(
                        state, format("{}{}=\"{}\", invalid calculation method={}", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4)));
                    ErrorsFound = true;
                }

                thisMixing.fromSpaceIndex = Util::FindItemInList(cAlphaArgs(5), state.dataHeatBal->space);
                if (thisMixing.fromSpaceIndex == 0) {
                    thisMixing.FromZone = Util::FindItemInList(cAlphaArgs(5), state.dataHeatBal->Zone);
                } else {
                    thisMixing.FromZone = state.dataHeatBal->space(thisMixing.fromSpaceIndex).zoneNum;
                }
                if ((thisMixing.FromZone == 0) && (thisMixing.fromSpaceIndex == 0)) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(5), cAlphaArgs(5));
                    ErrorsFound = true;
                }
                thisMixing.DeltaTemperature = rNumericArgs(5);

                if (NumAlpha > 5) {
                    if (lAlphaFieldBlanks(6)) {
                        if (lNumericFieldBlanks(5)) {
                            ShowWarningCustom(state,
                                              eoh,
                                              format("{} and {} are empty, a default temperature of {:.1R}C will be used.",
                                                     cAlphaFieldNames(6),
                                                     cNumericFieldNames(5),
                                                     -MixingTempLimit));
                        }
                    } else if ((thisMixing.deltaTempSched = Sched::GetSchedule(state, cAlphaArgs(6))) == nullptr) {
                        ShowWarningItemNotFound(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6), "");
                        if (lNumericFieldBlanks(5)) {
                            ShowContinueError(state, format("a default temperature of {:.1R}C will be used.", -MixingTempLimit));
                        }
                    } else if (!thisMixing.deltaTempSched->checkMinVal(state, Clusive::In, -MixingTempLimit)) {
                        Sched::ShowSevereBadMin(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6), Clusive::In, -MixingTempLimit);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(5)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("{} and {} are provided, {} will be used.", cAlphaFieldNames(6), cNumericFieldNames(5), cAlphaFieldNames(6)));
                    }
                }

                // Min indoor temp
                if (NumAlpha > 6) {
                    if (lAlphaFieldBlanks(7)) {
                        // Is this an error or is there a default?
                    } else if ((thisMixing.minIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(7))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
                        ErrorsFound = true;
                    } else if (!thisMixing.minIndoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(7), cAlphaArgs(7), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Max indoor temp
                if (NumAlpha > 7) {
                    if (lAlphaFieldBlanks(8)) {
                    } else if ((thisMixing.maxIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(8))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8));
                        ErrorsFound = true;
                    } else if (!thisMixing.maxIndoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(8), cAlphaArgs(8), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Min source temp
                if (NumAlpha > 8) {
                    if (lAlphaFieldBlanks(9)) {
                    } else if ((thisMixing.minSourceTempSched = Sched::GetSchedule(state, cAlphaArgs(9))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(9), cAlphaArgs(9));
                        ErrorsFound = true;
                    } else if (!thisMixing.minSourceTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(9), cAlphaArgs(9), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Max source temp
                if (NumAlpha > 9) {
                    if (lAlphaFieldBlanks(10)) {
                    } else if ((thisMixing.maxSourceTempSched = Sched::GetSchedule(state, cAlphaArgs(10))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(10), cAlphaArgs(10));
                        ErrorsFound = true;
                    } else if (!thisMixing.maxSourceTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(10), cAlphaArgs(10), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                if (NumAlpha > 10) {
                    if (lAlphaFieldBlanks(11)) {
                    } else if ((thisMixing.minOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(11))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(11), cAlphaArgs(11));
                        ErrorsFound = true;
                    } else if (!thisMixing.minOutdoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(11), cAlphaArgs(11), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                //
                if (NumAlpha > 11) {
                    if (lAlphaFieldBlanks(12)) {
                    } else if ((thisMixing.maxOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(12))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(12), cAlphaArgs(12));
                        ErrorsFound = true;
                    } else if (!thisMixing.maxOutdoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(12), cAlphaArgs(12), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                if (thisMixing.ZonePtr > 0) {
                    if (RepVarSet(thisMixing.ZonePtr)) {
                        RepVarSet(thisMixing.ZonePtr) = false;
                        SetupOutputVariable(state,
                                            "Zone Mixing Volume",
                                            Constant::Units::m3,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixVolume,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Current Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixVdotCurDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Standard Density Volume Flow Rate",
                                            Constant::Units::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixVdotStdDensity,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Mass",
                                            Constant::Units::kg,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixMass,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixMdot,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Sensible Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixHeatLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Sensible Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixHeatGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Latent Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixLatentLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Latent Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixLatentGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Total Heat Loss Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixTotalLoss,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Mixing Total Heat Gain Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->ZnAirRpt(thisMixing.ZonePtr).MixTotalGain,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Sum,
                                            state.dataHeatBal->Zone(thisMixing.ZonePtr).Name);
                    }
                }
                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "ZoneMixing",
                                     thisMixing.Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     thisMixing.EMSSimpleMixingOn,
                                     thisMixing.EMSimpleMixingFlowRate);
                }
            }
        }
    }

    // allocate MassConservation
    state.dataHeatBal->MassConservation.allocate(state.dataGlobal->NumOfZones);

    // added by BAN, 02/14
    if (state.dataHeatBal->TotMixing > 0) {
        ZoneMixingNum.allocate(state.dataHeatBal->TotMixing);
        // get source zones mixing objects index
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            int SourceCount = 0;
            for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
                if (ZoneNum == state.dataHeatBal->Mixing(Loop).FromZone) {
                    SourceCount += 1;
                    ZoneMixingNum(SourceCount) = Loop;
                }
            }
            // save mixing objects index for zones which serve as a source zone
            state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject = SourceCount;
            if (SourceCount > 0) {
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingSourcesPtr.allocate(SourceCount);
                for (int Loop = 1; Loop <= SourceCount; ++Loop) {
                    state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingSourcesPtr(Loop) = ZoneMixingNum(Loop);
                }
            }
        }

        // check zones which are used only as a source zones
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
                if (ZoneNum != state.dataHeatBal->Mixing(Loop).FromZone) {
                    continue;
                }
                state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone = true;
                for (int Loop1 = 1; Loop1 <= state.dataHeatBal->TotMixing; ++Loop1) {
                    if (ZoneNum == state.dataHeatBal->Mixing(Loop1).ZonePtr) {
                        state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone = false;
                        break;
                    }
                }
            }
        }
        // get receiving zones mixing objects index
        ZoneMixingNum = 0;
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            int ReceivingCount = 0;
            for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
                if (ZoneNum == state.dataHeatBal->Mixing(Loop).ZonePtr) {
                    ReceivingCount += 1;
                    ZoneMixingNum(ReceivingCount) = Loop;
                }
            }
            // save mixing objects index for zones which serve as a receiving zone
            state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject = ReceivingCount;
            if (ReceivingCount > 0) {
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingPtr.allocate(ReceivingCount);
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingFr.allocate(ReceivingCount);
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingFr = 0.0;
                for (int Loop = 1; Loop <= ReceivingCount; ++Loop) {
                    state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingPtr(Loop) = ZoneMixingNum(Loop);
                }
            }
            // flag zones used as both source and receiving zone
            if (state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject > 0 &&
                state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject > 0) {
                state.dataHeatBal->MassConservation(ZoneNum).IsSourceAndReceivingZone = true;
            }
        }
        if (allocated(ZoneMixingNum)) {
            ZoneMixingNum.deallocate();
        }
    }

    // zone mass conservation calculation order starts with receiving zones
    // and then proceeds to source zones
    int Loop2 = 0;
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (!state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone &&
            !state.dataHeatBal->MassConservation(ZoneNum).IsSourceAndReceivingZone) {
            Loop2 += 1;
            state.dataHeatBalFanSys->ZoneReOrder(Loop2) = ZoneNum;
        }
    }
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataHeatBal->MassConservation(ZoneNum).IsSourceAndReceivingZone) {
            Loop2 += 1;
            state.dataHeatBalFanSys->ZoneReOrder(Loop2) = ZoneNum;
        }
    }
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone) {
            Loop2 += 1;
            state.dataHeatBalFanSys->ZoneReOrder(Loop2) = ZoneNum;
        }
    }

    cCurrentModuleObject = "ZoneCrossMixing";
    int numZoneCrossMixingInputObjects = 0;
    int totZoneCrossMixing = 0; // Total ZoneCrossMixing instances after expansion to spaces
    EPVector<InternalHeatGains::GlobalInternalGainMiscObject> zoneCrossMixingInputObjects;
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              zoneCrossMixingInputObjects,
                                              numZoneCrossMixingInputObjects,
                                              totZoneCrossMixing,
                                              ErrorsFound,
                                              zoneListNotAllowed);
    state.dataHeatBal->TotCrossMixing = totZoneCrossMixing + state.dataHeatBal->airBoundaryMixing.size();

    if (state.dataHeatBal->TotCrossMixing > 0) {
        cCurrentModuleObject = "ZoneCrossMixing";
        state.dataHeatBal->CrossMixing.allocate(state.dataHeatBal->TotCrossMixing);

        int mixingNum = 0;
        for (int mixingInputNum = 1; mixingInputNum <= numZoneCrossMixingInputObjects; ++mixingInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     mixingInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            // Create one Mixing instance for every space associated with this input object
            auto const &thisMixingInput = zoneCrossMixingInputObjects(mixingInputNum);
            for (int Item1 = 1; Item1 <= thisMixingInput.numOfSpaces; ++Item1) {
                ++mixingNum;
                auto &thisMixing = state.dataHeatBal->CrossMixing(mixingNum);
                thisMixing.Name = thisMixingInput.names(Item1);
                thisMixing.spaceIndex = thisMixingInput.spaceNums(Item1);
                auto const &thisSpace = state.dataHeatBal->space(thisMixing.spaceIndex);
                thisMixing.ZonePtr = thisSpace.zoneNum;
                auto &thisZone = state.dataHeatBal->Zone(thisSpace.zoneNum);

                if (lAlphaFieldBlanks(3)) {
                    thisMixing.sched = Sched::GetScheduleAlwaysOn(state); // not an availability schedule, but defaults to constant-1.0
                } else if ((thisMixing.sched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                // Mixing equipment design level calculation method.
                AirflowSpec flow = static_cast<AirflowSpec>(getEnumValue(airflowSpecNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
                switch (flow) {
                case AirflowSpec::FlowPerZone:
                    thisMixing.DesignLevel = rNumericArgs(1);
                    if (lNumericFieldBlanks(1)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Cross Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(1)));
                    } else {
                        Real64 spaceFrac = 1.0;
                        if (!thisMixingInput.spaceListActive && (thisMixingInput.numOfSpaces > 1)) {
                            Real64 const zoneVolume = thisZone.Volume;
                            if (zoneVolume > 0.0) {
                                spaceFrac = thisSpace.Volume / zoneVolume;
                            } else {
                                ShowSevereError(state, format("{}Zone volume is zero when allocating Cross Mixing to Spaces.", RoutineName));
                                ShowContinueError(
                                    state, format("Occurs for {}=\"{}\" in Zone=\"{}\".", cCurrentModuleObject, thisMixingInput.Name, thisZone.Name));
                                ErrorsFound = true;
                            }
                        }

                        thisMixing.DesignLevel = rNumericArgs(1) * spaceFrac;
                    }
                    break;

                case AirflowSpec::FlowPerArea:
                    if (thisMixing.spaceIndex != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            thisMixing.DesignLevel = rNumericArgs(2) * thisSpace.FloorArea;
                            if (thisMixing.spaceIndex > 0) {
                                if (thisZone.FloorArea <= 0.0) {
                                    ShowWarningError(state,
                                                     format("{}{}=\"{}\", {} specifies {}, but Space Floor Area = 0.  0 Cross Mixing will result.",
                                                            RoutineName,
                                                            cCurrentModuleObject,
                                                            thisMixingInput.Name,
                                                            cAlphaFieldNames(4),
                                                            cNumericFieldNames(2)));
                                }
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/area specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisMixingInput.Name,
                                                   rNumericArgs(2)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(2)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Cross Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(2)));
                    }
                    break;

                case AirflowSpec::FlowPerPerson:
                    if (thisMixing.spaceIndex != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisMixing.DesignLevel = rNumericArgs(3) * thisSpace.TotOccupants;
                            if (thisSpace.TotOccupants <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Total Occupants = 0.  0 Cross Mixing will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisMixingInput.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisMixingInput.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Cross Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpec::AirChanges:
                    if (thisMixing.spaceIndex != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            thisMixing.DesignLevel = rNumericArgs(4) * thisSpace.Volume / Constant::rSecsInHour;
                            if (thisSpace.Volume <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Space Volume = 0.  0 Cross Mixing will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisMixingInput.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(4)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid ACH (air changes per hour) specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisMixingInput.Name,
                                                   rNumericArgs(4)));
                            ErrorsFound = true;
                        }
                    }
                    if (lNumericFieldBlanks(4)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Cross Mixing will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisMixingInput.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(4)));
                    }
                    break;

                default:
                    ShowSevereError(
                        state, format("{}{}=\"{}\", invalid calculation method={}", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4)));
                    ErrorsFound = true;
                }

                thisMixing.fromSpaceIndex = Util::FindItemInList(cAlphaArgs(5), state.dataHeatBal->space);
                if (thisMixing.fromSpaceIndex == 0) {
                    thisMixing.FromZone = Util::FindItemInList(cAlphaArgs(5), state.dataHeatBal->Zone);
                } else {
                    thisMixing.FromZone = state.dataHeatBal->space(thisMixing.fromSpaceIndex).zoneNum;
                }
                if ((thisMixing.FromZone == 0) && (thisMixing.fromSpaceIndex == 0)) {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\", invalid (not found) {}=\"{}\".",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           cAlphaFieldNames(5),
                                           cAlphaArgs(5)));
                    ErrorsFound = true;
                }
                thisMixing.DeltaTemperature = rNumericArgs(5);

                if (NumAlpha > 5) {
                    if (lAlphaFieldBlanks(6)) {
                        if (lNumericFieldBlanks(5)) {
                            ShowWarningCustom(state,
                                              eoh,
                                              format("{} and {} are empty, a default temperature of {:.1R}C will be used.",
                                                     cAlphaFieldNames(6),
                                                     cNumericFieldNames(5),
                                                     thisMixing.DeltaTemperature));
                        }
                    } else if ((thisMixing.deltaTempSched = Sched::GetSchedule(state, cAlphaArgs(6))) == nullptr) {
                        ShowWarningItemNotFound(state,
                                                eoh,
                                                cAlphaFieldNames(6),
                                                cAlphaArgs(6),
                                                format("Fixed delta temperature {:.1R}C will be used", thisMixing.DeltaTemperature));
                    } else if (!thisMixing.deltaTempSched->checkMinVal(state, Clusive::In, 0.0)) {
                        Sched::ShowSevereBadMin(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6), Clusive::In, 0.0);
                        ErrorsFound = true;
                    } else if (!lNumericFieldBlanks(5)) {
                        ShowWarningCustom(
                            state,
                            eoh,
                            format("{} and {} provided. {} will be used.", cAlphaFieldNames(6), cNumericFieldNames(5), cAlphaFieldNames(6)));
                    }
                }

                // Min indoor temp
                if (NumAlpha > 6) {
                    if (lAlphaFieldBlanks(7)) {
                    } else if ((thisMixing.minIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(7))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
                        ErrorsFound = true;
                    } else if (!thisMixing.minIndoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(7), cAlphaArgs(7), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Max indoor temp
                if (NumAlpha > 7) {
                    if (lAlphaFieldBlanks(8)) {
                    } else if ((thisMixing.maxIndoorTempSched = Sched::GetSchedule(state, cAlphaArgs(8))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8));
                        ErrorsFound = true;
                    } else if (!thisMixing.maxIndoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(8), cAlphaArgs(8), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Min source temp
                if (NumAlpha > 8) {
                    if (lAlphaFieldBlanks(9)) {
                    } else if ((thisMixing.minSourceTempSched = Sched::GetSchedule(state, cAlphaArgs(9))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(9), cAlphaArgs(9));
                        ErrorsFound = true;
                    } else if (!thisMixing.minSourceTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(9), cAlphaArgs(9), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Max source temp
                if (NumAlpha > 9) {
                    if (lAlphaFieldBlanks(10)) {
                    } else if ((thisMixing.maxSourceTempSched = Sched::GetSchedule(state, cAlphaArgs(10))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(10), cAlphaArgs(10));
                        ErrorsFound = true;
                    } else if (!thisMixing.maxSourceTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(10), cAlphaArgs(10), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                // Min outdoor temp
                if (NumAlpha > 10) {
                    if (lAlphaFieldBlanks(11)) {
                    } else if ((thisMixing.minOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(11))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(11), cAlphaArgs(11));
                        ErrorsFound = true;
                    } else if (!thisMixing.minOutdoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(11), cAlphaArgs(11), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }

                if (NumAlpha > 11) {
                    if (lAlphaFieldBlanks(12)) {
                    } else if ((thisMixing.maxOutdoorTempSched = Sched::GetSchedule(state, cAlphaArgs(12))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(12), cAlphaArgs(12));
                        ErrorsFound = true;
                    } else if (!thisMixing.maxOutdoorTempSched->checkMinMaxVals(state, Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, cAlphaFieldNames(12), cAlphaArgs(12), Clusive::In, -MixingTempLimit, Clusive::In, MixingTempLimit);
                        ErrorsFound = true;
                    }
                }
            }
        } // for (mixingInputNum)

        // Create CrossMixing objects from air boundary info
        for (auto const &thisAirBoundaryMixing : state.dataHeatBal->airBoundaryMixing) {
            ++mixingNum;
            // Create CrossMixing object from air boundary info
            int space1 = thisAirBoundaryMixing.space1;
            int space2 = thisAirBoundaryMixing.space2;
            int zone1 = state.dataHeatBal->space(space1).zoneNum;
            int zone2 = state.dataHeatBal->space(space2).zoneNum;
            auto &thisCrossMizing = state.dataHeatBal->CrossMixing(mixingNum);
            thisCrossMizing.Name = fmt::format("Air Boundary Mixing Zones {} and {}", zone1, zone2);
            thisCrossMizing.spaceIndex = space1;
            thisCrossMizing.ZonePtr = zone1;
            thisCrossMizing.sched = thisAirBoundaryMixing.sched;
            thisCrossMizing.DesignLevel = thisAirBoundaryMixing.mixingVolumeFlowRate;
            thisCrossMizing.FromZone = zone2;
            thisCrossMizing.fromSpaceIndex = space2;
        }
        assert(mixingNum == state.dataHeatBal->TotCrossMixing);
        for (int mixingRepNum = 1; mixingRepNum <= state.dataHeatBal->TotCrossMixing; ++mixingRepNum) {
            int zoneNum = state.dataHeatBal->CrossMixing(mixingRepNum).ZonePtr;
            if (zoneNum > 0) {
                std::string const &zoneName = state.dataHeatBal->Zone(zoneNum).Name;
                auto &thisZnAirRpt = state.dataHeatBal->ZnAirRpt(zoneNum);
                if (RepVarSet(zoneNum)) {
                    RepVarSet(zoneNum) = false;
                    SetupOutputVariable(state,
                                        "Zone Mixing Volume",
                                        Constant::Units::m3,
                                        thisZnAirRpt.MixVolume,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Current Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        thisZnAirRpt.MixVdotCurDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Standard Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        thisZnAirRpt.MixVdotStdDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass",
                                        Constant::Units::kg,
                                        thisZnAirRpt.MixMass,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        thisZnAirRpt.MixMdot,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Loss Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixHeatLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Gain Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixHeatGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Loss Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixLatentLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Gain Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixLatentGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Loss Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixTotalLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Gain Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixTotalGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        zoneName);
                }
            }
            int fromZoneNum = state.dataHeatBal->CrossMixing(mixingRepNum).FromZone;
            if (fromZoneNum > 0) {
                if (RepVarSet(fromZoneNum)) {
                    RepVarSet(fromZoneNum) = false;
                    std::string const &fromZoneName = state.dataHeatBal->Zone(fromZoneNum).Name;
                    auto &thisZnAirRpt = state.dataHeatBal->ZnAirRpt(fromZoneNum);
                    SetupOutputVariable(state,
                                        "Zone Mixing Volume",
                                        Constant::Units::m3,
                                        thisZnAirRpt.MixVolume,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Current Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        thisZnAirRpt.MixVdotCurDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Standard Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        thisZnAirRpt.MixVdotStdDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass",
                                        Constant::Units::kg,
                                        thisZnAirRpt.MixMass,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        thisZnAirRpt.MixMdot,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Loss Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixHeatLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Gain Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixHeatGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Loss Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixLatentLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Gain Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixLatentGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Loss Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixTotalLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Gain Energy",
                                        Constant::Units::J,
                                        thisZnAirRpt.MixTotalGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        fromZoneName);
                }
            }

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "ZoneCrossMixing",
                                 state.dataHeatBal->CrossMixing(mixingRepNum).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 state.dataHeatBal->CrossMixing(mixingRepNum).EMSSimpleMixingOn,
                                 state.dataHeatBal->CrossMixing(mixingRepNum).EMSimpleMixingFlowRate);
            }
        }
    }

    cCurrentModuleObject = "ZoneRefrigerationDoorMixing";
    state.dataHeatBal->TotRefDoorMixing = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    if (state.dataHeatBal->TotRefDoorMixing > 0) {
        state.dataHeatBal->RefDoorMixing.allocate(state.dataGlobal->NumOfZones);
        for (auto &e : state.dataHeatBal->RefDoorMixing) {
            e.NumRefDoorConnections = 0;
        }

        for (int Loop = 1; Loop <= state.dataHeatBal->TotRefDoorMixing; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
            NameThisObject = cAlphaArgs(1);

            int AlphaNum = 2;
            int Zone1Num = Util::FindItemInList(cAlphaArgs(AlphaNum), state.dataHeatBal->Zone);
            int space1Num = Util::FindItemInList(cAlphaArgs(AlphaNum), state.dataHeatBal->space);
            if ((Zone1Num == 0) && (space1Num == 0)) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", invalid (not found) {}=\"{}\".",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       cAlphaArgs(1),
                                       cAlphaFieldNames(AlphaNum),
                                       cAlphaArgs(AlphaNum)));
                ErrorsFound = true;
            } else if (Zone1Num == 0) {
                Zone1Num = state.dataHeatBal->space(space1Num).zoneNum;
            }

            ++AlphaNum; // 3
            int Zone2Num = Util::FindItemInList(cAlphaArgs(AlphaNum), state.dataHeatBal->Zone);
            int space2Num = Util::FindItemInList(cAlphaArgs(AlphaNum), state.dataHeatBal->space);
            if ((Zone2Num == 0) && (space2Num == 0)) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", invalid (not found) {}=\"{}\".",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       cAlphaArgs(1),
                                       cAlphaFieldNames(AlphaNum),
                                       cAlphaArgs(AlphaNum)));
                ErrorsFound = true;
            } else if (Zone2Num == 0) {
                Zone2Num = state.dataHeatBal->space(space2Num).zoneNum;
            }

            int spaceNumA = 0;
            int spaceNumB = 0;
            if (Zone1Num == Zone2Num) {
                ShowSevereError(state,
                                format("{}{}=\"{}\", The same zone name has been entered for both sides of a refrigerated door {}=\"{}\".",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       cAlphaArgs(1),
                                       cAlphaFieldNames(AlphaNum),
                                       cAlphaArgs(AlphaNum)));
                ErrorsFound = true;
            } else if (Zone1Num < Zone2Num) { // zone 1 will come first in soln loop, id zone 2 as mate zone
                ZoneNumA = Zone1Num;
                ZoneNumB = Zone2Num;
                spaceNumA = space1Num;
                spaceNumB = space2Num;
            } else { // zone 2 will come first in soln loop, id zone 1 as mate zone
                ZoneNumA = Zone2Num;
                ZoneNumB = Zone1Num;
                spaceNumA = space2Num;
                spaceNumB = space1Num;
            }

            auto &zoneA = state.dataHeatBal->RefDoorMixing(ZoneNumA);
            auto &zoneB = state.dataHeatBal->RefDoorMixing(ZoneNumB);

            if (!allocated(zoneA.openScheds)) {
                zoneA.DoorMixingObjectName.allocate(state.dataGlobal->NumOfZones);
                zoneA.openScheds.allocate(state.dataGlobal->NumOfZones);
                zoneA.DoorHeight.allocate(state.dataGlobal->NumOfZones);
                zoneA.DoorArea.allocate(state.dataGlobal->NumOfZones);
                zoneA.Protection.allocate(state.dataGlobal->NumOfZones);
                zoneA.MateZonePtr.allocate(state.dataGlobal->NumOfZones);
                zoneA.EMSRefDoorMixingOn.allocate(state.dataGlobal->NumOfZones);
                zoneA.EMSRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                zoneA.VolRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                zoneA.DoorProtTypeName.allocate(state.dataGlobal->NumOfZones);
                zoneA.DoorMixingObjectName = "";
                zoneA.openScheds = nullptr;
                zoneA.DoorHeight = 0.0;
                zoneA.DoorArea = 0.0;
                zoneA.Protection = RefDoorNone;
                zoneA.MateZonePtr = 0;
                zoneA.EMSRefDoorMixingOn = false;
                zoneA.EMSRefDoorFlowRate = 0.0;
                zoneA.VolRefDoorFlowRate = 0.0;
                zoneA.DoorProtTypeName = "";
            } // First refrigeration mixing in this zone

            if (!allocated(zoneB.openScheds)) {
                zoneB.DoorMixingObjectName.allocate(state.dataGlobal->NumOfZones);
                zoneB.openScheds.allocate(state.dataGlobal->NumOfZones);
                zoneB.DoorHeight.allocate(state.dataGlobal->NumOfZones);
                zoneB.DoorArea.allocate(state.dataGlobal->NumOfZones);
                zoneB.Protection.allocate(state.dataGlobal->NumOfZones);
                zoneB.MateZonePtr.allocate(state.dataGlobal->NumOfZones);
                zoneB.EMSRefDoorMixingOn.allocate(state.dataGlobal->NumOfZones);
                zoneB.EMSRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                zoneB.VolRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                zoneB.DoorProtTypeName.allocate(state.dataGlobal->NumOfZones);
                zoneB.DoorMixingObjectName = "";
                zoneB.openScheds = nullptr;
                zoneB.DoorHeight = 0.0;
                zoneB.DoorArea = 0.0;
                zoneB.Protection = RefDoorNone;
                zoneB.MateZonePtr = 0;
                zoneB.EMSRefDoorMixingOn = false;
                zoneB.EMSRefDoorFlowRate = 0.0;
                zoneB.VolRefDoorFlowRate = 0.0;
                zoneB.DoorProtTypeName = "";
            } // First refrigeration mixing in this zone

            ConnectionNumber = zoneA.NumRefDoorConnections + 1;
            zoneA.NumRefDoorConnections = ConnectionNumber;
            zoneA.ZonePtr = ZoneNumA;
            zoneA.spaceIndex = spaceNumA;
            zoneA.fromSpaceIndex = spaceNumB;
            zoneA.MateZonePtr(ConnectionNumber) = ZoneNumB;
            zoneA.DoorMixingObjectName(ConnectionNumber) = NameThisObject;
            // need to make sure same pair of zones is only entered once.
            if (zoneA.RefDoorMixFlag && zoneB.RefDoorMixFlag) {
                if (zoneA.NumRefDoorConnections > 1) {
                    for (int ConnectTest = 1; ConnectTest <= (ConnectionNumber - 1); ++ConnectTest) {
                        if (zoneA.MateZonePtr(ConnectTest) != zoneA.MateZonePtr(ConnectionNumber)) {
                            continue;
                        }
                        ShowSevereError(
                            state,
                            format("{}{}=\"{}\", and {}", RoutineName, cCurrentModuleObject, cAlphaArgs(1), zoneA.DoorMixingObjectName(ConnectTest)));
                        ShowContinueError(state,
                                          format(" Share same pair of zones: \"{}\" and \"{}\". Only one RefrigerationDoorMixing object is allowed "
                                                 "for any unique pair of zones.",
                                                 state.dataHeatBal->Zone(ZoneNumA).Name,
                                                 state.dataHeatBal->Zone(ZoneNumB).Name));
                        ErrorsFound = true;
                    } // ConnectTest
                } // NumRefDoorconnections > 1
            } else { // Both zones need to be flagged with ref doors
                zoneA.RefDoorMixFlag = true;
                zoneB.RefDoorMixFlag = true;
            } // Both zones already flagged with ref doors

            ++AlphaNum; // 4
            if (lAlphaFieldBlanks(AlphaNum)) {
                ShowSevereEmptyField(state, eoh, cAlphaFieldNames(AlphaNum));
                ErrorsFound = true;
            } else if ((zoneA.openScheds(ConnectionNumber) = Sched::GetSchedule(state, cAlphaArgs(AlphaNum))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(AlphaNum), cAlphaArgs(AlphaNum));
                ErrorsFound = true;
            } else if (!zoneA.openScheds(ConnectionNumber)->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, cAlphaFieldNames(AlphaNum), cAlphaArgs(AlphaNum), Clusive::In, 0.0, Clusive::In, 1.0);
                ErrorsFound = true;
            } //(lAlphaFieldBlanks(AlphaNum)) THEN

            int NumbNum = 1;
            if (lAlphaFieldBlanks(NumbNum)) {
                zoneA.DoorHeight(ConnectionNumber) = 3.0; // default height of 3 meters
                ShowWarningError(state,
                                 format("{}{}=\"{}{} is blank and the default value of 3.0 will be used.",
                                        RoutineName,
                                        cCurrentModuleObject,
                                        cAlphaArgs(1),
                                        cNumericFieldNames(NumbNum)));
            } else {
                zoneA.DoorHeight(ConnectionNumber) = rNumericArgs(NumbNum);
                if ((zoneA.DoorHeight(ConnectionNumber) < 0) || (zoneA.DoorHeight(ConnectionNumber) > 50.0)) {
                    ShowSevereError(
                        state,
                        format("{}{} = {} must have a door height between 0 and 50 meters. ", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ErrorsFound = true;
                }
            }

            ++NumbNum; // 2
            if (lAlphaFieldBlanks(NumbNum)) {
                zoneA.DoorArea(ConnectionNumber) = 9.0; // default area of 9 m2
                ShowWarningError(state,
                                 format("{}{}=\"{}{} is blank and the default value of 9 m2 will be used.",
                                        RoutineName,
                                        cCurrentModuleObject,
                                        cAlphaArgs(1),
                                        cNumericFieldNames(NumbNum)));
            } else {
                zoneA.DoorArea(ConnectionNumber) = rNumericArgs(NumbNum);
                if ((zoneA.DoorArea(ConnectionNumber) < 0) || (zoneA.DoorArea(ConnectionNumber) > 400.0)) {
                    ShowSevereError(
                        state,
                        format(
                            "{}{} = {} must have a door height between 0 and 400 square meters. ", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ErrorsFound = true;
                }
            }

            ++AlphaNum; // 5
            // Door protection type.
            if (lAlphaFieldBlanks(AlphaNum)) {
                zoneA.Protection(ConnectionNumber) = RefDoorNone;  // Default
                zoneA.DoorProtTypeName(ConnectionNumber) = "None"; // Default
                ShowWarningError(state,
                                 format("{}{}=\"{}\"  {} is blank. Default of no door protection will be used",
                                        RoutineName,
                                        cCurrentModuleObject,
                                        cAlphaArgs(1),
                                        cAlphaFieldNames(AlphaNum)));
            } else {
                if (cAlphaArgs(AlphaNum) == "NONE") {
                    zoneA.Protection(ConnectionNumber) = RefDoorNone;
                    zoneA.DoorProtTypeName(ConnectionNumber) = "None";
                } else if (cAlphaArgs(AlphaNum) == "AIRCURTAIN") {
                    zoneA.Protection(ConnectionNumber) = RefDoorAirCurtain;
                    zoneA.DoorProtTypeName(ConnectionNumber) = "AirCurtain";
                } else if (cAlphaArgs(AlphaNum) == "STRIPCURTAIN") {
                    zoneA.Protection(ConnectionNumber) = RefDoorStripCurtain;
                    zoneA.DoorProtTypeName(ConnectionNumber) = "StripCurtain";
                } else {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\", invalid calculation method={} with alphanum of 5: {}",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           cAlphaArgs(AlphaNum),
                                           cAlphaArgs(5)));
                    ErrorsFound = true;
                } // =none, etc.
            } // Blank

            if (ZoneNumA > 0) {
                if (RepVarSet(ZoneNumA)) {
                    RepVarSet(ZoneNumA) = false;
                    SetupOutputVariable(state,
                                        "Zone Mixing Volume",
                                        Constant::Units::m3,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixVolume,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Current Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixVdotCurDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Standard Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixVdotStdDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass",
                                        Constant::Units::kg,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixMass,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixMdot,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Loss Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixHeatLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Gain Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixHeatGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Loss Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixLatentLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Gain Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixLatentGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Loss Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixTotalLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Gain Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixTotalGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                }
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "ZoneRefDoorMixing",
                                 zoneA.Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 zoneA.EMSRefDoorMixingOn(ConnectionNumber),
                                 zoneA.EMSRefDoorFlowRate(ConnectionNumber));
            }

            if (ZoneNumB > 0) {
                if (RepVarSet(ZoneNumB)) {
                    RepVarSet(ZoneNumB) = false;
                    SetupOutputVariable(state,
                                        "Zone Mixing Volume",
                                        Constant::Units::m3,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixVolume,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Current Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixVdotCurDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Standard Density Volume Flow Rate",
                                        Constant::Units::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixVdotStdDensity,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass",
                                        Constant::Units::kg,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixMass,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass Flow Rate",
                                        Constant::Units::kg_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixMdot,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Loss Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixHeatLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Gain Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixHeatGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Loss Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixLatentLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Gain Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixLatentGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Loss Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixTotalLoss,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Gain Energy",
                                        Constant::Units::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixTotalGain,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Sum,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                }
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "ZoneRefDoorMixing",
                                 zoneB.Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 zoneA.EMSRefDoorMixingOn(ConnectionNumber),
                                 state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate(ConnectionNumber));
            }
        } // DO Loop=1,TotRefDoorMixing
    } // TotRefDoorMixing > 0)

    RepVarSet.deallocate();
    cAlphaArgs.deallocate();
    cAlphaFieldNames.deallocate();
    cNumericFieldNames.deallocate();
    rNumericArgs.deallocate();
    lAlphaFieldBlanks.deallocate();
    lNumericFieldBlanks.deallocate();

    TotInfilVentFlow.dimension(state.dataGlobal->NumOfZones, 0.0);

    auto divide_and_print_if_greater_than_zero = [&](const Real64 denominator, const Real64 numerator) {
        if (denominator > 0.0) {
            print(state.files.eio, "{:.3R},", numerator / denominator);
        } else {
            print(state.files.eio, "N/A,");
        }
    };

    for (int Loop = 1; Loop <= state.dataHeatBal->TotInfiltration; ++Loop) {
        if (Loop == 1) {
            print(state.files.eio,
                  Format_721,
                  "ZoneInfiltration",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/Exterior Surface Area {m3/s-m2},ACH - "
                  "Air Changes per Hour,Equation A - Constant Term Coefficient {},Equation B - Temperature Term Coefficient {1/C},Equation C - "
                  "Velocity Term Coefficient {s/m}, Equation D - Velocity Squared Term Coefficient {s2/m2}");
        }

        int ZoneNum = state.dataHeatBal->Infiltration(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "Infiltration-Illegal Zone specified", state.dataHeatBal->Infiltration(Loop).Name);
            continue;
        }
        TotInfilVentFlow(ZoneNum) += state.dataHeatBal->Infiltration(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "ZoneInfiltration",
              state.dataHeatBal->Infiltration(Loop).Name,
              state.dataHeatBal->Infiltration(Loop).sched->Name,
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).DesignLevel);

        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->Infiltration(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).ExteriorTotalSurfArea,
                                              state.dataHeatBal->Infiltration(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->Infiltration(Loop).DesignLevel * Constant::rSecsInHour);

        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).ConstantTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).TemperatureTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).VelocityTermCoef);
        print(state.files.eio, "{:.3R}\n", state.dataHeatBal->Infiltration(Loop).VelocitySQTermCoef);
    }

    if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
        for (int Loop = 1; Loop <= state.dataHeatBal->TotInfiltration; ++Loop) {
            int ZoneNum = state.dataHeatBal->Infiltration(Loop).ZonePtr;
            state.dataHeatBal->MassConservation(ZoneNum).InfiltrationPtr = Loop;
        }
    }

    for (int Loop = 1; Loop <= state.dataHeatBal->TotVentilation; ++Loop) {
        if (Loop == 1) {
            print(state.files.eio,
                  Format_721,
                  "ZoneVentilation",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                  "Changes per Hour,Fan Type {Exhaust;Intake;Natural},Fan Pressure Rise {Pa},Fan Efficiency {},Equation A - Constant Term "
                  "Coefficient {},Equation B - Temperature Term Coefficient {1/C},Equation C - Velocity Term Coefficient {s/m}, Equation D - "
                  "Velocity Squared Term Coefficient {s2/m2},Minimum Indoor Temperature{C}/Schedule,Maximum Indoor "
                  "Temperature{C}/Schedule,Delta Temperature{C}/Schedule,Minimum Outdoor Temperature{C}/Schedule,Maximum Outdoor "
                  "Temperature{C}/Schedule,Maximum WindSpeed{m/s}");
        }

        int ZoneNum = state.dataHeatBal->Ventilation(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "Ventilation-Illegal Zone specified", state.dataHeatBal->Ventilation(Loop).Name);
            continue;
        }
        TotInfilVentFlow(ZoneNum) += state.dataHeatBal->Ventilation(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "ZoneVentilation",
              state.dataHeatBal->Ventilation(Loop).Name,
              state.dataHeatBal->Ventilation(Loop).availSched->Name,
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);

        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).DesignLevel);

        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->Ventilation(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).TotOccupants, state.dataHeatBal->Ventilation(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->Ventilation(Loop).DesignLevel * Constant::rSecsInHour);

        if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Exhaust) {
            print(state.files.eio, "Exhaust,");
        } else if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Intake) {
            print(state.files.eio, "Intake,");
        } else if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Natural) {
            print(state.files.eio, "Natural,");
        } else if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Balanced) {
            print(state.files.eio, "Balanced,");
        } else {
            print(state.files.eio, "UNKNOWN,");
        }
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).FanPressure);
        print(state.files.eio, "{:.1R},", state.dataHeatBal->Ventilation(Loop).FanEfficiency);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).ConstantTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).TemperatureTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).VelocityTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).VelocitySQTermCoef);

        // TODO Should this also be prefixed with "Schedule: " like the following ones are?
        if (state.dataHeatBal->Ventilation(Loop).minIndoorTempSched != nullptr) {
            print(state.files.eio, "{},", state.dataHeatBal->Ventilation(Loop).minIndoorTempSched->Name);
        } else {
            print(state.files.eio, "{:.2R},", state.dataHeatBal->Ventilation(Loop).MinIndoorTemperature);
        }

        const auto print_temperature = [&](Sched::Schedule const *ptr, const Real64 value) {
            if (ptr != nullptr) {
                print(state.files.eio, "Schedule: {},", ptr->Name);
            } else {
                print(state.files.eio, "{:.2R},", value);
            }
        };

        print_temperature(state.dataHeatBal->Ventilation(Loop).maxIndoorTempSched, state.dataHeatBal->Ventilation(Loop).MaxIndoorTemperature);
        print_temperature(state.dataHeatBal->Ventilation(Loop).deltaTempSched, state.dataHeatBal->Ventilation(Loop).DelTemperature);
        print_temperature(state.dataHeatBal->Ventilation(Loop).minOutdoorTempSched, state.dataHeatBal->Ventilation(Loop).MinOutdoorTemperature);
        print_temperature(state.dataHeatBal->Ventilation(Loop).maxOutdoorTempSched, state.dataHeatBal->Ventilation(Loop).MaxOutdoorTemperature);

        print(state.files.eio, "{:.2R}\n", state.dataHeatBal->Ventilation(Loop).MaxWindSpeed);
    }

    TotMixingFlow.dimension(state.dataGlobal->NumOfZones, 0.0);
    for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
        if (Loop == 1) {
            print(state.files.eio,
                  Format_721,
                  "Mixing",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                  "Changes per Hour,From/Source Zone,Delta Temperature {C}");
        }

        int ZoneNum = state.dataHeatBal->Mixing(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "Mixing-Illegal Zone specified", state.dataHeatBal->Mixing(Loop).Name);
            continue;
        }
        TotMixingFlow(ZoneNum) += state.dataHeatBal->Mixing(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "Mixing",
              state.dataHeatBal->Mixing(Loop).Name,
              state.dataHeatBal->Mixing(Loop).sched->Name,
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Mixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->Mixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).TotOccupants, state.dataHeatBal->Mixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->Mixing(Loop).DesignLevel * Constant::rSecsInHour);

        print(state.files.eio, "{},", state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).FromZone).Name);
        print(state.files.eio, "{:.2R}\n", state.dataHeatBal->Mixing(Loop).DeltaTemperature);
    }

    for (int Loop = 1; Loop <= state.dataHeatBal->TotCrossMixing; ++Loop) {
        if (Loop == 1) {
            print(state.files.eio,
                  Format_721,
                  "CrossMixing",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                  "Changes per Hour,From/Source Zone,Delta Temperature {C}");
        }

        int ZoneNum = state.dataHeatBal->CrossMixing(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "CrossMixing-Illegal Zone specified", state.dataHeatBal->CrossMixing(Loop).Name);
            continue;
        }
        TotMixingFlow(ZoneNum) += state.dataHeatBal->CrossMixing(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "CrossMixing",
              state.dataHeatBal->CrossMixing(Loop).Name,
              state.dataHeatBal->CrossMixing(Loop).sched->Name,
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);

        print(state.files.eio, "{:.3R},", state.dataHeatBal->CrossMixing(Loop).DesignLevel);

        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->CrossMixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).TotOccupants, state.dataHeatBal->CrossMixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->CrossMixing(Loop).DesignLevel * Constant::rSecsInHour);

        print(state.files.eio, "{},", state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
        print(state.files.eio, "{:.2R}\n", state.dataHeatBal->CrossMixing(Loop).DeltaTemperature);
    }

    if (state.dataHeatBal->TotRefDoorMixing > 0) {
        static constexpr std::string_view Format_724("! <{} Airflow Stats Nominal>, {}\n");
        print(state.files.eio,
              Format_724,
              "RefrigerationDoorMixing",
              "Name, Zone 1 Name,Zone 2 Name,Door Opening Schedule Name,Door Height {m},Door Area {m2},Door Protection Type");
        for (ZoneNumA = 1; ZoneNumA <= (state.dataGlobal->NumOfZones - 1); ++ZoneNumA) {
            if (!state.dataHeatBal->RefDoorMixing(ZoneNumA).RefDoorMixFlag) {
                continue;
            }
            for (ConnectionNumber = 1; ConnectionNumber <= state.dataHeatBal->RefDoorMixing(ZoneNumA).NumRefDoorConnections; ++ConnectionNumber) {
                ZoneNumB = state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber);
                // TotMixingFlow(ZoneNum)=TotMixingFlow(ZoneNum)+RefDoorMixing(Loop)%!DesignLevel
                static constexpr std::string_view Format_723(" {} Airflow Stats Nominal, {},{},{},{},{:.3R},{:.3R},{}\n");
                print(state.files.eio,
                      Format_723,
                      "RefrigerationDoorMixing",
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectionNumber),
                      state.dataHeatBal->Zone(ZoneNumA).Name,
                      state.dataHeatBal->Zone(ZoneNumB).Name,
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).openScheds(ConnectionNumber)->Name,
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber),
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber),
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber));
            } // ConnectionNumber
        } // ZoneNumA
    } //(TotRefDoorMixing .GT. 0)

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        state.dataHeatBal->Zone(ZoneNum).NominalInfilVent = TotInfilVentFlow(ZoneNum);
        state.dataHeatBal->Zone(ZoneNum).NominalMixing = TotMixingFlow(ZoneNum);
    }

    if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
        // Check for infiltration in zone which are only a mixing source zone
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if ((state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing &&
                 state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone) &&
                (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment != DataHeatBalance::InfiltrationFlow::No)) {
                if (state.dataHeatBal->MassConservation(ZoneNum).InfiltrationPtr == 0) {
                    ShowSevereError(
                        state, format("{}: Infiltration object is not defined for zone = {}", RoutineName, state.dataHeatBal->Zone(ZoneNum).Name));
                    ShowContinueError(state, "Zone air mass flow balance requires infiltration object for source zones of mixing objects");
                }
            }
        }
        // Set up zone air mass balance output variables
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            SetupOutputVariable(state,
                                "Zone Air Mass Balance Supply Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->MassConservation(ZoneNum).InMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Mass Balance Exhaust Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->MassConservation(ZoneNum).ExhMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Mass Balance Return Mass Flow Rate",
                                Constant::Units::kg_s,
                                state.dataHeatBal->MassConservation(ZoneNum).RetMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            if ((state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) &&
                ((state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject +
                  state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject) > 0)) {
                SetupOutputVariable(state,
                                    "Zone Air Mass Balance Mixing Receiving Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataHeatBal->MassConservation(ZoneNum).MixingMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(ZoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Air Mass Balance Mixing Source Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataHeatBal->MassConservation(ZoneNum).MixingSourceMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(ZoneNum).Name);
            }
            if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment != DataHeatBalance::InfiltrationFlow::No) {
                if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationForZones == DataHeatBalance::InfiltrationZoneType::AllZones ||
                    (state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject > 0)) {
                    if (state.dataHeatBal->MassConservation(ZoneNum).InfiltrationPtr > 0) {
                        SetupOutputVariable(state,
                                            "Zone Air Mass Balance Infiltration Mass Flow Rate",
                                            Constant::Units::kg_s,
                                            state.dataHeatBal->MassConservation(ZoneNum).InfiltrationMassFlowRate,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(ZoneNum).Name);
                        SetupOutputVariable(state,
                                            "Zone Air Mass Balance Infiltration Status",
                                            Constant::Units::None,
                                            state.dataHeatBal->MassConservation(ZoneNum).IncludeInfilToZoneMassBal,
                                            OutputProcessor::TimeStepType::System,
                                            OutputProcessor::StoreType::Average,
                                            state.dataHeatBal->Zone(ZoneNum).Name);
                    }
                }
            }
        }
    }

    TotInfilVentFlow.deallocate();
    TotMixingFlow.deallocate();
    //           ' Area per Occupant {m2/person}, Occupant per Area {person/m2}, Interior Lighting {W/m2}, ',  &
    //           'Electric Load {W/m2}, Gas Load {W/m2}, Other Load {W/m2}, Hot Water Eq {W/m2}, Outdoor Controlled Baseboard Heat')
}

void GetRoomAirModelParameters(EnergyPlusData &state, bool &errFlag) // True if errors found during this input routine
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2001
    //       RE-ENGINEERED  April 2003, Weixiu Kong
    //                      December 2003, CC

    // PURPOSE OF THIS SUBROUTINE:
    //     Get room air model parameters for all zones at once

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas; // States which alpha value to read from a
    // "Number" line
    int NumNumbers; // Number of numbers encountered
    int Status;     // Notes if there was an error in processing the input
    int AirModelNum;
    int NumOfAirModels;
    int ZoneNum;
    bool IsNotOK;

    // Initialize default values for air model parameters
    state.dataRoomAir->AirModel.allocate(state.dataGlobal->NumOfZones);

    bool ErrorsFound = false;
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

    cCurrentModuleObject = "RoomAirModelType";
    NumOfAirModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    if (NumOfAirModels > state.dataGlobal->NumOfZones) {
        ShowSevereError(state, format("Too many {}.  Cannot exceed the number of Zones.", cCurrentModuleObject));
        ErrorsFound = true;
    }
    if (NumOfAirModels > 0) {
        state.dataRoomAir->IsZoneDispVent3Node.dimension(state.dataGlobal->NumOfZones, false);
        state.dataRoomAir->IsZoneCrossVent.dimension(state.dataGlobal->NumOfZones, false);
        state.dataRoomAir->IsZoneUFAD.dimension(state.dataGlobal->NumOfZones, false);
    }

    for (AirModelNum = 1; AirModelNum <= NumOfAirModels; ++AirModelNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 AirModelNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 Status,
                                                                 _,
                                                                 _,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);
        ZoneNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->Zone);
        if (ZoneNum != 0) {
            if (!state.dataRoomAir->AirModel(ZoneNum).Name.empty()) {
                ShowSevereError(state, format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2)));
                ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Duplicate zone name, only one type of roomair model is allowed per zone");
                ShowContinueError(state,
                                  format("Zone {} was already assigned a roomair model by {} = {}",
                                         state.dataIPShortCut->cAlphaArgs(2),
                                         cCurrentModuleObject,
                                         roomAirModelNamesUC[(int)state.dataRoomAir->AirModel(ZoneNum).AirModel]));
                ShowContinueError(
                    state,
                    format("Air Model Type for zone already set to {}", roomAirModelNamesUC[(int)state.dataRoomAir->AirModel(ZoneNum).AirModel]));
                ShowContinueError(state, format("Trying to overwrite with model type = {}", state.dataIPShortCut->cAlphaArgs(3)));
                ErrorsFound = true;
            }
            state.dataRoomAir->AirModel(ZoneNum).ZoneName = state.dataIPShortCut->cAlphaArgs(2);

            // state.dataRoomAir->AirModel(ZoneNum).AirModelName = state.dataIPShortCut->cAlphaArgs(1);
            state.dataRoomAir->AirModel(ZoneNum).AirModel =
                static_cast<RoomAir::RoomAirModel>(getEnumValue(roomAirModelNamesUC, state.dataIPShortCut->cAlphaArgs(3))); // is this arg1 or arg3?
            switch (state.dataRoomAir->AirModel(ZoneNum).AirModel) {
            case RoomAir::RoomAirModel::Mixing:
                // nothing to do here actually
                break;

            case RoomAir::RoomAirModel::DispVent1Node:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->DispVent1NodeModelUsed = true;
                IsNotOK = false;
                ValidateComponent(state,
                                  "RoomAirSettings:OneNodeDisplacementVentilation",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, format("In {}={}.", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
                break;

            case RoomAir::RoomAirModel::DispVent3Node:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->UCSDModelUsed = true;
                IsNotOK = false;
                ValidateComponent(state,
                                  "RoomAirSettings:ThreeNodeDisplacementVentilation",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, format("In {}={}.", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
                break;

            case RoomAir::RoomAirModel::CrossVent:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->UCSDModelUsed = true;
                IsNotOK = false;
                ValidateComponent(state,
                                  "RoomAirSettings:CrossVentilation",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, format("In {}={}.", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
                break;

            case RoomAir::RoomAirModel::UFADInt:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->UCSDModelUsed = true;
                ValidateComponent(state,
                                  "RoomAirSettings:UnderFloorAirDistributionInterior",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, format("In {}={}.", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
                break;

            case RoomAir::RoomAirModel::UFADExt:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->UCSDModelUsed = true;
                ValidateComponent(state,
                                  "RoomAirSettings:UnderFloorAirDistributionExterior",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, format("In {}={}.", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ErrorsFound = true;
                }
                break;

            case RoomAir::RoomAirModel::UserDefined:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAir->UserDefinedUsed = true;
                break;

            case RoomAir::RoomAirModel::AirflowNetwork:
                state.dataRoomAir->AirModel(ZoneNum).SimAirModel = true;
                if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirflowNetwork:SimulationControl") == 0) {
                    ShowSevereError(state,
                                    format("In {} = {}: {} = AIRFLOWNETWORK.",
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaFieldNames(3)));
                    ShowContinueError(state,
                                      "This model requires AirflowNetwork:* objects to form a complete network, including "
                                      "AirflowNetwork:Intrazone:Node and AirflowNetwork:Intrazone:Linkage.");
                    ShowContinueError(state, "AirflowNetwork:SimulationControl not found.");
                    ErrorsFound = true;
                }
                break;

            default:
                ShowWarningError(state, format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3)));
                ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, format("The mixing air model will be used for Zone ={}", state.dataIPShortCut->cAlphaArgs(2)));
                state.dataRoomAir->AirModel(ZoneNum).AirModel = RoomAir::RoomAirModel::Mixing;
            }

            state.dataRoomAir->AirModel(ZoneNum).TempCoupleScheme =
                static_cast<RoomAir::CouplingScheme>(getEnumValue(couplingSchemeNamesUC, state.dataIPShortCut->cAlphaArgs(4)));
            if (state.dataRoomAir->AirModel(ZoneNum).TempCoupleScheme == RoomAir::CouplingScheme::Invalid) {
                ShowWarningError(state, format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4)));
                ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, format("The direct coupling scheme will be used for Zone ={}", state.dataIPShortCut->cAlphaArgs(2)));
                state.dataRoomAir->AirModel(ZoneNum).TempCoupleScheme = RoomAir::CouplingScheme::Direct;
            }

        } else { // Zone Not Found
            ShowSevereError(state, format("{}, Zone not found={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(2)));
            ShowContinueError(state, format("occurs in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
            ErrorsFound = true;
        }
    } // AirModel_Param_Loop

    for (ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        // this used to be an if (NumOfAirModels == 0) block, but both the IF and the ELSE had the same content, these two lines:
        state.dataRoomAir->AirModel(ZoneNum).Name = "MIXING AIR MODEL FOR " + state.dataHeatBal->Zone(ZoneNum).Name;
        state.dataRoomAir->AirModel(ZoneNum).ZoneName = state.dataHeatBal->Zone(ZoneNum).Name;
        // set global flag for non-mixing model
        if (state.dataRoomAir->AirModel(ZoneNum).AirModel != RoomAir::RoomAirModel::Mixing) {
            state.dataRoomAir->anyNonMixingRoomAirModel = true;
        }
    }

    if (state.dataRoomAir->anyNonMixingRoomAirModel) {
        if (state.dataHeatBal->doSpaceHeatBalanceSimulation || state.dataHeatBal->doSpaceHeatBalanceSizing) {
            ShowSevereError(state, "Non-Mixing RoomAirModelType is not supported with ZoneAirHeatBalanceAlgorithm Space Heat Balance.");
            ErrorsFound = true;
        }
    }

    // Write RoomAir Model details onto EIO file
    static constexpr std::string_view RoomAirHeader("! <RoomAir Model>, Zone Name, Mixing/Mundt/UCSDDV/UCSDCV/UCSDUFI/UCSDUFE/User Defined\n");
    print(state.files.eio, RoomAirHeader);
    for (ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        static constexpr std::array<std::string_view, (int)RoomAir::RoomAirModel::Num> roomAirModelStrings = {"UserDefined",
                                                                                                              "Mixing/Well-Stirred",
                                                                                                              "OneNodeDisplacementVentilation",
                                                                                                              "ThreeNodeDisplacementVentilation",
                                                                                                              "CrossVentilation",
                                                                                                              "UnderFloorAirDistributionInterior",
                                                                                                              "UnderFloorAirDistributionExterior",
                                                                                                              "AirflowNetwork"};

        print(state.files.eio,
              "RoomAir Model,{},{}\n",
              state.dataHeatBal->Zone(ZoneNum).Name,
              roomAirModelStrings[(int)state.dataRoomAir->AirModel(ZoneNum).AirModel]);
    }

    if (ErrorsFound) {
        ShowSevereError(state, format("Errors found in processing input for {}", cCurrentModuleObject));
        errFlag = true;
    }
}

void InitAirHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for  initializations within the
    // air heat balance.

    // Do the following initializations (every time step):
    InitSimpleMixingConvectiveHeatGains(state);
}

void InitSimpleMixingConvectiveHeatGains(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       March 2003, FCW: allow individual window/door venting control
    //       DATE MODIFIED  April 2000
    //                      May 2009, Brent Griffith added EMS override to mixing and cross mixing flows
    //                      renamed routine and did some cleanup
    //                      August 2011, Therese Stovall added refrigeration door mixing flows

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the mixing and cross mixing flows

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 ZoneMixingFlowSum; // sum of zone mixing flows for a zone

    // Select type of airflow calculation
    if (state.dataHeatBal->AirFlowFlag) { // Simplified airflow calculation
        // Process the scheduled Mixing for air heat balance
        for (auto &thisMixing : state.dataHeatBal->Mixing) {
            thisMixing.DesiredAirFlowRate = thisMixing.DesignLevel * thisMixing.sched->getCurrentVal();
            if (thisMixing.EMSSimpleMixingOn) {
                thisMixing.DesiredAirFlowRate = thisMixing.EMSimpleMixingFlowRate;
            }
            thisMixing.DesiredAirFlowRateSaved = thisMixing.DesiredAirFlowRate;
        }

        // if zone air mass flow balance enforced calculate the fraction of
        // contribution of each mixing object to a zone mixed flow rate, BAN Feb 2014
        if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
            for (auto &massConservZone : state.dataHeatBal->MassConservation) {
                ZoneMixingFlowSum = 0.0;
                int NumOfMixingObjects = massConservZone.NumReceivingZonesMixingObject;
                for (int Loop = 1; Loop <= NumOfMixingObjects; ++Loop) {
                    ZoneMixingFlowSum += state.dataHeatBal->Mixing(Loop).DesignLevel;
                    massConservZone.ZoneMixingReceivingFr(Loop) = 0.0;
                }
                if (ZoneMixingFlowSum > 0.0) {
                    for (int Loop = 1; Loop <= NumOfMixingObjects; ++Loop) {
                        massConservZone.ZoneMixingReceivingFr(Loop) = state.dataHeatBal->Mixing(Loop).DesignLevel / ZoneMixingFlowSum;
                    }
                }
            }
        }

        // Process the scheduled CrossMixing for air heat balance
        for (auto &thisCrossMix : state.dataHeatBal->CrossMixing) {
            thisCrossMix.DesiredAirFlowRate = thisCrossMix.DesignLevel * thisCrossMix.sched->getCurrentVal();
            if (thisCrossMix.EMSSimpleMixingOn) {
                thisCrossMix.DesiredAirFlowRate = thisCrossMix.EMSimpleMixingFlowRate;
            }
        }

        // Note - do each Pair a Single time, so must do increment reports for both zones
        //       Can't have a pair that has ZoneA zone number = NumOfZones because organized
        //       in input with lowest zone # first no matter how input in idf

        // Process the scheduled Refrigeration Door mixing for air heat balance
        if (state.dataHeatBal->TotRefDoorMixing > 0) {
            for (int NZ = 1; NZ <= (state.dataGlobal->NumOfZones - 1);
                 ++NZ) { // Can't have %ZonePtr==NumOfZones because lesser zone # of pair placed in ZonePtr in input
                auto &thisRefDoor = state.dataHeatBal->RefDoorMixing(NZ);
                if (!thisRefDoor.RefDoorMixFlag) {
                    continue;
                }
                if (thisRefDoor.ZonePtr == NZ) {
                    for (int J = 1; J <= thisRefDoor.NumRefDoorConnections; ++J) {
                        thisRefDoor.VolRefDoorFlowRate(J) = 0.0;
                        if (thisRefDoor.EMSRefDoorMixingOn(J)) {
                            thisRefDoor.VolRefDoorFlowRate(J) = thisRefDoor.EMSRefDoorFlowRate(J);
                        }
                    }
                }
            }
        } // TotRefDoorMixing

        // Infiltration and ventilation calculations have been moved to a subroutine of CalcAirFlowSimple in HVAC Manager
    }
}

void CalcHeatBalanceAir(EnergyPlusData &state)
{

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the air component of the heat balance.

    if (state.dataGlobal->externalHVACManager) {
        if (!state.dataGlobal->externalHVACManagerInitialized) {
            initializeForExternalHVACManager(state);
        }
        state.dataGlobal->externalHVACManager(&state);
    } else {
        HVACManager::ManageHVAC(state);
    }
}

// END Algorithm Section of the Module

void initializeForExternalHVACManager(EnergyPlusData &state)
{
    // this function will ultimately provide a nice series of calls that initialize all the hvac stuff needed
    // to allow an external hvac manager to play nice with E+
    EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints(state);
}

void ReportZoneMeanAirTemp(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the report variables for the AirHeatBalance.
    if (state.dataHeatBalAirMgr->CalcExtraReportVarMyOneTimeFlag) {
        for (auto const *reqVar : state.dataOutputProcessor->reqVars) {
            if (reqVar->name == "ZONE WETBULB GLOBE TEMPERATURE") {
                if (reqVar->key.empty()) {
                    for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
                        auto &thisZnAirRpt = state.dataHeatBal->ZnAirRpt(ZoneLoop);
                        thisZnAirRpt.ReportWBGT = true;
                    }
                } else {
                    int ZoneLoop = Util::FindItemInList(Util::makeUPPER(reqVar->key), state.dataHeatBal->Zone);
                    if (ZoneLoop > 0) {
                        auto &thisZnAirRpt = state.dataHeatBal->ZnAirRpt(ZoneLoop);
                        thisZnAirRpt.ReportWBGT = true;
                    }
                }
            } else if (reqVar->name == "SPACE WETBULB GLOBE TEMPERATURE") {
                if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
                    if (reqVar->key.empty()) {
                        for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
                            auto &thisSpaceAirRpt = state.dataHeatBal->spaceAirRpt(spaceNum);
                            thisSpaceAirRpt.ReportWBGT = true;
                        }
                    } else {
                        int spaceNum = Util::FindItemInList(Util::makeUPPER(reqVar->key), state.dataHeatBal->space);
                        if (spaceNum > 0) {
                            auto &thisSpaceAirRpt = state.dataHeatBal->spaceAirRpt(spaceNum);
                            thisSpaceAirRpt.ReportWBGT = true;
                        }
                    }
                }
            }
        }

        // EMS sensor request WBGT check
        for (int loop = 1; loop <= state.dataRuntimeLang->NumSensors; ++loop) {
            if (state.dataRuntimeLang->Sensor(loop).OutputVarName == "ZONE WETBULB GLOBE TEMPERATURE") {
                int ZoneLoop = Util::FindItemInList(state.dataRuntimeLang->Sensor(loop).UniqueKeyName, state.dataHeatBal->Zone);
                if (ZoneLoop > 0) {
                    auto &thisZnAirRpt = state.dataHeatBal->ZnAirRpt(ZoneLoop);
                    thisZnAirRpt.ReportWBGT = true;
                }
            } else if (state.dataRuntimeLang->Sensor(loop).OutputVarName == "SPACE WETBULB GLOBE TEMPERATURE") {
                if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
                    int spaceNum = Util::FindItemInList(state.dataRuntimeLang->Sensor(loop).UniqueKeyName, state.dataHeatBal->space);
                    if (spaceNum > 0) {
                        auto &thisSpaceAirRpt = state.dataHeatBal->spaceAirRpt(spaceNum);
                        thisSpaceAirRpt.ReportWBGT = true;
                    }
                }
            }
        }
        state.dataHeatBalAirMgr->CalcExtraReportVarMyOneTimeFlag = false;
    }
    for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
        auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneLoop);
        calcMeanAirTemps(state, thisZoneHB.ZTAV, thisZoneHB.airHumRatAvg, thisZoneHB.MRT, state.dataHeatBal->ZnAirRpt(ZoneLoop), ZoneLoop);
        if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneLoop).spaceIndexes) {
                auto const &thisSpaceHB = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum);
                calcMeanAirTemps(
                    state, thisSpaceHB.ZTAV, thisSpaceHB.airHumRatAvg, thisSpaceHB.MRT, state.dataHeatBal->spaceAirRpt(spaceNum), ZoneLoop);
            }
        }
    }
}

void calcMeanAirTemps(EnergyPlusData &state,
                      Real64 const ZTAV,
                      Real64 const airHumRatAvg,
                      Real64 const MRT,
                      DataHeatBalance::AirReportVars &thisAirRpt,
                      int const zoneNum)
{
    // The mean air temperature is actually ZTAV which is the average
    // temperature of the air temperatures at the system time step for the
    // entire zone time step.
    thisAirRpt.MeanAirTemp = ZTAV;
    thisAirRpt.MeanAirHumRat = airHumRatAvg;
    thisAirRpt.OperativeTemp = 0.5 * (ZTAV + MRT);
    thisAirRpt.MeanAirDewPointTemp = Psychrometrics::PsyTdpFnWPb(state, thisAirRpt.MeanAirHumRat, state.dataEnvrn->OutBaroPress);

    // if operative temperature control is being used, then radiative fraction/weighting
    //  might be defined by user to be something different than 0.5, even scheduled over simulation period
    if (state.dataZoneCtrls->AnyOpTempControl) { // dig further...
        // find TempControlledZoneID from ZoneLoop index
        int TempControlledZoneID = state.dataHeatBal->Zone(zoneNum).TempControlledZoneIndex;
        if (state.dataHeatBal->Zone(zoneNum).IsControlled) {
            if ((state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).OpTempCtrl != DataZoneControls::TempCtrl::None)) {
                Real64 thisMRTFraction; // temp working value for radiative fraction/weight
                // is operative temp radiative fraction scheduled or fixed?
                if (state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).OpTempCtrl == DataZoneControls::TempCtrl::Scheduled) {
                    thisMRTFraction = state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).opTempRadiativeFractionSched->getCurrentVal();
                } else {
                    thisMRTFraction = state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).FixedRadiativeFraction;
                }
                thisAirRpt.OperativeTemp = 0.5 * (ZTAV + MRT);
                thisAirRpt.ThermOperativeTemp = (1.0 - thisMRTFraction) * ZTAV + thisMRTFraction * MRT;
            }
        }
    }
    if (thisAirRpt.ReportWBGT) {
        // note that the WetBulbTemp here is for temporary verification, it will not be another added reporting variable
        thisAirRpt.WetbulbGlobeTemp =
            0.7 * Psychrometrics::PsyTwbFnTdbWPb(state, ZTAV, airHumRatAvg, state.dataEnvrn->OutBaroPress) + 0.3 * thisAirRpt.OperativeTemp;
    }
}

} // namespace EnergyPlus::HeatBalanceAirManager
