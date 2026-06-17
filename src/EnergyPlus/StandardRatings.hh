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

#ifndef StandardRatings_hh_INCLUDED
#define StandardRatings_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1A.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitOperatingMode.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>

// C++ Headers
#include <string_view>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace StandardRatings {

    Real64 constexpr ConvFromSIToIP(3.412141633); // Conversion from SI to IP [3.412 Btu/hr-W]

    Real64 constexpr HeatingOutdoorCoilInletAirDBTempRated(8.33); // Outdoor air dry-bulb temp in degrees C (47F)
    // Test H1 or rated (low and High Speed) Std. AHRI 210/240

    Real64 constexpr HeatingOutdoorCoilInletAirDBTempH2Test(1.67); // Outdoor air dry-bulb temp in degrees C (35F)
    // Test H2 (low and High Speed) Std. AHRI 210/240

    Real64 constexpr HeatingOutdoorCoilInletAirDBTempH3Test(-8.33); // Outdoor air dry-bulb temp in degrees C (17F)
    // Test H3 (low and High Speed) Std. AHRI 210/240

    static constexpr std::string_view AHRI2017FOOTNOTE = R"html(<p>ANSI/AHRI ratings account for supply air fan heat and electric power.</p>
<ol>
  <li>EnergyPlus object type.</li>
  <li>Capacity less than 65K Btu/h (19050 W) - calculated as per AHRI Standard 210/240-2017.<br>
      Capacity of 65K Btu/h (19050 W) to less than 135K Btu/h (39565 W) - calculated as per AHRI Standard 340/360-2007.<br>
      Capacity from 135K (39565 W) to 250K Btu/hr (73268 W) - calculated as per AHRI Standard 365-2009 - Ratings not yet supported in EnergyPlus.
  </li>
  <li>SEER (User) is calculated using user-input PLF curve and cooling coefficient of degradation.<br>
      SEER (Standard) is calculated using the default PLF curve and cooling coefficient of degradation from the appropriate AHRI standard.
  </li>
</ol>)html";

    static constexpr std::string_view AHRI2023FOOTNOTE = R"html(<p>ANSI/AHRI ratings account for supply air fan heat and electric power.</p>
<ol>
  <li>EnergyPlus object type.</li>
  <li>
    Capacity less than 65K Btu/h (19050 W) - calculated as per AHRI Standard 210/240-2023.<br>
    Capacity of 65K Btu/h (19050 W) to less than 135K Btu/h (39565 W) - calculated as per AHRI Standard 340/360-2022.<br>
    Capacity from 135K (39565 W) to 250K Btu/hr (73268 W) - calculated as per AHRI Standard 365-2009 - Ratings not yet supported in EnergyPlus.
  </li>
  <li>
    SEER2 (User) is calculated using user-input PLF curve and cooling coefficient of degradation.<br>
    SEER2 (Standard) is calculated using the default PLF curve and cooling coefficient of degradation from the appropriate AHRI standard.
  </li>
  <li>Value for the Full Speed of the coil.</li>
