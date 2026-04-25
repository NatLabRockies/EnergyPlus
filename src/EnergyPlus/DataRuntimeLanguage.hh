// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
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

#ifndef DataRuntimeLanguage_hh_INCLUDED
#define DataRuntimeLanguage_hh_INCLUDED

// C++ Headers
#include <functional>
#include <unordered_set>
#include <utility>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Data only module for EMS runtime language

namespace DataRuntimeLanguage {

    // Data module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.

    enum class ErlKeywordParam // keyword parameters for types of Erl statements
    {
        Invalid = -1,
        None,     // statement type not set
        Return,   // Return statement, as in leave program
        Goto,     // Goto statement, used in parsing to manage IF-ElseIf-Else-EndIf and nesting
        Set,      // Set statement, as in assign RHS to LHS
        Run,      // Run statement, used to call a subroutine from a main program
        If,       // If statement, begins an IF-ElseIf-Else-EndIf logic block
        ElseIf,   // ElseIf statement, begins an ElseIf block
        Else,     // Else statement, begins an Else block
        EndIf,    // EndIf statement, terminates an IF-ElseIf-Else-EndIf logic block
        While,    // While statement, begins a While block
        EndWhile, // EndWhile statement, terminates a While block
        Num
    };

    enum class Value
    {
        Invalid = -1,
        Null,       // Erl entity type, "Null" value
        Number,     // Erl entity type,  hard numeric value
        String,     // Erl entity type,  character data
        Array,      // Erl entity type,  not used yet, for future array type
        Variable,   // Erl entity type,  Erl variable
        Expression, // Erl entity type,  Erl expression
        Trend,      // Erl entity type,  Erl trend variable
        Error,      // Erl entity type, processing of an expression failed, returned error
        Num
    };

    enum class PtrDataType
    {
        Invalid = -1,
        Real,    // data type for overloaded pointer management, double real
        Integer, // data type for overloaded pointer management, integer
        Logical, // data type for overloaded pointer management, logical
        Num
    };

    // Parameters for identifying operator types in Erl
    // The number of these parameters indicates the order of precedence
    enum class ErlFunc
    {
        Invalid = -1,
        Null,
        Literal,        // Just stores a literal value
        Negative,       // -  (unary) No LHS?
        Divide,         // /
        Multiply,       // *
        Subtract,       // -  (binary)
        Add,            // +  (binary)
        Equal,          // ==
        NotEqual,       // <>
        LessOrEqual,    // <=
        GreaterOrEqual, // >=
        LessThan,       // <
        GreaterThan,    // >
        RaiseToPower,   // ^
        LogicalAND,     // &&
        LogicalOR,      // ||
        // note there is an important check "> 15" to distinguish operators from functions
        //  so be careful if renumber these parameters.  Binary operator additions should get inserted here rather than appended

        // parameters for built-in Erl functions, these are processed like operators and numbering
        // must be sequential with the operators.
        // math functions
        Round,    // accessor for Fortran's DNINT()
        Mod,      // accessor for Fortran's MOD()
        Sin,      // accessor for Fortran's SIN()
        Cos,      // accessor for Fortran's COS()
        ArcSin,   // accessor for Fortran's ASIN()
        ArcCos,   // accessor for Fortran's ACOS()
        DegToRad, // Multiplies degrees by DegToRad
        RadToDeg, // Divides radians by DegToRad
        Exp,      // accessor for Fortran's EXP()
        Ln,       // accessor for Fortran's LOG()
        Max,      // accessor for Fortran's MAX()
        Min,      // accessor for Fortran's MIN()
        ABS,      // accessor for Fortran's ABS()
        RandU,    // accessor for Fortran's Random_Number() intrinsic, uniform distribution
        RandG,    // accessor for Gaussian/normal distribution random number
        RandSeed, // accessor for Fortran's Random_Seed() intrinsic

        // begin psychrometric routines
        RhoAirFnPbTdbW,    // accessor for E+ psych routine
        CpAirFnW,          // accessor for E+ psych routine
        HfgAirFnWTdb,      // accessor for E+ psych routine
        HgAirFnWTdb,       // accessor for E+ psych routine
        TdpFnTdbTwbPb,     // accessor for E+ psych routine
        TdpFnWPb,          // accessor for E+ psych routine
        HFnTdbW,           // accessor for E+ psych routine
        HFnTdbRhPb,        // accessor for E+ psych routine
        TdbFnHW,           // accessor for E+ psych routine
        RhovFnTdbRh,       // accessor for E+ psych routine
        RhovFnTdbRhLBnd0C, // accessor for E+ psych routine
        RhovFnTdbWPb,      // accessor for E+ psych routine
        RhFnTdbRhov,       // accessor for E+ psych routine
        RhFnTdbRhovLBnd0C, // accessor for E+ psych routine
        RhFnTdbWPb,        // accessor for E+ psych routine
        TwbFnTdbWPb,       // accessor for E+ psych routine
        VFnTdbWPb,         // accessor for E+ psych routine
        WFnTdpPb,          // accessor for E+ psych routine
        WFnTdbH,           // accessor for E+ psych routine
        WFnTdbTwbPb,       // accessor for E+ psych routine
        WFnTdbRhPb,        // accessor for E+ psych routine
        PsatFnTemp,        // accessor for E+ psych routine
        TsatFnHPb,         // accessor for E+ psych routine
        TsatFnPb,          // not public in PsychRoutines.cc so not really available in EMS.
        CpCW,              // accessor for E+ psych routine
        CpHW,              // accessor for E+ psych routine
        RhoH2O,            // accessor for E+ psych routine

