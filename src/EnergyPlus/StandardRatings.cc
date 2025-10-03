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
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>

namespace EnergyPlus {

namespace StandardRatings {

    // MODULE INFORMATION:
    //       AUTHOR         Chandan Sharma
    //       DATE WRITTEN   February 2012
    //       MODIFIED       February 2013, Bereket Nigusse
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module contains the subroutines required to calculate the following standard ratings of HVAC equipment
    // 1) Integrated Part Load Value (IPLV) rating for EIR and Reformulated EIR chillers
    // 2) a) Standard Rated (net) Cooling Capacity
    //    b) Seasonal Energy Efficiency Ratio (SEER)
    //    c) Energy Efficiency Ratio (EER),
    //    d) Integrated Energy Efficiency Ratio (IEER)
    //       for Air-to-Air Direct Expansion Air Conditioner and Heat Pumps having a single-speed compressor,
    //       fixed speed indoor supply air fan, and air-cooled condensers.
    // 3) Heating Seasonal Performance Factor (HSPF) for Air-Source Direct Expansion Heat Pumps having a single-speed compressor,
    //       fixed speed indoor supply air fan
    // 4) Seasonal Energy Efficiency Ratio (SEER) for Air-Source Direct Expansion multi-speed compressor Heat Pumps
    // 5) Heating Seasonal Performance Factor (HSPF) for Air-Source Direct Expansion multi-speed compressor Heat Pumps
    // METHODOLOGY EMPLOYED:
    // Using the user specified reference capacity, reference COP and performance curves, the chiller or DX coil models are executed
    // for standard test conditions as specified in ANSI/AHRI 550/590, 210/240 and 340/360. Then results of the simulated test points
    // are processed into standard ratings according to standard's procedures.

    // REFERENCES:
    // (1) AHRI Standard 550/590-2011:  Standard for Performance Rating of Water-Chilling Packages using the Vapor
    //                                  Compression Cycle. Arlington, VA:  Air-Conditioning, Heating,
    //                                  and Refrigeration Institute.
    // (2) ANSI/AHRI Standard 210/240-2008:  Standard for Performance Rating of Unitary Air-Conditioning and
    //                                       Air-Source Heat Pumps. Arlington, VA:  Air-Conditioning, Heating
    //                                       , and Refrigeration Institute.
    // (3) ANSI/AHRI Standard 340/360-2007:  Standard for Performance Rating of Commercial and Industrial
    //                                       Unitary Air-Conditioning and Heat Pump Equipment.  Arlington,
    //                                       VA:  Air-Conditioning, Heating, and Refrigeration Institute.

    // OTHER NOTES: none

    // Data
    Real64 constexpr IndoorCoilInletAirWetBulbTempRated(19.44);     // 19.44C (67F)  Tests A2, B2, B1, and F1
    Real64 constexpr OutdoorCoilInletAirDryBulbTempRated(35.0);     // 35.00C (95F)  Tests A2, B2, B1, and F1
    Real64 constexpr OutdoorCoilInletAirDryBulbTempTestA2(35.0);    // 35.00C (95F)  Test A2 (high speed)
    Real64 constexpr OutdoorCoilInletAirDryBulbTempTestB2(27.78);   // 27.78C (82F)  Test B2 (high speed)
    Real64 constexpr OutdoorCoilInletAirDryBulbTempTestB1(27.78);   // 27.78C (82F)  Test B1 (Low speed)
    Real64 constexpr OutdoorCoilInletAirDryBulbTempTestF1(19.44);   // 19.44C (67F)  Test B1 (Low speed)
    Real64 constexpr OutdoorCoilInletAirDryBulbTempTestEint(30.55); // 30.55 (87F) Test Eint

    // AHRI Standard 210/240-2008 Performance Test Conditions for Unitary Air-to-Air Air-Conditioning and Heat Pump Equipment
    // AHRI STANDARD 210/240 - 2023
    Real64 constexpr CoolingCoilInletAirWetBulbTempRated(19.44); // 19.44C (67F)  Tests A and B
    // AHRI STANDARD 210/240 - 2023
    Real64 constexpr OutdoorUnitInletAirDryBulbTempRated(35.0); // 35.00C (95F)  Test A (rated capacity)

    Real64 constexpr OutdoorUnitInletAirDryBulbTemp(27.78); // 27.78C (82F)  Test B (for SEER)
    Real64 constexpr AirMassFlowRatioRated(1.0);            // AHRI test is at the design flow rate and hence AirMassFlowRatio is 1.0
    // AHRI STANDARD 210/240 - 2017
    Real64 constexpr DefaultFanPowerPerEvapAirFlowRate(773.3); // 365 W/1000 scfm or 773.3 W/(m3/s). The AHRI standard
    // AHRI STANDARD 210/240 - 2023
    Real64 constexpr DefaultFanPowerPerEvapAirFlowRateSEER2(934.4); // 441 W/1000 scfm or 934.4 W/(m3/s). The AHRI standard
    // specifies a nominal/default fan electric power consumption per rated air
    // volume flow rate to account for indoor fan electric power consumption
    // when the standard tests are conducted on units that do not have an
    // indoor air circulating fan. Used if user doesn't enter a specific value.

    // AHRI STANDARD 210/240 - 2023
    Real64 constexpr PLRforSEER(0.5); // Part-load ratio for SEER calculation (single speed DX cooling coils)

    static constexpr std::array<Real64, 4> ReducedPLR = {1.0, 0.75, 0.50, 0.25};               // Reduced Capacity part-load conditions
    static constexpr std::array<Real64, 4> IEERWeightingFactor = {0.020, 0.617, 0.238, 0.125}; // EER Weighting factors (IEER)
    Real64 constexpr OADBTempLowReducedCapacityTest(18.3);                                     // Outdoor air dry-bulb temp in degrees C (65F)
    // Std. AHRI AHRI 340/360 Dry-bulb Temp at reduced capacity, <= 0.444

    // HSPF AHRI STANDARD 210/240 - 2017
    int constexpr TotalNumOfStandardDHRs(16);                                                  // Total number of standard design heating requirements
    static constexpr std::array<Real64, 6> TotalNumOfTemperatureBins = {9, 10, 13, 15, 18, 9}; // Total number of temperature bins for a region
    static constexpr std::array<Real64, 16> StandardDesignHeatingRequirement = {1465.36,
                                                                                2930.71,
                                                                                4396.07,
                                                                                5861.42,
                                                                                7326.78,
                                                                                8792.14,
                                                                                10257.49,
                                                                                11722.85,
                                                                                14653.56,
                                                                                17584.27,
                                                                                20514.98,
                                                                                23445.70,
                                                                                26376.41,
                                                                                29307.12,
                                                                                32237.83,
                                                                                38099.26};

    // HSPF2 AHRI STANDARD 210/240 - 2023
    // BPS: As Fractional Bin hours are not summing up to 1 we using an adjusted array of total number of Temperature Bins per region.
    // static constexpr std::array<Real64, 6> TotalNumOfTemperatureBinsHSPF2 = {8, 8, 11, 13, 16, 7}; // Total number of temperature bins for each
    // region
    static constexpr std::array<Real64, 6> TotalNumOfTemperatureBinsHSPF2 = {9, 10, 13, 15, 18, 9};

    // Standardized DHRs from ANSI/AHRI 210/240 - 2017
    Real64 constexpr CorrectionFactor(0.77); // A correction factor which tends to improve the agreement
    // Slope Adjustment Factor (Cx)  from ANSI/AHRI 210/240 - 2023 (Section 11.2.2 - Table 14)
    // 1. Variable Speed Load Factor (Cvs) | Variable Speed  Equipment
    static constexpr std::array<Real64, 6> VariableSpeedLoadFactor = {1.03, 0.99, 1.21, 1.07, 1.08, 1.03};
    // 2. Heating Load Line Equation Slope Factor (C) | other Equipment types
    static constexpr std::array<Real64, 6> SpeedLoadFactor = {1.10, 1.06, 1.30, 1.15, 1.16, 1.11};

    // between calculated and measured building loads, dimensionless.
    Real64 constexpr CyclicDegradationCoeff(0.25);
    // For Single Stage Systems, if the optional CFull and DFull tests are not performed, a
    // default value of 0.20 shall be used for the cooling Degradation Coefficient
    Real64 constexpr CyclicDegradationCoeffSEER2(0.20); // ANSI/AHRI 210/240 2023 Section 6.1.3.1

    // Default Heating Degradation Coefficient | HSPF2
    // ANSI/AHRI Standard 2023 Section 6.1.3.2.1 -> For Single Stage Systems, if the optional H1CFull test or H1CLow is not performed, a default value
    // of 0.25 shall be used for the heating Degradation Coefficient
    Real64 constexpr CyclicHeatingDegradationCoeffHSPF2(0.25);

    // AHRI STANDARD 210/240 - 2017 & 2023
    static constexpr std::array<Real64, 6> OutdoorDesignTemperature = {2.78, -2.78, -8.33, -15.0, -23.33, -1.11};

    // (Tzl)the zero-load temperature variation by climate region ANSI/AHRI 210/240 - 2023 (Section 11.2.2 - Table 14)
    // [ 58 57 56 55 55 57 ] in Fahrenheit
    static constexpr std::array<Real64, 6> ZoneLoadTemperature = {14.44, 13.89, 13.33, 12.78, 12.78, 13.89};

    // Outdoor design temperature for a region from ANSI/AHRI 210/240 - 2017 & 2023
    static constexpr std::array<Real64, 18> OutdoorBinTemperature = {
        16.67, 13.89, 11.11, 8.33, 5.56, 2.78, 0.00, -2.78, -5.56, -8.33, -11.11, -13.89, -16.67, -19.44, -22.22, -25.00, -27.78, -30.56};
    int constexpr NumberOfRegions{6};
    int constexpr NumberOfBins{18};
    // Fractional bin hours for different bin temperatures from ANSI/AHRI 210/240 - 2017
    static constexpr std::array<std::array<Real64, NumberOfBins>, NumberOfRegions> FracBinHoursAtOutdoorBinTemp = {
        {{0.291, 0.239, 0.194, 0.129, 0.081, 0.041, 0.019, 0.005, 0.001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
         {0.215, 0.189, 0.163, 0.143, 0.112, 0.088, 0.056, 0.024, 0.008, 0.002, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
         {0.153, 0.142, 0.138, 0.137, 0.135, 0.118, 0.092, 0.047, 0.021, 0.009, 0.005, 0.002, 0.001, 0.0, 0.0, 0.0, 0.0, 0.0},
         {0.132, 0.111, 0.103, 0.093, 0.1, 0.109, 0.126, 0.087, 0.055, 0.036, 0.026, 0.013, 0.006, 0.002, 0.001, 0.0, 0.0, 0.0},
         {0.106, 0.092, 0.086, 0.076, 0.078, 0.087, 0.102, 0.094, 0.074, 0.055, 0.047, 0.038, 0.029, 0.018, 0.01, 0.005, 0.002, 0.001},
         {0.113, 0.206, 0.215, 0.204, 0.141, 0.076, 0.034, 0.008, 0.003, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}}};

    // Fractional bin hours for different bin temperatures from ANSI/AHRI 210/240 - 2023 (Section 11.2.2 - Table 14)
    // As the Fractional Bin Hours for each region is not summing up to 1 we're using the adjusted one down below
    // static constexpr std::array<std::array<Real64, NumberOfBins>, NumberOfRegions> FracBinHoursAtOutdoorBinTempHSPF2 = {
    //{{0.0, 0.239, 0.194, 0.129, 0.081, 0.041, 0.019, 0.005, 0.001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    // {0.0, 0.0, 0.163, 0.143, 0.112, 0.088, 0.056, 0.024, 0.008, 0.002, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    // {0.0, 0.0, 0.138, 0.137, 0.135, 0.118, 0.092, 0.047, 0.021, 0.009, 0.005, 0.002, 0.001, 0.0, 0.0, 0.0, 0.0, 0.0},
    // {0.0, 0.0, 0.103, 0.093, 0.1, 0.109, 0.126, 0.087, 0.055, 0.036, 0.026, 0.013, 0.006, 0.002, 0.001, 0.0, 0.0, 0.0},
    // {0.0, 0.0, 0.086, 0.076, 0.078, 0.087, 0.102, 0.094, 0.074, 0.055, 0.047, 0.038, 0.029, 0.018, 0.01, 0.005, 0.002, 0.001},
    // {0.0, 0.0, 0.215, 0.204, 0.141, 0.076, 0.034, 0.008, 0.003, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}}};
    // Adjusted to sum up to 1
    static constexpr std::array<std::array<Real64, NumberOfBins>, NumberOfRegions> FracBinHoursAtOutdoorBinTempHSPF2 = {{{0.0,
                                                                                                                          0.337094499,
                                                                                                                          0.273624824,
                                                                                                                          0.181946403,
                                                                                                                          0.114245416,
                                                                                                                          0.057827927,
                                                                                                                          0.026798307,
                                                                                                                          0.007052186,
                                                                                                                          0.001410437,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0},
                                                                                                                         {0.0,
                                                                                                                          0.0,
                                                                                                                          0.273489933,
                                                                                                                          0.239932886,
                                                                                                                          0.187919463,
                                                                                                                          0.147651007,
                                                                                                                          0.093959732,
                                                                                                                          0.040268456,
                                                                                                                          0.013422819,
                                                                                                                          0.003355705,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0},
                                                                                                                         {0.0,
                                                                                                                          0.0,
                                                                                                                          0.195744681,
                                                                                                                          0.194326241,
                                                                                                                          0.191489362,
                                                                                                                          0.167375887,
                                                                                                                          0.130496454,
                                                                                                                          0.066666667,
                                                                                                                          0.029787234,
                                                                                                                          0.012765957,
                                                                                                                          0.007092199,
                                                                                                                          0.002836879,
                                                                                                                          0.00141844,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0},
                                                                                                                         {0.0,
                                                                                                                          0.0,
                                                                                                                          0.136063408,
                                                                                                                          0.122853369,
                                                                                                                          0.132100396,
                                                                                                                          0.143989432,
                                                                                                                          0.166446499,
                                                                                                                          0.114927345,
                                                                                                                          0.072655218,
                                                                                                                          0.047556143,
                                                                                                                          0.034346103,
                                                                                                                          0.017173052,
                                                                                                                          0.007926024,
                                                                                                                          0.002642008,
                                                                                                                          0.001321004,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0},
                                                                                                                         {0.0,
                                                                                                                          0.0,
                                                                                                                          0.10723192,
                                                                                                                          0.094763092,
                                                                                                                          0.097256858,
                                                                                                                          0.108478803,
                                                                                                                          0.127182045,
                                                                                                                          0.117206983,
                                                                                                                          0.092269327,
                                                                                                                          0.068578554,
                                                                                                                          0.058603491,
                                                                                                                          0.047381546,
                                                                                                                          0.036159601,
                                                                                                                          0.02244389,
                                                                                                                          0.012468828,
                                                                                                                          0.006234414,
                                                                                                                          0.002493766,
                                                                                                                          0.001246883},
                                                                                                                         {0.0,
                                                                                                                          0.0,
                                                                                                                          0.315712188,
                                                                                                                          0.299559471,
                                                                                                                          0.207048458,
                                                                                                                          0.111600587,
                                                                                                                          0.049926579,
                                                                                                                          0.01174743,
                                                                                                                          0.004405286,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0,
                                                                                                                          0.0}}};

    // Representative cooling season Outdoor air temperature bin from ANSI/AHRI 210/240-2008
    int constexpr NumOfOATempBins(8); // number of outdoor temperature bins for cooling season
    static constexpr std::array<Real64, NumOfOATempBins> OutdoorBinTemperatureSEER = {19.44, 22.22, 25.00, 27.78, 30.56, 33.33, 36.11, 38.89};
    // Fractional bin hours for different bin temperatures for cooling, from ANSI/AHRI 210/240 - 2008
    // AHRI STANDARD 210/240 - 2023 | This does sum up to 1
    static constexpr std::array<Real64, NumOfOATempBins> CoolFracBinHoursAtOutdoorBinTemp = {0.214, 0.231, 0.216, 0.161, 0.104, 0.052, 0.018, 0.004};

    Real64 constexpr HeatingIndoorCoilInletAirDBTempRated(21.11); // Heating coil entering air dry-bulb temperature in
    // degrees C (70F) Test H1, H2 and H3
    // (low and High Speed) Std. AHRI 210/240
    Real64 constexpr HeatingOutdoorCoilInletAirDBTempH0Test(16.67); // Outdoor air dry-bulb temp in degrees C (47F)
    // Test H0 (low and High Speed) Std. AHRI 210/240

    // ANSI/ASHRAE Standard 127-2012 -- Method of Testing for Rating Computer and Data Processing Room Unitary Air Conditioners
    //  Class 1 23.9C( 75.0F ) 23.9C( 75.0F ) 23.9C( 75.0F ) 23.9C( 75.0F )
    //  Class 2 29.4C( 85.0F ) 29.4C( 85.0F ) 29.4C( 85.0F ) 29.4C( 85.0F )
    //  Class 3 35.0C( 95.0F ) 35.0C( 95.0F ) 35.0C( 95.0F ) 35.0C( 95.0F )
    //  Class 4 40.5C( 105F ) 40.5C( 105F ) 40.5C( 105F ) 40.5C( 105F )
    static constexpr std::array<Real64, 4> IndoorDBTempClassI2IV = {23.9, 29.4, 35.0, 40.5};
    Real64 constexpr IndoorTDPA2D(11.1);
    // 35.0C( 95.0F ) 26.7C( 80.0F ) 18.3C( 65.0F ) 4.4C( 40.0F )
    static constexpr std::array<Real64, 4> OutdoorDBTempAllClassA2D = {35.0, 26.7, 18.3, 4.4};

    // AHRI Std. 340/360-2022 (IP)
    // Reduced Capacity part-load conditions
    static constexpr std::array<Real64, 4> ReducedPLRIEER = {0.25, 0.50, 0.75, 1.0};
    // Table 6
    // Cooling
    // Real64 CoilInletAirWetBulbTemp = 23.89;  // 75F
    Real64 constexpr CoilInletAirCoolDryBulbIEER(35);     // 95F
    Real64 constexpr CoilWaterOutletTempIEER(35);         // 95F
    Real64 constexpr CoilWaterInletTempIEER(29.44);       // 85F
    Real64 constexpr CoilInletEvapWetBulbTempIEER(23.89); // 75F
    Real64 constexpr CoilInletEvapDryBulbTempIEER(35);    // 95F
    // Heating
    Real64 constexpr CoilHeatingInletAirWetBulbTempIEER(6.11); // 43F
    Real64 constexpr CoilHeatingInletAirCoolDryBulbIEER(8.33); // 47F

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
                         ObjexxFCL::Optional<Real64 const> OpenMotorEff)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma, FSEC
        //       DATE WRITTEN   January 2012
        //       Modified       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //     Calculates Integrated Part Load Value (IPLV) for EIR and reformulated EIR chillers.
        //     Writes the result to EIO file.
        // METHODOLOGY EMPLOYED:
        // (1) Obtains the reference cooling capacity, reference COP and performance curves of the chiller
        // (2) Evaluates the cooling capacity at AHRI test conditions (Per AHRI 551/591,2011 Table 3)
        // (3) Evaluates the EIR at AHRI test conditions (Per AHRI 551/591,2011 Table 3)
        // (4) The EER is evaluated from the total cooling capacity and total electric power
        //     evaluated at the standard rated test conditions.  The IPLV is a weighted value of the COP evaluated
        //     at four different capacities of 100%, 75%, 50% and 25%.  The reduced capacity COPs are evaluated
        //     at different outdoor coil entering temperatures.
        // REFERENCES:
        // (1) AHRI Standard 551/591-2011:  Standard for Performance Rating of Water-Chilling Packages using the Vapor
        //                                  Compression Cycle. Arlington, VA:  Air-Conditioning, Heating,
        //                                  and Refrigeration Institute.

        // Using/Aliasing
        using namespace OutputReportPredefined;
        using Curve::CurveValue;
        using Curve::GetCurveName;
        using General::SolveRoot;

        Real64 constexpr Acc(0.0001);     // Accuracy of result
        int constexpr NumOfReducedCap(4); // Number of reduced capacity test conditions (100%,75%,50%,and 25%)
        int constexpr IterMax(500);       // Maximum number of iterations
        static constexpr std::array<Real64, 4> IPLVWeightingFactor = {0.010, 0.42, 0.45, 0.12}; // EER Weighting factors (IPLV)
        static constexpr std::string_view RoutineName("CalcChillerIPLV");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AvailChillerCap(0.0);               // Chiller available capacity at current operating conditions [W]
        Real64 EnteringWaterTempReduced(0.0);      // Entering Condenser Water Temperature at reduced conditions [C]
        Real64 EnteringAirDryBulbTempReduced(0.0); // Outdoor unit entering air dry-bulb temperature
        // at reduced capacity [C]
        Real64 EnteringAirWetBulbTempReduced(0.0); // Outdoor unit entering air wet-bulb temperature
        // at reduced capacity [C]
        Real64 CondenserInletTemp(0.0);   // Entering Condenser Temperature at reduced conditions [C]
        Real64 CondenserOutletTemp0(0.0); // Lower bound for condenser outlet temperature [C]
        Real64 CondenserOutletTemp1(0.0); // Upper bound for condenser outlet temperature [C]
        Real64 CondenserOutletTemp(0.0);  // Calculated condenser outlet temperature which corresponds
        // to EnteringWaterTempReduced above [C]
        Real64 Cp(0.0);         // Water specific heat [J/(kg*C)]
        Real64 Rho(0.0);        // Water density [kg/m3]
        Real64 EIR(0.0);        // Inverse of COP at reduced capacity test conditions (100%, 75%, 50%, and 25%)
        Real64 Power(0.0);      // Power at reduced capacity test conditions (100%, 75%, 50%, and 25%)
        Real64 COPReduced(0.0); // COP at reduced capacity test conditions (100%, 75%, 50%, and 25%)
        Real64 LoadFactor(0.0); // Fractional "on" time for last stage at the desired reduced capacity,
        // (dimensionless)
        Real64 DegradationCoeff(0.0);   // Degradation coefficient, (dimensionless)
        Real64 ChillerCapFT_rated(0.0); // Chiller capacity fraction at AHRI rated conditions (evaluated as a function of temperature)
        Real64 ChillerCapFT(0.0);       // Chiller capacity fraction (evaluated as a function of temperature)
        Real64 ChillerEIRFT_rated(0.0); // Chiller electric input ratio (EIR = 1 / COP) at AHRI rated conditions as a function of temperature
        Real64 ChillerEIRFT(0.0);       // Chiller electric input ratio (EIR = 1 / COP) as a function of temperature
        Real64 ChillerEIRFPLR(0.0);     // Chiller EIR as a function of part-load ratio (PLR)
        Real64 PartLoadRatio(0.0);      // Part load ratio (PLR) at which chiller is operating at reduced capacity
        int RedCapNum;                  // Integer counter for reduced capacity
        int SolFla;                     // Flag of solver

        // Initialize local variables
        AvailChillerCap = 0.0;
        EnteringWaterTempReduced = 0.0;
        EnteringAirDryBulbTempReduced = 0.0;
        EnteringAirWetBulbTempReduced = 0.0;
        CondenserInletTemp = 0.0;
        CondenserOutletTemp0 = 0.0;
        CondenserOutletTemp1 = 0.0;
        CondenserOutletTemp = 0.0;
        Cp = 0.0;
        Rho = 0.0;
        IPLVSI = 0.0;
        IPLVIP = 0.0;
        EIR = 0.0;
        Power = 0.0;
        COPReduced = 0.0;
        LoadFactor = 0.0;
        DegradationCoeff = 0.0;
        ChillerCapFT_rated = 0.0;
        ChillerCapFT = 0.0;
        ChillerEIRFT_rated = 0.0;
        ChillerEIRFT = 0.0;
        ChillerEIRFPLR = 0.0;
        PartLoadRatio = 0.0;

        CheckCurveLimitsForIPLV(state, ChillerName, ChillerType, CondenserType, CapFTempCurveIndex, EIRFTempCurveIndex);

        Real64 IPLV = 0.0;                        // current IPLV
        Real64 EvapOutletTemp = 0.0;              // evaporator leaving water temperature, deg C
        Real64 constexpr EvapOutletTempSI = 7.0;  // evaporator LWT, 2011 AHRI Std 551 / 591(SI)
        Real64 constexpr EvapOutletTempIP = 6.67; // evaporator LWT, 2011 AHRI Std 550 / 590(IP), (44F)
        bool ReportStdRatingsOnce = true;

        for (const auto &AhriStd : {AhriChillerStd::AHRI550_590, AhriChillerStd::AHRI551_591}) {
            if (AhriStd == AhriChillerStd::AHRI550_590) {
                EvapOutletTemp = EvapOutletTempIP;
                ReportStdRatingsOnce = true;
            } else if (AhriStd == AhriChillerStd::AHRI551_591) {
                EvapOutletTemp = EvapOutletTempSI;
                ReportStdRatingsOnce = false;
            }
            IPLV = 0.0;

            // IPLV calculations:
            for (RedCapNum = 0; RedCapNum < NumOfReducedCap; ++RedCapNum) {

                CondenserInletTemp = CondenserEnteringFluidTemperature(CondenserType, AhriStd, ReducedPLR[RedCapNum]);

                if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricEIR) {
                    if (RedCapNum == 0) {
                        // Get curve modifier values at rated conditions (load = 100%)
                        ChillerCapFT_rated = CurveValue(state, CapFTempCurveIndex, EvapOutletTemp, CondenserInletTemp);
                        ChillerEIRFT_rated = CurveValue(state, EIRFTempCurveIndex, EvapOutletTemp, CondenserInletTemp);

                        // Report rated capacity and chiller COP
                        if (ReportStdRatingsOnce) {
                            PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechRatCap, ChillerName, RefCap * ChillerCapFT_rated);
                            PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechRatEff, ChillerName, RefCOP / ChillerEIRFT_rated);
                        }
                    }

                    // Get capacity curve info with respect to CW setpoint and entering condenser temps
                    ChillerCapFT = CurveValue(state, CapFTempCurveIndex, EvapOutletTemp, CondenserInletTemp);

                    ChillerEIRFT = CurveValue(state, EIRFTempCurveIndex, EvapOutletTemp, CondenserInletTemp);

                    PartLoadRatio = ReducedPLR[RedCapNum] * ChillerCapFT_rated / ChillerCapFT;

                    if (PartLoadRatio >= MinUnloadRat) {
                        ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, PartLoadRatio);
                    } else {
                        ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, MinUnloadRat);
                        PartLoadRatio = MinUnloadRat;
                    }

                } else if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR) {
                    EnteringWaterTempReduced = CondenserInletTemp;
                    Cp = state.dataPlnt->PlantLoop(CondLoopNum).glycol->getSpecificHeat(state, EnteringWaterTempReduced, RoutineName);

                    Rho = state.dataPlnt->PlantLoop(CondLoopNum).glycol->getDensity(state, EnteringWaterTempReduced, RoutineName);

                    Real64 reducedPLR = ReducedPLR[RedCapNum];
                    CondenserOutletTemp0 = EnteringWaterTempReduced + 0.1;
                    CondenserOutletTemp1 = EnteringWaterTempReduced + 10.0;

                    // CONST_LAMBDA_CAPTURE Issue, see PR 9670
                    Real64 tmpEvapOutletTemp = EvapOutletTemp;
                    auto f = [&state,
                              EnteringWaterTempReduced,
                              Cp,
                              reducedPLR,
                              CondVolFlowRate,
                              Rho,
                              CapFTempCurveIndex,
                              EIRFTempCurveIndex,
                              EIRFPLRCurveIndex,
                              RefCap,
                              RefCOP,
                              OpenMotorEff,
                              tmpEvapOutletTemp,
                              ChillerCapFT_rated](Real64 const CondenserOutletTemp) {
                        Real64 AvailChillerCap(0.0);         // Chiller available capacity at current operating conditions [W]
                        Real64 CondenserInletTemp(0.0);      // Calculated condenser inlet temperature [C]
                        Real64 QEvap(0.0);                   // Rate of heat transfer to the evaporator coil [W]
                        Real64 QCond(0.0);                   // Rate of heat transfer to the condenser coil [W]
                        Real64 Power(0.0);                   // Power at reduced capacity test conditions (100%, 75%, 50%, and 25%)
                        Real64 ReformEIRChillerCapFT(0.0);   // Chiller capacity fraction (evaluated as a function of temperature)
                        Real64 ReformEIRChillerEIRFT(0.0);   // Chiller electric input ratio (EIR = 1 / COP) as a function of temperature
                        Real64 ReformEIRChillerEIRFPLR(0.0); // Chiller EIR as a function of part-load ratio (PLR)
                        Real64 PartLoadRatio(0.0);           // Chiller part load ratio

                        ReformEIRChillerCapFT = CurveValue(state, CapFTempCurveIndex, tmpEvapOutletTemp, CondenserOutletTemp);

                        ReformEIRChillerEIRFT = CurveValue(state, EIRFTempCurveIndex, tmpEvapOutletTemp, CondenserOutletTemp);

                        // Available chiller capacity as a function of temperature
                        AvailChillerCap = RefCap * ReformEIRChillerCapFT;

                        switch (state.dataCurveManager->curves(EIRFPLRCurveIndex)->numDims) {
                        case 1:
                            ReformEIRChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp);
                            break;
                        case 2:
                            ReformEIRChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp, reducedPLR);
                            break;
                        case 3:
                        default: // this default allows the simulation to continue, but will issue a warning, should be removed eventually
                            ReformEIRChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp, reducedPLR, 0.0);
                            break;
                        }

                        Power = (AvailChillerCap / RefCOP) * ReformEIRChillerEIRFPLR * ReformEIRChillerEIRFT;

                        if (reducedPLR >= 1.0) {
                            PartLoadRatio = reducedPLR;
                        } else {
                            PartLoadRatio = reducedPLR * ChillerCapFT_rated / ReformEIRChillerCapFT;
                        }

                        QEvap = AvailChillerCap * PartLoadRatio;

                        QCond = Power * OpenMotorEff + QEvap;

                        if (CondVolFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
                            CondenserInletTemp = CondenserOutletTemp - QCond / (CondVolFlowRate * Rho) / Cp;
                        }
                        return (EnteringWaterTempReduced - CondenserInletTemp) / EnteringWaterTempReduced;
                    };

                    General::SolveRoot(state, Acc, IterMax, SolFla, CondenserOutletTemp, f, CondenserOutletTemp0, CondenserOutletTemp1);
                    if (SolFla == -1) {
                        ShowWarningError(state, "Iteration limit exceeded in calculating Reform Chiller IPLV");
                        ShowContinueError(state, format("Reformulated Chiller IPLV calculation failed for {}", ChillerName));
                    } else if (SolFla == -2) {
                        ShowWarningError(state, "Bad starting values for calculating Reform Chiller IPLV");
                        ShowContinueError(state, format("Reformulated Chiller IPLV calculation failed for {}", ChillerName));
                    }

                    if (RedCapNum == 0) {
                        // Get curve modifier values at rated conditions (load = 100%)
                        ChillerCapFT_rated = CurveValue(state, CapFTempCurveIndex, EvapOutletTemp, CondenserOutletTemp);
                        ChillerEIRFT_rated = CurveValue(state, EIRFTempCurveIndex, EvapOutletTemp, CondenserOutletTemp);

                        // Report rated capacity and chiller COP
                        if (ReportStdRatingsOnce) {
                            PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechRatCap, ChillerName, RefCap * ChillerCapFT_rated);
                            PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechRatEff, ChillerName, RefCOP / ChillerEIRFT_rated);
                        }
                    }

                    ChillerCapFT = CurveValue(state, CapFTempCurveIndex, EvapOutletTemp, CondenserOutletTemp);

                    ChillerEIRFT = CurveValue(state, EIRFTempCurveIndex, EvapOutletTemp, CondenserOutletTemp);

                    PartLoadRatio = ReducedPLR[RedCapNum] * ChillerCapFT_rated / ChillerCapFT;

                    if (PartLoadRatio >= MinUnloadRat) {
                        switch (state.dataCurveManager->curves(EIRFPLRCurveIndex)->numDims) {
                        case 1:
                            ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp);
                            break;
                        case 2:
                            ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp, PartLoadRatio);
                            break;
                        case 3:
                        default: // this default allows the simulation to continue, but will issue a warning, should be removed eventually
                            ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp, PartLoadRatio, 0.0);
                            break;
                        }
                    } else {
                        switch (state.dataCurveManager->curves(EIRFPLRCurveIndex)->numDims) {
                        case 1:
                            ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp);
                            break;
                        case 2:
                            ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp, MinUnloadRat);
                            break;
                        case 3:
                        default: // this default allows the simulation to continue, but will issue a warning, should be removed eventually
                            ChillerEIRFPLR = CurveValue(state, EIRFPLRCurveIndex, CondenserOutletTemp, MinUnloadRat, 0.0);
                            break;
                        }
                        PartLoadRatio = MinUnloadRat;
                    }
                } else {
                    assert(false);
                }

                // Available chiller capacity as a function of temperature
                if (RefCap > 0.0 && RefCOP > 0.0 && ChillerCapFT > 0.0 && ChillerEIRFT > 0.0) {
                    AvailChillerCap = RefCap * ChillerCapFT;
                    Power = (AvailChillerCap / RefCOP) * ChillerEIRFPLR * ChillerEIRFT;
                    EIR = Power / (PartLoadRatio * AvailChillerCap);

                    if (ReducedPLR[RedCapNum] >= MinUnloadRat) {
                        COPReduced = 1.0 / EIR;
                    } else {
                        LoadFactor = (ReducedPLR[RedCapNum] * RefCap) / (MinUnloadRat * AvailChillerCap);
                        DegradationCoeff = 1.130 - 0.130 * LoadFactor;
                        COPReduced = 1.0 / (DegradationCoeff * EIR);
                    }
                    IPLV += IPLVWeightingFactor[RedCapNum] * COPReduced;
                } else {
                    {
                        if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricEIR) {
                            ShowWarningError(
                                state, format("Chiller:Electric:EIR = {}:  Integrated Part Load Value (IPLV) cannot be calculated.", ChillerName));
                        } else if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR) {

                            ShowWarningError(state,
                                             format("Chiller:Electric:ReformulatedEIR = {}:  Integrated Part Load Value (IPLV) cannot be calculated.",
                                                    ChillerName));
                        }
                    }
                    if (RefCap <= 0.0) {
                        ShowContinueError(
                            state,
                            format(" Check the chiller autosized or user specified capacity. Autosized or specified chiller capacity = {:.2R}",
                                   RefCap));
                    }
                    if (RefCOP <= 0.0) {
                        ShowContinueError(state, format(" Check the chiller reference or rated COP specified. Specified COP = {:.2R}", RefCOP));
                    }
                    if (ChillerCapFT <= 0.0) {
                        ShowContinueError(state,
                                          format(" Check limits in Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}.",
                                                 Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                                 GetCurveName(state, CapFTempCurveIndex)));
                        ShowContinueError(state, format(" ..ChillerCapFT value at standard test condition = {:.2R}", ChillerCapFT));
                    }
                    if (ChillerEIRFT <= 0.0) {
                        ShowContinueError(state,
                                          format(" Check limits in EIR Function of Temperature Curve, Curve Type = {}, Curve Name = {}.",
                                                 Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                                 GetCurveName(state, EIRFTempCurveIndex)));
                        ShowContinueError(state, format(" ..ChillerEIRFT value at standard test condition = {:.2R}", ChillerEIRFT));
                    }
                    IPLV = 0.0;
                    break;
                }
            }

            if (AhriStd == AhriChillerStd::AHRI550_590) {
                IPLVIP = IPLV;
            } else {
                IPLVSI = IPLV;
            }
        }
        // Writes the IPLV value to the EIO file and standard tabular output tables
        ReportChillerIPLV(state, ChillerName, ChillerType, IPLVSI, IPLVIP * ConvFromSIToIP);
    }

    void ReportChillerIPLV(EnergyPlusData &state,
                           std::string const &ChillerName,            // Name of Chiller for which IPLV is calculated
                           DataPlant::PlantEquipmentType ChillerType, // Type of Chiller - EIR or Reformulated EIR
                           Real64 const IPLVValueSI,                  // IPLV value in SI units {W/W}
                           Real64 const IPLVValueIP                   // IPLV value in IP units {Btu/W-h}
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   January 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the IPLV values in SI and IP units to
        // the "eio" and tabular output files for EIR Chillers.

        // Using/Aliasing
        using namespace OutputReportPredefined;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (state.dataHVACGlobal->StandardRatingsMyOneTimeFlag) {
            print(state.files.eio,
                  "{}\n",
                  "! <Chiller Standard Rating Information>, Component Type, Component Name, IPLV in SI Units {W/W}, IPLV in IP Units {Btu/W-h}");
            state.dataHVACGlobal->StandardRatingsMyOneTimeFlag = false;
        }

        {
            static constexpr std::string_view Format_991(" Chiller Standard Rating Information, {}, {}, {:.2R}, {:.2R}\n");
            if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricEIR) {

                print(state.files.eio, Format_991, "Chiller:Electric:EIR", ChillerName, IPLVValueSI, IPLVValueIP);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, ChillerName, "Chiller:Electric:EIR");

            } else if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR) {

                print(state.files.eio, Format_991, "Chiller:Electric:ReformulatedEIR", ChillerName, IPLVValueSI, IPLVValueIP);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechType, ChillerName, "Chiller:Electric:ReformulatedEIR");
            }
        }

        // Note: We don't want unit conversion, here, but it's ok since W/W will convert to itself since the column heading has "SI" as a hint
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechIPLVSI, ChillerName, IPLVValueSI, 2);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchMechIPLVIP, ChillerName, IPLVValueIP, 2);
    }

    void CheckCurveLimitsForIPLV(EnergyPlusData &state,
                                 std::string const &ChillerName,               // Name of Chiller
                                 DataPlant::PlantEquipmentType ChillerType,    // Type of Chiller - EIR or ReformulatedEIR
                                 DataPlant::CondenserType const CondenserType, // Type of Condenser - Air Cooled, Water Cooled or Evap Cooled
                                 int const CapFTempCurveIndex,                 // Index for the total cooling capacity modifier curve
                                 int const EIRFTempCurveIndex                  // Index for the energy input ratio modifier curve
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR            Chandan Sharma, FSEC
        //       DATE WRITTEN      January 2012
        //       MODIFIED          na
        //       RE-ENGINEERED     na

        // PURPOSE OF THIS SUBROUTINE:
        // Checks the limits of the various curves used in EIR chiller and returns .FALSE. if the limits do not include
        // the standard test condition(s).

        // Using/Aliasing
        using Curve::GetCurveMinMaxValues;
        using Curve::GetCurveName;

        // Following parameters are taken from AHRI 551/591,2011 Table 3
        Real64 constexpr HighEWTemp(30.0);       // Entering water temp in degrees C at full load capacity (85F)
        Real64 constexpr LowEWTemp(19.0);        // Entering water temp in degrees C at minimum reduced capacity (65F)
        Real64 constexpr OAHighEDBTemp(35.0);    // Outdoor air dry-bulb temp in degrees C at full load capacity (95F)
        Real64 constexpr OALowEDBTemp(12.78);    // Outdoor air dry-bulb temp in degrees C at part load capacity for AirCooled Chillers (55F)
        Real64 constexpr OAHighEWBTemp(24.0);    // Outdoor air wet-bulb temp in degrees C at full load capacity (75F)
        Real64 constexpr OALowEWBTemp(13.47);    // Outdoor air wet-bulb temp in degrees C at full load capacity for EvapCooled Chillers (56.25F)
        Real64 constexpr LeavingWaterTemp(6.67); // Evaporator leaving water temperature in degrees C [44 F]

        //  Minimum and Maximum independent variable limits from Total Cooling Capacity Function of Temperature Curve
        Real64 CapacityLWTempMin(0.0);           // Capacity modifier Min value (leaving water temp), from the Curve:BiQuadratic object
        Real64 CapacityLWTempMax(0.0);           // Capacity modifier Max value (leaving water temp), from the Curve:BiQuadratic object
        Real64 CapacityEnteringCondTempMin(0.0); // Capacity modifier Min value (entering cond temp), from the Curve:BiQuadratic object
        Real64 CapacityEnteringCondTempMax(0.0); // Capacity modifier Max value (entering cond temp), from the Curve:BiQuadratic object

        //  Minimum and Maximum independent variable limits from Energy Input Ratio (EIR) Function of Temperature Curve
        Real64 EIRLWTempMin(0.0);           // EIR modifier Min value (leaving water temp), from the Curve:BiQuadratic object
        Real64 EIRLWTempMax(0.0);           // EIR modifier Max value (leaving water temp), from the Curve:BiQuadratic object
        Real64 EIREnteringCondTempMin(0.0); // EIR modifier Min value (entering cond temp), from the Curve:BiQuadratic object
        Real64 EIREnteringCondTempMax(0.0); // EIR modifier Max value (entering cond temp), from the Curve:BiQuadratic object

        Real64 HighCondenserEnteringTempLimit(0.0); // High limit of entering condenser temperature
        Real64 LowCondenserEnteringTempLimit(0.0);  // Low limit of entering condenser temperature

        bool CapCurveIPLVLimitsExceeded(false); // Logical for capacity curve temperature limits being exceeded (IPLV calcs)
        bool EIRCurveIPLVLimitsExceeded(false); // Logical for EIR temperature limits being exceeded (IPLV calcs)

        GetCurveMinMaxValues(
            state, CapFTempCurveIndex, CapacityLWTempMin, CapacityLWTempMax, CapacityEnteringCondTempMin, CapacityEnteringCondTempMax);
        GetCurveMinMaxValues(state, EIRFTempCurveIndex, EIRLWTempMin, EIRLWTempMax, EIREnteringCondTempMin, EIREnteringCondTempMax);

        if (CondenserType == DataPlant::CondenserType::WaterCooled) {
            HighCondenserEnteringTempLimit = HighEWTemp;
            LowCondenserEnteringTempLimit = LowEWTemp;
        } else if (CondenserType == DataPlant::CondenserType::AirCooled) {
            HighCondenserEnteringTempLimit = OAHighEDBTemp;
            LowCondenserEnteringTempLimit = OALowEDBTemp;
        } else { // Evaporatively Cooled Condenser
            HighCondenserEnteringTempLimit = OAHighEWBTemp;
            LowCondenserEnteringTempLimit = OALowEWBTemp;
        }

        // Checking the limits of capacity modifying curve for temperatures (IPLV high and low test conditions)
        if (CapacityEnteringCondTempMax < HighCondenserEnteringTempLimit || CapacityEnteringCondTempMin > LowCondenserEnteringTempLimit ||
            CapacityLWTempMax < LeavingWaterTemp || CapacityLWTempMin > LeavingWaterTemp) {
            CapCurveIPLVLimitsExceeded = true;
        }
        // Checking the limits of EIR modifying curve for temperatures (IPLV high and low test conditions)
        if (EIREnteringCondTempMax < HighCondenserEnteringTempLimit || EIREnteringCondTempMin > LowCondenserEnteringTempLimit ||
            EIRLWTempMax < LeavingWaterTemp || EIRLWTempMin > LeavingWaterTemp) {
            EIRCurveIPLVLimitsExceeded = true;
        }

        // For IPLV:
        if (CapCurveIPLVLimitsExceeded || EIRCurveIPLVLimitsExceeded) {
            if (state.dataGlobal->DisplayExtraWarnings) {

                if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricEIR) {

                    ShowWarningError(
                        state,
                        format("Chiller:Electric:EIR = {}:  Integrated Part Load Value (IPLV) calculated is not at the AHRI test condition.",
                               ChillerName));
                } else if (ChillerType == DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR) {

                    ShowWarningError(
                        state,
                        format(
                            "Chiller:Electric:ReformulatedEIR = {}:  Integrated Part Load Value (IPLV) calculated is not at the AHRI test condition.",
                            ChillerName));
                }
                if (CapCurveIPLVLimitsExceeded) {
                    ShowContinueError(state,
                                      format(" Check limits in Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                             Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                             GetCurveName(state, CapFTempCurveIndex)));
                }
                if (EIRCurveIPLVLimitsExceeded) {
                    ShowContinueError(state,
                                      format(" Check limits in EIR Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                             Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                             GetCurveName(state, EIRFTempCurveIndex)));
                }
            }
        }
    }

    void CalcDXCoilStandardRating(
        EnergyPlusData &state,
        std::string const &DXCoilName,                             // Name of DX coil for which HSPF is calculated
        std::string const &DXCoilType,                             // Type of DX coil for which HSPF is calculated
        int const DXCoilType_Num,                                  // Integer Type of DX coil - heating or cooling
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
        Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023,
        Array1D<DataHeatBalance::RefrigCondenserType> CondenserType,
        ObjexxFCL::Optional_int_const
            RegionNum, // Region number for calculating HSPF of single speed DX heating coil //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional<Real64 const>
            MinOATCompressor, // Minimum OAT for heat pump compressor operation [C] //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional<Real64 const>
            OATempCompressorOn, // The outdoor temperature when the compressor is automatically turned //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional_bool_const
            OATempCompressorOnOffBlank, // Flag used to determine low temperature cut out factor //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional<HPdefrostControl const>
            DefrostControl,                              // defrost control; 1=timed, 2=on-demand //Autodesk:OPTIONAL Used without PRESENT check
        ObjexxFCL::Optional_bool_const ASHRAE127StdRprt, // true if user wishes to report ASHRAE 127 standard ratings
        ObjexxFCL::Optional<Real64 const> GrossRatedTotalCoolingCapacityVS, // Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}
        ObjexxFCL::Optional<Real64 const> RatedVolumetricAirFlowRateVS      //  Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, Chandan Sharma FSEC
        //       DATE WRITTEN   February 2010,
        //                      B. Nigusse, May 2010  Added EER and IEER Calculation
        //                      C. Sharma, March 2012  Added HSPF Calculation for single speed HP
        //                      B. Nigusse, August 2012 Added SEER Calculation for Multi-speed HP
        //                      B. Nigusse, November 2012 Added HSPF Calculation for Multi-speed HP

        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //     Calculates:
        //                 (1) Standard Rated (net) Cooling Capacity
        //                 (2) Seasonal Energy Efficiency Ratio (SEER)
        //                 (3) Energy Efficiency Ratio (EER),
        //                 (4) Integrated Energy Efficiency Ratio (IEER)
        //                 for Air-to-Air Direct Expansion Air Conditioner and Heat Pumps having a single-speed compressor,
        //                 fixed speed indoor supply air fan, and air-cooled condensers. Writes the result to EIO file.
        //                 (5) Heating Seasonal Performance Factor (HSPF) for Air-Source Direct Expansion Heat Pumps having
        //                  a single-speed compressor, fixed speed indoor supply air fan
        //                 (6) Standard Rated (net) Cooling Capacity; and
        //                 (7) Seasonal Energy Efficiency Ratio (SEER) for Air-to-Air Heat Pumps having multi-speed
        //                     compressor.
        //                 (8) Heating Seasonal Performance Factor (HSPF) for Air-to-Air Heat Pumps having multi-speed
        //                     compressor.
        // METHODOLOGY EMPLOYED:
        // (A) Methodology for calculating standard ratings for DX air conditioners
        //     (1) Obtains the rated condition parameters:
        //         Cooling capacity (User specified or Autosized Value)
        //         Rated Air volume flow rate through the DX Cooling Coil (User specified or autosized value)
        //     (2) Evaluates the total cooling coil capacity at AHRI test conditions 26.7C/19.4C/27.8C. Then net
        //         cooling capacity is determined from the total cooling capacity of the DX coil at the AHRI test
        //         conditions and accounting for the INDOOR supply air fan heat.
        //     (3) Calculates the electric power consumed by the DX Coil Unit (compressor + outdoor condenser fan).
        //         Evaluates the EIR capacity and flow fraction modifiers at 26.7C/19.4C/27.8C. The net electric
        //         power consumption is determined by adding the indoor fan electric power to the electric power
        //         consumption by the DX Coil Condenser Fan and Compressor at the AHRI test conditions.
        //     (4) The EER is evaluated from the total net cooling capacity and total electric power
        //         evaluated at the standard rated test conditions.  The IEER is a weighted value of the EER evaluated
        //         at four different capacities of 100%, 75%, 50% and 25%.  The reduced capacity EERs are evaluated
        //         at different outdoor coil entering air dry-bulb temperatures.
        // (B) Methodology for calculating standard ratings for DX air air source heat pumps
        //     (1) Obtains the rated condition parameters:
        //         heating capacity (User specified or Autosized Value), COP,  Rated Air volume flow rate through the
        //         DX Cooling Coil (User specified or autosized value) and Fan power per rated air flow rate
        //     (2) Evaluates the heating coil capacities for AHRI tests H1, H2 and H3 using the performance curves and
        //         input values specified at (1) above. Then net heating capacity is determined from the total heating capacity
        //         of the DX coil at the AHRI test conditions and accounting for the INDOOR supply air fan heat.
        //     (3) Calculates the electric power consumed by the DX Coil Unit (compressor + outdoor condenser fan).
        //         The net electric power consumption is determined by adding the indoor fan electric power to the
        //         electric power consumption by the DX Coil Condenser Fan and Compressor at the AHRI test conditions.
        //     (4) High Temperature Heating Standard (Net) Rating Capacity and Low Temperature Heating Standard (Net)
        //         Rating Capacity capacity are determined using tests H1 and H3 per ANSI/AHRI 210/240 2008.
        //     (5) The HSPF is evaluated from the total net heating capacity and total electric power
        //         evaluated at the standard rated test conditions. For user specified region number, the outdoor temperatures
        //         are Binned (grouped) and fractional bin hours for each bin over the entire heating season are taken
        //         from AHRI 210/240. Then for each bin, building load, heat pump energy and resistance space heating energy are
        //         calculated. The sum of building load divided by sum of heat pump and resistance space heating over the
        //         entire heating season gives the HSPF. The detailed calculation algorithms of calculating HSPF
        //         are described in Engineering Reference.
        // (C) Methodology for calculating standard ratings for Multi-Speed Heat Pumps
        //     Net Total Cooling Capacity and SEER
        //     (1) Obtains the rated condition parameters:
        //         Cooling capacity (User specified or Autosized Value)
        //         Rated Air volume flow rate through the DX Cooling Coil (User specified or autosized value)
        //     (2) Evaluates the total cooling coil capacity at AHRI A2 test conditions 26.7C/19.4C/35.0C. Then net
        //         cooling capacity is determined from the total cooling capacity of the DX coil at the AHRI A2 test
        //         conditions and accounting for the INDOOR supply air fan effect.  The net total cooling capacity
        //         is reported at the high (maximum) speed only.
        //     (3) Calculates the electric power consumed by the DX Coil Unit (compressor + outdoor condenser fan).
        //         Evaluates the EIR capacity and flow fraction modifiers at A2, B2, B1, and F1 test conditions per
        //         AHRI/ANSI Std. 210/240 test procedure for multi-speed compressor.  For any inter-
        //         mediate operating conditions (speed), the successive lower and the higher speed performance are
        //         weighed per the standard.  Electric Power consumption is determined by adding the indoor fan
        //         electric power to the electric power consumption by the outdoor DX Coil Fan and Compressor Power
        //         at the AHRI test conditions.  The net total cooling capacity is also corrected for the fan heat
        //         effect for SEER calculation.
        //     Net Heating Capacity and HSPF
        //     (4) Obtains the rated condition parameters:
        //         Heating capacity (User specified or Autosized Value)
        //         Rated Air volume flow rate through the DX Heating Coil (User specified or autosized value)
        //     (5) Evaluates the heating coil capacity at AHRI H12 test conditions 21.1C/15.6C/8.33C. Then net
        //         heating capacity is determined from the total heating capacity of the DX coil at the AHRI H12
        //         test conditions and accounting for the supply supply air fan effect.  The net heating capacity
        //         is reported at the high (maximum) speed only.
        //     (6) Calculates the electric power consumed by the DX Coil Unit (compressor + outdoor condenser fan).
        //         Evaluates the EIR capacity and flow fraction modifiers per AHRI/ANSI Std. 210/240 test procedures
        //         for two speed compressor (H01, H11, H21, H31, H12, H22, and H32 ). This procedure was modified
        //         for multispeed heat pumps. For any inter-mediate operating conditions (speed), the successive
        //         lower and the higher speed performance are weighed per the standard.
        //         Electric Power consumption is determined by adding the supply fan electric power to the electric
        //         power consumption by the outdoor DX Coil Fan and Compressor Power at the AHRI test conditions.
        //         The net heating capacity is also corrected for the fan heat effect for SEER calculation.
        // REFERENCES:
        // (1) ANSI/AHRI Standard 210/240-2008:  Standard for Performance Rating of Unitary Air-Conditioning and
        //                                       Air-Source Heat Pumps. Arlington, VA:  Air-Conditioning, Heating
        //                                       , and Refrigeration Institute.
        // (2) ANSI/AHRI Standard 340/360-2007:  Standard for Performance Rating of Commercial and Industrial
        //                                       Unitary Air-Conditioning and Heat Pump Equipment.  Arlington,
        //                                       VA:  Air-Conditioning, Heating, and Refrigeration Institute.

        // USE STATEMENTS:

        // Using/Aliasing
        using Curve::CurveValue;
        using Curve::GetCurveMinMaxValues;
        using HVAC::Coil_CoolingAirToAirVariableSpeed;
        using HVAC::CoilDX_CoolingSingleSpeed;
        using HVAC::CoilDX_HeatingEmpirical;
        using HVAC::CoilDX_MultiSpeedCooling;
        using HVAC::CoilDX_MultiSpeedHeating;

        // Argument array dimensioning
        RatedTotalCapacity.dim(ns);
        RatedCOP.dim(ns);
        CapFFlowCurveIndex.dim(ns);
        CapFTempCurveIndex.dim(ns);
        EIRFFlowCurveIndex.dim(ns);
        EIRFTempCurveIndex.dim(ns);
        PLFFPLRCurveIndex.dim(ns);
        RatedAirVolFlowRate.dim(ns);
        FanPowerPerEvapAirFlowRateFromInput.dim(ns);

        ////TODO: this will be passed as argument in this method later on
        // Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInputSEER2;
        FanPowerPerEvapAirFlowRateFromInput_2023.dim(ns);
        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // back on, if applicable, following automatic shut off. This field is
        // used only for HSPF calculation. [C]

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D<Real64> FanPowerPerEvapAirFlowRate(ns); // Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]

        // Intermediate values calculated from the inputs in the idf file
        int spnum; // compressor speed number

        // Calculated and reported to the EIO file
        Real64 SEER_User(0.0);     // Seasonal Energy Efficiency Ratio using user PLF curve in SI [W/W]
        Real64 SEER_Standard(0.0); // Seasonal Energy Efficiency Ratio using AHRI 210/240 PLF default curve & C_D in SI [W/W]
        Real64 EER(0.0);           // Energy Efficiency Ratio using AHRI 210-240 2017 in SI [W/W]
        Real64 IEER(0.0);          // Integrated Energy Efficiency Ratio in SI [W/W]

        // SEER2 ANSI/AHRI 210/240 Standard 2023 Ratings
        Real64 SEER2_User(0.0);     // Seasonal Energy Efficiency Ratio using user PLF curve in SI [W/W]
        Real64 SEER2_Standard(0.0); // Seasonal Energy Efficiency Ratio using AHRI 210/240 PLF default curve & C_D in SI [W/W]
        Real64 EER2(0.0);           // Energy Efficiency Ratio using AHRI 210/140 - 2023
        Real64 EER_2022(0.0);       // Energy Efficiency Ratio in SI [W/W]
        Real64 IEER_2022(0.0);      // Integrated Energy Efficiency Ratio in SI [W/W]

        Real64 HSPF(0.0);                       // Heating Seasonal Performance Factor in SI [W/W]
        Real64 NetHeatingCapRatedHighTemp(0.0); // Net Rated heating capacity at high temp [W]
        Real64 NetHeatingCapRatedLowTemp(0.0);  // Net Rated heating capacity at low temp [W]

        // HSPF2 ANSI/AHRI 210/240 Standard 2023 Ratings
        Real64 HSPF2_2023(0.0);                      // Heating Seasonal Performance Factor in SI [W/W]
        Real64 NetHeatingCapRatedHighTemp_2023(0.0); // Net Rated heating capacity at high temp [W]
        Real64 NetHeatingCapRatedLowTemp_2023(0.0);  // Net Rated heating capacity at low temp [W]

        Array1D<Real64> NetCoolingCapRated(ns);    // Net Cooling Coil capacity at Rated conditions, accounting for supply fan heat [W]
        Array1D<Real64> NetTotCoolingCapRated(16); // net total cooling capacity of DX Coils for the sixteen ASHRAE Std 127 Test conditions
        Array1D<Real64> TotElectricPowerRated(16); // total electric power of DX Coils for the sixteen ASHRAE Std 127 Test conditions

        Array1D<Real64> NetCoolingCapRated_2023(ns);    // Net Cooling Coil capacity at Rated conditions, accounting for supply fan heat [W]
        Array1D<Real64> NetTotCoolingCapRated_2023(16); // net total cooling capacity of DX Coils for the sixteen ASHRAE Std 127 Test conditions
        Array1D<Real64> TotElectricPowerRated_2023(16); // total electric power of DX Coils for the sixteen ASHRAE Std 127 Test conditions

        NetCoolingCapRated = 0.0;

        switch (DXCoilType_Num) {

        case CoilDX_CoolingSingleSpeed: { // Coil:Cooling:DX:SingleSpeed

            CheckCurveLimitsForStandardRatings(state,
                                               DXCoilName,
                                               DXCoilType,
                                               DXCoilType_Num,
                                               CapFTempCurveIndex(1),
                                               CapFFlowCurveIndex(1),
                                               EIRFTempCurveIndex(1),
                                               EIRFFlowCurveIndex(1),
                                               PLFFPLRCurveIndex(1));

            // Calculated Net Cooling Capacity, SEER, SEER Default, EER, and IEER of single speed DX cooling coils
            std::map<std::string, Real64> StandarRatingResults = SingleSpeedDXCoolingCoilStandardRatings(state,
                                                                                                         DXCoilName,
                                                                                                         DXCoilType,
                                                                                                         CapFTempCurveIndex(1),
                                                                                                         CapFFlowCurveIndex(1),
                                                                                                         EIRFTempCurveIndex(1),
                                                                                                         EIRFFlowCurveIndex(1),
                                                                                                         PLFFPLRCurveIndex(1),
                                                                                                         RatedTotalCapacity(1),
                                                                                                         RatedCOP(1),
                                                                                                         RatedAirVolFlowRate(1),
                                                                                                         FanPowerPerEvapAirFlowRateFromInput(1),
                                                                                                         FanPowerPerEvapAirFlowRateFromInput_2023(1),
                                                                                                         CondenserType(1));
            NetCoolingCapRated(1) = StandarRatingResults["NetCoolingCapRated"];
            SEER_User = StandarRatingResults["SEER_User"];
            SEER_Standard = StandarRatingResults["SEER_Standard"];
            EER = StandarRatingResults["EER"];
            IEER = StandarRatingResults["IEER"];

            NetCoolingCapRated_2023(1) = StandarRatingResults["NetCoolingCapRated2023"];
            SEER2_User = StandarRatingResults["SEER2_User"];
            SEER2_Standard = StandarRatingResults["SEER2_Standard"];
            EER_2022 = StandarRatingResults["EER_2022"];
            IEER_2022 = StandarRatingResults["IEER_2022"];

            // Writes the net rated cooling capacity, SEER, SEER Default, EER and IEER values to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated(1),
                               SEER_User * ConvFromSIToIP,
                               SEER_Standard * ConvFromSIToIP,
                               EER,
                               EER * ConvFromSIToIP,
                               IEER * ConvFromSIToIP,
                               NetHeatingCapRatedHighTemp,
                               NetHeatingCapRatedLowTemp,
                               HSPF * ConvFromSIToIP,
                               RegionNum,
                               false);

            // ANSI/AHRI 210/240 Std. 2023 Ratings
            // Writes the net rated cooling capacity, SEER2_USER, SEER2_Standard, EER and IEER values to the EIO file and standard tabular output
            // tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated_2023(1),
                               SEER2_User * ConvFromSIToIP,
                               SEER2_Standard * ConvFromSIToIP,
                               EER_2022,
                               EER_2022 * ConvFromSIToIP,
                               IEER_2022 * ConvFromSIToIP,
                               NetHeatingCapRatedHighTemp_2023,
                               NetHeatingCapRatedLowTemp_2023,
                               HSPF2_2023 * ConvFromSIToIP,
                               RegionNum,
                               true);

            if (ASHRAE127StdRprt) {
                DXCoolingCoilDataCenterStandardRatings(state,
                                                       DXCoilName,
                                                       DXCoilType,
                                                       CapFTempCurveIndex(1),
                                                       CapFFlowCurveIndex(1),
                                                       EIRFTempCurveIndex(1),
                                                       EIRFFlowCurveIndex(1),
                                                       PLFFPLRCurveIndex(1),
                                                       RatedTotalCapacity(1),
                                                       RatedCOP(1),
                                                       RatedAirVolFlowRate(1),
                                                       FanPowerPerEvapAirFlowRateFromInput(1),
                                                       NetTotCoolingCapRated,
                                                       TotElectricPowerRated);
                ReportDXCoolCoilDataCenterApplication(state, DXCoilType, DXCoilName, DXCoilType_Num, NetTotCoolingCapRated, TotElectricPowerRated);
            }
            break;
        }
        case CoilDX_HeatingEmpirical: { // Coil:Heating:DX:SingleSpeed

            CheckCurveLimitsForStandardRatings(state,
                                               DXCoilName,
                                               DXCoilType,
                                               DXCoilType_Num,
                                               CapFTempCurveIndex(1),
                                               CapFFlowCurveIndex(1),
                                               EIRFTempCurveIndex(1),
                                               EIRFFlowCurveIndex(1),
                                               PLFFPLRCurveIndex(1));
            // Calculate the standard ratings for single speed DX heating coil
            std::map<std::string, Real64> StandardRatingsResults =
                SingleSpeedDXHeatingCoilStandardRatings(state,
                                                        DXCoilType,
                                                        RatedTotalCapacity(1),
                                                        RatedCOP(1),
                                                        CapFFlowCurveIndex(1),
                                                        CapFTempCurveIndex(1),
                                                        EIRFFlowCurveIndex(1),
                                                        EIRFTempCurveIndex(1),
                                                        RatedAirVolFlowRate(1),
                                                        FanPowerPerEvapAirFlowRateFromInput(1),
                                                        FanPowerPerEvapAirFlowRateFromInput_2023(1),
                                                        RegionNum,
                                                        MinOATCompressor,
                                                        OATempCompressorOn,
                                                        OATempCompressorOnOffBlank,
                                                        DefrostControl);
            NetHeatingCapRatedHighTemp = StandardRatingsResults["NetHeatingCapRated"];
            NetHeatingCapRatedLowTemp = StandardRatingsResults["NetHeatingCapH3Test"];
            HSPF = StandardRatingsResults["HSPF"];

            NetHeatingCapRatedHighTemp_2023 = StandardRatingsResults["NetHeatingCapRated_2023"];
            NetHeatingCapRatedLowTemp_2023 = StandardRatingsResults["NetHeatingCapH3Test_2023"];
            HSPF2_2023 = StandardRatingsResults["HSPF2_2023"];
            IEER_2022 = StandardRatingsResults["IEER_2022"];
            // Writes the HSPF value to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated(1),
                               SEER_User * ConvFromSIToIP,
                               SEER_Standard * ConvFromSIToIP,
                               EER,
                               EER * ConvFromSIToIP,
                               IEER * ConvFromSIToIP,
                               NetHeatingCapRatedHighTemp,
                               NetHeatingCapRatedLowTemp,
                               HSPF * ConvFromSIToIP,
                               RegionNum,
                               false);

            // ANSI/AHRI 210/240 Std. 2023 Ratings
            // Writes the HSPF2 value to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated_2023(1),
                               SEER2_User * ConvFromSIToIP,
                               SEER2_Standard * ConvFromSIToIP,
                               EER_2022,
                               EER_2022 * ConvFromSIToIP,
                               IEER_2022 * ConvFromSIToIP,
                               NetHeatingCapRatedHighTemp_2023,
                               NetHeatingCapRatedLowTemp_2023,
                               HSPF2_2023 * ConvFromSIToIP,
                               RegionNum,
                               true);
            break;
        }
        case CoilDX_MultiSpeedCooling: { // Coil:Cooling:DX:MultiSpeed,

            for (spnum = 1; spnum <= ns; ++spnum) {
                CheckCurveLimitsForStandardRatings(state,
                                                   DXCoilName,
                                                   DXCoilType,
                                                   DXCoilType_Num,
                                                   CapFTempCurveIndex(spnum),
                                                   CapFFlowCurveIndex(spnum),
                                                   EIRFTempCurveIndex(spnum),
                                                   EIRFFlowCurveIndex(spnum),
                                                   PLFFPLRCurveIndex(spnum));
            }
            // Calculate the standard ratings for multispeed DX cooling coil
            std::map<std::string, Real64> StandardRatingsResult = MultiSpeedDXCoolingCoilStandardRatings(state,
                                                                                                         DXCoilType,
                                                                                                         DXCoilName,
                                                                                                         CapFTempCurveIndex,
                                                                                                         CapFFlowCurveIndex,
                                                                                                         EIRFTempCurveIndex,
                                                                                                         EIRFFlowCurveIndex,
                                                                                                         PLFFPLRCurveIndex,
                                                                                                         RatedTotalCapacity,
                                                                                                         RatedCOP,
                                                                                                         RatedAirVolFlowRate,
                                                                                                         FanPowerPerEvapAirFlowRateFromInput,
                                                                                                         FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                                                         ns,
                                                                                                         CondenserType);
            NetCoolingCapRated(ns) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
            SEER_User = StandardRatingsResult["SEER_User"];
            SEER_Standard = StandardRatingsResult["SEER_Standard"];
            EER = StandardRatingsResult["EER"];

            NetCoolingCapRated_2023(ns) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
            SEER2_User = StandardRatingsResult["SEER2_User"];
            SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
            EER2 = StandardRatingsResult["EER2"];

            IEER_2022 = StandardRatingsResult["IEER_2022"];
            EER_2022 = StandardRatingsResult["EER_2022"];

            // Writes the SEER value to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated(ns),
                               SEER_User * ConvFromSIToIP,
                               SEER_Standard * ConvFromSIToIP,
                               EER,
                               EER * ConvFromSIToIP,
                               0.0,
                               0.0,
                               0.0,
                               0.0,
                               0,
                               false);

            // Writes the SEER value to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated_2023(ns),
                               SEER2_User * ConvFromSIToIP,
                               SEER2_Standard * ConvFromSIToIP,
                               EER_2022,
                               EER_2022 * ConvFromSIToIP,
                               IEER_2022 * ConvFromSIToIP,
                               0.0,
                               0.0,
                               0.0,
                               0,
                               true);

            break;
        }
        case CoilDX_MultiSpeedHeating: { // Coil:Heating:DX:MultiSpeed

            for (spnum = 1; spnum <= ns; ++spnum) {
                CheckCurveLimitsForStandardRatings(state,
                                                   DXCoilName,
                                                   DXCoilType,
                                                   DXCoilType_Num,
                                                   CapFTempCurveIndex(spnum),
                                                   CapFFlowCurveIndex(spnum),
                                                   EIRFTempCurveIndex(spnum),
                                                   EIRFFlowCurveIndex(spnum),
                                                   PLFFPLRCurveIndex(spnum));
            }
            // Calculate Net heating capacity and HSPF & HSPF2 of multispeed DX heating coils
            std::map<std::string, Real64> StandardRatingsResult = MultiSpeedDXHeatingCoilStandardRatings(state,
                                                                                                         DXCoilName,
                                                                                                         DXCoilType,
                                                                                                         CapFTempCurveIndex,
                                                                                                         CapFFlowCurveIndex,
                                                                                                         EIRFTempCurveIndex,
                                                                                                         EIRFFlowCurveIndex,
                                                                                                         PLFFPLRCurveIndex,
                                                                                                         RatedTotalCapacity,
                                                                                                         RatedCOP,
                                                                                                         RatedAirVolFlowRate,
                                                                                                         FanPowerPerEvapAirFlowRateFromInput,
                                                                                                         FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                                                         ns,
                                                                                                         RegionNum,
                                                                                                         MinOATCompressor,
                                                                                                         OATempCompressorOn,
                                                                                                         OATempCompressorOnOffBlank,
                                                                                                         DefrostControl);

            NetHeatingCapRatedHighTemp = StandardRatingsResult["NetHeatingCapRatedHighTemp"];
            NetHeatingCapRatedLowTemp = StandardRatingsResult["NetHeatingCapRatedLowTemp"];
            HSPF = StandardRatingsResult["HSPF"];

            NetHeatingCapRatedHighTemp_2023 = StandardRatingsResult["NetHeatingCapRatedHighTemp_2023"];
            NetHeatingCapRatedLowTemp_2023 = StandardRatingsResult["NetHeatingCapRatedLowTemp_2023"];
            HSPF2_2023 = StandardRatingsResult["HSPF2_2023"];

            IEER_2022 = StandardRatingsResult["IEER_2022"];
            // ANSI/AHRI Std. 2017 Ratings
            // Writes the HSPF value to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated(ns),
                               SEER_User * ConvFromSIToIP,
                               SEER_Standard * ConvFromSIToIP,
                               EER,
                               EER * ConvFromSIToIP,
                               IEER * ConvFromSIToIP,
                               NetHeatingCapRatedHighTemp,
                               NetHeatingCapRatedLowTemp,
                               HSPF * ConvFromSIToIP,
                               RegionNum,
                               false);

            // ANSI/AHRI 210/240 Std. 2023 Ratings
            // Writes the HSPF2 value to the EIO file and standard tabular output tables
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated(ns),
                               SEER_User * ConvFromSIToIP,
                               SEER_Standard * ConvFromSIToIP,
                               EER,
                               EER * ConvFromSIToIP,
                               IEER_2022 * ConvFromSIToIP,
                               NetHeatingCapRatedHighTemp_2023,
                               NetHeatingCapRatedLowTemp_2023,
                               HSPF2_2023 * ConvFromSIToIP,
                               RegionNum,
                               true);

            break;
        }
        case Coil_CoolingAirToAirVariableSpeed: {
            for (spnum = 1; spnum <= ns; ++spnum) {
                CheckCurveLimitsForStandardRatings(state,
                                                   DXCoilName,
                                                   DXCoilType,
                                                   DXCoilType_Num,
                                                   CapFTempCurveIndex(spnum),
                                                   CapFFlowCurveIndex(spnum),
                                                   EIRFTempCurveIndex(spnum),
                                                   EIRFFlowCurveIndex(spnum),
                                                   PLFFPLRCurveIndex(spnum));
            }

            // Calculate the standard ratings for multispeed DX cooling coil
            std::map<std::string, Real64> StandardRatingsResult = VariableSpeedDXCoolingCoilStandardRatings(state,
                                                                                                            DXCoilType,
                                                                                                            DXCoilName,
                                                                                                            CapFTempCurveIndex,
                                                                                                            CapFFlowCurveIndex,
                                                                                                            EIRFTempCurveIndex,
                                                                                                            EIRFFlowCurveIndex,
                                                                                                            PLFFPLRCurveIndex(1),
                                                                                                            RatedTotalCapacity,
                                                                                                            RatedCOP,
                                                                                                            RatedAirVolFlowRate,
                                                                                                            FanPowerPerEvapAirFlowRateFromInput,
                                                                                                            FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                                                            ns,
                                                                                                            CondenserType(1),
                                                                                                            GrossRatedTotalCoolingCapacityVS,
                                                                                                            RatedVolumetricAirFlowRateVS);

            NetCoolingCapRated_2023(ns) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
            SEER2_User = StandardRatingsResult["SEER2_User"];
            SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
            EER2 = StandardRatingsResult["EER2"];

            IEER_2022 = StandardRatingsResult["IEER_2022"];
            EER_2022 = StandardRatingsResult["EER_2022"];

            NetCoolingCapRated(ns) = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];

            // Writes the SEER2 & IEER 2022 value to the EIO file and standard tabular output tables | 2023
            ReportDXCoilRating(state,
                               DXCoilType,
                               DXCoilName,
                               DXCoilType_Num,
                               NetCoolingCapRated_2023(ns),
                               SEER2_User * ConvFromSIToIP,
                               SEER2_Standard * ConvFromSIToIP,
                               EER_2022,
                               EER_2022 * ConvFromSIToIP,
                               IEER_2022 * ConvFromSIToIP,
                               0.0,
                               0.0,
                               0.0,
                               0,
                               true);
            break;
        }
        default:
            break; //... other DX Coil types will follow here
        }
    }

    void CalcTwoSpeedDXCoilRating(EnergyPlusData &state,
                                  std::string const &DXCoilName,
                                  std::string const &DXCoilType,
                                  int const DXCoilType_Num,
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
                                  int const PLFFPLRCurveIndex)
    {
        std::map<std::string, Real64> StandardRatingsResult;

        // Intermediate values calculated from the inputs in the idf file
        // SEER2 ANSI/AHRI 210/240 Standard 2023 Ratings
        Real64 SEER2_User(0.0);                     // Seasonal Energy Efficiency Ratio using user PLF curve in SI [W/W]
        Real64 SEER2_Standard(0.0);                 // Seasonal Energy Efficiency Ratio using AHRI 210/240 PLF default curve & C_D in SI [W/W]
        Real64 EER2(0.0);                           // Energy Efficiency Ratio using AHRI 210/240 - 2023
        Real64 NetCoolingCapRatedMaxSpeed2023(0.0); // net cooling capacity at maximum speed

        Real64 EER_2022(0.0);  // Energy Efficiency Ratio in SI [W/W]
        Real64 IEER_2022(0.0); // Integrated Energy Efficiency Ratio in SI [W/W]
        Real64 NetCoolingCapRated2022(0.0);

        Real64 EER(0.0);
        Real64 NetCoolingCapRated(0.0);

        StandardRatingsResult = TwoSpeedDXCoilStandardRatings(state,
                                                              DXCoilName,
                                                              DXCoilType,
                                                              DXCoilType_Num,
                                                              RatedTotalCapacity,
                                                              RatedTotCap2,
                                                              RatedCOP,
                                                              RatedCOP2,
                                                              CapFFlowCurveIndex, // only hs
                                                              CapFTempCurveIndex,
                                                              CCapFTemp2,
                                                              EIRFFlowCurveIndex, // only hs
                                                              EIRFTempCurveIndex,
                                                              EIRFTemp2,
                                                              RatedAirVolFlowRate,
                                                              RatedAirVolFlowRate2,
                                                              FanPowerPerEvapAirFlowRate_2023,
                                                              FanPowerPerEvapAirFlowRate_2023_LowSpeed,
                                                              CondenserType,
                                                              PLFFPLRCurveIndex);

        // From SEER2 implementation
        NetCoolingCapRatedMaxSpeed2023 = StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"];
        SEER2_User = StandardRatingsResult["SEER2_User"];
        SEER2_Standard = StandardRatingsResult["SEER2_Standard"];
        EER2 = StandardRatingsResult["EER2"];

        // From IEER 2022 implementation
        NetCoolingCapRated2022 = StandardRatingsResult["NetCoolingCapRatedMaxSpeed"];
        IEER_2022 = StandardRatingsResult["IEER_2022"];
        EER_2022 = StandardRatingsResult["EER_2022"];

        NetCoolingCapRated = NetCoolingCapRatedMaxSpeed2023;
        // for the Report routine to correctly initialize with EER value.
        if (StandardRatingsResult["SEER2_Standard"] > 0.0) {
            EER = EER2;
        } else if (StandardRatingsResult["IEER_2022"] > 0.0) {
            EER = EER_2022;
            NetCoolingCapRated = NetCoolingCapRated2022;
        }

        // Writes the SEER & IEER value to the EIO file and standard tabular output tables
        ReportDXCoilRating(state,
                           DXCoilType,
                           DXCoilName,
                           DXCoilType_Num,
                           NetCoolingCapRated,
                           SEER2_User * ConvFromSIToIP,
                           SEER2_Standard * ConvFromSIToIP,
                           EER,
                           EER * ConvFromSIToIP,
                           IEER_2022 * ConvFromSIToIP,
                           0.0,
                           0.0,
                           0.0,
                           0,
                           true);
    }

    std::map<std::string, Real64> TwoSpeedDXCoilStandardRatings(EnergyPlusData &state,
                                                                std::string const &DXCoilName,
                                                                std::string const &DXCoilType,
                                                                int const &DXCoilType_Num,
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
                                                                int const &PLFFPLRCurveIndex)
    {
        std::map<std::string, Real64> StandardRatingsResult;

        // Intermediate values calculated from the inputs in the idf file
        // SEER2 ANSI/AHRI 210/240 Standard 2023 Ratings
        Real64 SEER2_User(0.0);                     // Seasonal Energy Efficiency Ratio using user PLF curve in SI [W/W]
        Real64 SEER2_Standard(0.0);                 // Seasonal Energy Efficiency Ratio using AHRI 210/240 PLF default curve & C_D in SI [W/W]
        Real64 EER2(0.0);                           // Energy Efficiency Ratio using AHRI 210/240 - 2023
        Real64 NetCoolingCapRatedMaxSpeed2023(0.0); // net cooling capacity at maximum speed

        Real64 EER_2022(0.0);  // Energy Efficiency Ratio in SI [W/W]
        Real64 IEER_2022(0.0); // Integrated Energy Efficiency Ratio in SI [W/W]
        Real64 NetCoolingCapRated2022(0.0);

        int constexpr ns = 2;
        Array1D<Real64> NetTotCoolingCapRated_2023(16); // net total cooling capacity of DX Coils for the sixteen ASHRAE Std 127 Test conditions
        Array1D<Real64> TotElectricPowerRated_2023(16); // total electric power of DX Coils for the sixteen ASHRAE Std 127 Test conditions

        Array1D_int TSCCapFTemp;
        TSCCapFTemp.push_back(CapFTempCurveIndex(1));
        TSCCapFTemp.push_back(CCapFTemp2);

        Array1D<Real64> TSFanPowerPerEvapAirFlowRate2023;
        TSFanPowerPerEvapAirFlowRate2023.push_back(FanPowerPerEvapAirFlowRate_2023(1));
        TSFanPowerPerEvapAirFlowRate2023.push_back(FanPowerPerEvapAirFlowRate_2023_LowSpeed(1));

        Array1D<Real64> TSRatedTotCap;
        TSRatedTotCap.push_back(RatedTotalCapacity(1));
        TSRatedTotCap.push_back(RatedTotCap2);

        Array1D<Real64> TSRatedAirVolFlowRate;
        TSRatedAirVolFlowRate.push_back(RatedAirVolFlowRate(1));
        TSRatedAirVolFlowRate.push_back(RatedAirVolFlowRate2);

        Array1D_int TSEIRFTemp;
        TSEIRFTemp.push_back(EIRFTempCurveIndex(1));
        TSEIRFTemp.push_back(EIRFTemp2);

        Array1D<Real64> TSRatedCOP;
        TSRatedCOP.push_back(RatedCOP(1));
        TSRatedCOP.push_back(RatedCOP2);

        for (int spnum = 1; spnum <= ns; ++spnum) {
            // TODO:BPS Implement Two Speed Case :
            CheckCurveLimitsForStandardRatings(state,
                                               DXCoilName,
                                               DXCoilType,
                                               DXCoilType_Num,
                                               TSCCapFTemp(spnum),
                                               CapFFlowCurveIndex(1), // only HS
                                               TSEIRFTemp(spnum),
                                               EIRFFlowCurveIndex(1), // Only HS
                                               PLFFPLRCurveIndex);    // Only Coil Level
        }

        if (RatedTotalCapacity(1) > 0.0 && RatedAirVolFlowRate(1) > 0.0) {

            Real64 TotCapTempModFac =
                Curve::CurveValue(state, CapFTempCurveIndex(1), CoolingCoilInletAirWetBulbTempRated, CoilInletAirCoolDryBulbIEER);
            Real64 TotCapFlowModFac = Curve::CurveValue(state, CapFFlowCurveIndex(1), AirMassFlowRatioRated);
            NetCoolingCapRatedMaxSpeed2023 =
                RatedTotalCapacity(1) * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(1);
            // TODO: Commercial and industrial unitary air-conditioning condensing units with a capacity greater than 135,000 Btu/h (39564.59445
            // Watts) as defined in ANSI/AHRI Standard 365(I-P). | Scope 2.2.6 (ANSI/AHRI 340-360 2022)

            if (CondenserType(1) == DataHeatBalance::RefrigCondenserType::Air) {
                // ANSI/AHRI 210/240 Standard 2023 only applies for solely to Air Cooled Cooling Coils
                // Also, this standard applies to factory-made Unitary Air-conditioners and Unitary Air-source Heat Pumps with
                // capacities less than 65,000 Btu/h (19049.61955 Watts) | Section 2.1
                // Removal of water-cooled and evaporatively-cooled products from the scope | Foreword (ANSI/AHRI 210-240 2023)

                // SEER2 Calculations ANSI/AHRI 210/240 Standard 2023
                std::tie(NetCoolingCapRatedMaxSpeed2023, SEER2_User, SEER2_Standard, EER2) =
                    TwoSpeedDXCoolingCoilSEER2(state,
                                               // 2, // nsp will always be 2 in case of Two Speed Coil
                                               CapFFlowCurveIndex, // only HS
                                               TSRatedTotCap,
                                               TSCCapFTemp,
                                               TSFanPowerPerEvapAirFlowRate2023,
                                               TSRatedAirVolFlowRate,
                                               EIRFFlowCurveIndex, // only HS
                                               TSRatedCOP,
                                               TSEIRFTemp,
                                               PLFFPLRCurveIndex); // only coil level
            }

            // Calculate the IEER 2022 Standard ratings for Two Speed DX cooling coil | AHRI 340/360
            std::tie(IEER_2022, NetCoolingCapRated2022, EER_2022) = IEERCalculationTwoSpeed(state,
                                                                                            DXCoilType,
                                                                                            CondenserType,
                                                                                            TSCCapFTemp,
                                                                                            TSRatedTotCap,
                                                                                            CapFFlowCurveIndex,
                                                                                            TSFanPowerPerEvapAirFlowRate2023,
                                                                                            TSRatedAirVolFlowRate,
                                                                                            TSEIRFTemp,
                                                                                            TSRatedCOP,
                                                                                            EIRFFlowCurveIndex);
        } else {
            ShowSevereError(state,
                            "Standard Ratings: Coil:Cooling:DX:TwoSpeed either has a zero rated total cooling capacity or zero air flow rate. "
                            "Standard ratings cannot be calculated.");
        }

        // From SEER2 implementation
        StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRatedMaxSpeed2023;
        StandardRatingsResult["SEER2_User"] = SEER2_User;
        StandardRatingsResult["SEER2_Standard"] = SEER2_Standard;
        StandardRatingsResult["EER2"] = EER2;

        // From IEER2 implementation
        StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] = NetCoolingCapRated2022;
        StandardRatingsResult["IEER_2022"] = IEER_2022;
        StandardRatingsResult["EER_2022"] = EER_2022;

        return StandardRatingsResult;
    }

    Real64 SingleSpeedHeatingHSPF(const Real64 NetHeatingCapRated,
                                  ObjexxFCL::Optional_int_const RegionNum,
                                  const Real64 NetHeatingCapH3Test,
                                  const Real64 ElecPowerH3Test,
                                  const Real64 ElecPowerRated,
                                  const Real64 NetHeatingCapH2Test,
                                  const Real64 ElecPowerH2Test,
                                  ObjexxFCL::Optional<Real64 const> MinOATCompressor,
                                  ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,
                                  ObjexxFCL::Optional<Real64 const> OATempCompressorOn,
                                  ObjexxFCL::Optional<const HPdefrostControl> DefrostControl)
    {
        int BinNum;                           // bin number counter
        Int64 StandardDHRNum;                 // Integer counter for standardized DHRs
        Real64 CheckCOP(0.0);                 // Checking COP at an outdoor bin temperature against unity [-]
        Real64 DesignHeatingRequirement(0.0); // The amount of heating required to maintain a given indoor temperature
        // at a particular outdoor design temperature.  [W]
        Real64 DesignHeatingRequirementMin(0.0); // minimum design heating requirement [W]
        Real64 FractionalBinHours(0.0);          // Fractional bin hours for the heating season  [-]
        Real64 BuildingLoad(0.0);                // Building space conditioning load corresponding to an outdoor bin temperature [W]

        Real64 HeatingModeLoadFactor(0.0); // Heating mode load factor corresponding to an outdoor bin temperature  [-]
        Real64 NetHeatingCapReduced(0.0);  // Net Heating Coil capacity corresponding to an outdoor bin temperature [W]
        Real64 TotalBuildingLoad(0.0);     // Sum of building load over the entire heating season [W]

        Real64 TotalElectricalEnergy(0.0); // Sum of electrical energy consumed by the heatpump over the heating season [W]
        Real64 DemandDeforstCredit(1.0);   // A factor to adjust HSPF if coil has demand defrost control  [-]

        Real64 ElectricalPowerConsumption(0.0);            // Electrical power corresponding to an outdoor bin temperature [W]
        Real64 HeatPumpElectricalEnergy(0.0);              // Heatpump electrical energy corresponding to an outdoor bin temperature [W]
        Real64 TotalHeatPumpElectricalEnergy(0.0);         // Sum of Heatpump electrical energy over the entire heating season [W]
        Real64 ResistiveSpaceHeatingElectricalEnergy(0.0); // resistance heating electrical energy corresponding to an
        // outdoor bin temperature [W]
        Real64 TotalResistiveSpaceHeatingElectricalEnergy(0.0); // Sum of resistance heating electrical energy over the
        // entire heating season [W]
        Real64 PartLoadFactor;
        Real64 LowTempCutOutFactor(0.0); // Factor which corresponds to compressor operation depending on outdoor temperature
        Real64 OATempCompressorOff(0.0); // Minimum outdoor air temperature to turn the compressor off, [C]
        Real64 HSPF(0.0);

        if (RegionNum == 5) {
            DesignHeatingRequirementMin = NetHeatingCapRated;
        } else {
            DesignHeatingRequirementMin = NetHeatingCapRated * 1.8 * (18.33 - OutdoorDesignTemperature[static_cast<int64_t>(RegionNum) - 1]) / 60.0;
        }

        for (StandardDHRNum = 0; StandardDHRNum < TotalNumOfStandardDHRs - 1; ++StandardDHRNum) {
            if (StandardDesignHeatingRequirement[StandardDHRNum] <= DesignHeatingRequirementMin &&
                StandardDesignHeatingRequirement[StandardDHRNum + 1] >= DesignHeatingRequirementMin) {
                if ((DesignHeatingRequirementMin - StandardDesignHeatingRequirement[StandardDHRNum]) >
                    (StandardDesignHeatingRequirement[StandardDHRNum + 1] - DesignHeatingRequirementMin)) {
                    DesignHeatingRequirementMin = StandardDesignHeatingRequirement[StandardDHRNum + 1];
                } else {
                    DesignHeatingRequirementMin = StandardDesignHeatingRequirement[StandardDHRNum];
                }
            }
        }
        if (StandardDesignHeatingRequirement[0] >= DesignHeatingRequirementMin) {
            DesignHeatingRequirement = StandardDesignHeatingRequirement[0];
        } else if (StandardDesignHeatingRequirement[TotalNumOfStandardDHRs - 1] <= DesignHeatingRequirementMin) {
            DesignHeatingRequirement = StandardDesignHeatingRequirement[TotalNumOfStandardDHRs - 1];
        } else {
            DesignHeatingRequirement = DesignHeatingRequirementMin;
        }

        for (BinNum = 0; BinNum < TotalNumOfTemperatureBins[static_cast<int64_t>(RegionNum) - 1]; ++BinNum) {

            FractionalBinHours = FracBinHoursAtOutdoorBinTemp[static_cast<int64_t>(RegionNum) - 1][BinNum];

            BuildingLoad = (18.33 - OutdoorBinTemperature[BinNum]) / (18.33 - OutdoorDesignTemperature[static_cast<int64_t>(RegionNum) - 1]) *
                           CorrectionFactor * DesignHeatingRequirement;

            if ((OutdoorBinTemperature[BinNum] <= -8.33) || (OutdoorBinTemperature[BinNum] >= 7.22)) {
                NetHeatingCapReduced =
                    NetHeatingCapH3Test + (NetHeatingCapRated - NetHeatingCapH3Test) * (OutdoorBinTemperature[BinNum] + 8.33) / (16.67);
                ElectricalPowerConsumption = ElecPowerH3Test + (ElecPowerRated - ElecPowerH3Test) * (OutdoorBinTemperature[BinNum] + 8.33) / (16.67);
            } else {
                NetHeatingCapReduced =
                    NetHeatingCapH3Test + (NetHeatingCapH2Test - NetHeatingCapH3Test) * (OutdoorBinTemperature[BinNum] + 8.33) / (10.0);
                ElectricalPowerConsumption = ElecPowerH3Test + (ElecPowerH2Test - ElecPowerH3Test) * (OutdoorBinTemperature[BinNum] + 8.33) / (10.0);
            }

            if (NetHeatingCapReduced != 0.0) {
                HeatingModeLoadFactor = BuildingLoad / NetHeatingCapReduced;
            }

            if (HeatingModeLoadFactor > 1.0) {
                HeatingModeLoadFactor = 1.0;
            }

            PartLoadFactor = 1 - CyclicDegradationCoeff * (1 - HeatingModeLoadFactor);

            if (ElectricalPowerConsumption != 0.0) {
                CheckCOP = NetHeatingCapReduced / ElectricalPowerConsumption;
            }

            OATempCompressorOff = MinOATCompressor;

            if (CheckCOP < 1.0) {
                LowTempCutOutFactor = 0.0;
            } else {
                if (!OATempCompressorOnOffBlank) {
                    if (OutdoorBinTemperature[BinNum] <= OATempCompressorOff) {
                        LowTempCutOutFactor = 0.0;
                    } else if (OutdoorBinTemperature[BinNum] <= OATempCompressorOn) {
                        LowTempCutOutFactor = 0.5;
                    } else {
                        LowTempCutOutFactor = 1.0;
                    }
                } else {
                    LowTempCutOutFactor = 1.0;
                }
            }

            if (PartLoadFactor != 0.0) {
                HeatPumpElectricalEnergy =
                    (HeatingModeLoadFactor * ElectricalPowerConsumption * LowTempCutOutFactor) * FractionalBinHours / PartLoadFactor;
            }

            ResistiveSpaceHeatingElectricalEnergy =
                (BuildingLoad - HeatingModeLoadFactor * NetHeatingCapReduced * LowTempCutOutFactor) * FractionalBinHours;

            TotalBuildingLoad += (BuildingLoad * FractionalBinHours);

            TotalHeatPumpElectricalEnergy += HeatPumpElectricalEnergy;

            TotalResistiveSpaceHeatingElectricalEnergy += ResistiveSpaceHeatingElectricalEnergy;
        }

        TotalElectricalEnergy = TotalHeatPumpElectricalEnergy + TotalResistiveSpaceHeatingElectricalEnergy;

        if (DefrostControl == HPdefrostControl::Timed) {
            DemandDeforstCredit = 1.0; // Timed defrost control
        } else {
            DemandDeforstCredit = 1.03; // Demand defrost control
        }

        if (TotalElectricalEnergy != 0.0) {
            HSPF = TotalBuildingLoad * DemandDeforstCredit / TotalElectricalEnergy;
        }
        return HSPF;
    }

    Real64 SingleSpeedHeatingHSPF2(const Real64 NetHeatingCapRated_2023,
                                   ObjexxFCL::Optional_int_const RegionNum,
                                   const Real64 NetHeatingCapH3Test_2023,
                                   const Real64 ElecPowerH3Test2023,
                                   const Real64 ElecPowerRated2023,
                                   const Real64 NetHeatingCapH2Test2023,
                                   const Real64 ElecPowerH2Test2023,
                                   ObjexxFCL::Optional<Real64 const> MinOATCompressor,
                                   ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,
                                   ObjexxFCL::Optional<Real64 const> OATempCompressorOn,
                                   ObjexxFCL::Optional<const HPdefrostControl> DefrostControl)
    {
        Real64 DesignHeatingRequirement2023(0.0);   // HSPF2 minimum design heating requirement [W]
        Real64 FractionalBinHours2023(0.0);         // HSPF2 Fractional bin hours for the heating season  [-]
        Real64 BuildingLoad2023(0.0);               // HSPF2 Building space conditioning load corresponding to an outdoor bin temperature [W]
        Real64 NetHeatingCapReduced2023(0.0);       // HSPF2 Net Heating Coil capacity corresponding to an outdoor bin temperature [W]
        Real64 ElectricalPowerConsumption2023(0.0); // HSPF2 Electrical power corresponding to an outdoor bin temperature [W]
        Real64 HeatingModeLoadFactor2023(0.0);      // HSPF2 Heating mode load factor corresponding to an outdoor bin temperature  [-]
        Real64 PartLoadFactor2023;
        Real64 CheckCOP2023(0.0);                              // HSPF2 Checking COP at an outdoor bin temperature against unity [-]
        Real64 OATempCompressorOff2023(0.0);                   // HSPF2 Minimum outdoor air temperature to turn the compressor off, [C]
        Real64 LowTempCutOutFactor2023(0.0);                   // Factor which corresponds to compressor operation depending on outdoor temperature
        Real64 HeatPumpElectricalEnergy2023(0.0);              // HSPF2 Heatpump electrical energy corresponding to an outdoor bin temperature [W]
        Real64 ResistiveSpaceHeatingElectricalEnergy2023(0.0); // HSPF2 resistance heating electrical energy corresponding to an
        // outdoor bin temperature [W]
        Real64 TotalBuildingLoad2023(0.0);                          // Sum of building load over the entire heating season [W]
        Real64 TotalHeatPumpElectricalEnergy2023(0.0);              // HSPF2 Sum of Heatpump electrical energy over the entire heating season [W]
        Real64 TotalResistiveSpaceHeatingElectricalEnergy2023(0.0); // Sum of resistance heating electrical energy over the
        // entire heating season [W]
        Real64 TotalElectricalEnergy2023(0.0); // HSPF2 Sum of electrical energy consumed by the heatpump over the heating season [W]
        Real64 DemandDeforstCredit2023(1.0);   // A factor to adjust HSPF2 if coil has demand defrost control  [-]
        Real64 HSPF2_2023(0.0);

        // For ANSI/AHRI 210/240 Standard 2023 | Concept of DHRI min and max is removed
        // Section 11.2.2.1 Equation 11.104  which suggests QAFull is used instead of DHRI min
        DesignHeatingRequirement2023 = NetHeatingCapRated_2023;
        Int64 RN = static_cast<int64_t>(RegionNum);

        for (int BinNum2023 = 0; BinNum2023 < TotalNumOfTemperatureBinsHSPF2[RN - 1]; ++BinNum2023) {

            FractionalBinHours2023 = FracBinHoursAtOutdoorBinTempHSPF2[RN - 1][BinNum2023];

            BuildingLoad2023 = (ZoneLoadTemperature[RN - 1] - OutdoorBinTemperature[BinNum2023]) /
                               (ZoneLoadTemperature[RN - 1] - OutdoorDesignTemperature[RN - 1]) * SpeedLoadFactor[RN - 1] *
                               DesignHeatingRequirement2023;
            if ((OutdoorBinTemperature[BinNum2023] <= -8.33) || (OutdoorBinTemperature[BinNum2023] >= 7.22)) {
                NetHeatingCapReduced2023 = NetHeatingCapH3Test_2023 + (NetHeatingCapRated_2023 - NetHeatingCapH3Test_2023) *
                                                                          (OutdoorBinTemperature[BinNum2023] + 8.33) / (16.67);
                ElectricalPowerConsumption2023 =
                    ElecPowerH3Test2023 + (ElecPowerRated2023 - ElecPowerH3Test2023) * (OutdoorBinTemperature[BinNum2023] + 8.33) / (16.67);
            } else {
                NetHeatingCapReduced2023 = NetHeatingCapH3Test_2023 +
                                           (NetHeatingCapH2Test2023 - NetHeatingCapH3Test_2023) * (OutdoorBinTemperature[BinNum2023] + 8.33) / (10.0);
                ElectricalPowerConsumption2023 =
                    ElecPowerH3Test2023 + (ElecPowerH2Test2023 - ElecPowerH3Test2023) * (OutdoorBinTemperature[BinNum2023] + 8.33) / (10.0);
            }
            if (NetHeatingCapReduced2023 != 0.0) {
                HeatingModeLoadFactor2023 = BuildingLoad2023 / NetHeatingCapReduced2023;
            }

            if (HeatingModeLoadFactor2023 > 1.0) {
                HeatingModeLoadFactor2023 = 1.0;
            }
            PartLoadFactor2023 = 1 - CyclicHeatingDegradationCoeffHSPF2 * (1 - HeatingModeLoadFactor2023);
            if (ElectricalPowerConsumption2023 != 0.0) {
                CheckCOP2023 = NetHeatingCapReduced2023 / ElectricalPowerConsumption2023;
            }
            OATempCompressorOff2023 = MinOATCompressor;
            if (CheckCOP2023 < 1.0) {
                LowTempCutOutFactor2023 = 0.0;
            } else {
                if (!OATempCompressorOnOffBlank) {
                    if (OutdoorBinTemperature[BinNum2023] <= OATempCompressorOff2023) {
                        LowTempCutOutFactor2023 = 0.0;
                    } else if (OutdoorBinTemperature[BinNum2023] <= OATempCompressorOn) {
                        LowTempCutOutFactor2023 = 0.5;
                    } else {
                        LowTempCutOutFactor2023 = 1.0;
                    }
                } else {
                    LowTempCutOutFactor2023 = 1.0;
                }
            }

            if (PartLoadFactor2023 != 0.0) {
                HeatPumpElectricalEnergy2023 = (HeatingModeLoadFactor2023 * ElectricalPowerConsumption2023 * LowTempCutOutFactor2023) *
                                               FractionalBinHours2023 / PartLoadFactor2023;
            }

            ResistiveSpaceHeatingElectricalEnergy2023 =
                (BuildingLoad2023 - HeatingModeLoadFactor2023 * NetHeatingCapReduced2023 * LowTempCutOutFactor2023) * FractionalBinHours2023;

            TotalBuildingLoad2023 += (BuildingLoad2023 * FractionalBinHours2023);

            TotalHeatPumpElectricalEnergy2023 += HeatPumpElectricalEnergy2023;

            TotalResistiveSpaceHeatingElectricalEnergy2023 += ResistiveSpaceHeatingElectricalEnergy2023;
        }
        TotalElectricalEnergy2023 = TotalHeatPumpElectricalEnergy2023 + TotalResistiveSpaceHeatingElectricalEnergy2023;

        if (DefrostControl == HPdefrostControl::Timed) {
            DemandDeforstCredit2023 = 1.0; // Timed defrost control
        } else {
            DemandDeforstCredit2023 = 1.03; // Demand defrost control
        }

        if (TotalElectricalEnergy2023 != 0.0) {
            HSPF2_2023 = TotalBuildingLoad2023 * DemandDeforstCredit2023 / TotalElectricalEnergy2023;
        }
        return HSPF2_2023;
    }

    std::map<std::string, Real64> SingleSpeedDXHeatingCoilStandardRatings(
        EnergyPlusData &state,
        [[maybe_unused]] std::string const &DXCoilType,            // Type of DX coil for which HSPF is calculated
        Real64 const RatedTotalCapacity,                           // Reference capacity of DX coil [W]
        Real64 const RatedCOP,                                     // Reference coefficient of performance [W/W]
        int const CapFFlowCurveIndex,                              // Index for the capacity as a function of flow fraction modifier curve
        int const CapFTempCurveIndex,                              // Index for the capacity as a function of temperature modifier curve
        int const EIRFFlowCurveIndex,                              // Index for the EIR as a function of flow fraction modifier curve
        int const EIRFTempCurveIndex,                              // Index for the EIR as a function of temperature modifier curve
        Real64 const RatedAirVolFlowRate,                          // Rated air volume flow rate [m3/s]
        Real64 const FanPowerPerEvapAirFlowRateFromInput,          // 2017 Fan power per air volume flow rate [W/(m3/s)]
        Real64 const FanPowerPerEvapAirFlowRateFromInput_2023,     // 2023 Fan power per air volume flow rate [W/(m3/s)]
        ObjexxFCL::Optional_int_const RegionNum,                   // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor,        // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn,      // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank, // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl // defrost control; 1=timed, 2=on-demand
    )
    {
        Real64 NetHeatingCapRated(0.0);  // Net Heating Coil capacity at Rated conditions,
        Real64 NetHeatingCapH3Test(0.0); // Net Heating Coil capacity at H3 test conditions
        Real64 HSPF(0.0);                // seasonal energy efficiency ratio of multi speed DX cooling coil

        // ANSI/AHRI 210/240 Standard 2023
        Real64 NetHeatingCapRated_2023(0.0);  // Net Heating Coil capacity at Rated conditions,
        Real64 NetHeatingCapH3Test_2023(0.0); // Net Heating Coil capacity at H3 test conditions
        Real64 HSPF2_2023(0.0);               // seasonal energy efficiency ratio of multi speed DX cooling coil
        Real64 IEER_2022(0.0);
        std::map<std::string, Real64> StandardRatingsResults;
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Chandan Sharma
        //       DATE WRITTEN   February 2012
        //       MODIFIED       B Nigusse, December 2012
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // na

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using Curve::CurveValue;
        using Curve::GetCurveMinMaxValues;
        using Curve::GetCurveName;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // back on, if applicable, following automatic shut off. This field is
        // used only for HSPF calculation. [C]
        // accounting for supply fan heat [W]
        // accounting for supply fan heat [W]

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na
        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TotalHeatingCapRated(0.0);            // Heating Coil capacity at Rated conditions, without accounting supply fan heat [W]
        Real64 EIRRated(0.0);                        // EIR at Rated conditions [-]
        Real64 TotCapTempModFacRated(0.0);           // Total capacity as a function of temperature modifier at rated conditions [-]
        Real64 EIRTempModFacRated(0.0);              // EIR as a function of temperature modifier at rated conditions [-]
        Real64 TotalHeatingCapH2Test(0.0);           // Heating Coil capacity at H2 test conditions, without accounting supply fan heat [W]
        Real64 TotalHeatingCapH3Test(0.0);           // Heating Coil capacity at H3 test conditions, without accounting supply fan heat [W]
        Real64 CapTempModFacH2Test(0.0);             // Total capacity as a function of temperature modifier at H2 test conditions [-]
        Real64 EIRTempModFacH2Test(0.0);             // EIR as a function of temperature modifier at H2 test conditions [-]
        Real64 EIRH2Test(0.0);                       // EIR at H2 test conditions [-]
        Real64 CapTempModFacH3Test(0.0);             // Total capacity as a function of temperature modifier at H3 test conditions [-]
        Real64 EIRTempModFacH3Test(0.0);             // EIR as a function of temperature modifier at H3 test conditions [-]
        Real64 EIRH3Test(0.0);                       // EIR at H3 test conditions [-]
        Real64 TotCapFlowModFac(0.0);                // Total capacity modifier (function of actual supply air flow vs rated flow)
        Real64 EIRFlowModFac(0.0);                   // EIR modifier (function of actual supply air flow vs rated flow)
        Real64 FanPowerPerEvapAirFlowRate(0.0);      // 2017 Fan power per air volume flow rate [W/(m3/s)]
        Real64 FanPowerPerEvapAirFlowRate_2023(0.0); // 2023 Fan power per air volume flow rate [W/(m3/s)]

        Real64 ElecPowerRated;  // Total system power at Rated conditions accounting for supply fan heat [W]
        Real64 ElecPowerH2Test; // Total system power at H2 test conditions accounting for supply fan heat [W]
        Real64 ElecPowerH3Test; // Total system power at H3 test conditions accounting for supply fan heat [W]

        // For ANSI/AHRI 210/240 Standard 2023
        Real64 ElecPowerRated2023;  // Total system power at Rated conditions accounting for supply fan heat [W]
        Real64 ElecPowerH2Test2023; // Total system power at H2 test conditions accounting for supply fan heat [W]
        Real64 ElecPowerH3Test2023; // Total system power at H3 test conditions accounting for supply fan heat [W]

        Real64 NetHeatingCapH2Test;     // Net Heating Coil capacity at H2 test conditions accounting for supply fan heat [W]
        Real64 NetHeatingCapH2Test2023; // (for 2023 Standard) Net Heating Coil capacity at H2 test conditions accounting for supply fan heat [W]

        // Calculate the supply air fan electric power consumption.  The electric power consumption is estimated
        // using either user supplied or AHRI default value for fan power per air volume flow rate
        if (FanPowerPerEvapAirFlowRateFromInput <= 0.0) {
            FanPowerPerEvapAirFlowRate = DefaultFanPowerPerEvapAirFlowRate;
        } else {
            FanPowerPerEvapAirFlowRate = FanPowerPerEvapAirFlowRateFromInput;
        }
        if (FanPowerPerEvapAirFlowRateFromInput_2023 <= 0.0) {
            FanPowerPerEvapAirFlowRate_2023 = DefaultFanPowerPerEvapAirFlowRateSEER2;
        } else {
            FanPowerPerEvapAirFlowRate_2023 = FanPowerPerEvapAirFlowRateFromInput_2023;
        }

        TotCapFlowModFac = CurveValue(state, CapFFlowCurveIndex, AirMassFlowRatioRated);
        EIRFlowModFac = CurveValue(state, EIRFFlowCurveIndex, AirMassFlowRatioRated);

        {
            if (state.dataCurveManager->curves(CapFTempCurveIndex)->numDims == 1) {
                TotCapTempModFacRated = CurveValue(state, CapFTempCurveIndex, HeatingOutdoorCoilInletAirDBTempRated);

                CapTempModFacH2Test = CurveValue(state, CapFTempCurveIndex, HeatingOutdoorCoilInletAirDBTempH2Test);

                CapTempModFacH3Test = CurveValue(state, CapFTempCurveIndex, HeatingOutdoorCoilInletAirDBTempH3Test);
            } else {
                TotCapTempModFacRated =
                    CurveValue(state, CapFTempCurveIndex, HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempRated);

                CapTempModFacH2Test =
                    CurveValue(state, CapFTempCurveIndex, HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH2Test);

                CapTempModFacH3Test =
                    CurveValue(state, CapFTempCurveIndex, HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH3Test);
            }
        }

        {
            if (state.dataCurveManager->curves(EIRFTempCurveIndex)->numDims == 1) {
                EIRTempModFacRated = CurveValue(state, EIRFTempCurveIndex, HeatingOutdoorCoilInletAirDBTempRated);

                EIRTempModFacH2Test = CurveValue(state, EIRFTempCurveIndex, HeatingOutdoorCoilInletAirDBTempH2Test);

                EIRTempModFacH3Test = CurveValue(state, EIRFTempCurveIndex, HeatingOutdoorCoilInletAirDBTempH3Test);
            } else {
                EIRTempModFacRated =
                    CurveValue(state, EIRFTempCurveIndex, HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempRated);

                EIRTempModFacH2Test =
                    CurveValue(state, EIRFTempCurveIndex, HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH2Test);

                EIRTempModFacH3Test =
                    CurveValue(state, EIRFTempCurveIndex, HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH3Test);
            }
        }

        TotalHeatingCapRated = RatedTotalCapacity * TotCapTempModFacRated * TotCapFlowModFac;
        NetHeatingCapRated = TotalHeatingCapRated + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        NetHeatingCapRated_2023 = TotalHeatingCapRated + FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;

        TotalHeatingCapH2Test = RatedTotalCapacity * CapTempModFacH2Test * TotCapFlowModFac;
        NetHeatingCapH2Test = TotalHeatingCapH2Test + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        NetHeatingCapH2Test2023 = TotalHeatingCapH2Test + FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;

        TotalHeatingCapH3Test = RatedTotalCapacity * CapTempModFacH3Test * TotCapFlowModFac;
        NetHeatingCapH3Test = TotalHeatingCapH3Test + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        NetHeatingCapH3Test_2023 = TotalHeatingCapH3Test + FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;

        // check curves value
        if (TotCapTempModFacRated < 0.0 || CapTempModFacH2Test < 0.0 || CapTempModFacH3Test < 0.0 || EIRTempModFacRated < 0.0 ||
            EIRTempModFacH2Test < 0.0 || EIRTempModFacH3Test < 0.0) {
            if (TotCapTempModFacRated < 0.0) {
                ShowSevereError(
                    state,
                    format(" Invalid Total Heating Capacity Function of Temperature Curve value = {:.2R}, Curve Type = {}, Curve Name = {}",
                           TotCapTempModFacRated,
                           Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                           GetCurveName(state, CapFTempCurveIndex)));
                ShowContinueError(state,
                                  " ...Net heating capacity at high temperature is set to zero. The curve value must be > 0. Check the curve.");
                NetHeatingCapRated = 0.0;
                NetHeatingCapRated_2023 = 0.0;
            }
            if (CapTempModFacH3Test < 0.0) {
                ShowSevereError(
                    state,
                    format(" Invalid Total Heating Capacity Function of Temperature Curve value = {:.2R}, Curve Type = {}, Curve Name = {}",
                           CapTempModFacH3Test,
                           Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                           GetCurveName(state, CapFTempCurveIndex)));
                ShowContinueError(state, " ...Net heating capacity at low temperature is set to zero. The curve value must be > 0. Check the curve.");
                NetHeatingCapH3Test = 0.0;
                NetHeatingCapH3Test_2023 = 0.0;
            }
            if (CapTempModFacH2Test < 0.0) {
                ShowSevereError(
                    state,
                    format(" Invalid Total Heating Capacity Function of Temperature Curve value = {:.2R}, Curve Type = {}, Curve Name = {}",
                           CapTempModFacH2Test,
                           Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                           GetCurveName(state, CapFTempCurveIndex)));
                ShowContinueError(state, " ...HSPF calculation is incorrect. The curve value must be > 0. Check the curve.");
                NetHeatingCapH3Test = 0.0;
                NetHeatingCapH3Test_2023 = 0.0;
            }
            // check EIR curve values
            if (EIRTempModFacRated < 0.0) {
                ShowSevereError(state,
                                format(" Invalid EIR Function of Temperature Curve value = {:.2R}, Curve Type = {}, Curve Name = {}",
                                       EIRTempModFacRated,
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                ShowContinueError(state, " ...HSPF calculation is incorrect. The curve value must be > 0. Check the curve.");
            }
            if (EIRTempModFacH2Test < 0.0) {
                ShowSevereError(state,
                                format(" Invalid EIR Function of Temperature Curve value = {:.2R}, Curve Type = {}, Curve Name = {}",
                                       EIRTempModFacH2Test,
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                ShowContinueError(state, " ...HSPF calculation is incorrect. The curve value must be > 0. Check the curve.");
            }
            if (EIRTempModFacH3Test < 0.0) {
                ShowSevereError(state,
                                format(" Invalid EIR Function of Temperature Curve value = {:.2R}, Curve Type = {}, Curve Name = {}",
                                       EIRTempModFacH3Test,
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                ShowContinueError(state, " ...HSPF calculation is incorrect. The curve value must be > 0. Check the curve.");
            }
            ShowContinueError(state, " ...HSPF value has been reset to 0.0 and simulation is continuing.");
            HSPF = 0.0;
            HSPF2_2023 = 0.0;
            IEER_2022 = 0.0;

            StandardRatingsResults["NetHeatingCapRated"] = NetHeatingCapRated;
            StandardRatingsResults["NetHeatingCapH3Test"] = NetHeatingCapH3Test;
            StandardRatingsResults["HSPF"] = HSPF;
            StandardRatingsResults["NetHeatingCapRated_2023"] = NetHeatingCapRated_2023;
            StandardRatingsResults["NetHeatingCapH3Test_2023"] = NetHeatingCapH3Test_2023;
            StandardRatingsResults["HSPF2_2023"] = HSPF2_2023;
            return StandardRatingsResults;
        }

        if (RatedCOP > 0.0) { // RatedCOP <= 0.0 is trapped in GetInput, but keep this as "safety"

            EIRRated = EIRTempModFacRated * EIRFlowModFac / RatedCOP;
            EIRH2Test = EIRTempModFacH2Test * EIRFlowModFac / RatedCOP;
            EIRH3Test = EIRTempModFacH3Test * EIRFlowModFac / RatedCOP;
        }

        ElecPowerRated = EIRRated * TotalHeatingCapRated + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        ElecPowerH2Test = EIRH2Test * TotalHeatingCapH2Test + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        ElecPowerH3Test = EIRH3Test * TotalHeatingCapH3Test + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;

        ElecPowerRated2023 = EIRRated * TotalHeatingCapRated + FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;
        ElecPowerH2Test2023 = EIRH2Test * TotalHeatingCapH2Test + FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;
        ElecPowerH3Test2023 = EIRH3Test * TotalHeatingCapH3Test + FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;

        HSPF2_2023 = SingleSpeedHeatingHSPF2(NetHeatingCapRated_2023,
                                             RegionNum,
                                             NetHeatingCapH3Test_2023,
                                             ElecPowerH3Test2023,
                                             ElecPowerRated2023,
                                             NetHeatingCapH2Test2023,
                                             ElecPowerH2Test2023,
                                             MinOATCompressor,
                                             OATempCompressorOnOffBlank,
                                             OATempCompressorOn,
                                             DefrostControl);

        HSPF = SingleSpeedHeatingHSPF(NetHeatingCapRated,
                                      RegionNum,
                                      NetHeatingCapH3Test,
                                      ElecPowerH3Test,
                                      ElecPowerRated,
                                      NetHeatingCapH2Test,
                                      ElecPowerH2Test,
                                      MinOATCompressor,
                                      OATempCompressorOnOffBlank,
                                      OATempCompressorOn,
                                      DefrostControl);

        StandardRatingsResults["NetHeatingCapRated"] = NetHeatingCapRated;
        StandardRatingsResults["NetHeatingCapH3Test"] = NetHeatingCapH3Test;
        StandardRatingsResults["HSPF"] = HSPF;
        StandardRatingsResults["NetHeatingCapRated_2023"] = NetHeatingCapRated_2023;
        StandardRatingsResults["NetHeatingCapH3Test_2023"] = NetHeatingCapH3Test_2023;
        StandardRatingsResults["HSPF2_2023"] = HSPF2_2023;
        return StandardRatingsResults;
    }

    std::tuple<Real64, Real64> IEERSingleSpeedCooling(EnergyPlus::EnergyPlusData &state,
                                                      const int CapFTempCurveIndex,
                                                      const Real64 RatedTotalCapacity,
                                                      const Real64 TotCapFlowModFac,
                                                      const Real64 FanPowerPerEvapAirFlowRate,
                                                      const Real64 RatedAirVolFlowRate,
                                                      const int EIRFTempCurveIndex,
                                                      const Real64 RatedCOP,
                                                      const Real64 EIRFlowModFac)
    {
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int constexpr NumOfReducedCap(4);                  // Number of reduced capacity test conditions (100%,75%,50%,and 25%)
        Real64 TotCapTempModFac(0.0);                      // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 OutdoorUnitInletAirDryBulbTempReduced(0.0); // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]
        Real64 NetCoolingCapReduced(0.0);                  // Net Cooling Coil capacity at reduced conditions, accounting for supply fan heat [W]
        Real64 EIRTempModFac(0.0);                         // EIR modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 EIR(0.0);                                   // Energy Efficiency Ratio at AHRI test conditions for SEER [-]
        Real64 LoadFactor(0.0);                            // Fractional "on" time for last stage at the desired reduced capacity, (dimensionless)
        Real64 DegradationCoeff(0.0);                      // Degradation coefficient, (dimensionless)
        Real64 ElecPowerReducedCap(0.0);                   // Net power consumption (Cond Fan+Compressor) at reduced test condition [W]
        Real64 EERReduced(0.0);                            // EER at reduced capacity test conditions (100%, 75%, 50%, and 25%)
        Real64 IEER = 0.0;                                 // Integrated energy efficiency ratio of single speed DX cooling coil
        Real64 NetCoolingCapRated = 0.0;                   // net cooling capacity of single speed DX cooling coil

        // Calculate the net cooling capacity at the rated conditions (19.44C WB and 35.0C DB )
        TotCapTempModFac = Curve::CurveValue(state, CapFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        NetCoolingCapRated = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        // RedCapNum : Integer counter for reduced capacity
        for (int RedCapNum = 0; RedCapNum < NumOfReducedCap; ++RedCapNum) {
            // get the outdoor air dry bulb temperature for the reduced capacity test conditions
            if (ReducedPLR[RedCapNum] > 0.444) {
                OutdoorUnitInletAirDryBulbTempReduced = 5.0 + 30.0 * ReducedPLR[RedCapNum];
            } else {
                OutdoorUnitInletAirDryBulbTempReduced = OADBTempLowReducedCapacityTest;
            }
            TotCapTempModFac =
                Curve::CurveValue(state, CapFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
            NetCoolingCapReduced = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
            EIRTempModFac = Curve::CurveValue(state, EIRFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
            if (RatedCOP > 0.0) {
                EIR = EIRTempModFac * EIRFlowModFac / RatedCOP;
            } else {
                EIR = 0.0;
            }
            if (NetCoolingCapReduced > 0.0) {
                LoadFactor = ReducedPLR[RedCapNum] * NetCoolingCapRated / NetCoolingCapReduced;
            } else {
                LoadFactor = 1.0;
            }
            DegradationCoeff = 1.130 - 0.130 * LoadFactor;
            ElecPowerReducedCap = DegradationCoeff * EIR * (RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac);
            EERReduced = (LoadFactor * NetCoolingCapReduced) / (LoadFactor * ElecPowerReducedCap + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate);
            IEER += IEERWeightingFactor[RedCapNum] * EERReduced;
        }
        return std::make_tuple(IEER, NetCoolingCapRated);
    }

    Real64 GetIEEREquationResult(const Real64 A, const Real64 B, const Real64 C, const Real64 D)
    {
        return (0.020 * A) + (0.617 * B) + (0.238 * C) + (0.125 * D);
    }

    Real64 GetOutdoorUnitInletAirDryBulbTempReduced(Real64 const ReducedPLR, DataHeatBalance::RefrigCondenserType const CondenserType)
    {
        Real64 OutdoorUnitInletAirDryBulbTempReduced(0.0);
        // As per Table 9. IEER Part-Load Rating Conditions | AHRI Std.340/360-2022(IP)
        if (ReducedPLR == 0.25) {
            if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
                // Entering Dry Bulb Temperature (OAT)
                OutdoorUnitInletAirDryBulbTempReduced = 18.33; // 65F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                // Entering Condenser Water Temperature (EWT)
                OutdoorUnitInletAirDryBulbTempReduced = 12.77; // 55F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
                // Entering Air Wet-bulb/Air Dry-bulb/Makeup Water Temperature EWB/DB/MW
                // OutdoorUnitInletAirDryBulbTempReduced = 52.8F/65.0F/77.0F
                OutdoorUnitInletAirDryBulbTempReduced = 18.33;
            }
        } else if (ReducedPLR == 0.50) {
            if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
                OutdoorUnitInletAirDryBulbTempReduced = 20; // 68F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                OutdoorUnitInletAirDryBulbTempReduced = 16.66; // 62F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
                // OutdoorUnitInletAirDryBulbTempReduced = 57.5F / 68.0F / 77.0F EWB / DB / MW
                OutdoorUnitInletAirDryBulbTempReduced = 20;
            }
        } else if (ReducedPLR == 0.75) {
            if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
                OutdoorUnitInletAirDryBulbTempReduced = 27.5; // 81.5F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                OutdoorUnitInletAirDryBulbTempReduced = 23.05; // 73.5F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
                // OutdoorUnitInletAirDryBulbTempReduced = 66.2F / 81.5F / 77.0F EWB / DB / MW
                OutdoorUnitInletAirDryBulbTempReduced = 27.5; // 81.5F
            }
        } else if (ReducedPLR == 1.0) {
            if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
                OutdoorUnitInletAirDryBulbTempReduced = 35; // 95.0F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
                OutdoorUnitInletAirDryBulbTempReduced = 29.44; // 85.0F
            } else if (CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
                // OutdoorUnitInletAirDryBulbTempReduced = 75.0F / 95.0F / 85.0F EWB / DB / MW
                OutdoorUnitInletAirDryBulbTempReduced = 35;
            }
        }
        return OutdoorUnitInletAirDryBulbTempReduced;
    }

    Real64 CalculateInterMediateEER(EnergyPlus::EnergyPlusData &state,
                                    Real64 QAFull,
                                    Real64 OutdoorUnitInletAirDryBulbTempReduced,
                                    int CapFTempCurveIndex,
                                    Real64 CoolingCoilInletAirWetBulbTempRated,
                                    Real64 RatedTotalCapacity,
                                    Real64 TotCapFlowModFac,
                                    Real64 FanPowerPerEvapAirFlowRate_2023,
                                    Real64 RatedAirVolFlowRate,
                                    Real64 EIRFTempCurveIndex,
                                    Real64 RatedCOP,
                                    Real64 EIRFlowModFac,
                                    Real64 ReducedPLR)
    {
        Real64 TotCapTempModFac =
            Curve::CurveValue(state, CapFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
        // TBD: Discuss TotCapTempModFac Calculation for both Evap and Water Cooled.
        // This will be our Qlx
        Real64 NetCoolingCapReduced =
            RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;
        Real64 EIRTempModFac =
            Curve::CurveValue(state, EIRFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);

        Real64 EIR(0.0);
        if (RatedCOP > 0.0) {
            EIR = EIRTempModFac * EIRFlowModFac / RatedCOP;
        } else {
            EIR = 0.0;
        }
        Real64 EER(0.0); // Energy Efficiency Rating

        Real64 CD(0.0);               // Degradation Cofficient, (Btu/h)/(Btu/h)
        Real64 LF(0.0);               // Fraction "on" time for the last stage at the tested load Point | Load Factor
        Real64 PL = ReducedPLR * 100; // Percent Load

        Real64 Qlx = NetCoolingCapReduced; // Part Load Net Capacity (Btu/h) | Previously NetCoolingCapReduced
        if (Qlx > 0.0) {
            LF = ((PL / 100) * QAFull) / Qlx;
        } else {
            LF = 1.0;
        }
        // PC - Compressor power at the lowest machine unloading point operating at the applicable part-load Rating condition, W
        // PCD - Condenser Section Power, at the applicable part-load Rating condition, W
        Real64 PIF(0.0); // Indoor Fan Power, W
        Real64 PCT(0.0); // Control Circuit Power and any auxilary Power, W
        Real64 q(0.0);   // Cooling Capacity at the lowest machine unloading point operating at the applicable part-load Rating condition, Btu/h

        q = Qlx;
        PIF = FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate; // Calculated for each Speed
        PCT = 0;                                                     // Control Circuit Power  and any auxilary Power not in Energy Plus Object.
        Real64 PC_plus_PCD = EIR * (RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac);
        CD = (-0.13 * LF) + 1.13; // DegradationCoeff
        EER = (LF * q) / (LF * (CD * (PC_plus_PCD)) + PIF + PCT);
        return EER;
    }

    std::tuple<Real64, Real64, Real64, Real64> SEER2CalulcationCurveFit(EnergyPlusData &state,
                                                                        [[maybe_unused]] std::string const &CoilType,
                                                                        EnergyPlus::CoilCoolingDXCurveFitOperatingMode operatingMode)
    {
        Real64 EEER2(0.0);
        Real64 SEER2_User(0.0);
        Real64 SEER2_Standard(0.0);
        Real64 NetCoolingCapRated2023(0.0); // QAFull

        Array1D<int> MSCCapFTemp;
        Array1D<Real64> MSRatedTotCap;
        Array1D<int> MSCCapAirFFlow;
        Array1D<Real64> MSRatedEvaporatorFanPowerPerVolumeFlowRate2023;
        Array1D<Real64> MSRatedAirVolFlowRate;
        Array1D<int> MSEIRFTemp;
        Array1D<Real64> MSRatedCOP;
        Array1D<int> MSEIRAirFFlow;
        Array1D<int> MSPLRFPLF;

        int const nsp = operatingMode.speeds.size();

        for (int i = 0; i < nsp; ++i) {
            CoilCoolingDXCurveFitSpeed speed = operatingMode.speeds[i];
            MSCCapFTemp.push_back(speed.indexCapFT);

            // Calculate the rated cooling capacity for the speed using Gross Total Cooling Capacity
            // and Gross Total Cooling Capacity Fraction of the speed.
            MSRatedTotCap.push_back(speed.rated_total_capacity); // get the capcity at each speed bymultiplying this fraCTION WITH the gross.
            MSCCapAirFFlow.push_back(speed.indexCapFFF);
            MSRatedEvaporatorFanPowerPerVolumeFlowRate2023.push_back(speed.rated_evap_fan_power_per_volume_flow_rate_2023);
            // Calculate the rated evap air flow rate for the speed using Rated Evaporator Air flow Rate
            // and Rated Evaporator Air flow fraction of the speed
            MSRatedAirVolFlowRate.push_back(speed.evap_air_flow_rate);

            MSEIRFTemp.push_back(speed.indexEIRFT);
            MSRatedCOP.push_back(speed.ratedCOP);
            MSEIRAirFFlow.push_back(speed.indexEIRFFF);
            MSPLRFPLF.push_back(speed.indexPLRFPLF);
        }

        std::tie(NetCoolingCapRated2023, SEER2_User, SEER2_Standard, EEER2) =
            VariableSpeedDXCoolingCoilSEER2(state,
                                            nsp,
                                            MSCCapAirFFlow,
                                            MSRatedTotCap,
                                            MSCCapFTemp,
                                            MSRatedEvaporatorFanPowerPerVolumeFlowRate2023,
                                            MSRatedAirVolFlowRate,
                                            MSEIRAirFFlow,
                                            MSRatedCOP,
                                            MSEIRFTemp,
                                            MSPLRFPLF);

        return std::make_tuple(NetCoolingCapRated2023, SEER2_User, SEER2_Standard, EEER2);
    }

    std::tuple<Real64, Real64, Real64>
    IEERCalulcationCurveFit(EnergyPlusData &state, std::string const &CoilType, EnergyPlus::CoilCoolingDXCurveFitOperatingMode operatingMode)
    {
        Real64 IEER_2022(0.0);
        Real64 EER_2022(0.0);
        Real64 NetCoolingCapRated2022(0.0); // QAFull

        Array1D<int> MSCCapFTemp;
        Array1D<Real64> MSRatedTotCap;
        Array1D<int> MSCCapAirFFlow;
        Array1D<Real64> MSRatedEvaporatorFanPowerPerVolumeFlowRate2023;
        Array1D<Real64> MSRatedAirVolFlowRate;
        Array1D<int> MSEIRFTemp;
        Array1D<Real64> MSRatedCOP;
        Array1D<int> MSEIRAirFFlow;

        int const nsp = operatingMode.speeds.size();

        for (int i = 0; i < nsp; ++i) {
            CoilCoolingDXCurveFitSpeed speed = operatingMode.speeds[i];
            MSCCapFTemp.push_back(speed.indexCapFT);
            // Calculate the rated cooling capacity for the speed using Gross Total Cooling Capacity
            // and Gross Total Cooling Capacity Fraction of the speed.
            MSRatedTotCap.push_back(speed.rated_total_capacity); // get the capcity at each speed bymultiplying this fraCTION WITH the gross.
            MSCCapAirFFlow.push_back(speed.indexCapFFF);
            MSRatedEvaporatorFanPowerPerVolumeFlowRate2023.push_back(speed.rated_evap_fan_power_per_volume_flow_rate_2023);
            // Calculate the rated evap air flow rate for the speed using Rated Evaporator Air flow Rate
            // and Rated Evaporator Air flow fraction of the speed
            MSRatedAirVolFlowRate.push_back(speed.evap_air_flow_rate);
            MSEIRFTemp.push_back(speed.indexEIRFT);
            MSRatedCOP.push_back(speed.ratedCOP);
            MSEIRAirFFlow.push_back(speed.indexEIRFFF);
        }

        // For Condenser Type
        DataHeatBalance::RefrigCondenserType _CondenserType;
        switch (operatingMode.condenserType) {
        case CoilCoolingDXCurveFitOperatingMode::CondenserType::EVAPCOOLED:
            _CondenserType = DataHeatBalance::RefrigCondenserType::Evap;
            break;
        case CoilCoolingDXCurveFitOperatingMode::CondenserType::AIRCOOLED:
        default:
            _CondenserType = DataHeatBalance::RefrigCondenserType::Air;
            break;
        }

        std::tie(IEER_2022, NetCoolingCapRated2022, EER_2022) = IEERCalculationVariableSpeed(state,
                                                                                             CoilType,
                                                                                             nsp,
                                                                                             MSCCapFTemp,
                                                                                             MSRatedTotCap,
                                                                                             MSCCapAirFFlow,
                                                                                             MSRatedEvaporatorFanPowerPerVolumeFlowRate2023,
                                                                                             MSRatedAirVolFlowRate,
                                                                                             MSEIRFTemp,
                                                                                             MSRatedCOP,
                                                                                             MSEIRAirFFlow,
                                                                                             _CondenserType);

        return std::make_tuple(IEER_2022, NetCoolingCapRated2022, EER_2022);
    }

    std::tuple<Real64, Real64, Real64> IEERCalculationVariableSpeed(
        EnergyPlusData &state,
        std::string const &VSCoilType, // Type of DX coil
        int const nsp,
        Array1A_int const &CapFTempCurveIndex,
        Array1A<Real64> const &RatedTotalCapacity,
        Array1A_int const &CapFFlowCurveIndex,
        Array1A<Real64> const &FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        Array1A<Real64> const &RatedAirVolFlowRate,
        Array1A_int const &EIRFTempCurveIndex,
        Array1A<Real64> const &RatedCOP, // Reference coefficient of performance [W/W]
        Array1A_int const &EIRFFlowCurveIndex,
        DataHeatBalance::RefrigCondenserType const _CondenserType) // Type of condenser user by the DX Cooling Coil
    {
        Real64 IEER_2022(0.0);
        Real64 EER_2022(0.0);
        Real64 QAFull(0.0);
        Array1D<Real64> Q_A_Full(nsp);                        // Total cooling capacity at A2 test condition (High speed) | q_A_Full
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        // Calculate these for each speed
        // hint : curve index will change, AirFlow rate will remain same
        Array1D<Real64> TotCapFlowModFac(nsp); // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);    // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Array1D<Real64> TotCapTempModFac(nsp);
        Array1D<Real64> NetCoolingCapRated(nsp);
        Real64 OutdoorUnitInletAirDryBulbTempReduced(0.0); // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]
        // EER Reduced
        Real64 A(0.0); // EER, (Btu/h)/W, at 100% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 B(0.0); // EER, (Btu/h)/W, at 75% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 C(0.0); // EER, (Btu/h)/W, at 50% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 D(0.0); // EER, (Btu/h)/W, at 25% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)

        Array1D<DataHeatBalance::RefrigCondenserType> CondenserType(nsp);

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate_2023(spnum) = 0.0;
            if (FanPowerPerEvapAirFlowRateFromInput_2023(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
            } else {
                FanPowerPerEvapAirFlowRate_2023(spnum) = FanPowerPerEvapAirFlowRateFromInput_2023(spnum);
            }
            CondenserType(spnum) = _CondenserType;
        }

        for (int spnum = nsp; spnum > 0; --spnum) {
            // TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            // EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            // if CondenserType is AirCooled
            if (CondenserType(spnum) == DataHeatBalance::RefrigCondenserType::Air) {
                // Cooling Coil | Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                if (VSCoilType.find("Cooling") != std::string::npos) {
                    TotCapTempModFac(spnum) =
                        Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoolingCoilInletAirWetBulbTempRated, CoilInletAirCoolDryBulbIEER);
                } else { // Heating Coil | Calculate the net cooling capacity at the rated conditions (6.11C(43F) Wet Bulb and 8.33C(47F) Dry Bulb )
                    TotCapTempModFac(spnum) =
                        Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoilHeatingInletAirWetBulbTempIEER, CoilHeatingInletAirCoolDryBulbIEER);
                }
            }
            // if CondenserType is WaterCooled
            if (CondenserType(spnum) == DataHeatBalance::RefrigCondenserType::Water) {
                // Calculate the net cooling capacity at the rated conditions (35.0C(95F) Outlet and 29.44C(85F) Inlet )

                TotCapTempModFac(spnum) = Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoilWaterOutletTempIEER, CoilWaterInletTempIEER);
            }
            // if CondesnerType is EvaporativelyCooled
            if (CondenserType(spnum) == DataHeatBalance::RefrigCondenserType::Evap) {
                // Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                TotCapTempModFac(spnum) =
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoilInletEvapWetBulbTempIEER, CoilInletEvapDryBulbTempIEER);
            }

            // This Will be our QAFull
            NetCoolingCapRated(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFac(spnum) * TotCapFlowModFac(spnum) -
                                        FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_A_Full(spnum) = NetCoolingCapRated(spnum);
        }

        QAFull = Q_A_Full(nsp);

        Real64 _100PercentCoolCap = RatedTotalCapacity(nsp);
        Real64 _75PercentCoolCap = RatedTotalCapacity(nsp) * 0.75;
        Real64 _50PercentCoolCap = RatedTotalCapacity(nsp) * 0.50;
        Real64 _25PercentCoolCap = RatedTotalCapacity(nsp) * 0.25;

        Array1D<int> speedsForA;
        Array1D<int> speedsForB;
        Array1D<int> speedsForC;
        Array1D<int> speedsForD;
        Array1D<Real64> ratioArray;

        if (nsp > 4) {
            int smallerThanSpeedB = 0;
            int largerThanSpeedB = 0;
            int smallerThanSpeedC = 0;
            int largerThanSpeedC = 0;
            int smallerThanSpeedD = 0;
            int largerThanSpeedD = 0;
            bool bFound = false;
            bool cFound = false;
            bool dFound = false;
            for (int i = 1; i <= nsp; ++i) {
                Real64 ratioAtithSpeed = (RatedTotalCapacity(i) / _100PercentCoolCap) * 100;
                ratioArray.push_back(ratioAtithSpeed);
            }
            for (int i = 1; i <= nsp; ++i) {
                if ((int)(ratioArray(i)) == 100.0) {
                    speedsForA.push_back(i);
                    continue;
                } else if ((int)(ratioArray(i)) == 75.0) {
                    speedsForB.push_back(i);
                    bFound = true;
                    smallerThanSpeedB = 0;
                    largerThanSpeedB = 0;
                    continue;
                } else if ((int)(ratioArray(i)) == 50.0) {
                    speedsForC.push_back(i);
                    cFound = true;
                    smallerThanSpeedC = 0;
                    largerThanSpeedC = 0;
                    continue;
                } else if ((int)(ratioArray(i)) == 25.0) {
                    speedsForD.push_back(i);
                    dFound = true;
                    smallerThanSpeedD = 0;
                    largerThanSpeedD = 0;
                    continue;
                } else {
                    if (((int)(ratioArray(i)) > 0.0 && (int)(ratioArray(i)) < 25.0) && !dFound) {
                        if (smallerThanSpeedD == 0) {
                            smallerThanSpeedD = i;
                        } else {
                            if (std::abs(RatedTotalCapacity(smallerThanSpeedD) - _25PercentCoolCap) >
                                std::abs(RatedTotalCapacity(i) - _25PercentCoolCap)) {
                                smallerThanSpeedD = i;
                            }
                        }
                    }
                    if (((int)(ratioArray(i)) > 25.0 && (int)(ratioArray(i)) < 50.0) && !dFound) {
                        if (largerThanSpeedD == 0) {
                            largerThanSpeedD = i;
                        } else {
                            if (std::abs(RatedTotalCapacity(largerThanSpeedD) - _25PercentCoolCap) >
                                std::abs(RatedTotalCapacity(i) - _25PercentCoolCap)) {
                                largerThanSpeedD = i;
                            }
                        }
                    }
                    if (((int)(ratioArray(i)) > 25.0 && (int)(ratioArray(i)) < 50.0) && !cFound) {
                        if (smallerThanSpeedC == 0) {
                            smallerThanSpeedC = i;
                        } else {
                            if (std::abs(RatedTotalCapacity(smallerThanSpeedC) - _50PercentCoolCap) >
                                std::abs(RatedTotalCapacity(i) - _50PercentCoolCap)) {
                                smallerThanSpeedC = i;
                            }
                        }
                    }
                    if (((int)(ratioArray(i)) > 50.0 && (int)(ratioArray(i)) < 75.0) && !cFound) {
                        if (largerThanSpeedC == 0) {
                            largerThanSpeedC = i;
                        } else {
                            if (std::abs(RatedTotalCapacity(largerThanSpeedC) - _50PercentCoolCap) >
                                std::abs(RatedTotalCapacity(i) - _50PercentCoolCap)) {
                                largerThanSpeedC = i;
                            }
                        }
                    }
                    if (((int)(ratioArray(i)) > 50.0 && (int)(ratioArray(i)) < 75.0) && !bFound) {
                        if (smallerThanSpeedB == 0) {
                            smallerThanSpeedB = i;
                        } else {
                            if (std::abs(RatedTotalCapacity(smallerThanSpeedB) - _75PercentCoolCap) >
                                std::abs(RatedTotalCapacity(i) - _75PercentCoolCap)) {
                                smallerThanSpeedB = i;
                            }
                        }
                    }
                    if (((int)(ratioArray(i)) > 75.0 && (int)(ratioArray(i)) < 100.0) && !bFound) {
                        if (largerThanSpeedB == 0) {
                            largerThanSpeedB = i;
                        } else {
                            if (std::abs(RatedTotalCapacity(largerThanSpeedB) - _75PercentCoolCap) >
                                std::abs(RatedTotalCapacity(i) - _75PercentCoolCap)) {
                                largerThanSpeedB = i;
                            }
                        }
                    }
                }
            }

            if (smallerThanSpeedB != 0) {
                speedsForB.push_back(smallerThanSpeedB);
            }
            if (largerThanSpeedB != 0) {
                speedsForB.push_back(largerThanSpeedB);
            }

            if (smallerThanSpeedC != 0) {
                speedsForC.push_back(smallerThanSpeedC);
            }
            if (largerThanSpeedC != 0) {
                speedsForC.push_back(largerThanSpeedC);
            }

            if (smallerThanSpeedD != 0) {
                speedsForD.push_back(smallerThanSpeedD);
            }
            if (largerThanSpeedD != 0) {
                speedsForD.push_back(largerThanSpeedD);
            }

            // int totalEERCount = speedsForA.size() + speedsForB.size() + speedsForC.size() + speedsForD.size();
            // For A | 100% - ReducedPLRIEER[3]
            int RedCapNum = speedsForA(1);
            OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(1.00, CondenserType(RedCapNum));

            A = CalculateInterMediateEER(state,
                                         QAFull,
                                         OutdoorUnitInletAirDryBulbTempReduced,
                                         CapFTempCurveIndex(RedCapNum),
                                         CoolingCoilInletAirWetBulbTempRated,
                                         RatedTotalCapacity(RedCapNum),
                                         TotCapFlowModFac(RedCapNum),
                                         FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                         RatedAirVolFlowRate(RedCapNum),
                                         EIRFTempCurveIndex(RedCapNum),
                                         RatedCOP(RedCapNum),
                                         EIRFlowModFac(RedCapNum),
                                         ReducedPLRIEER[3]);

            // For B | 75% - ReducedPLRIEER[2]
            Real64 EER_BLow(0.0);
            Real64 EER_BHigh(0.0);
            for (int i = 1; i <= 2; ++i) {
                if ((unsigned long)i > speedsForB.size()) {
                    continue;
                }

                RedCapNum = speedsForB(i);
                OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(0.75, CondenserType(RedCapNum));

                Real64 EER = CalculateInterMediateEER(state,
                                                      QAFull,
                                                      OutdoorUnitInletAirDryBulbTempReduced,
                                                      CapFTempCurveIndex(RedCapNum),
                                                      CoolingCoilInletAirWetBulbTempRated,
                                                      RatedTotalCapacity(RedCapNum),
                                                      TotCapFlowModFac(RedCapNum),
                                                      FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                                      RatedAirVolFlowRate(RedCapNum),
                                                      EIRFTempCurveIndex(RedCapNum),
                                                      RatedCOP(RedCapNum),
                                                      EIRFlowModFac(RedCapNum),
                                                      ReducedPLRIEER[2]);
                if (speedsForB.size() == 1) {
                    B = EER;
                } else {
                    if (i == 1) {
                        EER_BLow = EER; // ?? Check first is low or high
                    } else if (i == 2) {
                        EER_BHigh = EER;
                    }
                }
            }
            // Do the interpolation for B if required
            if (speedsForB.size() == 2) {
                B = ((EER_BHigh - EER_BLow) / (ratioArray(speedsForB(2)) - ratioArray(speedsForB(1)))) * (75 - ratioArray(speedsForB(1))) + EER_BLow;
            }

            // For C | 50% - ReducedPLRIEER[1]
            Real64 EER_CLow(0.0);
            Real64 EER_CHigh(0.0);
            for (int i = 1; i <= 2; ++i) {
                if ((unsigned long)i > speedsForC.size()) {
                    continue;
                }

                RedCapNum = speedsForC(i);
                OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(0.50, CondenserType(RedCapNum));

                Real64 EER = CalculateInterMediateEER(state,
                                                      QAFull,
                                                      OutdoorUnitInletAirDryBulbTempReduced,
                                                      CapFTempCurveIndex(RedCapNum),
                                                      CoolingCoilInletAirWetBulbTempRated,
                                                      RatedTotalCapacity(RedCapNum),
                                                      TotCapFlowModFac(RedCapNum),
                                                      FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                                      RatedAirVolFlowRate(RedCapNum),
                                                      EIRFTempCurveIndex(RedCapNum),
                                                      RatedCOP(RedCapNum),
                                                      EIRFlowModFac(RedCapNum),
                                                      ReducedPLRIEER[1]);
                if (speedsForC.size() == 1) {
                    C = EER;
                } else {
                    if (i == 1) {
                        EER_CLow = EER; // ?? Check first is low or high
                    } else if (i == 2) {
                        EER_CHigh = EER;
                    }
                }
            }
            // Do the interpolation for C if required
            if (speedsForC.size() == 2) {
                C = ((EER_CHigh - EER_CLow) / (ratioArray(speedsForC(2)) - ratioArray(speedsForC(1)))) * (50 - ratioArray(speedsForC(1))) + EER_CLow;
            }

            // For D | 25% - ReducedPLRIEER[0]
            Real64 EER_DLow(0.0);
            Real64 EER_DHigh(0.0);
            for (int i = 1; i <= 2; ++i) {
                if ((unsigned long)i > speedsForD.size()) {
                    continue;
                }

                RedCapNum = speedsForD(i);
                OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(0.25, CondenserType(RedCapNum));

                Real64 EER = CalculateInterMediateEER(state,
                                                      QAFull,
                                                      OutdoorUnitInletAirDryBulbTempReduced,
                                                      CapFTempCurveIndex(RedCapNum),
                                                      CoolingCoilInletAirWetBulbTempRated,
                                                      RatedTotalCapacity(RedCapNum),
                                                      TotCapFlowModFac(RedCapNum),
                                                      FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                                      RatedAirVolFlowRate(RedCapNum),
                                                      EIRFTempCurveIndex(RedCapNum),
                                                      RatedCOP(RedCapNum),
                                                      EIRFlowModFac(RedCapNum),
                                                      ReducedPLRIEER[0]);
                if (speedsForD.size() == 1) {
                    D = EER;
                } else {
                    if (i == 1) {
                        EER_DLow = EER; // ?? Check first is low or high
                    } else if (i == 2) {
                        EER_DHigh = EER;
                    }
                }
            }
            // Do the interpolation for D if required
            if (speedsForD.size() == 2) {
                D = ((EER_DHigh - EER_DLow) / (ratioArray(speedsForD(2)) - ratioArray(speedsForD(1)))) * (25 - ratioArray(speedsForD(1))) + EER_DLow;
            }

            IEER_2022 = GetIEEREquationResult(A, B, C, D);
            EER_2022 = A;
        } else if (nsp == 2 || nsp == 3 || nsp == 4) {
            // 2, 3 & 4 Speeds
            Real64 QAFull_(0.0);
            std::tie(IEER_2022, QAFull_, EER_2022) = IEERCalculationMultiSpeed(state,
                                                                               VSCoilType,
                                                                               nsp,
                                                                               CapFTempCurveIndex,
                                                                               RatedTotalCapacity,
                                                                               CapFFlowCurveIndex,
                                                                               FanPowerPerEvapAirFlowRate_2023,
                                                                               RatedAirVolFlowRate,
                                                                               EIRFTempCurveIndex,
                                                                               RatedCOP,
                                                                               EIRFFlowCurveIndex,
                                                                               CondenserType);
        } else if (nsp == 1) {
            // 1 Speed
            Real64 QAFull_(0.0);
            std::tie(IEER_2022, QAFull_, EER_2022) = IEERCalculationSingleSpeed(state,
                                                                                VSCoilType,
                                                                                CapFTempCurveIndex(1),
                                                                                RatedTotalCapacity(1),
                                                                                TotCapFlowModFac(1),
                                                                                FanPowerPerEvapAirFlowRate_2023(1),
                                                                                RatedAirVolFlowRate(1),
                                                                                EIRFTempCurveIndex(1),
                                                                                RatedCOP(1),
                                                                                EIRFlowModFac(1),
                                                                                CondenserType(1));
        }

        return std::make_tuple(IEER_2022, QAFull, EER_2022);
    }

    std::tuple<Real64, Real64, Real64> IEERCalculationMultiSpeed(
        EnergyPlus::EnergyPlusData &state,
        std::string const &DXCoilType, // Type of DX coil
        int const nsp,
        Array1A_int const &CapFTempCurveIndex,
        Array1A<Real64> const &RatedTotalCapacity,
        Array1A_int const &CapFFlowCurveIndex,
        Array1A<Real64> const &FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        Array1A<Real64> const &RatedAirVolFlowRate,
        Array1A_int const &EIRFTempCurveIndex,
        Array1A<Real64> const &RatedCOP, // Reference coefficient of performance [W/W]
        Array1A_int const &EIRFFlowCurveIndex,
        Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType) // Type of condenser user by the DX Cooling Coil
    {
        Real64 IEER_2022(0.0);
        Real64 EER_2022(0.0);
        Real64 QAFull(0.0);
        Array1D<Real64> Q_A_Full(nsp);                        // Total cooling capacity at A2 test condition (High speed) | q_A_Full
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        // Calculate these for each speed
        // hint : curve index will change, AirFlow rate will remain same
        Array1D<Real64> TotCapFlowModFac(nsp); // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);    // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Array1D<Real64> TotCapTempModFac(nsp);
        Array1D<Real64> NetCoolingCapRated(nsp);
        Real64 OutdoorUnitInletAirDryBulbTempReduced(0.0); // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]
        Real64 EER(0.0);                                   // Energy Efficiency Rating
        Real64 PartLoad(0.0);
        // EER Reduced
        Real64 A(0.0); // EER, (Btu/h)/W, at 100% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 B(0.0); // EER, (Btu/h)/W, at 75% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 C(0.0); // EER, (Btu/h)/W, at 50% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 D(0.0); // EER, (Btu/h)/W, at 25% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate_2023(spnum) = 0.0;
            if (FanPowerPerEvapAirFlowRateFromInput_2023(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
            } else {
                FanPowerPerEvapAirFlowRate_2023(spnum) = FanPowerPerEvapAirFlowRateFromInput_2023(spnum);
            }
        }

        // int maxSpeed = nsp;
        for (int spnum = nsp; spnum > 0; --spnum) {
            // TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            // EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            // if CondenserType is AirCooled
            if (CondenserType(spnum) == DataHeatBalance::RefrigCondenserType::Air) {
                // Cooling Coil | Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                if (DXCoilType.find("Cooling") != std::string::npos) {
                    TotCapTempModFac(spnum) =
                        Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoolingCoilInletAirWetBulbTempRated, CoilInletAirCoolDryBulbIEER);
                } else { // Heating Coil | Calculate the net cooling capacity at the rated conditions (6.11C(43F) Wet Bulb and 8.33C(47F) Dry Bulb )
                    TotCapTempModFac(spnum) =
                        Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoilHeatingInletAirWetBulbTempIEER, CoilHeatingInletAirCoolDryBulbIEER);
                }
            }
            // if CondenserType is WaterCooled
            if (CondenserType(spnum) == DataHeatBalance::RefrigCondenserType::Water) {
                // Calculate the net cooling capacity at the rated conditions (35.0C(95F) Outlet and 29.44C(85F) Inlet )

                TotCapTempModFac(spnum) = Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoilWaterOutletTempIEER, CoilWaterInletTempIEER);
            }
            // if CondesnerType is EvaporativelyCooled
            if (CondenserType(spnum) == DataHeatBalance::RefrigCondenserType::Evap) {
                // Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                TotCapTempModFac(spnum) =
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), CoilInletEvapWetBulbTempIEER, CoilInletEvapDryBulbTempIEER);
            }

            // This Will be our QAFull
            NetCoolingCapRated(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFac(spnum) * TotCapFlowModFac(spnum) -
                                        FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_A_Full(spnum) = NetCoolingCapRated(spnum);
        }
        QAFull = Q_A_Full(nsp);

        Real64 _100PercentCoolCap = RatedTotalCapacity(nsp);

        Array1D<int> speedsForA;
        Array1D<int> speedsForB;
        Array1D<int> speedsForC;
        Array1D<int> speedsForD;
        Array1D<Real64> ratioArray;

        if (nsp == 4) {

            // RedCapNum : Integer counter for reduced capacity
            for (int RedCapNum = nsp; RedCapNum > 0; --RedCapNum) {

                PartLoad = ReducedPLRIEER[RedCapNum - 1.0];
                OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(PartLoad, CondenserType(RedCapNum));

                EER = CalculateInterMediateEER(state,
                                               QAFull,
                                               OutdoorUnitInletAirDryBulbTempReduced,
                                               CapFTempCurveIndex(RedCapNum),
                                               CoolingCoilInletAirWetBulbTempRated,
                                               RatedTotalCapacity(RedCapNum),
                                               TotCapFlowModFac(RedCapNum),
                                               FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                               RatedAirVolFlowRate(RedCapNum),
                                               EIRFTempCurveIndex(RedCapNum),
                                               RatedCOP(RedCapNum),
                                               EIRFlowModFac(RedCapNum),
                                               PartLoad);

                if (PartLoad == 1.00) {
                    A = EER;
                } else if (PartLoad == 0.75) {
                    B = EER;
                } else if (PartLoad == 0.50) {
                    C = EER;
                } else if (PartLoad == 0.25) {
                    D = EER;
                }
            }
            IEER_2022 = GetIEEREquationResult(A, B, C, D);
            EER_2022 = A;
        } else if (nsp == 3) {
            speedsForA.push_back(3);

            speedsForB.push_back(2);
            speedsForB.push_back(3);

            speedsForC.push_back(1);
            speedsForC.push_back(2);

            speedsForD.push_back(1);

            for (int i = 1; i <= nsp; ++i) {
                Real64 ratioAtithSpeed = (RatedTotalCapacity(i) / _100PercentCoolCap) * 100;
                ratioArray.push_back(ratioAtithSpeed);
            }

            // For A | 100% - ReducedPLRIEER[3]
            int RedCapNum = speedsForA(1);

            OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(1.00, CondenserType(RedCapNum));

            A = CalculateInterMediateEER(state,
                                         QAFull,
                                         OutdoorUnitInletAirDryBulbTempReduced,
                                         CapFTempCurveIndex(RedCapNum),
                                         CoolingCoilInletAirWetBulbTempRated,
                                         RatedTotalCapacity(RedCapNum),
                                         TotCapFlowModFac(RedCapNum),
                                         FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                         RatedAirVolFlowRate(RedCapNum),
                                         EIRFTempCurveIndex(RedCapNum),
                                         RatedCOP(RedCapNum),
                                         EIRFlowModFac(RedCapNum),
                                         ReducedPLRIEER[3]);

            // For B | 75% - ReducedPLRIEER[2]
            Real64 EER_BLow(0.0);
            Real64 EER_BHigh(0.0);
            for (int i = 1; i <= 2; ++i) {
                if ((unsigned long)i > speedsForB.size()) {
                    continue;
                }

                RedCapNum = speedsForB(i);
                OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(0.75, CondenserType(RedCapNum));

                EER = CalculateInterMediateEER(state,
                                               QAFull,
                                               OutdoorUnitInletAirDryBulbTempReduced,
                                               CapFTempCurveIndex(RedCapNum),
                                               CoolingCoilInletAirWetBulbTempRated,
                                               RatedTotalCapacity(RedCapNum),
                                               TotCapFlowModFac(RedCapNum),
                                               FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                               RatedAirVolFlowRate(RedCapNum),
                                               EIRFTempCurveIndex(RedCapNum),
                                               RatedCOP(RedCapNum),
                                               EIRFlowModFac(RedCapNum),
                                               ReducedPLRIEER[2]);

                if (i == 1) {
                    EER_BLow = EER; // ?? Check first is low or high
                } else if (i == 2) {
                    EER_BHigh = EER;
                }
            }
            // Do the interpolation for B if required
            if (speedsForB.size() == 2) {
                B = ((EER_BHigh - EER_BLow) / (ratioArray(speedsForB(2)) - ratioArray(speedsForB(1)))) * (75 - ratioArray(speedsForB(1))) + EER_BLow;
            }

            // For C | 50% - ReducedPLRIEER[1]
            Real64 EER_CLow(0.0);
            Real64 EER_CHigh(0.0);
            for (int i = 1; i <= 2; ++i) {
                if ((unsigned long)i > speedsForC.size()) {
                    continue;
                }

                RedCapNum = speedsForC(i);
                OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(0.50, CondenserType(RedCapNum));

                EER = CalculateInterMediateEER(state,
                                               QAFull,
                                               OutdoorUnitInletAirDryBulbTempReduced,
                                               CapFTempCurveIndex(RedCapNum),
                                               CoolingCoilInletAirWetBulbTempRated,
                                               RatedTotalCapacity(RedCapNum),
                                               TotCapFlowModFac(RedCapNum),
                                               FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                               RatedAirVolFlowRate(RedCapNum),
                                               EIRFTempCurveIndex(RedCapNum),
                                               RatedCOP(RedCapNum),
                                               EIRFlowModFac(RedCapNum),
                                               ReducedPLRIEER[1]);

                if (i == 1) {
                    EER_CLow = EER; // ?? Check first is low or high
                } else if (i == 2) {
                    EER_CHigh = EER;
                }
            }
            // Do the interpolation for C if required
            if (speedsForC.size() == 2) {
                C = ((EER_CHigh - EER_CLow) / (ratioArray(speedsForC(2)) - ratioArray(speedsForC(1)))) * (50 - ratioArray(speedsForC(1))) + EER_CLow;
            }

            // For D | 25% - ReducedPLRIEER[0]

            RedCapNum = speedsForD(1);
            OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(0.25, CondenserType(RedCapNum));

            D = CalculateInterMediateEER(state,
                                         QAFull,
                                         OutdoorUnitInletAirDryBulbTempReduced,
                                         CapFTempCurveIndex(RedCapNum),
                                         CoolingCoilInletAirWetBulbTempRated,
                                         RatedTotalCapacity(RedCapNum),
                                         TotCapFlowModFac(RedCapNum),
                                         FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                         RatedAirVolFlowRate(RedCapNum),
                                         EIRFTempCurveIndex(RedCapNum),
                                         RatedCOP(RedCapNum),
                                         EIRFlowModFac(RedCapNum),
                                         ReducedPLRIEER[0]);

            IEER_2022 = GetIEEREquationResult(A, B, C, D);
            EER_2022 = A;
        } else if (nsp == 2) {
            // Having 2 Speeds
            Real64 QAFull_(0.0);
            // Reversing the input arrays because IEERCalculationTwoSpeed is expecting High Speed Data before the Low Speed.
            Array1D<Real64> FanPowerPerEvapAirFlowRateHighAndLow(FanPowerPerEvapAirFlowRate_2023.size()); // Ensure ReversedArray has the same size
            std::reverse_copy(
                FanPowerPerEvapAirFlowRate_2023.begin(), FanPowerPerEvapAirFlowRate_2023.end(), FanPowerPerEvapAirFlowRateHighAndLow.begin());

            Array1D<Real64> RatedAirVolFlowRateHighAndLow(RatedAirVolFlowRate.size());
            std::reverse_copy(RatedAirVolFlowRate.begin(), RatedAirVolFlowRate.end(), RatedAirVolFlowRateHighAndLow.begin());

            Array1D<Real64> RatedCOPHighAndLow(RatedCOP.size());
            std::reverse_copy(RatedCOP.begin(), RatedCOP.end(), RatedCOPHighAndLow.begin());

            Array1D<Real64> RatedTotalCapacityHighAndLow(RatedTotalCapacity.size());
            std::reverse_copy(RatedTotalCapacity.begin(), RatedTotalCapacity.end(), RatedTotalCapacityHighAndLow.begin());

            std::tie(IEER_2022, QAFull_, EER_2022) = IEERCalculationTwoSpeed(state,
                                                                             DXCoilType,
                                                                             CondenserType,
                                                                             CapFTempCurveIndex,
                                                                             RatedTotalCapacityHighAndLow,
                                                                             CapFFlowCurveIndex, // Only for HIGH SPEED
                                                                             FanPowerPerEvapAirFlowRateHighAndLow,
                                                                             RatedAirVolFlowRateHighAndLow,
                                                                             EIRFTempCurveIndex,
                                                                             RatedCOPHighAndLow,
                                                                             EIRFFlowCurveIndex // Only for HIGH SPEED
            );
        } else if (nsp == 1) {
            // NA : The minimum number of speeds for cooling is 2 and the maximum number is 4 for Coil:Cooling:DX:MultiSpeed
        }
        return std::make_tuple(IEER_2022, QAFull, EER_2022);
    }

    std::tuple<Real64, Real64, Real64> IEERCalculationTwoSpeed(
        EnergyPlusData &state,
        std::string const &DXCoilType, // Type of DX coil
        Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType,
        Array1A_int const &CapFTempCurveIndex,
        Array1A<Real64> const &RatedTotalCapacity,
        Array1A_int const &CCapFFlowCurveIndex,                          //  | Only for HIGH SPEED
        Array1A<Real64> const &FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 Rated Fan Power per air volume flow rate through the evaporator coil
        Array1A<Real64> const &RatedAirVolFlowRate,
        Array1A_int const &EIRFTempCurveIndex,
        Array1A<Real64> const &RatedCOP,      // Reference coefficient of performance [W/W]
        Array1A_int const &EIRFFlowCurveIndex //  | Only for HIGH SPEED
    )
    {
        int constexpr nsp = 4; // As IEER Requires EER for at least 4 different Speeds,
        // we'll be carrying out the calculations with low and high speed in such a way that we have
        // A,B,C,D for the required IEER equation. So nsp is initialized as 4 here.
        Real64 IEER_2022(0.0);
        Real64 EER_2022(0.0);
        Real64 QAFull(0.0);
        Array1D<Real64> Q_A_Full(nsp);                        // Total cooling capacity at A2 test condition (High speed) | q_A_Full
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        // Calculate these for each speed
        // hint : curve index will change, AirFlow rate will remain same
        Array1D<Real64> TotCapFlowModFac(nsp); // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);    // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Array1D<Real64> TotCapTempModFac(nsp);
        Array1D<Real64> NetCoolingCapRated(nsp);
        Real64 OutdoorUnitInletAirDryBulbTempReduced(0.0); // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]
        Real64 EER(0.0);                                   // Energy Efficiency Rating
        Real64 PartLoad(0.0);
        // EER Reduced
        Real64 A(0.0); // EER, (Btu/h)/W, at 100% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 B(0.0); // EER, (Btu/h)/W, at 75% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 C(0.0); // EER, (Btu/h)/W, at 50% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 D(0.0); // EER, (Btu/h)/W, at 25% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Array1D<DataHeatBalance::RefrigCondenserType> _condenserType(nsp);

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate_2023(spnum) = 0.0;
            if (spnum == 1 || spnum == 2) { // First two speeds will have Low Speed Props
                if (FanPowerPerEvapAirFlowRateFromInput_2023(2) <= 0.0) {
                    FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
                } else {
                    FanPowerPerEvapAirFlowRate_2023(spnum) = FanPowerPerEvapAirFlowRateFromInput_2023(2);
                }
                _condenserType(spnum) = CondenserType(1);
            } else if (spnum == 3 || spnum == 4) {
                if (FanPowerPerEvapAirFlowRateFromInput_2023(1) <= 0.0) { // Last two speeds will have High Speed Props
                    FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
                } else {
                    FanPowerPerEvapAirFlowRate_2023(spnum) = FanPowerPerEvapAirFlowRateFromInput_2023(1);
                }
                _condenserType(spnum) = CondenserType(2);
            }
        }

        // Calculate QAFull, EIRFlowModeFac & TotCapFlowModFac
        for (int spnum = nsp; spnum > 0; --spnum) {

            if (spnum == 3 || spnum == 4) {
                if (DXCoilType == "Coil:Cooling:DX:TwoSpeed") {
                    TotCapFlowModFac(spnum) = Curve::CurveValue(state, CCapFFlowCurveIndex(1), AirMassFlowRatioRated);
                    EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(1), AirMassFlowRatioRated);
                } else {
                    // Coil:Cooling:DX:MultiSpeed || Coil:Cooling:DX:VariableSpeed
                    TotCapFlowModFac(spnum) = Curve::CurveValue(state, CCapFFlowCurveIndex(2), AirMassFlowRatioRated);
                    EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(2), AirMassFlowRatioRated);
                }
            } else if (spnum == 1 || spnum == 2) {
                if (DXCoilType == "Coil:Cooling:DX:TwoSpeed") {
                    TotCapFlowModFac(spnum) = 1; // As per IO Reference there are no CCapFFlowCurve for Low Speed | Section ??
                    EIRFlowModFac(spnum) = 1;    // As per IO Reference there are no EIRFFlowCurve for Low Speed | Section ??
                } else {
                    // Coil:Cooling:DX:MultiSpeed || Coil:Cooling:DX:VariableSpeed
                    TotCapFlowModFac(spnum) = Curve::CurveValue(state, CCapFFlowCurveIndex(1), AirMassFlowRatioRated);
                    EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(1), AirMassFlowRatioRated);
                }
            }

            if (_condenserType(spnum) == DataHeatBalance::RefrigCondenserType::Air) { // Case: CondenserType is AirCooled
                if (spnum == 3 || spnum == 4) {
                    // Cooling Coil | Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                    if (DXCoilType.find("Cooling") != std::string::npos) {
                        TotCapTempModFac(spnum) =
                            Curve::CurveValue(state, CapFTempCurveIndex(1), CoolingCoilInletAirWetBulbTempRated, CoilInletAirCoolDryBulbIEER);
                    } else { // Heating Coil | Calculate the net cooling capacity at the rated conditions (6.11C(43F) Wet Bulb and 8.33C(47F) Dry Bulb
                             // )
                        TotCapTempModFac(spnum) =
                            Curve::CurveValue(state, CapFTempCurveIndex(1), CoilHeatingInletAirWetBulbTempIEER, CoilHeatingInletAirCoolDryBulbIEER);
                    }
                } else if (spnum == 1 || spnum == 2) {
                    // Cooling Coil | Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                    if (DXCoilType.find("Cooling") != std::string::npos) {
                        TotCapTempModFac(spnum) =
                            Curve::CurveValue(state, CapFTempCurveIndex(2), CoolingCoilInletAirWetBulbTempRated, CoilInletAirCoolDryBulbIEER);
                    } else { // Heating Coil | Calculate the net cooling capacity at the rated conditions (6.11C(43F) Wet Bulb and 8.33C(47F) Dry Bulb
                             // )
                        TotCapTempModFac(spnum) =
                            Curve::CurveValue(state, CapFTempCurveIndex(2), CoilHeatingInletAirWetBulbTempIEER, CoilHeatingInletAirCoolDryBulbIEER);
                    }
                }
            } else if (_condenserType(spnum) == DataHeatBalance::RefrigCondenserType::Water) { // Case: CondenserType is WaterCooled
                // Calculate the net cooling capacity at the rated conditions (35.0C(95F) Outlet and 29.44C(85F) Inlet )
                if (spnum == 3 || spnum == 4) {
                    TotCapTempModFac(spnum) = Curve::CurveValue(state, CapFTempCurveIndex(1), CoilWaterOutletTempIEER, CoilWaterInletTempIEER);
                } else if (spnum == 1 || spnum == 2) {
                    TotCapTempModFac(spnum) = Curve::CurveValue(state, CapFTempCurveIndex(2), CoilWaterOutletTempIEER, CoilWaterInletTempIEER);
                }
            } else if (_condenserType(spnum) == DataHeatBalance::RefrigCondenserType::Evap) { // Case: CondesnerType is EvaporativelyCooled
                // Calculate the net cooling capacity at the rated conditions (23.89C(75F) Wet Bulb and 35.0C(95F) Dry Bulb )
                if (spnum == 3 || spnum == 4) {
                    TotCapTempModFac(spnum) =
                        Curve::CurveValue(state, CapFTempCurveIndex(1), CoilInletEvapWetBulbTempIEER, CoilInletEvapDryBulbTempIEER);
                } else if (spnum == 1 || spnum == 2) {
                    TotCapTempModFac(spnum) =
                        Curve::CurveValue(state, CapFTempCurveIndex(2), CoilInletEvapWetBulbTempIEER, CoilInletEvapDryBulbTempIEER);
                }
            }
            // This Will be our QAFull
            if (spnum == 3 || spnum == 4) {
                NetCoolingCapRated(spnum) = RatedTotalCapacity(1) * TotCapTempModFac(spnum) * TotCapFlowModFac(spnum) -
                                            FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(1);
            } else if (spnum == 1 || spnum == 2) {
                NetCoolingCapRated(spnum) = RatedTotalCapacity(2) * TotCapTempModFac(spnum) * TotCapFlowModFac(spnum) -
                                            FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(2);
            }
            Q_A_Full(spnum) = NetCoolingCapRated(spnum);
        }
        QAFull = Q_A_Full(nsp);

        // RedCapNum : Integer counter for reduced capacity
        for (int RedCapNum = nsp; RedCapNum > 0; --RedCapNum) {

            PartLoad = ReducedPLRIEER[(int)RedCapNum - 1.0];
            OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(PartLoad, _condenserType(RedCapNum));

            if (RedCapNum == 3 || RedCapNum == 4) {

                EER = CalculateInterMediateEER(state,
                                               QAFull,
                                               OutdoorUnitInletAirDryBulbTempReduced,
                                               CapFTempCurveIndex(1),
                                               CoolingCoilInletAirWetBulbTempRated,
                                               RatedTotalCapacity(1),
                                               TotCapFlowModFac(RedCapNum),
                                               FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                               RatedAirVolFlowRate(1),
                                               EIRFTempCurveIndex(1),
                                               RatedCOP(1),
                                               EIRFlowModFac(RedCapNum),
                                               PartLoad);

            } else if (RedCapNum == 1 || RedCapNum == 2) {

                EER = CalculateInterMediateEER(state,
                                               QAFull,
                                               OutdoorUnitInletAirDryBulbTempReduced,
                                               CapFTempCurveIndex(2),
                                               CoolingCoilInletAirWetBulbTempRated,
                                               RatedTotalCapacity(2),
                                               TotCapFlowModFac(RedCapNum),
                                               FanPowerPerEvapAirFlowRate_2023(RedCapNum),
                                               RatedAirVolFlowRate(2),
                                               EIRFTempCurveIndex(2),
                                               RatedCOP(2),
                                               EIRFlowModFac(RedCapNum),
                                               PartLoad);
            }

            if (PartLoad == 1.00) {
                A = EER;
            } else if (PartLoad == 0.75) {
                B = EER;
            } else if (PartLoad == 0.50) {
                C = EER;
            } else if (PartLoad == 0.25) {
                D = EER;
            }
        }

        IEER_2022 = GetIEEREquationResult(A, B, C, D);
        EER_2022 = A;
        return std::make_tuple(IEER_2022, QAFull, EER_2022);
    }

    std::tuple<Real64, Real64, Real64> IEERCalculationSingleSpeed(EnergyPlus::EnergyPlusData &state,
                                                                  std::string const &DXCoilType, // Type of DX coil for which HSPF is calculated
                                                                  const int CapFTempCurveIndex,
                                                                  const Real64 RatedTotalCapacity,
                                                                  const Real64 TotCapFlowModFac,
                                                                  const Real64 FanPowerPerEvapAirFlowRate,
                                                                  const Real64 RatedAirVolFlowRate,
                                                                  const int EIRFTempCurveIndex,
                                                                  const Real64 RatedCOP,
                                                                  const Real64 EIRFlowModFac,
                                                                  DataHeatBalance::RefrigCondenserType const CondenserType)
    {
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int constexpr NumOfReducedCap(4);                  // Number of reduced capacity test conditions (100%,75%,50%,and 25%)
        Real64 TotCapTempModFac(0.0);                      // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 OutdoorUnitInletAirDryBulbTempReduced(0.0); // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]
        Real64 EERReduced(0.0);                            // EER at reduced capacity test conditions (100%, 75%, 50%, and 25%)
        Real64 IEER = 0.0;                                 // Integareted energy efficiency ratio of single speed DX cooling coil
        Real64 NetCoolingCapRated = 0.0;                   // net cooling capacity of single speed DX cooling coil
        Real64 EER2 = 0.0;                                 // EER at high speed
        Real64 CoilInletAirWetBulbTemp = 19.44;            // 67F
        // EER Reduced
        Real64 A(0.0);      // EER, (Btu/h)/W, at 100% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 B(0.0);      // EER, (Btu/h)/W, at 75% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 C(0.0);      // EER, (Btu/h)/W, at 50% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 D(0.0);      // EER, (Btu/h)/W, at 25% Capacity at AHRI Standard Rating Conditions (see Table 6)| AHRI Std.340/360-2022(IP)
        Real64 QAFull(0.0); // Full Load Net Capacity (Btu/h) | Previously NetCoolingCapRated

        // if CondenserType is AirCooled
        if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
            // Calculate the net cooling capacity at the rated conditions (19.44C(67F) Wet Bulb and 35.0C(95F) Dry Bulb )
            if (DXCoilType.find("Cooling") != std::string::npos) {
                TotCapTempModFac = Curve::CurveValue(state, CapFTempCurveIndex, CoilInletAirWetBulbTemp, CoilInletAirCoolDryBulbIEER);
            } else {
                TotCapTempModFac =
                    Curve::CurveValue(state, CapFTempCurveIndex, CoilHeatingInletAirWetBulbTempIEER, CoilHeatingInletAirCoolDryBulbIEER);
            }
        }
        // if CondenserType is WaterCooled
        if (CondenserType == DataHeatBalance::RefrigCondenserType::Water) {
            // Calculate the net cooling capacity at the rated conditions (19.44C(67F) Wet Bulb and 29.44C(85F) Dry Bulb )
            TotCapTempModFac = Curve::CurveValue(state, CapFTempCurveIndex, CoilInletAirWetBulbTemp, CoilWaterInletTempIEER);
        }
        // if CondesnerType is EvaporativelyCooled
        if (CondenserType == DataHeatBalance::RefrigCondenserType::Evap) {
            // Calculate the net cooling capacity at the rated conditions (19.44C(67F) Wet Bulb and 35.0C(95F) Dry Bulb )
            TotCapTempModFac = Curve::CurveValue(state, CapFTempCurveIndex, CoilInletAirWetBulbTemp, CoilInletEvapDryBulbTempIEER);
        }
        // This Will be our QAFull
        NetCoolingCapRated = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        QAFull = NetCoolingCapRated;

        // RedCapNum : Integer counter for reduced capacity
        for (int RedCapNum = 0; RedCapNum < NumOfReducedCap; ++RedCapNum) {
            // Get the outdoor air dry bulb temperature for the reduced capacity test conditions
            OutdoorUnitInletAirDryBulbTempReduced = GetOutdoorUnitInletAirDryBulbTempReduced(ReducedPLRIEER[RedCapNum], CondenserType);

            EERReduced = CalculateInterMediateEER(state,
                                                  QAFull,
                                                  OutdoorUnitInletAirDryBulbTempReduced,
                                                  CapFTempCurveIndex,
                                                  CoolingCoilInletAirWetBulbTempRated,
                                                  RatedTotalCapacity,
                                                  TotCapFlowModFac,
                                                  FanPowerPerEvapAirFlowRate,
                                                  RatedAirVolFlowRate,
                                                  EIRFTempCurveIndex,
                                                  RatedCOP,
                                                  EIRFlowModFac,
                                                  ReducedPLRIEER[RedCapNum]);

            if (ReducedPLRIEER[RedCapNum] == 1.00) {
                A = EERReduced;
            } else if (ReducedPLRIEER[RedCapNum] == 0.75) {
                B = EERReduced;
            } else if (ReducedPLRIEER[RedCapNum] == 0.50) {
                C = EERReduced;
            } else if (ReducedPLRIEER[RedCapNum] == 0.25) {
                D = EERReduced;
            }
        }
        IEER = GetIEEREquationResult(A, B, C, D);
        EER2 = A;
        return std::make_tuple(IEER, QAFull, EER2);
    }

    Real64 EERSingleSpeedCooling(EnergyPlus::EnergyPlusData &state,
                                 const int CapFTempCurveIndex,
                                 const Real64 RatedTotalCapacity,
                                 const Real64 TotCapFlowModFac,
                                 const Real64 FanPowerPerEvapAirFlowRate,
                                 const Real64 RatedAirVolFlowRate,
                                 const int EIRFTempCurveIndex,
                                 const Real64 RatedCOP,
                                 const Real64 EIRFlowModFac)
    {
        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 EER = 0.0;                // energy efficiency ratio of single speed DX cooling coil
        Real64 TotCapTempModFac(0.0);    // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 NetCoolingCapRated(0.0);  // net cooling capacity of single speed DX cooling coil
        Real64 EIRTempModFac(0.0);       // EIR modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 EIR(0.0);                 // Energy Efficiency Ratio at AHRI test conditions for SEER [-]
        Real64 TotalElecPowerRated(0.0); // Net power consumption (Cond Fan+Compressor+Indoor Fan) at Rated test conditions [W]
        // Calculate the net cooling capacity at the rated conditions (19.44C WB and 35.0C DB )
        TotCapTempModFac = Curve::CurveValue(state, CapFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        NetCoolingCapRated = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        // Calculate Energy Efficiency Ratio (EER) at (19.44C WB and 35.0C DB ), ANSI/AHRI Std. 340/360
        EIRTempModFac = Curve::CurveValue(state, EIRFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        if (RatedCOP > 0.0) {
            // RatedCOP <= 0.0 is trapped in GetInput, but keep this as "safety"
            EIR = EIRTempModFac * EIRFlowModFac / RatedCOP;
        } else {
            EIR = 0.0;
        }
        TotalElecPowerRated = EIR * (RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac) + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        if (TotalElecPowerRated > 0.0) {
            EER = NetCoolingCapRated / TotalElecPowerRated;
        } else {
            EER = 0.0;
        }
        return EER;
    }

    std::tuple<Real64, Real64> SEERSingleStageCalculation(EnergyPlusData &state,
                                                          int CapFTempCurveIndex,
                                                          Real64 RatedTotalCapacity,
                                                          Real64 TotCapFlowModFac,
                                                          int EIRFTempCurveIndex,
                                                          Real64 EIRFlowModFac,
                                                          [[maybe_unused]] int EIRFFlowCurveIndex,
                                                          Real64 RatedCOP,
                                                          Real64 FanPowerPerEvapAirFlowRate,
                                                          Real64 RatedAirVolFlowRate,
                                                          int PLFFPLRCurveIndex,
                                                          Real64 const CyclicDegradationCoefficient)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC
        //       DATE WRITTEN   December 2012
        //       MODIFIED
        //       RE-ENGINEERED  Brijendra Singh, D-Alchemy

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the SEER values for single speed based on AHRI 210/230 2017 & 2023

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // AHRI Standard 340/360 (2022)

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // SEER calculations:
        Real64 TotCapTempModFac(0.0);  // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 TotCoolingCapAHRI(0.0); // Total Cooling Coil capacity (gross) at AHRI test conditions [W]
        Real64 EIRTempModFac(0.0);     // EIR modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 NetCoolingCapAHRI(0.0); // Net Cooling Coil capacity at AHRI TestB conditions, accounting for fan heat [W]
        Real64 EIR(0.0);               // Energy Efficiency Ratio at AHRI test conditions for SEER [-]
        Real64 TotalElecPower(0.0);    // Net power consumption (Cond Fan+Compressor+Indoor Fan) at AHRI test conditions [W]
        Real64 PartLoadFactorUser(
            0.0); // Part load factor based on user-input PLF curve and C_D value that accounts for thermal lag at compressor startup [-]
        Real64 PartLoadFactorStandard(
            0.0); // part-load factor that accounts for the cyclic degradation from AHRI Standard 210/240-2008 default PLF curve and C_D value, [-]
        Real64 SEER_User = 0.0;     // seasonal energy efficiency ratio of single speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 SEER_Standard = 0.0; // seasonal energy efficiency ratio of single speed DX cooling coil, from user-input PLF curve and C_D value

        TotCapTempModFac = Curve::CurveValue(state, CapFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        TotCoolingCapAHRI = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac;
        EIRTempModFac = Curve::CurveValue(state, EIRFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        EIR = (RatedCOP > 0.0) ? EIRTempModFac * EIRFlowModFac / RatedCOP : 0.0;

        // Calculate net cooling capacity
        NetCoolingCapAHRI = TotCoolingCapAHRI - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        TotalElecPower = EIR * TotCoolingCapAHRI + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
        // Calculate SEER value from the Energy Efficiency Ratio (EER) at the AHRI test conditions and the part load factor.
        // First evaluate the Part Load Factor curve at PLR = 0.5 (AHRI Standard 210/240)
        PartLoadFactorUser = Curve::CurveValue(state, PLFFPLRCurveIndex, PLRforSEER);
        PartLoadFactorStandard = 1.0 - (1 - PLRforSEER) * CyclicDegradationCoefficient;

        if (TotalElecPower > 0.0) {
            SEER_User = (NetCoolingCapAHRI / TotalElecPower) * PartLoadFactorUser;
            SEER_Standard = (NetCoolingCapAHRI / TotalElecPower) * PartLoadFactorStandard;
        }
        return std::make_tuple(SEER_User, SEER_Standard);
    }

    std::map<std::string, Real64> SingleSpeedDXCoolingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName,                         // Name of DX coil for which HSPF is calculated
        std::string const &DXCoilType,                         // Type of DX coil - heating or cooling
        int const CapFTempCurveIndex,                          // Index for the capacity as a function of temperature modifier curve
        int const CapFFlowCurveIndex,                          // Index for the capacity as a function of flow fraction modifier curve
        int const EIRFTempCurveIndex,                          // Index for the EIR as a function of temperature modifier curve
        int const EIRFFlowCurveIndex,                          // Index for the EIR as a function of flow fraction modifier curve
        int const PLFFPLRCurveIndex,                           // Index for the EIR vs part-load ratio curve
        Real64 const RatedTotalCapacity,                       // Rated gross total cooling capacity
        Real64 const RatedCOP,                                 // Rated gross COP
        Real64 const RatedAirVolFlowRate,                      // air flow rate through the coil at rated condition
        Real64 const FanPowerPerEvapAirFlowRateFromInput,      // 2017 Fan power per air volume flow rate through the evaporator coil
        Real64 const FanPowerPerEvapAirFlowRateFromInput_2023, // 2023 Fan power per air volume flow rate through the evaportor coil
        DataHeatBalance::RefrigCondenserType CondenserType)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC
        //       DATE WRITTEN   December 2012
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the standard ratings net cooling capacity and, SEER, EER and IEER values for single speed
        // DX cooling coils at the AHRI standard test condition(s).

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using Curve::CurveValue;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TotCapFlowModFac(0.0);           // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Real64 EIRFlowModFac(0.0);              // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Real64 TotCapTempModFac(0.0);           // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 FanPowerPerEvapAirFlowRate;      // 2017 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        Real64 FanPowerPerEvapAirFlowRate_2023; // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]

        Real64 NetCoolingCapRated(0.0); // net cooling capacity of single speed DX cooling coil
        Real64 SEER_User(0.0);          // seasonal energy efficiency ratio of single speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 SEER_Standard(0.0);      // seasonal energy efficiency ratio of single speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 EER(0.0);                // energy efficiency ratio of single speed DX cooling coil
        Real64 IEER(0.0);               // Integareted energy efficiency ratio of single speed DX cooling coil

        Real64 NetCoolingCapRated2023(0.0); // net cooling capacity of single speed DX cooling coil
        Real64 SEER2_User(0.0);     // seasonal energy efficiency ratio of single speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 SEER2_Standard(0.0); // seasonal energy efficiency ratio of single speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 EER_2022(0.0);       // energy efficiency ratio of single speed DX cooling coil
        Real64 IEER_2022(0.0);      // Integrated energy efficiency ratio

        std::map<std::string, Real64> StandarRatingResults;
        // StandarRatingResults["NetCoolingCapRated"] = NetCoolingCapRated;
        // StandarRatingResults["SEER_User"] = SEER_User;
        // StandarRatingResults["SEER_Standard"] = SEER_Standard;
        // StandarRatingResults["EER"] = EER;
        // StandarRatingResults["IEER"] = IEER;
        // StandarRatingResults["NetCoolingCapRated2023"] = NetCoolingCapRated2023;
        // StandarRatingResults["SEER2_User"] = SEER2_User;
        // StandarRatingResults["SEER2_Standard"] = SEER2_Standard;
        // StandarRatingResults["EER_2022"] = EER_2022;
        // StandarRatingResults["IEER_2022"] = IEER_2022;

        if (FanPowerPerEvapAirFlowRateFromInput <= 0.0) {
            FanPowerPerEvapAirFlowRate = DefaultFanPowerPerEvapAirFlowRate;
        } else {
            FanPowerPerEvapAirFlowRate = FanPowerPerEvapAirFlowRateFromInput;
        }

        if (FanPowerPerEvapAirFlowRateFromInput_2023 <= 0.0) {
            FanPowerPerEvapAirFlowRate_2023 = DefaultFanPowerPerEvapAirFlowRateSEER2;
        } else {
            FanPowerPerEvapAirFlowRate_2023 = FanPowerPerEvapAirFlowRateFromInput_2023;
        }

        // SEER2 standard applies to factory-made Unitary Air-conditioners and Unitary Air-source Heat Pumps with
        // capacities less than 65,000 Btu/h (19049.61955 Watts) | Section 2.1 (ANSI/AHRI 210-240 2023)
        if (RatedTotalCapacity > 0.0 && RatedAirVolFlowRate > 0.0) {
            // Standard Rating Cooling (net) Capacity calculations:
            TotCapFlowModFac = CurveValue(state, CapFFlowCurveIndex, AirMassFlowRatioRated);
            TotCapTempModFac = CurveValue(state, CapFTempCurveIndex, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
            // Net Cooling = Gross Capacity - Fan Power Consumption
            NetCoolingCapRated = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
            StandarRatingResults["NetCoolingCapRated"] = NetCoolingCapRated;
            NetCoolingCapRated2023 = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate_2023 * RatedAirVolFlowRate;
            StandarRatingResults["NetCoolingCapRated2023"] = NetCoolingCapRated2023;
            EIRFlowModFac = Curve::CurveValue(state, EIRFFlowCurveIndex, AirMassFlowRatioRated);

            // IEER calculations
            // EER calculations:
            EER = EERSingleSpeedCooling(state,
                                        CapFTempCurveIndex,
                                        RatedTotalCapacity,
                                        TotCapFlowModFac,
                                        FanPowerPerEvapAirFlowRate,
                                        RatedAirVolFlowRate,
                                        EIRFTempCurveIndex,
                                        RatedCOP,
                                        EIRFlowModFac);
            StandarRatingResults["EER"] = EER;

            // EER2 calculations:
            EER_2022 = EERSingleSpeedCooling(state,
                                             CapFTempCurveIndex,
                                             RatedTotalCapacity,
                                             TotCapFlowModFac,
                                             FanPowerPerEvapAirFlowRate_2023,
                                             RatedAirVolFlowRate,
                                             EIRFTempCurveIndex,
                                             RatedCOP,
                                             EIRFlowModFac);
            StandarRatingResults["EER_2022"] = EER_2022;

            // TODO: Commercial and industrial unitary air-conditioning condensing units with a capacity greater than 135,000 Btu/h (39564.59445
            // Watts) as defined in ANSI/AHRI Standard 365(I-P). | Scope 2.2.6 (ANSI/AHRI 340-360 2022)

            // SEER2 standard applies to factory-made Unitary Air-conditioners and Unitary Air-source Heat Pumps with
            // capacities less than 65,000 Btu/h (19049.61955 Watts) | Section 2.1 (ANSI/AHRI 210-240 2023)
            // Removal of water-cooled and evaporatively-cooled products from the scope | Foreword (ANSI/AHRI 210-240 2023)

            std::tie(SEER_User, SEER_Standard) = SEERSingleStageCalculation(state,
                                                                            CapFTempCurveIndex,
                                                                            RatedTotalCapacity,
                                                                            TotCapFlowModFac,
                                                                            EIRFTempCurveIndex,
                                                                            EIRFlowModFac,
                                                                            EIRFFlowCurveIndex,
                                                                            RatedCOP,
                                                                            FanPowerPerEvapAirFlowRate,
                                                                            RatedAirVolFlowRate,
                                                                            PLFFPLRCurveIndex,
                                                                            CyclicDegradationCoeff);
            StandarRatingResults["SEER_User"] = SEER_User;
            StandarRatingResults["SEER_Standard"] = SEER_Standard;
            if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
                std::tie(SEER2_User, SEER2_Standard) = SEERSingleStageCalculation(state,
                                                                                  CapFTempCurveIndex,
                                                                                  RatedTotalCapacity,
                                                                                  TotCapFlowModFac,
                                                                                  EIRFTempCurveIndex,
                                                                                  EIRFlowModFac,
                                                                                  EIRFFlowCurveIndex,
                                                                                  RatedCOP,
                                                                                  FanPowerPerEvapAirFlowRate_2023,
                                                                                  RatedAirVolFlowRate,
                                                                                  PLFFPLRCurveIndex,
                                                                                  CyclicDegradationCoeffSEER2);
                StandarRatingResults["SEER2_User"] = SEER2_User;
                StandarRatingResults["SEER2_Standard"] = SEER2_Standard;
            }

            std::tie(IEER, NetCoolingCapRated) = IEERSingleSpeedCooling(state,
                                                                        CapFTempCurveIndex,
                                                                        RatedTotalCapacity,
                                                                        TotCapFlowModFac,
                                                                        FanPowerPerEvapAirFlowRate,
                                                                        RatedAirVolFlowRate,
                                                                        EIRFTempCurveIndex,
                                                                        RatedCOP,
                                                                        EIRFlowModFac);

            StandarRatingResults["IEER"] = IEER;
            StandarRatingResults["NetCoolingCapRated"] = NetCoolingCapRated;

            // IEER 2022 Calculations
            if (DXCoilType == "Coil:Cooling:DX:SingleSpeed") {

                std::tie(IEER_2022, NetCoolingCapRated2023, EER_2022) = IEERCalculationSingleSpeed(state,
                                                                                                   DXCoilType,
                                                                                                   CapFTempCurveIndex,
                                                                                                   RatedTotalCapacity,
                                                                                                   TotCapFlowModFac,
                                                                                                   FanPowerPerEvapAirFlowRate_2023,
                                                                                                   RatedAirVolFlowRate,
                                                                                                   EIRFTempCurveIndex,
                                                                                                   RatedCOP,
                                                                                                   EIRFlowModFac,
                                                                                                   CondenserType);
                StandarRatingResults["IEER_2022"] = IEER_2022;
                StandarRatingResults["EER_2022"] = EER_2022;
                StandarRatingResults["NetCoolingCapRated2023"] = NetCoolingCapRated2023;
            }

        } else {
            ShowSevereError(state,
                            format("Standard Ratings: {} {} has esither zero rated total cooling capacity or zero rated air volume flow rate. "
                                   "Standard ratings cannot be calculated.",
                                   DXCoilType,
                                   DXCoilName));
        }
        return StandarRatingResults;
    }

    void DXCoolingCoilDataCenterStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilName,                    // Name of DX coil for which HSPF is calculated
        std::string const &DXCoilType,                    // Type of DX coil - heating or cooling
        int const CapFTempCurveIndex,                     // Index for the capacity as a function of temperature modifier curve
        int const CapFFlowCurveIndex,                     // Index for the capacity as a function of flow fraction modifier curve
        int const EIRFTempCurveIndex,                     // Index for the EIR as a function of temperature modifier curve
        int const EIRFFlowCurveIndex,                     // Index for the EIR as a function of flow fraction modifier curve
        [[maybe_unused]] int const PLFFPLRCurveIndex,     // Index for the EIR vs part-load ratio curve
        Real64 const RatedTotalCapacity,                  // Rated gross total cooling capacity
        Real64 const RatedCOP,                            // Rated gross COP
        Real64 const RatedAirVolFlowRate,                 // air flow rate through the coil at rated condition
        Real64 const FanPowerPerEvapAirFlowRateFromInput, // Fan power per air volume flow rate through the evaporator coil
        Array1D<Real64> &NetCoolingCapRated,              // net cooling capacity of single speed DX cooling coil
        Array1D<Real64> &TotElectricPowerRated            // total electric power including supply fan
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC
        //       DATE WRITTEN   October 2014
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the standard ratings net cooling capacity and total electric power
        // for room unitary air conditioners single speed DX cooling coils.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // ANSI/ASHRAE Standard 127-2012 - Method of Testing for Rating Computer and Data Processing
        //                                 Room Unitary Air Conditioners

        // Using/Aliasing
        using Curve::CurveValue;
        using Psychrometrics::PsyTwbFnTdbWPb;
        using Psychrometrics::PsyWFnTdpPb;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 TotCapFlowModFac(0.0);      // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Real64 EIRFlowModFac(0.0);         // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Real64 TotCapTempModFac(0.0);      // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 EIRTempModFac(0.0);         // EIR modifier (function of entering wetbulb, outside drybulb) [-]
        Real64 TotCoolingCapRated(0.0);    // Total Cooling Coil capacity (gross) at one of the rated test conditions [W]
        Real64 EIR(0.0);                   // Energy Efficiency Ratio at AHRI test conditions for SEER [-]
        Real64 FanPowerPerEvapAirFlowRate; // Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]

        Real64 TWBIndoor;  // indoor air dry bulb temperature
        Real64 TDBOutdoor; // outdor air dry bulb temperature

        if (FanPowerPerEvapAirFlowRateFromInput <= 0.0) {
            FanPowerPerEvapAirFlowRate = DefaultFanPowerPerEvapAirFlowRate;
        } else {
            FanPowerPerEvapAirFlowRate = FanPowerPerEvapAirFlowRateFromInput;
        }
        if (RatedTotalCapacity > 0.0) {
            int Num; // text number counter

            for (int ClassNum = 1; ClassNum <= 4; ++ClassNum) {
                TWBIndoor = PsyTwbFnTdbWPb(state,
                                           IndoorDBTempClassI2IV[ClassNum - 1],
                                           PsyWFnTdpPb(state, IndoorTDPA2D, state.dataEnvrn->StdBaroPress),
                                           state.dataEnvrn->StdBaroPress);
                for (int TestNum = 1; TestNum <= 4; ++TestNum) {
                    TDBOutdoor = OutdoorDBTempAllClassA2D[TestNum - 1];
                    Num = (ClassNum - 1) * 4 + TestNum;
                    // Standard Rating Net Cooling Capacity at Test A:
                    TotCapFlowModFac = CurveValue(state, CapFFlowCurveIndex, AirMassFlowRatioRated);
                    TotCapTempModFac = CurveValue(state, CapFTempCurveIndex, TWBIndoor, TDBOutdoor);
                    TotCoolingCapRated = RatedTotalCapacity * TotCapTempModFac * TotCapFlowModFac;
                    NetCoolingCapRated(Num) = TotCoolingCapRated - FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
                    // Standard Rating total electric power at Test A:
                    EIRTempModFac = CurveValue(state, EIRFTempCurveIndex, TWBIndoor, TDBOutdoor);
                    EIRFlowModFac = CurveValue(state, EIRFFlowCurveIndex, AirMassFlowRatioRated);
                    EIR = 0.0;
                    if (RatedCOP > 0.0) {
                        EIR = EIRTempModFac * EIRFlowModFac / RatedCOP;
                    }
                    // Calculate net cooling capacity at Test A:
                    TotElectricPowerRated(Num) = EIR * TotCoolingCapRated + FanPowerPerEvapAirFlowRate * RatedAirVolFlowRate;
                }
            }
        } else {
            ShowSevereError(state,
                            format("Standard Ratings: {} {} has zero rated total cooling capacity. Capacity and Power cannot be calculated.",
                                   DXCoilType,
                                   DXCoilName));
        }
    }

    std::tuple<Real64, Real64, Real64, Real64> MultiSpeedDXCoolingCoilSEER(EnergyPlusData &state,
                                                                           int const nsp,
                                                                           Array1A_int const CapFFlowCurveIndex,
                                                                           Array1A<Real64> const RatedTotalCapacity,
                                                                           Array1A_int const CapFTempCurveIndex,
                                                                           Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput,
                                                                           Array1A<Real64> const RatedAirVolFlowRate,
                                                                           Array1A_int const EIRFFlowCurveIndex,
                                                                           Array1A<Real64> const RatedCOP,
                                                                           Array1A_int EIRFTempCurveIndex,
                                                                           Array1A_int const PLFFPLRCurveIndex)
    {
        // Intermediate values calculated from the inputs in the idf file
        Array1D<Real64> FanPowerPerEvapAirFlowRate(nsp); // 2017 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        Array1D<Real64> TotCoolCapTestA2(nsp);           // Total cooling capacity at A2 test condition (High speed)
        Array1D<Real64> TotCoolCapTestB2(nsp);           // Total cooling capacity at B2 test condition (High speed)
        Array1D<Real64> TotCoolCapTestB1(nsp);           // Total cooling capacity at B1 test condition (Low speed)
        Array1D<Real64> TotCoolCapTestF1(nsp);           // Total cooling capacity at F1 test condition (Low speed)
        Array1D<Real64> OutdoorUnitPowerTestA2(nsp);     // Outdoor Unit electric power at A2 test condition (High speed)
        Array1D<Real64> OutdoorUnitPowerTestB2(nsp);     // Outdoor Unit electric power at B2 test condition (High speed)
        Array1D<Real64> OutdoorUnitPowerTestB1(nsp);     // Outdoor Unit electric power at B1 test condition (Low speed)
        Array1D<Real64> OutdoorUnitPowerTestF1(nsp);     // Outdoor Unit electric power at F1 test condition

        Real64 PartLoadRatio(0.0);      // compressor cycling ratio between successive speeds, [-]
        Real64 PartLoadFactorUser(0.0); // part-load factor based on user-input PLF curve and C_D value that accounts for the cyclic degradation, [-]
        Real64 PartLoadFactorStandard(
            0.0); // part-load factorn that accounts for the cyclic degradation from AHRI standard 210/240 default PLF curve and C_D value, [-]
        Real64 NetCoolingCapWeighted(0.0);              // net tot cooling cap weighted by the fraction of the binned cooling hours [W]
        Real64 TotCoolingElecPowerWeighted(0.0);        // net total cooling electric power input weighted by the fraction of the temperature bins
        Real64 TotCoolingElecPowerWeightedDefault(0.0); // net total cooling electric power input weighted by the fraction of the temperature bins
                                                        // from AHRI 201/240 default PLF curve and C_D value,

        // binned cooling hours
        Real64 BuildingCoolingLoad(0.0);    // Building space cooling load corresponding to an outdoor bin temperature [W]
        Real64 NetTotCoolCapBinned(0.0);    // Net tot cooling cap corresponding to an outdoor bin temperature [W]
        Real64 TotCoolElecPowerBinned(0.0); // Total cooling electric power corresponding to an outdoor bin temperature [W]
        Real64 TotCoolElecPowerBinnedDefault(
            0.0); // Total cooling electric power corresponding to an outdoor bin temperature from AHRI 201/240 default PLF curve and C_D value, [W]
        Real64 LoadFactor(0.0); // "on" time for last stage at the desired reduced capacity, (dimensionless)

        int BinNum;                              // bin number counter
        Array1D<Real64> NetCoolingCapRated(nsp); // net cooling capacity at each speed
        Array1D<Real64> TotCapFlowModFac(nsp);   // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);      // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Real64 CoolingCapacityLS(0.0);           // cooling capacity of Mult-speed DX coil at lower speed, [W]
        Real64 CoolingCapacityHS(0.0);           // cooling capacity of Mult-speed DX coil at higher speed, [W]
        Real64 CoolingElecPowerLS(0.0);          // outdoor unit electric power input at low speed, [W]
        Real64 CoolingElecPowerHS(0.0);          // outdoor unit electric power input at high speed, [W]
        Real64 CoolingCapacityMax(0.0);          // cooling capacity of Mult-speed DX coil at max speed, [W]
        Real64 CoolingElecPowerMax(0.0);         // outdoor unit electric power input at Max speed, [W]

        Real64 constexpr SizingFactor(1.10); // sizing factor per AHRI Std 210/240-2008

        Real64 NetCoolingCapRatedMaxSpeed = 0.0;
        Real64 SEER_User = 0.0;
        Real64 SEER_Standard = 0.0;
        Real64 EER = 0.0;

        NetCoolingCapWeighted = 0.0;
        TotCoolingElecPowerWeighted = 0.0;
        TotCoolingElecPowerWeightedDefault = 0.0;

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate(spnum) = 0.0;
            if (FanPowerPerEvapAirFlowRateFromInput(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate(spnum) = DefaultFanPowerPerEvapAirFlowRate;
            } else {
                FanPowerPerEvapAirFlowRate(spnum) = FanPowerPerEvapAirFlowRateFromInput(spnum);
            }
        }

        // Calculate the capacity and power for each speed
        for (int spnum = 1; spnum <= nsp; ++spnum) {
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, static_cast<int64_t>(CapFFlowCurveIndex(spnum)), AirMassFlowRatioRated);
            TotCoolCapTestA2(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

            TotCoolCapTestB2(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

            TotCoolCapTestB1(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

            TotCoolCapTestF1(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            if (RatedCOP(spnum) > 0.0) {
                OutdoorUnitPowerTestA2(spnum) =
                    TotCoolCapTestA2(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

                OutdoorUnitPowerTestB2(spnum) =
                    TotCoolCapTestB2(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

                OutdoorUnitPowerTestB1(spnum) =
                    TotCoolCapTestB1(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

                OutdoorUnitPowerTestF1(spnum) =
                    TotCoolCapTestF1(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
            }
        }
        // Standard Rating cooling (net) capacity calculations:
        NetCoolingCapRated(nsp) = TotCoolCapTestA2(nsp);
        NetCoolingCapRatedMaxSpeed = NetCoolingCapRated(nsp);

        // EER2 calculation |  Section 3.1.16(AHRI 210/240 2017)
        // A ratio of the cooling capacity in Btu/h to the Total Power in watts at AFull test conditions and expressed in Btu/(W-h)
        // In case of Coil:Cooling:DX:MultiSpeed coil we're picking the max speed.
        EER = TotCoolCapTestA2(nsp) / OutdoorUnitPowerTestA2(nsp);

        // Calculate the SEER value based on contribution of each outdoor air bin temperature
        for (BinNum = 0; BinNum < NumOfOATempBins; ++BinNum) {
            // Equation 11.60 (AHRI-2017)
            BuildingCoolingLoad = (OutdoorBinTemperatureSEER[BinNum] - 18.3) / (35.0 - 18.3) * (TotCoolCapTestA2(nsp) / SizingFactor);
            // determine the speed number
            CoolingCapacityMax = TotCoolCapTestB2(nsp) + ((TotCoolCapTestA2(nsp) - TotCoolCapTestB2(nsp)) /
                                                          (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                                             (OutdoorBinTemperatureSEER[BinNum] - OutdoorCoilInletAirDryBulbTempTestB2);
            CoolingElecPowerMax = OutdoorUnitPowerTestB2(nsp) + ((OutdoorUnitPowerTestA2(nsp) - OutdoorUnitPowerTestB2(nsp)) /
                                                                 (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                                                    (OutdoorBinTemperatureSEER[BinNum] - OutdoorCoilInletAirDryBulbTempTestB2);

            for (int spnum = 1; spnum <= nsp - 1; ++spnum) {
                CoolingCapacityLS = TotCoolCapTestF1(spnum) + ((TotCoolCapTestB1(spnum) - TotCoolCapTestF1(spnum)) /
                                                               (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                                                  (OutdoorBinTemperatureSEER[BinNum] - OutdoorCoilInletAirDryBulbTempTestF1);
                CoolingElecPowerLS = OutdoorUnitPowerTestF1(spnum) + ((OutdoorUnitPowerTestB1(spnum) - OutdoorUnitPowerTestF1(spnum)) /
                                                                      (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                                                         (OutdoorBinTemperatureSEER[BinNum] - OutdoorCoilInletAirDryBulbTempTestF1);
                CoolingCapacityHS = TotCoolCapTestB2(spnum + 1) + ((TotCoolCapTestA2(spnum + 1) - TotCoolCapTestB2(spnum + 1)) /
                                                                   (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                                                      (OutdoorBinTemperatureSEER[BinNum] - OutdoorCoilInletAirDryBulbTempTestB2);
                CoolingElecPowerHS =
                    OutdoorUnitPowerTestB2(spnum + 1) + ((OutdoorUnitPowerTestA2(spnum + 1) - OutdoorUnitPowerTestB2(spnum + 1)) /
                                                         (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                                            (OutdoorBinTemperatureSEER[BinNum] - OutdoorCoilInletAirDryBulbTempTestB2);

                if (BuildingCoolingLoad <= CoolingCapacityLS) {
                    PartLoadRatio = min(1.0, BuildingCoolingLoad / CoolingCapacityLS);
                    NetTotCoolCapBinned = PartLoadRatio * CoolingCapacityLS;
                    PartLoadFactorUser = Curve::CurveValue(state, PLFFPLRCurveIndex(spnum), PartLoadRatio);
                    PartLoadFactorStandard = 1.0 - CyclicDegradationCoeff * (1.0 - PartLoadRatio);
                    TotCoolElecPowerBinned = (PartLoadRatio / PartLoadFactorUser) * CoolingElecPowerLS;
                    TotCoolElecPowerBinnedDefault = (PartLoadRatio / PartLoadFactorStandard) * CoolingElecPowerLS;
                    goto SpeedLoop_exit;
                } else if (BuildingCoolingLoad < CoolingCapacityHS) {
                    // cycle between speed "spnum" and "spnum + 1"
                    LoadFactor = min(1.0, (CoolingCapacityHS - BuildingCoolingLoad) / (CoolingCapacityHS - CoolingCapacityLS));
                    LoadFactor = max(0.0, LoadFactor);
                    NetTotCoolCapBinned = LoadFactor * CoolingCapacityLS + (1.0 - LoadFactor) * CoolingCapacityHS;
                    TotCoolElecPowerBinned = LoadFactor * CoolingElecPowerLS + (1.0 - LoadFactor) * CoolingElecPowerHS;
                    TotCoolElecPowerBinnedDefault = TotCoolElecPowerBinned;

                    // TBD:  // Suggestion for improvement as per 2017 Standard MCQ & MCE are differently calculated
                    // and so does the load factor for Cooling and Power
                    // Real64 LoadFactorQEnt_2023 =
                    //     min(1.0, (TotCoolCapTestEint(spnum2023) - CoolingCapacityLS_2023) / (CoolingCapacityHS_2023 - CoolingCapacityLS_2023));
                    // LoadFactorQEnt_2023 = max(0.0, LoadFactorQEnt_2023);
                    // Real64 LoadFactorPEnt_2023 = min(
                    //     1.0, (OutdoorUnitPowerTestEint(spnum2023) - CoolingElecPowerLS_2023) / (CoolingElecPowerHS_2023 -
                    //     CoolingElecPowerLS_2023));
                    // LoadFactorPEnt_2023 = max(0.0, LoadFactorPEnt_2023);

                    // NetTotCoolCapBinned_2023 = LoadFactorQEnt_2023 * CoolingCapacityHS_2023 + (1.0 - LoadFactorQEnt_2023) * CoolingCapacityLS_2023;
                    // TotCoolElecPowerBinned_2023 =
                    //     LoadFactorPEnt_2023 * CoolingElecPowerHS_2023 + (1.0 - LoadFactorPEnt_2023) * CoolingElecPowerLS_2023;
                    // TotCoolElecPowerBinnedDefault_2023 = TotCoolElecPowerBinned_2023;
                    goto SpeedLoop_exit;
                } else if (BuildingCoolingLoad >= CoolingCapacityMax) {
                    NetTotCoolCapBinned = CoolingCapacityMax;
                    TotCoolElecPowerBinned = CoolingElecPowerMax;
                    TotCoolElecPowerBinnedDefault = CoolingElecPowerMax;
                    goto SpeedLoop_exit;
                }
            }
        SpeedLoop_exit:;

            NetCoolingCapWeighted += NetTotCoolCapBinned * CoolFracBinHoursAtOutdoorBinTemp[BinNum];
            TotCoolingElecPowerWeighted += TotCoolElecPowerBinned * CoolFracBinHoursAtOutdoorBinTemp[BinNum];
            TotCoolingElecPowerWeightedDefault += TotCoolElecPowerBinnedDefault * CoolFracBinHoursAtOutdoorBinTemp[BinNum];
        }
        SEER_User = 0.0;
        SEER_Standard = 0.0;
        if (TotCoolingElecPowerWeighted > 0.0) {
            SEER_User = NetCoolingCapWeighted / TotCoolingElecPowerWeighted;
            SEER_Standard = NetCoolingCapWeighted / TotCoolingElecPowerWeightedDefault;
        }

        return std::make_tuple(NetCoolingCapRatedMaxSpeed, SEER_User, SEER_Standard, EER);
    }

    std::pair<Real64, int> GetMatchingSpeedFromBuildingLoad(Real64 buildingLoad, const Array1A<Real64> &speedList)
    {
        std::pair<int, Real64> result = {-1, -1}; // Initialize result to indicate no suitable number found
        for (int i = 0; i < speedList.isize(); ++i) {
            Real64 scaledSpeed = speedList[i];
            if (scaledSpeed >= buildingLoad) {
                result = {speedList[i], i};
                break;
            }
        }
        return result;
    }

    // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
    // q - total bin capacity for SEER2 Standard.
    // e - total bin energy for SEER2 Standard.
    // NetTotCoolCapBinned_2023 - total bin capacity for SEER2 User.
    // TotCoolElecPowerBinned_2023 - total bin energy for SEER2 Standard.
    std::tuple<Real64, Real64, Real64, Real64>
    IntermediateCapacityAndPowerSEER2Case1(EnergyPlusData &state, Real64 bl, Real64 q_low, Real64 n, Real64 p_low, int PLFFPLRCurveIndex)
    {
        // Case I. Building load is less than Low Stage capacity, BL(tj) < qLow(tj). Calculate total bin capacity by using Equation 11.73
        // and total bin energy by using Equation 11.74.
        Real64 e(0.0);
        Real64 q(0.0);
        Real64 NetTotCoolCapBinned_2023(0.0);
        Real64 TotCoolElecPowerBinned_2023(0.0);
        Real64 clf_low = bl / q_low; // Equation 11.75 (AHRI-2023)

        // SEER2 STANDARD
        Real64 plf_low = 1.0 - CyclicDegradationCoeff * (1.0 - clf_low); // Equation 11.76 (AHRI-2023)
        q = clf_low * q_low * n;                                         // Total Bin Capacity, Equation 11.73 (AHRI-2023)
        e = clf_low * p_low * n / plf_low;                               // Total Bin Energy, Equation 11.74 (AHRI-2023)

        // SEER2 USER
        NetTotCoolCapBinned_2023 = clf_low * q_low * n;
        Real64 PartLoadFactorUser_2023 = Curve::CurveValue(state, PLFFPLRCurveIndex, clf_low);
        TotCoolElecPowerBinned_2023 = (clf_low / PartLoadFactorUser_2023) * p_low * n;
        return std::make_tuple(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023);
    }

    // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed & full
    // Speed
    // q - total bin capacity for SEER2 Standard.
    // e - total bin energy for SEER2 Standard.
    // NetTotCoolCapBinned_2023 - total bin capacity for SEER2 User.
    // TotCoolElecPowerBinned_2023 - total bin energy for SEER2 Standard.
    std::tuple<Real64, Real64, Real64, Real64> IntermediateCapacityAndPowerSEER2Case2A(
        Real64 p_int, Real64 q_int, Real64 q_low, Real64 bl, Real64 n, Real64 Q_E_Int, Real64 q_full, Real64 P_E_Int, Real64 p_full, Real64 p_low)
    {
        Real64 e(0.0);
        Real64 q(0.0);

        Real64 cop_low = q_low / p_low;
        Real64 cop_int = q_int / p_int;
        // Real64 cop_full = q_full / p_full; // About half of the variables in this function are unused
        // Low Speed
        Real64 cop_int_bin = cop_low + (cop_int - cop_low) / (q_int - q_low) * (bl - q_low); // Equation 11.101 (AHRI-2023)
        q = bl * n;                                                                          // 11.92 --> n is missing in the print ?
        e = q / cop_int_bin;                                                                 // 11.93 --> adjusted to 11.101
        Real64 NetTotCoolCapBinned_2023(0.0);
        Real64 TotCoolElecPowerBinned_2023(0.0);
        // full Speed
        Real64 LoadFactorQEnt_2023 = min(1.0, (Q_E_Int - q_low) / (q_full - q_low));
        LoadFactorQEnt_2023 = max(0.0, LoadFactorQEnt_2023);
        Real64 LoadFactorPEnt_2023 = min(1.0, (P_E_Int - p_low) / (p_full - p_low));
        LoadFactorPEnt_2023 = max(0.0, LoadFactorPEnt_2023);
        NetTotCoolCapBinned_2023 = LoadFactorQEnt_2023 * q_full + (1.0 - LoadFactorQEnt_2023) * q_low;
        TotCoolElecPowerBinned_2023 = LoadFactorPEnt_2023 * p_full + (1.0 - LoadFactorPEnt_2023) * p_low;
        return std::make_tuple(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023);
    }

    // Building load can be matched by modulating the compressor speed between low speed &
    // full Speed |Section 11.2.1.3.2 CASE 2 (AHRI 210-240 2023)
    // q - total bin capacity for SEER2 Standard.
    // e - total bin energy for SEER2 Standard.
    // NetTotCoolCapBinned_2023 - total bin capacity for SEER2 User.
    // TotCoolElecPowerBinned_2023 - total bin energy for SEER2 Standard.
    std::tuple<Real64, Real64, Real64, Real64> IntermediateCapacityAndPowerSEER2Case2B(
        Real64 p_int, Real64 bl, Real64 q_int, Real64 n, Real64 Q_E_Int, Real64 P_E_Int, Real64 q_low, Real64 p_low, Real64 q_full, Real64 p_full)
    {
        Real64 e(0.0);
        Real64 q(0.0);
        Real64 NetTotCoolCapBinned_2023(0.0);
        Real64 TotCoolElecPowerBinned_2023(0.0);
        // Real64 cop_low = q_low / p_low;
        Real64 cop_int = q_int / p_int;
        Real64 cop_full = q_full / p_full;
        Real64 LoadFactorQEnt_2023 = min(1.0, (Q_E_Int - q_low) / (q_full - q_low));
        LoadFactorQEnt_2023 = max(0.0, LoadFactorQEnt_2023);
        Real64 LoadFactorPEnt_2023 = min(1.0, (P_E_Int - p_low) / (p_full - p_low));
        LoadFactorPEnt_2023 = max(0.0, LoadFactorPEnt_2023);
        NetTotCoolCapBinned_2023 = LoadFactorQEnt_2023 * q_full + (1.0 - LoadFactorQEnt_2023) * q_low;
        TotCoolElecPowerBinned_2023 = LoadFactorPEnt_2023 * p_full + (1.0 - LoadFactorPEnt_2023) * p_low;
        // Full Speed
        Real64 cop_int_bin = cop_int + (cop_full - cop_int) / (q_full - q_int) * (bl - q_int); // Equation 11.102 (AHRI-2023)
        q = bl * n;                                                                            // 11.92 --> n is missing in the print ?
        e = q / cop_int_bin;                                                                   // 11.93 --> adjusted to 11.102
        return std::make_tuple(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023);
    }

    // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
    // q - total bin capacity for SEER2 Standard.
    // e - total bin energy for SEER2 Standard.
    // NetTotCoolCapBinned_2023 - total bin capacity for SEER2 User.
    // TotCoolElecPowerBinned_2023 - total bin energy for SEER2 Standard.
    std::tuple<Real64, Real64, Real64, Real64>
    IntermediateCapacityAndPowerSEER2Case3(Real64 q_full, Real64 p_full, Real64 CoolingCapacityMax_2023, Real64 CoolingElecPowerMax_2023, Real64 n)
    {
        Real64 e(0.0);
        Real64 q(0.0);
        q = q_full * n; // Equation 11.88 (AHRI-2023)
        e = p_full * n; // Equation 11.89 (AHRI-2023)
        Real64 NetTotCoolCapBinned_2023(0.0);
        Real64 TotCoolElecPowerBinned_2023(0.0);
        NetTotCoolCapBinned_2023 = CoolingCapacityMax_2023 * n;
        TotCoolElecPowerBinned_2023 = CoolingElecPowerMax_2023 * n;
        return std::make_tuple(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023);
    }

    // N_Cq - Capacity adjustment factor in cooling mode SEER2 #1
    // M_Cq - Capacity adjustment factor in cooling mode SEER2 #2
    std::tuple<Real64, Real64>
    CapacityAdjustmentFactorsInCoolingModeSEER2(Real64 q_F_low, Real64 q_B_low, Real64 BN, Real64 q_B_full, Real64 q_A_full, Real64 q_E_int)
    {
        // Equation 11.90 (AHRI-2023)
        Real64 q_87_low = q_F_low + (q_B_low - q_F_low) * ((OutdoorBinTemperatureSEER[BN] - 19.44 / 27.77 - 19.44));
        Real64 q_87_full = q_B_full + (q_A_full - q_B_full) * ((OutdoorBinTemperatureSEER[BN] - 19.44 / 27.77 - 19.44));
        // Equation 11.96 (AHRI-2023)
        Real64 N_Cq = (q_E_int - q_87_low) / (q_87_full - q_87_low);
        // Equation 11.95 (AHRI-2023)
        Real64 M_Cq = (q_B_low - q_F_low) / (27.77 - 19.44) * (1. - N_Cq) + (q_A_full - q_B_full) / (35.0 - 27.77) * N_Cq;
        return std::make_tuple(N_Cq, M_Cq);
    }

    // N_CE - Energy adjustment factor in cooling mode SEER2 #1
    // M_CE - Energy adjustment factor in cooling mode SEER2 #2
    std::tuple<Real64, Real64>
    EnergyAdjustmentFactorsInCoolingModeSEER2(Real64 p_F_low, Real64 p_B_low, Real64 BN, Real64 p_B_full, Real64 p_A_full, Real64 p_E_int)
    {
        // Equation 11.91 (AHRI-2023)
        Real64 p_87_low = p_F_low + (p_B_low - p_F_low) * ((OutdoorBinTemperatureSEER[BN] - 19.44 / 27.77 - 19.44));
        Real64 p_87_full = p_B_full + (p_A_full - p_B_full) * ((OutdoorBinTemperatureSEER[BN] - 19.44 / 27.77 - 19.44));

        // Equation 11.99 (AHRI-2023)
        Real64 N_CE = (p_E_int - p_87_low) / (p_87_full - p_87_low);

        // Equaition 11.98 (AHRI-2023)
        Real64 M_CE = (p_B_low - p_F_low) / (27.77 - 19.44) * (1. - N_CE) + (p_A_full - p_B_full) / (35.0 - 27.77) * N_CE;
        return std::make_tuple(N_CE, M_CE);
    }

    // q_int - Intermediate Steady State Capacity SEER2
    // p_int - Intermediate Steady State Power SEER2
    std::tuple<Real64, Real64> IntermediateSteadyStateCpacityAndPowerSEER2(Real64 q_E_int, Real64 M_Cq, Real64 p_E_int, Real64 M_CE, Real64 t)
    {
        // Equation 11.94 (AHRI-2023)
        Real64 q_int = q_E_int + M_Cq * (t - 30.55);

        // Equation 11.97 (AHRI-2023)
        Real64 p_int = p_E_int + M_CE * (t - 30.55);
        return std::make_tuple(q_int, p_int);
    }

    std::tuple<Real64, Real64, Real64, Real64> VariableSpeedDXCoolingCoilSEER2(EnergyPlusData &state,
                                                                               int const nsp,
                                                                               Array1A_int const CapFFlowCurveIndex,
                                                                               Array1A<Real64> const GrossRatedCapacityAtSpeedLevel,
                                                                               Array1A_int const CapFTempCurveIndex,
                                                                               Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                               Array1A<Real64> const LoopVolumetricAirFlowRateAtSpeedLevel,
                                                                               Array1A_int const EIRFFlowCurveIndex,
                                                                               Array1A<Real64> const RatedCOP,
                                                                               Array1A_int EIRFTempCurveIndex,
                                                                               Array1A_int const PLFFPLRCurveIndex)
    {

        Real64 NetCoolingCapRatedMaxSpeed2023 = 0.0;
        Real64 SEER2_User = 0.0;
        Real64 SEER2_Standard = 0.0;
        Real64 EER2 = 0.0;

        // Intermediate values calculated from the inputs in the idf file
        // ANSI/AHRI 210/240 Std. 2023
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        Array1D<Real64> Q_A_Full(nsp);                        // Total cooling capacity at A2 test condition (High speed) | q_A_Full
        Array1D<Real64> Q_B_Full(nsp);                        // Total cooling capacity at B2 test condition (High speed) | q_B_Full
        Array1D<Real64> Q_B_Low(nsp);                         // Total cooling capacity at B1 test condition (Low speed) | q_B_Low
        Array1D<Real64> Q_F_Low(nsp);                         // Total cooling capacity at F1 test condition (Low speed) | q_F_Low
        Array1D<Real64> Q_E_Int(nsp);                         // Total cooling capacity at Eint (Ev) test condition | q_E_Int
        Array1D<Real64> P_A_Full(nsp);                        // Outdoor Unit electric power at A2 test condition (High speed) | p_A_Full
        Array1D<Real64> P_B_Full(nsp);                        // Outdoor Unit electric power at B2 test condition (High speed) | p_B_Full
        Array1D<Real64> P_B_Low(nsp);                         // Outdoor Unit electric power at B1 test condition (Low speed) | p_B_Low
        Array1D<Real64> P_F_Low(nsp);                         // Outdoor Unit electric power at F1 test condition | p_F_Low
        Array1D<Real64> P_E_Int(nsp);                         // Outdoor Unit electric power at Eint (Ev) test conditon | p_E_Int

        Array1D<Real64> TotCapFlowModFac(nsp);        // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);           // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Array1D<Real64> NetCoolingCapRated_2023(nsp); // net cooling capacity at each speed
        Real64 q_low(0.0);                            // cooling capacity of Mult-speed DX coil at lower speed, [W]
        Real64 q_full(0.0);                           // cooling capacity of Mult-speed DX coil at higher speed, [W]
        Real64 p_low(0.0);                            // outdoor unit electric power input at low speed, [W]
        Real64 p_full(0.0);                           // outdoor unit electric power input at high speed, [W]
        Real64 CoolingCapacityMax_2023(0.0);          // cooling capacity of Mult-speed DX coil at max speed, [W]
        Real64 CoolingElecPowerMax_2023(0.0);         // outdoor unit electric power input at Max speed, [W]

        // binned cooling hours
        Real64 BuildingCoolingLoad_2023(0.0);    // Building space cooling load corresponding to an outdoor bin temperature [W]
        Real64 NetTotCoolCapBinned_2023(0.0);    // Net tot cooling cap corresponding to an outdoor bin temperature [W]
        Real64 TotCoolElecPowerBinned_2023(0.0); // Total cooling electric power corresponding to an outdoor bin temperature [W]

        Real64 constexpr SF(1.10); // Sizing Factor as per AHRI Std 210/240-2023 | equation 11.68
        // Real64 constexpr V(1);     // V = 0.93 for Variable Speed Heat Pumps, otherwise V = 1.0
        Real64 constexpr V(0.93); // V = 0.93 for Variable Speed Heat Pumps, otherwise V = 1.0
        // part-load factor based on user-input PLF curve and C_D value that accounts for the cyclic degradation, [-]
        // Real64 PartLoadFactorUser_2023(0.0);

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate_2023(spnum) = 0.0;
            if (FanPowerPerEvapAirFlowRateFromInput_2023(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
            } else {
                FanPowerPerEvapAirFlowRate_2023(spnum) = FanPowerPerEvapAirFlowRateFromInput_2023(spnum);
            }
        }

        // Calculate the capacity and power for each speed
        for (int spnum = 1; spnum <= nsp; ++spnum) {
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            Q_A_Full(spnum) =
                GrossRatedCapacityAtSpeedLevel(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

            Q_B_Full(spnum) =
                GrossRatedCapacityAtSpeedLevel(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

            Q_B_Low(spnum) =
                GrossRatedCapacityAtSpeedLevel(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

            Q_F_Low(spnum) =
                GrossRatedCapacityAtSpeedLevel(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

            Q_E_Int(spnum) =
                GrossRatedCapacityAtSpeedLevel(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestEint) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            if (RatedCOP(spnum) > 0.0) {
                P_A_Full(spnum) =
                    Q_A_Full(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

                P_B_Full(spnum) =
                    Q_B_Full(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

                P_B_Low(spnum) = Q_B_Low(spnum) * EIRFlowModFac(spnum) *
                                     Curve::CurveValue(
                                         state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) /
                                     RatedCOP(spnum) +
                                 FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

                P_F_Low(spnum) = Q_F_Low(spnum) * EIRFlowModFac(spnum) *
                                     Curve::CurveValue(
                                         state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) /
                                     RatedCOP(spnum) +
                                 FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);

                P_E_Int(spnum) =
                    Q_E_Int(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestEint) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate_2023(spnum) * LoopVolumetricAirFlowRateAtSpeedLevel(spnum);
            }
        }
        // Standard Rating cooling (net) capacity calculations:
        NetCoolingCapRated_2023(nsp) = Q_A_Full(nsp);
        NetCoolingCapRatedMaxSpeed2023 = NetCoolingCapRated_2023(nsp);

        // EER2 calculation |  Section 3.1.16(AHRI 210/240 2023)
        // A ratio of the cooling capacity in Btu/h to the Total Power in watts at AFull test conditions and expressed in Btu/(W-h)
        // In case of Coil:Cooling:DX:MultiSpeed coil we're picking the max speed.
        EER2 = Q_A_Full(nsp) / P_A_Full(nsp);
        // Calculate the SEER2 value based on contribution of each outdoor air bin temperature
        Real64 q_sum = 0.0;
        Real64 e_sum = 0.0;
        Real64 NetCoolingCapWeighted2_2023 = 0.0;
        Real64 TotCoolingElecPowerWeighted2_2023 = 0.0;
        SEER2_User = 0.0;
        SEER2_Standard = 0.0;
        std::vector<int> speedsUsed;
        // speedsUsed.push_back(1);
        for (int BN = 0; BN < NumOfOATempBins; ++BN) {
            // Equation 11.67 (AHRI-2023)
            BuildingCoolingLoad_2023 = ((OutdoorBinTemperatureSEER[BN] - 18.3) / (35.0 - 18.3) * (Q_A_Full(nsp) / SF)) * V;
            // determine the speed number
            CoolingCapacityMax_2023 =
                Q_B_Full(nsp) + ((Q_A_Full(nsp) - Q_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            CoolingElecPowerMax_2023 =
                P_B_Full(nsp) + ((P_A_Full(nsp) - P_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            // Equation 11.69 (AHRI-2023)
            q_low = Q_F_Low(1) + ((Q_B_Low(1) - Q_F_Low(1)) / (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                     (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestF1);
            // Equation 11.70 (AHRI-2023)4
            p_low = P_F_Low(1) + ((P_B_Low(1) - P_F_Low(1)) / (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                     (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestF1);
            // Equation 11.71 (AHRI-2023)
            q_full =
                Q_B_Full(nsp) + ((Q_A_Full(nsp) - Q_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            // Equation 11.72 (AHRI-2023)
            p_full =
                P_B_Full(nsp) + ((P_A_Full(nsp) - P_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            Real64 q(0.0);
            Real64 e(0.0);

            // # Intermediate Capacity
            Real64 q_A_full = Q_A_Full(nsp);
            Real64 q_B_full = Q_B_Full(nsp);
            Real64 q_B_low = Q_B_Low(1);
            Real64 q_F_low = Q_F_Low(1);
            // # Intermediate Power
            Real64 p_A_full = P_A_Full(nsp);
            Real64 p_B_full = P_B_Full(nsp);
            Real64 p_B_low = P_B_Low(1);
            Real64 p_F_low = P_F_Low(1);

            Real64 N_Cq(0.0);
            Real64 M_Cq(0.0);

            Real64 N_CE(0.0);
            Real64 M_CE(0.0);

            Real64 q_int(0.0);
            Real64 p_int(0.0);
            Real64 t = OutdoorBinTemperatureSEER[BN];
            Real64 n = CoolFracBinHoursAtOutdoorBinTemp[BN];
            Real64 bl = BuildingCoolingLoad_2023;
            if ((nsp >= 5 && nsp <= 10)) {
                // New speed selection strategy :
                auto result = GetMatchingSpeedFromBuildingLoad(BuildingCoolingLoad_2023, GrossRatedCapacityAtSpeedLevel);
                if (result.second != -1) {
                    int spnum = result.second + 1;
                    // found a speed that meets the building load
                    // # Intermediate Capacity
                    Real64 q_E_int = Q_E_Int(spnum);
                    std::tie(N_Cq, M_Cq) = CapacityAdjustmentFactorsInCoolingModeSEER2(q_F_low, q_B_low, BN, q_B_full, q_A_full, q_E_int);

                    // # Intermediate Power
                    Real64 p_E_int = P_E_Int(spnum);
                    std::tie(N_CE, M_CE) = EnergyAdjustmentFactorsInCoolingModeSEER2(p_F_low, p_B_low, BN, p_B_full, p_A_full, p_E_int);

                    std::tie(q_int, p_int) = IntermediateSteadyStateCpacityAndPowerSEER2(q_E_int, M_Cq, p_E_int, M_CE, t);

                    // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                    if (BuildingCoolingLoad_2023 <= q_low) {
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case1(state, bl, q_low, n, p_low, PLFFPLRCurveIndex(spnum));
                        speedsUsed.push_back(spnum);
                        goto SpeedLoop3_exit;
                    } else if (BuildingCoolingLoad_2023 < q_full) {
                        // Case 2A:
                        if (bl < q_int) {
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2A(
                                p_int, q_int, q_low, bl, n, Q_E_Int(spnum), q_full, P_E_Int(spnum), p_full, p_low);
                            speedsUsed.push_back(spnum);
                            goto SpeedLoop3_exit;
                        } else { // bl < q_full
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                            // full Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2B(
                                p_int, bl, q_int, n, Q_E_Int(spnum), P_E_Int(spnum), q_low, p_low, q_full, p_full);
                            speedsUsed.push_back(spnum);
                            goto SpeedLoop3_exit;
                        }
                    } else {
                        // Case 3:
                        // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case3(q_full, p_full, CoolingCapacityMax_2023, CoolingElecPowerMax_2023, n);
                        speedsUsed.push_back(spnum);
                        goto SpeedLoop3_exit;
                    }
                } else {
                    // << ",, BIN NUMBER (C3), " << BN + 1 << ", NO SPEEDS MATCHED ??, " << spnum << std::endl;
                }
            } else if (nsp == 4) {
                for (int spnum = 1; spnum <= nsp; ++spnum) {
                    // # Intermediate Capacity
                    Real64 q_E_int;
                    if (spnum == 2 || spnum == 3) {
                        q_E_int = (Q_E_Int(2) + Q_E_Int(3)) * 0.5;
                    } else {
                        q_E_int = Q_E_Int(spnum);
                    }
                    // # Intermediate Power
                    Real64 p_E_int;
                    if (spnum == 2 || spnum == 3) {
                        p_E_int = (P_E_Int(2) + P_E_Int(3)) * 0.5;
                    } else {
                        p_E_int = P_E_Int(spnum);
                    }

                    std::tie(N_Cq, M_Cq) = CapacityAdjustmentFactorsInCoolingModeSEER2(q_F_low, q_B_low, BN, q_B_full, q_A_full, q_E_int);

                    std::tie(N_CE, M_CE) = EnergyAdjustmentFactorsInCoolingModeSEER2(p_F_low, p_B_low, BN, p_B_full, p_A_full, p_E_int);

                    std::tie(q_int, p_int) = IntermediateSteadyStateCpacityAndPowerSEER2(q_E_int, M_Cq, p_E_int, M_CE, t);

                    // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                    if (bl <= q_low) {
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case1(state, bl, q_low, n, p_low, PLFFPLRCurveIndex(spnum));
                        // This is the case and speed we're looking for now we exit and try calculating against the next bin
                        goto SpeedLoop3_exit;
                    } else if (bl < q_full && (spnum == 2 || spnum == 3)) { // bl > q_low

                        // Case 2A:
                        if (bl < q_int) {
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed & full
                            // Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2A(
                                p_int, q_int, q_low, bl, n, Q_E_Int(spnum), q_full, P_E_Int(spnum), p_full, p_low);
                            goto SpeedLoop3_exit;
                        } else { // bl < q_full
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                            // full Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2B(
                                p_int, bl, q_int, n, Q_E_Int(spnum), P_E_Int(spnum), q_low, p_low, q_full, p_full);
                            goto SpeedLoop3_exit;
                        }
                    } else if (bl >= q_full && spnum > 3) {
                        // Case 3:
                        // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case3(q_full, p_full, CoolingCapacityMax_2023, CoolingElecPowerMax_2023, n);
                        goto SpeedLoop3_exit;
                    }
                }
            } else if (nsp == 3) {
                for (int spnum = 1; spnum <= nsp; ++spnum) {
                    // # Intermediate Capacity
                    Real64 q_E_int = Q_E_Int(spnum);
                    std::tie(N_Cq, M_Cq) = CapacityAdjustmentFactorsInCoolingModeSEER2(q_F_low, q_B_low, BN, q_B_full, q_A_full, q_E_int);

                    // # Intermediate Power
                    Real64 p_E_int = P_E_Int(spnum);
                    std::tie(N_CE, M_CE) = EnergyAdjustmentFactorsInCoolingModeSEER2(p_F_low, p_B_low, BN, p_B_full, p_A_full, p_E_int);

                    std::tie(q_int, p_int) = IntermediateSteadyStateCpacityAndPowerSEER2(q_E_int, M_Cq, p_E_int, M_CE, t);

                    // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                    if (bl <= q_low) {
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case1(state, bl, q_low, n, p_low, PLFFPLRCurveIndex(spnum));
                        // This is the case and speed we're looking for now we exit and try calculating against the next bin
                        goto SpeedLoop3_exit;
                    } else if (bl < q_full) { // bl > q_low
                        // Case 2A:
                        if (bl < q_int) {
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed & full
                            // Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2A(
                                p_int, q_int, q_low, bl, n, Q_E_Int(spnum), q_full, P_E_Int(spnum), p_full, p_low);
                            goto SpeedLoop3_exit;
                        } else { // bl < q_full
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                            // full Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2B(
                                p_int, bl, q_int, n, Q_E_Int(spnum), P_E_Int(spnum), q_low, p_low, q_full, p_full);
                            goto SpeedLoop3_exit;
                        }
                    } else { // bl >= q_full
                        // Case 3:
                        // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case3(q_full, p_full, CoolingCapacityMax_2023, CoolingElecPowerMax_2023, n);
                        goto SpeedLoop3_exit;
                    }
                }
            } else if (nsp == 2) {
                for (int spnum = 1; spnum <= nsp; ++spnum) {
                    // # Intermediate Capacity
                    Real64 q_E_int = Q_E_Int(1);
                    std::tie(N_Cq, M_Cq) = CapacityAdjustmentFactorsInCoolingModeSEER2(q_F_low, q_B_low, BN, q_B_full, q_A_full, q_E_int);

                    // # Intermediate Power
                    Real64 p_E_int = P_E_Int(1);
                    std::tie(N_CE, M_CE) = EnergyAdjustmentFactorsInCoolingModeSEER2(p_F_low, p_B_low, BN, p_B_full, p_A_full, p_E_int);

                    std::tie(q_int, p_int) = IntermediateSteadyStateCpacityAndPowerSEER2(q_E_int, M_Cq, p_E_int, M_CE, t);

                    // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                    if (bl <= q_low) {
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case1(state, bl, q_low, n, p_low, PLFFPLRCurveIndex(spnum));
                        // This is the case and speed we're looking for now we exit and try calculating against the next bin
                        goto SpeedLoop3_exit;
                    } else if (bl < q_full) { // bl > q_low
                        // Case 2A:
                        if (bl < q_int) {
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed & full
                            // Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2A(
                                p_int, q_int, q_low, bl, n, Q_E_Int(spnum), q_full, P_E_Int(spnum), p_full, p_low);
                            goto SpeedLoop3_exit;
                        } else { // bl < q_full
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                            // full Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2B(
                                p_int, bl, q_int, n, Q_E_Int(spnum), P_E_Int(spnum), q_low, p_low, q_full, p_full);
                            goto SpeedLoop3_exit;
                        }
                    } else if (spnum == nsp) { // bl >= q_full
                        // Case 3:
                        // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case3(q_full, p_full, CoolingCapacityMax_2023, CoolingElecPowerMax_2023, n);
                        goto SpeedLoop3_exit;
                    }
                }
            } else if (nsp == 1) {
                // Every calculation for each of the bins will be using only one speed i.e, Speed 1
                // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                // # Intermediate Capacity
                Real64 q_E_int = Q_E_Int(1);
                std::tie(N_Cq, M_Cq) = CapacityAdjustmentFactorsInCoolingModeSEER2(q_F_low, q_B_low, BN, q_B_full, q_A_full, q_E_int);

                // # Intermediate Power
                Real64 p_E_int = P_E_Int(1);
                std::tie(N_CE, M_CE) = EnergyAdjustmentFactorsInCoolingModeSEER2(p_F_low, p_B_low, BN, p_B_full, p_A_full, p_E_int);

                std::tie(q_int, p_int) = IntermediateSteadyStateCpacityAndPowerSEER2(q_E_int, M_Cq, p_E_int, M_CE, t);

                // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                if (BuildingCoolingLoad_2023 <= q_low) {
                    // Case 1:
                    std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                        IntermediateCapacityAndPowerSEER2Case1(state, bl, q_low, n, p_low, PLFFPLRCurveIndex(1));
                    goto SpeedLoop3_exit;

                } else if (BuildingCoolingLoad_2023 < q_full) {
                    // Case 2:
                    // Case 2A:
                    if (bl < q_int) {
                        // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed & full
                        // Speed
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case2A(p_int, q_int, q_low, bl, n, Q_E_Int(1), q_full, P_E_Int(1), p_full, p_low);
                        goto SpeedLoop3_exit;
                    } else { // bl < q_full
                        // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                        // full Speed
                        std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                            IntermediateCapacityAndPowerSEER2Case2B(p_int, bl, q_int, n, Q_E_Int(1), P_E_Int(1), q_low, p_low, q_full, p_full);
                        goto SpeedLoop3_exit;
                    }
                    // Case 2B:
                } else {
                    // Case 3:
                    // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
                    std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                        IntermediateCapacityAndPowerSEER2Case3(q_full, p_full, CoolingCapacityMax_2023, CoolingElecPowerMax_2023, n);
                    goto SpeedLoop3_exit;
                }
            }
        SpeedLoop3_exit:;
            NetCoolingCapWeighted2_2023 += NetTotCoolCapBinned_2023;
            TotCoolingElecPowerWeighted2_2023 += TotCoolElecPowerBinned_2023;

            q_sum += q;
            e_sum += e;
        }
        if (e_sum > 0.0) {
            SEER2_User = NetCoolingCapWeighted2_2023 / TotCoolingElecPowerWeighted2_2023;
            SEER2_Standard = q_sum / e_sum; // Equation 11.66 (AHRI-2023)
        }
        return std::make_tuple(NetCoolingCapRatedMaxSpeed2023, SEER2_User, SEER2_Standard, EER2);
    }

    std::tuple<Real64, Real64, Real64, Real64> TwoSpeedDXCoolingCoilSEER2(EnergyPlusData &state,
                                                                          // int const nsp,
                                                                          Array1A_int const CapFFlowCurveIndex,
                                                                          Array1A<Real64> const RatedTotalCapacity,
                                                                          Array1A_int const CapFTempCurveIndex,
                                                                          Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                          Array1A<Real64> const RatedAirVolFlowRate,
                                                                          Array1A_int const EIRFFlowCurveIndex,
                                                                          Array1A<Real64> const RatedCOP,
                                                                          Array1A_int EIRFTempCurveIndex,
                                                                          Array1A_int const PLFFPLRCurveIndex)
    {
        int nsp = 2;
        Real64 NetCoolingCapRatedMaxSpeed2023 = 0.0;
        Real64 SEER2_User = 0.0;
        Real64 SEER2_Standard = 0.0;
        Real64 EER2 = 0.0;

        Real64 constexpr SF(1.10); // Sizing Factor as per AHRI Std 210/240-2023 | equation 11.68
        Real64 constexpr V(1);     // V = 0.93 for Variable Speed Heat Pumps, otherwise V = 1.0

        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        Array1D<Real64> Q_A_Full(nsp);                        // Total cooling capacity at A2 test condition (High speed) | q_A_Full
        Array1D<Real64> Q_B_Full(nsp);                        // Total cooling capacity at B2 test condition (High speed) | q_B_Full
        Array1D<Real64> Q_B_Low(nsp);                         // Total cooling capacity at B1 test condition (Low speed) | q_B_Low
        Array1D<Real64> Q_F_Low(nsp);                         // Total cooling capacity at F1 test condition (Low speed) | q_F_Low
        Array1D<Real64> P_A_Full(nsp);                        // Outdoor Unit electric power at A2 test condition (High speed) | p_A_Full
        Array1D<Real64> P_B_Full(nsp);                        // Outdoor Unit electric power at B2 test condition (High speed) | p_B_Full
        Array1D<Real64> P_B_Low(nsp);                         // Outdoor Unit electric power at B1 test condition (Low speed) | p_B_Low
        Array1D<Real64> P_F_Low(nsp);                         // Outdoor Unit electric power at F1 test condition | p_F_Low

        Real64 PartLoadFactorUser_2023(0.0); // part-load factor based on user-input PLF curve and C_D value that accounts
        // for the cyclic degradation, [-]

        // binned cooling hours
        Real64 BuildingCoolingLoad_2023(0.0);    // Building space cooling load corresponding to an outdoor bin temperature [W]
        Real64 NetTotCoolCapBinned_2023(0.0);    // Net tot cooling cap corresponding to an outdoor bin temperature [W]
        Real64 TotCoolElecPowerBinned_2023(0.0); // Total cooling electric power corresponding to an outdoor bin temperature [W]

        Array1D<Real64> TotCapFlowModFac(nsp);        // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);           // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Array1D<Real64> NetCoolingCapRated_2023(nsp); // net cooling capacity at each speed

        Real64 FanPowerPerEvapAirFlowRate_2023_LS(0.0);
        Real64 FanPowerPerEvapAirFlowRate_2023_HS(0.0);
        // Low Stage
        if (FanPowerPerEvapAirFlowRateFromInput_2023(2) <= 0.0) {
            FanPowerPerEvapAirFlowRate_2023_LS = DefaultFanPowerPerEvapAirFlowRateSEER2;
            FanPowerPerEvapAirFlowRate_2023(1) = DefaultFanPowerPerEvapAirFlowRateSEER2;
        } else {
            FanPowerPerEvapAirFlowRate_2023_LS = FanPowerPerEvapAirFlowRateFromInput_2023(2);
            FanPowerPerEvapAirFlowRate_2023(1) = FanPowerPerEvapAirFlowRateFromInput_2023(2);
        }
        // High Stage
        if (FanPowerPerEvapAirFlowRateFromInput_2023(1) <= 0.0) {
            FanPowerPerEvapAirFlowRate_2023_HS = DefaultFanPowerPerEvapAirFlowRateSEER2;
            FanPowerPerEvapAirFlowRate_2023(2) = DefaultFanPowerPerEvapAirFlowRateSEER2;
        } else {
            FanPowerPerEvapAirFlowRate_2023_HS = FanPowerPerEvapAirFlowRateFromInput_2023(1);
            FanPowerPerEvapAirFlowRate_2023(2) = FanPowerPerEvapAirFlowRateFromInput_2023(1);
        }

        // Calculate the capacity and power for each speed
        // Low Stage
        TotCapFlowModFac(1) = 1; // As per IO Reference there are no CCapFFlowCurve for Low Speed | Section ??
        EIRFlowModFac(1) = 1;    // As per IO Reference there are no EIRFFlowCurve for Low Speed | Section ??

        Q_A_Full(1) = RatedTotalCapacity(2) *
                          Curve::CurveValue(state, CapFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                          TotCapFlowModFac(1) -
                      FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

        Q_B_Full(1) = RatedTotalCapacity(2) *
                          Curve::CurveValue(state, CapFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) *
                          TotCapFlowModFac(1) -
                      FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

        Q_B_Low(1) = RatedTotalCapacity(2) *
                         Curve::CurveValue(state, CapFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) *
                         TotCapFlowModFac(1) -
                     FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

        Q_F_Low(1) = RatedTotalCapacity(2) *
                         Curve::CurveValue(state, CapFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) *
                         TotCapFlowModFac(1) -
                     FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

        if (RatedCOP(2) > 0.0) {
            P_A_Full(1) =
                Q_A_Full(1) * EIRFlowModFac(1) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) /
                    RatedCOP(2) +
                FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

            P_B_Full(1) =
                Q_B_Full(1) * EIRFlowModFac(1) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) /
                    RatedCOP(2) +
                FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

            P_B_Low(1) =
                Q_B_Low(1) * EIRFlowModFac(1) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) /
                    RatedCOP(2) +
                FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);

            P_F_Low(1) =
                Q_F_Low(1) * EIRFlowModFac(1) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(2), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) /
                    RatedCOP(2) +
                FanPowerPerEvapAirFlowRate_2023(1) * RatedAirVolFlowRate(2);
        }
        // High Stage
        TotCapFlowModFac(2) = Curve::CurveValue(state, CapFFlowCurveIndex(1), AirMassFlowRatioRated);
        EIRFlowModFac(2) = Curve::CurveValue(state, EIRFFlowCurveIndex(1), AirMassFlowRatioRated);

        Q_A_Full(2) = RatedTotalCapacity(1) *
                          Curve::CurveValue(state, CapFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                          TotCapFlowModFac(2) -
                      FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

        Q_B_Full(2) = RatedTotalCapacity(1) *
                          Curve::CurveValue(state, CapFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) *
                          TotCapFlowModFac(2) -
                      FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

        Q_B_Low(2) = RatedTotalCapacity(1) *
                         Curve::CurveValue(state, CapFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) *
                         TotCapFlowModFac(2) -
                     FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

        Q_F_Low(2) = RatedTotalCapacity(1) *
                         Curve::CurveValue(state, CapFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) *
                         TotCapFlowModFac(2) -
                     FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

        if (RatedCOP(1) > 0.0) {
            P_A_Full(2) =
                Q_A_Full(2) * EIRFlowModFac(2) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) /
                    RatedCOP(1) +
                FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

            P_B_Full(2) =
                Q_B_Full(2) * EIRFlowModFac(2) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) /
                    RatedCOP(1) +
                FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

            P_B_Low(2) =
                Q_B_Low(2) * EIRFlowModFac(2) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) /
                    RatedCOP(1) +
                FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);

            P_F_Low(2) =
                Q_F_Low(2) * EIRFlowModFac(2) *
                    Curve::CurveValue(state, EIRFTempCurveIndex(1), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) /
                    RatedCOP(1) +
                FanPowerPerEvapAirFlowRate_2023(2) * RatedAirVolFlowRate(1);
        }

        // Standard Rating cooling (net) capacity calculations:
        NetCoolingCapRated_2023(2) = Q_A_Full(2);
        NetCoolingCapRatedMaxSpeed2023 = NetCoolingCapRated_2023(2);

        // EER2 calculation |  Section 3.1.16(AHRI 210/240 2023)
        // A ratio of the cooling capacity in Btu/h to the Total Power in watts at AFull test conditions and expressed in Btu/(W-h)
        // In case of Coil:Cooling:DX:TwoSpeed coil we're picking the High Speed for the same.
        EER2 = Q_A_Full(2) / P_A_Full(2);

        // Calculate the SEER value based on contribution of each outdoor air bin temperature
        Real64 q_sum = 0.0;
        Real64 e_sum = 0.0;
        Real64 NetCoolingCapWeighted2_2023 = 0.0;
        Real64 TotCoolingElecPowerWeighted2_2023 = 0.0;

        Real64 CoolingCapacityMax_2023(0.0);
        Real64 CoolingElecPowerMax_2023(0.0);

        Real64 q_low(0.0);
        Real64 p_low(0.0);
        Real64 q_full(0.0);
        Real64 p_full(0.0);

        q_sum = 0.0;
        e_sum = 0.0;
        NetCoolingCapWeighted2_2023 = 0.0;
        TotCoolingElecPowerWeighted2_2023 = 0.0;

        for (int BN = 0; BN < NumOfOATempBins; ++BN) {
            // Equation 11.67 (AHRI-2023)
            BuildingCoolingLoad_2023 = ((OutdoorBinTemperatureSEER[BN] - 18.3) / (35.0 - 18.3) * (Q_A_Full(nsp) / SF)) * V;

            // determine the speed number
            CoolingCapacityMax_2023 =
                Q_B_Full(nsp) + ((Q_A_Full(nsp) - Q_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            CoolingElecPowerMax_2023 =
                P_B_Full(nsp) + ((P_A_Full(nsp) - P_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);

            // LOW STAGE :
            // The calculated Low Stage system capacity rate at each bin temperature shall be calculated by Equation 11.69
            // Equation 11.69 (AHRI-2023)
            q_low = Q_F_Low(1) + ((Q_B_Low(1) - Q_F_Low(1)) / (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                     (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestF1);

            // The calculated Low Stage energy consumption at each bin temperature shall be calculated by Equation 11.70
            // Equation 11.70 (AHRI-2023)
            p_low = P_F_Low(1) + ((P_B_Low(1) - P_F_Low(1)) / (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                     (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestF1);

            // HIGH STAGE :
            // The calculated Full Stage system capacity at each bin temperature shall be calculated by Equation 11.71.
            // Equation 11.71 (AHRI-2023)
            q_full = Q_B_Full(2) + ((Q_A_Full(2) - Q_B_Full(2)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                       (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);

            // The calculated Full Stage energy consumption at each bin temperature shall be calculated by Equation 11.72.
            // Equation 11.72 (AHRI-2023)
            p_full = P_B_Full(2) + ((P_A_Full(2) - P_B_Full(2)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                       (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);

            Real64 bl = BuildingCoolingLoad_2023;
            // Real64 t = OutdoorBinTemperatureSEER[BN];
            Real64 n = CoolFracBinHoursAtOutdoorBinTemp[BN];
            Real64 q(0.0);
            Real64 e(0.0);
            // Section 6.1.3.1.2 | For Two-capacity Systems, if the optional CLow and DLow tests are not performed, a default value of 0.20 shall
            // be used for the Low Stage cooling Degradation Coefficient, cd_low. In this case, if using default value for cd_low, use default
            // value for cd_full. For Two-capacity Systems that lock out low capacity operation at high outdoor temperatures, if the optional
            // CFull and DFull tests are not performed, the default value for Full Stage shall be the value used for Low Stage.
            Real64 CyclicDegradationCoefficient(0.20);
            if (bl < q_low) {
                // Case I. Building load is less than Low Stage capacity, BL(tj) < qLow(tj). Calculate total bin capacity by using Equation 11.73
                // and total bin energy by using Equation 11.74.
                Real64 clf_low = bl / q_low; // Equation 11.75 (AHRI-2023)

                // SEER2 USER
                PartLoadFactorUser_2023 = Curve::CurveValue(state, PLFFPLRCurveIndex(1), clf_low);
                NetTotCoolCapBinned_2023 = clf_low * q_low * CoolFracBinHoursAtOutdoorBinTemp[BN];
                TotCoolElecPowerBinned_2023 = (clf_low / PartLoadFactorUser_2023) * p_low * CoolFracBinHoursAtOutdoorBinTemp[BN];

                // SEER2 STANDARD
                Real64 plf_low = 1.0 - CyclicDegradationCoefficient * (1.0 - clf_low); // Equation 11.76 (AHRI-2023)
                q = clf_low * q_low * n;                                               // Total Bin Capacity, Equation 11.73 (AHRI-2023)
                e = clf_low * p_low * n / plf_low;                                     // Total Bin Energy, Equation 11.74 (AHRI-2023)

            } else if (q_low < bl && bl < q_full) {
                // Case II. Building load is greater than the Low Stage capacity, but less than the Full Stage capacity, qLow(tj) < BL(tj) <
                // qFull(tj) and the unit cycles between "Low Stage" operation and "Full Stage" operation. Calculate total bin capacity by using
                // Equation 11.79 and total bin energy by using Equation 11.80

                // Prerequisites for Equations 11.79 & 11.80
                Real64 clf_low_c2 = (q_full - bl) / (q_full - q_low); // Equation 11.81 (AHRI-2023)
                Real64 clf_full_c2 = 1 - clf_low_c2;                  // Equation 11.82 (AHRI-2023)

                // SEER2 USER
                // Real64 LoadFactor_Low_2023 = min(1.0, (Q_E_Int(spnum) - q_low) / (q_full - q_low));
                // LoadFactorQEnt_2023 = max(0.0, LoadFactorQEnt_2023);
                // TODO:BPS : Visit again to figure out if there is a way to leverage Coil Data/Curve to get this.
                Real64 NetTotCoolCapBinned_2023_c2 = ((clf_low_c2 * q_low) + (clf_full_c2 * q_full)) * CoolFracBinHoursAtOutdoorBinTemp[BN];
                Real64 TotCoolElecPowerBinned_2023_c2 = ((clf_low_c2 * p_low) + (clf_full_c2 * p_full)) * CoolFracBinHoursAtOutdoorBinTemp[BN];

                // SEER2 STANDARD
                Real64 q_c2 = ((clf_low_c2 * q_low) + (clf_full_c2 * q_full)) * n; // Total Bin Capacity, Equation 11.79 (AHRI-2023)
                Real64 e_c2 = ((clf_low_c2 * p_low) + (clf_full_c2 * p_full)) * n; // Total Bin Energy, Equation 11.80 (AHRI-2023)

                // Case III. Building load is greater than the Low Stage capacity, but less than the Full Stage capacity, qLow(tj) < BL(tj) <
                // qFull(tj) and the unit cycles between "off" and "Full Stage" operation.Calculate total bin capacity by using Equation 11.83 and
                // total bin energy by using Equation 11.84

                // Prerequisites for Equations 11.83 & 11.84
                // If the optional c_full and d_full Tests (see Table 7 AHRI-2023) are not conducted, set ccd_full equal to the lower of a) the
                // ccd_low value calculated as per Equation 11.77 or b) the default value identified in Section 6.1.3.1
                // Real64 ccd_full_c3 = CyclicDegradationCoefficient;
                // Real64 clf_full_c3 = bl / q_full; // Equation 11.85 (AHRI-2023)

                // SEER2 USER
                // Real64 PartLoadFactorUser_2023_c3 = Curve::CurveValue(state, PLFFPLRCurveIndex(1), clf_full_c3);
                // Real64 NetTotCoolCapBinned_2023_c3 = clf_full_c3 * q_full * CoolFracBinHoursAtOutdoorBinTemp[BN];
                // Real64 TotCoolElecPowerBinned_2023_c3 = (clf_full_c3 / PartLoadFactorUser_2023_c3) * p_full * CoolFracBinHoursAtOutdoorBinTemp[BN];

                // The code below calculates coil Capacity and Energy for the case when a coil support for 'locked out'
                // of the low stage on the compressor when outdoor air is very hot.  In this case, the compressor will cycle
                // directly from off to the High Stage, bypassing the low stage.
                // EnergyPlus DX Cooling Coil data does not include a property for indicating if the coil supports this behavior,
                // so these values are not currently used.

                // SEER2 STANDARD
                // Real64 plf_full_c3 = 1 - (ccd_full_c3 * (1 - clf_full_c3)); // Equation 11.86 (AHRI-2023)
                // Real64 q_c3 = clf_full_c3 * q_full * n;                     // Total Bin Capacity, Equation  11.83 (AHRI-2023)
                // Real64 e_c3 = (clf_full_c3 * p_full * n) / plf_full_c3;     // Total Bin Energy, Equation 11.84 (AHRI-2023)

                // SEER2 USER
                NetTotCoolCapBinned_2023 = NetTotCoolCapBinned_2023_c2;
                TotCoolElecPowerBinned_2023 = TotCoolElecPowerBinned_2023_c2;
                // SEER2 STANDARD
                q = q_c2;
                e = e_c2;

            } else if (bl >= q_full) {
                // Case IV. Building load is greater than or equal to the unit capacity, BL(tj) >= qFull(tj).Calculate total bin capacity by using
                // Equation 11.88 and total bin energy by using Equation 11.89.
                // Section 11.2.1.3.3 CASE 4 - Building load is equal to or greater than unit capacity at full stage

                // SEER2 USER
                NetTotCoolCapBinned_2023 = CoolingCapacityMax_2023 * CoolFracBinHoursAtOutdoorBinTemp[BN];
                TotCoolElecPowerBinned_2023 = CoolingElecPowerMax_2023 * CoolFracBinHoursAtOutdoorBinTemp[BN];

                // SEER2 STANDARD
                q = q_full * n; // Equation 11.88 (AHRI-2023)
                e = p_full * n; // Equation 11.89 (AHRI-2023)
            }

            // SEER2 USER | Sum up Bin Capacity and Bin Energy
            NetCoolingCapWeighted2_2023 += NetTotCoolCapBinned_2023;
            TotCoolingElecPowerWeighted2_2023 += TotCoolElecPowerBinned_2023;

            // SEER2 STANDARD | Sum up Bin Capacity and Bin Energy
            q_sum += q;
            e_sum += e;
        }

        SEER2_User = 0.0;
        SEER2_Standard = 0.0;
        if (e_sum > 0.0) {
            SEER2_User = NetCoolingCapWeighted2_2023 / TotCoolingElecPowerWeighted2_2023;
            SEER2_Standard = q_sum / e_sum; // Equation 11.66 (AHRI-2023)
        }

        return std::make_tuple(NetCoolingCapRatedMaxSpeed2023, SEER2_User, SEER2_Standard, EER2);
    }

    std::tuple<Real64, Real64, Real64, Real64> MultiSpeedDXCoolingCoilSEER2(EnergyPlusData &state,
                                                                            int const nsp,
                                                                            Array1A_int const CapFFlowCurveIndex,
                                                                            Array1A<Real64> const RatedTotalCapacity,
                                                                            Array1A_int const CapFTempCurveIndex,
                                                                            Array1A<Real64> const FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                            Array1A<Real64> const RatedAirVolFlowRate,
                                                                            Array1A_int const EIRFFlowCurveIndex,
                                                                            Array1A<Real64> const RatedCOP,
                                                                            Array1A_int EIRFTempCurveIndex,
                                                                            Array1A_int const PLFFPLRCurveIndex)
    {
        // Intermediate values calculated from the inputs in the idf file
        // ANSI/AHRI 210/240 Std. 2023
        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // 2023 Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        Array1D<Real64> Q_A_Full(nsp);                        // Total cooling capacity at A2 test condition (High speed) | q_A_Full
        Array1D<Real64> Q_B_Full(nsp);                        // Total cooling capacity at B2 test condition (High speed) | q_B_Full
        Array1D<Real64> Q_B_Low(nsp);                         // Total cooling capacity at B1 test condition (Low speed) | q_B_Low
        Array1D<Real64> Q_F_Low(nsp);                         // Total cooling capacity at F1 test condition (Low speed) | q_F_Low
        Array1D<Real64> Q_E_Int(nsp);                         // Total cooling capacity at Eint (Ev) test condition | q_E_Int
        Array1D<Real64> P_A_Full(nsp);                        // Outdoor Unit electric power at A2 test condition (High speed) | p_A_Full
        Array1D<Real64> P_B_Full(nsp);                        // Outdoor Unit electric power at B2 test condition (High speed) | p_B_Full
        Array1D<Real64> P_B_Low(nsp);                         // Outdoor Unit electric power at B1 test condition (Low speed) | p_B_Low
        Array1D<Real64> P_F_Low(nsp);                         // Outdoor Unit electric power at F1 test condition | p_F_Low
        Array1D<Real64> P_E_Int(nsp);                         // Outdoor Unit electric power at Eint (Ev) test conditon | p_E_Int

        // part-load factor based on user-input PLF curve and C_D value that accounts for the cyclic degradation, [-]
        // Real64 PartLoadFactorUser_2023(0.0);

        Real64 NetCoolingCapWeighted_2023(0.0);         // net tot cooling cap weighted by the fraction of the binned cooling hours [W]
        Real64 TotCoolingElecPowerWeighted_2023(0.0);   // net total cooling electric power input weighted by the fraction of the temperature bins
        Real64 TotCoolingElecPowerWeightedDefault(0.0); // net total cooling electric power input weighted by the fraction of the temperature bins
                                                        // from AHRI 201/240 default PLF curve and C_D value,

        // binned cooling hours
        Real64 BuildingCoolingLoad_2023(0.0);    // Building space cooling load corresponding to an outdoor bin temperature [W]
        Real64 NetTotCoolCapBinned_2023(0.0);    // Net tot cooling cap corresponding to an outdoor bin temperature [W]
        Real64 TotCoolElecPowerBinned_2023(0.0); // Total cooling electric power corresponding to an outdoor bin temperature [W]

        Array1D<Real64> TotCapFlowModFac(nsp);        // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);           // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
        Array1D<Real64> NetCoolingCapRated_2023(nsp); // net cooling capacity at each speed
        Real64 q_low(0.0);                            // cooling capacity of Mult-speed DX coil at lower speed, [W]
        Real64 q_full(0.0);                           // cooling capacity of Mult-speed DX coil at higher speed, [W]
        Real64 p_low(0.0);                            // outdoor unit electric power input at low speed, [W]
        Real64 p_full(0.0);                           // outdoor unit electric power input at high speed, [W]
        Real64 CoolingCapacityMax_2023(0.0);          // cooling capacity of Mult-speed DX coil at max speed, [W]
        Real64 CoolingElecPowerMax_2023(0.0);         // outdoor unit electric power input at Max speed, [W]

        Real64 constexpr SF(1.10); // Sizing Factor as per AHRI Std 210/240-2023 | equation 11.68
        // Real64 constexpr V(1);     // V = 0.93 for Variable Speed Heat Pumps, otherwise V = 1.0
        Real64 constexpr V(0.93); // V = 0.93 for Variable Speed Heat Pumps, otherwise V = 1.0

        Real64 NetCoolingCapRatedMaxSpeed2023 = 0.0;
        Real64 SEER2_User = 0.0;
        Real64 SEER2_Standard = 0.0;
        Real64 EER2 = 0.0;

        NetCoolingCapWeighted_2023 = 0.0;
        TotCoolingElecPowerWeighted_2023 = 0.0;
        TotCoolingElecPowerWeightedDefault = 0.0;

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate_2023(spnum) = 0.0;
            if (FanPowerPerEvapAirFlowRateFromInput_2023(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
            } else {
                FanPowerPerEvapAirFlowRate_2023(spnum) = FanPowerPerEvapAirFlowRateFromInput_2023(spnum);
            }
        }

        // Calculate the capacity and power for each speed
        for (int spnum = 1; spnum <= nsp; ++spnum) {
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            Q_A_Full(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            Q_B_Full(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            Q_B_Low(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            Q_F_Low(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            Q_E_Int(spnum) =
                RatedTotalCapacity(spnum) *
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestEint) *
                    TotCapFlowModFac(spnum) -
                FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            if (RatedCOP(spnum) > 0.0) {
                P_A_Full(spnum) =
                    Q_A_Full(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

                P_B_Full(spnum) =
                    Q_B_Full(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB2) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

                P_B_Low(spnum) = Q_B_Low(spnum) * EIRFlowModFac(spnum) *
                                     Curve::CurveValue(
                                         state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestB1) /
                                     RatedCOP(spnum) +
                                 FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

                P_F_Low(spnum) = Q_F_Low(spnum) * EIRFlowModFac(spnum) *
                                     Curve::CurveValue(
                                         state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestF1) /
                                     RatedCOP(spnum) +
                                 FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

                P_E_Int(spnum) =
                    Q_E_Int(spnum) * EIRFlowModFac(spnum) *
                        Curve::CurveValue(
                            state, EIRFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestEint) /
                        RatedCOP(spnum) +
                    FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            }
        }
        // Standard Rating cooling (net) capacity calculations:
        NetCoolingCapRated_2023(nsp) = Q_A_Full(nsp);
        NetCoolingCapRatedMaxSpeed2023 = NetCoolingCapRated_2023(nsp);

        // EER2 calculation |  Section 3.1.16(AHRI 210/240 2023)
        // A ratio of the cooling capacity in Btu/h to the Total Power in watts at AFull test conditions and expressed in Btu/(W-h)
        // In case of Coil:Cooling:DX:MultiSpeed coil we're picking the max speed.
        EER2 = Q_A_Full(nsp) / P_A_Full(nsp);

        // Calculate the SEER value based on contribution of each outdoor air bin temperature
        Real64 q_sum = 0.0;
        Real64 e_sum = 0.0;
        Real64 NetCoolingCapWeighted2_2023 = 0.0;
        Real64 TotCoolingElecPowerWeighted2_2023 = 0.0;
        for (int BN = 0; BN < NumOfOATempBins; ++BN) {
            // Equation 11.67 (AHRI-2023)
            BuildingCoolingLoad_2023 = ((OutdoorBinTemperatureSEER[BN] - 18.3) / (35.0 - 18.3) * (Q_A_Full(nsp) / SF)) * V;
            // determine the speed number
            CoolingCapacityMax_2023 =
                Q_B_Full(nsp) + ((Q_A_Full(nsp) - Q_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            CoolingElecPowerMax_2023 =
                P_B_Full(nsp) + ((P_A_Full(nsp) - P_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);

            // Equation 11.69 (AHRI-2023)
            q_low = Q_F_Low(1) + ((Q_B_Low(1) - Q_F_Low(1)) / (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                     (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestF1);
            // Equation 11.70 (AHRI-2023)
            p_low = P_F_Low(1) + ((P_B_Low(1) - P_F_Low(1)) / (OutdoorCoilInletAirDryBulbTempTestB1 - OutdoorCoilInletAirDryBulbTempTestF1)) *
                                     (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestF1);
            // Equation 11.71 (AHRI-2023)
            q_full =
                Q_B_Full(nsp) + ((Q_A_Full(nsp) - Q_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);
            // Equation 11.72 (AHRI-2023)
            p_full =
                P_B_Full(nsp) + ((P_A_Full(nsp) - P_B_Full(nsp)) / (OutdoorCoilInletAirDryBulbTempTestA2 - OutdoorCoilInletAirDryBulbTempTestB2)) *
                                    (OutdoorBinTemperatureSEER[BN] - OutdoorCoilInletAirDryBulbTempTestB2);

            Real64 q(0.0);
            Real64 e(0.0);
            Real64 N_Cq(0.0);
            Real64 M_Cq(0.0);
            Real64 N_CE(0.0);
            Real64 M_CE(0.0);
            Real64 q_int(0.0);
            Real64 p_int(0.0);
            Real64 t = OutdoorBinTemperatureSEER[BN];
            Real64 n = CoolFracBinHoursAtOutdoorBinTemp[BN];
            Real64 bl = BuildingCoolingLoad_2023;
            for (int spnum = 1; spnum <= nsp; ++spnum) {
                // # Intermediate Capacity
                Real64 q_A_full = Q_A_Full(spnum);
                Real64 q_B_full = Q_B_Full(spnum);
                Real64 q_B_low = Q_B_Low(spnum);
                Real64 q_F_low = Q_F_Low(spnum);
                Real64 q_E_int = Q_E_Int(spnum);

                std::tie(N_Cq, M_Cq) = CapacityAdjustmentFactorsInCoolingModeSEER2(q_F_low, q_B_low, BN, q_B_full, q_A_full, q_E_int);

                // # Intermediate Power
                Real64 p_A_full = P_A_Full(spnum);
                Real64 p_B_full = P_B_Full(spnum);
                Real64 p_B_low = P_B_Low(spnum);
                Real64 p_F_low = P_F_Low(spnum);
                Real64 p_E_int = P_E_Int(spnum);

                std::tie(N_CE, M_CE) = EnergyAdjustmentFactorsInCoolingModeSEER2(p_F_low, p_B_low, BN, p_B_full, p_A_full, p_E_int);

                std::tie(q_int, p_int) = IntermediateSteadyStateCpacityAndPowerSEER2(q_E_int, M_Cq, p_E_int, M_CE, t);

                // Section 11.2.1.3.1 CASE 1 - Building load is no greater than unit capacity at low speed.
                if (bl <= q_low) {
                    std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                        IntermediateCapacityAndPowerSEER2Case1(state, bl, q_low, n, p_low, PLFFPLRCurveIndex(spnum));
                    // This is the case and speed we're looking for now we exit and try calculating against the next bin
                    goto SpeedLoop3_exit;
                } else {
                    //
                    if (spnum < nsp - 1) {
                        // As part of our new experiment if the first case is not satisfied then we'll go and try the next speed
                        // instead of going to the next case.
                    } else {
                        // if we're here then all the speeds (apart from max speed) failed to satisfy the first case.
                        // Now we've to try second case.
                        if (bl < q_int) {
                            // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed & full
                            // Speed
                            std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2A(
                                p_int, q_int, q_low, bl, n, Q_E_Int(spnum), q_full, P_E_Int(spnum), p_full, p_low);
                            goto SpeedLoop3_exit;
                        } else {
                            // if we're here then all the speeds (apart from max speed) failed to staisfy the case 2A
                            if (bl < q_full) {
                                // Section 11.2.1.3.2 CASE 2 - Building load can be matched by modulating the compressor speed between low speed &
                                // full Speed
                                std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) = IntermediateCapacityAndPowerSEER2Case2B(
                                    p_int, bl, q_int, n, Q_E_Int(spnum), P_E_Int(spnum), q_low, p_low, q_full, p_full);
                                goto SpeedLoop3_exit;
                            } else {
                                // if we're here then all the speeds (apart form max speed ?? ) failed to staisfy the case 2B
                                // max speed should include in cases 1,2A,2B or not ?? TBD:

                                if (spnum == nsp - 1) {
                                    // Case 4 if applicable for nsp-1 then we're skipping it to nsp( max speed )
                                } else {
                                    // Section 11.2.1.3.3 CASE 3 - Building load is equal to or greater than unit capacity at full stage
                                    std::tie(q, e, NetTotCoolCapBinned_2023, TotCoolElecPowerBinned_2023) =
                                        IntermediateCapacityAndPowerSEER2Case3(q_full, p_full, CoolingCapacityMax_2023, CoolingElecPowerMax_2023, n);
                                    goto SpeedLoop3_exit;
                                }
                            }
                        }
                    }
                }
            }
        SpeedLoop3_exit:;
            NetCoolingCapWeighted2_2023 += NetTotCoolCapBinned_2023;
            TotCoolingElecPowerWeighted2_2023 += TotCoolElecPowerBinned_2023;

            q_sum += q;
            e_sum += e;
        }
        SEER2_User = 0.0;
        SEER2_Standard = 0.0;
        if (e_sum > 0.0) {
            SEER2_User = NetCoolingCapWeighted2_2023 / TotCoolingElecPowerWeighted2_2023;
            SEER2_Standard = q_sum / e_sum; // Equation 11.66 (AHRI-2023)
        }

        return std::make_tuple(NetCoolingCapRatedMaxSpeed2023, SEER2_User, SEER2_Standard, EER2);
    }

    std::map<std::string, Real64> MultiSpeedDXCoolingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilType,                                  // Type of DX coil for which HSPF is calculated
        [[maybe_unused]] std::string const &DXCoilName,                 // Type of DX coil for which standard Ratings are calculated
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
        Array1D<DataHeatBalance::RefrigCondenserType> const &CondenserType)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC
        //       DATE WRITTEN   December 2012
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the standard ratings net cooling capacity and SEER values for multi speed DX cooling coils
        // at the AHRI standard test condition(s).

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using Curve::CurveValue;

        // Argument array dimensioning
        CapFTempCurveIndex.dim(nsp);
        CapFFlowCurveIndex.dim(nsp);
        EIRFTempCurveIndex.dim(nsp);
        EIRFFlowCurveIndex.dim(nsp);
        PLFFPLRCurveIndex.dim(nsp);
        RatedTotalCapacity.dim(nsp);
        RatedCOP.dim(nsp);
        RatedAirVolFlowRate.dim(nsp);
        FanPowerPerEvapAirFlowRateFromInput.dim(nsp);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // CHARACTER(len=*), PARAMETER    :: RoutineName='MultiSpeedDXCoolingCoilStandardRatings: ' ! Include trailing blank space
        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // Intermediate values calculated from the inputs in the idf file

        Real64 NetCoolingCapRatedMaxSpeed(0.0); // net cooling capacity at maximum speed
        Real64 SEER_User(0.0);     // seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 SEER_Standard(0.0); // seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2008 default PLF
                                   // curve and
                                   // C_D value
        Real64 EER(0.0);

        // Ratings Based on ANSI/AHRI 210/140
        Real64 NetCoolingCapRatedMaxSpeed2023(0.0); // net cooling capacity at maximum speed
        Real64 SEER2_User(0.0);     // seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 SEER2_Standard(0.0); // seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2008 default PLF
                                    // curve and
                                    // C_D value
        Real64 EER2(0.0);

        // IEER Calculation 2022
        Real64 IEER_2022(0.0);
        Real64 NetCoolingCapRated2023(0.0); // ?? for which speed (NetCoolingCapRatedMaxSpeed2023)
        Real64 EER_2022(0.0);
        std::map<std::string, Real64> StandardRatingsResult;
        // StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] = NetCoolingCapRatedMaxSpeed;
        // StandardRatingsResult["SEER_User"] = SEER_User;
        // StandardRatingsResult["SEER_Standard"] = SEER_Standard;
        // StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRatedMaxSpeed2023;
        // StandardRatingsResult["SEER2_User"] = SEER2_User;
        // StandardRatingsResult["SEER2_Standard"] = SEER2_Standard;
        Real64 TotCapFlowModFac = Curve::CurveValue(state, CapFFlowCurveIndex(nsp), AirMassFlowRatioRated);
        NetCoolingCapRatedMaxSpeed =
            RatedTotalCapacity(nsp) *
                Curve::CurveValue(state, CapFTempCurveIndex(nsp), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                TotCapFlowModFac -
            FanPowerPerEvapAirFlowRateFromInput(nsp) * RatedAirVolFlowRate(nsp);
        if (RatedTotalCapacity(nsp) > 0.0 && RatedAirVolFlowRate(nsp) > 0.0) {

            NetCoolingCapRated2023 =
                RatedTotalCapacity(nsp) *
                    Curve::CurveValue(state, CapFTempCurveIndex(nsp), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                    TotCapFlowModFac -
                FanPowerPerEvapAirFlowRateFromInput_2023(nsp) * RatedAirVolFlowRate(nsp);
            StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRated2023;
            // TODO: Commercial and industrial unitary air-conditioning condensing units with a capacity greater than 135,000 Btu/h (39564.59445
            // Watts) as defined in ANSI/AHRI Standard 365(I-P). | Scope 2.2.6 (ANSI/AHRI 340-360 2022)

            std::tie(NetCoolingCapRatedMaxSpeed, SEER_User, SEER_Standard, EER) = MultiSpeedDXCoolingCoilSEER(state,
                                                                                                              nsp,
                                                                                                              CapFFlowCurveIndex,
                                                                                                              RatedTotalCapacity,
                                                                                                              CapFTempCurveIndex,
                                                                                                              FanPowerPerEvapAirFlowRateFromInput,
                                                                                                              RatedAirVolFlowRate,
                                                                                                              EIRFFlowCurveIndex,
                                                                                                              RatedCOP,
                                                                                                              EIRFTempCurveIndex,
                                                                                                              PLFFPLRCurveIndex);

            if (CondenserType(1) == DataHeatBalance::RefrigCondenserType::Air) {
                // SEER2 standard applies to factory-made Unitary Air-conditioners and Unitary Air-source Heat Pumps with
                // capacities less than 65,000 Btu/h (19049.61955 Watts) | Section 2.1 (ANSI/AHRI 210-240 2023)
                // Removal of water-cooled and evaporatively-cooled products from the scope | Foreword (ANSI/AHRI 210-240 2023)
                //
                // SEER2 Calculations ANSI/AHRI 210/240 Standard 2023
                std::tie(NetCoolingCapRatedMaxSpeed2023, SEER2_User, SEER2_Standard, EER2) =
                    MultiSpeedDXCoolingCoilSEER2(state,
                                                 nsp,
                                                 CapFFlowCurveIndex,
                                                 RatedTotalCapacity,
                                                 CapFTempCurveIndex,
                                                 FanPowerPerEvapAirFlowRateFromInput_2023,
                                                 RatedAirVolFlowRate,
                                                 EIRFFlowCurveIndex,
                                                 RatedCOP,
                                                 EIRFTempCurveIndex,
                                                 PLFFPLRCurveIndex);
                StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRatedMaxSpeed2023;
            }
            // Gross total cooling capacity is greater than 65,000 Btu/h (19049.61955 Watts)
            // Section 2.1 (ANSI/AHRI 210-240 2023)
            std::tie(IEER_2022, NetCoolingCapRated2023, EER_2022) = IEERCalculationMultiSpeed(state,
                                                                                              DXCoilType,
                                                                                              nsp,
                                                                                              CapFTempCurveIndex,
                                                                                              RatedTotalCapacity,
                                                                                              CapFFlowCurveIndex,
                                                                                              // TotCapFlowModFac, // calculate for each speed
                                                                                              FanPowerPerEvapAirFlowRateFromInput_2023,
                                                                                              RatedAirVolFlowRate,
                                                                                              EIRFTempCurveIndex,
                                                                                              RatedCOP,
                                                                                              EIRFFlowCurveIndex,
                                                                                              // EIRFlowModFac, // calculate for each speed
                                                                                              CondenserType);
            StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRated2023;

        } else {
            ShowSevereError(state,
                            "Standard Ratings: Coil:Cooling:DX:MultiSpeed has eiher zero rated total cooling capacity or zero rated air vol flow "
                            "rate. Standard ratings cannot be calculated.");
        }

        StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] = NetCoolingCapRatedMaxSpeed;
        StandardRatingsResult["SEER_User"] = SEER_User;
        StandardRatingsResult["SEER_Standard"] = SEER_Standard;
        StandardRatingsResult["EER"] = EER;

        StandardRatingsResult["SEER2_User"] = SEER2_User;
        StandardRatingsResult["SEER2_Standard"] = SEER2_Standard;
        StandardRatingsResult["EER2"] = EER2;

        StandardRatingsResult["IEER_2022"] = IEER_2022;
        StandardRatingsResult["EER_2022"] = EER_2022;

        return StandardRatingsResult;
    }

    std::map<std::string, Real64> VariableSpeedDXCoolingCoilStandardRatings(
        EnergyPlusData &state,
        std::string const &DXCoilType,                                  // Type of DX coil for which HSPF is calculated
        [[maybe_unused]] std::string const &DXCoilName,                 // Name of the DX Coil for which Standard Ratings are calculated.
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
        Real64 VSRatedVolumetricAirFlowRate)
    {
        // Using/Aliasing
        using Curve::CurveValue;

        // Argument array dimensioning
        CapFTempCurveIndex.dim(nsp);
        CapFFlowCurveIndex.dim(nsp);
        EIRFTempCurveIndex.dim(nsp);
        EIRFFlowCurveIndex.dim(nsp);
        // PLFFPLRCurveIndex.dim(nsp);
        RatedTotalCapacity.dim(nsp);
        RatedCOP.dim(nsp);
        RatedAirVolFlowRate.dim(nsp);
        FanPowerPerEvapAirFlowRateFromInput.dim(nsp);

        // Ratings Based on ANSI/AHRI 210/140
        Real64 NetCoolingCapRatedMaxSpeed2023(0.0); // net cooling capacity at maximum speed
        Real64 SEER2_User(0.0);     // seasonal energy efficiency ratio of multi speed DX cooling coil, from user-input PLF curve and C_D value
        Real64 SEER2_Standard(0.0); // seasonal energy efficiency ratio of multi speed DX cooling coil, from AHRI Std 210/240-2008 default PLF
                                    // curve and
                                    // C_D value
        Real64 EER2(0.0);

        Real64 IEER_2022(0.0);
        // Real64 NetCoolingCapRated2022(0.0); // ?? for which speed (NetCoolingCapRatedMaxSpeed2023)
        Real64 EER_2022(0.0);

        std::map<std::string, Real64> StandardRatingsResult;

        Real64 GrossRatedTotalCoolingCapacityVS(0.0);
        if (VSGrossRatedTotalCoolingCapacity < 0) {
            GrossRatedTotalCoolingCapacityVS = RatedTotalCapacity(nsp);
        } else {
            GrossRatedTotalCoolingCapacityVS = VSGrossRatedTotalCoolingCapacity;
        }
        Real64 RatedVolumetricAirFlowRateVS(0.0);
        if (VSRatedVolumetricAirFlowRate < 0) {
            RatedVolumetricAirFlowRateVS = RatedAirVolFlowRate(nsp);
        } else {
            RatedVolumetricAirFlowRateVS = VSRatedVolumetricAirFlowRate;
        }

        Real64 CapacityScaleFactor(0.0);
        Real64 GrossRatedTotalCoolingCapacity(0.0);
        Real64 ReferenceUnitCapacityAtNominalSpeedLevel(0.0);

        Array1D<Real64> GrossRatedCapacityAtSpeedLevel(nsp);
        Array1D<Real64> ReferenceUnitCapacityAtSpeedLevel(nsp);

        Real64 AirFlowScaleFactor(0.0);
        Real64 RatedVolumetricAirFlowRate(0.0);
        Real64 ReferenceUnitVolAirFlowRateAtNominalSpeedLevel(0.0);

        Array1D<Real64> LoopVolumetricAirFlowRateAtSpeedLevel(nsp);
        Array1D<Real64> ReferenceUnitVolAirFlowRateAtSpeedLevel(nsp);

        int NominalSpeedLevel = nsp;
        for (int spnum = 1; spnum <= nsp; ++spnum) {
            ReferenceUnitCapacityAtSpeedLevel(spnum) = RatedTotalCapacity(spnum);
            ReferenceUnitVolAirFlowRateAtSpeedLevel(spnum) = RatedAirVolFlowRate(spnum);
        }
        GrossRatedTotalCoolingCapacity = GrossRatedTotalCoolingCapacityVS;
        ReferenceUnitCapacityAtNominalSpeedLevel = ReferenceUnitCapacityAtSpeedLevel(NominalSpeedLevel);
        CapacityScaleFactor =
            GrossRatedTotalCoolingCapacity / ReferenceUnitCapacityAtNominalSpeedLevel; // Section 1.41.21.1.6 | Equation (1.160) | IO-Ref

        RatedVolumetricAirFlowRate = RatedVolumetricAirFlowRateVS;
        ReferenceUnitVolAirFlowRateAtNominalSpeedLevel = ReferenceUnitVolAirFlowRateAtSpeedLevel(NominalSpeedLevel);
        AirFlowScaleFactor = RatedVolumetricAirFlowRate / (ReferenceUnitVolAirFlowRateAtNominalSpeedLevel *
                                                           CapacityScaleFactor); // Section 1.41.21.1.7 | Equation (1.162) | IO-Ref

        for (int sp = 1; sp <= nsp; ++sp) {
            GrossRatedCapacityAtSpeedLevel(sp) =
                CapacityScaleFactor * ReferenceUnitCapacityAtSpeedLevel(sp); //  Section 1.41.21.1.6 | Equation (1.161) | IO-Ref

            LoopVolumetricAirFlowRateAtSpeedLevel(sp) = AirFlowScaleFactor * ReferenceUnitVolAirFlowRateAtSpeedLevel(sp) *
                                                        CapacityScaleFactor; // Section 1.41.21.1.7 | Equation (1.163) | IO-Ref
        }

        if (GrossRatedTotalCoolingCapacityVS > 0.0 && RatedVolumetricAirFlowRateVS > 0.0) {

            Real64 TotCapFlowModFac = Curve::CurveValue(state, CapFFlowCurveIndex(nsp), AirMassFlowRatioRated);
            Real64 NetCoolingCapRated =
                GrossRatedCapacityAtSpeedLevel(nsp) *
                    Curve::CurveValue(state, CapFTempCurveIndex(nsp), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                    TotCapFlowModFac -
                FanPowerPerEvapAirFlowRateFromInput(nsp) * LoopVolumetricAirFlowRateAtSpeedLevel(nsp);
            StandardRatingsResult["NetCoolingCapRatedMaxSpeed"] = NetCoolingCapRated;

            NetCoolingCapRatedMaxSpeed2023 =
                GrossRatedCapacityAtSpeedLevel(nsp) *
                    Curve::CurveValue(state, CapFTempCurveIndex(nsp), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2) *
                    TotCapFlowModFac -
                FanPowerPerEvapAirFlowRateFromInput_2023(nsp) * LoopVolumetricAirFlowRateAtSpeedLevel(nsp);
            StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRatedMaxSpeed2023;
            // TODO: Commercial and industrial unitary air-conditioning condensing units with a capacity greater than 135,000 Btu/h (39564.59445
            // Watts) as defined in ANSI/AHRI Standard 365(I-P). | Scope 2.2.6 (ANSI/AHRI 340-360 2022)

            if (CondenserType == DataHeatBalance::RefrigCondenserType::Air) {
                // SEER2 standard applies to factory-made Unitary Air-conditioners and Unitary Air-source Heat Pumps with
                // capacities less than 65,000 Btu/h (19049.61955 Watts) | Section 2.1 (ANSI/AHRI 210-240 2023)
                // Removal of water-cooled and evaporatively-cooled products from the scope | Foreword (ANSI/AHRI 210-240 2023)
                Array1D<int> VSPLRFPLF;
                for (int spnum = 1; spnum <= nsp; ++spnum) {
                    VSPLRFPLF.push_back(PLFFPLRCurveIndex);
                }

                // SEER2 Calculations ANSI/AHRI 210/240 Standard 2023
                std::tie(NetCoolingCapRatedMaxSpeed2023, SEER2_User, SEER2_Standard, EER2) =
                    VariableSpeedDXCoolingCoilSEER2(state,
                                                    nsp,
                                                    CapFFlowCurveIndex,
                                                    GrossRatedCapacityAtSpeedLevel, // RatedTotalCapacity,
                                                    CapFTempCurveIndex,
                                                    FanPowerPerEvapAirFlowRateFromInput_2023,
                                                    LoopVolumetricAirFlowRateAtSpeedLevel, // RatedAirVolFlowRate,
                                                    EIRFFlowCurveIndex,
                                                    RatedCOP,
                                                    EIRFTempCurveIndex,
                                                    VSPLRFPLF);
                StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRatedMaxSpeed2023;
            }
            // IEER Calculation 2022
            std::tie(IEER_2022, NetCoolingCapRatedMaxSpeed2023, EER_2022) =
                IEERCalculationVariableSpeed(state,
                                             DXCoilType,
                                             nsp,
                                             CapFTempCurveIndex,
                                             GrossRatedCapacityAtSpeedLevel, // RatedTotalCapacity,
                                             CapFFlowCurveIndex,
                                             FanPowerPerEvapAirFlowRateFromInput_2023,
                                             LoopVolumetricAirFlowRateAtSpeedLevel, // RatedAirVolFlowRate,
                                             EIRFTempCurveIndex,
                                             RatedCOP,
                                             EIRFFlowCurveIndex,
                                             CondenserType);
            StandardRatingsResult["NetCoolingCapRatedMaxSpeed2023"] = NetCoolingCapRatedMaxSpeed2023;

        } else {
            ShowSevereError(state,
                            "Standard Ratings: Coil:Cooling:DX " + DXCoilType + // TODO: Use dynamic COIL TYPE and COIL INSTANCE name later
                                " has zero rated total cooling capacity. Standard ratings cannot be calculated.");
        }

        // From SEER2 implementation
        StandardRatingsResult["SEER2_User"] = SEER2_User;
        StandardRatingsResult["SEER2_Standard"] = SEER2_Standard;
        StandardRatingsResult["EER2"] = EER2;

        // From IEER2 implementation
        StandardRatingsResult["IEER_2022"] = IEER_2022;
        StandardRatingsResult["EER_2022"] = EER_2022;

        return StandardRatingsResult;
    }

    std::tuple<Real64, Real64, Real64> MultiSpeedDXHeatingCoilHSPF(
        EnergyPlusData &state,
        int const nsp,                                              // Number of compressor speed
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRateInput,    // 2017 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A_int const CapFTempCurveIndex,                       // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                       // Index for the capacity as a function of flow fraction modifier curve
        Array1A<Real64> const RatedTotalCapacity,                   // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedAirVolFlowRate,                  // Reference air flow rate of DX coil [m3/s]
        Array1A_int const EIRFFlowCurveIndex,                       // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                       // Index for the EIR as a function of temperature modifier curve
        Array1A<Real64> const RatedCOP,                             // Reference coefficient of performance [W/W]
        ObjexxFCL::Optional_int_const RegionNum,                    // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor,         // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn,       // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,  // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl) // defrost control; 1=timed, 2=on-demand
    {

        // Intermediate values calculated from the inputs in the idf file
        Real64 HSPF(0.0);
        Real64 NetHeatingCapRatedHighTemp(0.0);
        Real64 NetHeatingCapRatedLowTemp(0.0);

        Array1D<Real64> FanPowerPerEvapAirFlowRate(nsp); // Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]
        Array1D<Real64> TotHeatCapTestH0(nsp);           // Total cooling capacity at A2 test condition (High speed)
        Array1D<Real64> TotHeatCapTestH1(nsp);           // Total cooling capacity at B2 test condition (High speed)
        Array1D<Real64> TotHeatCapTestH2(nsp);           // Total cooling capacity at B1 test condition (Low speed)
        Array1D<Real64> TotHeatCapTestH3(nsp);           // Total cooling capacity at F1 test condition (Low speed)
        Array1D<Real64> OutdoorUnitPowerTestH0(nsp);     // Outdoor Unit electric power at A2 test condition (High speed)
        Array1D<Real64> OutdoorUnitPowerTestH1(nsp);     // Outdoor Unit electric power at B2 test condition (High speed)
        Array1D<Real64> OutdoorUnitPowerTestH2(nsp);     // Outdoor Unit electric power at B1 test condition (Low speed)
        Array1D<Real64> OutdoorUnitPowerTestH3(nsp);     // Outdoor Unit electric power at F1 test condition (Low speed)
        Real64 HeatingCapacityLS;                        // cooling capacity of Mult-speed DX coil at lower speed, [W]
        Real64 HeatingCapacityHS;                        // cooling capacity of Mult-speed DX coil at higher speed, [W]
        Real64 HeatingElecPowerLS;                       // outdoor unit electric power input at low speed, [W]
        Real64 HeatingElecPowerHS;                       // outdoor unit electric power input at high speed, [W]
        Real64 HeatingCapacityMax;                       // cooling capacity of Mult-speed DX coil at max speed, [W]
        Real64 HeatingElecPowerMax;                      // outdoor unit electric power input at Max speed, [W]
        Array1D<Real64> TotHeatCapTestH1High(nsp);       // net heating capacity high speed at H1 test conditon, [W]

        // Intermediate values calculated from the inputs in the idf file
        Array1D<Real64> TotCapFlowModFac(nsp); // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);    // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]

        Real64 TotCapTempModFacH0(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H0 Test [-]
        Real64 EIRTempModFacH0(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb)  at H0 Test[-]
        Real64 TotCapTempModFacH1(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H1 Test [-]
        Real64 EIRTempModFacH1(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb)  at H1 Test[-]
        Real64 TotCapTempModFacH2(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H2 Test [-]
        Real64 EIRTempModFacH2(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb)  at H2 Test[-]
        Real64 TotCapTempModFacH3(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H3 Test [-]
        Real64 EIRTempModFacH3(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb)  at H3 Test[-]

        Real64 OATempCompressorOff(0.0); // Minimum outdoor air temperature to turn the commpressor off
        Real64 PartLoadRatio(0.0);       // compressor cycling ratio between successive speeds, [-]
        Real64 PartLoadFraction(0.0);    // part-load fraction that account for the cyclic degradation, [-]

        Real64 NetHeatingCapWeighted(0.0);       // net total heating cap weighted by the fraction of the binned cooling hours [W]
        Real64 TotHeatingElecPowerWeighted(0.0); // net total heat pump and resistance heating electric Energy input weighted by
        // the fraction of the binned cooling hours
        Real64 BuildingHeatingLoad(0.0);      // Building space heating load corresponding to an outdoor bin temperature [W]
        Real64 NetTotHeatCapBinned(0.0);      // Net tot heatinging cap corresponding to an outdoor bin temperature [W]
        Real64 TotHeatElecPowerBinnedHP(0.0); // Total Heat Pump heating electric power consumption at outdoor bin temp [W]
        Real64 TotHeatElecPowerBinnedRH(0.0); // Total Resistance heating electric power consumption at outdoor bin temp [W]

        Real64 LoadFactor;               // Fractional "on" time for last stage at the desired reduced capacity, (dimensionless)
        Real64 LowTempCutOutFactor(0.0); // Factor which corresponds to compressor operation depending on outdoor temperature

        Real64 FractionalBinHours(0.0);       // Fractional bin hours for the heating season  [-]
        Real64 DemandDeforstCredit(1.0);      // A factor to adjust HSPF if coil has demand defrost control  [-]
        Real64 DesignHeatingRequirement(0.0); // The amount of heating required to maintain a given indoor temperature
        // at a particular outdoor design temperature.  [W]
        Real64 DesignHeatingRequirementMin(0.0); // minimum design heating requirement [W]
        Real64 DesignHeatingRequirementMax(0.0); // maximum design heating requirement [W]

        NetHeatingCapWeighted = 0.0;
        TotHeatingElecPowerWeighted = 0.0;

        for (int spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate(spnum) = 0.0;
            if (MSFanPowerPerEvapAirFlowRateInput(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate(spnum) = DefaultFanPowerPerEvapAirFlowRate;
            } else {
                FanPowerPerEvapAirFlowRate(spnum) = MSFanPowerPerEvapAirFlowRateInput(spnum);
            }
        }

        // Proceed withe HSPF value calculation
        for (int spnum = 1; spnum <= nsp; ++spnum) {
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);
            {
                if (state.dataCurveManager->curves(CapFTempCurveIndex(spnum))->numDims == 1) {
                    TotCapTempModFacH0 = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempH0Test);
                    TotCapTempModFacH1 = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempRated);
                    TotCapTempModFacH2 = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempH2Test);
                    TotCapTempModFacH3 = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempH3Test);
                } else {
                    TotCapTempModFacH0 = Curve::CurveValue(
                        state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH0Test);
                    TotCapTempModFacH1 = Curve::CurveValue(
                        state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempRated);
                    TotCapTempModFacH2 = Curve::CurveValue(
                        state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH2Test);
                    TotCapTempModFacH3 = Curve::CurveValue(
                        state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH3Test);
                }
            }

            TotHeatCapTestH0(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH0 * TotCapFlowModFac(spnum) +
                                      FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
            TotHeatCapTestH1(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH1 * TotCapFlowModFac(spnum) +
                                      FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
            TotHeatCapTestH2(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH2 * TotCapFlowModFac(spnum) +
                                      FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
            TotHeatCapTestH3(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH3 * TotCapFlowModFac(spnum) +
                                      FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);

            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            {
                if (state.dataCurveManager->curves(EIRFTempCurveIndex(spnum))->numDims == 1) {
                    EIRTempModFacH0 = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempH0Test);
                    EIRTempModFacH1 = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempRated);
                    EIRTempModFacH2 = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempH2Test);
                    EIRTempModFacH3 = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTempH3Test);
                } else {
                    EIRTempModFacH0 = Curve::CurveValue(
                        state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH0Test);
                    EIRTempModFacH1 = Curve::CurveValue(
                        state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempRated);
                    EIRTempModFacH2 = Curve::CurveValue(
                        state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH2Test);
                    EIRTempModFacH3 = Curve::CurveValue(
                        state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTempRated, HeatingOutdoorCoilInletAirDBTempH3Test);
                }
            }
            if (RatedCOP(spnum) > 0.0) {
                OutdoorUnitPowerTestH0(spnum) = TotHeatCapTestH0(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH0 / RatedCOP(spnum) +
                                                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
                OutdoorUnitPowerTestH1(spnum) = TotHeatCapTestH1(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH1 / RatedCOP(spnum) +
                                                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
                OutdoorUnitPowerTestH2(spnum) = TotHeatCapTestH2(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH2 / RatedCOP(spnum) +
                                                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
                OutdoorUnitPowerTestH3(spnum) = TotHeatCapTestH3(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH3 / RatedCOP(spnum) +
                                                FanPowerPerEvapAirFlowRate(spnum) * RatedAirVolFlowRate(spnum);
            }
        }

        // determine the HP capacity at the rated condition (AHRI H1 high speed test Condition); and determine the
        // the building heat requirement for the user specified region
        NetHeatingCapRatedHighTemp = TotHeatCapTestH1(nsp);
        NetHeatingCapRatedLowTemp = TotHeatCapTestH3(nsp);

        if (RegionNum == 5) {
            DesignHeatingRequirementMin = NetHeatingCapRatedHighTemp;
            DesignHeatingRequirementMax = 2.20 * NetHeatingCapRatedHighTemp;
        } else {
            DesignHeatingRequirementMin = NetHeatingCapRatedHighTemp * (18.33 - OutdoorDesignTemperature[RegionNum - 1]) / (60.0 / 1.80);
            DesignHeatingRequirementMax = 2.20 * DesignHeatingRequirementMin;
        }
        // Set the Design Heating Requirement to nearest standard value (From Table 18, AHRI/ANSI Std 210/240)
        for (int StandardDHRNum = 0; StandardDHRNum < TotalNumOfStandardDHRs - 1; ++StandardDHRNum) {
            if (DesignHeatingRequirementMin < StandardDesignHeatingRequirement[0]) {

                DesignHeatingRequirement = min(StandardDesignHeatingRequirement[0], DesignHeatingRequirementMax);

            } else if (DesignHeatingRequirementMin >= StandardDesignHeatingRequirement[StandardDHRNum] &&
                       DesignHeatingRequirementMin < StandardDesignHeatingRequirement[StandardDHRNum + 1]) {
                if ((DesignHeatingRequirementMin - StandardDesignHeatingRequirement[StandardDHRNum]) >
                    (StandardDesignHeatingRequirement[StandardDHRNum + 1] - DesignHeatingRequirementMin)) {

                    DesignHeatingRequirement = min(StandardDesignHeatingRequirement[StandardDHRNum + 1], DesignHeatingRequirementMax);
                } else {
                    DesignHeatingRequirement = min(StandardDesignHeatingRequirement[StandardDHRNum], DesignHeatingRequirementMax);
                }
            } else if (DesignHeatingRequirementMin >= StandardDesignHeatingRequirement[TotalNumOfStandardDHRs - 1]) {
                DesignHeatingRequirement = min(StandardDesignHeatingRequirement[StandardDHRNum], DesignHeatingRequirementMax);
            }
        }
        // The minimum temperature below which the compressor is turned off
        OATempCompressorOff = MinOATCompressor;

        for (int BinNum = 0; BinNum < TotalNumOfTemperatureBins[RegionNum - 1]; ++BinNum) { // NumOfOATempBins

            FractionalBinHours = FracBinHoursAtOutdoorBinTemp[RegionNum - 1][BinNum];

            // Calculate the building heating load
            BuildingHeatingLoad = (18.33 - OutdoorBinTemperature[BinNum]) / (18.33 - OutdoorDesignTemperature[RegionNum - 1]) * CorrectionFactor *
                                  DesignHeatingRequirement;

            if ((OutdoorBinTemperature[BinNum] <= -8.33) || (OutdoorBinTemperature[BinNum] >= 7.20)) {
                HeatingCapacityMax = TotHeatCapTestH3(nsp) + ((TotHeatCapTestH1(nsp) - TotHeatCapTestH3(nsp)) *
                                                              (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                              (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test));
                HeatingElecPowerMax =
                    OutdoorUnitPowerTestH3(nsp) + ((OutdoorUnitPowerTestH1(nsp) - OutdoorUnitPowerTestH3(nsp)) *
                                                   (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                   (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test));
            } else {
                HeatingCapacityMax = TotHeatCapTestH3(nsp) + ((TotHeatCapTestH2(nsp) - TotHeatCapTestH3(nsp)) *
                                                              (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                              (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test));
                HeatingElecPowerMax =
                    OutdoorUnitPowerTestH3(nsp) + ((OutdoorUnitPowerTestH2(nsp) - OutdoorUnitPowerTestH3(nsp)) *
                                                   (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                   (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test));
            }

            // determine the speed number
            for (int spnum = 1; spnum <= nsp - 1; ++spnum) {
                // Low Speed
                if (OutdoorBinTemperature[BinNum] < -8.33) {
                    HeatingCapacityLS = TotHeatCapTestH3(spnum) + ((TotHeatCapTestH1(spnum) - TotHeatCapTestH3(spnum)) *
                                                                   (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                                   (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test));
                    HeatingElecPowerLS =
                        OutdoorUnitPowerTestH3(spnum) + ((OutdoorUnitPowerTestH1(spnum) - OutdoorUnitPowerTestH3(spnum)) *
                                                         (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                         (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test));

                } else if (OutdoorBinTemperature[BinNum] >= 4.44) {
                    HeatingCapacityLS = TotHeatCapTestH1(spnum) + ((TotHeatCapTestH0(spnum) - TotHeatCapTestH1(spnum)) *
                                                                   (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempRated) /
                                                                   (HeatingOutdoorCoilInletAirDBTempH0Test - HeatingOutdoorCoilInletAirDBTempRated));
                    HeatingElecPowerLS =
                        OutdoorUnitPowerTestH1(spnum) + ((OutdoorUnitPowerTestH0(spnum) - OutdoorUnitPowerTestH1(spnum)) *
                                                         (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempRated) /
                                                         (HeatingOutdoorCoilInletAirDBTempH0Test - HeatingOutdoorCoilInletAirDBTempRated));
                } else {
                    HeatingCapacityLS = TotHeatCapTestH3(spnum) + ((TotHeatCapTestH2(spnum) - TotHeatCapTestH3(spnum)) *
                                                                   (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                                   (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test));
                    HeatingElecPowerLS =
                        OutdoorUnitPowerTestH3(spnum) + ((OutdoorUnitPowerTestH2(spnum) - OutdoorUnitPowerTestH3(spnum)) *
                                                         (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                         (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test));
                }
                // High Speed
                if ((OutdoorBinTemperature[BinNum] <= -8.33) || (OutdoorBinTemperature[BinNum] >= 7.20)) {
                    HeatingCapacityHS =
                        TotHeatCapTestH3(spnum + 1) + ((TotHeatCapTestH1(spnum + 1) - TotHeatCapTestH3(spnum + 1)) *
                                                       (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                       (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test));
                    HeatingElecPowerHS =
                        OutdoorUnitPowerTestH3(spnum + 1) + ((OutdoorUnitPowerTestH1(spnum + 1) - OutdoorUnitPowerTestH3(spnum + 1)) *
                                                             (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                             (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test));
                } else {
                    HeatingCapacityHS =
                        TotHeatCapTestH3(spnum + 1) + ((TotHeatCapTestH2(spnum + 1) - TotHeatCapTestH3(spnum + 1)) *
                                                       (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                       (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test));
                    HeatingElecPowerHS =
                        OutdoorUnitPowerTestH3(spnum + 1) + ((OutdoorUnitPowerTestH2(spnum + 1) - OutdoorUnitPowerTestH3(spnum + 1)) *
                                                             (OutdoorBinTemperature[BinNum] - HeatingOutdoorCoilInletAirDBTempH3Test) /
                                                             (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test));
                }
                LowTempCutOutFactor = 0.0;
                if (!OATempCompressorOnOffBlank) {
                    if (OutdoorBinTemperature[BinNum] <= OATempCompressorOff) {
                        LowTempCutOutFactor = 0.0;
                    } else if (OutdoorBinTemperature[BinNum] <= OATempCompressorOn) {
                        LowTempCutOutFactor = 0.5;
                    } else {
                        LowTempCutOutFactor = 1.0;
                    }
                } else {
                    LowTempCutOutFactor = 1.0;
                }

                if (BuildingHeatingLoad <= HeatingCapacityLS) {
                    if (HeatingCapacityLS > 0.0) {
                        PartLoadRatio = min(1.0, BuildingHeatingLoad / HeatingCapacityLS);
                    }
                    NetTotHeatCapBinned = BuildingHeatingLoad;
                    PartLoadFraction = 1.0 - CyclicDegradationCoeff * (1.0 - PartLoadRatio);
                    TotHeatElecPowerBinnedHP = (PartLoadRatio / PartLoadFraction) * HeatingElecPowerLS * LowTempCutOutFactor;
                    TotHeatElecPowerBinnedRH = BuildingHeatingLoad * (1.0 - LowTempCutOutFactor);
                    goto HeatSpeedLoop_exit;

                } else if (BuildingHeatingLoad < HeatingCapacityHS) {
                    // cycle between speed "spnum" and "spnum + 1"
                    LoadFactor = min(1.0, (HeatingCapacityHS - BuildingHeatingLoad) / (HeatingCapacityHS - HeatingCapacityLS));
                    LoadFactor = max(0.0, LoadFactor);

                    NetTotHeatCapBinned = BuildingHeatingLoad;
                    TotHeatElecPowerBinnedHP = LoadFactor * HeatingElecPowerLS + (1.0 - LoadFactor) * HeatingElecPowerHS;
                    TotHeatElecPowerBinnedHP *= LowTempCutOutFactor;
                    TotHeatElecPowerBinnedRH = BuildingHeatingLoad * (1.0 - LowTempCutOutFactor);
                    goto HeatSpeedLoop_exit;

                } else if (BuildingHeatingLoad >= HeatingCapacityMax) {
                    NetTotHeatCapBinned = BuildingHeatingLoad;
                    if (!OATempCompressorOnOffBlank && HeatingElecPowerMax > 0.0) {
                        if ((OutdoorBinTemperature[BinNum] <= OATempCompressorOff) || (HeatingCapacityMax / HeatingElecPowerMax < 1.0)) {
                            LowTempCutOutFactor = 0.0;
                        } else if ((OutdoorBinTemperature[BinNum] <= OATempCompressorOn) && (HeatingCapacityMax / HeatingElecPowerMax > 1.0)) {
                            LowTempCutOutFactor = 0.5;
                        } else if ((OutdoorBinTemperature[BinNum] > OATempCompressorOn) && (HeatingCapacityMax / HeatingElecPowerMax > 1.0)) {
                            LowTempCutOutFactor = 1.0;
                        }
                    } else {
                        LowTempCutOutFactor = 1.0;
                    }

                    TotHeatElecPowerBinnedHP = HeatingElecPowerMax * LowTempCutOutFactor;
                    TotHeatElecPowerBinnedRH = BuildingHeatingLoad - HeatingCapacityMax * LowTempCutOutFactor;
                    goto HeatSpeedLoop_exit;
                }
            }
        HeatSpeedLoop_exit:;

            NetHeatingCapWeighted += NetTotHeatCapBinned * FractionalBinHours;
            TotHeatingElecPowerWeighted += (TotHeatElecPowerBinnedHP + TotHeatElecPowerBinnedRH) * FractionalBinHours;
        }

        if (DefrostControl == HPdefrostControl::Timed) {
            DemandDeforstCredit = 1.0; // Timed defrost control
        } else {
            DemandDeforstCredit = 1.03; // Demand defrost control
        }

        if (TotHeatingElecPowerWeighted > 0.0) {
            HSPF = NetHeatingCapWeighted * DemandDeforstCredit / TotHeatingElecPowerWeighted;
        } else {
            HSPF = 0.0;
        }

        return std::make_tuple(NetHeatingCapRatedHighTemp, NetHeatingCapRatedLowTemp, HSPF);
    }

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
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl)   // defrost control; 1=timed, 2=on-demand
    {

        // Intermediate values calculated from the inputs in the idf file
        Real64 HSPF2_2023(0.0);
        Real64 NetHeatingCapRatedHighTemp_2023(0.0);
        Real64 NetHeatingCapRatedLowTemp_2023(0.0);

        int BinNum2023; // bin number counter
        int spnum;      // compressor speed number

        Array1D<Real64> FanPowerPerEvapAirFlowRate_2023(nsp); // Fan power per air volume flow rate through the evaporator coil [W/(m3/s)]

        // Real64 HeatingCapacityLS;                  // cooling capacity of Mult-speed DX coil at lower speed, [W]
        // Real64 HeatingCapacityHS;                  // cooling capacity of Mult-speed DX coil at higher speed, [W]
        // Real64 HeatingElecPowerLS;                 // outdoor unit electric power input at low speed, [W]
        // Real64 HeatingElecPowerHS;                 // outdoor unit electric power input at high speed, [W]
        // Real64 HeatingCapacityMax;                 // cooling capacity of Mult-speed DX coil at max speed, [W]
        // Real64 HeatingElecPowerMax;                // outdoor unit electric power input at Max speed, [W]
        // Array1D<Real64> TotHeatCapTestH1High(nsp); // net heating capacity high speed at H1 test conditon, [W]

        // Intermediate values calculated from the inputs in the idf file
        Array1D<Real64> TotCapFlowModFac(nsp); // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
        Array1D<Real64> EIRFlowModFac(nsp);    // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]

        Real64 TotCapTempModFacH0Low(0.0);  // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H0 Low Test [-]
        Real64 EIRTempModFacH0Low(0.0);     // EIR modifier (function of entering wetbulb, outside drybulb) at H0 Low Test[-]
        Real64 TotCapTempModFacH1Low(0.0);  // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H1 Low Test [-]
        Real64 EIRTempModFacH1Low(0.0);     // EIR modifier (function of entering wetbulb, outside drybulb) at H1 Low Test[-]
        Real64 TotCapTempModFacH2Int(0.0);  // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H2 Int Test [-]
        Real64 EIRTempModFacH2Int(0.0);     // EIR modifier (function of entering wetbulb, outside drybulb) at H2 Int Test[-]
        Real64 TotCapTempModFacH1Full(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H1 Full Test [-]
        Real64 EIRTempModFacH1Full(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb) at H1 Full Test[-]
        Real64 TotCapTempModFacH2Full(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H2 Full Test [-]
        Real64 EIRTempModFacH2Full(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb) at H2 Full Test[-]
        Real64 TotCapTempModFacH3Full(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H3 Full Test [-]
        Real64 EIRTempModFacH3Full(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb) at H3 Full Test[-]
        Real64 TotCapTempModFacH4Full(0.0); // Tot capacity modifier (function of entering wetbulb, outside drybulb) at H4 Full Test [-]
        Real64 EIRTempModFacH4Full(0.0);    // EIR modifier (function of entering wetbulb, outside drybulb) at H4 Full Test[-]

        Real64 OATempCompressorOff(0.0); // Minimum outdoor air temperature to turn the commpressor off

        Real64 NetHeatingCapWeighted(0.0);       // net total heating cap weighted by the fraction of the binned cooling hours [W]
        Real64 TotHeatingElecPowerWeighted(0.0); // net total heat pump and resistance heating electric Energy input weighted by
        // the fraction of the binned cooling hours

        Real64 n(0.0);     // Fractional bin hours for the heating season  [-]
        Real64 f_def(1.0); // Demand Defrost Credit, A factor to adjust HSPF if coil has demand defrost control  [-]

        NetHeatingCapWeighted = 0.0;
        TotHeatingElecPowerWeighted = 0.0;

        for (spnum = 1; spnum <= nsp; ++spnum) {
            FanPowerPerEvapAirFlowRate_2023(spnum) = 0.0;
            if (MSFanPowerPerEvapAirFlowRateInput_2023(spnum) <= 0.0) {
                FanPowerPerEvapAirFlowRate_2023(spnum) = DefaultFanPowerPerEvapAirFlowRateSEER2;
            } else {
                FanPowerPerEvapAirFlowRate_2023(spnum) = MSFanPowerPerEvapAirFlowRateInput_2023(spnum);
            }
        }

        Real64 HeatingOutdoorCoilInletAirDBTemp_H0LowTest = 16.66; // Outdoor air dry-bulb temp in degrees 16.66 C (62 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H0LowTest = 21.11;  // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Real64 HeatingOutdoorCoilInletAirDBTemp_H1LowTest = 8.33; // Outdoor air dry-bulb temp in degrees 8.33 C (47 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H1LowTest = 21.11; // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Real64 HeatingOutdoorCoilInletAirDBTemp_H2IntTest = 1.66; // Outdoor air dry-bulb temp in degrees 1.66 C (35 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H2IntTest = 21.11; // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Real64 HeatingOutdoorCoilInletAirDBTemp_H1FullTest = 8.33; // Outdoor air dry-bulb temp in degrees 8.33 C (47 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H1FullTest = 21.11; // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Real64 HeatingOutdoorCoilInletAirDBTemp_H2FullTest = 1.66; // Outdoor air dry-bulb temp in degrees  C (35 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H2FullTest = 21.11; // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Real64 HeatingOutdoorCoilInletAirDBTemp_H3FullTest = -8.33; // Outdoor air dry-bulb temp in degrees  C (17 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H3FullTest = 21.11;  // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Real64 HeatingOutdoorCoilInletAirDBTemp_H4FullTest = -15;  // Outdoor air dry-bulb temp in degrees  C (5 F)
        Real64 HeatingIndoorCoilInletAirDBTemp_H4FullTest = 21.11; // Indoor air dry-bulb temp in degrees 21.11 C (70 F)

        Array1D<Real64> Q_A_Full(nsp);

        Array1D<Real64> Q_H0_Low(nsp);  // Total cooling capacity at H0 Low test condition (Low speed)
        Array1D<Real64> Q_H1_Low(nsp);  // Total cooling capacity at H1 Low test condition (Low speed)
        Array1D<Real64> Q_H2_Int(nsp);  // Total cooling capacity at H2 Int test condition
        Array1D<Real64> Q_H1_Full(nsp); // Total cooling capacity at H1 Full test condition (High speed)
        Array1D<Real64> Q_H2_Full(nsp); // Total cooling capacity at H2 Full test condition (High speed)
        Array1D<Real64> Q_H3_Full(nsp); // Total cooling capacity at H3 Full test condition (High speed)
        Array1D<Real64> Q_H4_Full(nsp); // Total cooling capacity at H4 Full test condition (High speed)

        Array1D<Real64> P_H0_Low(nsp);  // Outdoor Unit electric power at H0 Low test condition (Low speed)
        Array1D<Real64> P_H1_Low(nsp);  // Outdoor Unit electric power at H1 Low test condition (Low speed)
        Array1D<Real64> P_H2_Int(nsp);  // Outdoor Unit electric power at H2 Int test condition
        Array1D<Real64> P_H1_Full(nsp); // Outdoor Unit electric power at H1 Full test condition (Full speed)
        Array1D<Real64> P_H2_Full(nsp); // Outdoor Unit electric power at H2 Full test condition (Full speed)
        Array1D<Real64> P_H3_Full(nsp); // Outdoor Unit electric power at H3 Full test condition (Full speed)
        Array1D<Real64> P_H4_Full(nsp); // Outdoor Unit electric power at H4 Full test condition (Full speed)

        // Proceed withe HSPF2 value calculation
        for (spnum = 1; spnum <= nsp; ++spnum) {
            TotCapFlowModFac(spnum) = Curve::CurveValue(state, CapFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            if (state.dataCurveManager->curves(CapFTempCurveIndex(spnum))->numDims == 1) {

                TotCapTempModFacH0Low = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H0LowTest);
                TotCapTempModFacH1Low = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H1LowTest);
                TotCapTempModFacH2Int = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H2IntTest);
                TotCapTempModFacH1Full = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H1FullTest);
                TotCapTempModFacH2Full = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H2FullTest);
                TotCapTempModFacH3Full = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H3FullTest);
                TotCapTempModFacH4Full = Curve::CurveValue(state, CapFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H4FullTest);
            } else {
                TotCapTempModFacH0Low = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H0LowTest, HeatingOutdoorCoilInletAirDBTemp_H0LowTest);
                TotCapTempModFacH1Low = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H1LowTest, HeatingOutdoorCoilInletAirDBTemp_H1LowTest);
                TotCapTempModFacH2Int = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H2IntTest, HeatingOutdoorCoilInletAirDBTemp_H2IntTest);
                TotCapTempModFacH1Full = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H1FullTest, HeatingOutdoorCoilInletAirDBTemp_H1FullTest);
                TotCapTempModFacH2Full = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H2FullTest, HeatingOutdoorCoilInletAirDBTemp_H2FullTest);
                TotCapTempModFacH3Full = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H3FullTest, HeatingOutdoorCoilInletAirDBTemp_H3FullTest);
                TotCapTempModFacH4Full = Curve::CurveValue(
                    state, CapFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H4FullTest, HeatingOutdoorCoilInletAirDBTemp_H4FullTest);
            }

            Real64 curveVal;
            switch (state.dataCurveManager->curves(CapFTempCurveIndex(spnum))->numDims) {
            case 1:
                curveVal = Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated);
                break;
            case 2:
            default: // this default allows the simulation to continue, but will issue a warning, should be removed eventually
                curveVal =
                    Curve::CurveValue(state, CapFTempCurveIndex(spnum), IndoorCoilInletAirWetBulbTempRated, OutdoorCoilInletAirDryBulbTempTestA2);
                break;
            }
            Q_A_Full(spnum) =
                RatedTotalCapacity(spnum) * curveVal * TotCapFlowModFac(spnum) - FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            Q_H0_Low(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH0Low * TotCapFlowModFac(spnum) +
                              FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_H1_Low(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH1Low * TotCapFlowModFac(spnum) +
                              FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_H2_Int(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH2Int * TotCapFlowModFac(spnum) +
                              FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_H1_Full(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH1Full * TotCapFlowModFac(spnum) +
                               FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_H2_Full(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH2Full * TotCapFlowModFac(spnum) +
                               FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_H3_Full(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH3Full * TotCapFlowModFac(spnum) +
                               FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            Q_H4_Full(spnum) = RatedTotalCapacity(spnum) * TotCapTempModFacH4Full * TotCapFlowModFac(spnum) +
                               FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);

            EIRFlowModFac(spnum) = Curve::CurveValue(state, EIRFFlowCurveIndex(spnum), AirMassFlowRatioRated);

            if (state.dataCurveManager->curves(EIRFTempCurveIndex(spnum))->numDims == 1) {

                EIRTempModFacH0Low = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H0LowTest);

                EIRTempModFacH1Low = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H1LowTest);
                EIRTempModFacH2Int = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H2IntTest);
                EIRTempModFacH1Full = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H1FullTest);
                EIRTempModFacH2Full = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H2FullTest);
                EIRTempModFacH3Full = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H3FullTest);
                EIRTempModFacH4Full = Curve::CurveValue(state, EIRFTempCurveIndex(spnum), HeatingOutdoorCoilInletAirDBTemp_H4FullTest);

            } else {

                EIRTempModFacH0Low = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H0LowTest, HeatingOutdoorCoilInletAirDBTemp_H0LowTest);
                EIRTempModFacH1Low = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H1LowTest, HeatingOutdoorCoilInletAirDBTemp_H1LowTest);
                EIRTempModFacH2Int = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H2IntTest, HeatingOutdoorCoilInletAirDBTemp_H2IntTest);
                EIRTempModFacH1Full = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H1FullTest, HeatingOutdoorCoilInletAirDBTemp_H1FullTest);
                EIRTempModFacH2Full = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H2FullTest, HeatingOutdoorCoilInletAirDBTemp_H2FullTest);
                EIRTempModFacH3Full = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H3FullTest, HeatingOutdoorCoilInletAirDBTemp_H3FullTest);
                EIRTempModFacH4Full = Curve::CurveValue(
                    state, EIRFTempCurveIndex(spnum), HeatingIndoorCoilInletAirDBTemp_H4FullTest, HeatingOutdoorCoilInletAirDBTemp_H4FullTest);
            }

            if (RatedCOP(spnum) > 0.0) {

                P_H0_Low(spnum) = Q_H0_Low(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH0Low / RatedCOP(spnum) +
                                  FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
                P_H1_Low(spnum) = Q_H1_Low(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH1Low / RatedCOP(spnum) +
                                  FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
                P_H2_Int(spnum) = Q_H2_Int(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH2Int / RatedCOP(spnum) +
                                  FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
                P_H1_Full(spnum) = Q_H1_Full(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH1Full / RatedCOP(spnum) +
                                   FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
                P_H2_Full(spnum) = Q_H2_Full(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH2Full / RatedCOP(spnum) +
                                   FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
                P_H3_Full(spnum) = Q_H3_Full(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH3Full / RatedCOP(spnum) +
                                   FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
                P_H4_Full(spnum) = Q_H4_Full(spnum) * EIRFlowModFac(spnum) * EIRTempModFacH4Full / RatedCOP(spnum) +
                                   FanPowerPerEvapAirFlowRate_2023(spnum) * RatedAirVolFlowRate(spnum);
            }
        }

        // determine the HP capacity at the rated condition (AHRI H1 high speed test Condition); and determine the
        // the building heat requirement for the user specified region
        NetHeatingCapRatedHighTemp_2023 = Q_H1_Full(nsp);
        NetHeatingCapRatedLowTemp_2023 = Q_H3_Full(nsp);

        // The minimum temperature below which the compressor is turned off
        Real64 t_Off = MinOATCompressor; // Heating off | outdoor Minimum temperature below which the compressor ceases to operate
        // The outdoor temperature when the compressor is automatically turned
        Real64 t_On = OATempCompressorOn; // Heating On | outdoor temperature at which the compressor reinitiates operation

        Int64 RN = static_cast<int64_t>(RegionNum);

        Real64 q_sum(0.0);
        Real64 e_sum(0.0);
        Real64 rh_sum(0.0);

        // The minimum temperature below which the compressor is turned off
        OATempCompressorOff = MinOATCompressor;

        // Equation 11.111 AHRI-2023
        Real64 t_ob = 7.22; //  temperature at which frosting influence on full stage performance begins 7.22 C (45 F)
        for (BinNum2023 = 0; BinNum2023 < 18; ++BinNum2023) { // NumOfOATempBins

            Real64 t = OutdoorBinTemperature[BinNum2023];
            n = FracBinHoursAtOutdoorBinTempHSPF2[RN - 1][BinNum2023];
            if (n == 0.0) {
                // we're skipping load calculations for any Temperature bin against which fractional hours are 0.0
                continue;
            }
            Real64 t_zl = ZoneLoadTemperature[RN - 1];
            Real64 t_od = OutdoorDesignTemperature[RN - 1];
            Real64 c_x = VariableSpeedLoadFactor[RN - 1];

            // For ANSI/AHRI 210/240 Standard 2023 | Concept of DHRI min and max is removed
            // Section 11.2.2.1 Equation 11.104  which suggests QAFull is used instead of DHRI min
            // While Calculaiting the Building load For heating-only heat pump units, replace Q_A_Full with Q_H_Full
            // Q_H_Full = the heating capacity at 47F determined from the H1N test for variable capacity systems and
            // from the H1Full test for other systems, Btu/h.
            Real64 bl = (t_zl - t) / (t_zl - t_od) * c_x * Q_H1_Full(nsp);

            Real64 q_full(0.0);
            Real64 p_full(0.0);
            Real64 cop_full(0.0);

            Real64 delta_full(0.0);
            Real64 hlf_full(0.0);
            Real64 e(0.0);
            Real64 rh(0.0);

            if (t >= t_ob) {
                q_full = Q_H3_Full(nsp) + (Q_H1_Full(nsp) - Q_H3_Full(nsp)) * ((t - (-8.33)) / (8.33 - (-8.33))); // Equation 11.112 AHRI-2023
                p_full = P_H3_Full(nsp) + (P_H1_Full(nsp) - P_H3_Full(nsp)) * ((t - (-8.33)) / (8.33 - (-8.33))); // Equation 11.117 AHRI-2023
            } else if (t >= (-8.33)) {
                q_full = Q_H3_Full(nsp) + (Q_H2_Full(nsp) - Q_H3_Full(nsp)) * ((t - (-8.33)) / (1.66 - (-8.33))); // Equation 11.113 AHRI-2023
                p_full = P_H3_Full(nsp) + (P_H2_Full(nsp) - P_H3_Full(nsp)) * ((t - (-8.33)) / (1.66 - (-8.33))); // Equation 11.118 AHRI-2023
            } else {
                q_full = Q_H4_Full(nsp) + (Q_H3_Full(nsp) - Q_H4_Full(nsp)) * ((t - (-8.33)) / ((-8.33) - (-15))); // Equation 11.114 AHRI-2023
                p_full = P_H4_Full(nsp) + (P_H3_Full(nsp) - P_H4_Full(nsp)) * ((t - (-8.33)) / ((-8.33) - (-15))); // Equation 11.119 AHRI-2023
            }

            cop_full = q_full / p_full;

            if (t <= t_Off || cop_full < 1.0) {
                delta_full = 0.0; // #Equation 11.125 AHRI-2023
            } else if (t > t_On) {
                delta_full = 1.0; // Equation 11.127 AHRI-2023
            } else {
                delta_full = 0.5; // #Equation 11.126 AHRI-2023
            }

            if (q_full > bl) {
                hlf_full = bl / q_full; // Equation 11.107 AHRI-2023
            } else {
                hlf_full = 1.0; // Equation 11.108 AHRI-2023
            }

            for (spnum = 1; spnum <= nsp - 1; ++spnum) {

                // Intermediate capacity
                Real64 q_H0_low = Q_H0_Low(spnum);
                Real64 q_H1_low = Q_H1_Low(spnum);
                Real64 q_H2_int = Q_H2_Int(spnum);
                Real64 q_H1_full = Q_H1_Full(spnum + 1);
                Real64 q_H2_full = Q_H2_Full(spnum + 1);
                Real64 q_H3_full = Q_H3_Full(spnum + 1);
                Real64 q_H4_full = Q_H4_Full(spnum + 1);
                // Equation 11.177 AHRI-2023
                //?? (replaced 62 with 35) in Ratio expression // (t=>35-47/62-47)
                Real64 q_35_low = // q_H1_low + (q_H0_low - q_H1_low) * ((t - (8.33)) / (1.66 - (8.33)));
                    q_H1_low + (q_H0_low - q_H1_low) * ((1.67 - (8.33)) / (16.67 - (8.33)));

                // Equation 11.191 AHRI-2023
                Real64 N_Hq = (q_H2_full != q_35_low) ? min(1.0, (q_H2_int - q_35_low) / (q_H2_full - q_35_low)) : 0.0;
                N_Hq = max(0.0, N_Hq);
                // Equation 11.190 AHRI-2023
                Real64 M_Hq = (q_H0_low - q_H1_low) / (16.66 - 8.33) * (1.0 - N_Hq) + (q_H2_full - q_H3_full) / (1.66 - (-8.33)) * N_Hq;

                // Intermediate Power
                Real64 p_H0_low = P_H0_Low(spnum);
                Real64 p_H1_low = P_H1_Low(spnum);
                Real64 p_H2_int = P_H2_Int(spnum);
                Real64 p_H1_full = P_H1_Full(spnum + 1);
                Real64 p_H2_full = P_H2_Full(spnum + 1);
                Real64 p_H3_full = P_H3_Full(spnum + 1);
                Real64 p_H4_full = P_H4_Full(spnum + 1);
                // Equation 11.178 AHRI - 2023
                //?? (replaced 62 with 35) in Ratio expression (t=>35 F-47/35-47)
                Real64 p_35_low = // p_H1_low + (p_H0_low - p_H1_low) * ((t - (8.33)) / (1.66 - (8.33)));
                    p_H1_low + (p_H0_low - p_H1_low) * ((1.67 - (8.33)) / (16.67 - (8.33)));

                // Equation 11.194 AHRI-2023
                Real64 N_HE = (p_H2_full != p_35_low) ? min(1.0, (p_H2_int - p_35_low) / (p_H2_full - p_35_low)) : 0.0;
                N_HE = max(0.0, N_HE);

                // Equation 11.193 AHRI-2023
                Real64 M_HE = (p_H0_low - p_H1_low) / (16.66 - 8.33) * (1.0 - N_HE) + (p_H2_full - p_H3_full) / (1.66 - (-8.33)) * N_HE;

                // Note: this is strange that there is no defrost cut in the low speed and doesn't use H2 or H3 low
                // Equation 11.177 AHRI-2023
                Real64 q_low; // = q_H1_low + (q_H0_low - q_H1_low) * ((t - (8.33)) / (16.66 - (8.33)));
                // Equation 11.178 AHRI-2023
                Real64 p_low; // = p_H1_low + (p_H0_low - p_H1_low) * ((t - (8.33)) / (16.66 - (8.33)));
                Real64 q_hs(0.0);
                Real64 p_hs(0.0);
                // Low Speed
                if (t >= 8.33) {
                    Real64 ratio = // (t - 8.33) / (16.67 - 8.33)
                        (t - HeatingOutdoorCoilInletAirDBTempRated) /
                        (HeatingOutdoorCoilInletAirDBTempH0Test - HeatingOutdoorCoilInletAirDBTempRated);
                    // equation 11.179
                    q_low = Q_H1_Low(spnum) + ((Q_H1_Low(spnum) - Q_H3_Full(spnum)) * ratio);
                    // equation 11.182
                    p_low = P_H1_Low(spnum) + ((P_H1_Low(spnum) - P_H3_Full(spnum)) * ratio);
                } else if (t >= 1.67 && t < 8.33) {
                    Real64 ratio = // (t - 1.67) / (8.33 - 1.67)
                        (t - HeatingOutdoorCoilInletAirDBTempH2Test) /
                        (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH2Test);
                    // equation 11.180
                    q_low = Q_H2_Int(spnum) + ((Q_H0_Low(spnum) - Q_H1_Low(spnum)) * ratio);
                    // equation 11.183
                    p_low = P_H2_Int(spnum) + ((P_H0_Low(spnum) - P_H1_Low(spnum)) * ratio);
                } else { // if (t < 1.67)
                    // for now Q_H2_Int is replaced with Q_H_Int, no equation for the later
                    // equation 11.181
                    q_low = Q_H2_Int(spnum);
                    // equation 11.184
                    p_low = P_H2_Int(spnum);
                }

                // High Speed
                if (t <= -15.0) {
                    Real64 ratio = // ((t - (-15.0)) / (8.33 - (-8.33)));
                        (t - HeatingOutdoorCoilInletAirDBTemp_H4FullTest) /
                        (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test);
                    // equation 11.205
                    q_hs = Q_H4_Full(spnum + 1) + ((Q_H1_Full(spnum + 1) - Q_H3_Full(spnum + 1)) * ratio);
                    // equation 11.206
                    p_hs = P_H4_Full(spnum + 1) + ((P_H1_Full(spnum + 1) - P_H3_Full(spnum + 1)) * ratio);
                } else if ((t > -15.0) && (t < -8.33)) {
                    Real64 ratio = // ((t - (-15.0)) / (-8.33 - (-15.0)));
                        (t - HeatingOutdoorCoilInletAirDBTemp_H4FullTest) /
                        (HeatingOutdoorCoilInletAirDBTempH3Test - HeatingOutdoorCoilInletAirDBTemp_H4FullTest);
                    // equation 11.203
                    q_hs = Q_H4_Full(spnum + 1) + ((Q_H3_Full(spnum + 1) - Q_H4_Full(spnum + 1)) * ratio);
                    // equation 11.204
                    p_hs = P_H4_Full(spnum + 1) + ((P_H3_Full(spnum + 1) - P_H4_Full(spnum + 1)) * ratio);
                } else if ((t > -8.33) && (t < t_ob)) {
                    Real64 ratio = //((t - (-8.33)) / (1.67 - (-8.33)));
                        (t - HeatingOutdoorCoilInletAirDBTempH3Test) /
                        (HeatingOutdoorCoilInletAirDBTempH2Test - HeatingOutdoorCoilInletAirDBTempH3Test);
                    // equation 11.201
                    q_hs = Q_H3_Full(spnum + 1) + ((Q_H2_Full(spnum + 1) - Q_H3_Full(spnum + 1)) * ratio);
                    // equation 11.202
                    p_hs = P_H3_Full(spnum + 1) + ((P_H2_Full(spnum + 1) - P_H3_Full(spnum + 1)) * ratio);
                } else if ((t >= t_ob) && (t <= -8.33)) {
                    Real64 ratio = // ((t - (-8.33)) / (8.33 - (-8.33)));
                        (t - HeatingOutdoorCoilInletAirDBTempH3Test) /
                        (HeatingOutdoorCoilInletAirDBTempRated - HeatingOutdoorCoilInletAirDBTempH3Test);
                    // equation 11.199
                    q_hs = Q_H3_Full(spnum + 1) + ((Q_H1_Full(spnum + 1) - Q_H3_Full(spnum + 1)) * ratio);
                    // equation 11.200
                    p_hs = P_H3_Full(spnum + 1) + ((P_H1_Full(spnum + 1) - P_H3_Full(spnum + 1)) * ratio);
                }

                Real64 cop_low = q_low / p_low;

                Real64 q_int = q_H2_int + M_Hq * (t - (1.66));
                Real64 p_int = p_H2_int + M_HE * (t - (1.66));
                Real64 cop_int = q_int / p_int;

                Real64 delta_low(0.0);
                if (bl <= q_low) {
                    // CASE 1 : Section 11.2.2.3.1 AHRI-2023
                    // Building Load is less than the capacity of the unit at the Low Compressor Speed (q_low >= bl)
                    if (t <= t_Off || cop_low < 1.0) {
                        delta_low = 0.0; //  Equation 11.159 AHRI-2023
                    } else if (t > t_On) {
                        delta_low = 1.0; // Equation 11.160 AHRI-2023
                    } else {
                        delta_low = 0.5; // Equation 11.161 AHRI-2023
                    }

                    Real64 hlf_low(0.0); // Par tLoad Ratio
                    if (q_low > 0.0) {
                        hlf_low = min(1.0, bl / q_low); // Equation 11.155 AHRI-2023
                    }
                    // Section 6.1.3.2.3 (AHRI-2023) For Variable Capacity Systems, if the optional H1CFull and H1CLow tests are not
                    // performed, a default value of 0.25 shall be used for the heating Degradation Coefficient
                    Real64 CyclicMSHeatingDegradationCoeffHSPF2 = 0.25;
                    // Part Load Fration
                    Real64 plf_low = 1.0 - CyclicMSHeatingDegradationCoeffHSPF2 * (1.0 - hlf_low); // Equation 11.156 AHRI-2023
                    // e = p_low * hlf_low * delta_low * n / plf_low;                                 // Equation 11.153 AHRI-2023
                    e = (hlf_low / plf_low) * p_low * delta_low * n;
                    rh = bl * (1.0 - delta_low) * n; // Equation 11.154 AHRI-2023
                    goto HeatSpeedLoop4_exit;
                } else if (bl < q_hs) { // bl > q_low
                    // (bl > q_low && bl < q_full) {
                    // CASE 2 : 11.2.2.3.2 AHRI-2023
                    // Building load can be matched by modulating the compressor speed between low speed and full speed, q_low < bl < q_full
                    Real64 cop_int_bin(0.0);
                    Real64 delta_int_bin(0.0);
                    if (bl <= q_int) {
                        cop_int_bin = cop_low + ((cop_int - cop_low) / (q_int - q_low)) * (bl - q_low);   // Equation 11.187 AHRI-2023
                    } else {                                                                              // if bl > q_int
                        cop_int_bin = cop_int + ((cop_full - cop_int) / (q_full - q_int)) * (bl - q_int); // Equation 11.188 AHRI-2023
                    }

                    delta_int_bin = 0.0;
                    if (!OATempCompressorOnOffBlank) {
                        if (t <= t_Off) {
                            delta_int_bin = 0.0;
                        } else if (t <= t_On) { // t > t_Off
                            delta_int_bin = 0.5;
                        } else {
                            delta_int_bin = 1.0;
                        }
                    } else {
                        delta_int_bin = 1.0;
                    }

                    rh = bl * (1.0 - delta_int_bin) * n;
                    Real64 q = bl * n;                   // Equation 11.185 AHRI-2023
                    e = q / cop_int_bin * delta_int_bin; // Equaiton 11.186 AHRI-2023
                    goto HeatSpeedLoop4_exit;
                } else if (bl >= q_full) {
                    // CASE 3 : 11.2.2.3.3 AHRI-2023
                    // Building Load is greater than the capacity of the unit at the Full Compressor Speed, q_full <= bl or (bl >= q_full:)
                    if (t <= -15.0) {
                        Real64 t_ratio = (t - (-15.0)) / ((8.33) - (-8.33));
                        // Equation 11.205 AHRI-2023
                        q_full = q_H4_full + (q_H1_full - q_H3_full) * t_ratio;
                        // Equation 11.206 AHRI-2023
                        p_full = p_H4_full + (p_H1_full - p_H3_full) * t_ratio;
                    } else if (t > (-15.0) && t < (-8.33)) {
                        Real64 t_ratio = (t - (-15.0)) / (-8.33 - (-15.0));
                        // Equation 11.203 AHRI-2023
                        q_full = q_H4_full + (q_H3_full - q_H4_full) * t_ratio;
                        // Equation 11.204 AHRI-2023
                        p_full = p_H4_full + (p_H3_full - p_H4_full) * t_ratio;
                    } else if (t > (-8.33) && t < t_ob) {
                        Real64 t_ratio = (t - (-8.33)) / (1.67 - (-8.33));
                        // Equation 11.201 AHRI-2023
                        q_full = q_H3_full + (q_H2_full - q_H3_full) * t_ratio;
                        // Equation 11.202 AHRI-2023
                        p_full = p_H3_full + (p_H2_full - p_H3_full) * t_ratio;
                    } else if (t >= t_ob || t == (-8.33)) {
                        Real64 t_ratio = (t - (-8.33)) / (8.33 - (-8.33));
                        // Equation 11.199 AHRI-2023
                        q_full = q_H3_full + (q_H1_full - q_H3_full) * t_ratio;
                        // Equation 11.200 AHRI-2023
                        p_full = p_H3_full + (p_H1_full - p_H3_full) * t_ratio;
                    }

                    // if not conducting H4 Test then use this block
                    // if (t >= t_ob || t <= (-8.33)) {
                    //     Real64 t_ratio = (t - (-8.33)) / ((8.33) - (-8.33));
                    //     // Equation 11.199 AHRI-2023
                    //     q_full = q_H3_full + (q_H1_full - q_H3_full) * t_ratio;
                    //     // Equation 11.200 AHRI-2023
                    //     p_full = p_H3_full + (p_H1_full - p_H3_full) * t_ratio;
                    // } else if ((-8.33) < t && t < t_ob) {
                    //     Real64 t_ratio = (t - (-8.33)) / (1.66 - (-8.33));
                    //     // Equation 11.201 AHRI-2023
                    //     q_full = q_H3_full + (q_H2_full - q_H3_full) * t_ratio;
                    //     // Equation 11.202 AHRI-2023
                    //     p_full = p_H3_full + (p_H2_full - p_H3_full) * t_ratio;
                    // }

                    if (!OATempCompressorOnOffBlank && p_full > 0.0) {
                        if ((t <= OATempCompressorOff) || (q_full / p_full < 1.0)) {
                            delta_full = 0.0;
                        } else if ((t <= OATempCompressorOn) && (q_full / p_full > 1.0)) { // t > OATempCompressorOff
                            delta_full = 0.5;
                        } else if ((t > OATempCompressorOn) && (q_full / p_full > 1.0)) {
                            delta_full = 1.0;
                        }
                    } else {
                        delta_full = 1.0;
                    }

                    hlf_full = 1.0; // Equation 11.170 AHRI-2023

                    // Equaiton 11.168 AHRI-2023
                    e = p_full * hlf_full * delta_full * n;
                    // Equation 11.169 AHRI - 2023
                    rh = (bl - q_full * hlf_full * delta_full) * n;
                    goto HeatSpeedLoop4_exit;
                }
            }

        HeatSpeedLoop4_exit:;
            q_sum += n * bl;
            e_sum += e;
            rh_sum += rh;
        }

        Real64 defrost_period = 90;   // Time between defrost terminations (for testing) in minutes
        Real64 defrost_MaxTime = 720; // Maximum time between defrosts allowed by controls in minutes
        Real64 t_Test_90 = max(defrost_period, 90.00);
        Real64 t_Max_90 = min(defrost_MaxTime, 720.00);
        // default t_Test_90/t_Max_90 = 0.125
        if (DefrostControl == HPdefrostControl::Timed) {
            // Equation 11.106 AHRI-2023
            f_def = 1.0; // Timed defrost control
        } else {
            // Equation 11.105 AHRI-2023
            f_def = 1 + 0.03 * (1 - (t_Test_90 / t_Max_90)); // Demand defrost control
            // f_def = 1.03;
        }

        if (e_sum > 0.0) {
            HSPF2_2023 = (q_sum / (e_sum + rh_sum)) * f_def; // Equation 11.103
        } else {
            HSPF2_2023 = 0.0;
        }

        return std::make_tuple(NetHeatingCapRatedHighTemp_2023, NetHeatingCapRatedLowTemp_2023, HSPF2_2023);
    }

    std::map<std::string, Real64> MultiSpeedDXHeatingCoilStandardRatings(
        EnergyPlusData &state,
        [[maybe_unused]] std::string const &DXCoilName,               // Name of DX coil for which HSPF is calculated
        [[maybe_unused]] std::string const &DXCoilType,               // Type of DX coil for which HSPF is calculated
        Array1A_int const CapFTempCurveIndex,                         // Index for the capacity as a function of temperature modifier curve
        Array1A_int const CapFFlowCurveIndex,                         // Index for the capacity as a function of flow fraction modifier curve
        Array1A_int const EIRFTempCurveIndex,                         // Index for the EIR as a function of temperature modifier curve
        Array1A_int const EIRFFlowCurveIndex,                         // Index for the EIR as a function of flow fraction modifier curve
        Array1A_int const PLFFPLRCurveIndex,                          // Index for the PLF vs part-load ratio curve
        Array1A<Real64> const RatedTotalCapacity,                     // Reference capacity of DX coil [W]
        Array1A<Real64> const RatedCOP,                               // Reference coefficient of performance [W/W]
        Array1A<Real64> const RatedAirVolFlowRate,                    // Reference air flow rate of DX coil [m3/s]
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRateInput,      // 2017 rated fan power per evap air flow rate [W/(m3/s)]
        Array1A<Real64> const MSFanPowerPerEvapAirFlowRateInput_2023, // 2023 rated fan power per evap air flow rate [W/(m3/s)]
        int const nsp,                                                // Number of compressor speeds
        ObjexxFCL::Optional_int_const RegionNum,                      // Region number for calculating HSPF of single speed DX heating coil
        ObjexxFCL::Optional<Real64 const> MinOATCompressor,           // Minimum OAT for heat pump compressor operation [C]
        ObjexxFCL::Optional<Real64 const> OATempCompressorOn,         // The outdoor temperature when the compressor is automatically turned
        ObjexxFCL::Optional_bool_const OATempCompressorOnOffBlank,    // Flag used to determine low temperature cut out factor
        ObjexxFCL::Optional<HPdefrostControl const> DefrostControl    // defrost control; 1=timed, 2=on-demand
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Nigusse, FSEC
        //       DATE WRITTEN   December 2012
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the standard ratings net heating capacity and HSPF values for multi speed DX heating coils
        // at the AHRI standard test condition(s).

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using Curve::CurveValue;

        // Argument array dimensioning
        CapFTempCurveIndex.dim(nsp);
        CapFFlowCurveIndex.dim(nsp);
        EIRFTempCurveIndex.dim(nsp);
        EIRFFlowCurveIndex.dim(nsp);
        PLFFPLRCurveIndex.dim(nsp);
        RatedTotalCapacity.dim(nsp);
        RatedCOP.dim(nsp);
        RatedAirVolFlowRate.dim(nsp);
        MSFanPowerPerEvapAirFlowRateInput.dim(nsp);
        MSFanPowerPerEvapAirFlowRateInput_2023.dim(nsp);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // back on, if applicable, following automatic shut off. This field is
        // used only for HSPF calculation. [C]

        // SUBROUTINE PARAMETER DEFINITIONS:
        // CHARACTER(len=*), PARAMETER      :: RoutineName='MultiSpeedDXHeatingCoilStandardRatings: ' ! Include trailing blank space

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 NetHeatingCapRatedHighTemp = 0.0; // net heating capacity at maximum speed and High Temp
        Real64 NetHeatingCapRatedLowTemp = 0.0;  // net heating capacity at maximum speed and low Temp
        Real64 HSPF = 0.0;                       // seasonale energy efficiency ratio of multi speed DX cooling coil

        Real64 NetHeatingCapRatedHighTemp_2023 = 0.0; // net heating capacity at maximum speed and High Temp
        Real64 NetHeatingCapRatedLowTemp_2023 = 0.0;  // net heating capacity at maximum speed and low Temp
        Real64 HSPF2_2023 = 0.0;                      // seasonale energy efficiency ratio of multi speed DX cooling coil

        std::map<std::string, Real64> StandardRatingsResult;
        // StandardRatingsResult["NetHeatingCapRatedHighTemp"] = NetHeatingCapRatedHighTemp;
        // StandardRatingsResult["NetHeatingCapRatedLowTemp"] = NetHeatingCapRatedLowTemp;
        // StandardRatingsResult["HSPF"] = HSPF;
        // StandardRatingsResult["NetHeatingCapRatedHighTemp_2023"] = NetHeatingCapRatedHighTemp_2023;
        // StandardRatingsResult["NetHeatingCapRatedLowTemp_2023"] = NetHeatingCapRatedLowTemp_2023;
        // StandardRatingsResult["HSPF2_2023"] = HSPF2_2023;

        // HSPF Calculation | AHRI 2017 Std.
        std::tie(NetHeatingCapRatedHighTemp, NetHeatingCapRatedLowTemp, HSPF) = MultiSpeedDXHeatingCoilHSPF(state,
                                                                                                            nsp,
                                                                                                            MSFanPowerPerEvapAirFlowRateInput,
                                                                                                            CapFTempCurveIndex,
                                                                                                            CapFFlowCurveIndex,
                                                                                                            RatedTotalCapacity,
                                                                                                            RatedAirVolFlowRate,
                                                                                                            EIRFFlowCurveIndex,
                                                                                                            EIRFTempCurveIndex,
                                                                                                            RatedCOP,
                                                                                                            RegionNum,
                                                                                                            MinOATCompressor,
                                                                                                            OATempCompressorOn,
                                                                                                            OATempCompressorOnOffBlank,
                                                                                                            DefrostControl);

        StandardRatingsResult["NetHeatingCapRatedHighTemp"] = NetHeatingCapRatedHighTemp;
        StandardRatingsResult["NetHeatingCapRatedLowTemp"] = NetHeatingCapRatedLowTemp;
        StandardRatingsResult["HSPF"] = HSPF;

        // HSPF2 Calculation | AHRI 2023 Std.
        std::tie(NetHeatingCapRatedHighTemp_2023, NetHeatingCapRatedLowTemp_2023, HSPF2_2023) =
            MultiSpeedDXHeatingCoilHSPF2(state,
                                         nsp,
                                         MSFanPowerPerEvapAirFlowRateInput_2023,
                                         CapFTempCurveIndex,
                                         CapFFlowCurveIndex,
                                         RatedTotalCapacity,
                                         RatedAirVolFlowRate,
                                         EIRFFlowCurveIndex,
                                         EIRFTempCurveIndex,
                                         RatedCOP,
                                         RegionNum,
                                         MinOATCompressor,
                                         OATempCompressorOn,
                                         OATempCompressorOnOffBlank,
                                         DefrostControl);

        StandardRatingsResult["NetHeatingCapRatedHighTemp_2023"] = NetHeatingCapRatedHighTemp_2023;
        StandardRatingsResult["NetHeatingCapRatedLowTemp_2023"] = NetHeatingCapRatedLowTemp_2023;
        StandardRatingsResult["HSPF2_2023"] = HSPF2_2023;

        return StandardRatingsResult;
    }

    void ReportDXCoilRating(EnergyPlusData &state,
                            std::string const &CompType,    // Type of component
                            std::string_view CompName,      // Name of component
                            int const CompTypeNum,          // TypeNum of component
                            Real64 const CoolCapVal,        // Standard total (net) cooling capacity for AHRI Std. 210/240 {W}
                            Real64 const SEERUserIP,        // SEER value in IP units from user PLR curve {Btu/W-h}
                            Real64 const SEERStandardIP,    // SEER value in IP units from AHRI Std 210/240-2008 default PLF curve and C_D {Btu/W-h}
                            Real64 const EERValueSI,        // EER value in SI units {W/W}
                            Real64 const EERValueIP,        // EER value in IP units {Btu/W-h}
                            Real64 const IEERValueIP,       // IEER value in IP units {Btu/W-h}
                            Real64 const HighHeatingCapVal, // High Temperature Heating Standard (Net) Rating Capacity
                            Real64 const LowHeatingCapVal,  // Low Temperature Heating Standard (Net) Rating Capacity
                            Real64 const HSPFValueIP,       // IEER value in IP units {Btu/W-h}
                            int const RegionNum,            // Region Number for which HSPF is calculated
                            bool const AHRI2023StandardRatings) // True if required AHRI/ANSI 210/240 Std. 2023 SEER2,HSPF2 Ratings.
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, Chandan Sharma
        //       DATE WRITTEN   February 2010
        //       MODIFIED       May 2010 (Added EER and IEER entries)
        //                      March 2012 (Added HSPF and High/Low Heating Capacity entries)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the standard rating (net) cooling capacity, SEER, EER and IEER values to
        // the "eio" and tabular output files for Single Speed compressor DX Cooling Coils.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing

        using namespace OutputReportPredefined;
        using HVAC::Coil_CoolingAirToAirVariableSpeed;
        using HVAC::CoilDX_CoolingSingleSpeed;
        using HVAC::CoilDX_CoolingTwoSpeed;
        using HVAC::CoilDX_HeatingEmpirical;
        using HVAC::CoilDX_MultiSpeedCooling;
        using HVAC::CoilDX_MultiSpeedHeating;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        //   or ANSI/AHRI Std. 340/360 {W}

        // for AHRI Std. 210/240 {W}
        // for AHRI Std. 210/240 {W}

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        switch (CompTypeNum) {

        case CoilDX_CoolingSingleSpeed: {
            if (!AHRI2023StandardRatings) {
                if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag) {
                    static constexpr std::string_view Format_994(
                        "! <DX Cooling Coil Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                        "Cooling Capacity {W}, Standard Rating Net COP {W/W}, EER {Btu/W-h}, SEER User {Btu/W-h}, SEER Standard {Btu/W-h}, "
                        "IEER "
                        "{Btu/W-h}");
                    print(state.files.eio, "{}\n", Format_994);
                    state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag = false;
                }

                static constexpr std::string_view Format_991(
                    " DX Cooling Coil Standard Rating Information, {}, {}, {:.1R}, {:.2R}, {:.2R}, {:.2R}, {:.2R}, {:.1R}\n");
                print(state.files.eio, Format_991, CompType, CompName, CoolCapVal, EERValueSI, EERValueIP, SEERUserIP, SEERStandardIP, IEERValueIP);

                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType, CompName, CompType);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSI, CompName, CoolCapVal, 1);
                // W/W is the same as Btuh/Btuh so that's fine too
                if (EERValueSI > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP, CompName, EERValueSI, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP, CompName, "N/A");
                }
                // Btu/W-h will convert to itself
                if (EERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP, CompName, EERValueIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP, CompName, "N/A");
                }
                if (SEERUserIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERUserIP, CompName, SEERUserIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERUserIP, CompName, "N/A");
                }
                if (SEERStandardIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERStandardIP, CompName, SEERStandardIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERStandardIP, CompName, "N/A");
                }
                if (IEERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP, CompName, IEERValueIP, 1);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP, CompName, "N/A");
                }
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstDXCoolCoil, StandardRatings::AHRI2017FOOTNOTE);
            } else {
                // ANSI/AHRI 210/240 Standard 2023 Ratings | SEER2
                if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag2) {
                    static constexpr std::string_view Format_991_(
                        "! <DX Cooling Coil AHRI 2023 Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                        "Cooling Capacity {W}, Standard Rating Net COP2 {W/W}, EER2 {Btu/W-h}, SEER2 User {Btu/W-h}, SEER2 Standard "
                        "{Btu/W-h}, "
                        "IEER 2022 "
                        "{Btu/W-h}");
                    print(state.files.eio, "{}\n", Format_991_);
                    state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag2 = false;
                }

                static constexpr std::string_view Format_991_(
                    " DX Cooling Coil AHRI 2023 Standard Rating Information, {}, {}, {:.1R}, {:.2R}, {:.2R}, {:.2R}, {:.2R}, {:.1R}\n");
                print(state.files.eio, Format_991_, CompType, CompName, CoolCapVal, EERValueSI, EERValueIP, SEERUserIP, SEERStandardIP, IEERValueIP);

                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType_2023, CompName, CompType);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSI_2023, CompName, CoolCapVal, 1);
                // W/W is the same as Btuh/Btuh so that's fine too
                if (EERValueSI > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP_2023, CompName, EERValueSI, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP_2023, CompName, "N/A");
                }
                // Btu/W-h will convert to itself
                if (EERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP_2023, CompName, EERValueIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP_2023, CompName, "N/A");
                }
                if (SEERUserIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2UserIP_2023, CompName, SEERUserIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2UserIP_2023, CompName, "N/A");
                }
                if (SEERStandardIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2StandardIP_2023, CompName, SEERStandardIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2StandardIP_2023, CompName, "N/A");
                }
                if (IEERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP_2023, CompName, IEERValueIP, 1);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP_2023, CompName, "N/A");
                }
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstDXCoolCoil_2023, StandardRatings::AHRI2023FOOTNOTE);
            }
            break;
        }
        case CoilDX_HeatingEmpirical:
        case CoilDX_MultiSpeedHeating: {
            if (!AHRI2023StandardRatings) {
                if (state.dataHVACGlobal->StandardRatingsMyHeatOneTimeFlag) {
                    static constexpr std::string_view Format_992(
                        "! <DX Heating Coil Standard Rating Information>, Component Type, Component Name, High Temperature Heating "
                        "(net) Rating Capacity {W}, Low Temperature Heating (net) Rating Capacity {W}, HSPF {Btu/W-h}, Region "
                        "Number\n");
                    print(state.files.eio, "{}", Format_992);
                    state.dataHVACGlobal->StandardRatingsMyHeatOneTimeFlag = false;
                }

                static constexpr std::string_view Format_993(" DX Heating Coil Standard Rating Information, {}, {}, {:.1R}, {:.1R}, {:.2R}, {}\n");
                print(state.files.eio, Format_993, CompType, CompName, HighHeatingCapVal, LowHeatingCapVal, HSPFValueIP, RegionNum);

                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilType, CompName, CompType);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilHighCap, CompName, HighHeatingCapVal, 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilLowCap, CompName, LowHeatingCapVal, 1);
                // Btu/W-h will convert to itself
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilHSPFIP, CompName, HSPFValueIP, 2);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilRegionNum, CompName, RegionNum);
                addFootNoteSubTable(
                    state, state.dataOutRptPredefined->pdstDXHeatCoil, "ANSI/AHRI ratings account for supply air fan heat and electric power.");
            } else {
                // ANSI/AHRI 210/240 Standard 2023 Ratings | HSPF2
                if (state.dataHVACGlobal->StandardRatingsMyHeatOneTimeFlag2) {
                    static constexpr std::string_view Format_992_(
                        "! <DX Heating Coil AHRI 2023 Standard Rating Information>, Component Type, Component Name, High Temperature Heating "
                        "(net) Rating Capacity {W}, Low Temperature Heating (net) Rating Capacity {W}, HSPF2 {Btu/W-h}, Region "
                        "Number\n");
                    print(state.files.eio, "{}", Format_992_);
                    state.dataHVACGlobal->StandardRatingsMyHeatOneTimeFlag2 = false;
                }

                static constexpr std::string_view Format_993_(
                    " DX Heating Coil AHRI 2023 Standard Rating Information, {}, {}, {:.1R}, {:.1R}, {:.2R}, {}\n");
                print(state.files.eio, Format_993_, CompType, CompName, HighHeatingCapVal, LowHeatingCapVal, HSPFValueIP, RegionNum);

                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilType_2023, CompName, CompType);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilHighCap_2023, CompName, HighHeatingCapVal, 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilLowCap_2023, CompName, LowHeatingCapVal, 1);
                // Btu/W-h will convert to itself
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilHSPF2IP_2023, CompName, HSPFValueIP, 2);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXHeatCoilRegionNum_2023, CompName, RegionNum);
                addFootNoteSubTable(state,
                                    state.dataOutRptPredefined->pdstDXHeatCoil_2023,
                                    "ANSI/AHRI 2023 (HSPF2) ratings account for supply air fan heat and electric power.");
            }
            break;
        }
        case CoilDX_CoolingTwoSpeed:
        case Coil_CoolingAirToAirVariableSpeed:
        case CoilDX_MultiSpeedCooling: {
            if (!AHRI2023StandardRatings) {
                if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag) {
                    static constexpr std::string_view Format_994(
                        "! <DX Cooling Coil Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                        "Cooling Capacity {W}, Standard Rating Net COP {W/W}, EER {Btu/W-h}, SEER User {Btu/W-h}, SEER Standard {Btu/W-h}, "
                        "IEER "
                        "{Btu/W-h}");
                    print(state.files.eio, "{}\n", Format_994);
                    state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag = false;
                }

                static constexpr std::string_view Format_995(
                    " DX Cooling Coil Standard Rating Information, {}, {}, {:.1R}, {:.2R}, {:.2R}, {:.2R}, {:.2R}, {}\n");
                print(state.files.eio, Format_995, CompType, CompName, CoolCapVal, EERValueSI, EERValueIP, SEERUserIP, SEERStandardIP, IEERValueIP);

                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType, CompName, CompType);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSI, CompName, CoolCapVal, 1);
                if (EERValueSI > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP, CompName, EERValueSI, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP, CompName, "N/A");
                }
                if (EERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP, CompName, EERValueIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP, CompName, "N/A");
                }
                if (SEERUserIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERUserIP, CompName, SEERUserIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERUserIP, CompName, "N/A");
                }
                if (SEERStandardIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERStandardIP, CompName, SEERStandardIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEERStandardIP, CompName, "N/A");
                }
                if (IEERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP, CompName, IEERValueIP, 1);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP, CompName, "N/A");
                }
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstDXCoolCoil, StandardRatings::AHRI2017FOOTNOTE);
            } else {
                // ANSI/AHRI 210/240 Standard 2023 Ratings | SEER2
                if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag2) {
                    static constexpr std::string_view Format_991_(
                        "! <DX Cooling Coil AHRI 2023 Standard Rating Information>, Component Type, Component Name, Standard Rating (Net) "
                        "Cooling Capacity {W}, Standard Rating Net COP2 {W/W}, EER2 {Btu/W-h}, SEER2 User {Btu/W-h}, SEER2 Standard "
                        "{Btu/W-h}, "
                        "IEER 2022 "
                        "{Btu/W-h}");
                    print(state.files.eio, "{}\n", Format_991_);
                    state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag2 = false;
                }

                static constexpr std::string_view Format_995_(
                    " DX Cooling Coil AHRI 2023 Standard Rating Information, {}, {}, {:.1R}, {:.2R}, {:.2R}, {:.2R}, {:.2R}, {:.1R}\n");
                print(state.files.eio, Format_995_, CompType, CompName, CoolCapVal, EERValueSI, EERValueIP, SEERUserIP, SEERStandardIP, IEERValueIP);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType_2023, CompName, CompType);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSI_2023, CompName, CoolCapVal, 1);
                // W/W is the same as Btuh/Btuh so that's fine too
                if (EERValueSI > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP_2023, CompName, EERValueSI, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilCOP_2023, CompName, "N/A");
                }
                // Btu/W-h will convert to itself
                if (EERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP_2023, CompName, EERValueIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilEERIP_2023, CompName, "N/A");
                }
                if (SEERUserIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2UserIP_2023, CompName, SEERUserIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2UserIP_2023, CompName, "N/A");
                }
                if (SEERStandardIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2StandardIP_2023, CompName, SEERStandardIP, 2);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilSEER2StandardIP_2023, CompName, "N/A");
                }
                if (IEERValueIP > 0.0) {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP_2023, CompName, IEERValueIP, 1);
                } else {
                    PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilIEERIP_2023, CompName, "N/A");
                }
                addFootNoteSubTable(state, state.dataOutRptPredefined->pdstDXCoolCoil_2023, StandardRatings::AHRI2023FOOTNOTE);
            }

            break;
        }
        default:
            break;
        }
    }

    void ReportDXCoolCoilDataCenterApplication(EnergyPlusData &state,
                                               std::string const &CompType,           // Type of component
                                               std::string_view CompName,             // Name of component
                                               int const CompTypeNum,                 // TypeNum of component
                                               Array1D<Real64> &NetCoolingCapRated,   // net cooling capacity of single speed DX cooling coil
                                               Array1D<Real64> &TotElectricPowerRated // total electric power including supply fan
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse
        //       DATE WRITTEN   October 2014
        //       MODIFIED       na
        //
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine writes the standard rating (net) cooling capacity and Electric Power for
        // for room unitary air conditioners single speed DX cooling coils to the "eio" and tabular
        // output files.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // ANSI/ASHRAE Standard 127-2012 - Method of Testing for Rating Computer and Data Processing
        //                                 Room Unitary Air Conditioners

        // Using/Aliasing

        using namespace OutputReportPredefined;
        using HVAC::CoilDX_CoolingSingleSpeed;

        if (CompTypeNum == CoilDX_CoolingSingleSpeed) {
            if (state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag3) {
                static constexpr std::string_view Format_101(
                    "! <DX Cooling Coil ASHRAE 127 Standard Ratings Information>, Component Type, Component Name, Standard 127 "
                    "Classification, Rated Net Cooling Capacity Test A {W}, Rated Total Electric Power Test A {W}, Rated Net "
                    "Cooling Capacity Test B {W}, Rated Total Electric Power Test B {W}, Rated Net Cooling Capacity Test C {W}, "
                    "Rated Total Electric Power Test C {W}, Rated Net Cooling Capacity Test D {W}, Rated Total Electric "
                    "Power Test D {W} \n");
                print(state.files.eio, "{}", Format_101);
                state.dataHVACGlobal->StandardRatingsMyCoolOneTimeFlag3 = false;
            }
            for (int ClassNum = 1; ClassNum <= 4; ++ClassNum) {
                int Num = (ClassNum - 1) * 4;
                std::string ClassName = format("Class {}", ClassNum);
                std::string CompNameNew = fmt::format("{}({})", CompName, ClassName);
                static constexpr std::string_view Format_102(
                    " DX Cooling Coil ASHRAE 127 Standard Ratings Information, {}, {}, {}, {:.1R}, {:.1R}, {:.1R}, "
                    "{:.1R}, {:.1R}, {:.1R}, {:.1R}, {:.1R}\n");
                print(state.files.eio,
                      Format_102,
                      CompType,
                      CompName,
                      ClassName,
                      NetCoolingCapRated(Num + 1),
                      TotElectricPowerRated(Num + 1),
                      NetCoolingCapRated(Num + 2),
                      TotElectricPowerRated(Num + 2),
                      NetCoolingCapRated(Num + 3),
                      TotElectricPowerRated(Num + 3),
                      NetCoolingCapRated(Num + 4),
                      TotElectricPowerRated(Num + 4));
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilType2, CompNameNew, CompType);
                // Note: If you call format("{:.1R}", NetCoolingCapRated(Num + 1)),
                // Then it's not the OutputReportPredefined::PreDefTableEntry prototype with Real64 that is called.
                // As a result, the entry isn't marked as being Real (origEntryIsReal) and unit conversion does not occur
                // Bad: PreDefTableEntry(state, pdchDXCoolCoilNetCapSIA, CompNameNew, format("{:.1R}", NetCoolingCapRated(Num + 1)));
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSIA, CompNameNew, NetCoolingCapRated(Num + 1), 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSIB, CompNameNew, NetCoolingCapRated(Num + 2), 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSIC, CompNameNew, NetCoolingCapRated(Num + 3), 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilNetCapSID, CompNameNew, NetCoolingCapRated(Num + 4), 1);

                // These will stay in W, so it doesn't matter as much, but let's be consistent
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilElecPowerA, CompNameNew, TotElectricPowerRated(Num + 1), 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilElecPowerB, CompNameNew, TotElectricPowerRated(Num + 2), 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilElecPowerC, CompNameNew, TotElectricPowerRated(Num + 3), 1);
                PreDefTableEntry(state, state.dataOutRptPredefined->pdchDXCoolCoilElecPowerD, CompNameNew, TotElectricPowerRated(Num + 4), 1);

                addFootNoteSubTable(state,
                                    state.dataOutRptPredefined->pdstDXCoolCoil2,
                                    "ANSI/ASHRAE Standard 127 includes supply fan heat effect and electric power.");
            }
        }
    }

    void CheckCurveLimitsForStandardRatings(EnergyPlusData &state,
                                            std::string const &DXCoilName, // Name of DX coil for which HSPF is calculated
                                            std::string const &DXCoilType, // Type of DX coil - heating or cooling
                                            int const DXCoilTypeNum,       // Integer type of DX coil - heating or cooling
                                            int const CapFTempCurveIndex,  // Index for the capacity as a function of temperature modifier curve
                                            int const CapFFlowCurveIndex,  // Index for the capacity as a function of flow fraction modifier curve
                                            int const EIRFTempCurveIndex,  // Index for the EIR as a function of temperature modifier curve
                                            int const EIRFFlowCurveIndex,  // Index for the EIR as a function of flow fraction modifier curve
                                            int const PLFFPLRCurveIndex    // Index for the EIR vs part-load ratio curve
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         D. Shirey/B. Nigusse, FSEC
        //       DATE WRITTEN   May 2010
        //       MODIFIED       Chandan Sharma, March 2012
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Checks the limits of the various curves used in DXCoil and returns .FALSE. if the limits do not include
        // the standard test condition(s).

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using Curve::CurveValue;
        using Curve::GetCurveIndex;
        using Curve::GetCurveMinMaxValues;
        using Curve::GetCurveName;
        using HVAC::CoilDX_CoolingSingleSpeed;
        using HVAC::CoilDX_HeatingEmpirical;
        using HVAC::CoilDX_MultiSpeedCooling;
        using HVAC::CoilDX_MultiSpeedHeating;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        static constexpr std::string_view RoutineName("CheckCurveLimitsForStandardRatings: "); // Include trailing blank space

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        //  Minimum and Maximum independent variable limits from Total Cooling Capacity Function of Temperature Curve
        Real64 CapacityWBTempMin(0.0); // Capacity modifier Min value (wet bulb temperature), from the Curve:BiQuadratic object
        Real64 CapacityWBTempMax(0.0); // Capacity modifier Max value (wet bulb temperature), from the Curve:BiQuadratic object
        Real64 CapacityDBTempMin(0.0); // Capacity modifier Min value (dry bulb temperature), from the Curve:BiQuadratic object
        Real64 CapacityDBTempMax(0.0); // Capacity modifier Max value (dry bulb temperature), from the Curve:BiQuadratic object

        //  Minimum and Maximum independent variable limits from Energy Input Ratio (EIR) Function of Temperature Curve
        Real64 EIRWBTempMin(0.0); // EIR modifier Min value (wet bulb temperature), from the Curve:BiQuadratic object
        Real64 EIRWBTempMax(0.0); // EIR modifier Max value (wet bulb temperature), from the Curve:BiQuadratic object
        Real64 EIRDBTempMin(0.0); // EIR modifier Min value (dry bulb temperature), from the Curve:BiQuadratic object
        Real64 EIRDBTempMax(0.0); // EIR modifier Max value (dry bulb temperature), from the Curve:BiQuadratic object

        //  Minimum and Maximum independent variable limits from Part Load Fraction Correlation Curve
        Real64 PLFFPLRMin(0.0); // Maximum value for Part Load Ratio, from the corresponding curve object
        Real64 PLFFPLRMax(0.0); // Minimum value for Part Load Ratio, from the corresponding curve object

        //  Minimum and Maximum independent variable limits from Total Cooling Capacity Function of Flow Fraction Curve
        Real64 CapacityFlowRatioMin(0.0); // Minimum value for flow fraction, from the corresponding curve object
        Real64 CapacityFlowRatioMax(0.0); // Maximum value for flow fraction, from the corresponding curve object

        //  Minimum and Maximum independent variable limits from Energy Input Ratio Function of Flow Fraction Curve
        Real64 EIRFlowRatioMin(0.0); // Minimum value for flow fraction, from the corresponding curve object
        Real64 EIRFlowRatioMax(0.0); // Maximum value for flow fraction, from the corresponding curve object

        //  Minimum and Maximum independent variable limits from Total Cooling Capacity Function of Temperature Curve
        Real64 HeatingCapODBTempMin(0.0); // Capacity modifier Min value (outdoor dry bulb temperature)
        Real64 HeatingCapODBTempMax(0.0); // Capacity modifier Max value (outdoor dry bulb temperature)
        Real64 HeatingCapIDBTempMin(0.0); // Capacity modifier Min value (indoor dry bulb temperature)
        Real64 HeatingCapIDBTempMax(0.0); // Capacity modifier Max value (indoor dry bulb temperature)

        //  Minimum and Maximum independent variable limits from Energy Input Ratio (EIR) Function of Temperature Curve
        Real64 HeatingEIRODBTempMin(0.0); // EIR modifier Min value (outdoor dry bulb temperature)
        Real64 HeatingEIRODBTempMax(0.0); // EIR modifier Max value (outdoor dry bulb temperature)
        Real64 HeatingEIRIDBTempMin(0.0); // EIR modifier Min value (indoor dry bulb temperature)
        Real64 HeatingEIRIDBTempMax(0.0); // EIR modifier Max value (indoor dry bulb temperature)

        bool CapCurveHighOATLimitsExceeded(false); // Logical for capacity curve temperature limits being exceeded (high temp)
        bool CapCurveFlowLimitsExceeded(false);    // Logical for capacity curve flow fraction limits being exceeded
        bool EIRCurveHighOATLimitsExceeded(false); // Logical for EIR curve temperature limits being exceeded (high temp)
        bool EIRCurveFlowLimitsExceeded(false);    // Logical for EIR curve flow fraction limits being exceeded

        bool HeatingCapCurveHSPFLimitsExceeded(false); // Logical for capacity curve temperature limits being exceeded
        // (HSPF calcs)
        bool HeatingEIRCurveHSPFLimitsExceeded(false); // Logical for EIR curve temperature limits being exceeded
        // (HSPF calcs)

        switch (DXCoilTypeNum) {

        case CoilDX_CoolingSingleSpeed: {
            bool CapCurveMidOATLimitsExceeded = false; // Logical for capacity curve temperature limits being exceeded (mid temp)
            bool EIRCurveMidOATLimitsExceeded = false; // Logical for EIR curve temperature limits being exceeded (mid temp)
            bool PLFfPLRforSEERLimitsExceeded = false; // Logical for PLF function of PLR limits being exceeded
            bool CapCurveIEERLimitsExceeded = false;   // Logical for capacity curve temperature limits being exceeded (IEER calcs)
            bool EIRCurveIEERLimitsExceeded = false;   // Logical for EIR temperature limits being exceeded (IEER calcs)

            GetCurveMinMaxValues(state, CapFTempCurveIndex, CapacityWBTempMin, CapacityWBTempMax, CapacityDBTempMin, CapacityDBTempMax);
            GetCurveMinMaxValues(state, EIRFTempCurveIndex, EIRWBTempMin, EIRWBTempMax, EIRDBTempMin, EIRDBTempMax);
            GetCurveMinMaxValues(state, CapFFlowCurveIndex, CapacityFlowRatioMin, CapacityFlowRatioMax);
            GetCurveMinMaxValues(state, EIRFFlowCurveIndex, EIRFlowRatioMin, EIRFlowRatioMax);
            GetCurveMinMaxValues(state, PLFFPLRCurveIndex, PLFFPLRMin, PLFFPLRMax);

            // Checking the limits of capacity modifying curve for temperatures
            if (CapacityDBTempMax < OutdoorCoilInletAirDryBulbTempRated || CapacityDBTempMin > OutdoorCoilInletAirDryBulbTempRated ||
                CapacityWBTempMax < CoolingCoilInletAirWetBulbTempRated || CapacityWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                CapCurveHighOATLimitsExceeded = true;
            }
            // Checking the limits of capacity modifying curve for flow fraction
            if (CapacityFlowRatioMax < AirMassFlowRatioRated || CapacityFlowRatioMin > AirMassFlowRatioRated) {
                CapCurveFlowLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for temperatures
            if (EIRDBTempMax < OutdoorCoilInletAirDryBulbTempRated || EIRDBTempMin > OutdoorCoilInletAirDryBulbTempRated ||
                EIRWBTempMax < CoolingCoilInletAirWetBulbTempRated || EIRWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                EIRCurveHighOATLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for flow fraction
            if (EIRFlowRatioMax < AirMassFlowRatioRated || EIRFlowRatioMin > AirMassFlowRatioRated) {
                EIRCurveFlowLimitsExceeded = true;
            }
            // Checking the limits of capacity modifying curve for temperatures (SEER calculation)
            if (CapacityDBTempMax < OutdoorCoilInletAirDryBulbTempTestB2 || CapacityDBTempMin > OutdoorCoilInletAirDryBulbTempTestB2 ||
                CapacityWBTempMax < CoolingCoilInletAirWetBulbTempRated || CapacityWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                CapCurveMidOATLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for temperatures (SEER calculation)
            if (EIRDBTempMax < OutdoorCoilInletAirDryBulbTempTestB2 || EIRDBTempMin > OutdoorCoilInletAirDryBulbTempTestB2 ||
                EIRWBTempMax < CoolingCoilInletAirWetBulbTempRated || EIRWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                EIRCurveMidOATLimitsExceeded = true;
            }
            // Checking the limits of Part Load Fraction for PLR (SEER calculation)
            if (PLFFPLRMax < PLRforSEER || PLFFPLRMin > PLRforSEER) {
                PLFfPLRforSEERLimitsExceeded = true;
            }
            // Checking the limits of capacity modifying curve for temperatures (IEER high and low test conditions)
            if (CapacityDBTempMax < OutdoorCoilInletAirDryBulbTempRated || CapacityDBTempMin > OADBTempLowReducedCapacityTest ||
                CapacityWBTempMax < CoolingCoilInletAirWetBulbTempRated || CapacityWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                CapCurveIEERLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for temperatures (IEER high and low test conditions)
            if (EIRDBTempMax < OutdoorCoilInletAirDryBulbTempRated || EIRDBTempMin > OADBTempLowReducedCapacityTest ||
                EIRWBTempMax < CoolingCoilInletAirWetBulbTempRated || EIRWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                EIRCurveIEERLimitsExceeded = true;
            }

            if (CapCurveHighOATLimitsExceeded || CapCurveFlowLimitsExceeded || EIRCurveHighOATLimitsExceeded || EIRCurveFlowLimitsExceeded ||
                CapCurveMidOATLimitsExceeded || EIRCurveMidOATLimitsExceeded || PLFfPLRforSEERLimitsExceeded || CapCurveIEERLimitsExceeded ||
                EIRCurveIEERLimitsExceeded) {

                ShowWarningError(
                    state,
                    format("The Standard Ratings is calculated for {} = {} but not at the AHRI test condition due to curve out of bound.",
                           DXCoilType,
                           DXCoilName));
                ShowContinueError(state,
                                  " Review the Standard Ratings calculations in the Engineering Reference for this coil type. Also, use "
                                  "Output:Diagnostics, DisplayExtraWarnings for further guidance.");

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowContinueError(state, format("{}The max and/or min limits specified in the corresponding curve objects", RoutineName));
                    ShowContinueError(state,
                                      " do not include the AHRI test conditions required to calculate one or more of the Standard Rating values.");
                }

                // For Standard Rating Cooling Capacity:
                if (CapCurveHighOATLimitsExceeded || CapCurveFlowLimitsExceeded) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(
                            state,
                            format("{}={}:  Standard Rating Cooling Capacity calculated is not at the AHRI test condition.", DXCoilType, DXCoilName));
                        if (CapCurveHighOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                       GetCurveName(state, CapFTempCurveIndex)));
                        }
                        if (CapCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, CapFFlowCurveIndex)));
                        }
                    }
                }

                // For EER:
                if (CapCurveHighOATLimitsExceeded || CapCurveFlowLimitsExceeded || EIRCurveHighOATLimitsExceeded || EIRCurveFlowLimitsExceeded) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(
                            state,
                            format("{}={}:  Energy Efficiency Ratio (EER) calculated is not at the AHRI test condition.", DXCoilType, DXCoilName));
                        if (CapCurveHighOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                       GetCurveName(state, CapFTempCurveIndex)));
                        }
                        if (CapCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, CapFFlowCurveIndex)));
                        }
                        if (EIRCurveHighOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                        }
                        if (EIRCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFFlowCurveIndex)));
                        }
                    }
                }

                // For SEER:
                if (CapCurveMidOATLimitsExceeded || EIRCurveMidOATLimitsExceeded || CapCurveFlowLimitsExceeded || EIRCurveFlowLimitsExceeded ||
                    PLFfPLRforSEERLimitsExceeded) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(state,
                                          format("{}={}:  Seasonal Energy Efficiency Ratio (SEER) calculated is not at the AHRI test condition.",
                                                 DXCoilType,
                                                 DXCoilName));
                        if (CapCurveMidOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                       GetCurveName(state, CapFTempCurveIndex)));
                        }
                        if (CapCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, CapFFlowCurveIndex)));
                        }
                        if (EIRCurveMidOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                        }
                        if (EIRCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFFlowCurveIndex)));
                        }
                        if (PLFfPLRforSEERLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Part Load Fraction Correlation Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(PLFFPLRCurveIndex)->curveType)],
                                       GetCurveName(state, PLFFPLRCurveIndex)));
                        }
                    }
                }

                // For IEER:
                if (CapCurveIEERLimitsExceeded || CapCurveFlowLimitsExceeded || EIRCurveIEERLimitsExceeded || EIRCurveFlowLimitsExceeded) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(state,
                                          format("{}={}:  Integrated Energy Efficiency Ratio (IEER) calculated is not at the AHRI test condition.",
                                                 DXCoilType,
                                                 DXCoilName));
                        if (CapCurveIEERLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                       GetCurveName(state, CapFTempCurveIndex)));
                        }
                        if (CapCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, CapFFlowCurveIndex)));
                        }
                        if (EIRCurveIEERLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in EIR Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                        }
                        if (EIRCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFFlowCurveIndex)));
                        }
                    }
                }

            } // End of curve error messages
            break;
        }
        case CoilDX_HeatingEmpirical: {
            {
                if (state.dataCurveManager->curves(CapFTempCurveIndex)->numDims == 1) {
                    GetCurveMinMaxValues(state, CapFTempCurveIndex, HeatingCapODBTempMin, HeatingCapODBTempMax);

                    // Checking the limits of capacity modifying curve for temperatures (IEER high and low test conditions)
                    if (HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test) {
                        HeatingCapCurveHSPFLimitsExceeded = true;
                    }
                } else {
                    GetCurveMinMaxValues(
                        state, CapFTempCurveIndex, HeatingCapIDBTempMin, HeatingCapIDBTempMax, HeatingCapODBTempMin, HeatingCapODBTempMax);

                    // Checking the limits of capacity modifying curve for temperatures (IEER high and low test conditions)
                    if (HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test ||
                        HeatingCapIDBTempMax < HeatingIndoorCoilInletAirDBTempRated || HeatingCapIDBTempMin > HeatingIndoorCoilInletAirDBTempRated) {
                        HeatingCapCurveHSPFLimitsExceeded = true;
                    }
                }
            }
            {
                if (state.dataCurveManager->curves(EIRFTempCurveIndex)->numDims == 1) {
                    GetCurveMinMaxValues(state, EIRFTempCurveIndex, HeatingEIRODBTempMin, HeatingEIRODBTempMax);

                    // Checking the limits of EIR modifying curve for temperatures (HSPF high and low test conditions)
                    if (HeatingEIRODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingEIRODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test) {
                        HeatingEIRCurveHSPFLimitsExceeded = true;
                    }
                } else {
                    GetCurveMinMaxValues(
                        state, EIRFTempCurveIndex, HeatingEIRIDBTempMin, HeatingEIRIDBTempMax, HeatingEIRODBTempMin, HeatingEIRODBTempMax);

                    // Checking the limits of EIR modifying curve for temperatures (HSPF high and low test conditions)
                    if (HeatingEIRODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingEIRODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test ||
                        HeatingEIRIDBTempMax < HeatingIndoorCoilInletAirDBTempRated || HeatingEIRIDBTempMin > HeatingIndoorCoilInletAirDBTempRated) {
                        HeatingEIRCurveHSPFLimitsExceeded = true;
                    }
                }
            }
            if (HeatingCapCurveHSPFLimitsExceeded || HeatingEIRCurveHSPFLimitsExceeded) {
                ShowWarningError(
                    state,
                    format("The Standard Ratings is calculated for {} = {} but not at the AHRI test condition due to curve out of bound.",
                           DXCoilType,
                           DXCoilName));
                ShowContinueError(state,
                                  " Review the Standard Ratings calculations in the Engineering Reference for this coil type. Also, use "
                                  "Output:Diagnostics, DisplayExtraWarnings for further guidance.");
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowContinueError(state, format("{}The max and/or min limits specified in the corresponding curve objects", RoutineName));
                    ShowContinueError(state,
                                      " do not include the AHRI test conditions required to calculate one or more of the Standard Rating values.");
                }
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(
                        state,
                        format("{}={}:  Heating Seasonal Performance Factor calculated is not at the AHRI test condition.", DXCoilType, DXCoilName));
                    ShowContinueError(state, " Review the Standard Ratings calculations in the Engineering Reference for this coil type.");
                    if (HeatingCapCurveHSPFLimitsExceeded) {
                        ShowContinueError(
                            state,
                            format(" Check limits in Total Heating Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                   Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                   GetCurveName(state, CapFTempCurveIndex)));
                    }
                    if (HeatingEIRCurveHSPFLimitsExceeded) {
                        ShowContinueError(state,
                                          format(" Check limits in EIR Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                                 Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                                 GetCurveName(state, EIRFTempCurveIndex)));
                    }
                }
            }

            //   MultiSpeed DX Coil Net Cooling Capacity and SEER:
            break;
        }
        case CoilDX_MultiSpeedCooling: {
            bool CapCurveLowOATLimitsExceeded = false; // Logical for capacity curve temperature limits being exceeded (low temp)
            bool EIRCurveLowOATLimitsExceeded = false; // Logical for EIR curve temperature limits being exceeded (Low temp)

            GetCurveMinMaxValues(state, CapFTempCurveIndex, CapacityWBTempMin, CapacityWBTempMax, CapacityDBTempMin, CapacityDBTempMax);
            GetCurveMinMaxValues(state, EIRFTempCurveIndex, EIRWBTempMin, EIRWBTempMax, EIRDBTempMin, EIRDBTempMax);
            GetCurveMinMaxValues(state, CapFFlowCurveIndex, CapacityFlowRatioMin, CapacityFlowRatioMax);
            GetCurveMinMaxValues(state, EIRFFlowCurveIndex, EIRFlowRatioMin, EIRFlowRatioMax);

            // Checking the limits of capacity modifying curve for temperatures
            if (CapacityDBTempMax < OutdoorCoilInletAirDryBulbTempRated || CapacityDBTempMin > OutdoorCoilInletAirDryBulbTempRated ||
                CapacityWBTempMax < CoolingCoilInletAirWetBulbTempRated || CapacityWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                CapCurveHighOATLimitsExceeded = true;
            }
            // Checking the limits of capacity modifying curve for flow fraction
            if (CapacityFlowRatioMax < AirMassFlowRatioRated || CapacityFlowRatioMin > AirMassFlowRatioRated) {
                CapCurveFlowLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for temperatures
            if (EIRDBTempMax < OutdoorCoilInletAirDryBulbTempRated || EIRDBTempMin > OutdoorCoilInletAirDryBulbTempRated ||
                EIRWBTempMax < CoolingCoilInletAirWetBulbTempRated || EIRWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                EIRCurveHighOATLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for flow fraction
            if (EIRFlowRatioMax < AirMassFlowRatioRated || EIRFlowRatioMin > AirMassFlowRatioRated) {
                EIRCurveFlowLimitsExceeded = true;
            }
            // Checking the limits of capacity modifying curve for temperatures (SEER calculation)
            if (CapacityDBTempMax < OutdoorCoilInletAirDryBulbTempTestF1 || CapacityDBTempMin > OutdoorCoilInletAirDryBulbTempTestF1 ||
                CapacityWBTempMax < CoolingCoilInletAirWetBulbTempRated || CapacityWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                CapCurveLowOATLimitsExceeded = true;
            }
            // Checking the limits of EIR modifying curve for temperatures (SEER calculation)
            if (EIRDBTempMax < OutdoorCoilInletAirDryBulbTempTestF1 || EIRDBTempMin > OutdoorCoilInletAirDryBulbTempTestF1 ||
                EIRWBTempMax < CoolingCoilInletAirWetBulbTempRated || EIRWBTempMin > CoolingCoilInletAirWetBulbTempRated) {
                EIRCurveLowOATLimitsExceeded = true;
            }

            if (CapCurveHighOATLimitsExceeded || CapCurveFlowLimitsExceeded || EIRCurveHighOATLimitsExceeded || EIRCurveFlowLimitsExceeded ||
                CapCurveLowOATLimitsExceeded || EIRCurveLowOATLimitsExceeded) {

                ShowWarningError(
                    state,
                    format("The Standard Ratings is calculated for {} = {} but not at the AHRI test condition due to curve out of bound.",
                           DXCoilType,
                           DXCoilName));
                ShowContinueError(state,
                                  " Review the Standard Ratings calculations in the Engineering Reference for this coil type. Also, use "
                                  "Output:Diagnostics, DisplayExtraWarnings for further guidance.");

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowContinueError(state, format("{}The max and/or min limits specified in the corresponding curve objects", RoutineName));
                    ShowContinueError(state,
                                      " do not include the AHRI test conditions required to calculate one or more of the Standard Rating values.");
                }

                // For Standard Rating Cooling Capacity:
                if (CapCurveHighOATLimitsExceeded || CapCurveFlowLimitsExceeded) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(state,
                                          format("{}={}:  The Standard Rating Cooling Capacity calculated is not at the AHRI test condition.",
                                                 DXCoilType,
                                                 DXCoilName));
                        if (CapCurveHighOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                       GetCurveName(state, CapFTempCurveIndex)));
                        }
                        if (CapCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, CapFFlowCurveIndex)));
                        }
                    }
                }

                // For MultiSpeed DX Coil SEER:

                if (CapCurveLowOATLimitsExceeded || EIRCurveLowOATLimitsExceeded || CapCurveFlowLimitsExceeded || EIRCurveFlowLimitsExceeded) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowContinueError(state,
                                          format("{}={}:  The Seasonal Energy Efficiency Ratio (SEER) calculated is not at the AHRI test condition.",
                                                 DXCoilType,
                                                 DXCoilName));
                        if (CapCurveLowOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                       GetCurveName(state, CapFTempCurveIndex)));
                        }
                        if (CapCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Total Cooling Capacity Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, CapFFlowCurveIndex)));
                        }
                        if (EIRCurveLowOATLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFTempCurveIndex)));
                        }
                        if (EIRCurveFlowLimitsExceeded) {
                            ShowContinueError(
                                state,
                                format(" Check limits in Energy Input Ratio Function of Flow Fraction Curve, Curve Type = {}, Curve Name = {}",
                                       Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFFlowCurveIndex)->curveType)],
                                       GetCurveName(state, EIRFFlowCurveIndex)));
                        }
                    }
                }

            } // End of curve error messages

            break;
        }
        case CoilDX_MultiSpeedHeating: {

            bool CapCurveOATLimitsExceeded = false; // Logical for capacity curve OD temp. limits being exceeded (low and High)
            {
                if (state.dataCurveManager->curves(CapFTempCurveIndex)->numDims == 1) {
                    GetCurveMinMaxValues(state, CapFTempCurveIndex, HeatingCapODBTempMin, HeatingCapODBTempMax);

                    if (HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test) {
                        CapCurveOATLimitsExceeded = true;
                    }
                    // Checking the limits of capacity modifying curve for temperatures (HSPF high and low test conditions)
                    if (HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test ||
                        HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempH0Test) {
                        HeatingCapCurveHSPFLimitsExceeded = true;
                    }

                } else {
                    GetCurveMinMaxValues(
                        state, CapFTempCurveIndex, HeatingCapIDBTempMin, HeatingCapIDBTempMax, HeatingCapODBTempMin, HeatingCapODBTempMax);

                    // Checking the limits of capacity modifying curve for temperatures (HSPF high and low test conditions)
                    if (HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test ||
                        HeatingCapIDBTempMax < HeatingIndoorCoilInletAirDBTempRated || HeatingCapIDBTempMin > HeatingIndoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempH0Test) {
                        HeatingCapCurveHSPFLimitsExceeded = true;
                    }
                }
            }

            {
                if (state.dataCurveManager->curves(EIRFTempCurveIndex)->numDims == 1) {
                    GetCurveMinMaxValues(state, EIRFTempCurveIndex, HeatingEIRODBTempMin, HeatingEIRODBTempMax);
                    // Checking the limits of EIR modifying curve for temperatures (HSPF high and low test conditions)
                    if (HeatingEIRODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingEIRODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test ||
                        HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempH0Test) {
                        HeatingEIRCurveHSPFLimitsExceeded = true;
                    }
                } else {
                    GetCurveMinMaxValues(
                        state, EIRFTempCurveIndex, HeatingEIRIDBTempMin, HeatingEIRIDBTempMax, HeatingEIRODBTempMin, HeatingEIRODBTempMax);

                    // Checking the limits of EIR modifying curve for temperatures (HSPF high and low test conditions)
                    if (HeatingEIRODBTempMax < HeatingOutdoorCoilInletAirDBTempRated ||
                        HeatingEIRODBTempMin > HeatingOutdoorCoilInletAirDBTempH3Test ||
                        HeatingEIRIDBTempMax < HeatingIndoorCoilInletAirDBTempRated || HeatingEIRIDBTempMin > HeatingIndoorCoilInletAirDBTempRated ||
                        HeatingCapODBTempMax < HeatingOutdoorCoilInletAirDBTempH0Test) {
                        HeatingEIRCurveHSPFLimitsExceeded = true;
                    }
                }
            }
            if (HeatingCapCurveHSPFLimitsExceeded || HeatingEIRCurveHSPFLimitsExceeded || CapCurveOATLimitsExceeded) {

                ShowWarningError(
                    state,
                    format("The Standard Ratings is calculated for {} = {} but not at the AHRI test condition due to curve out of bound.",
                           DXCoilType,
                           DXCoilName));
                ShowContinueError(state,
                                  " Review the Standard Ratings calculations in the Engineering Reference for this coil type. Also, use "
                                  "Output:Diagnostics, DisplayExtraWarnings for further guidance.");

                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowContinueError(state, format("{}The max and/or min limits specified in the corresponding curve objects", RoutineName));
                    ShowContinueError(state,
                                      " do not include the AHRI test conditions required to calculate one or more of the Standard Rating values.");
                }
            }
            if (CapCurveOATLimitsExceeded) {
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(
                        state, format("{}={}:  The Net Heating Capacity Calculated is not at the AHRI test condition.", DXCoilType, DXCoilName));
                    ShowContinueError(
                        state,
                        format(" Check limits in Total Heating Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                               Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                               GetCurveName(state, CapFTempCurveIndex)));
                }
            }

            if (HeatingCapCurveHSPFLimitsExceeded || HeatingEIRCurveHSPFLimitsExceeded) {
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowWarningError(state,
                                     format("{}={}:  The Heating Seasonal Performance Factor calculated is not at the AHRI test condition.",
                                            DXCoilType,
                                            DXCoilName));
                    if (HeatingCapCurveHSPFLimitsExceeded) {
                        ShowContinueError(
                            state,
                            format(" Check limits in Total Heating Capacity Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                   Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(CapFTempCurveIndex)->curveType)],
                                   GetCurveName(state, CapFTempCurveIndex)));
                    }
                    if (HeatingEIRCurveHSPFLimitsExceeded) {
                        ShowContinueError(state,
                                          format(" Check limits in EIR Function of Temperature Curve, Curve Type = {}, Curve Name = {}",
                                                 Curve::objectNames[static_cast<int>(state.dataCurveManager->curves(EIRFTempCurveIndex)->curveType)],
                                                 GetCurveName(state, EIRFTempCurveIndex)));
                    }
                }
            }
            break;
        }
            // case CoilDX_CoolingTwoSpeed: {
            //     GetCurveMinMaxValues(state, CapFTempCurveIndex, CapacityWBTempMin, CapacityWBTempMax, CapacityDBTempMin, CapacityDBTempMax);
            //     GetCurveMinMaxValues(state, EIRFTempCurveIndex, EIRWBTempMin, EIRWBTempMax, EIRDBTempMin, EIRDBTempMax);
            //     GetCurveMinMaxValues(state, CapFFlowCurveIndex, CapacityFlowRatioMin, CapacityFlowRatioMax);
            //     GetCurveMinMaxValues(state, EIRFFlowCurveIndex, EIRFlowRatioMin, EIRFlowRatioMax);
            //     GetCurveMinMaxValues(state, PLFFPLRCurveIndex, PLFFPLRMin, PLFFPLRMax);
            //     break;
            // }
        default:
            break;
        }
    }

    Real64 CondenserEnteringFluidTemperature(DataPlant::CondenserType const CondenserType,
                                             StandardRatings::AhriChillerStd const ChillerStd,
                                             Real64 LoadRatio)
    {
        Real64 CondenserEnteringFluidTemp = 0.0;
        if (ChillerStd == StandardRatings::AhriChillerStd::AHRI550_590) {

            if (CondenserType == DataPlant::CondenserType::WaterCooled) {
                // get the condenser entering water temperature for a given load ratio in deg C
                Real64 enteringWaterTemp = 18.33;
                if (LoadRatio > 0.50) {
                    enteringWaterTemp = 7.22 + 22.22 * LoadRatio;
                }
                CondenserEnteringFluidTemp = enteringWaterTemp;
            } else if (CondenserType == DataPlant::CondenserType::AirCooled) {
                // get the outdoor air dry bulb temperature for a given load ratio in deg C
                Real64 enteringAirDBTemp = 12.78;
                if (LoadRatio > 0.33) {
                    enteringAirDBTemp = 1.67 + 33.33 * LoadRatio;
                }
                CondenserEnteringFluidTemp = enteringAirDBTemp;
            } else { // EvaporativelyCooled Condenser
                // get the outdoor air wet bulb temperature for a given load ratio in deg C
                Real64 enteringAirWBTemp = 10.0 + 13.89 * LoadRatio;
                CondenserEnteringFluidTemp = enteringAirWBTemp;
            }

        } else if (ChillerStd == StandardRatings::AhriChillerStd::AHRI551_591) {

            if (CondenserType == DataPlant::CondenserType::WaterCooled) {
                // get the condenser entering water temperature for a given load ratio in deg C
                Real64 enteringWaterTemp = 19.0;
                if (LoadRatio > 0.50) {
                    enteringWaterTemp = 8.0 + 22.0 * LoadRatio;
                }
                CondenserEnteringFluidTemp = enteringWaterTemp;
            } else if (CondenserType == DataPlant::CondenserType::AirCooled) {
                // get the outdoor air dry bulb temperature for a given load ratio in deg C
                Real64 enteringAirDBTemp = 13.0;
                if (LoadRatio > 0.3125) {
                    enteringAirDBTemp = 3.0 + 32.0 * LoadRatio;
                }
                CondenserEnteringFluidTemp = enteringAirDBTemp;
            } else { // EvaporativelyCooled Condenser
                // get the outdoor air wet bulb temperature for a given load ratio in deg C
                Real64 enteringAirWBTemp = 10.0 + 14.0 * LoadRatio;
                CondenserEnteringFluidTemp = enteringAirWBTemp;
            }
        }
        return CondenserEnteringFluidTemp;
    }

} // namespace StandardRatings

} // namespace EnergyPlus
