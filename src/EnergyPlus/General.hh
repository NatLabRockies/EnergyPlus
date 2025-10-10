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

#ifndef General_hh_INCLUDED
#define General_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1A.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/Array2A.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Weather {
    enum class DateType;
    struct ReportPeriodData;
} // namespace Weather

namespace General {

    // A second version that does not require a payload -- use lambdas
    void SolveRoot(const EnergyPlusData &state,
                   Real64 Eps,   // required absolute accuracy
                   int MaxIte,   // maximum number of allowed iterations
                   int &Flag,    // integer storing exit status
                   Real64 &XRes, // value of x that solves f(x,Par) = 0
                   const std::function<Real64(Real64)> &f,
                   Real64 X_0,  // 1st bound of interval that contains the solution
                   Real64 X_1); // 2nd bound of interval that contains the solution

    void MovingAvg(Array1D<Real64> &DataIn, int NumItemsInAvg);

    void ProcessDateString(EnergyPlusData &state,
                           std::string const &String,
                           int &PMonth,
                           int &PDay,
                           int &PWeekDay,
                           Weather::DateType &DateType, // DateType found (-1=invalid, 1=month/day, 2=nth day in month, 3=last day in month)
                           bool &ErrorsFound,
                           ObjexxFCL::Optional_int PYear = _);

    void DetermineDateTokens(EnergyPlusData &state,
                             std::string const &String,
                             int &NumTokens,              // Number of tokens found in string
                             int &TokenDay,               // Value of numeric field found
                             int &TokenMonth,             // Value of Month field found (1=Jan, 2=Feb, etc)
                             int &TokenWeekday,           // Value of Weekday field found (1=Sunday, 2=Monday, etc), 0 if none
                             Weather::DateType &DateType, // DateType found (-1=invalid, 1=month/day, 2=nth day in month, 3=last day in month)
                             bool &ErrorsFound,           // Set to true if cannot process this string as a date
                             ObjexxFCL::Optional_int TokenYear = _ // Value of Year if one appears to be present and this argument is present
    );

    void ValidateMonthDay(EnergyPlusData &state,
                          std::string const &String, // REAL(r64) string being processed
                          int Day,
                          int Month,
                          bool &ErrorsFound);

    int OrdinalDay(int Month,        // Month, 1..12
                   int Day,          // Day of Month, not validated by month
                   int LeapYearValue // 1 if leap year indicated, 0 if not
    );

    void InvOrdinalDay(int Number, int &PMonth, int &PDay, int LeapYr);

    bool BetweenDateHoursLeftInclusive(int TestDate, int TestHour, int StartDate, int StartHour, int EndDate, int EndHour);

    bool BetweenDates(int TestDate,  // Date to test
                      int StartDate, // Start date in sequence
                      int EndDate    // End date in sequence
    );

    std::string CreateSysTimeIntervalString(EnergyPlusData &state);

    int nthDayOfWeekOfMonth(const EnergyPlusData &state,
                            int dayOfWeek,  // day of week (Sunday=1, Monday=2, ...)
                            int nthTime,    // nth time the day of the week occurs (first monday, third tuesday, ..)
                            int monthNumber // January = 1
    );

    Real64 SafeDivide(Real64 a, Real64 b);

    void Iterate(Real64 &ResultX, // ResultX is the final Iteration result passed back to the calling routine
                 Real64 Tol,      // Tolerance for Convergence
                 Real64 X0,       // Current value of X
                 Real64 Y0,       // Current value of the function Y(X)
                 Real64 &X1,      // First Previous values of X
                 Real64 &Y1,      // First Previous values of Y(X1)
                 int Iter,        // Number of iterations
                 int &Cnvg        // Convergence flag  Cnvg = 0:  Not converged
    );

    int FindNumberInList(int WhichNumber, Array1A_int ListOfItems, int NumItems);

    template <typename A> inline int FindNumberInList(int const WhichNumber, MArray1<A, int> const &ListOfItems, int const NumItems)
    {
        return FindNumberInList(WhichNumber, Array1D_int(ListOfItems), NumItems);
    }

