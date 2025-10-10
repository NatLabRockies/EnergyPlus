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
#include <cstdlib>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

#if defined(_WIN32) && _MSC_VER < 1900
#    define snprintf _snprintf
#endif

namespace EnergyPlus::General {

// Module containing routines for general use

// MODULE INFORMATION:
//       AUTHOR         Fred Buhl, Linda Lawrie
//       DATE WRITTEN   December 2001

// PURPOSE OF THIS MODULE:
// contains routines (most likely numeric) that may be needed in several parts
// of EnergyPlus

// MODULE PARAMETER DEFINITIONS
static constexpr std::string_view BlankString;

enum class ReportType
{
    Invalid = -1,
    DXF,
    DXFWireFrame,
    VRML,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(ReportType::Num)> ReportTypeNamesUC{"DXF", "DXF:WIREFRAME", "VRML"};

enum class AvailRpt
{
    Invalid = -1,
    None,
    NotByUniqueKeyNames,
    Verbose,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(AvailRpt::Num)> AvailRptNamesUC{"NONE", "NOTBYUNIQUEKEYNAMES", "VERBOSE"};

enum class ERLdebugOutputLevel
{
    Invalid = -1,
    None,
    ErrorsOnly,
    Verbose,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(ERLdebugOutputLevel::Num)> ERLdebugOutputLevelNamesUC{"NONE", "ERRORSONLY", "VERBOSE"};

enum class ReportName
{
    Invalid = -1,
    Constructions,
    Viewfactorinfo,
    Variabledictionary,
    Surfaces,
    Energymanagementsystem,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(ReportName::Num)> ReportNamesUC{
    "CONSTRUCTIONS", "VIEWFACTORINFO", "VARIABLEDICTIONARY", "SURFACES", "ENERGYMANAGEMENTSYSTEM"};

enum class RptKey
{
    Invalid = -1,
    Costinfo,
    DXF,
    DXFwireframe,
    VRML,
    Vertices,
    Details,
    DetailsWithVertices,
    Lines,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(RptKey::Num)> RptKeyNamesUC{
    "COSTINFO", "DXF", "DXF:WIREFRAME", "VRML", "VERTICES", "DETAILS", "DETAILSWITHVERTICES", "LINES"};

// A second version that does not require a payload -- use lambdas
void SolveRoot(const EnergyPlusData &state,
               Real64 Eps,   // required absolute accuracy
               int MaxIte,   // maximum number of allowed iterations
               int &Flag,    // integer storing exit status
               Real64 &XRes, // value of x that solves f(x,Par) = 0
               const std::function<Real64(Real64)> &f,
               Real64 X_0, // 1st bound of interval that contains the solution
               Real64 X_1) // 2nd bound of interval that contains the solution
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Michael Wetter
    //       DATE WRITTEN   March 1999
    //       MODIFIED       Fred Buhl November 2000, R. Raustad October 2006 - made subroutine RECURSIVE
    //                      L. Gu, May 2017 - allow both Bisection and RegulaFalsi

    // PURPOSE OF THIS SUBROUTINE:
    // Find the value of x between x0 and x1 such that f(x,Par)
    // is equal to zero.

    // METHODOLOGY EMPLOYED:
    // Uses the Regula Falsi (false position) method (similar to secant method)

    // REFERENCES:
    // See Press et al., Numerical Recipes in Fortran, Cambridge University Press,
    // 2nd edition, 1992. Page 347 ff.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    // = -2: f(x0) and f(x1) have the same sign
    // = -1: no convergence
    // >  0: number of iterations performed

    Real64 constexpr SMALL(1.e-10);
    Real64 X0 = X_0;   // present 1st bound
    Real64 X1 = X_1;   // present 2nd bound
    Real64 XTemp = X0; // new estimate
    int NIte = 0;      // number of iterations
    int AltIte = 0;    // an accounter used for Alternation choice

    Real64 Y0 = f(X0); // f at X0
    Real64 Y1 = f(X1); // f at X1
    // check initial values
    if (Y0 * Y1 > 0) {
        Flag = -2;
        XRes = X0;
        return;
    }
    XRes = XTemp;

    while (true) {

        Real64 DY = Y0 - Y1;
        if (std::abs(DY) < SMALL) {
            DY = SMALL;
        }
        if (std::abs(X1 - X0) < SMALL) {
            break;
        }
        // new estimation
        switch (state.dataRootFinder->HVACSystemRootFinding.HVACSystemRootSolverMethod) {
        case HVACSystemRootSolverAlgorithm::RegulaFalsi: {
            XTemp = (Y0 * X1 - Y1 * X0) / DY;
            break;
        }
        case HVACSystemRootSolverAlgorithm::Bisection: {
            XTemp = (X1 + X0) / 2.0;
            break;
        }
        case HVACSystemRootSolverAlgorithm::RegulaFalsiThenBisection: {
            if (NIte > state.dataRootFinder->HVACSystemRootFinding.NumOfIter) {
                XTemp = (X1 + X0) / 2.0;
            } else {
                XTemp = (Y0 * X1 - Y1 * X0) / DY;
            }
            break;
        }
        case HVACSystemRootSolverAlgorithm::BisectionThenRegulaFalsi: {
            if (NIte <= state.dataRootFinder->HVACSystemRootFinding.NumOfIter) {
                XTemp = (X1 + X0) / 2.0;
            } else {
                XTemp = (Y0 * X1 - Y1 * X0) / DY;
            }
            break;
        }
        case HVACSystemRootSolverAlgorithm::Alternation: {
            if (AltIte > state.dataRootFinder->HVACSystemRootFinding.NumOfIter) {
                XTemp = (X1 + X0) / 2.0;
                if (AltIte >= 2 * state.dataRootFinder->HVACSystemRootFinding.NumOfIter) {
                    AltIte = 0;
                }
            } else {
                XTemp = (Y0 * X1 - Y1 * X0) / DY;
            }
            break;
        }
        case HVACSystemRootSolverAlgorithm::ShortBisectionThenRegulaFalsi: {
            if (NIte < 3) {
                XTemp = (X1 + X0) / 2.0;
            } else {
                XTemp = (Y0 * X1 - Y1 * X0) / DY;
            }
            break;
        }
        default: {
            XTemp = (Y0 * X1 - Y1 * X0) / DY;
        }
        }

        Real64 const YTemp = f(XTemp);

        ++NIte;
        ++AltIte;

        // check convergence
        if (std::abs(YTemp) < Eps) {
            Flag = NIte;
            XRes = XTemp;
            return;
        };

        // OK, so we didn't converge, lets check max iterations to see if we should break early
        if (NIte > MaxIte) {
            break;
        }

        // Finally, if we make it here, we have not converged, and we still have iterations left, so continue
        // and reassign values (only if further iteration required)
        if (Y0 < 0.0) {
            if (YTemp < 0.0) {
                X0 = XTemp;
                Y0 = YTemp;
            } else {
                X1 = XTemp;
                Y1 = YTemp;
            }
        } else {
            if (YTemp < 0.0) {
                X1 = XTemp;
                Y1 = YTemp;
            } else {
                X0 = XTemp;
                Y0 = YTemp;
            }
        } // ( Y0 < 0 )
    } // Cont

