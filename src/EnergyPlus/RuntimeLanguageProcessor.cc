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
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/ArrayS.functions.hh>
// #include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/char.functions.hh>
#include <ObjexxFCL/random.hh>
#include <ObjexxFCL/string.functions.hh>
#include <ObjexxFCL/time.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus::RuntimeLanguageProcessor {

// MODULE INFORMATION:
//       AUTHOR         Peter Graham Ellis
//       DATE WRITTEN   June 2006
//       MODIFIED       Brent Griffith, May - August 2009
//       RE-ENGINEERED  na

// Using/Aliasing
using namespace DataRuntimeLanguage;

void InitializeRuntimeLanguage(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Rui Zhang February 2010
    //       RE-ENGINEERED  na

    // METHODOLOGY EMPLOYED:
    // One time run.  Must be run BEFORE anything gets parsed.

    // Using/Aliasing
    Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
    Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;

    Real64 tmpCurrentTime(0.0);
    Real64 tmpMinutes(0.0);
    Real64 tmpHours(0.0);
    Real64 tmpCurEnvirNum(0.0);
    Array1D_int datevalues(8);
    // value(1)   Current year
    // value(2)   Current month
    // value(3)   Current day
    // value(4)   Time difference with respect to UTC in minutes (0-59)
    // value(5)   Hour of the day (0-23)
    // value(6)   Minutes (0-59)
    // value(7)   Seconds (0-59)
    // value(8)   Milliseconds (0-999)

    if (state.dataRuntimeLangProcessor->InitializeOnce) {

        std::string datestring; // supposedly returns blank when no date available.

        state.dataRuntimeLang->emsVarBuiltInStart = state.dataRuntimeLang->NumErlVariables + 1;

        state.dataRuntimeLang->False = SetErlValueNumber(0.0);
        state.dataRuntimeLang->True = SetErlValueNumber(1.0);

        // Create constant built-in variables
        state.dataRuntimeLangProcessor->NullVariableNum = NewEMSVariable(state, "NULL", 0, SetErlValueNumber(0.0));
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->NullVariableNum).Value.Type = Value::Null;
        state.dataRuntimeLangProcessor->FalseVariableNum = NewEMSVariable(state, "FALSE", 0, state.dataRuntimeLang->False);
        state.dataRuntimeLangProcessor->TrueVariableNum = NewEMSVariable(state, "TRUE", 0, state.dataRuntimeLang->True);
        state.dataRuntimeLangProcessor->OffVariableNum = NewEMSVariable(state, "OFF", 0, state.dataRuntimeLang->False);
        state.dataRuntimeLangProcessor->OnVariableNum = NewEMSVariable(state, "ON", 0, state.dataRuntimeLang->True);
        state.dataRuntimeLangProcessor->PiVariableNum = NewEMSVariable(state, "PI", 0, SetErlValueNumber(Constant::Pi));
        state.dataRuntimeLangProcessor->TimeStepsPerHourVariableNum =
            NewEMSVariable(state, "TIMESTEPSPERHOUR", 0, SetErlValueNumber(double(state.dataGlobal->TimeStepsInHour)));

        // Create dynamic built-in variables
        state.dataRuntimeLangProcessor->YearVariableNum = NewEMSVariable(state, "YEAR", 0);
        state.dataRuntimeLangProcessor->CalendarYearVariableNum = NewEMSVariable(state, "CALENDARYEAR", 0);
        state.dataRuntimeLangProcessor->MonthVariableNum = NewEMSVariable(state, "MONTH", 0);
        state.dataRuntimeLangProcessor->DayOfMonthVariableNum = NewEMSVariable(state, "DAYOFMONTH", 0); // 'DAYOFMONTH'?
        state.dataRuntimeLangProcessor->DayOfWeekVariableNum = NewEMSVariable(state, "DAYOFWEEK", 0);
        state.dataRuntimeLangProcessor->DayOfYearVariableNum = NewEMSVariable(state, "DAYOFYEAR", 0);
        state.dataRuntimeLangProcessor->HourVariableNum = NewEMSVariable(state, "HOUR", 0);
        state.dataRuntimeLangProcessor->TimeStepNumVariableNum = NewEMSVariable(state, "TIMESTEPNUM", 0);
        state.dataRuntimeLangProcessor->MinuteVariableNum = NewEMSVariable(state, "MINUTE", 0);
        state.dataRuntimeLangProcessor->HolidayVariableNum = NewEMSVariable(state, "HOLIDAY", 0);
        state.dataRuntimeLangProcessor->DSTVariableNum = NewEMSVariable(state, "DAYLIGHTSAVINGS", 0);
        state.dataRuntimeLangProcessor->CurrentTimeVariableNum = NewEMSVariable(state, "CURRENTTIME", 0);
        state.dataRuntimeLangProcessor->SunIsUpVariableNum = NewEMSVariable(state, "SUNISUP", 0);
        state.dataRuntimeLangProcessor->IsRainingVariableNum = NewEMSVariable(state, "ISRAINING", 0);
        state.dataRuntimeLangProcessor->SystemTimeStepVariableNum = NewEMSVariable(state, "SYSTEMTIMESTEP", 0);
        state.dataRuntimeLangProcessor->ZoneTimeStepVariableNum = NewEMSVariable(state, "ZONETIMESTEP", 0);
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->ZoneTimeStepVariableNum).Value =
            SetErlValueNumber(state.dataGlobal->TimeStepZone);
        state.dataRuntimeLangProcessor->CurrentEnvironmentPeriodNum = NewEMSVariable(state, "CURRENTENVIRONMENT", 0);
        state.dataRuntimeLangProcessor->ActualDateAndTimeNum = NewEMSVariable(state, "ACTUALDATEANDTIME", 0);
        state.dataRuntimeLangProcessor->ActualTimeNum = NewEMSVariable(state, "ACTUALTIME", 0);
        state.dataRuntimeLangProcessor->WarmUpFlagNum = NewEMSVariable(state, "WARMUPFLAG", 0);

        // update the end of the built-in range so we can ignore those on API calls
        state.dataRuntimeLang->emsVarBuiltInEnd = state.dataRuntimeLang->NumErlVariables;

        GetRuntimeLanguageUserInput(state); // Load and parse all runtime language objects

        date_and_time(datestring, _, _, datevalues);
        if (datestring != "") {
            state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->ActualDateAndTimeNum).Value =
                SetErlValueNumber(double(sum(datevalues)));
            // datevalues(1)+datevalues(2)+datevalues(3)+  &
            // datevalues(5)+datevalues(6)+datevalues(7)+datevalues(8)
            state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->ActualTimeNum).Value =
                SetErlValueNumber(double(sum(datevalues({5, 8}))));
            // datevalues(5)+datevalues(6)+datevalues(7)+datevalues(8)
            //    ELSE
            //      ErlVariable(ActualDateAndTimeNum)%Value  = SetErlValueNumber(REAL(RANDOM_NUMBER(X=509),r64))
            //      ErlVariable(ActualTimeNum)%Value  = SetErlValueNumber(REAL(RANDOM_NUMBER(X=400),r64))
        }

        state.dataRuntimeLangProcessor->InitializeOnce = false;
    }

    // Update built-in variables
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->YearVariableNum).Value = SetErlValueNumber(double(state.dataEnvrn->Year));
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->CalendarYearVariableNum).Value =
        SetErlValueNumber(double(state.dataGlobal->CalendarYear));
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->MonthVariableNum).Value = SetErlValueNumber(double(state.dataEnvrn->Month));
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->DayOfMonthVariableNum).Value =
        SetErlValueNumber(double(state.dataEnvrn->DayOfMonth));
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->DayOfWeekVariableNum).Value =
        SetErlValueNumber(double(state.dataEnvrn->DayOfWeek));
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->DayOfYearVariableNum).Value =
        SetErlValueNumber(double(state.dataEnvrn->DayOfYear));
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->TimeStepNumVariableNum).Value =
        SetErlValueNumber(double(state.dataGlobal->TimeStep));

    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->DSTVariableNum).Value =
        SetErlValueNumber(double(state.dataEnvrn->DSTIndicator));
    // DSTadjust = REAL(DSTIndicator, r64)
    tmpHours = double(state.dataGlobal->HourOfDay - 1); // no, just stay on 0..23+ DSTadjust ! offset by 1 and daylight savings time
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->HourVariableNum).Value = SetErlValueNumber(tmpHours);

    if (TimeStepSys < state.dataGlobal->TimeStepZone) {
        // CurrentTime is for end of zone timestep, need to account for system timestep
        tmpCurrentTime = state.dataGlobal->CurrentTime - state.dataGlobal->TimeStepZone + SysTimeElapsed + TimeStepSys;
    } else {
        tmpCurrentTime = state.dataGlobal->CurrentTime;
    }
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->CurrentTimeVariableNum).Value = SetErlValueNumber(tmpCurrentTime);
    tmpMinutes = ((tmpCurrentTime - double(state.dataGlobal->HourOfDay - 1)) * 60.0); // -1.0 // off by 1
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->MinuteVariableNum).Value = SetErlValueNumber(tmpMinutes);
    // Subtract 7 from HolidayIndex to maintain compatability for EMS where 1=Holiday,2=SummerDesignDay, 3=WinterDesignDay, 4=CustomDay1,
    // 5=CustomDay2, but not <0
    if (state.dataEnvrn->HolidayIndex == 0) {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->HolidayVariableNum).Value = SetErlValueNumber(0.0);
    } else {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->HolidayVariableNum).Value =
            SetErlValueNumber(double(state.dataEnvrn->HolidayIndex - 7));
    }
    if (state.dataEnvrn->SunIsUp) {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->SunIsUpVariableNum).Value = SetErlValueNumber(1.0);
    } else {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->SunIsUpVariableNum).Value = SetErlValueNumber(0.0);
    }
    if (state.dataEnvrn->IsRain) {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->IsRainingVariableNum).Value = SetErlValueNumber(1.0);
    } else {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->IsRainingVariableNum).Value = SetErlValueNumber(0.0);
    }
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->SystemTimeStepVariableNum).Value = SetErlValueNumber(TimeStepSys);

    tmpCurEnvirNum = double(state.dataEnvrn->CurEnvirNum);
    state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->CurrentEnvironmentPeriodNum).Value = SetErlValueNumber(tmpCurEnvirNum);
    if (state.dataGlobal->WarmupFlag) {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->WarmUpFlagNum).Value = SetErlValueNumber(1.0);
    } else {
        state.dataRuntimeLang->ErlVariable(state.dataRuntimeLangProcessor->WarmUpFlagNum).Value = SetErlValueNumber(0.0);
    }
}

void BeginEnvrnInitializeRuntimeLanguage(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   March 2010
    //       MODIFIED       B. Griffith, added Sensor initialation
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // re initialize Erl for new simulation environment period

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // Using/Aliasing
    using OutputProcessor::SetInternalVariableValue;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // na

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int ActuatorUsedLoop;
    int EMSActuatorVariableNum;
    int ErlVariableNum;
    int TrendVarNum;
    int SensorNum;
    int TrendDepth;
    int loop;
    bool CycleThisVariable;

    // reinitialize state of Erl variable values to zero, this gets sensors and internal variables used
    for (ErlVariableNum = 1; ErlVariableNum <= state.dataRuntimeLang->NumErlVariables; ++ErlVariableNum) {
        // but skip constant built-in variables so don't overwrite them
        if (ErlVariableNum == state.dataRuntimeLangProcessor->NullVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->FalseVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->TrueVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->OffVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->OnVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->PiVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->ZoneTimeStepVariableNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->ActualDateAndTimeNum) {
            continue;
        }
        if (ErlVariableNum == state.dataRuntimeLangProcessor->ActualTimeNum) {
            continue;
        }

        // need to preserve curve index variables
        CycleThisVariable = false;
        for (loop = 1; loop <= state.dataRuntimeLang->NumEMSCurveIndices; ++loop) {
            if (ErlVariableNum == state.dataRuntimeLangProcessor->CurveIndexVariableNums(loop)) {
                CycleThisVariable = true;
            }
        }
        if (CycleThisVariable) {
            continue;
        }
        CycleThisVariable = false;
        for (loop = 1; loop <= state.dataRuntimeLang->NumEMSConstructionIndices; ++loop) {
            if (ErlVariableNum == state.dataRuntimeLangProcessor->ConstructionIndexVariableNums(loop)) {
                CycleThisVariable = true;
            }
        }
        if (CycleThisVariable) {
            continue;
        }

        if (state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.initialized) {
            state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value =
                SetErlValueNumber(0.0, state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value);
        }
    }
    // reinitialize state of actuators
    for (ActuatorUsedLoop = 1; ActuatorUsedLoop <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed;
         ++ActuatorUsedLoop) {
        EMSActuatorVariableNum = state.dataRuntimeLang->EMSActuatorUsed(ActuatorUsedLoop).ActuatorVariableNum;
        ErlVariableNum = state.dataRuntimeLang->EMSActuatorUsed(ActuatorUsedLoop).ErlVariableNum;
        state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.Type = Value::Null;
        *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).Actuated = false;
        switch (state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).PntrVarTypeUsed) {
        case PtrDataType::Real:
            *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).RealValue = 0.0;
            break;
        case PtrDataType::Integer:
            *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).IntValue = 0;
            break;
        case PtrDataType::Logical:
            *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).LogValue = false;
            break;
        default:
            break; // nothing to do for those
        }
    }

    // reinitialize trend variables so old data are purged
    for (TrendVarNum = 1; TrendVarNum <= state.dataRuntimeLang->NumErlTrendVariables; ++TrendVarNum) {
        TrendDepth = state.dataRuntimeLang->TrendVariable(TrendVarNum).LogDepth;
        state.dataRuntimeLang->TrendVariable(TrendVarNum).TrendValARR({1, TrendDepth}) = 0.0;
    }

    // reinitilize sensors
    for (SensorNum = 1; SensorNum <= state.dataRuntimeLang->NumSensors; ++SensorNum) {
        SetInternalVariableValue(
            state, state.dataRuntimeLang->Sensor(SensorNum).VariableType, state.dataRuntimeLang->Sensor(SensorNum).Index, 0.0, 0);
    }
}

