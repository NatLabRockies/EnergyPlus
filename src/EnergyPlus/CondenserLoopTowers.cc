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
// #include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CondenserLoopTowers.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>

namespace EnergyPlus {

namespace CondenserLoopTowers {

    // Module containing the routines dealing with the objects COOLING TOWER:SINGLE SPEED,
    // COOLING TOWER:TWO SPEED, and COOLING TOWER:VARIABLE SPEED

    // MODULE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   April 1998
    //       MODIFIED       Shirey, Raustad: Dec 2000; Shirey, Sept 2002, Raustad Mar 2005
    //                      B Griffith Aug 2006, added water consumption and water system interactions
    //                      T Hong, Aug 2008. Added fluid bypass for single speed cooling tower
    //                      Chandan Sharma, FSEC, February 2010, Added basin heater
    //                      A Flament, July 2010, added multi-cell capability for the 3 types of cooling tower
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Model the performance of cooling towers

    std::string const cCoolingTower_SingleSpeed("CoolingTower:SingleSpeed");
    std::string const cCoolingTower_TwoSpeed("CoolingTower:TwoSpeed");
    std::string const cCoolingTower_VariableSpeed("CoolingTower:VariableSpeed");
    std::string const cCoolingTower_VariableSpeedMerkel("CoolingTower:VariableSpeed:Merkel");

    CoolingTower *CoolingTower::factory(EnergyPlusData &state, std::string_view objectName)
    {
        // Process the input data for towers if it hasn't been done already
        if (state.dataCondenserLoopTowers->GetInput) {
            GetTowerInput(state);
            state.dataCondenserLoopTowers->GetInput = false;
        }
        // Now look for this particular tower in the list
        auto thisObj = std::find_if(state.dataCondenserLoopTowers->towers.begin(),
                                    state.dataCondenserLoopTowers->towers.end(),
                                    [&objectName](const CoolingTower &myObj) { return myObj.Name == objectName; });
        if (thisObj != state.dataCondenserLoopTowers->towers.end()) {
            return thisObj;
        }
        // If we didn't find it, fatal
        ShowFatalError(state, format("CoolingTowerFactory: Error getting inputs for tower named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void CoolingTower::simulate(EnergyPlusData &state,
                                [[maybe_unused]] const PlantLocation &calledFromLocation,
                                [[maybe_unused]] bool const FirstHVACIteration,
                                Real64 &CurLoad,
                                bool const RunFlag)
    {
        this->initialize(state);
        switch (this->TowerType) {
        case DataPlant::PlantEquipmentType::CoolingTower_SingleSpd:
            this->calculateSingleSpeedTower(state, CurLoad, RunFlag);
            break;
        case DataPlant::PlantEquipmentType::CoolingTower_TwoSpd:
            this->calculateTwoSpeedTower(state, CurLoad, RunFlag);
            break;
        case DataPlant::PlantEquipmentType::CoolingTower_VarSpd:
            this->calculateVariableSpeedTower(state, CurLoad, RunFlag);
            break;
        case DataPlant::PlantEquipmentType::CoolingTower_VarSpdMerkel:
            this->calculateMerkelVariableSpeedTower(state, CurLoad, RunFlag);
            break;
        default:
            ShowFatalError(state, format("Plant Equipment Type specified for {} is not a Cooling Tower.", this->Name));
        }
        this->calculateWaterUsage(state);
        this->update(state);
        this->report(state, RunFlag);
    }

    void CoolingTower::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                           [[maybe_unused]] const PlantLocation &calledFromLocation,
                                           Real64 &MaxLoad,
                                           Real64 &MinLoad,
                                           Real64 &OptLoad)
    {
        MinLoad = 0.0;
        MaxLoad = this->TowerNominalCapacity * this->HeatRejectCapNomCapSizingRatio;
        OptLoad = this->TowerNominalCapacity;
    }

    void CoolingTower::getSizingFactor(Real64 &SizFactor)
    {
        SizFactor = this->SizFac;
    }

    void CoolingTower::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
    {
        this->initialize(state);
        if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_VarSpdMerkel) {
            this->SizeVSMerkelTower(state);
        } else {
            this->SizeTower(state);
        }
    }

    void GetTowerInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    April 1998
        //       MODIFIED         Don Shirey, Jan 2001 and Sept/Oct 2002; Richard Raustad, FSEC, Feb 2005 (added VS tower)
        //                        B. Griffith, Aug. 2006 water consumption modeling and water system connections
        //                        T Hong, Aug. 2008: added fluid bypass for single speed tower
        //                        A Flament, July 2010, added multi-cell capability for the 3 types of cooling tower

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for cooling towers and stores it in towers data structure. Additional structure
        // (VSTower) stores the coefficients for each VS tower.

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in the data.

        static constexpr std::string_view routineName = "GetTowerInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int TowerNum;                      // Tower number, reference counter for towers data array
        int NumVSCoolToolsModelCoeffs = 0; // Number of CoolTools VS cooling tower coefficient objects
        int NumVSYorkCalcModelCoeffs = 0;  // Number of YorkCalc VS cooling tower coefficient objects
        int VSModelCoeffNum;               // Specific variable-speed tower coefficient object of interest
        int NumAlphas;                     // Number of elements in the alpha array
        int NumNums;                       // Number of elements in the numeric array
        int NumAlphas2;                    // Number of elements in the alpha2 array
        int NumNums2;                      // Number of elements in the numeric2 array
        int IOStat;                        // IO Status when calling get input subroutine
        int CoeffNum;                      // Index for reading user defined VS tower coefficients
        bool ErrorsFound(false);           // Logical flag set .TRUE. if errors found while getting input data
        Array1D<Real64> NumArray(33);      // Numeric input data array
        Array1D<Real64> NumArray2(43);     // Numeric input data array for VS tower coefficients
        Array1D_string AlphArray(16);      // Character string input data array
        Array1D_string AlphArray2(1);      // Character string input data array for VS tower coefficients

        std::unordered_map<std::string, std::string> UniqueSimpleTowerNames;

        constexpr std::array<std::string_view, static_cast<int>(EvapLoss::Num)> EvapLossNamesUC{"LOSSFACTOR", "SATURATEDEXIT"};
        constexpr std::array<std::string_view, static_cast<int>(PIM::Num)> PIMNamesUC{"NOMINALCAPACITY", "UFACTORTIMESAREAANDDESIGNWATERFLOWRATE"};
        constexpr std::array<std::string_view, static_cast<int>(Blowdown::Num)> BlowDownNamesUC = {"CONCENTRATIONRATIO", "SCHEDULEDRATE"};
        constexpr std::array<std::string_view, static_cast<int>(CellCtrl::Num)> CellCtrlNamesUC = {"MINIMALCELL", "MAXIMALCELL"};

        auto const &s_ip = state.dataInputProcessing->inputProcessor;
        auto &s_ipsc = state.dataIPShortCut;

        // Get number of all cooling towers specified in the input data file (idf)
        int NumSingleSpeedTowers = s_ip->getNumObjectsFound(state, cCoolingTower_SingleSpeed);
        int NumTwoSpeedTowers = s_ip->getNumObjectsFound(state, cCoolingTower_TwoSpeed);
        int NumVariableSpeedTowers = s_ip->getNumObjectsFound(state, cCoolingTower_VariableSpeed);
        int NumVSMerkelTowers = s_ip->getNumObjectsFound(state, cCoolingTower_VariableSpeedMerkel);
        int NumSimpleTowers = NumSingleSpeedTowers + NumTwoSpeedTowers + NumVariableSpeedTowers + NumVSMerkelTowers;

        if (NumSimpleTowers <= 0) {
            ShowFatalError(state,
                           "No Cooling Tower objects found in input, however, a branch object has specified a cooling tower. Search the input for "
                           "CoolingTower to determine the cause for this error.");
        }

        state.dataCondenserLoopTowers->GetInput = false;
        // See if load distribution manager has already gotten the input
        if (allocated(state.dataCondenserLoopTowers->towers)) {
            return;
        }

        // Allocate data structures to hold tower input data, report data and tower inlet conditions
        state.dataCondenserLoopTowers->towers.allocate(NumSimpleTowers);
        UniqueSimpleTowerNames.reserve(NumSimpleTowers);
        // Allocate variable-speed tower structure with data specific to this type
        if (NumVariableSpeedTowers > 0) {
            // Allow users to input model coefficients other than default
            NumVSCoolToolsModelCoeffs = s_ip->getNumObjectsFound(state, "CoolingTowerPerformance:CoolTools");
            NumVSYorkCalcModelCoeffs = s_ip->getNumObjectsFound(state, "CoolingTowerPerformance:YorkCalc");
        }

