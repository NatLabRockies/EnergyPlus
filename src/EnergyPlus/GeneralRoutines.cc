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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BaseboardRadiator.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ExhaustAirSystemManager.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACSingleDuctInduc.hh>
#include <EnergyPlus/HWBaseboardRadiator.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarCollectors.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/SteamBaseboardRadiator.hh>
#include <EnergyPlus/UnitHeater.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VentilatedSlab.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

// Integer constants for different system types handled by the routines in this file
enum GeneralRoutinesEquipNums
{
    ParallelPIUReheatNum = 1,
    SeriesPIUReheatNum = 2,
    HeatingCoilWaterNum = 3,
    BBWaterConvOnlyNum = 4,
    BBSteamRadConvNum = 5,
    BBWaterRadConvNum = 6,
    FourPipeFanCoilNum = 7,
    OutdoorAirUnitNum = 8,
    UnitHeaterNum = 9,
    UnitVentilatorNum = 10,
    VentilatedSlabNum = 11
};

enum class AirLoopHVACCompType
{
    Invalid = -1,
    SupplyPlenum,
    ZoneSplitter,
    ZoneMixer,
    ReturnPlenum,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(AirLoopHVACCompType::Num)> AirLoopHVACCompTypeNamesUC{
    "AIRLOOPHVAC:SUPPLYPLENUM", "AIRLOOPHVAC:ZONESPLITTER", "AIRLOOPHVAC:ZONEMIXER", "AIRLOOPHVAC:RETURNPLENUM"};

void ControlCompOutput(EnergyPlusData &state,
                       std::string const &CompName,                      // the component Name
                       std::string const &CompType,                      // Type of component
                       int &CompNum,                                     // Index of component in component array
                       bool const FirstHVACIteration,                    // flag for 1st HVAV iteration in the time step
                       Real64 const QZnReq,                              // zone load to be met
                       int const ActuatedNode,                           // node that controls unit output
                       Real64 const MaxFlow,                             // maximum water flow
                       Real64 const MinFlow,                             // minimum water flow
                       Real64 const ControlOffset,                       // really the tolerance
                       int &ControlCompTypeNum,                          // Internal type num for CompType
                       int &CompErrIndex,                                // for Recurring error call
                       ObjexxFCL::Optional_int_const TempInNode,         // inlet node for output calculation
                       ObjexxFCL::Optional_int_const TempOutNode,        // outlet node for output calculation
                       ObjexxFCL::Optional<Real64 const> AirMassFlow,    // air mass flow rate
                       ObjexxFCL::Optional_int_const Action,             // 1=reverse; 2=normal
                       ObjexxFCL::Optional_int_const EquipIndex,         // Identifier for equipment of Outdoor Air Unit "ONLY"
                       PlantLocation const &plantLoc,                    // for plant components, Location
                       ObjexxFCL::Optional_int_const ControlledZoneIndex // controlled zone index for the zone containing the component
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   April 2000
    //       MODIFIED       Brent Griffith, Sept 2010 update plant interactions

    // PURPOSE OF THIS SUBROUTINE:
    // The purpose of this subroutine is to control the output of heating or cooling
    // meet the zone load.

    // METHODOLOGY EMPLOYED:
    // Currently this is using an interval halving scheme to a control tolerance

    // SUBROUTINE PARAMETER DEFINITIONS:
    // Iteration maximum for reheat control
    static int constexpr MaxIter = 25;
    static Real64 const iter_fac = 1.0 / std::pow(2, MaxIter - 3);
    int constexpr iReverseAction = 1;
    int constexpr iNormalAction = 2;

    // Note - order in routine must match order below
    //  Plus -- order in ListOfComponents array must be in sorted order.
    int constexpr NumComponents = 11;
    static Array1D_string const ListOfComponents(NumComponents,
                                                 {"AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT",
                                                  "AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT",
                                                  "COIL:HEATING:WATER",
                                                  "ZONEHVAC:BASEBOARD:CONVECTIVE:WATER",
                                                  "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:STEAM",
                                                  "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:WATER",
                                                  "ZONEHVAC:FOURPIPEFANCOIL",
                                                  "ZONEHVAC:OUTDOORAIRUNIT",
                                                  "ZONEHVAC:UNITHEATER",
                                                  "ZONEHVAC:UNITVENTILATOR",
                                                  "ZONEHVAC:VENTILATEDSLAB"});

    // DERIVED TYPE DEFINITIONS
    // Interval Half Type used for Controller

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int SimCompNum; // internal number for case statement

    // Object Data
    auto &ZoneInterHalf = state.dataGeneralRoutines->ZoneInterHalf;
    auto &ZoneController = state.dataGeneralRoutines->ZoneController;

    if (ControlCompTypeNum != 0) {
        SimCompNum = ControlCompTypeNum;
    } else {
        SimCompNum = Util::FindItemInSortedList(CompType, ListOfComponents, NumComponents);
        ControlCompTypeNum = SimCompNum;
    }

    int Iter = 0; // Iteration limit for the interval halving process
    bool Converged = false;
    bool WaterCoilAirFlowControl = false; // True if controlling air flow through water coil, water flow fixed
    Real64 LoadMet = 0.0;                 // Actual output of unit (watts)
    Real64 HalvingPrec = 0.0;             // precision of halving algorithm
    Real64 CpAir;

    // At the beginning of every time step the value is reset to the User Input
    ZoneController.SetPoint = 0.0;

    // Set to converged controller
    ZoneInterHalf.MaxFlowCalc = true;
    ZoneInterHalf.MinFlowCalc = false;
    ZoneInterHalf.NormFlowCalc = false;
    ZoneInterHalf.MinFlowResult = false;
    ZoneInterHalf.MaxResult = 1.0;
    ZoneInterHalf.MinResult = 0.0;

    // Start the Solution Iteration
    while (!Converged) {

        if (FirstHVACIteration) {
            state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMaxAvail = MaxFlow;
            state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMinAvail = MinFlow;
            // Check to make sure that the Minimum Flow rate is less than the max.
            if (MinFlow > MaxFlow) {
                ShowSevereError(state, format("ControlCompOutput:{}:{}, Min Control Flow is > Max Control Flow", CompType, CompName));
                ShowContinueError(
                    state, format("Acuated Node={} MinFlow=[{:.3T}], Max Flow={:.3T}", state.dataLoopNodes->NodeID(ActuatedNode), MinFlow, MaxFlow));
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, "Program terminates due to preceding condition.");
            }
        } // End of FirstHVACIteration Conditional If
        // The interface managers can reset the Max or Min to available values during the time step
        // and these will then be the new setpoint limits for the controller to work within.
        if ((SimCompNum == 3) && (!present(AirMassFlow))) {
            ZoneController.MaxSetPoint = state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMaxAvail;
            ZoneController.MinSetPoint = state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMinAvail;
        } else {
            ZoneController.MaxSetPoint =
                min(state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMaxAvail, state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMax);
            ZoneController.MinSetPoint =
                max(state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMinAvail, state.dataLoopNodes->Node(ActuatedNode).MassFlowRateMin);
        }
        // The first time through run at maximum flow rate and find results
        if (ZoneInterHalf.MaxFlowCalc) {
            ZoneController.CalculatedSetPoint = ZoneController.MaxSetPoint;
            ZoneInterHalf.MaxFlow = ZoneController.MaxSetPoint;
            ZoneInterHalf.MaxFlowCalc = false;
            ZoneInterHalf.MinFlowCalc = true;
            // Record the maximum flow rates and set the flow to the minimum and find results
        } else if (ZoneInterHalf.MinFlowCalc) {
            ZoneInterHalf.MaxResult = ZoneController.SensedValue;
            ZoneController.CalculatedSetPoint = ZoneController.MinSetPoint;
            ZoneInterHalf.MinFlow = ZoneController.MinSetPoint;
            ZoneInterHalf.MinFlowCalc = false;
            ZoneInterHalf.MinFlowResult = true;
            // Record the minimum results and set flow to half way between the max and min and find results
        } else if (ZoneInterHalf.MinFlowResult) {
            ZoneInterHalf.MinResult = ZoneController.SensedValue;
            HalvingPrec = (ZoneInterHalf.MaxResult - ZoneInterHalf.MinResult) * iter_fac;
            ZoneInterHalf.MidFlow = (ZoneInterHalf.MaxFlow + ZoneInterHalf.MinFlow) / 2.0;
            ZoneController.CalculatedSetPoint = (ZoneInterHalf.MaxFlow + ZoneInterHalf.MinFlow) / 2.0;
            ZoneInterHalf.MinFlowResult = false;
            ZoneInterHalf.NormFlowCalc = true;
            // Record the Mid results and check all possibilities and start interval halving procedure
        } else if (ZoneInterHalf.NormFlowCalc) {
            ZoneInterHalf.MidResult = ZoneController.SensedValue;

            // First check to see if the component is running; if not converge and return
            if (ZoneInterHalf.MaxResult == ZoneInterHalf.MinResult) {
                // Set to converged controller
                ZoneInterHalf.MaxFlowCalc = true;
                ZoneInterHalf.MinFlowCalc = false;
                ZoneInterHalf.NormFlowCalc = false;
                ZoneInterHalf.MinFlowResult = false;
                ZoneInterHalf.MaxResult = 1.0;
                ZoneInterHalf.MinResult = 0.0;
                if ((SimCompNum >= 4) && (SimCompNum <= 6)) { // hot water baseboards use min flow
                    ZoneController.CalculatedSetPoint = 0.0;  // CR7253
                } else {
                    ZoneController.CalculatedSetPoint = ZoneInterHalf.MaxFlow; // CR7253
                }
                // Set the Actuated node MassFlowRate with zero value
                if (plantLoc.loopNum) { // this is a plant component
                    PlantUtilities::SetActuatedBranchFlowRate(state,
                                                              ZoneController.CalculatedSetPoint,
                                                              ActuatedNode,
                                                              plantLoc,
                                                              false); // Autodesk:OPTIONAL LoopSide, BranchIndex used without PRESENT check
                } else {                                              // assume not a plant component
                    state.dataLoopNodes->Node(ActuatedNode).MassFlowRate = ZoneController.CalculatedSetPoint;
                }
                return;
            }

            // The next series of checks is to determine what interval the current solution is in
            //   comparison to the setpoint and then respond appropriately.

            // Normal controller assumes that MaxResult will be greater than MinResult. First check
            // to make sure that this is the case
            if (ZoneInterHalf.MaxResult <= ZoneInterHalf.MinResult) {
                if (WaterCoilAirFlowControl) {
                    ZoneController.CalculatedSetPoint = ZoneInterHalf.MaxFlow;
                } else {
                    ZoneController.CalculatedSetPoint = ZoneInterHalf.MinFlow;
                }
                // set to converged controller
                Converged = true;
                ZoneInterHalf.MaxFlowCalc = true;
                ZoneInterHalf.MinFlowCalc = false;
                ZoneInterHalf.NormFlowCalc = false;
                ZoneInterHalf.MinFlowResult = false;
                ZoneInterHalf.MaxResult = 1.0;
                ZoneInterHalf.MinResult = 0.0;
                // MaxResult is greater than MinResult so simulation control algorithm may proceed normally
            } else {
                // Now check to see if the setpoint is outside the endpoints of the control range
                // First check to see if the water is too cold and if so set to the minimum flow.
                if (ZoneController.SetPoint <= ZoneInterHalf.MinResult) {
                    ZoneController.CalculatedSetPoint = ZoneInterHalf.MinFlow;
                    // Set to Converged Controller
                    Converged = true;
                    ZoneInterHalf.MaxFlowCalc = true;
                    ZoneInterHalf.MinFlowCalc = false;
                    ZoneInterHalf.NormFlowCalc = false;
                    ZoneInterHalf.MinFlowResult = false;
                    ZoneInterHalf.MaxResult = 1.0;
                    ZoneInterHalf.MinResult = 0.0;
                    // Then check if too hot and if so set it to the maximum flow
                } else if (ZoneController.SetPoint >= ZoneInterHalf.MaxResult) {
                    ZoneController.CalculatedSetPoint = ZoneInterHalf.MaxFlow;
                    // Set to Converged Controller
                    Converged = true;
                    ZoneInterHalf.MaxFlowCalc = true;
                    ZoneInterHalf.MinFlowCalc = false;
                    ZoneInterHalf.NormFlowCalc = false;
                    ZoneInterHalf.MinFlowResult = false;
                    ZoneInterHalf.MaxResult = 1.0;
                    ZoneInterHalf.MinResult = 0.0;
                    // If between the max and mid set to new flow and raise min to mid
                } else if (ZoneController.SetPoint >= ZoneInterHalf.MidResult) {
                    ZoneController.CalculatedSetPoint = (ZoneInterHalf.MaxFlow + ZoneInterHalf.MidFlow) / 2.0;
                    ZoneInterHalf.MinFlow = ZoneInterHalf.MidFlow;
                    ZoneInterHalf.MinResult = ZoneInterHalf.MidResult;
                    ZoneInterHalf.MidFlow = (ZoneInterHalf.MaxFlow + ZoneInterHalf.MidFlow) / 2.0;
                    // If between the min and mid set to new flow and lower Max to mid
                } else {
                    ZoneController.CalculatedSetPoint = (ZoneInterHalf.MinFlow + ZoneInterHalf.MidFlow) / 2.0;
                    ZoneInterHalf.MaxFlow = ZoneInterHalf.MidFlow;
                    ZoneInterHalf.MaxResult = ZoneInterHalf.MidResult;
                    ZoneInterHalf.MidFlow = (ZoneInterHalf.MinFlow + ZoneInterHalf.MidFlow) / 2.0;

                } // End of the Conditional for the actual interval halving scheme itself
            } // end of max > min check

        } // End of the Conditinal for the first 3 iterations for the interval halving

        // Make sure that the Calculated setpoint falls between the minimum and maximum allowed
        if (ZoneController.CalculatedSetPoint > ZoneController.MaxSetPoint) {
            ZoneController.CalculatedSetPoint = ZoneController.MaxSetPoint;
            Converged = true;
            ZoneInterHalf.MaxFlowCalc = true;
            ZoneInterHalf.MinFlowCalc = false;
            ZoneInterHalf.NormFlowCalc = false;
            ZoneInterHalf.MinFlowResult = false;
            ZoneInterHalf.MaxResult = 1.0;
            ZoneInterHalf.MinResult = 0.0;
        } else if (ZoneController.CalculatedSetPoint < ZoneController.MinSetPoint) {
            ZoneController.CalculatedSetPoint = ZoneController.MinSetPoint;
            Converged = true;
            ZoneInterHalf.MaxFlowCalc = true;
            ZoneInterHalf.MinFlowCalc = false;
            ZoneInterHalf.NormFlowCalc = false;
            ZoneInterHalf.MinFlowResult = false;
            ZoneInterHalf.MaxResult = 1.0;
            ZoneInterHalf.MinResult = 0.0;
        }

        // check if hunting down around the limit of a significant mass flow in systems.
        if ((Iter > MaxIter / 2) && (ZoneController.CalculatedSetPoint < DataBranchAirLoopPlant::MassFlowTolerance)) {
            ZoneController.CalculatedSetPoint = ZoneController.MinSetPoint;
            Converged = true;
            ZoneInterHalf.MaxFlowCalc = true;
            ZoneInterHalf.MinFlowCalc = false;
            ZoneInterHalf.NormFlowCalc = false;
            ZoneInterHalf.MinFlowResult = false;
            ZoneInterHalf.MaxResult = 1.0;
            ZoneInterHalf.MinResult = 0.0;
        }

        // Set the Actuated node MassFlowRate with the new value
        if (plantLoc.loopNum) { // this is a plant component
            PlantUtilities::SetActuatedBranchFlowRate(state,
                                                      ZoneController.CalculatedSetPoint,
                                                      ActuatedNode,
                                                      plantLoc,
                                                      false); // Autodesk:OPTIONAL LoopSide, BranchIndex used without PRESENT check
        } else {                                              // assume not a plant component, leave alone
            state.dataLoopNodes->Node(ActuatedNode).MassFlowRate = ZoneController.CalculatedSetPoint;
        }

        // The denominator of the control signal should be no less than 100 watts
        Real64 Denom = sign(max(std::abs(QZnReq), 100.0), QZnReq);
        if (present(Action)) {
            if (Action == iNormalAction) {
                Denom = max(std::abs(QZnReq), 100.0);
            } else if (Action == iReverseAction) {
                Denom = -max(std::abs(QZnReq), 100.0);
            } else {
                ShowFatalError(state, format("ControlCompOutput: Illegal Action argument =[{}]", Action));
            }
        }

        switch (SimCompNum) {      // Tuned If block changed to switch
        case ParallelPIUReheatNum: // 'AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT'
            // simulate series piu reheat coil
            WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompNum);
            // Calculate the control signal (the variable we are forcing to zero)
            CpAir = Psychrometrics::PsyCpAirFnW(
                state.dataLoopNodes->Node(TempOutNode).HumRat); // Autodesk:OPTIONAL TempInNode, TempOutNode used without PRESENT check
            LoadMet = CpAir * state.dataLoopNodes->Node(TempOutNode).MassFlowRate *
                      (state.dataLoopNodes->Node(TempOutNode).Temp -
                       state.dataLoopNodes->Node(TempInNode).Temp); // Autodesk:OPTIONAL TempInNode, TempOutNode used without PRESENT check
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case SeriesPIUReheatNum: // 'AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT'
            // simulate series piu reheat coil
            WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompNum);
            // Calculate the control signal (the variable we are forcing to zero)
            CpAir = Psychrometrics::PsyCpAirFnW(
                state.dataLoopNodes->Node(TempOutNode).HumRat); // Autodesk:OPTIONAL TempInNode, TempOutNode used without PRESENT check
            LoadMet = CpAir * state.dataLoopNodes->Node(TempOutNode).MassFlowRate *
                      (state.dataLoopNodes->Node(TempOutNode).Temp -
                       state.dataLoopNodes->Node(TempInNode).Temp); // Autodesk:OPTIONAL TempInNode, TempOutNode used without PRESENT check
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case HeatingCoilWaterNum: // 'COIL:HEATING:WATER'
            // Simulate reheat coil for the VAV system
            WaterCoils::SimulateWaterCoilComponents(state, CompName, FirstHVACIteration, CompNum);
            // Calculate the control signal (the variable we are forcing to zero)
            CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(TempOutNode).HumRat);
            if (present(AirMassFlow)) {
                LoadMet = AirMassFlow * CpAir * state.dataLoopNodes->Node(TempOutNode).Temp;
                ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            } else {
                WaterCoilAirFlowControl = true;
                LoadMet = state.dataLoopNodes->Node(TempOutNode).MassFlowRate * CpAir *
                          (state.dataLoopNodes->Node(TempOutNode).Temp -
                           state.dataLoopNodes->Node(TempInNode).Temp); // Autodesk:OPTIONAL TempInNode, TempOutNode used without PRESENT check
                ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            }
            break;

        case BBWaterConvOnlyNum: // 'ZONEHVAC:BASEBOARD:CONVECTIVE:WATER'
            // Simulate baseboard
            BaseboardRadiator::SimHWConvective(state, CompNum, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case BBSteamRadConvNum: // 'ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:STEAM'
            // Simulate baseboard
            SteamBaseboardRadiator::CalcSteamBaseboard(state, CompNum, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case BBWaterRadConvNum: // 'ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:WATER'
            // Simulate baseboard
            HWBaseboardRadiator::CalcHWBaseboard(state, CompNum, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case FourPipeFanCoilNum: // 'ZONEHVAC:FOURPIPEFANCOIL'
            // Simulate fancoil unit
            FanCoilUnits::Calc4PipeFanCoil(state, CompNum, ControlledZoneIndex, FirstHVACIteration, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case OutdoorAirUnitNum: //'ZONEHVAC:OUTDOORAIRUNIT'
            // Simulate outdoor air unit components
            OutdoorAirUnit::CalcOAUnitCoilComps(
                state, CompNum, FirstHVACIteration, EquipIndex, LoadMet); // Autodesk:OPTIONAL EquipIndex used without PRESENT check
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case UnitHeaterNum: // 'ZONEHVAC:UNITHEATER'
            // Simulate unit heater components
            UnitHeater::CalcUnitHeaterComponents(state, CompNum, FirstHVACIteration, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case UnitVentilatorNum: // 'ZONEHVAC:UNITVENTILATOR'
            // Simulate unit ventilator components
            UnitVentilator::CalcUnitVentilatorComponents(state, CompNum, FirstHVACIteration, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        case VentilatedSlabNum: // 'ZONEHVAC:VENTILATEDSLAB'
            // Simulate unit ventilator components
            VentilatedSlab::CalcVentilatedSlabComps(state, CompNum, FirstHVACIteration, LoadMet);
            // Calculate the control signal (the variable we are forcing to zero)
            ZoneController.SensedValue = (LoadMet - QZnReq) / Denom;
            break;

        default:
            ShowFatalError(state, format("ControlCompOutput: Illegal Component Number argument =[{}]", SimCompNum));
            break;
        }

        // Check for Controller convergence to see if within the offset
        if (std::abs(ZoneController.SensedValue) <= ControlOffset || std::abs(ZoneController.SensedValue) <= HalvingPrec) {
            // Set to converged controller
            ZoneInterHalf.MaxFlowCalc = true;
            ZoneInterHalf.MinFlowCalc = false;
            ZoneInterHalf.NormFlowCalc = false;
            ZoneInterHalf.MinFlowResult = false;
            ZoneInterHalf.MaxResult = 1.0;
            ZoneInterHalf.MinResult = 0.0;
            break;
        }
        if (!Converged) {
            bool BBConvergeCheckFlag = BBConvergeCheck(SimCompNum, ZoneInterHalf.MaxFlow, ZoneInterHalf.MinFlow);
            if (BBConvergeCheckFlag) {
                // Set to converged controller
                ZoneInterHalf.MaxFlowCalc = true;
                ZoneInterHalf.MinFlowCalc = false;
                ZoneInterHalf.NormFlowCalc = false;
                ZoneInterHalf.MinFlowResult = false;
                ZoneInterHalf.MaxResult = 1.0;
                ZoneInterHalf.MinResult = 0.0;
                break;
            }
        }

        ++Iter;
        if ((Iter > MaxIter) && (!state.dataGlobal->WarmupFlag)) {
            // if ( CompErrIndex == 0 ) {
            ShowWarningMessage(state, format("ControlCompOutput: Maximum iterations exceeded for {} = {}", CompType, CompName));
            ShowContinueError(state, format("... Load met       = {:.5T} W.", LoadMet));
            ShowContinueError(state, format("... Load requested = {:.5T} W.", QZnReq));
            ShowContinueError(state, format("... Error          = {:.8T} %.", std::abs((LoadMet - QZnReq) * 100.0 / Denom)));
            ShowContinueError(state, format("... Tolerance      = {:.8T} %.", ControlOffset * 100.0));
            ShowContinueError(state, "... Error          = (Load met - Load requested) / MAXIMUM(Load requested, 100)");
            ShowContinueError(state, format("... Actuated Node Mass Flow Rate ={:.9R} kg/s", state.dataLoopNodes->Node(ActuatedNode).MassFlowRate));
            ShowContinueErrorTimeStamp(state, "");
            ShowRecurringWarningErrorAtEnd(state,
                                           "ControlCompOutput: Maximum iterations error for " + CompType + " = " + CompName,
                                           CompErrIndex,
                                           std::abs((LoadMet - QZnReq) * 100.0 / Denom),
                                           std::abs((LoadMet - QZnReq) * 100.0 / Denom),
                                           _,
                                           "%",
                                           "%");
            //}
            ShowRecurringWarningErrorAtEnd(state,
                                           "ControlCompOutput: Maximum iterations error for " + CompType + " = " + CompName,
                                           CompErrIndex,
                                           std::abs((LoadMet - QZnReq) * 100.0 / Denom),
                                           std::abs((LoadMet - QZnReq) * 100.0 / Denom),
                                           _,
                                           "%",
                                           "%");
            break; // It will not converge this time
        } else if (Iter > MaxIter * 2) {
            break;
        }

    } // End of the Convergence Iteration
}

bool BBConvergeCheck(int const SimCompNum, Real64 const MaxFlow, Real64 const MinFlow)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   November 2017

    // PURPOSE OF THIS SUBROUTINE:
    // This is an additional check for the radiant/convective baseboard units
    // to see if they are converged or the flow is sufficiently converged to
    // procede with the simulation.  With the radiant component to these systems,
    // the impact on the load met is more difficult to calculate and the impact
    // on the actual system output is not as well behaved as for convective
    // systems.  This additional check avoids excessive iterations and max
    // iteration warnings and provides sufficiently converged results.  It is
    // only called from ControlCompOutput.

    // Return Value
    bool BBConvergeCheck;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static Real64 constexpr BBIterLimit = 0.00001;

    if (SimCompNum != BBSteamRadConvNum && SimCompNum != BBWaterRadConvNum) {
        // For all zone equipment except radiant/convective baseboard (steam and water) units:
        BBConvergeCheck = false;
    } else {
        // For steam and water radiant/convective baseboard units:
        if ((MaxFlow - MinFlow) > BBIterLimit) {
            BBConvergeCheck = false;
        } else {
            BBConvergeCheck = true;
        }
    }

    return BBConvergeCheck;
}

void CheckSysSizing(EnergyPlusData &state,
                    std::string_view const CompType, // Component Type (e.g. Chiller:Electric)
                    std::string const &CompName      // Component Name (e.g. Big Chiller)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   October 2002

    // PURPOSE OF THIS SUBROUTINE:
    // This routine is called when an "autosize" input is encountered in a component
    // sizing routine to check that the system sizing calculations have been done.

    // METHODOLOGY EMPLOYED:
    // Checks SysSizingRunDone flag. If false throws a fatal error.

    if (!state.dataSize->SysSizingRunDone) {
        ShowSevereError(state, format("For autosizing of {} {}, a system sizing run must be done.", CompType, CompName));
        if (state.dataSize->NumSysSizInput == 0) {
            ShowContinueError(state, "No \"Sizing:System\" objects were entered.");
        }
        if (!state.dataGlobal->DoSystemSizing) {
            ShowContinueError(state, R"(The "SimulationControl" object did not have the field "Do System Sizing Calculation" set to Yes.)");
        }
        ShowFatalError(state, "Program terminates due to previously shown condition(s).");
    }
}

void CheckThisAirSystemForSizing(EnergyPlusData &state, int const AirLoopNum, bool &AirLoopWasSized)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   October 2013

    AirLoopWasSized = false;
    if (state.dataSize->SysSizingRunDone) {
        for (int ThisAirSysSizineInputLoop = 1; ThisAirSysSizineInputLoop <= state.dataSize->NumSysSizInput; ++ThisAirSysSizineInputLoop) {
            if (state.dataSize->SysSizInput(ThisAirSysSizineInputLoop).AirLoopNum == AirLoopNum) {
                AirLoopWasSized = true;
                break;
            }
        }
    }
}

void CheckZoneSizing(EnergyPlusData &state,
                     std::string_view const CompType, // Component Type (e.g. Chiller:Electric)
                     std::string_view const CompName  // Component Name (e.g. Big Chiller)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   October 2002

    // PURPOSE OF THIS SUBROUTINE:
    // This routine is called when an "autosize" input is encountered in a component
    // sizing routine to check that the zone sizing calculations have been done.

    // METHODOLOGY EMPLOYED:
    // Checks ZoneSizingRunDone flag. If false throws a fatal error.

    if (!state.dataSize->ZoneSizingRunDone) {
        ShowSevereError(state, format("For autosizing of {} {}, a zone sizing run must be done.", CompType, CompName));
        if (state.dataSize->NumZoneSizingInput == 0) {
            ShowContinueError(state, "No \"Sizing:Zone\" objects were entered.");
        }
        if (!state.dataGlobal->DoZoneSizing) {
            ShowContinueError(state, R"(The "SimulationControl" object did not have the field "Do Zone Sizing Calculation" set to Yes.)");
        }
        ShowFatalError(state, "Program terminates due to previously shown condition(s).");
    }
}

void CheckThisZoneForSizing(EnergyPlusData &state,
                            int const ZoneNum, // zone index to be checked
                            bool &ZoneWasSized)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Oct 2013

    // PURPOSE OF THIS SUBROUTINE:
    // utility routine to see if a particular zone has a Sizing:Zone object for it
    // and that sizing was done.

    ZoneWasSized = false;
    if (state.dataSize->ZoneSizingRunDone) {
        for (int ThisSizingInput = 1; ThisSizingInput <= state.dataSize->NumZoneSizingInput; ++ThisSizingInput) {
            if (state.dataSize->ZoneSizingInput(ThisSizingInput).ZoneNum == ZoneNum) {
                ZoneWasSized = true;
                break;
            }
        }
    }
}

