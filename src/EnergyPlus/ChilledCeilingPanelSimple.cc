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
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChilledCeilingPanelSimple.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::CoolingPanelSimple {

// Module -- (ref: Object: ZoneHVAC:CoolingPanel:RadiantConvective:Water)

// Module containing the routines dealing with the simple (chilled ceiling) cooling panels

// MODULE INFORMATION:
//       AUTHOR         Rick Strand
//       DATE WRITTEN   Aug 2014

// PURPOSE OF THIS MODULE:
// The purpose of this module is to simulate simple chilled ceiling panels.  It is similar to
// hot water radiant/convective baseboard units and the code for this model used that model as
// a starting point.

// REFERENCES:
// Existing code for hot water baseboard models (radiant-convective variety)

// USE STATEMENTS:
// MODULE PARAMETER DEFINITIONS
std::string const cCMO_CoolingPanel_Simple("ZoneHVAC:CoolingPanel:RadiantConvective:Water");

void SimCoolingPanel(
    EnergyPlusData &state, std::string const &EquipName, int const ControlledZoneNum, bool const FirstHVACIteration, Real64 &PowerMet, int &CompIndex)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Aug 2014

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the simple cooling (chilled ceiling) panel.  It borrows heavily
    // from the hot water radiant-convective baseboard model code.

    // REFERENCES:
    // Existing code for hot water baseboard models (radiant-convective variety)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CoolingPanelNum; // Index of unit in baseboard array
    Real64 QZnReq;       // Zone load not yet satisfied
    Real64 MaxWaterFlow;
    Real64 MinWaterFlow;

    if (state.dataChilledCeilingPanelSimple->GetInputFlag) {
        GetCoolingPanelInput(state);
        state.dataChilledCeilingPanelSimple->GetInputFlag = false;
    }

    // Find the correct Baseboard Equipment
    if (CompIndex == 0) {
        CoolingPanelNum = Util::FindItemInList(EquipName,
                                               state.dataChilledCeilingPanelSimple->CoolingPanel,
                                               &CoolingPanelParams::Name,
                                               (int)state.dataChilledCeilingPanelSimple->CoolingPanel.size());
        if (CoolingPanelNum == 0) {
            ShowFatalError(state, format("SimCoolingPanelSimple: Unit not found={}", EquipName));
        }
        CompIndex = CoolingPanelNum;
    } else {
        CoolingPanelNum = CompIndex;
        if (CoolingPanelNum > (int)state.dataChilledCeilingPanelSimple->CoolingPanel.size() || CoolingPanelNum < 1) {
            ShowFatalError(state,
                           format("SimCoolingPanelSimple:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                  CoolingPanelNum,
                                  (int)state.dataChilledCeilingPanelSimple->CoolingPanel.size(),
                                  EquipName));
        }
        if (state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).CheckEquipName) {
            if (EquipName != state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).Name) {
                ShowFatalError(state,
                               format("SimCoolingPanelSimple: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      CoolingPanelNum,
                                      EquipName,
                                      state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).Name));
            }
            state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).CheckEquipName = false;
        }
    }

    if (CompIndex > 0) {

        auto &thisCP(state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum));

        InitCoolingPanel(state, CoolingPanelNum, ControlledZoneNum, FirstHVACIteration);

        QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP;

        // On the first HVAC iteration the system values are given to the controller, but after that
        // the demand limits are in place and there needs to be feedback to the Zone Equipment
        if (FirstHVACIteration) {
            MaxWaterFlow = thisCP.WaterMassFlowRateMax;
            MinWaterFlow = 0.0;
        } else {
            MaxWaterFlow = state.dataLoopNodes->Node(thisCP.WaterInletNode).MassFlowRateMaxAvail;
            MinWaterFlow = state.dataLoopNodes->Node(thisCP.WaterInletNode).MassFlowRateMinAvail;
        }

        switch (thisCP.EquipType) {
        case DataPlant::PlantEquipmentType::CoolingPanel_Simple: { // 'ZoneHVAC:CoolingPanel:RadiantConvective:Water'
            thisCP.CalcCoolingPanel(state, CoolingPanelNum);
        } break;
        default: {
            ShowSevereError(
                state,
                format("SimCoolingPanelSimple: Errors in CoolingPanel={}", state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).Name));
            ShowContinueError(
                state,
                format("Invalid or unimplemented equipment type={}", state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).EquipType));
            ShowFatalError(state, "Preceding condition causes termination.");
        } break;
        }

        PowerMet = thisCP.TotPower;

        UpdateCoolingPanel(state, CoolingPanelNum);

        state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).ReportCoolingPanel(state);

    } else {
        ShowFatalError(state, format("SimCoolingPanelSimple: Unit not found={}", EquipName));
    }
}

void GetCoolingPanelInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Aug 2014

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the input for the simple cooling panel units.

    // METHODOLOGY EMPLOYED:
    // Standard input processor calls--started from Daeho's radiant-convective water baseboard model.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetCoolingPanelInput:");
    static constexpr std::string_view routineName = "GetCoolingPanelInput";
    Real64 constexpr MaxFraction(1.0);
    Real64 constexpr MinFraction(0.0);
    Real64 constexpr MaxWaterTempAvg(30.0);       // Maximum limit of average water temperature in degree C
    Real64 constexpr MinWaterTempAvg(0.0);        // Minimum limit of average water temperature in degree C
    Real64 constexpr MaxWaterFlowRate(10.0);      // Maximum limit of water volume flow rate in m3/s
    Real64 constexpr MinWaterFlowRate(0.00001);   // Minimum limit of water volume flow rate in m3/s
    Real64 constexpr WaterMassFlowDefault(0.063); // Default water mass flow rate in kg/s
    int constexpr MinDistribSurfaces(1);          // Minimum number of surfaces that a baseboard heater can radiate to
    Real64 constexpr MinThrottlingRange(0.5);     // Smallest throttling range allowed in degrees Celsius
    static constexpr std::string_view MeanAirTemperature("MeanAirTemperature");
    static constexpr std::string_view MeanRadiantTemperature("MeanRadiantTemperature");
    static constexpr std::string_view OperativeTemperature("OperativeTemperature");
    static constexpr std::string_view OutsideAirDryBulbTemperature("OutdoorDryBulbTemperature");
    static constexpr std::string_view OutsideAirWetBulbTemperature("OutdoorWetBulbTemperature");
    static constexpr std::string_view ZoneTotalLoad("ZoneTotalLoad");
    static constexpr std::string_view ZoneConvectiveLoad("ZoneConvectiveLoad");
    static constexpr std::string_view Off("Off");
    static constexpr std::string_view SimpleOff("SimpleOff");
    static constexpr std::string_view VariableOff("VariableOff");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;  // Number of Alphas for each GetobjectItem call
    int NumNumbers; // Number of Numbers for each GetobjectItem call
    int SurfNum;    // Surface number Do loop counter
    int IOStat;
    bool ErrorsFound(false); // If errors detected in input
    int NumCoolingPanels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_CoolingPanel_Simple);

    auto &s_ipsc = state.dataIPShortCut;
    // Count total number of baseboard units

    state.dataChilledCeilingPanelSimple->CoolingPanel.allocate(NumCoolingPanels);

    // Get the data from the user input related to cooling panels
    for (int CoolingPanelNum = 1; CoolingPanelNum <= NumCoolingPanels; ++CoolingPanelNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCMO_CoolingPanel_Simple,
                                                                 CoolingPanelNum,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCMO_CoolingPanel_Simple, s_ipsc->cAlphaArgs(1)};

        state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).FieldNames.allocate(NumNumbers);
        state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).FieldNames = "";
        state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).FieldNames = s_ipsc->cNumericFieldNames;

        if (CoolingPanelNum > 1) {
            for (int CoolPanelNumI = 2; CoolPanelNumI <= NumCoolingPanels; ++CoolPanelNumI) {
                if (s_ipsc->cAlphaArgs(1) == state.dataChilledCeilingPanelSimple->CoolingPanel(CoolPanelNumI).Name) {
                    ErrorsFound = true;
                    ShowSevereError(state, format("{} is used as a name for more than one simple COOLING PANEL.", s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, "This is not allowed.");
                }
            }
        }

        auto &thisCP(state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum));
        thisCP.Name = s_ipsc->cAlphaArgs(1);                                   // Name of this simple cooling panel
        thisCP.EquipType = DataPlant::PlantEquipmentType::CoolingPanel_Simple; //'ZoneHVAC:CoolingPanel:RadiantConvective:Water'

        // Get schedule
        thisCP.Schedule = s_ipsc->cAlphaArgs(2);
        if (s_ipsc->lAlphaFieldBlanks(2)) {
            thisCP.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisCP.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

        // Get inlet node number
        thisCP.WaterInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                    s_ipsc->cAlphaArgs(3),
                                                                    ErrorsFound,
                                                                    DataLoopNode::ConnectionObjectType::ZoneHVACCoolingPanelRadiantConvectiveWater,
                                                                    s_ipsc->cAlphaArgs(1),
                                                                    DataLoopNode::NodeFluidType::Water,
                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                    DataLoopNode::ObjectIsNotParent);

        // Get outlet node number
        thisCP.WaterOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                     s_ipsc->cAlphaArgs(4),
                                                                     ErrorsFound,
                                                                     DataLoopNode::ConnectionObjectType::ZoneHVACCoolingPanelRadiantConvectiveWater,
                                                                     s_ipsc->cAlphaArgs(1),
                                                                     DataLoopNode::NodeFluidType::Water,
                                                                     DataLoopNode::ConnectionType::Outlet,
                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                     DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, cCMO_CoolingPanel_Simple, s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(3), s_ipsc->cAlphaArgs(4), "Chilled Water Nodes");

        thisCP.RatedWaterTemp = s_ipsc->rNumericArgs(1);
        if (thisCP.RatedWaterTemp > MaxWaterTempAvg + 0.001) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(1)));
            ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxWaterTempAvg));
            thisCP.RatedWaterTemp = MaxWaterTempAvg;
        } else if (thisCP.RatedWaterTemp < MinWaterTempAvg - 0.001) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(1)));
            ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinWaterTempAvg));
            thisCP.RatedWaterTemp = MinWaterTempAvg;
        }

        thisCP.RatedZoneAirTemp = s_ipsc->rNumericArgs(2);
        if (thisCP.RatedZoneAirTemp > MaxWaterTempAvg + 0.001) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(2)));
            ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxWaterTempAvg));
            thisCP.RatedZoneAirTemp = MaxWaterTempAvg;
        } else if (thisCP.RatedZoneAirTemp < MinWaterTempAvg - 0.001) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(2)));
            ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinWaterTempAvg));
            thisCP.RatedZoneAirTemp = MinWaterTempAvg;
        }

        thisCP.RatedWaterFlowRate = s_ipsc->rNumericArgs(3);
        if (thisCP.RatedWaterFlowRate < 0.00001 || thisCP.RatedWaterFlowRate > 10.0) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} is an invalid Standard Water mass flow rate.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(2)));
            ShowContinueError(state, format("...reset to a default value=[{:.1R}].", WaterMassFlowDefault));
            thisCP.RatedWaterFlowRate = WaterMassFlowDefault;
        }

        if (Util::SameString(s_ipsc->cAlphaArgs(5), "CoolingDesignCapacity")) {
            thisCP.CoolingCapMethod = DataSizing::CoolingDesignCapacity;
            if (!s_ipsc->lNumericFieldBlanks(4)) {
                thisCP.ScaledCoolingCapacity = s_ipsc->rNumericArgs(4);
                if (thisCP.ScaledCoolingCapacity < 0.0 && thisCP.ScaledCoolingCapacity != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                    ShowContinueError(state, format("Illegal {} = {:.7T}", s_ipsc->cNumericFieldNames(4), s_ipsc->rNumericArgs(4)));
                    ErrorsFound = true;
                }
            } else {
                if ((!s_ipsc->lAlphaFieldBlanks(6)) || (!s_ipsc->lAlphaFieldBlanks(7))) {
                    ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                    ShowContinueError(state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
                    ShowContinueError(state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(4)));
                    ErrorsFound = true;
                }
            }
        } else if (Util::SameString(s_ipsc->cAlphaArgs(5), "CapacityPerFloorArea")) {
            thisCP.CoolingCapMethod = DataSizing::CapacityPerFloorArea;
            if (!s_ipsc->lNumericFieldBlanks(5)) {
                thisCP.ScaledCoolingCapacity = s_ipsc->rNumericArgs(5);
                if (thisCP.ScaledCoolingCapacity < 0.0) {
                    ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                    ShowContinueError(state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
                    ShowContinueError(state, format("Illegal {} = {:.7T}", s_ipsc->cNumericFieldNames(5), s_ipsc->rNumericArgs(5)));
                    ErrorsFound = true;
                } else if (thisCP.ScaledCoolingCapacity == DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                    ShowContinueError(state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
                    ShowContinueError(state, format("Illegal {} = Autosize", s_ipsc->cNumericFieldNames(5)));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                ShowContinueError(state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
                ShowContinueError(state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(5)));
                ErrorsFound = true;
            }
        } else if (Util::SameString(s_ipsc->cAlphaArgs(5), "FractionOfAutosizedCoolingCapacity")) {
            thisCP.CoolingCapMethod = DataSizing::FractionOfAutosizedCoolingCapacity;
            if (!s_ipsc->lNumericFieldBlanks(6)) {
                thisCP.ScaledCoolingCapacity = s_ipsc->rNumericArgs(6);
                if (thisCP.ScaledCoolingCapacity < 0.0) {
                    ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                    ShowContinueError(state, format("Illegal {} = {:.7T}", s_ipsc->cNumericFieldNames(6), s_ipsc->rNumericArgs(6)));
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                ShowContinueError(state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
                ShowContinueError(state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(6)));
                ErrorsFound = true;
            }
        } else {
            ShowSevereError(state, format("{} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
            ShowContinueError(state, format("Illegal {} = {}", s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5)));
            ErrorsFound = true;
        }

        thisCP.WaterVolFlowRateMax = s_ipsc->rNumericArgs(7);
        if ((thisCP.WaterVolFlowRateMax <= MinWaterFlowRate) && thisCP.WaterVolFlowRateMax != DataSizing::AutoSize) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was less than the allowable minimum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(7)));
            ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinWaterFlowRate));
            thisCP.WaterVolFlowRateMax = MinWaterFlowRate;
        } else if (thisCP.WaterVolFlowRateMax > MaxWaterFlowRate) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(7)));
            ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxWaterFlowRate));
            thisCP.WaterVolFlowRateMax = MaxWaterFlowRate;
        }

        // Process the temperature control type
        if (Util::SameString(s_ipsc->cAlphaArgs(6), MeanAirTemperature)) {
            thisCP.controlType = ClgPanelCtrlType::MAT;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(6), MeanRadiantTemperature)) {
            thisCP.controlType = ClgPanelCtrlType::MRT;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(6), OperativeTemperature)) {
            thisCP.controlType = ClgPanelCtrlType::Operative;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(6), OutsideAirDryBulbTemperature)) {
            thisCP.controlType = ClgPanelCtrlType::ODB;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(6), OutsideAirWetBulbTemperature)) {
            thisCP.controlType = ClgPanelCtrlType::OWB;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(6), ZoneTotalLoad)) {
            thisCP.controlType = ClgPanelCtrlType::ZoneTotalLoad;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(6), ZoneConvectiveLoad)) {
            thisCP.controlType = ClgPanelCtrlType::ZoneConvectiveLoad;
        } else {
            ShowWarningError(state, format("Invalid {} ={}", s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6)));
            ShowContinueError(state, format("Occurs in {} = {}", RoutineName, s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, "Control reset to MAT control for this Simple Cooling Panel.");
            thisCP.controlType = ClgPanelCtrlType::MAT;
        }

        thisCP.ColdThrottlRange = s_ipsc->rNumericArgs(8);
        if (thisCP.ColdThrottlRange < MinThrottlingRange) {
            ShowWarningError(state, format("{}Cooling throttling range too small, reset to 0.5", cCMO_CoolingPanel_Simple));
            ShowContinueError(state, format("Occurs in Cooling Panel={}", thisCP.Name));
            thisCP.ColdThrottlRange = MinThrottlingRange;
        }

        thisCP.ColdSetptSchedName = s_ipsc->cAlphaArgs(7);
        if (s_ipsc->lAlphaFieldBlanks(7)) {
            thisCP.coldSetptSched = Sched::GetScheduleAlwaysOff(state);
        } else if ((thisCP.coldSetptSched = Sched::GetSchedule(state, thisCP.ColdSetptSchedName)) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(7), s_ipsc->cAlphaArgs(7));
            ErrorsFound = true;
        }

        if (Util::SameString(s_ipsc->cAlphaArgs(8), Off)) {
            thisCP.CondCtrlType = CondCtrl::NONE;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(8), SimpleOff)) {
            thisCP.CondCtrlType = CondCtrl::SIMPLEOFF;
        } else if (Util::SameString(s_ipsc->cAlphaArgs(8), VariableOff)) {
            thisCP.CondCtrlType = CondCtrl::VARIEDOFF;
        } else {
            thisCP.CondCtrlType = CondCtrl::SIMPLEOFF;
        }

        thisCP.CondDewPtDeltaT = s_ipsc->rNumericArgs(9);

        thisCP.FracRadiant = s_ipsc->rNumericArgs(10);
        if (thisCP.FracRadiant < MinFraction) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(10)));
            ShowContinueError(state, format("...reset to minimum value=[{:.2R}].", MinFraction));
            thisCP.FracRadiant = MinFraction;
        }
        if (thisCP.FracRadiant > MaxFraction) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(10)));
            ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
            thisCP.FracRadiant = MaxFraction;
        }

        // Remaining fraction is added to the zone as convective heat transfer
        if (thisCP.FracRadiant > MaxFraction) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", Fraction Radiant was higher than the allowable maximum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1)));
            thisCP.FracRadiant = MaxFraction;
            thisCP.FracConvect = 0.0;
        } else {
            thisCP.FracConvect = 1.0 - thisCP.FracRadiant;
        }

        thisCP.FracDistribPerson = s_ipsc->rNumericArgs(11);
        if (thisCP.FracDistribPerson < MinFraction) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was lower than the allowable minimum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(11)));
            ShowContinueError(state, format("...reset to minimum value=[{:.3R}].", MinFraction));
            thisCP.FracDistribPerson = MinFraction;
        }
        if (thisCP.FracDistribPerson > MaxFraction) {
            ShowWarningError(state,
                             format("{}{}=\"{}\", {} was higher than the allowable maximum.",
                                    RoutineName,
                                    cCMO_CoolingPanel_Simple,
                                    s_ipsc->cAlphaArgs(1),
                                    s_ipsc->cNumericFieldNames(11)));
            ShowContinueError(state, format("...reset to maximum value=[{:.3R}].", MaxFraction));
            thisCP.FracDistribPerson = MaxFraction;
        }

        thisCP.TotSurfToDistrib = NumNumbers - 11;
        if ((thisCP.TotSurfToDistrib < MinDistribSurfaces) && (thisCP.FracRadiant > MinFraction)) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", the number of surface/radiant fraction groups entered was less than the allowable minimum.",
                                   RoutineName,
                                   cCMO_CoolingPanel_Simple,
                                   s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("...the minimum that must be entered=[{}].", MinDistribSurfaces));
            ErrorsFound = true;
            thisCP.TotSurfToDistrib = 0; // error
        }

        thisCP.SurfaceName.allocate(thisCP.TotSurfToDistrib);
        thisCP.SurfaceName = "";
        thisCP.SurfacePtr.allocate(thisCP.TotSurfToDistrib);
        thisCP.SurfacePtr = 0;
        thisCP.FracDistribToSurf.allocate(thisCP.TotSurfToDistrib);
        thisCP.FracDistribToSurf = 0.0;

        // search zone equipment list structure for zone index
        for (int ctrlZone = 1; ctrlZone <= state.dataGlobal->NumOfZones; ++ctrlZone) {
            for (int zoneEquipTypeNum = 1; zoneEquipTypeNum <= state.dataZoneEquip->ZoneEquipList(ctrlZone).NumOfEquipTypes; ++zoneEquipTypeNum) {
                if (state.dataZoneEquip->ZoneEquipList(ctrlZone).EquipType(zoneEquipTypeNum) == DataZoneEquipment::ZoneEquipType::CoolingPanel &&
                    state.dataZoneEquip->ZoneEquipList(ctrlZone).EquipName(zoneEquipTypeNum) == thisCP.Name) {
                    thisCP.ZonePtr = ctrlZone;
                }
            }
        }
        if (thisCP.ZonePtr <= 0) {
            ShowSevereError(state, format("{}{}=\"{}\" is not on any ZoneHVAC:EquipmentList.", RoutineName, cCMO_CoolingPanel_Simple, thisCP.Name));
            ErrorsFound = true;
            continue;
        }

        Real64 AllFracsSummed = thisCP.FracDistribPerson;
        for (SurfNum = 1; SurfNum <= thisCP.TotSurfToDistrib; ++SurfNum) {
            thisCP.SurfaceName(SurfNum) = s_ipsc->cAlphaArgs(SurfNum + 8);
            thisCP.SurfacePtr(SurfNum) = HeatBalanceIntRadExchange::GetRadiantSystemSurface(
                state, cCMO_CoolingPanel_Simple, thisCP.Name, thisCP.ZonePtr, thisCP.SurfaceName(SurfNum), ErrorsFound);
            thisCP.FracDistribToSurf(SurfNum) = s_ipsc->rNumericArgs(SurfNum + 11);
            if (thisCP.FracDistribToSurf(SurfNum) > MaxFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {}was greater than the allowable maximum.",
                                        RoutineName,
                                        cCMO_CoolingPanel_Simple,
                                        s_ipsc->cAlphaArgs(1),
                                        s_ipsc->cNumericFieldNames(SurfNum + 8)));
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MaxFraction));
                thisCP.TotSurfToDistrib = MaxFraction;
            }
            if (thisCP.FracDistribToSurf(SurfNum) < MinFraction) {
                ShowWarningError(state,
                                 format("{}{}=\"{}\", {}was less than the allowable minimum.",
                                        RoutineName,
                                        cCMO_CoolingPanel_Simple,
                                        s_ipsc->cAlphaArgs(1),
                                        s_ipsc->cNumericFieldNames(SurfNum + 8)));
                ShowContinueError(state, format("...reset to maximum value=[{:.2R}].", MinFraction));
                thisCP.TotSurfToDistrib = MinFraction;
            }
            if (thisCP.SurfacePtr(SurfNum) != 0) {
                state.dataSurface->surfIntConv(thisCP.SurfacePtr(SurfNum)).getsRadiantHeat = true;
                state.dataSurface->allGetsRadiantHeatSurfaceList.emplace_back(thisCP.SurfacePtr(SurfNum));
            }

            AllFracsSummed += thisCP.FracDistribToSurf(SurfNum);
        } // Surfaces

        if (AllFracsSummed > (MaxFraction + 0.01)) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", Summed radiant fractions for people + surface groups > 1.0",
                                   RoutineName,
                                   cCMO_CoolingPanel_Simple,
                                   s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }
        if ((AllFracsSummed < (MaxFraction - 0.01)) &&
            (thisCP.FracRadiant > MinFraction)) { // User didn't distribute all of the | radiation warn that some will be lost
            ShowSevereError(state,
                            format("{}{}=\"{}\", Summed radiant fractions for people + surface groups < 1.0",
                                   RoutineName,
                                   cCMO_CoolingPanel_Simple,
                                   s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, "This would result in some of the radiant energy delivered by the high temp radiant heater being lost.");
            ShowContinueError(state, format("The sum of all radiation fractions to surfaces = {:.5T}", (AllFracsSummed - thisCP.FracDistribPerson)));
            ShowContinueError(state, format("The radiant fraction to people = {:.5T}", thisCP.FracDistribPerson));
            ShowContinueError(state, format("So, all radiant fractions including surfaces and people = {:.5T}", AllFracsSummed));
            ShowContinueError(state,
                              format("This means that the fraction of radiant energy that would be lost from the high temperature radiant heater "
                                     "would be = {:.5T}",
                                     (1.0 - AllFracsSummed)));
            ShowContinueError(state,
                              format("Please check and correct this so that all radiant energy is accounted for in {} = {}",
                                     cCMO_CoolingPanel_Simple,
                                     s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{}{}Errors found getting input. Program terminates.", RoutineName, cCMO_CoolingPanel_Simple));
    }

    // Setup Report variables for the Coils
    for (int CoolingPanelNum = 1; CoolingPanelNum <= NumCoolingPanels; ++CoolingPanelNum) {
        // CurrentModuleObject='ZoneHVAC:CoolingPanel:RadiantConvective:Water'
        auto &thisCP = state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum);
        SetupOutputVariable(state,
                            "Cooling Panel Total Cooling Rate",
                            Constant::Units::W,
                            thisCP.Power,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);
        SetupOutputVariable(state,
                            "Cooling Panel Total System Cooling Rate",
                            Constant::Units::W,
                            thisCP.TotPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);
        SetupOutputVariable(state,
                            "Cooling Panel Convective Cooling Rate",
                            Constant::Units::W,
                            thisCP.ConvPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);
        SetupOutputVariable(state,
                            "Cooling Panel Radiant Cooling Rate",
                            Constant::Units::W,
                            thisCP.RadPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);

        SetupOutputVariable(state,
                            "Cooling Panel Total Cooling Energy",
                            Constant::Units::J,
                            thisCP.Energy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisCP.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingPanel);
        SetupOutputVariable(state,
                            "Cooling Panel Total System Cooling Energy",
                            Constant::Units::J,
                            thisCP.TotEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisCP.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingPanel);
        SetupOutputVariable(state,
                            "Cooling Panel Convective Cooling Energy",
                            Constant::Units::J,
                            thisCP.ConvEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisCP.Name);
        SetupOutputVariable(state,
                            "Cooling Panel Radiant Cooling Energy",
                            Constant::Units::J,
                            thisCP.RadEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisCP.Name);

        SetupOutputVariable(state,
                            "Cooling Panel Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisCP.WaterMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);
        SetupOutputVariable(state,
                            "Cooling Panel Water Inlet Temperature",
                            Constant::Units::C,
                            thisCP.WaterInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);
        SetupOutputVariable(state,
                            "Cooling Panel Water Outlet Temperature",
                            Constant::Units::C,
                            thisCP.WaterOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisCP.Name);
    }
}

void InitCoolingPanel(EnergyPlusData &state, int const CoolingPanelNum, int const ControlledZoneNum, bool const FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Sept 2014

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the cooling panel units, and determines the UA values during simulation.

    // METHODOLOGY EMPLOYED:
    // The initialization subroutines borrowed from other sources and heat exchanger formulation for cooling panel.

    // REFERENCES:
    // Incropera and DeWitt, Fundamentals of Heat and Mass Transfer

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("ChilledCeilingPanelSimple:InitCoolingPanel");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 rho; // local fluid density
    Real64 Cp;  // local fluid specific heat

    auto &thisCP = state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum);
    auto &ThisInNode = state.dataLoopNodes->Node(thisCP.WaterInletNode);

    if (thisCP.ZonePtr <= 0) {
        thisCP.ZonePtr = ControlledZoneNum;
    }

    // Need to check all units to see if they are on ZoneHVAC:EquipmentList or issue warning
    if (!thisCP.ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        thisCP.ZoneEquipmentListChecked = true;
        if (!DataZoneEquipment::CheckZoneEquipmentList(state, cCMO_CoolingPanel_Simple, thisCP.Name)) {
            ShowSevereError(state,
                            format("InitCoolingPanel: Unit=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                   cCMO_CoolingPanel_Simple,
                                   thisCP.Name));
        }
    }

    if (thisCP.SetLoopIndexFlag) {
        if (allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state, thisCP.Name, thisCP.EquipType, thisCP.plantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "InitCoolingPanel: Program terminated for previous conditions.");
            }
            thisCP.SetLoopIndexFlag = false;
        }
    }

    if (!state.dataGlobal->SysSizingCalc) {
        if (thisCP.MySizeFlagCoolPanel && !thisCP.SetLoopIndexFlag) {
            // for each cooling panel do the sizing once.
            SizeCoolingPanel(state, CoolingPanelNum);
            thisCP.MySizeFlagCoolPanel = false;

            // set design mass flow rates
            if (thisCP.WaterInletNode > 0) {
                rho = thisCP.plantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                thisCP.WaterMassFlowRateMax = rho * thisCP.WaterVolFlowRateMax;
                PlantUtilities::InitComponentNodes(state, 0.0, thisCP.WaterMassFlowRateMax, thisCP.WaterInletNode, thisCP.WaterOutletNode);
            }
        }
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && thisCP.MyEnvrnFlag) {
        // Initialize

        rho = thisCP.plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);

        thisCP.WaterMassFlowRateMax = rho * thisCP.WaterVolFlowRateMax;

        PlantUtilities::InitComponentNodes(state, 0.0, thisCP.WaterMassFlowRateMax, thisCP.WaterInletNode, thisCP.WaterOutletNode);

        ThisInNode.Temp = 7.0;

        Cp = thisCP.plantLoc.loop->glycol->getSpecificHeat(state, ThisInNode.Temp, RoutineName);

        ThisInNode.Enthalpy = Cp * ThisInNode.Temp;
        ThisInNode.Quality = 0.0;
        ThisInNode.Press = 0.0;
        ThisInNode.HumRat = 0.0;

        thisCP.ZeroCPSourceSumHATsurf = 0.0;
        thisCP.CoolingPanelSource = 0.0;
        thisCP.CoolingPanelSrcAvg = 0.0;
        thisCP.LastCoolingPanelSrc = 0.0;
        thisCP.LastSysTimeElapsed = 0.0;
        thisCP.LastTimeStepSys = 0.0;

        thisCP.MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        thisCP.MyEnvrnFlag = true;
    }

    if (state.dataGlobal->BeginTimeStepFlag && FirstHVACIteration) {
        int ZoneNum = thisCP.ZonePtr;
        thisCP.ZeroCPSourceSumHATsurf = state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state);
        thisCP.CoolingPanelSrcAvg = 0.0;
        thisCP.LastCoolingPanelSrc = 0.0;
        thisCP.LastSysTimeElapsed = 0.0;
        thisCP.LastTimeStepSys = 0.0;
    }

    // Do the every time step initializations
    thisCP.WaterMassFlowRate = ThisInNode.MassFlowRate;
    thisCP.WaterInletTemp = ThisInNode.Temp;
    thisCP.WaterInletEnthalpy = ThisInNode.Enthalpy;
    thisCP.TotPower = 0.0;
    thisCP.Power = 0.0;
    thisCP.ConvPower = 0.0;
    thisCP.RadPower = 0.0;
    thisCP.TotEnergy = 0.0;
    thisCP.Energy = 0.0;
    thisCP.ConvEnergy = 0.0;
    thisCP.RadEnergy = 0.0;
}