    // if we make it here we haven't converged, so just set the flag and leave
    Flag = -1;
    XRes = XTemp;
}

void MovingAvg(Array1D<Real64> &DataIn, int const NumItemsInAvg)
{
    if (NumItemsInAvg <= 1) {
        return; // no need to average/smooth
    }

    Array1D<Real64> TempData(2 * DataIn.size()); // a scratch array twice the size, bottom end duplicate of top end

    for (std::size_t i = 1; i <= DataIn.size(); ++i) {
        TempData(i) = TempData(DataIn.size() + i) = DataIn(i); // initialize both bottom and top end
        DataIn(i) = 0.0;
    }

    for (std::size_t i = 1; i <= DataIn.size(); ++i) {
        for (int j = 1; j <= NumItemsInAvg; ++j) {
            DataIn(i) += TempData(DataIn.size() - NumItemsInAvg + i + j); // sum top end including NumItemsInAvg history terms
        }
        DataIn(i) /= NumItemsInAvg; // average to smooth over NumItemsInAvg window
    }
}

void ProcessDateString(EnergyPlusData &state,
                       std::string const &String,
                       int &PMonth,
                       int &PDay,
                       int &PWeekDay,
                       Weather::DateType &DateType, // DateType found (-1=invalid, 1=month/day, 2=nth day in month, 3=last day in month)
                       bool &ErrorsFound,
                       ObjexxFCL::Optional_int PYear)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 1999

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine will process a date from a string and determine
    // the proper month and day for that date string.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool errFlag;

