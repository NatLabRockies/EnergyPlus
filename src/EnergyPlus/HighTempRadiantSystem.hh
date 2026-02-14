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

#ifndef HighTempRadiantSystem_hh_INCLUDED
#define HighTempRadiantSystem_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HighTempRadiantSystem {

    // Types

    enum class RadControlType : int
    {
        Invalid = -1,
        MATControl,
        MRTControl,
        OperativeControl,
        MATSPControl,
        MRTSPControl,
        OperativeSPControl,
        Num
    };

    struct HighTempRadiantSystemData
    {
        // Members
        // Input data
        std::string Name;                      // name of hydronic radiant system
        Sched::Schedule *availSched = nullptr; // availability schedule
        int ZonePtr;                           // Point to this zone in the Zone derived type
        Constant::eResource HeaterType;        // Type of heater (NaturalGas or Electricity)
        Real64 MaxPowerCapac;                  // Maximum capacity of the radiant heater in Watts
        Real64 CombustionEffic;                // Combustion efficiency (only valid for a gas heater)
        Real64 FracRadiant;                    // Fraction of heater power that is given off as radiant heat
        Real64 FracLatent;                     // Fraction of heater power that is given off as latent heat
        Real64 FracLost;                       // Fraction of heater power that is lost to the outside environment
        Real64 FracConvect;                    // Fraction of heater power that is given off as convective heat
        // (by definition this is 1 minus the sum of all other fractions)
        RadControlType ControlType;            // Control type for the system (MAT, MRT, or op temp)
        Real64 ThrottlRange;                   // Throttling range for heating [C]
        Sched::Schedule *setptSched = nullptr; // Schedule for the zone setpoint temperature
        Real64 FracDistribPerson;              // Fraction of fraction radiant incident on a "person" in the space
        int TotSurfToDistrib;                  // Total number of surfaces the heater sends radiation to
        Array1D_string SurfaceName;            // Surface name in the list of surfaces heater sends radiation to
        Array1D_int SurfacePtr;                // Surface number in the list of surfaces heater sends radiation to
        Array1D<Real64> FracDistribToSurf;     // Fraction of fraction radiant incident on the surface
        // Other parameters
        Real64 ZeroHTRSourceSumHATsurf; // used in baseboard energy balance
        Real64 QHTRRadSource;           // Need to keep the last value in case we are still iterating
        Real64 QHTRRadSrcAvg;           // Need to keep the last value in case we are still iterating
        Real64 LastSysTimeElapsed;      // Need to keep the last value in case we are still iterating
        Real64 LastTimeStepSys;         // Need to keep the last value in case we are still iterating
        Real64 LastQHTRRadSrc;          // Need to keep the last value in case we are still iterating

        // Report data
        Real64 ElecPower;                              // system electric consumption in Watts
        Real64 ElecEnergy;                             // system electric consumption in Joules
        Real64 GasPower;                               // system gas consumption in Watts
        Real64 GasEnergy;                              // system gas consumption in Joules
        Real64 HeatPower;                              // actual heating sent to zone (convective and radiative) in Watts
        Real64 HeatEnergy;                             // actual heating sent to zone (convective and radiative) in Joules
        DataSizing::DesignSizingType HeatingCapMethod; // - Method for High Temperature Radiant heating capacity scalable sizing calculation
                                                       // (HeatingDesignCapacity,
                                                       // CapacityPerFloorArea, FracOfAutosizedHeatingCapacity)
        Real64
            ScaledHeatingCapacity; // - High Temperature Radiant scaled maximum heating capacity {W} or scalable variable for sizing in {-}, or {W/m2}

        // Default Constructor
        HighTempRadiantSystemData()
            : ZonePtr(0), HeaterType(Constant::eResource::Invalid), MaxPowerCapac(0.0), CombustionEffic(0.0), FracRadiant(0.0), FracLatent(0.0),
              FracLost(0.0), FracConvect(0.0), ControlType(RadControlType::Invalid), ThrottlRange(0.0), FracDistribPerson(0.0), TotSurfToDistrib(0),
              ZeroHTRSourceSumHATsurf(0.0), QHTRRadSource(0.0), QHTRRadSrcAvg(0.0), LastSysTimeElapsed(0.0), LastTimeStepSys(0.0),
              LastQHTRRadSrc(0.0), ElecPower(0.0), ElecEnergy(0.0), GasPower(0.0), GasEnergy(0.0), HeatPower(0.0), HeatEnergy(0.0),
              HeatingCapMethod(DataSizing::DesignSizingType::Invalid), ScaledHeatingCapacity(0.0)
        {
        }
    };

    struct HighTempRadSysNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        HighTempRadSysNumericFieldData()
        {
        }
    };

    // Functions

    void SimHighTempRadiantSystem(EnergyPlusData &state,
                                  std::string_view CompName,     // name of the low temperature radiant system
                                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                  Real64 &LoadMet,               // load met by the radiant system, in Watts
                                  int &CompIndex);

    void GetHighTempRadiantSystem(EnergyPlusData &state, bool &ErrorsFound); // Error flag if problems encountered on reading user input

    void InitHighTempRadiantSystem(EnergyPlusData &state,
                                   bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                   int const RadSysNum // Index for the low temperature radiant system under consideration within the derived types
    );

    void SizeHighTempRadiantSystem(EnergyPlusData &state, int const RadSysNum);

    void CalcHighTempRadiantSystem(EnergyPlusData &state, int const RadSysNum); // name of the low temperature radiant system

    void CalcHighTempRadiantSystemSP(EnergyPlusData &state,
                                     bool const FirstHVACIteration, // true if this is the first HVAC iteration at this system time step !unused1208
                                     int const RadSysNum            // name of the low temperature radiant system
    );

    void UpdateHighTempRadiantSystem(EnergyPlusData &state,
                                     int const RadSysNum, // Index for the low temperature radiant system under consideration within the derived types
                                     Real64 &LoadMet      // load met by the radiant system, in Watts
    );

    void UpdateHTRadSourceValAvg(EnergyPlusData &state, bool &HighTempRadSysOn); // .TRUE. if the radiant system has run this zone time step

    void DistributeHTRadGains(EnergyPlusData &state);

    void ReportHighTempRadiantSystem(EnergyPlusData &state,
                                     int RadSysNum); // Index for the low temperature radiant system under consideration within the derived types

} // namespace HighTempRadiantSystem

struct HighTempRadiantSystemData : BaseGlobalStruct
{

    // Standard, run-of-the-mill variables...
    int NumOfHighTempRadSys = 0; // Number of hydronic low tempererature radiant systems
    Array1D_bool MySizeFlag;
    Array1D_bool CheckEquipName;

    // Object Data
    Array1D<HighTempRadiantSystem::HighTempRadiantSystemData> HighTempRadSys;
    Array1D<HighTempRadiantSystem::HighTempRadSysNumericFieldData> HighTempRadSysNumericFields;

    bool GetInputFlag = true;
    bool firstTime = true; // For one-time initializations
    bool MyEnvrnFlag = true;
    bool ZoneEquipmentListChecked = false; // True after the Zone Equipment List has been checked for items

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumOfHighTempRadSys = 0;
        MySizeFlag.clear();
        CheckEquipName.clear();

        HighTempRadSys.clear();
        HighTempRadSysNumericFields.clear();

        GetInputFlag = true;
        firstTime = true;
        MyEnvrnFlag = true;
        ZoneEquipmentListChecked = false;
    }
};

} // namespace EnergyPlus

#endif