</ol>)html";

    // Defrost strategy (heat pump only)
    enum class DefrostStrat
    {
        Invalid = -1,
        ReverseCycle, // uses reverse cycle defrost strategy
        Resistive,    // uses electric resistance heater for defrost
        Num,
    };

    static constexpr std::array<std::string_view, static_cast<int>(DefrostStrat::Num)> DefrostStratUC = {"REVERSECYCLE", "RESISTIVE"};

    // Defrost control  (heat pump only)
    enum class HPdefrostControl : int
    {
        Invalid = -1,
        Timed,    // defrost cycle is timed
        OnDemand, // defrost cycle occurs only when required
        Num
    };

    static constexpr std::array<std::string_view, static_cast<int>(HPdefrostControl::Num)> HPdefrostControlUC = {"TIMED", "ONDEMAND"};

    enum class AhriChillerStd
    {
        Invalid = -1,
        AHRI550_590,
        AHRI551_591,
        Num
    };

    constexpr std::array<std::string_view, static_cast<int>(AhriChillerStd::Num)> AhriChillerStdNamesUC{"AHRI550_590", "AHRI551_591"};

    // Functions

    void CalcChillerIPLV(EnergyPlusData &state,
                         std::string const &ChillerName,               // Name of Chiller for which IPLV is calculated
                         DataPlant::PlantEquipmentType ChillerType,    // Type of Chiller - EIR or Reformulated EIR
                         Real64 const RefCap,                          // Reference capacity of chiller [W]
                         Real64 const RefCOP,                          // Reference coefficient of performance [W/W]
                         DataPlant::CondenserType const CondenserType, // Type of Condenser - Air Cooled, Water Cooled or Evap Cooled
                         int const CapFTempCurveIndex,                 // Index for the total cooling capacity modifier curve
                         int const EIRFTempCurveIndex,                 // Index for the energy input ratio modifier curve
                         int const EIRFPLRCurveIndex,                  // Index for the EIR vs part-load ratio curve
                         Real64 const MinUnloadRat,                    // Minimum unloading ratio
                         Real64 &IPLVSI,                               // IPLV.SI determined using AHRI Std 551/591 (SI)
                         Real64 &IPLVIP,                               // IPLV.IP determined using AHRI Std 550/590 (IP)
                         ObjexxFCL::Optional<Real64 const> CondVolFlowRate,
                         ObjexxFCL::Optional_int_const CondLoopNum,
                         ObjexxFCL::Optional<Real64 const> OpenMotorEff);

    void ReportChillerIPLV(EnergyPlusData &state,
                           std::string const &ChillerName,            // Name of Chiller for which IPLV is calculated
                           DataPlant::PlantEquipmentType ChillerType, // Type of Chiller - EIR or Reformulated EIR
                           Real64 const IPLVValueSI,                  // IPLV value in SI units {W/W}
                           Real64 const IPLVValueIP                   // IPLV value in IP units {Btu/W-h}
    );

    void CheckCurveLimitsForIPLV(EnergyPlusData &state,
                                 std::string const &ChillerName,               // Name of Chiller
                                 DataPlant::PlantEquipmentType ChillerType,    // Type of Chiller - EIR or ReformulatedEIR
                                 DataPlant::CondenserType const CondenserType, // Type of Condenser - Air Cooled, Water Cooled or Evap Cooled
                                 int const CapFTempCurveIndex,                 // Index for the total cooling capacity modifier curve
                                 int const EIRFTempCurveIndex                  // Index for the energy input ratio modifier curve
    );

    void CalcDXCoilStandardRating(
        EnergyPlusData &state,
        std::string const &DXCoilName,                             // Name of DX coil for which HSPF is calculated
        HVAC::CoilType coilType,                                   // Integer Type of DX coil - heating or cooling
        int const ns,                                              // Number of compressor speeds
        Array1A<Real64> const RatedTotalCapacity,                  // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedCOP,                            // Reference coefficient of performance [W/W]
        Array1A_int const CapFFlowCurveIndex,                      // Index for the capacity as a function of flow fraction modifier curve
        Array1A_int const CapFTempCurveIndex,                      // Index for the capacity as a function of temperature modifier curve
        Array1A_int const EIRFFlowCurveIndex,                      // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                      // Index for the EIR as a function of temperature modifier curve
        Array1A_int const PLFFPLRCurveIndex,                       // Index for the PLF vs part-load ratio curve
        Array1A<Real64> const RatedAirVolFlowRate,                 // Reference air flow rate of DX coil [m3/s]
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput, // Reference fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInputSEER2,
        Array1D<DataHeatBalance::RefrigCondenserType> CondenserType,
        ObjexxFCL::Optional_int_const RegionNum =
            _, // Region number for calculating HSPF of single speed DX heating coil //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional<Real64 const> MinOATCompressor =
            _, // Minimum OAT for heat pump compressor operation [C] //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn =
            _, // The outdoor temperature when the compressor is automatically turned //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank =
            _, // Flag used to determine low temperature cut out factor //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl =
            _,                                               // defrost control; 1=timed, 2=on-demand //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional_bool_const ASHRAE127StdRprt = _, // true if user wishes to report ASHRAE 127 standard ratings
        ObjexxFCL::Optional<Real64 const> GrossRatedTotalCoolingCapacityVS =
            _,                                                             // Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}
        ObjexxFCL::Optional<Real64 const> RatedVolumetricAirFlowRateVS = _ //  Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}
    );

    void CalcTwoSpeedDXCoilRating(EnergyPlusData &state,
                                  std::string const &DXCoilName,
                                  HVAC::CoilType const coilType,
                                  Array1A<Real64> const &RatedTotalCapacity,
                                  Real64 const RatedTotCap2,
                                  Array1A<Real64> const &RatedCOP,
                                  Real64 const RatedCOP2,
                                  Array1A_int const &CapFFlowCurveIndex, // only hs
                                  Array1A_int const &CapFTempCurveIndex,
                                  int const CCapFTemp2,
                                  Array1A_int const &EIRFFlowCurveIndex, // only hs
                                  Array1A_int const &EIRFTempCurveIndex,
                                  int const EIRFTemp2,
                                  Array1A<Real64> const &RatedAirVolFlowRate,
                                  Real64 const RatedAirVolFlowRate2,
                                  Array1A<Real64> const &FanPowerPerEvapAirFlowRate_2023,
                                  Array1A<Real64> const &FanPowerPerEvapAirFlowRate_2023_LowSpeed,
                                  Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType,
                                  int const PLFFPLRCurveIndex);

    std::map<std::string, Real64> TwoSpeedDXCoilStandardRatings(EnergyPlusData &state,
                                                                std::string const &DXCoilName,
                                                                HVAC::CoilType const coilType,
                                                                Array1A<Real64> const &RatedTotalCapacity,
                                                                Real64 const &RatedTotCap2,
                                                                Array1A<Real64> const &RatedCOP,
                                                                Real64 const &RatedCOP2,
                                                                Array1A_int const &CapFFlowCurveIndex, // only hs
                                                                Array1A_int const &CapFTempCurveIndex,
                                                                int const &CCapFTemp2,
                                                                Array1A_int const &EIRFFlowCurveIndex, // only hs
                                                                Array1A_int const &EIRFTempCurveIndex,
                                                                int const &EIRFTemp2,
                                                                Array1A<Real64> const &RatedAirVolFlowRate,
                                                                Real64 const &RatedAirVolFlowRate2,
                                                                Array1A<Real64> const &FanPowerPerEvapAirFlowRate_2023,
                                                                Array1A<Real64> const &FanPowerPerEvapAirFlowRate_2023_LowSpeed,
                                                                Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType,
                                                                int const &PLFFPLRCurveIndex);

    // Real64 NetHeatingCapRated <- Net Heating Coil capacity at Rated conditions,
    // Real64 NetHeatingCapH3Test <- Net Heating Coil capacity at H3 test conditions
    // Real64 HSPF <- seasonale energy efficiency ratio of multi speed DX cooling coil
    // ANSI/AHRI 210/240 Standard 2023
    // Real64 NetHeatingCapRated2023 <- Net Heating Coil capacity at Rated conditions,
    // Real64 NetHeatingCapH3Test2023 <- Net Heating Coil capacity at H3 test conditions
    // Real64 HSPF_2023 <- seasonale energy efficiency ratio of multi speed DX cooling coil
    std::map<std::string, Real64> SingleSpeedDXHeatingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &coilName,
        HVAC::CoilType const coilType,                                 // Type of DX coil for which HSPF is calculated
        Real64 const RatedTotalCapacity,                               // Reference capacity of DX coil [W]
        Real64 const RatedCOP,                                         // Reference coefficient of performance [W/W]
        int const CapFFlowCurveIndex,                                  // Index for the capacity as a function of flow fraction modifier curve
        int const CapFTempCurveIndex,                                  // Index for the capacity as a function of temperature modifier curve
        int const EIRFFlowCurveIndex,                                  // Index for the EIR as a function of flow fraction modifier curve
        int const EIRFTempCurveIndex,                                  // Index for the EIR as a function of temperature modifier curve
        Real64 const RatedAirVolFlowRate,                              // Rated air volume flow rate [m3/s]
        Real64 const FanPowerPerEvapAirFlowRateFromInput,              // 2017 Fan power per air volume flow rate [W/(m3/s)]
        Real64 const FanPowerPerEvapAirFlowRateFromInput_2023,         // 2023 Fan power per air volume flow rate [W/(m3/s)]
        ObjexxFCL::Optional_int_const RegionNum = _,                   // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor = _,        // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn = _,      // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank = _, // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl = _ // defrost control; 1=timed, 2=on-demand
    );

    Real64 SingleSpeedHeatingHSPF(
        const Real64 NetHeatingCapRated,                    // Net Heating Coil capacity at Rated conditions,
        ObjexxFCL::Optional_int_const RegionNum,            // Region number for calculating HSPF of single speed DX heating coil
        const Real64 NetHeatingCapH3Test,                   // Net Heating Coil capacity at H3 test conditions
        const Real64 ElecPowerH3Test,                       // Total system power at H3 test conditions accounting for supply fan heat [W]
        const Real64 ElecPowerRated,                        // Total system power at Rated conditions accounting for supply fan heat [W]
        const Real64 NetHeatingCapH2Test,                   // Net Heating Coil capacity at H2 test conditions accounting for supply fan heat [W]
        const Real64 ElecPowerH2Test,                       // Total system power at H2 test conditions accounting for supply fan heat [W]
        ObjexxFCL::Optional<Real64 const> MinOATCompressor, // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,   // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn,        // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional<const HPdefrostControl> DefrostControl); // defrost control; 1=timed, 2=on-demand

    Real64 SingleSpeedHeatingHSPF2(
        const Real64 NetHeatingCapRated_2023,                        // Net Heating Coil capacity at Rated conditions,
        ObjexxFCL::Optional_int_const RegionNum,                     // Region number for calculating HSPF of single speed DX heating coil
        const Real64 NetHeatingCapH3Test_2023,                       // Net Heating Coil capacity at H3 test conditions
        const Real64 ElecPowerH3Test2023,                            // Total system power at H3 test conditions accounting for supply fan heat [W]
        const Real64 ElecPowerRated2023,                             // Total system power at Rated conditions accounting for supply fan heat [W]
        const Real64 NetHeatingCapH2Test2023,                        // (for 2023 Standard) Net Heating Coil capacity at H2 test conditions
                                                                     // accounting for supply fan heat [W]
        const Real64 ElecPowerH2Test2023,                            // Total system power at H2 test conditions accounting for supply fan heat [W]
        ObjexxFCL::Optional<Real64 const> MinOATCompressor,          // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,   // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn,        // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional<const HPdefrostControl> DefrostControl); // defrost control; 1=timed, 2=on-demand

    // Real64 NetCoolingCapRated -> net cooling capacity of single speed DX cooling coil
    // Real64 SEER_User ->  seasonal energy efficiency ratio of single speed DX cooling coil, from user PLF curve
    // Real64 SEER_Standard -> seasonal energy efficiency ratio of single speed DX cooling coil,
    //                        from AHRI Std 210/240-2008 default PLF curve and C_D value
    // Real64 EER -> energy efficiency ratio of single speed DX cooling coil
    // Real64 IEER -> Integareted energy efficiency ratio of single speed DX cooling coil

    // #Calculations as per ANSI/AHRI 210/240 Standard 2023
    // Real64 NetCoolingCapRated_2023
    // Real64 EER_2023
    // Real64 IEER_2023
    // Real64 SEER2_User
    // Real64 SEER2_Standard
    std::map<std::string, Real64> SingleSpeedDXCoolingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName,                         // Name of DX coil for which HSPF is calculated
        HVAC::CoilType const coilType,                         // Type of DX coil - heating or cooling
        int const CapFTempCurveIndex,                          // Index for the capacity as a function of temperature modifier curve
        int const CapFFlowCurveIndex,                          // Index for the capacity as a function of flow fraction modifier curve
        int const EIRFTempCurveIndex,                          // Index for the EIR as a function of temperature modifier curve
        int const EIRFFlowCurveIndex,                          // Index for the EIR as a function of flow fraction modifier curve
        int const PLFFPLRCurveIndex,                           // Index for the EIR vs part-load ratio curve
        Real64 const RatedTotalCapacity,                       // Rated gross total cooling capacity
        Real64 const RatedCOP,                                 // Rated gross COP
        Real64 const RatedAirVolFlowRate,                      // air flow rate through the coil at rated condition
        Real64 const FanPowerPerEvapAirFlowRateFromInput,      // 2017 Rated Fan power per air volume flow rate through the evaporator coil
        Real64 const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        DataHeatBalance::RefrigCondenserType CondenserType);

    std::tuple<Real64, Real64> IEERSingleSpeedCooling(
        EnergyPlusData &state,
        const int CapFTempCurveIndex,            // Index for the capacity as a function of temperature modifier curve
        const Real64 RatedTotalCapacity,         // Rated gross total cooling capacity
        const Real64 TotCapFlowModFac,           // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        const Real64 FanPowerPerEvapAirFlowRate, // either of the 2017 or 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        const Real64 RatedAirVolFlowRate,        // air flow rate through the coil at rated condition
        const int EIRFTempCurveIndex,            // Index for the EIR as a function of temperature modifier curve
        const Real64 RatedCOP,                   // Rated gross COP
        const Real64 EIRFlowModFac);             // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]

    // Calculations as per AHRI 340/360 Standard 2022
    // Real64 NetCoolingCapRated -> Net cooling capacity at high speed for CurveFit DX Cooling Coil
    // Real64 IEER -> Integareted energy efficiency ratio of CurveFit DX cooling coil
    // Real64 EER -> Energy efficiency ratio at high speed for CurveFit DX cooling coil
    std::tuple<Real64, Real64, Real64>
    IEERCalculationCurveFit(EnergyPlusData &state, HVAC::CoilType const coilType, EnergyPlus::CoilCoolingDXCurveFitOperatingMode operatingMode);

    // NetCoolingCapRatedMaxSpeed2023 --> net cooling capacity at maximum speed
    // SEER2_User --> seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
    // SEER2_Standard --> seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2023 default PLF
    // curve and  C_D value
    // EER2 --> Energy Efficiency Ratio from std 210/240-2023 at AFull
    std::tuple<Real64, Real64, Real64, Real64>
    SEER2CalculationCurveFit(EnergyPlusData &state, HVAC::CoilType const coilType, EnergyPlus::CoilCoolingDXCurveFitOperatingMode operatingMode);

    // Calculations as per AHRI 340/360 Standard 2022
    // Real64 NetCoolingCapRated -> Net cooling capacity at high speed for Variable Speed DX Cooling Coil
    // Real64 IEER -> Integareted energy efficiency ratio of Variable Speed DX cooling coil
    // Real64 EER -> Energy efficiency ratio at high speed for Variable Speed DX cooling coil
    std::tuple<Real64, Real64, Real64> IEERCalculationVariableSpeed(
        EnergyPlusData &state,
        HVAC::CoilType coilType,
        int const nsp,
        Array1A_int const &CapFTempCurveIndex,
        Array1A<Real64> const &RatedTotalCapacity, // Rated gross total cooling capacity
        Array1A_int const &CapFFlowCurveIndex,
        Array1A<Real64> const &FanPowerPerEvapAirFlowRate, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        Array1A<Real64> const &RatedAirVolFlowRate,
        Array1A_int const &EIRFTempCurveIndex,
        Array1A<Real64> const &RatedCOP, // Reference coefficient of performance [W/W]
        Array1A_int const &EIRFFlowCurveIndex,
        DataHeatBalance::RefrigCondenserType const CondenserType); // Type of condenser user by the DX Cooling Coil

    // Calculations as per AHRI 340/360 Standard 2022
    // Real64 NetCoolingCapRated -> Net cooling capacity at high speed for MultiSpeed DX Cooling Coil
    // Real64 IEER -> Integareted energy efficiency ratio of MultiSpeed DX cooling coil
    // Real64 EER -> Energy efficiency ratio at high speed for MultiSpeed DX cooling coil
    std::tuple<Real64, Real64, Real64> IEERCalculationMultiSpeed(
        EnergyPlusData &state,
        HVAC::CoilType coilType,
        int const nsp, // Number of Speeds
        Array1A_int const &CapFTempCurveIndex,
        Array1A<Real64> const &RatedTotalCapacity, // Rated gross total cooling capacity
        Array1A_int const &CapFFlowCurveIndex,
        Array1A<Real64> const &FanPowerPerEvapAirFlowRate, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        Array1A<Real64> const &RatedAirVolFlowRate,
        Array1A_int const &EIRFTempCurveIndex,
        Array1A<Real64> const &RatedCOP, // Reference coefficient of performance [W/W]
        Array1A_int const &EIRFFlowCurveIndex,
        Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType); // Type of condenser user by the DX Cooling Coil

    // Calculations as per AHRI 340/360 Standard 2022
    // Real64 NetCoolingCapRated -> Net cooling capacity at high speed for Two Speed DX Cooling Coil
    // Real64 IEER -> Integareted energy efficiency ratio of Two Speed DX cooling coil
    // Real64 EER -> Energy efficiency ratio at high speed for Two Speed DX cooling coil
    std::tuple<Real64, Real64, Real64> IEERCalculationTwoSpeed(
        EnergyPlusData &state,
        HVAC::CoilType const coilType,                                      // Type of DX coil
        Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType, // Type of condenser user by the DX Cooling Coil
        Array1A_int const &CapFTempCurveIndex,
        Array1A<Real64> const &RatedTotalCapacity,         // Rated gross total cooling capacity
        Array1A_int const &HSCCapFFlowCurveIndex,          //  | Only for HIGH SPEED
        Array1A<Real64> const &FanPowerPerEvapAirFlowRate, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        Array1A<Real64> const &RatedAirVolFlowRate,
        Array1A_int const &EIRFTempCurveIndex,
        Array1A<Real64> const &RatedCOP,        // Reference coefficient of performance [W/W]
        Array1A_int const &HSEIRFFlowCurveIndex //  | Only for HIGH SPEED
    );

    // Calculations as per AHRI 340/360 Standard 2022
    // Real64 NetCoolingCapRated -> Net cooling capacity for Single Speed DX Cooling Coil
    // Real64 IEER -> Integareted energy efficiency ratio for Single Speed DX cooling coil
    // Real64 EER -> Energy efficiency ratio for Single Speed DX cooling coil
    std::tuple<Real64, Real64, Real64> IEERCalculationSingleSpeed(
        EnergyPlus::EnergyPlusData &state,
        HVAC::CoilType coilType,
        const int CapFTempCurveIndex,            // Index for the capacity as a function of temperature modifier curve
        const Real64 RatedTotalCapacity,         // Rated gross total cooling capacity
        const Real64 TotCapFlowModFac,           // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        const Real64 FanPowerPerEvapAirFlowRate, // either of the 2017 or 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        const Real64 RatedAirVolFlowRate,        // air flow rate through the coil at rated condition
        const int EIRFTempCurveIndex,            // Index for the EIR as a function of temperature modifier curve
        const Real64 RatedCOP,                   // Rated gross COP
        const Real64 EIRFlowModFac,              // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        DataHeatBalance::RefrigCondenserType const CondenserType); // Type of condenser user by the DX Cooling Coil

    Real64 EERSingleSpeedCooling(
        EnergyPlus::EnergyPlusData &state,
        const int CapFTempCurveIndex,            // Index for the capacity as a function of temperature modifier curve
        const Real64 RatedTotalCapacity,         // Rated gross total cooling capacity
        const Real64 TotCapFlowModFac,           // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        const Real64 FanPowerPerEvapAirFlowRate, // either of the 2017 or 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        const Real64 RatedAirVolFlowRate,        // air flow rate through the coil at rated condition
        const int EIRFTempCurveIndex,            // Index for the EIR as a function of temperature modifier curve
        const Real64 RatedCOP,                   // Rated gross COP
        const Real64 EIRFlowModFac);             // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]

    std::tuple<Real64, Real64> SEERSingleStageCalculation(
        EnergyPlusData &state,
        const int CapFTempCurveIndex,               // Index for the capacity as a function of temperature modifier curve
        const Real64 RatedTotalCapacity,            // Rated gross total cooling capacity
        const Real64 TotCapFlowModFac,              // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        const int EIRFTempCurveIndex,               // Index for the EIR as a function of temperature modifier curve
        Real64 EIRFlowModFac,                       // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        const int EIRFFlowCurveIndex,               // Index for the EIR as a function of flow fraction modifier curve
        const Real64 RatedCOP,                      // Rated gross COP
        const Real64 FanPowerPerEvapAirFlowRate,    // either of the 2017 or 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        const Real64 RatedAirVolFlowRate,           // air flow rate through the coil at rated condition
        const int PLFFPLRCurveIndex,                // Index for the EIR vs part-load ratio curve
        Real64 const CyclicDegradationCoefficient); // Cyclic Degradation Coefficient either for ANSI/AHRI 2017 or 2023

    void DXCoolingCoilDataCenterStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName,                    // Name of DX coil for which HSPF is calculated
        HVAC::CoilType const coilType,                    // Type of DX coil - heating or cooling
        int const CapFTempCurveIndex,                     // Index for the capacity as a function of temperature modifier curve
        int const CapFFlowCurveIndex,                     // Index for the capacity as a function of flow fraction modifier curve
        int const EIRFTempCurveIndex,                     // Index for the EIR as a function of temperature modifier curve
        int const EIRFFlowCurveIndex,                     // Index for the EIR as a function of flow fraction modifier curve
        int const PLFFPLRCurveIndex,                      // Index for the EIR vs part-load ratio curve
        Real64 const RatedTotalCapacity,                  // Rated gross total cooling capacity
        Real64 const RatedCOP,                            // Rated gross COP
        Real64 const RatedAirVolFlowRate,                 // air flow rate through the coil at rated condition
        Real64 const FanPowerPerEvapAirFlowRateFromInput, // Fan power per air volume flow rate through the evaporator coil
        Array1D<Real64> &NetCoolingCapRated,              // net cooling capacity of single speed DX cooling coil
        Array1D<Real64> &TotElectricPowerRated            // total electric power including supply fan
    );

    // NetCoolingCapRatedMaxSpeed --> net cooling capacity at maximum speed
    // SEER_User --> seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
    // SEER_Standard --> seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2008 default PLF
    // curve and C_D value
    // EER --> Energy Efficiency Ratio
    std::tuple<Real64, Real64, Real64, Real64>
    MultiSpeedDXCoolingCoilSEER(EnergyPlusData &state,
                                int const nsp,                            // Number of compressor speeds
                                Array1A_int const CapFFlowCurveIndex,     // Index for the capacity as a function of flow fraction modifier curve
                                Array1A<Real64> const RatedTotalCapacity, // Reference capacity of DX coil [W]
                                Array1A_int const CapFTempCurveIndex,     // Index for the capacity as a function of temperature modifier curve
                                Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput, // 2017 rated fan power per evap air flow rate [W/(m3/s)]
                                Array1A<Real64> const RatedAirVolFlowRate,                 // Reference air flow rate of DX coil [m3/s]
                                Array1A_int const EIRFFlowCurveIndex, // Index for the EIR as a function of flow fraction modifier curve
                                Array1A<Real64> const RatedCOP,       // Reference coefficient of performance [W/W]
                                Array1A_int EIRFTempCurveIndex,       // Index for the EIR as a function of temperature modifier curve
                                Array1A_int const PLFFPLRCurveIndex); // Index for the PLF vs part-load ratio curve

    // NetCoolingCapRatedMaxSpeed2023 --> net cooling capacity at maximum(High) speed
    // SEER2_User --> seasonal energy efficiency ratio of Two speed DX cooling coil, from user-input PLF curve and C_D value
    // SEER2_Standard --> seasonal energy efficiency ratio of Two speed DX cooling coil, from AHRI Std 210/240-2023 default PLF
    // curve and  C_D value
    // EER2 --> Energy Efficiency Ratio of Two speed DX Cooling coil, from AHRI std 210/240-2023
    std::tuple<Real64, Real64, Real64, Real64> TwoSpeedDXCoolingCoilSEER2(
        EnergyPlusData &state,
        // int const nsp,                                                  // Number of compressor speeds | 2
        Array1A_int const CapFFlowCurveIndex,                           // Index for the capacity as a function of flow fraction modifier curve
        Array1A<Real64> const RatedTotalCapacity,                       // Reference capacity of DX coil [W]
        Array1A_int const CapFTempCurveIndex,                           // Index for the capacity as a function of temperature modifier curve
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const RatedAirVolFlowRate,                      // Reference air flow rate of DX coil [m3/s]
        Array1A_int const EIRFFlowCurveIndex,                           // Index for the EIR as a function of flow fraction modifier curve
        Array1A<Real64> const RatedCOP,                                 // Reference coefficient of performance [W/W]
        Array1A_int EIRFTempCurveIndex,                                 // Index for the EIR as a function of temperature modifier curve
        Array1A_int const PLFFPLRCurveIndex);                           // Index for the PLF vs part-load ratio curve

    // NetCoolingCapRatedMaxSpeed2023 --> net cooling capacity at maximum speed
    // SEER2_User --> seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
    // SEER2_Standard --> seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2023 default PLF
    // curve and  C_D value
    // EER2 --> Energy Efficiency Ratio from std 210/240-2023 at AFull
    std::tuple<Real64, Real64, Real64, Real64> MultiSpeedDXCoolingCoilSEER2(
        EnergyPlusData &state,
        int const nsp,                                                  // Number of compressor speeds
        Array1A_int const CapFFlowCurveIndex,                           // Index for the capacity as a function of flow fraction modifier curve
        Array1A<Real64> const RatedTotalCapacity,                       // Reference capacity of DX coil [W]
        Array1A_int const CapFTempCurveIndex,                           // Index for the capacity as a function of temperature modifier curve
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const RatedAirVolFlowRate,                      // Reference air flow rate of DX coil [m3/s]
        Array1A_int const EIRFFlowCurveIndex,                           // Index for the EIR as a function of flow fraction modifier curve
        Array1A<Real64> const RatedCOP,                                 // Reference coefficient of performance [W/W]
        Array1A_int EIRFTempCurveIndex,                                 // Index for the EIR as a function of temperature modifier curve
        Array1A_int const PLFFPLRCurveIndex);                           // Index for the PLF vs part-load ratio curve

    // NetCoolingCapRatedMaxSpeed2023 --> net cooling capacity at maximum speed
    // SEER2_User --> seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
    // SEER2_Standard --> seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2023 default PLF
    // curve and  C_D value
    // EER2 --> Energy Efficiency Ratio from std 210/240-2023 at AFull
    std::tuple<Real64, Real64, Real64, Real64> VariableSpeedDXCoolingCoilSEER2(
        EnergyPlusData &state,
        int const nsp,                                                  // Number of compressor speeds
        Array1A_int const CapFFlowCurveIndex,                           // Index for the capacity as a function of flow fraction modifier curve
        Array1A<Real64> const RatedTotalCapacity,                       // Reference capacity of DX coil [W]
        Array1A_int const CapFTempCurveIndex,                           // Index for the capacity as a function of temperature modifier curve
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const RatedAirVolFlowRate,                      // Reference air flow rate of DX coil [m3/s]
        Array1A_int const EIRFFlowCurveIndex,                           // Index for the EIR as a function of flow fraction modifier curve
        Array1A<Real64> const RatedCOP,                                 // Reference coefficient of performance [W/W]
        Array1A_int const EIRFTempCurveIndex,                           // Index for the EIR as a function of temperature modifier curve
        Array1A_int const PLFFPLRCurveIndex);                           // Index for the PLF vs part-load ratio curve

    // Real64 NetCoolingCapRatedMaxSpeed -> net cooling capacity at maximum speed
    // Real64 SEER_User -> seasonal energy efficiency ratio of variable speed DX cooling coil, from user PLF curve
    // Real64 SEER_Standard -> seasonal energy efficiency ratio of variable speed DX cooling coil, from AHRI Std 210/240-2008 default PLF curve and
    //                        default C_D value
    // Ratings based on ANSI/AHRI 210.240 2023 Standard
    // Real64 NetCoolingCapRatedMaxSpeed -> net cooling capacity at maximum speed
    // Real64 SEER_User -> seasonal energy efficiency ratio of variable speed DX cooling coil, from user PLF curve
    // Real64 SEER_Standard -> seasonal energy efficiency ratio of variable speed DX cooling coil, from AHRI Std 210/240-2008 default PLF curve and
    //                       default C_D value
    std::map<std::string, Real64> VariableSpeedDXCoolingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName, // Name of DX coil for which standard ratings are calculated
        HVAC::CoilType const coilType,
        Array1A_int const CapFTempCurveIndex,                           // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                           // Index for the capacity as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                           // Index for the EIR as a function of temperature modifier curve
        Array1A_int const EIRFFlowCurveIndex,                           // Index for the EIR as a function of flow fraction modifier curve
        int const PLFFPLRCurveIndex,                                    // Index for the PLF vs part-load ratio curve
        Array1A<Real64> const RatedTotalCapacity,                       // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedCOP,                                 // Reference coefficient of performance [W/W]
        Array1A<Real64> const RatedAirVolFlowRate,                      // Reference air flow rate of DX coil [m3/s]
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput,      // 2017 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        int const nsp,                                                  // Number of compressor speeds
        DataHeatBalance::RefrigCondenserType const &CondenserType,
        Real64 VSGrossRatedTotalCoolingCapacity,
        Real64 VSRatedVolumetricAirFlowRate);

    // Real64 NetCoolingCapRatedMaxSpeed -> net cooling capacity at maximum speed
    // Real64 SEER_User -> seasonal energy efficiency ratio of multi speed DX cooling coil, from user PLF curve
    // Real64 SEER_Standard -> seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2008 default PLF curve and
    //                        default C_D value
    // Ratings based on ANSI/AHRI 210.240 2023 Standard
    // Real64 NetCoolingCapRatedMaxSpeed -> net cooling capacity at maximum speed
    // Real64 SEER_User -> seasonal energy efficiency ratio of multi speed DX cooling coil, from user PLF curve
    // Real64 SEER_Standard -> seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2008 default PLF curve and
    //                       default C_D value
    std::map<std::string, Real64> MultiSpeedDXCoolingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName,                                  // Name of DX coil for which standard Ratings are calculated
        HVAC::CoilType const coilType,                                  // Type of DX coil for which HSPF is calculated
        Array1A_int const CapFTempCurveIndex,                           // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                           // Index for the capacity as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                           // Index for the EIR as a function of temperature modifier curve
        Array1A_int const EIRFFlowCurveIndex,                           // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const PLFFPLRCurveIndex,                            // Index for the PLF vs part-load ratio curve
        Array1A<Real64> const RatedTotalCapacity,                       // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedCOP,                                 // Reference coefficient of performance [W/W]
        Array1A<Real64> const RatedAirVolFlowRate,                      // Reference air flow rate of DX coil [m3/s]
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput,      // 2017 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        int const nsp,                                                  // Number of compressor speeds
        Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType);

    // Real64 NetHeatingCapRatedHighTemp --> net heating capacity at maximum speed and High Temp
    // Real64 NetHeatingCapRatedLowTemp --> net heating capacity at maximum speed and low Temp
    // Real64 HSPF --> seasonale energy efficiency ratio of multi speed DX cooling coil | 2017
    std::tuple<Real64, Real64, Real64> MultiSpeedDXHeatingCoilHSPF(
        EnergyPlusData &state,
        int const nsp,                                                   // Number of compressor speed
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRateInput,         // 2017 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A_int const CapFTempCurveIndex,                            // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                            // Index for the capacity as a function of flow fraction modifier curve
        Array1A<Real64> const RatedTotalCapacity,                        // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedAirVolFlowRate,                       // Reference air flow rate of DX coil [m3/s]
        Array1A_int const EIRFFlowCurveIndex,                            // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                            // Index for the EIR as a function of temperature modifier curve
        Array1A<Real64> const RatedCOP,                                  // Reference coefficient of performance [W/W]
        ObjexxFCL::Optional_int_const RegionNum = _,                     // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor = _,          // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn = _,        // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank = _,   // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl = _); // defrost control; 1=timed, 2=on-demand

    // Real64 NetHeatingCapRatedHighTemp_2023 --> net heating capacity at maximum speed and High Temp
    // Real64 NetHeatingCapRatedLowTemp_2023 --> net heating capacity at maximum speed and low Temp
    // Real64 HSPF2_2023 --> seasonale energy efficiency ratio of multi speed DX cooling coil | 2023
    std::tuple<Real64, Real64, Real64> MultiSpeedDXHeatingCoilHSPF2(
        EnergyPlusData &state,
        int const nsp,                                                // Number of compressor speed
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRateInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A_int const CapFTempCurveIndex,                         // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                         // Index for the capacity as a function of flow fraction modifier curve
        Array1A<Real64> const RatedTotalCapacity,                     // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedAirVolFlowRate,                    // Reference air flow rate of DX coil [m3/s]
        Array1A_int const EIRFFlowCurveIndex,                         // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                         // Index for the EIR as a function of temperature modifier curve
        Array1A<Real64> const RatedCOP,                               // Reference coefficient of performance [W/W]
        ObjexxFCL::Optional_int_const RegionNum,                      // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor,           // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn,         // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,    // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl);  // defrost control; 1=timed, 2=on-demand

    // Real64 NetHeatingCapRatedHighTemp --> net heating capacity at maximum speed and High Temp
    // Real64 NetHeatingCapRatedLowTemp --> net heating capacity at maximum speed and low Temp
    // Real64 HSPF --> seasonale energy efficiency ratio of multi speed DX cooling coil
    // AHRI 210/240 2023 Std. Ratings
    // Real64 NetHeatingCapRatedHighTemp2023 --> net heating capacity at maximum speed and High Temp
    // Real64 NetHeatingCapRatedLowTemp2023 --> net heating capacity at maximum speed and low Temp
    // Real64 HSPF2_2023 --> seasonale energy efficiency ratio of multi speed DX cooling coil
    std::map<std::string, Real64> MultiSpeedDXHeatingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName,                                 // Name of DX coil for which HSPF is calculated
        HVAC::CoilType const coilType,                                 // Type of DX coil for which HSPF is calculated
        Array1A_int const CapFTempCurveIndex,                          // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                          // Index for the capacity as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                          // Index for the EIR as a function of temperature modifier curve
        Array1A_int const EIRFFlowCurveIndex,                          // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const PLFFPLRCurveIndex,                           // Index for the PLF vs part-load ratio curve
        Array1A<Real64> const RatedTotalCapacity,                      // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedCOP,                                // Reference coefficient of performance [W/W]
        Array1A<Real64> const RatedAirVolFlowRate,                     // Reference air flow rate of DX coil [m3/s]
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRate,            // 2017 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRate_2023,       // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        int const nsp,                                                 // Number of compressor speed
        ObjexxFCL::Optional_int_const RegionNum = _,                   // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor = _,        // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn = _,      // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank = _, // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl = _ // defrost control; 1=timed, 2=on-demand
    );

    void ReportDXCoilRating(EnergyPlusData &state,
                            std::string_view CompName,       // Name of component
                            HVAC::CoilType const compType,   // TypeNum of component
                            Real64 const CoolCapVal,         // Standard total (net) cooling capacity for AHRI Std. 210/240 {W}
                            Real64 const SEERValueIP,        // SEER value in IP units from user PLR curve {Btu/W-h}
                            Real64 const SEERValueDefaultIP, // SEER value in IP units from AHRI Std 210/240-2008 default PLF curve and C_D {Btu/W-h}
                            Real64 const EERValueSI,         // EER value in SI units {W/W}
                            Real64 const EERValueIP,         // EER value in IP units {Btu/W-h}
                            Real64 const IEERValueIP,        // IEER value in IP units {Btu/W-h}
                            Real64 const HighHeatingCapVal,  // High Temperature Heating Standard (Net) Rating Capacity
                            Real64 const LowHeatingCapVal,   // Low Temperature Heating Standard (Net) Rating Capacity
                            Real64 const HSPFValueIP,        // IEER value in IP units {Btu/W-h}
                            int const RegionNum,             // Region Number for which HSPF is calculated
                            bool const AHRI2023StandardRatings = false); // True if required AHRI/ANSI 210/240 Std. 2023 SEER2,HSPF2 Ratings.

    void ReportDXCoolCoilDataCenterApplication(EnergyPlusData &state,
                                               std::string_view const &CompName,      // Name of component
                                               HVAC::CoilType const CompType,         // TypeNum of component
                                               Array1D<Real64> &NetCoolingCapRated,   // net cooling capacity of single speed DX cooling coil
                                               Array1D<Real64> &TotElectricPowerRated // total electric power including supply fan
    );

    void CheckCurveLimitsForStandardRatings(EnergyPlusData &state,
                                            std::string const &DXCoilName, // Name of DX coil for which HSPF is calculated
                                            HVAC::CoilType const coilType, // Integer type of DX coil - heating or cooling
                                            int const CapFTempCurveIndex,  // Index for the capacity as a function of temperature modifier curve
                                            int const CapFFlowCurveIndex,  // Index for the capacity as a function of flow fraction modifier curve
                                            int const EIRFTempCurveIndex,  // Index for the EIR as a function of temperature modifier curve
                                            int const EIRFFlowCurveIndex,  // Index for the EIR as a function of flow fraction modifier curve
                                            int const PLFFPLRCurveIndex    // Index for the EIR vs part-load ratio curve
    );

    Real64 CondenserEnteringFluidTemperature(
        DataPlant::CondenserType const CondenserType,     // Chiller Condenser Type: AirCooled, WaterCooled, or EvaporativelyCooled
        StandardRatings::AhriChillerStd const ChillerStd, // AHRI Std 550/590 (IP), or AHRI Std 551/591 (SI)
        Real64 LoadRatio                                  // AHRI Std test load ratio: 1.0, 0.75, 0.5, 0.25
    );

} // namespace StandardRatings

} // namespace EnergyPlus

#endif