    int FstNum = int(Util::ProcessNumber(String, errFlag));
    DateType = Weather::DateType::Invalid;
    if (!errFlag) {
        // Entered single number, do inverse JDay
        if (FstNum == 0) {
            PMonth = 0;
            PDay = 0;
            DateType = Weather::DateType::MonthDay;
        } else if (FstNum < 0 || FstNum > 366) {
            ShowSevereError(state, format("Invalid Julian date Entered={}", String));
            ErrorsFound = true;
        } else {
            InvOrdinalDay(FstNum, PMonth, PDay, 0);
            DateType = Weather::DateType::LastDayInMonth;
        }
    } else {
        int NumTokens = 0;
        int TokenDay = 0;
        int TokenMonth = 0;
        int TokenWeekday = 0;
        // Error when processing as number, try x/x
        if (!present(PYear)) {
            DetermineDateTokens(state, String, NumTokens, TokenDay, TokenMonth, TokenWeekday, DateType, ErrorsFound);
        } else {
            int TokenYear = 0;
            DetermineDateTokens(state, String, NumTokens, TokenDay, TokenMonth, TokenWeekday, DateType, ErrorsFound, TokenYear);
            PYear = TokenYear;
        }
        if (DateType == Weather::DateType::MonthDay) {
            PDay = TokenDay;
            PMonth = TokenMonth;
        } else if (DateType == Weather::DateType::NthDayInMonth || DateType == Weather::DateType::LastDayInMonth) {
            // interpret as TokenDay TokenWeekday in TokenMonth
            PDay = TokenDay;
            PMonth = TokenMonth;
            PWeekDay = TokenWeekday;
        }
    }
}

void DetermineDateTokens(EnergyPlusData &state,
                         std::string const &String,
                         int &NumTokens,                   // Number of tokens found in string
                         int &TokenDay,                    // Value of numeric field found
                         int &TokenMonth,                  // Value of Month field found (1=Jan, 2=Feb, etc)
                         int &TokenWeekday,                // Value of Weekday field found (1=Sunday, 2=Monday, etc), 0 if none
                         Weather::DateType &DateType,      // DateType found (-1=invalid, 1=month/day, 2=nth day in month, 3=last day in month)
                         bool &ErrorsFound,                // Set to true if cannot process this string as a date
                         ObjexxFCL::Optional_int TokenYear // Value of Year if one appears to be present and this argument is present
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is invoked for date fields that appear to be strings (give
    // error when ProcessNumber is used).

    // METHODOLOGY EMPLOYED:
    // Delete everything that is extraneous to the date information needed.  Process what
    // is left.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr int NumSingleChars(3);
    static constexpr std::array<std::string_view, NumSingleChars> SingleChars{"/", ":", "-"};
    static constexpr int NumDoubleChars(6);
    static constexpr std::array<std::string_view, NumDoubleChars> DoubleChars{
        "ST ", "ND ", "RD ", "TH ", "OF ", "IN "}; // Need trailing spaces: Want these only at end of words
    static constexpr std::array<std::string_view, 12> Months{"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    static constexpr std::array<std::string_view, 7> Weekdays{"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string CurrentString = String;
    Array1D_string Fields(3);
    bool InternalError = false;
    bool WkDayInMonth = false;

    NumTokens = 0;
    TokenDay = 0;
    TokenMonth = 0;
    TokenWeekday = 0;
    DateType = Weather::DateType::Invalid;
    if (present(TokenYear)) {
        TokenYear = 0;
    }
    // Take out separator characters, other extraneous stuff

    for (int Loop = 0; Loop < NumSingleChars; ++Loop) {
        size_t Pos = index(CurrentString, SingleChars[Loop]);
        while (Pos != std::string::npos) {
            CurrentString[Pos] = ' ';
            Pos = index(CurrentString, SingleChars[Loop]);
        }
    }

    for (int Loop = 0; Loop < NumDoubleChars; ++Loop) {
        size_t Pos = index(CurrentString, DoubleChars[Loop]);
        while (Pos != std::string::npos) {
            CurrentString.replace(Pos, 2, "  ");
            Pos = index(CurrentString, DoubleChars[Loop]);
            WkDayInMonth = true;
        }
    }

    strip(CurrentString);
    if (CurrentString == BlankString) {
        ShowSevereError(state, format("Invalid date field={}", String));
        ErrorsFound = true;
    } else {
        int Loop = 0;
        bool errFlag = false;
        int NumField1;
        int NumField2;
        int NumField3;
        while (Loop < 3) { // Max of 3 fields
            if (CurrentString == BlankString) {
                break;
            }
            size_t Pos = index(CurrentString, ' ');
            ++Loop;
            if (Pos == std::string::npos) {
                Pos = CurrentString.length();
            }
            Fields(Loop) = CurrentString.substr(0, Pos);
            CurrentString.erase(0, Pos);
            strip(CurrentString);
        }
        if (not_blank(CurrentString)) {
            ShowSevereError(state, format("Invalid date field={}", String));
            ErrorsFound = true;
        } else if (Loop == 2) {
            // Field must be Day Month or Month Day (if both numeric, mon / day)
            InternalError = false;
            NumField1 = int(Util::ProcessNumber(Fields(1), errFlag));
            if (errFlag) {
                // Month day, but first field is not numeric, 2nd must be
                NumField2 = int(Util::ProcessNumber(Fields(2), errFlag));
                if (errFlag) {
                    ShowSevereError(state, format("Invalid date field={}", String));
                    InternalError = true;
                } else {
                    TokenDay = NumField2;
                }
                TokenMonth = Util::FindItemInList(Fields(1).substr(0, 3), Months.begin(), Months.end());
                ValidateMonthDay(state, String, TokenDay, TokenMonth, InternalError);
                if (!InternalError) {
                    DateType = Weather::DateType::MonthDay;
                } else {
                    ErrorsFound = true;
                }
            } else {
                // Month Day, first field was numeric, if 2nd is, then it's month<num> day<num>
                NumField2 = int(Util::ProcessNumber(Fields(2), errFlag));
                if (!errFlag) {
                    TokenMonth = NumField1;
                    TokenDay = NumField2;
                    ValidateMonthDay(state, String, TokenDay, TokenMonth, InternalError);
                    if (!InternalError) {
                        DateType = Weather::DateType::MonthDay;
                    } else {
                        ErrorsFound = true;
                    }
                } else { // 2nd field was not numeric.  Must be Month
                    TokenDay = NumField1;
                    TokenMonth = Util::FindItemInList(Fields(2).substr(0, 3), Months.begin(), Months.end());
                    ValidateMonthDay(state, String, TokenDay, TokenMonth, InternalError);
                    if (!InternalError) {
                        DateType = Weather::DateType::MonthDay;
                        NumTokens = 2;
                    } else {
                        ErrorsFound = true;
                    }
                }
            }
        } else if (Loop == 3) {
            // Field must be some combination of <num> Weekday Month (if WkDayInMonth true)
            if (WkDayInMonth) {
                NumField1 = int(Util::ProcessNumber(Fields(1), errFlag));
                if (!errFlag) { // the expected result
                    TokenDay = NumField1;
                    TokenWeekday = Util::FindItemInList(Fields(2).substr(0, 3), Weekdays.begin(), Weekdays.end());
                    if (TokenWeekday == 0) {
                        TokenMonth = Util::FindItemInList(Fields(2).substr(0, 3), Months.begin(), Months.end());
                        TokenWeekday = Util::FindItemInList(Fields(3).substr(0, 3), Weekdays.begin(), Weekdays.end());
                        if (TokenMonth == 0 || TokenWeekday == 0) {
                            InternalError = true;
                        }
                    } else {
                        TokenMonth = Util::FindItemInList(Fields(3).substr(0, 3), Months.begin(), Months.end());
                        if (TokenMonth == 0) {
                            InternalError = true;
                        }
                    }
                    DateType = Weather::DateType::NthDayInMonth;
                    NumTokens = 3;
                    if (TokenDay < 0 || TokenDay > 5) {
                        InternalError = true;
                    }
                } else { // first field was not numeric....
                    if (Fields(1) == "LA") {
                        DateType = Weather::DateType::LastDayInMonth;
                        NumTokens = 3;
                        TokenWeekday = Util::FindItemInList(Fields(2).substr(0, 3), Weekdays.begin(), Weekdays.end());
                        if (TokenWeekday == 0) {
                            TokenMonth = Util::FindItemInList(Fields(2).substr(0, 3), Months.begin(), Months.end());
                            TokenWeekday = Util::FindItemInList(Fields(3).substr(0, 3), Weekdays.begin(), Weekdays.end());
                            if (TokenMonth == 0 || TokenWeekday == 0) {
                                InternalError = true;
                            }
                        } else {
                            TokenMonth = Util::FindItemInList(Fields(3).substr(0, 3), Months.begin(), Months.end());
                            if (TokenMonth == 0) {
                                InternalError = true;
                            }
                        }
                    } else { // error....
                        ShowSevereError(state, format("First date field not numeric, field={}", String));
                    }
                }
            } else { // mm/dd/yyyy or yyyy/mm/dd
                NumField1 = int(Util::ProcessNumber(Fields(1), errFlag));
                NumField2 = int(Util::ProcessNumber(Fields(2), errFlag));
                NumField3 = int(Util::ProcessNumber(Fields(3), errFlag));
                DateType = Weather::DateType::MonthDay;
                // error detection later..
                if (NumField1 > 100) {
                    if (present(TokenYear)) {
                        TokenYear = NumField1;
                    }
                    TokenMonth = NumField2;
                    TokenDay = NumField3;
                } else if (NumField3 > 100) {
                    if (present(TokenYear)) {
                        TokenYear = NumField3;
                    }
                    TokenMonth = NumField1;
                    TokenDay = NumField2;
                }
            }
        } else {
            // Not enough or too many fields
            ShowSevereError(state, format("Invalid date field={}", String));
            ErrorsFound = true;
        }
    }

    if (InternalError) {
        DateType = Weather::DateType::Invalid;
        ErrorsFound = true;
    }
}

void ValidateMonthDay(EnergyPlusData &state,
                      std::string const &String, // REAL(r64) string being processed
                      int const Day,
                      int const Month,
                      bool &ErrorsFound)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine validates a potential Day, Month values, produces an error
    // message when not valid, and sets error flag.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::array<int, 12> EndMonthDay = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    bool InternalError = false;
    if (Month < 1 || Month > 12) {
        InternalError = true;
    }
    if (!InternalError) {
        if (Day < 1 || Day > EndMonthDay[Month - 1]) {
            InternalError = true;
        }
    }
    if (InternalError) {
        ShowSevereError(state, format("Invalid Month Day date format={}", String));
        ErrorsFound = true;
    } else {
        ErrorsFound = false;
    }
}

int OrdinalDay(int const Month,        // Month, 1..12
               int const Day,          // Day of Month, not validated by month
               int const LeapYearValue // 1 if leap year indicated, 0 if not
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   September 1997
    //       RE-ENGINEERED  from JDAYF in BLAST/IBLAST

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns the appropriate Julian Day value for the input
    // Month and Day.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    static constexpr std::array<int, 12> EndDayofMonth = {31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    // End day numbers of each month (without Leap Year)

    if (Month == 1) {
        //                                       CASE 1: JANUARY
        return Day;

    } else if (Month == 2) {
        //                                       CASE 2: FEBRUARY
        return Day + EndDayofMonth[0];

    } else if ((Month >= 3) && (Month <= 12)) {
        //                                       CASE 3: REMAINING MONTHS
        return Day + EndDayofMonth[Month - 2] + LeapYearValue;

    } else {
        return 0;
    }
}

void InvOrdinalDay(int const Number, int &PMonth, int &PDay, int const LeapYr)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 1999

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine performs and inverse Julian Day
    // calculation, using an input JulianDay and returning
    // appropriate Month and Day.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::array<int, 13> EndOfMonth = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int WMonth;
    int LeapAddPrev;
    int LeapAddCur;

    if (Number < 0 || Number > 366) {
        return;
    }
    for (WMonth = 1; WMonth <= 12; ++WMonth) {
        if (WMonth == 1) {
            LeapAddPrev = 0;
            LeapAddCur = 0;
        } else if (WMonth == 2) {
            LeapAddPrev = 0;
            LeapAddCur = LeapYr;
        } else {
            LeapAddPrev = LeapYr;
            LeapAddCur = LeapYr;
        }
        if (Number > (EndOfMonth[WMonth - 1] + LeapAddPrev) && Number <= (EndOfMonth[WMonth] + LeapAddCur)) {
            break;
        }
    }
    PMonth = WMonth;
    PDay = Number - (EndOfMonth[WMonth - 1] + LeapAddCur);
}

bool BetweenDateHoursLeftInclusive(
    int const TestDate, int const TestHour, int const StartDate, int const StartHour, int const EndDate, int const EndHour)
{
    Real64 TestRatioOfDay = TestHour / 24.0;
    Real64 StartRatioOfDay = StartHour / 24.0;
    Real64 EndRatioOfDay = EndHour / 24.0;

    if (StartDate + StartRatioOfDay <= EndDate + EndRatioOfDay) { // Start Date <= End Date
        return (StartDate + StartRatioOfDay <= TestDate + TestRatioOfDay) && (TestDate + TestRatioOfDay <= EndDate + EndRatioOfDay);
    } else { // EndDate < StartDate
        return (EndDate + EndRatioOfDay <= TestDate + TestRatioOfDay) && (TestDate + TestRatioOfDay <= StartDate + StartRatioOfDay);
    }
}

bool BetweenDates(int const TestDate,  // Date to test
                  int const StartDate, // Start date in sequence
                  int const EndDate    // End date in sequence
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   June 2000

    // PURPOSE OF THIS FUNCTION:
    // This function returns true if the TestDate is between
    // (StartDate <= TestDate <= EndDate).

    // METHODOLOGY EMPLOYED:
    // The input dates are Julian Day format, year is irrelevant.
    // Thus, if StartDate > EndDate (i.e. StartDate = 1Dec and EndDate = 31Jan),
    // this routine accommodates.

    // REFERENCES:
    // Adapted from BLAST BTWEEN function.

    bool BetweenDates = false; // Default case

    if (StartDate <= EndDate) { // Start Date <= End Date
        if (TestDate >= StartDate && TestDate <= EndDate) {
            BetweenDates = true;
        }
    } else { // EndDate < StartDate
        if (TestDate <= EndDate || TestDate >= StartDate) {
            BetweenDates = true;
        }
    }

    return BetweenDates;
}

std::string CreateSysTimeIntervalString(EnergyPlusData &state)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   April 2003