void ValidateComponent(EnergyPlusData &state,
                       std::string_view CompType,   // Component Type (e.g. Chiller:Electric)
                       std::string const &CompName, // Component Name (e.g. Big Chiller)
                       bool &IsNotOK,               // .TRUE. if this component pair is invalid
                       std::string_view CallString  // Context of this pair -- for error message
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2002

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine can be called to validate the component type-name pairs that
    // are so much a part of the EnergyPlus input.  The main drawback to this validation
    // has been that the "GetInput" routine may not have been called and/or exists in
    // another module from the one with the list.  This means that validation must be
    // done later, perhaps after simulation has already started or perhaps raises an
    // array bound error instead.

    // METHODOLOGY EMPLOYED:
    // Uses existing routines in InputProcessor.  GetObjectItemNum uses the "standard"
    // convention of the Name of the item/object being the first Alpha Argument.

    IsNotOK = false;
    if (CompType == "HEATPUMP:AIRTOWATER:COOLING" || CompType == "HEATPUMP:AIRTOWATER:HEATING") {
        CompType = "HEATPUMP:AIRTOWATER";
    }
    int ItemNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, std::string{CompType}, CompName);

    if (ItemNum < 0) {
        ShowSevereError(state, format("During {} Input, Invalid Component Type input={}", CallString, CompType));
        ShowContinueError(state, format("Component name={}", CompName));
        IsNotOK = true;
    } else if (ItemNum == 0) {
        ShowSevereError(state, format("During {} Input, Invalid Component Name input={}", CallString, CompName));
        ShowContinueError(state, format("Component type={}", CompType));
        IsNotOK = true;
    }
}

