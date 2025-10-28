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

#ifndef EconomicTariff_hh_INCLUDED
#define EconomicTariff_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1A.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace EconomicTariff {

    enum class ObjType
    {
        Invalid = -1,
        Tariff,
        Qualify,
        ChargeSimple,
        ChargeBlock,
        Ratchet,
        Variable,
        Computation,
        Category,
        Native,
        AssignCompute,
        Num
    };

    enum class EconConv
    {
        Invalid = -1,
        USERDEF,
        KWH,
        THERM,
        MMBTU, // million btu
        MJ,
        KBTU,
        MCF, // thousand cubic feet
        CCF, // hundred cubic feet
        M3,  // cubic meter
        GAL,
        KGAL, // thousand gallons
        Num
    };

    enum class DemandWindow
    {
        Invalid = -1,
        Quarter,
        Half,
        Hour,
        Day,
        Week,
        Num
    };

    enum class BuySell
    {
        Invalid = -1,
        BuyFromUtility,
        SellToUtility,
        NetMetering,
        Num
    };

    // For several different objects that reference seasons
    enum class Season
    {
        Invalid = -1,
        Unused, // Can't use 0 because these things appear in schedules
        Winter,
        Spring,
        Summer,
        Fall,
        Annual,
        Monthly,
        Num
    };

    // For AssignVariablePt
    int constexpr varIsArgument(1); // if used as a value or on right side of expression
    int constexpr varIsAssigned(2); // if variable is assigned to or on left side of expression

    // For ComputeSteps
    // All are negative because all variables are positive
    enum class Op
    {
        Invalid = -1,
        SUM,
        MULTIPLY,
        SUBTRACT,
        DIVIDE,
        ABSOLUTEVALUE,
        INTEGER,
        SIGN,
        ROUND,
        MAXIMUM,
        MINIMUM,
        EXCEEDS,
        ANNUALMINIMUM,
        ANNUALMAXIMUM,
        ANNUALSUM,
        ANNUALAVERAGE,
        ANNUALOR,
        ANNUALAND,
        ANNUALMAXIMUMZERO,
        ANNUALMINIMUMZERO,
        IF,
        GREATERTHAN,
        GREATEREQUAL,
        LESSTHAN,
        LESSEQUAL,
        EQUAL,
        NOTEQUAL,
        AND,
        OR,
        NOT,
        ADD,
        NOOP,
        Num
    };

    // not predefined variable (user defined name - many variables and all objects)
    // used in econvar%specific
    int constexpr varUserDefined(-1);
    int constexpr varNotYetDefined(-2);

    enum class Cat
    {
        Invalid = -1,
        EnergyCharges,
        DemandCharges,
        ServiceCharges,
        Basis,
        Adjustment,
        Surcharge,
        Subtotal,
        Taxes,
        Total,
        NotIncluded,
        Num
    };

    enum class Native
    {
        Invalid = -1,
        TotalEnergy,
        TotalDemand,
        PeakEnergy,
        PeakDemand,
        ShoulderEnergy,
        ShoulderDemand,
        OffPeakEnergy,
        OffPeakDemand,
        MidPeakEnergy,
        MidPeakDemand,
        PeakExceedsOffPeak,
        OffPeakExceedsPeak,
        PeakExceedsMidPeak,
        MidPeakExceedsPeak,
        PeakExceedsShoulder,
        ShoulderExceedsPeak,
        IsWinter,
        IsNotWinter,
        IsSpring,
        IsNotSpring,
        IsSummer,
        IsNotSummer,
        IsAutumn,
        IsNotAutumn,
        PeakAndShoulderEnergy,
        PeakAndShoulderDemand,
        PeakAndMidPeakEnergy,
        PeakAndMidPeakDemand,
        ShoulderAndOffPeakEnergy,
        ShoulderAndOffPeakDemand,
        PeakAndOffPeakEnergy,
        PeakAndOffPeakDemand,
        RealTimePriceCosts,
        AboveCustomerBaseCosts,
        BelowCustomerBaseCosts,
        AboveCustomerBaseEnergy,
        BelowCustomerBaseEnergy,
        Num
    };

    int constexpr NumMonths(12);
    int constexpr maxNumBlk(15);

    enum class Period
    {
        Invalid = -1,
        Unused, // Can't use 0 because these things appear in schedules
        Peak,
        Shoulder,
        OffPeak,
        MidPeak,
        Num
    };

    enum class MeterType
    {
        Invalid = -1,
        ElecSimple,
        ElecProduced,
        ElecPurchased,
        ElecSurplusSold,
        ElecNet,
        Water,
        Gas,
        Other,
        Num
    };

    enum class VarUnitType
    {
        Invalid = -1,
        Energy,
        Demand,
        Dimensionless,
        Currency,
        Num
    };

    // Types

    struct EconVarType
    {
        // Members
        std::string name;       // name of the economics object or variable
        int tariffIndx;         // index of the tariff name in the tariff array
        ObjType kindOfObj;      // enumerated list for the kind of economics object
        int index;              // pointer to item in specific array
        Array1D<Real64> values; // values
        // the following items are not part of the object description
        bool isArgument; // flag if the variable is ever used as an argument (value needed)
        bool isAssigned; // flag if the variable is ever assigned to
        int specific;    // the specific type of variable - see enumerated lists
        // the following items are used in determinging the dependency relationship of variables
        // and consist of an operator and a list of variables.
        int cntMeDependOn;         // count of items in depend this line depends upon
        Op Operator = Op::Invalid; // operator used in equation (usually opSUM or opNOOP)
        int firstOperand;          // first item in the operand array
        int lastOperand;           // last item in the operand array
        bool activeNow;            // flag if the econVar is used in the current tariff
        bool isEvaluated;          // flag if the economics object that results in this variable
        // has already been evaulated
        bool isReported;                                // flag if the econVar has been reported in the output file
        VarUnitType varUnitType = VarUnitType::Invalid; // variable unit type: energy, demand, dimensionless, currency

        // Default Constructor
        EconVarType()
            : tariffIndx(0), kindOfObj(ObjType::Invalid), index(0), values(NumMonths, 0.0), isArgument(false), isAssigned(false), specific(0),
              cntMeDependOn(0), firstOperand(0), lastOperand(0), activeNow(false), isEvaluated(false), isReported(false)
        {
        }
    };

    struct TariffType
    {
        // Members
        std::string tariffName;                                      // name of the tariff
        std::string reportMeter;                                     // name of the report meter
        int reportMeterIndx;                                         // index of the report meter
        MeterType kindMtr = MeterType::Invalid;                      // kind of electric meter - see enumerated list above, 0 is not electric
        Constant::eResource resource = Constant::eResource::Invalid; // based on list of DataGlobalConstants
        EconConv convChoice;                                         // enumerated choice index of the conversion factor
        Real64 energyConv;                                           // energy conversion factor
        Real64 demandConv;                                           // demand conversion factor
        Sched::Schedule *periodSched = nullptr;                      // period schedule
        Sched::Schedule *seasonSched = nullptr;                      // season schedule
        Sched::Schedule *monthSched = nullptr;                       // month schedule
        DemandWindow demandWindow;                                   // enumerated list of the kind of demand window
        Real64 demWinTime;                                           // length of time for the demand window
        Real64 monthChgVal;                                          // monthly charge value
        int monthChgPt;                                              // pointer to a variable that contains the monthly charge
        // if 0 then use monthChgVal
        Real64 minMonthChgVal; // minimum monthly charge value
        int minMonthChgPt;     // pointer to a variable that contains the minimum monthly charge
        // if 0 then use minMonthChgVal
        Sched::Schedule *chargeSched = nullptr;  // index to the charge schedule
        Sched::Schedule *baseUseSched = nullptr; // index to the baseline use schedule
        std::string groupName;                   // name of the group
        std::string monetaryUnit;                // text string representing monetary unit, usually $
        BuySell buyOrSell;                       // enumerated choice index of the buy or sell options
        // index to the first and last category variables
        int firstCategory; // first category referenced
        int lastCategory;  // last category referenced
        // pointers to econ variables for categories
        std::array<int, (int)Cat::Num> cats;
        // index to the first and last native variables (energies and demands from the simulation)
        int firstNative;
        int lastNative;
        // native variables (based on energy and demands from the simulation)
        std::array<int, (int)Native::Num> natives = {0};
        // arrays for holding gathered values
        Array1D<std::array<Real64, (int)Period::Num>> gatherEnergy;
        Array1D<std::array<Real64, (int)Period::Num>> gatherDemand;
        Real64 collectTime;
        Real64 collectEnergy;
        // arryas for holding real time pricing gathered values
        Array1D<Real64> RTPcost;
        Array1D<Real64> RTPaboveBaseCost;
        Array1D<Real64> RTPbelowBaseCost;
        Array1D<Real64> RTPaboveBaseEnergy;
        Array1D<Real64> RTPbelowBaseEnergy;
        Array1D<Season> seasonForMonth;
        // overall qualification of the rate
        bool isQualified;
        int ptDisqualifier;
        // overall selection and annual cost
        bool isSelected;
        Real64 totalAnnualCost;
        Real64 totalAnnualEnergy;

        // Default Constructor
        TariffType()
            : reportMeterIndx(0), convChoice(EconConv::USERDEF), energyConv(0.0), demandConv(0.0), demandWindow(DemandWindow::Invalid),
              demWinTime(0.0), monthChgVal(0.0), monthChgPt(0), minMonthChgVal(0.0), minMonthChgPt(0), firstCategory(0), lastCategory(0),
              firstNative(0), lastNative(0), gatherEnergy(NumMonths, {0.0, 0.0, 0.0, 0.0}), gatherDemand(NumMonths, {0.0, 0.0, 0.0, 0.0}),
              collectTime(0.0), collectEnergy(0.0), RTPcost(NumMonths, 0.0), RTPaboveBaseCost(NumMonths, 0.0), RTPbelowBaseCost(NumMonths, 0.0),
              RTPaboveBaseEnergy(NumMonths, 0.0), RTPbelowBaseEnergy(NumMonths, 0.0), seasonForMonth(NumMonths, Season::Invalid), isQualified(false),
              ptDisqualifier(0), isSelected(false), totalAnnualCost(0.0), totalAnnualEnergy(0.0)
        {
        }
    };

    struct QualifyType
    {
        // Members
        int namePt;          // index of the name and variable in the variable array
        int tariffIndx;      // index of the tariff name in the tariff array
        int sourcePt;        // index of the variable in the variable array
        bool isMaximum;      // indicator if maximum test otherwise minimum
        Real64 thresholdVal; // value of the threshold
        int thresholdPt;     // pointer to the variable holding the values
        Season season;       // enumerated list of the kind of season
        bool isConsecutive;  // indicator if consecutive months otherwise count
        int numberOfMonths;  // number of months the test must be good for

        // Default Constructor
        QualifyType()
            : namePt(0), tariffIndx(0), sourcePt(0), isMaximum(false), thresholdVal(0.0), thresholdPt(0), isConsecutive(false), numberOfMonths(0)
        {
        }
    };

    struct ChargeSimpleType
    {
        // Members
        int namePt;                      // index of the name and variable in the variable array
        int tariffIndx;                  // index of the tariff name in the tariff array
        int sourcePt;                    // index of the variable in the variable array
        Season season = Season::Invalid; // enumerated list of the kind of season
        int categoryPt;                  // index of the category in the variable array
        Real64 costPerVal;               // cost per unit value
        int costPerPt;                   // cost per unit index in the variable array (0 is flag for no variable)

        // Default Constructor
        ChargeSimpleType() : namePt(0), tariffIndx(0), sourcePt(0), categoryPt(0), costPerVal(0.0), costPerPt(0)
        {
        }
    };

    struct ChargeBlockType
    {
        // Members
        int namePt;                      // index of the name and variable in the variable array
        int tariffIndx;                  // index of the tariff name in the tariff array
        int sourcePt;                    // index of the variable in the variable array
        Season season = Season::Invalid; // enumerated list of the kind of season
        int categoryPt;                  // index of the category in the variable array
        int remainingPt;                 // index of the remaining into variable in the variable array
        Real64 blkSzMultVal;             // block size multiplier value
        int blkSzMultPt;                 // block size variable in the variable array (0 is flag for no variable)
        int numBlk;                      // number of blocks used
        Array1D<Real64> blkSzVal;        // array of block size values
        Array1D_int blkSzPt;             // block size variables index to the variable array (0 is no variable)
        Array1D<Real64> blkCostVal;      // array of block cost values
        Array1D_int blkCostPt;           // block cost variables index to the variable array (0 is no variable)

        // Default Constructor
        ChargeBlockType()
            : namePt(0), tariffIndx(0), sourcePt(0), categoryPt(0), remainingPt(0), blkSzMultVal(0.0), blkSzMultPt(0), numBlk(0),
              blkSzVal(maxNumBlk, 0.0), blkSzPt(maxNumBlk, 0), blkCostVal(maxNumBlk, 0.0), blkCostPt(maxNumBlk, 0)
        {
        }
    };

    struct RatchetType
    {
        // Members
        int namePt;                          // index of the name and variable in the variable array
        int tariffIndx;                      // index of the tariff name in the tariff array
        int baselinePt;                      // index of the baseline variable in the variable array
        int adjustmentPt;                    // index fo the adjustment variable in the variable array
        Season seasonFrom = Season::Invalid; // enumerated list of the kind of season
        Season seasonTo = Season::Invalid;   // enumerated list of the kind of season
        Real64 multiplierVal;                // value of the ratchet multiplier
        int multiplierPt;                    // multiplier variable in the variable array (0 for no variable)
        Real64 offsetVal;                    // value of the ratchet offset
        int offsetPt;                        // offset variable in the variable array (0 for no variable)

        // Default Constructor
        RatchetType() : namePt(0), tariffIndx(0), baselinePt(0), adjustmentPt(0), multiplierVal(0.0), multiplierPt(0), offsetVal(0.0), offsetPt(0)
        {
        }
    };

    struct ComputationType
    {
        // Members
        std::string computeName; // name of the compute
        int firstStep;           // index to steps array for the first step in this compute steps
        int lastStep;            // index to steps array for the last step in this compute steps
        bool isUserDef;          // if the computation steps were user defined

        // Default Constructor
        ComputationType() : firstStep(0), lastStep(0), isUserDef(false)
        {
        }
    };

    struct StackType
    {
        // Members
        int varPt = 0.0;        // pointer to item in specific array
        Array1D<Real64> values; // values

        StackType() : varPt(0.0), values(NumMonths, 0.0)
        {
        }
    };

    void UpdateUtilityBills(EnergyPlusData &state);

    enum class StepType
    {
        Op,
        Var,
        EOL
    };

    struct Step
    {
        StepType type;
        Op op;
        int varNum;
    };

    //======================================================================================================================
    //======================================================================================================================

    //    GET INPUT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void GetInputEconomicsTariff(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsQualify(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsChargeSimple(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsChargeBlock(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsRatchet(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsVariable(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsComputation(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void GetInputEconomicsCurrencyType(EnergyPlusData &state, bool &ErrorsFound); // true if errors found during getting input objects.

    void parseComputeLine(EnergyPlusData &state, std::string const &lineOfCompute, int const fromTariff);

    void GetLastWord(std::string const &lineOfText, std::string::size_type &endOfScan, std::string &aWord);

    void initializeMonetaryUnit(EnergyPlusData &state);

    int FindTariffIndex(EnergyPlusData &state,
                        std::string const &nameOfTariff,
                        std::string const &nameOfReferingObj,
                        bool &ErrorsFound,
                        std::string const &nameOfCurObj);

    void warnIfNativeVarname(
        EnergyPlusData &state, std::string const &objName, int const curTariffIndex, bool &ErrorsFound, std::string const &curobjName);

    int AssignVariablePt(EnergyPlusData &state,
                         std::string_view const stringIn,
                         bool const flagIfNotNumeric,
                         int const useOfVar,
                         int const varSpecific,
                         ObjType const econObjKind,
                         int const objIndex,
                         int const tariffPt);

    void incrementEconVar(EnergyPlusData &state);

    void incrementSteps(EnergyPlusData &state);

    std::string RemoveSpaces(EnergyPlusData &state, std::string_view const StringIn);

    void CreateCategoryNativeVariables(EnergyPlusData &state);

    int lookupOperator(std::string const &opString);

    //======================================================================================================================
    //======================================================================================================================

    //    DEFAULT COMPUTATION RELATED ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void CreateDefaultComputation(EnergyPlusData &state);

    void addOperand(EnergyPlusData &state, int varMe, int varOperand);

    void addChargesToOperand(EnergyPlusData &state, int curTariff, int curPointer);

    //======================================================================================================================
    //======================================================================================================================

    //    GATHER TIMESTEP VALUES ROUTINE

    //======================================================================================================================
    //======================================================================================================================

    void GatherForEconomics(EnergyPlusData &state);

    bool isWithinRange(EnergyPlusData &state, int const testVal, int const minThreshold, int const maxThreshold);

    //======================================================================================================================
    //======================================================================================================================

    //    COMPUTE THE UTILITY BILLS AND CREATE REPORTS

    //======================================================================================================================
    //======================================================================================================================

    void ComputeTariff(EnergyPlusData &state);

    void pushStack(EnergyPlusData &state, Array1A<Real64> const monthlyArray, int const variablePointer);

    void popStack(EnergyPlusData &state, Array1A<Real64> monthlyArray, int &variablePointer);

    void evaluateChargeSimple(EnergyPlusData &state, int const usingVariable);

    void evaluateChargeBlock(EnergyPlusData &state, int const usingVariable);

    void evaluateRatchet(EnergyPlusData &state, int const usingVariable);

    void evaluateQualify(EnergyPlusData &state, int const usingVariable);

    void addMonthlyCharge(EnergyPlusData &state, int const usingVariable);

    void checkMinimumMonthlyCharge(EnergyPlusData &state, int const curTariff);

    void setNativeVariables(EnergyPlusData &state);

    void LEEDtariffReporting(EnergyPlusData &state);

    void WriteTabularTariffReports(EnergyPlusData &state);

    void showWarningsBasedOnTotal(EnergyPlusData &state);

    void getMaxAndSum(EnergyPlusData &state, int const varPointer, Real64 &sumResult, Real64 &maxResult);

    void ReportEconomicVariable(EnergyPlusData &state,
                                std::string const &titleString,
                                bool const includeCategory,
                                bool const showCurrencySymbol,
                                std::string const &forString,
                                OutputReportTabular::tabularReportStyle &style);

    void selectTariff(EnergyPlusData &state);

    void GetMonthlyCostForResource(EnergyPlusData &state, Constant::eResource const inResourceNumber, Array1A<Real64> outMonthlyCosts);

} // namespace EconomicTariff