        // Simulation Management Functions
        FatalHaltEp,  // accessor for E+ error management, "Fatal" level
        SevereWarnEp, // accessor for E+ error management, "Severe" level
        WarnEp,       // accessor for E+ error management, "Warning" level

        // Trend variable handling Functions
        TrendValue,     // accessor for Erl Trend variables, instance value
        TrendAverage,   // accessor for Erl Trend variables, average value
        TrendMax,       // accessor for Erl Trend variables, max value
        TrendMin,       // accessor for Erl Trend variables, min value
        TrendDirection, // accessor for Erl Trend variables, slope value
        TrendSum,       // accessor for Erl Trend variables, sum value

        // Curve and Table access function
        CurveValue,

        // Weather data query functions
        TodayIsRain,             // Access TodayIsRain(hour, timestep)
        TodayIsSnow,             // Access TodayIsSnow(hour, timestep)
        TodayOutDryBulbTemp,     // Access TodayOutDryBulbTemp(hour, timestep)
        TodayOutDewPointTemp,    // Access TodayOutDewPointTemp(hour, timestep)
        TodayOutBaroPress,       // Access TodayOutBaroPress(hour, timestep)
        TodayOutRelHum,          // Access TodayOutRelHum(hour, timestep)
        TodayWindSpeed,          // Access TodayWindSpeed(hour, timestep)
        TodayWindDir,            // Access TodayWindDir(hour, timestep)
        TodaySkyTemp,            // Access TodaySkyTemp(hour, timestep)
        TodayHorizIRSky,         // Access TodayHorizIRSky(hour, timestep)
        TodayBeamSolarRad,       // Access TodayBeamSolarRad(hour, timestep)
        TodayDifSolarRad,        // Access TodayDifSolarRad(hour, timestep)
        TodayAlbedo,             // Access TodayAlbedo(hour, timestep)
        TodayLiquidPrecip,       // Access TodayLiquidPrecip(hour, timestep)
        TomorrowIsRain,          // Access TomorrowIsRain(hour, timestep)
        TomorrowIsSnow,          // Access TomorrowIsSnow(hour, timestep)
        TomorrowOutDryBulbTemp,  // Access TomorrowOutDryBulbTemp(hour, timestep)
        TomorrowOutDewPointTemp, // Access TomorrowOutDewPointTemp(hour, timestep)
        TomorrowOutBaroPress,    // Access TomorrowOutBaroPress(hour, timestep)
        TomorrowOutRelHum,       // Access TomorrowOutRelHum(hour, timestep)
        TomorrowWindSpeed,       // Access TomorrowWindSpeed(hour, timestep)
        TomorrowWindDir,         // Access TomorrowWindDir(hour, timestep)
        TomorrowSkyTemp,         // Access TomorrowSkyTemp(hour, timestep)
        TomorrowHorizIRSky,      // Access TomorrowHorizIRSky(hour, timestep)
        TomorrowBeamSolarRad,    // Access TomorrowBeamSolarRad(hour, timestep)
        TomorrowDifSolarRad,     // Access TomorrowDifSolarRad(hour, timestep)
        TomorrowAlbedo,          // Access TomorrowAlbedo(hour, timestep)
        TomorrowLiquidPrecip,    // Access TomorrowLiquidPrecip(hour, timestep)
        Num
    };

    int constexpr NumPossibleOperators(96); // total number of operators and built-in functions

    int constexpr MaxWhileLoopIterations(1000000); // protect from infinite loop in WHILE loops

    // Types

    struct OutputVarSensorType
    {
        // Members
        std::string Name;          // name of associated Erl Variable
        std::string UniqueKeyName; // unique key name associated with output variable
        std::string OutputVarName; // name of output variable
        bool CheckedOkay;          // set to true once checked out okay
        OutputProcessor::VariableType VariableType;
        int Index;                        // ref index in output processor, points to variable
        int VariableNum;                  // ref to global variable in runtime language
        Sched::Schedule *sched = nullptr; // ref index ptr to schedule service (filled if Schedule Value)
        //  INTEGER                                 :: VarType       = 0

