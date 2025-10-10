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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChillerGasAbsorption.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::ChillerGasAbsorption {

// MODULE INFORMATION:
//    AUTHOR         Jason Glazer of GARD Analytics, Inc.
//                   for Gas Research Institute
//    DATE WRITTEN   March 2001
//    MODIFIED       Brent Griffith, Nov 2010 plant upgrades, generalize fluid properties

// PURPOSE OF THIS MODULE:
//    This module simulates the performance of the direct fired
//    absorption chiller.
// METHODOLOGY EMPLOYED:
//    Once the PlantLoopManager determines that the absorber chiller
//    is available to meet a loop cooling demand, it calls SimGasAbsorption
//    which in turn calls the appropriate Absorption Chiller model.
// REFERENCES:
//    DOE-2.1e Supplement
//    PG&E CoolTools GasMod
// OTHER NOTES:
//    The curves on this model follow the DOE-2 approach of using
//    electric and heat input ratios.  In addition, the temperature
//    correction curve has two independent variables for the
//    chilled water temperature and either the entering or leaving
//    condenser water temperature.
//    The code was originally adopted from the ChillerAbsorption
//    routine but has been extensively modified.
//    Development of this module was funded by the Gas Research Institute.
//    (Please see copyright and disclaimer information at end of module)

GasAbsorberSpecs *GasAbsorberSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    // Process the input data if it hasn't been done already
    if (state.dataChillerGasAbsorption->getGasAbsorberInputs) {
        GetGasAbsorberInput(state);
        state.dataChillerGasAbsorption->getGasAbsorberInputs = false;
    }
    // Now look for this particular pipe in the list
    auto thisObj = std::find_if(state.dataChillerGasAbsorption->GasAbsorber.begin(),
                                state.dataChillerGasAbsorption->GasAbsorber.end(),
                                [&objectName](const GasAbsorberSpecs &myObj) { return myObj.Name == objectName; });
    if (thisObj != state.dataChillerGasAbsorption->GasAbsorber.end()) {
        return thisObj;
    }
    // If we didn't find it, fatal
    ShowFatalError(state, format("LocalGasAbsorberFactory: Error getting inputs for comp named: {}", objectName)); // LCOV_EXCL_LINE
    // Shut up the compiler
    return nullptr; // LCOV_EXCL_LINE
}

void GasAbsorberSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
{
    DataPlant::BrLoopType brIdentity(DataPlant::BrLoopType::NoMatch);

    int branchTotalComp = calledFromLocation.branch->TotalComponents;

    for (int iComp = 1; iComp <= branchTotalComp; iComp++) {
        // kind of a hacky way to find the location of this, but it's what plantloopequip was doing
        int compInletNodeNum = calledFromLocation.branch->Comp(iComp).NodeNumIn;
        // Match inlet node name of calling branch to determine if this call is for heating or cooling
        if (compInletNodeNum == this->ChillReturnNodeNum) { // Operate as chiller
            brIdentity = DataPlant::BrLoopType::Chiller;
            break;
        } else if (compInletNodeNum == this->HeatReturnNodeNum) { // Operate as heater
            brIdentity = DataPlant::BrLoopType::Heater;
            break;
        } else if (compInletNodeNum == this->CondReturnNodeNum) { // called from condenser loop
            brIdentity = DataPlant::BrLoopType::Condenser;
            break;
        } else {
            brIdentity = DataPlant::BrLoopType::NoMatch;
        }
    }

    if (brIdentity == DataPlant::BrLoopType::Chiller) {
        // Calculate Node Values
        // Calculate Equipment and Update Variables
        this->InCoolingMode = RunFlag != 0;
        this->initialize(state);
        this->calculateChiller(state, CurLoad);
        this->updateCoolRecords(state, CurLoad, RunFlag);
    } else if (brIdentity == DataPlant::BrLoopType::Heater) {
        // Calculate Node Values
        // Calculate Equipment and Update Variables
        this->InHeatingMode = RunFlag != 0;
        this->initialize(state);
        this->calculateHeater(state, CurLoad, RunFlag);
        this->updateHeatRecords(state, CurLoad, RunFlag);
    } else if (brIdentity == DataPlant::BrLoopType::Condenser) {
        if (this->CDplantLoc.loopNum > 0) {
            PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                                this->CDplantLoc.loopNum,
                                                                this->CDplantLoc.loopSideNum,
                                                                DataPlant::PlantEquipmentType::Chiller_DFAbsorption,
                                                                this->CondReturnNodeNum,
                                                                this->CondSupplyNodeNum,
                                                                this->TowerLoad,
                                                                this->CondReturnTemp,
                                                                this->CondSupplyTemp,
                                                                this->CondWaterFlowRate,
                                                                FirstHVACIteration);
        }
    } else {
        // Error, nodes do not match
        ShowSevereError(state, format("Invalid call to Gas Absorber Chiller {}", this->Name));
        ShowContinueError(state, "Node connections in branch are not consistent with object nodes.");
        ShowFatalError(state, "Preceding conditions cause termination.");
    }
}

void GasAbsorberSpecs::getDesignCapacities(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
{
    bool matchfound = false;

    int branchTotalComp = calledFromLocation.branch->TotalComponents;

    for (int iComp = 1; iComp <= branchTotalComp; iComp++) {
        // kind of a hacky way to find the location of this, but it's what plantloopequip was doing
        int compInletNodeNum = calledFromLocation.branch->Comp(iComp).NodeNumIn;

        if (compInletNodeNum == this->ChillReturnNodeNum) { // Operate as chiller
            MinLoad = this->NomCoolingCap * this->MinPartLoadRat;
            MaxLoad = this->NomCoolingCap * this->MaxPartLoadRat;
            OptLoad = this->NomCoolingCap * this->OptPartLoadRat;
            matchfound = true;
            break;
        } else if (compInletNodeNum == this->HeatReturnNodeNum) { // Operate as heater
            Real64 Sim_HeatCap = this->NomCoolingCap * this->NomHeatCoolRatio;
            MinLoad = Sim_HeatCap * this->MinPartLoadRat;
            MaxLoad = Sim_HeatCap * this->MaxPartLoadRat;
            OptLoad = Sim_HeatCap * this->OptPartLoadRat;
            matchfound = true;
            break;
        } else if (compInletNodeNum == this->CondReturnNodeNum) { // called from condenser loop
            MinLoad = 0.0;
            MaxLoad = 0.0;
            OptLoad = 0.0;
            matchfound = true;
            break;
        } else {
            matchfound = false;
        }
    }

    if (!matchfound) {
        // Error, nodes do not match
        ShowSevereError(state, format("SimGasAbsorber: Invalid call to Gas Absorbtion Chiller-Heater {}", this->Name));
        ShowContinueError(state, "Node connections in branch are not consistent with object nodes.");
        ShowFatalError(state, "Preceding conditions cause termination.");
    } // Operate as Chiller or Heater
}

void GasAbsorberSpecs::getSizingFactor(Real64 &_SizFac)
{
    _SizFac = this->SizFac;
}

void GasAbsorberSpecs::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation)
{
    this->initialize(state);

    // kind of a hacky way to find the location of this, but it's what plantloopequip was doing
    int BranchInletNodeNum = calledFromLocation.branch->NodeNumIn;

    if (BranchInletNodeNum == this->ChillReturnNodeNum) {       // Operate as chiller
        this->size(state);                                      // only call from chilled water loop
    } else if (BranchInletNodeNum == this->HeatReturnNodeNum) { // Operate as heater
        // don't do anything here
    } else if (BranchInletNodeNum == this->CondReturnNodeNum) { // called from condenser loop
                                                                // don't do anything here
    } else {                                                    // Error, nodes do not match
        ShowSevereError(state, format("SimGasAbsorber: Invalid call to Gas Absorbtion Chiller-Heater {}", this->Name));
        ShowContinueError(state, "Node connections in branch are not consistent with object nodes.");
        ShowFatalError(state, "Preceding conditions cause termination.");
    } // Operate as Chiller or Heater
}

void GasAbsorberSpecs::getDesignTemperatures(Real64 &TempCondInDesign, Real64 &TempEvapOutDesign)
{
    TempEvapOutDesign = this->TempDesCHWSupply;
    TempCondInDesign = this->TempDesCondReturn;
}