void ParseStack(EnergyPlusData &state, int const StackNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith June 2009
    //                      Brent Griffith March 2012, add WHILE loops
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Parsing a block of text creates a program stack in DataRuntimeLanguage.
    // This routine only executes once for each Erl program.

    // METHODOLOGY EMPLOYED:
    // Loop over each line of Erl code and parse based on statement keyword

    // Using/Aliasing

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr IfDepthAllowed(5);        // depth of IF block nesting
    int constexpr ELSEIFLengthAllowed(200); // number of ELSEIFs allowed
    int constexpr WhileDepthAllowed(1);     // depth of While block nesting

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LineNum;
    int StackNum2;
    std::string::size_type Pos;
    int ExpressionNum;
    int VariableNum;
    std::string Line;      // local copy of a single line of Erl program code
    std::string Keyword;   // local copy of statement keyword parsed from line (Run, Set, If, etc)
    std::string Remainder; // local copy of what is left for text in the line after keyword
    std::string Expression;
    std::string Variable;
    int NestedIfDepth;    // indicates depth into If statement,
    int NestedWhileDepth; // indicates depth into While statement
    int InstructionNum;
    int InstructionNum2;
    int GotoNum;
    Array1D_int SavedIfInstructionNum(IfDepthAllowed); // index is depth of If statements
    Array2D_int SavedGotoInstructionNum(ELSEIFLengthAllowed, IfDepthAllowed);
    Array1D_int NumGotos(IfDepthAllowed); // index is depth of If statements,
    int SavedWhileInstructionNum;
    int SavedWhileExpressionNum;
    int NumWhileGotos;
    Array1D_bool ReadyForElse(IfDepthAllowed);
    Array1D_bool ReadyForEndif(IfDepthAllowed);

    LineNum = 1;
    NestedIfDepth = 0;
    ReadyForElse = false;
    ReadyForEndif = false;
    SavedIfInstructionNum = 0;
    SavedGotoInstructionNum = 0;
    NumGotos = 0;
    NestedWhileDepth = 0;
    SavedWhileInstructionNum = 0;
    SavedWhileExpressionNum = 0;
    NumWhileGotos = 0;

    auto &thisErlStack = state.dataRuntimeLang->ErlStack(StackNum);

    while (LineNum <= thisErlStack.NumLines) {

        Line = stripped(thisErlStack.Line(LineNum));
        if (len(Line) == 0) {
            ++LineNum;
            continue; // Blank lines can be skipped
        }

        Pos = scan(Line, ' ');
        if (Pos == std::string::npos) {
            Pos = len(Line);
            Remainder.clear();
        } else {
            Remainder = stripped(Line.substr(Pos + 1));
        }
        //    Keyword = Util::makeUPPER(Line(1:Pos-1))
        Keyword = Line.substr(0, Pos);

        // the functionality in each block of this parser structure is so different that a regular IF block seems reasonable
        if (Keyword == "RETURN") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "RETURN \"{}\"\n", Line);
            }
            if (Remainder.empty()) {
                InstructionNum = AddInstruction(state, StackNum, LineNum, RuntimeLanguageProcessor::ErlKeywordParam::Return);
            } else {
                ParseExpression(state, Remainder, StackNum, ExpressionNum, Line);
                InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::Return, ExpressionNum);
            }

        } else if (Keyword == "SET") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "SET \"{}\"\n", Line);
            }
            Pos = scan(Remainder, '=');
            if (Pos == std::string::npos) {
                AddError(state, StackNum, LineNum, "Equal sign missing for the SET instruction.");
            } else if (Pos == 0) {
                AddError(state, StackNum, LineNum, "Variable name missing for the SET instruction.");
            } else {
                Variable = stripped(Remainder.substr(0, Pos)); // VariableName would be more expressive
                VariableNum = NewEMSVariable(state, Variable, StackNum);
                // Check for invalid variable name

                if (Pos + 1 < Remainder.length()) {
                    Expression = stripped(Remainder.substr(Pos + 1));
                } else {
                    Expression.clear();
                }
                if (Expression.empty()) {
                    AddError(state, StackNum, LineNum, "Expression missing for the SET instruction.");
                } else {
                    ParseExpression(state, Expression, StackNum, ExpressionNum, Line);
                    InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::Set, VariableNum, ExpressionNum);
                }
            }

        } else if (Keyword == "RUN") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "RUN \"{}\"\n", Line);
            }
            if (Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Program or Subroutine name missing for the RUN instruction.");
            } else {
                Pos = scan(Remainder, ' ');
                if (Pos == std::string::npos) {
                    Pos = Remainder.length();
                }
                Variable = Util::makeUPPER(stripped(Remainder.substr(0, Pos))); // really the subroutine, or reference to instruction set
                StackNum2 = Util::FindItemInList(Variable, state.dataRuntimeLang->ErlStack);
                if (StackNum2 == 0) {
                    AddError(state, StackNum, LineNum, "Program or Subroutine name [" + Variable + "] not found for the RUN instruction.");
                } else {
                    InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::Run, StackNum2);
                }
            }

        } else if (Keyword == "IF") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "IF \"{}\"\n", Line);
                print(state.files.debug, "NestedIf={}\n", NestedIfDepth);
            }
            if (Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Expression missing for the IF instruction.");
                ExpressionNum = 0;
            } else {
                Expression = stripped(Remainder);
                ParseExpression(state, Expression, StackNum, ExpressionNum, Line);
            }

            ++NestedIfDepth;
            ReadyForElse(NestedIfDepth) = true;
            ReadyForEndif(NestedIfDepth) = true;
            if (NestedIfDepth > IfDepthAllowed) {
                AddError(state, StackNum, LineNum, "Detected IF nested deeper than is allowed; need to terminate an earlier IF instruction.");
                break;
            } else {
                InstructionNum = AddInstruction(state,
                                                StackNum,
                                                LineNum,
                                                DataRuntimeLanguage::ErlKeywordParam::If,
                                                ExpressionNum); // Arg2 added at next ELSEIF, ELSE, ENDIF
                SavedIfInstructionNum(NestedIfDepth) = InstructionNum;
            }

        } else if (Keyword == "ELSEIF") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "ELSEIF \"{}\"\n", Line);
                print(state.files.debug, "NestedIf={}\n", NestedIfDepth);
            }
            if (NestedIfDepth == 0) {
                AddError(state, StackNum, LineNum, "Starting IF instruction missing for the ELSEIF instruction.");
                break; // Getting strange error on DEALLOCATE for the next instruction that I try to add, so doing EXIT here
            }

            // Complete the preceding block with a GOTO instruction
            InstructionNum = AddInstruction(state, StackNum, 0, DataRuntimeLanguage::ErlKeywordParam::Goto); // Arg2 is added at the ENDIF
            ++NumGotos(NestedIfDepth);
            if (NumGotos(NestedIfDepth) > ELSEIFLengthAllowed) {
                AddError(state, StackNum, LineNum, "Detected ELSEIF series that is longer than allowed; terminate earlier IF instruction.");
                break;
            } else {
                SavedGotoInstructionNum(NumGotos(NestedIfDepth), NestedIfDepth) = InstructionNum;
            }

            if (Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Expression missing for the ELSEIF instruction.");
                ExpressionNum = 0;
            } else {
                Expression = stripped(Remainder);
                ParseExpression(state, Expression, StackNum, ExpressionNum, Line);
            }

            InstructionNum = AddInstruction(state,
                                            StackNum,
                                            LineNum,
                                            DataRuntimeLanguage::ErlKeywordParam::If,
                                            ExpressionNum); // Arg2 added at next ELSEIF, ELSE, ENDIF
            thisErlStack.Instruction(SavedIfInstructionNum(NestedIfDepth)).Argument2 = InstructionNum;
            SavedIfInstructionNum(NestedIfDepth) = InstructionNum;

        } else if (Keyword == "ELSE") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "ELSE \"{}\"\n", Line);
                print(state.files.debug, "NestedIf={}\n", NestedIfDepth);
            }
            if (NestedIfDepth == 0) {
                AddError(state, StackNum, LineNum, "Starting IF instruction missing for the ELSE instruction.");
                break; // Getting strange error on DEALLOCATE for the next instruction that I try to add, so doing EXIT here
            }
            if (!ReadyForElse(NestedIfDepth)) {
                AddError(state, StackNum, LineNum, "ELSE statement without corresponding IF statement.");
            }
            ReadyForElse(NestedIfDepth) = false;

            // Complete the preceding block with a GOTO instruction
            InstructionNum = AddInstruction(state, StackNum, 0, DataRuntimeLanguage::ErlKeywordParam::Goto); // Arg2 is added at the ENDIF
            ++NumGotos(NestedIfDepth);
            if (NumGotos(NestedIfDepth) > ELSEIFLengthAllowed) {
                AddError(state, StackNum, LineNum, "Detected ELSEIF-ELSE series that is longer than allowed.");
                break;
            } else {
                SavedGotoInstructionNum(NumGotos(NestedIfDepth), NestedIfDepth) = InstructionNum;
            }

            if (!Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Nothing is allowed to follow the ELSE instruction.");
            }

            InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::Else); // can make this into a KeywordIf?
            thisErlStack.Instruction(SavedIfInstructionNum(NestedIfDepth)).Argument2 = InstructionNum;
            SavedIfInstructionNum(NestedIfDepth) = InstructionNum;

        } else if (Keyword == "ENDIF") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "ENDIF \"{}\"\n", Line);
                print(state.files.debug, "NestedIf={}\n", NestedIfDepth);
            }
            if (NestedIfDepth == 0) {
                AddError(state, StackNum, LineNum, "Starting IF instruction missing for the ENDIF instruction.");
                break; // PE Getting strange error on DEALLOCATE for the next instruction that I try to add, so doing EXIT here
            }

            if (!ReadyForEndif(NestedIfDepth)) {
                AddError(state, StackNum, LineNum, "ENDIF statement without corresponding IF stetement.");
            }
            ReadyForEndif(NestedIfDepth) = false;
            ReadyForElse(NestedIfDepth) = false;

            if (!Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Nothing is allowed to follow the ENDIF instruction.");
            }

            InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::EndIf);
            thisErlStack.Instruction(SavedIfInstructionNum(NestedIfDepth)).Argument2 = InstructionNum;

            // Go back and complete all of the GOTOs that terminate each IF and ELSEIF block
            for (GotoNum = 1; GotoNum <= NumGotos(NestedIfDepth); ++GotoNum) {
                InstructionNum2 = SavedGotoInstructionNum(GotoNum, NestedIfDepth);
                thisErlStack.Instruction(InstructionNum2).Argument1 = InstructionNum;
                SavedGotoInstructionNum(GotoNum, NestedIfDepth) = 0;
            }

            NumGotos(NestedIfDepth) = 0;
            SavedIfInstructionNum(NestedIfDepth) = 0;
            --NestedIfDepth;

        } else if (Keyword == "WHILE") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "WHILE \"{}\"\n", Line);
            }
            if (Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Expression missing for the WHILE instruction.");
                ExpressionNum = 0;
            } else {
                Expression = stripped(Remainder);
                ParseExpression(state, Expression, StackNum, ExpressionNum, Line);
            }

            ++NestedWhileDepth;
            if (NestedWhileDepth > WhileDepthAllowed) {
                AddError(state, StackNum, LineNum, "Detected WHILE nested deeper than is allowed; need to terminate an earlier WHILE instruction.");
                break;
            } else {
                InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::While, ExpressionNum);
                SavedWhileInstructionNum = InstructionNum;
                SavedWhileExpressionNum = ExpressionNum;
            }

        } else if (Keyword == "ENDWHILE") {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "ENDWHILE \"{}\"\n", Line);
            }
            if (NestedWhileDepth == 0) {
                AddError(state, StackNum, LineNum, "Starting WHILE instruction missing for the ENDWHILE instruction.");
                break;
            }
            if (!Remainder.empty()) {
                AddError(state, StackNum, LineNum, "Nothing is allowed to follow the ENDWHILE instruction.");
            }

            InstructionNum = AddInstruction(state, StackNum, LineNum, DataRuntimeLanguage::ErlKeywordParam::EndWhile);
            thisErlStack.Instruction(SavedWhileInstructionNum).Argument2 = InstructionNum;
            thisErlStack.Instruction(InstructionNum).Argument1 = SavedWhileExpressionNum;
            thisErlStack.Instruction(InstructionNum).Argument2 = SavedWhileInstructionNum;

            NestedWhileDepth = 0;
            SavedWhileInstructionNum = 0;
            SavedWhileExpressionNum = 0;

        } else {
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "ERROR \"{}\"\n", Line);
            }
            AddError(state, StackNum, LineNum, "Unknown keyword [" + Keyword + "].");
        }

        ++LineNum;
    } // LineNum

    if (NestedIfDepth == 1) {
        AddError(state, StackNum, 0, "Missing an ENDIF instruction needed to terminate an earlier IF instruction.");
    } else if (NestedIfDepth > 1) {
        AddError(state, StackNum, 0, format("Missing {} ENDIF instructions needed to terminate earlier IF instructions.", NestedIfDepth));
    }

    //  ALLOCATE(DummyError(ErlStack(StackNum)%NumErrors))
    //  DummyError = ErlStack(StackNum)%Error
}

int AddInstruction(EnergyPlusData &state,
                   int const StackNum,
                   int const LineNum,
                   DataRuntimeLanguage::ErlKeywordParam Keyword,
                   ObjexxFCL::Optional_int_const Argument1, // Erl variable index
                   ObjexxFCL::Optional_int_const Argument2)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Adds an instruction to a stack.

    // METHODOLOGY EMPLOYED:

    // Return value
    int InstructionNum;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    // Object Data
    ErlStackType TempStack;

    auto &thisErlStack = state.dataRuntimeLang->ErlStack(StackNum);

    if (thisErlStack.NumInstructions == 0) {
        thisErlStack.Instruction.allocate(1);
        thisErlStack.NumInstructions = 1;
    } else {
        TempStack = thisErlStack;
        thisErlStack.Instruction.deallocate();
        thisErlStack.Instruction.allocate(thisErlStack.NumInstructions + 1);
        thisErlStack.Instruction({1, thisErlStack.NumInstructions}) = TempStack.Instruction({1, thisErlStack.NumInstructions});
        ++thisErlStack.NumInstructions;
    }

    InstructionNum = thisErlStack.NumInstructions;
    thisErlStack.Instruction(InstructionNum).LineNum = LineNum;
    thisErlStack.Instruction(InstructionNum).Keyword = Keyword;

    if (present(Argument1)) {
        thisErlStack.Instruction(InstructionNum).Argument1 = Argument1;
    }
    if (present(Argument2)) {
        thisErlStack.Instruction(InstructionNum).Argument2 = Argument2;
    }

    return InstructionNum;
}

void AddError(EnergyPlusData &state,
              int const StackNum,      // index pointer to location in ErlStack structure
              int const LineNum,       // Erl program line number
              std::string const &Error // error message to be added to ErlStack
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Adds an error message to a stack.

    // METHODOLOGY EMPLOYED:

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int ErrorNum; // local count of errors for this ErlStack

    // Object Data
    ErlStackType TempStack; // temporary copy of single ErlStack

    auto &thisErlStack = state.dataRuntimeLang->ErlStack(StackNum);
    if (thisErlStack.NumErrors == 0) {
        thisErlStack.Error.allocate(1);
        thisErlStack.NumErrors = 1;
    } else {
        TempStack = thisErlStack;
        thisErlStack.Error.deallocate();
        thisErlStack.Error.allocate(thisErlStack.NumErrors + 1);
        thisErlStack.Error({1, thisErlStack.NumErrors}) = TempStack.Error({1, thisErlStack.NumErrors});
        ++thisErlStack.NumErrors;
    }

    ErrorNum = thisErlStack.NumErrors;
    if (LineNum > 0) {
        thisErlStack.Error(ErrorNum) = format("Line {}:  {} \"{}\"", LineNum, Error, thisErlStack.Line(LineNum));
    } else {
        thisErlStack.Error(ErrorNum) = Error;
    }
}

ErlValueType EvaluateStack(EnergyPlusData &state, int const StackNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith, May 2009
    //                      Brent Griffith, March 2012, add While loop support
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Runs a stack with the interpreter.

    // Return value
    ErlValueType ReturnValue;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int InstructionNum;
    int InstructionNum2;
    int ExpressionNum;
    int ESVariableNum;
    int WhileLoopExitCounter;      // to avoid infinite loop in While loop
    bool seriousErrorFound(false); // once it gets set true (inside EvaluateExpresssion) it will trigger a fatal (in WriteTrace)

    WhileLoopExitCounter = 0;
    ReturnValue.Type = Value::Number;
    ReturnValue.Number = 0.0;

    auto const &thisErlStack = state.dataRuntimeLang->ErlStack(StackNum);

    InstructionNum = 1;
    while (InstructionNum <= thisErlStack.NumInstructions) {

        auto const &thisInstruction = thisErlStack.Instruction(InstructionNum);

        {
            DataRuntimeLanguage::ErlKeywordParam const SELECT_CASE_var = thisInstruction.Keyword;

            if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::None) {
                // There probably shouldn't be any of these

            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::Return) {
                if (thisInstruction.Argument1 > 0) {
                    ReturnValue = EvaluateExpression(state, thisInstruction.Argument1, seriousErrorFound);
                }
                WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                break; // RETURN always terminates an instruction stack

            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::Set) {

                ESVariableNum = thisInstruction.Argument1;
                auto &thisErlVar = state.dataRuntimeLang->ErlVariable(ESVariableNum);
                ReturnValue = EvaluateExpression(state, thisInstruction.Argument2, seriousErrorFound);
                if ((!thisErlVar.ReadOnly) && (!thisErlVar.Value.TrendVariable)) {
                    // #10279 - We don't do `thisErlVar.Value = ReturnValue;` because we don't want to copy TrendVariable stuff
                    thisErlVar.Value.Type = ReturnValue.Type;
                    thisErlVar.Value.Number = ReturnValue.Number;
                    // thisErlVar.Value.String = ReturnValue.String;
                    thisErlVar.Value.Error = ReturnValue.Error;
                    thisErlVar.Value.initialized = ReturnValue.initialized;
                } else if (thisErlVar.Value.TrendVariable) {
                    thisErlVar.Value.Number = ReturnValue.Number;
                    thisErlVar.Value.Error = ReturnValue.Error;
                }

                WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);

            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::Run) {
                ReturnValue.Type = Value::String;
                ReturnValue.String = "";
                WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                ReturnValue = EvaluateStack(state, thisInstruction.Argument1);
            } else if ((SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::If) ||
                       (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::Else)) { // same???
                ExpressionNum = thisInstruction.Argument1;
                InstructionNum2 = thisInstruction.Argument2;
                if (ExpressionNum > 0) { // could be 0 if this was an ELSE
                    ReturnValue = EvaluateExpression(state, ExpressionNum, seriousErrorFound);
                    WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                    if (ReturnValue.Number == 0.0) { //  This is the FALSE case
                        // Eventually should handle strings and arrays too
                        InstructionNum = InstructionNum2;
                        continue;
                    }
                } else {
                    // KeywordELSE  -- kind of a kludge
                    ReturnValue.Type = Value::Number;
                    ReturnValue.Number = 1.0;
                    WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                }
            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::Goto) {
                InstructionNum = thisInstruction.Argument1;

                // For debug purposes only...
                ReturnValue.Type = Value::String;
                ReturnValue.String = ""; // IntegerToString(InstructionNum)

                continue;
                // PE if this ever went out of bounds, would the DO loop save it?  or need check here?

            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::EndIf) {
                ReturnValue.Type = Value::String;
                ReturnValue.String = "";
                WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);

            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::While) {
                // evaluate expression at while, skip to past endwhile if not true
                ExpressionNum = thisInstruction.Argument1;
                InstructionNum2 = thisInstruction.Argument2;
                ReturnValue = EvaluateExpression(state, ExpressionNum, seriousErrorFound);
                WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                if (ReturnValue.Number == 0.0) { //  This is the FALSE case
                    // Eventually should handle strings and arrays too
                    InstructionNum = InstructionNum2;
                    // CYCLE
                }
            } else if (SELECT_CASE_var == DataRuntimeLanguage::ErlKeywordParam::EndWhile) {

                // reevaluate expression at While and goto there if true, otherwise continue
                ExpressionNum = thisInstruction.Argument1;
                InstructionNum2 = thisInstruction.Argument2;
                ReturnValue = EvaluateExpression(state, ExpressionNum, seriousErrorFound);
                if ((ReturnValue.Number != 0.0) && (WhileLoopExitCounter <= MaxWhileLoopIterations)) { //  This is the True case
                    // Eventually should handle strings and arrays too
                    WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound); // duplicative?
                    InstructionNum = InstructionNum2;
                    ++WhileLoopExitCounter;

                    continue;
                } else { // false, leave while block
                    if (WhileLoopExitCounter > MaxWhileLoopIterations) {
                        WhileLoopExitCounter = 0;
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Maximum WHILE loop iteration limit reached";
                        WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                    } else {
                        ReturnValue.Type = Value::Number;
                        ReturnValue.Number = 0.0;
                        WriteTrace(state, StackNum, InstructionNum, ReturnValue, seriousErrorFound);
                        WhileLoopExitCounter = 0;
                    }
                }
            } else {
                ShowFatalError(state, "Fatal error in RunStack:  Unknown keyword.");
            }
        }

        ++InstructionNum;
    } // InstructionNum

    return ReturnValue;
}

void WriteTrace(EnergyPlusData &state, int const StackNum, int const InstructionNum, ErlValueType const &ReturnValue, bool const seriousErrorFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith, May 2009
    //                      Brent Griffith, May 2016, added bool and fatal error messages for runtime problems with math and unitialized vars
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:

    // METHODOLOGY EMPLOYED:

    // Using/Aliasing
    using General::CreateSysTimeIntervalString;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LineNum;
    std::string NameString;
    std::string LineNumString;
    std::string LineString;
    std::string cValueString;
    std::string TimeString;
    std::string DuringWarmup;

    if ((!state.dataRuntimeLang->OutputFullEMSTrace) && (!state.dataRuntimeLang->OutputEMSErrors) && (!seriousErrorFound)) {
        return;
    }

    if ((state.dataRuntimeLang->OutputEMSErrors) && (!state.dataRuntimeLang->OutputFullEMSTrace) && (!seriousErrorFound)) {
        // see if error needs to be reported.
        if (ReturnValue.Type != Value::Error) {
            return;
        }
    }

    if (!state.dataRuntimeLangProcessor->WriteTraceMyOneTimeFlag) {
        print(state.files.edd, "****  Begin EMS Language Processor Error and Trace Output  *** \n");
        print(state.files.edd, "<Erl program name, line #, line text, result, occurrence timing information ... >\n");
        state.dataRuntimeLangProcessor->WriteTraceMyOneTimeFlag = true;
    }
    // if have not return'd yet then write out full trace

    NameString = state.dataRuntimeLang->ErlStack(StackNum).Name;
    LineNum = state.dataRuntimeLang->ErlStack(StackNum).Instruction(InstructionNum).LineNum;
    LineNumString = fmt::to_string(LineNum);
    LineString = state.dataRuntimeLang->ErlStack(StackNum).Line(LineNum);
    cValueString = ValueToString(ReturnValue);

    // put together timestamp info
    if (state.dataGlobal->WarmupFlag) {
        if (!state.dataGlobal->DoingSizing) {
            DuringWarmup = " During Warmup, Occurrence info=";
        } else {
            DuringWarmup = " During Warmup & Sizing, Occurrence info=";
        }
    } else {
        if (!state.dataGlobal->DoingSizing) {
            DuringWarmup = " Occurrence info=";
        } else {
            DuringWarmup = " During Sizing, Occurrence info=";
        }
    }
    TimeString = DuringWarmup + state.dataEnvrn->EnvironmentName + ", " + state.dataEnvrn->CurMnDy + ' ' + CreateSysTimeIntervalString(state);

    if (state.dataRuntimeLang->OutputFullEMSTrace || (state.dataRuntimeLang->OutputEMSErrors && (ReturnValue.Type == Value::Error))) {
        print(state.files.edd, "{},Line {},{},{},{}\n", NameString, LineNumString, LineString, cValueString, TimeString);
    }

    if (seriousErrorFound) { // throw EnergyPlus severe then fatal
        ShowSevereError(state, "Problem found in EMS EnergyPlus Runtime Language.");
        ShowContinueError(state, format("Erl program name: {}", NameString));
        ShowContinueError(state, format("Erl program line number: {}", LineNumString));
        ShowContinueError(state, format("Erl program line text: {}", LineString));
        ShowContinueError(state, format("Error message: {}", cValueString));
        ShowContinueErrorTimeStamp(state, "");
        ShowFatalError(state, "Previous EMS error caused program termination.");
    }
}