        // Default Constructor
        OutputVarSensorType() : CheckedOkay(false), VariableType(OutputProcessor::VariableType::Invalid), Index(0), VariableNum(0)
        {
        }
    };

    struct InternalVarsAvailableType
    {
        // Members
        // structure for internal data available for use in Erl that are not sourced by output variables
        std::string DataTypeName;    // general internal variable name registered, All uppercase
        std::string UniqueIDName;    // unique id for internal var, All uppercase
        std::string Units;           // registered units, used for reporting and checks.
        PtrDataType PntrVarTypeUsed; // data type used: integer (PntrInteger) or real (PntrReal)
        Real64 *RealValue;           // POINTER to the REAL value that is being accessed
        int *IntValue;               // POINTER to the Integer value that is being accessed

        // Default Constructor
        InternalVarsAvailableType() : PntrVarTypeUsed(PtrDataType::Invalid), RealValue(nullptr), IntValue(nullptr)
        {
        }
    };

    struct InternalVarsUsedType
    {
        // Members
        // structure for internal data that user has selected to use in Erl.
        std::string Name;                 // Erl variable name
        std::string InternalDataTypeName; // general internal variable name, All uppercase
        std::string UniqueIDName;         // unique id for internal var, All uppercase
        bool CheckedOkay;                 // set to true once matched to available internal var
        int ErlVariableNum;               // points to global Erl variable, matches Name
        int InternVarNum;                 // points to index match in EMSInternalVarsAvailable structure

        // Default Constructor
        InternalVarsUsedType() : CheckedOkay(false), ErlVariableNum(0), InternVarNum(0)
        {
        }
    };

    struct EMSActuatorAvailableType
    {
        // Members
        // structure for all the actuators available for use in Erl
        std::string ComponentTypeName; // general actuator name registered, All uppercase
        std::string UniqueIDName;      // unique id for actuator, All uppercase
        std::string ControlTypeName;   // control type id for actuator, All uppercase
        std::string Units;             // control value units, used for reporting and checks.
        int handleCount;               // Number of times you tried to get a handle on this actuator,
                                       // whether from EMS:Actuator or getActuatorHandle (API)
        PtrDataType PntrVarTypeUsed;   // data type used: integer (PntrInteger), real (PntrReal) or logical (PntrLogical)
        bool *Actuated;                // POINTER to the logical value that signals EMS is actuating
        Real64 *RealValue;             // POINTER to the REAL value that is being actuated
        int *IntValue;                 // POINTER to the Integer value that is being actuated
        bool *LogValue;                // POINTER to the Logical value that is being actuated

        // Default Constructor
        EMSActuatorAvailableType()
            : handleCount(0), PntrVarTypeUsed(PtrDataType::Invalid), Actuated(nullptr), RealValue(nullptr), IntValue(nullptr), LogValue(nullptr)
        {
        }
    };

    struct ActuatorUsedType
    {
        // Members
        // structure for actuators user selected to use in Erl
        std::string Name;              // Erl variable name
        std::string ComponentTypeName; // general actuator name, All uppercase
        std::string UniqueIDName;      // unique id for actuator, All uppercase
        std::string ControlTypeName;   // control type id for actuator, All uppercase
        bool CheckedOkay;              // set to true once matched to available actuator
        int ErlVariableNum;            // points to global Erl variable, matches Name
        int ActuatorVariableNum;       // points to index match in EMSActuatorAvailable structure
        bool wasActuated = false;      // issue #10944: true once any Erl program has set this actuator

        // Default Constructor
        ActuatorUsedType() : CheckedOkay(false), ErlVariableNum(0), ActuatorVariableNum(0)
        {
        }
    };

    struct EMSProgramCallManagementType
    {
        // Members
        // structure for Erl program calling managers
        std::string Name;                     // user defined name for calling manager
        EMSManager::EMSCallFrom CallingPoint; // EMS Calling point for this manager, see parameters emsCallFrom*
        int NumErlPrograms;                   // count of total number of Erl programs called by this manager
        Array1D_int ErlProgramARR;            // list of integer pointers to Erl programs used by this manager

        // Default Constructor
        EMSProgramCallManagementType() : CallingPoint(EMSManager::EMSCallFrom::Invalid), NumErlPrograms(0)
        {
        }
    };

