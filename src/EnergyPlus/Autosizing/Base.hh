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

#ifndef Base_hh_INCLUDED
#define Base_hh_INCLUDED

// EnergyPlus headers
#include <EnergyPlus/AirLoopHVACDOAS.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/api/TypeDefs.h>

#include <ObjexxFCL/Optional.hh>

namespace EnergyPlus {

enum class AutoSizingType
{
    // align with DataHVACGlobals so scalable sizing strings can be applied
    // this will not be necessary when scalable sizing is moved to BaseSizerWithScalableInputs
    Invalid = -1,
    CoolingAirFlowSizing = 1,
    CoolingWaterflowSizing = 2,
    HeatingWaterflowSizing = 3,
    CoolingWaterDesAirInletTempSizing = 4,
    CoolingWaterDesAirInletHumRatSizing = 5,
    CoolingWaterDesWaterInletTempSizing = 6,
    CoolingWaterDesAirOutletTempSizing = 7,
    CoolingWaterDesAirOutletHumRatSizing = 8,
    CoolingWaterNumofTubesPerRowSizing = 9,
    HeatingWaterDesAirInletTempSizing = 10,
    HeatingWaterDesAirInletHumRatSizing = 11,
    HeatingWaterDesCoilLoadUsedForUASizing = 12,
    HeatingWaterDesCoilWaterVolFlowUsedForUASizing = 13,
    HeatingAirFlowSizing = 14,
    HeatingAirflowUASizing = 15,
    SystemAirFlowSizing = 16,
    CoolingCapacitySizing = 17,
    HeatingCapacitySizing = 18,
    WaterHeatingCapacitySizing = 19,
    WaterHeatingCoilUASizing = 20,
    SystemCapacitySizing = 21, // not used
    CoolingSHRSizing = 22,
    HeatingDefrostSizing = 23, // not used
    MaxHeaterOutletTempSizing = 24,
    AutoCalculateSizing = 25,
    ZoneCoolingLoadSizing = 26,
    ZoneHeatingLoadSizing = 27,
    MinSATempCoolingSizing = 28, // not used
    MaxSATempHeatingSizing = 29, // not used
    ASHRAEMinSATCoolingSizing = 30,
    ASHRAEMaxSATHeatingSizing = 31,
    HeatingCoilDesAirInletTempSizing = 32,
    HeatingCoilDesAirOutletTempSizing = 33,
    HeatingCoilDesAirInletHumRatSizing = 34,
    DesiccantDehumidifierBFPerfDataFaceVelocitySizing = 35,
    Num
};

enum class AutoSizingResultType
{
    Invalid = -1,
    NoError,    // no errors found
    ErrorType1, // sizing error
    ErrorType2, // uninitialized sizing type
    Num
};

struct BaseSizer
{

    Real64 stdRhoAir = 0.0;

    int zoneAirFlowSizMethod = 0;
    bool dataScalableSizingON = false;
    bool dataScalableCapSizingON = false;
    bool isCoilReportObject = false; // provides access to coil reporting
    bool isFanReportObject = false;  // provides access to fan reporting
    bool initialized = false;        // indicates initializeWithinEP was called
    AutoSizingResultType errorType = AutoSizingResultType::NoError;
    AutoSizingType sizingType = AutoSizingType::Invalid;
    std::string sizingString;
    std::string sizingStringScalable;
    bool overrideSizeString = true;
    Real64 originalValue = 0.0;
    Real64 autoSizedValue = 0.0;
    bool wasAutoSized = false;
    bool hardSizeNoDesignRun = false;
    bool sizingDesRunThisAirSys = false;
    bool sizingDesRunThisZone = false;
    bool sizingDesValueFromParent = false;
    bool airLoopSysFlag = false;
    bool oaSysFlag = false;
    int coilType_Num = 0;
    std::string compType;
    std::string compName;
    bool isEpJSON = false;

    bool sysSizingRunDone = false;
    bool zoneSizingRunDone = false;
    int curSysNum = 0;
    int curOASysNum = 0;
    int curZoneEqNum = 0;
    HVAC::AirDuctType curDuctType = HVAC::AirDuctType::Invalid;
    int curTermUnitSizingNum = 0; // index in zone equipment vector - for single duct, IU, and PIU
    int numPrimaryAirSys = 0;
    int numSysSizInput = 0;
    bool doSystemSizing = false;
    int numZoneSizingInput = 0;
    bool doZoneSizing = false;
    bool autoCalculate = false; // indicator that AutoCalculate is used