void ValidateComponent(EnergyPlusData &state,
                       std::string_view CompType,      // Component Type (e.g. Chiller:Electric)
                       std::string const &CompValType, // Component "name" field type
                       std::string const &CompName,    // Component Name (e.g. Big Chiller)
                       bool &IsNotOK,                  // .TRUE. if this component pair is invalid
                       std::string_view CallString     // Context of this pair -- for error message
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2002

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine can be called to validate the component type-name pairs that
    // are so much a part of the EnergyPlus input.  The main drawback to this validation
    // has been that the "GetInput" routine may not have been called and/or exists in
    // another module from the one with the list.  This means that validation must be
    // done later, perhaps after simulation has already started or perhaps raises an
    // array bound error instead.

    // METHODOLOGY EMPLOYED:
    // Uses existing routines in InputProcessor.  GetObjectItemNum uses the "standard"
    // convention of the Name of the item/object being the first Alpha Argument.

    IsNotOK = false;

    int ItemNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, CompType, CompValType, CompName);

    if (ItemNum < 0) {
        ShowSevereError(state, format("During {} Input, Invalid Component Type input={}", CallString, CompType));
        ShowContinueError(state, format("Component name={}", CompName));
        IsNotOK = true;
    } else if (ItemNum == 0) {
        ShowSevereError(state, format("During {} Input, Invalid Component Name input={}", CallString, CompName));
        ShowContinueError(state, format("Component type={}", CompType));
        IsNotOK = true;
    }
}

