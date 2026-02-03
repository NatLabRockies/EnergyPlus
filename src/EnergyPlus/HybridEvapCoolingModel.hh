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

#ifndef HybridEvapCoolingModel_hh_INCLUDED
#define HybridEvapCoolingModel_hh_INCLUDED

// C++ Headers
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

#define MINIMUM_LOAD_TO_ACTIVATE 0.5 // (kw) sets a minimum load to avoid the system fluttering on and off.
#define IMPLAUSIBLE_POWER 10000000

// EnergyPlus Headers
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HybridEvapCoolingModel {

    enum class SYSTEMOUTPUTS
    {
        Invalid = -1,
        VENTILATION_AIR_V,
        SUPPLY_MASS_FLOW,
        SYSTEM_FUEL_USE,
        SUPPLY_AIR_TEMP,
        MIXED_AIR_TEMP,
        SUPPLY_AIR_HR,
        MIXED_AIR_HR,
        OSUPPLY_FAN_POWER,
        OSECOND_FUEL_USE,
        OTHIRD_FUEL_USE,
        OWATER_USE,
        OEXTERNAL_STATIC_PRESSURE,
        Num
    };

    class CModeSolutionSpace
    {
    public:
        std::vector<Real64> MassFlowRatio;
        std::vector<Real64> OutdoorAirFraction;
    };

    class CMode
    {
    public:
        CMode();

        // finish init above
        int ModeID;
        CModeSolutionSpace sol;
        std::string ModeName;
        int Tsa_curve_pointer;
        int HRsa_curve_pointer;
        int Psa_curve_pointer;
        int SFPsa_curve_pointer;
        int ESPsa_curve_pointer;
        int SFUsa_curve_pointer;
        int TFUsa_curve_pointer;
        int WUsa_curve_pointer;

        Real64 Max_Msa;
        Real64 Min_Msa;
        Real64 Min_OAF;
        Real64 Max_OAF;
        Real64 Minimum_Outdoor_Air_Temperature;
        Real64 Maximum_Outdoor_Air_Temperature;
        Real64 Minimum_Outdoor_Air_Humidity_Ratio;
        Real64 Maximum_Outdoor_Air_Humidity_Ratio;
        Real64 Minimum_Outdoor_Air_Relative_Humidity;
        Real64 Maximum_Outdoor_Air_Relative_Humidity;
        Real64 Minimum_Return_Air_Temperature;
        Real64 Maximum_Return_Air_Temperature;
        Real64 Minimum_Return_Air_Humidity_Ratio;
        Real64 Maximum_Return_Air_Humidity_Ratio;
        Real64 Minimum_Return_Air_Relative_Humidity;
        Real64 Maximum_Return_Air_Relative_Humidity;
        Real64 ModelScalingFactor;
        int MODE_BLOCK_OFFSET_Alpha;
        int BLOCK_HEADER_OFFSET_Alpha;
        int MODE1_BLOCK_OFFSET_Number;
        int MODE_BLOCK_OFFSET_Number;
        int BLOCK_HEADER_OFFSET_Number;
        bool ValidPointer(int curve_pointer);
        bool ParseMode(EnergyPlusData &state,
                       int ModeCounter,
                       std::vector<CMode> *OperatingModes,
                       Real64 ScalingFactor,
                       Array1D_string Alphas,
                       Array1D_string cAlphaFields,
                       Array1D<Real64> Numbers,
                       Array1D_string cNumericFields,
                       Array1D<bool> lAlphaBlanks,
                       std::string cCurrentModuleObject);
        void InitializeCurve(int curveType, int CurveID);
        Real64 CalculateCurveVal(EnergyPlusData &state, Real64 Tosa, Real64 Wosa, Real64 Tra, Real64 Wra, Real64 Msa, Real64 OSAF, int curveType);
        bool InitializeOSAFConstraints(Real64 minOSAF, Real64 maxOSAF);
        bool InitializeMsaRatioConstraints(Real64 minMsa, Real64 maxMsa);
        bool InitializeOutdoorAirTemperatureConstraints(Real64 min, Real64 max);
        bool InitializeOutdoorAirHumidityRatioConstraints(Real64 min, Real64 max);
        bool InitializeOutdoorAirRelativeHumidityConstraints(Real64 min, Real64 max);
        bool InitializeReturnAirTemperatureConstraints(Real64 min, Real64 max);
        bool InitializeReturnAirHumidityRatioConstraints(Real64 min, Real64 max);
        bool InitializeReturnAirRelativeHumidityConstraints(Real64 min, Real64 max);
        void GenerateSolutionSpace();
        bool MeetsOAEnvConstraints(Real64 Tosa, Real64 Wosa, Real64 RHos);

    private:
    };

    class CSetting
    {
    public:
        CSetting()
            : Runtime_Fraction(0), Mode(0), Outdoor_Air_Fraction(0), Unscaled_Supply_Air_Mass_Flow_Rate(0), ScaledSupply_Air_Mass_Flow_Rate(0),
              Supply_Air_Ventilation_Volume(0), ScaledSupply_Air_Ventilation_Volume(0), Supply_Air_Mass_Flow_Rate_Ratio(0), SupplyAirTemperature(0),
              Mixed_Air_Temperature(0), SupplyAirW(0), Mixed_Air_W(0), TotalSystem(0), SensibleSystem(0), LatentSystem(0), TotalZone(0),
              SensibleZone(0), LatentZone(0), ElectricalPower(IMPLAUSIBLE_POWER), SupplyFanElectricPower(0), SecondaryFuelConsumptionRate(0),
              ThirdFuelConsumptionRate(0), WaterConsumptionRate(0), ExternalStaticPressure(0)
        {
        }
        Real64 Runtime_Fraction;
        Real64 Mode;
        Real64 Outdoor_Air_Fraction;
        Real64 Unscaled_Supply_Air_Mass_Flow_Rate;
        Real64 ScaledSupply_Air_Mass_Flow_Rate;
        Real64 Supply_Air_Ventilation_Volume;
        Real64 ScaledSupply_Air_Ventilation_Volume;
        Real64 Supply_Air_Mass_Flow_Rate_Ratio;
        Real64 SupplyAirTemperature;
        Real64 Mixed_Air_Temperature;
        Real64 SupplyAirW;
        Real64 Mixed_Air_W;
        Real64 TotalSystem;
        Real64 SensibleSystem;
        Real64 LatentSystem;
        Real64 TotalZone;    // W
        Real64 SensibleZone; // W
        Real64 LatentZone;   // W
        Real64 ElectricalPower;
        Real64 SupplyFanElectricPower;
        Real64 SecondaryFuelConsumptionRate;
        Real64 ThirdFuelConsumptionRate;
        Real64 WaterConsumptionRate;
        Real64 ExternalStaticPressure;

        CMode oMode;
    };

    class CStepInputs
    {
    public:
        CStepInputs()
            : Tosa(0), Tra(0), RHosa(0), RHra(0), RequestedCoolingLoad(0), RequestedHeatingLoad(0), ZoneMoistureLoad(0), ZoneDehumidificationLoad(0),
              MinimumOA(0)
        {
        }
        Real64 Tosa;
        Real64 Tra;
        Real64 RHosa;
        Real64 RHra;
        Real64 RequestedCoolingLoad;
        Real64 RequestedHeatingLoad;
        Real64 ZoneMoistureLoad;
        Real64 ZoneDehumidificationLoad;
        Real64 MinimumOA;
    };

    class Model // begin declaration of the class
    {
    public: // begin public section
        Model();

        // Default Constructor
        std::string Name; // user identifier
        bool Initialized; // initialization flag ensures the system object is initialized only once.
        int ZoneNum; // stores the current zone associated with the system, this is currently not used but is expected to be used in the next set of
                     // functionality additions.
        Sched::Schedule *availSched = nullptr; // Pointer to the correct schedule
        int ZoneNodeNum;                       // index of zone air node in node structure
        std::string AvailManagerListName;      // Name of an availability manager list object
        Avail::Status availStatus = Avail::Status::NoAction;

        Real64 SystemMaximumSupplyAirFlowRate;           // taken from IDF N1, the system max supply flow rate in m3/s.
        bool FanHeatGain;                                // .TRUE. = fan heat gain is accounted for in the lookup tables
        std::string FanHeatGainLocation;                 // Fan heat gain location
        Real64 FanHeatInAirFrac;                         // the fraction of fan heat in air stream to calculate fan heat gain if not in lookup tables
        Real64 ScalingFactor;                            // taken from IDF N3, linear scaling factor.
        Real64 ScaledSystemMaximumSupplyAirMassFlowRate; // the scaled system max supply mass flow rate in m3/s.
        Real64 ScaledSystemMaximumSupplyAirVolumeFlowRate;     // the scaled system max supply volume flow rate in m3/s.
        Constant::eFuel firstFuel = Constant::eFuel::Invalid;  // First fuel type, currently electricity is only option
        Constant::eFuel secondFuel = Constant::eFuel::Invalid; // Second fuel type
        Constant::eFuel thirdFuel = Constant::eFuel::Invalid;  // Third fuel type

        int UnitOn;                          // feels like it should be a bool but its an output and I couldn't get it to work as a bool
        Real64 UnitTotalCoolingRate;         // unit output to zone, total cooling rate [W]
        Real64 UnitTotalCoolingEnergy;       // unit output to zone, total cooling energy [J]
        Real64 UnitSensibleCoolingRate;      // unit sensible cooling rate [W]
        Real64 UnitSensibleCoolingEnergy;    // unit sensible cooling energy [J]
        Real64 UnitLatentCoolingRate;        // unit latent cooling rate [W]
        Real64 UnitLatentCoolingEnergy;      // unit latent cooling energy [J]
        Real64 SystemTotalCoolingRate;       // system output to zone, total cooling rate [W]
        Real64 SystemTotalCoolingEnergy;     // system output to zone, total cooling energy [J]
        Real64 SystemSensibleCoolingRate;    // system sensible cooling rate [W]
        Real64 SystemSensibleCoolingEnergy;  // system sensible cooling energy [J]
        Real64 SystemLatentCoolingRate;      // system latent cooling rate [W]
        Real64 SystemLatentCoolingEnergy;    // system latent cooling energy [J]
        Real64 UnitTotalHeatingRate;         // unit output to zone, total heating rate [W]
        Real64 UnitTotalHeatingEnergy;       // unit output to zone, total heating energy [J]
        Real64 UnitSensibleHeatingRate;      // unit sensible heating rate [W]
        Real64 UnitSensibleHeatingEnergy;    // unit sensible heating energy [J]
        Real64 UnitLatentHeatingRate;        // unit latent heating rate [W]
        Real64 UnitLatentHeatingEnergy;      // unit latent heating energy [J]
        Real64 SystemTotalHeatingRate;       // system output to zone, total heating rate [W]
        Real64 SystemTotalHeatingEnergy;     // system output to zone, total heating energy [J]
        Real64 SystemSensibleHeatingRate;    // system sensible heating rate [W]
        Real64 SystemSensibleHeatingEnergy;  // system sensible heating energy [J]
        Real64 SystemLatentHeatingRate;      // system latent heating rate [W]
        Real64 SystemLatentHeatingEnergy;    // system latent heating energy [J]
        Real64 SupplyFanElectricPower;       //
        Real64 SupplyFanElectricEnergy;      //
        Real64 SecondaryFuelConsumptionRate; //
        Real64 SecondaryFuelConsumption;     //
        Real64 ThirdFuelConsumptionRate;     //
        Real64 ThirdFuelConsumption;         //
        Real64 WaterConsumptionRate;         //
        Real64 WaterConsumption;             //
        Real64 QSensZoneOut;                 // W
        Real64 QLatentZoneOut;               // W
        Real64 QLatentZoneOutMass;           // kg/s
        Real64 ExternalStaticPressure;       //
        Real64 RequestedHumidificationMass;
        Real64 RequestedHumidificationLoad;
        Real64 RequestedHumidificationEnergy;
        Real64 RequestedDeHumidificationMass;
        Real64 RequestedDeHumidificationLoad;
        Real64 RequestedDeHumidificationEnergy;
        Real64 RequestedLoadToHeatingSetpoint;
        Real64 RequestedLoadToCoolingSetpoint;
        Sched::Schedule *TsaMinSched = nullptr;
        Sched::Schedule *TsaMaxSched = nullptr;
        Sched::Schedule *RHsaMinSched = nullptr;
        Sched::Schedule *RHsaMaxSched = nullptr;
        int PrimaryMode;
        Real64 PrimaryModeRuntimeFraction;
        Real64 averageOSAF;
        int ErrorCode;
        bool StandBy;
        int InletNode;
        int OutletNode;
        int SecondaryInletNode;         // This is usually OA node feeding into the purge/secondary side
        int SecondaryOutletNode;        // This outlet node of the secondary side and inlet to the secondary fan
        Real64 FinalElectricalPower;    // Output fuel use in W
        Real64 FinalElectricalEnergy;   // Output fuel energy use in J
        Real64 InletMassFlowRate;       // Inlet is primary process air node at inlet to cooler
        Real64 InletVolumetricFlowRate; // Inlet is primary process air node at inlet to cooler
        Real64 InletTemp;
        Real64 InletWetBulbTemp;
        Real64 InletHumRat;
        Real64 InletEnthalpy;
        Real64 InletPressure;
        Real64 InletRH;
        Real64 OutletVolumetricFlowRate;
        Real64 OutletMassFlowRate; // Inlet is primary process air node at inlet to cooler
        Real64 PowerLossToAir;
        Real64 FanHeatTemp;
        Real64 OutletTemp;
        Real64 OutletWetBulbTemp;
        Real64 OutletHumRat;
        Real64 OutletEnthalpy;
        Real64 OutletPressure;
        Real64 OutletRH;
        Real64 SecInletMassFlowRate; // Inlet is primary process air node at inlet to cooler
        Real64 SecInletTemp;
        Real64 SecInletWetBulbTemp;
        Real64 SecInletHumRat;
        Real64 SecInletEnthalpy;
        Real64 SecInletPressure;
        Real64 SecInletRH;
        Real64 SecOutletVolumetricFlowRate;
        Real64 SecOutletMassFlowRate; // Inlet is primary process air node at inlet to cooler
        Real64 SecOutletTemp;
        Real64 SecOutletWetBulbTemp;
        Real64 SecOutletHumRat;
        Real64 SecOutletEnthalpy;
        Real64 SecOutletPressure;
        Real64 SecOutletRH;
        Real64 Wsa;
        Real64 SupplyVentilationAir;
        Real64 SupplyVentilationVolume;

        bool OutdoorAir;
        Real64 MinOA_Msa;
        int OARequirementsPtr; // Index to DesignSpecification:OutdoorAir object

        Real64 Tsa;
        int ModeCounter;
        bool CoolingRequested;
        bool HeatingRequested;
        bool VentilationRequested;
        bool DehumidificationRequested;
        bool HumidificationRequested;
        // non-initializer
        std::vector<int> Tsa_curve_pointer;
        std::vector<int> HRsa_curve_pointer;
        std::vector<int> Psa_curve_pointer;
        std::vector<CMode> OperatingModes;
        std::vector<CSetting> CurrentOperatingSettings;

        CSetting OptimalSetting;
        CSetting oStandBy;

        std::vector<CSetting> Settings;
        // methods
        int CurrentPrimaryMode();
        Real64 CurrentPrimaryRuntimeFraction();
        Real64 CalculatePartRuntimeFraction(Real64 MinOA_Msa,
                                            Real64 Mvent,
                                            Real64 RequestedCoolingLoad,
                                            Real64 RequestedHeatingLoad,
                                            Real64 SensibleRoomORZone,
                                            Real64 RequestedDehumidificationLoad,
                                            Real64 RequestedMoistureLoad,
                                            Real64 LatentRoomORZone);
        bool ParseMode(EnergyPlusData &state,
                       Array1D_string Alphas,
                       Array1D_string cAlphaFields,
                       Array1D<Real64> Numbers,
                       Array1D_string cNumericFields,
                       Array1D<bool> lAlphaBlanks,
                       std::string cCurrentModuleObject);
        void doStep(EnergyPlusData &state,
                    Real64 RequestedLoad,
                    Real64 ZoneHeatingLoad,
                    Real64 OutputRequiredToHumidify,
                    Real64 OutputRequiredToDehumidify,
                    Real64 DesignMinVR);
        void Initialize(int ZoneNumber);
        void InitializeModelParams();
        void ResetOutputs();
        bool MeetsSupplyAirTOC(EnergyPlusData &state, Real64 Tosa);
        bool MeetsSupplyAirRHOC(EnergyPlusData &state, Real64 Wosa);
        Real64 CheckVal_T(EnergyPlusData &state, Real64 T);
        Real64 CheckVal_W(EnergyPlusData &state, Real64 W, Real64 T, Real64 P); // pascals
        bool SetStandByMode(EnergyPlusData &state, CMode Mode0, Real64 Tosa, Real64 Wosa, Real64 Tra, Real64 Wra);
        Real64 CalculateTimeStepAverage(SYSTEMOUTPUTS val);
        int SetOperatingSetting(EnergyPlusData &state, CStepInputs StepIns);
        void DetermineCoolingVentilationOrHumidificationNeeds(CStepInputs &StepIns);

    private: // begin private section
        // number of times in a day it failed resulting in a warning.
        std::vector<int> SAT_OC_MetinMode_v;
        std::vector<int> SAHR_OC_MetinMode_v;
        bool WarnOnceFlag;

        int count_EnvironmentConditionsNotMet;
        int count_EnvironmentConditionsMetOnce;
        int count_SAHR_OC_MetOnce;
        int count_SAT_OC_MetOnce;
        int count_DidWeMeetLoad;
        int count_DidWeNotMeetLoad;

        bool optimal_EnvCondMet;
        bool RunningPeakCapacity_EnvCondMet;

        std::vector<Real64> PolygonXs;
        std::vector<Real64> PolygonYs;
    };
} // namespace HybridEvapCoolingModel
} // namespace EnergyPlus

#endif
