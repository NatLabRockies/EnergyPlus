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

#ifndef IceThermalStorage_hh_INCLUDED
#define IceThermalStorage_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace IceThermalStorage {

    enum class CurveVars
    {
        Invalid = -1,
        FracChargedLMTD,
        FracDischargedLMTD,
        LMTDMassFlow,
        LMTDFracCharged,
        Num
    };

    enum class DetIce
    {
        Invalid = -1,
        InsideMelt,  // Inside melt system--charge starting with bare coil
        OutsideMelt, // Outside melt system--charge from existing ice layer on coil
        Num
    };

    enum class ITSType
    {
        Invalid = -1,
        IceOnCoilInternal,
        IceOnCoilExternal,
        Num
    };

    struct ThermalStorageSizingData
    {
        std::string name;
        Real64 onPeakStart = 0.0;
        Real64 onPeakEnd = 0.0;
        Real64 sizingFactor = 1.0;
    };

    struct SimpleIceStorageData : PlantComponent
    {
        std::string Name;                     // User identifier
        std::string ITSType;                  // Ice Thermal Storage Type
        enum ITSType ITSType_Num;             // Storage Type as number (IceOnCoilInternal,IceOnCoilExternal)
        int MapNum;                           // Number to Map structure
        int UratePtr;                         // Charging/Discharging SchedulePtr: u value schedule
        Real64 ITSNomCap;                     // Design nominal capacity of Ice Thermal Storage [J] (user input in GJ)
        bool NomCapacityWasAutoSized = false; // Design storage capacity of Ice Thermal Storage system was autosized
        int PltInletNodeNum;                  // Node number on the inlet side of the plant
        int PltOutletNodeNum;                 // Node number on the outlet side of the plant
                                              // loop topology variables
        PlantLocation plantLoc;
        Real64 DesignMassFlowRate;
        Real64 FreezeTemp;
        bool ResetXForITSFlag;
        bool MyEnvrnFlag;
        Real64 UAIceCh;
        Real64 UAIceDisCh;
        Real64 HLoss;
        Real64 XCurIceFrac;
        Real64 ITSMassFlowRate;
        Real64 ITSInletTemp;
        Real64 ITSOutletTemp;
        Real64 ITSOutletSetPointTemp;
        Real64 ITSCoolingRate;
        Real64 ITSCoolingEnergy;
        bool CheckEquipName;

        Real64 MyLoad;            // load requested by plant [W]
        Real64 Urate;             // [fraction]
        Real64 IceFracRemain;     // Fraction of ice remaining in storage [fraction]
        Real64 ITSChargingRate;   // [W]
        Real64 ITSChargingEnergy; // [J]
        Real64 ITSmdot;           // [kg/s]

        // Duplicated reporting vars for now. Investigate diffs when time to remove.
        Real64 ITSCoolingRate_rep;   // [W]
        Real64 ITSCoolingEnergy_rep; // [J]

        bool MyPlantScanFlag;
        bool MyEnvrnFlag2;
        int TESSizingIndex = 0;

        // Default Constructor
        SimpleIceStorageData()
            : MapNum(0), UratePtr(0), ITSNomCap(0.0), PltInletNodeNum(0), PltOutletNodeNum(0), plantLoc{}, DesignMassFlowRate(0.0), FreezeTemp(0.0),
              ResetXForITSFlag(false), MyEnvrnFlag(true), UAIceCh(0.0), UAIceDisCh(0.0), HLoss(0.0), XCurIceFrac(0.0), ITSMassFlowRate(0.0),
              ITSInletTemp(0.0), ITSOutletTemp(0.0), ITSOutletSetPointTemp(0.0), ITSCoolingRate(0.0), ITSCoolingEnergy(0.0), CheckEquipName(true),
              MyLoad(0.0), Urate(0.0), IceFracRemain(0.0), ITSChargingRate(0.0), ITSChargingEnergy(0.0), ITSmdot(0.0), ITSCoolingRate_rep(0.0),
              ITSCoolingEnergy_rep(0.0), MyPlantScanFlag(true), MyEnvrnFlag2(true)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void initialize(EnergyPlusData &state);

        void size(EnergyPlusData &state);

        void CalcIceStorageDormant(EnergyPlusData &state);

        void CalcIceStorageCapacity(EnergyPlusData &state, Real64 &MaxCap, Real64 &MinCap, Real64 &OptCap);

        void CalcIceStorageDischarge(EnergyPlusData &state, Real64 myLoad, bool RunFlag, Real64 MaxCap);

        void CalcQiceDischageMax(EnergyPlusData &state, Real64 &QiceMin);

        void CalcIceStorageCharge(EnergyPlusData &state);

        void CalcQiceChargeMaxByChiller(EnergyPlusData &state, Real64 &QiceMaxByChiller);

        void CalcQiceChargeMaxByITS(Real64 chillerOutletTemp, Real64 &QiceMaxByITS);

        void CalcUAIce(Real64 XCurIceFrac_loc, Real64 &UAIceCh_loc, Real64 &UAIceDisCh_loc, Real64 &HLoss_loc);

        void UpdateNode(EnergyPlusData &state, Real64 myLoad, bool RunFlag);

        void RecordOutput(Real64 myLoad, bool RunFlag);

        void setupOutputVars(EnergyPlusData &state);
    };

    struct DetailedIceStorageData : PlantComponent
    {
        std::string Name;                     // User identifier // What kind of a comment is this?
        Sched::Schedule *availSched;          // schedule (availability?)
        Real64 NomCapacity;                   // Design storage capacity of Ice Thermal Storage system [W-hr]
        bool NomCapacityWasAutoSized = false; // Design storage capacity of Ice Thermal Storage system was autosized
        // (User input for this parameter in GJ--need to convert to W-hr)
        int PlantInNodeNum;  // Plant inlet node number for ice storage unit
        int PlantOutNodeNum; // Plant outlet node number for ice storage unit
        PlantLocation plantLoc;
        Real64 DesignMassFlowRate;
        int MapNum;                           // Number to Map structure
        std::string DischargeCurveName;       // Curve name for discharging (used to find the curve index)
        int DischargeCurveNum;                // Curve index for discharging
        enum CurveVars DischargeCurveTypeNum; // Integer version of discharging curve independent variables type
        std::string ChargeCurveName;          // Curve name for charging (used to find the curve index)
        int ChargeCurveNum;                   // Curve index for charging
        enum CurveVars ChargeCurveTypeNum;    // Integer version of charging curve independent variables type
        Real64 CurveFitTimeStep;              // Time step used to generate performance data [hours]
        Real64 DischargeParaElecLoad;         // Parasitic electric load duing discharging [dimensionless]
        // (This is multiplied by the tank capacity to obtain elec consump)
        Real64 ChargeParaElecLoad; // Parasitic electric load duing charging [dimensionless]
        // (This is multiplied by the tank capacity to obtain elec consump)
        Real64 TankLossCoeff; // Fraction of total storage capacity lost per hour [1/hours]
        Real64 FreezingTemp;  // Freezing/melting temperature of ice storage unit [C]
        // Reporting data
        Real64 CompLoad;                  // load requested by plant [W]
        Real64 IceFracChange;             // Change in fraction of ice stored during the time step [fraction]
        Real64 IceFracRemaining;          // Fraction of ice remaining in storage [fraction]
        std::string ThawProcessIndicator; // User input determining whether system is inside or outside melt
        enum DetIce ThawProcessIndex;     // Conversion of thaw process indicator to integer index
        Real64 IceFracOnCoil;             // Fraction of ice on the coil (affects charging) [fraction]
        Real64 DischargingRate;           // Rate at which energy is being added (thawing) to ice unit [W]
        Real64 DischargingEnergy;         // Total energy added to the ice storage unit [J]
        Real64 ChargingRate;              // Rate at which energy is removed (freezing) to ice unit [W]
        Real64 ChargingEnergy;            // Total energy removed from ice storage unit [J]
        Real64 MassFlowRate;              // Total mass flow rate to component [kg/s]
        Real64 BypassMassFlowRate;        // Mass flow rate that bypasses the ice unit locally [kg/s]
        Real64 TankMassFlowRate;          // Mass flow rate through the ice storage unit [kg/s]
        Real64 InletTemp;                 // Component inlet temperature (same as bypass temperature) [C]
        Real64 OutletTemp;                // Component outlet temperature (blended) [C]
        Real64 TankOutletTemp;            // Ice storage unit outlet temperature [C]
        Real64 ParasiticElecRate;         // Parasitic electrical energy rate consumed by ice storage [W]
        Real64 ParasiticElecEnergy;       // Total parasitic electrical energy consumed by ice storage [J]
        int DischargeIterErrors;          // Number of max iterations exceeded errors during discharging
        int DischargeErrorCount;          // Index for error counting routine
        int ChargeIterErrors;             // Number of max iterations exceeded errors during charging
        int ChargeErrorCount;             // Index for error counting routine
        bool ResetXForITSFlag;
        bool MyEnvrnFlag;
        bool CheckEquipName;
        bool MyPlantScanFlag;
        bool MyEnvrnFlag2;
        int TESSizingIndex = 0;

        // Default Constructor
        DetailedIceStorageData()
            : NomCapacity(0.0), PlantInNodeNum(0), PlantOutNodeNum(0), plantLoc{}, DesignMassFlowRate(0.0), MapNum(0), DischargeCurveNum(0),
              ChargeCurveNum(0), CurveFitTimeStep(1.0), DischargeParaElecLoad(0.0), ChargeParaElecLoad(0.0), TankLossCoeff(0.0), FreezingTemp(0.0),
              CompLoad(0.0), IceFracChange(0.0), IceFracRemaining(1.0), IceFracOnCoil(1.0), DischargingRate(0.0), DischargingEnergy(0.0),
              ChargingRate(0.0), ChargingEnergy(0.0), MassFlowRate(0.0), BypassMassFlowRate(0.0), TankMassFlowRate(0.0), InletTemp(0.0),
              OutletTemp(0.0), TankOutletTemp(0.0), ParasiticElecRate(0.0), ParasiticElecEnergy(0.0), DischargeIterErrors(0), DischargeErrorCount(0),
              ChargeIterErrors(0), ChargeErrorCount(0), ResetXForITSFlag(false), MyEnvrnFlag(true), CheckEquipName(true), MyPlantScanFlag(true),
              MyEnvrnFlag2(true)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void initialize(EnergyPlusData &state);

        void size(EnergyPlusData &state);

        void SimDetailedIceStorage(EnergyPlusData &state);

        void UpdateDetailedIceStorage(EnergyPlusData &state);

        void ReportDetailedIceStorage(EnergyPlusData &state);

        void setupOutputVars(EnergyPlusData &state);
    };

    void GetIceStorageInput(EnergyPlusData &state);

    Real64 CalcDetIceStorLMTDstar(Real64 Tin,  // ice storage unit inlet temperature
                                  Real64 Tout, // ice storage unit outlet (setpoint) temperature
                                  Real64 Tfr   // freezing temperature
    );

    Real64 CalcQstar(EnergyPlusData &state,
                     int CurveIndex,                 // curve index
                     enum CurveVars CurveIndVarType, // independent variable type for ice storage
                     Real64 FracCharged,             // fraction charged for ice storage unit
                     Real64 LMTDstar,                // normalized log mean temperature difference across the ice storage unit
                     Real64 MassFlowstar             // normalized mass flow rate through the ice storage unit
    );

    Real64 TempSItoIP(Real64 Temp);

    Real64 TempIPtoSI(Real64 Temp);

    void UpdateIceFractions(EnergyPlusData const &state);

} // namespace IceThermalStorage

struct IceThermalStorageData : BaseGlobalStruct
{

    bool getITSInput = true;
    int NumThermalStorageSizing = 0;
    int NumSimpleIceStorage = 0;
    int NumDetailedIceStorage = 0;
    int TotalNumIceStorage = 0;
    EPVector<IceThermalStorage::ThermalStorageSizingData> ThermalStorageSizing;
    EPVector<IceThermalStorage::SimpleIceStorageData> SimpleIceStorage;
    EPVector<IceThermalStorage::DetailedIceStorageData> DetailedIceStorage;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->getITSInput = true;
        this->NumSimpleIceStorage = 0;
        this->NumDetailedIceStorage = 0;
        this->TotalNumIceStorage = 0;
        this->SimpleIceStorage.deallocate();
        this->DetailedIceStorage.deallocate();
    }
};

} // namespace EnergyPlus

#endif