void SizeCoolingPanel(EnergyPlusData &state, int const CoolingPanelNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Sept 2016

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sizes the simple chilled ceiling panel.  The process used here
    // was derived from the low temperature radiant system model and adapted for
    // cooling only.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizeCoolingPanel");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false); // If errors detected in input
    bool IsAutoSize(false);  // Indicator to autosize
    Real64 DesCoilLoad;      // design autosized or user specified capacity
    Real64 TempSize;         // autosized value of coil input field
    Real64 rho;
    Real64 Cp;
    Real64 WaterVolFlowMaxCoolDes(0.0);  // Design chilled water flow for reporting
    Real64 WaterVolFlowMaxCoolUser(0.0); // User hard-sized chilled water flow for reporting

    DesCoilLoad = 0.0;
    state.dataSize->DataScalableCapSizingON = false;

    auto &thisCP(state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum));

    std::string_view const CompType = "ZoneHVAC:CoolingPanel:RadiantConvective:Water";
    std::string_view const CompName = thisCP.Name;

    IsAutoSize = false;
    if (thisCP.ScaledCoolingCapacity == DataSizing::AutoSize) {
        IsAutoSize = true;
    }

    if (state.dataSize->CurZoneEqNum > 0) {

        auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
        int SizingMethod = HVAC::CoolingCapacitySizing;
        bool PrintFlag = true; // TRUE when sizing information is reported in the eio file
        bool errorsFound = false;
        int CapSizingMethod = thisCP.CoolingCapMethod;
        zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;

        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
            if (CapSizingMethod == DataSizing::CoolingDesignCapacity && thisCP.ScaledCoolingCapacity > 0.0) {
                TempSize = thisCP.ScaledCoolingCapacity;
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, errorsFound);
            } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                state.dataSize->DataScalableCapSizingON = true;
                TempSize = thisCP.ScaledCoolingCapacity * state.dataHeatBal->Zone(thisCP.ZonePtr).FloorArea;
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, errorsFound);
                state.dataSize->DataScalableCapSizingON = false;
            } else if (CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                if (thisCP.WaterVolFlowRateMax == DataSizing::AutoSize) {
                    ShowSevereError(state, format("{}: auto-sizing cannot be done for {} = {}\".", RoutineName, CompType, thisCP.Name));
                    ShowContinueError(state,
                                      "The \"SimulationControl\" object must have the field \"Do Zone Sizing Calculation\" set to Yes when the "
                                      "Cooling Design Capacity Method = \"FractionOfAutosizedCoolingCapacity\".");
                    ErrorsFound = true;
                }
            }
        } else { // Autosize or hard-size with sizing run
            if (CapSizingMethod == DataSizing::CoolingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                if (CapSizingMethod == DataSizing::CoolingDesignCapacity) {
                    if (state.dataSize->ZoneSizingRunDone) {
                        CheckZoneSizing(state, CompType, CompName);
                        state.dataSize->DataConstantUsedForSizing =
                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
                        state.dataSize->DataFractionUsedForSizing = 1.0;
                    }
                    TempSize = thisCP.ScaledCoolingCapacity;
                } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                    if (state.dataSize->ZoneSizingRunDone) {
                        CheckZoneSizing(state, CompType, CompName);
                        zoneEqSizing.CoolingCapacity = true;
                        zoneEqSizing.DesCoolingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
                    }
                    TempSize = thisCP.ScaledCoolingCapacity * state.dataHeatBal->Zone(thisCP.ZonePtr).FloorArea;
                    state.dataSize->DataScalableCapSizingON = true;
                } else if (CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                    CheckZoneSizing(state, CompType, CompName);
                    zoneEqSizing.CoolingCapacity = true;
                    zoneEqSizing.DesCoolingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
                    TempSize = zoneEqSizing.DesCoolingLoad * thisCP.ScaledCoolingCapacity;
                    state.dataSize->DataScalableCapSizingON = true;

                } else {
                    TempSize = thisCP.ScaledCoolingCapacity;
                }
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, errorsFound);
                state.dataSize->DataConstantUsedForSizing = 0.0;
                state.dataSize->DataFractionUsedForSizing = 0.0;
                state.dataSize->DataScalableCapSizingON = false;
            } else {
                DesCoilLoad = 0.0;
            }
        }
        // finally cooling capacity is saved in this variable
        thisCP.ScaledCoolingCapacity = DesCoilLoad;
    }

    IsAutoSize = false;
    if (thisCP.WaterVolFlowRateMax == DataSizing::AutoSize) {
        IsAutoSize = true;
    }
    if (state.dataSize->CurZoneEqNum > 0) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
            if (thisCP.WaterVolFlowRateMax > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, CompType, thisCP.Name, "User-Specified Maximum Cold Water Flow [m3/s]", thisCP.WaterVolFlowRateMax);
            }
        } else { // Autosize or hard-size with sizing run
            if (thisCP.WaterInletNode > 0 && thisCP.WaterOutletNode > 0) {
                int PltSizCoolNum =
                    PlantUtilities::MyPlantSizingIndex(state, CompType, thisCP.Name, thisCP.WaterInletNode, thisCP.WaterOutletNode, ErrorsFound);
                if (PltSizCoolNum > 0) {
                    if (DesCoilLoad >= HVAC::SmallLoad) {
                        rho = thisCP.plantLoc.loop->glycol->getDensity(state, 5., RoutineName);
                        Cp = thisCP.plantLoc.loop->glycol->getSpecificHeat(state, 5.0, RoutineName);
                        WaterVolFlowMaxCoolDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizCoolNum).DeltaT * Cp * rho);
                    } else {
                        WaterVolFlowMaxCoolDes = 0.0;
                    }
                } else {
                    ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                    ShowContinueError(state, format("Occurs in ZoneHVAC:CoolingPanel:RadiantConvective:Water Object={}", thisCP.Name));
                }
            }

            if (IsAutoSize) {
                thisCP.WaterVolFlowRateMax = WaterVolFlowMaxCoolDes;
                BaseSizer::reportSizerOutput(state, CompType, thisCP.Name, "Design Size Maximum Cold Water Flow [m3/s]", WaterVolFlowMaxCoolDes);
            } else { // hard-size with sizing data
                if (thisCP.WaterVolFlowRateMax > 0.0 && WaterVolFlowMaxCoolDes > 0.0) {
                    WaterVolFlowMaxCoolUser = thisCP.WaterVolFlowRateMax;
                    BaseSizer::reportSizerOutput(state,
                                                 CompType,
                                                 thisCP.Name,
                                                 "Design Size Maximum Cold Water Flow [m3/s]",
                                                 WaterVolFlowMaxCoolDes,
                                                 "User-Specified Maximum Cold Water Flow [m3/s]",
                                                 WaterVolFlowMaxCoolUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(WaterVolFlowMaxCoolDes - WaterVolFlowMaxCoolUser) / WaterVolFlowMaxCoolUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state,
                                        format("SizeCoolingPanel: Potential issue with equipment sizing for "
                                               "ZoneHVAC:CoolingPanel:RadiantConvective:Water = \"{}\".",
                                               thisCP.Name));
                            ShowContinueError(state, format("User-Specified Maximum Cool Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxCoolUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Maximum Cool Water Flow of {:.5R} [m3/s]", WaterVolFlowMaxCoolDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }

    PlantUtilities::RegisterPlantCompDesignFlow(state, thisCP.WaterInletNode, thisCP.WaterVolFlowRateMax);
    BaseSizer::calcCoilWaterFlowRates(state,
                                      thisCP.Name,
                                      "ZoneHVAC:CoolingPanel:RadiantConvective:Water",
                                      thisCP.WaterVolFlowRateMax,
                                      thisCP.plantLoc.loopNum,
                                      state.dataSize->CurZoneEqNum,
                                      state.dataSize->CurSysNum,
                                      state.dataSize->CurOASysNum,
                                      state.dataSize->FinalZoneSizing,
                                      state.dataSize->FinalSysSizing);

    if (!thisCP.SizeCoolingPanelUA(state)) {
        ShowFatalError(state, "SizeCoolingPanelUA: Program terminated for previous conditions.");
    }
}

bool CoolingPanelParams::SizeCoolingPanelUA(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   June 2017

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sizes UA value for the simple chilled ceiling panel.

    // These initializations are mainly the calculation of the UA value for the heat exchanger formulation of the simple cooling panel
    Real64 RatCapToTheoMax; // Ratio of unit capacity to theoretical maximum output based on rated parameters

    Real64 constexpr Cp = 4120.0; // Just an approximation, don't need to get an exact number
    Real64 const MDot = this->RatedWaterFlowRate;
    Real64 const MDotXCp = Cp * MDot;
    Real64 const Qrated = this->ScaledCoolingCapacity;
    Real64 const Tinletr = this->RatedWaterTemp;
    Real64 const Tzoner = this->RatedZoneAirTemp;

    if (Tinletr >= Tzoner) {
        ShowSevereError(state,
                        format("SizeCoolingPanelUA: Unit=[{},{}] has a rated water temperature that is higher than the rated zone temperature.",
                               cCMO_CoolingPanel_Simple,
                               this->Name));
        ShowContinueError(state,
                          "Such a situation would not lead to cooling and thus the rated water or zone temperature or both should be adjusted.");
        this->UA = 1.0;
        return false;
    }

    if ((Tzoner - Tinletr) < 0.5) {
        RatCapToTheoMax = std::abs(Qrated) / (MDotXCp * 0.5); // Avoid a divide by zero error
    } else {
        RatCapToTheoMax = std::abs(Qrated) / (MDotXCp * std::abs(Tinletr - Tzoner));
    }
    if ((RatCapToTheoMax < 1.1) && (RatCapToTheoMax > 0.9999)) {
        // close to unity with some graciousness given in case the approximation of Cp causes a problem
        RatCapToTheoMax = 0.9999;
    } else if (RatCapToTheoMax >= 1.1) {
        ShowSevereError(state,
                        format("SizeCoolingPanelUA: Unit=[{},{}] has a cooling capacity that is greater than the maximum possible value.",
                               cCMO_CoolingPanel_Simple,
                               this->Name));
        ShowContinueError(state, "The result of this is that a UA value is impossible to calculate.");
        ShowContinueError(state, "Check the rated input for temperatures, flow, and capacity for this unit.");
        ShowContinueError(state, "The ratio of the capacity to the rated theoretical maximum must be less than unity.");
        ShowContinueError(state,
                          "The most likely cause for this is probably either the capacity (whether autosized or hardwired) being too high, the "
                          "rated flow being too low, rated temperatures being too close to each other, or all of those reasons.");
        ShowContinueError(state,
                          "Compare the rated capacity in your input to the product of the rated mass flow rate, Cp of water, and the difference "
                          "between the rated temperatures.");
        ShowContinueError(
            state, "If the rated capacity is higher than this product, then the cooling panel would violate the Second Law of Thermodynamics.");
        this->UA = 1.0;
        return false;
    }

    this->UA = -MDotXCp * log(1.0 - RatCapToTheoMax);
    if (this->UA <= 0.0) {
        ShowSevereError(state,
                        format("SizeCoolingPanelUA: Unit=[{},{}] has a zero or negative calculated UA value.", cCMO_CoolingPanel_Simple, this->Name));
        ShowContinueError(state,
                          "This is not allowed.  Please check the rated input parameters for this device to ensure that the values are correct.");
        return false;
    }

    return true;
}

void CoolingPanelParams::CalcCoolingPanel(EnergyPlusData &state, int const CoolingPanelNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Sept 2014

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates both the convective and radiant heat transfer rate
    // for the simple cooling panel.  The process used here was derived from the hot
    // water baseboard radiant/convective heater and adapted for cooling.

    // REFERENCES:
    // Existing code for hot water baseboard models (radiant-convective variety)
    // Incropera and DeWitt, Fundamentals of Heat and Mass Transfer

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr MinFrac(0.0005); // Minimum fraction that delivers radiant heats to surfaces
    int constexpr Maxiter(20);        // Maximum number of iterations to achieve tolerance
    Real64 constexpr IterTol(0.005);  // Tolerance of 0.5%
    static constexpr std::string_view RoutineName("CalcCoolingPanel");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 RadHeat;
    Real64 CoolingPanelCool;
    Real64 waterMassFlowRate;
    Real64 CapacitanceWater;
    Real64 NTU;
    Real64 Effectiveness;
    Real64 Cp;
    Real64 MCpEpsAct;
    Real64 MCpEpsLow;
    Real64 MCpEpsHigh;
    Real64 MdotLow;
    Real64 MdotHigh;
    Real64 FracGuess;
    Real64 MdotGuess;
    Real64 MCpEpsGuess;
    Real64 ControlTemp;
    Real64 SetPointTemp;
    Real64 OffTempCool;
    Real64 FullOnTempCool;
    Real64 MassFlowFrac;
    Real64 LoadMet;
    bool CoolingPanelOn;
    Real64 waterOutletTemp;

    int ZoneNum = this->ZonePtr;
    Real64 QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
    Real64 waterInletTemp = this->WaterInletTemp;
    Real64 waterMassFlowRateMax = this->WaterMassFlowRateMax;
    Real64 Xr = this->FracRadiant;

    CoolingPanelOn = this->availSched->getCurrentVal() > 0;

    // Calculate the "zone" temperature for determining the output of the cooling panel
    auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
    Real64 Tzone = Xr * thisZoneHB.MRT + ((1.0 - Xr) * thisZoneHB.MAT);

    // Logical controls: if the WaterInletTemperature is higher than Tzone, do not run the panel
    if (waterInletTemp >= Tzone) {
        CoolingPanelOn = false;
    }

    // Condensation Controls based on dewpoint temperature of the zone.
    // The assumption here is that condensation might take place if the inlet water temperature
    // is below the dewpoint temperature of the space.  This assumption is made because we are
    // probably dealing with a metal panel and the surface temperature of the panel will be very
    // close to the inlet water temperature in certain places.  Thus, if the water inlet temperature
    // is below the dewpoint temperature, then we might have condensation.  We need to deal with this
    // possibility based on the user selected method.  The good news here is that we don't have to
    // iterate like in the low temperature radiant systems because the inlet water condition is known
    // not calculated.  So, we can deal with this upfront rather than after calculation and then more
    // iteration.
    Real64 DewPointTemp =
        Psychrometrics::PsyTdpFnWPb(state, state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRat, state.dataEnvrn->OutBaroPress);

    if (waterInletTemp < (DewPointTemp + this->CondDewPtDeltaT) && (CoolingPanelOn)) {

        // Condensation is possible so invoke the three possible ways of handling this based on the user's choice...

        if (this->CondCtrlType == CondCtrl::NONE) {
            // Condensation control is "off" which means don't do anything, simply let it run and ignore condensation
        } else if (this->CondCtrlType == CondCtrl::SIMPLEOFF) {
            // For "simple off", simply turn the simple cooling panel off to avoid condensation
            waterMassFlowRate = 0.0;
            CoolingPanelOn = false;
            // Produce a warning message so that user knows the system was shut-off due to potential for condensation
            if (!state.dataGlobal->WarmupFlag) {
                if (this->CondErrIndex == 0) { // allow errors up to number of radiant systems
                    ShowWarningMessage(state,
                                       format("{} [{}] inlet water temperature below dew-point temperature--potential for condensation exists",
                                              cCMO_CoolingPanel_Simple,
                                              this->Name));
                    ShowContinueError(state, "Flow to the simple cooling panel will be shut-off to avoid condensation");
                    ShowContinueError(state, format("Water inlet temperature = {:.2R}", waterInletTemp));
                    ShowContinueError(state, format("Zone dew-point temperature + safety delta T= {:.2R}", DewPointTemp + this->CondDewPtDeltaT));
                    ShowContinueErrorTimeStamp(state, "");
                    ShowContinueError(state,
                                      format("Note that a {:.4R} C safety was chosen in the input for the shut-off criteria", this->CondDewPtDeltaT));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               cCMO_CoolingPanel_Simple + " [" + this->Name + "] condensation shut-off occurrence continues.",
                                               this->CondErrIndex,
                                               DewPointTemp,
                                               DewPointTemp,
                                               _,
                                               "C",
                                               "C");
            }

        } else if (this->CondCtrlType == CondCtrl::VARIEDOFF) {
            // Varied off is the most complex because it tries to run by reducing the inlet temperature
            // As a result of this, there is some bypass/recirculation that has to take place.
            // We might not have enough flow rate to meet whatever load we have, but at least
            // the system is still running at some partial load and avoiding condensation.
            waterInletTemp = DewPointTemp + this->CondDewPtDeltaT;
        }
    }

    // The next IF block is to find the mass flow rate based on what type of control the user has requested.  Load based controls
    // vary the flow to meet the zone load calculated by the user-defined thermostat.  Temperature based controls vary the flow
    // based on a comparison between the control temperature and the setpoint schedule and throttling range.

    if ((this->controlType == ClgPanelCtrlType::ZoneTotalLoad) || (this->controlType == ClgPanelCtrlType::ZoneConvectiveLoad)) {

        if (QZnReq < -HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) && (CoolingPanelOn)) {

            Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, waterInletTemp, RoutineName);

            // Find the actual load: this parameter modifies what the response of the system should be.  For total load control, the system tries
            // to meet the QZnReq.  For convective load control, the convective output of the device equals QZnReq which means that the load on
            // the panel is higher as is its output.  Total load control will miss the setpoint temperature but will likely get there with time.
            // Convective load control will hit the setpoint short term better but will result in overcooling in the long run probably.
            if (this->controlType == ClgPanelCtrlType::ZoneConvectiveLoad) {
                QZnReq = QZnReq / this->FracConvect;
            }

            // Now for a small amount of iteration.  Try to find the value of mass flow rate that will come the closest to giving
            // the proper value for MCpEpsAct.  Limit iterations to avoid too much time wasting.
            MCpEpsAct = QZnReq / (waterInletTemp - Tzone);
            MCpEpsLow = 0.0;
            MdotLow = 0.0;
            MCpEpsHigh = waterMassFlowRateMax * Cp * (1.0 - exp(-this->UA / (waterMassFlowRateMax * Cp)));
            MdotHigh = waterMassFlowRateMax;
            if (MCpEpsAct <= MCpEpsLow) {
                MCpEpsAct = MCpEpsLow;
                waterMassFlowRate = 0.0;
                state.dataLoopNodes->Node(this->WaterInletNode).MassFlowRate = 0.0;
                CoolingPanelOn = false;
            } else if (MCpEpsAct >= MCpEpsHigh) {
                MCpEpsAct = MCpEpsHigh;
                waterMassFlowRate = waterMassFlowRateMax;
                state.dataLoopNodes->Node(this->WaterInletNode).MassFlowRate = waterMassFlowRateMax;
            } else {
                for (int iter = 1; iter <= Maxiter; ++iter) {
                    FracGuess = (MCpEpsAct - MCpEpsLow) / (MCpEpsHigh - MCpEpsLow);
                    MdotGuess = MdotHigh * FracGuess;
                    MCpEpsGuess = MdotGuess * Cp * (1.0 - exp(-this->UA / (MdotGuess * Cp)));
                    if (MCpEpsGuess <= MCpEpsAct) {
                        MCpEpsLow = MCpEpsGuess;
                        MdotLow = MdotGuess;
                    } else { // MCpEpsGuess > MCpEpsAct
                        MCpEpsHigh = MCpEpsGuess;
                        MdotHigh = MdotGuess;
                    }
                    if (((MCpEpsAct - MCpEpsGuess) / MCpEpsAct) <= IterTol) {
                        waterMassFlowRate = MdotGuess;
                        state.dataLoopNodes->Node(this->WaterInletNode).MassFlowRate = waterMassFlowRate;
                        break;
                    }
                }
            }

        } else {
            CoolingPanelOn = false;
        }

    } else { // temperature control rather than zone load control

        if (CoolingPanelOn) {

            ControlTemp = this->getCoolingPanelControlTemp(state, ZoneNum);

            SetPointTemp = this->coldSetptSched->getCurrentVal();
            OffTempCool = SetPointTemp - 0.5 * this->ColdThrottlRange;
            FullOnTempCool = SetPointTemp + 0.5 * this->ColdThrottlRange;

            if (ControlTemp <= OffTempCool) {
                MassFlowFrac = 0.0;
                CoolingPanelOn = false;
            } else if (ControlTemp >= FullOnTempCool) {
                MassFlowFrac = 1.0;
            } else {
                MassFlowFrac = (ControlTemp - OffTempCool) / this->ColdThrottlRange;
                if (MassFlowFrac < MinFrac) {
                    MassFlowFrac = MinFrac;
                }
            }

            waterMassFlowRate = MassFlowFrac * waterMassFlowRateMax;
        }
    }

    if (CoolingPanelOn) {
        PlantUtilities::SetComponentFlowRate(state, waterMassFlowRate, this->WaterInletNode, this->WaterOutletNode, this->plantLoc);
        if (waterMassFlowRate <= 0.0) {
            CoolingPanelOn = false;
        }
    }

    if (CoolingPanelOn) {
        // Now simulate the system...
        Cp = this->plantLoc.loop->glycol->getSpecificHeat(state, waterInletTemp, RoutineName);
        Effectiveness = 1.0 - exp(-this->UA / (waterMassFlowRate * Cp));
        if (Effectiveness <= 0.0) {
            Effectiveness = 0.0;
        } else if (Effectiveness >= 1.0) {
            Effectiveness = 1.0;
        }
        CoolingPanelCool = (Effectiveness)*waterMassFlowRate * Cp * (waterInletTemp - Tzone);
        waterOutletTemp = this->WaterInletTemp - (CoolingPanelCool / (waterMassFlowRate * Cp));
        RadHeat = CoolingPanelCool * this->FracRadiant;
        state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).CoolingPanelSource = RadHeat;

        if (this->FracRadiant <= MinFrac) {
            LoadMet = CoolingPanelCool;
        } else {

            // Now, distribute the radiant energy of all systems to the appropriate surfaces, to people, and the air
            DistributeCoolingPanelRadGains(state);
            // Now "simulate" the system by recalculating the heat balances
            HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(state, ZoneNum);

            HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(state, ZoneNum);

            // Here an assumption is made regarding radiant heat transfer to people.
            // While the radiant heat transfer to people array will be used by the thermal comfort
            // routines, the energy transfer to people would get lost from the perspective
            // of the heat balance.  So, to avoid this net loss of energy which clearly
            // gets added to the zones, we must account for it somehow.  This assumption
            // that all energy radiated to people is converted to convective energy is
            // not very precise, but at least it conserves energy. The system impact to heat balance
            // should include this.
            LoadMet = (state.dataHeatBal->Zone(ZoneNum).sumHATsurf(state) - this->ZeroCPSourceSumHATsurf) + (CoolingPanelCool * this->FracConvect) +
                      (RadHeat * this->FracDistribPerson);
        }
        this->WaterOutletEnthalpy = this->WaterInletEnthalpy - CoolingPanelCool / waterMassFlowRate;

    } else { // cooling panel off
        CapacitanceWater = 0.0;
        NTU = 0.0;
        Effectiveness = 0.0;
        waterOutletTemp = waterInletTemp;
        CoolingPanelCool = 0.0;
        LoadMet = 0.0;
        RadHeat = 0.0;
        waterMassFlowRate = 0.0;
        this->CoolingPanelSource = 0.0;
        this->WaterOutletEnthalpy = this->WaterInletEnthalpy;
    }

    this->WaterOutletTemp = waterOutletTemp;
    this->WaterMassFlowRate = waterMassFlowRate;
    this->TotPower = LoadMet;
    this->Power = CoolingPanelCool;
    this->ConvPower = CoolingPanelCool - RadHeat;
    this->RadPower = RadHeat;
}