void CalcBasinHeaterPower(EnergyPlusData &state,
                          Real64 const Capacity,     // Basin heater capacity per degree C below setpoint (W/C)
                          Sched::Schedule *sched,    // basin heater schedule
                          Real64 const SetPointTemp, // setpoint temperature for basin heater operation (C)
                          Real64 &Power              // Basin heater power (W)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Chandan Sharma, FSEC
    //       DATE WRITTEN   Feb 2010

    // PURPOSE OF THIS SUBROUTINE:
    // To calculate basin heater power when the evaporative cooled equipment is not operating
    // and outdoor air dry-bulb temperature is below the set-point

    // METHODOLOGY EMPLOYED:
    // Checks to see whether schedule for basin heater exists or not. If the schedule exists,
    // the basin heater is operated for the schedule specified otherwise the heater runs
    // for the entire simulation timestep whenever the outdoor temperature is below setpoint
    // and water is not flowing through the evaporative cooled equipment.

    Power = 0.0;
    // Operate basin heater anytime outdoor temperature is below setpoint and water is not flowing through the equipment
    // IF schedule exists, basin heater performance can be scheduled OFF
    if (sched != nullptr) {
        Real64 BasinHeaterSch = sched->getCurrentVal();
        if (Capacity > 0.0 && BasinHeaterSch > 0.0) {
            Power = max(0.0, Capacity * (SetPointTemp - state.dataEnvrn->OutDryBulbTemp));
        }
    } else {
        // IF schedule does not exist, basin heater operates anytime outdoor dry-bulb temp is below setpoint
        if (Capacity > 0.0) {
            Power = max(0.0, Capacity * (SetPointTemp - state.dataEnvrn->OutDryBulbTemp));
        }
    }
}

void TestAirPathIntegrity(EnergyPlusData &state, bool &ErrFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine tests supply, return and overall air path integrity.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool errFlag;
    Array2D_int ValRetAPaths;
    Array2D_int NumRAPNodes;
    Array2D_int ValSupAPaths;
    Array2D_int NumSAPNodes;

    NumSAPNodes.allocate(state.dataLoopNodes->NumOfNodes, state.dataHVACGlobal->NumPrimaryAirSys);
    NumRAPNodes.allocate(state.dataLoopNodes->NumOfNodes, state.dataHVACGlobal->NumPrimaryAirSys);
    ValRetAPaths.allocate(state.dataLoopNodes->NumOfNodes, state.dataHVACGlobal->NumPrimaryAirSys);
    ValSupAPaths.allocate(state.dataLoopNodes->NumOfNodes, state.dataHVACGlobal->NumPrimaryAirSys);
    NumSAPNodes = 0;
    NumRAPNodes = 0;
    ValRetAPaths = 0;
    ValSupAPaths = 0;

    TestSupplyAirPathIntegrity(state, errFlag);
    if (errFlag) {
        ErrFound = true;
    }
    TestReturnAirPathIntegrity(state, errFlag, ValRetAPaths);
    if (errFlag) {
        ErrFound = true;
    }

    // Final tests, look for duplicate nodes
    for (int Loop = 1; Loop <= state.dataHVACGlobal->NumPrimaryAirSys; ++Loop) {
        if (ValRetAPaths(1, Loop) != 0) {
            continue;
        }
        if (state.dataAirLoop->AirToZoneNodeInfo(Loop).NumReturnNodes <= 0) {
            continue;
        }
        ValRetAPaths(1, Loop) = state.dataAirLoop->AirToZoneNodeInfo(Loop).ZoneEquipReturnNodeNum(1);
    }

    for (int Loop = 1; Loop <= state.dataHVACGlobal->NumPrimaryAirSys; ++Loop) {
        for (int Loop1 = 1; Loop1 <= state.dataLoopNodes->NumOfNodes; ++Loop1) {
            int TestNode = ValRetAPaths(Loop1, Loop);
            int Count = 0;
            for (int Loop2 = 1; Loop2 <= state.dataHVACGlobal->NumPrimaryAirSys; ++Loop2) {
                for (int Loop3 = 1; Loop3 <= state.dataLoopNodes->NumOfNodes; ++Loop3) {
                    if (Loop2 == Loop && Loop1 == Loop3) {
                        continue; // Don't count test node
                    }
                    if (ValRetAPaths(Loop3, Loop2) == 0) {
                        break;
                    }
                    if (ValRetAPaths(Loop3, Loop2) == TestNode) {
                        ++Count;
                    }
                }
            }
            if (Count > 0) {
                ShowSevereError(state, "Duplicate Node detected in Return Air Paths");
                ShowContinueError(state, format("Test Node={}", state.dataLoopNodes->NodeID(TestNode)));
                ShowContinueError(state, format("In Air Path={}", state.dataAirLoop->AirToZoneNodeInfo(Loop).AirLoopName));
                ErrFound = true;
            }
        }
    }

    NumSAPNodes.deallocate();
    NumRAPNodes.deallocate();
    ValRetAPaths.deallocate();
    ValSupAPaths.deallocate();
}