    struct ErlValueType
    {
        // Members
        // instance data structure for the values taken by Erl variables, nested structure in ErlVariable
        Value Type;         // value type, eg. ValueNumber,
        Real64 Number;      // numeric value instance for Erl variable
        std::string String; // string data types in Erl (not used yet)
        int Variable;       // Pointer to another Erl variable
        //  Might be good to change names to VariableNum and ExpressionNum just to be clear
        int Expression;      // Pointer to another Erl expression (e.g. compound operators)
        bool TrendVariable;  // true if Erl variable is really a trend variable
        int TrendVarPointer; // index to match in TrendVariable structure
        std::string Error;   // holds error message string for reporting
        bool initialized;    // true if number value has been SET (ie. has been on LHS in SET expression)

        // Default Constructor
        ErlValueType() : Type(Value::Null), Number(0.0), Variable(0), Expression(0), TrendVariable(false), TrendVarPointer(0), initialized(false)
        {
        }

        // Member Constructor
        ErlValueType(Value const Type,          // value type, eg. ValueNumber,
                     Real64 const Number,       // numeric value instance for Erl variable
                     std::string const &String, // string data types in Erl (not used yet)
                     int const Variable,        // Pointer to another Erl variable
                     int const Expression,      // Pointer to another Erl expression (e.g. compound operators)
                     bool const TrendVariable,  // true if Erl variable is really a trend variable
                     int const TrendVarPointer, // index to match in TrendVariable structure
                     std::string const &Error,  // holds error message string for reporting
                     bool const initialized)
            : Type(Type), Number(Number), String(String), Variable(Variable), Expression(Expression), TrendVariable(TrendVariable),
              TrendVarPointer(TrendVarPointer), Error(Error), initialized(initialized)
        {
        }
    };

    struct ErlVariableType
    {
        // Members
        // structure for Erl variables
        std::string Name;            // Erl Variable Name
        int StackNum;                // 0 for global Erl variables, index in ErlStack structure if local
        ErlValueType Value;          // values taken by Erl variables
        bool ReadOnly;               // true if Erl variable is read-only
        bool SetByExternalInterface; // set to true if value is set by ExternalInterface
        bool SetByGlobalVariable;
        bool SetByInternalVariable;

        // Default Constructor
        ErlVariableType() : StackNum(0), ReadOnly(false), SetByExternalInterface(false), SetByGlobalVariable(false), SetByInternalVariable(false)
        {
        }
    };

    struct InstructionType
    {
        // Members
        // nested structure inside ErlStack that holds program instructions
        int LineNum;                                  // Erl program line number reference
        DataRuntimeLanguage::ErlKeywordParam Keyword; // type of instruction for this line, e.g. KeywordSet, KeywordIf, etc
        int Argument1;                                // Index to a variable, function, expression, or stack
        int Argument2;                                // Index to a variable, function, expression, or stack

        // Default Constructor
        InstructionType() : LineNum(0), Keyword(DataRuntimeLanguage::ErlKeywordParam::None), Argument1(0), Argument2(0)
        {
        }
    };

    struct ErlStackType // Stores Erl programs in a stack of statements/instructions
    {
        // Members
        std::string Name;                     // Erl program or subroutine name, user defined
        int NumLines;                         // count of lines in Erl program or subroutine
        Array1D_string Line;                  // string array holding lines of Erl code (for processing)
        int NumInstructions;                  // count of program instructions in stack
        Array1D<InstructionType> Instruction; // structure array of program instructions
        int NumErrors;                        // count of errors during stack parsing
        Array1D_string Error;                 // array of error messages from stack parsing

        // Default Constructor
        ErlStackType() : NumLines(0), NumInstructions(0), NumErrors(0)
        {
        }
    };

    struct ErlExpressionType
    {
        // Members
        ErlFunc Operator;              // indicates the type of operator or function 1..64
        int NumOperands;               // count of operands in expression
        Array1D<ErlValueType> Operand; // holds Erl values for operands in expression

        // Default Constructor
        ErlExpressionType() : Operator(ErlFunc::Invalid), NumOperands(0)
        {
        }
    };

    struct Operator
    {
        // Members
        // structure for operators and functions, used to look up information about each operator or function
        std::string_view symbol; // string representation of operator or function (for reporting)
        int numOperands = 0;     // count of operands or function arguments.
    };