    // terminal units
    bool termUnitSingDuct = false; // single duct terminal unit
    bool termUnitPIU = false;      // powered induction unit
    bool termUnitIU = false;       // induction terminal unit
    bool zoneEqFanCoil = false;    // fan coil zone equipment
    bool otherEqType = false;      // this covers the ELSE type switch
    bool zoneEqUnitHeater = false; // unit heater zone equipment
    bool zoneEqUnitVent = false;   // unit ventilator zone equipment
    bool zoneEqVentedSlab = false; // ventilated slab zone equipment

    // global sizing data
    DataSizing::OAControl minOA = DataSizing::OAControl::Invalid;

    // global Data* sizing constants
    bool dataEMSOverrideON = false;
    Real64 dataEMSOverride = 0.0;
    bool dataAutosizable = false;
    // HeatingWaterflowSizer
    Real64 dataConstantUsedForSizing = 0.0;
    Real64 dataFractionUsedForSizing = 0.0;
    bool dataDXCoolsLowSpeedsAutozize = false;

    // HeatingWaterDesCoilWaterVolFlowUsedForUASizer
    int dataPltSizHeatNum = 0;
    // HeatingWaterDesCoilWaterVolFlowUsedForUASizer, HeaterWaterflowSizing
    int dataWaterLoopNum = 0;
    // CoolingWaterflowSizing, CoolingWaterDesAirInletTempSizer
    int dataFanIndex = 0;
    HVAC::FanType dataFanType = HVAC::FanType::Invalid;
    // CoolingWaterflowSizing
    Real64 dataWaterCoilSizCoolDeltaT = 0.0;
    // HeaterWaterflowSizing
    Real64 dataWaterCoilSizHeatDeltaT = 0.0;
    Real64 dataCapacityUsedForSizing = 0.0;

    // CoolingWaterDesWaterInletTempSizer, CoolingWaterNumofTubesPerRowSizer
    int dataPltSizCoolNum = 0;

    // CoolingWaterDesAirInletHumRatSizer, CoolingWaterDesAirOutletHumRatSizer
    Real64 dataDesInletAirHumRat = 0.0;
    // CoolingWaterDesAirInletTempSizer
    Real64 dataAirFlowUsedForSizing = 0.0;
    Real64 dataDesInletAirTemp = 0.0;
    bool dataDesAccountForFanHeat = false;
    HVAC::FanPlace dataFanPlacement = HVAC::FanPlace::Invalid;

    // CoolingWaterDesAirInletHumRatSizer, HeatingWaterDesAirInletHumRatSizer,
    // HeatingWaterDesAirInletTempSizer
    Real64 dataFlowUsedForSizing = 0.0;

    // CoolingWaterDesAirOutletHumRatSizer
    Real64 dataDesOutletAirHumRat = 0.0;
    Real64 dataDesInletWaterTemp = 0.0;
    Real64 dataDesOutletAirTemp = 0.0;

    // CoolingWaterNumofTubesPerRowSizer, HeatingWaterDesCoilWaterVolFlowUsedForUASizer
    Real64 dataWaterFlowUsedForSizing = 0.0;

    // CoolingSHRSizing
    Real64 dataSizingFraction = 1.0;
    int dataDXSpeedNum = 0;

    // WaterHeatingCapacitySizing
    bool dataDesicRegCoil = false;

    // WaterHeatingCapacitySizing
    Real64 dataHeatSizeRatio = 0.0;

    // ASHRAEMinSATCoolingSizing, ASHRAEMaxSATHeatingSizing
    int dataZoneUsedForSizing = 0;

    // HeatingCoilDesAirInletTempSizing,
    int dataDesicDehumNum = 0;

    // WaterHeatingCoilUASizing
    bool dataNomCapInpMeth = false;
    int dataCoilNum = 0;
    HVAC::FanOp dataFanOp = HVAC::FanOp::Invalid;
    Real64 dataDesignCoilCapacity = 0.0;
    bool dataErrorsFound = false;