    // PURPOSE OF THIS FUNCTION:
    // This function creates the current time interval of the system time step.

    // Using/Aliasing
    Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
    Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;

    Real64 constexpr FracToMin(60.0);

    //  ActualTimeS=INT(CurrentTime)+(SysTimeElapsed+(CurrentTime - INT(CurrentTime)))
    // CR6902  ActualTimeS=INT(CurrentTime-TimeStepZone)+SysTimeElapsed
    // [DC] TODO: Improve display accuracy up to fractional seconds using hh:mm:ss.0 format
    Real64 ActualTimeS = state.dataGlobal->CurrentTime - state.dataGlobal->TimeStepZone + SysTimeElapsed;
    Real64 ActualTimeE = ActualTimeS + TimeStepSys;
    int ActualTimeHrS = int(ActualTimeS);
    //  ActualTimeHrE=INT(ActualTimeE)
    int ActualTimeMinS = nint((ActualTimeS - ActualTimeHrS) * FracToMin);

    if (ActualTimeMinS == 60) {
        ++ActualTimeHrS;
        ActualTimeMinS = 0;
    }
    const std::string TimeStmpS = format("{:02}:{:02}", ActualTimeHrS, ActualTimeMinS);
    Real64 minutes = ((ActualTimeE - static_cast<int>(ActualTimeE)) * FracToMin);