void TestSupplyAirPathIntegrity(EnergyPlusData &state, bool &ErrFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine tests supply air path integrity and displays the loop for each branch.
    // Also, input and output nodes.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string PrimaryAirLoopName; // Air Loop to which this supply air path is connected
    Array1D_bool FoundSupplyPlenum;
    Array1D_bool FoundZoneSplitter;
    Array1D_string FoundNames;
    int NumErr = 0; // Error Counter //Autodesk:Init Initialization added

    // Do by Paths
    ShowMessage(state, "Testing Individual Supply Air Path Integrity");
    ErrFound = false;

    print(state.files.bnd, "{}\n", "! ===============================================================");
    static constexpr std::string_view Format_700("! <#Supply Air Paths>,<Number of Supply Air Paths>");
    print(state.files.bnd, "{}\n", Format_700);
    print(state.files.bnd, " #Supply Air Paths,{}\n", state.dataZoneEquip->NumSupplyAirPaths);
    static constexpr std::string_view Format_702("! <Supply Air Path>,<Supply Air Path Count>,<Supply Air Path Name>,<AirLoopHVAC Name>");
    print(state.files.bnd, "{}\n", Format_702);
    static constexpr std::string_view Format_703("! <#Components on Supply Air Path>,<Number of Components>");
    print(state.files.bnd, "{}\n", Format_703);
    static constexpr std::string_view Format_704(
        "! <Supply Air Path Component>,<Component Count>,<Component Type>,<Component Name>,<AirLoopHVAC Name>");
    print(state.files.bnd, "{}\n", Format_704);
    static constexpr std::string_view Format_707("! <#Outlet Nodes on Supply Air Path Component>,<Number of Nodes>");
    print(state.files.bnd, "{}\n", Format_707);
    static constexpr std::string_view Format_708(
        "! <Supply Air Path Component Nodes>,<Node Count>,<Component Type>,<Component Name>,<Inlet Node Name>,<Outlet "
        "Node Name>,<AirLoopHVAC Name>");
    print(state.files.bnd, "{}\n", Format_708);

    for (int BCount = 1; BCount <= state.dataZoneEquip->NumSupplyAirPaths; ++BCount) {

        // Determine which air loop this supply air path is connected to
        int Found = 0;
        for (int Count1 = 1; Count1 <= state.dataHVACGlobal->NumPrimaryAirSys; ++Count1) {
            PrimaryAirLoopName = state.dataAirLoop->AirToZoneNodeInfo(Count1).AirLoopName;
            Found = 0;
            for (int Count2 = 1; Count2 <= state.dataAirLoop->AirToZoneNodeInfo(Count1).NumSupplyNodes; ++Count2) {
                if (state.dataZoneEquip->SupplyAirPath(BCount).InletNodeNum ==
                    state.dataAirLoop->AirToZoneNodeInfo(Count1).ZoneEquipSupplyNodeNum(Count2)) {
                    Found = Count2;
                }
            }
            if (Found != 0) {
                break;
            }
        }
        if (Found == 0) {
            PrimaryAirLoopName = "**Unknown**";
        }

        print(state.files.bnd, " Supply Air Path,{},{},{}\n", BCount, state.dataZoneEquip->SupplyAirPath(BCount).Name, PrimaryAirLoopName);
        print(state.files.bnd, "   #Components on Supply Air Path,{}\n", state.dataZoneEquip->SupplyAirPath(BCount).NumOfComponents);

        std::string AirPathNodeName = state.dataLoopNodes->NodeID(state.dataZoneEquip->SupplyAirPath(BCount).InletNodeNum);

        for (int Count = 1; Count <= state.dataZoneEquip->SupplyAirPath(BCount).NumOfComponents; ++Count) {

            print(state.files.bnd,
                  "   Supply Air Path Component,{},{},{},{}\n",
                  Count,
                  state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count),
                  state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count),
                  PrimaryAirLoopName);

            AirLoopHVACCompType CompType = static_cast<AirLoopHVACCompType>(
                getEnumValue(AirLoopHVACCompTypeNamesUC, Util::makeUPPER(state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count))));

            switch (CompType) {
            case AirLoopHVACCompType::SupplyPlenum: {
                for (int Count2 = 1; Count2 <= state.dataZonePlenum->NumZoneSupplyPlenums; ++Count2) {
                    if (state.dataZonePlenum->ZoneSupPlenCond(Count2).ZonePlenumName !=
                        state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count)) {
                        continue;
                    }
                    if (Count == 1 && AirPathNodeName != state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneSupPlenCond(Count2).InletNode)) {
                        ShowSevereError(state, format("Error in AirLoopHVAC:SupplyPath={}", state.dataZoneEquip->SupplyAirPath(BCount).Name));
                        ShowContinueError(state,
                                          format("For AirLoopHVAC:SupplyPlenum={}", state.dataZonePlenum->ZoneSupPlenCond(Count2).ZonePlenumName));
                        ShowContinueError(state, format("Expected inlet node (supply air path)={}", AirPathNodeName));
                        ShowContinueError(state,
                                          format("Encountered node name (supply plenum)={}",
                                                 state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneSupPlenCond(Count2).OutletNode(1))));
                        ErrFound = true;
                        ++NumErr;
                    }
                    print(state.files.bnd,
                          "     #Outlet Nodes on Supply Air Path Component,{}\n",
                          state.dataZonePlenum->ZoneSupPlenCond(Count2).NumOutletNodes);
                    for (int Count1 = 1; Count1 <= state.dataZonePlenum->ZoneSupPlenCond(Count2).NumOutletNodes; ++Count1) {
                        print(state.files.bnd,
                              "     Supply Air Path Component Nodes,{},{},{},{},{},{}\n",
                              Count1,
                              state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count),
                              state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count),
                              state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneSupPlenCond(Count2).InletNode),
                              state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneSupPlenCond(Count2).OutletNode(Count1)),
                              PrimaryAirLoopName);
                    }
                }
            } break;
            case AirLoopHVACCompType::ZoneSplitter: {
                for (int Count2 = 1; Count2 <= state.dataSplitterComponent->NumSplitters; ++Count2) {
                    if (state.dataSplitterComponent->SplitterCond(Count2).SplitterName !=
                        state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count)) {
                        continue;
                    }
                    if (Count == 1 && AirPathNodeName != state.dataLoopNodes->NodeID(state.dataSplitterComponent->SplitterCond(Count2).InletNode)) {
                        ShowSevereError(state, format("Error in AirLoopHVAC:SupplyPath={}", state.dataZoneEquip->SupplyAirPath(BCount).Name));
                        ShowContinueError(state,
                                          format("For AirLoopHVAC:ZoneSplitter={}", state.dataSplitterComponent->SplitterCond(Count2).SplitterName));
                        ShowContinueError(state, format("Expected inlet node (supply air path)={}", AirPathNodeName));
                        ShowContinueError(state,
                                          format("Encountered node name (zone splitter)={}",
                                                 state.dataLoopNodes->NodeID(state.dataSplitterComponent->SplitterCond(Count2).InletNode)));
                        ErrFound = true;
                        ++NumErr;
                    }
                    print(state.files.bnd,
                          "     #Outlet Nodes on Supply Air Path Component,{}\n",
                          state.dataSplitterComponent->SplitterCond(Count2).NumOutletNodes);
                    for (int Count1 = 1; Count1 <= state.dataSplitterComponent->SplitterCond(Count2).NumOutletNodes; ++Count1) {
                        print(state.files.bnd,
                              "     Supply Air Path Component Nodes,{},{},{},{},{},{}\n",
                              Count1,
                              state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count),
                              state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count),
                              state.dataLoopNodes->NodeID(state.dataSplitterComponent->SplitterCond(Count2).InletNode),
                              state.dataLoopNodes->NodeID(state.dataSplitterComponent->SplitterCond(Count2).OutletNode(Count1)),
                              PrimaryAirLoopName);
                    }
                }
            } break;
            default: {
                ShowSevereError(
                    state, format("Invalid Component Type in Supply Air Path={}", state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count)));
                ErrFound = true;
                ++NumErr;
            } break;
            }
        }

        if (state.dataZoneEquip->SupplyAirPath(BCount).NumNodes > 0) {
            static constexpr std::string_view Format_705("! <#Nodes on Supply Air Path>,<Number of Nodes>");
            print(state.files.bnd, "{}\n", Format_705);
            static constexpr std::string_view Format_706("! <Supply Air Path Node>,<Node Type>,<Node Count>,<Node Name>,<AirLoopHVAC Name>");
            print(state.files.bnd, "{}\n", Format_706);
            print(state.files.bnd, "#Nodes on Supply Air Path,{}\n", state.dataZoneEquip->SupplyAirPath(BCount).NumNodes);
            for (int Count2 = 1; Count2 <= state.dataZoneEquip->SupplyAirPath(BCount).NumNodes; ++Count2) {
                if (state.dataZoneEquip->SupplyAirPath(BCount).NodeType(Count2) == DataZoneEquipment::AirNodeType::PathInlet) {
                    print(state.files.bnd,
                          "   Supply Air Path Node,Inlet Node,{},{},{}\n",
                          Count2,
                          state.dataLoopNodes->NodeID(state.dataZoneEquip->SupplyAirPath(BCount).Node(Count2)),
                          PrimaryAirLoopName);
                } else if (state.dataZoneEquip->SupplyAirPath(BCount).NodeType(Count2) == DataZoneEquipment::AirNodeType::Intermediate) {
                    print(state.files.bnd,
                          "   Supply Air Path Node,Through Node,{},{},{}\n",
                          Count2,
                          state.dataLoopNodes->NodeID(state.dataZoneEquip->SupplyAirPath(BCount).Node(Count2)),
                          PrimaryAirLoopName);
                } else if (state.dataZoneEquip->SupplyAirPath(BCount).NodeType(Count2) == DataZoneEquipment::AirNodeType::Outlet) {
                    print(state.files.bnd,
                          "   Supply Air Path Node,Outlet Node,{},{},{}\n",
                          Count2,
                          state.dataLoopNodes->NodeID(state.dataZoneEquip->SupplyAirPath(BCount).Node(Count2)),
                          PrimaryAirLoopName);
                }
            }
        }
    }

    if (state.dataSplitterComponent->NumSplitters == 0) {
        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ZoneSplitter") > 0) {
            SplitterComponent::GetSplitterInput(state);
        }
    }
    if (state.dataZonePlenum->NumZoneSupplyPlenums == 0 && state.dataZonePlenum->NumZoneReturnPlenums == 0) {
        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:SupplyPlenum") > 0) {
            ZonePlenum::GetZonePlenumInput(state);
        }
    }

    // now the reverse.  is every zone splitter and supply plenum on supply air path
    FoundSupplyPlenum.dimension(state.dataZonePlenum->NumZoneSupplyPlenums, false);
    FoundZoneSplitter.dimension(state.dataSplitterComponent->NumSplitters, false);
    FoundNames.allocate(state.dataZonePlenum->NumZoneSupplyPlenums);
    for (int Count1 = 1; Count1 <= state.dataZonePlenum->NumZoneSupplyPlenums; ++Count1) {
        for (int BCount = 1; BCount <= state.dataZoneEquip->NumSupplyAirPaths; ++BCount) {
            for (int Count = 1; Count <= state.dataZoneEquip->SupplyAirPath(BCount).NumOfComponents; ++Count) {
                if (state.dataZonePlenum->ZoneSupPlenCond(Count1).ZonePlenumName != state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count) ||
                    state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count) != "AIRLOOPHVAC:SUPPLYPLENUM") {
                    continue;
                }
                if (FoundSupplyPlenum(Count1)) {
                    ShowSevereError(
                        state,
                        format("AirLoopHVAC:SupplyPlenum=\"{}\", duplicate entry.", state.dataZonePlenum->ZoneSupPlenCond(Count1).ZonePlenumName));
                    ShowContinueError(state, format("already exists on AirLoopHVAC:SupplyPath=\"{}\".", FoundNames(Count1)));
                    ErrFound = true;
                } else {
                    // record use
                    FoundSupplyPlenum(Count1) = true;
                    FoundNames(Count1) = state.dataZoneEquip->SupplyAirPath(BCount).Name;
                }
            }
        }
    }
    FoundNames.deallocate();
    FoundNames.allocate(state.dataSplitterComponent->NumSplitters);
    for (int Count1 = 1; Count1 <= state.dataSplitterComponent->NumSplitters; ++Count1) {
        for (int BCount = 1; BCount <= state.dataZoneEquip->NumSupplyAirPaths; ++BCount) {
            for (int Count = 1; Count <= state.dataZoneEquip->SupplyAirPath(BCount).NumOfComponents; ++Count) {
                if (state.dataSplitterComponent->SplitterCond(Count1).SplitterName !=
                        state.dataZoneEquip->SupplyAirPath(BCount).ComponentName(Count) ||
                    state.dataZoneEquip->SupplyAirPath(BCount).ComponentType(Count) != "AIRLOOPHVAC:ZONESPLITTER") {
                    continue;
                }
                if (FoundZoneSplitter(Count1)) {
                    ShowSevereError(
                        state,
                        format("AirLoopHVAC:ZoneSplitter=\"{}\", duplicate entry.", state.dataSplitterComponent->SplitterCond(Count1).SplitterName));
                    ShowContinueError(state, format("already exists on AirLoopHVAC:SupplyPath=\"{}\".", FoundNames(Count1)));
                    ErrFound = true;
                } else {
                    // record use
                    FoundZoneSplitter(Count1) = true;
                    FoundNames(Count1) = state.dataZoneEquip->SupplyAirPath(BCount).Name;
                }
            }
        }
    }
    FoundNames.deallocate();

    if (!all(FoundSupplyPlenum)) {
        for (int Count1 = 1; Count1 <= state.dataZonePlenum->NumZoneSupplyPlenums; ++Count1) {
            if (FoundSupplyPlenum(Count1)) {
                continue;
            }
            ShowSevereError(state,
                            format("AirLoopHVAC:SupplyPlenum=\"{}\", not found on any AirLoopHVAC:SupplyPath.",
                                   state.dataZonePlenum->ZoneSupPlenCond(Count1).ZonePlenumName));
        }
    }

    if (!all(FoundZoneSplitter)) {
        for (int Count1 = 1; Count1 <= state.dataSplitterComponent->NumSplitters; ++Count1) {
            if (FoundZoneSplitter(Count1)) {
                continue;
            }
            ShowSevereError(state,
                            format("AirLoopHVAC:ZoneSplitter=\"{}\", not found on any AirLoopHVAC:SupplyPath.",
                                   state.dataSplitterComponent->SplitterCond(Count1).SplitterName));
        }
    }

    FoundSupplyPlenum.deallocate();
    FoundZoneSplitter.deallocate();

    if (ErrFound) {
        ShowSevereError(state, "Supply Air Path(s) did not pass integrity testing");
    } else {
        ShowMessage(state, "All Supply Air Paths passed integrity testing");
    }
}