    // CoolingAirFlowSizing
    Real64 dataBypassFrac = 0.0;

    bool dataIsDXCoil = false;

    Real64 dataNonZoneNonAirloopValue = 0.0;

    bool printWarningFlag = false;
    std::string callingRoutine;
    EPVector<DataSizing::SystemSizingInputData> sysSizingInputData;
    EPVector<DataSizing::ZoneSizingInputData> zoneSizingInput;
    EPVector<DataSizing::ZoneEqSizingData> unitarySysEqSizing;
    EPVector<DataSizing::ZoneEqSizingData> oaSysEqSizing;
    EPVector<DataSizing::ZoneEqSizingData> zoneEqSizing;
    EPVector<DataAirLoop::OutsideAirSysProps> outsideAirSys;
    EPVector<DataSizing::TermUnitSizingData> termUnitSizing;
    EPVector<DataSizing::TermUnitZoneSizingData> termUnitFinalZoneSizing;
    EPVector<DataSizing::ZoneSizingData> finalZoneSizing;
    EPVector<DataSizing::SystemSizingData> finalSysSizing;
    EPVector<DataSizing::PlantSizingData> plantSizData;
    EPVector<DataAirSystems::DefinePrimaryAirSystem> primaryAirSystem;
    std::vector<AirLoopHVACDOAS::AirLoopDOAS> airloopDOAS;
    EPVector<DataAirLoop::AirLoopControlData> airLoopControlInfo;

    // public methods

    virtual void initializeWithinEP(EnergyPlusData &state,
                                    std::string_view const _compType,
                                    std::string_view const _compName,
                                    bool _printWarningFlag,
                                    std::string_view const _callingRoutine);

    virtual Real64 size(EnergyPlusData &state, Real64 originalValue, bool &errorsFound) = 0;

    std::string getLastErrorMessages();

    void overrideSizingString(std::string_view const string);

protected:
    std::string lastErrorMessages;

    void addErrorMessage(std::string const &s);

    void initializeFromAPI(EnergyPlusData &state, Real64 elevation); // don't accidentally call this direct component from outside

    void preSize(EnergyPlusData &state, Real64 originalValue);

    void selectSizerOutput(EnergyPlusData &state, bool &errorsFound);

    void select2StgDXHumCtrlSizerOutput(EnergyPlusData &state, bool &errorsFound);

    bool isValidCoilType(std::string const &compType);

    bool isValidFanType(std::string const &compType);

    bool checkInitialized(EnergyPlusData &state, bool &errorsFound);

    void clearState();

public:
    static void reportSizerOutput(EnergyPlusData &state,
                                  std::string_view CompType,
                                  std::string_view CompName,
                                  std::string_view VarDesc,
                                  Real64 VarValue,
                                  ObjexxFCL::Optional_string_const UsrDesc = _,
                                  ObjexxFCL::Optional<Real64 const> UsrValue = _);

    static void reportSizerStrOutput(
        EnergyPlusData &state, std::string_view CompType, std::string_view CompName, std::string_view VarDesc, std::string_view VarValue);

    static Real64 setOAFracForZoneEqSizing(const EnergyPlusData &state, Real64 desMassFlow, DataSizing::ZoneEqSizingData const &zoneEqSizing);
    static Real64 setHeatCoilInletTempForZoneEqSizing(Real64 outAirFrac,
                                                      DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                      DataSizing::ZoneSizingData const &finalZoneSizing);
    static Real64 setHeatCoilInletHumRatForZoneEqSizing(Real64 outAirFrac,
                                                        DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                        DataSizing::ZoneSizingData const &finalZoneSizing);
    static Real64 setCoolCoilInletTempForZoneEqSizing(Real64 outAirFrac,
                                                      DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                      DataSizing::ZoneSizingData const &finalZoneSizing);
    static Real64 setCoolCoilInletHumRatForZoneEqSizing(Real64 outAirFrac,
                                                        DataSizing::ZoneEqSizingData const &zoneEqSizing,
                                                        DataSizing::ZoneSizingData const &finalZoneSizing);
};

} // namespace EnergyPlus

#endif