void GetGasAbsorberInput(EnergyPlusData &state)
{
    //       AUTHOR:          Jason Glazer
    //       DATE WRITTEN:    March 2001
    // This routine will get the input
    // required by the Direct Fired Absorption chiller model in the object ChillerHeater:Absorption:DirectFired

    static constexpr std::string_view routineName = "GetGasAbsorptionInput";

    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool Okay;
    bool Get_ErrorsFound(false);
    int NumGasAbsorbers(0); // number of Absorption Chillers specified in input

    auto &s_ipsc = state.dataIPShortCut;

    s_ipsc->cCurrentModuleObject = "ChillerHeater:Absorption:DirectFired";
    NumGasAbsorbers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (NumGasAbsorbers <= 0) {
        ShowSevereError(state, format("No {} equipment found in input file", s_ipsc->cCurrentModuleObject));
        Get_ErrorsFound = true;
    }

    if (allocated(state.dataChillerGasAbsorption->GasAbsorber)) {
        return;
    }

    // ALLOCATE ARRAYS
    state.dataChillerGasAbsorption->GasAbsorber.allocate(NumGasAbsorbers);

    // LOAD ARRAYS

    for (int AbsorberNum = 1; AbsorberNum <= NumGasAbsorbers; ++AbsorberNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 AbsorberNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        // Get_ErrorsFound will be set to True if problem was found, left untouched otherwise
        GlobalNames::VerifyUniqueChillerName(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), Get_ErrorsFound, s_ipsc->cCurrentModuleObject + " Name");

        auto &thisChiller = state.dataChillerGasAbsorption->GasAbsorber(AbsorberNum);
        thisChiller.Name = s_ipsc->cAlphaArgs(1);
        std::string ChillerName = s_ipsc->cCurrentModuleObject + " Named " + thisChiller.Name;

        // Assign capacities
        thisChiller.NomCoolingCap = s_ipsc->rNumericArgs(1);
        if (thisChiller.NomCoolingCap == DataSizing::AutoSize) {
            thisChiller.NomCoolingCapWasAutoSized = true;
        }
        thisChiller.NomHeatCoolRatio = s_ipsc->rNumericArgs(2);
        // Assign efficiencies
        thisChiller.FuelCoolRatio = s_ipsc->rNumericArgs(3);
        thisChiller.FuelHeatRatio = s_ipsc->rNumericArgs(4);
        thisChiller.ElecCoolRatio = s_ipsc->rNumericArgs(5);
        thisChiller.ElecHeatRatio = s_ipsc->rNumericArgs(6);

        // Assign Node Numbers to specified nodes
        thisChiller.ChillReturnNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                             s_ipsc->cAlphaArgs(2),
                                                                             Get_ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                                             s_ipsc->cAlphaArgs(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Inlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
        thisChiller.ChillSupplyNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                             s_ipsc->cAlphaArgs(3),
                                                                             Get_ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                                             s_ipsc->cAlphaArgs(1),
                                                                             DataLoopNode::NodeFluidType::Water,
                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(2), s_ipsc->cAlphaArgs(3), "Chilled Water Nodes");
        // Condenser node processing depends on condenser type, see below
        thisChiller.HeatReturnNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(6),
                                                                            Get_ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                            NodeInputManager::CompFluidStream::Tertiary,
                                                                            DataLoopNode::ObjectIsNotParent);
        thisChiller.HeatSupplyNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            s_ipsc->cAlphaArgs(7),
                                                                            Get_ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                                            s_ipsc->cAlphaArgs(1),
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Tertiary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(6), s_ipsc->cAlphaArgs(7), "Hot Water Nodes");
        if (Get_ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing node input for {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            Get_ErrorsFound = false;
        }

        // Assign Part Load Ratios
        thisChiller.MinPartLoadRat = s_ipsc->rNumericArgs(7);
        thisChiller.MaxPartLoadRat = s_ipsc->rNumericArgs(8);
        thisChiller.OptPartLoadRat = s_ipsc->rNumericArgs(9);
        // Assign Design Conditions
        thisChiller.TempDesCondReturn = s_ipsc->rNumericArgs(10);
        thisChiller.TempDesCHWSupply = s_ipsc->rNumericArgs(11);
        thisChiller.EvapVolFlowRate = s_ipsc->rNumericArgs(12);
        if (thisChiller.EvapVolFlowRate == DataSizing::AutoSize) {
            thisChiller.EvapVolFlowRateWasAutoSized = true;
        }
        if (Util::SameString(s_ipsc->cAlphaArgs(16), "AirCooled")) {
            thisChiller.CondVolFlowRate = 0.0011; // Condenser flow rate not used for this cond type
        } else {
            thisChiller.CondVolFlowRate = s_ipsc->rNumericArgs(13);
            if (thisChiller.CondVolFlowRate == DataSizing::AutoSize) {
                thisChiller.CondVolFlowRateWasAutoSized = true;
            }
        }
        thisChiller.HeatVolFlowRate = s_ipsc->rNumericArgs(14);
        if (thisChiller.HeatVolFlowRate == DataSizing::AutoSize) {
            thisChiller.HeatVolFlowRateWasAutoSized = true;
        }
        // Assign Curve Numbers
        if (s_ipsc->lAlphaFieldBlanks(8)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(8));
            Get_ErrorsFound = true;
        } else if ((thisChiller.CoolCapFTCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(8))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(8), s_ipsc->cAlphaArgs(8));
            Get_ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(9)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(9));
            Get_ErrorsFound = true;
        } else if ((thisChiller.FuelCoolFTCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(9))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(9), s_ipsc->cAlphaArgs(9));
            Get_ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(10)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(10));
            Get_ErrorsFound = true;
        } else if ((thisChiller.FuelCoolFPLRCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(10))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(10), s_ipsc->cAlphaArgs(10));
            Get_ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(11)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(11));
            Get_ErrorsFound = true;
        } else if ((thisChiller.ElecCoolFTCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(11))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(11), s_ipsc->cAlphaArgs(11));
            Get_ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(12)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(12));
            Get_ErrorsFound = true;
        } else if ((thisChiller.ElecCoolFPLRCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(12))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(12), s_ipsc->cAlphaArgs(12));
            Get_ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(13)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(13));
            Get_ErrorsFound = true;
        } else if ((thisChiller.HeatCapFCoolCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(13))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(13), s_ipsc->cAlphaArgs(13));
            Get_ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(14)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(14));
            Get_ErrorsFound = true;
        } else if ((thisChiller.FuelHeatFHPLRCurve = Curve::GetCurve(state, s_ipsc->cAlphaArgs(14))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(14), s_ipsc->cAlphaArgs(14));
            Get_ErrorsFound = true;
        }

        if (Get_ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing curve input for {}={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            Get_ErrorsFound = false;
        }

        if (Util::SameString(s_ipsc->cAlphaArgs(15), "LeavingCondenser")) {
            thisChiller.isEnterCondensTemp = false;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(15), "EnteringCondenser")) {
            thisChiller.isEnterCondensTemp = true;
        } else {
            thisChiller.isEnterCondensTemp = true;
            ShowWarningError(state, format("{}=\"{}\", invalid value", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}=\"{}\"", s_ipsc->cAlphaFieldNames(15), s_ipsc->cAlphaArgs(15)));
            ShowContinueError(state, "resetting to EnteringCondenser, simulation continues");
        }
        // Assign Other Parameters
        if (Util::SameString(s_ipsc->cAlphaArgs(16), "AirCooled")) {
            thisChiller.isWaterCooled = false;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(16), "WaterCooled")) {
            thisChiller.isWaterCooled = true;
        } else {
            thisChiller.isWaterCooled = true;
            ShowWarningError(state, format("{}=\"{}\", invalid value", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(16), s_ipsc->cAlphaArgs(16)));
            ShowContinueError(state, "resetting to WaterCooled, simulation continues");
        }
        if (!thisChiller.isEnterCondensTemp && !thisChiller.isWaterCooled) {
            thisChiller.isEnterCondensTemp = true;
            ShowWarningError(state, format("{}=\"{}\", invalid value", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, "Invalid to have both LeavingCondenser and AirCooled.");
            ShowContinueError(state, "resetting to EnteringCondenser, simulation continues");
        }
        if (thisChiller.isWaterCooled) {
            if (s_ipsc->lAlphaFieldBlanks(5)) {
                ShowSevereError(state, format("{}=\"{}\", invalid value", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "For WaterCooled chiller the condenser outlet node is required.");
                Get_ErrorsFound = true;
            }
            thisChiller.CondReturnNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_ipsc->cAlphaArgs(4),
                                                    Get_ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                    s_ipsc->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
            thisChiller.CondSupplyNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_ipsc->cAlphaArgs(5),
                                                    Get_ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                    s_ipsc->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(
                state, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(4), s_ipsc->cAlphaArgs(5), "Condenser Water Nodes");
        } else {
            thisChiller.CondReturnNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    s_ipsc->cAlphaArgs(4),
                                                    Get_ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ChillerHeaterAbsorptionDirectFired,
                                                    s_ipsc->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::OutsideAirReference,
                                                    NodeInputManager::CompFluidStream::Secondary,
                                                    DataLoopNode::ObjectIsNotParent);
            // Condenser outlet node not used for air or evap cooled condenser so ignore cAlphaArgs( 5 )
            // Connection not required for air or evap cooled condenser so no call to TestCompSet here
            OutAirNodeManager::CheckAndAddAirNodeNumber(state, thisChiller.CondReturnNodeNum, Okay);
            if (!Okay) {
                ShowWarningError(state, format("{}, Adding OutdoorAir:Node={}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(4)));
            }
        }
        thisChiller.CHWLowLimitTemp = s_ipsc->rNumericArgs(15);
        thisChiller.FuelHeatingValue = s_ipsc->rNumericArgs(16);
        thisChiller.SizFac = s_ipsc->rNumericArgs(17);

        // Validate fuel type input
        thisChiller.FuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, s_ipsc->cAlphaArgs(17)));
        if (thisChiller.FuelType == Constant::eFuel::Invalid) {
            ShowSevereError(state, format("{}=\"{}\", invalid value", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("Invalid {}={}", s_ipsc->cAlphaFieldNames(17), s_ipsc->cAlphaArgs(17)));
            ShowContinueError(
                state, "Valid choices are Electricity, NaturalGas, Propane, Diesel, Gasoline, FuelOilNo1, FuelOilNo2,OtherFuel1 or OtherFuel2");
            Get_ErrorsFound = true;
        }
    }

    if (Get_ErrorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", s_ipsc->cCurrentModuleObject));
    }
}

