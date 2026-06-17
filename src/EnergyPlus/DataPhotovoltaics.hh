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

#ifndef DataPhotovoltaics_hh_INCLUDED
#define DataPhotovoltaics_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace DataPhotovoltaics {

    Real64 constexpr MinIrradiance = 0.3; // [W/m2] Assume no operation if Ic is below this number (W/m2)

    enum class PVModel
    {
        Invalid = -1,
        Simple,
        TRNSYS,
        Sandia,
        Num
    };

    enum class CellIntegration
    {
        Invalid = -1,             // cell temp method not set
        Decoupled,                // cell temp method based on energy balance
        DecoupledUllebergDynamic, // cell temp method based on energy bal with capacity
        SurfaceOutsideFace,       // cell temp method based on coupling to E+'s heat balance
        TranspiredCollector,      // cell temp method based on coupling to unglazed transpired co
        ExteriorVentedCavity,     // cell temp method based on coupling to nat vent exterior cavi
        PVTSolarCollector,        // cell temp method based on coupling to PVT model
        Num
    };

    enum class Efficiency
    {
        Invalid = -1,
        Fixed,     // simple PV, constant efficiency
        Scheduled, // simple PV, scheduled efficiency
        Num
    };

    enum class SiPVCells
    {
        Invalid = -1,
        Crystalline,
        Amorphous,
        Num
    };

    struct SimplePVParamsStruct
    {
        // Members
        std::string Name;                    // name as identified in Sandia database
        Real64 AreaCol;                      // effective area of solar collection
        Real64 ActiveFraction;               // fraction of parent surface that has active solar cells
        Efficiency EfficencyInputMode;       // to schedule or not
        Sched::Schedule *effSched = nullptr; // index pointer for efficiency schedule
        Real64 PVEfficiency;                 // fixed or current PV efficiency

        // Default Constructor
        SimplePVParamsStruct() : AreaCol(0.0), ActiveFraction(0.0), EfficencyInputMode(Efficiency::Invalid), PVEfficiency(0.0)
        {
        }
    };

    struct TRNSYSPVModuleParamsStruct // for  GENERATOR:PV:Equivalent One-Diode Model
    {
        // Members
        std::string Name;
        int CellsInSeries;           // cells in series [-]
        SiPVCells CellType;          // type of PV cell (crystalline, amorphous )
        Real64 Area;                 // module area [m2]
        Real64 TauAlpha;             // tau alpha product at normal incidence [-]
        Real64 SemiConductorBandgap; // electron bandgap [eV]
        Real64 ShuntResistance;      // shunt resistance [ohms]
        Real64 RefIsc;               // short circuit current at reference conditions [A/K]
        Real64 RefVoc;               // open circuit voltage at reference conditions [V/K]
        Real64 RefTemperature;       // temperature at reference conditions
        Real64 RefInsolation;        // radiation at reference conditions [W/m2]
        Real64 Imp;                  // current at max power [A]
        Real64 Vmp;                  // voltage at max power [V]
        Real64 TempCoefIsc;          // temperature coefficient of short circuit current
        Real64 TempCoefVoc;          // temperature coefficient of open circuit voltage
        Real64 NOCTAmbTemp;          // ambient temperature at NOCT [C]
        Real64 NOCTCellTemp;         // cell temperature at NOCT [C]
        Real64 NOCTInsolation;       // radiation at NOCT [W/m2]
        Real64 HeatLossCoef;         // heat loss coefficient [W/m2.K]
        Real64 HeatCapacity;         // total heat capacity (only used in TC mode 1)

        // Default Constructor
        TRNSYSPVModuleParamsStruct()
            : CellsInSeries(0), CellType(SiPVCells::Invalid), Area(0.0), TauAlpha(0.0), SemiConductorBandgap(0.0), ShuntResistance(0.0), RefIsc(0.0),
              RefVoc(0.0), RefTemperature(0.0), RefInsolation(0.0), Imp(0.0), Vmp(0.0), TempCoefIsc(0.0), TempCoefVoc(0.0), NOCTAmbTemp(0.0),
              NOCTCellTemp(0.0), NOCTInsolation(0.0), HeatLossCoef(0.0), HeatCapacity(0.0)
        {
        }
    };

    struct TRNSYSPVCalcStruct
    {
        // Members
        Real64 Insolation;      // radiation [W/m2]
        Real64 ArrayCurrent;    // array current at current conditions [A]
        Real64 ArrayVoltage;    // array voltage at current conditions [V]
        Real64 ArrayPower;      // array power at current conditions [W]
        Real64 ArrayEfficiency; // array efficiency at current conditions [0..1]
        Real64 CellTemp;        // array cell temperature at current conditions [C]
        Real64 CellTempK;       // array cell temperature (for setting last cell temp) [K]
        Real64 TimeElapsed;     // time previous update of last cell temp
        Real64 LastCellTempK;   // array cell temperature at previous conditions [K]
        Real64 ArrayIsc;        // array short circuit current at current conditions [A]
        Real64 ArrayVoc;        // array open circuit voltage at current conditions [V]

        // Default Constructor
        TRNSYSPVCalcStruct()
            : Insolation(0.0), ArrayCurrent(0.0), ArrayVoltage(0.0), ArrayPower(0.0), ArrayEfficiency(0.0), CellTemp(0.0), CellTempK(0.0),
              TimeElapsed(0.0), LastCellTempK(0.0), ArrayIsc(0.0), ArrayVoc(0.0)
        {
        }
    };

    struct SNLModuleParamsStuct // for PV MODULE:SANDIA PARAMETERS
    {
        // Members
        std::string name;    // name as identified in Sandia database
        Real64 Acoll;        // Active collector area (m2, single module)
        Real64 NcellSer;     // Number of cells in series in a module's cell-string (unitless)
        Real64 NparSerCells; // Number of cell-strings in parallel in module (unitless)
        Real64 Isc0;         // Short circuit current at reference conditions (Amps)
        Real64 Voc0;         // Open circuit voltage at reference conditions (Volts)
        Real64 Imp0;         // Max power point current at reference conditions (Amps)
        Real64 Vmp0;         // Voltage at max power at reference conditions (Volts)
        Real64 aIsc;         // Normalized temperature coefficient for Isc (Amps/degC) Isc temperature coeff
        Real64 aImp;         // Normalized temperature coefficient for Imp (1/degC) Imp temperature coeff
        Real64 c_0;          // Empirical coefficients relating Imp to Ee (unitless)
        //   coefficient relating Imp to irradiance
        Real64 c_1; // Empirical coefficients relating Imp to Ee (unitless)
        //   coefficient relating Voc to irradiance
        Real64 BVoc0; // Temperature coefficient for module open-circuit-voltage at reference conditions
        //   (Volts/degC)
        Real64 mBVoc; // Coefficient for irradiance dependence of open-circuit-voltage-temperature
        //  coefficient  (V/°C)
        Real64 BVmp0; // Temperature coefficient for module maximum-power-voltage at reference conditions
        //   (V/°C)
        Real64 mBVmp; // Coefficient for irradiance dependence of maximum-power-voltage-temperature
        //   coefficient (V/°C)
        Real64 DiodeFactor; // Empirically determined 'diode factor' for individual cells (unitless)
        Real64 c_2;         // Empirical coefficients relating Vmp to Ee (unitless)
        //   (coefficient relating Vmp to irradiance)
        Real64 c_3; // Empirical coefficients relating Vmp to Ee (unitless)
        //   (coefficient relating Vmp to irradiance)
        Real64 a_0; // Empirical coefficients for f1(AMa) polynomial (unitless)
        Real64 a_1; // Empirical coefficients for f1(AMa) polynomial (unitless)
        Real64 a_2; // Empirical coefficients for f1(AMa) polynomial (unitless)
        Real64 a_3; // Empirical coefficients for f1(AMa) polynomial (unitless)
        Real64 a_4; // Empirical coefficients for f1(AMa) polynomial (unitless)
        Real64 b_0; // Empirical coefficients for f1(AOI) polynomial (unitless)
        Real64 b_1; // Empirical coefficients for f1(AOI) polynomial (unitless)
        Real64 b_2; // Empirical coefficients for f1(AOI) polynomial (unitless)
        Real64 b_3; // Empirical coefficients for f1(AOI) polynomial (unitless)
        Real64 b_4; // Empirical coefficients for f1(AOI) polynomial (unitless)
        Real64 b_5; // Empirical coefficients for f1(AOI) polynomial (unitless)
        Real64 DT0; // Temperature difference between Tc and Tm at Eo (°C),
        // (This is d(Tc) in Sandia database)
        Real64 fd; // Fraction of diffuse irradiance used by module (unitless)
        Real64 a;  // Empirical coefficient for module temp.at low wind,
        // high solar irradiance (unitless)
        Real64 b; // Empirical coefficient relating module temp.
        // decrease with increasing wind speed (unitless)
        Real64 c_4;  // Empirical coefficients relating Ix to Ee (unitless)
        Real64 c_5;  // Empirical coefficients relating Ix to Ee (unitless)
        Real64 Ix0;  // Current at V = 0.5 Voc and at reference conditions (Amps)
        Real64 Ixx0; // Current at V = 0.5 (Vmp + Voc) and at reference conditions (Amps)
        Real64 c_6;  // Empirical coefficients relating Ixx to Ee (unitless)
        Real64 c_7;  // Empirical coefficients relating Ixx to Ee (unitless)

        // Default Constructor
        SNLModuleParamsStuct()
            : Acoll(0.0), NcellSer(0.0), NparSerCells(0.0), Isc0(0.0), Voc0(0.0), Imp0(0.0), Vmp0(0.0), aIsc(0.0), aImp(0.0), c_0(0.0), c_1(0.0),
              BVoc0(0.0), mBVoc(0.0), BVmp0(0.0), mBVmp(0.0), DiodeFactor(0.0), c_2(0.0), c_3(0.0), a_0(0.0), a_1(0.0), a_2(0.0), a_3(0.0), a_4(0.0),
              b_0(0.0), b_1(0.0), b_2(0.0), b_3(0.0), b_4(0.0), b_5(0.0), DT0(0.0), fd(0.0), a(0.0), b(0.0), c_4(0.0), c_5(0.0), Ix0(0.0), Ixx0(0.0),
              c_6(0.0), c_7(0.0)
        {
        }
    };

    struct SNLPVInputStruct // for data obtained elsewhere in EnergyPlus
    {
        // Members
        Real64 IcBeam;         // incident beam solar (W/m2)
        Real64 IcDiffuse;      // incident diffuse solar (W/m2)
        Real64 IncidenceAngle; // angle from normal for beam (deg)
        Real64 ZenithAngle;    // solar zenith angle (deg)
        Real64 Tamb;           // outdoor drybulb temperature (C)
        Real64 WindSpeed;      // outdoor windspeed. (m/s)
        Real64 Altitude;       // elevation above sea level. (m)

        // Default Constructor
        SNLPVInputStruct() : IcBeam(0.0), IcDiffuse(0.0), IncidenceAngle(0.0), ZenithAngle(0.0), Tamb(0.0), WindSpeed(0.0), Altitude(0.0)
        {
        }
    };

    struct SNLPVCalcStruct // hold calculated results from PV modeling.
    {
        // Members
        Real64 Vmp;         // (Volts) maximum power voltage
        Real64 Imp;         // (Amps) maximum power current
        Real64 Pmp;         // (W) (was kJ/hr) maximum power point power
        Real64 EffMax;      // (unitless) conversion efficiency at max power point
        Real64 Isc;         // (Amps) short circuit current
        Real64 Voc;         // (Volts) open circuit voltage
        Real64 Tcell;       // (deg C) solar cell operating temperature
        Real64 Tback;       // (deg C) solar module operation temp, at back of module
        Real64 AMa;         // (unitless) Absolute Air mass
        Real64 F1;          // (unitless) holds result of "AMa-Function" for solar spectrum influence
        Real64 F2;          // (unitless) holds result of AOI-Function for angle-of-incidence
        Real64 Ix;          // (Amps) Current at V = 0.5 Voc
        Real64 Vx;          // (Volts) Voltage at 0.5 Voc
        Real64 Ixx;         // (Amps) current at V = 0.5(Vmpp + Voc)
        Real64 Vxx;         // (Volts) voltage at 0.5(Vmpp + Voc)
        Real64 SurfaceSink; // (Watts) energy balance term to account for electricity leaving

        // Default Constructor
        SNLPVCalcStruct()
            : Vmp(0.0), Imp(0.0), Pmp(0.0), EffMax(0.0), Isc(0.0), Voc(0.0), Tcell(0.0), Tback(0.0), AMa(0.0), F1(0.0), F2(0.0), Ix(0.0), Vx(0.0),
              Ixx(0.0), Vxx(0.0), SurfaceSink(0.0)
        {
        }
    };

    struct PVReportVariables // for  GENERATOR:PV:EQUIVALENT ONE-DIODE MODEL
    {
        // Members
        Real64 DCPower;         // Direct Current power from PV array
        Real64 DCEnergy;        // Direct Current energy from PV array
        Real64 ArrayEfficiency; // array efficiency at current conditions [0..1]
        Real64 CellTemp;        // array cell temperature at current conditions [C]
        Real64 ArrayIsc;        // array short circuit current at current conditions [A]
        Real64 ArrayVoc;        // array open circuit voltage at current conditions [V]
        Real64 ArrayCurrent;
        Real64 ArrayVoltage;

        // Default Constructor
        PVReportVariables()
            : DCPower(0.0), DCEnergy(0.0), ArrayEfficiency(0.0), CellTemp(0.0), ArrayIsc(0.0), ArrayVoc(0.0), ArrayCurrent(0.0), ArrayVoltage(0.0)
        {
        }
    };

    struct PVArrayStruct
    {
        // Members
        std::string Name;
        std::string SurfaceName; // named surface in heat balance domain
        std::string PerfObjName;
        int SurfacePtr;                      // index for named surface
        int Zone;                            // index for zone (for getting any zone multipliers)
        PVModel PVModelType;                 // type of performance modeling, Simple, TRNSYS or Equivalent 1-diode, or Sandia/King model
        CellIntegration CellIntegrationMode; // how are PV cells integrated with other E+ modeling
        Real64 NumModNSeries;                // number of modules in series in one string
        Real64 NumSeriesNParall;             // number of series strings in parallel
        int UTSCPtr;                         // pointer to UTSC number for INTEGRATED TRANSPIRED COLLECTOR mode
        int ExtVentCavPtr;                   // pointer to Exterior Vented Cavity EXTERIOR VENTED CAVITY
        int PVTPtr;                          // pointer to PVT model
        Real64 SurfaceSink;                  // PV power "sink" for integration
        PVReportVariables Report;            // report variables
        // nested structs for user input parameters
        SimplePVParamsStruct SimplePVModule;       // simple model input params
        TRNSYSPVModuleParamsStruct TRNSYSPVModule; // equivalent one-diode input params
        SNLModuleParamsStuct SNLPVModule;          // Sandia/King model input parameter data
        // nested structs for model input from elsewhere and calculations
        TRNSYSPVCalcStruct TRNSYSPVcalc;
        SNLPVInputStruct SNLPVinto; // model input from elsewhere in EnergyPlus
        SNLPVCalcStruct SNLPVCalc;  // calc'd data for GENERATOR:PV:Sandia model

        // Default Constructor
        PVArrayStruct()
            : SurfacePtr(0), Zone(0), PVModelType(PVModel::Invalid), CellIntegrationMode(CellIntegration::Invalid), NumModNSeries(1.0),
              NumSeriesNParall(1.0), UTSCPtr(0), ExtVentCavPtr(0), PVTPtr(0), SurfaceSink(0.0)
        {
        }
    };
} // namespace DataPhotovoltaics

struct PhotovoltaicsData : BaseGlobalStruct
{

    int NumPVs = 0;                 // count of number of PV generators
    int Num1DiodePVModuleTypes = 0; // count for Equivalent one-diode model
    int NumSimplePVModuleTypes = 0; // count of number of input objs for simple model
    int NumSNLPVModuleTypes = 0;    // count of number of input objs for Sandia model
    Real64 ShuntResistance = 0.0;   // old "RSH" in common block of trnsys code
    Array1D<DataPhotovoltaics::PVArrayStruct> PVarray;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumPVs = 0;
        Num1DiodePVModuleTypes = 0;
        NumSimplePVModuleTypes = 0;
        NumSNLPVModuleTypes = 0;
        ShuntResistance = 0.0;
        PVarray.deallocate();
    }
};

} // namespace EnergyPlus

#endif
