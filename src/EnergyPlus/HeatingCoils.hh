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

#ifndef HeatingCoils_hh_INCLUDED
#define HeatingCoils_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatingCoils {

    // MODULE PARAMETER DEFINITIONS
    Real64 constexpr MinAirMassFlow(0.001);

    enum class HeatObjTypes // reclaim heat object types
    {
        Invalid = -1,
        COMPRESSORRACK_REFRIGERATEDCASE,
        COIL_DX_COOLING, // single speed DX
        COIL_DX_MULTISPEED,
        COIL_DX_MULTIMODE,
        CONDENSER_REFRIGERATION,
        COIL_DX_VARIABLE_COOLING,
        COIL_COOLING_DX_NEW, // Coil:Cooling:DX main one-for-all coil
        Num
    };

    struct HeatingCoilEquipConditions
    {
        // Members
        std::string Name;             // Name of the HeatingCoil
        std::string HeatingCoilType;  // Type of HeatingCoil ie. Heating or Cooling
        std::string HeatingCoilModel; // Type of HeatingCoil ie. Simple, Detailed, etc.

        HVAC::CoilType coilType = HVAC::CoilType::Invalid;
        int coilReportNum = -1;

        Constant::eFuel FuelType = Constant::eFuel::Invalid; // Type of fuel used, reference resource type integers
        Sched::Schedule *availSched = nullptr;               // availability schedule
        int InsuffTemperatureWarn = 0;                       // Used for recurring error message
        Real64 InletAirMassFlowRate = 0.0;                   // MassFlow through the HeatingCoil being Simulated [kg/Sec]
        Real64 OutletAirMassFlowRate = 0.0;
        Real64 InletAirTemp = 0.0;
        Real64 OutletAirTemp = 0.0;
        Real64 InletAirHumRat = 0.0;
        Real64 OutletAirHumRat = 0.0;
        Real64 InletAirEnthalpy = 0.0;
        Real64 OutletAirEnthalpy = 0.0;
        Real64 HeatingCoilLoad = 0.0; // Total Load on the Coil [J]
        Real64 HeatingCoilRate = 0.0; // Total Coil Rate on the Coil [W]
        Real64 FuelUseLoad = 0.0;     // Fuel Usage of Coil [J]
        Real64 ElecUseLoad = 0.0;     // Electric Usage of Coil [J]
        Real64 FuelUseRate = 0.0;     // Fuel Usage of Coil [W]
        Real64 ElecUseRate = 0.0;     // Electric Usage of Coil [W]
        Real64 Efficiency = 0.0;      // HeatingCoil Efficiency Value
        Real64 NominalCapacity = 0.0; // Nominal Capacity of Coil [W]
        Real64 DesiredOutletTemp = 0.0;
        Real64 DesiredOutletHumRat = 0.0;
        Real64 AvailTemperature = 0.0; // Used in heat recovery test [C]
        int AirInletNodeNum = 0;
        int AirOutletNodeNum = 0;
        int TempSetPointNodeNum = 0; // If applicable this is the node number that the temp setpoint exists.
        int Control = 0;
        int PLFCurveIndex = 0;                 // Index for part-load factor curve index for gas heating coil
        Real64 ParasiticElecLoad = 0.0;        // parasitic electric load associated with the gas heating coil
        Real64 ParasiticFuelConsumption = 0.0; // parasitic fuel consumption associated with the gas heating coil (standing pilot light) [J]
        Real64 ParasiticFuelRate = 0.0;        // avg. parasitic fuel consumption rate with the gas heating coil (standing pilot light) [W]
        Real64 ParasiticFuelCapacity = 0.0;    // capacity of parasitic fuel consumption rate, input by user [W]
        Real64 RTF = 0.0;                      // Heater runtime fraction, including PLF curve impacts
        int RTFErrorIndex = 0;                 // used in recurring error warnings
        int RTFErrorCount = 0;                 // used in recurring error warnings
        int PLFErrorIndex = 0;                 // used in recurring error warnings
        int PLFErrorCount = 0;                 // used in recurring error warnings
        std::string ReclaimHeatingCoilName;    // Name of reclaim heating coil
        int ReclaimHeatingSourceIndexNum = 0;  // Index to reclaim heating source (condenser) of a specific type
        HeatObjTypes ReclaimHeatingSource = HeatObjTypes::Invalid; // The source for the Reclaim Heating Coil
        int NumOfStages = 0;                                       // Number of speeds
        Array1D<Real64> MSNominalCapacity;                         // Nominal Capacity MS AC Furnace [W]
        Array1D<Real64> MSEfficiency;                              // Efficiency for MS AC Furnace [dimensionless]
        Array1D<Real64> MSParasiticElecLoad;                       // Parasitic elec load MS AC Furnace (gas only) [W]
        bool DesiccantRegenerationCoil = false; // true if it is a regeneration air heating coil defined in Desiccant Dehumidifier system
        int DesiccantDehumNum = 0;              // index to desiccant dehumidifier object
        bool FaultyCoilSATFlag = false;         // True if the coil has SAT sensor fault
        int FaultyCoilSATIndex = 0;             // Index of the fault object corresponding to the coil
        Real64 FaultyCoilSATOffset = 0.0;       // Coil SAT sensor offset
        bool reportCoilFinalSizes = true;       // one time report of sizes to coil report
        int AirLoopNum = 0;                     // Airloop number
    };

    struct HeatingCoilNumericFieldData
    {
        Array1D_string FieldNames;
    };

    void SimulateHeatingCoilComponents(EnergyPlusData &state,
                                       std::string_view CompName,
                                       bool FirstHVACIteration,
                                       ObjexxFCL::Optional<Real64 const> QCoilReq = _, // coil load to be met
                                       ObjexxFCL::Optional_int CompIndex = _,
                                       ObjexxFCL::Optional<Real64> QCoilActual = _, // coil load actually delivered returned to calling component
                                       ObjexxFCL::Optional_bool_const SuppHeat = _, // True if current heating coil is a supplemental heating coil
                                       ObjexxFCL::Optional<HVAC::FanOp const> fanOp = _,    // fan operating mode, FanOp::Cycling or FanOp::Continuous
                                       ObjexxFCL::Optional<Real64 const> PartLoadRatio = _, // part-load ratio of heating coil
                                       ObjexxFCL::Optional_int StageNum = _,
                                       ObjexxFCL::Optional<Real64 const> SpeedRatio = _ // Speed ratio of MultiStage heating coil
    );

    void GetHeatingCoilInput(EnergyPlusData &state);

    void InitHeatingCoil(EnergyPlusData &state, int CoilNum, bool FirstHVACIteration, Real64 QCoilRequired);

    void SizeHeatingCoil(EnergyPlusData &state, int CoilNum);

    void CalcElectricHeatingCoil(EnergyPlusData &state,
                                 int CoilNum, // index to heating coil
                                 Real64 &QCoilReq,
                                 Real64 &QCoilActual,     // coil load actually delivered (W)
                                 HVAC::FanOp const fanOp, // fan operating mode
                                 Real64 PartLoadRatio     // part-load ratio of heating coil
    );

    void CalcMultiStageElectricHeatingCoil(EnergyPlusData &state,
                                           int const CoilNum,       // the number of the electric heating coil to be simulated
                                           Real64 const SpeedRatio, // SpeedRatio varies between 1.0 (maximum speed) and 0.0 (minimum speed)
                                           Real64 const CycRatio,   // cycling part load ratio
                                           int const StageNum,      // Stage number
                                           HVAC::FanOp const fanOp, // Fan operation mode
                                           Real64 &QCoilActual,     // coil load actually delivered (W)
                                           bool const SuppHeat);

    void CalcFuelHeatingCoil(EnergyPlusData &state,
                             int CoilNum, // index to heating coil
                             Real64 QCoilReq,
                             Real64 &QCoilActual,     // coil load actually delivered (W)
                             HVAC::FanOp const fanOp, // fan operating mode
                             Real64 PartLoadRatio     // part-load ratio of heating coil
    );

    void CalcMultiStageGasHeatingCoil(EnergyPlusData &state,
                                      int const CoilNum,       // the number of the Gas heating coil to be simulated
                                      Real64 const SpeedRatio, // SpeedRatio varies between 1.0 (maximum speed) and 0.0 (minimum speed)
                                      Real64 const CycRatio,   // cycling part load ratio
                                      int const StageNum,      // Speed number
                                      HVAC::FanOp const fanOp  // Fan operation mode
    );

    void CalcDesuperheaterHeatingCoil(EnergyPlusData &state,
                                      int CoilNum,        // index to desuperheater heating coil
                                      Real64 QCoilReq,    // load requested by the simulation for load based control [W]
                                      Real64 &QCoilActual // coil load actually delivered
    );

    void UpdateHeatingCoil(EnergyPlusData &state, int CoilNum);

    void ReportHeatingCoil(EnergyPlusData &state, int CoilNum, bool coilIsSuppHeater);

    void GetCoilIndex(EnergyPlusData &state, std::string const &HeatingCoilName, int &HeatingCoilIndex, bool &ErrorsFound);

    void CheckHeatingCoilSchedule(EnergyPlusData &state,
                                  std::string_view const CompType, // unused1208
                                  std::string const &CompName,
                                  Real64 &Value,
                                  int &CompIndex);

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string_view const CoilType, // must match coil types in this module
                           std::string const &CoilName,     // must match coil names for the coil type
                           bool &ErrorsFound                // set to true if problem
    );

    Sched::Schedule *GetCoilAvailSched(EnergyPlusData &state,
                                       std::string const &CoilType, // must match coil types in this module
                                       std::string const &CoilName, // must match coil names for the coil type
                                       bool &ErrorsFound            // set to true if problem
    );

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string_view CoilType,   // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    );

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string_view CoilType,   // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    );

    int GetHeatReclaimSourceIndex(EnergyPlusData &state,
                                  std::string const &CoilType, // must match coil types in this module
                                  std::string const &CoilName, // must match coil names for the coil type
                                  bool &ErrorsFound            // set to true if problem
    );

    int GetCoilControlNodeNum(EnergyPlusData &state,
                              std::string const &CoilType, // must match coil types in this module
                              std::string const &CoilName, // must match coil names for the coil type
                              bool &ErrorsFound            // set to true if problem
    );

    HVAC::CoilType GetHeatingCoilTypeNum(EnergyPlusData &state,
                                         std::string_view const CoilType, // must match coil types in this module
                                         std::string const &CoilName,     // must match coil names for the coil type
                                         bool &ErrorsFound                // set to true if problem
    );

    int GetHeatingCoilIndex(EnergyPlusData &state,
                            std::string const &CoilType, // must match coil types in this module
                            std::string const &CoilName, // must match coil names for the coil type
                            bool &ErrorsFound            // set to true if problem
    );

    int GetHeatingCoilPLFCurveIndex(EnergyPlusData &state,
                                    std::string const &CoilType, // must match coil types in this module
                                    std::string const &CoilName, // must match coil names for the coil type
                                    bool &ErrorsFound            // set to true if problem
    );

    int GetHeatingCoilNumberOfStages(EnergyPlusData &state,
                                     std::string const &CoilType, // must match coil types in this module
                                     std::string const &CoilName, // must match coil names for the coil type
                                     bool &ErrorsFound            // set to true if problem
    );

    // sets data to a coil that is used as a regeneration air heating coil in
    // desiccant dehumidification system
    void SetHeatingCoilData(EnergyPlusData &state,
                            int CoilNum,                                            // Number of electric or gas heating Coil
                            bool &ErrorsFound,                                      // Set to true if certain errors found
                            ObjexxFCL::Optional_bool DesiccantRegenerationCoil = _, // Flag that this coil is used as regeneration air heating coil
                            ObjexxFCL::Optional_int DesiccantDehumIndex = _         // Index for the desiccant dehum system where this caoil is used
    );

    void SetHeatingCoilAirLoopNumber(EnergyPlusData &state, std::string const &HeatingCoilName, int AirLoopNum, bool &ErrorsFound);

} // namespace HeatingCoils