//******************************************************************************************

//  Expression Processor

//******************************************************************************************

void ParseExpression(EnergyPlusData &state,
                     std::string const &InString, // String of expression text written in the Runtime Language
                     int const StackNum,          // Parent StackNum??
                     int &ExpressionNum,          // index of expression in structure
                     std::string const &Line      // Actual line from string
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith, May 2009
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Parsing string into a series of tokens

    // METHODOLOGY EMPLOYED:

    // Using/Aliasing

    // Locals
    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxDoLoopCounts(500);

    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    //  CHARACTER(len=120), DIMENSION(MaxErrors) :: Error  ! Errors should be stored with the stack
    int NumErrors;
    std::string::size_type Pos;
    std::string StringToken;
    char NextChar;
    bool PeriodFound;
    bool MinusFound;
    bool PlusFound;
    bool MultFound;
    bool DivFound;
    bool ErrorFlag;
    bool OperatorProcessing;
    int CountDoLooping;
    bool LastED; // last character in a numeric was an E or D

    CountDoLooping = 0;
    NumErrors = 0;
    //  Error = 'No errors.'

    // Break the string into tokens
    int NumTokens(0);
    std::string String(InString);

    // Following is a workaround to parse unitary operators as first value in the expression.
    // i.e. Set X = -1
    // this creates Set X = 0-1
    // and seems to work.

    assert(!String.empty());
    if (String[0] == '-') {
        String = "0" + String;
    } else if (String[0] == '+') {
        String = "0" + String;
    }
    std::string::size_type LastPos(String.length());
    Pos = 0;
    OperatorProcessing = false; // true when an operator is found until terminated by non-operator
    MinusFound = false;
    MultFound = false;
    DivFound = false;
    while (Pos < LastPos) {
        ++CountDoLooping;
        if (CountDoLooping > MaxDoLoopCounts) {
            ShowSevereError(state, format("EMS ParseExpression: Entity={}", state.dataRuntimeLang->ErlStack(StackNum).Name));
            ShowContinueError(state, format("...Line={}", Line));
            ShowContinueError(state, format("...Failed to process String=\"{}\".", String));
            ShowFatalError(state, "...program terminates due to preceding condition.");
        }
        NextChar = String[Pos];
        if (NextChar == ' ') {
            ++Pos;
            continue;
        }

        // Extend the token array
        state.dataRuntimeLangProcessor->PEToken.redimension(++NumTokens);

        // Get the next token
        StringToken = "";
        PeriodFound = false;
        PlusFound = false;
        ErrorFlag = false;
        LastED = false;
        if (is_any_of(NextChar, "0123456789.")) {
            // Parse a number literal token
            ++Pos;
            StringToken += NextChar;
            OperatorProcessing = false;
            MultFound = false;
            DivFound = false;

            if (NextChar == '.') {
                PeriodFound = true;
            }

            while (Pos < LastPos) {
                NextChar = String[Pos];
                if (is_any_of(NextChar, "0123456789.eEdD")) {
                    ++Pos;
                    if (NextChar == '.') {
                        if (PeriodFound) {
                            // ERROR:  two periods appearing in a number literal!
                            ShowSevereError(state, format("EMS Parse Expression, for \"{}\".", state.dataRuntimeLang->ErlStack(StackNum).Name));
                            ShowContinueError(state, format("...Line=\"{}\".", Line));
                            ShowContinueError(state, format("...Bad String=\"{}\".", String));
                            ShowContinueError(state, "...Two decimal points detected in String.");
                            ++NumErrors;
                            ErrorFlag = true;
                            break;
                        } else {
                            PeriodFound = true;
                        }
                    }
                    if (is_any_of(NextChar, "eEdD")) {
                        StringToken += NextChar;
                        if (LastED) {
                            ShowSevereError(state, format("EMS Parse Expression, for \"{}\".", state.dataRuntimeLang->ErlStack(StackNum).Name));
                            ShowContinueError(state, format("...Line=\"{}\".", Line));
                            ShowContinueError(state, format("...Bad String=\"{}\".", String));
                            ShowContinueError(state, "...Two D/E in numeric String.");
                            ++NumErrors;
                            ErrorFlag = true;
                            // error
                            break;
                        } else {
                            LastED = true;
                        }
                    } else {
                        StringToken += NextChar;
                    }
                } else if (is_any_of(NextChar, "+-")) { // +/- following an ED is okay.
                    if (LastED) {
                        StringToken += NextChar;
                        ++Pos;
                        LastED = false;
                    } else {
                        // +/- will be processed on next pass, nothing needs to be done after a numeral
                        break;
                    }
                } else if (is_any_of(NextChar, " +-*/^=<>)")) { // Any binary operator is okay
                    break;                                      // End of token
                } else {
                    // Error: strange sequence of characters:  return TokenString//NextChar   e.g.,  234.44a or 234.44%
                    StringToken += NextChar;
                    break;
                }
            }

            // Save the number token
            if (!ErrorFlag) {
                state.dataRuntimeLangProcessor->PEToken(NumTokens).Type = Token::Number;
                state.dataRuntimeLangProcessor->PEToken(NumTokens).String = StringToken;
                if (state.dataSysVars->DeveloperFlag) {
                    print(state.files.debug, "Number=\"{}\"\n", StringToken);
                }
                state.dataRuntimeLangProcessor->PEToken(NumTokens).Number = Util::ProcessNumber(StringToken, ErrorFlag);
                if (state.dataSysVars->DeveloperFlag && ErrorFlag) {
                    print(state.files.debug, "{}\n", "Numeric error flagged");
                }
                if (MinusFound) {
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Number = -state.dataRuntimeLangProcessor->PEToken(NumTokens).Number;
                    MinusFound = false;
                }
                if (ErrorFlag) {
                    // Error: something wrong with this number!
                    ShowSevereError(state, format("EMS Parse Expression, for \"{}\".", state.dataRuntimeLang->ErlStack(StackNum).Name));
                    ShowContinueError(state, format("...Line=\"{}\".", Line));
                    ShowContinueError(state, format("...Bad String=\"{}\".", String));
                    ShowContinueError(state, format("Invalid numeric=\"{}\".", StringToken));
                    ++NumErrors;
                }
            }

        } else if (is_any_of(NextChar, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) {
            // Parse an undetermined string token (could be a variable, subroutine, or named operator)
            ++Pos;
            StringToken += NextChar;
            OperatorProcessing = false;
            MultFound = false;
            DivFound = false;

            while (Pos < LastPos) {
                NextChar = String[Pos];
                if (is_any_of(NextChar, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789")) {
                    ++Pos;
                    StringToken += NextChar;
                } else if (is_any_of(NextChar, " +-*/^=<>()")) {
                    break; // End of token
                } else {
                    // Error: bad syntax:  return TokenString//NextChar   e.g.,  var1$ or b%
                    break;
                }
            }

            // Save the variable token
            state.dataRuntimeLangProcessor->PEToken(NumTokens).Type = Token::Variable;
            state.dataRuntimeLangProcessor->PEToken(NumTokens).String = StringToken;
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "Variable=\"{}\"\n", StringToken);
            }
            state.dataRuntimeLangProcessor->PEToken(NumTokens).Variable = NewEMSVariable(state, StringToken, StackNum);

        } else if (is_any_of(NextChar, "+-*/^=<>@|&")) {
            // Parse an operator token
            if (NextChar == '-') {
                StringToken = "-";
                if (MultFound) {
                    ShowSevereError(state, format("EMS Parse Expression, for \"{}\".", state.dataRuntimeLang->ErlStack(StackNum).Name));
                    ShowContinueError(state, format("...Line = \"{}\".", Line));
                    ShowContinueError(state, "...Minus sign used on the right side of multiplication sign.");
                    ShowContinueError(state, "...Use parenthesis to wrap appropriate variables. For example, X * ( -Y ).");
                    ++NumErrors;
                    MultFound = false;
                } else if (DivFound) {
                    ShowSevereError(state, format("EMS Parse Expression, for \"{}\".", state.dataRuntimeLang->ErlStack(StackNum).Name));
                    ShowContinueError(state, format("...Line = \"{}\".", Line));
                    ShowContinueError(state, "...Minus sign used on the right side of division sign.");
                    ShowContinueError(state, "...Use parenthesis to wrap appropriate variables. For example, X / ( -Y ).");
                    ++NumErrors;
                    DivFound = false;
                } else if (OperatorProcessing && (NextChar == '-')) {
                    // if operator was deterined last pass and this character is a -, then insert a 0 before the minus and treat as subtraction
                    // example: change "Var == -1" to "Var == 0-1"
                    OperatorProcessing = false;
                    String.insert(Pos, "0");
                    ++LastPos;
                    StringToken = "0";
                    MultFound = false;
                    DivFound = false;
                } else {
                    StringToken = NextChar;
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Type = Token::Operator;
                }
            } else { // any other character process as operator
                StringToken = NextChar;
                state.dataRuntimeLangProcessor->PEToken(NumTokens).Type = Token::Operator;
            }

            // parse an operator if found,
            // returns true and increments position, other wise returns false and leaves state untouched
            const auto parse = [&](const char *string, ErlFunc op, bool case_insensitive) {
                const size_t len = strlen(string);
                const std::string potential_match = String.substr(Pos, len);

                if ((case_insensitive && Util::SameString(potential_match, string)) || (!case_insensitive && potential_match == string)) {
                    if (state.dataSysVars->DeveloperFlag) {
                        print(state.files.debug, "OPERATOR \"{}\"\n", potential_match);
                    }
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = op;
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).String = potential_match;
                    Pos += (len - 1);
                    return true;
                } else {
                    return false;
                }
            };

            // case insensitive wrapper call to parse
            const auto i_parse = [&](const char *string, const ErlFunc op) { return parse(string, op, true); };

            // First check for two character operators:  == <> <= >= || &&
            std::string const cc(String.substr(Pos, 2));
            if (parse("==", ErlFunc::Equal, false) || parse("<>", ErlFunc::NotEqual, false) || parse("<=", ErlFunc::LessOrEqual, false) ||
                parse(">=", ErlFunc::GreaterOrEqual, false) || parse("||", ErlFunc::LogicalOR, false) || parse("&&", ErlFunc::LogicalAND, false)) {
                // One of the comparision / logical operators
                OperatorProcessing = true;

            } else if (String[Pos] == '@') { // next check for builtin functions signaled by "@"

                if (i_parse("@Round", ErlFunc::Round) || i_parse("@Mod", ErlFunc::Mod) || i_parse("@Sin", ErlFunc::Sin) ||
                    i_parse("@Cos", ErlFunc::Cos) || i_parse("@ArcCos", ErlFunc::ArcCos) || i_parse("@ArcSin", ErlFunc::ArcSin) ||
                    i_parse("@DegToRad", ErlFunc::DegToRad) || i_parse("@RadToDeg", ErlFunc::RadToDeg) || i_parse("@Exp", ErlFunc::Exp) ||
                    i_parse("@Ln", ErlFunc::Ln) || i_parse("@Max", ErlFunc::Max) || i_parse("@Min", ErlFunc::Min) || i_parse("@Abs", ErlFunc::ABS) ||
                    i_parse("@RANDOMUNIFORM", ErlFunc::RandU) || i_parse("@RANDOMNORMAL", ErlFunc::RandG) ||
                    i_parse("@SEEDRANDOM", ErlFunc::RandSeed) || i_parse("@RhoAirFnPbTdbW", ErlFunc::RhoAirFnPbTdbW) ||
                    i_parse("@CpAirFnW", ErlFunc::CpAirFnW) || i_parse("@HfgAirFnWTdb", ErlFunc::HfgAirFnWTdb) ||
                    i_parse("@HgAirFnWTdb", ErlFunc::HgAirFnWTdb) || i_parse("@TdpFnTdbTwbPb", ErlFunc::TdpFnTdbTwbPb) ||
                    i_parse("@TdpFnWPb", ErlFunc::TdpFnWPb) || i_parse("@HFnTdbW", ErlFunc::HFnTdbW) || i_parse("@HFnTdbRhPb", ErlFunc::HFnTdbRhPb) ||
                    i_parse("@TdbFnHW", ErlFunc::TdbFnHW) || i_parse("@RhovFnTdbRhLBnd0C", ErlFunc::RhovFnTdbRhLBnd0C) ||
                    i_parse("@RhovFnTdbRh", ErlFunc::RhovFnTdbRh) || i_parse("@RhovFnTdbWPb", ErlFunc::RhovFnTdbWPb) ||
                    i_parse("@RhFnTdbRhovLBnd0C", ErlFunc::RhFnTdbRhovLBnd0C) || i_parse("@RhFnTdbRhov", ErlFunc::RhFnTdbRhov) ||
                    i_parse("@RhFnTdbWPb", ErlFunc::RhFnTdbWPb) || i_parse("@TwbFnTdbWPb", ErlFunc::TwbFnTdbWPb) ||
                    i_parse("@VFnTdbWPb", ErlFunc::VFnTdbWPb) || i_parse("@WFnTdpPb", ErlFunc::WFnTdpPb) || i_parse("@WFnTdbH", ErlFunc::WFnTdbH) ||
                    i_parse("@WFnTdbTwbPb", ErlFunc::WFnTdbTwbPb) || i_parse("@WFnTdbRhPb", ErlFunc::WFnTdbRhPb) ||
                    i_parse("@PsatFnTemp", ErlFunc::PsatFnTemp) || i_parse("@TsatFnHPb", ErlFunc::TsatFnHPb) ||
                    i_parse("@TsatFnPb", ErlFunc::TsatFnPb) || i_parse("@CpCW", ErlFunc::CpCW) || i_parse("@CpHW", ErlFunc::CpHW) ||
                    i_parse("@RhoH2O", ErlFunc::RhoH2O) || i_parse("@FATALHALTEP", ErlFunc::FatalHaltEp) ||
                    i_parse("@SEVEREWARNEP", ErlFunc::SevereWarnEp) || i_parse("@WARNEP", ErlFunc::WarnEp) ||
                    i_parse("@TRENDVALUE", ErlFunc::TrendValue) || i_parse("@TRENDAVERAGE", ErlFunc::TrendAverage) ||
                    i_parse("@TRENDMAX", ErlFunc::TrendMax) || i_parse("@TRENDMIN", ErlFunc::TrendMin) ||
                    i_parse("@TRENDDIRECTION", ErlFunc::TrendDirection) || i_parse("@TRENDSUM", ErlFunc::TrendSum) ||
                    i_parse("@CURVEVALUE", ErlFunc::CurveValue) || i_parse("@TODAYISRAIN", ErlFunc::TodayIsRain) ||
                    i_parse("@TODAYISSNOW", ErlFunc::TodayIsSnow) || i_parse("@TODAYOUTDRYBULBTEMP", ErlFunc::TodayOutDryBulbTemp) ||
                    i_parse("@TODAYOUTDEWPOINTTEMP", ErlFunc::TodayOutDewPointTemp) || i_parse("@TODAYOUTBAROPRESS", ErlFunc::TodayOutBaroPress) ||
                    i_parse("@TODAYOUTRELHUM", ErlFunc::TodayOutRelHum) || i_parse("@TODAYWINDSPEED", ErlFunc::TodayWindSpeed) ||
                    i_parse("@TODAYWINDDIR", ErlFunc::TodayWindDir) || i_parse("@TODAYSKYTEMP", ErlFunc::TodaySkyTemp) ||
                    i_parse("@TODAYHORIZIRSKY", ErlFunc::TodayHorizIRSky) || i_parse("@TODAYBEAMSOLARRAD", ErlFunc::TodayBeamSolarRad) ||
                    i_parse("@TODAYDIFSOLARRAD", ErlFunc::TodayDifSolarRad) || i_parse("@TODAYALBEDO", ErlFunc::TodayAlbedo) ||
                    i_parse("@TODAYLIQUIDPRECIP", ErlFunc::TodayLiquidPrecip) || i_parse("@TOMORROWISRAIN", ErlFunc::TomorrowIsRain) ||
                    i_parse("@TOMORROWISSNOW", ErlFunc::TomorrowIsSnow) || i_parse("@TOMORROWOUTDRYBULBTEMP", ErlFunc::TomorrowOutDryBulbTemp) ||
                    i_parse("@TOMORROWOUTDEWPOINTTEMP", ErlFunc::TomorrowOutDewPointTemp) ||
                    i_parse("@TOMORROWOUTBAROPRESS", ErlFunc::TomorrowOutBaroPress) || i_parse("@TOMORROWOUTRELHUM", ErlFunc::TomorrowOutRelHum) ||
                    i_parse("@TOMORROWWINDSPEED", ErlFunc::TomorrowWindSpeed) || i_parse("@TOMORROWWINDDIR", ErlFunc::TomorrowWindDir) ||
                    i_parse("@TOMORROWSKYTEMP", ErlFunc::TomorrowSkyTemp) || i_parse("@TOMORROWHORIZIRSKY", ErlFunc::TomorrowHorizIRSky) ||
                    i_parse("@TOMORROWBEAMSOLARRAD", ErlFunc::TomorrowBeamSolarRad) ||
                    i_parse("@TOMORROWDIFSOLARRAD", ErlFunc::TomorrowDifSolarRad) || i_parse("@TOMORROWALBEDO", ErlFunc::TomorrowAlbedo) ||
                    i_parse("@TOMORROWLIQUIDPRECIP", ErlFunc::TomorrowLiquidPrecip)) {
                    // was a built in function operator
                } else { // throw error
                    if (state.dataSysVars->DeveloperFlag) {
                        print(state.files.debug, "ERROR \"{}\"\n", String);
                    }
                    ShowFatalError(state, format("EMS Runtime Language: did not find valid input for built-in function ={}", String));
                }
            } else {
                // Check for remaining single character operators
                state.dataRuntimeLangProcessor->PEToken(NumTokens).String = StringToken;
                MultFound = false;
                DivFound = false;

                if (state.dataSysVars->DeveloperFlag) {
                    print(state.files.debug, "OPERATOR \"{}\"\n", StringToken);
                }

                if (StringToken == "+") {
                    if (!OperatorProcessing) {
                        state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::Add;
                        OperatorProcessing = true;
                    } else {
                        PlusFound = true;
                        OperatorProcessing = false;
                    }
                } else if (StringToken == "-") {
                    if (!OperatorProcessing) {
                        state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::Subtract;
                        OperatorProcessing = true;
                    } else {
                        MinusFound = true;
                        OperatorProcessing = false;
                    }
                } else if (StringToken == "*") {
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::Multiply;
                    MultFound = true;
                    OperatorProcessing = true;
                } else if (StringToken == "/") {
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::Divide;
                    DivFound = true;
                    OperatorProcessing = true;
                } else if (StringToken == "<") {
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::LessThan;
                    OperatorProcessing = true;
                } else if (StringToken == ">") {
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::GreaterThan;
                    OperatorProcessing = true;
                } else if (StringToken == "^") {
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Operator = ErlFunc::RaiseToPower;
                    OperatorProcessing = true;
                } else if (StringToken == "0" && (NextChar == '-')) {
                    // process string insert = "0"
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).Type = Token::Number;
                    state.dataRuntimeLangProcessor->PEToken(NumTokens).String = StringToken;
                } else {
                    // Uh OH, this should never happen! throw error
                    if (state.dataSysVars->DeveloperFlag) {
                        print(state.files.debug, "ERROR \"{}\"\n", StringToken);
                    }
                    ShowFatalError(state, format("EMS, caught unexpected token = \"{}\" ; while parsing string={}", StringToken, String));
                }
            }

            ++Pos;

        } else if (is_any_of(NextChar, "()")) {
            // Parse a parenthesis token
            ++Pos;
            StringToken = NextChar;
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "PAREN \"{}\"\n", StringToken);
            }
            state.dataRuntimeLangProcessor->PEToken(NumTokens).Type = Token::Parenthesis;
            state.dataRuntimeLangProcessor->PEToken(NumTokens).String = StringToken;
            if (NextChar == '(') {
                state.dataRuntimeLangProcessor->PEToken(NumTokens).Parenthesis = Token::ParenthesisLeft;
                OperatorProcessing = true;
            }
            if (NextChar == ')') {
                state.dataRuntimeLangProcessor->PEToken(NumTokens).Parenthesis = Token::ParenthesisRight;
            }

        } else if (is_any_of(NextChar, "\"")) {
            // Parse a string literal token
            if (state.dataSysVars->DeveloperFlag) {
                print(state.files.debug, "{}\n", "LITERAL STRING");
            }
            ++Pos;

        } else {
            // Error: bad start to the token
        }
    }

    if (NumErrors > 0) {
        if (state.dataSysVars->DeveloperFlag) {
            print(state.files.debug, "{}\n", "ERROR OUT");
        }
        ShowFatalError(state, "EMS, previous errors cause termination.");
    }

    ExpressionNum = ProcessTokens(state, state.dataRuntimeLangProcessor->PEToken, NumTokens, StackNum, String);
}