void GasAbsorberSpecs::setupOutputVariables(EnergyPlusData &state)
{
    std::string_view const sFuelType = Constant::eFuelNames[static_cast<int>(this->FuelType)];

    SetupOutputVariable(state,
                        "Chiller Heater Evaporator Cooling Rate",
                        Constant::Units::W,
                        this->CoolingLoad,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Evaporator Cooling Energy",
                        Constant::Units::J,
                        this->CoolingEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Chillers);

    SetupOutputVariable(state,
                        "Chiller Heater Heating Rate",
                        Constant::Units::W,
                        this->HeatingLoad,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Heating Energy",
                        Constant::Units::J,
                        this->HeatingEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Boilers);

    SetupOutputVariable(state,
                        "Chiller Heater Condenser Heat Transfer Rate",
                        Constant::Units::W,
                        this->TowerLoad,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Condenser Heat Transfer Energy",
                        Constant::Units::J,
                        this->TowerEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::EnergyTransfer,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::HeatRejection);

    SetupOutputVariable(state,
                        format("Chiller Heater {} Rate", sFuelType),
                        Constant::Units::W,
                        this->FuelUseRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    // Do not include this on meters, this would duplicate the cool fuel and heat fuel
    SetupOutputVariable(state,
                        format("Chiller Heater {} Energy", sFuelType),
                        Constant::Units::J,
                        this->FuelEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        format("Chiller Heater Cooling {} Rate", sFuelType),
                        Constant::Units::W,
                        this->CoolFuelUseRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        format("Chiller Heater Cooling {} Energy", sFuelType),
                        Constant::Units::J,
                        this->CoolFuelEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eFuel2eResource[(int)this->FuelType],
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling);

    SetupOutputVariable(state,
                        "Chiller Heater Cooling COP",
                        Constant::Units::W_W,
                        this->FuelCOP,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        format("Chiller Heater Heating {} Rate", sFuelType),
                        Constant::Units::W,
                        this->HeatFuelUseRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        format("Chiller Heater Heating {} Energy", sFuelType),
                        Constant::Units::J,
                        this->HeatFuelEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eFuel2eResource[(int)this->FuelType],
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Heating);

    SetupOutputVariable(state,
                        "Chiller Heater Electricity Rate",
                        Constant::Units::W,
                        this->ElectricPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    // Do not include this on meters, this would duplicate the cool electric and heat electric
    SetupOutputVariable(state,
                        "Chiller Heater Electricity Energy",
                        Constant::Units::J,
                        this->ElectricEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater Cooling Electricity Rate",
                        Constant::Units::W,
                        this->CoolElectricPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Cooling Electricity Energy",
                        Constant::Units::J,
                        this->CoolElectricEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Cooling);

    SetupOutputVariable(state,
                        "Chiller Heater Heating Electricity Rate",
                        Constant::Units::W,
                        this->HeatElectricPower,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Heating Electricity Energy",
                        Constant::Units::J,
                        this->HeatElectricEnergy,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Sum,
                        this->Name,
                        Constant::eResource::Electricity,
                        OutputProcessor::Group::Plant,
                        OutputProcessor::EndUseCat::Heating);

    SetupOutputVariable(state,
                        "Chiller Heater Evaporator Inlet Temperature",
                        Constant::Units::C,
                        this->ChillReturnTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Evaporator Outlet Temperature",
                        Constant::Units::C,
                        this->ChillSupplyTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Evaporator Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->ChillWaterFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    if (this->isWaterCooled) {
        SetupOutputVariable(state,
                            "Chiller Heater Condenser Inlet Temperature",
                            Constant::Units::C,
                            this->CondReturnTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Chiller Heater Condenser Outlet Temperature",
                            Constant::Units::C,
                            this->CondSupplyTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Chiller Heater Condenser Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->CondWaterFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    } else {
        SetupOutputVariable(state,
                            "Chiller Heater Condenser Inlet Temperature",
                            Constant::Units::C,
                            this->CondReturnTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    }

    SetupOutputVariable(state,
                        "Chiller Heater Heating Inlet Temperature",
                        Constant::Units::C,
                        this->HotWaterReturnTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Heating Outlet Temperature",
                        Constant::Units::C,
                        this->HotWaterSupplyTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Heating Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->HotWaterFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater Cooling Part Load Ratio",
                        Constant::Units::None,
                        this->CoolPartLoadRatio,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Maximum Cooling Rate",
                        Constant::Units::W,
                        this->CoolingCapacity,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Heating Part Load Ratio",
                        Constant::Units::None,
                        this->HeatPartLoadRatio,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
    SetupOutputVariable(state,
                        "Chiller Heater Maximum Heating Rate",
                        Constant::Units::W,
                        this->HeatingCapacity,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heater Runtime Fraction",
                        Constant::Units::None,
                        this->FractionOfPeriodRunning,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->Name);
}

void GasAbsorberSpecs::oneTimeInit_new(EnergyPlusData &state)
{

    this->setupOutputVariables(state);

    // Locate the chillers on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(state,
                                            this->Name,
                                            DataPlant::PlantEquipmentType::Chiller_DFAbsorption,
                                            this->CWplantLoc,
                                            errFlag,
                                            this->CHWLowLimitTemp,
                                            _,
                                            _,
                                            this->ChillReturnNodeNum,
                                            _);
    if (errFlag) {
        ShowFatalError(state, "InitGasAbsorber: Program terminated due to previous condition(s).");
    }

    PlantUtilities::ScanPlantLoopsForObject(
        state, this->Name, DataPlant::PlantEquipmentType::Chiller_DFAbsorption, this->HWplantLoc, errFlag, _, _, _, this->HeatReturnNodeNum, _);
    if (errFlag) {
        ShowFatalError(state, "InitGasAbsorber: Program terminated due to previous condition(s).");
    }

    if (this->isWaterCooled) {
        PlantUtilities::ScanPlantLoopsForObject(
            state, this->Name, DataPlant::PlantEquipmentType::Chiller_DFAbsorption, this->CDplantLoc, errFlag, _, _, _, this->CondReturnNodeNum, _);
        if (errFlag) {
            ShowFatalError(state, "InitGasAbsorber: Program terminated due to previous condition(s).");
        }
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWplantLoc, this->CDplantLoc, DataPlant::PlantEquipmentType::Chiller_DFAbsorption, true);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->HWplantLoc, this->CDplantLoc, DataPlant::PlantEquipmentType::Chiller_DFAbsorption, true);
    }

    PlantUtilities::InterConnectTwoPlantLoopSides(
        state, this->CWplantLoc, this->HWplantLoc, DataPlant::PlantEquipmentType::Chiller_DFAbsorption, true);

    // check if outlet node of chilled water side has a setpoint.
    if ((state.dataLoopNodes->Node(this->ChillSupplyNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
        (state.dataLoopNodes->Node(this->ChillSupplyNodeNum).TempSetPointHi == DataLoopNode::SensedNodeFlagValue)) {
        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
            if (!this->ChillSetPointErrDone) {
                ShowWarningError(state, format("Missing temperature setpoint on cool side for chiller heater named {}", this->Name));
                ShowContinueError(state, "  A temperature setpoint is needed at the outlet node of this chiller, use a SetpointManager");
                ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                this->ChillSetPointErrDone = true;
            }
        } else {
            // need call to EMS to check node
            errFlag = false; // but not really fatal yet, but should be.
            EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->ChillSupplyNodeNum, HVAC::CtrlVarType::Temp, errFlag);
            state.dataLoopNodes->NodeSetpointCheck(this->ChillSupplyNodeNum).needsSetpointChecking = false;
            if (errFlag) {
                if (!this->ChillSetPointErrDone) {
                    ShowWarningError(state, format("Missing temperature setpoint on cool side for chiller heater named {}", this->Name));
                    ShowContinueError(state, "  A temperature setpoint is needed at the outlet node of this chiller evaporator ");
                    ShowContinueError(state, "  use a Setpoint Manager to establish a setpoint at the chiller evaporator outlet node ");
                    ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at the outlet node ");
                    ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                    this->ChillSetPointErrDone = true;
                }
            }
        }
        this->ChillSetPointSetToLoop = true;
        state.dataLoopNodes->Node(this->ChillSupplyNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(this->CWplantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->ChillSupplyNodeNum).TempSetPointHi =
            state.dataLoopNodes->Node(this->CWplantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
    }
    // check if outlet node of hot water side has a setpoint.
    if ((state.dataLoopNodes->Node(this->HeatSupplyNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
        (state.dataLoopNodes->Node(this->HeatSupplyNodeNum).TempSetPointLo == DataLoopNode::SensedNodeFlagValue)) {
        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
            if (!this->HeatSetPointErrDone) {
                ShowWarningError(state, format("Missing temperature setpoint on heat side for chiller heater named {}", this->Name));
                ShowContinueError(state, "  A temperature setpoint is needed at the outlet node of this chiller, use a SetpointManager");
                ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                this->HeatSetPointErrDone = true;
            }
        } else {
            // need call to EMS to check node
            errFlag = false; // but not really fatal yet, but should be.
            EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->HeatSupplyNodeNum, HVAC::CtrlVarType::Temp, errFlag);
            state.dataLoopNodes->NodeSetpointCheck(this->HeatSupplyNodeNum).needsSetpointChecking = false;
            if (errFlag) {
                if (!this->HeatSetPointErrDone) {
                    ShowWarningError(state, format("Missing temperature setpoint on heat side for chiller heater named {}", this->Name));
                    ShowContinueError(state, "  A temperature setpoint is needed at the outlet node of this chiller heater ");
                    ShowContinueError(state, "  use a Setpoint Manager to establish a setpoint at the heater side outlet node ");
                    ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at the outlet node ");
                    ShowContinueError(state, "  The overall loop setpoint will be assumed for heater side. The simulation continues ... ");
                    this->HeatSetPointErrDone = true;
                }
            }
        }
        this->HeatSetPointSetToLoop = true;
        state.dataLoopNodes->Node(this->HeatSupplyNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(this->HWplantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->HeatSupplyNodeNum).TempSetPointLo =
            state.dataLoopNodes->Node(this->HWplantLoc.loop->TempSetPointNodeNum).TempSetPointLo;
    }
}

void GasAbsorberSpecs::initialize(EnergyPlusData &state)
{
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   June 2003

    // This subroutine is for initializations of direct fired absorption chiller
    // components.

    // Uses the status flags to trigger initializations.

    static constexpr std::string_view RoutineName("InitGasAbsorber");

    Real64 rho = 0.0;  // local fluid density
    Real64 mdot = 0.0; // lcoal fluid mass flow rate

    // Init more variables

    int CondInletNode = this->CondReturnNodeNum;
    int CondOutletNode = this->CondSupplyNodeNum;
    int HeatInletNode = this->HeatReturnNodeNum;
    int HeatOutletNode = this->HeatSupplyNodeNum;

    if (this->envrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {

        if (this->isWaterCooled) {
            // init max available condenser water flow rate
            if (this->CDplantLoc.loopNum > 0) {
                rho = this->CDplantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            } else {
                rho = Psychrometrics::RhoH2O(Constant::InitConvTemp);
            }

            this->DesCondMassFlowRate = rho * this->CondVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->DesCondMassFlowRate, CondInletNode, CondOutletNode);
        }

        if (this->HWplantLoc.loopNum > 0) {
            rho = this->HWplantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
        } else {
            rho = Psychrometrics::RhoH2O(Constant::InitConvTemp);
        }
        this->DesHeatMassFlowRate = rho * this->HeatVolFlowRate;
        // init available hot water flow rate
        PlantUtilities::InitComponentNodes(state, 0.0, this->DesHeatMassFlowRate, HeatInletNode, HeatOutletNode);

        if (this->CWplantLoc.loopNum > 0) {
            rho = this->CWplantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
        } else {
            rho = Psychrometrics::RhoH2O(Constant::InitConvTemp);
        }
        this->DesEvapMassFlowRate = rho * this->EvapVolFlowRate;
        // init available hot water flow rate
        PlantUtilities::InitComponentNodes(state, 0.0, this->DesEvapMassFlowRate, this->ChillReturnNodeNum, this->ChillSupplyNodeNum);

        this->envrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->envrnFlag = true;
    }

    // this component model works off setpoints on the leaving node
    // fill from plant if needed
    if (this->ChillSetPointSetToLoop) {
        state.dataLoopNodes->Node(this->ChillSupplyNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(this->CWplantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->ChillSupplyNodeNum).TempSetPointHi =
            state.dataLoopNodes->Node(this->CWplantLoc.loop->TempSetPointNodeNum).TempSetPointHi;
    }

    if (this->HeatSetPointSetToLoop) {
        state.dataLoopNodes->Node(this->HeatSupplyNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(this->HWplantLoc.loop->TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->HeatSupplyNodeNum).TempSetPointLo =
            state.dataLoopNodes->Node(this->HWplantLoc.loop->TempSetPointNodeNum).TempSetPointLo;
    }

    if ((this->isWaterCooled) && ((this->InHeatingMode) || (this->InCoolingMode))) {
        mdot = this->DesCondMassFlowRate;

        PlantUtilities::SetComponentFlowRate(state, mdot, this->CondReturnNodeNum, this->CondSupplyNodeNum, this->CDplantLoc);

    } else {
        mdot = 0.0;
        if (this->CDplantLoc.loopNum > 0 && this->isWaterCooled) {
            PlantUtilities::SetComponentFlowRate(state, mdot, this->CondReturnNodeNum, this->CondSupplyNodeNum, this->CDplantLoc);
        }
    }
}

void GasAbsorberSpecs::size(EnergyPlusData &state)
{
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   June 2003
    //       MODIFIED       November 2013 Daeho Kang, add component sizing table entries

    // This subroutine is for sizing direct fired gas absorption chiller components for which
    // capacities and flow rates have not been specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains evaporator flow rate from the plant sizing array. Calculates nominal capacity from
    // the evaporator flow rate and the chilled water loop design delta T. The condenser flow rate
    // is calculated from the nominal capacity, the COP, and the condenser loop design delta T.

    static constexpr std::string_view RoutineName("SizeGasAbsorber");

    Real64 Cp;                     // local fluid specific heat
    Real64 rho;                    // local fluid density
    Real64 NomCapUser;             // Hardsized nominal capacity for reporting
    Real64 EvapVolFlowRateUser;    // Hardsized evaporator volume flow rate for reporting
    Real64 CondVolFlowRateUser;    // Hardsized condenser flow rate for reporting
    Real64 HeatRecVolFlowRateUser; // Hardsized generator flow rate for reporting

    bool ErrorsFound = false;
    Real64 tmpNomCap = this->NomCoolingCap;
    Real64 tmpEvapVolFlowRate = this->EvapVolFlowRate;
    Real64 tmpCondVolFlowRate = this->CondVolFlowRate;
    Real64 tmpHeatRecVolFlowRate = this->HeatVolFlowRate;

    int PltSizCondNum = 0; // Plant Sizing index for condenser loop
    if (this->isWaterCooled) {
        PltSizCondNum = this->CDplantLoc.loop->PlantSizNum;
    }
    int PltSizHeatNum = this->HWplantLoc.loop->PlantSizNum;
    int PltSizCoolNum = this->CWplantLoc.loop->PlantSizNum;

    if (PltSizCoolNum > 0) {
        if (state.dataSize->PlantSizData(PltSizCoolNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            Cp = this->CWplantLoc.loop->glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);
            rho = this->CWplantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            tmpNomCap = Cp * rho * state.dataSize->PlantSizData(PltSizCoolNum).DeltaT * state.dataSize->PlantSizData(PltSizCoolNum).DesVolFlowRate *
                        this->SizFac;
            if (!this->NomCoolingCapWasAutoSized) {
                tmpNomCap = this->NomCoolingCap;
            }
        } else {
            if (this->NomCoolingCapWasAutoSized) {
                tmpNomCap = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->NomCoolingCapWasAutoSized) {
                this->NomCoolingCap = tmpNomCap;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "ChillerHeater:Absorption:DirectFired", this->Name, "Design Size Nominal Cooling Capacity [W]", tmpNomCap);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(
                        state, "ChillerHeater:Absorption:DirectFired", this->Name, "Initial Design Size Nominal Cooling Capacity [W]", tmpNomCap);
                }
            } else {
                if (this->NomCoolingCap > 0.0 && tmpNomCap > 0.0) {
                    NomCapUser = this->NomCoolingCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeater:Absorption:DirectFired",
                                                     this->Name,
                                                     "Design Size Nominal Cooling Capacity [W]",
                                                     tmpNomCap,
                                                     "User-Specified Nominal Cooling Capacity [W]",
                                                     NomCapUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpNomCap - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(
                                    state,
                                    format("SizeChillerHeaterAbsorptionDirectFired: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state, format("User-Specified Nominal Capacity of {:.2R} [W]", NomCapUser));
                                ShowContinueError(state, format("differs from Design Size Nominal Capacity of {:.2R} [W]", tmpNomCap));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpNomCap = NomCapUser;
                }
            }
        }
    } else {
        if (this->NomCoolingCapWasAutoSized) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, format("SizeGasAbsorber: ChillerHeater:Absorption:DirectFired=\"{}\", autosize error.", this->Name));
                ShowContinueError(state, "Autosizing of Direct Fired Absorption Chiller nominal cooling capacity requires");
                ShowContinueError(state, "a cooling loop Sizing:Plant object.");
                ErrorsFound = true;
            }
        } else {
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                if (this->NomCoolingCap > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, "ChillerHeater:Absorption:DirectFired", this->Name, "User-Specified Nominal Capacity [W]", this->NomCoolingCap);
                }
            }
        }
    }

    if (PltSizCoolNum > 0) {
        if (state.dataSize->PlantSizData(PltSizCoolNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            tmpEvapVolFlowRate = state.dataSize->PlantSizData(PltSizCoolNum).DesVolFlowRate * this->SizFac;
            if (!this->EvapVolFlowRateWasAutoSized) {
                tmpEvapVolFlowRate = this->EvapVolFlowRate;
            }
        } else {
            if (this->EvapVolFlowRateWasAutoSized) {
                tmpEvapVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->EvapVolFlowRateWasAutoSized) {
                this->EvapVolFlowRate = tmpEvapVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "Design Size Design Chilled Water Flow Rate [m3/s]",
                                                 tmpEvapVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "Initial Design Size Design Chilled Water Flow Rate [m3/s]",
                                                 tmpEvapVolFlowRate);
                }
            } else {
                if (this->EvapVolFlowRate > 0.0 && tmpEvapVolFlowRate > 0.0) {
                    EvapVolFlowRateUser = this->EvapVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeater:Absorption:DirectFired",
                                                     this->Name,
                                                     "Design Size Design Chilled Water Flow Rate [m3/s]",
                                                     tmpEvapVolFlowRate,
                                                     "User-Specified Design Chilled Water Flow Rate [m3/s]",
                                                     EvapVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpEvapVolFlowRate - EvapVolFlowRateUser) / EvapVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeChillerAbsorptionDirectFired: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Design Chilled Water Flow Rate of {:.5R} [m3/s]", EvapVolFlowRateUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Design Chilled Water Flow Rate of {:.5R} [m3/s]", tmpEvapVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpEvapVolFlowRate = EvapVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->EvapVolFlowRateWasAutoSized) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, format("SizeGasAbsorber: ChillerHeater:Absorption:DirectFired=\"{}\", autosize error.", this->Name));
                ShowContinueError(state, "Autosizing of Direct Fired Absorption Chiller evap flow rate requires");
                ShowContinueError(state, "a cooling loop Sizing:Plant object.");
                ErrorsFound = true;
            }
        } else {
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                if (this->EvapVolFlowRate > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "User-Specified Design Chilled Water Flow Rate [m3/s]",
                                                 this->EvapVolFlowRate);
                }
            }
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->ChillReturnNodeNum, tmpEvapVolFlowRate);

    if (PltSizHeatNum > 0) {
        if (state.dataSize->PlantSizData(PltSizHeatNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
            tmpHeatRecVolFlowRate = state.dataSize->PlantSizData(PltSizHeatNum).DesVolFlowRate * this->SizFac;
            if (!this->HeatVolFlowRateWasAutoSized) {
                tmpHeatRecVolFlowRate = this->HeatVolFlowRate;
            }

        } else {
            if (this->HeatVolFlowRateWasAutoSized) {
                tmpHeatRecVolFlowRate = 0.0;
            }
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->HeatVolFlowRateWasAutoSized) {
                this->HeatVolFlowRate = tmpHeatRecVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "Design Size Design Hot Water Flow Rate [m3/s]",
                                                 tmpHeatRecVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "Initial Design Size Design Hot Water Flow Rate [m3/s]",
                                                 tmpHeatRecVolFlowRate);
                }
            } else {
                if (this->HeatVolFlowRate > 0.0 && tmpHeatRecVolFlowRate > 0.0) {
                    HeatRecVolFlowRateUser = this->HeatVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeater:Absorption:DirectFired",
                                                     this->Name,
                                                     "Design Size Design Hot Water Flow Rate [m3/s]",
                                                     tmpHeatRecVolFlowRate,
                                                     "User-Specified Design Hot Water Flow Rate [m3/s]",
                                                     HeatRecVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpHeatRecVolFlowRate - HeatRecVolFlowRateUser) / HeatRecVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(
                                    state,
                                    format("SizeChillerHeaterAbsorptionDirectFired: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Design Hot Water Flow Rate of {:.5R} [m3/s]", HeatRecVolFlowRateUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Design Hot Water Flow Rate of {:.5R} [m3/s]", tmpHeatRecVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpHeatRecVolFlowRate = HeatRecVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->HeatVolFlowRateWasAutoSized) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, format("SizeGasAbsorber: ChillerHeater:Absorption:DirectFired=\"{}\", autosize error.", this->Name));
                ShowContinueError(state, "Autosizing of Direct Fired Absorption Chiller hot water flow rate requires");
                ShowContinueError(state, "a heating loop Sizing:Plant object.");
                ErrorsFound = true;
            }
        } else {
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                if (this->HeatVolFlowRate > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "User-Specified Design Hot Water Flow Rate [m3/s]",
                                                 this->HeatVolFlowRate);
                }
            }
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, this->HeatReturnNodeNum, tmpHeatRecVolFlowRate);

    if (PltSizCondNum > 0 && PltSizCoolNum > 0) {
        if (state.dataSize->PlantSizData(PltSizCoolNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow && tmpNomCap > 0.0) {

            Cp = this->CDplantLoc.loop->glycol->getSpecificHeat(state, this->TempDesCondReturn, RoutineName);
            rho = this->CDplantLoc.loop->glycol->getDensity(state, this->TempDesCondReturn, RoutineName);
            tmpCondVolFlowRate = tmpNomCap * (1.0 + this->FuelCoolRatio) / (state.dataSize->PlantSizData(PltSizCondNum).DeltaT * Cp * rho);
            if (!this->CondVolFlowRateWasAutoSized) {
                tmpCondVolFlowRate = this->CondVolFlowRate;
            }
            // IF (PlantFirstSizesOkayToFinalize) GasAbsorber(ChillNum)%CondVolFlowRate = tmpCondVolFlowRate
        } else {
            if (this->CondVolFlowRateWasAutoSized) {
                tmpCondVolFlowRate = 0.0;
            }
            // IF (PlantFirstSizesOkayToFinalize) GasAbsorber(ChillNum)%CondVolFlowRate = tmpCondVolFlowRate
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->CondVolFlowRateWasAutoSized) {
                this->CondVolFlowRate = tmpCondVolFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "Design Size Design Condenser Water Flow Rate [m3/s]",
                                                 tmpCondVolFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "Initial Design Size Design Condenser Water Flow Rate [m3/s]",
                                                 tmpCondVolFlowRate);
                }
            } else {
                if (this->CondVolFlowRate > 0.0 && tmpCondVolFlowRate > 0.0) {
                    CondVolFlowRateUser = this->CondVolFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ChillerHeater:Absorption:DirectFired",
                                                     this->Name,
                                                     "Design Size Design Condenser Water Flow Rate [m3/s]",
                                                     tmpCondVolFlowRate,
                                                     "User-Specified Design Condenser Water Flow Rate [m3/s]",
                                                     CondVolFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpCondVolFlowRate - CondVolFlowRateUser) / CondVolFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeChillerAbsorptionDirectFired: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state,
                                                  format("User-Specified Design Condenser Water Flow Rate of {:.5R} [m3/s]", CondVolFlowRateUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Design Condenser Water Flow Rate of {:.5R} [m3/s]", tmpCondVolFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                    tmpCondVolFlowRate = CondVolFlowRateUser;
                }
            }
        }
    } else {
        if (this->CondVolFlowRateWasAutoSized) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                ShowSevereError(state, format("SizeGasAbsorber: ChillerHeater:Absorption:DirectFired=\"{}\", autosize error.", this->Name));
                ShowContinueError(state, "Autosizing of Direct Fired Absorption Chiller condenser flow rate requires a condenser");
                ShowContinueError(state, "loop Sizing:Plant object.");
                ErrorsFound = true;
            }
        } else {
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                if (this->CondVolFlowRate > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 "ChillerHeater:Absorption:DirectFired",
                                                 this->Name,
                                                 "User-Specified Design Condenser Water Flow Rate [m3/s]",
                                                 this->CondVolFlowRate);
                }
            }
        }
    }

    // save the design condenser water volumetric flow rate for use by the condenser water loop sizing algorithms
    if (this->isWaterCooled) {
        PlantUtilities::RegisterPlantCompDesignFlow(state, this->CondReturnNodeNum, tmpCondVolFlowRate);
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }

    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
        // create predefined report
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, this->Name, "ChillerHeater:Absorption:DirectFired");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomEff, this->Name, this->FuelCoolRatio);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->NomCoolingCap);

        // std 229 new Chillers table
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerType, this->Name, "ChillerHeater:Absorption:DirectFired");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefCap, this->Name, this->NomCoolingCap);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefEff, this->Name, this->FuelCoolRatio);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedCap, this->Name, this->FuelCoolRatio); // did not find rated cap
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerRatedEff,
                                                 this->Name,
                                                 this->NomCoolingCap); // did not find rated eff or cop ; also Eff == COP?
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerIPLVinSI, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerIPLVinIP, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerPlantloopName,
                                                 this->Name,
                                                 (this->CWplantLoc.loop != nullptr) ? this->CWplantLoc.loop->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerPlantloopBranchName,
                                                 this->Name,
                                                 (this->CWplantLoc.loop != nullptr) ? this->CWplantLoc.branch->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerCondLoopName,
                                                 this->Name,
                                                 (this->CDplantLoc.loop != nullptr) ? this->CDplantLoc.loop->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchChillerCondLoopBranchName,
                                                 this->Name,
                                                 (this->CDplantLoc.branch != nullptr) ? this->CDplantLoc.branch->Name : "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerMinPLR, this->Name, this->MinPartLoadRat);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerFuelType, this->Name, Constant::eResourceNames[static_cast<int>(this->FuelType)]);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerRatedEntCondTemp, this->Name, this->TempDesCondReturn); // Rated==Ref?
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRatedLevEvapTemp, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefEntCondTemp, this->Name, this->TempDesCondReturn);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRefLevEvapTemp, this->Name, "N/A");

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerDesSizeRefCHWFlowRate, this->Name, this->DesEvapMassFlowRate);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchChillerDesSizeRefCondFluidFlowRate, this->Name, this->DesCondMassFlowRate);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerHeatRecPlantloopName, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerHeatRecPlantloopBranchName, this->Name, "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchChillerRecRelCapFrac, this->Name, "N/A");
    }
}

