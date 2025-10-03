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
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatPumpWaterToWaterHEATING.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::HeatPumpWaterToWaterHEATING {

// Module containing the routines dealing with the Water to Water Heat Pump (Heating)

// MODULE INFORMATION:
//       AUTHOR         ARUN
//       DATE WRITTEN   7/18/2000
//       MODIFIED       ARUN: 6/27/2002: Cycle Time
//                      L Lawrie: V1.1.1 (5/20/2003) add meters and energy to several reporting variables
//                      L Lawrie: V1.1.1 (5/20/2003) restructure modules to comply with standard templates
//                      B. Griffith, Sept 2010, plant upgrades, generalize fluid properties

// PURPOSE OF THIS MODULE:
// This module simulates a water to Water Heat Pump (Heating)

// METHODOLOGY EMPLOYED:
// This simulation is based on a set of selected parameters,
// Which are obtained using Parameter Estimation technique.

// MODULE PARAMETER DEFINITIONS
std::string const ModuleCompName("HeatPump:WaterToWater:ParameterEstimation:Heating");
std::string const ModuleCompNameUC("HEATPUMP:WATERTOWATER:PARAMETERESTIMATION:HEATING");
std::string const GSHPRefrigerant("R22");

GshpPeHeatingSpecs *GshpPeHeatingSpecs::factory(EnergyPlusData &state, const std::string &objectName)
{
    if (state.dataHPWaterToWaterHtg->GetWWHPHeatingInput) {
        GetGshpInput(state);
        state.dataHPWaterToWaterHtg->GetWWHPHeatingInput = false;
    }
    auto thisObj = std::find_if(state.dataHPWaterToWaterHtg->GSHP.begin(),
                                state.dataHPWaterToWaterHtg->GSHP.end(),
                                [&objectName](const GshpPeHeatingSpecs &myObj) { return myObj.Name == objectName; });
    if (thisObj != state.dataHPWaterToWaterHtg->GSHP.end()) {
        return thisObj;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("WWHPHeatingFactory: Error getting inputs for heat pump named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void GshpPeHeatingSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, [[maybe_unused]] bool RunFlag)
{

    // Simulate the model for the Demand "MyLoad"
    if (calledFromLocation.loopNum == this->LoadPlantLoc.loopNum) { // chilled water loop
        this->initialize(state);
        this->calculate(state, CurLoad);
        this->update(state);
    } else if (calledFromLocation.loopNum == this->SourcePlantLoc.loopNum) { // condenser loop
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            this->SourcePlantLoc.loopNum,
                                                            this->SourcePlantLoc.loopSideNum,
                                                            DataPlant::PlantEquipmentType::HPWaterEFHeating,
                                                            this->SourceSideInletNodeNum,
                                                            this->SourceSideOutletNodeNum,
                                                            -this->QSource,
                                                            this->SourceSideWaterInletTemp,
                                                            this->SourceSideWaterOutletTemp,
                                                            this->SourceSideWaterMassFlowRate,
                                                            FirstHVACIteration);
    } else {
        ShowFatalError(state, format("SimHPWatertoWaterHEATING:: Invalid loop connection {}, Requested Unit={}", ModuleCompName, this->Name));
    }
}

void GshpPeHeatingSpecs::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                             [[maybe_unused]] const PlantLocation &calledFromLocation,
                                             Real64 &MaxLoad,
                                             Real64 &MinLoad,
                                             Real64 &OptLoad)
{
    MinLoad = this->NomCap * this->MinPartLoadRat;
    MaxLoad = this->NomCap * this->MaxPartLoadRat;
    OptLoad = this->NomCap * this->OptPartLoadRat;
}

void GshpPeHeatingSpecs::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
{
    if (this->plantScanFlag) {
        // Locate the heating on the plant loops for later usage
        bool errFlag = false;
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::HPWaterPEHeating,
                                                this->SourcePlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->SourceSideInletNodeNum,
                                                _);
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::HPWaterPEHeating, this->LoadPlantLoc, errFlag, _, _, _, this->LoadSideInletNodeNum, _);
        if (errFlag) {
            ShowFatalError(state, "InitGshp: Program terminated due to previous condition(s).");
        }

        PlantUtilities::InterConnectTwoPlantLoopSides(state, this->LoadPlantLoc, this->SourcePlantLoc, this->WWHPPlantType, true);
        this->plantScanFlag = false;
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-magic-numbers"
void GetGshpInput(EnergyPlusData &state)
{
    //       SUBROUTINE INFORMATION:
    //       DATE WRITTEN:    April 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This routine will get the input
    // required by the GSHP models.  As such
    // it will interact with the Input Scanner to retrieve
    // information from the input file, count the number of
    // GSHPs and begin to fill the
    // arrays associated with the type GSHP.

    static constexpr std::string_view routineName = "GetGshpInput";
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;                // Number of elements in the alpha array
    int NumNums;                  // Number of elements in the numeric array
    int IOStat;                   // IO Status when calling get input subroutine
    Array1D_string AlphArray(5);  // character string data
    Array1D<Real64> NumArray(23); // numeric data

    bool ErrorsFound(false);

    state.dataHPWaterToWaterHtg->NumGSHPs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, ModuleCompName);

    if (state.dataHPWaterToWaterHtg->NumGSHPs <= 0) {
        ShowSevereError(state, format("{}: No Equipment found", ModuleCompName));
        ErrorsFound = true;
    }

    // Allocate Arrays
    state.dataHPWaterToWaterHtg->GSHP.allocate(state.dataHPWaterToWaterHtg->NumGSHPs);

    for (int GSHPNum = 1; GSHPNum <= state.dataHPWaterToWaterHtg->NumGSHPs; ++GSHPNum) {
        auto &thisGSHP = state.dataHPWaterToWaterHtg->GSHP(GSHPNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state, ModuleCompNameUC, GSHPNum, AlphArray, NumAlphas, NumArray, NumNums, IOStat);

        ErrorObjectHeader eoh{routineName, ModuleCompNameUC, AlphArray(1)};

        thisGSHP.Name = AlphArray(1);

        thisGSHP.WWHPPlantType = DataPlant::PlantEquipmentType::HPWaterPEHeating;

        thisGSHP.COP = NumArray(1);
        if (NumArray(1) == 0.0) {
            ShowSevereError(state, format("{}:COP = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        // zero values for NumArray 3 - 6 checked in input - idd
        thisGSHP.NomCap = NumArray(2);

        thisGSHP.MinPartLoadRat = NumArray(3);

        thisGSHP.MaxPartLoadRat = NumArray(4);

        thisGSHP.OptPartLoadRat = NumArray(5);

        thisGSHP.LoadSideVolFlowRate = NumArray(6);
        if (NumArray(6) == 0.0) {
            ShowSevereError(state, format("{}:Load Side Flow Rate = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.SourceSideVolFlowRate = NumArray(7);
        if (NumArray(7) == 0.0) {
            ShowSevereError(state, format("{}:Source Side Flow Rate = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.LoadSideUACoeff = NumArray(8);
        if (NumArray(8) == 0.0) {
            ShowSevereError(state, format("{}:Load Side Heat Transfer Coeffcient = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.SourceSideUACoeff = NumArray(9);
        if (NumArray(9) == 0.0) {
            ShowSevereError(state, format("{}:Source Side Heat Transfer Coeffcient = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.CompPistonDisp = NumArray(10);
        if (NumArray(10) == 0.0) {
            ShowSevereError(state, format("{}:Compressor Piston displacement/Storke = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.CompClearanceFactor = NumArray(11);
        if (NumArray(11) == 0.0) {
            ShowSevereError(state, format("{}:Compressor Clearance Factor = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.CompSucPressDrop = NumArray(12);
        if (NumArray(12) == 0.0) {
            ShowSevereError(state, format("{}: Pressure Drop = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.SuperheatTemp = NumArray(13);
        if (NumArray(13) == 0.0) {
            ShowSevereError(state, format("{}:Source Side SuperHeat = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.PowerLosses = NumArray(14);
        if (NumArray(14) == 0.0) {
            ShowSevereError(state, format("{}:Compressor Power Loss = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }
        thisGSHP.LossFactor = NumArray(15);
        if (NumArray(15) == 0.0) {
            ShowSevereError(state, format("{}:Efficiency = 0.0, Heatpump={}", ModuleCompName, thisGSHP.Name));
            ErrorsFound = true;
        }

        thisGSHP.HighPressCutoff = NumArray(16);
        if (NumArray(16) == 0.0) {
            thisGSHP.HighPressCutoff = 500000000.0;
        }

        thisGSHP.LowPressCutoff = NumArray(17);
        if (NumArray(17) == 0.0) {
            thisGSHP.LowPressCutoff = 0.0;
        }

        thisGSHP.SourceSideInletNodeNum = GetOnlySingleNode(state,
                                                            AlphArray(2),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatPumpWaterToWaterParameterEstimationHeating,
                                                            thisGSHP.Name,
                                                            DataLoopNode::NodeFluidType::Water,
                                                            DataLoopNode::ConnectionType::Inlet,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            DataLoopNode::ObjectIsNotParent);

        thisGSHP.SourceSideOutletNodeNum = GetOnlySingleNode(state,
                                                             AlphArray(3),
                                                             ErrorsFound,
                                                             DataLoopNode::ConnectionObjectType::HeatPumpWaterToWaterParameterEstimationHeating,
                                                             thisGSHP.Name,
                                                             DataLoopNode::NodeFluidType::Water,
                                                             DataLoopNode::ConnectionType::Outlet,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             DataLoopNode::ObjectIsNotParent);

        thisGSHP.LoadSideInletNodeNum = GetOnlySingleNode(state,
                                                          AlphArray(4),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::HeatPumpWaterToWaterParameterEstimationHeating,
                                                          thisGSHP.Name,
                                                          DataLoopNode::NodeFluidType::Water,
                                                          DataLoopNode::ConnectionType::Inlet,
                                                          NodeInputManager::CompFluidStream::Secondary,
                                                          DataLoopNode::ObjectIsNotParent);

        thisGSHP.LoadSideOutletNodeNum = GetOnlySingleNode(state,
                                                           AlphArray(5),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatPumpWaterToWaterParameterEstimationHeating,
                                                           thisGSHP.Name,
                                                           DataLoopNode::NodeFluidType::Water,
                                                           DataLoopNode::ConnectionType::Outlet,
                                                           NodeInputManager::CompFluidStream::Secondary,
                                                           DataLoopNode::ObjectIsNotParent);

        // Test node sets
        BranchNodeConnections::TestCompSet(state, ModuleCompNameUC, thisGSHP.Name, AlphArray(2), AlphArray(3), "Condenser Water Nodes");
        BranchNodeConnections::TestCompSet(state, ModuleCompNameUC, thisGSHP.Name, AlphArray(4), AlphArray(5), "Hot Water Nodes");

        // save the design source side flow rate for use by plant loop sizing algorithms
        PlantUtilities::RegisterPlantCompDesignFlow(state, thisGSHP.SourceSideInletNodeNum, 0.5 * thisGSHP.SourceSideVolFlowRate);

        if ((thisGSHP.refrig = Fluid::GetRefrig(state, GSHPRefrigerant)) == nullptr) {
            ShowSevereItemNotFound(state, eoh, "Refrigerant", GSHPRefrigerant);
            ErrorsFound = true;
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("Errors Found in getting {} Input", ModuleCompNameUC));
    }

    // CurrentModuleObject='HeatPump:WaterToWater:ParameterEstimation:Heating'
    for (int GSHPNum = 1; GSHPNum <= state.dataHPWaterToWaterHtg->NumGSHPs; ++GSHPNum) {
        auto &thisGSHP = state.dataHPWaterToWaterHtg->GSHP(GSHPNum);
        SetupOutputVariable(state,
                            "Heat Pump Electricity Rate",
                            Constant::Units::W,
                            thisGSHP.Power,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Electricity Energy",
                            Constant::Units::J,
                            thisGSHP.Energy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisGSHP.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::Heating);

        SetupOutputVariable(state,
                            "Heat Pump Load Side Heat Transfer Rate",
                            Constant::Units::W,
                            thisGSHP.QLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Load Side Heat Transfer Energy",
                            Constant::Units::J,
                            thisGSHP.QLoadEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisGSHP.Name);

        SetupOutputVariable(state,
                            "Heat Pump Source Side Heat Transfer Rate",
                            Constant::Units::W,
                            thisGSHP.QSource,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Source Side Heat Transfer Energy",
                            Constant::Units::J,
                            thisGSHP.QSourceEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisGSHP.Name);

        SetupOutputVariable(state,
                            "Heat Pump Load Side Outlet Temperature",
                            Constant::Units::C,
                            thisGSHP.LoadSideWaterOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Load Side Inlet Temperature",
                            Constant::Units::C,
                            thisGSHP.LoadSideWaterInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Source Side Outlet Temperature",
                            Constant::Units::C,
                            thisGSHP.SourceSideWaterOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Source Side Inlet Temperature",
                            Constant::Units::C,
                            thisGSHP.SourceSideWaterInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Load Side Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisGSHP.LoadSideWaterMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
        SetupOutputVariable(state,
                            "Heat Pump Source Side Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisGSHP.SourceSideWaterMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisGSHP.Name);
    }
}
#pragma clang diagnostic pop

void GshpPeHeatingSpecs::initialize(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    July 2007

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("InitGshp");

    // For each new environment
    if (state.dataGlobal->BeginEnvrnFlag && this->beginEnvironFlag) {
        this->QLoad = 0.0;
        this->QSource = 0.0;
        this->Power = 0.0;
        this->QLoadEnergy = 0.0;
        this->QSourceEnergy = 0.0;
        this->Energy = 0.0;
        this->LoadSideWaterInletTemp = 0.0;
        this->SourceSideWaterInletTemp = 0.0;
        this->LoadSideWaterOutletTemp = 0.0;
        this->SourceSideWaterOutletTemp = 0.0;
        this->SourceSideWaterMassFlowRate = 0.0;
        this->LoadSideWaterMassFlowRate = 0.0;
        this->IsOn = false;
        this->MustRun = true;

        this->beginEnvironFlag = false;
        Real64 rho = state.dataPlnt->PlantLoop(this->LoadPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
        this->LoadSideDesignMassFlow = this->LoadSideVolFlowRate * rho;

        PlantUtilities::InitComponentNodes(state, 0.0, this->LoadSideDesignMassFlow, this->LoadSideInletNodeNum, this->LoadSideOutletNodeNum);

        rho = state.dataPlnt->PlantLoop(this->SourcePlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
        this->SourceSideDesignMassFlow = this->SourceSideVolFlowRate * rho;

        PlantUtilities::InitComponentNodes(state, 0.0, this->SourceSideDesignMassFlow, this->SourceSideInletNodeNum, this->SourceSideOutletNodeNum);
        if (state.dataLoopNodes->Node(this->SourceSideOutletNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) {
            state.dataLoopNodes->Node(this->SourceSideOutletNodeNum).TempSetPoint = 0.0;
        }
        state.dataLoopNodes->Node(this->SourceSideInletNodeNum).Temp = state.dataLoopNodes->Node(this->SourceSideOutletNodeNum).TempSetPoint + 30.0;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->beginEnvironFlag = true;
    }

    // On every call
    this->Running = 0;
    this->MustRun = true;                    // Reset MustRun Flag to TRUE
    this->LoadSideWaterMassFlowRate = 0.0;   // Load Side mass flow rate, water side
    this->SourceSideWaterMassFlowRate = 0.0; // Source Side mass flow rate, water side
    this->Power = 0.0;                       // power consumption
    this->QLoad = 0.0;                       // heat rejection from Load Side coil
    this->QSource = 0.0;
}

void GshpPeHeatingSpecs::calculate(EnergyPlusData &state, Real64 &MyLoad)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR
    //       DATE WRITTEN   Sept. 1998
    //       MODIFIED       April 1999
    //                      September 2002, SJR
    //       RE-ENGINEERED  Mar2000

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 gamma(1.114); // Expansion Coefficient
    constexpr Real64 HeatBalTol(0.0005);
    constexpr Real64 RelaxParam(0.6);
    constexpr Real64 SmallNum(1.0e-20);
    constexpr int IterationLimit(500);
    constexpr const char *RoutineName("CalcGshpModel");
    constexpr const char *RoutineNameLoadSideTemp("CalcGSHPModel:LoadSideTemp");
    constexpr const char *RoutineNameSourceSideTemp("CalcGSHPModel:SourceSideTemp");
    constexpr const char *RoutineNameCompressInletTemp("CalcGSHPModel:CompressInletTemp");
    constexpr const char *RoutineNameSuctionPr("CalcGSHPModel:SuctionPr");
    constexpr const char *RoutineNameCompSuctionTemp("CalcGSHPModel:CompSuctionTemp");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 CompSuctionTemp;
    Real64 CompSuctionEnth;
    Real64 CompSuctionDensity;
    Real64 CompSuctionSatTemp;
    Real64 DutyFactor;

    if (MyLoad > 0.0) {
        this->MustRun = true;
        this->IsOn = true;
    } else {
        this->MustRun = false;
        this->IsOn = false;
    }
    this->LoadSideWaterInletTemp = state.dataLoopNodes->Node(this->LoadSideInletNodeNum).Temp;
    this->SourceSideWaterInletTemp = state.dataLoopNodes->Node(this->SourceSideInletNodeNum).Temp;

    //*******Set flow based on "run" flags**********
    // Set flows if the heat pump is not running
    if (!this->MustRun) {
        this->LoadSideWaterMassFlowRate = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, this->LoadSideWaterMassFlowRate, this->LoadSideInletNodeNum, this->LoadSideOutletNodeNum, this->LoadPlantLoc);
        this->SourceSideWaterMassFlowRate = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, this->SourceSideWaterMassFlowRate, this->SourceSideInletNodeNum, this->SourceSideOutletNodeNum, this->SourcePlantLoc);
        PlantUtilities::PullCompInterconnectTrigger(state,
                                                    this->LoadPlantLoc,
                                                    this->CondMassFlowIndex,
                                                    this->SourcePlantLoc,
                                                    DataPlant::CriteriaType::MassFlowRate,
                                                    this->SourceSideWaterMassFlowRate);
        // now initialize simulation variables for "heat pump off"
        this->LoadSideWaterOutletTemp = this->LoadSideWaterInletTemp;
        this->SourceSideWaterOutletTemp = this->SourceSideWaterInletTemp;
        return; // if heat pump is not running return without simulation, power, Q already zeroed in init
    } else {    // the heat pump must run, request design flow

        this->LoadSideWaterMassFlowRate = this->LoadSideDesignMassFlow;
        PlantUtilities::SetComponentFlowRate(
            state, this->LoadSideWaterMassFlowRate, this->LoadSideInletNodeNum, this->LoadSideOutletNodeNum, this->LoadPlantLoc);

        this->SourceSideWaterMassFlowRate = this->SourceSideDesignMassFlow;
        PlantUtilities::SetComponentFlowRate(
            state, this->SourceSideWaterMassFlowRate, this->SourceSideInletNodeNum, this->SourceSideOutletNodeNum, this->SourcePlantLoc);
        // if there's no flow, turn the "heat pump off"
        if (this->LoadSideWaterMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance ||
            this->SourceSideWaterMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
            this->LoadSideWaterMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, this->LoadSideWaterMassFlowRate, this->LoadSideInletNodeNum, this->LoadSideOutletNodeNum, this->LoadPlantLoc);
            this->SourceSideWaterMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, this->SourceSideWaterMassFlowRate, this->SourceSideInletNodeNum, this->SourceSideOutletNodeNum, this->SourcePlantLoc);
            PlantUtilities::PullCompInterconnectTrigger(state,
                                                        this->LoadPlantLoc,
                                                        this->CondMassFlowIndex,
                                                        this->SourcePlantLoc,
                                                        DataPlant::CriteriaType::MassFlowRate,
                                                        this->SourceSideWaterMassFlowRate);
            this->LoadSideWaterOutletTemp = this->LoadSideWaterInletTemp;
            this->SourceSideWaterOutletTemp = this->SourceSideWaterInletTemp;
            return;
        }
        PlantUtilities::PullCompInterconnectTrigger(state,
                                                    this->LoadPlantLoc,
                                                    this->CondMassFlowIndex,
                                                    this->SourcePlantLoc,
                                                    DataPlant::CriteriaType::MassFlowRate,
                                                    this->SourceSideWaterMassFlowRate);
    }

    //***********BEGIN CALCULATION****************
    // initialize the source and load side heat transfer rates for the simulation
    Real64 initialQSource = 0.0;
    Real64 initialQLoad = 0.0;
    int IterationCount = 0;

    Real64 CpSourceSide =
        state.dataPlnt->PlantLoop(this->SourcePlantLoc.loopNum).glycol->getSpecificHeat(state, this->SourceSideWaterInletTemp, RoutineName);

    Real64 CpLoadSide =
        state.dataPlnt->PlantLoop(this->LoadPlantLoc.loopNum).glycol->getSpecificHeat(state, this->LoadSideWaterInletTemp, RoutineName);

    // Determine effectiveness of Source Side (the Evaporator in heating mode)
    Real64 SourceSideEffect = 1.0 - std::exp(-this->SourceSideUACoeff / (CpSourceSide * this->SourceSideWaterMassFlowRate));
    // Determine effectiveness of Load Side the condenser in heating mode
    Real64 LoadSideEffect = 1.0 - std::exp(-this->LoadSideUACoeff / (CpLoadSide * this->LoadSideWaterMassFlowRate));

    while (true) { // main loop to solve model equations
        ++IterationCount;
        // Determine Source Side temperature
        Real64 SourceSideTemp =
            this->SourceSideWaterInletTemp - initialQSource / (SourceSideEffect * CpSourceSide * this->SourceSideWaterMassFlowRate);

        // To determine Load Side temperature condenser
        Real64 LoadSideTemp = this->LoadSideWaterInletTemp + initialQLoad / (LoadSideEffect * CpLoadSide * this->LoadSideWaterMassFlowRate);

        // Determine the evaporating and condensing pressures
        Real64 SourceSidePressure = this->refrig->getSatPressure(state, SourceSideTemp, RoutineNameSourceSideTemp);
        Real64 LoadSidePressure = this->refrig->getSatPressure(state, LoadSideTemp, RoutineNameLoadSideTemp);

        // check cutoff pressures
        if (SourceSidePressure < this->LowPressCutoff) {
            ShowSevereError(state, format("{}=\"{}\" Heating Source Side Pressure Less than the Design Minimum", ModuleCompName, this->Name));
            ShowContinueError(
                state,
                format("Source Side Pressure={:.2T} and user specified Design Minimum Pressure={:.2T}", SourceSidePressure, this->LowPressCutoff));
            ShowFatalError(state, "Preceding Conditions cause termination.");
        }
        if (LoadSidePressure > this->HighPressCutoff) {
            ShowSevereError(state, format("{}=\"{}\" Heating Load Side Pressure greater than the Design Maximum", ModuleCompName, this->Name));
            ShowContinueError(
                state,
                format("Load Side Pressure={:.2T} and user specified Design Maximum Pressure={:.2T}", LoadSidePressure, this->HighPressCutoff));
            ShowFatalError(state, "Preceding Conditions cause termination.");
        }

        // Determine Suction Pressure at compressor inlet
        Real64 SuctionPr = SourceSidePressure - this->CompSucPressDrop;
        // Determine Discharge Pressure at compressor exit
        Real64 DischargePr = LoadSidePressure + this->CompSucPressDrop;
        // check cutoff pressures
        if (SuctionPr < this->LowPressCutoff) {
            ShowSevereError(state, format("{}=\"{}\" Heating Suction Pressure Less than the Design Minimum", ModuleCompName, this->Name));
            ShowContinueError(
                state, format("Heating Suction Pressure={:.2T} and user specified Design Minimum Pressure={:.2T}", SuctionPr, this->LowPressCutoff));
            ShowFatalError(state, "Preceding Conditions cause termination.");
        }
        if (DischargePr > this->HighPressCutoff) {
            ShowSevereError(state, format("{}=\"{}\" Heating Discharge Pressure greater than the Design Maximum", ModuleCompName, this->Name));
            ShowContinueError(
                state,
                format("Heating Discharge Pressure={:.2T} and user specified Design Maximum Pressure={:.2T}", DischargePr, this->HighPressCutoff));
            ShowFatalError(state, "Preceding Conditions cause termination.");
        }

        // Determine the Source Side Outlet Enthalpy
        Real64 qualOne = 1.0;
        Real64 SourceSideOutletEnth = this->refrig->getSatEnthalpy(state, SourceSideTemp, qualOne, RoutineNameSourceSideTemp);

        // Determine Load Side Outlet Enthalpy
        Real64 qualZero = 0.0;
        Real64 LoadSideOutletEnth = this->refrig->getSatEnthalpy(state, LoadSideTemp, qualZero, RoutineNameLoadSideTemp);

        // Determine superheated temperature of the Source Side outlet/compressor inlet
        Real64 CompressInletTemp = SourceSideTemp + this->SuperheatTemp;
        // Determine the enathalpy of the super heated fluid at Source Side outlet
        Real64 SuperHeatEnth = this->refrig->getSupHeatEnthalpy(state, CompressInletTemp, SourceSidePressure, RoutineNameCompressInletTemp);

        // Determining the suction state of the fluid from inlet state involves interation
        // Method employed...
        // Determine the saturated temp at suction pressure, shoot out into the superheated region find the enthalpy
        // check that with the inlet enthalpy ( as suction loss is isenthalpic). Iterate till desired accuracy is reached

        CompSuctionSatTemp = this->refrig->getSatTemperature(state, SuctionPr, RoutineNameSuctionPr);

        Real64 T110 = CompSuctionSatTemp;
        // Shoot into the super heated region
        Real64 T111 = CompSuctionSatTemp + 80;

        // Iterate to find the Suction State - given suction pressure and superheat enthalpy
        while (true) {
            CompSuctionTemp = 0.5 * (T110 + T111);

            CompSuctionEnth = this->refrig->getSupHeatEnthalpy(state, CompSuctionTemp, SuctionPr, RoutineNameCompSuctionTemp);
            if (std::abs(CompSuctionEnth - SuperHeatEnth) / SuperHeatEnth < 0.0001) {
                break;
            }

            if (CompSuctionEnth < SuperHeatEnth) {
                T110 = CompSuctionTemp;
            } else {
                T111 = CompSuctionTemp;
            }
        }

        // Determine the Mass flow rate of refrigerant
        CompSuctionDensity = this->refrig->getSupHeatDensity(state, CompSuctionTemp, SuctionPr, RoutineNameCompSuctionTemp);
        Real64 MassRef = this->CompPistonDisp * CompSuctionDensity *
                         (1.0 + this->CompClearanceFactor - this->CompClearanceFactor * std::pow(DischargePr / SuctionPr, 1.0 / gamma));

        // Find the  Source Side Heat Transfer
        this->QSource = MassRef * (SourceSideOutletEnth - LoadSideOutletEnth);

        // Determine the theoretical power
        this->Power = this->PowerLosses + (MassRef * gamma / (gamma - 1) * SuctionPr / CompSuctionDensity / this->LossFactor *
                                           (std::pow(DischargePr / SuctionPr, (gamma - 1) / gamma) - 1));

        // Determine the Loadside HeatRate (QLoad)
        this->QLoad = this->Power + this->QSource;

        // convergence and iteration limit check
        if (std::abs((this->QLoad - initialQLoad) / (initialQLoad + SmallNum)) < HeatBalTol || IterationCount > IterationLimit) {
            if (IterationCount > IterationLimit) {
                ShowWarningError(state, format("{} did not converge", ModuleCompName));
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Heatpump Name = {}", this->Name));
                ShowContinueError(
                    state,
                    format("Heat Inbalance (%)             = {:G}", std::abs(100.0 * (this->QLoad - initialQLoad) / (initialQLoad + SmallNum))));
                ShowContinueError(state, format("Load-side heat transfer rate   = {:G}", this->QLoad));
                ShowContinueError(state, format("Source-side heat transfer rate = {:G}", this->QSource));
                ShowContinueError(state, format("Source-side mass flow rate     = {:G}", this->SourceSideWaterMassFlowRate));
                ShowContinueError(state, format("Load-side mass flow rate       = {:G}", this->LoadSideWaterMassFlowRate));
                ShowContinueError(state, format("Source-side inlet temperature  = {:G}", this->SourceSideWaterInletTemp));
                ShowContinueError(state, format("Load-side inlet temperature    = {:G}", this->LoadSideWaterInletTemp));
            }
            goto LOOPLoadEnth_exit;

        } else { // update load
            initialQLoad += RelaxParam * (this->QLoad - initialQLoad);
            initialQSource += RelaxParam * (this->QSource - initialQSource);
        }
    }
LOOPLoadEnth_exit:;

    // Control Strategy
    if (std::abs(MyLoad) < this->QLoad) {
        DutyFactor = std::abs(MyLoad) / this->QLoad;
        this->QLoad = std::abs(MyLoad);
        this->Power *= DutyFactor;
        this->QSource *= DutyFactor;

        // Determine the Exterior fluid temperature at the Load Side oulet and eveporator outlet...
        // Refrigerant = "Steam"
        this->LoadSideWaterOutletTemp = this->LoadSideWaterInletTemp + this->QLoad / (this->LoadSideWaterMassFlowRate * CpLoadSide);
        this->SourceSideWaterOutletTemp = this->SourceSideWaterInletTemp - this->QSource / (this->SourceSideWaterMassFlowRate * CpSourceSide);
        return;
    }

    this->LoadSideWaterOutletTemp = this->LoadSideWaterInletTemp + this->QLoad / (this->LoadSideWaterMassFlowRate * CpLoadSide);
    this->SourceSideWaterOutletTemp = this->SourceSideWaterInletTemp - this->QSource / (this->SourceSideWaterMassFlowRate * CpSourceSide);
    // REPORT VAR
    this->Running = 1;
}

void GshpPeHeatingSpecs::update(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    October 1998

    if (!this->MustRun) {
        // set node temperatures
        state.dataLoopNodes->Node(this->SourceSideOutletNodeNum).Temp = state.dataLoopNodes->Node(this->SourceSideInletNodeNum).Temp;
        state.dataLoopNodes->Node(this->LoadSideOutletNodeNum).Temp = state.dataLoopNodes->Node(this->LoadSideInletNodeNum).Temp;
        this->Power = 0.0;
        this->Energy = 0.0;
        this->QSource = 0.0;
        this->QLoad = 0.0;
        this->QSourceEnergy = 0.0;
        this->QLoadEnergy = 0.0;
        this->SourceSideWaterInletTemp = state.dataLoopNodes->Node(this->SourceSideInletNodeNum).Temp;
        this->SourceSideWaterOutletTemp = state.dataLoopNodes->Node(this->SourceSideOutletNodeNum).Temp;
        this->LoadSideWaterInletTemp = state.dataLoopNodes->Node(this->LoadSideInletNodeNum).Temp;
        this->LoadSideWaterOutletTemp = state.dataLoopNodes->Node(this->LoadSideOutletNodeNum).Temp;

    } else {
        // set node temperatures
        state.dataLoopNodes->Node(this->LoadSideOutletNodeNum).Temp = this->LoadSideWaterOutletTemp;
        state.dataLoopNodes->Node(this->SourceSideOutletNodeNum).Temp = this->SourceSideWaterOutletTemp;

        // set node flow rates;  for these load based models
        // assume that the sufficient Source Side flow rate available

        Real64 const ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        this->Energy = this->Power * ReportingConstant;
        this->QSourceEnergy = QSource * ReportingConstant;
        this->QLoadEnergy = QLoad * ReportingConstant;
    }
}
void GshpPeHeatingSpecs::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}
void GshpPeHeatingSpecs::oneTimeInit_new([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::HeatPumpWaterToWaterHEATING