int ProcessTokens(
    EnergyPlusData &state, const Array1D<TokenType> &TokenIN, int const NumTokensIN, int const StackNum, std::string const &ParsingString)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Processes tokens into expressions.

    // METHODOLOGY EMPLOYED:
    // Uses recursion to handle tokens with compound expressions

    // Return value
    int ExpressionNum;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Pos;
    int LastPos;
    int TokenNum;
    int NumTokens;
    int Depth;
    int NumSubTokens;
    int NewNumTokens;
    int OperatorNum;
    int NumOperands;
    int ParenthWhileCounter; // used to trap for unbalanced parentheses

    // Object Data
    Array1D<TokenType> Token(TokenIN);
    Array1D<TokenType> SubTokenList;

    ExpressionNum = 0;
    NumTokens = NumTokensIN;

    // Process parentheses
    Pos = 0;
    for (TokenNum = 1; TokenNum <= NumTokens; ++TokenNum) {
        if (Token(TokenNum).Type == Token::Parenthesis) {
            Pos = TokenNum;
            break;
        }
    }

    ParenthWhileCounter = 0;

    while ((Pos > 0) && (ParenthWhileCounter < 50)) {
        ++ParenthWhileCounter;
        Depth = 0;
        for (TokenNum = 1; TokenNum <= NumTokens; ++TokenNum) {
            if (Token(TokenNum).Type == Token::Parenthesis) {
                if (Token(TokenNum).Parenthesis == Token::ParenthesisLeft) {
                    if (Depth == 0) {
                        Pos = TokenNum; // Record position of first left parenthesis
                    }
                    ++Depth;
                }
                if (Token(TokenNum).Parenthesis == Token::ParenthesisRight) {
                    --Depth;
                    if (Depth == 0) {
                        LastPos = TokenNum;
                        NumSubTokens = LastPos - Pos - 1;
                        SubTokenList.allocate(NumSubTokens);
                        SubTokenList({1, NumSubTokens}) = Token({Pos + 1, LastPos - 1}); // Need to check that these don't exceed bounds
                        ExpressionNum = ProcessTokens(state, SubTokenList, NumSubTokens, StackNum, ParsingString);
                        SubTokenList.deallocate();

                        // Replace the parenthetical tokens with one expression token
                        NewNumTokens = NumTokens - NumSubTokens - 1;
                        if (NewNumTokens > 0) {
                            if (LastPos + 1 <= NumTokens) {
                                Token({Pos + 1, NewNumTokens}) = Token({LastPos + 1, _});
                            }
                            Token.redimension(NewNumTokens);
                            Token(Pos).Type = Token::Expression;
                            Token(Pos).Expression = ExpressionNum;
                            Token(Pos).String = "Expr";
                            NumTokens = NewNumTokens;
                        }

                        // Reset loop for next parenthetical set
                        break;
                    }
                }
            }
        }

        // This repeats code again...  Just checks to see if there are any more parentheses to be found
        Pos = 0;
        for (TokenNum = 1; TokenNum <= NumTokens; ++TokenNum) {
            if (Token(TokenNum).Type == Token::Parenthesis) {
                Pos = TokenNum;
                break;
            }
        }
    }

    if (ParenthWhileCounter == 50) { // symptom of mismatched parenthesis
        ShowSevereError(state, "EMS error parsing parentheses, check that parentheses are balanced");
        ShowContinueError(state, format("String being parsed=\"{}\".", ParsingString));
        ShowFatalError(state, "Program terminates due to preceding error.");
    }

    // Process operators and builtin functions
    // Loop thru all operators and group expressions in the order of precedence
    for (OperatorNum = 1; OperatorNum <= NumPossibleOperators; ++OperatorNum) {

        // Find the next occurrence of the operator
        Pos = 0; //  position in sequence of tokens
        for (TokenNum = 1; TokenNum <= NumTokens; ++TokenNum) {
            if ((Token(TokenNum).Type == Token::Operator) && (Token(TokenNum).Operator == static_cast<ErlFunc>(OperatorNum))) {
                Pos = TokenNum;
                break;
            }
        }

        while (Pos > 0) {
            if (Pos == 1) {
                // if first token is for a built-in function starting with "@" then okay, otherwise the operator needs a LHS
                if (static_cast<int>(Token(TokenNum).Operator) > static_cast<int>(ErlFunc::LogicalOR)) { // we have a function expression to set up
                    ExpressionNum = NewExpression(state);
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operator = static_cast<ErlFunc>(OperatorNum);
                    NumOperands = ErlFuncNumOperands[OperatorNum];
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).NumOperands = NumOperands;
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand.allocate(NumOperands);

                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Type = static_cast<Value>(static_cast<int>(Token(Pos + 1).Type));
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Number = Token(Pos + 1).Number;
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Expression = Token(Pos + 1).Expression;
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Variable = Token(Pos + 1).Variable;
                    if (Token(Pos + 1).Variable > 0) {
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).TrendVariable =
                            state.dataRuntimeLang->ErlVariable(Token(Pos + 1).Variable).Value.TrendVariable;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).TrendVarPointer =
                            state.dataRuntimeLang->ErlVariable(Token(Pos + 1).Variable).Value.TrendVarPointer;
                    }
                    if ((NumOperands >= 2) && (NumTokens >= 3)) {
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Type =
                            static_cast<Value>(static_cast<int>(Token(Pos + 2).Type));
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Number = Token(Pos + 2).Number;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Expression = Token(Pos + 2).Expression;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Variable = Token(Pos + 2).Variable;
                    }

                    if ((NumOperands >= 3) && (NumTokens >= 4)) {
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(3).Type =
                            static_cast<Value>(static_cast<int>(Token(Pos + 3).Type));
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(3).Number = Token(Pos + 3).Number;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(3).Expression = Token(Pos + 3).Expression;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(3).Variable = Token(Pos + 3).Variable;
                        if ((NumOperands == 3) && (NumTokens - 4 > 0)) { // too many tokens for this non-binary operator
                            ShowFatalError(state, "EMS error parsing tokens, too many for built-in function");
                        }
                    }

                    if ((NumOperands >= 4) && (NumTokens >= 5)) {
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(4).Type =
                            static_cast<Value>(static_cast<int>(Token(Pos + 4).Type));
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(4).Number = Token(Pos + 4).Number;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(4).Expression = Token(Pos + 4).Expression;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(4).Variable = Token(Pos + 4).Variable;
                        if ((NumOperands == 4) && (NumTokens - 5 > 0)) { // too many tokens for this non-binary operator
                            ShowFatalError(state, "EMS error parsing tokens, too many for built-in function");
                        }
                    }

                    if ((NumOperands == 5) && (NumTokens >= 6)) {
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(5).Type =
                            static_cast<Value>(static_cast<int>(Token(Pos + 5).Type));
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(5).Number = Token(Pos + 5).Number;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(5).Expression = Token(Pos + 5).Expression;
                        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(5).Variable = Token(Pos + 5).Variable;
                        if ((NumOperands == 5) && (NumTokens - 6 > 0)) { // too many tokens for this non-binary operator
                            ShowFatalError(state, "EMS error parsing tokens, too many for  built-in function");
                        }
                    }
                    break;
                } else {
                    ShowSevereError(state, format("The operator \"{}\" is missing the left-hand operand!", ErlFuncNamesUC[OperatorNum]));
                    ShowContinueError(state, format("String being parsed=\"{}\".", ParsingString));
                    break;
                }
            } else if (Pos == NumTokens) {
                ShowSevereError(state, format("The operator \"{}\" is missing the right-hand operand!", ErlFuncNamesUC[OperatorNum]));
                ShowContinueError(state, format("String being parsed=\"{}\".", ParsingString));
                break;
            } else {

                ExpressionNum = NewExpression(state);
                state.dataRuntimeLang->ErlExpression(ExpressionNum).Operator = static_cast<ErlFunc>(OperatorNum);
                NumOperands = ErlFuncNumOperands[OperatorNum];
                state.dataRuntimeLang->ErlExpression(ExpressionNum).NumOperands = NumOperands;
                state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand.allocate(NumOperands);

                // PE commment: Need a right-hand and left-hand check for these, not just number of operators
                // Unification of TYPEs would turn these into one-liners

                state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Type = static_cast<Value>(static_cast<int>(Token(Pos - 1).Type));
                state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Number = Token(Pos - 1).Number;
                state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Expression = Token(Pos - 1).Expression;
                state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Variable = Token(Pos - 1).Variable;

                if (NumOperands >= 2) {
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Type = static_cast<Value>(static_cast<int>(Token(Pos + 1).Type));
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Number = Token(Pos + 1).Number;
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Expression = Token(Pos + 1).Expression;
                    state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(2).Variable = Token(Pos + 1).Variable;
                }

                // Replace the three tokens with one expression token
                if ((NumOperands == 2) && (NumTokens - 2 > 0)) {
                    if (Pos + 2 <= NumTokens) {
                        Token({Pos, NumTokens - 2}) = Token({Pos + 2, _});
                    }
                    Token(Pos - 1).Type = Token::Expression;
                    Token(Pos - 1).Expression = ExpressionNum;
                    Token(Pos - 1).String = "Expr";
                    NumTokens -= 2;
                    Token.redimension(NumTokens);
                }
            }

            // Find the next occurrence of the operator  (this repeats code, but don't have better idea)
            Pos = 0;
            for (TokenNum = 1; TokenNum <= NumTokens; ++TokenNum) {
                if ((Token(TokenNum).Type == Token::Operator) && (Token(TokenNum).Operator == static_cast<ErlFunc>(OperatorNum))) {
                    Pos = TokenNum;
                    break;
                }
            }
        }
    }

    // Should be down to just one token now
    if (Token(1).Type == Token::Number) {
        ExpressionNum = NewExpression(state);
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operator = ErlFunc::Literal;
        state.dataRuntimeLang->ErlExpression(ExpressionNum).NumOperands = 1;
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand.allocate(1);
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Type = static_cast<Value>(static_cast<int>(Token(1).Type));
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Number = Token(1).Number;
    } else if (Token(1).Type == Token::Variable) {
        ExpressionNum = NewExpression(state);
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operator = ErlFunc::Literal;
        state.dataRuntimeLang->ErlExpression(ExpressionNum).NumOperands = 1;
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand.allocate(1);
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Type = static_cast<Value>(static_cast<int>(Token(1).Type));
        state.dataRuntimeLang->ErlExpression(ExpressionNum).Operand(1).Variable = Token(1).Variable;
    }

    Token.deallocate();

    return ExpressionNum;
}

int NewExpression(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Creates a new expression.

    // METHODOLOGY EMPLOYED:

    // Return value

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION LOCAL VARIABLE DECLARATIONS:

    // Object Data

    if (state.dataRuntimeLang->NumExpressions == 0) {
        state.dataRuntimeLang->ErlExpression.allocate(1);
        state.dataRuntimeLang->NumExpressions = 1;
    } else {
        state.dataRuntimeLang->ErlExpression.redimension(++state.dataRuntimeLang->NumExpressions);
    }

    return state.dataRuntimeLang->NumExpressions;
}