void GasAbsorberSpecs::calculateChiller(EnergyPlusData &state, Real64 &MyLoad)
{
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   March 2001

    // Simulate a direct fired (gas consuming) absorption chiller using
    // curves and inputs similar to DOE-2.1e

    // METHODOLOGY EMPLOYED:
    // Curve fit of performance data

    // REFERENCES:
    // 1.  DOE-2.1e Supplement and source code
    // 2.  CoolTools GasMod work

    // FlowLock = 0  if mass flow rates may be changed by loop components
    // FlowLock = 1  if mass flow rates may not be changed by loop components

    static constexpr std::string_view RoutineName("CalcGasAbsorberChillerModel");

    // Local copies of GasAbsorberSpecs Type
    // all variables that are local copies of data structure
    // variables are prefaced with an "l" for local.
    // Local copies of GasAbsorberReportVars Type
    Real64 lCoolingLoad(0.0);              // cooling load on the chiller (previously called QEvap)
    Real64 lTowerLoad(0.0);                // load on the cooling tower/condenser (previously called QCond)
    Real64 lCoolFuelUseRate(0.0);          // instantaneous use of gas for period for cooling
    Real64 lCoolElectricPower(0.0);        // parasitic electric power used  for cooling
    Real64 lChillSupplyTemp(0.0);          // reporting: evaporator outlet temperature (was EvapOutletTemp)
    Real64 lCondSupplyTemp(0.0);           // reporting: condenser outlet temperature (was CondOutletTemp)
    Real64 lCondWaterMassFlowRate(0.0);    // reporting: condenser mass flow rate (was Condmdot)
    Real64 lCoolPartLoadRatio(0.0);        // operating part load ratio (load/capacity for cooling)
    Real64 lAvailableCoolingCapacity(0.0); // current capacity after temperature adjustment
    Real64 lFractionOfPeriodRunning(0.0);
    Real64 PartLoadRat(0.0);           // actual operating part load ratio of unit (ranges from minplr to 1)
    Real64 lChillWaterMassflowratemax; // Maximum flow rate through the evaporator

    // other local variables
    Real64 ChillSupplySetPointTemp(0.0);

    Real64 calcCondTemp; // the condenser temperature used for curve calculation
    // either return or supply depending on user input
    Real64 revisedEstimateAvailCap; // final estimate of available capacity if using leaving
    // condenser water temperature
    Real64 errorAvailCap; // error fraction on final estimate of AvailableCoolingCapacity
    DataPlant::LoopSideLocation LoopSideNum;

    // set node values to data structure values for nodes

    int lChillReturnNodeNum = this->ChillReturnNodeNum; // Node number on the inlet side of the plant
    int lChillSupplyNodeNum = this->ChillSupplyNodeNum; // Node number on the outlet side of the plant
    int lCondReturnNodeNum = this->CondReturnNodeNum;   // Node number on the inlet side of the condenser

    // set local copies of data from rest of input structure

    Real64 lNomCoolingCap = this->NomCoolingCap;         // W - design nominal capacity of Absorber
    Real64 lFuelCoolRatio = this->FuelCoolRatio;         // ratio of fuel input to cooling output
    Real64 lFuelHeatRatio = this->FuelHeatRatio;         // ratio of fuel input to heating output
    Real64 lElecCoolRatio = this->ElecCoolRatio;         // ratio of electricity input to cooling output
    Real64 lMinPartLoadRat = this->MinPartLoadRat;       // min allowed operating frac full load
    Real64 lMaxPartLoadRat = this->MaxPartLoadRat;       // max allowed operating frac full load
    bool lIsEnterCondensTemp = this->isEnterCondensTemp; // if using entering conderser water temperature is TRUE, exiting is FALSE
    bool lIsWaterCooled = this->isWaterCooled;           // if water cooled it is TRUE
    Real64 lCHWLowLimitTemp = this->CHWLowLimitTemp;     // Chilled Water Lower Limit Temperature

    Real64 lHeatElectricPower = this->HeatElectricPower; // parasitic electric power used  for heating
    Real64 lHeatFuelUseRate = this->HeatFuelUseRate;     // instantaneous use of gas for period for heating
    Real64 lHeatPartLoadRatio = this->HeatPartLoadRatio; // operating part load ratio (load/capacity for heating)

    // initialize entering conditions
    Real64 lChillReturnTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp;
    Real64 lChillWaterMassFlowRate = state.dataLoopNodes->Node(lChillReturnNodeNum).MassFlowRate;
    Real64 lCondReturnTemp = state.dataLoopNodes->Node(lCondReturnNodeNum).Temp;
    // Commenting this could be cause of diffs - lCondWaterMassFlowRate = Node(lCondReturnNodeNum).MassFlowRate;
    switch (this->CWplantLoc.loop->LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        ChillSupplySetPointTemp = state.dataLoopNodes->Node(lChillSupplyNodeNum).TempSetPoint;
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        ChillSupplySetPointTemp = state.dataLoopNodes->Node(lChillSupplyNodeNum).TempSetPointHi;
    } break;
    default: {
        assert(false);
    } break;
    }
    Real64 ChillDeltaTemp = std::abs(lChillReturnTemp - ChillSupplySetPointTemp);

    // local fluid specific heat for chilled water
    Real64 Cp_CW = this->CWplantLoc.loop->glycol->getSpecificHeat(state, lChillReturnTemp, RoutineName);
    // local fluid specific heat for condenser water
    Real64 Cp_CD = 0; // putting this here as a dummy initialization to hush the compiler warning, in real runs this value should never be used
    if (this->CDplantLoc.loopNum > 0) {
        Cp_CD = this->CDplantLoc.loop->glycol->getSpecificHeat(state, lChillReturnTemp, RoutineName);
    }

    // If no loop demand or Absorber OFF, return
    // will need to modify when absorber can act as a boiler
    if (MyLoad >= 0 || !((this->InHeatingMode) || (this->InCoolingMode))) {
        // set node temperatures
        lChillSupplyTemp = lChillReturnTemp;
        lCondSupplyTemp = lCondReturnTemp;
        lCondWaterMassFlowRate = 0.0;
        if (lIsWaterCooled) {
            PlantUtilities::SetComponentFlowRate(state, lCondWaterMassFlowRate, this->CondReturnNodeNum, this->CondSupplyNodeNum, this->CDplantLoc);
        }
        // Commenting this could cause diffs - ChillDeltaTemp = 0.0;
        lFractionOfPeriodRunning = min(1.0, max(lHeatPartLoadRatio, lCoolPartLoadRatio) / lMinPartLoadRat);
    } else {

        // if water cooled use the input node otherwise just use outside air temperature
        if (lIsWaterCooled) {
            // most manufacturers rate have tables of entering condenser water temperature
            // but a few use leaving condenser water temperature so we have a flag
            // when leaving is used it uses the previous iterations value of the value
            lCondReturnTemp = state.dataLoopNodes->Node(lCondReturnNodeNum).Temp;
            if (lIsEnterCondensTemp) {
                calcCondTemp = lCondReturnTemp;
            } else {
                if (this->oldCondSupplyTemp == 0) {
                    this->oldCondSupplyTemp = lCondReturnTemp + 8.0; // if not previously estimated assume 8C greater than return
                }
                calcCondTemp = this->oldCondSupplyTemp;
            }
            // Set mass flow rates
            lCondWaterMassFlowRate = this->DesCondMassFlowRate;
            PlantUtilities::SetComponentFlowRate(state, lCondWaterMassFlowRate, this->CondReturnNodeNum, this->CondSupplyNodeNum, this->CDplantLoc);
        } else {
            // air cooled
            state.dataLoopNodes->Node(lCondReturnNodeNum).Temp = state.dataLoopNodes->Node(lCondReturnNodeNum).OutAirDryBulb;
            calcCondTemp = state.dataLoopNodes->Node(lCondReturnNodeNum).OutAirDryBulb;
            lCondReturnTemp = state.dataLoopNodes->Node(lCondReturnNodeNum).Temp;
            lCondWaterMassFlowRate = 0.0;
            if (this->CDplantLoc.loopNum > 0) {
                PlantUtilities::SetComponentFlowRate(
                    state, lCondWaterMassFlowRate, this->CondReturnNodeNum, this->CondSupplyNodeNum, this->CDplantLoc);
            }
        }

        // Determine available cooling capacity using the setpoint temperature
        lAvailableCoolingCapacity = lNomCoolingCap * this->CoolCapFTCurve->value(state, ChillSupplySetPointTemp, calcCondTemp);

        // Calculate current load for cooling
        MyLoad = sign(max(std::abs(MyLoad), lAvailableCoolingCapacity * lMinPartLoadRat), MyLoad);
        MyLoad = sign(min(std::abs(MyLoad), lAvailableCoolingCapacity * lMaxPartLoadRat), MyLoad);

        // Determine the following variables depending on if the flow has been set in
        // the nodes (flowlock=1 to 2) or if the amount of load is still be determined (flowlock=0)
        //    chilled water flow,
        //    cooling load taken by the chiller, and
        //    supply temperature
        lChillWaterMassflowratemax = this->DesEvapMassFlowRate;

        switch (this->CWplantLoc.side->FlowLock) {
        case DataPlant::FlowLock::Unlocked: { // mass flow rates may be changed by loop components
            this->PossibleSubcooling = false;
            lCoolingLoad = std::abs(MyLoad);
            if (ChillDeltaTemp != 0.0) {
                lChillWaterMassFlowRate = std::abs(lCoolingLoad / (Cp_CW * ChillDeltaTemp));
                if (lChillWaterMassFlowRate - lChillWaterMassflowratemax > DataBranchAirLoopPlant::MassFlowTolerance) {
                    this->PossibleSubcooling = true;
                }

                PlantUtilities::SetComponentFlowRate(
                    state, lChillWaterMassFlowRate, this->ChillReturnNodeNum, this->ChillSupplyNodeNum, this->CWplantLoc);
                // Commenting this could cause diffs - lChillSupplyTemp = ChillSupplySetPointTemp;
            } else {
                lChillWaterMassFlowRate = 0.0;
                ShowRecurringWarningErrorAtEnd(state,
                                               "GasAbsorberChillerModel:Cooling\"" + this->Name + "\", DeltaTemp = 0 in mass flow calculation",
                                               this->DeltaTempCoolErrCount);
            }
            lChillSupplyTemp = ChillSupplySetPointTemp;
        } break;
        case DataPlant::FlowLock::Locked: { // mass flow rates may not be changed by loop components
            lChillWaterMassFlowRate = state.dataLoopNodes->Node(lChillReturnNodeNum).MassFlowRate;
            if (this->PossibleSubcooling) {
                lCoolingLoad = std::abs(MyLoad);

                ChillDeltaTemp = lCoolingLoad / lChillWaterMassFlowRate / Cp_CW;
                lChillSupplyTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - ChillDeltaTemp;
            } else {
                ChillDeltaTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - ChillSupplySetPointTemp;
                lCoolingLoad = std::abs(lChillWaterMassFlowRate * Cp_CW * ChillDeltaTemp);
                lChillSupplyTemp = ChillSupplySetPointTemp;
            }
            // Check that the Chiller Supply outlet temp honors both plant loop temp low limit and also the chiller low limit
            if (lChillSupplyTemp < lCHWLowLimitTemp) {
                if ((state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - lCHWLowLimitTemp) > DataPlant::DeltaTempTol) {
                    lChillSupplyTemp = lCHWLowLimitTemp;
                    ChillDeltaTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - lChillSupplyTemp;
                    lCoolingLoad = lChillWaterMassFlowRate * Cp_CW * ChillDeltaTemp;
                } else {
                    lChillSupplyTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp;
                    ChillDeltaTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - lChillSupplyTemp;
                    lCoolingLoad = lChillWaterMassFlowRate * Cp_CW * ChillDeltaTemp;
                }
            }
            if (lChillSupplyTemp < state.dataLoopNodes->Node(lChillSupplyNodeNum).TempMin) {
                if ((state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - state.dataLoopNodes->Node(lChillSupplyNodeNum).TempMin) >
                    DataPlant::DeltaTempTol) {
                    lChillSupplyTemp = state.dataLoopNodes->Node(lChillSupplyNodeNum).TempMin;
                    ChillDeltaTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - lChillSupplyTemp;
                    lCoolingLoad = lChillWaterMassFlowRate * Cp_CW * ChillDeltaTemp;
                } else {
                    lChillSupplyTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp;
                    ChillDeltaTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - lChillSupplyTemp;
                    lCoolingLoad = lChillWaterMassFlowRate * Cp_CW * ChillDeltaTemp;
                }
            }

            // Checks Coolingload on the basis of the machine limits.
            if (lCoolingLoad > std::abs(MyLoad)) {
                if (lChillWaterMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                    lCoolingLoad = std::abs(MyLoad);
                    ChillDeltaTemp = lCoolingLoad / lChillWaterMassFlowRate / Cp_CW;
                    lChillSupplyTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - ChillDeltaTemp;
                } else {
                    lChillSupplyTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp;
                    ChillDeltaTemp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp - lChillSupplyTemp;
                    lCoolingLoad = lChillWaterMassFlowRate * Cp_CW * ChillDeltaTemp;
                }
            }
        } break;
        default:
            break;
        }

        // Calculate operating part load ratio for cooling
        PartLoadRat = min(std::abs(MyLoad) / lAvailableCoolingCapacity, lMaxPartLoadRat);
        PartLoadRat = max(lMinPartLoadRat, PartLoadRat);

        if (lAvailableCoolingCapacity > 0.0) {
            if (std::abs(MyLoad) / lAvailableCoolingCapacity < lMinPartLoadRat) {
                lCoolPartLoadRatio = MyLoad / lAvailableCoolingCapacity;
            } else {
                lCoolPartLoadRatio = PartLoadRat;
            }
        } else { // Else if AvailableCoolingCapacity < 0.0
            lCoolPartLoadRatio = 0.0;
        }

        // calculate the fraction of the time period that the chiller would be running
        // use maximum from heating and cooling sides
        if (lCoolPartLoadRatio < lMinPartLoadRat || lHeatPartLoadRatio < lMinPartLoadRat) {
            lFractionOfPeriodRunning = min(1.0, max(lHeatPartLoadRatio, lCoolPartLoadRatio) / lMinPartLoadRat);
        } else {
            lFractionOfPeriodRunning = 1.0;
        }

        // Calculate fuel consumption for cooling
        // fuel used for cooling availCap * HIR * HIR-FT * HIR-FPLR
        lCoolFuelUseRate = lAvailableCoolingCapacity * lFuelCoolRatio * this->FuelCoolFTCurve->value(state, lChillSupplyTemp, calcCondTemp) *
                           this->FuelCoolFPLRCurve->value(state, lCoolPartLoadRatio) * lFractionOfPeriodRunning;

        // Calculate electric parasitics used
        // based on nominal capacity, not available capacity,
        // electric used for cooling nomCap * %OP * EIR * EIR-FT * EIR-FPLR
        lCoolElectricPower = lNomCoolingCap * lElecCoolRatio * lFractionOfPeriodRunning *
                             this->ElecCoolFTCurve->value(state, lChillSupplyTemp, calcCondTemp) *
                             this->ElecCoolFPLRCurve->value(state, lCoolPartLoadRatio);

        // determine conderser load which is cooling load plus the
        // fuel used for cooling times the burner efficiency plus
        // the electricity used
        lTowerLoad = lCoolingLoad + lCoolFuelUseRate / lFuelHeatRatio + lCoolElectricPower;

        // for water cooled condenser make sure enough flow rate
        // for air cooled condenser just set supply to return temperature
        if (lIsWaterCooled) {
            if (lCondWaterMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                lCondSupplyTemp = lCondReturnTemp + lTowerLoad / (lCondWaterMassFlowRate * Cp_CD);
            } else {
                if (this->lCondWaterMassFlowRate_Index == 0) {
                    ShowSevereError(state, format("CalcGasAbsorberChillerModel: Condenser flow = 0, for Gas Absorber Chiller={}", this->Name));
                    ShowContinueErrorTimeStamp(state, "");
                    // ShowFatalError(state, "Program Terminates due to previous error condition.");
                }
                ShowRecurringSevereErrorAtEnd(state,
                                              format("CalcGasAbsorberChillerModel: Condenser flow = 0, for Gas Absorber Chiller={}: Condenser flow "
                                                     "rate = 0 severe error warning continues...",
                                                     this->Name),                // Message automatically written to "error file" at end of simulation
                                              this->lCondWaterMassFlowRate_Index // Recurring message index, if zero, next available index is assigned
                );
            }
        } else {
            lCondSupplyTemp = lCondReturnTemp; // if air cooled condenser just set supply and return to same temperature
        }

        // save the condenser water supply temperature for next iteration if that is used in lookup
        // and if capacity is large enough error than report problem
        this->oldCondSupplyTemp = lCondSupplyTemp;
        if (!lIsEnterCondensTemp) {
            // calculate the fraction of the estimated error between the capacity based on the previous
            // iteration's value of condenser supply temperature and the actual calculated condenser supply
            // temperature.  If this becomes too common then may need to iterate a solution instead of
            // relying on previous iteration method.
            revisedEstimateAvailCap = lNomCoolingCap * this->CoolCapFTCurve->value(state, ChillSupplySetPointTemp, lCondSupplyTemp);
            if (revisedEstimateAvailCap > 0.0) {
                errorAvailCap = std::abs((revisedEstimateAvailCap - lAvailableCoolingCapacity) / revisedEstimateAvailCap);
                if (errorAvailCap > 0.05) { // if more than 5% error in estimate
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "GasAbsorberChillerModel:\"" + this->Name + "\", poor Condenser Supply Estimate",
                                                   this->CondErrCount,
                                                   errorAvailCap,
                                                   errorAvailCap);
                }
            }
        }
    } // IF(MyLoad>=0 .OR. .NOT. RunFlag)
    // Write into the Report Variables except for nodes
    this->CoolingLoad = lCoolingLoad;
    this->TowerLoad = lTowerLoad;
    this->CoolFuelUseRate = lCoolFuelUseRate;
    this->CoolElectricPower = lCoolElectricPower;
    this->CondReturnTemp = lCondReturnTemp;
    this->ChillReturnTemp = lChillReturnTemp;
    this->CondSupplyTemp = lCondSupplyTemp;
    this->ChillSupplyTemp = lChillSupplyTemp;
    this->ChillWaterFlowRate = lChillWaterMassFlowRate;
    this->CondWaterFlowRate = lCondWaterMassFlowRate;
    this->CoolPartLoadRatio = lCoolPartLoadRatio;
    this->CoolingCapacity = lAvailableCoolingCapacity;
    this->FractionOfPeriodRunning = lFractionOfPeriodRunning;

    // write the combined heating and cooling fuel used and electric used
    this->FuelUseRate = lCoolFuelUseRate + lHeatFuelUseRate;
    this->ElectricPower = lCoolElectricPower + lHeatElectricPower;
}