    std::string TimeStmpE = format("{:02}:{:2.0F}", static_cast<int>(ActualTimeE), minutes);

    if (TimeStmpE[3] == ' ') {
        TimeStmpE[3] = '0';
    }
    return TimeStmpS + " - " + TimeStmpE;
}

// returns the Julian date for the first, second, etc. day of week for a given month
int nthDayOfWeekOfMonth(const EnergyPlusData &state,
                        int const dayOfWeek,  // day of week (Sunday=1, Monday=2, ...)
                        int const nthTime,    // nth time the day of the week occurs (first monday, third tuesday, ..)
                        int const monthNumber // January = 1
)
{
    // J. Glazer - August 2017
    int firstDayOfMonth = OrdinalDay(monthNumber, 1, state.dataEnvrn->CurrentYearIsLeapYear);
    int dayOfWeekForFirstDay = (state.dataEnvrn->RunPeriodStartDayOfWeek + firstDayOfMonth - 1) % 7;
    if (dayOfWeek >= dayOfWeekForFirstDay) {
        return firstDayOfMonth + (dayOfWeek - dayOfWeekForFirstDay) + 7 * (nthTime - 1);
    } else {
        return firstDayOfMonth + ((dayOfWeek + 7) - dayOfWeekForFirstDay) + 7 * (nthTime - 1);
    }
}

Real64 SafeDivide(Real64 const a, Real64 const b)
{

    // returns a / b while preventing division by zero

    // Locals
    Real64 constexpr SMALL(1.E-10);

    if (std::abs(b) >= SMALL) {
        return a / b;
    } else {
        return a / sign(SMALL, b);
    }
}

void Iterate(Real64 &ResultX,  // ResultX is the final Iteration result passed back to the calling routine
             Real64 const Tol, // Tolerance for Convergence
             Real64 const X0,  // Current value of X
             Real64 const Y0,  // Current value of the function Y(X)
             Real64 &X1,       // First Previous values of X
             Real64 &Y1,       // First Previous values of Y(X1)
             int const Iter,   // Number of iterations
             int &Cnvg         // Convergence flag  Cnvg = 0:  Not converged
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   March 2004

    // PURPOSE OF THIS SUBROUTINE:
    // Iteratively solves for the value of X which satisfies Y(X)=0.
    // The subroutine tests for convergence and provides a new guess for the value of the
    // independent variable X.

    // REFERENCES:
    // Linear Correction based on the RegulaFalsi routine in EnergyPlus

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr small(1.e-9); // Small Number used to approximate zero
    Real64 constexpr Perturb(0.1); // Perturbation applied to X to initialize iteration

    // Check for convergence by comparing change in X
    if (Iter != 1) {
        if (std::abs(X0 - X1) < Tol || Y0 == 0.0) {
            ResultX = X0;
            Cnvg = 1;
            return;
        }
    }

    // Not converged.
    Cnvg = 0;
    if (Iter == 1) {

        // New guess is specified by Perturb
        if (std::abs(X0) > small) {
            ResultX = X0 * (1.0 + Perturb);
        } else {
            ResultX = Perturb;
        }

    } else {

        // New guess calculated from LINEAR FIT of most recent two points
        Real64 DY = Y0 - Y1;
        if (std::abs(DY) < small) {
            DY = small;
        }
        // new estimation

        ResultX = (Y0 * X1 - Y1 * X0) / DY;
    }

    X1 = X0;
    Y1 = Y0;
}

int FindNumberInList(int const WhichNumber, Array1A_int const ListOfItems, int const NumItems)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   September 2001

    // PURPOSE OF THIS FUNCTION:
    // This function looks up a number(integer) in a similar list of
    // items and returns the index of the item in the list, if found.

    // Argument array dimensioning
    ListOfItems.dim(_);

    for (int Count = 1; Count <= NumItems; ++Count) {
        if (WhichNumber == ListOfItems(Count)) {
            return Count;
        }
    }

    return 0;
}

void DecodeMonDayHrMin(int const Item, // word containing encoded month, day, hour, minute
                       int &Month,     // month in integer format (1-12)
                       int &Day,       // day in integer format (1-31)
                       int &Hour,      // hour in integer format (1-24)
                       int &Minute     // minute in integer format (0:59)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine decodes the "packed" integer representation of
    // the Month, Day, Hour, and Minute.  Packed integers are used to
    // save memory allocation.  Original idea for this routine is contained
    // in DECMDH, BLAST code, by Jean Baugh.

    // METHODOLOGY EMPLOYED:
    // Using maximum integer concept the original date can be decoded
    // from the packed single word.  This relies on 4 byte integer representation
    // as a minimum (capable of representing up to 2,147,483,647).

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr int DecMon(100 * 100 * 100);
    static constexpr int DecDay(100 * 100);
    static constexpr int DecHr(100);

    int TmpItem = Item;
    Month = TmpItem / DecMon;
    TmpItem = (TmpItem - Month * DecMon);
    Day = TmpItem / DecDay;
    TmpItem -= Day * DecDay;
    Hour = TmpItem / DecHr;
    Minute = mod(TmpItem, DecHr);
}

void EncodeMonDayHrMin(int &Item,       // word containing encoded month, day, hour, minute
                       int const Month, // month in integer format (1:12)
                       int const Day,   // day in integer format (1:31)
                       int const Hour,  // hour in integer format (1:24)
                       int const Minute // minute in integer format (0:59)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine encodes the "packed" integer representation of
    // the Month, Day, Hour, and Minute.  Packed integers are used to
    // save memory allocation.  Original idea for this routine is contained
    // in DECMDH, BLAST code, by Jean Baugh.

    // METHODOLOGY EMPLOYED:
    // Using maximum integer concept the original date can be decoded
    // from the packed single word.  This relies on 4 byte integer representation
    // as a minimum (capable of representing up to 2,147,483,647).

    Item = ((Month * 100 + Day) * 100 + Hour) * 100 + Minute;
}

std::string CreateTimeString(Real64 const Time) // Time in seconds
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil
    //       DATE WRITTEN   January 2005