ErlValueType EvaluateExpression(EnergyPlusData &state, int const ExpressionNum, bool &seriousErrorFound)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith, May 2009
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Evaluates an expression.

    // METHODOLOGY EMPLOYED:

    // USE, INTRINSIC :: IEEE_ARITHMETIC, ONLY : IEEE_IS_NAN ! Use IEEE_IS_NAN when GFortran supports it
    // Using/Aliasing
    using namespace Psychrometrics;
    using Curve::CurveValue;

    // Return value
    ErlValueType ReturnValue;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int thisTrend;      // local temporary
    int thisIndex;      // local temporary
    Real64 thisAverage; // local temporary
    int loop;           // local temporary
    Real64 thisSlope;   // local temporary
    Real64 thisMax;     // local temporary
    Real64 thisMin;     // local temporary
    int OperandNum;
    int SeedN;              // number of digits in the number used to seed the generator
    Array1D_int SeedIntARR; // local temporary for random seed
    Real64 tmpRANDU1;       // local temporary for uniform random number
    Real64 tmpRANDU2;       // local temporary for uniform random number
    Real64 tmpRANDG;        // local temporary for gaussian random number
    Real64 UnitCircleTest;  // local temporary for Box-Muller algo
    Real64 TestValue;       // local temporary

    // Object Data
    Array1D<ErlValueType> Operand;

    constexpr std::string_view EMSBuiltInFunction = "EMS Built-In Function";

    ReturnValue.Type = Value::Number;
    ReturnValue.Number = 0.0;

    if (ExpressionNum > 0) {
        auto const &thisErlExpression = state.dataRuntimeLang->ErlExpression(ExpressionNum);
        // is there a way to keep these and not allocate and deallocate all the time?
        Operand.allocate(thisErlExpression.NumOperands);
        // Reduce operands down to literals
        for (OperandNum = 1; OperandNum <= thisErlExpression.NumOperands; ++OperandNum) {
            auto &thisOperand = Operand(OperandNum);
            thisOperand = thisErlExpression.Operand(OperandNum);
            if (thisOperand.Type == Value::Expression) {
                thisOperand = EvaluateExpression(state, thisOperand.Expression, seriousErrorFound); // recursive call
                // check if recursive call found an error in nested expression, want to preserve error message from that
                if (seriousErrorFound) {
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = thisOperand.Error;
                }

            } else if (thisOperand.Type == Value::Variable) {
                auto const &thisErlVar = state.dataRuntimeLang->ErlVariable(thisOperand.Variable);
                if (thisErlVar.Value.initialized) { // check that value has been initialized
                    thisOperand = thisErlVar.Value;
                } else { // value has never been set
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "EvaluateExpression: Variable = '" + thisErlVar.Name + "' used in expression has not been initialized!";
                    if (!state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation && !state.dataEMSMgr->FinishProcessingUserInput) {

                        // check if this is an arg in CurveValue,
                        if (thisErlExpression.Operator !=
                            ErlFunc::CurveValue) { // padding the argument list for CurveValue is too common to fatal on.  only reported to EDD
                            seriousErrorFound = true;
                        }
                    }
                }
            }
        }

        if (ReturnValue.Type != Value::Error) {

            // Perform the operation

            switch (thisErlExpression.Operator) {

            case ErlFunc::Literal:
                ReturnValue = Operand(1);
                ReturnValue.initialized = true;
                break;

            case ErlFunc::Negative: // unary minus sign.  parsing does not work yet
                ReturnValue = SetErlValueNumber(-1.0 * Operand(1).Number);
                break;

            case ErlFunc::Divide:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if (Operand(2).Number == 0.0) {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "EvaluateExpression: Divide By Zero in EMS Program!";
                        if (!state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation && !state.dataEMSMgr->FinishProcessingUserInput) {
                            seriousErrorFound = true;
                        }
                    } else {
                        ReturnValue = SetErlValueNumber(Operand(1).Number / Operand(2).Number);
                    }
                }
                break;

            case ErlFunc::Multiply:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    ReturnValue = SetErlValueNumber(Operand(1).Number * Operand(2).Number);
                }
                break;

            case ErlFunc::Subtract:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    ReturnValue = SetErlValueNumber(Operand(1).Number - Operand(2).Number);
                }
                break;

            case ErlFunc::Add:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    ReturnValue = SetErlValueNumber(Operand(1).Number + Operand(2).Number);
                }
                break;

            case ErlFunc::Equal:
                if (Operand(1).Type == Operand(2).Type) {
                    if (Operand(1).Type == Value::Null) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else if ((Operand(1).Type == Value::Number) && (Operand(1).Number == Operand(2).Number)) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                } else {
                    ReturnValue = state.dataRuntimeLang->False;
                }
                break;

            case ErlFunc::NotEqual:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if (Operand(1).Number != Operand(2).Number) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::LessOrEqual:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if (Operand(1).Number <= Operand(2).Number) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::GreaterOrEqual:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if (Operand(1).Number >= Operand(2).Number) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::LessThan:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if (Operand(1).Number < Operand(2).Number) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::GreaterThan:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if (Operand(1).Number > Operand(2).Number) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::RaiseToPower:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    TestValue = std::pow(Operand(1).Number, Operand(2).Number);
                    if (std::isnan(TestValue)) {
                        // throw Error
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error =
                            format("EvaluateExpression: Attempted to raise to power with incompatible numbers: {:.6T} raised to {:.6T}",
                                   Operand(1).Number,
                                   Operand(2).Number);
                        if (!state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation && !state.dataEMSMgr->FinishProcessingUserInput) {
                            seriousErrorFound = true;
                        }
                    } else {
                        ReturnValue = SetErlValueNumber(TestValue);
                    }
                }
                break;

            case ErlFunc::LogicalAND:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if ((Operand(1).Number == state.dataRuntimeLang->True.Number) && (Operand(2).Number == state.dataRuntimeLang->True.Number)) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::LogicalOR:
                if ((Operand(1).Type == Value::Number) && (Operand(2).Type == Value::Number)) {
                    if ((Operand(1).Number == state.dataRuntimeLang->True.Number) || (Operand(2).Number == state.dataRuntimeLang->True.Number)) {
                        ReturnValue = state.dataRuntimeLang->True;
                    } else {
                        ReturnValue = state.dataRuntimeLang->False;
                    }
                }
                break;

            case ErlFunc::Round:
                ReturnValue = SetErlValueNumber(nint(Operand(1).Number));
                break;

            case ErlFunc::Mod:
                ReturnValue = SetErlValueNumber(mod(Operand(1).Number, Operand(2).Number));
                break;

            case ErlFunc::Sin:
                ReturnValue = SetErlValueNumber(std::sin(Operand(1).Number));
                break;

            case ErlFunc::Cos:
                ReturnValue = SetErlValueNumber(std::cos(Operand(1).Number));
                break;

            case ErlFunc::ArcSin:
                ReturnValue = SetErlValueNumber(std::asin(Operand(1).Number));
                break;

            case ErlFunc::ArcCos:
                ReturnValue = SetErlValueNumber(std::acos(Operand(1).Number));
                break;

            case ErlFunc::DegToRad:
                ReturnValue = SetErlValueNumber(Operand(1).Number * Constant::DegToRad);
                break;

            case ErlFunc::RadToDeg:
                ReturnValue = SetErlValueNumber(Operand(1).Number / Constant::DegToRad);
                break;

            case ErlFunc::Exp:
                if ((Operand(1).Number < 700.0) && (Operand(1).Number > -20.0)) {
                    ReturnValue = SetErlValueNumber(std::exp(Operand(1).Number));
                } else if (Operand(1).Number <= -20.0) {
                    ReturnValue = SetErlValueNumber(0.0);
                } else {
                    // throw Error
                    ReturnValue.Error =
                        format("EvaluateExpression: Attempted to calculate exponential value of too large a number: {:.4T}", Operand(1).Number);
                    ReturnValue.Type = Value::Error;
                    if (!state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation && !state.dataEMSMgr->FinishProcessingUserInput) {
                        seriousErrorFound = true;
                    }
                }
                break;

            case ErlFunc::Ln:
                if (Operand(1).Number > 0.0) {
                    ReturnValue = SetErlValueNumber(std::log(Operand(1).Number));
                } else {
                    // throw error,
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = format("EvaluateExpression: Natural Log of zero or less! ln of value = {:.4T}", Operand(1).Number);
                    if (!state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation && !state.dataEMSMgr->FinishProcessingUserInput) {
                        seriousErrorFound = true;
                    }
                }
                break;

            case ErlFunc::Max:
                ReturnValue = SetErlValueNumber(max(Operand(1).Number, Operand(2).Number));
                break;

            case ErlFunc::Min:
                ReturnValue = SetErlValueNumber(min(Operand(1).Number, Operand(2).Number));
                break;

            case ErlFunc::ABS:
                ReturnValue = SetErlValueNumber(std::abs(Operand(1).Number));
                break;

            case ErlFunc::RandU:
                RANDOM_NUMBER(tmpRANDU1);
                tmpRANDU1 = Operand(1).Number + (Operand(2).Number - Operand(1).Number) * tmpRANDU1;
                ReturnValue = SetErlValueNumber(tmpRANDU1);
                break;

            case ErlFunc::RandG:
                while (true) { // Box-Muller algorithm
                    RANDOM_NUMBER(tmpRANDU1);
                    RANDOM_NUMBER(tmpRANDU2);
                    tmpRANDU1 = 2.0 * tmpRANDU1 - 1.0;
                    tmpRANDU2 = 2.0 * tmpRANDU2 - 1.0;
                    UnitCircleTest = square(tmpRANDU1) + square(tmpRANDU2);
                    if (UnitCircleTest > 0.0 && UnitCircleTest < 1.0) {
                        break;
                    }
                }
                tmpRANDG = std::sqrt(-2.0 * std::log(UnitCircleTest) / UnitCircleTest);
                tmpRANDG *= tmpRANDU1; // standard normal ran
                //  x     = ran      * sigma             + mean
                tmpRANDG = tmpRANDG * Operand(2).Number + Operand(1).Number;
                tmpRANDG = max(tmpRANDG, Operand(3).Number); // min limit
                tmpRANDG = min(tmpRANDG, Operand(4).Number); // max limit
                ReturnValue = SetErlValueNumber(tmpRANDG);
                break;

            case ErlFunc::RandSeed:
                // convert arg to an integer array for the seed.
                RANDOM_SEED(SeedN); // obtains processor's use size as output
                SeedIntARR.allocate(SeedN);
                for (loop = 1; loop <= SeedN; ++loop) {
                    if (loop == 1) {
                        SeedIntARR(loop) = std::floor(Operand(1).Number);
                    } else {
                        SeedIntARR(loop) = std::floor(Operand(1).Number) * loop;
                    }
                }
                RANDOM_SEED(_, SeedIntARR);
                ReturnValue = SetErlValueNumber(double(SeedIntARR(1))); // just return first number pass as seed
                SeedIntARR.deallocate();
                break;

            case ErlFunc::RhoAirFnPbTdbW:
                ReturnValue = SetErlValueNumber(PsyRhoAirFnPbTdbW(state,
                                                                  Operand(1).Number,
                                                                  Operand(2).Number,
                                                                  Operand(3).Number,
                                                                  EMSBuiltInFunction)); // result =>   density of moist air (kg/m3) | pressure
                                                                                        // (Pa) | drybulb (C) | Humidity ratio (kg water
                                                                                        // vapor/kg dry air) | called from
                break;

            case ErlFunc::CpAirFnW:
                ReturnValue = SetErlValueNumber(PsyCpAirFnW(Operand(1).Number)); // result =>   heat capacity of air
                                                                                 // {J/kg-C} | Humidity ratio (kg water vapor/kg dry air)
                break;

            case ErlFunc::HfgAirFnWTdb:
                // BG comment these two psych funct seems confusing (?) is this the enthalpy of water in the air?
                ReturnValue = SetErlValueNumber(PsyHfgAirFnWTdb(Operand(1).Number, Operand(2).Number)); // result =>   heat of vaporization
                                                                                                        // for moist air {J/kg} | Humidity
                                                                                                        // ratio (kg water vapor/kg dry air) |
                                                                                                        // drybulb (C)
                break;

            case ErlFunc::HgAirFnWTdb:
                // confusing ?  seems like this is really classical Hfg, heat of vaporization
                ReturnValue = SetErlValueNumber(PsyHgAirFnWTdb(Operand(1).Number, Operand(2).Number)); // result =>   enthalpy of the gas
                                                                                                       // {units?} | Humidity ratio (kg water
                                                                                                       // vapor/kg dry air) | drybulb (C)
                break;

            case ErlFunc::TdpFnTdbTwbPb:
                ReturnValue = SetErlValueNumber(
                    PsyTdpFnTdbTwbPb(state,
                                     Operand(1).Number,
                                     Operand(2).Number,
                                     Operand(3).Number,
                                     EMSBuiltInFunction)); // result =>   dew-point temperature {C} | drybulb (C) | wetbulb (C) | pressure (Pa)
                break;

            case ErlFunc::TdpFnWPb:
                ReturnValue = SetErlValueNumber(PsyTdpFnWPb(
                    state,
                    Operand(1).Number,
                    Operand(2).Number,
                    EMSBuiltInFunction)); // result =>  dew-point temperature {C} | Humidity ratio (kg water vapor/kg dry air) | pressure (Pa)
                break;

            case ErlFunc::HFnTdbW:
                ReturnValue = SetErlValueNumber(
                    PsyHFnTdbW(Operand(1).Number,
                               Operand(2).Number)); // result =>  enthalpy (J/kg) | drybulb (C) | Humidity ratio (kg water vapor/kg dry air)
                break;

            case ErlFunc::HFnTdbRhPb:
                ReturnValue = SetErlValueNumber(PsyHFnTdbRhPb(
                    state,
                    Operand(1).Number,
                    Operand(2).Number,
                    Operand(3).Number,
                    EMSBuiltInFunction)); // result =>  enthalpy (J/kg) | drybulb (C) | relative humidity value (0.0 - 1.0) | pressure (Pa)
                break;

            case ErlFunc::TdbFnHW:
                ReturnValue = SetErlValueNumber(PsyTdbFnHW(
                    Operand(1).Number,
                    Operand(2).Number)); // result =>  dry-bulb temperature {C} | enthalpy (J/kg) | Humidity ratio (kg water vapor/kg dry air)
                break;

            case ErlFunc::RhovFnTdbRh:
                ReturnValue = SetErlValueNumber(PsyRhovFnTdbRh(
                    state,
                    Operand(1).Number,
                    Operand(2).Number,
                    EMSBuiltInFunction)); // result =>  Vapor density in air (kg/m3) | drybulb (C) | relative humidity value (0.0 - 1.0)
                break;

            case ErlFunc::RhovFnTdbRhLBnd0C:
                ReturnValue = SetErlValueNumber(PsyRhovFnTdbRhLBnd0C(
                    Operand(1).Number,
                    Operand(2).Number)); // result =>  Vapor density in air (kg/m3) | drybulb (C) | relative humidity value (0.0 - 1.0)
                break;

            case ErlFunc::RhovFnTdbWPb:
                ReturnValue = SetErlValueNumber(
                    PsyRhovFnTdbWPb(Operand(1).Number, Operand(2).Number, Operand(3).Number)); // result =>  Vapor density in air (kg/m3) |
                                                                                               // drybulb (C) | Humidity ratio (kg water
                                                                                               // vapor/kg dry air) | pressure (Pa)
                break;

            case ErlFunc::RhFnTdbRhov:
                ReturnValue = SetErlValueNumber(
                    PsyRhFnTdbRhov(state,
                                   Operand(1).Number,
                                   Operand(2).Number,
                                   EMSBuiltInFunction)); // result => relative humidity value (0.0-1.0) | drybulb (C) | vapor density in air (kg/m3)
                break;

            case ErlFunc::RhFnTdbRhovLBnd0C:
                ReturnValue = SetErlValueNumber(
                    PsyRhFnTdbRhovLBnd0C(state,
                                         Operand(1).Number,
                                         Operand(2).Number,
                                         EMSBuiltInFunction)); // relative humidity value (0.0-1.0) | drybulb (C) | vapor density in air (kg/m3)
                break;

            case ErlFunc::RhFnTdbWPb:
                ReturnValue = SetErlValueNumber(PsyRhFnTdbWPb(state,
                                                              Operand(1).Number,
                                                              Operand(2).Number,
                                                              Operand(3).Number,
                                                              EMSBuiltInFunction)); // result =>  relative humidity value (0.0-1.0) | drybulb
                                                                                    // (C) | Humidity ratio (kg water vapor/kg dry air) |
                                                                                    // pressure (Pa)
                break;

            case ErlFunc::TwbFnTdbWPb:
                ReturnValue = SetErlValueNumber(PsyTwbFnTdbWPb(state,
                                                               Operand(1).Number,
                                                               Operand(2).Number,
                                                               Operand(3).Number,
                                                               EMSBuiltInFunction)); // result=> Temperature Wet-Bulb {C} | drybulb (C) |
                                                                                     // Humidity ratio (kg water vapor/kg dry air) | pressure
                                                                                     // (Pa)
                break;

            case ErlFunc::VFnTdbWPb:
                ReturnValue = SetErlValueNumber(PsyVFnTdbWPb(state,
                                                             Operand(1).Number,
                                                             Operand(2).Number,
                                                             Operand(3).Number,
                                                             EMSBuiltInFunction)); // result=> specific volume {m3/kg} | drybulb (C) |
                                                                                   // Humidity ratio (kg water vapor/kg dry air) | pressure
                                                                                   // (Pa)
                break;

            case ErlFunc::WFnTdpPb:
                ReturnValue = SetErlValueNumber(PsyWFnTdpPb(
                    state,
                    Operand(1).Number,
                    Operand(2).Number,
                    EMSBuiltInFunction)); // result=> humidity ratio  (kg water vapor/kg dry air) | dew point temperature (C) | pressure (Pa)
                break;

            case ErlFunc::WFnTdbH:
                ReturnValue = SetErlValueNumber(
                    PsyWFnTdbH(state,
                               Operand(1).Number,
                               Operand(2).Number,
                               EMSBuiltInFunction)); // result=> humidity ratio  (kg water vapor/kg dry air) | drybulb (C) | enthalpy (J/kg)
                break;

            case ErlFunc::WFnTdbTwbPb:
                ReturnValue = SetErlValueNumber(PsyWFnTdbTwbPb(state,
                                                               Operand(1).Number,
                                                               Operand(2).Number,
                                                               Operand(3).Number,
                                                               EMSBuiltInFunction)); // result=> humidity ratio  (kg water vapor/kg dry air) |
                                                                                     // drybulb (C) | wet-bulb temperature {C} | pressure (Pa)
                break;

            case ErlFunc::WFnTdbRhPb:
                ReturnValue = SetErlValueNumber(PsyWFnTdbRhPb(state,
                                                              Operand(1).Number,
                                                              Operand(2).Number,
                                                              Operand(3).Number,
                                                              EMSBuiltInFunction)); // result=> humidity ratio  (kg water vapor/kg dry air) |
                                                                                    // drybulb (C) | relative humidity value (0.0-1.0) |
                                                                                    // pressure (Pa)
                break;

            case ErlFunc::PsatFnTemp:
                ReturnValue = SetErlValueNumber(
                    PsyPsatFnTemp(state, Operand(1).Number, EMSBuiltInFunction)); // result=> saturation pressure {Pascals} | drybulb (C)
                break;

            case ErlFunc::TsatFnHPb:
                ReturnValue =
                    SetErlValueNumber(PsyTsatFnHPb(state,
                                                   Operand(1).Number,
                                                   Operand(2).Number,
                                                   EMSBuiltInFunction)); // result=> saturation temperature {C} | enthalpy {J/kg} | pressure (Pa)
                break;

            // I'm not sure why FuncTsatFnPb was commented out, but it goes all the way back to the Fortran implementation, so it's staying like that
            // for now.
            //      CASE (FuncTsatFnPb)
            //        ReturnValue = NumberValue( &   ! result=> saturation temperature {C}
            //                        PsyTsatFnPb(Operand(1)%Number, & ! pressure (Pa)
            //                                    'EMS Built-In Function') )
            case ErlFunc::CpCW:
                ReturnValue =
                    SetErlValueNumber(CPCW(Operand(1).Number)); // result => specific heat of water (J/kg-K) = 4180.d0 | temperature (C) unused
                break;

            case ErlFunc::CpHW:
                ReturnValue =
                    SetErlValueNumber(CPHW(Operand(1).Number)); // result => specific heat of water (J/kg-K) = 4180.d0 | temperature (C) unused
                break;

            case ErlFunc::RhoH2O:
                ReturnValue = SetErlValueNumber(RhoH2O(Operand(1).Number)); // result => density of water (kg/m3) | temperature (C)
                break;

            case ErlFunc::FatalHaltEp:
                ShowSevereError(state, "EMS user program found serious problem and is halting simulation");
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, format("EMS user program halted simulation with error code = {:.2T}", Operand(1).Number));
                ReturnValue = SetErlValueNumber(Operand(1).Number); // returns back the error code
                break;

            case ErlFunc::SevereWarnEp:
                ShowSevereError(state, format("EMS user program issued severe warning with error code = {:.2T}", Operand(1).Number));
                ShowContinueErrorTimeStamp(state, "");
                ReturnValue = SetErlValueNumber(Operand(1).Number); // returns back the error code
                break;

            case ErlFunc::WarnEp:
                ShowWarningError(state, format("EMS user program issued warning with error code = {:.2T}", Operand(1).Number));
                ShowContinueErrorTimeStamp(state, "");
                ReturnValue = SetErlValueNumber(Operand(1).Number); // returns back the error code
                break;

            case ErlFunc::TrendValue:
                // find TrendVariable , first operand is ErlVariable
                if (Operand(1).TrendVariable) {
                    thisTrend = Operand(1).TrendVarPointer;
                    // second operand is number for index
                    thisIndex = std::floor(Operand(2).Number);
                    if (thisIndex >= 1) {
                        if (thisIndex <= state.dataRuntimeLang->TrendVariable(thisTrend).LogDepth) {
                            ReturnValue = SetErlValueNumber(state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(thisIndex), Operand(1));
                        } else {
                            ReturnValue.Type = Value::Error;
                            ReturnValue.Error = "Built-in trend function called with index larger than what is being logged";
                        }
                    } else {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Built-in trend function called with index less than 1";
                    }
                } else { // not registered as a trend variable
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "Variable used with built-in trend function is not associated with a registered trend variable";
                }
                break;

            case ErlFunc::TrendAverage:
                // find TrendVariable , first operand is ErlVariable
                if (Operand(1).TrendVariable) {
                    thisTrend = Operand(1).TrendVarPointer;
                    thisIndex = std::floor(Operand(2).Number);
                    if (thisIndex >= 1) {
                        if (thisIndex <= state.dataRuntimeLang->TrendVariable(thisTrend).LogDepth) {
                            // calculate average
                            thisAverage = sum(state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR({1, thisIndex})) / double(thisIndex);
                            ReturnValue = SetErlValueNumber(thisAverage, Operand(1));
                        } else {
                            ReturnValue.Type = Value::Error;
                            ReturnValue.Error = "Built-in trend function called with index larger than what is being logged";
                        }
                    } else {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Built-in trend function called with index less than 1";
                    }
                } else { // not registered as a trend variable
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "Variable used with built-in trend function is not associated with a registered trend variable";
                }
                break;

            case ErlFunc::TrendMax:
                if (Operand(1).TrendVariable) {
                    thisTrend = Operand(1).TrendVarPointer;
                    thisIndex = std::floor(Operand(2).Number);
                    if (thisIndex >= 1) {
                        if (thisIndex <= state.dataRuntimeLang->TrendVariable(thisTrend).LogDepth) {
                            thisMax = 0.0;
                            if (thisIndex == 1) {
                                thisMax = state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(1);
                            } else {
                                for (loop = 2; loop <= thisIndex; ++loop) {
                                    if (loop == 2) {
                                        thisMax = max(state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(1),
                                                      state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(2));
                                    } else {
                                        thisMax = max(thisMax, state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(loop));
                                    }
                                }
                            }
                            ReturnValue = SetErlValueNumber(thisMax, Operand(1));
                        } else {
                            ReturnValue.Type = Value::Error;
                            ReturnValue.Error = "Built-in trend function called with index larger than what is being logged";
                        }
                    } else {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Built-in trend function called with index less than 1";
                    }
                } else { // not registered as a trend variable
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "Variable used with built-in trend function is not associated with a registered trend variable";
                }
                break;

            case ErlFunc::TrendMin:
                if (Operand(1).TrendVariable) {
                    thisTrend = Operand(1).TrendVarPointer;
                    thisIndex = std::floor(Operand(2).Number);
                    if (thisIndex >= 1) {
                        if (thisIndex <= state.dataRuntimeLang->TrendVariable(thisTrend).LogDepth) {
                            thisMin = 0.0;
                            if (thisIndex == 1) {
                                thisMin = state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(1);
                            } else {
                                for (loop = 2; loop <= thisIndex; ++loop) {
                                    if (loop == 2) {
                                        thisMin = min(state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(1),
                                                      state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(2));
                                    } else {
                                        thisMin = min(thisMin, state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR(loop));
                                    }
                                }
                            }
                            ReturnValue = SetErlValueNumber(thisMin, Operand(1));

                        } else {
                            ReturnValue.Type = Value::Error;
                            ReturnValue.Error = "Built-in trend function called with index larger than what is being logged";
                        }

                    } else {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Built-in trend function called with index less than 1";
                    }
                } else { // not registered as a trend variable
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "Variable used with built-in trend function is not associated with a registered trend variable";
                }
                break;

            case ErlFunc::TrendDirection:
                if (Operand(1).TrendVariable) {
                    // do a linear least squares fit and get slope of line
                    thisTrend = Operand(1).TrendVarPointer;
                    thisIndex = std::floor(Operand(2).Number);
                    if (thisIndex >= 1) {

                        if (thisIndex <= state.dataRuntimeLang->TrendVariable(thisTrend).LogDepth) {
                            // closed form solution for slope of linear least squares fit
                            thisSlope = (sum(state.dataRuntimeLang->TrendVariable(thisTrend).TimeARR({1, thisIndex})) *
                                             sum(state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR({1, thisIndex})) -
                                         thisIndex * sum((state.dataRuntimeLang->TrendVariable(thisTrend).TimeARR({1, thisIndex}) *
                                                          state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR({1, thisIndex})))) /
                                        (pow_2(sum(state.dataRuntimeLang->TrendVariable(thisTrend).TimeARR({1, thisIndex}))) -
                                         thisIndex * sum(pow(state.dataRuntimeLang->TrendVariable(thisTrend).TimeARR({1, thisIndex}), 2)));
                            ReturnValue = SetErlValueNumber(thisSlope, Operand(1)); // rate of change per hour
                        } else {
                            ReturnValue.Type = Value::Error;
                            ReturnValue.Error = "Built-in trend function called with index larger than what is being logged";
                        }

                    } else {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Built-in trend function called with index less than 1";
                    }
                } else { // not registered as a trend variable
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "Variable used with built-in trend function is not associated with a registered trend variable";
                }
                break;

            case ErlFunc::TrendSum:
                if (Operand(1).TrendVariable) {

                    thisTrend = Operand(1).TrendVarPointer;
                    thisIndex = std::floor(Operand(2).Number);
                    if (thisIndex >= 1) {
                        if (thisIndex <= state.dataRuntimeLang->TrendVariable(thisTrend).LogDepth) {
                            ReturnValue =
                                SetErlValueNumber(sum(state.dataRuntimeLang->TrendVariable(thisTrend).TrendValARR({1, thisIndex})), Operand(1));
                        } else {
                            ReturnValue.Type = Value::Error;
                            ReturnValue.Error = "Built-in trend function called with index larger than what is being logged";
                        }
                    } else {
                        ReturnValue.Type = Value::Error;
                        ReturnValue.Error = "Built-in trend function called with index less than 1";
                    }
                } else { // not registered as a trend variable
                    ReturnValue.Type = Value::Error;
                    ReturnValue.Error = "Variable used with built-in trend function is not associated with a registered trend variable";
                }
                break;

            case ErlFunc::CurveValue:
                if (Operand(3).Type == Value::Null && Operand(4).Type == Value::Null && Operand(5).Type == Value::Null &&
                    Operand(6).Type == Value::Null) {
                    ReturnValue =
                        SetErlValueNumber(CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number)); // curve index | X value | Y
                                                                                                                // value, 2nd independent | Z
                                                                                                                // Value, 3rd independent | 4th
                                                                                                                // independent | 5th independent
                } else if (Operand(4).Type == Value::Null && Operand(5).Type == Value::Null && Operand(6).Type == Value::Null) {
                    Real64 curveVal = 0.0;
                    switch (state.dataCurveManager->curves(std::floor(Operand(1).Number))->numDims) {
                    case 1:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number);
                        break;
                    case 2:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number);
                        break;
                    }
                    ReturnValue = SetErlValueNumber(curveVal);
                } else if (Operand(5).Type == Value::Null && Operand(6).Type == Value::Null) {
                    Real64 curveVal = 0.0;
                    switch (state.dataCurveManager->curves(std::floor(Operand(1).Number))->numDims) {
                    case 1:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number);
                        break;
                    case 2:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number);
                        break;
                    case 3:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number, Operand(4).Number);
                        break;
                    }
                    ReturnValue = SetErlValueNumber(curveVal);
                } else if (Operand(6).Type == Value::Null) {
                    Real64 curveVal = 0.0;
                    switch (state.dataCurveManager->curves(std::floor(Operand(1).Number))->numDims) {
                    case 1:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number);
                        break;
                    case 2:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number);
                        break;
                    case 3:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number, Operand(4).Number);
                        break;
                    case 4:
                        curveVal = CurveValue(
                            state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number, Operand(4).Number, Operand(5).Number);
                        break;
                    }
                    ReturnValue = SetErlValueNumber(curveVal);
                } else {
                    Real64 curveVal = 0.0;
                    switch (state.dataCurveManager->curves(std::floor(Operand(1).Number))->numDims) {
                    case 1:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number);
                        break;
                    case 2:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number);
                        break;
                    case 3:
                        curveVal = CurveValue(state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number, Operand(4).Number);
                        break;
                    case 4:
                        curveVal = CurveValue(
                            state, std::floor(Operand(1).Number), Operand(2).Number, Operand(3).Number, Operand(4).Number, Operand(5).Number);
                        break;
                    case 5:
                        curveVal = CurveValue(state,
                                              std::floor(Operand(1).Number),
                                              Operand(2).Number,
                                              Operand(3).Number,
                                              Operand(4).Number,
                                              Operand(5).Number,
                                              Operand(6).Number);
                        break;
                    }
                    ReturnValue = SetErlValueNumber(curveVal);
                }
                break;

            case ErlFunc::TodayIsRain:
            case ErlFunc::TodayIsSnow:
            case ErlFunc::TodayOutDryBulbTemp:
            case ErlFunc::TodayOutDewPointTemp:
            case ErlFunc::TodayOutBaroPress:
            case ErlFunc::TodayOutRelHum:
            case ErlFunc::TodayWindSpeed:
            case ErlFunc::TodayWindDir:
            case ErlFunc::TodaySkyTemp:
            case ErlFunc::TodayHorizIRSky:
            case ErlFunc::TodayBeamSolarRad:
            case ErlFunc::TodayDifSolarRad:
            case ErlFunc::TodayAlbedo:
            case ErlFunc::TodayLiquidPrecip: {
                int iHour = (Operand(1).Number + 1); // Operand 1 is hour from 0:23
                int iTimeStep = Operand(2).Number;
                if ((iHour > 0) && (iHour <= 24) && (iTimeStep > 0) && (iTimeStep <= state.dataGlobal->TimeStepsInHour)) {
                    auto const &today = state.dataWeather->wvarsHrTsToday(iTimeStep, iHour);
                    ReturnValue.initialized = true;
                    ReturnValue.Type = Value::Number;
                    switch (thisErlExpression.Operator) {
                    case ErlFunc::TodayIsRain: {
                        ReturnValue.Number = today.IsRain ? 1.0 : 0.0;
                    } break;
                    case ErlFunc::TodayIsSnow: {
                        ReturnValue.Number = today.IsSnow ? 1.0 : 0.0;
                    } break;
                    case ErlFunc::TodayOutDryBulbTemp: {
                        ReturnValue.Number = today.OutDryBulbTemp;
                    } break;
                    case ErlFunc::TodayOutDewPointTemp: {
                        ReturnValue.Number = today.OutDewPointTemp;
                    } break;
                    case ErlFunc::TodayOutBaroPress: {
                        ReturnValue.Number = today.OutBaroPress;
                    } break;
                    case ErlFunc::TodayOutRelHum: {
                        ReturnValue.Number = today.OutRelHum;
                    } break;
                    case ErlFunc::TodayWindSpeed: {
                        ReturnValue.Number = today.WindSpeed;
                    } break;
                    case ErlFunc::TodayWindDir: {
                        ReturnValue.Number = today.WindDir;
                    } break;
                    case ErlFunc::TodaySkyTemp: {
                        ReturnValue.Number = today.SkyTemp;
                    } break;
                    case ErlFunc::TodayHorizIRSky: {
                        ReturnValue.Number = today.HorizIRSky;
                    } break;
                    case ErlFunc::TodayBeamSolarRad: {
                        ReturnValue.Number = today.BeamSolarRad;
                    } break;
                    case ErlFunc::TodayDifSolarRad: {
                        ReturnValue.Number = today.DifSolarRad;
                    } break;
                    case ErlFunc::TodayAlbedo: {
                        ReturnValue.Number = today.Albedo;
                    } break;
                    case ErlFunc::TodayLiquidPrecip: {
                        ReturnValue.Number = today.LiquidPrecip;
                    } break;
                    default: {
                        assert(false);
                    } break;
                    }
                } else {
                    ReturnValue.Type = DataRuntimeLanguage::Value::Error;
                    ReturnValue.Error = format("{} function called with invalid arguments: Hour={:.1R}, Timestep={:.1R}",
                                               ErlFuncNamesUC[(int)thisErlExpression.Operator],
                                               Operand(1).Number,
                                               Operand(2).Number);
                }
            } break;

            case ErlFunc::TomorrowIsRain:
            case ErlFunc::TomorrowIsSnow:
            case ErlFunc::TomorrowOutDryBulbTemp:
            case ErlFunc::TomorrowOutDewPointTemp:
            case ErlFunc::TomorrowOutBaroPress:
            case ErlFunc::TomorrowOutRelHum:
            case ErlFunc::TomorrowWindSpeed:
            case ErlFunc::TomorrowWindDir:
            case ErlFunc::TomorrowSkyTemp:
            case ErlFunc::TomorrowHorizIRSky:
            case ErlFunc::TomorrowBeamSolarRad:
            case ErlFunc::TomorrowDifSolarRad:
            case ErlFunc::TomorrowAlbedo:
            case ErlFunc::TomorrowLiquidPrecip: {
                int iHour = (Operand(1).Number + 1); // Operand 1 is hour from 0:23
                int iTimeStep = Operand(2).Number;
                if ((iHour > 0) && (iHour <= Constant::iHoursInDay) && (iTimeStep > 0) && (iTimeStep <= state.dataGlobal->TimeStepsInHour)) {
                    auto const &tomorrow = state.dataWeather->wvarsHrTsTomorrow(iTimeStep, iHour);
                    ReturnValue.initialized = true;
                    ReturnValue.Type = Value::Number;
                    switch (thisErlExpression.Operator) {
                    case ErlFunc::TomorrowIsRain: {
                        ReturnValue.Number = tomorrow.IsRain ? 1.0 : 0.0;
                    } break;
                    case ErlFunc::TomorrowIsSnow: {
                        ReturnValue.Number = tomorrow.IsSnow ? 1.0 : 0.0;
                    } break;
                    case ErlFunc::TomorrowOutDryBulbTemp: {
                        ReturnValue.Number = tomorrow.OutDryBulbTemp;
                    } break;
                    case ErlFunc::TomorrowOutDewPointTemp: {
                        ReturnValue.Number = tomorrow.OutDewPointTemp;
                    } break;
                    case ErlFunc::TomorrowOutBaroPress: {
                        ReturnValue.Number = tomorrow.OutBaroPress;
                    } break;
                    case ErlFunc::TomorrowOutRelHum: {
                        ReturnValue.Number = tomorrow.OutRelHum;
                    } break;
                    case ErlFunc::TomorrowWindSpeed: {
                        ReturnValue.Number = tomorrow.WindSpeed;
                    } break;
                    case ErlFunc::TomorrowWindDir: {
                        ReturnValue.Number = tomorrow.WindDir;
                    } break;
                    case ErlFunc::TomorrowSkyTemp: {
                        ReturnValue.Number = tomorrow.SkyTemp;
                    } break;
                    case ErlFunc::TomorrowHorizIRSky: {
                        ReturnValue.Number = tomorrow.HorizIRSky;
                    } break;
                    case ErlFunc::TomorrowBeamSolarRad: {
                        ReturnValue.Number = tomorrow.BeamSolarRad;
                    } break;
                    case ErlFunc::TomorrowDifSolarRad: {
                        ReturnValue.Number = tomorrow.DifSolarRad;
                    } break;
                    case ErlFunc::TomorrowAlbedo: {
                        ReturnValue.Number = tomorrow.Albedo;
                    } break;
                    case ErlFunc::TomorrowLiquidPrecip: {
                        ReturnValue.Number = tomorrow.LiquidPrecip;
                    } break;
                    default: {
                        assert(false);
                    } break;
                    }
                } else {
                    ReturnValue.Type = DataRuntimeLanguage::Value::Error;
                    ReturnValue.Error = format("{} function called with invalid arguments: Hour={:.1R}, Timestep={:.1R}",
                                               ErlFuncNamesUC[(int)thisErlExpression.Operator],
                                               Operand(1).Number,
                                               Operand(2).Number);
                }
            } break;

            case ErlFunc::Invalid:
            case ErlFunc::Null:
            case ErlFunc::TsatFnPb:
            case ErlFunc::Num: {
                // throw Error, these cases are not supported -- they all make sense except TsatFnPb which was commented out above a long time ago
                ShowFatalError(state, "caught unexpected Expression(ExpressionNum)%Operator in EvaluateExpression");
            } break;
            } // switch (FunctionCode)
        }
        Operand.deallocate();
    }

    return ReturnValue;
}

void GetRuntimeLanguageUserInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith April 2009
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the runtime language objects from the input file.
    // GetInput is called from other modules that reference runtime language objects.
    // The runtime language objects are all loaded in one pass

    // METHODOLOGY EMPLOYED:
    // The runtime language objects are all loaded in one step, names registered, etc.  They are parsed in a second step
    // once all the object names are known.

    // Using/Aliasing
    using Curve::GetCurveIndex;

    // Locals
    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view RoutineName = "GetRuntimeLanguageUserInput: ";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int GlobalNum;
    int StackNum;
    int ErrorNum;
    int NumAlphas; // Number of elements in the alpha array
    int NumNums;   // Number of elements in the numeric array
    int IOStat;    // IO Status when calling get input subroutine
    bool ErrorsFound(false);
    int VariableNum(0); // temporary
    int RuntimeReportVarNum;
    bool Found;
    OutputProcessor::TimeStepType sovTimeStepType; // temporary
    OutputProcessor::StoreType sovStoreType;       // temporary
    std::string EndUseSubCatString;

    int TrendNum;
    int NumTrendSteps;
    int loop;
    int ErlVarLoop;
    int CurveIndexNum;
    int MaxNumAlphas(0);  // argument for call to GetObjectDefMaxArgs
    int MaxNumNumbers(0); // argument for call to GetObjectDefMaxArgs
    int TotalArgs(0);     // argument for call to GetObjectDefMaxArgs
    Array1D_string cAlphaFieldNames;
    Array1D_string cNumericFieldNames;
    Array1D_bool lNumericFieldBlanks;
    Array1D_bool lAlphaFieldBlanks;
    Array1D_string cAlphaArgs;
    Array1D<Real64> rNumericArgs;
    std::string cCurrentModuleObject;
    int ConstructNum;
    bool errFlag;
    std::string::size_type lbracket;
    std::string UnitsA;
    std::string UnitsB;
    Constant::Units curUnit(Constant::Units::None);
    std::string::size_type ptr;

    if (state.dataRuntimeLangProcessor->GetInput) { // GetInput check is redundant with the InitializeRuntimeLanguage routine
        state.dataRuntimeLangProcessor->GetInput = false;

        cCurrentModuleObject = "EnergyManagementSystem:Sensor";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = NumNums;
        MaxNumAlphas = NumAlphas;
        cCurrentModuleObject = "EnergyManagementSystem:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:ProgramCallingManager";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:Program";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:Subroutine";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:OutputVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:MeteredOutputVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:Variable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Variable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Variable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        //  cCurrentModuleObject = 'EnergyManagementSystem:Sensor'
        //  CALL state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject,TotalArgs,NumAlphas,NumNums)
        //  MaxNumNumbers=MAX(MaxNumNumbers,NumNums)
        //  MaxNumAlphas=MAX(MaxNumAlphas,NumAlphas)
        cCurrentModuleObject = "EnergyManagementSystem:GlobalVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:CurveOrTableIndexVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:ConstructionIndexVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);

        cAlphaFieldNames.allocate(MaxNumAlphas);
        cAlphaArgs.allocate(MaxNumAlphas);
        lAlphaFieldBlanks.dimension(MaxNumAlphas, false);
        cNumericFieldNames.allocate(MaxNumNumbers);
        rNumericArgs.dimension(MaxNumNumbers, 0.0);
        lNumericFieldBlanks.dimension(MaxNumNumbers, false);

        cCurrentModuleObject = "EnergyManagementSystem:GlobalVariable";

        if (state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables +
                state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables +
                state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables >
            0) {
            for (GlobalNum = 1;
                 GlobalNum <= state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables +
                                  state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables +
                                  state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables;
                 ++GlobalNum) {
                // If we process the ExternalInterface actuators, all we need to do is to change the
                // name of the module object, and add an offset for the variable number
                // This is done in the following IF/THEN section.
                if (GlobalNum <= state.dataRuntimeLang->NumUserGlobalVariables) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCurrentModuleObject,
                                                                             GlobalNum,
                                                                             cAlphaArgs,
                                                                             NumAlphas,
                                                                             rNumericArgs,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericFieldBlanks,
                                                                             lAlphaFieldBlanks,
                                                                             cAlphaFieldNames,
                                                                             cNumericFieldNames);
                } else if (GlobalNum > state.dataRuntimeLang->NumUserGlobalVariables &&
                           GlobalNum <= state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables) {
                    cCurrentModuleObject = "ExternalInterface:Variable";
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCurrentModuleObject,
                                                                             GlobalNum - state.dataRuntimeLang->NumUserGlobalVariables,
                                                                             cAlphaArgs,
                                                                             NumAlphas,
                                                                             rNumericArgs,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericFieldBlanks,
                                                                             lAlphaFieldBlanks,
                                                                             cAlphaFieldNames,
                                                                             cNumericFieldNames);
                } else if (GlobalNum > state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables &&
                           GlobalNum <= state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables +
                                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables) {
                    cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Variable";
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCurrentModuleObject,
                                                                             GlobalNum - state.dataRuntimeLang->NumUserGlobalVariables -
                                                                                 state.dataRuntimeLang->NumExternalInterfaceGlobalVariables,
                                                                             cAlphaArgs,
                                                                             NumAlphas,
                                                                             rNumericArgs,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericFieldBlanks,
                                                                             lAlphaFieldBlanks,
                                                                             cAlphaFieldNames,
                                                                             cNumericFieldNames);

                } else if (GlobalNum > state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables +
                                           state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables &&
                           GlobalNum <= state.dataRuntimeLang->NumUserGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceGlobalVariables +
                                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables +
                                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables) {
                    cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Variable";
                    state.dataInputProcessing->inputProcessor->getObjectItem(
                        state,
                        cCurrentModuleObject,
                        GlobalNum - state.dataRuntimeLang->NumUserGlobalVariables - state.dataRuntimeLang->NumExternalInterfaceGlobalVariables -
                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables,
                        cAlphaArgs,
                        NumAlphas,
                        rNumericArgs,
                        NumNums,
                        IOStat,
                        lNumericFieldBlanks,
                        lAlphaFieldBlanks,
                        cAlphaFieldNames,
                        cNumericFieldNames);
                }

                // loop over each alpha and register variable named as global Erl variable
                for (ErlVarLoop = 1; ErlVarLoop <= NumAlphas; ++ErlVarLoop) {
                    if ((cCurrentModuleObject.compare("ExternalInterface:FunctionalMockupUnitImport:To:Variable") == 0)) {
                        if (ErlVarLoop == 1) {
                            // Only validate first field of object ExternalInterface:FunctionalMockupUnitImport:To:Variable.
                            // This object is allowed to contain fields that do not need to be valid EMS fields (e.g. path to the FMU).
                            ValidateEMSVariableName(
                                state, cCurrentModuleObject, cAlphaArgs(ErlVarLoop), cAlphaFieldNames(ErlVarLoop), errFlag, ErrorsFound);
                        }
                    } else {
                        ValidateEMSVariableName(
                            state, cCurrentModuleObject, cAlphaArgs(ErlVarLoop), cAlphaFieldNames(ErlVarLoop), errFlag, ErrorsFound);
                    }
                    if (lAlphaFieldBlanks(ErlVarLoop)) {
                        ShowWarningError(state, format("{}{}", RoutineName, cCurrentModuleObject));
                        ShowContinueError(state, format("Blank {}", cAlphaFieldNames(1)));
                        ShowContinueError(state, "Blank entry will be skipped, and the simulation continues");
                    } else if (!errFlag) {
                        VariableNum = FindEMSVariable(state, cAlphaArgs(ErlVarLoop), 0);
                        // Still need to check for conflicts with program and function names too

                        if (VariableNum > 0) {
                            ShowSevereError(state, format("{}{}, invalid entry.", RoutineName, cCurrentModuleObject));
                            ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(ErlVarLoop), cAlphaArgs(ErlVarLoop)));
                            ShowContinueError(state, "Name conflicts with an existing global variable name");
                            ErrorsFound = true;
                        } else {
                            VariableNum = NewEMSVariable(state, cAlphaArgs(ErlVarLoop), 0);
                            if (GlobalNum > state.dataRuntimeLang->NumUserGlobalVariables) {
                                // Initialize variables for the ExternalInterface variables.
                                // This object requires an initial value.
                                ExternalInterfaceInitializeErlVariable(state, VariableNum, SetErlValueNumber(rNumericArgs(1)), false);
                            }
                        }
                    }
                }
            }
        }

        cCurrentModuleObject = "EnergyManagementSystem:CurveOrTableIndexVariable";
        state.dataRuntimeLang->NumEMSCurveIndices = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataRuntimeLang->NumEMSCurveIndices > 0) {
            state.dataRuntimeLangProcessor->CurveIndexVariableNums.dimension(state.dataRuntimeLang->NumEMSCurveIndices, 0);
            for (loop = 1; loop <= state.dataRuntimeLang->NumEMSCurveIndices; ++loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         loop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                // check if variable name is unique and well formed
                ValidateEMSVariableName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), errFlag, ErrorsFound);
                if (lAlphaFieldBlanks(1)) {
                    ShowSevereError(state, format("{}{}", RoutineName, cCurrentModuleObject));
                    ShowContinueError(state, format("Blank {}", cAlphaFieldNames(1)));
                    ShowContinueError(state, "Blank entry for Erl variable name is not allowed");
                    ErrorsFound = true;
                } else if (!errFlag) {
                    VariableNum = FindEMSVariable(state, cAlphaArgs(1), 0);
                    if (VariableNum > 0) {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}", cAlphaFieldNames(1)));
                        ShowContinueError(state, "Name conflicts with an existing variable name");
                        ErrorsFound = true;
                    } else {
                        // create new EMS variable
                        VariableNum = NewEMSVariable(state, cAlphaArgs(1), 0);
                        // store variable num
                        state.dataRuntimeLangProcessor->CurveIndexVariableNums(loop) = VariableNum;
                    }
                }

                CurveIndexNum = GetCurveIndex(state, cAlphaArgs(2)); // curve name
                if (CurveIndexNum == 0) {
                    if (lAlphaFieldBlanks(2)) {
                        ShowSevereError(state, format("{}{}=\"{} blank field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Blank {}", cAlphaFieldNames(2)));
                        ShowContinueError(state, "Blank entry for curve or table name is not allowed");
                    } else {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "Curve or table was not found.");
                    }
                    ErrorsFound = true;
                } else {
                    // fill Erl variable with curve index
                    state.dataRuntimeLang->ErlVariable(VariableNum).Value = SetErlValueNumber(double(CurveIndexNum));
                }
            }

        } // NumEMSCurveIndices > 0

        cCurrentModuleObject = "EnergyManagementSystem:ConstructionIndexVariable";
        state.dataRuntimeLang->NumEMSConstructionIndices = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataRuntimeLang->NumEMSConstructionIndices > 0) {
            state.dataRuntimeLangProcessor->ConstructionIndexVariableNums.dimension(state.dataRuntimeLang->NumEMSConstructionIndices, 0);
            for (loop = 1; loop <= state.dataRuntimeLang->NumEMSConstructionIndices; ++loop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         loop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                // check if variable name is unique and well formed
                ValidateEMSVariableName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), errFlag, ErrorsFound);
                if (lAlphaFieldBlanks(1)) {
                    ShowSevereError(state, format("{}{}", RoutineName, cCurrentModuleObject));
                    ShowContinueError(state, format("Blank {}", cAlphaFieldNames(1)));
                    ShowContinueError(state, "Blank entry for Erl variable name is not allowed");
                    ErrorsFound = true;
                } else if (!errFlag) {
                    VariableNum = FindEMSVariable(state, cAlphaArgs(1), 0);
                    if (VariableNum > 0) {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}", cAlphaFieldNames(1)));
                        ShowContinueError(state, "Name conflicts with an existing variable name");
                        ErrorsFound = true;
                    } else {
                        // create new EMS variable
                        VariableNum = NewEMSVariable(state, cAlphaArgs(1), 0);
                        // store variable num
                        state.dataRuntimeLangProcessor->ConstructionIndexVariableNums(loop) = VariableNum;
                    }
                } else {
                    continue;
                }

                ConstructNum = Util::FindItemInList(cAlphaArgs(2), state.dataConstruction->Construct);

                if (ConstructNum == 0) {
                    if (lAlphaFieldBlanks(2)) {
                        ShowSevereError(state, format("{}{}=\"{} blank field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Blank {}", cAlphaFieldNames(2)));
                        ShowContinueError(state, "Blank entry for construction name is not allowed");
                    } else {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "Construction was not found.");
                    }
                    ErrorsFound = true;
                } else {
                    // fill Erl variable with curve index
                    state.dataRuntimeLang->ErlVariable(VariableNum).Value = SetErlValueNumber(double(ConstructNum));
                }
            }

        } // NumEMSConstructionIndices > 0

        state.dataRuntimeLang->NumErlStacks = state.dataRuntimeLang->NumErlPrograms + state.dataRuntimeLang->NumErlSubroutines;
        state.dataRuntimeLang->ErlStack.allocate(state.dataRuntimeLang->NumErlStacks);
        state.dataRuntimeLangProcessor->ErlStackUniqueNames.reserve(static_cast<unsigned>(state.dataRuntimeLang->NumErlStacks));

        if (state.dataRuntimeLang->NumErlPrograms > 0) {
            cCurrentModuleObject = "EnergyManagementSystem:Program";
            for (StackNum = 1; StackNum <= state.dataRuntimeLang->NumErlPrograms; ++StackNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         StackNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataRuntimeLangProcessor->ErlStackUniqueNames,
                                                         cAlphaArgs(1),
                                                         cCurrentModuleObject,
                                                         cAlphaFieldNames(1),
                                                         ErrorsFound);

                ValidateEMSProgramName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), "Programs", errFlag, ErrorsFound);
                if (!errFlag) {
                    state.dataRuntimeLang->ErlStack(StackNum).Name = cAlphaArgs(1);
                }

                if (NumAlphas > 1) {
                    state.dataRuntimeLang->ErlStack(StackNum).Line.allocate(NumAlphas - 1);
                    state.dataRuntimeLang->ErlStack(StackNum).NumLines = NumAlphas - 1;
                    state.dataRuntimeLang->ErlStack(StackNum).Line({1, NumAlphas - 1}) = cAlphaArgs({2, NumAlphas}); // note array assignment
                }

            } // ProgramNum
        }

        if (state.dataRuntimeLang->NumErlSubroutines > 0) {
            cCurrentModuleObject = "EnergyManagementSystem:Subroutine";
            for (StackNum = state.dataRuntimeLang->NumErlPrograms + 1; StackNum <= state.dataRuntimeLang->NumErlStacks; ++StackNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         StackNum - state.dataRuntimeLang->NumErlPrograms,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataRuntimeLangProcessor->ErlStackUniqueNames,
                                                         cAlphaArgs(1),
                                                         cCurrentModuleObject,
                                                         cAlphaFieldNames(1),
                                                         ErrorsFound);

                ValidateEMSProgramName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), "Subroutines", errFlag, ErrorsFound);
                if (!errFlag) {
                    state.dataRuntimeLang->ErlStack(StackNum).Name = cAlphaArgs(1);
                }

                if (NumAlphas > 1) {
                    state.dataRuntimeLang->ErlStack(StackNum).Line.allocate(NumAlphas - 1);
                    state.dataRuntimeLang->ErlStack(StackNum).NumLines = NumAlphas - 1;
                    state.dataRuntimeLang->ErlStack(StackNum).Line({1, NumAlphas - 1}) = cAlphaArgs({2, NumAlphas}); // note array assignment
                }
            }
        }

        cCurrentModuleObject = "EnergyManagementSystem:TrendVariable";
        state.dataRuntimeLang->NumErlTrendVariables = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataRuntimeLang->NumErlTrendVariables > 0) {
            state.dataRuntimeLang->TrendVariable.allocate(state.dataRuntimeLang->NumErlTrendVariables);
            for (TrendNum = 1; TrendNum <= state.dataRuntimeLang->NumErlTrendVariables; ++TrendNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         TrendNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                Util::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

                ValidateEMSVariableName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), errFlag, ErrorsFound);
                if (!errFlag) {
                    state.dataRuntimeLang->TrendVariable(TrendNum).Name = cAlphaArgs(1);
                }

                VariableNum = FindEMSVariable(state, cAlphaArgs(2), 0);
                // Still need to check for conflicts with program and function names too
                if (VariableNum == 0) { // did not find it
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                    ShowContinueError(state, "Did not find a match with an EMS variable name");
                    ErrorsFound = true;
                } else { // found it.
                    state.dataRuntimeLang->TrendVariable(TrendNum).ErlVariablePointer = VariableNum;
                    // register the trend pointer in ErlVariable.
                    state.dataRuntimeLang->ErlVariable(VariableNum).Value.TrendVariable = true;
                    state.dataRuntimeLang->ErlVariable(VariableNum).Value.TrendVarPointer = TrendNum;
                    state.dataRuntimeLang->ErlVariable(VariableNum).Value.initialized = true; // Cannot figure out how to get around needing this,
                }

                NumTrendSteps = std::floor(rNumericArgs(1));
                if (NumTrendSteps > 0) {
                    state.dataRuntimeLang->TrendVariable(TrendNum).LogDepth = NumTrendSteps;
                    // setup data arrays using NumTrendSteps
                    state.dataRuntimeLang->TrendVariable(TrendNum).TrendValARR.allocate(NumTrendSteps);
                    state.dataRuntimeLang->TrendVariable(TrendNum).TrendValARR = 0.0; // array init
                    state.dataRuntimeLang->TrendVariable(TrendNum).tempTrendARR.allocate(NumTrendSteps);
                    state.dataRuntimeLang->TrendVariable(TrendNum).tempTrendARR = 0.0; // array init
                    state.dataRuntimeLang->TrendVariable(TrendNum).TimeARR.allocate(NumTrendSteps);
                    // construct time data array for use with other calculations later
                    // current time is zero, each value in trend log array is one zone timestep further back in time
                    // units are hours.  all terms negative, getting increasingly negative the further back in time
                    //  further back in time is higher index in array
                    for (loop = 1; loop <= NumTrendSteps; ++loop) {
                        if (loop == 1) {
                            state.dataRuntimeLang->TrendVariable(TrendNum).TimeARR(loop) = -state.dataGlobal->TimeStepZone;
                            continue;
                        } else {
                            state.dataRuntimeLang->TrendVariable(TrendNum).TimeARR(loop) =
                                state.dataRuntimeLang->TrendVariable(TrendNum).TimeARR(loop - 1) - state.dataGlobal->TimeStepZone; // fractional hours
                        }
                    }
                } else {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={:.2T}", cNumericFieldNames(1), rNumericArgs(1)));
                    ShowContinueError(state, "must be greater than zero");
                    ErrorsFound = true;
                }

            } // trendnum
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting EMS Runtime Language input. Preceding condition causes termination.");
        }

        // Parse the runtime language code
        for (StackNum = 1; StackNum <= state.dataRuntimeLang->NumErlStacks; ++StackNum) {
            ParseStack(state, StackNum);

            if (state.dataRuntimeLang->ErlStack(StackNum).NumErrors > 0) {
                ShowSevereError(
                    state,
                    format("Errors found parsing EMS Runtime Language program or subroutine = {}", state.dataRuntimeLang->ErlStack(StackNum).Name));
                for (ErrorNum = 1; ErrorNum <= state.dataRuntimeLang->ErlStack(StackNum).NumErrors; ++ErrorNum) {
                    ShowContinueError(state, state.dataRuntimeLang->ErlStack(StackNum).Error(ErrorNum));
                }
                ErrorsFound = true;
            }
        } // StackNum

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in parsing EMS Runtime Language input. Preceding condition causes termination.");
        }

        if ((state.dataRuntimeLang->NumEMSOutputVariables > 0) || (state.dataRuntimeLang->NumEMSMeteredOutputVariables > 0)) {
            state.dataRuntimeLangProcessor->RuntimeReportVar.allocate(state.dataRuntimeLang->NumEMSOutputVariables +
                                                                      state.dataRuntimeLang->NumEMSMeteredOutputVariables);
        }

        if (state.dataRuntimeLang->NumEMSOutputVariables > 0) {
            cCurrentModuleObject = "EnergyManagementSystem:OutputVariable";
            for (RuntimeReportVarNum = 1; RuntimeReportVarNum <= state.dataRuntimeLang->NumEMSOutputVariables; ++RuntimeReportVarNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         RuntimeReportVarNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataRuntimeLangProcessor->RuntimeReportVarUniqueNames,
                                                         cAlphaArgs(1),
                                                         cCurrentModuleObject,
                                                         cAlphaFieldNames(1),
                                                         ErrorsFound);

                lbracket = index(cAlphaArgs(1), '[');
                if (lbracket == std::string::npos) {
                    UnitsA = "";
                    //          if (lAlphaFieldBlanks(6)) then
                    //            CALL ShowWarningError(state, RoutineName//TRIM(cCurrentModuleObject)//'="'//TRIM(cAlphaArgs(1))//' no units
                    //            indicated.') CALL ShowContinueError(state, '...no units indicated for this variable. [] is assumed.')
                    //            cAlphaArgs(1)=TRIM(cAlphaArgs(1))//' []'
                    //          endif
                    UnitsB = cAlphaArgs(6);
                    lbracket = index(UnitsB, '[');
                    ptr = index(UnitsB, ']');
                    if (lbracket != std::string::npos) {
                        UnitsB[lbracket] = ' ';
                        if (ptr != std::string::npos) {
                            UnitsB[ptr] = ' ';
                        }
                        strip(UnitsB);
                    }
                } else { // units shown on Name field (7.2 and pre versions)
                    ptr = index(cAlphaArgs(1), ']');
                    if (ptr != std::string::npos) {
                        UnitsA = cAlphaArgs(1).substr(lbracket + 1, ptr - lbracket - 1);
                    } else {
                        UnitsA = cAlphaArgs(1).substr(lbracket + 1);
                    }
                    cAlphaArgs(1).erase(lbracket - 1);
                    UnitsB = cAlphaArgs(6);
                    lbracket = index(UnitsB, '[');
                    ptr = index(UnitsB, ']');
                    if (lbracket != std::string::npos) {
                        UnitsB[lbracket] = ' ';
                        if (ptr != std::string::npos) {
                            UnitsB[ptr] = ' ';
                        }
                        strip(UnitsB);
                    }
                    if (UnitsA != "" && UnitsB != "") {
                        if (UnitsA != UnitsB) {
                            ShowWarningError(state, format("{}{}=\"{} mismatched units.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                            ShowContinueError(state, format("...Units entered in {} (deprecated use)=\"{}\"", cAlphaFieldNames(1), UnitsA));
                            ShowContinueError(state, format("...{}=\"{}\" (will be used)", cAlphaFieldNames(6), UnitsB));
                        }
                    } else if (UnitsB == "" && UnitsA != "") {
                        UnitsB = UnitsA;
                        ShowWarningError(state,
                                         format("{}{}=\"{}\" using deprecated units designation.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("...Units entered in {} (deprecated use)=\"{}\"", cAlphaFieldNames(1), UnitsA));
                    }
                }
                curUnit = static_cast<Constant::Units>(getEnumValue(Constant::unitNamesUC, Util::makeUPPER(UnitsB)));

                state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Name = cAlphaArgs(1);

                if (!lAlphaFieldBlanks(5)) {
                    // Lookup the Runtime Language Context, i.e., PROGRAM, FUNCTION, or global
                    Found = false;
                    for (StackNum = 1; StackNum <= state.dataRuntimeLang->NumErlStacks; ++StackNum) {
                        if (state.dataRuntimeLang->ErlStack(StackNum).Name == cAlphaArgs(5)) {
                            Found = true;
                            break;
                        }
                    }
                    if (!Found) {
                        StackNum = 0;
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(5), cAlphaArgs(5)));
                        ShowContinueError(state, "EMS program or subroutine not found.");
                        ErrorsFound = true;
                    }
                } else {
                    StackNum = 0;
                }

                VariableNum = FindEMSVariable(state, cAlphaArgs(2), StackNum);

                if (VariableNum == 0) {
                    if (lAlphaFieldBlanks(5)) {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "EMS variable not found among global variables.");
                    } else if (StackNum != 0) {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, format("EMS variable not found among local variables in {}", cAlphaArgs(5)));
                    }
                    ErrorsFound = true;
                    //        ELSEIF (INDEX('0123456789',cAlphaArgs(2)(1:1)) > 0) THEN
                    //            CALL ShowSevereError(state, 'Invalid '//TRIM(cAlphaFieldNames(2))//'='//TRIM(cAlphaArgs(2)))
                    //            CALL ShowContinueError(state, 'Entered in '//TRIM(cCurrentModuleObject)//'='//TRIM(cAlphaArgs(1)))
                    //            CALL ShowContinueError(state, 'Names used as Erl output variables cannot start with numeric characters.')
                    //            ErrorsFound = .TRUE.
                } else {
                    state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).VariableNum = VariableNum;
                }

                if (cAlphaArgs(3) == "AVERAGED") {
                    sovStoreType = OutputProcessor::StoreType::Average;
                } else if (cAlphaArgs(3) == "SUMMED") {
                    sovStoreType = OutputProcessor::StoreType::Sum;
                } else {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(3), cAlphaArgs(3)));
                    ShowContinueError(state, "...valid values are Averaged or Summed.");
                    ErrorsFound = true;
                }

                if (cAlphaArgs(4) == "ZONETIMESTEP") {
                    sovTimeStepType = OutputProcessor::TimeStepType::Zone;
                } else if (cAlphaArgs(4) == "SYSTEMTIMESTEP") {
                    sovTimeStepType = OutputProcessor::TimeStepType::System;
                } else {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(4), cAlphaArgs(4)));
                    ShowContinueError(state, "...valid values are ZoneTimestep or SystemTimestep.");
                    ErrorsFound = true;
                }

                if (curUnit != Constant::Units::Invalid) {
                    SetupOutputVariable(state,
                                        cAlphaArgs(1),
                                        curUnit,
                                        state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Value,
                                        sovTimeStepType,
                                        sovStoreType,
                                        "EMS");
                } else {
                    SetupOutputVariable(state,
                                        cAlphaArgs(1),
                                        Constant::Units::customEMS,
                                        state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Value,
                                        sovTimeStepType,
                                        sovStoreType,
                                        "EMS",
                                        Constant::eResource::Invalid,
                                        OutputProcessor::Group::Invalid,
                                        OutputProcessor::EndUseCat::Invalid,
                                        "",   // EndUseSubCat
                                        "",   // ZoneName
                                        1,    // ZoneMult
                                        1,    // ZoneListMult
                                        "",   // SpaceType
                                        -999, // indexGroupKey
                                        UnitsB);
                }
                // Last field is index key, no indexing here so mimic weather output data

            } // RuntimeReportVarNum
        } // NumEMSOutputVariables > 0

        if (state.dataRuntimeLang->NumEMSMeteredOutputVariables > 0) {
            cCurrentModuleObject = "EnergyManagementSystem:MeteredOutputVariable";
            for (loop = 1; loop <= state.dataRuntimeLang->NumEMSMeteredOutputVariables; ++loop) {
                RuntimeReportVarNum = state.dataRuntimeLang->NumEMSOutputVariables + loop;
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         loop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                GlobalNames::VerifyUniqueInterObjectName(state,
                                                         state.dataRuntimeLangProcessor->RuntimeReportVarUniqueNames,
                                                         cAlphaArgs(1),
                                                         cCurrentModuleObject,
                                                         cAlphaFieldNames(1),
                                                         ErrorsFound);

                lbracket = index(cAlphaArgs(1), '[');
                if (lbracket == std::string::npos) {
                    UnitsA = "";
                    //          if (lAlphaFieldBlanks(9)) then
                    //            CALL ShowWarningError(state, RoutineName//TRIM(cCurrentModuleObject)//'="'//TRIM(cAlphaArgs(1))//' no units
                    //            indicated.') CALL ShowContinueError(state, '...no units indicated for this variable. [] is assumed.')
                    //            cAlphaArgs(1)=TRIM(cAlphaArgs(1))//' []'
                    //          endif
                    UnitsB = cAlphaArgs(9);
                    lbracket = index(UnitsB, '[');
                    ptr = index(UnitsB, ']');
                    if (lbracket != std::string::npos) {
                        UnitsB[lbracket] = ' ';
                        if (ptr != std::string::npos) {
                            UnitsB[ptr] = ' ';
                        }
                        strip(UnitsB);
                    }
                } else { // units shown on Name field (7.2 and pre versions)
                    ptr = index(cAlphaArgs(1), ']');
                    if (ptr != std::string::npos) {
                        UnitsA = cAlphaArgs(1).substr(lbracket + 1, ptr - lbracket - 1);
                    } else {
                        UnitsA = cAlphaArgs(1).substr(lbracket + 1);
                    }
                    cAlphaArgs(1).erase(lbracket - 1);
                    UnitsB = cAlphaArgs(9);
                    lbracket = index(UnitsB, '[');
                    ptr = index(UnitsB, ']');
                    if (lbracket != std::string::npos) {
                        UnitsB[lbracket] = ' ';
                        if (ptr != std::string::npos) {
                            UnitsB[ptr] = ' ';
                        }
                        strip(UnitsB);
                    }
                    if (UnitsA != "" && UnitsB != "") {
                        if (UnitsA != UnitsB) {
                            ShowWarningError(state, format("{}{}=\"{} mismatched units.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                            ShowContinueError(state, format("...Units entered in {} (deprecated use)=\"{}\"", cAlphaFieldNames(1), UnitsA));
                            ShowContinueError(state, format("...{}=\"{}\" (will be used)", cAlphaFieldNames(9), UnitsB));
                        }
                    } else if (UnitsB == "" && UnitsA != "") {
                        UnitsB = UnitsA;
                        ShowWarningError(state,
                                         format("{}{}=\"{}\" using deprecated units designation.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("...Units entered in {} (deprecated use)=\"{}\"", cAlphaFieldNames(1), UnitsA));
                    }
                }
                curUnit = static_cast<Constant::Units>(getEnumValue(Constant::unitNamesUC, Util::makeUPPER(UnitsB)));

                state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Name = cAlphaArgs(1);

                if (!lAlphaFieldBlanks(4)) {
                    // Lookup the Runtime Language Context, i.e., PROGRAM, FUNCTION, or global
                    Found = false;
                    for (StackNum = 1; StackNum <= state.dataRuntimeLang->NumErlStacks; ++StackNum) {
                        if (state.dataRuntimeLang->ErlStack(StackNum).Name == cAlphaArgs(4)) {
                            Found = true;
                            break;
                        }
                    }
                    if (!Found) {
                        StackNum = 0;
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(4), cAlphaArgs(4)));
                        ShowContinueError(state, "EMS program or subroutine not found.");
                        ErrorsFound = true;
                    }
                } else {
                    StackNum = 0;
                }

                VariableNum = FindEMSVariable(state, cAlphaArgs(2), StackNum);
                if (VariableNum == 0) {
                    if (lAlphaFieldBlanks(4)) {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "EMS variable not found among global variables.");
                    } else if (StackNum != 0) {
                        ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, format("EMS variable not found among local variables in {}", cAlphaArgs(5)));
                    }
                    ErrorsFound = true;
                    //        ELSEIF (INDEX('0123456789',cAlphaArgs(2)(1:1)) > 0) THEN
                    //            CALL ShowSevereError(state, 'Invalid '//TRIM(cAlphaFieldNames(2))//'='//TRIM(cAlphaArgs(2)))
                    //            CALL ShowContinueError(state, 'Entered in '//TRIM(cCurrentModuleObject)//'='//TRIM(cAlphaArgs(1)))
                    //            CALL ShowContinueError(state, 'Names used as Erl output variables cannot start with numeric characters.')
                    //            ErrorsFound = .TRUE.
                } else {
                    state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).VariableNum = VariableNum;
                }

                sovStoreType = OutputProcessor::StoreType::Sum; // all metered vars are sum type

                if (cAlphaArgs(3) == "ZONETIMESTEP") {
                    sovTimeStepType = OutputProcessor::TimeStepType::Zone;
                } else if (cAlphaArgs(3) == "SYSTEMTIMESTEP") {
                    sovTimeStepType = OutputProcessor::TimeStepType::System;
                } else {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(4), cAlphaArgs(4)));
                    ShowContinueError(state, "...valid values are ZoneTimestep or SystemTimestep.");
                    ErrorsFound = true;
                }

                // Resource Type
                Constant::eResource resource =
                    static_cast<Constant::eResource>(getEnumValue(Constant::eResourceNamesUC, Util::makeUPPER(cAlphaArgs(5))));

                if (resource == Constant::eResource::Invalid) {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(5), cAlphaArgs(5)));
                    ErrorsFound = true;
                }

                // Group Type
                OutputProcessor::Group sovGroup;

                if (cAlphaArgs(6) == "BUILDING") {
                    sovGroup = OutputProcessor::Group::Building;
                } else if (cAlphaArgs(6) == "HVAC") {
                    sovGroup = OutputProcessor::Group::HVAC;
                } else if (cAlphaArgs(6) == "PLANT") {
                    sovGroup = OutputProcessor::Group::Plant;
                } else if (cAlphaArgs(6) == "SYSTEM") {
                    sovGroup = OutputProcessor::Group::HVAC;
                } else {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(6), cAlphaArgs(6)));
                    ErrorsFound = true;
                }

                // End Use Type
                OutputProcessor::EndUseCat sovEndUseCat;

                if (cAlphaArgs(7) == "HEATING") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Heating;
                } else if (cAlphaArgs(7) == "COOLING") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Cooling;
                } else if (cAlphaArgs(7) == "INTERIORLIGHTS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::InteriorLights;
                } else if (cAlphaArgs(7) == "EXTERIORLIGHTS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::ExteriorLights;
                } else if (cAlphaArgs(7) == "INTERIOREQUIPMENT") {
                    sovEndUseCat = OutputProcessor::EndUseCat::InteriorEquipment;
                } else if (cAlphaArgs(7) == "EXTERIOREQUIPMENT") {
                    sovEndUseCat = OutputProcessor::EndUseCat::ExteriorEquipment;
                } else if (cAlphaArgs(7) == "FANS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Fans;
                } else if (cAlphaArgs(7) == "PUMPS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Pumps;
                } else if (cAlphaArgs(7) == "HEATREJECTION") {
                    sovEndUseCat = OutputProcessor::EndUseCat::HeatRejection;
                } else if (cAlphaArgs(7) == "HUMIDIFIER") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Humidification;
                } else if (cAlphaArgs(7) == "HEATRECOVERY") {
                    sovEndUseCat = OutputProcessor::EndUseCat::HeatRecovery;
                } else if (cAlphaArgs(7) == "WATERSYSTEMS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::WaterSystem;
                } else if (cAlphaArgs(7) == "REFRIGERATION") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Refrigeration;
                } else if (cAlphaArgs(7) == "ONSITEGENERATION") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Cogeneration;
                } else if (cAlphaArgs(7) == "HEATINGCOILS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::HeatingCoils;
                } else if (cAlphaArgs(7) == "COOLINGCOILS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::CoolingCoils;
                } else if (cAlphaArgs(7) == "CHILLERS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Chillers;
                } else if (cAlphaArgs(7) == "BOILERS") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Boilers;
                } else if (cAlphaArgs(7) == "BASEBOARD") {
                    sovEndUseCat = OutputProcessor::EndUseCat::Baseboard;
                } else if (cAlphaArgs(7) == "HEATRECOVERYFORCOOLING") {
                    sovEndUseCat = OutputProcessor::EndUseCat::HeatRecoveryForCooling;
                } else if (cAlphaArgs(7) == "HEATRECOVERYFORHEATING") {
                    sovEndUseCat = OutputProcessor::EndUseCat::HeatRecoveryForHeating;
                } else {
                    ShowSevereError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("Invalid {}={}", cAlphaFieldNames(7), cAlphaArgs(7)));
                    ErrorsFound = true;
                }

                // Additional End Use Types Only Used for EnergyTransfer
                if ((resource != Constant::eResource::EnergyTransfer) &&
                    (sovEndUseCat == OutputProcessor::EndUseCat::HeatingCoils || sovEndUseCat == OutputProcessor::EndUseCat::CoolingCoils ||
                     sovEndUseCat == OutputProcessor::EndUseCat::Chillers || sovEndUseCat == OutputProcessor::EndUseCat::Boilers ||
                     sovEndUseCat == OutputProcessor::EndUseCat::Baseboard || sovEndUseCat == OutputProcessor::EndUseCat::HeatRecoveryForCooling ||
                     sovEndUseCat == OutputProcessor::EndUseCat::HeatRecoveryForHeating)) {
                    ShowWarningError(state, format("{}{}=\"{} invalid field.", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("Invalid {}={} for {}={}", cAlphaFieldNames(5), cAlphaArgs(5), cAlphaFieldNames(7), cAlphaArgs(7)));
                    ShowContinueError(state, format("Field {} is reset from {} to EnergyTransfer", cAlphaFieldNames(5), cAlphaArgs(5)));
                    resource = Constant::eResource::EnergyTransfer;
                }

                if (!lAlphaFieldBlanks(8)) {
                    EndUseSubCatString = cAlphaArgs(8);

                    SetupOutputVariable(state,
                                        cAlphaArgs(1),
                                        curUnit,
                                        state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Value,
                                        sovTimeStepType,
                                        sovStoreType,
                                        "EMS",
                                        resource,
                                        sovGroup,
                                        sovEndUseCat,
                                        EndUseSubCatString);
                } else { // no subcat
                    SetupOutputVariable(state,
                                        cAlphaArgs(1),
                                        curUnit,
                                        state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Value,
                                        sovTimeStepType,
                                        sovStoreType,
                                        "EMS",
                                        resource,
                                        sovGroup,
                                        sovEndUseCat);
                }
            }
        } // NumEMSMeteredOutputVariables > 0

        cAlphaFieldNames.deallocate();
        cAlphaArgs.deallocate();
        lAlphaFieldBlanks.deallocate();
        cNumericFieldNames.deallocate();
        rNumericArgs.deallocate();
        lNumericFieldBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting EMS Runtime Language input. Preceding condition causes termination.");
        }

    } // GetInput
}