void GasAbsorberSpecs::calculateHeater(EnergyPlusData &state, Real64 &MyLoad, bool const RunFlag)
{
    //       AUTHOR         Jason Glazer and Michael J. Witte
    //       DATE WRITTEN   March 2001
    // Simulate a direct fired (gas consuming) absorption chiller using
    // curves and inputs similar to DOE-2.1e

    // METHODOLOGY EMPLOYED:
    // Curve fit of performance data

    // REFERENCES:
    // 1.  DOE-2.1e Supplement and source code
    // 2.  CoolTools GasMod work

    // FlowLock = 0  if mass flow rates may be changed by loop components
    // FlowLock = 1  if mass flow rates may not be changed by loop components
    // FlowLock = 2  if overloaded and mass flow rates has changed to a small amount and Tout drops
    //                 below Setpoint

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("CalcGasAbsorberHeaterModel");

    // Local copies of GasAbsorberSpecs Type
    // all variables that are local copies of data structure
    // variables are prefaced with an "l" for local.
    Real64 lNomCoolingCap;    // W - design nominal capacity of Absorber
    Real64 lNomHeatCoolRatio; // ratio of heating to cooling capacity
    Real64 lFuelHeatRatio;    // ratio of fuel input to heating output
    Real64 lElecHeatRatio;    // ratio of electricity input to heating output
    int lHeatReturnNodeNum;   // absorber steam inlet node number, water side
    int lHeatSupplyNodeNum;   // absorber steam outlet node number, water side
    Real64 lMinPartLoadRat;   // min allowed operating frac full load
    Real64 lMaxPartLoadRat;   // max allowed operating frac full load
    // Local copies of GasAbsorberReportVars Type
    Real64 lHeatingLoad(0.0);              // heating load on the chiller
    Real64 lCoolFuelUseRate(0.0);          // instantaneous use of gas for period for cooling
    Real64 lHeatFuelUseRate(0.0);          // instantaneous use of gas for period for heating
    Real64 lCoolElectricPower(0.0);        // parasitic electric power used  for cooling
    Real64 lHeatElectricPower(0.0);        // parasitic electric power used  for heating
    Real64 lHotWaterReturnTemp(0.0);       // reporting: hot water return (inlet) temperature
    Real64 lHotWaterSupplyTemp(0.0);       // reporting: hot water supply (outlet) temperature
    Real64 lHotWaterMassFlowRate(0.0);     // reporting: hot water mass flow rate
    Real64 lCoolPartLoadRatio(0.0);        // operating part load ratio (load/capacity for cooling)
    Real64 lHeatPartLoadRatio(0.0);        // operating part load ratio (load/capacity for heating)
    Real64 lAvailableHeatingCapacity(0.0); // current heating capacity
    Real64 lFractionOfPeriodRunning(0.0);
    // other local variables
    Real64 HeatDeltaTemp(0.0); // hot water temperature difference
    Real64 HeatSupplySetPointTemp(0.0);
    Real64 Cp_HW; // local fluid specific heat for hot water

    // set node values to data structure values for nodes

    lHeatReturnNodeNum = this->HeatReturnNodeNum;
    lHeatSupplyNodeNum = this->HeatSupplyNodeNum;

    // set local copies of data from rest of input structure

    lNomCoolingCap = this->NomCoolingCap;
    lNomHeatCoolRatio = this->NomHeatCoolRatio;
    lFuelHeatRatio = this->FuelHeatRatio;
    lElecHeatRatio = this->ElecHeatRatio;
    lMinPartLoadRat = this->MinPartLoadRat;
    lMaxPartLoadRat = this->MaxPartLoadRat;

    Cp_HW = this->HWplantLoc.loop->glycol->getSpecificHeat(state, lHotWaterReturnTemp, RoutineName);

    lCoolElectricPower = this->CoolElectricPower;
    lCoolFuelUseRate = this->CoolFuelUseRate;
    lCoolPartLoadRatio = this->CoolPartLoadRatio;

    // initialize entering conditions
    lHotWaterReturnTemp = state.dataLoopNodes->Node(lHeatReturnNodeNum).Temp;
    lHotWaterMassFlowRate = state.dataLoopNodes->Node(lHeatReturnNodeNum).MassFlowRate;
    switch (this->HWplantLoc.loop->LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        HeatSupplySetPointTemp = state.dataLoopNodes->Node(lHeatSupplyNodeNum).TempSetPoint;
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        HeatSupplySetPointTemp = state.dataLoopNodes->Node(lHeatSupplyNodeNum).TempSetPointLo;
    } break;
    default: {
        assert(false);
    } break;
    }
    HeatDeltaTemp = std::abs(lHotWaterReturnTemp - HeatSupplySetPointTemp);

    // If no loop demand or Absorber OFF, return
    // will need to modify when absorber can act as a boiler
    if (MyLoad <= 0 || !RunFlag) {
        // set node temperatures
        lHotWaterSupplyTemp = lHotWaterReturnTemp;
        // Commenting this could cause diffs - HeatDeltaTemp = 0.0;
        lFractionOfPeriodRunning = min(1.0, max(lHeatPartLoadRatio, lCoolPartLoadRatio) / lMinPartLoadRat);
    } else {

        // Determine available heating capacity using the current cooling load
        lAvailableHeatingCapacity =
            this->NomHeatCoolRatio * this->NomCoolingCap * this->HeatCapFCoolCurve->value(state, (this->CoolingLoad / this->NomCoolingCap));

        // Calculate current load for heating
        MyLoad = sign(max(std::abs(MyLoad), this->HeatingCapacity * lMinPartLoadRat), MyLoad);
        MyLoad = sign(min(std::abs(MyLoad), this->HeatingCapacity * lMaxPartLoadRat), MyLoad);

        // Determine the following variables depending on if the flow has been set in
        // the nodes (flowlock=1 to 2) or if the amount of load is still be determined (flowlock=0)
        //    chilled water flow,
        //    cooling load taken by the chiller, and
        //    supply temperature
        switch (this->HWplantLoc.side->FlowLock) {
        case DataPlant::FlowLock::Unlocked: { // mass flow rates may be changed by loop components
            lHeatingLoad = std::abs(MyLoad);
            if (HeatDeltaTemp != 0) {
                lHotWaterMassFlowRate = std::abs(lHeatingLoad / (Cp_HW * HeatDeltaTemp));

                PlantUtilities::SetComponentFlowRate(
                    state, lHotWaterMassFlowRate, this->HeatReturnNodeNum, this->HeatSupplyNodeNum, this->HWplantLoc);

            } else {
                lHotWaterMassFlowRate = 0.0;
                ShowRecurringWarningErrorAtEnd(state,
                                               "GasAbsorberChillerModel:Heating\"" + this->Name + "\", DeltaTemp = 0 in mass flow calculation",
                                               this->DeltaTempHeatErrCount);
            }
            lHotWaterSupplyTemp = HeatSupplySetPointTemp;
        } break;
        case DataPlant::FlowLock::Locked: { // mass flow rates may not be changed by loop components
            lHotWaterSupplyTemp = HeatSupplySetPointTemp;
            lHeatingLoad = std::abs(lHotWaterMassFlowRate * Cp_HW * HeatDeltaTemp);
        } break;
        default:
            break;
        }

        // Calculate operating part load ratio for cooling
        if (lAvailableHeatingCapacity <= 0.0) {
            lAvailableHeatingCapacity = 0.0;
            lHeatPartLoadRatio = 0.0;
        } else {
            lHeatPartLoadRatio = lHeatingLoad / lAvailableHeatingCapacity;
        }

        // Calculate fuel consumption for cooling
        // fuel used for cooling availCap * HIR * HIR-FT * HIR-FPLR

        lHeatFuelUseRate = lAvailableHeatingCapacity * lFuelHeatRatio * this->FuelHeatFHPLRCurve->value(state, lHeatPartLoadRatio);

        // calculate the fraction of the time period that the chiller would be running
        // use maximum from heating and cooling sides
        lFractionOfPeriodRunning = min(1.0, max(lHeatPartLoadRatio, lCoolPartLoadRatio) / lMinPartLoadRat);

        // Calculate electric parasitics used
        // for heating based on nominal capacity not available capacity
        lHeatElectricPower = lNomCoolingCap * lNomHeatCoolRatio * lElecHeatRatio * lFractionOfPeriodRunning;
        // Coodinate electric parasitics for heating and cooling to avoid double counting
        // Total electric is the max of heating electric or cooling electric
        // If heating electric is greater, leave cooling electric and subtract if off of heating elec
        // If cooling electric is greater, set heating electric to zero
        if (lHeatElectricPower <= lCoolElectricPower) {
            lHeatElectricPower = 0.0;
        } else {
            lHeatElectricPower -= lCoolElectricPower;
        }

    } // IF(MyLoad==0 .OR. .NOT. RunFlag)
    // Write into the Report Variables except for nodes
    this->HeatingLoad = lHeatingLoad;
    this->HeatFuelUseRate = lHeatFuelUseRate;
    this->HeatElectricPower = lHeatElectricPower;
    this->HotWaterReturnTemp = lHotWaterReturnTemp;
    this->HotWaterSupplyTemp = lHotWaterSupplyTemp;
    this->HotWaterFlowRate = lHotWaterMassFlowRate;
    this->HeatPartLoadRatio = lHeatPartLoadRatio;
    this->HeatingCapacity = lAvailableHeatingCapacity;
    this->FractionOfPeriodRunning = lFractionOfPeriodRunning;

    // write the combined heating and cooling fuel used and electric used
    this->FuelUseRate = lCoolFuelUseRate + lHeatFuelUseRate;
    this->ElectricPower = lCoolElectricPower + lHeatElectricPower;
}

