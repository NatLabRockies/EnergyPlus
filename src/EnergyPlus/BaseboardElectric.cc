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

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BaseboardElectric.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace BaseboardElectric {
    // Module containing the routines dealing with the BASEBOARD Electric HEATER
    // component(s).

    // MODULE INFORMATION:  Richard Liesen
    //       DATE WRITTEN   Nov 2001
    //       RE-ENGINEERED  na

    // MODULE PARAMETER DEFINITIONS
    constexpr std::string_view cCMO_BBRadiator_Electric = "ZoneHVAC:Baseboard:Convective:Electric";
    constexpr Real64 SimpConvAirFlowSpeed(0.5); // m/s

    void SimElectricBaseboard(EnergyPlusData &state, std::string const &EquipName, int const ControlledZoneNum, Real64 &PowerMet, int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the Electric Baseboard units.

        int BaseboardNum; // index of unit in baseboard array
        Real64 QZnReq;    // zone load not yet satisfied

        if (state.dataBaseboardElectric->getInputFlag) {
            GetBaseboardInput(state);
            state.dataBaseboardElectric->getInputFlag = false;
        }

        auto &baseboard = state.dataBaseboardElectric;

        // Find the correct Baseboard Equipment
        if (CompIndex == 0) {
            BaseboardNum = Util::FindItemInList(EquipName, baseboard->baseboards, &BaseboardParams::EquipName);
            if (BaseboardNum == 0) {
                ShowFatalError(state, format("SimElectricBaseboard: Unit not found={}", EquipName));
            }
            CompIndex = BaseboardNum;
        } else {
            BaseboardNum = CompIndex;
            int numBaseboards = (int)baseboard->baseboards.size();
            if (BaseboardNum > numBaseboards || BaseboardNum < 1) {
                ShowFatalError(state,
                               format("SimElectricBaseboard:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      BaseboardNum,
                                      numBaseboards,
                                      EquipName));
            }
            if (baseboard->baseboards(BaseboardNum).CheckEquipName) {
                if (EquipName != baseboard->baseboards(BaseboardNum).EquipName) {
                    ShowFatalError(state,
                                   format("SimElectricBaseboard: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          BaseboardNum,
                                          EquipName,
                                          baseboard->baseboards(BaseboardNum).EquipName));
                }
                baseboard->baseboards(BaseboardNum).CheckEquipName = false;
            }
        }

        InitBaseboard(state, BaseboardNum, ControlledZoneNum);

        QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;

        // Simulate baseboard
        SimElectricConvective(state, BaseboardNum, QZnReq);

        PowerMet = baseboard->baseboards(BaseboardNum).Power;

        baseboard->baseboards(BaseboardNum).Energy = baseboard->baseboards(BaseboardNum).Power * state.dataHVACGlobal->TimeStepSysSec;
        baseboard->baseboards(BaseboardNum).ElecUseLoad = baseboard->baseboards(BaseboardNum).ElecUseRate * state.dataHVACGlobal->TimeStepSysSec;
    }

    void GetBaseboardInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the input for the Baseboard units.

        // METHODOLOGY EMPLOYED:
        // Standard input processor calls.

        // Using/Aliasing
        using DataSizing::AutoSize;
        using DataSizing::CapacityPerFloorArea;
        using DataSizing::FractionOfAutosizedHeatingCapacity;
        using DataSizing::HeatingDesignCapacity;
        using GlobalNames::VerifyUniqueBaseboardName;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetBaseboardInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetBaseboardInput";
        int constexpr iHeatCAPMAlphaNum(3);                   // get input index to baseboard heating capacity sizing method
        int constexpr iHeatDesignCapacityNumericNum(1);       // get input index to baseboard heating capacity
        int constexpr iHeatCapacityPerFloorAreaNumericNum(2); // get input index to baseboard heating capacity per floor area sizing
        int constexpr iHeatFracOfAutosizedCapacityNumericNum(
            3); //  get input index to baseboard heating capacity sizing as fraction of autosized heating capacity

        auto &s_ipsc = state.dataIPShortCut;

        auto &baseboard = state.dataBaseboardElectric;
        std::string_view cCurrentModuleObject = cCMO_BBRadiator_Electric;

        int NumConvElecBaseboards = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        baseboard->baseboards.allocate(NumConvElecBaseboards);

        if (NumConvElecBaseboards > 0) { // Get the data for cooling schemes
            bool ErrorsFound(false);     // If errors detected in input
            int NumAlphas = 0;
            int NumNums = 0;
            int IOStat = 0;
            int BaseboardNum = 0;
            for (int ConvElecBBNum = 1; ConvElecBBNum <= NumConvElecBaseboards; ++ConvElecBBNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         ConvElecBBNum,
                                                                         s_ipsc->cAlphaArgs,
                                                                         NumAlphas,
                                                                         s_ipsc->rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         s_ipsc->lNumericFieldBlanks,
                                                                         s_ipsc->lAlphaFieldBlanks,
                                                                         s_ipsc->cAlphaFieldNames,
                                                                         s_ipsc->cNumericFieldNames);

                baseboard->baseboards(ConvElecBBNum).FieldNames.assign(s_ipsc->cNumericFieldNames.begin(), s_ipsc->cNumericFieldNames.end());

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

                // ErrorsFound will be set to True if problem was found, left untouched otherwise
                VerifyUniqueBaseboardName(state, cCurrentModuleObject, s_ipsc->cAlphaArgs(1), ErrorsFound, format("{} Name", cCurrentModuleObject));

                ++BaseboardNum;
                auto &thisBaseboard = baseboard->baseboards(BaseboardNum);
                thisBaseboard.EquipName = s_ipsc->cAlphaArgs(1);                 // name of this baseboard
                thisBaseboard.EquipType = Util::makeUPPER(cCurrentModuleObject); // the type of baseboard-rename change
                thisBaseboard.Schedule = s_ipsc->cAlphaArgs(2);
                if (s_ipsc->lAlphaFieldBlanks(2)) {
                    thisBaseboard.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisBaseboard.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                }
                // get inlet node number
                thisBaseboard.BaseboardEfficiency = s_ipsc->rNumericArgs(4);

                // Determine baseboard electric heating design capacity sizing method
                if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "HeatingDesignCapacity")) {
                    thisBaseboard.HeatingCapMethod = HeatingDesignCapacity;
                    if (!s_ipsc->lNumericFieldBlanks(iHeatDesignCapacityNumericNum)) {
                        thisBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatDesignCapacityNumericNum);
                        if (thisBaseboard.ScaledHeatingCapacity < 0.0 && thisBaseboard.ScaledHeatingCapacity != AutoSize) {
                            ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                            ShowContinueError(state,
                                              format("Illegal {} = {:.7T}",
                                                     s_ipsc->cNumericFieldNames(iHeatDesignCapacityNumericNum),
                                                     s_ipsc->rNumericArgs(iHeatDesignCapacityNumericNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                        ShowContinueError(
                            state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(iHeatDesignCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "CapacityPerFloorArea")) {
                    thisBaseboard.HeatingCapMethod = CapacityPerFloorArea;
                    if (!s_ipsc->lNumericFieldBlanks(iHeatCapacityPerFloorAreaNumericNum)) {
                        thisBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum);
                        if (thisBaseboard.ScaledHeatingCapacity <= 0.0) {
                            ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                            ShowContinueError(
                                state,
                                format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                            ShowContinueError(state,
                                              format("Illegal {} = {:.7T}",
                                                     s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum),
                                                     s_ipsc->rNumericArgs(iHeatCapacityPerFloorAreaNumericNum)));
                            ErrorsFound = true;
                        } else if (thisBaseboard.ScaledHeatingCapacity == AutoSize) {
                            ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                            ShowContinueError(
                                state,
                                format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                            ShowContinueError(state,
                                              format("Illegal {} = AutoSize", s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                        ShowContinueError(
                            state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(state,
                                          format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(iHeatCapacityPerFloorAreaNumericNum)));
                        ErrorsFound = true;
                    }
                } else if (Util::SameString(s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum), "FractionOfAutosizedHeatingCapacity")) {
                    thisBaseboard.HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
                    if (!s_ipsc->lNumericFieldBlanks(iHeatFracOfAutosizedCapacityNumericNum)) {
                        thisBaseboard.ScaledHeatingCapacity = s_ipsc->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum);
                        if (thisBaseboard.ScaledHeatingCapacity < 0.0) {
                            ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                            ShowContinueError(state,
                                              format("Illegal {} = {:.7T}",
                                                     s_ipsc->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum),
                                                     s_ipsc->rNumericArgs(iHeatFracOfAutosizedCapacityNumericNum)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                        ShowContinueError(
                            state, format("Input for {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                        ShowContinueError(
                            state, format("Blank field not allowed for {}", s_ipsc->cNumericFieldNames(iHeatFracOfAutosizedCapacityNumericNum)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", cCurrentModuleObject, thisBaseboard.EquipName));
                    ShowContinueError(state,
                                      format("Illegal {} = {}", s_ipsc->cAlphaFieldNames(iHeatCAPMAlphaNum), s_ipsc->cAlphaArgs(iHeatCAPMAlphaNum)));
                    ErrorsFound = true;
                }

                thisBaseboard.ZonePtr = DataZoneEquipment::GetZoneEquipControlledZoneNum(
                    state, DataZoneEquipment::ZoneEquipType::BaseboardConvectiveElectric, thisBaseboard.EquipName);
            }

            if (ErrorsFound) {
                ShowFatalError(state, format("{} Errors found in getting input.  Preceding condition(s) cause termination.", RoutineName));
            }
        }

        for (int BaseboardNum = 1; BaseboardNum <= NumConvElecBaseboards; ++BaseboardNum) {

            // Setup Report variables for the Electric Baseboards
            // CurrentModuleObject='ZoneHVAC:Baseboard:Convective:Electric'

            auto &thisBaseboard = baseboard->baseboards(BaseboardNum);
            SetupOutputVariable(state,
                                "Baseboard Total Heating Energy",
                                Constant::Units::J,
                                thisBaseboard.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisBaseboard.EquipName,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Baseboard);

            SetupOutputVariable(state,
                                "Baseboard Total Heating Rate",
                                Constant::Units::W,
                                thisBaseboard.Power,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipName);

            SetupOutputVariable(state,
                                "Baseboard Electricity Energy",
                                Constant::Units::J,
                                thisBaseboard.ElecUseLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisBaseboard.EquipName,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);

            SetupOutputVariable(state,
                                "Baseboard Electricity Rate",
                                Constant::Units::W,
                                thisBaseboard.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisBaseboard.EquipName);
        }
    }

    void InitBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes the Baseboard units during simulation.

        auto &baseboard = state.dataBaseboardElectric;

        if (!state.dataGlobal->SysSizingCalc && baseboard->baseboards(BaseboardNum).MySizeFlag) {
            // for each coil, do the sizing once.
            SizeElectricBaseboard(state, BaseboardNum);
            baseboard->baseboards(BaseboardNum).MySizeFlag = false;
        }

        // Set the reporting variables to zero at each timestep.
        baseboard->baseboards(BaseboardNum).Energy = 0.0;
        baseboard->baseboards(BaseboardNum).Power = 0.0;
        baseboard->baseboards(BaseboardNum).ElecUseLoad = 0.0;
        baseboard->baseboards(BaseboardNum).ElecUseRate = 0.0;

        // Do the every time step initializations
        int ZoneNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
        baseboard->baseboards(BaseboardNum).AirInletTemp = state.dataLoopNodes->Node(ZoneNode).Temp;
        baseboard->baseboards(BaseboardNum).AirInletHumRat = state.dataLoopNodes->Node(ZoneNode).HumRat;
    }

    void SizeElectricBaseboard(EnergyPlusData &state, int const BaseboardNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing electric baseboard components for which nominal capacities have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data. UAs are
        // calculated by numerically inverting the baseboard calculation routine.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeElectricBaseboard");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TempSize; // autosized value of coil input field
        state.dataSize->DataScalableCapSizingON = false;

        if (state.dataSize->CurZoneEqNum > 0) {
            auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
            auto &baseboard = state.dataBaseboardElectric->baseboards(BaseboardNum);

            std::string_view const CompType = baseboard.EquipType;
            std::string_view const CompName = baseboard.EquipName;
            state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
            state.dataSize->DataZoneNumber = baseboard.ZonePtr;
            int SizingMethod = HVAC::HeatingCapacitySizing;
            int FieldNum = 1;
            std::string const SizingString = format("{} [W]", baseboard.FieldNames[FieldNum - 1]);
            int CapSizingMethod = baseboard.HeatingCapMethod;
            ZoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
            if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                    if (baseboard.ScaledHeatingCapacity == DataSizing::AutoSize) {
                        CheckZoneSizing(state, CompType, CompName);
                        ZoneEqSizing.HeatingCapacity = true;
                        ZoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    }
                    TempSize = baseboard.ScaledHeatingCapacity;
                } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                    ZoneEqSizing.HeatingCapacity = true;
                    ZoneEqSizing.DesHeatingLoad = baseboard.ScaledHeatingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                    TempSize = ZoneEqSizing.DesHeatingLoad;
                    state.dataSize->DataScalableCapSizingON = true;
                } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                    CheckZoneSizing(state, CompType, CompName);
                    ZoneEqSizing.HeatingCapacity = true;
                    state.dataSize->DataFracOfAutosizedHeatingCapacity = baseboard.ScaledHeatingCapacity;
                    ZoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
                    TempSize = DataSizing::AutoSize;
                    state.dataSize->DataScalableCapSizingON = true;
                } else {
                    TempSize = baseboard.ScaledHeatingCapacity;
                }
                bool PrintFlag = true; // TRUE when sizing information is reported in the eio file
                bool errorsFound = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                baseboard.NominalCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                state.dataSize->DataScalableCapSizingON = false;
            }
        }
    }

    void SimElectricConvective(EnergyPlusData &state, int const BaseboardNum, Real64 const LoadMet)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Nov 2001

        // PURPOSE OF THIS SUBROUTINE: This subroutine calculates the heat exchange rate
        // in a pure Electricconvective baseboard heater.

        // METHODOLOGY EMPLOYED:
        // Currently this is primarily modified from HW Convective baseboard which has connections to
        //  a water loop and was necessary to calculate temps, flow rates and other things.  This
        //  model might be made more sophisticated and might use some of those data structures in the future
        //  so they are left in place even though this model does not utilize them.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AirOutletTemp;
        Real64 QBBCap;

        auto &baseboard = state.dataBaseboardElectric->baseboards(BaseboardNum);

        Real64 AirInletTemp = baseboard.AirInletTemp;
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(baseboard.AirInletHumRat);
        Real64 AirMassFlowRate = SimpConvAirFlowSpeed;
        Real64 CapacitanceAir = CpAir * AirMassFlowRate;
        // currently only the efficiency is used to calculate the electric consumption.  There could be some
        //  thermal loss that could be accounted for with this efficiency input.
        Real64 Effic = baseboard.BaseboardEfficiency;

        if (baseboard.availSched->getCurrentVal() > 0.0 && LoadMet >= HVAC::SmallLoad) {

            // if the load exceeds the capacity than the capacity is set to the BB limit.
            if (LoadMet > baseboard.NominalCapacity) {
                QBBCap = baseboard.NominalCapacity;
            } else {
                QBBCap = LoadMet;
            }

            // this could be utilized somehow or even reported so the data structures are left in place
            AirOutletTemp = AirInletTemp + QBBCap / CapacitanceAir;

            // The Baseboard electric Load is calculated using the efficiency
            baseboard.ElecUseRate = QBBCap / Effic;

        } else {
            // if there is an off condition the BB does nothing.
            AirOutletTemp = AirInletTemp;
            QBBCap = 0.0;
            baseboard.ElecUseRate = 0.0;
        }

        baseboard.AirOutletTemp = AirOutletTemp;
        baseboard.Power = QBBCap;
    }

} // namespace BaseboardElectric

} // namespace EnergyPlus