void ReportRuntimeLanguage(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:

    // METHODOLOGY EMPLOYED:

    // USE STATEMENTS:

    // Locals
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int RuntimeReportVarNum;
    int VariableNum;

    for (RuntimeReportVarNum = 1;
         RuntimeReportVarNum <= state.dataRuntimeLang->NumEMSOutputVariables + state.dataRuntimeLang->NumEMSMeteredOutputVariables;
         ++RuntimeReportVarNum) {
        VariableNum = state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).VariableNum;
        if (state.dataRuntimeLang->ErlVariable(VariableNum).Value.Type == Value::Number) {
            state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Value =
                state.dataRuntimeLang->ErlVariable(VariableNum).Value.Number;
        } else {
            state.dataRuntimeLangProcessor->RuntimeReportVar(RuntimeReportVarNum).Value = 0.0;
        }
    }
}

ErlValueType SetErlValueNumber(Real64 const Number, ObjexxFCL::Optional<ErlValueType const> OrigValue)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         P. Ellis
    //       DATE WRITTEN   unknown

    ErlValueType newValue;

    if (present(OrigValue)) { // preserve other parts of structure and only updated Value%Number
        newValue = OrigValue;
        newValue.Number = Number;
    } else {
        newValue.Type = Value::Number;
        newValue.Number = Number;
    }

    newValue.initialized = true;
    return newValue;
}

