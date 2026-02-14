A New Autosizing Library for EnergyPlus
=======================================

A major part of EnergyPlus is the ability to autosize components and systems.
The logic, algorithms, and coefficients used for doing this sizing is partially captured in a large `RequestSizing` function, but there is also some logic that is spread throughout the code, and some blocks are repeated in many locations.

# Overview

This work will organize the existing autosizing capabilities into a well-organized source code subdirectory `src/EnergyPlus/Autosizing`.
In this folder, a base class will be created which defines the capabilities for all possible autosized values.
The final form of this base class is not yet locked down, but it will presumably contain at least this:

```c++
enum class AutoSizingType { AutoCalculate,
                            ChillerWaterCoilFlowSizing,
                            ..., Unknown }

enum class AutoSizingResultType { Success, Error }

struct BaseSizer {
    Real64 originalValue;
    Real64 autosizedValue;
    bool wasAutosized;

    virtual void initializeWithinEP(...) = 0;
    virtual AutoSizingResultType size(...) = 0;
}
```

Derived types will be created from this base class, and each derived type will contain the algorithms, logic, and coefficients, necessary for sizing that value.
However, the static data is not the only thing needed for all sizing, some autosizing also needs dynamic load data to evaluate sizes.

A possible autosizing derived class could look like the following:

```c++
// declaration
struct ChilledWaterCoilFlowSizer : BaseSizer {
    constexpr coefficientAlpha = 3.14159;

    ChilledWaterCoilFlowSizer() {
            this->sizingType = AutoSizingType::ChilledWaterCoilFlowSizing;
    }
    ~ChilledWaterCoilFlowSizer() = default;

    void initializeWithinEP(...) override;
    SizingStatus size(EnergyPlusData &state,Real64 &_originalSize) override;
}

// definition
bool ChilledWaterCoilFlowSizer::size(EnergyPlusData &state,_originalSize) {
    // sizes chilled water coil water flow rates
    this->originalValue = _originalSize;
    if (_originalValue == DataSizing::Autosize) {
        this->wasAutosized = true;
    }
    // sizing etc
    if (this->wasAutosized) {
        this->autosizedValue = appropriate sizing data/calculation;
    } else {
        this->autosizedValue = this->originalValue;
    }
    return AutoSizingResultType::Success;
}
```

The `initializeWithinEP(...)` and `size(...)` methods will try to use unified arguments across all types to achieve additional gains, but if not then we'll work around that.

Each component in EnergyPlus that has an autosized value would just create an instance of a derived sizer class.
The component can call the size(...) function, retrieve the autosized value, and also inspect the instance to get more data if needed.
In the long run, once we have these sizers fully organized, we may be able to leverage polymorphism and allow components to store an intermediate class, but this is not important immediately.

# Implementation

The goal of this will be to make the size() functions totally portable -- no global data access.
However, this will require a massive amount of data to be passed in or set up.
The hope is that ultimately these sizer functions can be exposed via the API, which would make this situation more awkward.
To allow this to be implemented throughout the code in a reasonable amount of time, we will make the size() functions fully portable, and instead rely on an initialization function to bring in data.
At first, the sizer classes will have just one initialization function, akin to `initializeWithinEP()`.
The purpose of this function is to bring all EnergyPlus "global" data into the struct members.
This will enable the actual call to `size` to be portable, without a dependence on global data.
Once we start exposing this via API, we will add a new function called `initializeFromAPI`, which will require the user to pass in all the required data that would otherwise be setup inside EnergyPlus.
This approach has a benefit of generally being able to stay out of the way of other refactors, including the global state refactor currently underway.

The base class will contain as much information and functionality as possible in order to reduce the burden on each individual sizer class.
Although the final form will evolve a little over time, we anticipate the base class to have its own `initializeWithinEPlus` function, as well as `preSize` (check if sizing was performed for a specific zone or system) and `reportSizerOutput` (printing to eio) functions that derived types must call.

# Testing

Unit testing will be vitally important during this.
From the very beginning we will attempt to achieve 100% unit test coverage by calling size() with varying initialization. Comparison of existing example file results to eliminate diffs.



# Expected Changes

This feature will have zero input changes, and expected to have zero (or at least *extremely* small) diffs.
There's really no reason for diffs unless something is fixed along the way.

# Process