Real64 CoolingPanelParams::getCoolingPanelControlTemp(EnergyPlusData &state, int const ZoneNum) const
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   July 2016

    // METHODOLOGY EMPLOYED:
    // This subroutine sets the control temperature for the simple cooling panel.

    // Using/Aliasing

    switch (this->controlType) {
    case ClgPanelCtrlType::MAT: {
        return state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
    } break;
    case ClgPanelCtrlType::MRT: {
        return state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MRT;
    } break;
    case ClgPanelCtrlType::Operative: {
        return 0.5 * (state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT +
                      state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MRT);
    } break;
    case ClgPanelCtrlType::ODB: {
        return state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp;
    } break;
    case ClgPanelCtrlType::OWB: {
        return state.dataHeatBal->Zone(ZoneNum).OutWetBulbTemp;
    } break;
    default: { // Should never get here
        assert(false);
        return -99990; // Compiler wants a return value for every path, so give an invalid value
    } break;
    }
}

void UpdateCoolingPanel(EnergyPlusData &state, int const CoolingPanelNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Sept 2014
    //                      February 2001
    //       MODIFIED       Aug 2007 Daeho Kang (Add the update of radiant source)

    // REFERENCES:
    // Existing code for hot water baseboard models (radiant-convective variety)

    // Using/Aliasing
    Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
    Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
    auto &thisCP(state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum));

    // First, update the running average if necessary...
    if (thisCP.LastSysTimeElapsed == SysTimeElapsed) {
        thisCP.CoolingPanelSrcAvg -= thisCP.LastCoolingPanelSrc * thisCP.LastTimeStepSys / state.dataGlobal->TimeStepZone;
    }
    // Update the running average and the "last" values with the current values of the appropriate variables
    thisCP.CoolingPanelSrcAvg += thisCP.CoolingPanelSource * TimeStepSys / state.dataGlobal->TimeStepZone;

    thisCP.LastCoolingPanelSrc = thisCP.CoolingPanelSource;
    thisCP.LastSysTimeElapsed = SysTimeElapsed;
    thisCP.LastTimeStepSys = TimeStepSys;

    int WaterInletNode = thisCP.WaterInletNode;
    int WaterOutletNode = thisCP.WaterOutletNode;

    auto &ThisInNode(state.dataLoopNodes->Node(WaterInletNode));
    auto &ThisOutNode(state.dataLoopNodes->Node(WaterOutletNode));

    // Set the outlet water nodes for the panel
    PlantUtilities::SafeCopyPlantNode(state, WaterInletNode, WaterOutletNode);
    ThisOutNode.Temp = thisCP.WaterOutletTemp;
    ThisOutNode.Enthalpy = thisCP.WaterOutletEnthalpy;
    ThisInNode.MassFlowRate = thisCP.WaterMassFlowRate;
    ThisOutNode.MassFlowRate = thisCP.WaterMassFlowRate;
    ThisInNode.MassFlowRateMax = thisCP.WaterMassFlowRateMax;
    ThisOutNode.MassFlowRateMax = thisCP.WaterMassFlowRateMax;
}