    static constexpr std::array<std::string_view, (int)ErlFunc::Num> ErlFuncNamesUC = {
        "",   // Null
        "",   // Literal
        "-",  // Negative
        "/",  // Divide
        "*",  // Multiply
        "-",  // Subtract
        "+",  // Add
        "==", // Equal
        "<>", // NotEqual
        "<=", // LessOrEqual
        ">=", // GreaterOrEqual
        "<",  // LessThan
        ">",  // GreaterThan
        "^",  // RaiseToPower
        "&&", // LogicalAND
        "||", // LogicalOR
        // note there is an important check "> 15" to distinguish operators from functions
        //  so be careful if renumber these parameters.  Binary operator additions should get inserted here rather than appended

        // parameters for built-in Erl functions, these are processed like operators and numbering
        // must be sequential with the operators.
        // math functions
        "@ROUND",          // Round
        "@MOD",            // Mod
        "@SIN",            // Sin
        "@COS",            // Cos
        "@ARCSIN",         // ArcSin
        "@ARCCOS",         // ArcCos
        "@DEGTORAD",       // DegToRad
        "@RADTODEG",       // RadToDeg
        "@EXP",            // Exp
        "@LN",             // Ln
        "@MAX",            // Max
        "@MIN",            // Min
        "@ABS",            // Abs
        "@RANDOMUNIFORMU", // RandU
        "@RANDOMGAUSSIAN", // RandG
        "@SEEDRANDOM",     // RandSeed

        // begin psychrometric routines
        "@RHOAIRFNPBTDBW",    // RhoAirFnPbTdbW
        "@CPAIRFNW",          // CpAirFnW
        "@HFGAIRFNWTDB",      // HfgAirFnWTdb
        "@HGAIRFNWTDB",       // HgAirFnWTdb
        "@TDPFNTDBTWBPB",     // TdpFnTdbTwbPb
        "@TDPFNWPB",          // TdpFnWPb
        "@HFNTDBW",           // HFnTdbW
        "@HFNTDBRHPB",        // HFnTdbRhPb
        "@TDBFNHW",           // TdbFnHW
        "@RHOVFNTDBRH",       // RhovFnTdbRh
        "@RHOVFNTDBRHLBND0C", // RhovFnTdbRhLBnd0C
        "@RHOVFNTDBWPB",      // RhovFnTdbWPb
        "@RHFNTDBRHOV",       // RhFnTdbRhov
        "@RHFNTDBRHOVBND0C",  // RhFnTdbRhovLBnd0C
        "@RHFNTDBWPB",        // RhFnTdbWPb
        "@TWBFNTDBWPB",       // TwbFnTdbWPb
        "@VFNTDBWPB",         // VFnTdbWPb
        "@WFNTDPPB",          // WFnTdpPb
        "@WFNTDBH",           // WFnTdbH
        "@WFNTDBTWBPB",       // WFnTdbTwbPb
        "@WFNTDBRHPB",        // WFnTdbRhPb
        "@PSATFNTEMP",        // PsatFnTemp
        "@TSATFNHPB",         // TsatFnHPb
        "@TSATFNPB",          // TsatFnPb
        "@CPCW",              // CpCW
        "@CPHW",              // CpHW
        "@RHOH2O",            // RhoH2O

        // Simulation Management Functions
        "@FATALHALTEP",  // FatalHaltEp
        "@SEVEREWARNEP", // SevereWarnEp
        "@WARNEP",       // WarnEp

        // Trend variable handling Functions
        "@TRENDVALUE",     // TrendValue
        "@TRENDAVERAGE",   // TrendAverage
        "@TRENDMAX",       // TrendMax
        "@TRENDMIN",       // TrendMin
        "@TRENDDIRECTION", // TrendDirection
        "@TRENDSUM",       // TrendSum

        // Curve and Table access function
        "@CURVEVALUE", // CurveValue

        // Weather data query functions
        "@TODAYISRAIN",             // TodayIsRain
        "@TODAYISSNOW",             // TodayIsSnow
        "@TODAYOUTDRYBULBTEMP",     // TodayOutDryBulbTemp
        "@TODAYOUTDEWPOINTTEMP",    // TodayOutDewPointTemp
        "@TODAYOUTBAROPRESS",       // TodayOutBaroPress
        "@TODAYOUTRELHUM",          // TodayOutRelHum
        "@TODAYWINDSPEED",          // TodayWindSpeed
        "@TODAYWINDDIR",            // TodayWindDir
        "@TODAYSKYTEMP",            // TodaySkyTemp
        "@TODAYHORIZRSKY",          // TodayHorizIRSky
        "@TODAYBEAMSOLARRAD",       // TodayBeamSolarRad
        "@TODAYDIFSOLARRAD",        // TodayDifSolarRad
        "@TODAYALBEDO",             // TodayAlbedo
        "@TODAYLIQUIDPRECIP",       // TodayLiquidPrecip
        "@TOMORROWISRAIN",          // TomorrowIsRain
        "@TOMORROWISSNOW",          // TomorrowIsSnow
        "@TOMORROWOUTDRYBULBTEMP",  // TomorrowOutDryBulbTemp
        "@TOMORROWOUTDEWPOINTTEMP", // TomorrowOutDewPointTemp
        "@TOMORROWOUTBAROPRESS",    // TomorrowOutBaroPress
        "@TOMORROWOUTRELHUM",       // TomorrowOutRelHum
        "@TOMORROWWINDSPEED",       // TomorrowWindSpeed
        "@TOMORROWWINDDIR",         // TomorrowWindDir
        "@TOMORROWSKYTEMP",         // TomorrowSkyTemp
        "@TOMORROWHORIZRSKY",       // TomorrowHorizIRSky
        "@TOMORROWBEAMSOLARRAD",    // TomorrowBeamSolarRad
        "@TOMORROWDIFSOLARRAD",     // TomorrowDifSolarRad
        "@TOMORROWALBEDO",          // TomorrowAlbedo
        "@TOMORROWLIQUIDPRECIP"     // TomorrowLiquidPrecip
    };