    // PURPOSE OF THIS FUNCTION:
    // This function creates the time stamp string from the time value specified in seconds.
    // Inspired by similar function CreateSysTimeIntervalString() in General.cc
    // However, this function provides better accuracy for sub-minute time steps
    // by also showing information down to the 10th of a second.
    // Note that Time is expected to be specified in REAL(r64).

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int Hours;      // Number of hours <= 24
    int Minutes;    // Remaining minutes < 60
    Real64 Seconds; // Remaining seconds < 60

    ParseTime(Time, Hours, Minutes, Seconds);

    // TimeStamp written with formatting
    // "hh:mm:ss.s"
    return fmt::format("{:02d}:{:02d}:{:04.1f}", Hours, Minutes, Seconds);
}

void ParseTime(Real64 const Time, // Time value in seconds
               int &Hours,        // Number of hours
               int &Minutes,      // Number of minutes < 60
               Real64 &Seconds    // Number of seconds < 60
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil
    //       DATE WRITTEN   January 2005

    // PURPOSE OF THIS FUNCTION:
    // This subroutine decomposes a time value specified in seconds
    // into a triplet { hours : minutes : seconds } such that
    // - minutes < 60
    // - seconds < 60

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int constexpr MinToSec = 60;
    int constexpr HourToSec = 60 * 60;

    // Get number of hours
    // This might undershoot the actual number of hours. See DO WHILE loop.
    Hours = int(Time) / HourToSec;

    // Compute remainder in seconds
    Real64 Remainder = (Time - Hours * HourToSec);

    // Compute minutes
    Minutes = int(Remainder) / MinToSec;

    // Compute remainder in seconds
    Remainder -= Minutes * MinToSec;

    // Compute seconds
    Seconds = Remainder;
}

void ScanForReports(EnergyPlusData &state,
                    std::string const &reportName,
                    bool &DoReport,
                    ObjexxFCL::Optional_string_const ReportKey,
                    ObjexxFCL::Optional_string Option1,
                    ObjexxFCL::Optional_string Option2)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2009

    // PURPOSE OF THIS SUBROUTINE:
    // This routine scans for the global "reports" settings, such as Variable Dictionary,
    // Surfaces (and options), Constructions, etc.

    // METHODOLOGY EMPLOYED:
    // First time routine is called, all the viable combinations/settings for the reports are
    // stored in SAVEd variables.  Later callings will retrieve those.