void UpdateCoolingPanelSourceValAvg(EnergyPlusData &state,
                                    bool &CoolingPanelSysOn) // .TRUE. if the radiant system has run this zone time step
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Sept 2014

    // PURPOSE OF THIS SUBROUTINE:
    // To transfer the average value of the heat source over the entire
    // zone time step back to the heat balance routines so that the heat
    // balance algorithms can simulate one last time with the average source
    // to maintain some reasonable amount of continuity and energy balance
    // in the temperature and flux histories.

    // METHODOLOGY EMPLOYED:
    // All of the record keeping for the average term is done in the Update
    // routine so the only other thing that this subroutine does is check to
    // see if the system was even on.  If any average term is non-zero, then
    // one or more of the radiant systems was running.

    // REFERENCES:
    // Existing code for hot water baseboard models (radiant-convective variety)

    CoolingPanelSysOn = false;

    // If this was never allocated, then there are no radiant systems in this input file (just RETURN)
    if (!allocated(state.dataChilledCeilingPanelSimple->CoolingPanel)) {
        return;
    }

    // If it was allocated, then we have to check to see if this was running at all...
    for (int CoolingPanelNum = 1; CoolingPanelNum <= (int)state.dataChilledCeilingPanelSimple->CoolingPanel.size(); ++CoolingPanelNum) {
        if (state.dataChilledCeilingPanelSimple->CoolingPanel(CoolingPanelNum).CoolingPanelSrcAvg != 0.0) {
            CoolingPanelSysOn = true;
            break; // DO loop
        }
    }

    for (auto &cp : state.dataChilledCeilingPanelSimple->CoolingPanel) {
        cp.CoolingPanelSource = cp.CoolingPanelSrcAvg;
    }

    DistributeCoolingPanelRadGains(state); // CoolingPanelRadSource has been modified so we need to redistribute gains
}