    static constexpr std::array<int, (int)ErlFunc::Num> ErlFuncNumOperands = {
        0, // Null
        1, // Literal
        0, // Negative
        2, // Divide
        2, // Multiply
        2, // Subtract
        2, // Add
        2, // Equal
        2, // NotEqual
        2, // LessOrEqual
        2, // GreaterOrEqual
        2, // LessThan
        2, // GreaterThan
        2, // RaiseToPower
        2, // LogicalAND
        2, // LogicalOR
        1, // Round
        2, // Mod
        1, // Sin
        1, // Cos
        1, // ArcSin
        1, // ArcCos
        1, // DegToRad
        1, // RadToDeg
        1, // Exp
        1, // Ln
        2, // Max
        2, // Min
        1, // ABS
        2, // RandU
        4, // RandG
        1, // RandSeed

        // begin psychrometric routines
        3, // RhoAirFnPbTdbW
        1, // CpAirFnW
        2, // HfgAirFnWTdb
        2, // HgAirFnWTdb
        3, // TdpFnTdbTwbPb
        2, // TdpFnWPb
        2, // HFnTdbW
        3, // HFnTdbRhPb
        2, // TdbFnHW
        2, // RhovFnTdbRh
        2, // RhovFnTdbRhLBnd0C
        3, // RhovFnTdbWPb
        2, // RhFnTdbRhov
        2, // RhFnTdbRhovLBnd0C
        3, // RhFnTdbWPb
        3, // TwbFnTdbWPb
        3, // VFnTdbWPb
        2, // WFnTdpPb
        2, // WFnTdbH
        3, // WFnTdbTwbPb
        4, // WFnTdbRhPb
        1, // PsatFnTemp
        2, // TsatFnHPb
        1, // TsatFnPb
        1, // CpCW
        1, // CpHW
        1, // RhoH2O

        // Simulation Management Functions
        1, // FatalHaltEp
        1, // SevereWarnEp
        1, // WarnEp

        // Trend variable handling Functions
        2, // TrendValue
        2, // TrendAverage
        2, // TrendMax
        2, // TrendMin
        2, // TrendDirection
        2, // TrendSum

        // Curve and Table access function
        6, // CurveValue

        // Weather data query functions
        2, // TodayIsRain
        2, // TodayIsSnow
        2, // TodayOutDryBulbTemp
        2, // TodayOutDewPointTemp
        2, // TodayOutBaroPress
        2, // TodayOutRelHum
        2, // TodayWindSpeed
        2, // TodayWindDir
        2, // TodaySkyTemp
        2, // TodayHorizIRSky
        2, // TodayBeamSolarRad
        2, // TodayDifSolarRad
        2, // TodayAlbedo
        2, // TodayLiquidPrecip
        2, // TomorrowIsRain
        2, // TomorrowIsSnow
        2, // TomorrowOutDryBulbTemp
        2, // TomorrowOutDewPointTemp
        2, // TomorrowOutBaroPress
        2, // TomorrowOutRelHum
        2, // TomorrowWindSpeed
        2, // TomorrowWindDir
        2, // TomorrowSkyTemp
        2, // TomorrowHorizIRSky
        2, // TomorrowBeamSolarRad
        2, // TomorrowDifSolarRad
        2, // TomorrowAlbedo
        2  // TomorrowLiquidPrecip
    };

    struct TrendVariableType
    {
        // Members
        std::string Name;
        int ErlVariablePointer;       // the Erl variable being logged in trend
        int LogDepth;                 // number of timesteps back
        Array1D<Real64> TrendValARR;  // the main storage of trend data
        Array1D<Real64> tempTrendARR; // temporary holder during push
        Array1D<Real64> TimeARR;      // hours back in time for trend points

        // Default Constructor
        TrendVariableType() : ErlVariablePointer(0), LogDepth(0)
        {
        }
    };