    if (state.dataGeneral->GetReportInput) {

        int NumNames;
        int NumNumbers;
        int IOStat;
        int RepNum;

        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        cCurrentModuleObject = "Output:Surfaces:List";

        int NumReports = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        enum
        {
            EMPTY,
            LINES,
            VERTICES,
            DETAILS,
            DETAILSWITHVERTICES,
            COSTINFO,
            VIEWFACTORINFO,
            DECAYCURVESFROMCOMPONENTLOADSSUMMARY
        };
        std::map<std::string, int> localMap = {{"", EMPTY},
                                               {"LINES", LINES},
                                               {"VERTICES", VERTICES},
                                               {"DETAILS", DETAILS},
                                               {"DETAILED", DETAILS},
                                               {"DETAIL", DETAILS},
                                               {"DETAILSWITHVERTICES", DETAILSWITHVERTICES},
                                               {"DETAILVERTICES", DETAILSWITHVERTICES},
                                               {"COSTINFO", COSTINFO},
                                               {"VIEWFACTORINFO", VIEWFACTORINFO},
                                               {"DECAYCURVESFROMCOMPONENTLOADSSUMMARY", DECAYCURVESFROMCOMPONENTLOADSSUMMARY}};

        for (RepNum = 1; RepNum <= NumReports; ++RepNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     RepNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumNames,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            try {
                int value = localMap[state.dataIPShortCut->cAlphaArgs(1)];
                switch (value) {
                case LINES:
                    state.dataGeneral->LineRpt = true;
                    state.dataGeneral->LineRptOption1 = state.dataIPShortCut->cAlphaArgs(2);
                    break;
                case VERTICES:
                    state.dataGeneral->SurfVert = true;
                    break;
                case DETAILS:
                    state.dataGeneral->SurfDet = true;
                    break;
                case DETAILSWITHVERTICES:
                    state.dataGeneral->SurfDetWVert = true;
                    break;
                case COSTINFO:
                    //   Custom case for reporting surface info for cost estimates (for first costs in optimizing)
                    state.dataGeneral->CostInfo = true;
                    break;
                case VIEWFACTORINFO: // actual reporting is in HeatBalanceIntRadExchange
                    state.dataGeneral->ViewFactorInfo = true;
                    state.dataGeneral->ViewRptOption1 = state.dataIPShortCut->cAlphaArgs(2);
                    break;
                case DECAYCURVESFROMCOMPONENTLOADSSUMMARY: // Should the Radiant to Convective Decay Curves from the
                                                           // load component report appear in the EIO file
                    state.dataGlobal->ShowDecayCurvesInEIO = true;
                    break;
                default: // including empty
                    ShowWarningError(state, format("{}: No {} supplied.", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(1)));
                    ShowContinueError(state,
                                      R"( Legal values are: "Lines", "Vertices", "Details", "DetailsWithVertices", "CostInfo", "ViewFactorIinfo".)");
                }
            } catch (int e) {
                ShowWarningError(state,
                                 format("{}: Invalid {}=\"{}\" supplied.",
                                        cCurrentModuleObject,
                                        state.dataIPShortCut->cAlphaFieldNames(1),
                                        state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  R"( Legal values are: "Lines", "Vertices", "Details", "DetailsWithVertices", "CostInfo", "ViewFactorIinfo".)");
            }
        }

        cCurrentModuleObject = "Output:Surfaces:Drawing";

        NumReports = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        for (RepNum = 1; RepNum <= NumReports; ++RepNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     RepNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumNames,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ReportType checkReportType =
                static_cast<ReportType>(getEnumValue(ReportTypeNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(1))));

            switch (checkReportType) {
            case ReportType::DXF: {
                state.dataGeneral->DXFReport = true;
                state.dataGeneral->DXFOption1 = state.dataIPShortCut->cAlphaArgs(2);
                state.dataGeneral->DXFOption2 = state.dataIPShortCut->cAlphaArgs(3);
            } break;
            case ReportType::DXFWireFrame: {
                state.dataGeneral->DXFWFReport = true;
                state.dataGeneral->DXFWFOption1 = state.dataIPShortCut->cAlphaArgs(2);
                state.dataGeneral->DXFWFOption2 = state.dataIPShortCut->cAlphaArgs(3);
            } break;
            case ReportType::VRML: {
                state.dataGeneral->VRMLReport = true;
                state.dataGeneral->VRMLOption1 = state.dataIPShortCut->cAlphaArgs(2);
                state.dataGeneral->VRMLOption2 = state.dataIPShortCut->cAlphaArgs(3);
            } break;
            default:
                break;
            }
        }

        RepNum = state.dataInputProcessing->inputProcessor->getNumSectionsFound("Report Variable Dictionary");
        if (RepNum > 0) {
            state.dataGeneral->VarDict = true;
            state.dataGeneral->VarDictOption1 = "REGULAR";
            state.dataGeneral->VarDictOption2 = "";
        }

        cCurrentModuleObject = "Output:VariableDictionary";

        NumReports = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        for (RepNum = 1; RepNum <= NumReports; ++RepNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     RepNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumNames,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            state.dataGeneral->VarDict = true;
            state.dataGeneral->VarDictOption1 = state.dataIPShortCut->cAlphaArgs(1);
            state.dataGeneral->VarDictOption2 = state.dataIPShortCut->cAlphaArgs(2);
        }

        cCurrentModuleObject = "Output:Constructions";
        NumReports = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        for (RepNum = 1; RepNum <= NumReports; ++RepNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     RepNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumNames,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(1), "CONSTRUCTIONS")) {
                state.dataGeneral->Constructions = true;
            } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(1), "MATERIALS")) {
                state.dataGeneral->Materials = true;
            }
            if (NumNames > 1) {
                if (Util::SameString(state.dataIPShortCut->cAlphaArgs(2), "CONSTRUCTIONS")) {
                    state.dataGeneral->Constructions = true;
                } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(2), "MATERIALS")) {
                    state.dataGeneral->Materials = true;
                }
            }
        }

        cCurrentModuleObject = "Output:EnergyManagementSystem";
        NumReports = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        for (RepNum = 1; RepNum <= NumReports; ++RepNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     RepNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumNames,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            state.dataGeneral->EMSoutput = true;

            AvailRpt CheckAvailRpt = static_cast<AvailRpt>(getEnumValue(AvailRptNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(1))));
            state.dataRuntimeLang->OutputEMSActuatorAvailSmall = (CheckAvailRpt == AvailRpt::NotByUniqueKeyNames);
            state.dataRuntimeLang->OutputEMSActuatorAvailFull = (CheckAvailRpt == AvailRpt::Verbose);

            CheckAvailRpt = static_cast<AvailRpt>(getEnumValue(AvailRptNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(2))));
            state.dataRuntimeLang->OutputEMSInternalVarsSmall = (CheckAvailRpt == AvailRpt::NotByUniqueKeyNames);
            state.dataRuntimeLang->OutputEMSInternalVarsFull = (CheckAvailRpt == AvailRpt::Verbose);

            ERLdebugOutputLevel CheckERLlevel =
                static_cast<ERLdebugOutputLevel>(getEnumValue(ERLdebugOutputLevelNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(3))));
            state.dataRuntimeLang->OutputEMSErrors =
                (CheckERLlevel == ERLdebugOutputLevel::ErrorsOnly || CheckERLlevel == ERLdebugOutputLevel::Verbose);
            state.dataRuntimeLang->OutputFullEMSTrace = (CheckERLlevel == ERLdebugOutputLevel::Verbose);
        }

        state.dataGeneral->GetReportInput = false;
    }

    // Process the Scan Request
    DoReport = false;

    ReportName rptName = static_cast<ReportName>(getEnumValue(ReportNamesUC, Util::makeUPPER(Util::makeUPPER(reportName))));
    switch (rptName) {
    case ReportName::Constructions: {
        if (present(ReportKey)) {
            if (Util::SameString(ReportKey(), "Constructions")) {
                DoReport = state.dataGeneral->Constructions;
            }
            if (Util::SameString(ReportKey(), "Materials")) {
                DoReport = state.dataGeneral->Materials;
            }
        }
    } break;
    case ReportName::Viewfactorinfo: {
        DoReport = state.dataGeneral->ViewFactorInfo;
        if (present(Option1)) {
            Option1 = state.dataGeneral->ViewRptOption1;
        }
    } break;
    case ReportName::Variabledictionary: {
        DoReport = state.dataGeneral->VarDict;
        if (present(Option1)) {
            Option1 = state.dataGeneral->VarDictOption1;
        }
        if (present(Option2)) {
            Option2 = state.dataGeneral->VarDictOption2;
        }
        //    CASE ('SCHEDULES')
        //     DoReport=SchRpt
        //      IF (PRESENT(Option1)) Option1=SchRptOption
    } break;
    case ReportName::Surfaces: {
        RptKey rptKey = static_cast<RptKey>(getEnumValue(RptKeyNamesUC, Util::makeUPPER(ReportKey())));
        switch (rptKey) { // Autodesk:OPTIONAL ReportKey used without PRESENT check
        case RptKey::Costinfo: {
            DoReport = state.dataGeneral->CostInfo;
        } break;
        case RptKey::DXF: {
            DoReport = state.dataGeneral->DXFReport;
            if (present(Option1)) {
                Option1 = state.dataGeneral->DXFOption1;
            }
            if (present(Option2)) {
                Option2 = state.dataGeneral->DXFOption2;
            }
        } break;
        case RptKey::DXFwireframe: {
            DoReport = state.dataGeneral->DXFWFReport;
            if (present(Option1)) {
                Option1 = state.dataGeneral->DXFWFOption1;
            }
            if (present(Option2)) {
                Option2 = state.dataGeneral->DXFWFOption2;
            }
        } break;
        case RptKey::VRML: {
            DoReport = state.dataGeneral->VRMLReport;
            if (present(Option1)) {
                Option1 = state.dataGeneral->VRMLOption1;
            }
            if (present(Option2)) {
                Option2 = state.dataGeneral->VRMLOption2;
            }
        } break;
        case RptKey::Vertices: {
            DoReport = state.dataGeneral->SurfVert;
        } break;
        case RptKey::Details: {
            DoReport = state.dataGeneral->SurfDet;
        } break;
        case RptKey::DetailsWithVertices: {
            DoReport = state.dataGeneral->SurfDetWVert;
        } break;
        case RptKey::Lines: {
            DoReport = state.dataGeneral->LineRpt;
            if (present(Option1)) {
                Option1 = state.dataGeneral->LineRptOption1;
            }
        } break;
        default:
            break;
        }
    } break;
    case ReportName::Energymanagementsystem: {
        DoReport = state.dataGeneral->EMSoutput;
    } break;
    default:
        break;
    }
}