void TestReturnAirPathIntegrity(EnergyPlusData &state, bool &ErrFound, Array2S_int ValRetAPaths)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine tests return air path integrity and displays the loop for each branch.
    // Also, input and output nodes.

    // REFERENCES:
    // Return Air Path Validity Rules:
    //  Last component (zone mixer or zone return plenum) must resolve to
    //  be the outlet node for the return air path.  Inlets to this component must be outlets from
    //  previous components or "controlled zone outlets"?.
    //  (though converse not true -- each outlet in previous components do not
    //  have to be inlets on this item -- though they must be inputs somewhere in the stream).
    //  If multiple components and no mixer, then a zone return plenums "outlet" must
    //  be represented as an inlet on a later plenum.  i.e. some zone return plenums are
    //  really acting as "mixers" in a sense.  These do not need to be stepwise in succession.
    //  Same caveat for inlets from previous item.
    //  If multiple components and mixer, then prior condition (nested plenums) is allowed as long as
    //  those aren't duplicated as mixer inlets.  (i.e. zone rp 1 => zone rp 2 => zone mixer but
    //  zone rp 1 outlet should not also be inlet to mixer.
    //  Can have (nzrp -- nested zone return plenum, pzrp -- parallel zone return plenum):
    //  nzrp 1 => nzrp 2 & pzrp 3 => zm (inlets from nzrp 2 and pzrp 3).  Or, likewise:
    //  pzrp 1 & pzrp 2 => zm => pzrp 3 (outlets from pzrp 1/2 are inlets to zm whose outlet is an
    //  inlet to pzrp 3 whose outlet is the outlet for the return air path.

    //  Cannot have duplicate nodes in the "inlet" stream?  (i.e. cannot have same zone feeding two independent
    //  plenums, for example).  Similarly, Same return plenum can't be in two air loops nor as two independent
    //  return plenums in one return air path.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string PrimaryAirLoopName; // Air Loop to which this return air path is connected
    Array1D_bool FoundReturnPlenum;
    Array1D_bool FoundZoneMixer;
    Array1D_string FoundNames;
    Array1D_int AllNodes;

    // Formats

    // Do by Paths
    ShowMessage(state, "Testing Individual Return Air Path Integrity");
    ErrFound = false;
    int NumErr = 0;

    print(state.files.bnd, "{}\n", "! ===============================================================");
    static constexpr std::string_view Format_700("! <#Return Air Paths>,<Number of Return Air Paths>");
    print(state.files.bnd, "{}\n", Format_700);
    print(state.files.bnd, " #Return Air Paths,{}\n", state.dataZoneEquip->NumReturnAirPaths);
    static constexpr std::string_view Format_702("! <Return Air Path>,<Return Air Path Count>,<Return Air Path Name>,<AirLoopHVAC Name>");
    print(state.files.bnd, "{}\n", Format_702);
    static constexpr std::string_view Format_703("! <#Components on Return Air Path>,<Number of Components>");
    print(state.files.bnd, "{}\n", Format_703);
    static constexpr std::string_view Format_704(
        "! <Return Air Path Component>,<Component Count>,<Component Type>,<Component Name>,<AirLoopHVAC Name>");
    print(state.files.bnd, "{}\n", Format_704);
    static constexpr std::string_view Format_707("! <#Inlet Nodes on Return Air Path Component>,<Number of Nodes>");
    print(state.files.bnd, "{}\n", Format_707);
    static constexpr std::string_view Format_708(
        "! <Return Air Path Component Nodes>,<Node Count>,<Component Type>,<Component Name>,<Inlet Node Name>,<Outlet "
        "Node Name>,<AirLoopHVAC Name>");
    print(state.files.bnd, "{}\n", Format_708);

    AllNodes.allocate(state.dataLoopNodes->NumOfNodes);

    for (int BCount = 1; BCount <= state.dataZoneEquip->NumReturnAirPaths; ++BCount) {
        //             Determine which air loop this supply air path is connected to
        int Found = 0;
        for (int Count1 = 1; Count1 <= state.dataHVACGlobal->NumPrimaryAirSys; ++Count1) {
            PrimaryAirLoopName = state.dataAirLoop->AirToZoneNodeInfo(Count1).AirLoopName;
            Found = 0;
            for (int Count2 = 1; Count2 <= state.dataAirLoop->AirToZoneNodeInfo(Count1).NumReturnNodes; ++Count2) {
                if (state.dataZoneEquip->ReturnAirPath(BCount).OutletNodeNum ==
                    state.dataAirLoop->AirToZoneNodeInfo(Count1).ZoneEquipReturnNodeNum(Count2)) {
                    Found = Count2;
                }
            }
            if (Found != 0) {
                break;
            }
        }
        if (Found == 0) {
            PrimaryAirLoopName = "**Unknown**";
        }

        print(state.files.bnd, " Return Air Path,{},{},{}\n", BCount, state.dataZoneEquip->ReturnAirPath(BCount).Name, PrimaryAirLoopName);

        int NumComp = state.dataZoneEquip->ReturnAirPath(BCount).NumOfComponents;
        print(state.files.bnd, "   #Components on Return Air Path,{}\n", NumComp);

        std::string const &AirPathNodeName = state.dataLoopNodes->NodeID(state.dataZoneEquip->ReturnAirPath(BCount).OutletNodeNum);

        int MixerCount = 0;
        for (int Count = 1; Count <= NumComp; ++Count) {
            print(state.files.bnd,
                  "   Return Air Path Component,{},{},{},{}\n",
                  Count,
                  state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(Count),
                  state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(Count),
                  PrimaryAirLoopName);

            if (Util::SameString(state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(Count), "AirLoopHVAC:ZoneMixer")) {
                ++MixerCount;
            }
        }

        if (MixerCount > 1) {
            ShowSevereError(state, format("Too many zone mixers in Return Air Path={}", state.dataZoneEquip->ReturnAirPath(BCount).Name));
            ErrFound = true;
            ++NumErr;
            continue;
        }

        AllNodes = 0;
        int CountNodes = 0;

        if (NumComp > 0) {

            AirLoopHVACCompType CompType = static_cast<AirLoopHVACCompType>(
                getEnumValue(AirLoopHVACCompTypeNamesUC, Util::makeUPPER(state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(NumComp))));

            switch (CompType) {
            case AirLoopHVACCompType::ZoneMixer: {
                for (int Count2 = 1; Count2 <= state.dataMixerComponent->NumMixers; ++Count2) {
                    if (state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(NumComp) != state.dataMixerComponent->MixerCond(Count2).MixerName) {
                        continue;
                    }
                    // Found correct Mixer (by name), check outlet node vs. return air path outlet node
                    if (AirPathNodeName != state.dataLoopNodes->NodeID(state.dataMixerComponent->MixerCond(Count2).OutletNode)) {
                        ShowSevereError(state, format("Error in Return Air Path={}", state.dataZoneEquip->ReturnAirPath(BCount).Name));
                        ShowContinueError(state, format("For Connector:Mixer={}", state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(NumComp)));
                        ShowContinueError(state, format("Expected outlet node (return air path)={}", AirPathNodeName));
                        ShowContinueError(state,
                                          format("Encountered node name (mixer)={}",
                                                 state.dataLoopNodes->NodeID(state.dataMixerComponent->MixerCond(Count2).OutletNode)));
                        ErrFound = true;
                        ++NumErr;
                    } else {
                        ++CountNodes;
                        AllNodes(CountNodes) = state.dataMixerComponent->MixerCond(Count2).OutletNode;
                        if (state.dataZoneEquip->ReturnAirPath(BCount).OutletNodeNum == state.dataMixerComponent->MixerCond(Count2).OutletNode) {
                            state.dataZoneEquip->ReturnAirPath(BCount).OutletRetPathCompNum = NumComp;
                        }
                        for (int Loop = 1; Loop <= state.dataMixerComponent->MixerCond(Count2).NumInletNodes; ++Loop) {
                            ++CountNodes;
                            AllNodes(CountNodes) = state.dataMixerComponent->MixerCond(Count2).InletNode(Loop);
                        }
                    }
                    print(state.files.bnd,
                          "     #Inlet Nodes on Return Air Path Component,{}\n",
                          state.dataMixerComponent->MixerCond(Count2).NumInletNodes);
                    for (int Count1 = 1; Count1 <= state.dataMixerComponent->MixerCond(Count2).NumInletNodes; ++Count1) {
                        print(state.files.bnd,
                              "     Return Air Path Component Nodes,{},{},{},{},{},{}\n",
                              Count1,
                              state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(NumComp),
                              state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(NumComp),
                              state.dataLoopNodes->NodeID(state.dataMixerComponent->MixerCond(Count2).InletNode(Count1)),
                              state.dataLoopNodes->NodeID(state.dataMixerComponent->MixerCond(Count2).OutletNode),
                              PrimaryAirLoopName);
                    }
                }
            } break;
            case AirLoopHVACCompType::ReturnPlenum: {
                for (int Count2 = 1; Count2 <= state.dataZonePlenum->NumZoneReturnPlenums; ++Count2) {
                    if (state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(NumComp) !=
                        state.dataZonePlenum->ZoneRetPlenCond(Count2).ZonePlenumName) {
                        continue;
                    }
                    if (AirPathNodeName != state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneRetPlenCond(Count2).OutletNode)) {
                        ShowSevereError(state, format("Error in Return Air Path={}", state.dataZoneEquip->ReturnAirPath(BCount).Name));
                        ShowContinueError(
                            state, format("For AirLoopHVAC:ReturnPlenum={}", state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(NumComp)));
                        ShowContinueError(state, format("Expected outlet node (return air path)={}", AirPathNodeName));
                        ShowContinueError(state,
                                          format("Encountered node name (zone return plenum)={}",
                                                 state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneRetPlenCond(Count2).OutletNode)));
                        ErrFound = true;
                        ++NumErr;
                    } else {
                        ++CountNodes;
                        AllNodes(CountNodes) = state.dataZonePlenum->ZoneRetPlenCond(Count2).OutletNode;
                        if (state.dataZoneEquip->ReturnAirPath(BCount).OutletNodeNum == state.dataZonePlenum->ZoneRetPlenCond(Count2).OutletNode) {
                            state.dataZoneEquip->ReturnAirPath(BCount).OutletRetPathCompNum = NumComp;
                        }
                        for (int Loop = 1; Loop <= state.dataZonePlenum->ZoneRetPlenCond(Count2).NumInletNodes; ++Loop) {
                            ++CountNodes;
                            AllNodes(CountNodes) = state.dataZonePlenum->ZoneRetPlenCond(Count2).InletNode(Loop);
                        }
                    }
                    print(state.files.bnd,
                          "     #Inlet Nodes on Return Air Path Component,{}\n",
                          state.dataZonePlenum->ZoneRetPlenCond(Count2).NumInletNodes);
                    for (int Count1 = 1; Count1 <= state.dataZonePlenum->ZoneRetPlenCond(Count2).NumInletNodes; ++Count1) {
                        print(state.files.bnd,
                              "     Return Air Path Component Nodes,{},{},{},{},{},{}\n",
                              Count1,
                              state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(NumComp),
                              state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(NumComp),
                              state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneRetPlenCond(Count2).InletNode(Count1)),
                              state.dataLoopNodes->NodeID(state.dataZonePlenum->ZoneRetPlenCond(Count2).OutletNode),
                              PrimaryAirLoopName);
                    }
                }
            } break;
            default: // This already validated in GetReturnAirPath
                break;
            }
        }

        if (NumComp > 1) {
            for (int Count3 = 1; Count3 <= NumComp - 1; ++Count3) {

                AirLoopHVACCompType CompType = static_cast<AirLoopHVACCompType>(
                    getEnumValue(AirLoopHVACCompTypeNamesUC, Util::makeUPPER(state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(Count3))));

                switch (CompType) {
                case AirLoopHVACCompType::ZoneMixer: {
                    for (int Count2 = 1; Count2 <= state.dataMixerComponent->NumMixers; ++Count2) {
                        if (state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(Count3) !=
                            state.dataMixerComponent->MixerCond(Count2).MixerName) {
                            continue;
                        }
                        for (int Loop = 1; Loop <= state.dataMixerComponent->MixerCond(Count2).NumInletNodes; ++Loop) {
                            ++CountNodes;
                            AllNodes(CountNodes) = state.dataMixerComponent->MixerCond(Count2).InletNode(Loop);
                        }
                    }
                } break;
                case AirLoopHVACCompType::ReturnPlenum: {
                    for (int Count2 = 1; Count2 <= state.dataZonePlenum->NumZoneReturnPlenums; ++Count2) {
                        if (state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(Count3) !=
                            state.dataZonePlenum->ZoneRetPlenCond(Count2).ZonePlenumName) {
                            continue;
                        }
                        for (int Loop = 1; Loop <= state.dataZonePlenum->ZoneRetPlenCond(Count2).NumInletNodes; ++Loop) {
                            ++CountNodes;
                            AllNodes(CountNodes) = state.dataZonePlenum->ZoneRetPlenCond(Count2).InletNode(Loop);
                        }
                    }
                } break;
                default: // This already validated in GetReturnAirPath
                    break;
                }
            }
        }
        if (CountNodes > 0) {
            static constexpr std::string_view Format_705("! <#Nodes on Return Air Path>,<Number of Nodes>");
            print(state.files.bnd, "{}\n", Format_705);
            static constexpr std::string_view Format_706("! <Return Air Path Node>,<Node Type>,<Node Count>,<Node Name>,<AirLoopHVAC Name>");
            print(state.files.bnd, "{}\n", Format_706);
            print(state.files.bnd, "   #Nodes on Return Air Path,{}\n", CountNodes);
            for (int Count2 = 1; Count2 <= CountNodes; ++Count2) {
                if (Count2 == 1) {
                    print(state.files.bnd,
                          "   Return Air Path Node,Outlet Node,{},{},{}\n",
                          Count2,
                          state.dataLoopNodes->NodeID(AllNodes(Count2)),
                          PrimaryAirLoopName);
                } else {
                    print(state.files.bnd,
                          "   Return Air Path Node,Inlet Node,{},{},{}\n",
                          Count2,
                          state.dataLoopNodes->NodeID(AllNodes(Count2)),
                          PrimaryAirLoopName);
                }
            }
        }
        // Determine Air Loop this Return Air Path is on
        for (int Count2 = 1; Count2 <= state.dataHVACGlobal->NumPrimaryAirSys; ++Count2) {
            if (state.dataAirLoop->AirToZoneNodeInfo(Count2).NumReturnNodes > 0) {
                if (AllNodes(1) == state.dataAirLoop->AirToZoneNodeInfo(Count2).ZoneEquipReturnNodeNum(1)) {
                    const int WAirLoop = Count2;
                    ValRetAPaths(_, WAirLoop) = 0;
                    ValRetAPaths({1, CountNodes}, WAirLoop) = AllNodes({1, CountNodes});
                    break;
                }
            } else {
                ShowWarningError(state,
                                 format("TestReturnAirPathIntegrity: Air Loop has no Zone Equipment Return Node={}",
                                        state.dataAirLoop->AirToZoneNodeInfo(Count2).AirLoopName));
            }
        }
    }

    AllNodes.deallocate();

    if (state.dataMixerComponent->NumMixers == 0) {
        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ZoneMixer") > 0) {
            MixerComponent::GetMixerInput(state);
        }
    }
    if (state.dataZonePlenum->NumZoneSupplyPlenums == 0 && state.dataZonePlenum->NumZoneReturnPlenums == 0) {
        if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ReturnPlenum") > 0) {
            ZonePlenum::GetZonePlenumInput(state);
        }
    }

    // now the reverse.  is every zone Mixer and Return plenum on Return air path
    FoundReturnPlenum.dimension(state.dataZonePlenum->NumZoneReturnPlenums, false);
    FoundZoneMixer.dimension(state.dataMixerComponent->NumMixers, false);
    FoundNames.allocate(state.dataZonePlenum->NumZoneReturnPlenums);
    for (int Count1 = 1; Count1 <= state.dataZonePlenum->NumZoneReturnPlenums; ++Count1) {
        for (int BCount = 1; BCount <= state.dataZoneEquip->NumReturnAirPaths; ++BCount) {
            for (int Count = 1; Count <= state.dataZoneEquip->ReturnAirPath(BCount).NumOfComponents; ++Count) {
                if (state.dataZonePlenum->ZoneRetPlenCond(Count1).ZonePlenumName != state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(Count) ||
                    state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(Count) != "AIRLOOPHVAC:RETURNPLENUM") {
                    continue;
                }
                if (FoundReturnPlenum(Count1)) {
                    ShowSevereError(
                        state,
                        format("AirLoopHVAC:ReturnPlenum=\"{}\", duplicate entry.", state.dataZonePlenum->ZoneRetPlenCond(Count1).ZonePlenumName));
                    ShowContinueError(state, format("already exists on AirLoopHVAC:ReturnPath=\"{}\".", FoundNames(Count1)));
                    ErrFound = true;
                } else {
                    // record use
                    FoundReturnPlenum(Count1) = true;
                    FoundNames(Count1) = state.dataZoneEquip->ReturnAirPath(BCount).Name;
                }
            }
        }
        if (PurchasedAirManager::CheckPurchasedAirForReturnPlenum(state, Count1)) {
            FoundReturnPlenum(Count1) = true;
        }
    }
    FoundNames.deallocate();
    FoundNames.allocate(state.dataMixerComponent->NumMixers);
    for (int Count1 = 1; Count1 <= state.dataMixerComponent->NumMixers; ++Count1) {
        for (int BCount = 1; BCount <= state.dataZoneEquip->NumReturnAirPaths; ++BCount) {
            for (int Count = 1; Count <= state.dataZoneEquip->ReturnAirPath(BCount).NumOfComponents; ++Count) {
                if (state.dataMixerComponent->MixerCond(Count1).MixerName != state.dataZoneEquip->ReturnAirPath(BCount).ComponentName(Count) ||
                    state.dataZoneEquip->ReturnAirPath(BCount).ComponentType(Count) != "AIRLOOPHVAC:ZONEMIXER") {
                    continue;
                }
                if (FoundZoneMixer(Count1)) {
                    ShowSevereError(state,
                                    format("AirLoopHVAC:ZoneMixer=\"{}\", duplicate entry.", state.dataMixerComponent->MixerCond(Count1).MixerName));
                    ShowContinueError(state, format("already exists on AirLoopHVAC:ReturnPath=\"{}\".", FoundNames(Count1)));
                    ErrFound = true;
                } else {
                    // record use
                    FoundZoneMixer(Count1) = true;
                    FoundNames(Count1) = state.dataZoneEquip->ReturnAirPath(BCount).Name;
                }
            }
        }
        if (!FoundZoneMixer(Count1)) { // could be as child on other items
            // PIU Units
            if (PoweredInductionUnits::PIUnitHasMixer(state, state.dataMixerComponent->MixerCond(Count1).MixerName)) {
                FoundZoneMixer(Count1) = true;
            }
        }
        if (!FoundZoneMixer(Count1)) { // could be as child on other items
            // fourPipeInduction units
            if (HVACSingleDuctInduc::FourPipeInductionUnitHasMixer(state, state.dataMixerComponent->MixerCond(Count1).MixerName)) {
                FoundZoneMixer(Count1) = true;
            }
        }
        if (!FoundZoneMixer(Count1)) { // could be as child on other items
            // Exhaust Systems
            if (ExhaustAirSystemManager::ExhaustSystemHasMixer(state, state.dataMixerComponent->MixerCond(Count1).MixerName)) {
                FoundZoneMixer(Count1) = true;
            }
        }
    }
    FoundNames.deallocate();

    if (!all(FoundReturnPlenum)) {
        for (int Count1 = 1; Count1 <= state.dataZonePlenum->NumZoneReturnPlenums; ++Count1) {
            if (FoundReturnPlenum(Count1)) {
                continue;
            }
            ShowSevereError(state,
                            format("AirLoopHVAC:ReturnPlenum=\"{}\", not found on any AirLoopHVAC:ReturnPath.",
                                   state.dataZonePlenum->ZoneRetPlenCond(Count1).ZonePlenumName));
        }
    }

    if (!all(FoundZoneMixer)) {
        for (int Count1 = 1; Count1 <= state.dataMixerComponent->NumMixers; ++Count1) {
            if (FoundZoneMixer(Count1)) {
                continue;
            }
            ShowSevereError(state,
                            format("AirLoopHVAC:ZoneMixer=\"{}\", not found on any AirLoopHVAC:ReturnPath, AirLoopHVAC:ExhaustSystem, "
                                   "AirTerminal:SingleDuct:SeriesPIU:Reheat,",
                                   state.dataMixerComponent->MixerCond(Count1).MixerName));
            ShowContinueError(state, "AirTerminal:SingleDuct:ParallelPIU:Reheat or AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction.");
        }
    }

    FoundReturnPlenum.deallocate();
    FoundZoneMixer.deallocate();

    if (ErrFound) {
        ShowSevereError(state, "Return Air Path(s) did not pass integrity testing");
    } else {
        ShowMessage(state, "All Return Air Paths passed integrity testing");
    }
}

