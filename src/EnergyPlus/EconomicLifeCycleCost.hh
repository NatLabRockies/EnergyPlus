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

#ifndef EconomicLifeCycleCost_hh_INCLUDED
#define EconomicLifeCycleCost_hh_INCLUDED

// C++ Headers
#include <map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace EconomicLifeCycleCost {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS:
    enum class DiscConv
    {
        Invalid = -1,
        BeginOfYear,
        MidYear,
        EndOfYear,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(DiscConv::Num)> DiscConvNamesUC{"BEGINNINGOFYEAR", "MIDYEAR", "ENDOFYEAR"};

    constexpr std::array<std::string_view, static_cast<int>(DiscConv::Num)> DiscConvNames{"BeginningOfYear", "MidYear", "EndOfYear"};

    enum class InflAppr
    {
        Invalid = -1,
        ConstantDollar,
        CurrentDollar,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(InflAppr::Num)> InflApprNamesUC{"CONSTANTDOLLAR", "CURRENTDOLLAR"};

    constexpr std::array<std::string_view, static_cast<int>(InflAppr::Num)> InflApprNames{"ConstantDollar", "CurrentDollar"};

    enum class DeprMethod
    {
        Invalid = -1,
        MACRS3,
        MACRS5,
        MACRS7,
        MACRS10,
        MACRS15,
        MACRS20,
        Straight27,
        Straight31,
        Straight39,
        Straight40,
        None,
        Num
    };

    int constexpr SizeDepr(41);

    constexpr std::array<std::array<Real64, SizeDepr>, static_cast<int>(DeprMethod::Num)> DepreciationPercentTable{
        {{33.33, 44.45, 14.81, 7.41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
         {20.0, 32.0, 19.2, 11.52, 11.52, 5.76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0,    0,    0,    0,     0,     0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
         {14.29, 24.49, 17.49, 12.49, 8.93, 8.92, 8.93, 4.46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0,     0,     0,     0,     0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
         {10.0, 18.0, 14.4, 11.52, 9.22, 7.37, 6.55, 6.55, 6.56, 6.55, 3.28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0,    0,    0,    0,     0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0},
         {5.0, 9.5, 8.55, 7.7, 6.93, 6.23, 5.9, 5.9, 5.91, 5.9, 5.91, 5.9, 5.91, 5.9, 5.91, 2.95, 0, 0, 0, 0, 0,
          0,   0,   0,    0,   0,    0,    0,   0,   0,    0,   0,    0,   0,    0,   0,    0,    0, 0, 0, 0},
         {3.75,  7.219, 6.677, 6.177, 5.713, 5.285, 4.888, 4.522, 4.462, 4.461, 4.462, 4.461, 4.462, 4.461,
          4.462, 4.461, 4.462, 4.461, 4.462, 4.461, 2.231, 0,     0,     0,     0,     0,     0,     0,
          0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
         {1.97,  3.636, 3.636, 3.636, 3.636, 3.636, 3.636, 3.636, 3.636, 3.637, 3.636, 3.637, 3.636, 3.637,
          3.636, 3.637, 3.636, 3.637, 3.636, 3.637, 3.636, 3.637, 3.636, 3.637, 3.636, 3.637, 3.636, 3.485,
          0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
         {1.72,  3.175, 3.175, 3.175, 3.175, 3.175, 3.175, 3.174, 3.175, 3.174, 3.175, 3.174, 3.175, 3.174,
          3.175, 3.174, 3.175, 3.174, 3.175, 3.174, 3.175, 3.174, 3.175, 3.174, 3.175, 3.174, 3.175, 3.174,
          3.175, 3.174, 3.175, 3.042, 0,     0,     0,     0,     0,     0,     0,     0,     0},
         {1.391, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564,
          2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564,
          2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 2.564, 1.177, 0},
         {1.354, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5,  2.5,
          2.5,   2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 1.146}}};

    constexpr std::array<std::string_view, static_cast<int>(DeprMethod::Num)> DeprMethodNamesUC{
        "MODIFIEDACCELERATEDCOSTRECOVERYSYSTEM-3YEAR",
        "MODIFIEDACCELERATEDCOSTRECOVERYSYSTEM-5YEAR",
        "MODIFIEDACCELERATEDCOSTRECOVERYSYSTEM-7YEAR",
        "MODIFIEDACCELERATEDCOSTRECOVERYSYSTEM-10YEAR",
        "MODIFIEDACCELERATEDCOSTRECOVERYSYSTEM-15YEAR",
        "MODIFIEDACCELERATEDCOSTRECOVERYSYSTEM-20YEAR",
        "STRAIGHTLINE-27YEAR",
        "STRAIGHTLINE-31YEAR",
        "STRAIGHTLINE-39YEAR",
        "STRAIGHTLINE-40YEAR",
        "NONE",
    };

    constexpr std::array<std::string_view, static_cast<int>(DeprMethod::Num)> DeprMethodNames{
        "ModifiedAcceleratedCostRecoverySystem-3year",
        "ModifiedAcceleratedCostRecoverySystem-5year",
        "ModifiedAcceleratedCostRecoverySystem-7year",
        "ModifiedAcceleratedCostRecoverySystem-10year",
        "ModifiedAcceleratedCostRecoverySystem-15year",
        "ModifiedAcceleratedCostRecoverySystem-20year",
        "StraightLine-27year",
        "StraightLine-31year",
        "StraightLine-39year",
        "StraightLine-40year",
        "None",
    };

    enum CostCategory
    {
        Invalid = -1,
        Maintenance,
        Repair,
        Operation,
        Replacement,
        MinorOverhaul,
        MajorOverhaul,
        OtherOperational,
        Water,
        Energy,
        TotOper,
        Construction,
        Salvage,
        OtherCapital,
        TotCaptl,
        TotEnergy,
        TotGrand,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(CostCategory::Num)> CostCategoryNames{"Maintenance",
                                                                                                  "Repair",
                                                                                                  "Operation",
                                                                                                  "Replacement",
                                                                                                  "Minor Overhaul",
                                                                                                  "Major Overhaul",
                                                                                                  "Other Operational",
                                                                                                  "Water",
                                                                                                  "Energy",
                                                                                                  "Total Operation",
                                                                                                  "Construction",
                                                                                                  "Salvage",
                                                                                                  "Other Capital",
                                                                                                  "Total Capital",
                                                                                                  "Total Energy",
                                                                                                  "Grand Total"};

    constexpr std::array<std::string_view, static_cast<int>(CostCategory::Num)> CostCategoryNamesNoSpace{"Maintenance",
                                                                                                         "Repair",
                                                                                                         "Operation",
                                                                                                         "Replacement",
                                                                                                         "MinorOverhaul",
                                                                                                         "MajorOverhaul",
                                                                                                         "OtherOperational",
                                                                                                         "Water",
                                                                                                         "Energy",
                                                                                                         "TotalOperational",
                                                                                                         "Construction",
                                                                                                         "Salvage",
                                                                                                         "OtherCapital",
                                                                                                         "TotalCapital",
                                                                                                         "TotalEnergy",
                                                                                                         "GrandTotal"};

    constexpr std::array<std::string_view, static_cast<int>(CostCategory::Num)> CostCategoryNamesUC{"MAINTENANCE",
                                                                                                    "REPAIR",
                                                                                                    "OPERATION",
                                                                                                    "REPLACEMENT",
                                                                                                    "MINOR OVERHAUL",
                                                                                                    "MAJOR OVERHAUL",
                                                                                                    "OTHER OPERATIONAL",
                                                                                                    "WATER",
                                                                                                    "ENERGY",
                                                                                                    "TOTAL OPERATIONAL",
                                                                                                    "CONSTRUCTION",
                                                                                                    "SALVAGE",
                                                                                                    "OTHER CAPITAL",
                                                                                                    "TOTAL CAPITAL",
                                                                                                    "TOTAL ENERGY",
                                                                                                    "GRAND TOTAL"};
    constexpr std::array<std::string_view, static_cast<int>(CostCategory::Num)> CostCategoryNamesUCNoSpace{"MAINTENANCE",
                                                                                                           "REPAIR",
                                                                                                           "OPERATION",
                                                                                                           "REPLACEMENT",
                                                                                                           "MINOROVERHAUL",
                                                                                                           "MAJOROVERHAUL",
                                                                                                           "OTHEROPERATIONAL",
                                                                                                           "WATER",
                                                                                                           "ENERGY",
                                                                                                           "TOTALOPERATIONAL",
                                                                                                           "CONSTRUCTION",
                                                                                                           "SALVAGE",
                                                                                                           "OTHERCAPITAL", // No space
                                                                                                           "TOTALCAPITAL",
                                                                                                           "TOTALENERGY",
                                                                                                           "GRANDTOTAL"};

    constexpr std::string_view Total{"Total"};
    constexpr std::string_view TotalUC{"TOTAL"};

    // The NIST supplement includes UPV* factors for
    //   Electricity
    //   Natural gas
    //   Distillate oil - FuelOilNo1
    //   Liquified petroleum gas - Propane
    //   Residual oil - FuelOilNo2
    //   Coal

    enum class StartCosts
    {
        Invalid = -1,
        ServicePeriod,
        BasePeriod,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(StartCosts::Num)> StartCostNamesUC{"SERVICEPERIOD", "BASEPERIOD"};

    enum class SourceKindType
    {
        Invalid = -1,
        Recurring,
        Nonrecurring,
        Resource,
        Sum,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(SourceKindType::Num)> SourceKindTypeNames{"Recurring", "Nonrecurring"};

    enum class ResourceCostCategory
    {
        Invalid = -1,
        Water,
        Energy,
        Num
    };
    constexpr std::array<std::string_view, static_cast<int>(ResourceCostCategory::Num)> ResourceCostCategoryNames{"Water Cost", "Energy Cost"};

    enum class PrValKind
    {
        Invalid = -1,
        Energy,
        NonEnergy,
        NotComputed,
        Num
    };

    // Types

    struct RecurringCostsType
    {
        // Members
        std::string name;            // Name
        std::string lineItem;        // Line Item
        CostCategory category;       // Category
        Real64 cost;                 // Cost
        StartCosts startOfCosts;     // Start of Costs
        int yearsFromStart;          // Years from Start 0 - 100
        int monthsFromStart;         // Months from Start 0 - 11
        int totalMonthsFromStart;    // Total months (12 x years) + months
        int repeatPeriodYears;       // Repeat Period Years 1 - 100
        int repeatPeriodMonths;      // Repeat Period Months 0 - 11
        int totalRepeatPeriodMonths; // Total months (12 x years) + months
        Real64 annualEscalationRate; // Annual escalation rate

        // Default Constructor
        RecurringCostsType()
            : category(CostCategory::Maintenance), cost(0.0), startOfCosts(StartCosts::ServicePeriod), yearsFromStart(0), monthsFromStart(0),
              totalMonthsFromStart(0), repeatPeriodYears(0), repeatPeriodMonths(0), totalRepeatPeriodMonths(0), annualEscalationRate(0.0)
        {
        }
    };

    struct NonrecurringCostType
    {
        // Members
        std::string name;         // Name
        std::string lineItem;     // Line Item
        CostCategory category;    // Category
        Real64 cost;              // Cost
        StartCosts startOfCosts;  // Start of Costs
        int yearsFromStart;       // Years from Start 0 - 100
        int monthsFromStart;      // Months from Start 0 - 11
        int totalMonthsFromStart; // Total months (12 x years) + months

        // Default Constructor
        NonrecurringCostType()
            : category(CostCategory::Construction), cost(0.0), startOfCosts(StartCosts::ServicePeriod), yearsFromStart(0), monthsFromStart(0),
              totalMonthsFromStart(0)
        {
        }
    };

    struct UsePriceEscalationType
    {
        // Members
        std::string name; // Name
        Constant::eResource resource =
            Constant::eResource::Invalid; // resource like electricity or natural gas (uses definitions from DataGlobalConstants)
        int escalationStartYear = 0;      // Escalation Start Year 1900-2100
        int escalationStartMonth = 0;     // Escalation Start Month 1 to 12
        Array1D<Real64> Escalation;       // Escalation by year, first year is baseDateYear
        // last year is baseDateYear + lengthStudyYears - 1
    };

    struct UseAdjustmentType
    {
        // Members
        std::string name; // Name
        Constant::eResource resource =
            Constant::eResource::Invalid; // resource like electricity or natural gas (uses definitions from DataGlobalConstants)
        Array1D<Real64> Adjustment;       // Adjustment by year, first year is baseDateYear
        // last year is baseDateYear + lengthStudyYears - 1
    };

    struct CashFlowType
    {
        // Members
        std::string name;             // Name - just for labeling output - use Category for aggregation
        SourceKindType SourceKind;    // 1=recurring, 2=nonrecurring, 3=resource
        Constant::eResource Resource; // resource like electricity or natural gas (uses definitions from DataGlobalConstants)
        CostCategory Category;        // uses "costCat" constants above
        Array1D<Real64> mnAmount;     // cashflow dollar amount by month, first year is baseDateYear
        // last year is baseDateYear + lengthStudyYears - 1
        Array1D<Real64> yrAmount;  // cashflow dollar amount by year, first year is baseDateYear
        PrValKind pvKind;          // kind of present value 1=energy, 2=non-energy,3=not computed but summed
        Real64 presentValue;       // total present value for cashflow
        Real64 orginalCost;        // original cost from recurring, non-recurring or energy cost
        Array1D<Real64> yrPresVal; // present value by year, first year is baseDateYear

        // Default Constructor
        CashFlowType()
            : SourceKind(SourceKindType::Invalid), Resource(Constant::eResource::Invalid), Category(CostCategory::Invalid),
              pvKind(PrValKind::Invalid), presentValue(0.), orginalCost(0.)
        {
        }
    };

    // Functions

    void GetInputForLifeCycleCost(EnergyPlusData &state);

    void ComputeLifeCycleCostAndReport(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    GET INPUT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void GetInputLifeCycleCostParameters(EnergyPlusData &state);

    void GetInputLifeCycleCostRecurringCosts(EnergyPlusData &state);

    void GetInputLifeCycleCostNonrecurringCost(EnergyPlusData &state);

    void GetInputLifeCycleCostUsePriceEscalation(EnergyPlusData &state);

    void GetInputLifeCycleCostUseAdjustment(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    COMPUTATION ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void ExpressAsCashFlows(EnergyPlusData &state);

    void ComputeEscalatedEnergyCosts(EnergyPlusData &state);

    void ComputePresentValue(EnergyPlusData &state);

    void ComputeTaxAndDepreciation(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    OUTPUT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void WriteTabularLifeCycleCostReport(EnergyPlusData &state);

} // namespace EconomicLifeCycleCost

struct EconomicLifeCycleCostData : BaseGlobalStruct
{
    // related to LifeCycleCost:Parameters
    bool LCCparamPresent = false; // If a LifeCycleCost:Parameters object is present
    std::string LCCname;          // Name
    EconomicLifeCycleCost::DiscConv discountConvention = EconomicLifeCycleCost::DiscConv::EndOfYear;     // Discounting Convention
    EconomicLifeCycleCost::InflAppr inflationApproach = EconomicLifeCycleCost::InflAppr::ConstantDollar; // Inflation Approach
    Real64 realDiscountRate = 0.0;                                                                       // Real Discount Rate
    Real64 nominalDiscountRate = 0.0;                                                                    // Nominal Discount Rate
    Real64 inflation = 0.0;                                                                              // Inflation
    int baseDateMonth = 0;                                                                               // Base Date Month (1=Jan, 12=Dec)
    int baseDateYear = 0;                                                                                // Base Date Year  1900-2100
    int serviceDateMonth = 0;                                                                            // Service Date Month (1=Jan, 12=Dec)
    int serviceDateYear = 0;                                                                             // Service Date Year 1900-2100
    int lengthStudyYears = 0;                                                                            // Length of Study Period in Years
    int lengthStudyTotalMonths = 0; // Length of Study expressed in months (years x 12)
    Real64 taxRate = 0.0;           // Tax rate
    EconomicLifeCycleCost::DeprMethod depreciationMethod = EconomicLifeCycleCost::DeprMethod::None; // Depreciation Method
    // derived
    int lastDateYear = 0; // Last Date Year (base date year + length of study period in years)
    int numRecurringCosts = 0;
    int numNonrecurringCost = 0;
    int numUsePriceEscalation = 0;
    int numUseAdjustment = 0;
    int numCashFlow = 0;
    int numResourcesUsed = 0;
    bool GetInput_GetLifeCycleCostInput = true;

    // from former statistics in GetInputLifeCycleCostUsePriceEscalation()
    int UsePriceEscalation_escStartYear = 0;
    int UsePriceEscalation_escNumYears = 0;
    int UsePriceEscalation_escEndYear = 0;
    int UsePriceEscalation_earlierEndYear = 0;
    int UsePriceEscalation_laterStartYear = 0;
    int UsePriceEscalation_curEsc = 0;
    int UsePriceEscalation_curFld = 0;

    // from former statistics in ExpressAsCashFlows
    int ExpressAsCashFlows_baseMonths1900 = 0;    // number of months since 1900 for base period
    int ExpressAsCashFlows_serviceMonths1900 = 0; // number of months since 1900 for service period

    // present value factors
    Array1D<Real64> SPV;
    std::map<int, std::array<Real64, static_cast<int>(Constant::eResource::Num)>> energySPV; // yearly equivalent to FEMP UPV* values

    // arrays related to computing after tax cashflow and present value
    Array1D<Real64> DepreciatedCapital;
    Array1D<Real64> TaxableIncome;
    Array1D<Real64> Taxes;
    Array1D<Real64> AfterTaxCashFlow;
    Array1D<Real64> AfterTaxPresentValue;

    // arrays related to escalated energy costs
    Array1D<Real64> EscalatedTotEnergy;
    std::map<int, std::array<Real64, static_cast<int>(Constant::eResource::Num)>> EscalatedEnergy;

    std::vector<EconomicLifeCycleCost::RecurringCostsType> RecurringCosts;
    std::vector<EconomicLifeCycleCost::NonrecurringCostType> NonrecurringCost;
    EPVector<EconomicLifeCycleCost::UsePriceEscalationType> UsePriceEscalation;
    EPVector<EconomicLifeCycleCost::UseAdjustmentType> UseAdjustment;
    std::vector<EconomicLifeCycleCost::CashFlowType> CashFlow;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->LCCparamPresent = false;
        this->LCCname.clear();
        this->discountConvention = EconomicLifeCycleCost::DiscConv::EndOfYear;
        this->inflationApproach = EconomicLifeCycleCost::InflAppr::ConstantDollar;
        this->realDiscountRate = 0.0;
        this->nominalDiscountRate = 0.0;
        this->inflation = 0.0;
        this->baseDateMonth = 0;
        this->baseDateYear = 0;
        this->serviceDateMonth = 0;
        this->serviceDateYear = 0;
        this->lengthStudyYears = 0;
        this->lengthStudyTotalMonths = 0;
        this->taxRate = 0.0;
        this->depreciationMethod = EconomicLifeCycleCost::DeprMethod::None;
        this->lastDateYear = 0;
        this->numRecurringCosts = 0;
        this->numNonrecurringCost = 0;
        this->numUsePriceEscalation = 0;
        this->numUseAdjustment = 0;
        this->numCashFlow = 0;
        this->numResourcesUsed = 0;
        this->GetInput_GetLifeCycleCostInput = true;
        this->UsePriceEscalation_escStartYear = 0;
        this->UsePriceEscalation_escNumYears = 0;
        this->UsePriceEscalation_escEndYear = 0;
        this->UsePriceEscalation_earlierEndYear = 0;
        this->UsePriceEscalation_laterStartYear = 0;
        this->UsePriceEscalation_curEsc = 0;
        this->UsePriceEscalation_curFld = 0;
        this->ExpressAsCashFlows_baseMonths1900 = 0;
        this->ExpressAsCashFlows_serviceMonths1900 = 0;
        this->SPV.deallocate();
        this->energySPV.clear();
        this->DepreciatedCapital.deallocate();
        this->TaxableIncome.deallocate();
        this->Taxes.deallocate();
        this->AfterTaxCashFlow.deallocate();
        this->AfterTaxPresentValue.deallocate();
        this->EscalatedTotEnergy.deallocate();
        this->EscalatedEnergy.clear();
        this->RecurringCosts.clear();
        this->NonrecurringCost.clear();
        this->UsePriceEscalation.deallocate();
        this->UseAdjustment.deallocate();
        this->CashFlow.clear();
    }
};

} // namespace EnergyPlus

#endif
