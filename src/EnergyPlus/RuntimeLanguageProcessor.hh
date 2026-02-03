// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

#ifndef RuntimeLanguageProcessor_hh_INCLUDED
#define RuntimeLanguageProcessor_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace RuntimeLanguageProcessor {

    // Using/Aliasing
    using DataRuntimeLanguage::ErlFunc;
    using DataRuntimeLanguage::ErlValueType;

    int constexpr MaxErrors(20);

    enum class Token
    {
        Invalid = -1,
        Number = 1,            // matches the ValueNumber
        Variable = 4,          // matches the ValueVariable
        Expression = 5,        // matches the ValueExpression
        Operator = 7,          // includes basic operators and built-in functions.
        Parenthesis = 9,       // parenthesis token
        ParenthesisLeft = 10,  // indicates left side parenthesis found in parsing
        ParenthesisRight = 11, // indicates right side parenthesis found in parsing
        Num
    };

    struct TokenType
    {
        // Members
        // structure for token information for parsing Erl code
        Token Type;         // token type, eg. TokenNumber
        Real64 Number;      // May want to store all literals as a variable?
        std::string String; // Serves double duty, also saves string version of token for easy debugging
        ErlFunc Operator;   // identifies operator or function 1..64
        int Variable;       // points to a variable in ErlVariable structure
        Token Parenthesis;  // identifies if token is left or right parenthesis
        int Expression;     // points to an expression in ErlExpression structure
        std::string Error;  // holds token processing error message content

        // Default Constructor
        TokenType() : Type(Token::Invalid), Number(0.0), Operator(ErlFunc::Invalid), Variable(0), Parenthesis(Token::Invalid), Expression(0)
        {
        }
    };

    struct RuntimeReportVarType
    {
        // Members
        std::string Name; // name of custom Erl report variable
        int VariableNum;  // pointer to Erl variable associated with custom report variable
        Real64 Value;     // Value registered with output processor for report variable

        // Default Constructor
        RuntimeReportVarType() : VariableNum(0), Value(0.0)
        {
        }
    };

    void InitializeRuntimeLanguage(EnergyPlusData &state);

    void BeginEnvrnInitializeRuntimeLanguage(EnergyPlusData &state);

    void ParseStack(EnergyPlusData &state, int StackNum);

    int AddInstruction(EnergyPlusData &state,
                       int StackNum,
                       int LineNum,
                       DataRuntimeLanguage::ErlKeywordParam Keyword,
                       ObjexxFCL::Optional_int_const Argument1 = _, // Erl variable index
                       ObjexxFCL::Optional_int_const Argument2 = _);

    void AddError(EnergyPlusData &state,
                  int StackNum,            // index pointer to location in ErlStack structure
                  int LineNum,             // Erl program line number
                  std::string const &Error // error message to be added to ErlStack
    );

    ErlValueType EvaluateStack(EnergyPlusData &state, int StackNum);

    void WriteTrace(EnergyPlusData &state, int StackNum, int InstructionNum, ErlValueType const &ReturnValue, bool seriousErrorFound);

    void ParseExpression(EnergyPlusData &state,
                         std::string const &InString, // String of expression text written in the Runtime Language
                         int StackNum,                // Parent StackNum??
                         int &ExpressionNum,          // index of expression in structure
                         std::string const &Line      // Actual line from string
    );

    int ProcessTokens(EnergyPlusData &state, const Array1D<TokenType> &TokenIN, int NumTokensIN, int StackNum, std::string const &ParsingString);

    int NewExpression(EnergyPlusData &state);

    ErlValueType EvaluateExpression(EnergyPlusData &state, int ExpressionNum, bool &seriousErrorFound);

    void GetRuntimeLanguageUserInput(EnergyPlusData &state);

    void ReportRuntimeLanguage(EnergyPlusData &state);

    ErlValueType SetErlValueNumber(Real64 Number, ObjexxFCL::Optional<ErlValueType const> OrigValue = _);

    ErlValueType StringValue(std::string const &String);

    std::string ValueToString(ErlValueType const &Value);

    int FindEMSVariable(EnergyPlusData &state,
                        std::string const &VariableName, // variable name in Erl
                        int StackNum);

    int NewEMSVariable(EnergyPlusData &state, std::string const &VariableName, int StackNum, ObjexxFCL::Optional<ErlValueType const> Value = _);

    void ExternalInterfaceSetErlVariable(EnergyPlusData &state,
                                         int varNum,  // The variable index to be written during run time
                                         Real64 value // The real time value of the variable to be set
    );

    void ExternalInterfaceInitializeErlVariable(EnergyPlusData &state,
                                                int varNum,                       // The variable index to be written during run time
                                                ErlValueType const &initialValue, // The initial value
                                                bool setToNull                    // Flag, if true, value will be initialized to Null
    );

    bool isExternalInterfaceErlVariable(EnergyPlusData &state, int varNum); // The variable index to be written during run time

} // namespace RuntimeLanguageProcessor