    template <typename Container,
              class = typename std::enable_if<
                  !std::is_same<typename Container::value_type, std::string>::value>::type> // Container needs isize() and operator(i) and value_type
    inline int FindNumberInList(int const WhichNumber, Container const &ListOfItems, int Container::value_type::*num_p)
    {
        int FindNumberInList(0);
        for (int Count = 1, NumItems = ListOfItems.isize(); Count <= NumItems; ++Count) {
            if (WhichNumber == ListOfItems(Count).*num_p) {
                FindNumberInList = Count;
                break;
            }
        }
        return FindNumberInList;
    }

    void DecodeMonDayHrMin(int Item,   // word containing encoded month, day, hour, minute
                           int &Month, // month in integer format (1-12)
                           int &Day,   // day in integer format (1-31)
                           int &Hour,  // hour in integer format (1-24)
                           int &Minute // minute in integer format (0:59)
    );

    void EncodeMonDayHrMin(int &Item, // word containing encoded month, day, hour, minute
                           int Month, // month in integer format (1:12)
                           int Day,   // day in integer format (1:31)
                           int Hour,  // hour in integer format (1:24)
                           int Minute // minute in integer format (0:59)
    );

    std::string CreateTimeString(Real64 Time); // Time in seconds

    void ParseTime(Real64 Time,    // Time value in seconds
                   int &Hours,     // Number of hours
                   int &Minutes,   // Number of minutes < 60
                   Real64 &Seconds // Number of seconds < 60
    );

    void ScanForReports(EnergyPlusData &state,
                        std::string const &reportName,
                        bool &DoReport,
                        ObjexxFCL::Optional_string_const ReportKey = _,
                        ObjexxFCL::Optional_string Option1 = _,
                        ObjexxFCL::Optional_string Option2 = _);

    void CheckCreatedZoneItemName(EnergyPlusData &state,
                                  std::string_view calledFrom,              // routine called from
                                  std::string const &CurrentObject,         // object being parsed
                                  std::string const &ZoneName,              // Zone Name associated
                                  std::string::size_type MaxZoneNameLength, // maximum length of zonelist zone names
                                  std::string const &ItemName,              // Item name (People, Lights, etc object)
                                  Array1_string const &ItemNames,           // Item Names to check for duplication
                                  int NumItems,                             // Number of items in ItemNames array
                                  std::string &ResultName,                  // Resultant name
                                  bool &errFlag                             // Error flag set to true if error found here.
    );

    template <typename Container, class = typename std::enable_if<!std::is_same<typename Container::value_type, std::string>::value>::type>
    inline void CheckCreatedZoneItemName(EnergyPlusData &state,
                                         std::string_view const calledFrom,              // routine called from
                                         std::string const &CurrentObject,               // object being parsed
                                         std::string const &ZoneName,                    // Zone Name associated
                                         std::string::size_type const MaxZoneNameLength, // maximum length of zonelist zone names
                                         std::string const &ItemName,                    // Item name (People, Lights, etc object)
                                         Container const &Items,                         // Items to check for duplication Names
                                         int const NumItems,                             // Number of items in ItemNames array
                                         std::string &ResultName,                        // Resultant name
                                         bool &errFlag                                   // Error flag set to true if error found here.
    )
    {
        Array1D_string ItemNames(Items.size());
        for (std::size_t i = 0, e = Items.size(); i < e; ++i) {
            ItemNames[i] = Items[i].Name;
        }
        CheckCreatedZoneItemName(state, calledFrom, CurrentObject, ZoneName, MaxZoneNameLength, ItemName, ItemNames, NumItems, ResultName, errFlag);
    }

    bool isReportPeriodBeginning(EnergyPlusData &state, int periodIdx);

    void findReportPeriodIdx(EnergyPlusData &state,
                             const Array1D<Weather::ReportPeriodData> &ReportPeriodInputData,
                             int nReportPeriods,
                             Array1D_bool &inReportPeriodFlags);