One beauty of this method is that sizers can be added incrementally and component models can easily revert between the legacy and new implementations in case diffs are encountered.
We will determine as we go which sizer to implement each step of the way, and make revisions to the implementation if function arguments need to change in the base class.
The actual order doesn't matter too much as we will have converted all of them by the end of this project anyway.

# API

Now, once we have a pattern nailed down and approved, we will start considering the design of the API.
The basic idea is that we will need to wrap the C++ classes with C interfaces, and then create Python bindings on top of those.
We won't do this at first because we will want to simplify and solidify the interface and required data and arguments as much as possible first.

# Example Base Class

The base class will be inherited by all sizer classes to minimize the coding effort in new sizer classes. This NFP concept was applied to the base and sizer classes for review and comments. Two existing water heating coil sizing functions have been tested using this method (see enum class AutoSizingType).

    #ifndef Base_hh_INCLUDED
    #define Base_hh_INCLUDED

    #include <EnergyPlus/AirLoopHVACDOAS.hh>
    #include <EnergyPlus/DataAirLoop.hh>
    #include <EnergyPlus/DataSizing.hh>
    #include <EnergyPlus/api/TypeDefs.h>

    namespace EnergyPlus {

        enum class AutoSizingType {
            AutoCalculate,
            HeatingAirflowUASizing,
            HeatingWaterDesAirInletTempSizing,
            Unknown
        };

        enum class AutoSizingResultType {
            NoError,
            ErrorType1
        };

        struct BaseSizer {

        AutoSizingType sizingType = AutoSizingType::Unknown;
        std::string sizingString = "";
        Real64 originalValue = 0.0;
        Real64 autoSizedValue = 0.0;
        bool wasAutoSized = false;
        bool hardSizeNoDesignRun = false;
        bool sizingDesRunThisAirSys = false;
        bool sizingDesRunThisZone = false;
        bool sizingDesValueFromParent = false;
        bool airLoopSysFlag = false;
        bool oaSysFlag = false;
        std::string compType = "";
        std::string compName = "";

        bool sysSizingRunDone = false;
        bool zoneSizingRunDone = false;
        int curSysNum = 0;
        int curOASysNum = 0;
        int curZoneEqNum = 0;
        int curDuctType = 0;
        int curTermUnitSizingNum = 0;
        int numPrimaryAirSys = 0;
        int numSysSizInput = 0;
        bool doSystemSizing = false;
        int numZoneSizingInput = 0;
        bool doZoneSizing = false;

        // terminal units
        bool termUnitSingDuct = false; // single duct terminal unit
        bool termUnitPIU = false;      // powered induction unit
        bool termUnitIU = false;       // induction terminal unit
        bool zoneEqFanCoil = false;    // fan coil zone equipment
        bool otherEqType = false;      // this covers the ELSE types

        bool printWarningFlag = false;
        std::string callingRoutine = "";
        Array1D<DataSizing::SystemSizingInputData> sysSizingInputData;
        Array1D<DataSizing::ZoneSizingInputData> zoneSizingInput;
        Array1D<DataSizing::ZoneEqSizingData> unitarySysEqSizing;
        Array1D<DataSizing::ZoneEqSizingData> oaSysEqSizing;
        Array1D<DataSizing::ZoneEqSizingData> zoneEqSizing;
        Array1D<DataAirLoop::OutsideAirSysProps> outsideAirSys;
        Array1D<EnergyPlus::DataSizing::TermUnitSizingData> termUnitSizing;
        Array1D<EnergyPlus::DataSizing::ZoneSizingData> finalZoneSizing;
        Array1D<EnergyPlus::DataSizing::SystemSizingData> finalSysSizing;
        std::vector<AirLoopHVACDOAS::AirLoopDOAS> airloopDOAS;

        virtual void initializeWithinEP(EnergyPlusData &state,
                               std::string const &_compType,
                               std::string const &_compName,
                               bool _printWarningFlag);

        void preSize(EnergyPlusData &state, Real64 originalValue);

        virtual AutoSizingResultType size(EnergyPlusData &state,
                               Real64 originalValue) = 0;

        static void reportSizerOutput(std::string const &CompType,
                               std::string const &CompName,
                               std::string const &VarDesc,
                               Real64 VarValue,
                               Optional_string_const UsrDesc = _,
                               Optional<Real64 const> UsrValue = _);

        void selectSizerOutput();

        static void clear_state();
        };

        extern bool oneTimeBaseSizerFlag;

    } // namespace EnergyPlus

    #endif