struct EconomicTariffData : BaseGlobalStruct
{

    int numEconVar = 0;
    int sizeEconVar = 0;

    // holds the outbound connections for each variable
    Array1D<EconomicTariff::Step> operands; // sized to sizeOperand
    int numOperand = 0;
    int sizeOperand = 0;

    int numTariff = 0;
    int numQualify = 0;
    int numChargeSimple = 0;
    int numChargeBlock = 0;
    int numRatchet = 0;
    int numComputation = 0;

    // list of pointers to variable, 0 end of line, negative indicate operations
    Array1D<EconomicTariff::Step> steps;
    Array1D<EconomicTariff::Step> stepsCopy;

    int numSteps = 0;
    int sizeSteps = 0;

    int topOfStack = 0;
    int sizeStack = 0;

    bool Update_GetInput = true;
    int addOperand_prevVarMe = 0;

    Array1D<EconomicTariff::EconVarType> econVar;
    EPVector<EconomicTariff::TariffType> tariff;
    EPVector<EconomicTariff::QualifyType> qualify;
    EPVector<EconomicTariff::ChargeSimpleType> chargeSimple;
    EPVector<EconomicTariff::ChargeBlockType> chargeBlock;
    EPVector<EconomicTariff::RatchetType> ratchet;
    EPVector<EconomicTariff::ComputationType> computation;
    Array1D<EconomicTariff::StackType> stack;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->numEconVar = 0;
        this->sizeEconVar = 0;
        this->operands.deallocate();
        this->numOperand = 0;
        this->sizeOperand = 0;
        this->numTariff = 0;
        this->numQualify = 0;
        this->numChargeSimple = 0;
        this->numChargeBlock = 0;
        this->numRatchet = 0;
        this->numComputation = 0;
        this->steps.deallocate();
        this->stepsCopy.deallocate();
        this->numSteps = 0;
        this->sizeSteps = 0;
        this->topOfStack = 0;
        this->sizeStack = 0;
        this->Update_GetInput = true;
        this->addOperand_prevVarMe = 0;
        this->econVar.deallocate();
        this->tariff.deallocate();
        this->qualify.deallocate();
        this->chargeSimple.deallocate();
        this->chargeBlock.deallocate();
        this->ratchet.deallocate();
        this->computation.deallocate();
        this->stack.deallocate();
    }
};

} // namespace EnergyPlus

#endif