void CalcComponentSensibleLatentOutput(Real64 const MassFlow,  // air mass flow rate, {kg/s}
                                       Real64 const TDB2,      // dry-bulb temperature at state 2 {C}
                                       Real64 const W2,        // humidity ratio at state 2
                                       Real64 const TDB1,      // dry-bulb temperature at  at state 1 {C}
                                       Real64 const W1,        // humidity ratio at state 1
                                       Real64 &SensibleOutput, // sensible output rate (state 2 -> State 1), {W}
                                       Real64 &LatentOutput,   // latent output rate (state 2 -> State 1), {W}
                                       Real64 &TotalOutput     // total = sensible + latent putput rate (state 2 -> State 1), {W}
)
{

    // Purpose:
    // returns total, sensible and latent heat rate of change of moist air transitioning
    // between two states. The moist air energy transfer can be cooling or heating process
    // across a cooling, a heating coil, or an HVAC component.

    // Methodology:
    // Q_total = m_dot * (h2 - h1)
    // Q_sensible = m_dot * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(TDB2, W2, TDB1, W1);
    // or Q_sensible = m_dot * cp_moistair_MinHumRat * (TDB2 - TDB1)
    //    cp_moistair_MinHumRat = Psychrometrics::PsyCpAirFnW(min(W2, W1));
    // Q_latent = Q_total - Q_latent;

    TotalOutput = 0.0;
    LatentOutput = 0.0;
    SensibleOutput = 0.0;
    if (MassFlow > 0.0) {
        TotalOutput = MassFlow * (Psychrometrics::PsyHFnTdbW(TDB2, W2) - Psychrometrics::PsyHFnTdbW(TDB1, W1)); // total addition/removal rate, {W};
        SensibleOutput = MassFlow * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(TDB2, W2, TDB1, W1); // sensible addition/removal rate, {W};
        LatentOutput = TotalOutput - SensibleOutput;                                                // latent addition/removal rate, {W}
    }
}