        // Load data structures with cooling tower input data
        s_ipsc->cCurrentModuleObject = cCoolingTower_SingleSpeed;
        for (int SingleSpeedTowerNumber = 1; SingleSpeedTowerNumber <= NumSingleSpeedTowers; ++SingleSpeedTowerNumber) {
            TowerNum = SingleSpeedTowerNumber;
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                SingleSpeedTowerNumber,
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

            GlobalNames::VerifyUniqueInterObjectName(
                state, UniqueSimpleTowerNames, AlphArray(1), s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);
            auto &tower = state.dataCondenserLoopTowers->towers(TowerNum);
            tower.Name = AlphArray(1);
            tower.TowerType = DataPlant::PlantEquipmentType::CoolingTower_SingleSpd;
            tower.TowerMassFlowRateMultiplier = 2.5;
            tower.WaterInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                          AlphArray(2),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::CoolingTowerSingleSpeed,
                                                                          tower.Name,
                                                                          DataLoopNode::NodeFluidType::Water,
                                                                          DataLoopNode::ConnectionType::Inlet,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          DataLoopNode::ObjectIsNotParent);
            tower.WaterOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           AlphArray(3),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::CoolingTowerSingleSpeed,
                                                                           tower.Name,
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Outlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state, s_ipsc->cCurrentModuleObject, tower.Name, AlphArray(2), AlphArray(3), "Chilled Water Nodes");
            tower.DesignWaterFlowRate = NumArray(1);
            if (tower.DesignWaterFlowRate == DataSizing::AutoSize) {
                tower.DesignWaterFlowRateWasAutoSized = true;
            }
            tower.HighSpeedAirFlowRate = NumArray(2);
            if (tower.HighSpeedAirFlowRate == DataSizing::AutoSize) {
                tower.HighSpeedAirFlowRateWasAutoSized = true;
            }
            tower.HighSpeedFanPower = NumArray(3);
            if (tower.HighSpeedFanPower == DataSizing::AutoSize) {
                tower.HighSpeedFanPowerWasAutoSized = true;
            }
            tower.HighSpeedTowerUA = NumArray(4);
            if (tower.HighSpeedTowerUA == DataSizing::AutoSize) {
                tower.HighSpeedTowerUAWasAutoSized = true;
            }
            tower.FreeConvAirFlowRate = NumArray(5);
            if (tower.FreeConvAirFlowRate == DataSizing::AutoSize) {
                tower.FreeConvAirFlowRateWasAutoSized = true;
            }
            tower.FreeConvAirFlowRateSizingFactor = NumArray(6);
            tower.FreeConvTowerUA = NumArray(7);
            if (tower.FreeConvTowerUA == DataSizing::AutoSize) {
                tower.FreeConvTowerUAWasAutoSized = true;
            }
            tower.FreeConvTowerUASizingFactor = NumArray(8);
            tower.HeatRejectCapNomCapSizingRatio = NumArray(9);
            tower.TowerNominalCapacity = NumArray(10);
            if (tower.TowerNominalCapacity == DataSizing::AutoSize) {
                tower.TowerNominalCapacityWasAutoSized = true;
            }
            tower.TowerFreeConvNomCap = NumArray(11);
            if (tower.TowerFreeConvNomCap == DataSizing::AutoSize) {
                tower.TowerFreeConvNomCapWasAutoSized = true;
            }
            tower.TowerFreeConvNomCapSizingFactor = NumArray(12);
            if (NumAlphas >= 4) {
                tower.PerformanceInputMethod_Num = static_cast<PIM>(getEnumValue(PIMNamesUC, Util::makeUPPER(AlphArray(4))));
                if (tower.PerformanceInputMethod_Num == PIM::Invalid) {
                    ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), AlphArray(4));
                    ErrorsFound = true;
                }
            } else {
                // Since Performance Input Method has been omitted then assume it to be UA and DESIGN WATER FLOW RATE
                tower.PerformanceInputMethod_Num = PIM::UFactor;
            }
            // cooling tower design inlet conditions
            tower.DesInletAirDBTemp = NumArray(13);
            if (tower.DesInletAirDBTemp == 0) {
                tower.DesInletAirDBTemp = 35.0;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignInletWB = NumArray(14);
            if (tower.DesignInletWB == 0) {
                tower.DesignInletWB = 25.6;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignApproach = NumArray(15);
            if (tower.DesignApproach == DataSizing::AutoSize || tower.DesignApproach == 0) {
                tower.DesignApproach = 3.9;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignRange = NumArray(16);
            if (tower.DesignRange == DataSizing::AutoSize || tower.DesignRange == 0) {
                tower.DesignRange = 5.5;
                tower.TowerInletCondsAutoSize = true;
            }
            // set tower design water outlet and inlet temperatures
            tower.DesOutletWaterTemp = tower.DesignInletWB + tower.DesignApproach;
            tower.DesInletWaterTemp = tower.DesOutletWaterTemp + tower.DesignRange;
            //   Basin heater power as a function of temperature must be greater than or equal to 0
            tower.BasinHeaterPowerFTempDiff = NumArray(17);
            if (NumArray(17) < 0.0) {
                ShowSevereCustom(state, eoh, "Basin heater power as a function of temperature difference must be >= 0");
                ErrorsFound = true;
            }

            tower.BasinHeaterSetPointTemp = NumArray(18);

            if (tower.BasinHeaterPowerFTempDiff > 0.0) {
                if (NumNums < 18) {
                    tower.BasinHeaterSetPointTemp = 2.0;
                }
                if (tower.BasinHeaterSetPointTemp < 2.0) {
                    ShowWarningCustom(state, eoh, format("{} is less than 2 deg C. Freezing could occur.", s_ipsc->cNumericFieldNames(18)));
                }
            }

            if (!AlphArray(5).empty()) {
                if ((tower.basinHeaterSched = Sched::GetSchedule(state, AlphArray(5))) == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            s_ipsc->cAlphaFieldNames(5),
                                            AlphArray(5),
                                            "Basin heater operation will not be modeled and the simulation continues");
                }
            }

            // begin water use and systems get input
            tower.EvapLossMode = static_cast<EvapLoss>(getEnumValue(EvapLossNamesUC, Util::makeUPPER(AlphArray(6))));

            tower.UserEvapLossFactor = NumArray(19);        //  N11 , \field Evaporation Loss Factor
            tower.DriftLossFraction = NumArray(20) / 100.0; //  N12, \field Drift Loss Percent
            tower.ConcentrationRatio = NumArray(21);        //  N13, \field Blowdown Concentration Ratio
            tower.SizFac = NumArray(25);                    //  N17  \field Sizing Factor
            if (tower.SizFac <= 0.0) {
                tower.SizFac = 1.0;
            }

            tower.BlowdownMode = static_cast<Blowdown>(getEnumValue(BlowDownNamesUC, Util::makeUPPER(AlphArray(7))));
            if (tower.BlowdownMode == Blowdown::Schedule) {
                if ((tower.blowdownSched = Sched::GetSchedule(state, AlphArray(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(8), AlphArray(8));
                    ErrorsFound = true;
                }
            }

            if (AlphArray(9).empty()) {
                tower.SuppliedByWaterSystem = false;
            } else { // water from storage tank
                WaterManager::SetupTankDemandComponent(
                    state, AlphArray(1), s_ipsc->cCurrentModuleObject, AlphArray(9), ErrorsFound, tower.WaterTankID, tower.WaterTankDemandARRID);
                tower.SuppliedByWaterSystem = true;
            }

            //   outdoor air inlet node

            if (s_ipsc->lAlphaFieldBlanks(10)) {
                tower.OutdoorAirInletNodeNum = 0;
            } else {
                tower.OutdoorAirInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                   AlphArray(10),
                                                                                   ErrorsFound,
                                                                                   DataLoopNode::ConnectionObjectType::CoolingTowerSingleSpeed,
                                                                                   tower.Name,
                                                                                   DataLoopNode::NodeFluidType::Air,
                                                                                   DataLoopNode::ConnectionType::OutsideAirReference,
                                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                                   DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, tower.OutdoorAirInletNodeNum)) {
                    ShowSevereCustom(state,
                                     eoh,
                                     format("Outdoor Air Inlet Node Name not valid Outdoor Air Node= {}"
                                            "does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.",
                                            AlphArray(10)));
                    ErrorsFound = true;
                }
            }

            //   fluid bypass for single speed tower
            if (s_ipsc->lAlphaFieldBlanks(11) || AlphArray(11).empty()) {
                tower.CapacityControl = CapacityCtrl::FanCycling; // FanCycling
            } else if ((tower.CapacityControl = static_cast<CapacityCtrl>(getEnumValue(CapacityCtrlNamesUC, AlphArray(11)))) ==
                       CapacityCtrl::Invalid) {
                tower.CapacityControl = CapacityCtrl::FanCycling;
                ShowWarningInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(11), AlphArray(11), "The default Fan Cycling is used.");
            }

            // added for multi-cell
            tower.NumCell = NumArray(22);
            if ((NumNums < 22) && (tower.NumCell == 0)) {
                // assume Number of Cells not entered and should be defaulted
                tower.NumCell = 1;
            }
            tower.MinFracFlowRate = NumArray(23);
            if ((NumNums < 23) && (tower.MinFracFlowRate == 0.0)) {
                // assume Cell Minimum Water Flow Rate Fraction not entered and should be defaulted
                tower.MinFracFlowRate = 0.33;
            }
            tower.MaxFracFlowRate = NumArray(24);
            if ((NumNums < 24) && (tower.MaxFracFlowRate == 0.0)) {
                // assume Cell Maximum Water Flow Rate Fraction not entered and should be defaulted
                tower.MaxFracFlowRate = 2.5;
            }

            //   cell control for single speed tower
            if (!s_ipsc->lAlphaFieldBlanks(12)) {
                tower.cellCtrl = static_cast<CellCtrl>(getEnumValue(CellCtrlNamesUC, Util::makeUPPER(AlphArray(12))));
            }

            //   High speed air flow rate must be greater than free convection air flow rate.
            //   Can't tell yet if autosized, check later in initialize.
            if (tower.HighSpeedAirFlowRate <= tower.FreeConvAirFlowRate && tower.HighSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereCustom(state, eoh, "Free convection air flow rate must be less than the design air flow rate.");
                ErrorsFound = true;
            }

            //   Check various inputs if Performance Input Method = "UA and Design Water Flow Rate"
            if (tower.PerformanceInputMethod_Num == PIM::UFactor) {
                if (tower.DesignWaterFlowRate == 0.0) {
                    ShowSevereCustom(state, eoh, "Tower performance input method requires a design water flow rate greater than zero.");
                    ErrorsFound = true;
                }
                if (tower.HighSpeedTowerUA <= tower.FreeConvTowerUA && tower.HighSpeedTowerUA != DataSizing::AutoSize) {
                    ShowSevereCustom(state, eoh, "Free convection UA must be less than the design tower UA.");
                    ErrorsFound = true;
                }
                if (tower.FreeConvTowerUA > 0.0 && tower.FreeConvAirFlowRate == 0.0) {
                    ShowSevereCustom(
                        state, eoh, "Free convection air flow rate must be greater than zero when free convection UA is greater than zero.");
                    ErrorsFound = true;
                }
            } else if (tower.PerformanceInputMethod_Num == PIM::NominalCapacity) {
                if (tower.TowerNominalCapacity == 0.0) {
                    ShowSevereCustom(state, eoh, "Tower performance input method requires valid nominal capacity.");
                    ErrorsFound = true;
                }
                if (tower.DesignWaterFlowRate != 0.0) {
                    if (tower.DesignWaterFlowRate > 0.0) {
                        ShowWarningCustom(state, eoh, "Nominal capacity input method and design water flow rate have been specified.");
                    } else {
                        ShowSevereCustom(
                            state, eoh, "Nominal capacity input method has been specified and design water flow rate is being autosized.");
                    }
                    ShowContinueError(state, "Design water flow rate will be set according to nominal tower capacity.");
                }
                if (tower.HighSpeedTowerUA != 0.0) {
                    if (tower.HighSpeedTowerUA > 0.0) {
                        ShowWarningCustom(state, eoh, "Nominal tower capacity and design tower UA have been specified.");
                    } else {
                        ShowSevereCustom(state, eoh, "Nominal tower capacity has been specified and design tower UA is being autosized.");
                    }
                    ShowContinueError(state, "Design tower UA will be set according to nominal tower capacity.");
                }
                if (tower.FreeConvTowerUA != 0.0) {
                    if (tower.FreeConvTowerUA > 0.0) {
                        ShowWarningCustom(state, eoh, "Nominal capacity input method and free convection UA have been specified.");
                    } else {
                        ShowSevereCustom(state, eoh, "Nominal capacity input method has been specified and free convection UA is being autosized.");
                    }
                    ShowContinueError(state, "Free convection UA will be set according to nominal tower capacity.");
                }
                if (tower.TowerFreeConvNomCap >= tower.TowerNominalCapacity) {
                    ShowSevereCustom(state, eoh, "Free convection nominal capacity must be less than the nominal (design) tower capacity.");
                    ErrorsFound = true;
                }
                if (tower.TowerFreeConvNomCap > 0.0 && tower.FreeConvAirFlowRate == 0.0) {
                    ShowSevereCustom(
                        state, eoh, "Free convection air flow must be greater than zero when tower free convection capacity is specified.");
                    ErrorsFound = true;
                }
            } else { // Tower performance input method is not specified as a valid "choice"
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), AlphArray(4));
                ErrorsFound = true;
            }

            if (NumAlphas > 12) {
                tower.EndUseSubcategory = AlphArray(13);
            } else {
                tower.EndUseSubcategory = "General";
            }
        } // End Single-Speed Tower Loop

        s_ipsc->cCurrentModuleObject = cCoolingTower_TwoSpeed;
        for (int TwoSpeedTowerNumber = 1; TwoSpeedTowerNumber <= NumTwoSpeedTowers; ++TwoSpeedTowerNumber) {
            TowerNum = NumSingleSpeedTowers + TwoSpeedTowerNumber;
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                TwoSpeedTowerNumber,
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

            GlobalNames::VerifyUniqueInterObjectName(
                state, UniqueSimpleTowerNames, AlphArray(1), s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);

            auto &tower = state.dataCondenserLoopTowers->towers(TowerNum);
            tower.Name = AlphArray(1);
            tower.TowerType = DataPlant::PlantEquipmentType::CoolingTower_TwoSpd;
            tower.TowerMassFlowRateMultiplier = 2.5;
            tower.WaterInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                          AlphArray(2),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::CoolingTowerTwoSpeed,
                                                                          tower.Name,
                                                                          DataLoopNode::NodeFluidType::Water,
                                                                          DataLoopNode::ConnectionType::Inlet,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          DataLoopNode::ObjectIsNotParent);
            tower.WaterOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           AlphArray(3),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::CoolingTowerTwoSpeed,
                                                                           tower.Name,
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Outlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state, s_ipsc->cCurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Chilled Water Nodes");

            if (NumAlphas >= 4) {
                tower.PerformanceInputMethod_Num = static_cast<PIM>(getEnumValue(PIMNamesUC, Util::makeUPPER(AlphArray(4))));
            } else {
                // Since Performance Input Method has been omitted then assume it to be UA and DESIGN WATER FLOW RATE
                tower.PerformanceInputMethod_Num = PIM::UFactor;
            }
            tower.DesignWaterFlowRate = NumArray(1);
            if (tower.DesignWaterFlowRate == DataSizing::AutoSize) {
                tower.DesignWaterFlowRateWasAutoSized = true;
            }
            tower.HighSpeedAirFlowRate = NumArray(2);
            if (tower.HighSpeedAirFlowRate == DataSizing::AutoSize) {
                tower.HighSpeedAirFlowRateWasAutoSized = true;
            }
            tower.HighSpeedFanPower = NumArray(3);
            if (tower.HighSpeedFanPower == DataSizing::AutoSize) {
                tower.HighSpeedFanPowerWasAutoSized = true;
            }
            tower.HighSpeedTowerUA = NumArray(4);
            if (tower.HighSpeedTowerUA == DataSizing::AutoSize) {
                tower.HighSpeedTowerUAWasAutoSized = true;
            }
            tower.LowSpeedAirFlowRate = NumArray(5);
            if (tower.LowSpeedAirFlowRate == DataSizing::AutoSize) {
                tower.LowSpeedAirFlowRateWasAutoSized = true;
            }

            tower.LowSpeedAirFlowRateSizingFactor = NumArray(6);
            tower.LowSpeedFanPower = NumArray(7);
            if (tower.LowSpeedFanPower == DataSizing::AutoSize) {
                tower.LowSpeedFanPowerWasAutoSized = true;
            }
            tower.LowSpeedFanPowerSizingFactor = NumArray(8);
            tower.LowSpeedTowerUA = NumArray(9);
            if (tower.LowSpeedTowerUA == DataSizing::AutoSize) {
                tower.LowSpeedTowerUAWasAutoSized = true;
            }
            tower.LowSpeedTowerUASizingFactor = NumArray(10);
            tower.FreeConvAirFlowRate = NumArray(11);
            if (tower.FreeConvAirFlowRate == DataSizing::AutoSize) {
                tower.FreeConvAirFlowRateWasAutoSized = true;
            }
            tower.FreeConvAirFlowRateSizingFactor = NumArray(12);
            tower.FreeConvTowerUA = NumArray(13);
            if (tower.FreeConvTowerUA == DataSizing::AutoSize) {
                tower.FreeConvTowerUAWasAutoSized = true;
            }
            tower.FreeConvTowerUASizingFactor = NumArray(14);
            tower.HeatRejectCapNomCapSizingRatio = NumArray(15);
            tower.TowerNominalCapacity = NumArray(16);

            tower.TowerLowSpeedNomCap = NumArray(17);
            if (tower.TowerLowSpeedNomCap == DataSizing::AutoSize) {
                tower.TowerLowSpeedNomCapWasAutoSized = true;
            }
            tower.TowerLowSpeedNomCapSizingFactor = NumArray(18);
            tower.TowerFreeConvNomCap = NumArray(19);
            if (tower.TowerFreeConvNomCap == DataSizing::AutoSize) {
                tower.TowerFreeConvNomCapWasAutoSized = true;
            }
            tower.TowerFreeConvNomCapSizingFactor = NumArray(20);
            // cooling tower design inlet conditions
            tower.DesInletAirDBTemp = NumArray(21);
            if (tower.DesInletAirDBTemp == 0) {
                tower.DesInletAirDBTemp = 35.0;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignInletWB = NumArray(22);
            if (tower.DesignInletWB == 0) {
                tower.DesignInletWB = 25.6;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignApproach = NumArray(23);
            if (tower.DesignApproach == DataSizing::AutoSize || tower.DesignApproach == 0) {
                tower.DesignApproach = 3.9;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignRange = NumArray(24);
            if (tower.DesignRange == DataSizing::AutoSize || tower.DesignRange == 0) {
                tower.DesignRange = 5.5;
                tower.TowerInletCondsAutoSize = true;
            }
            // set tower design water outlet and inlet temperatures
            tower.DesOutletWaterTemp = tower.DesignInletWB + tower.DesignApproach;
            tower.DesInletWaterTemp = tower.DesOutletWaterTemp + tower.DesignRange;
            //   Basin heater power as a function of temperature must be greater than or equal to 0
            tower.BasinHeaterPowerFTempDiff = NumArray(25);
            if (NumArray(25) < 0.0) {
                ShowSevereCustom(state, eoh, "Basin heater power as a function of temperature difference must be >= 0");
                ErrorsFound = true;
            }

            tower.BasinHeaterSetPointTemp = NumArray(26);
            if (tower.BasinHeaterPowerFTempDiff > 0.0) {
                if (NumNums < 26) {
                    tower.BasinHeaterSetPointTemp = 2.0;
                }
                if (tower.BasinHeaterSetPointTemp < 2.0) {
                    ShowWarningCustom(state, eoh, format("{} is less than 2 deg C. Freezing could occur.", s_ipsc->cNumericFieldNames(26)));
                }
            }

            if (!AlphArray(5).empty()) {
                if ((tower.basinHeaterSched = Sched::GetSchedule(state, AlphArray(5))) == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            s_ipsc->cAlphaFieldNames(5),
                                            AlphArray(5),
                                            "Basin heater operation will not be modeled and the simulation continues");
                }
            }

            // begin water use and systems get input
            tower.EvapLossMode = static_cast<EvapLoss>(getEnumValue(EvapLossNamesUC, Util::makeUPPER(AlphArray(6))));
            tower.UserEvapLossFactor = NumArray(27);        //  N23 , \field Evaporation Loss Factor
            tower.DriftLossFraction = NumArray(28) / 100.0; //  N24, \field Drift Loss Percent
            tower.ConcentrationRatio = NumArray(29);        //  N17, \field Blowdown Concentration Ratio
            tower.SizFac = NumArray(33);                    //  N21  \field Sizing Factor
            if (tower.SizFac <= 0.0) {
                tower.SizFac = 1.0;
            }

            tower.BlowdownMode = static_cast<Blowdown>(getEnumValue(BlowDownNamesUC, Util::makeUPPER(AlphArray(7))));
            if (tower.BlowdownMode == Blowdown::Schedule) {
                if ((tower.blowdownSched = Sched::GetSchedule(state, AlphArray(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(8), AlphArray(8));
                    ErrorsFound = true;
                }
            }

            // added for multi-cell
            tower.NumCell = NumArray(30);
            if ((NumNums < 30) && (tower.NumCell == 0)) {
                // assume Number of Cells not entered and should be defaulted
                tower.NumCell = 1;
            }
            tower.MinFracFlowRate = NumArray(31);
            if ((NumNums < 31) && (tower.MinFracFlowRate == 0.0)) {
                // assume Cell Minimum Water Flow Rate Fraction not entered and should be defaulted
                tower.MinFracFlowRate = 0.33;
            }
            tower.MaxFracFlowRate = NumArray(32);
            if ((NumNums < 32) && (tower.MaxFracFlowRate == 0.0)) {
                // assume Cell Maximum Water Flow Rate Fraction not entered and should be defaulted
                tower.MaxFracFlowRate = 2.5;
            }

            //   cell control for two speed tower
            if (!s_ipsc->lAlphaFieldBlanks(11)) {
                tower.cellCtrl = static_cast<CellCtrl>(getEnumValue(CellCtrlNamesUC, Util::makeUPPER(AlphArray(11))));
            }

            if (s_ipsc->lAlphaFieldBlanks(9)) {
                tower.SuppliedByWaterSystem = false;
            } else { // water from storage tank
                WaterManager::SetupTankDemandComponent(
                    state, AlphArray(1), s_ipsc->cCurrentModuleObject, AlphArray(9), ErrorsFound, tower.WaterTankID, tower.WaterTankDemandARRID);
                tower.SuppliedByWaterSystem = true;
            }

            //   outdoor air inlet node
            if (s_ipsc->lAlphaFieldBlanks(10)) {
                tower.OutdoorAirInletNodeNum = 0;
            } else {
                tower.OutdoorAirInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                   AlphArray(10),
                                                                                   ErrorsFound,
                                                                                   DataLoopNode::ConnectionObjectType::CoolingTowerTwoSpeed,
                                                                                   tower.Name,
                                                                                   DataLoopNode::NodeFluidType::Air,
                                                                                   DataLoopNode::ConnectionType::OutsideAirReference,
                                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                                   DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, tower.OutdoorAirInletNodeNum)) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(10), AlphArray(10));
                    ErrorsFound = true;
                }
            }

            //   High speed air flow rate must be greater than low speed air flow rate.
            //   Can't tell yet if autosized, check later in initialize.
            if (tower.HighSpeedAirFlowRate <= tower.LowSpeedAirFlowRate && tower.HighSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} \"{}\". Low speed air flow rate must be less than the high speed air flow rate.",
                                       s_ipsc->cCurrentModuleObject,
                                       tower.Name));
                ErrorsFound = true;
            }
            //   Low speed air flow rate must be greater than free convection air flow rate.
            //   Can't tell yet if autosized, check later in initialize.
            if (tower.LowSpeedAirFlowRate <= tower.FreeConvAirFlowRate && tower.LowSpeedAirFlowRate != DataSizing::AutoSize) {
                ShowSevereError(state,
                                format("{} \"{}\". Free convection air flow rate must be less than the low speed air flow rate.",
                                       s_ipsc->cCurrentModuleObject,
                                       tower.Name));
                ErrorsFound = true;
            }

            //   Check various inputs if Performance Input Method = "UA and Design Water Flow Rate"
            if (tower.PerformanceInputMethod_Num == PIM::UFactor) {
                if (tower.DesignWaterFlowRate == 0.0) {
                    ShowSevereError(state,
                                    format("{} \"{}\". Tower performance input method requires a design water flow rate greater than zero.",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name));
                    ErrorsFound = true;
                }
                if (tower.HighSpeedTowerUA <= tower.LowSpeedTowerUA && tower.HighSpeedTowerUA != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} \"{}\". Tower UA at low fan speed must be less than the tower UA at high fan speed.",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name));
                    ErrorsFound = true;
                }
                if (tower.LowSpeedTowerUA <= tower.FreeConvTowerUA && tower.LowSpeedTowerUA != DataSizing::AutoSize) {
                    ShowSevereError(state,
                                    format("{} \"{}\". Tower UA at free convection air flow rate must be less than the tower UA at low fan speed.",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name));
                    ErrorsFound = true;
                }
                if (tower.FreeConvTowerUA > 0.0 && tower.FreeConvAirFlowRate == 0.0) {
                    ShowSevereError(
                        state,
                        format("{} \"{}\". Free convection air flow rate must be greater than zero when free convection UA is greater than zero.",
                               s_ipsc->cCurrentModuleObject,
                               tower.Name));
                    ErrorsFound = true;
                }
            } else if (tower.PerformanceInputMethod_Num == PIM::NominalCapacity) {
                if (tower.TowerNominalCapacity == 0.0) {
                    ShowSevereError(state,
                                    format("{} \"{}\". Tower performance input method requires valid high-speed nominal capacity.",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name));
                    ErrorsFound = true;
                }
                if (tower.TowerLowSpeedNomCap == 0.0) {
                    ShowSevereError(state,
                                    format("{} \"{}\". Tower performance input method requires valid low-speed nominal capacity.",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name));
                    ErrorsFound = true;
                }
                if (tower.DesignWaterFlowRate != 0.0) {
                    if (tower.DesignWaterFlowRate > 0.0) {
                        ShowWarningError(state,
                                         format("{} \"{}\". Nominal capacity input method and design water flow rate have been specified.",
                                                s_ipsc->cCurrentModuleObject,
                                                tower.Name));
                    } else {
                        ShowSevereError(
                            state,
                            format("{} \"{}\". Nominal capacity input method has been specified and design water flow rate is being autosized.",
                                   s_ipsc->cCurrentModuleObject,
                                   tower.Name));
                    }
                    ShowContinueError(state, "Design water flow rate will be set according to nominal tower capacity.");
                }
                if (tower.HighSpeedTowerUA != 0.0) {
                    if (tower.HighSpeedTowerUA > 0.0) {
                        ShowWarningError(state,
                                         format("{} \"{}\". Nominal capacity input method and tower UA at high fan speed have been specified.",
                                                s_ipsc->cCurrentModuleObject,
                                                tower.Name));
                    } else {
                        ShowSevereError(
                            state,
                            format("{} \"{}\". Nominal capacity input method has been specified and tower UA at high fan speed is being autosized.",
                                   s_ipsc->cCurrentModuleObject,
                                   tower.Name));
                    }
                    ShowContinueError(state, "Tower UA at high fan speed will be set according to nominal tower capacity.");
                }
                if (tower.LowSpeedTowerUA != 0.0) {
                    if (tower.LowSpeedTowerUA > 0.0) {
                        ShowWarningError(state,
                                         format("{} \"{}\". Nominal capacity input method and tower UA at low fan speed have been specified.",
                                                s_ipsc->cCurrentModuleObject,
                                                tower.Name));
                    } else {
                        ShowSevereError(
                            state,
                            format("{} \"{}\". Nominal capacity input method has been specified and tower UA at low fan speed is being autosized.",
                                   s_ipsc->cCurrentModuleObject,
                                   tower.Name));
                    }
                    ShowContinueError(state, "Tower UA at low fan speed will be set according to nominal tower capacity.");
                }
                if (tower.FreeConvTowerUA != 0.0) {
                    if (tower.FreeConvTowerUA > 0.0) {
                        ShowWarningError(state,
                                         format("{} \"{}\". Nominal capacity input method and free convection UA have been specified.",
                                                s_ipsc->cCurrentModuleObject,
                                                tower.Name));
                    } else {
                        ShowSevereError(
                            state,
                            format("{} \"{}\". Nominal capacity input method has been specified and free convection UA is being autosized.",
                                   s_ipsc->cCurrentModuleObject,
                                   tower.Name));
                    }
                    ShowContinueError(state, "Free convection UA will be set according to nominal tower capacity.");
                }
                if (tower.TowerLowSpeedNomCap >= tower.TowerNominalCapacity) {
                    ShowSevereError(state,
                                    format("{} \"{}\". Low-speed nominal capacity must be less than the high-speed nominal capacity.",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name));
                    ErrorsFound = true;
                }
                if (!tower.TowerLowSpeedNomCapWasAutoSized) {
                    if (tower.TowerFreeConvNomCap >= tower.TowerLowSpeedNomCap) {
                        ShowSevereError(state,
                                        format("{} \"{}\". Free convection nominal capacity must be less than the low-speed nominal capacity.",
                                               s_ipsc->cCurrentModuleObject,
                                               tower.Name));
                        ErrorsFound = true;
                    }
                }
                if (tower.TowerFreeConvNomCap > 0.0 && tower.FreeConvAirFlowRate == 0.0) {
                    ShowSevereCustom(
                        state, eoh, "Free convection air flow must be greater than zero when tower free convection capacity is specified.");
                    ErrorsFound = true;
                }
            } else { // Tower performance input method is not specified as a valid "choice"
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), AlphArray(4));
                ErrorsFound = true;
            }

            if (NumAlphas > 11) {
                tower.EndUseSubcategory = AlphArray(12);
            } else {
                tower.EndUseSubcategory = "General";
            }
        } // End Two-Speed Tower Loop

        s_ipsc->cCurrentModuleObject = cCoolingTower_VariableSpeed;
        for (int VariableSpeedTowerNumber = 1; VariableSpeedTowerNumber <= NumVariableSpeedTowers; ++VariableSpeedTowerNumber) {
            TowerNum = NumSingleSpeedTowers + NumTwoSpeedTowers + VariableSpeedTowerNumber;
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                VariableSpeedTowerNumber,
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
            GlobalNames::VerifyUniqueInterObjectName(
                state, UniqueSimpleTowerNames, AlphArray(1), s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);

            auto &tower = state.dataCondenserLoopTowers->towers(TowerNum);
            tower.Name = AlphArray(1);
            tower.TowerType = DataPlant::PlantEquipmentType::CoolingTower_VarSpd;
            tower.WaterInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                          AlphArray(2),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::CoolingTowerVariableSpeed,
                                                                          AlphArray(1),
                                                                          DataLoopNode::NodeFluidType::Water,
                                                                          DataLoopNode::ConnectionType::Inlet,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          DataLoopNode::ObjectIsNotParent);
            tower.WaterOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           AlphArray(3),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::CoolingTowerVariableSpeed,
                                                                           AlphArray(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Outlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state, s_ipsc->cCurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Chilled Water Nodes");

            if ((Util::SameString(AlphArray(4), "CoolToolsUserDefined") || Util::SameString(AlphArray(4), "YorkCalcUserDefined")) &&
                s_ipsc->lAlphaFieldBlanks(5)) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("A {} must be specified when {} is specified as CoolToolsUserDefined or YorkCalcUserDefined",
                                        s_ipsc->cAlphaFieldNames(5),
                                        s_ipsc->cAlphaFieldNames(4)));
                ErrorsFound = true;
            } else if ((Util::SameString(AlphArray(4), "CoolToolsCrossFlow") || Util::SameString(AlphArray(4), "YorkCalc")) &&
                       !s_ipsc->lAlphaFieldBlanks(5)) {
                ShowWarningCustom(state,
                                  eoh,
                                  "A Tower Model Coefficient Name is specified and the Tower Model Type is not specified as "
                                  "CoolToolsUserDefined or YorkCalcUserDefined. The CoolingTowerPerformance:CoolTools "
                                  "(orCoolingTowerPerformance:YorkCalc) data object will not be used.");
            } else {
                tower.ModelCoeffObjectName = AlphArray(5);
            }

            if (!s_ipsc->lAlphaFieldBlanks(6)) {
                tower.FanPowerfAirFlowCurve = Curve::GetCurveIndex(state, AlphArray(6));
                if (tower.FanPowerfAirFlowCurve == 0) {
                    ShowWarningCustom(state,
                                      eoh,
                                      format("The Fan Power Ratio as a function of Air Flow Rate Ratio Curve Name specified as {} was not found."
                                             "Fan Power as a function of Air Flow Rate Ratio will default to Fan Power = (Air Flow Rate Ratio)^3."
                                             "The simulation continues.",
                                             AlphArray(6)));
                }
            }

            auto &vstower = state.dataCondenserLoopTowers->towers(TowerNum);

            if (Util::SameString(AlphArray(4), "CoolToolsCrossFlow")) {
                tower.TowerModelType = ModelType::CoolToolsXFModel;
                //     set cross-flow model coefficients
                //       Outputs approach in C
                vstower.Coeff[0] = 0.52049709836241;
                vstower.Coeff[1] = -10.617046395344;
                vstower.Coeff[2] = 10.7292974722538;
                vstower.Coeff[3] = -2.74988377158227;
                vstower.Coeff[4] = 4.73629943913743;
                vstower.Coeff[5] = -8.25759700874711;
                vstower.Coeff[6] = 1.57640938114136;
                vstower.Coeff[7] = 6.51119643791324;
                vstower.Coeff[8] = 1.50433525206692;
                vstower.Coeff[9] = -3.2888529287801;
                vstower.Coeff[10] = 0.0257786145353773;
                vstower.Coeff[11] = 0.182464289315254;
                vstower.Coeff[12] = -0.0818947291400898;
                vstower.Coeff[13] = -0.215010003996285;
                vstower.Coeff[14] = 0.0186741309635284;
                vstower.Coeff[15] = 0.0536824177590012;
                vstower.Coeff[16] = -0.00270968955115031;
                vstower.Coeff[17] = 0.00112277498589279;
                vstower.Coeff[18] = -0.00127758497497718;
                vstower.Coeff[19] = 0.0000760420796601607;
                vstower.Coeff[20] = 1.43600088336017;
                vstower.Coeff[21] = -0.5198695909109;
                vstower.Coeff[22] = 0.117339576910507;
                vstower.Coeff[23] = 1.50492810819924;
                vstower.Coeff[24] = -0.135898905926974;
                vstower.Coeff[25] = -0.152577581866506;
                vstower.Coeff[26] = -0.0533843828114562;
                vstower.Coeff[27] = 0.00493294869565511;
                vstower.Coeff[28] = -0.00796260394174197;
                vstower.Coeff[29] = 0.000222619828621544;
                vstower.Coeff[30] = -0.0543952001568055;
                vstower.Coeff[31] = 0.00474266879161693;
                vstower.Coeff[32] = -0.0185854671815598;
                vstower.Coeff[33] = 0.00115667701293848;
                vstower.Coeff[34] = 0.000807370664460284;

                //       set minimum and maximum boundaries for CoolTools crossflow model input variables
                vstower.MinInletAirWBTemp = -1.0;
                vstower.MaxInletAirWBTemp = 26.6667;
                vstower.MinRangeTemp = 1.1111;
                vstower.MaxRangeTemp = 11.1111;
                vstower.MinApproachTemp = 1.1111;
                vstower.MaxApproachTemp = 11.1111;
                vstower.MinWaterFlowRatio = 0.75;
                vstower.MaxWaterFlowRatio = 1.25;

            } else if (Util::SameString(AlphArray(4), "YorkCalc")) {
                tower.TowerModelType = ModelType::YorkCalcModel;
                //     set counter-flow model coefficients
                //       Outputs approach in C
                vstower.Coeff[0] = -0.359741205;
                vstower.Coeff[1] = -0.055053608;
                vstower.Coeff[2] = 0.0023850432;
                vstower.Coeff[3] = 0.173926877;
                vstower.Coeff[4] = -0.0248473764;
                vstower.Coeff[5] = 0.00048430224;
                vstower.Coeff[6] = -0.005589849456;
                vstower.Coeff[7] = 0.0005770079712;
                vstower.Coeff[8] = -0.00001342427256;
                vstower.Coeff[9] = 2.84765801111111;
                vstower.Coeff[10] = -0.121765149;
                vstower.Coeff[11] = 0.0014599242;
                vstower.Coeff[12] = 1.680428651;
                vstower.Coeff[13] = -0.0166920786;
                vstower.Coeff[14] = -0.0007190532;
                vstower.Coeff[15] = -0.025485194448;
                vstower.Coeff[16] = 0.0000487491696;
                vstower.Coeff[17] = 0.00002719234152;
                vstower.Coeff[18] = -0.0653766255555556;
                vstower.Coeff[19] = -0.002278167;
                vstower.Coeff[20] = 0.0002500254;
                vstower.Coeff[21] = -0.0910565458;
                vstower.Coeff[22] = 0.00318176316;
                vstower.Coeff[23] = 0.000038621772;
                vstower.Coeff[24] = -0.0034285382352;
                vstower.Coeff[25] = 0.00000856589904;
                vstower.Coeff[26] = -0.000001516821552;

                //       set minimum and maximum boundaries for YorkCalc model input variables
                vstower.MinInletAirWBTemp = -34.4;
                vstower.MaxInletAirWBTemp = 29.4444;
                vstower.MinRangeTemp = 1.1111;
                vstower.MaxRangeTemp = 22.2222;
                vstower.MinApproachTemp = 1.1111;
                vstower.MaxApproachTemp = 40.0;
                vstower.MinWaterFlowRatio = 0.75;
                vstower.MaxWaterFlowRatio = 1.25;
                vstower.MaxLiquidToGasRatio = 8.0;

            } else if (Util::SameString(AlphArray(4), "CoolToolsUserDefined")) {
                tower.TowerModelType = ModelType::CoolToolsUserDefined;
                // Nested Get-input routines below.  Should pull out of here and read in beforehand.
                for (VSModelCoeffNum = 1; VSModelCoeffNum <= NumVSCoolToolsModelCoeffs; ++VSModelCoeffNum) {
                    s_ip->getObjectItem(
                        state, "CoolingTowerPerformance:CoolTools", VSModelCoeffNum, AlphArray2, NumAlphas2, NumArray2, NumNums2, IOStat);

                    if (!Util::SameString(AlphArray2(1), tower.ModelCoeffObjectName)) {
                        continue;
                    }
                    vstower.FoundModelCoeff = true;
                    // verify the correct number of coefficients for the CoolTools model
                    if (NumNums2 != 43) {
                        ShowSevereError(state,
                                        format("CoolingTower:VariableSpeed \"{}\". The number of numeric inputs for object "
                                               "CoolingTowerPerformance:CoolTools \"{}\" must equal 43.",
                                               tower.Name,
                                               tower.ModelCoeffObjectName));
                        ErrorsFound = true;
                    } else {

                        vstower.MinInletAirWBTemp = NumArray2(1);
                        vstower.MaxInletAirWBTemp = NumArray2(2);
                        vstower.MinRangeTemp = NumArray2(3);
                        vstower.MaxRangeTemp = NumArray2(4);
                        vstower.MinApproachTemp = NumArray2(5);
                        vstower.MaxApproachTemp = NumArray2(6);
                        vstower.MinWaterFlowRatio = NumArray2(7);
                        vstower.MaxWaterFlowRatio = NumArray2(8);

                        for (CoeffNum = 9; CoeffNum <= NumNums2; ++CoeffNum) {
                            vstower.Coeff[CoeffNum - 9] = NumArray2(CoeffNum);
                        }
                    }
                    break;
                }
                if (!vstower.FoundModelCoeff) {
                    ShowSevereError(state,
                                    format("CoolingTower:VariableSpeed \"{}\". User defined name for variable speed cooling tower model coefficients "
                                           "object not found = {}",
                                           tower.Name,
                                           tower.ModelCoeffObjectName));
                    ErrorsFound = true;
                }
            } else if (Util::SameString(AlphArray(4), "YorkCalcUserDefined")) {
                tower.TowerModelType = ModelType::YorkCalcUserDefined;
                // Nested Get-input routines below.  Should pull out of here and read in beforehand.
                for (VSModelCoeffNum = 1; VSModelCoeffNum <= NumVSYorkCalcModelCoeffs; ++VSModelCoeffNum) {
                    s_ip->getObjectItem(
                        state, "CoolingTowerPerformance:YorkCalc", VSModelCoeffNum, AlphArray2, NumAlphas2, NumArray2, NumNums2, IOStat);
                    if (!Util::SameString(AlphArray2(1), tower.ModelCoeffObjectName)) {
                        continue;
                    }
                    vstower.FoundModelCoeff = true;
                    // verify the correct number of coefficients for the YorkCalc model
                    if (NumNums2 != 36) {
                        ShowSevereError(state,
                                        format("CoolingTower:VariableSpeed \"{}\". The number of numeric inputs for object "
                                               "CoolingTowerPerformance:YorkCalc \"{}\" must equal 36.",
                                               tower.Name,
                                               tower.ModelCoeffObjectName));
                        ErrorsFound = true;
                    } else {

                        vstower.MinInletAirWBTemp = NumArray2(1);
                        vstower.MaxInletAirWBTemp = NumArray2(2);
                        vstower.MinRangeTemp = NumArray2(3);
                        vstower.MaxRangeTemp = NumArray2(4);
                        vstower.MinApproachTemp = NumArray2(5);
                        vstower.MaxApproachTemp = NumArray2(6);
                        vstower.MinWaterFlowRatio = NumArray2(7);
                        vstower.MaxWaterFlowRatio = NumArray2(8);
                        vstower.MaxLiquidToGasRatio = NumArray2(9);

                        for (CoeffNum = 10; CoeffNum <= NumNums2; ++CoeffNum) {
                            vstower.Coeff[CoeffNum - 10] = NumArray2(CoeffNum);
                        }
                    }
                    break;
                }

                if (!vstower.FoundModelCoeff) {
                    ShowSevereError(state,
                                    format("{} \"{}\". User defined name for variable speed cooling tower model coefficients object not found = {}",
                                           s_ipsc->cCurrentModuleObject,
                                           tower.Name,
                                           tower.ModelCoeffObjectName));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(5), AlphArray(5));
                ErrorsFound = true;
            }

            tower.TowerMassFlowRateMultiplier = vstower.MaxWaterFlowRatio;

            //   check user defined minimums to be greater than 0
            if (vstower.MinApproachTemp < 0.0) {
                ShowSevereCustom(state, eoh, "User defined minimum approach temperature must be > 0");
                ErrorsFound = true;
            }
            if (vstower.MinRangeTemp < 0.0) {
                ShowSevereCustom(state, eoh, "User defined minimum range temperature must be > 0");
                ErrorsFound = true;
            }
            if (vstower.MinWaterFlowRatio < 0.0) {
                ShowSevereCustom(state, eoh, "User defined minimum water flow rate ratio must be > 0");
                ErrorsFound = true;
            }

            //   check that the user defined maximums are greater than the minimums
            if (vstower.MaxApproachTemp < vstower.MinApproachTemp) {
                ShowSevereCustom(state, eoh, "User defined maximum approach temperature must be > the minimum approach temperature");
                ErrorsFound = true;
            }
            if (vstower.MaxRangeTemp < vstower.MinRangeTemp) {
                ShowSevereCustom(state, eoh, "User defined maximum range temperature must be > the minimum range temperature");
                ErrorsFound = true;
            }
            if (vstower.MaxWaterFlowRatio < vstower.MinWaterFlowRatio) {
                ShowSevereCustom(state, eoh, "User defined maximum water flow rate ratio must be > the minimum water flow rate ratio");
                ErrorsFound = true;
            }

            tower.DesignInletWB = NumArray(1);
            if (NumArray(1) < vstower.MinInletAirWBTemp || NumArray(1) > vstower.MaxInletAirWBTemp) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("The design inlet air wet-bulb temperature of {:5.2F}"
                                        "must be within the model limits of {:5.2F} and {:5.2F} degrees C",
                                        tower.DesignInletWB,
                                        vstower.MinInletAirWBTemp,
                                        vstower.MaxInletAirWBTemp));
                ErrorsFound = true;
            }

            tower.DesignApproach = NumArray(2);
            if (NumArray(2) < vstower.MinApproachTemp || NumArray(2) > vstower.MaxApproachTemp) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("The design approach temperature of {:5.2F}"
                                        "must be within the model limits of {:5.2F} and {:5.2F} degrees C",
                                        tower.DesignApproach,
                                        vstower.MinApproachTemp,
                                        vstower.MaxApproachTemp));
                ErrorsFound = true;
            }

            tower.DesignRange = NumArray(3);
            if (NumArray(3) < vstower.MinRangeTemp || NumArray(3) > vstower.MaxRangeTemp) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("The design range temperature of {:5.2F}"
                                        "must be within the model limits of {:5.2F} and {:5.2F} degrees C",
                                        tower.DesignRange,
                                        vstower.MinRangeTemp,
                                        vstower.MaxRangeTemp));
                ErrorsFound = true;
            }

            // set tower design water outlet and inlet temperatures
            tower.DesOutletWaterTemp = tower.DesignInletWB + tower.DesignApproach;
            tower.DesInletWaterTemp = tower.DesOutletWaterTemp + tower.DesignRange;

            tower.DesignWaterFlowRate = NumArray(4);
            if (tower.DesignWaterFlowRate == DataSizing::AutoSize) {
                tower.DesignWaterFlowRateWasAutoSized = true;
            }
            if (NumArray(4) <= 0.0 && NumArray(4) != DataSizing::AutoSize) {
                ShowSevereCustom(state, eoh, "Design water flow rate must be > 0");
                ErrorsFound = true;
            }

            tower.HighSpeedAirFlowRate = NumArray(5);
            if (tower.HighSpeedAirFlowRate == DataSizing::AutoSize) {
                tower.HighSpeedAirFlowRateWasAutoSized = true;
            }
            if (NumArray(5) <= 0.0 && NumArray(5) != DataSizing::AutoSize) {
                ShowSevereCustom(state, eoh, "Design air flow rate must be > 0");
                ErrorsFound = true;
            }

            tower.HighSpeedFanPower = NumArray(6);
            if (tower.HighSpeedFanPower == DataSizing::AutoSize) {
                tower.HighSpeedFanPowerWasAutoSized = true;
            }
            if (NumArray(6) <= 0.0 && NumArray(6) != DataSizing::AutoSize) {
                ShowSevereCustom(state, eoh, "Design fan power must be > 0");
                ErrorsFound = true;
            }

            //   minimum air flow rate fraction must be >= 0.2 and <= 0.5, below this value the tower fan cycles to maintain the setpoint
            tower.MinimumVSAirFlowFrac = NumArray(7);
            tower.MinimumVSAirFlowFrac = NumArray(7);
            if (NumArray(7) < 0.2 || NumArray(7) > 0.5) {
                ShowSevereCustom(state, eoh, "Minimum VS air flow rate ratio must be >= 0.2 and <= 0.5");
                ErrorsFound = true;
            }

            //   fraction of tower capacity in free convection regime must be >= to 0 and <= 0.2
            tower.FreeConvectionCapacityFraction = NumArray(8);
            if (NumArray(8) < 0.0 || NumArray(8) > 0.2) {
                ShowSevereCustom(state, eoh, "Fraction of tower capacity in free convection regime must be >= 0 and <= 0.2");
                ErrorsFound = true;
            }

            //   Basin heater power as a function of temperature must be greater than or equal to 0
            tower.BasinHeaterPowerFTempDiff = NumArray(9);
            if (NumArray(9) < 0.0) {
                ShowSevereCustom(state, eoh, "Basin heater power as a function of temperature difference must be >= 0");
                ErrorsFound = true;
            }

            tower.BasinHeaterSetPointTemp = NumArray(10);
            if (tower.BasinHeaterPowerFTempDiff > 0.0) {
                if (NumNums < 10) {
                    tower.BasinHeaterSetPointTemp = 2.0;
                }
                if (tower.BasinHeaterSetPointTemp < 2.0) {
                    ShowWarningCustom(state, eoh, format("{} is less than 2 deg C. Freezing could occur.", s_ipsc->cNumericFieldNames(10)));
                }
            }

            // Performance Input Method for Variable Speed Towers is assigned to be UA AND DESIGN WATER FLOW RATE
            // for autosizing calculations (see SizeTower)
            tower.PerformanceInputMethod_Num = PIM::UFactor;

            if (!AlphArray(7).empty()) {
                if ((tower.basinHeaterSched = Sched::GetSchedule(state, AlphArray(7))) == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            s_ipsc->cAlphaFieldNames(7),
                                            AlphArray(7),
                                            "Basin heater operation will not be modeled and the simulation continues.");
                }
            }

            // begin water use and systems get input
            tower.EvapLossMode = static_cast<EvapLoss>(getEnumValue(EvapLossNamesUC, Util::makeUPPER(AlphArray(8))));
            tower.UserEvapLossFactor = NumArray(11);        //  N11 , \field Evaporation Loss Factor
            tower.DriftLossFraction = NumArray(12) / 100.0; //  N12, \field Drift Loss Percent
            tower.ConcentrationRatio = NumArray(13);        //  N13, \field Blowdown Concentration Ratio
            tower.SizFac = NumArray(17);                    //  N14  \field Sizing Factor
            if (tower.SizFac <= 0.0) {
                tower.SizFac = 1.0;
            }

            tower.BlowdownMode = static_cast<Blowdown>(getEnumValue(BlowDownNamesUC, Util::makeUPPER(AlphArray(9))));
            if (tower.BlowdownMode == Blowdown::Schedule) {
                if ((tower.blowdownSched = Sched::GetSchedule(state, AlphArray(10))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(10), AlphArray(10));
                    ErrorsFound = true;
                }
            }

            // added for multi-cell
            tower.NumCell = NumArray(14);
            if ((NumNums < 14) && (tower.NumCell == 0)) {
                // assume Number of Cells not entered and should be defaulted
                tower.NumCell = 1;
            }
            tower.MinFracFlowRate = NumArray(15);
            if ((NumNums < 15) && (tower.MinFracFlowRate == 0.0)) {
                // assume Cell Minimum Water Flow Rate Fraction not entered and should be defaulted
                tower.MinFracFlowRate = 0.33;
            }
            tower.MaxFracFlowRate = NumArray(16);
            if ((NumNums < 16) && (tower.MaxFracFlowRate == 0.0)) {
                // assume Cell Maximum Water Flow Rate Fraction not entered and should be defaulted
                tower.MaxFracFlowRate = 2.5;
            }

            //   cell control for variable speed tower
            if (!s_ipsc->lAlphaFieldBlanks(13)) {
                tower.cellCtrl = static_cast<CellCtrl>(getEnumValue(CellCtrlNamesUC, Util::makeUPPER(AlphArray(13))));
            }

            if (s_ipsc->lAlphaFieldBlanks(11)) {
                tower.SuppliedByWaterSystem = false;
            } else { // water from storage tank
                WaterManager::SetupTankDemandComponent(
                    state, AlphArray(1), s_ipsc->cCurrentModuleObject, AlphArray(11), ErrorsFound, tower.WaterTankID, tower.WaterTankDemandARRID);
                tower.SuppliedByWaterSystem = true;
            }

            //   outdoor air inlet node
            if (s_ipsc->lAlphaFieldBlanks(12)) {
                tower.OutdoorAirInletNodeNum = 0;
            } else {
                tower.OutdoorAirInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                                   AlphArray(12),
                                                                                   ErrorsFound,
                                                                                   DataLoopNode::ConnectionObjectType::CoolingTowerVariableSpeed,
                                                                                   tower.Name,
                                                                                   DataLoopNode::NodeFluidType::Air,
                                                                                   DataLoopNode::ConnectionType::OutsideAirReference,
                                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                                   DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, tower.OutdoorAirInletNodeNum)) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(12), AlphArray(12));
                    ErrorsFound = true;
                }
            }
            if (NumAlphas > 13) {
                tower.EndUseSubcategory = AlphArray(14);
            } else {
                tower.EndUseSubcategory = "General";
            }

        } // End Variable-Speed Tower Loop

        s_ipsc->cCurrentModuleObject = cCoolingTower_VariableSpeedMerkel;
        for (int MerkelVSTowerNum = 1; MerkelVSTowerNum <= NumVSMerkelTowers; ++MerkelVSTowerNum) {
            TowerNum = NumSingleSpeedTowers + NumTwoSpeedTowers + NumVariableSpeedTowers + MerkelVSTowerNum;
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                MerkelVSTowerNum,
                                AlphArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, AlphArray(1)};
            GlobalNames::VerifyUniqueInterObjectName(
                state, UniqueSimpleTowerNames, AlphArray(1), s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);
            auto &tower = state.dataCondenserLoopTowers->towers(TowerNum);
            tower.Name = AlphArray(1);
            tower.TowerType = DataPlant::PlantEquipmentType::CoolingTower_VarSpdMerkel;
            tower.WaterInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                          AlphArray(2),
                                                                          ErrorsFound,
                                                                          DataLoopNode::ConnectionObjectType::CoolingTowerVariableSpeedMerkel,
                                                                          AlphArray(1),
                                                                          DataLoopNode::NodeFluidType::Water,
                                                                          DataLoopNode::ConnectionType::Inlet,
                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                          DataLoopNode::ObjectIsNotParent);
            tower.WaterOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           AlphArray(3),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::CoolingTowerVariableSpeedMerkel,
                                                                           AlphArray(1),
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Outlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
            BranchNodeConnections::TestCompSet(state, s_ipsc->cCurrentModuleObject, AlphArray(1), AlphArray(2), AlphArray(3), "Chilled Water Nodes");

            if (Util::SameString(AlphArray(4), "UFactorTimesAreaAndDesignWaterFlowRate")) {
                tower.PerformanceInputMethod_Num = PIM::UFactor;
            } else if (Util::SameString(AlphArray(4), "NominalCapacity")) {
                tower.PerformanceInputMethod_Num = PIM::NominalCapacity;
            } else {
                ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), AlphArray(4));
                ErrorsFound = true;
            }

            tower.FanPowerfAirFlowCurve = Curve::GetCurveIndex(state, AlphArray(5));
            if (tower.FanPowerfAirFlowCurve == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), AlphArray(5));
                ErrorsFound = true;
            }

            tower.HeatRejectCapNomCapSizingRatio = NumArray(1);
            tower.TowerNominalCapacity = NumArray(2);
            if (tower.TowerNominalCapacity == DataSizing::AutoSize) {
                tower.TowerNominalCapacityWasAutoSized = true;
            }
            tower.TowerFreeConvNomCap = NumArray(3);
            if (tower.TowerFreeConvNomCap == DataSizing::AutoSize) {
                tower.TowerFreeConvNomCapWasAutoSized = true;
            }
            tower.TowerFreeConvNomCapSizingFactor = NumArray(4);
            tower.DesignWaterFlowRate = NumArray(5);
            if (tower.DesignWaterFlowRate == DataSizing::AutoSize) {
                tower.DesignWaterFlowRateWasAutoSized = true;
            }
            tower.DesignWaterFlowPerUnitNomCap = NumArray(6);
            tower.HighSpeedAirFlowRate = NumArray(7);
            if (tower.HighSpeedAirFlowRate == DataSizing::AutoSize) {
                tower.HighSpeedAirFlowRateWasAutoSized = true;
            }
            tower.DefaultedDesignAirFlowScalingFactor = s_ipsc->lNumericFieldBlanks(8);
            tower.DesignAirFlowPerUnitNomCap = NumArray(8);
            tower.MinimumVSAirFlowFrac = NumArray(9);
            tower.HighSpeedFanPower = NumArray(10);
            if (tower.HighSpeedFanPower == DataSizing::AutoSize) {
                tower.HighSpeedFanPowerWasAutoSized = true;
            }
            tower.DesignFanPowerPerUnitNomCap = NumArray(11);
            tower.FreeConvAirFlowRate = NumArray(12);
            if (tower.FreeConvAirFlowRate == DataSizing::AutoSize) {
                tower.FreeConvAirFlowRateWasAutoSized = true;
            }
            tower.FreeConvAirFlowRateSizingFactor = NumArray(13);
            tower.HighSpeedTowerUA = NumArray(14);
            if (tower.HighSpeedTowerUA == DataSizing::AutoSize) {
                tower.HighSpeedTowerUAWasAutoSized = true;
            }
            tower.FreeConvTowerUA = NumArray(15);
            if (tower.FreeConvTowerUA == DataSizing::AutoSize) {
                tower.FreeConvTowerUAWasAutoSized = true;
            }
            tower.FreeConvTowerUASizingFactor = NumArray(16);

            tower.UAModFuncAirFlowRatioCurvePtr = Curve::GetCurveIndex(state, AlphArray(6));
            if (tower.UAModFuncAirFlowRatioCurvePtr == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), AlphArray(6));
                ErrorsFound = true;
            }

            tower.UAModFuncWetBulbDiffCurvePtr = Curve::GetCurveIndex(state, AlphArray(7));
            if (tower.UAModFuncWetBulbDiffCurvePtr == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(7), AlphArray(7));
                ErrorsFound = true;
            }

            tower.UAModFuncWaterFlowRatioCurvePtr = Curve::GetCurveIndex(state, AlphArray(8));
            if (tower.UAModFuncWaterFlowRatioCurvePtr == 0) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(8), AlphArray(8));
                ErrorsFound = true;
            }
            // cooling tower design inlet conditions
            tower.DesInletAirDBTemp = NumArray(17);
            if (tower.DesInletAirDBTemp == 0) {
                tower.DesInletAirDBTemp = 35.0;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignInletWB = NumArray(18);
            if (tower.DesignInletWB == 0) {
                tower.DesignInletWB = 25.6;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignApproach = NumArray(19);
            if (tower.DesignApproach == DataSizing::AutoSize || tower.DesignApproach == 0) {
                tower.DesignApproach = 3.9;
                tower.TowerInletCondsAutoSize = true;
            }
            tower.DesignRange = NumArray(20);
            if (tower.DesignRange == DataSizing::AutoSize || tower.DesignRange == 0) {
                tower.DesignRange = 5.5;
                tower.TowerInletCondsAutoSize = true;
            }
            // set tower design water outlet and inlet temperatures
            tower.DesOutletWaterTemp = tower.DesignInletWB + tower.DesignApproach;
            tower.DesInletWaterTemp = tower.DesOutletWaterTemp + tower.DesignRange;
            //   Basin heater power as a function of temperature must be greater than or equal to 0
            tower.BasinHeaterPowerFTempDiff = NumArray(21);
            if (NumArray(21) < 0.0) {
                ShowSevereCustom(state, eoh, "Basin heater power as a function of temperature difference must be >= 0");
                ErrorsFound = true;
            }

            tower.BasinHeaterSetPointTemp = NumArray(22);
            if (tower.BasinHeaterPowerFTempDiff > 0.0) {
                if (NumNums < 22) {
                    tower.BasinHeaterSetPointTemp = 2.0;
                }
                if (tower.BasinHeaterSetPointTemp < 2.0) {
                    ShowWarningCustom(state, eoh, format("{} is less than 2 deg C. Freezing could occur.", s_ipsc->cNumericFieldNames(22)));
                }
            }

            if (!AlphArray(9).empty()) {
                if ((tower.basinHeaterSched = Sched::GetSchedule(state, AlphArray(9))) == nullptr) {
                    ShowWarningItemNotFound(state,
                                            eoh,
                                            s_ipsc->cAlphaFieldNames(9),
                                            AlphArray(9),
                                            "Basin heater operation will not be modeled and the simulation continues");
                }
            }

            // begin water use and systems get input
            tower.EvapLossMode = static_cast<EvapLoss>(getEnumValue(EvapLossNamesUC, Util::makeUPPER(AlphArray(10))));
            tower.UserEvapLossFactor = NumArray(23);        //  N23 , \field Evaporation Loss Factor
            tower.DriftLossFraction = NumArray(24) / 100.0; //  N24, \field Drift Loss Percent
            tower.ConcentrationRatio = NumArray(25);        //  N25, \field Blowdown Concentration Ratio
            tower.SizFac = NumArray(29);                    //  N29  \field Sizing Factor
            if (tower.SizFac <= 0.0) {
                tower.SizFac = 1.0;
            }

            tower.BlowdownMode = static_cast<Blowdown>(getEnumValue(BlowDownNamesUC, Util::makeUPPER(AlphArray(11))));
            if (tower.BlowdownMode == Blowdown::Schedule) {
                if ((tower.blowdownSched = Sched::GetSchedule(state, AlphArray(12))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(12), AlphArray(12));
                    ErrorsFound = true;
                }
            }

            // added for multi-cell
            tower.NumCell = NumArray(26);
            if ((NumNums < 26) && (tower.NumCell == 0)) {
                // assume Number of Cells not entered and should be defaulted
                tower.NumCell = 1;
            }
            tower.MinFracFlowRate = NumArray(27);
            if ((NumNums < 27) && (tower.MinFracFlowRate == 0.0)) {
                // assume Cell Minimum Water Flow Rate Fraction not entered and should be defaulted
                tower.MinFracFlowRate = 0.33;
            }
            tower.MaxFracFlowRate = NumArray(28);
            if ((NumNums < 28) && (tower.MaxFracFlowRate == 0.0)) {
                // assume Cell Maximum Water Flow Rate Fraction not entered and should be defaulted
                tower.MaxFracFlowRate = 2.5;
            }
            tower.TowerMassFlowRateMultiplier = tower.MaxFracFlowRate;
            //   cell control for variable speed Merkel tower
            if (!s_ipsc->lAlphaFieldBlanks(15)) {
                tower.cellCtrl = static_cast<CellCtrl>(getEnumValue(CellCtrlNamesUC, Util::makeUPPER(AlphArray(15))));
            }

            if (s_ipsc->lAlphaFieldBlanks(13)) {
                tower.SuppliedByWaterSystem = false;
            } else { // water from storage tank
                WaterManager::SetupTankDemandComponent(
                    state, AlphArray(1), s_ipsc->cCurrentModuleObject, AlphArray(13), ErrorsFound, tower.WaterTankID, tower.WaterTankDemandARRID);
                tower.SuppliedByWaterSystem = true;
            }

            //   outdoor air inlet node
            if (s_ipsc->lAlphaFieldBlanks(14)) {
                tower.OutdoorAirInletNodeNum = 0;
            } else {
                tower.OutdoorAirInletNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        AlphArray(14),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::CoolingTowerVariableSpeedMerkel,
                                                        tower.Name,
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, tower.OutdoorAirInletNodeNum)) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(14), AlphArray(14));
                    ErrorsFound = true;
                }
            }
            if (NumAlphas > 15) {
                tower.EndUseSubcategory = AlphArray(16);
            } else {
                tower.EndUseSubcategory = "General";
            }

        } // end merkel vs tower loop

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting cooling tower input.");
        }
    }

    void CoolingTower::oneTimeInit(EnergyPlusData &state)
    {
        // Locate the tower on the plant loops for later usage
        bool ErrorsFound = false;
        PlantUtilities::ScanPlantLoopsForObject(state, this->Name, this->TowerType, this->plantLoc, ErrorsFound, _, _, _, _, _);
        if (ErrorsFound) {
            ShowFatalError(state, "initialize: Program terminated due to previous condition(s).");
        }

        // check if setpoint on outlet node
        this->SetpointIsOnOutlet = !((state.dataLoopNodes->Node(this->WaterOutletNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
                                     (state.dataLoopNodes->Node(this->WaterOutletNodeNum).TempSetPointHi == DataLoopNode::SensedNodeFlagValue));
    }

    void CoolingTower::initEachEnvironment(EnergyPlusData &state)
    {
        static constexpr std::string_view RoutineName("CoolingTower::initEachEnvironment");
        Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
        this->DesWaterMassFlowRate = this->DesignWaterFlowRate * rho;
        this->DesWaterMassFlowRatePerCell = this->DesWaterMassFlowRate / this->NumCell;
        PlantUtilities::InitComponentNodes(state, 0.0, this->DesWaterMassFlowRate, this->WaterInletNodeNum, this->WaterOutletNodeNum);
    }

    void CoolingTower::initialize(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2002
        //       MODIFIED       Don Shirey Sept/Oct 2002, F Buhl Oct 2002
        //       RE-ENGINEERED  R. Raustad, Oct 2005, moved Max/MinAvail to Init and allowed more than design
        //                      water flow rate to pass through towers (up to 2.5 and 1.25 times the design flow
        //                      for 1 or 2-speed and variable speed towers, respectively). Flow multiplier for
        //                      VS Tower is defaulted to 1.25 and can be reassigned by user.

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Cooling Tower components and for
        // final checking of tower inputs (post autosizing)

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        if (this->oneTimeFlag) {
            this->setupOutputVariables(state);
            this->oneTimeInit(state);
            this->oneTimeFlag = false;
        }

        // Begin environment initializations
        if (this->envrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
            this->initEachEnvironment(state);
            this->envrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->envrnFlag = true;
        }

        // Each time initializations
        this->WaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;

        if (this->OutdoorAirInletNodeNum != 0) {
            this->AirTemp = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).Temp;
            this->AirHumRat = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).HumRat;
            this->AirPress = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).Press;
            this->AirWetBulb = state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).OutAirWetBulb;
        } else {
            this->AirTemp = state.dataEnvrn->OutDryBulbTemp;
            this->AirHumRat = state.dataEnvrn->OutHumRat;
            this->AirPress = state.dataEnvrn->OutBaroPress;
            this->AirWetBulb = state.dataEnvrn->OutWetBulbTemp;
        }

        this->WaterMassFlowRate =
            PlantUtilities::RegulateCondenserCompFlowReqOp(state, this->plantLoc, this->DesWaterMassFlowRate * this->TowerMassFlowRateMultiplier);

        PlantUtilities::SetComponentFlowRate(state, this->WaterMassFlowRate, this->WaterInletNodeNum, this->WaterOutletNodeNum, this->plantLoc);

        // Added for fluid bypass. 8/2008
        this->BypassFraction = 0.0;
        this->BasinHeaterPower = 0.0;
        this->airFlowRateRatio = 0.0;
    }

    void CoolingTower::setupOutputVariables(EnergyPlusData &state)
    {
        // Set up output variables CurrentModuleObject='CoolingTower:SingleSpeed'
        if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd) {
            SetupOutputVariable(state,
                                "Cooling Tower Inlet Temperature",
                                Constant::Units::C,
                                this->InletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Outlet Temperature",
                                Constant::Units::C,
                                this->OutletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Mass Flow Rate",
                                Constant::Units::kg_s,
                                this->WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Heat Transfer Rate",
                                Constant::Units::W,
                                this->Qactual,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Rate",
                                Constant::Units::W,
                                this->FanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Energy",
                                Constant::Units::J,
                                this->FanEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection,
                                this->EndUseSubcategory);
            // Added for fluid bypass
            SetupOutputVariable(state,
                                "Cooling Tower Bypass Fraction",
                                Constant::Units::None,
                                this->BypassFraction,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Operating Cells Count",
                                Constant::Units::None,
                                this->NumCellOn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Cycling Ratio",
                                Constant::Units::None,
                                this->FanCyclingRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            if (this->BasinHeaterPowerFTempDiff > 0.0) {
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Rate",
                                    Constant::Units::W,
                                    this->BasinHeaterPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Energy",
                                    Constant::Units::J,
                                    this->BasinHeaterConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    this->Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Plant,
                                    OutputProcessor::EndUseCat::HeatRejection,
                                    "BasinHeater");
            }
        }

        // CurrentModuleObject='CoolingTower:TwoSpeed'
        if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
            SetupOutputVariable(state,
                                "Cooling Tower Inlet Temperature",
                                Constant::Units::C,
                                this->InletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Outlet Temperature",
                                Constant::Units::C,
                                this->OutletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Mass Flow Rate",
                                Constant::Units::kg_s,
                                this->WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Heat Transfer Rate",
                                Constant::Units::W,
                                this->Qactual,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Rate",
                                Constant::Units::W,
                                this->FanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Energy",
                                Constant::Units::J,
                                this->FanEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection,
                                this->EndUseSubcategory);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Cycling Ratio",
                                Constant::Units::None,
                                this->FanCyclingRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Speed Level",
                                Constant::Units::None,
                                this->SpeedSelected,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Operating Cells Count",
                                Constant::Units::None,
                                this->NumCellOn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            if (this->BasinHeaterPowerFTempDiff > 0.0) {
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Rate",
                                    Constant::Units::W,
                                    this->BasinHeaterPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Energy",
                                    Constant::Units::J,
                                    this->BasinHeaterConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    this->Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Plant,
                                    OutputProcessor::EndUseCat::HeatRejection,
                                    "BasinHeater");
            }
        }

        // CurrentModuleObject='CoolingTower:VariableSpeed'
        if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_VarSpd) {
            SetupOutputVariable(state,
                                "Cooling Tower Inlet Temperature",
                                Constant::Units::C,
                                this->InletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Outlet Temperature",
                                Constant::Units::C,
                                this->OutletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Mass Flow Rate",
                                Constant::Units::kg_s,
                                this->WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Heat Transfer Rate",
                                Constant::Units::W,
                                this->Qactual,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Rate",
                                Constant::Units::W,
                                this->FanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Energy",
                                Constant::Units::J,
                                this->FanEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection,
                                this->EndUseSubcategory);
            SetupOutputVariable(state,
                                "Cooling Tower Air Flow Rate Ratio",
                                Constant::Units::None,
                                this->AirFlowRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Part Load Ratio",
                                Constant::Units::None,
                                this->FanCyclingRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Operating Cells Count",
                                Constant::Units::None,
                                this->NumCellOn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            if (this->BasinHeaterPowerFTempDiff > 0.0) {
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Rate",
                                    Constant::Units::W,
                                    this->BasinHeaterPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Energy",
                                    Constant::Units::J,
                                    this->BasinHeaterConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    this->Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Plant,
                                    OutputProcessor::EndUseCat::HeatRejection,
                                    "BasinHeater");
            }
        }

        // CurrentModuleObject='CoolingTower:VariableSpeed:Merkel'
        if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_VarSpdMerkel) {
            SetupOutputVariable(state,
                                "Cooling Tower Inlet Temperature",
                                Constant::Units::C,
                                this->InletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Outlet Temperature",
                                Constant::Units::C,
                                this->OutletWaterTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Mass Flow Rate",
                                Constant::Units::kg_s,
                                this->WaterMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Heat Transfer Rate",
                                Constant::Units::W,
                                this->Qactual,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Rate",
                                Constant::Units::W,
                                this->FanPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Electricity Energy",
                                Constant::Units::J,
                                this->FanEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection,
                                this->EndUseSubcategory);
            SetupOutputVariable(state,
                                "Cooling Tower Fan Speed Ratio",
                                Constant::Units::None,
                                this->AirFlowRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);

            SetupOutputVariable(state,
                                "Cooling Tower Operating Cells Count",
                                Constant::Units::None,
                                this->NumCellOn,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            if (this->BasinHeaterPowerFTempDiff > 0.0) {
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Rate",
                                    Constant::Units::W,
                                    this->BasinHeaterPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    this->Name);
                SetupOutputVariable(state,
                                    "Cooling Tower Basin Heater Electricity Energy",
                                    Constant::Units::J,
                                    this->BasinHeaterConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    this->Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::Plant,
                                    OutputProcessor::EndUseCat::HeatRejection,
                                    "BasinHeater");
            }
        }
        // setup common water reporting for all types of towers.
        if (this->SuppliedByWaterSystem) {
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->MakeUpVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume",
                                Constant::Units::m3,
                                this->MakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Storage Tank Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->TankSupplyVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Storage Tank Water Volume",
                                Constant::Units::m3,
                                this->TankSupplyVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);
            SetupOutputVariable(state,
                                "Cooling Tower Starved Storage Tank Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->StarvedMakeUpVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Starved Storage Tank Water Volume",
                                Constant::Units::m3,
                                this->StarvedMakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Mains Water Volume",
                                Constant::Units::m3,
                                this->StarvedMakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);
        } else { // tower water from mains and gets metered
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume Flow Rate",
                                Constant::Units::m3_s,
                                this->MakeUpVdot,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Water Volume",
                                Constant::Units::m3,
                                this->MakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);
            SetupOutputVariable(state,
                                "Cooling Tower Make Up Mains Water Volume",
                                Constant::Units::m3,
                                this->MakeUpVol,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::HeatRejection);
        }

        SetupOutputVariable(state,
                            "Cooling Tower Water Evaporation Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->EvaporationVdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Water Evaporation Volume",
                            Constant::Units::m3,
                            this->EvaporationVol,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Water Drift Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->DriftVdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Water Drift Volume",
                            Constant::Units::m3,
                            this->DriftVol,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Water Blowdown Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->BlowdownVdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Water Blowdown Volume",
                            Constant::Units::m3,
                            this->BlowdownVol,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Approach",
                            Constant::Units::C,
                            this->coolingTowerApproach,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Cooling Tower Range",
                            Constant::Units::C,
                            this->coolingTowerRange,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
    }

    void CoolingTower::SizeTower(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2002
        //       MODIFIED       Don Shirey, Sept/Oct 2002; Richard Raustad, Feb 2005

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Cooling Tower Components for which capacities and flow rates
        // have not been specified in the input. This subroutine also calculates tower UA if the user
        // has specified tower performance via the "Nominal Capacity" method.

        // METHODOLOGY EMPLOYED:
        // Obtains condenser flow rate from the plant sizing array. If tower performance is specified
        // via the "Nominal Capacity" method, the water flow rate is directly proportional to capacity.

        // SUBROUTINE PARAMETER DEFINITIONS:

        int constexpr MaxIte(500);    // Maximum number of iterations
        Real64 constexpr Acc(0.0001); // Accuracy of result
        static constexpr std::string_view RoutineName("SizeTower");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SolFla;                       // Flag of solver
        Real64 DesTowerLoad(0.0);         // Design tower load [W]
        Real64 UA0;                       // Lower bound for UA [W/C]
        Real64 UA1;                       // Upper bound for UA [W/C]
        Real64 UA;                        // Calculated UA value
        Real64 DesTowerInletWaterTemp;    // design tower inlet water temperature
        Real64 DesTowerExitWaterTemp;     // design tower exit water temperature
        Real64 DesTowerWaterDeltaT;       // design tower temperature range
        Real64 DesTowerApproachFromPlant; // design tower approach temperature from plant sizing object
        Real64 TolTemp(0.04);             // DeltaT and DesignApproach diffs tolerance between plant sizing data and user input in cooling tower
        // for warning message reporting purpose only

        Real64 tmpDesignWaterFlowRate = this->DesignWaterFlowRate;
        Real64 tmpHighSpeedFanPower = this->HighSpeedFanPower;
        Real64 tmpHighSpeedAirFlowRate = this->HighSpeedAirFlowRate;
        Real64 tmpLowSpeedAirFlowRate = this->LowSpeedAirFlowRate;

        auto &PlantSizData(state.dataSize->PlantSizData);

        // Find the appropriate Plant Sizing object
        int PltSizCondNum = this->plantLoc.loop->PlantSizNum;

        if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd ||
            this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
            if (this->TowerInletCondsAutoSize) {
                if (PltSizCondNum > 0) {
                    // use plant sizing data
                    DesTowerExitWaterTemp = PlantSizData(PltSizCondNum).ExitTemp;
                    DesTowerInletWaterTemp = DesTowerExitWaterTemp + PlantSizData(PltSizCondNum).DeltaT;
                    DesTowerWaterDeltaT = PlantSizData(PltSizCondNum).DeltaT;
                } else {
                    // set hard wired input assumptions
                    // AssumedDeltaT = 11.0;
                    // AssumedExitTemp = 21.0;
                    DesTowerWaterDeltaT = 11.0;
                    DesTowerExitWaterTemp = 21.0;
                    DesTowerInletWaterTemp = DesTowerExitWaterTemp + DesTowerWaterDeltaT;
                }
            } else {
                // use tower sizing data
                DesTowerExitWaterTemp = this->DesOutletWaterTemp;
                DesTowerInletWaterTemp = this->DesInletWaterTemp;
                DesTowerWaterDeltaT = this->DesignRange;
                if (PltSizCondNum > 0) {
                    // check the tower range against the plant sizing data
                    if (std::abs(DesTowerWaterDeltaT - PlantSizData(PltSizCondNum).DeltaT) > TolTemp) {
                        ShowWarningError(state,
                                         format("Error when autosizing the load for cooling tower = {}. Tower Design Range Temperature is different "
                                                "from the Design Loop Delta Temperature.",
                                                this->Name));
                        ShowContinueError(state, format("Tower Design Range Temperature specified in tower = {}", this->Name));
                        ShowContinueError(state,
                                          format("is inconsistent with Design Loop Delta Temperature specified in Sizing:Plant object = {}.",
                                                 PlantSizData(PltSizCondNum).PlantLoopName));
                        ShowContinueError(state, format("..The Design Range Temperature specified in tower is = {:.2T}", this->DesignRange));
                        ShowContinueError(state,
                                          format("..The Design Loop Delta Temperature specified in plant sizing data is = {:.2T}",
                                                 PlantSizData(PltSizCondNum).DeltaT));
                    }
                    // check if the tower approach is different from plant sizing data
                    DesTowerApproachFromPlant = PlantSizData(PltSizCondNum).ExitTemp - this->DesignInletWB;
                    if (std::abs(DesTowerApproachFromPlant - this->DesignApproach) > TolTemp) {
                        ShowWarningError(state,
                                         format("Error when autosizing the UA for cooling tower = {}. Tower Design Approach Temperature is "
                                                "inconsistent with Approach from Plant Sizing Data.",
                                                this->Name));
                        ShowContinueError(state,
                                          format("The Design Approach Temperature from inputs specified in Sizing:Plant object = {}",
                                                 PlantSizData(PltSizCondNum).PlantLoopName));
                        ShowContinueError(state, format("is inconsistent with Design Approach Temperature specified in tower = {}.", this->Name));
                        ShowContinueError(state,
                                          format("..The Design Approach Temperature from inputs specified is = {:.2T}", DesTowerApproachFromPlant));
                        ShowContinueError(state, format("..The Design Approach Temperature specified in tower is = {:.2T}", this->DesignApproach));
                    }
                }
            }
        } else { // CoolingTower_VariableSpeed
            if (PltSizCondNum > 0) {
                // use plant sizing data
                DesTowerExitWaterTemp = PlantSizData(PltSizCondNum).ExitTemp;
                DesTowerInletWaterTemp = DesTowerExitWaterTemp + PlantSizData(PltSizCondNum).DeltaT;
                DesTowerWaterDeltaT = PlantSizData(PltSizCondNum).DeltaT;
            } else {
                // set hard wired input assumptions
                DesTowerWaterDeltaT = 11.0;
                DesTowerExitWaterTemp = 21.0;
                DesTowerInletWaterTemp = DesTowerExitWaterTemp + DesTowerWaterDeltaT;
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::UFactor && (!this->HighSpeedTowerUAWasAutoSized)) {
            if (PltSizCondNum > 0) {
                Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                DesTowerLoad = rho * Cp * this->DesignWaterFlowRate * DesTowerWaterDeltaT;
                this->TowerNominalCapacity = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;

            } else {
                Real64 AssumedDeltaT = DesTowerWaterDeltaT;
                Real64 AssumedExitTemp = DesTowerExitWaterTemp;

                Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, AssumedExitTemp, RoutineName);
                Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, AssumedExitTemp, RoutineName);

                DesTowerLoad = rho * Cp * this->DesignWaterFlowRate * AssumedDeltaT;
                this->TowerNominalCapacity = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
            }
        }

        if (this->DesignWaterFlowRateWasAutoSized) {
            if (PltSizCondNum > 0) {
                if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                    tmpDesignWaterFlowRate = PlantSizData(PltSizCondNum).DesVolFlowRate * this->SizFac;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                    }
                } else {
                    tmpDesignWaterFlowRate = 0.0;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                    }
                }
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Design Water Flow Rate [m3/s]",
                                                 this->DesignWaterFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Design Water Flow Rate [m3/s]",
                                                 this->DesignWaterFlowRate);
                }
            } else {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    ShowSevereError(state, format("Autosizing error for cooling tower object = {}", this->Name));
                    ShowFatalError(state, "Autosizing of cooling tower condenser flow rate requires a loop Sizing:Plant object.");
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::NominalCapacity) {
            // Design water flow rate is assumed to be 3 gpm per ton (SI equivalent 5.382E-8 m3/s per watt)
            this->DesignWaterFlowRate = 5.382e-8 * this->TowerNominalCapacity;
            tmpDesignWaterFlowRate = this->DesignWaterFlowRate;
            if (Util::SameString(DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], "CoolingTower:SingleSpeed")) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Design Water Flow Rate based on tower nominal capacity [m3/s]",
                                                 this->DesignWaterFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Design Water Flow Rate based on tower nominal capacity [m3/s]",
                                                 this->DesignWaterFlowRate);
                }
            } else if (Util::SameString(DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], "CoolingTower:TwoSpeed")) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Design Water Flow Rate based on tower high-speed nominal capacity [m3/s]",
                                                 this->DesignWaterFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Design Water Flow Rate based on tower high-speed nominal capacity [m3/s]",
                                                 this->DesignWaterFlowRate);
                }
            }
        }

        PlantUtilities::RegisterPlantCompDesignFlow(state, this->WaterInletNodeNum, tmpDesignWaterFlowRate);

        if (this->HighSpeedFanPowerWasAutoSized) {
            // We assume the nominal fan power is 0.0105 times the design load
            if (this->PerformanceInputMethod_Num == PIM::NominalCapacity) {
                this->HighSpeedFanPower = 0.0105 * this->TowerNominalCapacity;
                tmpHighSpeedFanPower = this->HighSpeedFanPower;
            } else {
                if (PltSizCondNum > 0) {
                    if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                        Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
                        Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                        DesTowerLoad = rho * Cp * tmpDesignWaterFlowRate * DesTowerWaterDeltaT;
                        tmpHighSpeedFanPower = 0.0105 * DesTowerLoad;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->HighSpeedFanPower = tmpHighSpeedFanPower;
                        }
                    } else {
                        tmpHighSpeedFanPower = 0.0;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->HighSpeedFanPower = tmpHighSpeedFanPower;
                        }
                    }
                } else {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        ShowSevereError(state, "Autosizing of cooling tower fan power requires a loop Sizing:Plant object.");
                        ShowFatalError(state, format(" Occurs in cooling tower object= {}", this->Name));
                    }
                }
            }
            if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd ||
                this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_VarSpd) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Fan Power at Design Air Flow Rate [W]",
                                                 this->HighSpeedFanPower);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Fan Power at Design Air Flow Rate [W]",
                                                 this->HighSpeedFanPower);
                }
            } else if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Fan Power at High Fan Speed [W]",
                                                 this->HighSpeedFanPower);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Fan Power at High Fan Speed [W]",
                                                 this->HighSpeedFanPower);
                }
            }
        }

        if (this->HighSpeedAirFlowRateWasAutoSized) {
            // Plant Sizing Object is not required to AUTOSIZE this field since its simply a multiple of another field.
            tmpHighSpeedAirFlowRate = tmpHighSpeedFanPower * 0.5 * (101325.0 / state.dataEnvrn->StdBaroPress) / 190.0;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->HighSpeedAirFlowRate = tmpHighSpeedAirFlowRate;
            }

            if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd ||
                this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_VarSpd) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Design Air Flow Rate [m3/s]",
                                                 this->HighSpeedAirFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Design Air Flow Rate [m3/s]",
                                                 this->HighSpeedAirFlowRate);
                }
            } else if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Air Flow Rate at High Fan Speed [m3/s]",
                                                 this->HighSpeedAirFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Air Flow Rate at High Fan Speed [m3/s]",
                                                 this->HighSpeedAirFlowRate);
                }
            }
        }

        if (this->HighSpeedTowerUAWasAutoSized) {
            if (PltSizCondNum > 0) {
                if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                    Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
                    Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                    DesTowerLoad = rho * Cp * tmpDesignWaterFlowRate * DesTowerWaterDeltaT;
                    // This conditional statement is to trap when the user specified condenser/tower water design setpoint
                    //  temperature is less than design inlet air wet bulb temperature
                    if (PlantSizData(PltSizCondNum).ExitTemp <= this->DesignInletWB) {
                        ShowSevereError(state,
                                        format("Error when autosizing the UA value for cooling tower = {}. Design Loop Exit Temperature must be "
                                               "greater than {:.2T} C when autosizing the tower UA.",
                                               this->Name,
                                               this->DesignInletWB));
                        ShowContinueError(state,
                                          format("The Design Loop Exit Temperature specified in Sizing:Plant object = {} ({:.2T} C)",
                                                 PlantSizData(PltSizCondNum).PlantLoopName,
                                                 PlantSizData(PltSizCondNum).ExitTemp));
                        ShowContinueError(
                            state, format("is less than or equal to the design inlet air wet-bulb temperature of {:.2T} C.", this->DesignInletWB));
                        ShowContinueError(state,
                                          format("If using HVACTemplate:Plant:ChilledWaterLoop, then check that input field Condenser Water Design "
                                                 "Setpoint must be > {:.2T} C if autosizing the cooling tower.",
                                                 this->DesignInletWB));
                        ShowFatalError(state, format("Autosizing of cooling tower fails for tower = {}.", this->Name));
                    }

                    Real64 const solveDesignWaterMassFlow = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                    UA0 = 0.0001 * DesTowerLoad;                                          // Assume deltaT = 10000K (limit)
                    UA1 = DesTowerLoad;                                                   // Assume deltaT = 1K
                    this->WaterTemp = DesTowerInletWaterTemp;
                    this->AirTemp = this->DesInletAirDBTemp; // 35.0;
                    this->AirWetBulb = this->DesignInletWB;  // 25.6;
                    this->AirPress = state.dataEnvrn->StdBaroPress;
                    this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                    auto f1 = [&state, this, DesTowerLoad, solveDesignWaterMassFlow, tmpHighSpeedAirFlowRate, Cp](Real64 UA) {
                        Real64 const OutWaterTemp =
                            this->calculateSimpleTowerOutletTemp(state, solveDesignWaterMassFlow, tmpHighSpeedAirFlowRate, UA);
                        Real64 const CoolingOutput = Cp * solveDesignWaterMassFlow * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                        return (DesTowerLoad - CoolingOutput) / DesTowerLoad;
                    };
                    General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f1, UA0, UA1);
                    if (SolFla == -1) {
                        ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                        ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                    } else if (SolFla == -2) {
                        ShowSevereError(state, "Bad starting values for UA");
                        ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                    }

                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedTowerUA = UA;
                    }
                    this->TowerNominalCapacity = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                } else {
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedTowerUA = 0.0;
                    }
                }
                if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                } else if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                }
            } else {
                if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {

                    Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
                    Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                    DesTowerLoad = rho * Cp * tmpDesignWaterFlowRate * DesTowerWaterDeltaT;
                    // This conditional statement is to trap when the user specified condenser/tower water design setpoint
                    //  temperature is less than design inlet air wet bulb temperature
                    // Note JM 2018-11-22
                    // * If actually user-specified:
                    //  this->DesOutletWaterTemp = this->DesignInletWB
                    //                                           + this->DesignApproach;
                    //  DesTowerExitWaterTemp = this->DesOutletWaterTemp;
                    //  => This basically means that approach is negative, which is impossible (must be > 0 per IDD)
                    // * If not, hardcoded above to 21C
                    if (DesTowerExitWaterTemp <= this->DesignInletWB) {
                        ShowSevereError(state,
                                        format("Error when autosizing the UA value for cooling tower = {}. Design Tower Exit Temperature must be "
                                               "greater than {:.2T} C when autosizing the tower UA.",
                                               this->Name,
                                               this->DesignInletWB));
                        ShowContinueError(state, format("The User-specified Design Loop Exit Temperature={:.2T}", DesTowerExitWaterTemp));
                        ShowContinueError(
                            state, format("is less than or equal to the design inlet air wet-bulb temperature of {:.2T} C.", this->DesignInletWB));

                        if (this->TowerInletCondsAutoSize) {
                            ShowContinueError(state,
                                              format("Because you did not specify the Design Approach Temperature, and you do not have a "
                                                     "Sizing:Plant object, it was defaulted to {:.2T} C.",
                                                     DesTowerExitWaterTemp));
                        } else {
                            // Should never get there...
                            ShowContinueError(state,
                                              format("The Design Loop Exit Temperature is the sum of the design air inlet wet-bulb temperature= "
                                                     "{:.2T} C plus the cooling tower design approach temperature = {:.2T}C.",
                                                     this->DesignInletWB,
                                                     this->DesignApproach));
                        }
                        ShowContinueError(state,
                                          format("If using HVACTemplate:Plant:ChilledWaterLoop, then check that input field Condenser Water Design "
                                                 "Setpoint must be > {:.2T} C if autosizing the cooling tower.",
                                                 this->DesignInletWB));
                        ShowFatalError(state, format("Autosizing of cooling tower fails for tower = {}.", this->Name));
                    }

                    Real64 const solveWaterMassFlow = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                    UA0 = 0.0001 * DesTowerLoad;                                    // Assume deltaT = 10000K (limit)
                    UA1 = DesTowerLoad;                                             // Assume deltaT = 1K
                    this->WaterTemp = DesTowerInletWaterTemp;
                    this->AirTemp = this->DesInletAirDBTemp; // 35.0;
                    this->AirWetBulb = this->DesignInletWB;  // 25.6;
                    this->AirPress = state.dataEnvrn->StdBaroPress;
                    this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                    auto f = [&state, this, DesTowerLoad, solveWaterMassFlow, tmpHighSpeedAirFlowRate, Cp](Real64 UA) {
                        Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterMassFlow, tmpHighSpeedAirFlowRate, UA);
                        Real64 const CoolingOutput = Cp * solveWaterMassFlow * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                        return (DesTowerLoad - CoolingOutput) / DesTowerLoad;
                    };
                    General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                    if (SolFla == -1) {
                        ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                        ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                    } else if (SolFla == -2) {
                        ShowSevereError(state, "Bad starting values for UA");
                        ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                    }

                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedTowerUA = UA;
                    }
                    this->TowerNominalCapacity = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                } else {
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedTowerUA = 0.0;
                    }
                }
                if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                } else if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at High Fan Speed [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::NominalCapacity) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {
                // nominal capacity doesn't include compressor heat; predefined factor was 1.25 W heat rejection per W of delivered cooling but now is
                // a user input
                Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, 29.44, RoutineName);     // 85F design exiting water temp
                Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, 29.44, RoutineName); // 85F design exiting water temp

                DesTowerLoad = this->TowerNominalCapacity * this->HeatRejectCapNomCapSizingRatio;
                Real64 const solveWaterFlowRate = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                UA0 = 0.0001 * DesTowerLoad;                                    // Assume deltaT = 10000K (limit)
                UA1 = DesTowerLoad;                                             // Assume deltaT = 1K
                this->WaterTemp = this->DesInletWaterTemp;                      // 35.0; // 95F design inlet water temperature
                this->AirTemp = this->DesInletAirDBTemp;                        // 95F design inlet air dry-bulb temp
                this->AirWetBulb = this->DesignInletWB;                         // 78F design inlet air wet-bulb temp
                this->AirPress = state.dataEnvrn->StdBaroPress;
                this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                auto f = [&state, this, DesTowerLoad, solveWaterFlowRate, tmpHighSpeedAirFlowRate, Cp](Real64 UA) {
                    Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlowRate, tmpHighSpeedAirFlowRate, UA);
                    Real64 const CoolingOutput = Cp * solveWaterFlowRate * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                    return (DesTowerLoad - CoolingOutput) / DesTowerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                } else if (SolFla == -2) {
                    ShowSevereError(state, "Bad starting values for UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->HighSpeedTowerUA = UA;
                }
            } else {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->HighSpeedTowerUA = 0.0;
                }
            }
            if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                 this->HighSpeedTowerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial U-Factor Times Area Value at Design Air Flow Rate [W/C]",
                                                 this->HighSpeedTowerUA);
                }
            } else if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "U-Factor Times Area Value at High Fan Speed [W/C]",
                                                 this->HighSpeedTowerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial U-Factor Times Area Value at High Fan Speed [W/C]",
                                                 this->HighSpeedTowerUA);
                }
            }
        }

        if (this->LowSpeedAirFlowRateWasAutoSized) {

            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->LowSpeedAirFlowRate = this->LowSpeedAirFlowRateSizingFactor * this->HighSpeedAirFlowRate;
                tmpLowSpeedAirFlowRate = this->LowSpeedAirFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Low Fan Speed Air Flow Rate [m3/s]",
                                                 this->LowSpeedAirFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Low Fan Speed Air Flow Rate [m3/s]",
                                                 this->LowSpeedAirFlowRate);
                }
            } else {
                tmpLowSpeedAirFlowRate = this->LowSpeedAirFlowRateSizingFactor * tmpHighSpeedAirFlowRate;
            }
        }

        if (this->LowSpeedFanPowerWasAutoSized) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->LowSpeedFanPower = this->LowSpeedFanPowerSizingFactor * this->HighSpeedFanPower;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Fan Power at Low Fan Speed [W]",
                                                 this->LowSpeedFanPower);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Fan Power at Low Fan Speed [W]",
                                                 this->LowSpeedFanPower);
                }
            }
        }

        if (this->LowSpeedTowerUAWasAutoSized && state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            this->LowSpeedTowerUA = this->LowSpeedTowerUASizingFactor * this->HighSpeedTowerUA;
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "U-Factor Times Area Value at Low Fan Speed [W/K]",
                                             this->LowSpeedTowerUA);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Initial U-Factor Times Area Value at Low Fan Speed [W/K]",
                                             this->LowSpeedTowerUA);
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::NominalCapacity) {
            if (this->TowerLowSpeedNomCapWasAutoSized) {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->TowerLowSpeedNomCap = this->TowerLowSpeedNomCapSizingFactor * this->TowerNominalCapacity;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Low Speed Nominal Capacity [W]",
                                                     this->TowerLowSpeedNomCap);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Low Speed Nominal Capacity [W]",
                                                     this->TowerLowSpeedNomCap);
                    }
                }
            }
            if (this->TowerFreeConvNomCapWasAutoSized) {
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->TowerFreeConvNomCap = this->TowerFreeConvNomCapSizingFactor * this->TowerNominalCapacity;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Free Convection Nominal Capacity [W]",
                                                     this->TowerFreeConvNomCap);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Free Convection Nominal Capacity [W]",
                                                     this->TowerFreeConvNomCap);
                    }
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::NominalCapacity &&
            Util::SameString(DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], "CoolingTower:TwoSpeed")) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow && this->TowerLowSpeedNomCap > 0.0) {

                // nominal capacity doesn't include compressor heat; predefined factor was 1.25 W heat rejection per W of evap cooling but now is a
                // user input
                Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, 29.44, RoutineName);     // 85F design exiting water temp
                Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, 29.44, RoutineName); // 85F design exiting water temp
                DesTowerLoad = this->TowerLowSpeedNomCap * this->HeatRejectCapNomCapSizingRatio;
                Real64 const solveWaterFlow = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                UA0 = 0.0001 * DesTowerLoad;                                // Assume deltaT = 10000K (limit)
                UA1 = DesTowerLoad;                                         // Assume deltaT = 1K
                this->WaterTemp = this->DesInletWaterTemp;                  // 35.0; // 95F design inlet water temperature
                this->AirTemp = this->DesInletAirDBTemp;                    // 35.0; // 95F design inlet air dry-bulb temp
                this->AirWetBulb = this->DesignInletWB;                     // 25.6; // 78F design inlet air wet-bulb temp
                this->AirPress = state.dataEnvrn->StdBaroPress;
                this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                auto f = [&state, this, DesTowerLoad, solveWaterFlow, tmpLowSpeedAirFlowRate, Cp](Real64 UA) {
                    Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow, tmpLowSpeedAirFlowRate, UA);
                    Real64 const CoolingOutput = Cp * solveWaterFlow * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                    return (DesTowerLoad - CoolingOutput) / DesTowerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                } else if (SolFla == -2) {
                    ShowSevereError(state, "Bad starting values for UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->LowSpeedTowerUA = UA;
                }
            } else {
                this->LowSpeedTowerUA = 0.0;
            }
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Low Fan Speed U-Factor Times Area Value [W/K]",
                                             this->LowSpeedTowerUA);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Initial Low Fan Speed U-Factor Times Area Value [W/K]",
                                             this->LowSpeedTowerUA);
            }
        }

        if (this->FreeConvAirFlowRateWasAutoSized) {
            this->FreeConvAirFlowRate = this->FreeConvAirFlowRateSizingFactor * tmpHighSpeedAirFlowRate;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->FreeConvAirFlowRate = this->FreeConvAirFlowRateSizingFactor * this->HighSpeedAirFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Free Convection Regime Air Flow Rate [m3/s]",
                                                 this->FreeConvAirFlowRate);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Free Convection Regime Air Flow Rate [m3/s]",
                                                 this->FreeConvAirFlowRate);
                }
            }
        }

        if (this->FreeConvTowerUAWasAutoSized) {
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                this->FreeConvTowerUA = this->FreeConvTowerUASizingFactor * this->HighSpeedTowerUA;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Free Convection U-Factor Times Area Value [W/K]",
                                                 this->FreeConvTowerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Free Convection U-Factor Times Area Value [W/K]",
                                                 this->FreeConvTowerUA);
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::NominalCapacity) {
            if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow && this->TowerFreeConvNomCap > 0.0) {
                // nominal capacity doesn't include compressor heat; predefined factor was 1.25 W heat rejection per W of evap cooling but now user
                // input
                Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, 29.44, RoutineName);     // 85F design exiting water temp
                Real64 const Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, 29.44, RoutineName); // 85F design exiting water temp
                DesTowerLoad = this->TowerFreeConvNomCap * this->HeatRejectCapNomCapSizingRatio;
                Real64 const solveWaterFlow = rho * this->DesignWaterFlowRate; // design water mass flow rate
                UA0 = 0.0001 * DesTowerLoad;                                   // Assume deltaT = 10000K (limit)
                UA1 = DesTowerLoad;                                            // Assume deltaT = 1K
                this->WaterTemp = this->DesInletWaterTemp;                     // 35.0; // 95F design inlet water temperature
                this->AirTemp = this->DesInletAirDBTemp;                       // 35.0; // 95F design inlet air dry-bulb temp
                this->AirWetBulb = this->DesignInletWB;                        // 25.6; // 78F design inlet air wet-bulb temp
                this->AirPress = state.dataEnvrn->StdBaroPress;
                this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                auto f = [&state, this, DesTowerLoad, solveWaterFlow, Cp](Real64 UA) {
                    Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow, this->FreeConvAirFlowRate, UA);
                    Real64 const CoolingOutput = Cp * solveWaterFlow * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                    return (DesTowerLoad - CoolingOutput) / DesTowerLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                } else if (SolFla == -2) {
                    ShowSevereError(state, "Bad starting values for UA calculations");
                    ShowContinueError(state, "Tower inlet design water temperature assumed to be 35.0 C.");
                    ShowContinueError(state, "Tower inlet design air dry-bulb temperature assumed to be 35.0 C.");
                    ShowContinueError(state, "Tower inlet design air wet-bulb temperature assumed to be 25.6 C.");
                    ShowContinueError(state,
                                      format("Tower load assumed to be {:.3T} times free convection capacity of {:.0T} W.",
                                             this->HeatRejectCapNomCapSizingRatio,
                                             this->TowerFreeConvNomCap));

                    Real64 OutWaterTemp; // outlet water temperature during sizing [C]

                    OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow, this->FreeConvAirFlowRate, UA0);
                    Real64 CoolingOutput = Cp * solveWaterFlow * (this->WaterTemp - OutWaterTemp); // tower capacity during sizing [W]
                    ShowContinueError(state, format("Tower capacity at lower UA guess ({:.4T}) = {:.0T} W.", UA0, CoolingOutput));

                    OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow, this->FreeConvAirFlowRate, UA1);
                    CoolingOutput = Cp * solveWaterFlow * (this->WaterTemp - OutWaterTemp);
                    ShowContinueError(state, format("Tower capacity at upper UA guess ({:.4T}) = {:.0T} W.", UA1, CoolingOutput));

                    if (CoolingOutput < DesTowerLoad) {
                        ShowContinueError(state, "Free convection capacity should be less than tower capacity at upper UA guess.");
                    }
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                }
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->FreeConvTowerUA = UA;
                }
            } else {
                this->FreeConvTowerUA = 0.0;
            }
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                             this->FreeConvTowerUA);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Initial U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                             this->FreeConvTowerUA);
            }
        }

        // calibrate variable speed tower model based on user input by finding calibration water flow rate ratio that
        // yields an approach temperature that matches user input
        if (Util::SameString(DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], "CoolingTower:VariableSpeed")) {

            //   check range for water flow rate ratio (make sure RegulaFalsi converges)
            Real64 MaxWaterFlowRateRatio = 0.5; // maximum water flow rate ratio which yields desired approach temp
            Real64 Tapproach = 0.0;             // temporary tower approach temp variable [C]
            Real64 const FlowRateRatioStep = (this->MaxWaterFlowRatio - this->MinWaterFlowRatio) / 10.0;
            bool ModelCalibrated = true;
            Real64 ModelWaterFlowRatioMax = this->MaxWaterFlowRatio * 4.0; // maximum water flow rate ratio used for model calibration
            //   find a flow rate large enough to provide an approach temperature > than the user defined approach
            Real64 WaterFlowRateRatio(0.0); // tower water flow rate ratio
            while (Tapproach < this->DesignApproach && MaxWaterFlowRateRatio <= ModelWaterFlowRatioMax) {
                WaterFlowRateRatio = MaxWaterFlowRateRatio;
                Tapproach = this->calculateVariableSpeedApproach(state, WaterFlowRateRatio, 1.0, this->DesignInletWB, this->DesignRange);
                if (Tapproach < this->DesignApproach) {
                    MaxWaterFlowRateRatio += FlowRateRatioStep;
                }
                // a water flow rate large enough to provide an approach temperature > than the user defined approach does not exist
                // within the tolerances specified by the user
                if ((MaxWaterFlowRateRatio == 0.5 && Tapproach > this->DesignApproach) || MaxWaterFlowRateRatio >= ModelWaterFlowRatioMax) {
                    ModelCalibrated = false;
                    break;
                }
            }

            Real64 WaterFlowRatio(0.0); // tower water flow rate ratio found during model calibration

            if (ModelCalibrated) {
                auto f = [&state, this](Real64 FlowRatio) {
                    Real64 Tact = this->calculateVariableSpeedApproach(state, FlowRatio, 1.0, this->DesignInletWB, this->DesignRange);
                    return this->DesignApproach - Tact;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, WaterFlowRatio, f, DataPrecisionGlobals::constant_pointfive, MaxWaterFlowRateRatio);

                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating tower water flow ratio during calibration");
                    ShowContinueError(state,
                                      "Inlet air wet-bulb, range, and/or approach temperature does not allow calibration of water flow rate ratio "
                                      "for this variable-speed cooling tower.");
                    ShowFatalError(state, format("Cooling tower calibration failed for tower {}", this->Name));
                } else if (SolFla == -2) {
                    ShowSevereError(state, "Bad starting values for cooling tower water flow rate ratio calibration.");
                    ShowContinueError(state,
                                      "Inlet air wet-bulb, range, and/or approach temperature does not allow calibration of water flow rate ratio "
                                      "for this variable-speed cooling tower.");
                    ShowFatalError(state, format("Cooling tower calibration failed for tower {}.", this->Name));
                }
            } else {
                ShowSevereError(state, "Bad starting values for cooling tower water flow rate ratio calibration.");
                ShowContinueError(state, "Design inlet air wet-bulb or range temperature must be modified to achieve the design approach");
                ShowContinueError(state,
                                  format("A water flow rate ratio of {:.6F} was calculated to yield an approach temperature of {:.2F}.",
                                         WaterFlowRateRatio,
                                         Tapproach));
                ShowFatalError(state, format("Cooling tower calibration failed for tower {}.", this->Name));
            }

            this->CalibratedWaterFlowRate = this->DesignWaterFlowRate / WaterFlowRatio;

            if (WaterFlowRatio < this->MinWaterFlowRatio || WaterFlowRatio > this->MaxWaterFlowRatio) {
                ShowWarningError(state,
                                 format("CoolingTower:VariableSpeed, \"{}\" the calibrated water flow rate ratio is determined to be {:9.6F}. This "
                                        "is outside the valid range of {:.2F} to {:.2F}.",
                                        this->Name,
                                        WaterFlowRatio,
                                        this->MinWaterFlowRatio,
                                        this->MaxWaterFlowRatio));
            }

            Real64 const rho =
                this->plantLoc.loop->glycol->getDensity(state, (this->DesignInletWB + this->DesignApproach + this->DesignRange), RoutineName);
            Real64 const Cp =
                this->plantLoc.loop->glycol->getSpecificHeat(state, (this->DesignInletWB + this->DesignApproach + this->DesignRange), RoutineName);

            this->TowerNominalCapacity = ((rho * tmpDesignWaterFlowRate) * Cp * this->DesignRange);
            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Nominal Capacity [W]",
                                             this->TowerNominalCapacity);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Initial Nominal Capacity [W]",
                                             this->TowerNominalCapacity);
            }
            this->FreeConvAirFlowRate = this->MinimumVSAirFlowFrac * this->HighSpeedAirFlowRate;

            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Air Flow Rate in free convection regime [m3/s]",
                                             this->FreeConvAirFlowRate);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Initial Air Flow Rate in free convection regime [m3/s]",
                                             this->FreeConvAirFlowRate);
            }
            this->TowerFreeConvNomCap = this->TowerNominalCapacity * this->FreeConvectionCapacityFraction;

            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Tower capacity in free convection regime at design conditions [W]",
                                             this->TowerFreeConvNomCap);
            }
            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                BaseSizer::reportSizerOutput(state,
                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                             this->Name,
                                             "Initial Tower capacity in free convection regime at design conditions [W]",
                                             this->TowerFreeConvNomCap);
            }
        }
        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            // create predefined report
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMechType, this->Name, DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)]);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->TowerNominalCapacity);

            // create std 229 new table for cooling towers and fluid coolers
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCType, this->Name, DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)]);
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchCTFCCondLoopName,
                                                     this->Name,
                                                     (this->plantLoc.loop != nullptr) ? this->plantLoc.loop->Name : "N/A");
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchCTFCCondLoopBranchName,
                                                     this->Name,
                                                     (this->plantLoc.branch != nullptr) ? plantLoc.branch->Name : "N/A");
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchCTFCFluidType,
                                                     this->Name,
                                                     this->plantLoc.loop->glycol->Name); // Fluid Name more reasonable than FluidType
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCRange, this->Name, this->DesignRange);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCApproach, this->Name, this->DesignApproach);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCDesFanPwr, this->Name, this->HighSpeedFanPower); // equivalent to Design Fan Power?
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCDesInletAirWBT, this->Name, this->DesignInletWB);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCDesWaterFlowRate, this->Name, this->DesignWaterFlowRate, 6);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCLevWaterSPTemp, this->Name, this->DesOutletWaterTemp);
        }

        // input error checking
        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            bool ErrorsFound = false;
            if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_SingleSpd) {
                if (this->DesignWaterFlowRate > 0.0) {
                    if (this->FreeConvAirFlowRate >= this->HighSpeedAirFlowRate) {
                        ShowSevereError(state,
                                        format("{} \"{}\". Free convection air flow rate must be less than the design air flow rate.",
                                               cCoolingTower_SingleSpeed,
                                               this->Name));
                        ErrorsFound = true;
                    }
                    if (this->FreeConvTowerUA >= this->HighSpeedTowerUA) {
                        ShowSevereError(
                            state,
                            format("{} \"{}\". Free convection UA must be less than the design tower UA.", cCoolingTower_SingleSpeed, this->Name));
                        ErrorsFound = true;
                    }
                }
            }

            if (this->TowerType == DataPlant::PlantEquipmentType::CoolingTower_TwoSpd) {
                if (this->DesignWaterFlowRate > 0.0) {
                    if (this->HighSpeedAirFlowRate <= this->LowSpeedAirFlowRate) {
                        ShowSevereError(state,
                                        format("{} \"{}\". Low speed air flow rate must be less than the high speed air flow rate.",
                                               cCoolingTower_TwoSpeed,
                                               this->Name));
                        ErrorsFound = true;
                    }
                    if (this->LowSpeedAirFlowRate <= this->FreeConvAirFlowRate) {
                        ShowSevereError(state,
                                        format("{} \"{}\". Free convection air flow rate must be less than the low speed air flow rate.",
                                               cCoolingTower_TwoSpeed,
                                               this->Name));
                        ErrorsFound = true;
                    }
                    if (this->HighSpeedTowerUA <= this->LowSpeedTowerUA) {
                        ShowSevereError(state,
                                        format("{} \"{}\". Tower UA at low fan speed must be less than the tower UA at high fan speed.",
                                               cCoolingTower_TwoSpeed,
                                               this->Name));
                        ErrorsFound = true;
                    }
                    if (this->LowSpeedTowerUA <= this->FreeConvTowerUA) {
                        ShowSevereError(
                            state,
                            format("{} \"{}\". Tower UA at free convection air flow rate must be less than the tower UA at low fan speed.",
                                   cCoolingTower_TwoSpeed,
                                   this->Name));
                        ErrorsFound = true;
                    }
                }
            }
            if (ErrorsFound) {
                ShowFatalError(state, "initialize: Program terminated due to previous condition(s).");
            }
        }
    }

    void CoolingTower::SizeVSMerkelTower(EnergyPlusData &state)
    {

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500);    // Maximum number of iterations
        Real64 constexpr Acc(0.0001); // Accuracy of result
        static constexpr std::string_view RoutineName("SizeTower");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SolFla; // Flag of solver
        Real64 tmpHighSpeedFanPower;

        Real64 UA0;                       // Lower bound for UA [W/C]
        Real64 UA1;                       // Upper bound for UA [W/C]
        Real64 DesTowerLoad;              // Design tower load [W]
        Real64 Cp(0);                     // local specific heat for fluid
        Real64 rho(0);                    // local density for fluid
        Real64 UA;                        // Calculated UA value
        Real64 DesTowerInletWaterTemp;    // design tower inlet water temperature
        Real64 DesTowerExitWaterTemp;     // design tower exit water temperature
        Real64 DesTowerWaterDeltaT;       // design tower temperature range
        Real64 DesTowerApproachFromPlant; // design tower approach temperature from plant sizing object
        Real64 TolTemp(0.04);             // DeltaT and DesignApproach diffs tolerance between plant sizing data and user input in cooling tower
        // for warning message reporting purpose only

        // Find the appropriate Plant Sizing object
        int PltSizCondNum = this->plantLoc.loop->PlantSizNum;

        Real64 tmpNomTowerCap = this->TowerNominalCapacity;
        Real64 tmpDesignWaterFlowRate = this->DesignWaterFlowRate;
        Real64 tmpTowerFreeConvNomCap = this->TowerFreeConvNomCap;
        Real64 tmpDesignAirFlowRate = this->HighSpeedAirFlowRate;
        Real64 tmpFreeConvAirFlowRate = this->FreeConvAirFlowRate;
        Real64 DesTowerInletAirWBTemp = this->DesignInletWB;
        Real64 DesTowerInletAirDBTemp = this->DesInletAirDBTemp;

        auto const &PlantSizData(state.dataSize->PlantSizData);

        if (this->TowerInletCondsAutoSize) {
            if (PltSizCondNum > 0) {
                // use plant sizing data
                DesTowerExitWaterTemp = PlantSizData(PltSizCondNum).ExitTemp;
                DesTowerInletWaterTemp = DesTowerExitWaterTemp + PlantSizData(PltSizCondNum).DeltaT;
                DesTowerWaterDeltaT = PlantSizData(PltSizCondNum).DeltaT;
            } else {
                // set default values to replace hard wired input assumptions
                DesTowerExitWaterTemp = this->DesOutletWaterTemp;
                DesTowerInletWaterTemp = this->DesInletWaterTemp;
                DesTowerWaterDeltaT = this->DesignRange;
            }
        } else {
            // use tower sizing data
            DesTowerExitWaterTemp = this->DesOutletWaterTemp;
            DesTowerInletWaterTemp = this->DesInletWaterTemp;
            DesTowerWaterDeltaT = this->DesignRange;
            if (PltSizCondNum > 0) {
                // check the tower range against the plant sizing data
                if (std::abs(DesTowerWaterDeltaT - PlantSizData(PltSizCondNum).DeltaT) > TolTemp) {
                    ShowWarningError(state,
                                     format("Error when autosizing the load for cooling tower = {}. Tower Design Range Temperature is different from "
                                            "the Design Loop Delta Temperature.",
                                            this->Name));
                    ShowContinueError(state, format("Tower Design Range Temperature specified in tower = {}", this->Name));
                    ShowContinueError(state,
                                      format("is inconsistent with Design Loop Delta Temperature specified in Sizing:Plant object = {}.",
                                             PlantSizData(PltSizCondNum).PlantLoopName));
                    ShowContinueError(state, format("..The Design Range Temperature specified in tower is = {:.2T}", this->DesignRange));
                    ShowContinueError(
                        state,
                        format("..The Design Loop Delta Temperature specified in plant sizing data is = {:.2T}", PlantSizData(PltSizCondNum).DeltaT));
                }
                // check if the tower approach is different from plant sizing data
                DesTowerApproachFromPlant = PlantSizData(PltSizCondNum).ExitTemp - this->DesignInletWB;
                if (std::abs(DesTowerApproachFromPlant - this->DesignApproach) > TolTemp) {
                    ShowWarningError(state,
                                     format("Error when autosizing the UA for cooling tower = {}. Tower Design Approach Temperature is inconsistent "
                                            "with Approach from Plant Sizing Data.",
                                            this->Name));
                    ShowContinueError(state,
                                      format("The Design Approach Temperature from inputs specified in Sizing:Plant object = {}",
                                             PlantSizData(PltSizCondNum).PlantLoopName));
                    ShowContinueError(state, format("is inconsistent with Design Approach Temperature specified in tower = {}.", this->Name));
                    ShowContinueError(state,
                                      format("..The Design Approach Temperature from inputs specified is = {:.2T}", DesTowerApproachFromPlant));
                    ShowContinueError(state, format("..The Design Approach Temperature specified in tower is = {:.2T}", this->DesignApproach));
                }
            }
        }

        if (this->PerformanceInputMethod_Num == PIM::NominalCapacity) {

            if (PltSizCondNum > 0) { // get nominal capacity from PlantSizData(PltSizCondNum)%DeltaT and PlantSizData(PltSizCondNum)%DesVolFlowRate
                if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                    rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                    Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                    DesTowerLoad = rho * Cp * PlantSizData(PltSizCondNum).DesVolFlowRate * DesTowerWaterDeltaT * this->SizFac;
                    tmpNomTowerCap = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                } else {
                    if (this->TowerNominalCapacityWasAutoSized) {
                        tmpNomTowerCap = 0.0;
                    }
                }
            } else {                                  // PltSizCondNum = 0
                if (!this->TowerInletCondsAutoSize) { // can use design data entered into tower object
                    if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {
                        rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                        Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                        DesTowerLoad = rho * Cp * this->DesignWaterFlowRate * DesTowerWaterDeltaT * this->SizFac;
                        tmpNomTowerCap = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                    } else {
                        if (this->TowerNominalCapacityWasAutoSized) {
                            tmpNomTowerCap = 0.0;
                        }
                    }
                } else { // do not have enough data to size.
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize && this->TowerNominalCapacityWasAutoSized) {
                        ShowSevereError(state, format("Autosizing error for cooling tower object = {}", this->Name));
                        ShowFatalError(state, "Autosizing of cooling tower nominal capacity requires a loop Sizing:Plant object.");
                    }
                }
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->TowerNominalCapacityWasAutoSized) {
                    this->TowerNominalCapacity = tmpNomTowerCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Nominal Capacity [W]",
                                                     tmpNomTowerCap);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Design Nominal Capacity [W]",
                                                     this->TowerNominalCapacity);
                    }
                } else { // Hard-sized with sizing data
                    if (this->TowerNominalCapacity > 0.0 && tmpNomTowerCap > 0.0) {
                        Real64 NomCapUser(0.0);
                        NomCapUser = this->TowerNominalCapacity;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Design Nominal Capacity [W]",
                                                         tmpNomTowerCap,
                                                         "User-Specified Nominal Capacity [W]",
                                                         NomCapUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(tmpNomTowerCap - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state, format("SizeVSMerkelTower: Potential issue with equipment sizing for {}", this->Name));
                                    ShowContinueError(state, format("User-Specified Nominal Capacity of {:.2R} [W]", NomCapUser));
                                    ShowContinueError(state, format("differs from Design Size Nominal Capacity of {:.2R} [W]", tmpNomTowerCap));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                            tmpNomTowerCap = NomCapUser;
                        }
                    }
                }
            }

            tmpTowerFreeConvNomCap = tmpNomTowerCap * this->TowerFreeConvNomCapSizingFactor;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->TowerFreeConvNomCapWasAutoSized) {
                    this->TowerFreeConvNomCap = tmpTowerFreeConvNomCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Free Convection Nominal Capacity [W]",
                                                     this->TowerFreeConvNomCap);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Design Free Convection Nominal Capacity [W]",
                                                     this->TowerFreeConvNomCap);
                    }
                } else { // Hard-sized with sizing data
                    if (this->TowerFreeConvNomCap > 0.0 && tmpTowerFreeConvNomCap > 0.0) {
                        Real64 NomCapUser(0.0);
                        NomCapUser = this->TowerFreeConvNomCap;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Design Free Convection Nominal Capacity [W]",
                                                         tmpTowerFreeConvNomCap,
                                                         "User-Specified Free Convection Nominal Capacity [W]",
                                                         NomCapUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(tmpTowerFreeConvNomCap - NomCapUser) / NomCapUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state, format("SizeVSMerkelTower: Potential issue with equipment sizing for {}", this->Name));
                                    ShowContinueError(state, format("User-Specified Free Convection Nominal Capacity of {:.2R} [W]", NomCapUser));
                                    ShowContinueError(
                                        state,
                                        format("differs from Design Size Free Convection Nominal Capacity of {:.2R} [W]", tmpTowerFreeConvNomCap));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                            tmpTowerFreeConvNomCap = NomCapUser;
                        }
                    }
                }
            }

            tmpDesignWaterFlowRate = tmpNomTowerCap * this->DesignWaterFlowPerUnitNomCap;
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->DesignWaterFlowRateWasAutoSized) {
                    // for nominal cap input method, get design water flow rate from nominal cap and scalable sizing factor

                    this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Water Flow Rate [m3/s]",
                                                     this->DesignWaterFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Design Water Flow Rate [m3/s]",
                                                     this->DesignWaterFlowRate);
                    }

                } else { // Hard-sized with sizing data
                    if (this->DesignWaterFlowRate > 0.0 && tmpDesignWaterFlowRate > 0.0) {
                        Real64 NomDesWaterFlowUser(0.0);
                        NomDesWaterFlowUser = this->DesignWaterFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Design Water Flow Rate [m3/s]",
                                                         this->DesignWaterFlowRate,
                                                         "User-Specified Design Water Flow Rate [m3/s]",
                                                         NomDesWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(tmpDesignWaterFlowRate - NomDesWaterFlowUser) / NomDesWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state, format("SizeVSMerkelTower: Potential issue with equipment sizing for {}", this->Name));
                                    ShowContinueError(state, format("User-Specified Design Water Flow Rate of {:.2R} [m3/s]", NomDesWaterFlowUser));
                                    ShowContinueError(state, format("differs from Design Water Flow Rate of {:.2R} [m3/s]", tmpDesignWaterFlowRate));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                            tmpDesignWaterFlowRate = NomDesWaterFlowUser;
                        }
                    }
                }
            }

            PlantUtilities::RegisterPlantCompDesignFlow(state, this->WaterInletNodeNum, tmpDesignWaterFlowRate);

            if (this->DefaultedDesignAirFlowScalingFactor) {
                tmpDesignAirFlowRate = tmpNomTowerCap * this->DesignAirFlowPerUnitNomCap * (101325.0 / state.dataEnvrn->StdBaroPress);
            } else {
                tmpDesignAirFlowRate = tmpNomTowerCap * this->DesignAirFlowPerUnitNomCap;
            }
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->HighSpeedAirFlowRateWasAutoSized) {
                    this->HighSpeedAirFlowRate = tmpDesignAirFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Air Flow Rate [m3/s]",
                                                     this->HighSpeedAirFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Design Air Flow Rate [m3/s]",
                                                     this->HighSpeedAirFlowRate);
                    }
                } else { // Hard-sized with sizing data
                    Real64 DesignAirFlowRateUser(0.0);
                    DesignAirFlowRateUser = this->HighSpeedAirFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Air Flow Rate [m3/s]",
                                                     tmpDesignAirFlowRate,
                                                     "User-Specified Design Air Flow Rate [m3/s]",
                                                     DesignAirFlowRateUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpDesignAirFlowRate - DesignAirFlowRateUser) / DesignAirFlowRateUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeVSMerkelTower: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(state, format("User-Specified Design Air Flow Rate of {:.2R} [m3/s]", DesignAirFlowRateUser));
                                ShowContinueError(state, format("differs from Design Air Flow Rate of {:.2R} [m3/s]", tmpDesignAirFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                        tmpDesignAirFlowRate = DesignAirFlowRateUser;
                    }
                }
            }
            tmpFreeConvAirFlowRate = tmpDesignAirFlowRate * this->FreeConvAirFlowRateSizingFactor;

            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (this->FreeConvAirFlowRateWasAutoSized) {
                    this->FreeConvAirFlowRate = tmpFreeConvAirFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Free Convection Regime Air Flow Rate [m3/s]",
                                                     this->FreeConvAirFlowRate);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Design Free Convection Regime Air Flow Rate [m3/s]",
                                                     this->FreeConvAirFlowRate);
                    }
                } else { // Hard-sized with sizing data
                    Real64 FreeConvAirFlowUser(0.0);
                    FreeConvAirFlowUser = this->FreeConvAirFlowRate;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Design Free Convection Regime Air Flow Rate [m3/s]",
                                                     tmpFreeConvAirFlowRate,
                                                     "User-Specified Design Free Convection Regime Air Flow Rate [m3/s]",
                                                     FreeConvAirFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(tmpFreeConvAirFlowRate - FreeConvAirFlowUser) / FreeConvAirFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state, format("SizeVSMerkelTower: Potential issue with equipment sizing for {}", this->Name));
                                ShowContinueError(
                                    state,
                                    format("User-Specified Design Free Convection Regime Air Flow Rate of {:.2R} [m3/s]", FreeConvAirFlowUser));
                                ShowContinueError(
                                    state,
                                    format("differs from Design Free Convection Regime Air Flow Rate of {:.2R} [m3/s]", tmpFreeConvAirFlowRate));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                        tmpFreeConvAirFlowRate = FreeConvAirFlowUser;
                    }
                }
            }

            // now calculate UA values from nominal capacities and flow rates
            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                if (PltSizCondNum > 0) { // user has a plant sizing object
                    Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                    this->WaterTemp = DesTowerInletWaterTemp;
                } else { // probably no plant sizing object
                    Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, Constant::InitConvTemp, RoutineName);
                    this->WaterTemp = DesTowerInletWaterTemp; // 35.0; // design condition
                }
                rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);

                // full speed fan tower UA
                Real64 const solveLoad = tmpNomTowerCap * this->HeatRejectCapNomCapSizingRatio;
                Real64 const solveWaterFlow = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                UA0 = 0.0001 * solveLoad;                                   // Assume deltaT = 10000K (limit)
                UA1 = solveLoad;                                            // Assume deltaT = 1K

                this->AirTemp = this->DesInletAirDBTemp; // 35.0;
                this->AirWetBulb = this->DesignInletWB;  // 25.6;
                this->AirPress = state.dataEnvrn->StdBaroPress;
                this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                auto f = [&state, this, solveLoad, solveWaterFlow, tmpDesignAirFlowRate, Cp](Real64 UA) {
                    Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow, tmpDesignAirFlowRate, UA);
                    Real64 const CoolingOutput = Cp * solveWaterFlow * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                    return (solveLoad - CoolingOutput) / solveLoad;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                    ShowFatalError(state, format("calculating cooling tower UA failed for tower {}", this->Name));
                } else if (SolFla == -2) {
                    ShowSevereError(state, "Bad starting values for UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                }
                this->HighSpeedTowerUA = UA;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "U-Factor Times Area Value at Full Speed Air Flow Rate [W/C]",
                                                 this->HighSpeedTowerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial U-Factor Times Area Value at Full Speed Air Flow Rate [W/C]",
                                                 this->HighSpeedTowerUA);
                }
                // free convection tower UA
                Real64 const solveLoad1 = tmpTowerFreeConvNomCap * this->HeatRejectCapNomCapSizingRatio;
                Real64 solveWaterFlow1 = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                UA0 = 0.0001 * solveLoad1;                             // Assume deltaT = 10000K (limit)
                UA0 = max(UA0, 1.0);                                   // limit to 1.0
                UA1 = solveLoad1;                                      // Assume deltaT = 1K
                this->AirTemp = this->DesInletAirDBTemp;               // 35.0;
                this->AirWetBulb = this->DesignInletWB;                // 25.6;
                this->AirPress = state.dataEnvrn->StdBaroPress;
                this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                auto f2 = [&state, this, solveLoad1, solveWaterFlow1, tmpFreeConvAirFlowRate, Cp](Real64 UA) {
                    Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow1, tmpFreeConvAirFlowRate, UA);
                    Real64 const CoolingOutput = Cp * solveWaterFlow1 * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                    return (solveLoad1 - CoolingOutput) / solveLoad1;
                };
                General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f2, UA0, UA1);
                if (SolFla == -1) {
                    ShowSevereError(state, "Iteration limit exceeded in calculating tower free convection UA");
                    ShowFatalError(state, format("calculating cooling tower UA failed for tower {}", this->Name));
                } else if (SolFla == -2) {
                    ShowSevereError(state, "Bad starting values for UA");
                    ShowFatalError(state, format("Autosizing of cooling tower UA failed for free convection tower {}", this->Name));
                }
                this->FreeConvTowerUA = UA;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                                 this->FreeConvTowerUA);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                                 this->FreeConvTowerUA);
                }
            }
        } else if (this->PerformanceInputMethod_Num == PIM::UFactor) {
            // UA input method

            if (this->DesignWaterFlowRateWasAutoSized) { // get from plant sizing
                // UA input method using plant sizing for flow rate, whereas Nominal capacity method uses scalable sizing factor per cap
                if (PltSizCondNum > 0) {
                    if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                        tmpDesignWaterFlowRate = PlantSizData(PltSizCondNum).DesVolFlowRate * this->SizFac;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                             this->Name,
                                                             "Design Water Flow Rate [m3/s]",
                                                             this->DesignWaterFlowRate);
                            }
                            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                             this->Name,
                                                             "Initial Design Water Flow Rate [m3/s]",
                                                             this->DesignWaterFlowRate);
                            }
                        }
                    } else {
                        tmpDesignWaterFlowRate = 0.0;
                    }

                } else {
                    if (!this->TowerInletCondsAutoSize) {
                        if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {
                            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                                this->DesignWaterFlowRate = tmpDesignWaterFlowRate;
                                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Design Water Flow Rate [m3/s]",
                                                                 this->DesignWaterFlowRate);
                                }
                                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Initial Design Water Flow Rate [m3/s]",
                                                                 this->DesignWaterFlowRate);
                                }
                            }
                        } else {
                            tmpDesignWaterFlowRate = 0.0;
                        }
                    } else {
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            ShowSevereError(state, format("Autosizing error for cooling tower object = {}", this->Name));
                            ShowFatalError(state, "Autosizing of cooling tower nominal capacity requires a loop Sizing:Plant object.");
                        }
                    }
                }
            }
            PlantUtilities::RegisterPlantCompDesignFlow(state, this->WaterInletNodeNum, tmpDesignWaterFlowRate);

            if (this->HighSpeedTowerUAWasAutoSized) {
                // get nominal capacity from PlantSizData(PltSizCondNum)%DeltaT and PlantSizData(PltSizCondNum)%DesVolFlowRate
                if (PltSizCondNum > 0) {
                    if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                        rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                        Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                        DesTowerLoad = rho * Cp * PlantSizData(PltSizCondNum).DesVolFlowRate * DesTowerWaterDeltaT * this->SizFac;
                        tmpNomTowerCap = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->TowerNominalCapacity = tmpNomTowerCap;
                            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                             this->Name,
                                                             "Nominal Capacity [W]",
                                                             this->TowerNominalCapacity);
                            }
                            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                             this->Name,
                                                             "Initial Nominal Capacity [W]",
                                                             this->TowerNominalCapacity);
                            }
                        }
                    } else {
                        tmpNomTowerCap = 0.0;
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            this->TowerNominalCapacity = tmpNomTowerCap;
                            if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                             this->Name,
                                                             "Nominal Capacity [W]",
                                                             this->TowerNominalCapacity);
                            }
                            if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                BaseSizer::reportSizerOutput(state,
                                                             DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                             this->Name,
                                                             "Initial Nominal Capacity [W]",
                                                             this->TowerNominalCapacity);
                            }
                        }
                    }
                } else {
                    if (!this->TowerInletCondsAutoSize) {
                        if (this->DesignWaterFlowRate >= HVAC::SmallWaterVolFlow) {
                            rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                            Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                            DesTowerLoad = rho * Cp * this->DesignWaterFlowRate * DesTowerWaterDeltaT * this->SizFac;
                            tmpNomTowerCap = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                                this->TowerNominalCapacity = tmpNomTowerCap;
                                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Initial Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                            }
                        } else {
                            tmpNomTowerCap = 0.0;
                            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                                this->TowerNominalCapacity = tmpNomTowerCap;
                                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Initial Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                            }
                        }
                    } else {
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            ShowSevereError(state, format("Autosizing error for cooling tower object = {}", this->Name));
                            ShowFatalError(state, "Autosizing of cooling tower nominal capacity requires a loop Sizing:Plant object.");
                        }
                    }
                }
                if (this->TowerFreeConvNomCapWasAutoSized) {
                    tmpTowerFreeConvNomCap = tmpNomTowerCap * this->TowerFreeConvNomCapSizingFactor;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->TowerFreeConvNomCap = tmpTowerFreeConvNomCap;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Free Convection Nominal Capacity [W]",
                                                         this->TowerFreeConvNomCap);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial Free Convection Nominal Capacity [W]",
                                                         this->TowerFreeConvNomCap);
                        }
                    }
                }
                if (this->HighSpeedAirFlowRateWasAutoSized) {
                    if (this->DefaultedDesignAirFlowScalingFactor) {
                        tmpDesignAirFlowRate = tmpNomTowerCap * this->DesignAirFlowPerUnitNomCap * (101325.0 / state.dataEnvrn->StdBaroPress);
                    } else {
                        tmpDesignAirFlowRate = tmpNomTowerCap * this->DesignAirFlowPerUnitNomCap;
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedAirFlowRate = tmpDesignAirFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Design Air Flow Rate [m3/s]",
                                                         this->HighSpeedAirFlowRate);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial Design Air Flow Rate [m3/s]",
                                                         this->HighSpeedAirFlowRate);
                        }
                    }
                }
                if (this->FreeConvAirFlowRateWasAutoSized) {
                    tmpFreeConvAirFlowRate = tmpDesignAirFlowRate * this->FreeConvAirFlowRateSizingFactor;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->FreeConvAirFlowRate = tmpFreeConvAirFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Free Convection Regime Air Flow Rate [m3/s]",
                                                         this->FreeConvAirFlowRate);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial Free Convection Regime Air Flow Rate [m3/s]",
                                                         this->FreeConvAirFlowRate);
                        }
                    }
                }
                // now calculate UA values from nominal capacities and flow rates
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    rho = this->plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
                    Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                    // full speed fan tower UA
                    Real64 const solveLoad = tmpNomTowerCap * this->HeatRejectCapNomCapSizingRatio;
                    Real64 const solveWaterFlow = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                    UA0 = 0.0001 * solveLoad;                                   // Assume deltaT = 10000K (limit)
                    UA1 = solveLoad;                                            // Assume deltaT = 1K
                    this->WaterTemp = DesTowerInletWaterTemp;
                    this->AirTemp = this->DesInletAirDBTemp; // 35.0;
                    this->AirWetBulb = this->DesignInletWB;  // 25.6;
                    this->AirPress = state.dataEnvrn->StdBaroPress;
                    this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                    auto f = [&state, this, solveLoad, solveWaterFlow, tmpDesignAirFlowRate, Cp](Real64 UA) {
                        Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow, tmpDesignAirFlowRate, UA);
                        Real64 const CoolingOutput = Cp * solveWaterFlow * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                        return (solveLoad - CoolingOutput) / solveLoad;
                    };
                    General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f, UA0, UA1);
                    if (SolFla == -1) {
                        ShowSevereError(state, "Iteration limit exceeded in calculating tower UA");
                        ShowFatalError(state, format("calculating cooling tower UA failed for tower {}", this->Name));
                    } else if (SolFla == -2) {
                        ShowSevereError(state, "Bad starting values for UA");
                        ShowFatalError(state, format("Autosizing of cooling tower UA failed for tower {}", this->Name));
                    }
                    this->HighSpeedTowerUA = UA;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "U-Factor Times Area Value at Full Speed Air Flow Rate [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at Full Speed Air Flow Rate [W/C]",
                                                     this->HighSpeedTowerUA);
                    }
                    // free convection tower UA
                    Real64 const solveLoad2 = tmpTowerFreeConvNomCap * this->HeatRejectCapNomCapSizingRatio;
                    Real64 const solveWaterFlow2 = rho * tmpDesignWaterFlowRate; // design water mass flow rate
                    UA0 = 0.0001 * solveLoad2;                                   // Assume deltaT = 10000K (limit)
                    UA1 = solveLoad2;                                            // Assume deltaT = 1K
                    this->WaterTemp = DesTowerInletWaterTemp;
                    this->AirTemp = DesTowerInletAirDBTemp;    // 35.0;
                    this->AirWetBulb = DesTowerInletAirWBTemp; // 25.6;
                    this->AirPress = state.dataEnvrn->StdBaroPress;
                    this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                    auto f3 = [&state, this, solveLoad2, solveWaterFlow2, tmpFreeConvAirFlowRate, Cp](Real64 UA) {
                        Real64 const OutWaterTemp = this->calculateSimpleTowerOutletTemp(state, solveWaterFlow2, tmpFreeConvAirFlowRate, UA);
                        Real64 const CoolingOutput = Cp * solveWaterFlow2 * (this->WaterTemp - OutWaterTemp); // tower cooling output [W]
                        return (solveLoad2 - CoolingOutput) / solveLoad2;
                    };
                    General::SolveRoot(state, Acc, MaxIte, SolFla, UA, f3, UA0, UA1);
                    if (SolFla == -1) {
                        ShowSevereError(state, "Iteration limit exceeded in calculating tower free convection UA");
                        ShowFatalError(state, format("calculating cooling tower UA failed for tower {}", this->Name));
                    } else if (SolFla == -2) {
                        ShowSevereError(state, "Bad starting values for UA");
                        ShowFatalError(state, format("Autosizing of cooling tower UA failed for free convection tower {}", this->Name));
                    }
                    this->LowSpeedTowerUA = UA;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                                     this->FreeConvTowerUA);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                                     this->FreeConvTowerUA);
                    }
                }

            } else { // full speed UA given

                if (this->FreeConvTowerUAWasAutoSized) { // determine from scalable sizing factor
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->FreeConvTowerUA = this->HighSpeedTowerUA * this->FreeConvTowerUASizingFactor;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                                         this->FreeConvTowerUA);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial U-Factor Times Area Value at Free Convection Air Flow Rate [W/C]",
                                                         this->FreeConvTowerUA);
                        }
                    }
                }
                Real64 OutWaterTemp;
                if (this->HighSpeedAirFlowRateWasAutoSized) { // given UA but not air flow rate
                    // need an air flow rate to find capacity from UA but flow rate is scaled off capacity
                    // get nominal capacity from PlantSizData(PltSizCondNum)%DeltaT and PlantSizData(PltSizCondNum)%DesVolFlowRate
                    if (PltSizCondNum > 0) {
                        if (PlantSizData(PltSizCondNum).DesVolFlowRate >= HVAC::SmallWaterVolFlow) {
                            rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                            Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);
                            DesTowerLoad = rho * Cp * PlantSizData(PltSizCondNum).DesVolFlowRate * DesTowerWaterDeltaT;
                            tmpNomTowerCap = DesTowerLoad / this->HeatRejectCapNomCapSizingRatio;
                            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                                this->TowerNominalCapacity = tmpNomTowerCap;
                                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Initial Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                            }
                        } else {
                            tmpNomTowerCap = rho = Cp = 0.0; // rho and Cp added: Used below
                            if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                                this->TowerNominalCapacity = tmpNomTowerCap;
                                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                                    BaseSizer::reportSizerOutput(state,
                                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                                 this->Name,
                                                                 "Initial Nominal Capacity [W]",
                                                                 this->TowerNominalCapacity);
                                }
                            }
                        }

                    } else {
                        tmpNomTowerCap = 0.0; // Suppress uninitialized warnings
                        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                            ShowSevereError(state, format("Autosizing error for cooling tower object = {}", this->Name));
                            ShowFatalError(state, "Autosizing of cooling tower nominal capacity requires a loop Sizing:Plant object.");
                        }
                    }

                    if (this->DefaultedDesignAirFlowScalingFactor) {
                        tmpDesignAirFlowRate = tmpNomTowerCap * this->DesignAirFlowPerUnitNomCap * (101325.0 / state.dataEnvrn->StdBaroPress);
                    } else {
                        tmpDesignAirFlowRate = tmpNomTowerCap * this->DesignAirFlowPerUnitNomCap;
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->HighSpeedAirFlowRate = tmpDesignAirFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Design Air Flow Rate [m3/s]",
                                                         this->HighSpeedAirFlowRate);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial Design Air Flow Rate [m3/s]",
                                                         this->HighSpeedAirFlowRate);
                        }
                    }

                } else { // UA and Air flow rate given, so find Nominal Cap from running model

                    rho = this->plantLoc.loop->glycol->getDensity(state, DesTowerExitWaterTemp, RoutineName);
                    Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, DesTowerExitWaterTemp, RoutineName);

                    this->WaterTemp = DesTowerInletWaterTemp;
                    this->AirTemp = DesTowerInletAirDBTemp;    // 35.0;
                    this->AirWetBulb = DesTowerInletAirWBTemp; // 25.6;
                    this->AirPress = state.dataEnvrn->StdBaroPress;
                    this->AirHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, this->AirTemp, this->AirWetBulb, this->AirPress);
                    OutWaterTemp =
                        this->calculateSimpleTowerOutletTemp(state, rho * tmpDesignWaterFlowRate, this->HighSpeedAirFlowRate, this->HighSpeedTowerUA);
                    tmpNomTowerCap = Cp * rho * tmpDesignWaterFlowRate * (this->WaterTemp - OutWaterTemp);
                    tmpNomTowerCap /= this->HeatRejectCapNomCapSizingRatio;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->TowerNominalCapacity = tmpNomTowerCap;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Nominal Capacity [W]",
                                                         this->TowerNominalCapacity);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial Nominal Capacity [W]",
                                                         this->TowerNominalCapacity);
                        }
                    }

                } // both UA and air flow rate given

                if (this->FreeConvAirFlowRateWasAutoSized) {
                    tmpFreeConvAirFlowRate = tmpDesignAirFlowRate * this->FreeConvAirFlowRateSizingFactor;
                    if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                        this->FreeConvAirFlowRate = tmpFreeConvAirFlowRate;
                        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Free Convection Regime Air Flow Rate [m3/s]",
                                                         this->FreeConvAirFlowRate);
                        }
                        if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                            BaseSizer::reportSizerOutput(state,
                                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                         this->Name,
                                                         "Initial Free Convection Regime Air Flow Rate [m3/s]",
                                                         this->FreeConvAirFlowRate);
                        }
                    }
                }

                OutWaterTemp =
                    this->calculateSimpleTowerOutletTemp(state, rho * tmpDesignWaterFlowRate, tmpFreeConvAirFlowRate, this->FreeConvTowerUA);
                tmpTowerFreeConvNomCap = Cp * rho * tmpDesignWaterFlowRate * (this->WaterTemp - OutWaterTemp);
                tmpTowerFreeConvNomCap /= this->HeatRejectCapNomCapSizingRatio;
                if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
                    this->TowerFreeConvNomCap = tmpTowerFreeConvNomCap;
                    if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Free Convection Nominal Capacity [W]",
                                                     this->TowerFreeConvNomCap);
                    }
                    if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                        BaseSizer::reportSizerOutput(state,
                                                     DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                     this->Name,
                                                     "Initial Free Convection Nominal Capacity [W]",
                                                     this->TowerFreeConvNomCap);
                    }
                }
            }
        }

        tmpHighSpeedFanPower = tmpNomTowerCap * this->DesignFanPowerPerUnitNomCap;
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (this->HighSpeedFanPowerWasAutoSized) {

                this->HighSpeedFanPower = tmpHighSpeedFanPower;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Design Fan Power [W]",
                                                 this->HighSpeedFanPower);
                }
                if (state.dataPlnt->PlantFirstSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Initial Design Fan Power [W]",
                                                 this->HighSpeedFanPower);
                }
            } else { // Hard-sized with sizing data
                Real64 HighSpeedFanPowerUser(0.0);
                HighSpeedFanPowerUser = this->HighSpeedAirFlowRate;
                if (state.dataPlnt->PlantFinalSizesOkayToReport) {
                    BaseSizer::reportSizerOutput(state,
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 "Design Fan Power [W]",
                                                 tmpHighSpeedFanPower,
                                                 "User-Specified Design Fan Power [W]",
                                                 HighSpeedFanPowerUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(tmpHighSpeedFanPower - HighSpeedFanPowerUser) / HighSpeedFanPowerUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizeVSMerkelTower: Potential issue with equipment sizing for {}", this->Name));
                            ShowContinueError(state, format("User-Specified Design Fan Power of {:.2R} [W]", HighSpeedFanPowerUser));
                            ShowContinueError(state, format("differs from Design Fan Power of {:.2R} [W]", tmpHighSpeedFanPower));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }

        if (state.dataPlnt->PlantFinalSizesOkayToReport) {
            // create predefined report
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMechType, this->Name, DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)]);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechNomCap, this->Name, this->TowerNominalCapacity);

            // create std 229 new table for cooling towers and fluid coolers
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCType, this->Name, DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)]);
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchCTFCCondLoopName,
                                                     this->Name,
                                                     (this->plantLoc.loop != nullptr) ? this->plantLoc.loop->Name : "N/A");
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchCTFCCondLoopBranchName,
                                                     this->Name,
                                                     (this->plantLoc.branch != nullptr) ? plantLoc.branch->Name : "N/A");
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchCTFCFluidType,
                                                     this->Name,
                                                     this->plantLoc.loop->glycol->Name); // Fluid Name more reasonable than FluidType
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCRange, this->Name, this->DesignRange);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCApproach, this->Name, this->DesignApproach);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCDesFanPwr, this->Name, this->HighSpeedFanPower); // equivalent to Design Fan Power?
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCDesInletAirWBT, this->Name, this->DesignInletWB);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCTFCDesWaterFlowRate, this->Name, this->DesignWaterFlowRate, 6);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCTFCLevWaterSPTemp, this->Name, this->DesOutletWaterTemp);
        }
    } // namespace CondenserLoopTowers

    void CoolingTower::calculateSingleSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 1998
        //       MODIFIED       Aug. 2008, T Hong, Added fluid bypass for single speed cooling tower
        //                      The OutletWaterTemp from calculateSimpleTowerOutletTemp can be lower than 0 degreeC
        //                      which may not be allowed in practice if water is the tower fluid.
        //                      Chandan Sharma, FSEC, February 2010, Added basin heater
        //                      Jul. 2010, A Flament, added multi-cell capability for the 3 types of cooling tower
        //                      Jun. 2016, R Zhang, Applied the condenser supply water temperature sensor fault model
        //                      Jul. 2016, R Zhang, Applied the cooling tower fouling fault model
        //       RE-ENGINEERED  Jan. 2001, Richard Raustad

        // PURPOSE OF THIS SUBROUTINE:
        // To simulate the operation of a single-speed fan cooling tower.

        // METHODOLOGY EMPLOYED:
        // The cooling tower is modeled using effectiveness-NTU relationships for
        // counterflow heat exchangers based on Merkel's theory.
        // The subroutine calculates the period of time required to meet a
        // leaving water temperature setpoint. It assumes that part-load
        // operation represents a linear interpolation of two steady-state regimes.
        // Cyclic losses are neglected. The period of time required to meet the
        // leaving water temperature setpoint is used to determine the required
        // fan power and energy. Free convection regime is also modeled. This
        // occurs when the pump is operating and the fan is off. If free convection
        // regime cooling is all that is required for a given time step, the leaving
        // water temperature is allowed to fall below the leaving water temperature
        // setpoint (free cooling). At times when the cooling tower fan is required,
        // the leaving water temperature is at or above the setpoint.
        // A RunFlag is passed by the upper level manager to indicate the ON/OFF status,
        // or schedule, of the cooling tower. If the tower is OFF, outlet water
        // temperature and flow rate are passed through the model from inlet node to
        // outlet node without intervention (with the exception of free convection
        // where water temperature is allowed to float below the outlet water set
        // point). Reports are also updated with fan power and energy being zero.
        // When the RunFlag indicates an ON condition for the cooling tower, the
        // mass flow rate and water temperature are read from the inlet node of the
        // cooling tower (water-side). The outdoor air wet-bulb temperature is used
        // as the entering condition to the cooling tower (air-side). Input deck
        // parameters are read for the free convection regime (pump ON and fan OFF)
        // and a leaving water temperature is calculated. If the leaving water temperature
        // is at or below the setpoint, the calculated leaving water temperature is
        // placed on the outlet node and no fan power is used. If the calculated leaving
        // water temperature is above the setpoint, the cooling tower fan is turned on
        // and design parameters are used to again calculate the leaving water temperature.
        // If the calculated leaving water temperature is below the setpoint, a fan
        // run-time fraction is calculated and used to determine fan power. The leaving
        // water temperature setpoint is placed on the outlet node. If the calculated
        // leaving water temperature is at or above the setpoint, the calculated
        // leaving water temperature is placed on the outlet node and the fan runs at
        // full power. Water mass flow rate is passed from inlet node to outlet node
        // with no intervention.
        // If a tower has multiple cells, the specified inputs of or the autosized capacity
        //  and air/water flow rates are for the entire tower. The number of cells to operate
        //  is first determined based on the user entered minimal and maximal water flow fractions
        //  per cell. If the loads are not met, more cells (if available) will operate to meet
        //  the loads. Inside each cell, the capacity controls still apply. Each cell operates
        //  in the same way.

        // REFERENCES:
        // ASHRAE HVAC1KIT: A Toolkit for Primary HVAC System Energy Calculation. 1999.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("calculateSingleSpeedTower");
        int constexpr MaxIteration(100); // Maximum fluid bypass iteration calculations
        static constexpr std::string_view MaxItChar("100");
        Real64 constexpr BypassFractionThreshold(0.01); // Threshold to stop bypass iteration
        Real64 constexpr OWTLowerLimit(0.0);            // The limit of tower exit fluid temperature used in the fluid bypass
        //  calculation to avoid fluid freezing. For water, it is 0 degreeC,
        //  for glycols, it can be much lower. The fluid type is stored at the loop.
        //  Current choices are Water and Steam, needs to expand for glycols

        // set inlet and outlet nodes
        this->Qactual = 0.0;
        this->FanPower = 0.0;
        this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;

        Real64 freeConvTowerUA = this->FreeConvTowerUA;
        Real64 highSpeedTowerUA = this->HighSpeedTowerUA;

        // water temperature setpoint
        Real64 TempSetPoint = 0.0;
        switch (this->plantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            if (this->SetpointIsOnOutlet) {
                TempSetPoint = state.dataLoopNodes->Node(this->WaterOutletNodeNum).TempSetPoint;
            } else {
                TempSetPoint = this->plantLoc.side->TempSetPoint;
            }
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            if (this->SetpointIsOnOutlet) {
                TempSetPoint = state.dataLoopNodes->Node(this->WaterOutletNodeNum).TempSetPointHi;
            } else {
                TempSetPoint = this->plantLoc.side->TempSetPointHi;
            }
        } break;
        default:
            break;
        }

        // If there is a fault of condenser SWT Sensor
        if (this->FaultyCondenserSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyCondenserSWTIndex;
            Real64 TowerOutletTemp_ff = TempSetPoint;

            // calculate the sensor offset using fault information
            this->FaultyCondenserSWTOffset = state.dataFaultsMgr->FaultsCondenserSWTSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint = TowerOutletTemp_ff - this->FaultyCondenserSWTOffset;
        }

        // If there is a fault of cooling tower fouling
        if (this->FaultyTowerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyTowerFoulingIndex;
            Real64 FreeConvTowerUA_ff = this->FreeConvTowerUA;
            Real64 HighSpeedTowerUA_ff = this->HighSpeedTowerUA;

            // calculate the Faulty Tower Fouling Factor using fault information
            this->FaultyTowerFoulingFactor = state.dataFaultsMgr->FaultsTowerFouling(FaultIndex).CalFaultyTowerFoulingFactor(state);

            // update the tower UA values at faulty cases
            freeConvTowerUA = FreeConvTowerUA_ff * this->FaultyTowerFoulingFactor;
            highSpeedTowerUA = HighSpeedTowerUA_ff * this->FaultyTowerFoulingFactor;
        }

        // Added for fluid bypass. First assume no fluid bypass
        int BypassFlag = 0; // Flag indicator for fluid bypass (0 - no bypass, 1 - bypass)
        Real64 BypassFraction2 = 0.0;
        this->BypassFraction = 0.0;

        // Added for multi-cell. Determine the number of cells operating
        int NumCellMin(0);
        int NumCellMax(0);
        Real64 WaterMassFlowRatePerCellMin = 0.0;
        if (this->DesWaterMassFlowRate > 0.0) {
            WaterMassFlowRatePerCellMin = this->DesWaterMassFlowRate * this->MinFracFlowRate / this->NumCell;
            Real64 WaterMassFlowRatePerCellMax = this->DesWaterMassFlowRate * this->MaxFracFlowRate / this->NumCell;

            // round it up to the nearest integer
            NumCellMin = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMax) + 0.9999), this->NumCell);
            NumCellMax = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMin) + 0.9999), this->NumCell);
        }
        // cap min at 1
        if (NumCellMin <= 0) {
            NumCellMin = 1;
        }
        if (NumCellMax <= 0) {
            NumCellMax = 1;
        }
        if (this->cellCtrl == CellCtrl::MinCell) {
            this->NumCellOn = NumCellMin;
        } else {
            this->NumCellOn = NumCellMax;
        }
        Real64 WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;

        // Do not RETURN here if flow rate is less than SmallMassFlow. Check basin heater and then RETURN.

        bool returnFlagSet = false;
        this->checkMassFlowAndLoad(state, MyLoad, RunFlag, returnFlagSet);
        if (returnFlagSet) {
            return;
        }

        bool IncrNumCellFlag = true; // determine if yes or no we increase the number of cells // set value to true to enter in the loop

        Real64 UAdesign = 0.0; // UA value at design conditions (entered by user or calculated)
        Real64 OutletWaterTempOFF;
        Real64 FanModeFrac = 0.0;
        Real64 AirFlowRate = 0.0;
        while (IncrNumCellFlag) {
            IncrNumCellFlag = false;

            //   Initialize local variables to the free convection design values
            UAdesign = freeConvTowerUA / this->NumCell;
            AirFlowRate = this->FreeConvAirFlowRate / this->NumCell;
            OutletWaterTempOFF = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->OutletWaterTemp = OutletWaterTempOFF;
            FanModeFrac = 0.0;

            OutletWaterTempOFF = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRate, UAdesign);

            //   Assume Setpoint was met using free convection regime (pump ON and fan OFF)
            this->FanPower = 0.0;
            this->OutletWaterTemp = OutletWaterTempOFF;

            if (OutletWaterTempOFF > TempSetPoint) {
                //     Setpoint was not met (or free conv. not used), turn on cooling tower fan
                UAdesign = highSpeedTowerUA / this->NumCell;
                AirFlowRate = this->HighSpeedAirFlowRate / this->NumCell;

                // The fan power is for all cells operating
                Real64 const FanPowerOn = this->HighSpeedFanPower * this->NumCellOn / this->NumCell;

                this->OutletWaterTemp = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRate, UAdesign);

                if (this->OutletWaterTemp <= TempSetPoint) {
                    if (this->CapacityControl == CapacityCtrl::FanCycling || this->OutletWaterTemp <= OWTLowerLimit) {
                        //           Setpoint was met with pump ON and fan ON, calculate run-time fraction
                        FanModeFrac = (TempSetPoint - OutletWaterTempOFF) / (this->OutletWaterTemp - OutletWaterTempOFF);
                        this->FanPower = FanModeFrac * FanPowerOn;
                        this->OutletWaterTemp = TempSetPoint;
                    } else {
                        // FluidBypass, fan runs at full speed for the entire time step
                        FanModeFrac = 1.0;
                        this->FanPower = FanPowerOn;
                        BypassFlag = 1;
                    }
                } else {
                    //         Setpoint was not met, cooling tower ran at full capacity
                    FanModeFrac = 1.0;
                    this->FanPower = FanPowerOn;
                    // if possible increase the number of cells and do the calculations again with the new water mass flow rate per cell
                    if (this->NumCellOn < this->NumCell && (this->WaterMassFlowRate / (this->NumCellOn + 1)) >= WaterMassFlowRatePerCellMin) {
                        ++this->NumCellOn;
                        WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;
                        IncrNumCellFlag = true;
                    }
                }
            } else if (OutletWaterTempOFF < TempSetPoint) {
                // Need to bypass in free convection cooling mode if bypass is allowed
                if (this->CapacityControl == CapacityCtrl::FluidBypass) {
                    if (OutletWaterTempOFF > OWTLowerLimit) {
                        BypassFlag = 1;
                    }
                }
            }
        }

        // Calculate bypass fraction since OWTLowerLimit < OutletWaterTemp < TempSetPoint.
        // The iteration ends when the number of iterations exceeds the limit or the difference
        //  between the new and old bypass fractions is less than the threshold.
        if (BypassFlag == 1) {
            // Inlet water temperature lower than setpoint, assume 100% bypass, tower fan off
            if (this->InletWaterTemp <= TempSetPoint) {
                this->FanPower = 0.0;
                this->BypassFraction = 1.0;
                this->OutletWaterTemp = this->InletWaterTemp;
            } else {
                if (std::abs(this->InletWaterTemp - this->OutletWaterTemp) <= 0.01) {
                    // Outlet temp is close enough to inlet temp, assume 100% bypass, tower fan off
                    this->BypassFraction = 1.0;
                    this->FanPower = 0.0;
                } else {
                    Real64 bypassFraction = (TempSetPoint - this->OutletWaterTemp) / (this->InletWaterTemp - this->OutletWaterTemp);
                    if (bypassFraction > 1.0 || bypassFraction < 0.0) {
                        // Bypass cannot meet setpoint, assume no bypass
                        this->BypassFraction = 0.0;
                    } else {
                        int NumIteration = 0;
                        Real64 BypassFractionPrev = bypassFraction;
                        Real64 OutletWaterTempPrev = this->OutletWaterTemp;
                        while (NumIteration < MaxIteration) {
                            ++NumIteration;
                            // need to iterate for the new OutletWaterTemp while bypassing tower water
                            this->OutletWaterTemp =
                                this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell * (1.0 - bypassFraction), AirFlowRate, UAdesign);
                            // Calc new BypassFraction based on the new OutletWaterTemp
                            if (std::abs(this->OutletWaterTemp - OWTLowerLimit) <= 0.01) {
                                BypassFraction2 = bypassFraction;
                                break;
                            } else if (this->OutletWaterTemp < OWTLowerLimit) {
                                // Set OutletWaterTemp = OWTLowerLimit, and use linear interpolation to calculate the bypassFraction
                                BypassFraction2 = BypassFractionPrev - (BypassFractionPrev - bypassFraction) * (OutletWaterTempPrev - OWTLowerLimit) /
                                                                           (OutletWaterTempPrev - this->OutletWaterTemp);
                                this->OutletWaterTemp = this->calculateSimpleTowerOutletTemp(
                                    state, WaterMassFlowRatePerCell * (1.0 - BypassFraction2), AirFlowRate, UAdesign);
                                if (this->OutletWaterTemp < OWTLowerLimit) {
                                    // Use previous iteration values
                                    BypassFraction2 = BypassFractionPrev;
                                    this->OutletWaterTemp = OutletWaterTempPrev;
                                }
                                break;
                            } else {
                                BypassFraction2 = (TempSetPoint - this->OutletWaterTemp) / (this->InletWaterTemp - this->OutletWaterTemp);
                            }

                            // Compare two BypassFraction to determine when to stop
                            if (std::abs(BypassFraction2 - bypassFraction) <= BypassFractionThreshold) {
                                break;
                            }
                            BypassFractionPrev = bypassFraction;
                            OutletWaterTempPrev = this->OutletWaterTemp;
                            bypassFraction = BypassFraction2;
                        }
                        if (NumIteration > MaxIteration) {
                            ShowWarningError(
                                state, format("Cooling tower fluid bypass iteration exceeds maximum limit of {} for {}", MaxItChar, this->Name));
                        }
                        this->BypassFraction = BypassFraction2;
                        // may not meet TempSetPoint due to limit of tower outlet temp to OWTLowerLimit
                        this->OutletWaterTemp = (1.0 - BypassFraction2) * this->OutletWaterTemp + BypassFraction2 * this->InletWaterTemp;
                    }
                }
            }
        }

        // output the fraction of the time step the fan is ON
        this->FanCyclingRatio = FanModeFrac;
        // Should this be water inlet node num?????
        Real64 const CpWater =
            this->plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp, RoutineName);

        this->Qactual = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);
        this->airFlowRateRatio = (AirFlowRate * this->NumCell) / this->HighSpeedAirFlowRate;
    }

    void CoolingTower::calculateTwoSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 1998
        //       MODIFIED       Feb. 2010, Chandan Sharma, FSEC, Added basin heater
        //                      Jul. 2010, A Flament, added multi-cell capability for the 3 types of cooling tower
        //                      Jun. 2016, R Zhang, Applied the condenser supply water temperature sensor fault model
        //                      Jul. 2016, R Zhang, Applied the cooling tower fouling fault model

        // PURPOSE OF THIS SUBROUTINE:
        // To simulate the operation of a cooling tower with a two-speed fan.

        // METHODOLOGY EMPLOYED:
        // The cooling tower is modeled using effectiveness-NTU relationships for
        // counterflow heat exchangers based on Merkel's theory.
        // The subroutine calculates the period of time required to meet a
        // leaving water temperature setpoint. It assumes that part-load
        // operation represents a linear interpolation of three steady-state regimes
        // (high-speed fan operation, low-speed fan operation and free convection regime).
        // Cyclic losses are neglected. The period of time required to meet the
        // leaving water temperature setpoint is used to determine the required
        // fan power and energy. Free convection regime is also modeled. This
        // occurs when the pump is operating and the fan is off. If free convection
        // regime cooling is all that is required for a given time step, the leaving
        // water temperature is allowed to fall below the leaving water temperature
        // setpoint (free cooling). At times when the cooling tower fan is required,
        // the leaving water temperature is at or above the setpoint.
        // A RunFlag is passed by the upper level manager to indicate the ON/OFF status,
        // or schedule, of the cooling tower. If the tower is OFF, outlet water
        // temperature and flow rate are passed through the model from inlet node to
        // outlet node without intervention (with the exception of free convection
        // where water temperature is allowed to float below the outlet water set
        // point). Reports are also updated with fan power and fan energy being zero.
        // When the RunFlag indicates an ON condition for the cooling tower, the
        // mass flow rate and water temperature are read from the inlet node of the
        // cooling tower (water-side). The outdoor air wet-bulb temperature is used
        // as the entering condition to the cooling tower (air-side). Input deck
        // parameters are read for the free convection regime (pump ON and fan OFF)
        // and a leaving water temperature is calculated. If the leaving water temperature
        // is at or below the setpoint, the calculated leaving water temperature is
        // placed on the outlet node and no fan power is used. If the calculated leaving
        // water temperature is above the setpoint, the cooling tower fan is turned on
        // and parameters for low fan speed are used to again calculate the leaving
        // water temperature. If the calculated leaving water temperature is
        // below the setpoint, a fan run-time fraction (FanModeFrac) is calculated and
        // used to determine fan power. The leaving water temperature setpoint is placed
        // on the outlet node. If the calculated leaving water temperature is at or above
        // the setpoint, the cooling tower fan is turned on 'high speed' and the routine is
        // repeated. If the calculated leaving water temperature is below the setpoint,
        // a fan run-time fraction is calculated for the second stage fan and fan power
        // is calculated as FanModeFrac*HighSpeedFanPower+(1-FanModeFrac)*LowSpeedFanPower.
        // If the calculated leaving water temperature is above the leaving water temp.
        // setpoint, the calculated leaving water temperature is placed on the outlet
        // node and the fan runs at full power (High Speed Fan Power). Water mass flow
        // rate is passed from inlet node to outlet node with no intervention.
        // If a tower has multiple cells, the specified inputs of or the autosized capacity
        //  and air/water flow rates are for the entire tower. The number of cells to operate
        //  is first determined based on the user entered minimal and maximal water flow fractions
        //  per cell. If the loads are not met, more cells (if available) will operate to meet
        //  the loads. Each cell operates in same way - same fan speed etc.
        // REFERENCES:
        // ASHRAE HVAC1KIT: A Toolkit for Primary HVAC System Energy Calculation. 1999.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("calculateTwoSpeedTower");

        // init
        this->Qactual = 0.0;
        this->FanPower = 0.0;
        this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;

        Real64 freeConvTowerUA = this->FreeConvTowerUA;
        Real64 highSpeedTowerUA = this->HighSpeedTowerUA;

        // water temperature setpoint
        Real64 TempSetPoint = 0.0;
        switch (this->plantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            if (this->SetpointIsOnOutlet) {
                TempSetPoint = state.dataLoopNodes->Node(this->WaterOutletNodeNum).TempSetPoint;
            } else {
                TempSetPoint = this->plantLoc.side->TempSetPoint;
            }
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            if (this->SetpointIsOnOutlet) {
                TempSetPoint = state.dataLoopNodes->Node(this->WaterOutletNodeNum).TempSetPointHi;
            } else {
                TempSetPoint = this->plantLoc.side->TempSetPointHi;
            }
        } break;
        default:
            break;
        }

        // If there is a fault of condenser SWT Sensor
        if (this->FaultyCondenserSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyCondenserSWTIndex;
            Real64 TowerOutletTemp_ff = TempSetPoint;

            // calculate the sensor offset using fault information
            this->FaultyCondenserSWTOffset = state.dataFaultsMgr->FaultsCondenserSWTSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint = TowerOutletTemp_ff - this->FaultyCondenserSWTOffset;
        }

        // If there is a fault of cooling tower fouling
        if (this->FaultyTowerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyTowerFoulingIndex;
            Real64 FreeConvTowerUA_ff = this->FreeConvTowerUA;
            Real64 HighSpeedTowerUA_ff = this->HighSpeedTowerUA;

            // calculate the Faulty Tower Fouling Factor using fault information
            this->FaultyTowerFoulingFactor = state.dataFaultsMgr->FaultsTowerFouling(FaultIndex).CalFaultyTowerFoulingFactor(state);

            // update the tower UA values at faulty cases
            freeConvTowerUA = FreeConvTowerUA_ff * this->FaultyTowerFoulingFactor;
            highSpeedTowerUA = HighSpeedTowerUA_ff * this->FaultyTowerFoulingFactor;
        }

        // Do not RETURN here if flow rate is less than SmallMassFlow. Check basin heater and then RETURN.
        if (this->plantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) {
            return; // TODO: WTF
        }
        bool returnFlagSet = false;
        this->checkMassFlowAndLoad(state, MyLoad, RunFlag, returnFlagSet);
        if (returnFlagSet) {
            return;
        }

        // Added for multi-cell. Determine the number of cells operating
        Real64 WaterMassFlowRatePerCellMin = 0.0;
        Real64 WaterMassFlowRatePerCellMax;
        int NumCellMin(0);
        int NumCellMax(0);
        Real64 WaterMassFlowRatePerCell;
        if (this->DesWaterMassFlowRate > 0.0) {
            WaterMassFlowRatePerCellMin = this->DesWaterMassFlowRate * this->MinFracFlowRate / this->NumCell;
            WaterMassFlowRatePerCellMax = this->DesWaterMassFlowRate * this->MaxFracFlowRate / this->NumCell;

            // round it up to the nearest integer
            NumCellMin = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMax) + 0.9999), this->NumCell);
            NumCellMax = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMin) + 0.9999), this->NumCell);
        }

        // cap min at 1
        if (NumCellMin <= 0) {
            NumCellMin = 1;
        }
        if (NumCellMax <= 0) {
            NumCellMax = 1;
        }

        if (this->cellCtrl == CellCtrl::MinCell) {
            this->NumCellOn = NumCellMin;
        } else {
            this->NumCellOn = NumCellMax;
        }

        WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;

        bool IncrNumCellFlag = true;

        Real64 AirFlowRate = 0.0;
        Real64 FanModeFrac = 0.0;
        int SpeedSel = 0;
        while (IncrNumCellFlag) {
            IncrNumCellFlag = false;

            // set local variable for tower
            Real64 UAdesign = freeConvTowerUA / this->NumCell; // where is NumCellOn?
            AirFlowRate = this->FreeConvAirFlowRate / this->NumCell;
            this->WaterMassFlowRate = state.dataLoopNodes->Node(this->WaterInletNodeNum).MassFlowRate;
            Real64 OutletWaterTemp1stStage = this->OutletWaterTemp;
            Real64 OutletWaterTemp2ndStage = this->OutletWaterTemp;
            FanModeFrac = 0.0;

            Real64 OutletWaterTempOFF = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRate, UAdesign);

            //     Setpoint was met using free convection regime (pump ON and fan OFF)
            this->FanPower = 0.0;
            this->OutletWaterTemp = OutletWaterTempOFF;

            if (OutletWaterTempOFF > TempSetPoint) {
                //     Setpoint was not met (or free conv. not used),turn on cooling tower 1st stage fan
                UAdesign = this->LowSpeedTowerUA / this->NumCell;
                AirFlowRate = this->LowSpeedAirFlowRate / this->NumCell;
                Real64 const FanPowerLow = this->LowSpeedFanPower * this->NumCellOn / this->NumCell;

                OutletWaterTemp1stStage = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRate, UAdesign);

                if (OutletWaterTemp1stStage <= TempSetPoint) {
                    //         Setpoint was met with pump ON and fan ON 1st stage, calculate fan mode fraction
                    FanModeFrac = (TempSetPoint - OutletWaterTempOFF) / (OutletWaterTemp1stStage - OutletWaterTempOFF);
                    this->FanPower = FanModeFrac * FanPowerLow;
                    this->OutletWaterTemp = TempSetPoint;
                    this->Qactual *= FanModeFrac;
                    SpeedSel = 1;
                } else {
                    //         Setpoint was not met, turn on cooling tower 2nd stage fan
                    UAdesign = highSpeedTowerUA / this->NumCell;
                    AirFlowRate = this->HighSpeedAirFlowRate / this->NumCell;
                    Real64 const FanPowerHigh = this->HighSpeedFanPower * this->NumCellOn / this->NumCell;

                    OutletWaterTemp2ndStage = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRate, UAdesign);

                    if ((OutletWaterTemp2ndStage <= TempSetPoint) && UAdesign > 0.0) {
                        //           Setpoint was met with pump ON and fan ON 2nd stage, calculate fan mode fraction
                        FanModeFrac = (TempSetPoint - OutletWaterTemp1stStage) / (OutletWaterTemp2ndStage - OutletWaterTemp1stStage);
                        this->FanPower = (FanModeFrac * FanPowerHigh) + (1.0 - FanModeFrac) * FanPowerLow;
                        this->OutletWaterTemp = TempSetPoint;
                        SpeedSel = 2;
                    } else {
                        //           Setpoint was not met, cooling tower ran at full capacity
                        this->OutletWaterTemp = OutletWaterTemp2ndStage;
                        this->FanPower = FanPowerHigh;
                        SpeedSel = 2;
                        FanModeFrac = 1.0;
                        // if possible increase the number of cells and do the calculations again with the new water mass flow rate per cell
                        if (this->NumCellOn < this->NumCell && (this->WaterMassFlowRate / (this->NumCellOn + 1)) >= WaterMassFlowRatePerCellMin) {
                            ++this->NumCellOn;
                            WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;
                            IncrNumCellFlag = true;
                        }
                    }
                }
            }
        }

        // output the fraction of the time step the fan is ON
        this->FanCyclingRatio = FanModeFrac;
        this->SpeedSelected = SpeedSel;

        Real64 const CpWater =
            this->plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp, RoutineName);
        this->Qactual = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);
        this->airFlowRateRatio = (AirFlowRate * this->NumCell) / this->HighSpeedAirFlowRate;
    }

    void CoolingTower::calculateVariableSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Feb 2005
        //       MODIFIED       Jul. 2010, A Flament, added multi-cell capability for the 3 types of cooling tower
        //                      Jul. 2010, B Griffith, general fluid props
        //                      Jun. 2016, R Zhang, Applied the condenser supply water temperature sensor fault model
        //                      Jul. 2016, R Zhang, Applied the cooling tower fouling fault model

        // PURPOSE OF THIS SUBROUTINE:
        // To simulate the operation of a variable-speed fan cooling tower.

        // METHODOLOGY EMPLOYED:
        // For each simulation time step, a desired range temperature (Twater,inlet-Twater,setpoint) and desired approach
        // temperature (Twater,setpoint-Tair,WB) is calculated which meets the outlet water temperature setpoint. This
        // desired range and approach temperature also provides a balance point for the empirical model where:
        // Tair,WB + Twater,range + Tapproach = Node(WaterInletNode)%Temp
        // Calculation of water outlet temperature uses one of the following equations:
        // Twater,outlet = Tair,WB + Tapproach          (1)  or
        // Twater,outlet = Twater,inlet - Twater,range  (2)
        // If a solution (or balance) is found, these 2 calculation methods are equal. Equation 2 is used to calculate
        // the outlet water temperature in the free convection regime and at the minimum or maximum fan speed so that
        // if a solution is not reached, the outlet water temperature is approximately equal to the inlet water temperature
        // and the tower fan must be varied to meet the setpoint. Equation 1 is used when the fan speed is varied between
        // the minimum and maximum fan speed to meet the outlet water temperature setpoint.
        // The outlet water temperature in the free convection regime is first calculated to see if the setpoint is met.
        // If the setpoint is met, the fan is OFF and the outlet water temperature is allowed to float below the set
        // point temperature. If the setpoint is not met, the outlet water temperature is re-calculated at the minimum
        // fan speed. If the setpoint is met, the fan is cycled to exactly meet the outlet water temperature setpoint.
        // If the setpoint is not met at the minimum fan speed, the outlet water temperature is re-calculated at the
        // maximum fan speed. If the setpoint at the maximum fan speed is not met, the fan runs at maximum speed the
        // entire time step. If the setpoint is met at the maximum fan speed, the fan speed is varied to meet the setpoint.
        // If a tower has multiple cells, the specified inputs of or the autosized capacity
        //  and air/water flow rates are for the entire tower. The number of cells to operate
        //  is first determined based on the user entered minimal and maximal water flow fractions
        //  per cell. If the loads are not met, more cells (if available) will operate to meet
        //  the loads. Inside each cell, the fan speed varies in the same way.
        // REFERENCES:
        // Benton, D.J., Bowmand, C.F., Hydeman, M., Miller, P.,
        // "An Improved Cooling Tower Algorithm for the CoolToolsTM Simulation Model".
        // ASHRAE Transactions 2002, V. 108, Pt. 1.
        // York International Corporation, "YORKcalcTM Software, Chiller-Plant Energy-Estimating Program",
        // Form 160.00-SG2 (0502). 2002.

        // SUBROUTINE PARAMETER DEFINITIONS:

        int constexpr MaxIte(500);    // Maximum number of iterations
        Real64 constexpr Acc(0.0001); // Accuracy of result
        static constexpr std::string_view RoutineName("calculateVariableSpeedTower");

        // Added for multi-cell. Determine the number of cells operating
        Real64 WaterMassFlowRatePerCellMin = 0.0;
        Real64 WaterMassFlowRatePerCellMax;
        int NumCellMin(0);
        int NumCellMax(0);
        Real64 WaterMassFlowRatePerCell;
        if (this->DesWaterMassFlowRate > 0.0) {
            WaterMassFlowRatePerCellMin = this->DesWaterMassFlowRate * this->MinFracFlowRate / this->NumCell;
            WaterMassFlowRatePerCellMax = this->DesWaterMassFlowRate * this->MaxFracFlowRate / this->NumCell;

            // round it up to the nearest integer
            NumCellMin = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMax) + 0.9999), this->NumCell);
            NumCellMax = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMin) + 0.9999), this->NumCell);
        }

        // cap min at 1
        if (NumCellMin <= 0) {
            NumCellMin = 1;
        }
        if (NumCellMax <= 0) {
            NumCellMax = 1;
        }

        if (this->cellCtrl == CellCtrl::MinCell) {
            this->NumCellOn = NumCellMin;
        } else {
            this->NumCellOn = NumCellMax;
        }

        WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;

        // Initialize subroutine variables
        this->Qactual = 0.0;
        this->FanPower = 0.0;
        this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;

        this->WaterUsage = 0.0;
        Real64 Twb = this->AirWetBulb;
        Real64 TwbCapped = this->AirWetBulb;

        // water temperature setpoint
        Real64 TempSetPoint(0.0); // Outlet water temperature setpoint (C)
        switch (this->plantLoc.loop->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            TempSetPoint = this->plantLoc.side->TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            TempSetPoint = this->plantLoc.side->TempSetPointHi;
        } break;
        default: {
            assert(false);
        } break;
        }

        // If there is a fault of condenser SWT Sensor
        if (this->FaultyCondenserSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyCondenserSWTIndex;
            Real64 TowerOutletTemp_ff = TempSetPoint;

            // calculate the sensor offset using fault information
            this->FaultyCondenserSWTOffset = state.dataFaultsMgr->FaultsCondenserSWTSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the TempSetPoint
            TempSetPoint = TowerOutletTemp_ff - this->FaultyCondenserSWTOffset;
        }

        Real64 Tr = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - TempSetPoint;
        Real64 Ta = TempSetPoint - this->AirWetBulb;

        // Do not RETURN here if flow rate is less than MassFlowTolerance. Check basin heater and then RETURN.
        if (this->plantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked) {
            return; // TODO: WTF
        }

        bool returnFlagSet = false;
        this->checkMassFlowAndLoad(state, MyLoad, RunFlag, returnFlagSet);
        if (returnFlagSet) {
            return;
        }

        // loop to increment NumCell if we cannot meet the setpoint with the actual number of cells calculated above
        bool IncrNumCellFlag = true;
        Real64 OutletWaterTempOFF;             // Outlet water temperature with fan OFF (C)
        Real64 OutletWaterTempON = 0.0;        // Outlet water temperature with fan ON at maximum fan speed (C)
        Real64 FreeConvectionCapFrac = 0.0;    // fraction of tower capacity in free convection
        Real64 WaterFlowRateRatioCapped = 0.0; // Water flow rate ratio passed to VS tower model
        Real64 TrCapped;                       // range temp passed to VS tower model
        Real64 TaCapped;                       // approach temp passed to VS tower model
        while (IncrNumCellFlag) {
            IncrNumCellFlag = false;
            // Initialize inlet node water properties
            Real64 const WaterDensity =
                this->plantLoc.loop->glycol->getDensity(state, state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp, RoutineName);
            Real64 const WaterFlowRateRatio = WaterMassFlowRatePerCell / (WaterDensity * this->CalibratedWaterFlowRate / this->NumCell);

            // check independent inputs with respect to model boundaries
            this->checkModelBounds(state, Twb, Tr, Ta, WaterFlowRateRatio, TwbCapped, TrCapped, TaCapped, WaterFlowRateRatioCapped);

            //   determine the free convection capacity by finding the outlet temperature at full air flow and multiplying
            //   the tower's full capacity temperature difference by the percentage of tower capacity in free convection
            //   regime specified by the user

            this->airFlowRateRatio = 1.0;
            OutletWaterTempOFF = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->OutletWaterTemp = OutletWaterTempOFF;
            FreeConvectionCapFrac = this->FreeConvectionCapacityFraction;
            OutletWaterTempON = this->calculateVariableTowerOutletTemp(state, WaterFlowRateRatioCapped, this->airFlowRateRatio, TwbCapped);

            if (OutletWaterTempON > TempSetPoint) {
                this->FanCyclingRatio = 1.0;
                this->airFlowRateRatio = 1.0;
                this->FanPower = this->HighSpeedFanPower * this->NumCellOn / this->NumCell;
                this->OutletWaterTemp = OutletWaterTempON;
                // if possible increase the number of cells and do the calculations again with the new water mass flow rate per cell
                if (this->NumCellOn < this->NumCell && (this->WaterMassFlowRate / (this->NumCellOn + 1)) > WaterMassFlowRatePerCellMin) {
                    ++this->NumCellOn;
                    WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;
                    IncrNumCellFlag = true;
                }
            }
        }

        // find the correct air ratio only if full flow is  too much
        if (OutletWaterTempON < TempSetPoint) {
            //   outlet water temperature is calculated in the free convection regime
            OutletWaterTempOFF = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp -
                                 FreeConvectionCapFrac * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - OutletWaterTempON);
            //   fan is OFF
            this->FanCyclingRatio = 0.0;
            //   air flow ratio is assumed to be the fraction of tower capacity in the free convection regime (fan is OFF but air is flowing)
            this->airFlowRateRatio = FreeConvectionCapFrac;

            // Assume setpoint was met using free convection regime (pump ON and fan OFF)
            this->FanPower = 0.0;
            this->OutletWaterTemp = OutletWaterTempOFF;

            if (OutletWaterTempOFF > TempSetPoint) {
                // Setpoint was not met, turn on cooling tower fan at minimum fan speed

                this->airFlowRateRatio = this->MinimumVSAirFlowFrac;
                Real64 OutletWaterTempMIN; // Outlet water temperature with fan at minimum speed (C)
                OutletWaterTempMIN = this->calculateVariableTowerOutletTemp(state, WaterFlowRateRatioCapped, this->airFlowRateRatio, TwbCapped);

                if (OutletWaterTempMIN < TempSetPoint) {
                    //         if setpoint was exceeded, cycle the fan at minimum air flow to meet the setpoint temperature
                    if (this->FanPowerfAirFlowCurve == 0) {
                        this->FanPower = pow_3(this->airFlowRateRatio) * this->HighSpeedFanPower * this->NumCellOn / this->NumCell;
                    } else {
                        Real64 const FanCurveValue = Curve::CurveValue(state, this->FanPowerfAirFlowCurve, this->airFlowRateRatio);
                        this->FanPower = max(0.0, (this->HighSpeedFanPower * FanCurveValue)) * this->NumCellOn / this->NumCell;
                    }
                    //       fan is cycling ON and OFF at the minimum fan speed. Adjust fan power and air flow rate ratio according to cycling rate
                    this->FanCyclingRatio = ((OutletWaterTempOFF - TempSetPoint) / (OutletWaterTempOFF - OutletWaterTempMIN));
                    this->FanPower *= this->FanCyclingRatio;
                    this->OutletWaterTemp = TempSetPoint;
                    this->airFlowRateRatio =
                        (this->FanCyclingRatio * this->MinimumVSAirFlowFrac) + ((1 - this->FanCyclingRatio) * FreeConvectionCapFrac);
                } else {
                    //       if setpoint was not met at minimum fan speed, set fan speed to maximum
                    this->airFlowRateRatio = 1.0;
                    //         fan will not cycle and runs the entire time step
                    this->FanCyclingRatio = 1.0;

                    this->OutletWaterTemp =
                        this->calculateVariableTowerOutletTemp(state, WaterFlowRateRatioCapped, this->airFlowRateRatio, TwbCapped);

                    // Setpoint was met with pump ON and fan ON at full flow
                    // Calculate the fraction of full air flow to exactly meet the setpoint temperature
                    auto f = [&state, this, WaterFlowRateRatioCapped, TwbCapped, Tr, Ta](Real64 FlowRatio) {
                        Real64 TapproachActual = this->calculateVariableSpeedApproach(state, WaterFlowRateRatioCapped, FlowRatio, TwbCapped, Tr);
                        return Ta - TapproachActual;
                    };
                    int SolFla = 0;
                    General::SolveRoot(state, Acc, MaxIte, SolFla, this->airFlowRateRatio, f, this->MinimumVSAirFlowFrac, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            ShowWarningError(
                                state,
                                format("Cooling tower iteration limit exceeded when calculating air flow rate ratio for tower {}", this->Name));
                        }
                        //           IF RegulaFalsi cannot find a solution then provide detailed output for debugging
                    } else if (SolFla == -2) {
                        if (!state.dataGlobal->WarmupFlag) {

                            if (this->CoolingTowerAFRRFailedCount < 1) {
                                ++this->CoolingTowerAFRRFailedCount;
                                ShowWarningError(
                                    state,
                                    format("CoolingTower:VariableSpeed \"{}\" - Cooling tower air flow rate ratio calculation failed ", this->Name));
                                ShowContinueError(state,
                                                  format("...with conditions as Twb = {:5.2F}, Trange = {:5.2F}, Tapproach = {:5.2F}, and water flow "
                                                         "rate ratio = {:5.2F}",
                                                         TwbCapped,
                                                         Tr,
                                                         Ta,
                                                         WaterFlowRateRatioCapped));
                                ShowContinueError(state, "...a solution could not be found within the valid range of air flow rate ratios");
                                ShowContinueErrorTimeStamp(
                                    state, format(" ...Valid air flow rate ratio range = {:5.2F} to 1.0.", this->MinimumVSAirFlowFrac));
                                ShowContinueError(state, "...Consider modifying the design approach or design range temperature for this tower.");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               "CoolingTower:VariableSpeed \"" + this->Name +
                                                                   "\" - Cooling tower air flow rate ratio calculation failed error continues.",
                                                               this->CoolingTowerAFRRFailedIndex);
                            }
                        }
                    }

                    //         Use theoretical cubic for determination of fan power if user has not specified a fan power ratio curve
                    if (this->FanPowerfAirFlowCurve == 0) {
                        this->FanPower = pow_3(this->airFlowRateRatio) * this->HighSpeedFanPower * this->NumCellOn / this->NumCell;
                    } else {
                        Real64 const FanCurveValue = Curve::CurveValue(state, this->FanPowerfAirFlowCurve, this->airFlowRateRatio);
                        this->FanPower = max(0.0, (this->HighSpeedFanPower * FanCurveValue)) * this->NumCellOn / this->NumCell;
                    }
                    //           outlet water temperature is calculated as the inlet air wet-bulb temperature plus tower approach temperature
                    this->OutletWaterTemp = Twb + Ta;
                }
            }
        }

        Real64 const CpWater =
            this->plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp, RoutineName);
        this->Qactual = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);

        //   calculate end time of current time step
        Real64 const CurrentEndTime = state.dataGlobal->CurrentTime + state.dataHVACGlobal->SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (CurrentEndTime > this->CurrentEndTimeLast && state.dataHVACGlobal->TimeStepSys >= this->TimeStepSysLast) {
            if (this->PrintLGMessage) {
                ++this->VSErrorCountFlowFrac;
                //       Show single warning and pass additional info to ShowRecurringWarningErrorAtEnd
                if (this->VSErrorCountFlowFrac < 2) {
                    ShowWarningError(state, this->LGBuffer1);
                    ShowContinueError(state, this->LGBuffer2);
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Liquid to gas ratio is out of range error continues...",
                                                          DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                          this->Name),
                                                   this->ErrIndexLG,
                                                   this->LGLast,
                                                   this->LGLast);
                }
            }
        }

        //   save last system time step and last end time of current time step (used to determine if warning is valid)
        this->TimeStepSysLast = state.dataHVACGlobal->TimeStepSys;
        this->CurrentEndTimeLast = CurrentEndTime;

        //   warn user on first occurrence if flow fraction is greater than maximum for the YorkCalc model, use recurring warning stats
        if (this->TowerModelType == ModelType::YorkCalcModel || this->TowerModelType == ModelType::YorkCalcUserDefined) {
            this->PrintLGMessage = false;
            //      Do not report error message in free convection regime
            if (this->airFlowRateRatio > this->MinimumVSAirFlowFrac) {
                Real64 const FlowFraction = WaterFlowRateRatioCapped / this->airFlowRateRatio;
                //        Flow fractions greater than a MaxLiquidToGasRatio of 8 are not reliable using the YorkCalc model
                if (FlowFraction > this->MaxLiquidToGasRatio) {
                    //          Report warnings only during actual simulation
                    if (!state.dataGlobal->WarmupFlag) {
                        this->PrintLGMessage = true;
                        this->LGBuffer1 = format("{} \"{}\" - Liquid to gas ratio (L/G) is out of range at {:5.2F}.",
                                                 DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                 this->Name,
                                                 FlowFraction);
                        this->LGBuffer2 = format(" ...Valid maximum ratio = {:5.2F}. Occurrence info = {}, {} {}",
                                                 this->MaxLiquidToGasRatio,
                                                 state.dataEnvrn->EnvironmentName,
                                                 state.dataEnvrn->CurMnDy,
                                                 General::CreateSysTimeIntervalString(state));

                        this->LGLast = FlowFraction;
                    }
                }
            }
        }
    }

    void CoolingTower::calculateMerkelVariableSpeedTower(EnergyPlusData &state, Real64 &MyLoad, bool RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.Griffith
        //       DATE WRITTEN   August 2013
        //       MODIFIED       Jun. 2016, R Zhang, Applied the condenser supply water temperature sensor fault model
        //                      Jul. 2016, R Zhang, Applied the cooling tower fouling fault model

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate variable speed tower model using Merkel's theory with UA adjustments developed by Scheier

        // METHODOLOGY EMPLOYED:
        // Find a fan speed that operates the tower to meet MyLoad

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr DesignWetBulb(25.56); // tower outdoor air entering wetbulb for design [C]
        int constexpr MaxIte(500);             // Maximum number of iterations for solver
        Real64 constexpr Acc(1.e-3);           // Accuracy of solver result
        static constexpr std::string_view RoutineName("calculateMerkelVariableSpeedTower");

        Real64 const CpWater =
            this->plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp, RoutineName);
        this->Qactual = 0.0;
        this->FanPower = 0.0;
        this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;

        Real64 freeConvTowerUA = this->FreeConvTowerUA;
        Real64 highSpeedTowerUA = this->HighSpeedTowerUA;

        // If there is a fault of condenser SWT Sensor
        if (this->FaultyCondenserSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyCondenserSWTIndex;
            // calculate the sensor offset using fault information
            this->FaultyCondenserSWTOffset = state.dataFaultsMgr->FaultsCondenserSWTSensor(FaultIndex).CalFaultOffsetAct(state);
        }

        // If there is a fault of cooling tower fouling
        if (this->FaultyTowerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = this->FaultyTowerFoulingIndex;
            Real64 FreeConvTowerUA_ff = this->FreeConvTowerUA;
            Real64 HighSpeedTowerUA_ff = this->HighSpeedTowerUA;

            // calculate the Faulty Tower Fouling Factor using fault information
            this->FaultyTowerFoulingFactor = state.dataFaultsMgr->FaultsTowerFouling(FaultIndex).CalFaultyTowerFoulingFactor(state);

            // update the tower UA values at faulty cases
            freeConvTowerUA = FreeConvTowerUA_ff * this->FaultyTowerFoulingFactor;
            highSpeedTowerUA = HighSpeedTowerUA_ff * this->FaultyTowerFoulingFactor;
        }

        Real64 WaterMassFlowRatePerCellMin = 0.0;
        Real64 WaterMassFlowRatePerCellMax;

        // Added for multi-cell. Determine the number of cells operating
        int NumCellMin = 0;
        int NumCellMax = 0;
        Real64 WaterMassFlowRatePerCell;
        Real64 UAdesignPerCell;
        Real64 AirFlowRatePerCell;
        if (this->DesWaterMassFlowRate > 0.0) {
            WaterMassFlowRatePerCellMin = this->DesWaterMassFlowRate * this->MinFracFlowRate / this->NumCell;
            WaterMassFlowRatePerCellMax = this->DesWaterMassFlowRate * this->MaxFracFlowRate / this->NumCell;

            // round it up to the nearest integer
            NumCellMin = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMax) + 0.9999), this->NumCell);
            NumCellMax = min(int((this->WaterMassFlowRate / WaterMassFlowRatePerCellMin) + 0.9999), this->NumCell);
        }

        // cap min at 1
        if (NumCellMin <= 0) {
            NumCellMin = 1;
        }
        if (NumCellMax <= 0) {
            NumCellMax = 1;
        }

        if (this->cellCtrl == CellCtrl::MinCell) {
            this->NumCellOn = NumCellMin;
        } else {
            this->NumCellOn = NumCellMax;
        }

        WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;

        if ((std::abs(MyLoad) <= HVAC::SmallLoad) || !RunFlag) {
            // tower doesn't need to do anything
            this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->FanPower = 0.0;
            this->airFlowRateRatio = 0.0;
            this->Qactual = 0.0;
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
            return;
        } else if (this->WaterMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance || (MyLoad > HVAC::SmallLoad)) {
            // for multiple cells, we assume that it's a common basin
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
            return;
        }

        // first find free convection cooling rate
        UAdesignPerCell = freeConvTowerUA / this->NumCell;
        AirFlowRatePerCell = this->FreeConvAirFlowRate / this->NumCell;
        this->WaterMassFlowRate = state.dataLoopNodes->Node(this->WaterInletNodeNum).MassFlowRate;
        Real64 OutletWaterTempOFF = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRatePerCell, UAdesignPerCell);

        Real64 FreeConvQdot = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - OutletWaterTempOFF);
        this->FanPower = 0.0;

        if (std::abs(MyLoad) <= FreeConvQdot) { // can meet load with free convection and fan off

            this->OutletWaterTemp = OutletWaterTempOFF;
            this->airFlowRateRatio = 0.0;
            this->Qactual = FreeConvQdot;
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);

            return;
        }

        // next find full fan speed cooling rate
        UAdesignPerCell = highSpeedTowerUA / this->NumCell;
        AirFlowRatePerCell = this->HighSpeedAirFlowRate / this->NumCell;
        this->airFlowRateRatio = 1.0;
        Real64 WaterFlowRateRatio = WaterMassFlowRatePerCell / this->DesWaterMassFlowRatePerCell;
        Real64 UAwetbulbAdjFac = Curve::CurveValue(state, this->UAModFuncWetBulbDiffCurvePtr, (DesignWetBulb - this->AirWetBulb));
        Real64 UAairflowAdjFac = Curve::CurveValue(state, this->UAModFuncAirFlowRatioCurvePtr, this->airFlowRateRatio);
        Real64 UAwaterflowAdjFac = Curve::CurveValue(state, this->UAModFuncWaterFlowRatioCurvePtr, WaterFlowRateRatio);
        Real64 UAadjustedPerCell = UAdesignPerCell * UAwetbulbAdjFac * UAairflowAdjFac * UAwaterflowAdjFac;
        this->OutletWaterTemp = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRatePerCell, UAadjustedPerCell);
        Real64 FullSpeedFanQdot =
            this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);
        Real64 FanPowerAdjustFac = 0.0;
        if (FullSpeedFanQdot <= std::abs(MyLoad)) { // full speed is what we want.

            if ((FullSpeedFanQdot + HVAC::SmallLoad) < std::abs(MyLoad) && (this->NumCellOn < this->NumCell) &&
                ((this->WaterMassFlowRate / (this->NumCellOn + 1)) >= WaterMassFlowRatePerCellMin)) {
                // If full fan and not meeting setpoint, then increase number of cells until all are used or load is satisfied
                bool IncrNumCellFlag = true; // set value to true to enter in the loop
                while (IncrNumCellFlag) {
                    ++this->NumCellOn;
                    WaterMassFlowRatePerCell = this->WaterMassFlowRate / this->NumCellOn;
                    WaterFlowRateRatio = WaterMassFlowRatePerCell / this->DesWaterMassFlowRatePerCell;
                    UAwaterflowAdjFac = Curve::CurveValue(state, this->UAModFuncWaterFlowRatioCurvePtr, WaterFlowRateRatio);
                    UAadjustedPerCell = UAdesignPerCell * UAwetbulbAdjFac * UAairflowAdjFac * UAwaterflowAdjFac;
                    this->OutletWaterTemp =
                        this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRatePerCell, UAadjustedPerCell);
                    IncrNumCellFlag = (FullSpeedFanQdot + HVAC::SmallLoad) < std::abs(MyLoad) && (this->NumCellOn < this->NumCell) &&
                                      ((this->WaterMassFlowRate / (this->NumCellOn + 1)) >= WaterMassFlowRatePerCellMin);
                }
                FullSpeedFanQdot =
                    this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);
            }
            this->Qactual = FullSpeedFanQdot;
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
            // now calculate fan power
            FanPowerAdjustFac = Curve::CurveValue(state, this->FanPowerfAirFlowCurve, this->airFlowRateRatio);
            this->FanPower = this->HighSpeedFanPower * FanPowerAdjustFac * this->NumCellOn / this->NumCell;

            return;
        }

        // next find minimum air flow ratio cooling rate
        this->airFlowRateRatio = this->MinimumVSAirFlowFrac;
        AirFlowRatePerCell = this->airFlowRateRatio * this->HighSpeedAirFlowRate / this->NumCell;
        UAairflowAdjFac = Curve::CurveValue(state, this->UAModFuncAirFlowRatioCurvePtr, this->airFlowRateRatio);
        UAadjustedPerCell = UAdesignPerCell * UAwetbulbAdjFac * UAairflowAdjFac * UAwaterflowAdjFac;
        this->OutletWaterTemp = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRatePerCell, UAadjustedPerCell);
        Real64 MinSpeedFanQdot =
            this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);

        if (std::abs(MyLoad) <= MinSpeedFanQdot) { // min fan speed already exceeds load)
            this->Qactual = MinSpeedFanQdot;
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
            // now calculate fan power
            FanPowerAdjustFac = Curve::CurveValue(state, this->FanPowerfAirFlowCurve, this->airFlowRateRatio);
            this->FanPower = this->HighSpeedFanPower * FanPowerAdjustFac * this->NumCellOn / this->NumCell;
            return;
        }

        if ((MinSpeedFanQdot < std::abs(MyLoad)) && (std::abs(MyLoad) < FullSpeedFanQdot)) {
            // load can be refined by modulating fan speed, call regula-falsi
            auto f = [&state, this, MyLoad, WaterMassFlowRatePerCell, UAdesignPerCell, UAwetbulbAdjFac, UAwaterflowAdjFac, CpWater](
                         Real64 airFlowRateRatioLocal) {
                Real64 const AirFlowRatePerCell = airFlowRateRatioLocal * this->HighSpeedAirFlowRate / this->NumCell;
                Real64 const UAairflowAdjFac = Curve::CurveValue(state, this->UAModFuncAirFlowRatioCurvePtr, airFlowRateRatioLocal);
                Real64 const UAadjustedPerCell = UAdesignPerCell * UAwetbulbAdjFac * UAairflowAdjFac * UAwaterflowAdjFac;
                Real64 OutletWaterTempTrial =
                    this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRatePerCell, UAadjustedPerCell);
                Real64 const Qdot =
                    this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - OutletWaterTempTrial);
                return std::abs(MyLoad) - Qdot;
            };
            int SolFla = 0;
            General::SolveRoot(state, Acc, MaxIte, SolFla, this->airFlowRateRatio, f, this->MinimumVSAirFlowFrac, 1.0);

            if (SolFla == -1) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->VSMerkelAFRErrorIter < 1) {
                        ++this->VSMerkelAFRErrorIter;
                        ShowWarningError(state,
                                         format("{} - Iteration limit exceeded calculating variable speed fan ratio for unit = {}",
                                                cCoolingTower_VariableSpeedMerkel,
                                                this->Name));
                        ShowContinueError(state,
                                          format("Estimated air flow ratio  = {:.4R}",
                                                 (std::abs(MyLoad) - MinSpeedFanQdot) / (FullSpeedFanQdot - MinSpeedFanQdot)));
                        ShowContinueError(state, format("Calculated air flow ratio = {:.4R}", this->airFlowRateRatio));
                        ShowContinueErrorTimeStamp(state,
                                                   "The calculated air flow ratio will be used and the simulation continues. Occurrence info:");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        cCoolingTower_VariableSpeedMerkel + " \"" + this->Name +
                            "\" - Iteration limit exceeded calculating air flow ratio error continues. air flow ratio statistics follow.",
                        this->VSMerkelAFRErrorIterIndex,
                        this->airFlowRateRatio,
                        this->airFlowRateRatio);
                }
            } else if (SolFla == -2) {
                this->airFlowRateRatio = (std::abs(MyLoad) - MinSpeedFanQdot) / (FullSpeedFanQdot - MinSpeedFanQdot);
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->VSMerkelAFRErrorFail < 1) {
                        ++this->VSMerkelAFRErrorFail;
                        ShowWarningError(state,
                                         format("{} - solver failed calculating variable speed fan ratio for unit = {}",
                                                cCoolingTower_VariableSpeedMerkel,
                                                this->Name));
                        ShowContinueError(state, format("Estimated air flow ratio  = {:.4R}", this->airFlowRateRatio));
                        ShowContinueErrorTimeStamp(state, "The estimated air flow ratio will be used and the simulation continues. Occurrence info:");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        cCoolingTower_VariableSpeedMerkel + " \"" + this->Name +
                            "\" - solver failed calculating air flow ratio error continues. air flow ratio statistics follow.",
                        this->VSMerkelAFRErrorFailIndex,
                        this->airFlowRateRatio,
                        this->airFlowRateRatio);
                }
            }

            // now rerun to get performance with AirFlowRateRatio
            AirFlowRatePerCell = this->airFlowRateRatio * this->HighSpeedAirFlowRate / this->NumCell;

            UAairflowAdjFac = Curve::CurveValue(state, this->UAModFuncAirFlowRatioCurvePtr, this->airFlowRateRatio);
            UAadjustedPerCell = UAdesignPerCell * UAwetbulbAdjFac * UAairflowAdjFac * UAwaterflowAdjFac;

            this->OutletWaterTemp = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRatePerCell, UAadjustedPerCell);
            this->Qactual = this->WaterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - this->OutletWaterTemp);
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);

            // now calculate fan power
            FanPowerAdjustFac = Curve::CurveValue(state, this->FanPowerfAirFlowCurve, this->airFlowRateRatio);
            this->FanPower = this->HighSpeedFanPower * FanPowerAdjustFac * this->NumCellOn / this->NumCell;
        }
    }

    Real64 CoolingTower::calculateSimpleTowerOutletTemp(EnergyPlusData &state,
                                                        Real64 const waterMassFlowRate,
                                                        Real64 const AirFlowRate,
                                                        Real64 const UAdesign)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 1998
        //       RE-ENGINEERED  Shirey, Raustad, Jan 2001

        // PURPOSE OF THIS SUBROUTINE:
        // See purpose for Single Speed or Two Speed tower model

        // METHODOLOGY EMPLOYED:
        // See methodology for Single Speed or Two Speed tower model

        // REFERENCES:
        // Merkel, F. 1925.  Verduftungskuhlung. VDI Forschungsarbeiten, Nr 275, Berlin.
        // ASHRAE     1999.  HVAC1KIT: A Toolkit for Primary HVAC System Energy Calculations.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("calculateSimpleTowerOutletTemp");

        // initialize some local variables
        Real64 QactualLocal = 0.0; // Actual heat transfer rate between tower water and air [W]

        // set local tower inlet and outlet temperature variables
        this->InletWaterTemp = this->WaterTemp;
        Real64 OutletWaterTempLocal = this->InletWaterTemp;
        Real64 InletAirTemp = this->AirTemp;       // Dry-bulb temperature of air entering the tower [C]
        Real64 InletAirWetBulb = this->AirWetBulb; // Wetbulb temp of entering moist air [C]

        if (UAdesign == 0.0) {
            return OutletWaterTempLocal;
        }

        // set water and air properties
        Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, this->AirPress, InletAirTemp, this->AirHumRat); // Density of air [kg/m3]
        Real64 AirMassFlowRate = AirFlowRate * AirDensity;                                                           // Mass flow rate of air [kg/s]
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->AirHumRat);                                                 // Heat capacity of air [J/kg/K]
        Real64 CpWater = this->plantLoc.loop->glycol->getSpecificHeat(state, this->WaterTemp, RoutineName); // Heat capacity of water [J/kg/K]
        Real64 InletAirEnthalpy =
            Psychrometrics::PsyHFnTdbRhPb(state, this->AirWetBulb, 1.0, this->AirPress); // Enthalpy of entering moist air [J/kg]

        // initialize exiting wet bulb temperature before iterating on final solution
        Real64 OutletAirWetBulb = InletAirWetBulb + 6.0; // Wetbulb temp of exiting moist air [C]

        // Calculate mass flow rates
        if (waterMassFlowRate <= 0.0) {
            OutletWaterTempLocal = this->InletWaterTemp;
            return OutletWaterTempLocal;
        }

        Real64 MdotCpWater = waterMassFlowRate * CpWater; // Water mass flow rate times the heat capacity [W/K]

        int Iter = 0;
        Real64 OutletAirEnthalpy;                   // Enthalpy of exiting moist air [J/kg]
        Real64 WetBulbError = 1.0;                  // Calculated error for exiting wet-bulb temperature between iterations [delta K/K]
        Real64 DeltaTwb = 1.0;                      // Absolute value of difference between inlet and outlet air wet-bulb temp [C]
        Real64 OutletAirWetBulbLast;                // temporary Wetbulb temp of exiting moist air [C]
        int constexpr IterMax(50);                  // Maximum number of iterations allowed
        Real64 constexpr WetBulbTolerance(0.00001); // Maximum error for exiting wet-bulb temperature between iterations [delta K/K]
        Real64 constexpr DeltaTwbTolerance(0.001);  // Maximum error (tolerance) in DeltaTwb for iteration convergence [C]
        while ((WetBulbError > WetBulbTolerance) && (Iter <= IterMax) && (DeltaTwb > DeltaTwbTolerance)) {
            ++Iter;
            //        OutletAirEnthalpy = PsyHFnTdbRhPb(OutletAirWetBulb,1.0,OutBaroPress)
            OutletAirEnthalpy = Psychrometrics::PsyHFnTdbRhPb(state, OutletAirWetBulb, 1.0, this->AirPress);
            // calculate the airside specific heat and capacity
            Real64 const CpAirside =
                (OutletAirEnthalpy - InletAirEnthalpy) /
                (OutletAirWetBulb - InletAirWetBulb);               // Delta enthalpy of the tower air divides by delta air wet-bulb temp [J/kg/K]
            Real64 const AirCapacity = AirMassFlowRate * CpAirside; // MdotCp of air through the tower
            // calculate the minimum to maximum capacity ratios of airside and waterside
            Real64 const CapacityRatioMin = min(AirCapacity, MdotCpWater);    // Minimum capacity of airside and waterside
            Real64 const CapacityRatioMax = max(AirCapacity, MdotCpWater);    // Maximum capacity of airside and waterside
            Real64 const CapacityRatio = CapacityRatioMin / CapacityRatioMax; // Ratio of minimum to maximum capacity
            // Calculate heat transfer coefficient and number of transfer units (NTU)
            Real64 const UAactual = UAdesign * CpAirside / CpAir;        // UA value at actual conditions [W/C]
            Real64 const NumTransferUnits = UAactual / CapacityRatioMin; // Number of transfer Units [NTU]
            // calculate heat exchanger effectiveness
            Real64 effectiveness; // Effectiveness of the heat exchanger [-]
            if (CapacityRatio <= 0.995) {
                Real64 Exponent = NumTransferUnits * (1.0 - CapacityRatio);
                if (Exponent >= 700.0) {
                    effectiveness = NumTransferUnits / (1.0 + NumTransferUnits);
                } else {
                    effectiveness = (1.0 - std::exp(-1.0 * NumTransferUnits * (1.0 - CapacityRatio))) /
                                    (1.0 - CapacityRatio * std::exp(-1.0 * NumTransferUnits * (1.0 - CapacityRatio)));
                }
            } else {
                effectiveness = NumTransferUnits / (1.0 + NumTransferUnits);
            }
            // calculate water to air heat transfer and store last exiting WB temp of air
            QactualLocal = effectiveness * CapacityRatioMin * (this->InletWaterTemp - InletAirWetBulb);
            OutletAirWetBulbLast = OutletAirWetBulb;
            // calculate new exiting wet bulb temperature of airstream
            OutletAirWetBulb = InletAirWetBulb + QactualLocal / AirCapacity;
            // Check error tolerance and exit if satisfied
            DeltaTwb = std::abs(OutletAirWetBulb - InletAirWetBulb);
            // Add KelvinConv to denominator below convert OutletAirWetBulbLast to Kelvin to avoid divide by zero.
            // Wet bulb error units are delta K/K
            WetBulbError = std::abs((OutletAirWetBulb - OutletAirWetBulbLast) / (OutletAirWetBulbLast + Constant::Kelvin));
        }

        if (QactualLocal >= 0.0) {
            OutletWaterTempLocal = this->InletWaterTemp - QactualLocal / MdotCpWater;
        } else {
            OutletWaterTempLocal = this->InletWaterTemp;
        }
        return OutletWaterTempLocal;
    }

    Real64 CoolingTower::calculateVariableTowerOutletTemp(EnergyPlusData &state,
                                                          Real64 const WaterFlowRateRatio,    // current water flow rate ratio (capped if applicable)
                                                          Real64 const airFlowRateRatioLocal, // current air flow rate ratio
                                                          Real64 const Twb // current inlet air wet-bulb temperature (C, capped if applicable)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   Feb. 2005

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the leaving water temperature of the variable speed cooling tower.

        // METHODOLOGY EMPLOYED:
        // The range temperature is varied to determine balance point where model output (Tapproach),
        // range temperature and inlet air wet-bulb temperature show a balance as:
        // Twb + Tapproach + Trange = Node(WaterInletNode)%Temp

        // REFERENCES:
        // Benton, D.J., Bowmand, C.F., Hydeman, M., Miller, P.,
        // "An Improved Cooling Tower Algorithm for the CoolToolsTM Simulation Model".
        // ASHRAE Transactions 2002, V. 108, Pt. 1.
        // York International Corporation, "YORKcalcTM Software, Chiller-Plant Energy-Estimating Program",
        // Form 160.00-SG2 (0502). 2002.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500);    // Maximum number of iterations
        Real64 constexpr Acc(0.0001); // Accuracy of result

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 constexpr VSTowerMaxRangeTemp(22.2222); // set VS cooling tower range maximum value used for solver

        // determine tower outlet water temperature
        Real64 Tr; // range temperature which results in an energy balance
        auto f = [&state, this, WaterFlowRateRatio, airFlowRateRatioLocal, Twb](Real64 Trange) {
            // call model to determine approach temperature given other independent variables (range temp is being varied to find balance)
            Real64 Tapproach = this->calculateVariableSpeedApproach(state, WaterFlowRateRatio, airFlowRateRatioLocal, Twb, Trange);
            // calculate residual based on a balance where Twb + Ta + Tr = Inlet Water Temp
            return (Twb + Tapproach + Trange) - this->WaterTemp;
        };
        int SolFla = 0;
        General::SolveRoot(state, Acc, MaxIte, SolFla, Tr, f, 0.001, max(this->MaxRangeTemp, VSTowerMaxRangeTemp));

        // calculate outlet temperature
        Real64 outletWaterTempLocal = this->WaterTemp - min(Tr, this->MaxRangeTemp);

        if (SolFla == -1) {
            ShowSevereError(state, "Iteration limit exceeded in calculating tower nominal capacity at minimum air flow ratio");
            ShowContinueError(
                state,
                "Design inlet air wet-bulb or approach temperature must be modified to achieve an acceptable range at the minimum air flow rate");
            ShowContinueError(state, format("Cooling tower simulation failed to converge for tower {}", this->Name));
        } else if (SolFla == -2) {
            // bad starting value means that solution corresponds to a range that is beyond
            // the bounds of the model; The maximum range is used here
            outletWaterTempLocal = this->WaterTemp - this->MaxRangeTemp;
            ++this->VSErrorCountTRCalc;
            if (this->VSErrorCountTRCalc < 2) {
                ShowWarningError(
                    state,
                    format("The range for the cooling tower {} likely exceeds the bounds of the model. The maximum range of the model is used {}.",
                           this->Name,
                           this->MaxRangeTemp));
                ShowContinueError(state,
                                  format(" ... Occurrence info = {}, {} {}",
                                         state.dataEnvrn->EnvironmentName,
                                         state.dataEnvrn->CurMnDy,
                                         General::CreateSysTimeIntervalString(state)));
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("The range for the cooling tower {} likely exceeds the bounds of the model. The maximum range of the model is used {}",
                           this->Name,
                           this->MaxRangeTemp),
                    this->ErrIndexTRCalc);
            }
        }
        return outletWaterTempLocal;
    }

    Real64 CoolingTower::calculateVariableSpeedApproach(EnergyPlusData &state,
                                                        Real64 const PctWaterFlow,      // Water flow ratio of cooling tower
                                                        Real64 const airFlowRatioLocal, // Air flow ratio of cooling tower
                                                        Real64 const Twb,               // Inlet air wet-bulb temperature [C]
                                                        Real64 const Tr // Cooling tower range (outlet water temp minus inlet air wet-bulb temp) [C]
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   Feb. 2005

        // PURPOSE OF THIS FUNCTION:
        // Calculate tower approach temperature (e.g. outlet water temp minus inlet air wet-bulb temp)
        // given air flow ratio, water flow ratio, inlet air wet-bulb temp, and tower range.

        // METHODOLOGY EMPLOYED:
        // Calculation method used empirical models from CoolTools or York to determine performance
        // of variable speed (variable air flow rate) cooling towers.

        // REFERENCES:
        // Benton, D.J., Bowmand, C.F., Hydeman, M., Miller, P.,
        // "An Improved Cooling Tower Algorithm for the CoolToolsTM Simulation Model".
        // ASHRAE Transactions 2002, V. 108, Pt. 1.
        // York International Corporation, "YORKcalcTM Software, Chiller-Plant Energy-Estimating Program",
        // Form 160.00-SG2 (0502). 2002.

        if (this->TowerModelType == ModelType::YorkCalcModel || this->TowerModelType == ModelType::YorkCalcUserDefined) {
            Real64 PctAirFlow = airFlowRatioLocal;
            Real64 FlowFactor = PctWaterFlow / PctAirFlow;
            return this->Coeff[0] + this->Coeff[1] * Twb + this->Coeff[2] * Twb * Twb + this->Coeff[3] * Tr + this->Coeff[4] * Twb * Tr +
                   this->Coeff[5] * Twb * Twb * Tr + this->Coeff[6] * Tr * Tr + this->Coeff[7] * Twb * Tr * Tr +
                   this->Coeff[8] * Twb * Twb * Tr * Tr + this->Coeff[9] * FlowFactor + this->Coeff[10] * Twb * FlowFactor +
                   this->Coeff[11] * Twb * Twb * FlowFactor + this->Coeff[12] * Tr * FlowFactor + this->Coeff[13] * Twb * Tr * FlowFactor +
                   this->Coeff[14] * Twb * Twb * Tr * FlowFactor + this->Coeff[15] * Tr * Tr * FlowFactor +
                   this->Coeff[16] * Twb * Tr * Tr * FlowFactor + this->Coeff[17] * Twb * Twb * Tr * Tr * FlowFactor +
                   this->Coeff[18] * FlowFactor * FlowFactor + this->Coeff[19] * Twb * FlowFactor * FlowFactor +
                   this->Coeff[20] * Twb * Twb * FlowFactor * FlowFactor + this->Coeff[21] * Tr * FlowFactor * FlowFactor +
                   this->Coeff[22] * Twb * Tr * FlowFactor * FlowFactor + this->Coeff[23] * Twb * Twb * Tr * FlowFactor * FlowFactor +
                   this->Coeff[24] * Tr * Tr * FlowFactor * FlowFactor + this->Coeff[25] * Twb * Tr * Tr * FlowFactor * FlowFactor +
                   this->Coeff[26] * Twb * Twb * Tr * Tr * FlowFactor * FlowFactor;

        } else { // empirical model is CoolTools format
            //     the CoolTools model actually uses PctFanPower = AirFlowRatio^3 as an input to the model
            Real64 PctAirFlow = pow_3(airFlowRatioLocal);
            return this->Coeff[0] + this->Coeff[1] * PctAirFlow + this->Coeff[2] * PctAirFlow * PctAirFlow +
                   this->Coeff[3] * PctAirFlow * PctAirFlow * PctAirFlow + this->Coeff[4] * PctWaterFlow +
                   this->Coeff[5] * PctAirFlow * PctWaterFlow + this->Coeff[6] * PctAirFlow * PctAirFlow * PctWaterFlow +
                   this->Coeff[7] * PctWaterFlow * PctWaterFlow + this->Coeff[8] * PctAirFlow * PctWaterFlow * PctWaterFlow +
                   this->Coeff[9] * PctWaterFlow * PctWaterFlow * PctWaterFlow + this->Coeff[10] * Twb + this->Coeff[11] * PctAirFlow * Twb +
                   this->Coeff[12] * PctAirFlow * PctAirFlow * Twb + this->Coeff[13] * PctWaterFlow * Twb +
                   this->Coeff[14] * PctAirFlow * PctWaterFlow * Twb + this->Coeff[15] * PctWaterFlow * PctWaterFlow * Twb +
                   this->Coeff[16] * Twb * Twb + this->Coeff[17] * PctAirFlow * Twb * Twb + this->Coeff[18] * PctWaterFlow * Twb * Twb +
                   this->Coeff[19] * Twb * Twb * Twb + this->Coeff[20] * Tr + this->Coeff[21] * PctAirFlow * Tr +
                   this->Coeff[22] * PctAirFlow * PctAirFlow * Tr + this->Coeff[23] * PctWaterFlow * Tr +
                   this->Coeff[24] * PctAirFlow * PctWaterFlow * Tr + this->Coeff[25] * PctWaterFlow * PctWaterFlow * Tr +
                   this->Coeff[26] * Twb * Tr + this->Coeff[27] * PctAirFlow * Twb * Tr + this->Coeff[28] * PctWaterFlow * Twb * Tr +
                   this->Coeff[29] * Twb * Twb * Tr + this->Coeff[30] * Tr * Tr + this->Coeff[31] * PctAirFlow * Tr * Tr +
                   this->Coeff[32] * PctWaterFlow * Tr * Tr + this->Coeff[33] * Twb * Tr * Tr + this->Coeff[34] * Tr * Tr * Tr;
        }
    }

    void CoolingTower::checkModelBounds(EnergyPlusData &state,
                                        Real64 Twb,                      // current inlet air wet-bulb temperature (C)
                                        Real64 Tr,                       // requested range temperature for current time step (C)
                                        Real64 Ta,                       // requested approach temperature for current time step (C)
                                        Real64 WaterFlowRateRatio,       // current water flow rate ratio at water inlet node
                                        Real64 &TwbCapped,               // bounded value of inlet air wet-bulb temperature (C)
                                        Real64 &TrCapped,                // bounded value of range temperature (C)
                                        Real64 &TaCapped,                // bounded value of approach temperature (C)
                                        Real64 &WaterFlowRateRatioCapped // bounded value of water flow rate ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Feb 2005

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables are within the limits used during the
        // development of the empirical model.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a variable speed cooling tower are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // The range of each independent variable is provided either by the CoolTools or York model limits, or
        // specified by the user if the model is User Defined (in either the CoolTools or York model format).
        // These limits are tested in this subroutine each time step and returned for use by the calling routine.
        // The independent variables capped here may or may not be passed to the empirical model in the calling
        // routine depending on their use.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string OutputChar;   // character string for warning messages
        std::string OutputCharLo; // character string for warning messages
        std::string OutputCharHi; // character string for warning messages
        std::string TrimValue;    // character string for warning messages
        // current end time is compared with last to see if time step changed

        //   initialize capped variables in case independent variables are in bounds
        TwbCapped = Twb;
        TrCapped = Tr;
        TaCapped = Ta;
        WaterFlowRateRatioCapped = WaterFlowRateRatio;

        //   calculate end time of current time step
        Real64 CurrentEndTime = state.dataGlobal->CurrentTime + state.dataHVACGlobal->SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (CurrentEndTime > this->CurrentEndTimeLast && state.dataHVACGlobal->TimeStepSys >= this->TimeStepSysLast) {
            if (this->PrintTrMessage) {
                ++this->VSErrorCountTR;
                if (this->VSErrorCountTR < 2) {
                    ShowWarningError(state, this->TrBuffer1);
                    ShowContinueError(state, this->TrBuffer2);
                    ShowContinueError(state, this->TrBuffer3);
                    ShowContinueError(state, " ...Range temperatures outside model boundaries may not adversely affect tower performance.");
                    ShowContinueError(state, " ...This is not an unexpected occurrence when simulating actual conditions.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Tower range temperature is out of range error continues...",
                                                          DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                          this->Name),
                                                   this->ErrIndexTR,
                                                   this->TrLast,
                                                   this->TrLast);
                }
            }
            if (this->PrintTwbMessage) {
                ++this->VSErrorCountIAWB;
                if (this->VSErrorCountIAWB < 6) {
                    ShowWarningError(state, this->TwbBuffer1);
                    ShowContinueError(state, this->TwbBuffer2);
                    ShowContinueError(state, this->TwbBuffer3);
                    ShowContinueError(state, " ...Wet-bulb temperatures outside model boundaries may not adversely affect tower performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Inlet air wet-bulb temperature is out of range error continues...",
                                                          DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                          this->Name),
                                                   this->ErrIndexIAWB,
                                                   this->TwbLast,
                                                   this->TwbLast);
                }
            }
            if (this->PrintTaMessage) {
                ++this->VSErrorCountTA;
                if (this->VSErrorCountTA < 2) {
                    ShowWarningError(state, this->TaBuffer1);
                    ShowContinueError(state, this->TaBuffer2);
                    ShowContinueError(state, this->TaBuffer3);
                    ShowContinueError(state, " ...Approach temperatures outside model boundaries may not adversely affect tower performance.");
                    ShowContinueError(state, " ...This is not an unexpected occurrence when simulating actual conditions.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Tower approach temperature is out of range error continues...",
                                                          DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                          this->Name),
                                                   this->ErrIndexTA,
                                                   this->TaLast,
                                                   this->TaLast);
                }
            }
            if (this->PrintWFRRMessage) {
                ++this->VSErrorCountWFRR;
                if (this->VSErrorCountWFRR < 6) {
                    ShowWarningError(state, this->WFRRBuffer1);
                    ShowContinueError(state, this->WFRRBuffer2);
                    ShowContinueError(state, this->WFRRBuffer3);
                    ShowContinueError(state, " ...Water flow rate ratios outside model boundaries may not adversely affect tower performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Water flow rate ratio is out of range error continues...",
                                                          DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                          this->Name),
                                                   this->ErrIndexWFRR,
                                                   this->WaterFlowRateRatioLast,
                                                   this->WaterFlowRateRatioLast);
                }
            }
        }

        //   save last system time step and last end time of current time step (used to determine if warning is valid)
        this->TimeStepSysLast = state.dataHVACGlobal->TimeStepSys;
        this->CurrentEndTimeLast = CurrentEndTime;

        //   check boundaries of independent variables and post warnings to individual buffers to print at end of time step
        if (Twb < this->MinInletAirWBTemp || Twb > this->MaxInletAirWBTemp) {
            OutputChar = format("{:.2R}", Twb);
            OutputCharLo = format("{:.2R}", this->MinInletAirWBTemp);
            OutputCharHi = format("{:.2R}", this->MaxInletAirWBTemp);
            if (Twb < this->MinInletAirWBTemp) {
                TwbCapped = this->MinInletAirWBTemp;
            }
            if (Twb > this->MaxInletAirWBTemp) {
                TwbCapped = this->MaxInletAirWBTemp;
            }
            if (!state.dataGlobal->WarmupFlag) {
                this->PrintTwbMessage = true;
                this->TwbBuffer1 = format("{} \"{}\" - Inlet air wet-bulb temperature is outside model boundaries at {}.",
                                          DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                          this->Name,
                                          OutputChar);
                this->TwbBuffer2 = " ...Valid range = " + OutputCharLo + " to " + OutputCharHi +
                                   ". Occurrence info = " + state.dataEnvrn->EnvironmentName + ", " + state.dataEnvrn->CurMnDy + ' ' +
                                   General::CreateSysTimeIntervalString(state);
                TrimValue = format("{:.6R}", TwbCapped);
                this->TwbBuffer3 = " ...Inlet air wet-bulb temperature passed to the model = " + TrimValue;
                this->TwbLast = Twb;
            } else {
                this->PrintTwbMessage = false;
            }
        } else {
            this->PrintTwbMessage = false;
        }

        if (Tr < this->MinRangeTemp || Tr > this->MaxRangeTemp) {
            OutputChar = format("{:.2R}", Tr);
            OutputCharLo = format("{:.2R}", this->MinRangeTemp);
            OutputCharHi = format("{:.2R}", this->MaxRangeTemp);
            if (Tr < this->MinRangeTemp) {
                TrCapped = this->MinRangeTemp;
            }
            if (Tr > this->MaxRangeTemp) {
                TrCapped = this->MaxRangeTemp;
            }
            if (!state.dataGlobal->WarmupFlag) {
                this->PrintTrMessage = true;
                this->TrBuffer1 = format("{} \"{}\" - Tower range temperature is outside model boundaries at {}.",
                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                         this->Name,
                                         OutputChar);
                this->TrBuffer2 = " ...Valid range = " + OutputCharLo + " to " + OutputCharHi +
                                  ". Occurrence info = " + state.dataEnvrn->EnvironmentName + ", " + state.dataEnvrn->CurMnDy + ' ' +
                                  General::CreateSysTimeIntervalString(state);
                TrimValue = format("{:.5R}", Tr);
                this->TrBuffer3 = " ...Tower range temperature passed to the model = " + TrimValue;
                this->TrLast = Tr;
            } else {
                this->PrintTrMessage = false;
            }
        } else {
            this->PrintTrMessage = false;
        }

        if (Ta < this->MinApproachTemp || Ta > this->MaxApproachTemp) {
            OutputChar = format("{:.2R}", Ta);
            OutputCharLo = format("{:.2R}", this->MinApproachTemp);
            OutputCharHi = format("{:.2R}", this->MaxApproachTemp);
            if (Ta < this->MinApproachTemp) {
                TaCapped = this->MinApproachTemp;
            }
            if (Ta > this->MaxApproachTemp) {
                TaCapped = this->MaxApproachTemp;
            }
            if (!state.dataGlobal->WarmupFlag) {
                this->PrintTaMessage = true;
                this->TaBuffer1 = format("{} \"{}\" - Tower approach temperature is outside model boundaries at {}.",
                                         DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                         this->Name,
                                         OutputChar);
                this->TaBuffer2 = " ...Valid range = " + OutputCharLo + " to " + OutputCharHi +
                                  ". Occurrence info = " + state.dataEnvrn->EnvironmentName + ", " + state.dataEnvrn->CurMnDy + ' ' +
                                  General::CreateSysTimeIntervalString(state);
                TrimValue = format("{:.5R}", Ta);
                this->TaBuffer3 = " ...Tower approach temperature passed to the model = " + TrimValue;
                this->TaLast = Ta;
            } else {
                this->PrintTaMessage = false;
            }
        } else {
            this->PrintTaMessage = false;
        }

        if (this->TowerModelType == ModelType::YorkCalcModel || this->TowerModelType == ModelType::YorkCalcUserDefined) {
            //     Water flow rate ratio warning not valid for YorkCalc model, print liquid to gas ratio
            //     warning instead (bottom of Subroutine VariableSpeedTower)
            this->PrintWFRRMessage = false;
        } else {
            if (WaterFlowRateRatio < this->MinWaterFlowRatio || WaterFlowRateRatio > this->MaxWaterFlowRatio) {
                OutputChar = format("{:.2R}", WaterFlowRateRatio);
                OutputCharLo = format("{:.2R}", this->MinWaterFlowRatio);
                OutputCharHi = format("{:.2R}", this->MaxWaterFlowRatio);
                if (WaterFlowRateRatio < this->MinWaterFlowRatio) {
                    WaterFlowRateRatioCapped = this->MinWaterFlowRatio;
                }
                if (WaterFlowRateRatio > this->MaxWaterFlowRatio) {
                    WaterFlowRateRatioCapped = this->MaxWaterFlowRatio;
                }
                if (!state.dataGlobal->WarmupFlag) {
                    this->PrintWFRRMessage = true;
                    this->WFRRBuffer1 = format("{} \"{}\" - Water flow rate ratio is outside model boundaries at {}.",
                                               DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                               this->Name,
                                               OutputChar);
                    this->WFRRBuffer2 = " ...Valid range = " + OutputCharLo + " to " + OutputCharHi +
                                        ". Occurrence info = " + state.dataEnvrn->EnvironmentName + ", " + state.dataEnvrn->CurMnDy + ' ' +
                                        General::CreateSysTimeIntervalString(state);
                    TrimValue = format("{:.5R}", WaterFlowRateRatioCapped);
                    this->WFRRBuffer3 = " ...Water flow rate ratio passed to the model = " + TrimValue;
                    this->WaterFlowRateRatioLast = WaterFlowRateRatio;
                } else {
                    this->PrintWFRRMessage = false;
                }
            } else {
                this->PrintWFRRMessage = false;
            }
        }
    }

    void CoolingTower::calculateWaterUsage(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   August 2006
        //       MODIFIED       T Hong, Aug. 2008. Added fluid bypass for single speed cooling tower
        //                      A Flament, July 2010. Added multi-cell capability

        // PURPOSE OF THIS SUBROUTINE:
        // Collect tower water usage calculations for reuse by all the tower models.

        // REFERENCES:
        // Code for this routine started from VariableSpeedTower

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("calculateWaterUsage");

        Real64 EvapVdot = 0.0;
        Real64 AverageWaterTemp = (this->InletWaterTemp + this->OutletWaterTemp) / 2.0;

        // Set water and air properties
        if (this->EvapLossMode == EvapLoss::MoistTheory) {

            Real64 const AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, this->AirPress, this->AirTemp, this->AirHumRat);
            Real64 const AirMassFlowRate = this->airFlowRateRatio * this->HighSpeedAirFlowRate * AirDensity * this->NumCellOn / this->NumCell;
            Real64 const InletAirEnthalpy = Psychrometrics::PsyHFnTdbRhPb(state, this->AirWetBulb, 1.0, this->AirPress);

            if (AirMassFlowRate > 0.0) {
                // Calculate outlet air conditions for determining water usage

                Real64 const OutletAirEnthalpy = InletAirEnthalpy + this->Qactual / AirMassFlowRate;
                Real64 const OutletAirTSat = Psychrometrics::PsyTsatFnHPb(state, OutletAirEnthalpy, this->AirPress);
                Real64 const OutletAirHumRatSat = Psychrometrics::PsyWFnTdbH(state, OutletAirTSat, OutletAirEnthalpy);

                // calculate specific humidity ratios (HUMRAT to mass of moist air not dry air)
                Real64 const InSpecificHumRat = this->AirHumRat / (1 + this->AirHumRat);
                Real64 const OutSpecificHumRat = OutletAirHumRatSat / (1 + OutletAirHumRatSat);

                // calculate average air temp for density call
                Real64 const TairAvg = (this->AirTemp + OutletAirTSat) / 2.0;

                // Amount of water evaporated, get density water at air temp or 4 C if too cold
                Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, max(TairAvg, 4.0), RoutineName);

                EvapVdot = (AirMassFlowRate * (OutSpecificHumRat - InSpecificHumRat)) / rho; // [m3/s]
                if (EvapVdot < 0.0) {
                    EvapVdot = 0.0;
                }
            } else {
                EvapVdot = 0.0;
            }

        } else if (this->EvapLossMode == EvapLoss::UserFactor) {
            Real64 const rho = this->plantLoc.loop->glycol->getDensity(state, AverageWaterTemp, RoutineName);

            EvapVdot = this->UserEvapLossFactor * (this->InletWaterTemp - this->OutletWaterTemp) * (this->WaterMassFlowRate / rho);
            if (EvapVdot < 0.0) {
                EvapVdot = 0.0;
            }
        } else {
            // should never come here
        }

        //   amount of water lost due to drift
        Real64 driftVdot = this->DesignWaterFlowRate * this->NumCellOn / this->NumCell * this->DriftLossFraction * this->airFlowRateRatio;

        Real64 BlowDownVdot(0.0);
        if (this->BlowdownMode == Blowdown::Schedule) {
            // Amount of water lost due to blow down (purging contaminants from tower basin)
            BlowDownVdot = (this->blowdownSched != nullptr) ? this->blowdownSched->getCurrentVal() : 0.0;
        } else if (this->BlowdownMode == Blowdown::Concentration) {
            if (this->ConcentrationRatio > 2.0) { // protect divide by zero
                BlowDownVdot = EvapVdot / (this->ConcentrationRatio - 1) - driftVdot;
            } else {
                BlowDownVdot = EvapVdot - driftVdot;
            }
            if (BlowDownVdot < 0.0) {
                BlowDownVdot = 0.0;
            }
        } else {
            // should never come here
        }

        // Added for fluid bypass
        if (this->CapacityControl == CapacityCtrl::FluidBypass) {
            if (this->EvapLossMode == EvapLoss::UserFactor) {
                EvapVdot *= (1 - this->BypassFraction);
            }
            driftVdot *= (1 - this->BypassFraction);
            BlowDownVdot *= (1 - this->BypassFraction);
        }

        Real64 const makeUpVdot = EvapVdot + driftVdot + BlowDownVdot;

        // set demand request in Water Storage if needed
        Real64 StarvedVdot = 0.0;
        Real64 tankSupplyVdot = 0.0;
        if (this->SuppliedByWaterSystem) {

            // set demand request
            state.dataWaterData->WaterStorage(this->WaterTankID).VdotRequestDemand(this->WaterTankDemandARRID) = makeUpVdot;

            Real64 const AvailTankVdot = state.dataWaterData->WaterStorage(this->WaterTankID)
                                             .VdotAvailDemand(this->WaterTankDemandARRID); // check what tank can currently provide

            tankSupplyVdot = makeUpVdot;      // init
            if (AvailTankVdot < makeUpVdot) { // calculate starved flow
                StarvedVdot = makeUpVdot - AvailTankVdot;
                tankSupplyVdot = AvailTankVdot;
            }
        } else { // supplied by mains
        }

        //   total water usage
        // update report variables
        this->EvaporationVdot = EvapVdot;
        this->EvaporationVol = EvapVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->DriftVdot = driftVdot;
        this->DriftVol = driftVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->BlowdownVdot = BlowDownVdot;
        this->BlowdownVol = BlowDownVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->MakeUpVdot = makeUpVdot;
        this->MakeUpVol = makeUpVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->TankSupplyVdot = tankSupplyVdot;
        this->TankSupplyVol = tankSupplyVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->StarvedMakeUpVdot = StarvedVdot;
        this->StarvedMakeUpVol = StarvedVdot * (state.dataHVACGlobal->TimeStepSysSec);
        this->coolingTowerApproach = this->OutletWaterTemp - this->AirWetBulb;
        this->coolingTowerRange = this->InletWaterTemp - this->OutletWaterTemp;
    }

    void CoolingTower::update(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    October 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for passing results to the outlet water node.

        // set node information
        PlantUtilities::SafeCopyPlantNode(state, this->WaterInletNodeNum, this->WaterOutletNodeNum);
        state.dataLoopNodes->Node(this->WaterOutletNodeNum).Temp = this->OutletWaterTemp;

        if (this->plantLoc.side->FlowLock == DataPlant::FlowLock::Unlocked || state.dataGlobal->WarmupFlag) {
            return;
        }

        // Check flow rate through tower and compare to design flow rate, show warning if greater than Design * Multiplier
        if (state.dataLoopNodes->Node(this->WaterOutletNodeNum).MassFlowRate > this->DesWaterMassFlowRate * this->TowerMassFlowRateMultiplier) {
            ++this->HighMassFlowErrorCount;
            if (this->HighMassFlowErrorCount < 2) {
                ShowWarningError(state, format("{} \"{}\"", DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], this->Name));
                ShowContinueError(state, " Condenser Loop Mass Flow Rate is much greater than the towers design mass flow rate.");
                ShowContinueError(
                    state, format(" Condenser Loop Mass Flow Rate = {:.6T}", state.dataLoopNodes->Node(this->WaterOutletNodeNum).MassFlowRate));
                ShowContinueError(state, format(" Tower Design Mass Flow Rate   = {:.6T}", this->DesWaterMassFlowRate));
                ShowContinueErrorTimeStamp(state, "");
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\"  Condenser Loop Mass Flow Rate is much greater than the towers design mass flow rate error continues...",
                           DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                           this->Name),
                    this->HighMassFlowErrorIndex,
                    state.dataLoopNodes->Node(this->WaterOutletNodeNum).MassFlowRate,
                    state.dataLoopNodes->Node(this->WaterOutletNodeNum).MassFlowRate);
            }
        }

        // Check if OutletWaterTemp is below the minimum condenser loop temp and warn user
        Real64 const LoopMinTemp = this->plantLoc.loop->MinTemp;
        bool const outletWaterTempTooLow = this->OutletWaterTemp < LoopMinTemp;
        bool const flowIsOn = this->WaterMassFlowRate > 0.0;
        if (outletWaterTempTooLow && flowIsOn) {
            ++this->OutletWaterTempErrorCount;
            if (this->OutletWaterTempErrorCount < 2) {
                ShowWarningError(state, format("{} \"{}\"", DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], this->Name));
                ShowContinueError(
                    state,
                    format("Cooling tower water outlet temperature ({:.2F} C) is below the specified minimum condenser loop temp of {:.2F} C",
                           this->OutletWaterTemp,
                           LoopMinTemp));
                ShowContinueErrorTimeStamp(state, "");
            } else {
                ShowRecurringWarningErrorAtEnd(
                    state,
                    format("{} \"{}\" Cooling tower water outlet temperature is below the specified minimum condenser loop temp error continues...",
                           DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                           this->Name),
                    this->OutletWaterTempErrorIndex,
                    this->OutletWaterTemp,
                    this->OutletWaterTemp);
            }
        }

        // Check if water mass flow rate is small (e.g. no flow) and warn user
        if (this->WaterMassFlowRate > 0.0 && this->WaterMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance) {
            ++this->SmallWaterMassFlowErrorCount;
            if (this->SmallWaterMassFlowErrorCount < 2) {
                ShowWarningError(state, format("{} \"{}\"", DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)], this->Name));
                ShowContinueError(state, "Cooling tower water mass flow rate near zero.");
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Actual Mass flow = {:.2T}", this->WaterMassFlowRate));
            } else {
                ShowRecurringWarningErrorAtEnd(state,
                                               format("{} \"{}\"  Cooling tower water mass flow rate near zero error continues...",
                                                      DataPlant::PlantEquipTypeNames[static_cast<int>(this->TowerType)],
                                                      this->Name),
                                               this->SmallWaterMassFlowErrorIndex,
                                               this->WaterMassFlowRate,
                                               this->WaterMassFlowRate);
            }
        }
    }

    void CoolingTower::report(EnergyPlusData &state, bool const RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    October 1998

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variables for the tower.

        Real64 const ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        if (!RunFlag) {
            this->InletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->Qactual = 0.0;
            this->FanPower = 0.0;
            this->FanEnergy = 0.0;
            this->AirFlowRatio = 0.0;
            this->WaterAmountUsed = 0.0;
            this->BasinHeaterConsumption = this->BasinHeaterPower * ReportingConstant;
            this->FanCyclingRatio = 0.0;
            this->BypassFraction = 0.0; // added for fluid bypass
            this->NumCellOn = 0;
            this->SpeedSelected = 0;
        } else {
            this->InletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->FanEnergy = this->FanPower * ReportingConstant;
            this->AirFlowRatio = this->airFlowRateRatio;
            this->WaterAmountUsed = this->WaterUsage * ReportingConstant;
            this->BasinHeaterConsumption = this->BasinHeaterPower * ReportingConstant;
        }
    }

    void CoolingTower::checkMassFlowAndLoad(EnergyPlusData &state, Real64 const MyLoad, bool RunFlag, bool &returnFlagSet)
    {
        if ((MyLoad > -HVAC::SmallLoad) || !RunFlag) {
            // tower doesn't need to do anything
            this->OutletWaterTemp = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp;
            this->FanPower = 0.0;
            this->airFlowRateRatio = 0.0;
            this->Qactual = 0.0;
            this->WaterMassFlowRate = 0.0;
            PlantUtilities::SetComponentFlowRate(state, this->WaterMassFlowRate, this->WaterInletNodeNum, this->WaterOutletNodeNum, this->plantLoc);
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
            returnFlagSet = true;
        } else if (this->WaterMassFlowRate <= DataBranchAirLoopPlant::MassFlowTolerance) {
            // for multiple cells, we assume that it's a common basin
            CalcBasinHeaterPower(
                state, this->BasinHeaterPowerFTempDiff, this->basinHeaterSched, this->BasinHeaterSetPointTemp, this->BasinHeaterPower);
            returnFlagSet = true;
        }
    }

    Real64 CoolingTower::getDynamicMaxCapacity(EnergyPlusData &state)
    {
        // TODO: does not include faults object impact
        static constexpr std::string_view routineName("getDynamicMaxCapacity");
        Real64 outletWaterTemp = 0.0;
        Real64 constexpr designWetBulb = 25.56;
        Real64 constexpr airFlowRateRatio = 1.0;
        Real64 constexpr waterFlowRateRatio = 1.0;
        Real64 const CpWater =
            this->plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp, routineName);
        // use operating max (i.e., at current plant flow rates, could be 0 if plant is off) or max available capacity?
        // Real64 waterMassFlowRate = state.dataLoopNodes->Node(this->WaterInletNodeNum).MassFlowRateMaxAvail;
        Real64 waterMassFlowRate = this->DesWaterMassFlowRate;
        this->AirWetBulb = (this->OutdoorAirInletNodeNum > 0) ? state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).OutAirWetBulb
                                                              : state.dataEnvrn->OutWetBulbTemp;

        switch (this->TowerType) {
        case DataPlant::PlantEquipmentType::CoolingTower_SingleSpd:
        case DataPlant::PlantEquipmentType::CoolingTower_TwoSpd: {
            // only needed for calculateSimpleTowerOutletTemp
            this->AirTemp =
                (this->OutdoorAirInletNodeNum > 0) ? state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).Temp : state.dataEnvrn->OutDryBulbTemp;
            Real64 WaterMassFlowRatePerCell = waterMassFlowRate / this->NumCell;
            Real64 UAdesign = this->HighSpeedTowerUA / this->NumCell; // could save these calculations in e.g., this->DesUAPerCell
            Real64 AirFlowRate = this->HighSpeedAirFlowRate / this->NumCell;
            outletWaterTemp = this->calculateSimpleTowerOutletTemp(state, WaterMassFlowRatePerCell, AirFlowRate, UAdesign);
        } break;
        case DataPlant::PlantEquipmentType::CoolingTower_VarSpd: {
            Real64 TrCapped; // range temp passed to VS tower model
            Real64 TaCapped; // approach temp passed to VS tower model
            Real64 Twb = this->AirWetBulb;
            Real64 TwbCapped = this->AirWetBulb;
            // water temperature setpoint
            Real64 TempSetPoint(0.0); // Outlet water temperature setpoint (C)
            switch (this->plantLoc.loop->LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                TempSetPoint = this->plantLoc.side->TempSetPoint;
            } break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                TempSetPoint = this->plantLoc.side->TempSetPointHi;
            } break;
            default: {
                assert(false);
            } break;
            }
            Real64 Tr = state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - TempSetPoint;
            Real64 Ta = TempSetPoint - this->AirWetBulb;
            Real64 waterFlowRateRatioCapped = 0.0; // Water flow rate ratio passed to VS tower model
            // check independent inputs with respect to model boundaries
            this->checkModelBounds(state, Twb, Tr, Ta, waterFlowRateRatio, TwbCapped, TrCapped, TaCapped, waterFlowRateRatioCapped);
            outletWaterTemp = this->calculateVariableTowerOutletTemp(state, waterFlowRateRatioCapped, airFlowRateRatio, TwbCapped);
        } break;
        case DataPlant::PlantEquipmentType::CoolingTower_VarSpdMerkel: {
            // only needed for calculateSimpleTowerOutletTemp
            this->AirTemp =
                (this->OutdoorAirInletNodeNum > 0) ? state.dataLoopNodes->Node(this->OutdoorAirInletNodeNum).Temp : state.dataEnvrn->OutDryBulbTemp;
            Real64 const UAdesignPerCell = this->HighSpeedTowerUA / this->NumCell;
            Real64 const airFlowRatePerCell = this->HighSpeedAirFlowRate / this->NumCell;
            Real64 const waterMassFlowRatePerCell = waterMassFlowRate / this->NumCell;
            Real64 const WaterFlowRateRatio = waterMassFlowRatePerCell / this->DesWaterMassFlowRatePerCell; // this should always be 1 ?
            Real64 const UAwetbulbAdjFac = Curve::CurveValue(state, this->UAModFuncWetBulbDiffCurvePtr, (designWetBulb - this->AirWetBulb));
            Real64 const UAairflowAdjFac = Curve::CurveValue(state, this->UAModFuncAirFlowRatioCurvePtr, airFlowRateRatio);
            Real64 const UAwaterflowAdjFac = Curve::CurveValue(state, this->UAModFuncWaterFlowRatioCurvePtr, WaterFlowRateRatio);
            Real64 const UAadjustedPerCell = UAdesignPerCell * UAwetbulbAdjFac * UAairflowAdjFac * UAwaterflowAdjFac;
            outletWaterTemp = this->calculateSimpleTowerOutletTemp(state, waterMassFlowRatePerCell, airFlowRatePerCell, UAadjustedPerCell);
        } break;
        default:
            assert(false);
        }
        return waterMassFlowRate * CpWater * (state.dataLoopNodes->Node(this->WaterInletNodeNum).Temp - outletWaterTemp);
    }

} // namespace CondenserLoopTowers

} // namespace EnergyPlus