    // EMS Actuator fast duplicate check lookup support
    typedef std::tuple<std::string, std::string, std::string> EMSActuatorKey;
    struct EMSActuatorKey_hash
    {
        inline static void hash_combine(std::size_t &seed, std::string const &s)
        {
            std::hash<std::string> hasher;
            seed ^= hasher(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        inline std::size_t operator()(EMSActuatorKey const &key) const
        {
            std::size_t seed(0);
            hash_combine(seed, std::get<0>(key));
            hash_combine(seed, std::get<1>(key));
            hash_combine(seed, std::get<2>(key));
            return seed;
        }
    };

    void ValidateEMSVariableName(EnergyPlusData &state,
                                 std::string const &cModuleObject, // the current object name
                                 std::string const &cFieldValue,   // the field value
                                 std::string const &cFieldName,    // the current field name
                                 bool &errFlag,                    // true if errors found in this routine, false otherwise.
                                 bool &ErrorsFound                 // true if errors found in this routine, untouched otherwise.
    );

    void ValidateEMSProgramName(EnergyPlusData &state,
                                std::string const &cModuleObject, // the current object name
                                std::string const &cFieldValue,   // the field value
                                std::string const &cFieldName,    // the current field name
                                std::string const &cSubType,      // sub type = Program or Subroutine
                                bool &errFlag,                    // true if errors found in this routine, false otherwise.
                                bool &ErrorsFound                 // true if errors found in this routine, untouched otherwise.
    );

} // namespace DataRuntimeLanguage

struct RuntimeLanguageData : BaseGlobalStruct
{

    // In the API, we allow the user to manipulate user-defined EMS globals, but we skip the built-in ones to avoid
    // problems.  The built-in ones will not always start at zero, so we keep a start/end to ignore that specific range.
    int emsVarBuiltInStart = 0;
    int emsVarBuiltInEnd = 0;

    int NumProgramCallManagers = 0;       // count of Erl program managers with calling points
    int NumSensors = 0;                   // count of EMS sensors used in model (data from output variables)
    int numActuatorsUsed = 0;             // count of EMS actuators used in model
    int numEMSActuatorsAvailable = 0;     // count of EMS actuators available for use in such a model
    int maxEMSActuatorsAvailable = 0;     // count of EMS current maximum actuators available for use in such a model
    int NumInternalVariablesUsed = 0;     // count of EMS internal variables used in model
    int numEMSInternalVarsAvailable = 0;  // count of EMS internal variables available for use in such a model
    int maxEMSInternalVarsAvailable = 0;  // count of EMS current maximum internal variables available for use in such a model
    int varsAvailableAllocInc = 1000;     // allocation increment for variable arrays
    int NumErlPrograms = 0;               // count of Erl programs in model
    int NumErlSubroutines = 0;            // count of Erl subroutines in model
    int NumUserGlobalVariables = 0;       // count of global EMS variables defined by user
    int NumErlVariables = 0;              // count of Erl variables
    int NumErlStacks = 0;                 // count of Erl program stacks in model. sum of programs and subroutines
    int NumExpressions = 0;               // count of Erl expressions
    int NumEMSOutputVariables = 0;        // count of EMS output variables, custom output variables from Erl
    int NumEMSMeteredOutputVariables = 0; // count of EMS metered output variables, custom meters from Erl
    int NumErlTrendVariables = 0;         // count of EMS trend variables in model
    int NumEMSCurveIndices = 0;           // count of EMS curve index variables in model
    int NumEMSConstructionIndices = 0;    // count of EMS construction index variables in model

    // ######################################################################################################################################
    //  code for ExternalInterface
    int NumExternalInterfaceGlobalVariables = 0;                           // count of ExternalInterface runtime variable
    int NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables = 0; // count of ExternalInterface runtime variable for FMUImport
    // will be updated with values from ExternalInterface
    int NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables = 0; // count of ExternalInterface runtime variable for FMUExport
    // will be updated with values from ExternalInterface
    int NumExternalInterfaceActuatorsUsed = 0;                           // count of ExternalInterface Actuators
    int NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed = 0; // count of ExternalInterface Actuators for FMUImport
    int NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed = 0; // count of ExternalInterface Actuators for FMUExport

    // ######################################################################################################################################

    bool OutputEDDFile = false;               // set to true if user requests EDD output file be written
    bool OutputFullEMSTrace = false;          // how much to write out to trace, if true do verbose for each line
    bool OutputEMSErrors = false;             // how much to write out to trace, if true include Erl error messages
    bool OutputEMSActuatorAvailFull = false;  // how much to write out to EDD file, if true dump full combinatorial actuator list
    bool OutputEMSActuatorAvailSmall = false; // how much to write out to EDD file, if true dump actuator list without key names
    bool OutputEMSInternalVarsFull = false;   // how much to write out to EDD file, if true dump full combinatorial internal list
    bool OutputEMSInternalVarsSmall = false;  // how much to write out to EDD file, if true dump internal list without key names

    Array2D_bool EMSConstructActuatorChecked;
    Array2D_bool EMSConstructActuatorIsOkay;

