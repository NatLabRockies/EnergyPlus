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

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>

namespace EnergyPlus {

namespace ZoneDehumidifier {

    // Module containing the routines dealing with the ZoneDehumidifier

    // MODULE INFORMATION:
    //       AUTHOR         Don Shirey, FSEC
    //       DATE WRITTEN   July/Aug 2009
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Calculate the performance of zone (room) air dehumidifiers.  Meant to model
    // conventional direct expansion (DX) cooling-based room air dehumidifiers
    // (reject 100% of condenser heat to the zone air), but the approach
    // might be able to be used to model other room air dehumidifier types.

    // METHODOLOGY EMPLOYED:
    // Model as a piece of zone equipment, with inputs for water removal and
    // energy factor at rated conditions (26.7C, 60% RH). Then provide curve objects
    // to describe performance at off-rated conditions. A part-load cycling curve
    // input is also provided. It is assumed that this equipment dehumidifies but
    // heats the air. If used in tandem with another system that cools and dehumidifies,
    // then the zone dehumidifier should be specified as the lowest cooling priority
    // in the ZoneHVAC:EquipmentList object. The cooling and dehumidification system
    // operates first to meet the temperature setpoint (and possibly the high humidity
    // setpoint as well). If additional dehumidification is needed, then the zone
    // dehumidifier operates. The excess sensible heat generated by the dehumidifier
    // is carried over to the next HVAC time step.

    // OTHER NOTES:
    // Example manufacturer's data at:
    //   http://www.thermastor.com/HI-E-DRY-100/HI-E-DRY-100-Spec.pdf
    //   http://www.thermastor.com/HI-E-DRY-195/HI-E-DRY-195-Spec.pdf

    // Using/Aliasing
    using namespace DataLoopNode;