void CheckCreatedZoneItemName(EnergyPlusData &state,
                              std::string_view const calledFrom,              // routine called from
                              std::string const &CurrentObject,               // object being parsed
                              std::string const &ZoneName,                    // Zone Name associated
                              std::string::size_type const MaxZoneNameLength, // maximum length of zonelist zone names
                              std::string const &ItemName,                    // Item name (People, Lights, etc object)
                              Array1_string const &ItemNames,                 // Item Names to check for duplication
                              int const NumItems,                             // Number of items in ItemNames array
                              std::string &ResultName,                        // Resultant name
                              bool &errFlag                                   // Error flag set to true if error found here.
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 2012

    // PURPOSE OF THIS SUBROUTINE:
    // This routine checks "global" objects (that is, ones with ZoneList used in the name
    // specification) along with a specific name for the current object for length and duplication
    // with previous objects of that class.

    errFlag = false;
    std::string::size_type const ItemNameLength = len(ItemName);
    std::string::size_type const ItemLength = len(ZoneName) + ItemNameLength;
    ResultName = ZoneName + ' ' + ItemName;
    bool TooLong = false;
    if (ItemLength > Constant::MaxNameLength) {
        ShowWarningError(state, fmt::format("{}{} Combination of ZoneList and Object Name generate a name too long.", calledFrom, CurrentObject));
        ShowContinueError(state, format("Object Name=\"{}\".", ItemName));
        ShowContinueError(state, format("ZoneList/Zone Name=\"{}\".", ZoneName));
        ShowContinueError(state,
                          format("Item length=[{}] > Maximum Length=[{}]. You may need to shorten the names.", ItemLength, Constant::MaxNameLength));
        ShowContinueError(state,
                          format("Shortening the Object Name by [{}] characters will assure uniqueness for this ZoneList.",
                                 MaxZoneNameLength + 1 + ItemNameLength - Constant::MaxNameLength));
        ShowContinueError(state, format("name that will be used (may be needed in reporting)=\"{}\".", ResultName));
        TooLong = true;
    }

    int FoundItem = Util::FindItemInList(ResultName, ItemNames, NumItems);

    if (FoundItem != 0) {
        ShowSevereError(state, fmt::format("{}{}=\"{}\", Duplicate Generated name encountered.", calledFrom, CurrentObject, ItemName));
        ShowContinueError(state, format("name=\"{}\" has already been generated or entered as {} item=[{}].", ResultName, CurrentObject, FoundItem));
        if (TooLong) {
            ShowContinueError(state, "Duplicate name likely caused by the previous \"too long\" warning.");
        }
        ResultName = "xxxxxxx";
        errFlag = true;
    }
}

bool isReportPeriodBeginning(EnergyPlusData &state, const int periodIdx)
{
    int currentDate;
    int reportStartDate = state.dataWeather->ReportPeriodInput(periodIdx).startJulianDate;
    int reportStartHour = state.dataWeather->ReportPeriodInput(periodIdx).startHour;
    if (state.dataWeather->ReportPeriodInput(periodIdx).startYear > 0) {
        currentDate = Weather::computeJulianDate(state.dataEnvrn->Year, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth);
    } else {
        currentDate = Weather::computeJulianDate(0, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth);
    }
    return (currentDate == reportStartDate && state.dataGlobal->HourOfDay == reportStartHour);
}

void findReportPeriodIdx(EnergyPlusData &state,
                         const Array1D<Weather::ReportPeriodData> &ReportPeriodInputData,
                         const int nReportPeriods,
                         Array1D_bool &inReportPeriodFlags)
{
    // return an array of flags, indicating whether the current time is in reporting period i
    int currentDate;
    for (int i = 1; i <= nReportPeriods; i++) {
        int reportStartDate = ReportPeriodInputData(i).startJulianDate;
        int reportStartHour = ReportPeriodInputData(i).startHour;
        int reportEndDate = ReportPeriodInputData(i).endJulianDate;
        int reportEndHour = ReportPeriodInputData(i).endHour;
        if (ReportPeriodInputData(i).startYear > 0) {
            currentDate = Weather::computeJulianDate(state.dataEnvrn->Year, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth);
        } else {
            currentDate = Weather::computeJulianDate(0, state.dataEnvrn->Month, state.dataEnvrn->DayOfMonth);
        }
        if (General::BetweenDateHoursLeftInclusive(
                currentDate, state.dataGlobal->HourOfDay, reportStartDate, reportStartHour, reportEndDate, reportEndHour)) {
            inReportPeriodFlags(i) = true;
        }
    }
}

Real64 rotAzmDiffDeg(Real64 AzmA, Real64 AzmB)
{
    // This function takes two (azimuth) angles in Degree(s),
    // and returns the rotational angle difference in Degree(s).

    Real64 diff = AzmB - AzmA;
    if (diff > 180.0) {
        diff = 360.0 - diff;
    } else if (diff < -180.0) {
        diff = 360.0 + diff;
    }
    return std::abs(diff);
}

} // namespace EnergyPlus::General