ErlValueType StringValue(std::string const &String)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         P. Ellis
    //       DATE WRITTEN   unkown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // convert string to Erl Value structure

    // METHODOLOGY EMPLOYED:
    // <description>

    // REFERENCES:
    // na

    // USE STATEMENTS:
    // na

    // Return value
    ErlValueType Value;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:
    // na

    // FUNCTION PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS:
    // na

    // DERIVED TYPE DEFINITIONS:
    // na

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    // na

    Value.Type = Value::String;
    Value.String = String;

    return Value;
}

std::string ValueToString(ErlValueType const &Value)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         P. Ellis
    //       DATE WRITTEN   Unknown
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // <description>

    // METHODOLOGY EMPLOYED:
    // <description>

    // REFERENCES:
    // na

    // Using/Aliasing

    // Return value
    std::string String;

    // Locals
    // FUNCTION ARGUMENT DEFINITIONS:

    String = "";

    switch (Value.Type) {
    case Value::Number:
        if (Value.Number == 0.0) {
            String = "0.0";
        } else {
            String = format("{:.6T}", Value.Number); //(String)
        }
        break;

    case Value::String:
        String = Value.String;
        break;

    case Value::Array:
        // TBD
        break;

    case Value::Error:
        String = " *** Error: " + Value.Error + " *** ";
        break;

    default:
        // Nothing to do
        break;
    }

    return String;
}

int FindEMSVariable(EnergyPlusData &state,
                    std::string const &VariableName, // variable name in Erl
                    int const StackNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:

    // Return value
    int VariableNum;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    bool Found;
    int TrendVarNum;

    Found = false;
    std::string const UppercaseName = Util::makeUPPER(VariableName);

    // check in ErlVariables
    for (VariableNum = 1; VariableNum <= state.dataRuntimeLang->NumErlVariables; ++VariableNum) {
        if (state.dataRuntimeLang->ErlVariable(VariableNum).Name == UppercaseName) {
            if ((state.dataRuntimeLang->ErlVariable(VariableNum).StackNum == StackNum) ||
                (state.dataRuntimeLang->ErlVariable(VariableNum).StackNum == 0)) {
                Found = true;
                break;
            }
        }
    }

    // check in Trend variables
    for (TrendVarNum = 1; TrendVarNum <= state.dataRuntimeLang->NumErlTrendVariables; ++TrendVarNum) {
        if (state.dataRuntimeLang->TrendVariable(TrendVarNum).Name == UppercaseName) {
            VariableNum = state.dataRuntimeLang->TrendVariable(TrendVarNum).ErlVariablePointer;
            if ((state.dataRuntimeLang->ErlVariable(VariableNum).StackNum == StackNum) ||
                (state.dataRuntimeLang->ErlVariable(VariableNum).StackNum == 0)) {
                Found = true;
                break;
            }
        }
    }

    if (!Found) {
        VariableNum = 0;
    }

    return VariableNum;
}

int NewEMSVariable(EnergyPlusData &state, std::string const &VariableName, int const StackNum, ObjexxFCL::Optional<ErlValueType const> Value)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Creates new variable if it doesn't exist.  If exists, returns existing variable number.

    int VariableNum = FindEMSVariable(state, VariableName, StackNum);

    if (VariableNum == 0) { // Variable does not exist anywhere yet
        if (state.dataRuntimeLang->NumErlVariables == 0) {
            state.dataRuntimeLang->ErlVariable.allocate(1);
            state.dataRuntimeLang->NumErlVariables = 1;
        } else { // Extend the variable array
            state.dataRuntimeLang->ErlVariable.redimension(++state.dataRuntimeLang->NumErlVariables);
        }

        // Add the new variable
        VariableNum = state.dataRuntimeLang->NumErlVariables;
        auto &thisErlVar = state.dataRuntimeLang->ErlVariable(VariableNum);
        thisErlVar.Name = Util::makeUPPER(VariableName);
        thisErlVar.StackNum = StackNum;
        thisErlVar.Value.Type = Value::Number; // ErlVariable values are numbers
    }

    if (present(Value)) {
        state.dataRuntimeLang->ErlVariable(VariableNum).Value = Value;
    }

    return VariableNum;
}

void ExternalInterfaceSetErlVariable(EnergyPlusData &state,
                                     int const varNum,  // The variable index to be written during run time
                                     Real64 const value // The real time value of the vairable to be set
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rui Zhang
    //       DATE WRITTEN   February 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This is the ExternalInterface runtime write ErlVariable function

    // METHODOLOGY EMPLOYED:
    // USE STATEMENTS:

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    state.dataRuntimeLang->ErlVariable(varNum).Value = SetErlValueNumber(value);
}

void ExternalInterfaceInitializeErlVariable(EnergyPlusData &state,
                                            int const varNum,                 // The variable index to be written during run time
                                            ErlValueType const &initialValue, // The initial value
                                            bool const setToNull              // Flag, if true, value will be initialized to Null
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael Wetter
    //       DATE WRITTEN   February 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets flags for ExternalInterface variables

    // METHODOLOGY EMPLOYED:
    // USE STATEMENTS:

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    // Set initial value
    if (setToNull) {
        state.dataRuntimeLang->ErlVariable(varNum).Value.Type = Value::Null;
    } else {
        state.dataRuntimeLang->ErlVariable(varNum).Value = initialValue;
    }

    // Set variables to read-only as we don't want that other programs write to them
    state.dataRuntimeLang->ErlVariable(varNum).ReadOnly = true;
    // Set flag that it is used by the ExternalInterface. This is needed to make sure that the ExternalInterface
    // interface writes only to ExternalInterface variables, and not to other ErlVariable
    state.dataRuntimeLang->ErlVariable(varNum).SetByExternalInterface = true;
}

bool isExternalInterfaceErlVariable(EnergyPlusData &state, int const varNum) // The variable index to be written during run time
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael Wetter
    //       DATE WRITTEN   February 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This function checks if an Erl name obtained from the xml file
    // is indeed specified as a ExternalInterface variable in the idf file

    // METHODOLOGY EMPLOYED:
    // USE STATEMENTS:

    // Return value
    bool isExternalInterfaceVar; // Set to true if the variable is a ExternalInterface variable

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    isExternalInterfaceVar = state.dataRuntimeLang->ErlVariable(varNum).SetByExternalInterface;

    return isExternalInterfaceVar;
}

} // namespace EnergyPlus::RuntimeLanguageProcessor