void DistributeCoolingPanelRadGains(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Sept 2014

    // PURPOSE OF THIS SUBROUTINE:
    // To distribute the gains from the hot water baseboard heater
    // as specified in the user input file.  This includes distribution
    // of long wavelength radiant gains to surfaces and "people."

    // METHODOLOGY EMPLOYED:
    // We must cycle through all of the radiant systems because each
    // surface could feel the effect of more than one radiant system.
    // Note that the energy radiated to people is assumed to affect them
    // but them it is assumed to be convected to the air.

    // REFERENCES:
    // Existing code for hot water baseboard models (radiant-convective variety)

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr SmallestArea(0.001); // Smallest area in meters squared (to avoid a divide by zero)

    // Initialize arrays
    for (auto &thisCP : state.dataChilledCeilingPanelSimple->CoolingPanel) {
        for (int radSurfNum = 1; radSurfNum <= thisCP.TotSurfToDistrib; ++radSurfNum) {
            int surfNum = thisCP.SurfacePtr(radSurfNum);
            state.dataHeatBalFanSys->surfQRadFromHVAC(surfNum).CoolingPanel = 0.0;
        }
    }
    state.dataHeatBalFanSys->ZoneQCoolingPanelToPerson = 0.0;

    for (auto &thisCP : state.dataChilledCeilingPanelSimple->CoolingPanel) {
        int ZoneNum = thisCP.ZonePtr;
        if (ZoneNum <= 0) {
            continue;
        }
        state.dataHeatBalFanSys->ZoneQCoolingPanelToPerson(ZoneNum) += thisCP.CoolingPanelSource * thisCP.FracDistribPerson;

        for (int RadSurfNum = 1; RadSurfNum <= thisCP.TotSurfToDistrib; ++RadSurfNum) {
            int SurfNum = thisCP.SurfacePtr(RadSurfNum);
            auto &ThisSurf(state.dataSurface->Surface(SurfNum));
            if (ThisSurf.Area > SmallestArea) {
                Real64 ThisSurfIntensity = (thisCP.CoolingPanelSource * thisCP.FracDistribToSurf(RadSurfNum) / ThisSurf.Area);
                state.dataHeatBalFanSys->surfQRadFromHVAC(SurfNum).CoolingPanel += ThisSurfIntensity;
                // CR 8074, trap for excessive intensity (throws off surface balance )
                if (ThisSurfIntensity > DataHeatBalFanSys::MaxRadHeatFlux) {
                    ShowSevereError(state, "DistributeCoolingPanelRadGains:  excessive thermal radiation heat flux intensity detected");
                    ShowContinueError(state, format("Surface = {}", ThisSurf.Name));
                    ShowContinueError(state, format("Surface area = {:.3R} [m2]", ThisSurf.Area));
                    ShowContinueError(state, format("Occurs in {} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                    ShowContinueError(state, format("Radiation intensity = {:.2R} [W/m2]", ThisSurfIntensity));
                    ShowContinueError(state, format("Assign a larger surface area or more surfaces in {}", cCMO_CoolingPanel_Simple));
                    ShowFatalError(state, "DistributeCoolingPanelRadGains:  excessive thermal radiation heat flux intensity detected");
                }
            } else {
                ShowSevereError(state, "DistributeCoolingPanelRadGains:  surface not large enough to receive thermal radiation heat flux");
                ShowContinueError(state, format("Surface = {}", ThisSurf.Name));
                ShowContinueError(state, format("Surface area = {:.3R} [m2]", ThisSurf.Area));
                ShowContinueError(state, format("Occurs in {} = {}", cCMO_CoolingPanel_Simple, thisCP.Name));
                ShowContinueError(state, format("Assign a larger surface area or more surfaces in {}", cCMO_CoolingPanel_Simple));
                ShowFatalError(state, "DistributeCoolingPanelRadGains:  surface not large enough to receive thermal radiation heat flux");
            }
        }
    }
}

void CoolingPanelParams::ReportCoolingPanel(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   Aug 2014

    // REFERENCES:
    // Existing code for hot water baseboard models (radiant-convective variety)

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // All of the power numbers are negative for cooling.  This is because they will have a negative
    // or cooling impact on the surfaces/zones.  However, the output variables are noted as cooling.
    // So, their sign should be positive if actually cooling and we need to reverse the sign here.
    // This should not have an impact on any of the internal variables or the heat balances because
    // those use other variables.
    this->TotPower = -this->TotPower;
    this->Power = -this->Power;
    this->ConvPower = -this->ConvPower;
    this->RadPower = -this->RadPower;

    this->TotEnergy = this->TotPower * TimeStepSysSec;
    this->Energy = this->Power * TimeStepSysSec;
    this->ConvEnergy = this->ConvPower * TimeStepSysSec;
    this->RadEnergy = this->RadPower * TimeStepSysSec;
}

} // namespace EnergyPlus::CoolingPanelSimple
