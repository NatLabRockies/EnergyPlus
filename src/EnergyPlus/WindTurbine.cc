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
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/CommandLineInterface.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StringUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindTurbine.hh>

namespace EnergyPlus {

// (ref: Object: Generator:WindTurbine)

namespace WindTurbine {
    // Module containing the data for wind turbine system

    // MODULE INFORMATION:
    //       AUTHOR         Daeho Kang
    //       DATE WRITTEN   October 2009

    // PURPOSE OF THIS MODULE:
    // This module is to calculate the electrical power output that wind turbine systems produce.
    // Both horizontal and vertical axis wind turbine systems are modeled.

    // REFERENCES:
    // Sathyajith Mathew. 2006. Wind Energy: Fundamental, Resource Analysis and Economics. Springer,
    //     Chap. 2, pp. 11-15
    // Mazharul Islam, David S.K. Ting, and Amir Fartaj. 2008. Aerodynamic Models for Darrieus-type sSraight-bladed
    //     Vertical Axis Wind Turbines. Renewable & Sustainable Energy Reviews, Volume 12, pp.1087-1109

    constexpr std::array<std::string_view, static_cast<int>(ControlType::Num)> ControlNamesUC{
        "FIXEDSPEEDFIXEDPITCH",
        "FIXEDSPEEDVARIABLEPITCH",
        "VARIABLESPEEDFIXEDPITCH",
        "VARIABLESPEEDVARIABLEPITCH",
    };

    constexpr std::array<std::string_view, static_cast<int>(RotorType::Num)> RotorNamesUC{
        "HORIZONTALAXISWINDTURBINE",
        "VERTICALAXISWINDTURBINE",
    };

    void SimWindTurbine(EnergyPlusData &state,
                        [[maybe_unused]] GeneratorType const GeneratorType, // Type of Generator
                        std::string const &GeneratorName,                   // User specified name of Generator
                        int &GeneratorIndex,                                // Generator index
                        bool const RunFlag,                                 // ON or OFF
                        [[maybe_unused]] Real64 const WTLoad                // Electrical load on WT (not used)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   October 2009

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the simulation of wind turbine component.
        // This drivers manages the calls to all of the other drivers and simulation algorithms.

        // Using/Aliasing

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int WindTurbineNum;
        // Obtains and allocates heat balance related parameters from input

        if (state.dataWindTurbine->GetInputFlag) {
            GetWindTurbineInput(state);
            state.dataWindTurbine->GetInputFlag = false;
        }

        if (GeneratorIndex == 0) {
            WindTurbineNum = Util::FindItemInList(GeneratorName, state.dataWindTurbine->WindTurbineSys);
            if (WindTurbineNum == 0) {
                ShowFatalError(state, format("SimWindTurbine: Specified Generator not one of Valid Wind Turbine Generators {}", GeneratorName));
            }
            GeneratorIndex = WindTurbineNum;
        } else {
            WindTurbineNum = GeneratorIndex;
            int NumWindTurbines = (int)state.dataWindTurbine->WindTurbineSys.size();
            if (WindTurbineNum > NumWindTurbines || WindTurbineNum < 1) {
                ShowFatalError(state,
                               format("SimWindTurbine: Invalid GeneratorIndex passed={}, Number of Wind Turbine Generators={}, Generator name={}",
                                      WindTurbineNum,
                                      NumWindTurbines,
                                      GeneratorName));
            }
            if (GeneratorName != state.dataWindTurbine->WindTurbineSys(WindTurbineNum).Name) {
                ShowFatalError(state,
                               format("SimMWindTurbine: Invalid GeneratorIndex passed={}, Generator name={}, stored Generator Name for that index={}",
                                      WindTurbineNum,
                                      GeneratorName,
                                      state.dataWindTurbine->WindTurbineSys(WindTurbineNum).Name));
            }
        }

        InitWindTurbine(state, WindTurbineNum);

        CalcWindTurbine(state, WindTurbineNum, RunFlag);

        ReportWindTurbine(state, WindTurbineNum);
    }

    void GetWTGeneratorResults(EnergyPlusData &state,
                               [[maybe_unused]] GeneratorType const GeneratorType, // Type of Generator
                               int const GeneratorIndex,                           // Generator number
                               Real64 &GeneratorPower,                             // Electrical power
                               Real64 &GeneratorEnergy,                            // Electrical energy
                               Real64 &ThermalPower,
                               Real64 &ThermalEnergy)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Aug. 2008
        //       MODIFIED       D Kang, October 2009 for Wind Turbine

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine provides a "get" method to collect results for individual electric load centers.

        GeneratorPower = state.dataWindTurbine->WindTurbineSys(GeneratorIndex).Power;
        GeneratorEnergy = state.dataWindTurbine->WindTurbineSys(GeneratorIndex).Energy;

        // Thermal energy is ignored
        ThermalPower = 0.0;
        ThermalEnergy = 0.0;
    }