    void SimZoneDehumidifier(EnergyPlusData &state,
                             std::string const &CompName,                    // Name of the zone dehumidifier
                             int const ZoneNum,                              // Number of zone being served
                             [[maybe_unused]] bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                             Real64 &QSensOut,                               // Sensible capacity delivered to zone (W)
                             Real64 &QLatOut,                                // Latent capacity delivered to zone (kg/s), dehumidify = negative
                             int &CompIndex                                  // Index to the zone dehumidifier
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey, FSEC
        //       DATE WRITTEN   July/Aug 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a zone dehumidifier.

        // METHODOLOGY EMPLOYED:
        // Call appropriate subroutines to get input values, initialize variables, model performanc
        // update node information, report model outputs.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneDehumidNum;   // Index of zone dehumidifier being simulated
        Real64 QZnDehumidReq; // Zone dehumidification load required (kg moisture/sec)

        if (state.dataZoneDehumidifier->GetInputFlag) {
            GetZoneDehumidifierInput(state);
            state.dataZoneDehumidifier->GetInputFlag = false;
        }

        // Find the correct zone dehumidifier
        if (CompIndex == 0) {
            ZoneDehumidNum = Util::FindItemInList(CompName, state.dataZoneDehumidifier->ZoneDehumid);
            if (ZoneDehumidNum == 0) {
                ShowFatalError(state, format("SimZoneDehumidifier: Unit not found= {}", CompName));
            }
            CompIndex = ZoneDehumidNum;
        } else {
            ZoneDehumidNum = CompIndex;
            int NumDehumidifiers = (int)state.dataZoneDehumidifier->ZoneDehumid.size();
            if (ZoneDehumidNum > NumDehumidifiers || ZoneDehumidNum < 1) {
                ShowFatalError(state,
                               format("SimZoneDehumidifier:  Invalid CompIndex passed= {}, Number of Units= {}, Entered Unit name= {}",
                                      ZoneDehumidNum,
                                      NumDehumidifiers,
                                      CompName));
            }
            if (state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidNum).CheckEquipName) {
                if (CompName != state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidNum).Name) {
                    ShowFatalError(state,
                                   format("SimZoneDehumidifier: Invalid CompIndex passed={}, Unit name= {}, stored Unit Name for that index= {}",
                                          ZoneDehumidNum,
                                          CompName,
                                          state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidNum).Name));
                }
                state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidNum).CheckEquipName = false;
            }
        }

        QZnDehumidReq = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum).RemainingOutputReqToDehumidSP; // Negative means dehumidify

        InitZoneDehumidifier(state, ZoneDehumidNum);

        CalcZoneDehumidifier(state, ZoneDehumidNum, QZnDehumidReq, QSensOut, QLatOut);

        UpdateZoneDehumidifier(state, ZoneDehumidNum);

        ReportZoneDehumidifier(state, ZoneDehumidNum);
    }

    void GetZoneDehumidifierInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey, FSEC
        //       DATE WRITTEN   July/Aug 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Retrieve the inputs from the input data file (idf) being simulated.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology using available utility routines where appropriate.

        // Using/Aliasing
        using NodeInputManager::GetOnlySingleNode;
        using WaterManager::SetupTankSupplyComponent;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName = "GetZoneDehumidifierInput";
        static std::string const CurrentModuleObject("ZoneHVAC:Dehumidifier:DX");
        Real64 constexpr RatedInletAirTemp(26.7);
        Real64 constexpr RatedInletAirRH(60.0);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ZoneDehumidIndex;          // Loop index
        int NumAlphas(0);              // Number of Alphas to allocate arrays, then used for each GetObjectItem call
        int NumNumbers(0);             // Number of Numbers to allocate arrays, then used for each GetObjectItem call
        int IOStatus;                  // Used in GetObjectItem
        bool ErrorsFound(false);       // Set to true if errors in input, fatal at end of routine
        Array1D_string Alphas;         // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> Numbers;       // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int TotalArgs(0);              // Total number of alpha and numeric arguments (max)

        int NumDehumidifiers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        state.dataZoneDehumidifier->ZoneDehumid.allocate(NumDehumidifiers);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);

        Alphas.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        Numbers.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        for (ZoneDehumidIndex = 1; ZoneDehumidIndex <= NumDehumidifiers; ++ZoneDehumidIndex) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     ZoneDehumidIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            auto &dehumid = state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidIndex);
            // A1,  \field Name
            dehumid.Name = Alphas(1);
            dehumid.UnitType = CurrentModuleObject; // 'ZoneHVAC:Dehumidifier:DX'

            // A2,  \field Availability Schedule Name
            if (lAlphaBlanks(2)) {
                dehumid.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((dehumid.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            // A3 , \field Air Inlet Node Name
            dehumid.AirInletNodeNum = GetOnlySingleNode(state,
                                                        Alphas(3),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::ZoneHVACDehumidifierDX,
                                                        Alphas(1),
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Inlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        ObjectIsNotParent);

            // A4 , \field Air Outlet Node Name
            dehumid.AirOutletNodeNum = GetOnlySingleNode(state,
                                                         Alphas(4),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::ZoneHVACDehumidifierDX,
                                                         Alphas(1),
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Outlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         ObjectIsNotParent);

            // N1,  \field Rated Water Removal
            dehumid.RatedWaterRemoval = Numbers(1);
            if (dehumid.RatedWaterRemoval <= 0.0) {
                ShowSevereError(state, format("{} must be greater than zero.", cNumericFields(1)));
                ShowContinueError(state, format("Value specified = {:.5T}", Numbers(1)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, dehumid.Name));
                ErrorsFound = true;
            }

            // N2,  \field Rated Energy Factor
            dehumid.RatedEnergyFactor = Numbers(2);
            if (dehumid.RatedEnergyFactor <= 0.0) {
                ShowSevereError(state, format("{} must be greater than zero.", cNumericFields(2)));
                ShowContinueError(state, format("Value specified = {:.5T}", Numbers(2)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, dehumid.Name));
                ErrorsFound = true;
            }

            // N3,  \field Rated Air Flow Rate
            dehumid.RatedAirVolFlow = Numbers(3);
            if (dehumid.RatedAirVolFlow <= 0.0) {
                ShowSevereError(state, format("{} must be greater than zero.", cNumericFields(3)));
                ShowContinueError(state, format("Value specified = {:.5T}", Numbers(3)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, dehumid.Name));
                ErrorsFound = true;
            }

            // A5,  \field Water Removal Curve Name
            if (lAlphaBlanks(5)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(5));
                ErrorsFound = true;
            } else if ((dehumid.WaterRemovalCurve = Curve::GetCurve(state, Alphas(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                ErrorsFound = true;
            } else if (dehumid.WaterRemovalCurve->numDims != 2) {
                Curve::ShowSevereCurveDims(state, eoh, cAlphaFields(5), Alphas(5), "2", dehumid.WaterRemovalCurve->numDims);
                ErrorsFound = true;
            } else {
                Real64 CurveVal = dehumid.WaterRemovalCurve->value(state, RatedInletAirTemp, RatedInletAirRH);
                if (CurveVal > 1.10 || CurveVal < 0.90) {
                    ShowWarningError(state, format("{} output is not equal to 1.0", cAlphaFields(5)));
                    ShowContinueError(state, format("(+ or -10%) at rated conditions for {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Curve output at rated conditions = {:.3T}", CurveVal));
                }
            }

            // A6,  \field Energy Factor Curve Name
            if (lAlphaBlanks(6)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(6));
                ErrorsFound = true;
            } else if ((dehumid.EnergyFactorCurve = Curve::GetCurve(state, Alphas(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), Alphas(6));
                ErrorsFound = true;
            } else if (dehumid.EnergyFactorCurve->numDims != 2) {
                Curve::ShowSevereCurveDims(state, eoh, cAlphaFields(6), Alphas(6), "2", dehumid.EnergyFactorCurve->numDims);
                ErrorsFound = true;
            } else {
                Real64 CurveVal = dehumid.EnergyFactorCurve->value(state, RatedInletAirTemp, RatedInletAirRH);
                if (CurveVal > 1.10 || CurveVal < 0.90) {
                    ShowWarningError(state, format("{} output is not equal to 1.0", cAlphaFields(6)));
                    ShowContinueError(state, format("(+ or -10%) at rated conditions for {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Curve output at rated conditions = {:.3T}", CurveVal));
                }
            }

            // A7,  \field Part Load Fraction Correlation Curve Name
            if (lAlphaBlanks(7)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(7));
                ErrorsFound = true;
            } else if ((dehumid.PartLoadCurve = Curve::GetCurve(state, Alphas(7))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(7), Alphas(7));
                ErrorsFound = true;
            } else if (dehumid.PartLoadCurve->numDims != 1) {
                Curve::ShowSevereCurveDims(state, eoh, cAlphaFields(7), Alphas(7), "1", dehumid.PartLoadCurve->numDims);
                ErrorsFound = true;
            }

            // N4,  \field Minimum Dry-Bulb Temperature for Dehumidifier Operation
            // N5,  \field Maximum Dry-Bulb Temperature for Dehumidifier Operation
            dehumid.MinInletAirTemp = Numbers(4);
            dehumid.MaxInletAirTemp = Numbers(5);

            if (dehumid.MinInletAirTemp >= dehumid.MaxInletAirTemp) {
                ShowSevereError(state, format("{} must be greater than {}", cNumericFields(5), cNumericFields(4)));
                ShowContinueError(state, format("{} specified = {:.1T}", cNumericFields(5), Numbers(5)));
                ShowContinueError(state, format("{} specified = {:.1T}", cNumericFields(4), Numbers(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, dehumid.Name));
                ErrorsFound = true;
            }

            // N6,  \field Off Cycle Parasitic Electric Load
            dehumid.OffCycleParasiticLoad = Numbers(6); // Off Cycle Parasitic Load [W]

            if (dehumid.OffCycleParasiticLoad < 0.0) {
                ShowSevereError(state, format("{} must be >= zero.", cNumericFields(6)));
                ShowContinueError(state, format("Value specified = {:.2T}", Numbers(6)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, dehumid.Name));
                ErrorsFound = true;
            }

            // A8;  \field Condensate Collection Water Storage Tank Name
            dehumid.CondensateCollectName = Alphas(8);
            if (lAlphaBlanks(8)) {
                dehumid.CondensateCollectMode = CondensateOutlet::Discarded;
            } else {
                dehumid.CondensateCollectMode = CondensateOutlet::ToTank;
                SetupTankSupplyComponent(state,
                                         dehumid.Name,
                                         CurrentModuleObject,
                                         dehumid.CondensateCollectName,
                                         ErrorsFound,
                                         dehumid.CondensateTankID,
                                         dehumid.CondensateTankSupplyARRID);
            }

        } //   DO ZoneDehumidIndex=1,NumDehumidifiers

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}:{}: Errors found in input.", routineName, CurrentModuleObject));
        }

        for (ZoneDehumidIndex = 1; ZoneDehumidIndex <= NumDehumidifiers; ++ZoneDehumidIndex) {
            auto &dehumid = state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidIndex);
            // Set up report variables for the dehumidifiers
            SetupOutputVariable(state,
                                "Zone Dehumidifier Sensible Heating Rate",
                                Constant::Units::W,
                                dehumid.SensHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Sensible Heating Energy",
                                Constant::Units::J,
                                dehumid.SensHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Removed Water Mass Flow Rate",
                                Constant::Units::kg_s,
                                dehumid.WaterRemovalRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Removed Water Mass",
                                Constant::Units::kg,
                                dehumid.WaterRemoved,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Electricity Rate",
                                Constant::Units::W,
                                dehumid.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Electricity Energy",
                                Constant::Units::J,
                                dehumid.ElecConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                dehumid.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Off Cycle Parasitic Electricity Rate",
                                Constant::Units::W,
                                dehumid.OffCycleParasiticElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Off Cycle Parasitic Electricity Energy",
                                Constant::Units::J,
                                dehumid.OffCycleParasiticElecCons,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Part Load Ratio",
                                Constant::Units::None,
                                dehumid.DehumidPLR,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Runtime Fraction",
                                Constant::Units::None,
                                dehumid.DehumidRTF,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);
            SetupOutputVariable(state,
                                "Zone Dehumidifier Outlet Air Temperature",
                                Constant::Units::C,
                                dehumid.OutletAirTemp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                dehumid.Name);

            if (dehumid.CondensateCollectMode == CondensateOutlet::ToTank) {
                SetupOutputVariable(state,
                                    "Zone Dehumidifier Condensate Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    dehumid.DehumidCondVolFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    dehumid.Name);
                SetupOutputVariable(state,
                                    "Zone Dehumidifier Condensate Volume",
                                    Constant::Units::m3,
                                    dehumid.DehumidCondVol,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    dehumid.Name,
                                    Constant::eResource::OnSiteWater,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Condensate);
            }
        }
    }

    void InitZoneDehumidifier(EnergyPlusData &state, int const ZoneDehumNum) // Number of the current zone dehumidifier being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey, FSEC
        //       DATE WRITTEN   July/Aug 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes information for the zone dehumidifier model

        // METHODOLOGY EMPLOYED:
        // Use status flags to trigger various initializations

        // Using/Aliasing
        using DataZoneEquipment::CheckZoneEquipmentList;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyWFnTdbRhPb;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitZoneDehumidifier");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirInletNode;      // Inlet air node number
        Real64 RatedAirHumrat; // Humidity ratio (kg/kg) at rated inlet air conditions of 26.6667C, 60% RH
        Real64 RatedAirDBTemp; // Dry-bulb air temperature at rated conditions 26.6667C
        Real64 RatedAirRH;     // Relative humidity of air (0.6 --> 60%) at rated conditions

        auto &dehumid = state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumNum);
        // Need to check all dehumidifiers to see if they are on Zone Equipment List or issue warning
        if (!dehumid.ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            dehumid.ZoneEquipmentListChecked = true;
            if (!CheckZoneEquipmentList(state, dehumid.UnitType, dehumid.Name)) {
                ShowSevereError(
                    state,
                    format("InitZoneDehumidifier: Zone Dehumidifier=\"{},{}\" is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                           dehumid.UnitType,
                           dehumid.Name));
            }
        }

        AirInletNode = dehumid.AirInletNodeNum;
        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && dehumid.MyEnvrnFlag) {

            // Set the mass flow rates from the input volume flow rates, at rated conditions of 26.6667C, 60% RH
            // Might default back to STP later after discussion with M. Witte, use StdRhoAir instead of calc'd RhoAir at rated conditions
            RatedAirDBTemp = 26.6667; // 26.6667 C, 80F
            RatedAirRH = 0.6;         // 60% RH
            RatedAirHumrat = PsyWFnTdbRhPb(state, RatedAirDBTemp, RatedAirRH, state.dataEnvrn->StdBaroPress, RoutineName);
            dehumid.RatedAirMassFlow =
                PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, RatedAirDBTemp, RatedAirHumrat, RoutineName) * dehumid.RatedAirVolFlow;

            // Set the node max and min mass flow rates on inlet node... outlet node gets updated in UPDATE subroutine
            state.dataLoopNodes->Node(AirInletNode).MassFlowRateMax = dehumid.RatedAirMassFlow;
            state.dataLoopNodes->Node(AirInletNode).MassFlowRateMaxAvail = dehumid.RatedAirMassFlow;
            state.dataLoopNodes->Node(AirInletNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(AirInletNode).MassFlowRateMin = 0.0;

            dehumid.MyEnvrnFlag = false;
        } // End one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            dehumid.MyEnvrnFlag = true;
        }

        // These initializations are done every iteration
        state.dataLoopNodes->Node(AirInletNode).MassFlowRate = dehumid.RatedAirMassFlow;

        // Zero out the report variables
        dehumid.SensHeatingRate = 0.0;                                        // Zone Dehumidifier Sensible Heating Rate [W]
        dehumid.SensHeatingEnergy = 0.0;                                      // Zone Dehumidifier Sensible Heating Energy [J]
        dehumid.WaterRemovalRate = 0.0;                                       // Zone Dehumidifier Water Removal Rate [kg/s]
        dehumid.WaterRemoved = 0.0;                                           // Zone Dehumidifier Water Removed [kg]
        dehumid.ElecPower = 0.0;                                              // Zone Dehumidifier Electric Power [W]
        dehumid.ElecConsumption = 0.0;                                        // Zone Dehumidifier Electric Consumption [J]
        dehumid.DehumidPLR = 0.0;                                             // Zone Dehumidifier Part-Load Ratio [-]
        dehumid.DehumidRTF = 0.0;                                             // Zone Dehumidifier Runtime Fraction [-]
        dehumid.OffCycleParasiticElecPower = 0.0;                             // Zone Dehumidifier Off-Cycle Parasitic Electric Power [W]
        dehumid.OffCycleParasiticElecCons = 0.0;                              // Zone Dehumidifier Off-Cycle Parasitic Electric Consumption [J]
        dehumid.DehumidCondVolFlowRate = 0.0;                                 // Zone Dehumidifier Condensate Volumetric Flow Rate [m3/s]
        dehumid.DehumidCondVol = 0.0;                                         // Zone Dehumidifier Condensate Volume [m3]
        dehumid.OutletAirTemp = state.dataLoopNodes->Node(AirInletNode).Temp; // Zone Dehumidifier Outlet Air Temperature [C]
    }

    void CalcZoneDehumidifier(EnergyPlusData &state,
                              int const ZoneDehumNum,     // Index number of the current zone dehumidifier being simulated
                              Real64 const QZnDehumidReq, // Dehumidification load to be met (kg/s), negative value means dehumidification load
                              Real64 &SensibleOutput,     // Sensible (heating) output (W), sent to load predictor for next simulation time step
                              Real64 &LatentOutput        // Latent (dehumidification) output provided (kg/s)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey, FSEC
        //       DATE WRITTEN   July/Aug 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the delivered capacity, electric energy consumption and water/condensate
        // removal rates for the zone dehumidifier.

        // METHODOLOGY EMPLOYED:
        // Cycle the dehumidifier as needed to meet the remaining zone dehumidification load.
        // Send excess sensible heat to zone energy balance (via SensibleOutput) for next HVAC time step,
        // so set the dehumidifier outlet air temp = inlet air temp to avoid double counting excess sensible.

        // REFERENCES:
        // na

        // Using/Aliasing
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyHfgAirFnWTdb;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhFnTdbWPb;
        using Psychrometrics::RhoH2O;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("CalcZoneDehumidifier");

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 WaterRemovalRateFactor; // Adjustment to  Rate Water Removal as a function of inlet air T and RH
        Real64 WaterRemovalVolRate;    // Actual water removal rate at current inlet air conditions (L/day)
        Real64 WaterRemovalMassRate;   // Actual water removal rate at current inlet air conditions (kg/s)
        Real64 EnergyFactorAdjFactor;  // Adjustment to Rate Energy Factor as a function of inlet air T and RH
        Real64 EnergyFactor;           // Actual Energy Factor as a function of inlet air T and RH
        Real64 InletAirTemp;           // Dry-bulb temperature of air entering the dehumidifier (C)
        Real64 InletAirHumRat;         // Humidity ratio of the air entering the dehumidifier (kg/kg)
        Real64 InletAirRH;             // Relative humidity of air entering the dehumidifier (%)
        Real64 OutletAirTemp;          // Dry-bulb temperature of air leaving the dehumidifier (C)
        Real64 OutletAirHumRat;        // Humidity ratio of air leaving the dehumidifier (kg/kg)
        Real64 PLR;                    // Part-load ratio = (dehumid load to be met)/(dehumid capacity of the dehumidifier)
        Real64 PLF;                    // Part-load fraction (-), RuntimeFraction = PLR/PLF
        Real64 RunTimeFraction;        // Dehumidifier runtime fraction (-)
        Real64 ElectricPowerOnCycle;   // Electric power when dehumidifier is operating (W)
        Real64 ElectricPowerAvg;       // Average electric power for this dehumidifier (W)
        Real64 hfg;                    // Enthalpy of evaporation of inlet air (J/kg)
        Real64 AirMassFlowRate;        // Air mass flow rate through this dehumidifier (kg/s)
        Real64 Cp;                     // Heat capacity of inlet air (J/kg-C)
        int AirInletNodeNum(0);        // Node number for the inlet air to the dehumidifier

        auto &dehumid = state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumNum);

        SensibleOutput = 0.0;
        LatentOutput = 0.0;
        WaterRemovalRateFactor = 0.0;
        AirMassFlowRate = 0.0;
        PLR = 0.0;
        PLF = 0.0;
        EnergyFactorAdjFactor = 0.0;
        RunTimeFraction = 0.0;
        ElectricPowerAvg = 0.0;
        ElectricPowerOnCycle = 0.0;

        AirInletNodeNum = dehumid.AirInletNodeNum;

        InletAirTemp = state.dataLoopNodes->Node(AirInletNodeNum).Temp;
        InletAirHumRat = state.dataLoopNodes->Node(AirInletNodeNum).HumRat;
        InletAirRH = 100.0 * PsyRhFnTdbWPb(state, InletAirTemp, InletAirHumRat, state.dataEnvrn->OutBaroPress, RoutineName); // RH in percent (%)

        if (QZnDehumidReq < 0.0 && dehumid.availSched->getCurrentVal() > 0.0 && InletAirTemp >= dehumid.MinInletAirTemp &&
            InletAirTemp <= dehumid.MaxInletAirTemp) {
            // A dehumidification load is being requested and dehumidifier is available (schedule value > 0)
            //  and the inlet air temperature is within the min/max values specified by user input

            WaterRemovalRateFactor = dehumid.WaterRemovalCurve->value(state, InletAirTemp, InletAirRH);
            // Warn user if curve output goes negative
            if (WaterRemovalRateFactor <= 0.0) {
                if (dehumid.WaterRemovalCurveErrorCount < 1) {
                    ++dehumid.WaterRemovalCurveErrorCount;
                    ShowWarningError(state, format("{} \"{}\":", dehumid.UnitType, dehumid.Name));
                    ShowContinueError(state, format(" Water Removal Rate Curve output is <= 0.0 ({:.5T}).", WaterRemovalRateFactor));
                    ShowContinueError(
                        state,
                        format(
                            " Negative value occurs using an inlet air dry-bulb temperature of {:.2T} and an inlet air relative humidity of {:.1T}.",
                            InletAirTemp,
                            InletAirRH));
                    ShowContinueErrorTimeStamp(state, " Dehumidifier turned off for this time step but simulation continues.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   dehumid.UnitType + " \"" + dehumid.Name +
                                                       "\": Water Removal Rate Curve output is <= 0.0 warning continues...",
                                                   dehumid.WaterRemovalCurveErrorIndex,
                                                   WaterRemovalRateFactor,
                                                   WaterRemovalRateFactor);
                }
                WaterRemovalRateFactor = 0.0;
            }

            WaterRemovalVolRate = WaterRemovalRateFactor * dehumid.RatedWaterRemoval;

            WaterRemovalMassRate =
                WaterRemovalVolRate / (Constant::rSecsInDay * 1000.0) *
                RhoH2O(max((InletAirTemp - 11.0), 1.0)); //(L/d)/(24 hr/day *3600 sec/hr * 1000 L/m3) | Density of water, minimum temp = 1.0C

            if (WaterRemovalMassRate > 0.0) {
                PLR = max(0.0, min(1.0, -QZnDehumidReq / WaterRemovalMassRate));
            } else {
                PLR = 0.0;
                RunTimeFraction = 0.0;
            }

            EnergyFactorAdjFactor = dehumid.EnergyFactorCurve->value(state, InletAirTemp, InletAirRH);

            // Warn user if curve output goes negative
            if (EnergyFactorAdjFactor <= 0.0) {
                if (dehumid.EnergyFactorCurveErrorCount < 1) {
                    ++dehumid.EnergyFactorCurveErrorCount;
                    ShowWarningError(state, format("{} \"{}\":", dehumid.UnitType, dehumid.Name));
                    ShowContinueError(state, format(" Energy Factor Curve output is <= 0.0 ({:.5T}).", EnergyFactorAdjFactor));
                    ShowContinueError(
                        state,
                        format(
                            " Negative value occurs using an inlet air dry-bulb temperature of {:.2T} and an inlet air relative humidity of {:.1T}.",
                            InletAirTemp,
                            InletAirRH));
                    ShowContinueErrorTimeStamp(state, " Dehumidifier turned off for this time step but simulation continues.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   dehumid.UnitType + " \"" + dehumid.Name +
                                                       "\": Energy Factor Curve output is <= 0.0 warning continues...",
                                                   dehumid.EnergyFactorCurveErrorIndex,
                                                   EnergyFactorAdjFactor,
                                                   EnergyFactorAdjFactor);
                }
                ElectricPowerAvg = 0.0;
                PLR = 0.0;
                RunTimeFraction = 0.0;
            } else {
                // EnergyFactorAdjFactor is not negative, so proceed with calculations
                EnergyFactor = EnergyFactorAdjFactor * dehumid.RatedEnergyFactor;

                if (dehumid.PartLoadCurve != nullptr) {
                    PLF = dehumid.PartLoadCurve->value(state, PLR); // Calculate part load fraction
                } else {
                    PLF = 1.0;
                }

                if (PLF < 0.7) {
                    if (dehumid.LowPLFErrorCount < 1) {
                        ++dehumid.LowPLFErrorCount;
                        ShowWarningError(state, format("{} \"{}\":", dehumid.UnitType, dehumid.Name));
                        ShowContinueError(
                            state, format(" The Part Load Fraction Correlation Curve output is ({:.2T}) at a part-load ratio ={:.3T}", PLF, PLR));
                        ShowContinueErrorTimeStamp(state,
                                                   " PLF curve values must be >= 0.7.  PLF has been reset to 0.7 and simulation is continuing.");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       dehumid.UnitType + " \"" + dehumid.Name +
                                                           "\": Part Load Fraction Correlation Curve output < 0.7 warning continues...",
                                                       dehumid.LowPLFErrorIndex,
                                                       PLF,
                                                       PLF);
                    }
                    PLF = 0.7;
                }

                if (PLF > 1.0) {
                    if (dehumid.HighPLFErrorCount < 1) {
                        ++dehumid.HighPLFErrorCount;
                        ShowWarningError(state, format("{} \"{}\":", dehumid.UnitType, dehumid.Name));
                        ShowContinueError(
                            state, format(" The Part Load Fraction Correlation Curve output is ({:.2T}) at a part-load ratio ={:.3T}", PLF, PLR));
                        ShowContinueErrorTimeStamp(state,
                                                   " PLF curve values must be < 1.0.  PLF has been reset to 1.0 and simulation is continuing.");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("{} \"{}\": Part Load Fraction Correlation Curve output > 1.0 warning continues...",
                                                              dehumid.UnitType,
                                                              dehumid.Name),
                                                       dehumid.HighPLFErrorIndex,
                                                       PLF,
                                                       PLF);
                    }
                    PLF = 1.0;
                }

                if (PLF > 0.0 && PLF >= PLR) {
                    RunTimeFraction = PLR / PLF; // Calculate dehumidifier runtime fraction
                } else {
                    if (dehumid.PLFPLRErrorCount < 1) {
                        ++dehumid.PLFPLRErrorCount;
                        ShowWarningError(state, format("{} \"{}\":", dehumid.UnitType, dehumid.Name));
                        ShowContinueError(
                            state,
                            format("The part load fraction was less than the part load ratio calculated for this time step [PLR={:.4T}, PLF={:.4T}].",
                                   PLR,
                                   PLF));
                        ShowContinueError(state, "Runtime fraction reset to 1 and the simulation will continue.");
                        ShowContinueErrorTimeStamp(state, "");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       dehumid.UnitType + " \"" + dehumid.Name +
                                                           "\": Part load fraction less than part load ratio warning continues...",
                                                       dehumid.PLFPLRErrorIndex);
                    }
                    RunTimeFraction = 1.0;
                }

                if (RunTimeFraction > 1.0 && std::abs(RunTimeFraction - 1.0) > 0.001) {
                    if (dehumid.HighRTFErrorCount < 1) {
                        ++dehumid.HighRTFErrorCount;
                        ShowWarningError(state, format("{} \"{}\":", dehumid.UnitType, dehumid.Name));
                        ShowContinueError(state, format("The runtime fraction for this zone dehumidifier exceeded 1.0 [{:.4T}].", RunTimeFraction));
                        ShowContinueError(state, "Runtime fraction reset to 1 and the simulation will continue.");
                        ShowContinueErrorTimeStamp(state, "");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       dehumid.UnitType + " \"" + dehumid.Name +
                                                           "\": Runtime fraction for zone dehumidifier exceeded 1.0 warning continues...",
                                                       dehumid.HighRTFErrorIndex,
                                                       RunTimeFraction,
                                                       RunTimeFraction);
                    }
                    RunTimeFraction = 1.0;
                }

                // ElectricPowerOnCycle = Water removal volumetric rate (L/day) / (Energy Factor(L/kWh) * 24 hrs/day ) * 1000 Wh/kWh
                ElectricPowerOnCycle = WaterRemovalVolRate / (EnergyFactor * 24.0) * 1000.0; // Watts
                // ElectricPowerAvg     = ElectricPowerOnCycle * RTF + (1-RTF)*OffCycleParsiticLoad
                ElectricPowerAvg = ElectricPowerOnCycle * RunTimeFraction + (1.0 - RunTimeFraction) * dehumid.OffCycleParasiticLoad; // average Watts
            }

            LatentOutput = WaterRemovalMassRate * PLR; // Average moisture removal rate, kg/s, for this timestep
            hfg = PsyHfgAirFnWTdb(InletAirHumRat, InletAirTemp);
            SensibleOutput = (LatentOutput * hfg) + ElectricPowerAvg; // Average sensible output, Watts
            // Send SensibleOutput to zone air heat balance via SysDepZoneLoads in ZoneEquipmentManager

            state.dataLoopNodes->Node(AirInletNodeNum).MassFlowRate = dehumid.RatedAirMassFlow * PLR;
            AirMassFlowRate = state.dataLoopNodes->Node(AirInletNodeNum).MassFlowRate; // Average air mass flow for this timestep
            Cp = PsyCpAirFnW(InletAirHumRat);                                          // Heat capacity of air
            if (AirMassFlowRate > 0.0 && Cp > 0.0) {
                OutletAirTemp = InletAirTemp + (ElectricPowerOnCycle + (WaterRemovalMassRate * hfg)) / (dehumid.RatedAirMassFlow * Cp);
                OutletAirHumRat = InletAirHumRat - LatentOutput / AirMassFlowRate;
            } else {
                OutletAirTemp = InletAirTemp;
                OutletAirHumRat = InletAirHumRat;
            }

        } else {

            // No load or not available or inlet air temps beyond min/max limits, then set outlet conditions
            // equal to inlet conditions and PLR = RTF = 0.0
            OutletAirTemp = InletAirTemp;
            OutletAirHumRat = InletAirHumRat;
            PLR = 0.0;
            RunTimeFraction = 0.0;
            state.dataLoopNodes->Node(AirInletNodeNum).MassFlowRate = 0.0;
            // If available but didn't operate, then set electric power = off cycle parasitic load.
            // Else, electric power = 0.0
            if (dehumid.availSched->getCurrentVal() > 0.0) {
                ElectricPowerAvg = dehumid.OffCycleParasiticLoad; // off cycle parasitic is on entire timestep
            } else {
                ElectricPowerAvg = 0.0;
            }
        }

        dehumid.OutletAirTemp = OutletAirTemp; // Update report variable here. Node outlet Temp set equal
        //   to Node inlet Temp in Update subroutine
        dehumid.OutletAirHumRat = OutletAirHumRat; // Store in structure, updated outlet node in Update subroutine

        // Use inlet air temperature in outlet air enthalpy calculation... since the sensible heat output
        // from the dehumidifier is being sent directly to the zone air heat balance for next hvac simulation time step
        dehumid.OutletAirEnthalpy = PsyHFnTdbW(InletAirTemp, OutletAirHumRat);

        dehumid.SensHeatingRate = SensibleOutput; // Report variable update, W,  avg sens output when unit is 'on'
        dehumid.WaterRemovalRate = LatentOutput;  // Report variable update, kg/s
        LatentOutput = -LatentOutput;             // change sign... negative is dehumidification in zone air balance

        dehumid.OffCycleParasiticElecPower = (1.0 - RunTimeFraction) * dehumid.OffCycleParasiticLoad;
        dehumid.ElecPower = ElectricPowerAvg;
        dehumid.DehumidPLR = PLR;
        dehumid.DehumidRTF = RunTimeFraction;
    }

    void UpdateZoneDehumidifier(EnergyPlusData &state, int const ZoneDehumNum) // Number of the current zone dehumidifier being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey, FSEC
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for passing results to the outlet air node.

        auto &dehumid = state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumNum);
        auto &airInletNode = state.dataLoopNodes->Node(dehumid.AirInletNodeNum);
        auto &airOutletNode = state.dataLoopNodes->Node(dehumid.AirOutletNodeNum);

        // Changed outlet node properties
        airOutletNode.Enthalpy = dehumid.OutletAirEnthalpy;
        airOutletNode.HumRat = dehumid.OutletAirHumRat;
        // Set outlet temp = inlet temp; send excess sensible heat directly to air heat balance
        // (via SensibleOutput and QSensOut) for the next hvac simulation time step.
        airOutletNode.Temp = airInletNode.Temp;

        // Pass through output node properties
        airOutletNode.Quality = airInletNode.Quality;
        airOutletNode.Press = airInletNode.Press;
        airOutletNode.MassFlowRate = airInletNode.MassFlowRate;
        airOutletNode.MassFlowRateMin = airInletNode.MassFlowRateMin;
        airOutletNode.MassFlowRateMax = airInletNode.MassFlowRateMax;
        airOutletNode.MassFlowRateMinAvail = airInletNode.MassFlowRateMinAvail;
        airOutletNode.MassFlowRateMaxAvail = airInletNode.MassFlowRateMaxAvail;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airOutletNode.CO2 = airInletNode.CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airOutletNode.GenContam = airInletNode.GenContam;
        }
    }

    void ReportZoneDehumidifier(EnergyPlusData &state, int const DehumidNum) // Index of the current zone dehumidifier being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey, FSEC
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Fills some of the report variables for the zone dehumidifiers

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        using Psychrometrics::RhoH2O;

        // DERIVED TYPE DEFINITIONS:
        // na

        auto &dehumid = state.dataZoneDehumidifier->ZoneDehumid(DehumidNum);

        dehumid.SensHeatingEnergy = dehumid.SensHeatingRate * TimeStepSysSec;
        dehumid.WaterRemoved = dehumid.WaterRemovalRate * TimeStepSysSec;
        dehumid.ElecConsumption = dehumid.ElecPower * TimeStepSysSec;
        dehumid.OffCycleParasiticElecCons = dehumid.OffCycleParasiticElecPower * TimeStepSysSec;

        // Dehumidifier water collection to water storage tank (if needed)
        if (dehumid.CondensateCollectMode == CondensateOutlet::ToTank) {
            // Calculate and report condensation rate (how much water extracted from the air stream)
            // Volumetric flow of water in m3/s for water system interactions

            int AirInletNodeNum = dehumid.AirInletNodeNum;
            Real64 InletAirTemp = state.dataLoopNodes->Node(AirInletNodeNum).Temp;
            Real64 OutletAirTemp = max((InletAirTemp - 11.0), 1.0); // Assume coil outlet air is 11C (20F) lower than inlet air temp
            Real64 RhoWater = RhoH2O(OutletAirTemp);                // Density of water, minimum temp = 1.0 C

            if (RhoWater > 0.0) {
                dehumid.DehumidCondVolFlowRate = dehumid.WaterRemovalRate / RhoWater;
            }

            dehumid.DehumidCondVol = dehumid.DehumidCondVolFlowRate * TimeStepSysSec;

            state.dataWaterData->WaterStorage(dehumid.CondensateTankID).VdotAvailSupply(dehumid.CondensateTankSupplyARRID) =
                dehumid.DehumidCondVolFlowRate;
            // Assume water outlet temp = air outlet temp.... same assumption in other places in code (e.g., water coil component)
            state.dataWaterData->WaterStorage(dehumid.CondensateTankID).TwaterSupply(dehumid.CondensateTankSupplyARRID) = OutletAirTemp;
        }
    }

    bool GetZoneDehumidifierNodeNumber(EnergyPlusData &state, int const NodeNumber) // Node being tested
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // After making sure get input is done, the node number of indicated
        // zone dehumidifier is returned.

        // Return value
        bool FindZoneDehumidifierNodeNumber; // Zone Dehumidifier Node Number Check

        if (state.dataZoneDehumidifier->GetInputFlag) {
            GetZoneDehumidifierInput(state);
            state.dataZoneDehumidifier->GetInputFlag = false;
        }

        FindZoneDehumidifierNodeNumber = false;
        for (int ZoneDehumidIndex = 1; ZoneDehumidIndex <= (int)state.dataZoneDehumidifier->ZoneDehumid.size(); ++ZoneDehumidIndex) {
            if (NodeNumber == state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidIndex).AirInletNodeNum) {
                FindZoneDehumidifierNodeNumber = true;
                break;
            }
            if (NodeNumber == state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidIndex).AirOutletNodeNum) {
                FindZoneDehumidifierNodeNumber = true;
                break;
            }
        }

        return FindZoneDehumidifierNodeNumber;
    }

    int getZoneDehumidifierIndex(EnergyPlusData &state, std::string_view CompName)
    {
        if (state.dataZoneDehumidifier->GetInputFlag) {
            GetZoneDehumidifierInput(state);
            state.dataZoneDehumidifier->GetInputFlag = false;
        }

        for (int ZoneDehumidNum = 1; ZoneDehumidNum <= (int)state.dataZoneDehumidifier->ZoneDehumid.size(); ++ZoneDehumidNum) {
            if (Util::SameString(state.dataZoneDehumidifier->ZoneDehumid(ZoneDehumidNum).Name, CompName)) {
                return ZoneDehumidNum;
            }
        }

        return 0;
    }

} // namespace ZoneDehumidifier

} // namespace EnergyPlus