    Real64 rotAzmDiffDeg(Real64 AzmA, Real64 AzmB);

    inline Real64 epexp(const Real64 numerator, const Real64 denominator)
    {
        if (denominator == 0.0) {
            return 0.0;
        } else {
            return std::exp(numerator / denominator);
        }
    }
} // namespace General

constexpr Real64 Interp(Real64 const Lower, Real64 const Upper, Real64 const InterpFac)
{
    return Lower + InterpFac * (Upper - Lower);
}

struct InterpCoeffs
{
    Real64 x1;
    Real64 x2;
};

inline void GetInterpCoeffs(Real64 X, Real64 X1, Real64 X2, InterpCoeffs &c)
{
    c.x1 = (X - X1) / (X2 - X1);
    c.x2 = (X2 - X) / (X2 - X1);
}

inline Real64 Interp2(Real64 Fx1, Real64 Fx2, InterpCoeffs const &c)
{
    return c.x1 * Fx1 + c.x2 * Fx2;
}

// Disaggregated implementation of bilinear interpolation so that coefficients can be used with multiple variables
struct BilinearInterpCoeffs
{
    Real64 denom;
    Real64 x1y1;
    Real64 x1y2;
    Real64 x2y1;
    Real64 x2y2;
};

inline void GetBilinearInterpCoeffs(
    Real64 const X, Real64 const Y, Real64 const X1, Real64 const X2, Real64 const Y1, Real64 const Y2, BilinearInterpCoeffs &coeffs)
{
    if (X1 == X2 && Y1 == Y2) {
        coeffs.denom = coeffs.x1y1 = 1.0;
        coeffs.x1y2 = coeffs.x2y1 = coeffs.x2y2 = 0.0;
    } else if (X1 == X2) {
        coeffs.denom = (Y2 - Y1);
        coeffs.x1y1 = (Y2 - Y);
        coeffs.x1y2 = (Y - Y1);
        coeffs.x2y1 = coeffs.x2y2 = 0.0;
    } else if (Y1 == Y2) {
        coeffs.denom = (X2 - X1);
        coeffs.x1y1 = (X2 - X);
        coeffs.x2y1 = (X - X1);
        coeffs.x1y2 = coeffs.x2y2 = 0.0;
    } else {
        coeffs.denom = (X2 - X1) * (Y2 - Y1);
        coeffs.x1y1 = (X2 - X) * (Y2 - Y);
        coeffs.x2y1 = (X - X1) * (Y2 - Y);
        coeffs.x1y2 = (X2 - X) * (Y - Y1);
        coeffs.x2y2 = (X - X1) * (Y - Y1);
    }
}

inline Real64 BilinearInterp(Real64 const Fx1y1, Real64 const Fx1y2, Real64 const Fx2y1, Real64 const Fx2y2, BilinearInterpCoeffs const &coeffs)
{
    return (coeffs.x1y1 * Fx1y1 + coeffs.x2y1 * Fx2y1 + coeffs.x1y2 * Fx1y2 + coeffs.x2y2 * Fx2y2) / coeffs.denom;
}

struct GeneralData : BaseGlobalStruct
{
    bool GetReportInput = true;
    bool SurfVert = false;
    bool SurfDet = false;
    bool SurfDetWVert = false;
    bool DXFReport = false;
    bool DXFWFReport = false;
    bool VRMLReport = false;
    bool CostInfo = false;
    bool ViewFactorInfo = false;
    bool Constructions = false;
    bool Materials = false;
    bool LineRpt = false;
    bool VarDict = false;
    bool EMSoutput = false;
    Real64 XNext = 0.0; // used in root finder
    std::string DXFOption1;
    std::string DXFOption2;
    std::string DXFWFOption1;
    std::string DXFWFOption2;
    std::string VRMLOption1;
    std::string VRMLOption2;
    std::string ViewRptOption1;
    std::string LineRptOption1;
    std::string VarDictOption1;
    std::string VarDictOption2;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) GeneralData();
    }
};

} // namespace EnergyPlus

#endif