    void GetWindTurbineInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   October 2009

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets input data for wind turbine components
        // and stores it in the wind turbine data structure.

        static constexpr std::string_view routineName = "GetWindTurbineInput";

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const CurrentModuleObject("Generator:WindTurbine");
        Real64 constexpr SysEffDefault(0.835); // Default value of overall system efficiency
        Real64 constexpr MaxTSR(12.0);         // Maximum tip speed ratio
        Real64 constexpr DefaultPC(0.25);      // Default power coefficient
        Real64 constexpr MaxPowerCoeff(0.59);  // Maximum power coefficient
        Real64 constexpr DefaultH(50.0);       // Default of height for local wind speed

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // If errors detected in input
        int WindTurbineNum;      // Wind turbine number
        int NumAlphas;           // Number of Alphas for each GetobjectItem call
        int NumNumbers;          // Number of Numbers for each GetobjectItem call
        int NumArgs;
        int IOStat;
        Array1D_string cAlphaArgs;     // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> rNumericArgs;  // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.

        // Initializations and allocations
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumArgs, NumAlphas, NumNumbers);
        cAlphaArgs.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        rNumericArgs.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        int NumWindTurbines = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        state.dataWindTurbine->WindTurbineSys.allocate(NumWindTurbines);

        for (WindTurbineNum = 1; WindTurbineNum <= NumWindTurbines; ++WindTurbineNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     WindTurbineNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), CurrentModuleObject, ErrorsFound);

            auto &windTurbine = state.dataWindTurbine->WindTurbineSys(WindTurbineNum);

            windTurbine.Name = state.dataIPShortCut->cAlphaArgs(1); // Name of wind turbine

            if (lAlphaBlanks(2)) {
                windTurbine.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((windTurbine.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }
            // Select rotor type
            windTurbine.rotorType =
                static_cast<RotorType>(getEnumValue(WindTurbine::RotorNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(3))));
            if (windTurbine.rotorType == RotorType::Invalid) {
                if (state.dataIPShortCut->cAlphaArgs(3).empty()) {
                    windTurbine.rotorType = RotorType::HorizontalAxis;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\".",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cAlphaFields(3),
                                           state.dataIPShortCut->cAlphaArgs(3)));
                    ErrorsFound = true;
                }
            }

            // Select control type
            windTurbine.controlType =
                static_cast<ControlType>(getEnumValue(WindTurbine::ControlNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(4))));
            if (windTurbine.controlType == ControlType::Invalid) {
                if (state.dataIPShortCut->cAlphaArgs(4).empty()) {
                    windTurbine.controlType = ControlType::VariableSpeedVariablePitch;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\".",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cAlphaFields(4),
                                           state.dataIPShortCut->cAlphaArgs(4)));
                    ErrorsFound = true;
                }
            }

            windTurbine.RatedRotorSpeed = state.dataIPShortCut->rNumericArgs(1); // Maximum rotor speed in rpm
            if (windTurbine.RatedRotorSpeed <= 0.0) {
                if (lNumericBlanks(1)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(1)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(1),
                                           rNumericArgs(1)));
                }
                ErrorsFound = true;
            }

            windTurbine.RotorDiameter = state.dataIPShortCut->rNumericArgs(2); // Rotor diameter in m
            if (windTurbine.RotorDiameter <= 0.0) {
                if (lNumericBlanks(2)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(2)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.1R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(2),
                                           rNumericArgs(2)));
                }
                ErrorsFound = true;
            }

            windTurbine.RotorHeight = state.dataIPShortCut->rNumericArgs(3); // Overall height of the rotor
            if (windTurbine.RotorHeight <= 0.0) {
                if (lNumericBlanks(3)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(3)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.1R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(3),
                                           rNumericArgs(3)));
                }
                ErrorsFound = true;
            }

            windTurbine.NumOfBlade = state.dataIPShortCut->rNumericArgs(4); // Total number of blade
            if (windTurbine.NumOfBlade == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\" invalid {}=[{:.0R}] must be greater than zero.",
                                       CurrentModuleObject,
                                       state.dataIPShortCut->cAlphaArgs(1),
                                       cNumericFields(4),
                                       rNumericArgs(4)));
                ErrorsFound = true;
            }

            windTurbine.RatedPower = state.dataIPShortCut->rNumericArgs(5); // Rated average power
            if (windTurbine.RatedPower == 0.0) {
                if (lNumericBlanks(5)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(5)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(5),
                                           rNumericArgs(5)));
                }
                ErrorsFound = true;
            }

            windTurbine.RatedWindSpeed = state.dataIPShortCut->rNumericArgs(6); // Rated wind speed
            if (windTurbine.RatedWindSpeed == 0.0) {
                if (lNumericBlanks(6)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(6)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(6),
                                           rNumericArgs(6)));
                }
                ErrorsFound = true;
            }

            windTurbine.CutInSpeed = state.dataIPShortCut->rNumericArgs(7); // Minimum wind speed for system operation
            if (windTurbine.CutInSpeed == 0.0) {
                if (lNumericBlanks(7)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(7)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(7),
                                           rNumericArgs(7)));
                }
                ErrorsFound = true;
            }

            windTurbine.CutOutSpeed = state.dataIPShortCut->rNumericArgs(8); // Minimum wind speed for system operation
            if (windTurbine.CutOutSpeed == 0.0) {
                if (lNumericBlanks(8)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(8)));
                } else if (windTurbine.CutOutSpeed <= windTurbine.RatedWindSpeed) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than {}=[{:.2R}].",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(8),
                                           rNumericArgs(8),
                                           cNumericFields(6),
                                           rNumericArgs(6)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(8),
                                           rNumericArgs(8)));
                }
                ErrorsFound = true;
            }

            windTurbine.SysEfficiency = state.dataIPShortCut->rNumericArgs(9); // Overall wind turbine system efficiency
            if (lNumericBlanks(9) || windTurbine.SysEfficiency == 0.0 || windTurbine.SysEfficiency > 1.0) {
                windTurbine.SysEfficiency = SysEffDefault;
                ShowWarningError(state,
                                 format("{}=\"{}\" invalid {}=[{:.2R}].",
                                        CurrentModuleObject,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        cNumericFields(9),
                                        state.dataIPShortCut->rNumericArgs(9)));
                ShowContinueError(state, format("...The default value of {:.3R} was assumed. for {}", SysEffDefault, cNumericFields(9)));
            }

            windTurbine.MaxTipSpeedRatio = state.dataIPShortCut->rNumericArgs(10); // Maximum tip speed ratio
            if (windTurbine.MaxTipSpeedRatio == 0.0) {
                if (lNumericBlanks(10)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(10)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(10),
                                           rNumericArgs(10)));
                }
                ErrorsFound = true;
            }
            if (windTurbine.SysEfficiency > MaxTSR) {
                windTurbine.SysEfficiency = MaxTSR;
                ShowWarningError(state,
                                 format("{}=\"{}\" invalid {}=[{:.2R}].",
                                        CurrentModuleObject,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        cNumericFields(10),
                                        state.dataIPShortCut->rNumericArgs(10)));
                ShowContinueError(state, format("...The default value of {:.1R} was assumed. for {}", MaxTSR, cNumericFields(10)));
            }

            windTurbine.MaxPowerCoeff = state.dataIPShortCut->rNumericArgs(11); // Maximum power coefficient
            if (windTurbine.rotorType == RotorType::HorizontalAxis && windTurbine.MaxPowerCoeff == 0.0) {
                if (lNumericBlanks(11)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(11)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(11),
                                           rNumericArgs(11)));
                }
                ErrorsFound = true;
            }
            if (windTurbine.MaxPowerCoeff > MaxPowerCoeff) {
                windTurbine.MaxPowerCoeff = DefaultPC;
                ShowWarningError(state,
                                 format("{}=\"{}\" invalid {}=[{:.2R}].",
                                        CurrentModuleObject,
                                        state.dataIPShortCut->cAlphaArgs(1),
                                        cNumericFields(11),
                                        state.dataIPShortCut->rNumericArgs(11)));
                ShowContinueError(state, format("...The default value of {:.2R} will be used. for {}", DefaultPC, cNumericFields(11)));
            }

            windTurbine.LocalAnnualAvgWS = state.dataIPShortCut->rNumericArgs(12); // Local wind speed annually averaged
            if (windTurbine.LocalAnnualAvgWS == 0.0) {
                if (lNumericBlanks(12)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\" invalid {} is necessary for accurate prediction but input is blank.",
                                            CurrentModuleObject,
                                            state.dataIPShortCut->cAlphaArgs(1),
                                            cNumericFields(12)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(12),
                                           rNumericArgs(12)));
                    ErrorsFound = true;
                }
            }

            windTurbine.HeightForLocalWS = state.dataIPShortCut->rNumericArgs(13); // Height of local meteorological station
            if (windTurbine.HeightForLocalWS == 0.0) {
                if (windTurbine.LocalAnnualAvgWS == 0.0) {
                    windTurbine.HeightForLocalWS = 0.0;
                } else {
                    windTurbine.HeightForLocalWS = DefaultH;
                    if (lNumericBlanks(13)) {
                        ShowWarningError(state,
                                         format("{}=\"{}\" invalid {} is necessary for accurate prediction but input is blank.",
                                                CurrentModuleObject,
                                                state.dataIPShortCut->cAlphaArgs(1),
                                                cNumericFields(13)));
                        ShowContinueError(state, format("...The default value of {:.2R} will be used. for {}", DefaultH, cNumericFields(13)));
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                               CurrentModuleObject,
                                               state.dataIPShortCut->cAlphaArgs(1),
                                               cNumericFields(13),
                                               rNumericArgs(13)));
                        ErrorsFound = true;
                    }
                }
            }

            windTurbine.ChordArea = state.dataIPShortCut->rNumericArgs(14); // Chord area of a single blade for VAWTs
            if (windTurbine.rotorType == RotorType::VerticalAxis && windTurbine.ChordArea == 0.0) {
                if (lNumericBlanks(14)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(14)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(14),
                                           rNumericArgs(14)));
                }
                ErrorsFound = true;
            }

            windTurbine.DragCoeff = state.dataIPShortCut->rNumericArgs(15); // Blade drag coefficient
            if (windTurbine.rotorType == RotorType::VerticalAxis && windTurbine.DragCoeff == 0.0) {
                if (lNumericBlanks(15)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(15)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(15),
                                           rNumericArgs(15)));
                }
                ErrorsFound = true;
            }

            windTurbine.LiftCoeff = state.dataIPShortCut->rNumericArgs(16); // Blade lift coefficient
            if (windTurbine.rotorType == RotorType::VerticalAxis && windTurbine.LiftCoeff == 0.0) {
                if (lNumericBlanks(16)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(16)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=[{:.2R}] must be greater than zero.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cNumericFields(16),
                                           rNumericArgs(16)));
                }
                ErrorsFound = true;
            }

            windTurbine.PowerCoeffs[0] = state.dataIPShortCut->rNumericArgs(17); // Empirical power coefficient C1
            if (lNumericBlanks(17)) {
                windTurbine.PowerCoeffs[0] = 0.0;
            }
            windTurbine.PowerCoeffs[1] = state.dataIPShortCut->rNumericArgs(18); // Empirical power coefficient C2
            if (lNumericBlanks(18)) {
                windTurbine.PowerCoeffs[1] = 0.0;
            }
            windTurbine.PowerCoeffs[2] = state.dataIPShortCut->rNumericArgs(19); // Empirical power coefficient C3
            if (lNumericBlanks(19)) {
                windTurbine.PowerCoeffs[2] = 0.0;
            }
            windTurbine.PowerCoeffs[3] = state.dataIPShortCut->rNumericArgs(20); // Empirical power coefficient C4
            if (lNumericBlanks(20)) {
                windTurbine.PowerCoeffs[3] = 0.0;
            }
            windTurbine.PowerCoeffs[4] = state.dataIPShortCut->rNumericArgs(21); // Empirical power coefficient C5
            if (lNumericBlanks(21)) {
                windTurbine.PowerCoeffs[4] = 0.0;
            }
            windTurbine.PowerCoeffs[5] = state.dataIPShortCut->rNumericArgs(22); // Empirical power coefficient C6
            if (lNumericBlanks(22)) {
                windTurbine.PowerCoeffs[5] = 0.0;
            }
        }

        cAlphaArgs.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        rNumericArgs.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{} errors occurred in input.  Program terminates.", CurrentModuleObject));
        }

        for (WindTurbineNum = 1; WindTurbineNum <= NumWindTurbines; ++WindTurbineNum) {
            auto &windTurbine = state.dataWindTurbine->WindTurbineSys(WindTurbineNum);
            SetupOutputVariable(state,
                                "Generator Produced AC Electricity Rate",
                                Constant::Units::W,
                                windTurbine.Power,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                windTurbine.Name);
            SetupOutputVariable(state,
                                "Generator Produced AC Electricity Energy",
                                Constant::Units::J,
                                windTurbine.Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                windTurbine.Name,
                                Constant::eResource::ElectricityProduced,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::WindTurbine);
            SetupOutputVariable(state,
                                "Generator Turbine Local Wind Speed",
                                Constant::Units::m_s,
                                windTurbine.LocalWindSpeed,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                windTurbine.Name);
            SetupOutputVariable(state,
                                "Generator Turbine Local Air Density",
                                Constant::Units::kg_m3,
                                windTurbine.LocalAirDensity,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                windTurbine.Name);
            SetupOutputVariable(state,
                                "Generator Turbine Tip Speed Ratio",
                                Constant::Units::None,
                                windTurbine.TipSpeedRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                windTurbine.Name);
            switch (windTurbine.rotorType) {
            case RotorType::HorizontalAxis: {
                SetupOutputVariable(state,
                                    "Generator Turbine Power Coefficient",
                                    Constant::Units::None,
                                    windTurbine.PowerCoeff,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    windTurbine.Name);
            } break;
            case RotorType::VerticalAxis: {
                SetupOutputVariable(state,
                                    "Generator Turbine Chordal Component Velocity",
                                    Constant::Units::m_s,
                                    windTurbine.ChordalVel,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    windTurbine.Name);
                SetupOutputVariable(state,
                                    "Generator Turbine Normal Component Velocity",
                                    Constant::Units::m_s,
                                    windTurbine.NormalVel,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    windTurbine.Name);
                SetupOutputVariable(state,
                                    "Generator Turbine Relative Flow Velocity",
                                    Constant::Units::m_s,
                                    windTurbine.RelFlowVel,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    windTurbine.Name);
                SetupOutputVariable(state,
                                    "Generator Turbine Attack Angle",
                                    Constant::Units::deg,
                                    windTurbine.AngOfAttack,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    windTurbine.Name);
            } break;
            default:
                break;
            }
        }
    }

    void InitWindTurbine(EnergyPlusData &state, int const WindTurbineNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   Oct 2009
        //       MODIFIED       Linda K. Lawrie, December 2009 for reading stat file

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads monthly average wind speed from stat file and then
        // determines annual average wind speed. Differences between this TMY wind speed
        // and local wind speed that the user inputs are then factored.
        // IF the user has no local wind data and does not enter the local wind speed to be factored,
        // then the factor of 1 is assigned, so that wind speed estimated
        // at the particular rotor height is used with no factorization.
        // It also initializes module variables at each time step.

        static char const TabChr('\t'); // Tab character

        Array1D<Real64> MonthWS(12);

        // Estimate average annual wind speed once
        if (state.dataWindTurbine->MyOneTimeFlag) {
            Real64 AnnualTMYWS = 0.0;
            if (FileSystem::fileExists(state.files.inStatFilePath.filePath)) {
                auto statFile = state.files.inStatFilePath.open(state, "InitWindTurbine");
                bool wsStatFound = false; // logical noting that wind stats were found
                while (statFile.good()) { // end of file
                    auto lineIn = statFile.readLine();
                    // reconcile line with different versions of stat file
                    size_t lnPtr = index(lineIn.data, "Wind Speed");
                    if (lnPtr == std::string::npos) {
                        continue;
                    }
                    // have hit correct section.
                    while (statFile.good()) { // find daily avg line
                        lineIn = statFile.readLine();
                        lnPtr = index(lineIn.data, "Daily Avg");
                        if (lnPtr == std::string::npos) {
                            continue;
                        }
                        // tab delimited file
                        lineIn.data.erase(0, lnPtr + 10);
                        MonthWS = 0.0;
                        wsStatFound = true;
                        bool warningShown = false; // true if the <365 warning has already been shown
                        for (int mon = 1; mon <= 12; ++mon) {
                            lnPtr = index(lineIn.data, TabChr);
                            if (lnPtr != 1) {
                                if ((lnPtr == std::string::npos) || (!stripped(lineIn.data.substr(0, lnPtr)).empty())) {
                                    if (lnPtr != std::string::npos) {
                                        bool error = false;
                                        MonthWS(mon) = Util::ProcessNumber(lineIn.data.substr(0, lnPtr), error);

                                        if (error) {
                                            // probably should throw some error here
                                        }

                                        lineIn.data.erase(0, lnPtr + 1);
                                    }
                                } else { // blank field
                                    if (!warningShown) {
                                        ShowWarningError(state,
                                                         format("InitWindTurbine: read from {} file shows <365 days in weather file. Annual average "
                                                                "wind speed used will be inaccurate.",
                                                                state.files.inStatFilePath.filePath));
                                        lineIn.data.erase(0, lnPtr + 1);
                                        warningShown = true;
                                    }
                                }
                            } else { // two tabs in succession
                                if (!warningShown) {
                                    ShowWarningError(state,
                                                     format("InitWindTurbine: read from {} file shows <365 days in weather file. Annual average wind "
                                                            "speed used will be inaccurate.",
                                                            state.files.inStatFilePath.filePath));
                                    lineIn.data.erase(0, lnPtr + 1);
                                    warningShown = true;
                                }
                            }
                        }
                        break;
                    }
                    if (wsStatFound) {
                        break;
                    }
                }
                if (wsStatFound) {
                    AnnualTMYWS = sum(MonthWS) / 12.0;
                } else {
                    ShowWarningError(
                        state, "InitWindTurbine: stat file did not include Wind Speed statistics. TMY Wind Speed adjusted at the height is used.");
                }
            } else { // No stat file
                ShowWarningError(state, "InitWindTurbine: stat file missing. TMY Wind Speed adjusted at the height is used.");
            }

            // assign this value to all the wind turbines once here
            for (auto &wt : state.dataWindTurbine->WindTurbineSys) {
                wt.AnnualTMYWS = AnnualTMYWS;
            }

            state.dataWindTurbine->MyOneTimeFlag = false;
        }

        auto &windTurbine = state.dataWindTurbine->WindTurbineSys(WindTurbineNum);

        // Factor differences between TMY wind data and local wind data once
        if (windTurbine.AnnualTMYWS > 0.0 && windTurbine.WSFactor == 0.0 && windTurbine.LocalAnnualAvgWS > 0) {
            // Convert the annual wind speed to the local wind speed at the height of the local station, then factor
            Real64 LocalTMYWS = windTurbine.AnnualTMYWS * state.dataEnvrn->WeatherFileWindModCoeff *
                                std::pow(windTurbine.HeightForLocalWS / state.dataEnvrn->SiteWindBLHeight, state.dataEnvrn->SiteWindExp);
            windTurbine.WSFactor = LocalTMYWS / windTurbine.LocalAnnualAvgWS;
        }
        // Assign factor of 1.0 if no stat file or no input of local average wind speed
        if (windTurbine.WSFactor == 0.0) {
            windTurbine.WSFactor = 1.0;
        }

        // Do every time step initialization
        windTurbine.Power = 0.0;
        windTurbine.TotPower = 0.0;
        windTurbine.PowerCoeff = 0.0;
        windTurbine.TipSpeedRatio = 0.0;
        windTurbine.ChordalVel = 0.0;
        windTurbine.NormalVel = 0.0;
        windTurbine.RelFlowVel = 0.0;
        windTurbine.AngOfAttack = 0.0;
        windTurbine.TanForce = 0.0;
        windTurbine.NorForce = 0.0;
        windTurbine.TotTorque = 0.0;
    }

    void CalcWindTurbine(EnergyPlusData &state,
                         int const WindTurbineNum,           // System is on
                         [[maybe_unused]] bool const RunFlag // System is on
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   October 2009

        // REFERENCES:
        // Sathyajith Mathew. 2006. Wind Energy: Fundamental, Resource Analysis and Economics. Springer,
        //     Chap. 2, pp. 11-15
        // Mazharul Islam, David S.K. Ting, and Amir Fartaj. 2008. Aerodynamic Models for Darrieus-type sSraight-bladed
        //     Vertical Axis Wind Turbines. Renewable & Sustainable Energy Reviews, Volume 12, pp.1087-1109

        using DataEnvironment::OutBaroPressAt;
        using DataEnvironment::OutDryBulbTempAt;
        using DataEnvironment::OutWetBulbTempAt;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyWFnTdbTwbPb;

        Real64 constexpr MaxTheta(90.0);   // Maximum of theta
        Real64 constexpr MaxDegree(360.0); // Maximum limit of outdoor air wind speed in m/s
        Real64 constexpr SecInMin(60.0);

        Real64 LocalWindSpeed;   // Ambient wind speed at the specific height in m/s
        Real64 RotorH;           // Height of the rotor in m
        Real64 RotorD;           // Diameter of the rotor in m
        Real64 LocalHumRat;      // Local humidity ratio of the air at the specific height
        Real64 LocalAirDensity;  // Local density at the specific height in m
        Real64 PowerCoeff;       // Power coefficient
        Real64 SweptArea;        // Swept area of the rotor in m2
        Real64 WTPower(0.0);     // Total Power generated by the turbine in the quasi-steady state in Watts
        Real64 Power;            // Actual power of the turbine in Watts
        Real64 TipSpeedRatio;    // Tip speed ratio (TSR)
        Real64 TipSpeedRatioAtI; // Tip speed ratio at the ith time step
        Real64 AzimuthAng;       // Azimuth angle of blades in degree
        Real64 ChordalVel;       // Chordal velocity component in m/s
        Real64 NormalVel;        // Normal velocity component in m/s
        Real64 AngOfAttack;      // Angle of attack of a single blade in degree
        Real64 RelFlowVel;       // Relative flow velocity component in m/s
        Real64 TanForce;         // Tangential force in N.m
        Real64 NorForce;         // Normal force in N.m
        Real64 RotorVel;         // Rotor velocity in m/s
        Real64 AvgTanForce;      // Average tangential force in N.m
        Real64 Constant;         // Constants within integrand of tangential force
        Real64 IntRelFlowVel;    // Integration of relative flow velocity
        Real64 TotTorque;        // Total torque for the number of blades
        Real64 Omega;            // Angular velocity of rotor in rad/s
        Real64 TanForceCoeff;    // Tangential force coefficient
        Real64 NorForceCoeff;    // Normal force coefficient
        Real64 Period;           // Period of sine and cosine functions
        Real64 C1;               // Empirical power coefficient C1
        Real64 C2;               // Empirical power coefficient C2
        Real64 C3;               // Empirical power coefficient C3
        Real64 C4;               // Empirical power coefficient C4
        Real64 C5;               // Empirical power coefficient C5
        Real64 C6;               // Empirical power coefficient C6
        Real64 LocalTemp;        // Ambient air temperature at the height in degree C
        Real64 LocalPress;       // Local atmospheric pressure at the particular height in Pa
        Real64 InducedVel;       // Induced velocity on the rotor in m/s
        // unused REAL(r64) :: SysEfficiency     ! Overall wind turbine system efficiency including generator and inverter
        Real64 MaxPowerCoeff; // Maximum power coefficient
        Real64 RotorSpeed;    // Speed of rotors

        auto &windTurbine = state.dataWindTurbine->WindTurbineSys(WindTurbineNum);
        // Estimate local velocity and density
        RotorH = windTurbine.RotorHeight;
        RotorD = windTurbine.RotorDiameter;
        RotorSpeed = windTurbine.RatedRotorSpeed;
        LocalTemp = OutDryBulbTempAt(state, RotorH);
        LocalPress = OutBaroPressAt(state, RotorH);
        LocalHumRat = PsyWFnTdbTwbPb(state, LocalTemp, OutWetBulbTempAt(state, RotorH), LocalPress);
        LocalAirDensity = PsyRhoAirFnPbTdbW(state, LocalPress, LocalTemp, LocalHumRat);
        LocalWindSpeed = DataEnvironment::WindSpeedAt(state, RotorH);
        LocalWindSpeed /= windTurbine.WSFactor;

        // Check wind conditions for system operation
        if (windTurbine.availSched->getCurrentVal() > 0 && LocalWindSpeed > windTurbine.CutInSpeed && LocalWindSpeed < windTurbine.CutOutSpeed) {

            // System is on
            Period = 2.0 * Constant::Pi;
            Omega = (RotorSpeed * Period) / SecInMin;
            SweptArea = (Constant::Pi * pow_2(RotorD)) / 4;
            TipSpeedRatio = (Omega * (RotorD / 2.0)) / LocalWindSpeed;

            // Limit maximum tip speed ratio
            if (TipSpeedRatio > windTurbine.MaxTipSpeedRatio) {
                TipSpeedRatio = windTurbine.MaxTipSpeedRatio;
            }

            switch (windTurbine.rotorType) {
            case RotorType::HorizontalAxis: { // Horizontal axis wind turbine
                MaxPowerCoeff = windTurbine.MaxPowerCoeff;
                // Check if empirical constants are available
                C1 = windTurbine.PowerCoeffs[0];
                C2 = windTurbine.PowerCoeffs[1];
                C3 = windTurbine.PowerCoeffs[2];
                C4 = windTurbine.PowerCoeffs[3];
                C5 = windTurbine.PowerCoeffs[4];
                C6 = windTurbine.PowerCoeffs[5];

                Real64 const LocalWindSpeed_3(pow_3(LocalWindSpeed));
                if (C1 > 0.0 && C2 > 0.0 && C3 > 0.0 && C4 >= 0.0 && C5 > 0.0 && C6 > 0.0) {
                    // Analytical approximation
                    // Maximum power, i.e., rotor speed is at maximum, and pitch angle is zero
                    // TipSpeedRatioAtI = 1.0 / ( ( 1.0 / ( TipSpeedRatio + 0.08 * PitchAngle ) ) - ( 0.035 / ( pow_3( PitchAngle ) + 1.0 ) ) );
                    // //Tuned PitchAngle is zero
                    TipSpeedRatioAtI = TipSpeedRatio / (1.0 - (TipSpeedRatio * 0.035));
                    // PowerCoeff = C1 * ( ( C2 / TipSpeedRatioAtI ) - ( C3 * PitchAngle ) - ( C4 * std::pow( PitchAngle, 1.5 ) ) - C5 ) * (
                    // std::exp( -( C6 / TipSpeedRatioAtI ) ) ); //Tuned PitchAngle is zero
                    PowerCoeff = C1 * ((C2 / TipSpeedRatioAtI) - C5) * std::exp(-(C6 / TipSpeedRatioAtI));
                    if (PowerCoeff > MaxPowerCoeff) {
                        PowerCoeff = MaxPowerCoeff;
                    }
                    WTPower = 0.5 * LocalAirDensity * PowerCoeff * SweptArea * LocalWindSpeed_3;
                } else { // Simple approximation
                    WTPower = 0.5 * LocalAirDensity * SweptArea * LocalWindSpeed_3 * MaxPowerCoeff;
                    PowerCoeff = MaxPowerCoeff;
                }
                // Maximum of rated power
                if (LocalWindSpeed >= windTurbine.RatedWindSpeed || WTPower > windTurbine.RatedPower) {
                    WTPower = windTurbine.RatedPower;
                    PowerCoeff = WTPower / (0.5 * LocalAirDensity * SweptArea * LocalWindSpeed_3);
                }
                // Recalculated Cp at the rated power
                windTurbine.PowerCoeff = PowerCoeff;
            } break;
            case RotorType::VerticalAxis: { // Vertical axis wind turbine
                RotorVel = Omega * (RotorD / 2.0);
                // Recalculated omega, if TSR is greater than the maximum
                if (TipSpeedRatio >= windTurbine.MaxTipSpeedRatio) {
                    RotorVel = LocalWindSpeed * windTurbine.MaxTipSpeedRatio;
                    Omega = RotorVel / (RotorD / 2.0);
                }

                AzimuthAng = MaxDegree / windTurbine.NumOfBlade;
                // Azimuth angle between zero and 90 degree
                if (AzimuthAng > MaxTheta) { // Number of blades is 2 or 3
                    AzimuthAng -= MaxTheta;
                    if (AzimuthAng == MaxTheta) { // 2 blades
                        AzimuthAng = 0.0;
                    }
                } else if (AzimuthAng == MaxTheta) { // 4 blades
                    AzimuthAng = 0.0;
                }

                InducedVel = LocalWindSpeed * 2.0 / 3.0;
                // Velocity components
                Real64 const sin_AzimuthAng = std::sin(AzimuthAng * Constant::DegToRad);
                Real64 const cos_AzimuthAng = std::cos(AzimuthAng * Constant::DegToRad);
                ChordalVel = RotorVel + InducedVel * cos_AzimuthAng;
                NormalVel = InducedVel * sin_AzimuthAng;
                RelFlowVel = std::sqrt(pow_2(ChordalVel) + pow_2(NormalVel));

                // Angle of attack
                AngOfAttack = std::atan((sin_AzimuthAng / ((RotorVel / LocalWindSpeed) / (InducedVel / LocalWindSpeed) + cos_AzimuthAng)));

                // Force coefficients
                Real64 const sin_AngOfAttack = std::sin(AngOfAttack * Constant::DegToRad);
                Real64 const cos_AngOfAttack = std::cos(AngOfAttack * Constant::DegToRad);
                TanForceCoeff = std::abs(windTurbine.LiftCoeff * sin_AngOfAttack - windTurbine.DragCoeff * cos_AngOfAttack);
                NorForceCoeff = windTurbine.LiftCoeff * cos_AngOfAttack + windTurbine.DragCoeff * sin_AngOfAttack;

                // Net tangential and normal forces
                Real64 const RelFlowVel_2(pow_2(RelFlowVel));
                Real64 const density_fac(0.5 * LocalAirDensity * windTurbine.ChordArea * RelFlowVel_2);
                TanForce = TanForceCoeff * density_fac;
                NorForce = NorForceCoeff * density_fac;
                Constant = (1.0 / Period) * (TanForce / RelFlowVel_2);

                // Relative flow velocity is the only function of theta in net tangential force
                // Integral of cos(theta) on zero to 2pi goes to zero
                // Integrate constants only
                IntRelFlowVel = pow_2(RotorVel) * Period + pow_2(InducedVel) * Period;

                // Average tangential force on a single blade
                AvgTanForce = Constant * IntRelFlowVel;
                TotTorque = windTurbine.NumOfBlade * AvgTanForce * (RotorD / 2.0);
                WTPower = TotTorque * Omega;

                // Check if power produced is greater than maximum or rated power
                if (WTPower > windTurbine.RatedPower) {
                    WTPower = windTurbine.RatedPower;
                }

                windTurbine.ChordalVel = ChordalVel;
                windTurbine.NormalVel = NormalVel;
                windTurbine.RelFlowVel = RelFlowVel;
                windTurbine.TanForce = TanForce;
                windTurbine.NorForce = NorForce;
                windTurbine.TotTorque = TotTorque;
            } break;
            default: {
                assert(false);
            } break;
            }

            if (WTPower > windTurbine.RatedPower) {
                WTPower = windTurbine.RatedPower;
            }

            // Actual power generated by the wind turbine system
            Power = WTPower * windTurbine.SysEfficiency;

            windTurbine.Power = Power;
            windTurbine.TotPower = WTPower;
            windTurbine.LocalWindSpeed = LocalWindSpeed;
            windTurbine.LocalAirDensity = LocalAirDensity;
            windTurbine.TipSpeedRatio = TipSpeedRatio;

        } else { // System is off
            windTurbine.Power = 0.0;
            windTurbine.TotPower = 0.0;
            windTurbine.PowerCoeff = 0.0;
            windTurbine.LocalWindSpeed = LocalWindSpeed;
            windTurbine.LocalAirDensity = LocalAirDensity;
            windTurbine.TipSpeedRatio = 0.0;
            windTurbine.ChordalVel = 0.0;
            windTurbine.NormalVel = 0.0;
            windTurbine.RelFlowVel = 0.0;
            windTurbine.AngOfAttack = 0.0;
            windTurbine.TanForce = 0.0;
            windTurbine.NorForce = 0.0;
            windTurbine.TotTorque = 0.0;
        }
    }

    void ReportWindTurbine(EnergyPlusData &state, int const WindTurbineNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Daeho Kang
        //       DATE WRITTEN   October 2009

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine fills remaining report variables.

        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &windTurbine = state.dataWindTurbine->WindTurbineSys(WindTurbineNum);

        windTurbine.Energy = windTurbine.Power * TimeStepSysSec;
    }

    //*****************************************************************************************

} // namespace WindTurbine

} // namespace EnergyPlus
