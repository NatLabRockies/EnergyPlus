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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneratorDynamicsManager.hh>
#include <EnergyPlus/GeneratorFuelSupply.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::MicroCHPElectricGenerator {

// MODULE INFORMATION:
//       AUTHOR         Brent Griffith
//       DATE WRITTEN   June 2006
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This module simulates the operation of Internal Combustion and Stirling Engine
//  residential-scale generators for combined heat and power.

// METHODOLOGY EMPLOYED:
// Once the ElectricPowerManager determines that the Combustion Generator
// is available to meet an electric load demand, it calls SimCombustionGenerator
// which in turn calls the Combustion model.
// See DataFuelCells.cc for structures and variables

// REFERENCES:
// IEA/ECBCS Annex 42 model specification titled: " A Generic Model Specification for
// Combustion-based Residential CHP Devices"  Alex Ferguson, Nick Kelly, IEA Annex 42.
// Module developed from

PlantComponent *MicroCHPDataStruct::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data
    if (state.dataCHPElectGen->getMicroCHPInputFlag) {
        GetMicroCHPGeneratorInput(state);
        state.dataCHPElectGen->getMicroCHPInputFlag = false;
    }

    // Now look for this object
    for (auto &thisMCHP : state.dataCHPElectGen->MicroCHP) {
        if (thisMCHP.Name == objectName) {
            return &thisMCHP;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalMicroCHPGenFactory: Error getting inputs for micro-CHP gen named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void GetMicroCHPGeneratorInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Brent Griffith
    //       DATE WRITTEN:    July 2005

    // PURPOSE OF THIS SUBROUTINE:
    // This routine will get the input
    // required by the Micro CHP Generator models.

    // METHODOLOGY EMPLOYED:
    // EnergyPlus input processor
    static constexpr std::string_view routineName = "GetMicroCHPGeneratorInput";

    Array1D_string AlphArray(25);  // character string data
    Array1D<Real64> NumArray(200); // numeric data TODO deal with allocatable for extensible

    auto &s_ipsc = state.dataIPShortCut;

    if (state.dataCHPElectGen->MyOneTimeFlag) {
        int NumAlphas = 0;        // Number of elements in the alpha array
        int NumNums = 0;          // Number of elements in the numeric array
        int IOStat = 0;           // IO Status when calling get input subroutine
        bool ErrorsFound = false; // error flag

        // call to Fuel supply module to set up data there.
        GeneratorFuelSupply::GetGeneratorFuelSupplyInput(state);

        // First get the Micro CHP Parameters so they can be nested in structure later
        s_ipsc->cCurrentModuleObject = "Generator:MicroCHP:NonNormalizedParameters";
        state.dataCHPElectGen->NumMicroCHPParams = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (state.dataCHPElectGen->NumMicroCHPParams <= 0) {
            ShowSevereError(state, format("No {} equipment specified in input file", s_ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }

        state.dataCHPElectGen->MicroCHPParamInput.allocate(state.dataCHPElectGen->NumMicroCHPParams);

        for (int CHPParamNum = 1; CHPParamNum <= state.dataCHPElectGen->NumMicroCHPParams; ++CHPParamNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     CHPParamNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, AlphArray(1)};

            auto &microCHPParams = state.dataCHPElectGen->MicroCHPParamInput(CHPParamNum);

            std::string ObjMSGName = s_ipsc->cCurrentModuleObject + " Named " + AlphArray(1);

            microCHPParams.Name = AlphArray(1);        // A1 name
            microCHPParams.MaxElecPower = NumArray(1); // N1 Maximum Electric Power [W]
            microCHPParams.MinElecPower = NumArray(2); // N2 Minimum Electric Power [W]
            microCHPParams.MinWaterMdot = NumArray(3); // N3 Minimum Cooling Water Flow Rate [kg/s]
            microCHPParams.MaxWaterTemp = NumArray(4); // N3 Maximum Cooling Water Inlet Temp [C]

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
                ErrorsFound = true;
            } else if ((microCHPParams.ElecEffCurve = Curve::GetCurve(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), AlphArray(2));
                ErrorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                ErrorsFound = true;
            } else if ((microCHPParams.ThermalEffCurve = Curve::GetCurve(state, AlphArray(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), AlphArray(3));
                ErrorsFound = true;
            }

            if (Util::SameString(AlphArray(4), "InternalControl")) {
                microCHPParams.InternalFlowControl = true; //  A4, \field Cooling Water Flow Rate Mode
                microCHPParams.PlantFlowControl = false;
            }
            if ((!(Util::SameString(AlphArray(4), "InternalControl"))) && (!(Util::SameString(AlphArray(4), "PlantControl")))) {
                ShowSevereError(state, format("Invalid, {} = {}", s_ipsc->cAlphaFieldNames(4), AlphArray(4)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
            if (microCHPParams.InternalFlowControl) { // get the curve
                if (s_ipsc->lAlphaFieldBlanks(5)) {
                    ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(5));
                    ErrorsFound = true;
                } else if ((microCHPParams.WaterFlowCurve = Curve::GetCurve(state, AlphArray(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), AlphArray(5));
                    ErrorsFound = true;
                }
            }

            if (s_ipsc->lAlphaFieldBlanks(6)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(6));
                ErrorsFound = true;
            } else if ((microCHPParams.AirFlowCurve = Curve::GetCurve(state, AlphArray(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), AlphArray(6));
                ErrorsFound = true;
            }

            //  Name of Curve for Air Flow Rate
            microCHPParams.DeltaPelMax = NumArray(5);       // N5 Maximum rate of change in net electrical power [W/s]
            microCHPParams.DeltaFuelMdotMax = NumArray(6);  // N6 Maximum Rate of change in fuel flow rate [kg/s2]
            microCHPParams.UAhx = NumArray(7);              // N7 Heat Exchanger UA_hx
            microCHPParams.UAskin = NumArray(8);            // N8 Skin Loss UA_loss
            microCHPParams.RadiativeFraction = NumArray(9); // N9 radiative fraction for skin losses
            microCHPParams.MCeng = NumArray(10);            // N10 Aggregated Thermal Mass of Generator MC_eng
            if (microCHPParams.MCeng <= 0.0) {
                ShowSevereError(state, format("Invalid, {} = {:.5R}", s_ipsc->cNumericFieldNames(10), NumArray(10)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ShowContinueError(state, "Thermal mass must be greater than zero");
                ErrorsFound = true;
            }
            microCHPParams.MCcw = NumArray(11); // Aggregated Thermal Mass of Heat Recovery MC_cw
            if (microCHPParams.MCcw <= 0.0) {
                ShowSevereError(state, format("Invalid, {} = {:.5R}", s_ipsc->cNumericFieldNames(11), NumArray(11)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ShowContinueError(state, "Thermal mass must be greater than zero");
                ErrorsFound = true;
            }
            microCHPParams.Pstandby = NumArray(12); // N12 Standby Power [W]

            if (Util::SameString(AlphArray(7), "TimeDelay")) {
                microCHPParams.WarmUpByTimeDelay = true;
                microCHPParams.WarmUpByEngineTemp = false;
            }
            if ((!(Util::SameString(AlphArray(7), "NominalEngineTemperature"))) && (!(Util::SameString(AlphArray(7), "TimeDelay")))) {
                ShowSevereError(state, format("Invalid, {} = {}", s_ipsc->cAlphaFieldNames(7), AlphArray(7)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
            microCHPParams.kf = NumArray(13);          // N13 Warmup Fuel Flow Rate Coefficient k_f
            microCHPParams.TnomEngOp = NumArray(14);   // N14 Nominal Engine Operating Temperature [C]
            microCHPParams.kp = NumArray(15);          // N15 Warmup Power Coefficient k_p
            microCHPParams.Rfuelwarmup = NumArray(16); // N16 Warm Up Fuel Flow Rate Limit Ratio
            microCHPParams.WarmUpDelay = NumArray(17); // N17 Warm Up Delay Time

            microCHPParams.PcoolDown = NumArray(18); // N18 Cool Down Power

            microCHPParams.CoolDownDelay = NumArray(19); // N19 Cool Down Delay Time in seconds

            if (Util::SameString(AlphArray(8), "MandatoryCoolDown")) {
                microCHPParams.MandatoryFullCoolDown = true;
                microCHPParams.WarmRestartOkay = false;
            }
            if ((!(Util::SameString(AlphArray(8), "MandatoryCoolDown"))) && (!(Util::SameString(AlphArray(8), "OptionalCoolDown")))) {
                ShowSevereError(state, format("Invalid, {} = {}", s_ipsc->cAlphaFieldNames(8), AlphArray(8)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        s_ipsc->cCurrentModuleObject = "Generator:MicroCHP";
        state.dataCHPElectGen->NumMicroCHPs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (state.dataCHPElectGen->NumMicroCHPs <= 0) {
            // shouldn't ever come here?
            ShowSevereError(state, format("No {} equipment specified in input file", s_ipsc->cCurrentModuleObject));
            ErrorsFound = true;
        }

        if (!(allocated(state.dataCHPElectGen->MicroCHP))) {
            state.dataCHPElectGen->MicroCHP.allocate(state.dataCHPElectGen->NumMicroCHPs); // inits handled in derived type definitions
        }

        // load in Micro CHPs
        for (int GeneratorNum = 1; GeneratorNum <= state.dataCHPElectGen->NumMicroCHPs; ++GeneratorNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     s_ipsc->cCurrentModuleObject,
                                                                     GeneratorNum,
                                                                     AlphArray,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     s_ipsc->lAlphaFieldBlanks,
                                                                     s_ipsc->cAlphaFieldNames,
                                                                     s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, AlphArray(1)};

            auto &microCHP = state.dataCHPElectGen->MicroCHP(GeneratorNum);
            // GENERATOR:MICRO CHP,
            microCHP.DynamicsControlID = GeneratorNum;
            microCHP.Name = AlphArray(1);         //  A1 Generator name
            microCHP.ParamObjName = AlphArray(2); //  A2 Micro CHP Parameter Object Name
            // find input structure
            int thisParamID = Util::FindItemInList(AlphArray(2), state.dataCHPElectGen->MicroCHPParamInput);
            if (thisParamID != 0) {
                microCHP.A42Model = state.dataCHPElectGen->MicroCHPParamInput(thisParamID); // entire structure of input data assigned here!
            } else {
                ShowSevereError(state, format("Invalid, {} = {}", s_ipsc->cAlphaFieldNames(2), AlphArray(2)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }

            if (!s_ipsc->lAlphaFieldBlanks(3)) {
                microCHP.ZoneName = AlphArray(3); //  A3 Zone Name
                microCHP.ZoneID = Util::FindItemInList(microCHP.ZoneName, state.dataHeatBal->Zone);
                if (microCHP.ZoneID == 0) {
                    ShowSevereError(state, format("Invalid, {} = {}", s_ipsc->cAlphaFieldNames(3), AlphArray(3)));
                    ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                    ErrorsFound = true;
                }
            } else {
                microCHP.ZoneID = 0;
            }
            microCHP.PlantInletNodeName = AlphArray(4);  //  A4 Cooling Water Inlet Node Name
            microCHP.PlantOutletNodeName = AlphArray(5); //  A5 Cooling Water Outlet Node Name
            // find node ids for water path
            microCHP.PlantInletNodeID = NodeInputManager::GetOnlySingleNode(state,
                                                                            AlphArray(4),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::GeneratorMicroCHP,
                                                                            AlphArray(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
            microCHP.PlantOutletNodeID = NodeInputManager::GetOnlySingleNode(state,
                                                                             AlphArray(5),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::GeneratorMicroCHP,
                                                                             AlphArray(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state, s_ipsc->cCurrentModuleObject, AlphArray(1), AlphArray(4), AlphArray(5), "Heat Recovery Nodes");

            microCHP.AirInletNodeName = AlphArray(6); //  A6 Air Inlet Node Name
            // check the node connections
            microCHP.AirInletNodeID = NodeInputManager::GetOnlySingleNode(state,
                                                                          AlphArray(6),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::GeneratorMicroCHP,
                                                                          AlphArray(1),
                                                                          DataLoopNode::NodeFluidType::Air,
                                                                          DataLoopNode::ConnectionType::Inlet,
                                                                          NodeInputManager::CompFluidStream::Secondary,
                                                                          DataLoopNode::ObjectIsNotParent);

            microCHP.AirOutletNodeName = AlphArray(7); //  A7 Air Outlet Node Name
            microCHP.AirOutletNodeID = NodeInputManager::GetOnlySingleNode(state,
                                                                           AlphArray(7),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::GeneratorMicroCHP,
                                                                           AlphArray(1),
                                                                           DataLoopNode::NodeFluidType::Air,
                                                                           DataLoopNode::ConnectionType::Outlet,
                                                                           NodeInputManager::CompFluidStream::Secondary,
                                                                           DataLoopNode::ObjectIsNotParent);

            microCHP.FuelSupplyID = Util::FindItemInList(AlphArray(8), state.dataGenerator->FuelSupply); // Fuel Supply ID
            if (microCHP.FuelSupplyID == 0) {
                ShowSevereError(state, format("Invalid, {} = {}", s_ipsc->cAlphaFieldNames(8), AlphArray(8)));
                ShowContinueError(state, format("Entered in {}={}", s_ipsc->cCurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(9)) {
                microCHP.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((microCHP.availSched = Sched::GetSchedule(state, AlphArray(9))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(9), AlphArray(9));
                ErrorsFound = true;
            }
            microCHP.A42Model.TengLast = 20.0;      // inits
            microCHP.A42Model.TempCWOutLast = 20.0; // inits
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", s_ipsc->cCurrentModuleObject));
        }

        // setup report variables
        for (int GeneratorNum = 1; GeneratorNum <= state.dataCHPElectGen->NumMicroCHPs; ++GeneratorNum) {
        }

        state.dataCHPElectGen->MyOneTimeFlag = false;
    }
}

void MicroCHPDataStruct::setupOutputVars(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Generator Off Mode Time",
                        Constant::Units::s,
                        this->A42Model.OffModeTime,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Standby Mode Time",
                        Constant::Units::s,
                        this->A42Model.StandyByModeTime,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Warm Up Mode Time",
                        Constant::Units::s,
                        this->A42Model.WarmUpModeTime,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Normal Operating Mode Time",
                        Constant::Units::s,
                        this->A42Model.NormalModeTime,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Cool Down Mode Time",
                        Constant::Units::s,
                        this->A42Model.CoolDownModeTime,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Produced AC Electricity Rate",
                        Constant::Units::W,
                        this->A42Model.ACPowerGen,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Produced AC Electricity Energy",
                        Constant::Units::J,
                        this->A42Model.ACEnergyGen,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::ElectricityProduced,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cogeneration);

    SetupOutputVariable(state,
                        "Generator Produced Thermal Rate",
                        Constant::Units::W,
                        this->A42Model.QdotHR,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Produced Thermal Energy",
                        Constant::Units::J,
                        this->A42Model.TotalHeatEnergyRec,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cogeneration);

    SetupOutputVariable(state,
                        "Generator Electric Efficiency",
                        Constant::Units::None,
                        this->A42Model.ElecEff,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Thermal Efficiency",
                        Constant::Units::None,
                        this->A42Model.ThermEff,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Gross Input Heat Rate",
                        Constant::Units::W,
                        this->A42Model.Qgross,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Steady State Engine Heat Generation Rate",
                        Constant::Units::W,
                        this->A42Model.Qgenss,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Engine Heat Exchange Rate",
                        Constant::Units::W,
                        this->A42Model.QdotHX,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Air Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->A42Model.MdotAir,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel Molar Flow Rate",
                        Constant::Units::kmol_s,
                        this->A42Model.NdotFuel,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->A42Model.MdotFuel,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Engine Temperature",
                        Constant::Units::C,
                        this->A42Model.Teng,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Coolant Inlet Temperature",
                        Constant::Units::C,
                        this->A42Model.HeatRecInletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Coolant Outlet Temperature",
                        Constant::Units::C,
                        this->A42Model.HeatRecOutletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    // this next one needs to be reconciled with non-gas fuel constituents.
    //   need custom resourceTypeKey or something for user defined fuel compositions.
    SetupOutputVariable(state,
                        "Generator Fuel HHV Basis Energy",
                        Constant::Units::J,
                        this->A42Model.FuelEnergyHHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::NaturalGas,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cogeneration);

    SetupOutputVariable(state,
                        "Generator Fuel HHV Basis Rate",
                        Constant::Units::W,
                        this->A42Model.FuelEnergyUseRateHHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel LHV Basis Energy",
                        Constant::Units::J,
                        this->A42Model.FuelEnergyLHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel LHV Basis Rate",
                        Constant::Units::W,
                        this->A42Model.FuelEnergyUseRateLHV,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel Compressor Electricity Rate",
                        Constant::Units::W,
                        this->A42Model.FuelCompressPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel Compressor Electricity Energy",
                        Constant::Units::J,
                        this->A42Model.FuelCompressEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Fuel Compressor Skin Heat Loss Rate",
                        Constant::Units::W,
                        this->A42Model.FuelCompressSkinLoss,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Zone Sensible Heat Transfer Rate",
                        Constant::Units::W,
                        this->A42Model.SkinLossPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Zone Sensible Heat Transfer Energy",
                        Constant::Units::J,
                        this->A42Model.SkinLossEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Zone Convection Heat Transfer Rate",
                        Constant::Units::W,
                        this->A42Model.SkinLossConvect,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Generator Zone Radiation Heat Transfer Rate",
                        Constant::Units::W,
                        this->A42Model.SkinLossRadiat,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    if (this->ZoneID > 0) {
        SetupZoneInternalGain(state,
                              this->ZoneID,
                              this->Name,
                              DataHeatBalance::IntGainType::GeneratorMicroCHP,
                              &this->A42Model.SkinLossConvect,
                              nullptr,
                              &this->A42Model.SkinLossRadiat);
    }
}

void MicroCHPDataStruct::simulate(EnergyPlusData &state,
                                  [[maybe_unused]] const EnergyPlus::PlantLocation &calledFromLocation,
                                  bool FirstHVACIteration,
                                  [[maybe_unused]] Real64 &CurLoad,
                                  [[maybe_unused]] bool RunFlag)
{
    // empty function to emulate current behavior as of conversion to using the PlantComponent calling structure.
    // calls from the plant side only update the nodes.
    // calls from the ElectricPowerServiceManger call the init, calc, and update worker functions

    PlantUtilities::UpdateComponentHeatRecoverySide(state,
                                                    this->CWPlantLoc.loopNum,
                                                    this->CWPlantLoc.loopSideNum,
                                                    DataPlant::PlantEquipmentType::Generator_MicroCHP,
                                                    this->PlantInletNodeID,
                                                    this->PlantOutletNodeID,
                                                    this->A42Model.QdotHR,
                                                    this->A42Model.HeatRecInletTemp,
                                                    this->A42Model.HeatRecOutletTemp,
                                                    this->PlantMassFlowRate,
                                                    FirstHVACIteration);
}

void MicroCHPDataStruct::onInitLoopEquip(EnergyPlusData &state, const EnergyPlus::PlantLocation &)
{
    static constexpr std::string_view RoutineName("MicroCHPDataStruct::onInitLoopEquip");

    Real64 rho = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum)
                     .glycol->getDensity(state, state.dataLoopNodes->Node(this->PlantInletNodeID).Temp, RoutineName);
    if (this->A42Model.InternalFlowControl) { // got a curve
        this->PlantMassFlowRateMax =
            2.0 * this->A42Model.WaterFlowCurve->value(state, this->A42Model.MaxElecPower, state.dataLoopNodes->Node(this->PlantInletNodeID).Temp);
    } else if (this->CWPlantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply) {
        if (state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).MaxMassFlowRate > 0.0) {
            this->PlantMassFlowRateMax = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).MaxMassFlowRate;
        } else if (state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).PlantSizNum > 0) {
            this->PlantMassFlowRateMax = state.dataSize->PlantSizData(this->CWPlantLoc.loopNum).DesVolFlowRate * rho;
        } else {
            this->PlantMassFlowRateMax = 2.0;
        }

    } else if (this->CWPlantLoc.loopSideNum == DataPlant::LoopSideLocation::Demand) {
        this->PlantMassFlowRateMax = 2.0; // would like to use plant loop max but not ready yet
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->PlantInletNodeID, this->PlantMassFlowRateMax / rho);

    this->A42Model.ElecEff = this->A42Model.ElecEffCurve->value(
        state, this->A42Model.MaxElecPower, this->PlantMassFlowRateMax, state.dataLoopNodes->Node(this->PlantInletNodeID).Temp);

    this->A42Model.ThermEff = this->A42Model.ThermalEffCurve->value(
        state, this->A42Model.MaxElecPower, this->PlantMassFlowRateMax, state.dataLoopNodes->Node(this->PlantInletNodeID).Temp);

    GeneratorDynamicsManager::SetupGeneratorControlStateManager(state, this->DynamicsControlID);
}

void MicroCHPDataStruct::InitMicroCHPNoNormalizeGenerators(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         BGriffith
    //       DATE WRITTEN   March 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    this->oneTimeInit(state);

    if (!state.dataGlobal->SysSizingCalc && this->MySizeFlag && !this->MyPlantScanFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        this->MySizeFlag = false;
    }

    if (this->MySizeFlag) {
        return;
    }

    int DynaCntrlNum = this->DynamicsControlID;

    if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {
        // reset to starting condition for different environment runperiods, design days
        this->A42Model.TengLast = 20.0;
        this->A42Model.TempCWOutLast = 20.0;
        this->A42Model.TimeElapsed = 0.0;
        this->A42Model.OpMode = DataGenerators::OperatingMode::Invalid;
        this->A42Model.OffModeTime = 0.0;
        this->A42Model.StandyByModeTime = 0.0;
        this->A42Model.WarmUpModeTime = 0.0;
        this->A42Model.NormalModeTime = 0.0;
        this->A42Model.CoolDownModeTime = 0.0;
        this->A42Model.Pnet = 0.0;
        this->A42Model.ElecEff = 0.0;
        this->A42Model.Qgross = 0.0;
        this->A42Model.ThermEff = 0.0;
        this->A42Model.Qgenss = 0.0;
        this->A42Model.NdotFuel = 0.0;
        this->A42Model.MdotFuel = 0.0;
        this->A42Model.Teng = 20.0;
        this->A42Model.TcwIn = 20.0;
        this->A42Model.TcwOut = 20.0;
        this->A42Model.MdotAir = 0.0;
        this->A42Model.QdotSkin = 0.0;
        this->A42Model.QdotConvZone = 0.0;
        this->A42Model.QdotRadZone = 0.0;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).LastOpMode = DataGenerators::OperatingMode::Off;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).CurrentOpMode = DataGenerators::OperatingMode::Off;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).FractionalDayofLastShutDown = 0.0;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).FractionalDayofLastStartUp = 0.0;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).HasBeenOn = false;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).DuringStartUp = false;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).DuringShutDown = false;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).FuelMdotLastTimestep = 0.0;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).PelLastTimeStep = 0.0;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).NumCycles = 0;

        state.dataGenerator->FuelSupply(this->FuelSupplyID).QskinLoss = 0.0;

        PlantUtilities::InitComponentNodes(state, 0.0, this->PlantMassFlowRateMax, this->PlantInletNodeID, this->PlantOutletNodeID);
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    Real64 TimeElapsed =
        state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;
    if (this->A42Model.TimeElapsed != TimeElapsed) {
        // The simulation has advanced to the next system timestep.  Save conditions from the end of the previous system
        // timestep for use as the initial conditions of each iteration that does not advance the system timestep.
        this->A42Model.TengLast = this->A42Model.Teng;
        this->A42Model.TempCWOutLast = this->A42Model.TcwOut;
        this->A42Model.TimeElapsed = TimeElapsed;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).LastOpMode = state.dataGenerator->GeneratorDynamics(DynaCntrlNum).CurrentOpMode;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).FuelMdotLastTimestep = this->A42Model.MdotFuel;
        state.dataGenerator->GeneratorDynamics(DynaCntrlNum).PelLastTimeStep = this->A42Model.Pnet;
    }

    if (!this->A42Model.InternalFlowControl) {

        Real64 mdot = this->PlantMassFlowRateMax;
        PlantUtilities::SetComponentFlowRate(state, mdot, this->PlantInletNodeID, this->PlantOutletNodeID, this->CWPlantLoc);
        this->PlantMassFlowRate = mdot;
    }
}

void MicroCHPDataStruct::CalcMicroCHPNoNormalizeGeneratorModel(EnergyPlusData &state,
                                                               bool const RunFlagElectCenter, // TRUE when Generator operating
                                                               bool const RunFlagPlant,
                                                               Real64 const MyElectricLoad, // Generator demand
                                                               Real64 const MyThermalLoad)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR        B Griffith
    //       DATE WRITTEN   July 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Main calculation subroutine for the IEA Annex 42 model

    // METHODOLOGY EMPLOYED:
    // curve fit, dynamic control limits,

    // REFERENCES:
    // IEA Annex 42 FC-COGEN-SIM "A Generic Model Specification for Combustion-based Residential CHP Devices"
    // Alex Ferguson, Nick Kelly, Version 3, June 26, 2006

    static constexpr std::string_view RoutineName("CalcMicroCHPNoNormalizeGeneratorModel");

    DataGenerators::OperatingMode CurrentOpMode = DataGenerators::OperatingMode::Invalid;
    Real64 NdotFuel;
    Real64 AllowedLoad = 0.0;
    Real64 PLRforSubtimestepStartUp(1.0);
    Real64 PLRforSubtimestepShutDown(0.0);

    GeneratorDynamicsManager::ManageGeneratorControlState(state,
                                                          this->DynamicsControlID,
                                                          RunFlagElectCenter,
                                                          RunFlagPlant,
                                                          MyElectricLoad,
                                                          MyThermalLoad,
                                                          AllowedLoad,
                                                          CurrentOpMode,
                                                          PLRforSubtimestepStartUp,
                                                          PLRforSubtimestepShutDown);

    Real64 Teng = this->A42Model.Teng;
    Real64 TcwOut = this->A42Model.TcwOut;

    Real64 thisAmbientTemp;
    if (this->ZoneID > 0) {
        thisAmbientTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->ZoneID).MAT;
    } else { // outdoor location, no zone
        thisAmbientTemp = state.dataEnvrn->OutDryBulbTemp;
    }

    Real64 Pnetss = 0.0;
    Real64 Pstandby = 0.0; // power draw during standby, positive here means negative production
    Real64 Pcooler = 0.0;  // power draw during cool down, positive here means negative production
    Real64 ElecEff = 0.0;
    Real64 MdotAir = 0.0;
    Real64 Qgenss = 0.0;
    Real64 MdotCW = 0.0;
    Real64 TcwIn = 0.0;
    Real64 MdotFuel = 0.0;
    Real64 Qgross = 0.0;
    Real64 ThermEff = 0.0;

    switch (CurrentOpMode) {
    case DataGenerators::OperatingMode::Off: { // same as standby in model spec but no Pnet standby electricity losses.

        Qgenss = 0.0;
        TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp; // C
        Pnetss = 0.0;
        Pstandby = 0.0;
        Pcooler = this->A42Model.PcoolDown * PLRforSubtimestepShutDown;
        ElecEff = 0.0;
        ThermEff = 0.0;
        Qgross = 0.0;
        NdotFuel = 0.0;
        MdotFuel = 0.0;
        MdotAir = 0.0;

        MdotCW = 0.0;
        PlantUtilities::SetComponentFlowRate(state, MdotCW, this->PlantInletNodeID, this->PlantOutletNodeID, this->CWPlantLoc);
        this->PlantMassFlowRate = MdotCW;
    } break;
    case DataGenerators::OperatingMode::Standby: {
        Qgenss = 0.0;
        TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp; // C
        Pnetss = 0.0;
        Pstandby = this->A42Model.Pstandby * (1.0 - PLRforSubtimestepShutDown);
        Pcooler = this->A42Model.PcoolDown * PLRforSubtimestepShutDown;
        ElecEff = 0.0;
        ThermEff = 0.0;
        Qgross = 0.0;
        NdotFuel = 0.0;
        MdotFuel = 0.0;
        MdotAir = 0.0;

        MdotCW = 0.0;
        PlantUtilities::SetComponentFlowRate(state, MdotCW, this->PlantInletNodeID, this->PlantOutletNodeID, this->CWPlantLoc);
        this->PlantMassFlowRate = MdotCW;
    } break;
    case DataGenerators::OperatingMode::WarmUp: {
        if (this->A42Model.WarmUpByTimeDelay) {
            // Internal combustion engine.  This is just like normal  operation but no net power yet.
            Pnetss = MyElectricLoad; // W
            Pstandby = 0.0;
            Pcooler = this->A42Model.PcoolDown * PLRforSubtimestepShutDown;
            TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp;          // C
            MdotCW = state.dataLoopNodes->Node(this->PlantInletNodeID).MassFlowRate; // kg/s
            if (this->A42Model.InternalFlowControl) {
                MdotCW = GeneratorDynamicsManager::FuncDetermineCWMdotForInternalFlowControl(state, this->DynamicsControlID, Pnetss, TcwIn);
            }
            ElecEff = this->A42Model.ElecEffCurve->value(state, Pnetss, MdotCW, TcwIn);
            ElecEff = max(0.0, ElecEff); // protect against bad curve result

            if (ElecEff > 0.0) {           // trap divide by bad thing
                Qgross = Pnetss / ElecEff; // W
            } else {
                Qgross = 0.0;
            }
            ThermEff = this->A42Model.ThermalEffCurve->value(state, Pnetss, MdotCW, TcwIn);
            ThermEff = max(0.0, ThermEff); // protect against bad curve result

            Qgenss = ThermEff * Qgross; // W

            MdotFuel = Qgross / (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0) *
                       state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
            //  kMol/s = (J/s) /(KJ/mol * 1000 J/KJ * 1000 mol/kmol)

            bool ConstrainedIncreasingNdot(false);
            bool ConstrainedDecreasingNdot(false);
            Real64 MdotFuelAllowed = 0.0;

            GeneratorDynamicsManager::ManageGeneratorFuelFlow(
                state, this->DynamicsControlID, MdotFuel, MdotFuelAllowed, ConstrainedIncreasingNdot, ConstrainedDecreasingNdot);

            if (ConstrainedIncreasingNdot || ConstrainedDecreasingNdot) { // recalculate Pnetss with new NdotFuel with iteration
                MdotFuel = MdotFuelAllowed;
                NdotFuel = MdotFuel / state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
                Qgross = NdotFuel * (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0);

                for (int i = 1; i <= 20; ++i) { // iterating here  could add use of search method
                    Pnetss = Qgross * ElecEff;
                    if (this->A42Model.InternalFlowControl) {
                        MdotCW = GeneratorDynamicsManager::FuncDetermineCWMdotForInternalFlowControl(state, this->DynamicsControlID, Pnetss, TcwIn);
                    }
                    ElecEff = this->A42Model.ElecEffCurve->value(state, Pnetss, MdotCW, TcwIn);
                    ElecEff = max(0.0, ElecEff); // protect against bad curve result
                }

                ThermEff = this->A42Model.ThermalEffCurve->value(state, Pnetss, MdotCW, TcwIn);
                ThermEff = max(0.0, ThermEff); // protect against bad curve result
                Qgenss = ThermEff * Qgross;    // W
            }
            Pnetss = 0.0; // no actually power produced here.
            NdotFuel = MdotFuel / state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
            MdotAir = this->A42Model.AirFlowCurve->value(state, MdotFuel);
            MdotAir = max(0.0, MdotAir); // protect against bad curve result

        } else if (this->A42Model.WarmUpByEngineTemp) {
            // Stirling engine mode warm up
            //   find MdotFuelMax
            Real64 Pmax = this->A42Model.MaxElecPower;
            Pstandby = 0.0;
            Pcooler = this->A42Model.PcoolDown * PLRforSubtimestepShutDown;          // could be here with part load in cool down
            TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp;          // C
            MdotCW = state.dataLoopNodes->Node(this->PlantInletNodeID).MassFlowRate; // kg/s
            ElecEff = this->A42Model.ElecEffCurve->value(state, Pmax, MdotCW, TcwIn);
            ElecEff = max(0.0, ElecEff); // protect against bad curve result
            if (ElecEff > 0.0) {         // trap divide by bad thing
                Qgross = Pmax / ElecEff; // W
            } else {
                Qgross = 0.0;
            }
            NdotFuel = Qgross / (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0);
            //  kMol/s = (J/s) /(KJ/mol * 1000 J/KJ * 1000 mol/kmol)
            Real64 MdotFuelMax = NdotFuel * state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;

            Real64 MdotFuelWarmup;
            if (Teng > thisAmbientTemp) {
                MdotFuelWarmup =
                    MdotFuelMax + this->A42Model.kf * MdotFuelMax * ((this->A42Model.TnomEngOp - thisAmbientTemp) / (Teng - thisAmbientTemp));
                // check that numerical answer didn't blow up beyond limit, and reset if it did
                if (MdotFuelWarmup > this->A42Model.Rfuelwarmup * MdotFuelMax) {
                    MdotFuelWarmup = this->A42Model.Rfuelwarmup * MdotFuelMax;
                }
            } else { // equal would divide by zero
                MdotFuelWarmup = this->A42Model.Rfuelwarmup * MdotFuelMax;
            }

            if (this->A42Model.TnomEngOp > thisAmbientTemp) {
                Pnetss = Pmax * this->A42Model.kp * ((Teng - thisAmbientTemp) / (this->A42Model.TnomEngOp - thisAmbientTemp));
            } else { // equal would divide by zero
                Pnetss = Pmax;
            }

            MdotFuel = MdotFuelWarmup;
            NdotFuel = MdotFuel / state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
            MdotAir = this->A42Model.AirFlowCurve->value(state, MdotFuelWarmup);
            MdotAir = max(0.0, MdotAir); // protect against bad curve result
            Qgross = NdotFuel * (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0);
            ThermEff = this->A42Model.ThermalEffCurve->value(state, Pmax, MdotCW, TcwIn);
            Qgenss = ThermEff * Qgross; // W
        }
    } break;
    case DataGenerators::OperatingMode::Normal: {
        if (PLRforSubtimestepStartUp < 1.0) {
            if (RunFlagElectCenter) {
                Pnetss = MyElectricLoad; // W
            }
            if (RunFlagPlant) {
                Pnetss = AllowedLoad;
            }
        } else {
            Pnetss = AllowedLoad;
        }
        Pstandby = 0.0;
        Pcooler = 0.0;
        TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp;          // C
        MdotCW = state.dataLoopNodes->Node(this->PlantInletNodeID).MassFlowRate; // kg/s
        if (this->A42Model.InternalFlowControl) {
            MdotCW = GeneratorDynamicsManager::FuncDetermineCWMdotForInternalFlowControl(state, this->DynamicsControlID, Pnetss, TcwIn);
        }

        ElecEff = this->A42Model.ElecEffCurve->value(state, Pnetss, MdotCW, TcwIn);
        ElecEff = max(0.0, ElecEff); // protect against bad curve result

        if (ElecEff > 0.0) {           // trap divide by bad thing
            Qgross = Pnetss / ElecEff; // W
        } else {
            Qgross = 0.0;
        }

        ThermEff = this->A42Model.ThermalEffCurve->value(state, Pnetss, MdotCW, TcwIn);
        ThermEff = max(0.0, ThermEff); // protect against bad curve result
        Qgenss = ThermEff * Qgross;    // W
        MdotFuel = Qgross / (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0) *
                   state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
        //  kMol/s = (J/s) /(KJ/mol * 1000 J/KJ * 1000 mol/kmol)

        bool ConstrainedIncreasingNdot(false);
        bool ConstrainedDecreasingNdot(false);
        Real64 MdotFuelAllowed = 0.0;

        GeneratorDynamicsManager::ManageGeneratorFuelFlow(
            state, this->DynamicsControlID, MdotFuel, MdotFuelAllowed, ConstrainedIncreasingNdot, ConstrainedDecreasingNdot);

        if (ConstrainedIncreasingNdot || ConstrainedDecreasingNdot) { // recalculate Pnetss with new NdotFuel with iteration
            MdotFuel = MdotFuelAllowed;
            NdotFuel = MdotFuel / state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
            Qgross = NdotFuel * (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0);

            for (int i = 1; i <= 20; ++i) { // iterating here,  could add use of search method error signal
                Pnetss = Qgross * ElecEff;
                if (this->A42Model.InternalFlowControl) {
                    MdotCW = GeneratorDynamicsManager::FuncDetermineCWMdotForInternalFlowControl(state, this->DynamicsControlID, Pnetss, TcwIn);
                }
                ElecEff = this->A42Model.ElecEffCurve->value(state, Pnetss, MdotCW, TcwIn);
                ElecEff = max(0.0, ElecEff); // protect against bad curve result
            }

            ThermEff = this->A42Model.ThermalEffCurve->value(state, Pnetss, MdotCW, TcwIn);
            ThermEff = max(0.0, ThermEff); // protect against bad curve result
            Qgenss = ThermEff * Qgross;    // W
        }

        NdotFuel = MdotFuel / state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
        MdotAir = this->A42Model.AirFlowCurve->value(state, MdotFuel);
        MdotAir = max(0.0, MdotAir); // protect against bad curve result
        if (PLRforSubtimestepStartUp < 1.0) {
            Pnetss = AllowedLoad;
        }
    } break;
    case DataGenerators::OperatingMode::CoolDown: {
        Pnetss = 0.0;
        Pstandby = 0.0;
        Pcooler = this->A42Model.PcoolDown;
        TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp;          // C
        MdotCW = state.dataLoopNodes->Node(this->PlantInletNodeID).MassFlowRate; // kg/s
        if (this->A42Model.InternalFlowControl) {
            MdotCW = GeneratorDynamicsManager::FuncDetermineCWMdotForInternalFlowControl(state, this->DynamicsControlID, Pnetss, TcwIn);
        }
        NdotFuel = 0.0;
        MdotFuel = 0.0;
        MdotAir = 0.0;
        ElecEff = 0.0;
        ThermEff = 0.0;
        Qgross = 0.0;
        Qgenss = 0.0;
    } break;
    default:
        break;
    }

    for (int i = 1; i <= 20; ++i) { // sequential search with exit criteria
        // calculate new value for engine temperature
        // for Stirling in warmup, need to include dependency of Qgness on Teng
        if ((this->A42Model.WarmUpByEngineTemp) && (CurrentOpMode == DataGenerators::OperatingMode::WarmUp)) {

            Real64 Pmax = this->A42Model.MaxElecPower;
            TcwIn = state.dataLoopNodes->Node(this->PlantInletNodeID).Temp;          // C
            MdotCW = state.dataLoopNodes->Node(this->PlantInletNodeID).MassFlowRate; // kg/s
            ElecEff = this->A42Model.ElecEffCurve->value(state, Pmax, MdotCW, TcwIn);
            ElecEff = max(0.0, ElecEff); // protect against bad curve result
            if (ElecEff > 0.0) {         // trap divide by bad thing
                Qgross = Pmax / ElecEff; // W
            } else {
                Qgross = 0.0;
            }
            NdotFuel = Qgross / (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0);
            //  kMol/s = (J/s) /(KJ/mol * 1000 J/KJ * 1000 mol/kmol)
            Real64 MdotFuelMax = NdotFuel * state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;

            Real64 MdotFuelWarmup;
            if (Teng > thisAmbientTemp) {
                MdotFuelWarmup =
                    MdotFuelMax + this->A42Model.kf * MdotFuelMax * ((this->A42Model.TnomEngOp - thisAmbientTemp) / (Teng - thisAmbientTemp));

                // check that numerical answer didn't blow up beyond limit, and reset if it did
                if (MdotFuelWarmup > this->A42Model.Rfuelwarmup * MdotFuelMax) {
                    MdotFuelWarmup = this->A42Model.Rfuelwarmup * MdotFuelMax;
                }
                if (this->A42Model.TnomEngOp > thisAmbientTemp) {
                    Pnetss = Pmax * this->A42Model.kp * ((Teng - thisAmbientTemp) / (this->A42Model.TnomEngOp - thisAmbientTemp));
                } else { // equal would divide by zero
                    Pnetss = Pmax;
                }
            } else { // equal would divide by zero
                MdotFuelWarmup = this->A42Model.Rfuelwarmup * MdotFuelMax;
            }
            MdotFuel = MdotFuelWarmup;
            NdotFuel = MdotFuel / state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
            MdotAir = this->A42Model.AirFlowCurve->value(state, MdotFuelWarmup);
            MdotAir = max(0.0, MdotAir); // protect against bad curve result
            Qgross = NdotFuel * (state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000.0 * 1000.0);
            ThermEff = this->A42Model.ThermalEffCurve->value(state, Pmax, MdotCW, TcwIn);
            ThermEff = max(0.0, ThermEff); // protect against bad curve result
            Qgenss = ThermEff * Qgross;    // W
        }

        Real64 dt = state.dataHVACGlobal->TimeStepSysSec;

        Teng = FuncDetermineEngineTemp(
            TcwOut, this->A42Model.MCeng, this->A42Model.UAhx, this->A42Model.UAskin, thisAmbientTemp, Qgenss, this->A42Model.TengLast, dt);

        Real64 Cp = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getSpecificHeat(state, TcwIn, RoutineName);

        TcwOut =
            FuncDetermineCoolantWaterExitTemp(TcwIn, this->A42Model.MCcw, this->A42Model.UAhx, MdotCW * Cp, Teng, this->A42Model.TempCWOutLast, dt);

        // form balance and exit once met.
        bool EnergyBalOK = CheckMicroCHPThermalBalance(this->A42Model.MaxElecPower,
                                                       TcwIn,
                                                       TcwOut,
                                                       Teng,
                                                       thisAmbientTemp,
                                                       this->A42Model.UAhx,
                                                       this->A42Model.UAskin,
                                                       Qgenss,
                                                       this->A42Model.MCeng,
                                                       this->A42Model.MCcw,
                                                       MdotCW * Cp);

        if (EnergyBalOK && (i > 4)) {
            break;
        }
    }

    this->PlantMassFlowRate = MdotCW;
    this->A42Model.Pnet = Pnetss - Pcooler - Pstandby;
    this->A42Model.ElecEff = ElecEff;
    this->A42Model.Qgross = Qgross;
    this->A42Model.ThermEff = ThermEff;
    this->A42Model.Qgenss = Qgenss;
    this->A42Model.NdotFuel = NdotFuel;
    this->A42Model.MdotFuel = MdotFuel;
    this->A42Model.Teng = Teng;
    this->A42Model.TcwOut = TcwOut;
    this->A42Model.TcwIn = TcwIn;
    this->A42Model.MdotAir = MdotAir;
    this->A42Model.QdotSkin = this->A42Model.UAskin * (Teng - thisAmbientTemp);

    this->A42Model.OpMode = CurrentOpMode;
}

Real64 FuncDetermineEngineTemp(Real64 const TcwOut,   // hot water leaving temp
                               Real64 const MCeng,    // Fictitious mass and heat capacity of engine
                               Real64 const UAHX,     // Heat exchanger UA
                               Real64 const UAskin,   // Skin losses UA
                               Real64 const Troom,    // surrounding zone temperature C
                               Real64 const Qgenss,   // steady state generator heat generation
                               Real64 const TengLast, // engine temp at previous time step
                               Real64 const time      // elapsed time since previous evaluation
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Feb. 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Calculate engine temperature,

    // METHODOLOGY EMPLOYED:
    // model is dynamic in that previous condition affects current timestep
    //  solve ode for engine temp using analytical solution

    Real64 a = ((UAHX * TcwOut / MCeng) + (UAskin * Troom / MCeng) + (Qgenss / MCeng));
    Real64 b = ((-1.0 * UAHX / MCeng) + (-1.0 * UAskin / MCeng));

    return (TengLast + a / b) * std::exp(b * time) - a / b;
}

Real64 FuncDetermineCoolantWaterExitTemp(Real64 const TcwIn,      // hot water inlet temp
                                         Real64 const MCcw,       // Fictitious mass and heat capacity of coolant hx
                                         Real64 const UAHX,       // Heat exchanger UA
                                         Real64 const MdotCpcw,   // mass flow and specific heat of coolant water
                                         Real64 const Teng,       // engine mass temperature C
                                         Real64 const TcwoutLast, // coolant water leaving temp at previous time step
                                         Real64 const time        // elapsed time since previous evaluation
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Feb. 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Calculate coolant water leaving temperature,

    // METHODOLOGY EMPLOYED:
    // model is dynamic in that previous condition affects current timestep
    //  solve ode for coolant water outlet temp using analytical solution

    Real64 a = (MdotCpcw * TcwIn / MCcw) + (UAHX * Teng / MCcw);
    Real64 b = ((-1.0 * MdotCpcw / MCcw) + (-1.0 * UAHX / MCcw));

    if (b * time < (-1.0 * Constant::MaxEXPArg)) {
        return -a / b;
    } else {
        return (TcwoutLast + a / b) * std::exp(b * time) - a / b;
    }
}

bool CheckMicroCHPThermalBalance(Real64 const NomHeatGen, // nominal heat generation rate for scaling
                                 Real64 const TcwIn,      // hot water inlet temp
                                 Real64 const TcwOut,     // hot water leaving temp
                                 Real64 const Teng,       // engine mass temperature C
                                 Real64 const Troom,      // surrounding zone temperature C
                                 Real64 const UAHX,       // Heat exchanger UA
                                 Real64 const UAskin,     // Skin losses UA
                                 Real64 const Qgenss,     // steady state generator heat generation
                                 Real64 const MCeng,      // Fictitious mass and heat capacity of engine
                                 Real64 const MCcw,       // Fictitious mass and heat capacity of coolant hx
                                 Real64 const MdotCpcw    // mass flow and specific heat of coolant water
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Feb. 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Check for energy balance to test if can exit iteration loop

    // METHODOLOGY EMPLOYED:
    // put all terms of dynamic energy balances on RHS and compute magnitude of imbalance
    //  compare imbalance to scalable thresholds and make a boolean conclusion.

    // first compute derivatives using a + bT
    // derivative of engine temp wrt time
    Real64 a = ((UAHX * TcwOut / MCeng) + (UAskin * Troom / MCeng) + (Qgenss / MCeng));
    Real64 b = ((-1.0 * UAHX / MCeng) + (-1.0 * UAskin / MCeng));
    Real64 DTengDTime = a + b * Teng;

    // derivative of coolant exit temp wrt time
    Real64 c = (MdotCpcw * TcwIn / MCcw) + (UAHX * Teng / MCcw);
    Real64 d = ((-1.0 * MdotCpcw / MCcw) + (-1.0 * UAHX / MCcw));
    Real64 DCoolOutTDtime = c + d * TcwOut;

    // energy imbalance for engine control volume
    Real64 magImbalEng = UAHX * (TcwOut - Teng) + UAskin * (Troom - Teng) + Qgenss - MCeng * DTengDTime;

    // energy imbalance for coolant control volume
    Real64 magImbalCooling = MdotCpcw * (TcwIn - TcwOut) + UAHX * (Teng - TcwOut) - MCcw * DCoolOutTDtime;

    // criteria for when to call energy balance okay
    Real64 threshold = NomHeatGen / 10000000.0;

    return (threshold > magImbalEng) && (threshold > magImbalCooling);
}

void FigureMicroCHPZoneGains(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Couple equipment skin losses to the Zone Heat Balance

    // METHODOLOGY EMPLOYED:
    // This routine adds up the various skin losses and then
    //  sets the values in the ZoneIntGain structure

    if (state.dataCHPElectGen->NumMicroCHPs == 0) {
        return;
    }

    if (state.dataGlobal->BeginEnvrnFlag && state.dataCHPElectGen->MyEnvrnFlag) {
        for (auto &e : state.dataGenerator->FuelSupply) {
            e.QskinLoss = 0.0;
        }
        for (auto &e : state.dataCHPElectGen->MicroCHP) {
            e.A42Model.QdotSkin = 0.0;
            e.A42Model.SkinLossConvect = 0.0;
            e.A42Model.SkinLossRadiat = 0.0;
        }
        state.dataCHPElectGen->MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataCHPElectGen->MyEnvrnFlag = true;
    }

    for (int CHPnum = 1; CHPnum <= state.dataCHPElectGen->NumMicroCHPs; ++CHPnum) {
        Real64 TotalZoneHeatGain = state.dataGenerator->FuelSupply(state.dataCHPElectGen->MicroCHP(CHPnum).FuelSupplyID).QskinLoss +
                                   state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.QdotSkin;

        state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.QdotConvZone =
            TotalZoneHeatGain * (1 - state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.RadiativeFraction);
        state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.SkinLossConvect = state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.QdotConvZone;
        state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.QdotRadZone =
            TotalZoneHeatGain * state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.RadiativeFraction;
        state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.SkinLossRadiat = state.dataCHPElectGen->MicroCHP(CHPnum).A42Model.QdotRadZone;
    }
}

void MicroCHPDataStruct::CalcUpdateHeatRecovery(EnergyPlusData &state) const
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Aug 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // update plant loop interactions, do any calcs needed

    static constexpr std::string_view RoutineName("CalcUpdateHeatRecovery");

    PlantUtilities::SafeCopyPlantNode(state, this->PlantInletNodeID, this->PlantOutletNodeID);

    state.dataLoopNodes->Node(this->PlantOutletNodeID).Temp = this->A42Model.TcwOut;

    Real64 Cp = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getSpecificHeat(state, this->A42Model.TcwIn, RoutineName);

    state.dataLoopNodes->Node(this->PlantOutletNodeID).Enthalpy = this->A42Model.TcwOut * Cp;
}

void MicroCHPDataStruct::getDesignCapacities(
    [[maybe_unused]] EnergyPlusData &state, const EnergyPlus::PlantLocation &, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
{
    MaxLoad = state.dataGenerator->GeneratorDynamics(this->DynamicsControlID).QdotHXMax;
    MinLoad = state.dataGenerator->GeneratorDynamics(this->DynamicsControlID).QdotHXMin;
    OptLoad = state.dataGenerator->GeneratorDynamics(this->DynamicsControlID).QdotHXOpt;
}

void MicroCHPDataStruct::UpdateMicroCHPGeneratorRecords(EnergyPlusData &state) // Generator number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // update variables in structures linked to output reports

    static constexpr std::string_view RoutineName("UpdateMicroCHPGeneratorRecords");

    this->A42Model.ACPowerGen = this->A42Model.Pnet;                                             // electrical power produced [W]
    this->A42Model.ACEnergyGen = this->A42Model.Pnet * state.dataHVACGlobal->TimeStepSysSec;     // energy produced (J)
    this->A42Model.QdotHX = this->A42Model.UAhx * (this->A42Model.Teng - this->A42Model.TcwOut); //  heat recovered rate (W)

    Real64 Cp = state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).glycol->getSpecificHeat(state, this->A42Model.TcwIn, RoutineName);

    this->A42Model.QdotHR = this->PlantMassFlowRate * Cp * (this->A42Model.TcwOut - this->A42Model.TcwIn);
    this->A42Model.TotalHeatEnergyRec = this->A42Model.QdotHR * state.dataHVACGlobal->TimeStepSysSec; // heat recovered energy (J)

    this->A42Model.HeatRecInletTemp = this->A42Model.TcwIn;   // Heat Recovery Loop Inlet Temperature (C)
    this->A42Model.HeatRecOutletTemp = this->A42Model.TcwOut; // Heat Recovery Loop Outlet Temperature (C)

    this->A42Model.FuelCompressPower = state.dataGenerator->FuelSupply(this->FuelSupplyID).PfuelCompEl;
    // electrical power used by fuel supply compressor [W]
    this->A42Model.FuelCompressEnergy =
        state.dataGenerator->FuelSupply(this->FuelSupplyID).PfuelCompEl * state.dataHVACGlobal->TimeStepSys * Constant::rSecsInHour; // elect energy
    this->A42Model.FuelCompressSkinLoss = state.dataGenerator->FuelSupply(this->FuelSupplyID).QskinLoss;
    // heat rate of losses.by fuel supply compressor [W]
    this->A42Model.FuelEnergyHHV = this->A42Model.NdotFuel * state.dataGenerator->FuelSupply(this->FuelSupplyID).HHV *
                                   state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec * state.dataHVACGlobal->TimeStepSys *
                                   Constant::rSecsInHour;
    // reporting: Fuel Energy used (W)
    this->A42Model.FuelEnergyUseRateHHV = this->A42Model.NdotFuel * state.dataGenerator->FuelSupply(this->FuelSupplyID).HHV *
                                          state.dataGenerator->FuelSupply(this->FuelSupplyID).KmolPerSecToKgPerSec;
    // reporting: Fuel Energy used (J)
    this->A42Model.FuelEnergyLHV =
        this->A42Model.NdotFuel * state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000000.0 * state.dataHVACGlobal->TimeStepSysSec;
    // reporting: Fuel Energy used (W)
    this->A42Model.FuelEnergyUseRateLHV = this->A42Model.NdotFuel * state.dataGenerator->FuelSupply(this->FuelSupplyID).LHV * 1000000.0;

    this->A42Model.SkinLossPower = this->A42Model.QdotConvZone + this->A42Model.QdotRadZone;
    this->A42Model.SkinLossEnergy = (this->A42Model.QdotConvZone + this->A42Model.QdotRadZone) * state.dataHVACGlobal->TimeStepSysSec;
    this->A42Model.SkinLossConvect = this->A42Model.QdotConvZone;
    this->A42Model.SkinLossRadiat = this->A42Model.QdotRadZone;

    // update node data for air inlet (and outlet)
    if (this->AirInletNodeID > 0) {
        state.dataLoopNodes->Node(this->AirInletNodeID).MassFlowRate = this->A42Model.MdotAir;
    }
    if (this->AirOutletNodeID > 0) {
        state.dataLoopNodes->Node(this->AirOutletNodeID).MassFlowRate = this->A42Model.MdotAir;
        state.dataLoopNodes->Node(this->AirOutletNodeID).Temp = this->A42Model.Teng;
    }
}
void MicroCHPDataStruct::oneTimeInit(EnergyPlusData &state)
{
    if (this->myFlag) {
        this->setupOutputVars(state);
        this->myFlag = false;
    }

    if (this->MyPlantScanFlag) {
        if (allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(
                state, this->Name, DataPlant::PlantEquipmentType::Generator_MicroCHP, this->CWPlantLoc, errFlag, _, _, _, _, _);

            if (errFlag) {
                ShowFatalError(state, "InitMicroCHPNoNormalizeGenerators: Program terminated for previous conditions.");
            }

            if (!this->A42Model.InternalFlowControl) {
                // IF this is on the supply side and not internal flow control then reset flow priority to lower
                if (this->CWPlantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply) {
                    DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::TakesWhatGets;
                }
            }

            this->MyPlantScanFlag = false;
        }
    }
}
} // namespace EnergyPlus::MicroCHPElectricGenerator