    // Object Data
    Array1D<DataRuntimeLanguage::ErlVariableType> ErlVariable;                        // holds Erl variables in a structure array
    Array1D<DataRuntimeLanguage::ErlStackType> ErlStack;                              // holds Erl programs in separate "stacks"
    Array1D<DataRuntimeLanguage::ErlExpressionType> ErlExpression;                    // holds Erl expressions in structure array
    Array1D<DataRuntimeLanguage::TrendVariableType> TrendVariable;                    // holds Erl trend variables in a structure array
    Array1D<DataRuntimeLanguage::OutputVarSensorType> Sensor;                         // EMS:SENSOR objects used (from output variables)
    Array1D<DataRuntimeLanguage::EMSActuatorAvailableType> EMSActuatorAvailable;      // actuators that could be used
    Array1D<DataRuntimeLanguage::ActuatorUsedType> EMSActuatorUsed;                   // actuators that are used
    Array1D<DataRuntimeLanguage::InternalVarsAvailableType> EMSInternalVarsAvailable; // internal data that could be used
    Array1D<DataRuntimeLanguage::InternalVarsUsedType> EMSInternalVarsUsed;           // internal data that are used
    Array1D<DataRuntimeLanguage::EMSProgramCallManagementType> EMSProgramCallManager; // program calling managers
    DataRuntimeLanguage::ErlValueType Null = DataRuntimeLanguage::ErlValueType(
        DataRuntimeLanguage::Value::Null, 0.0, "", 0, 0, false, 0, "", true); // special "null" Erl variable value instance
    DataRuntimeLanguage::ErlValueType False = DataRuntimeLanguage::ErlValueType(
        DataRuntimeLanguage::Value::Null, 0.0, "", 0, 0, false, 0, "", true); // special "false" Erl variable value instance
    DataRuntimeLanguage::ErlValueType True = DataRuntimeLanguage::ErlValueType(
        DataRuntimeLanguage::Value::Null, 0.0, "", 0, 0, false, 0, "", true); // special "True" Erl variable value instance, gets reset

    std::map<std::tuple<std::string, std::string, std::string>, int> EMSActuatorAvailableMap;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumProgramCallManagers = 0;
        this->NumSensors = 0;
        this->numActuatorsUsed = 0;
        this->numEMSActuatorsAvailable = 0;
        this->maxEMSActuatorsAvailable = 0;
        this->NumInternalVariablesUsed = 0;
        this->numEMSInternalVarsAvailable = 0;
        this->maxEMSInternalVarsAvailable = 0;
        this->varsAvailableAllocInc = 1000;
        this->NumErlPrograms = 0;
        this->NumErlSubroutines = 0;
        this->NumUserGlobalVariables = 0;
        this->NumErlVariables = 0;
        this->NumErlStacks = 0;
        this->NumExpressions = 0;
        this->NumEMSOutputVariables = 0;
        this->NumEMSMeteredOutputVariables = 0;
        this->NumErlTrendVariables = 0;
        this->NumEMSCurveIndices = 0;
        this->NumEMSConstructionIndices = 0;
        this->NumExternalInterfaceGlobalVariables = 0;
        this->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables = 0;
        this->NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables = 0;
        this->NumExternalInterfaceActuatorsUsed = 0;
        this->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed = 0;
        this->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed = 0;
        this->OutputEDDFile = false;
        this->OutputFullEMSTrace = false;
        this->OutputEMSErrors = false;
        this->OutputEMSActuatorAvailFull = false;
        this->OutputEMSActuatorAvailSmall = false;
        this->OutputEMSInternalVarsFull = false;
        this->OutputEMSInternalVarsSmall = false;
        this->EMSConstructActuatorChecked.deallocate();
        this->EMSConstructActuatorIsOkay.deallocate();
        this->ErlVariable.deallocate();
        this->ErlStack.deallocate();
        this->ErlExpression.deallocate();
        this->TrendVariable.deallocate();
        this->Sensor.deallocate();
        this->EMSActuatorAvailable.deallocate();
        this->EMSActuatorUsed.deallocate();
        this->EMSInternalVarsAvailable.deallocate();
        this->EMSInternalVarsUsed.deallocate();
        this->EMSProgramCallManager.deallocate();
        this->Null = DataRuntimeLanguage::ErlValueType(DataRuntimeLanguage::Value::Null, 0.0, "", 0, 0, false, 0, "", true);
        this->False = DataRuntimeLanguage::ErlValueType(DataRuntimeLanguage::Value::Null, 0.0, "", 0, 0, false, 0, "", true);
        this->True = DataRuntimeLanguage::ErlValueType(DataRuntimeLanguage::Value::Null, 0.0, "", 0, 0, false, 0, "", true);

        this->EMSActuatorAvailableMap.clear();
    }
};

} // namespace EnergyPlus

#endif