# Example sizer() class


    #ifndef HeatingAirflowUASizing_hh_INCLUDED
    #define HeatingAirflowUASizing_hh_INCLUDED

    #include <EnergyPlus/Autosizing/Base.hh>
    #include <EnergyPlus/DataSizing.hh>
    #include <ObjexxFCL/Array1D.hh>

    namespace EnergyPlus {

    struct HeatingAirflowUASizer : BaseSizer {

        HeatingAirflowUASizer() {
            this->sizingType = AutoSizingType::HeatingAirflowUASizing;
        }
        ~HeatingAirflowUASizer() = default;

        void initializeWithinEP(EnergyPlusData &state,
            std::string const &_compType, std::string const &_compName,
            bool printWarningFlag) override;

        AutoSizingResultType size(EnergyPlusData &state,
            Real64 originalValue) override;
    };

    } // namespace EnergyPlus

    #endif

# Example size() function

    EnergyPlus::AutoSizingResultType HeatingAirflowUASizer::size
                     (EnergyPlusData &state, Real64 _originalValue) {

        AutoSizingResultType errorsFound = AutoSizingResultType::NoError;
        // perform any necessary checks for sizing (e.g., SizingRunDone)
        this->preSize(state, _originalValue);
        // size and report
        if (this->curZoneEqNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisZone) {
                if (this->printWarningFlag && this->originalValue > 0.0) {
                    HeatingAirflowUASizer::reportSizerOutput(
                            this->compType, this->compName,
                            "User-Specified " + this->sizingString,
                            _originalValue);
                }
                this->autoSizedValue = _originalValue;
            } else {
                if (this->termUnitSingDuct &&
                   (this->curTermUnitSizingNum > 0)) {
                    this->autoSizedValue = DataEnvironment::StdRhoAir *
                        this->termUnitSizing(this->curTermUnitSizingNum)
                        .AirVolFlow;
                } else if ((this->termUnitPIU || this->termUnitIU) &&
                           (this->curTermUnitSizingNum > 0)) {
                    this->autoSizedValue = DataEnvironment::StdRhoAir *
                        this->termUnitSizing(this->curTermUnitSizingNum).AirVolFlow *
                        this->termUnitSizing(this->curTermUnitSizingNum).ReheatAirFlowMult;
                } else if (this->zoneEqFanCoil) {
                    this->autoSizedValue =
                            DataEnvironment::StdRhoAir *
                            this->finalZoneSizing(this->curZoneEqNum).DesHeatVolFlow;
                } else if (this->otherEqType) {
                    if (this->zoneEqSizing(this->curZoneEqNum).SystemAirFlow) {
                        this->autoSizedValue =
                            this->zoneEqSizing(this->curZoneEqNum).AirVolFlow *
                            DataEnvironment::StdRhoAir;
                    } else if (this->zoneEqSizing(this->curZoneEqNum).HeatingAirFlow) {
                        this->autoSizedValue =
                            this->zoneEqSizing(this->curZoneEqNum).HeatingAirVolFlow *
                            DataEnvironment::StdRhoAir;
                    } else {
                        this->autoSizedValue =
                            this->finalZoneSizing(this->curZoneEqNum).DesHeatMassFlow;
                    }
                } else {
                    errorsFound = AutoSizingResultType::ErrorType1;
                }
            }
        } else if (this->curSysNum > 0) {
            if (!this->wasAutoSized && !this->sizingDesRunThisAirSys) {
                if (this->printWarningFlag && this->originalValue > 0.0) {
                    HeatingAirflowUASizer::reportSizerOutput(
                        this->compType, this->compName,
                        "User-Specified " + this->sizingString, _originalValue);
                }
                this->autoSizedValue = _originalValue;
            } else {
                if (this->curOASysNum > 0) {
                    if (this->outsideAirSys(this->curOASysNum).AirLoopDOASNum > -1) {
                        this->autoSizedValue = this->airloopDOAS[this->outsideAirSys(
                                this->curOASysNum).AirLoopDOASNum].SizingMassFlow /
                                               DataEnvironment::StdRhoAir;
                    } else {
                        this->autoSizedValue =
                            this->finalSysSizing(this->curSysNum).DesOutAirVolFlow;
                    }
                } else {
                    if (this->curDuctType == DataHVACGlobals::Main) {
                        if (this->finalSysSizing(this->curSysNum).SysAirMinFlowRat > 0.0) {
                            this->autoSizedValue =
                                this->finalSysSizing(this->curSysNum).SysAirMinFlowRat *
                                    this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                        } else {
                            this->autoSizedValue =
                                this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                        }
                    } else if (this->curDuctType == DataHVACGlobals::Cooling) {
                        if (this->finalSysSizing(this->curSysNum).SysAirMinFlowRat > 0.0) {
                            this->autoSizedValue =
                                this->finalSysSizing(this->curSysNum).SysAirMinFlowRat *
                                    this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                        } else {
                            this->autoSizedValue =
                                this->finalSysSizing(this->curSysNum).DesCoolVolFlow;
                        }
                    } else if (this->curDuctType == DataHVACGlobals::Heating) {
                        this->autoSizedValue =
                            this->finalSysSizing(this->curSysNum).DesHeatVolFlow;
                    } else {
                        this->autoSizedValue =
                            this->finalSysSizing(this->curSysNum).DesMainVolFlow;
                    }
                }
                // convert to mass flow for UA calculation
                this->autoSizedValue *= DataEnvironment::StdRhoAir;
            }
        }
        if (this->autoSizedValue < DataHVACGlobals::SmallAirVolFlow)
            this->autoSizedValue = 0.0;
        if (this->wasAutoSized || this->curOASysNum > 0)
            this->selectSizerOutput(); // design vs autosized diff check
        return errorsFound;
    }