void GasAbsorberSpecs::updateCoolRecords(EnergyPlusData &state,
                                         Real64 const MyLoad, // current load
                                         bool const RunFlag   // TRUE if Absorber operating
)
{
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   March 2001

    int lChillReturnNodeNum = this->ChillReturnNodeNum;
    int lChillSupplyNodeNum = this->ChillSupplyNodeNum;
    int lCondReturnNodeNum = this->CondReturnNodeNum;
    int lCondSupplyNodeNum = this->CondSupplyNodeNum;

    if (MyLoad == 0 || !RunFlag) {
        state.dataLoopNodes->Node(lChillSupplyNodeNum).Temp = state.dataLoopNodes->Node(lChillReturnNodeNum).Temp;
        if (this->isWaterCooled) {
            state.dataLoopNodes->Node(lCondSupplyNodeNum).Temp = state.dataLoopNodes->Node(lCondReturnNodeNum).Temp;
        }
    } else {
        state.dataLoopNodes->Node(lChillSupplyNodeNum).Temp = this->ChillSupplyTemp;
        if (this->isWaterCooled) {
            state.dataLoopNodes->Node(lCondSupplyNodeNum).Temp = this->CondSupplyTemp;
        }
    }

    // convert power to energy and instantaneous use to use over the time step
    this->CoolingEnergy = this->CoolingLoad * state.dataHVACGlobal->TimeStepSysSec;
    this->TowerEnergy = this->TowerLoad * state.dataHVACGlobal->TimeStepSysSec;
    this->FuelEnergy = this->FuelUseRate * state.dataHVACGlobal->TimeStepSysSec;
    this->CoolFuelEnergy = this->CoolFuelUseRate * state.dataHVACGlobal->TimeStepSysSec;
    this->ElectricEnergy = this->ElectricPower * state.dataHVACGlobal->TimeStepSysSec;
    this->CoolElectricEnergy = this->CoolElectricPower * state.dataHVACGlobal->TimeStepSysSec;
    if (this->CoolFuelUseRate != 0.0) {
        this->FuelCOP = this->CoolingLoad / this->CoolFuelUseRate;
    } else {
        this->FuelCOP = 0.0;
    }
}