void CalcZoneSensibleLatentOutput(Real64 const MassFlow,  // air mass flow rate, {kg/s}
                                  Real64 const TDBEquip,  // dry-bulb temperature at equipment outlet {C}
                                  Real64 const WEquip,    // humidity ratio at equipment outlet
                                  Real64 const TDBZone,   // dry-bulb temperature at zone air node {C}
                                  Real64 const WZone,     // humidity ratio at zone air node
                                  Real64 &SensibleOutput, // sensible output rate (state 2 -> State 1), {W}
                                  Real64 &LatentOutput,   // latent output rate (state 2 -> State 1), {W}
                                  Real64 &TotalOutput     // total = sensible + latent putput rate (state 2 -> State 1), {W}
)
{

    // Purpose:
    // returns total, sensible and latent heat rate of transfer between the supply air zone inlet
    // node and zone air node. The moist air energy transfer can be cooling or heating depending
    // on the supply air zone inlet node and zone air node conditions.

    // Methodology:
    // Q_total = m_dot * (hEquip - hZone)
    // Q_sensible = m_dot * Psychrometrics::PsyDeltaHSenFnTdbEquipTdbWZone(TDBEquip, TDBZone, WZone);
    // or Q_sensible = m_dot * cp_moistair_zoneHumRat * (TDBEquip - TDBZone)
    //    cp_moistair_zoneHumRat = Psychrometrics::PsyCpAirFnW(WZone);
    // Q_latent = Q_total - Q_latent;

    TotalOutput = 0.0;
    LatentOutput = 0.0;
    SensibleOutput = 0.0;
    if (MassFlow > 0.0) {
        TotalOutput = MassFlow * (Psychrometrics::PsyHFnTdbW(TDBEquip, WEquip) -
                                  Psychrometrics::PsyHFnTdbW(TDBZone, WZone));                         // total addition/removal rate, {W};
        SensibleOutput = MassFlow * Psychrometrics::PsyDeltaHSenFnTdb2Tdb1W(TDBEquip, TDBZone, WZone); // sensible addition/removal rate, {W};
        LatentOutput = TotalOutput - SensibleOutput;                                                   // latent addition/removal rate, {W}
    }
}

Real64 calcZoneSensibleOutput(Real64 const MassFlow, // air mass flow rate, {kg/s}
                              Real64 const TDBEquip, // dry-bulb temperature at equipment outlet {C}
                              Real64 const TDBZone,  // dry-bulb temperature at zone air node {C}
                              Real64 const WZone     // humidity ratio at zone air node
)
{

    // Purpose:
    // returns sensible heat rate of transfer between the supply air zone inlet node and
    // zone air node. The moist air energy transfer can be cooling or heating depending
    // on the supply air zone inlet node and zone air node conditions.

    // Methodology:
    // Q_sensible = m_dot * Psychrometrics::PsyDeltaHSenFnTdbEquipTdbWZone(TDBEquip, TDBZone, WZone);
    // or Q_sensible = m_dot * cp_moistair_zoneHumRat * (TDBEquip - TDBZone)
    //    cp_moistair_zoneHumRat = Psychrometrics::PsyCpAirFnW(WZone);

    Real64 sensibleOutput = 0.0; // sensible output rate (state 2 -> State 1), {W}
    if (MassFlow > 0.0) {
        sensibleOutput = MassFlow * Psychrometrics::PsyDeltaHSenFnTdb2Tdb1W(TDBEquip, TDBZone, WZone); // sensible addition/removal rate, {W};
    }
    return sensibleOutput;
}
} // namespace EnergyPlus