# Example component sizing call

## WaterCoils::SizeWaterCoil()

    HeatingAirflowUASizer sizer; // constructor

    // initialize sizer class
    sizer.initializeWithinEP(state,
        DataHVACGlobals::cAllCoilTypes (DataHVACGlobals::Coil_HeatingWater),
        WaterCoil(CoilNum).Name, false);

    // get sizing result
    AutoSizingResultType result = sizer.size(state, DataSizing::AutoSize);

    // set component sizing
    WaterCoil(CoilNum).DesAirMassFlowRate = sizer.autoSizedValue;
    WaterCoil(CoilNum).InletAirMassFlowRate = sizer.autoSizedValue;

    // coil sizing reports as needed
    coilSelectionReportObj->setCoilEntAirMassFlow(CompName, CompType,
        WaterCoil(CoilNum).DesAirMassFlowRate, CurSysNum, CurZoneEqNum);


    if (result != AutoSizingResultType::NoError) {
        ShowSevereError("Developer Error: autosizing of water Heating
            coil air flow used for UA failed.");
        ShowContinueError("Occurs in water heating coil object= " +
            WaterCoil(CoilNum).Name);
        ErrorsFound = true;
    }

# Unit Testing

    TEST_F(AutoSizingFixture, HeatingAirflowUASizingGauntlet)
    {
    // this global state is what would be set up by E+ currently
    DataEnvironment::StdRhoAir = 1.2;
    EnergyPlus::DataSizing::ZoneEqSizing.allocate(1);

    // create the sizer, set flags and specify the sizing configuration
    HeatingAirflowUASizer sizer;

    // ZONE EQUIPMENT TESTING
    DataSizing::CurZoneEqNum = 1;
    DataSizing::CurTermUnitSizingNum = 1;
    DataSizing::TermUnitSingDuct = true;

    sizer.initializeWithinEP(this->state,
        DataHVACGlobals::cAllCoilTypes(DataHVACGlobals::Coil_HeatingWater), "MyWaterCoil", true);

    // Test #1 - Zone Equipment, no autosizing
    Real64 inputValue = 5;

    AutoSizingResultType result = sizer.size(state, inputValue);

    EXPECT_EQ(AutoSizingResultType::NoError, result);
    EXPECT_FALSE(sizer.wasAutoSized);
    EXPECT_NEAR(5.0, sizer.autoSizedValue, 0.01); // hard-sized value
    sizer.autoSizedValue = 0.0;           // reset for next test

    std::string eiooutput =
        std::string("! <Component Sizing Information>, Component Type,
                        Component Name, Input Field Description, Value\n"
                      " Component Sizing Information, Coil:Heating:Water,
                        MyWaterCoil,
                        User-Specified Heating Coil Airflow For UA, 5.00000\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));

# Documentation

Review of existing documentation for necessary changes

# IDD changes and Transition

None.