void GasAbsorberSpecs::updateHeatRecords(EnergyPlusData &state,
                                         Real64 const MyLoad, // current load
                                         bool const RunFlag   // TRUE if Absorber operating
)
{
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   March 2001

    int lHeatReturnNodeNum = this->HeatReturnNodeNum;
    int lHeatSupplyNodeNum = this->HeatSupplyNodeNum;

    if (MyLoad == 0 || !RunFlag) {
        state.dataLoopNodes->Node(lHeatSupplyNodeNum).Temp = state.dataLoopNodes->Node(lHeatReturnNodeNum).Temp;
    } else {
        state.dataLoopNodes->Node(lHeatSupplyNodeNum).Temp = this->HotWaterSupplyTemp;
    }

    // convert power to energy and instantaneous use to use over the time step
    this->HeatingEnergy = this->HeatingLoad * state.dataHVACGlobal->TimeStepSysSec;
    this->FuelEnergy = this->FuelUseRate * state.dataHVACGlobal->TimeStepSysSec;
    this->HeatFuelEnergy = this->HeatFuelUseRate * state.dataHVACGlobal->TimeStepSysSec;
    this->ElectricEnergy = this->ElectricPower * state.dataHVACGlobal->TimeStepSysSec;
    this->HeatElectricEnergy = this->HeatElectricPower * state.dataHVACGlobal->TimeStepSysSec;
}

void GasAbsorberSpecs::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::ChillerGasAbsorption