struct RuntimeLanguageProcessorData : BaseGlobalStruct
{
    bool AlreadyDidOnce = false;
    bool GetInput = true;
    bool InitializeOnce = true;
    bool MyEnvrnFlag = true;
    int NullVariableNum = 0;
    int FalseVariableNum = 0;
    int TrueVariableNum = 0;
    int OffVariableNum = 0;
    int OnVariableNum = 0;
    int PiVariableNum = 0;
    Array1D_int CurveIndexVariableNums;
    Array1D_int ConstructionIndexVariableNums;
    int YearVariableNum = 0;
    int CalendarYearVariableNum = 0;
    int MonthVariableNum = 0;
    int DayOfMonthVariableNum = 0;
    int DayOfWeekVariableNum = 0;
    int DayOfYearVariableNum = 0;
    int HourVariableNum = 0;
    int TimeStepsPerHourVariableNum = 0;
    int TimeStepNumVariableNum = 0;
    int MinuteVariableNum = 0;
    int HolidayVariableNum = 0;
    int DSTVariableNum = 0;
    int CurrentTimeVariableNum = 0;
    int SunIsUpVariableNum = 0;
    int IsRainingVariableNum = 0;
    int SystemTimeStepVariableNum = 0;
    int ZoneTimeStepVariableNum = 0;
    int CurrentEnvironmentPeriodNum = 0;
    int ActualDateAndTimeNum = 0;
    int ActualTimeNum = 0;
    int WarmUpFlagNum = 0;
    Array1D<RuntimeLanguageProcessor::RuntimeReportVarType> RuntimeReportVar;
    std::unordered_map<std::string, std::string> ErlStackUniqueNames;
    std::unordered_map<std::string, std::string> RuntimeReportVarUniqueNames;
    bool WriteTraceMyOneTimeFlag = false;
    Array1D<RuntimeLanguageProcessor::TokenType> Token;
    Array1D<RuntimeLanguageProcessor::TokenType> PEToken;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->AlreadyDidOnce = false;
        this->GetInput = true;
        this->InitializeOnce = true;
        this->MyEnvrnFlag = true;
        this->NullVariableNum = 0;
        this->FalseVariableNum = 0;
        this->TrueVariableNum = 0;
        this->OffVariableNum = 0;
        this->OnVariableNum = 0;
        this->PiVariableNum = 0;
        this->CurveIndexVariableNums.clear();
        this->ConstructionIndexVariableNums.clear();
        this->YearVariableNum = 0;
        this->CalendarYearVariableNum = 0;
        this->MonthVariableNum = 0;
        this->DayOfMonthVariableNum = 0;
        this->DayOfWeekVariableNum = 0;
        this->DayOfYearVariableNum = 0;
        this->HourVariableNum = 0;
        this->TimeStepsPerHourVariableNum = 0;
        this->TimeStepNumVariableNum = 0;
        this->MinuteVariableNum = 0;
        this->HolidayVariableNum = 0;
        this->DSTVariableNum = 0;
        this->CurrentTimeVariableNum = 0;
        this->SunIsUpVariableNum = 0;
        this->IsRainingVariableNum = 0;
        this->SystemTimeStepVariableNum = 0;
        this->ZoneTimeStepVariableNum = 0;
        this->CurrentEnvironmentPeriodNum = 0;
        this->ActualDateAndTimeNum = 0;
        this->ActualTimeNum = 0;
        this->WarmUpFlagNum = 0;
        this->RuntimeReportVar.clear();
        this->ErlStackUniqueNames.clear();
        this->RuntimeReportVarUniqueNames.clear();
        this->WriteTraceMyOneTimeFlag = false;
        this->PEToken.clear();
        this->Token.clear();
    }
};

} // namespace EnergyPlus

#endif