struct HeatingCoilsData : BaseGlobalStruct
{

    int NumDesuperheaterCoil = 0; // Total number of desuperheater heating coil objects in input
    int NumElecCoil = 0;
    int NumElecCoilMultiStage = 0;
    int NumFuelCoil = 0;
    int NumGasCoilMultiStage = 0;
    int NumHeatingCoils = 0; // The Number of HeatingCoils found in the Input
    Array1D_bool MySizeFlag;
    Array1D_bool ValidSourceType;  // Used to determine if a source for a desuperheater heating coil is valid
    bool GetCoilsInputFlag = true; // Flag set to make sure you get input once
    bool CoilIsSuppHeater = false; // Flag set to indicate the heating coil is a supplemental heater
    Array1D_bool CheckEquipName;
    Array1D<HeatingCoils::HeatingCoilEquipConditions> HeatingCoil;
    Array1D<HeatingCoils::HeatingCoilNumericFieldData> HeatingCoilNumericFields;
    bool MyOneTimeFlag = true; // one time initialization flag
    bool InputErrorsFound = false;

    int MaxNums = 0;                    // Maximum number of numeric input fields
    int MaxAlphas = 0;                  // Maximum number of alpha input fields
    int TotalArgs = 0;                  // Total number of alpha and numeric arguments (max) for a certain object in the input file
    int ValidSourceTypeCounter = 0;     // Counter used to determine if desuperheater source name is valid
    bool HeatingCoilFatalError = false; // used for error checking
    Array1D_bool MySPTestFlag;          // used for error checking
    Array1D_bool ShowSingleWarning;     // Used for single warning message for desuperheater coil
    Array1D_bool MyEnvrnFlag;           // one time environment flag

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumDesuperheaterCoil = 0;
        this->NumElecCoil = 0;
        this->NumElecCoilMultiStage = 0;
        this->NumFuelCoil = 0;
        this->NumGasCoilMultiStage = 0;
        this->NumHeatingCoils = 0;
        this->MySizeFlag.deallocate();
        this->ValidSourceType.deallocate();
        this->GetCoilsInputFlag = true;
        this->CoilIsSuppHeater = false;
        this->CheckEquipName.deallocate();
        this->HeatingCoil.deallocate();
        this->HeatingCoilNumericFields.deallocate();
        this->MyOneTimeFlag = true;
        this->InputErrorsFound = false;
        this->MaxNums = 0;
        this->MaxAlphas = 0;
        this->TotalArgs = 0;
        this->ValidSourceTypeCounter = 0;
        this->HeatingCoilFatalError = false;
        this->MySPTestFlag.clear();
        this->ShowSingleWarning.clear();
        this->MyEnvrnFlag.clear();
    }
};

} // namespace EnergyPlus

#endif
