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

#ifndef PlantOperationEnums_hh_INCLUDED
#define PlantOperationEnums_hh_INCLUDED

namespace EnergyPlus::DataPlant {

// Parameters for loop flow request priority, used in logic to deal with Node%MassFlowRequest for determining overall loop flow rate
enum class LoopFlowStatus
{
    Invalid = -1,        // component's status is not yet set
    NeedyAndTurnsLoopOn, // component is a "winner" for loop flow requests active valve inside component that modulates flow gets the loop going under
                         // most conditions
    NeedyIfLoopOn, // component is a "winner" for loop flow requests but doesn't normally get the loop going to start with once loop is going, may
                   // increase needs, non-zero minimums
    TakesWhatGets, // component is a "loser" for loop flow requests, but if the loop is on it, it does make flow requests (for s/m resolution)
    Num
};

enum class LoopSideLocation
{
    Invalid = -1,
    Demand,
    Supply,
    Num
};

constexpr std::array<LoopSideLocation, static_cast<int>(LoopSideLocation::Num)> LoopSideOther = {LoopSideLocation::Supply, LoopSideLocation::Demand};

constexpr std::array<std::string_view, static_cast<int>(LoopSideLocation::Num)> DemandSupplyNames = {"Demand", "Supply"};

// Parameters for scheme types
// Used in TYPE(OperationData)%Type
// As in PlantLoop(:)%OpScheme(:)%Type
// Also in PlantLoop()LoopSide()Branch()Comp()%CurOpSchemeType
// this may be changed later...
enum class OpScheme
{ // Changed to enum: Better semantic fit and allows use in switch statements: Suggest this migration throughout EnergyPlus (and probably C++11
  // enum "class")
    Invalid = -1,
    NoControl,                // Scheme Type placeholder for items such as pipes
    HeatingRB,                // Scheme Type for Heating Load Range Based Operation
    CoolingRB,                // Scheme Type for Cooling  Load Range Based Operation
    WetBulbRB,                // Scheme Type for Wet bulb range based Operation
    DryBulbRB,                // Scheme Type for Dry bulb range based Operation
    DewPointRB,               // Scheme Type for Dew point range based Operation
    RelHumRB,                 // Scheme Type for relative humidity range based Operation
    DryBulbTDB,               // Scheme Type for dry bulb temp range based Operation
    WetBulbTDB,               // Scheme Type for wet bulb temp based Operation
    DewPointTDB,              // Scheme Type for dew point temp based Operation
    CompSetPtBased,           // Temp Based Control
    Uncontrolled,             // Scheme Type for Uncontrolled Operation
    EMS,                      // Scheme Type for EMS based operation user Define scheme
    Pump,                     // Not really an OpScheme, just a placeholder
    Demand,                   // Placeholder for demand side equipment such as coils
    FreeRejection,            // Scheme Type for waterside economizers and the like
    WSEcon,                   // Scheme Type for waterside economizers and the like
    ThermalEnergyStorage,     // Scheme Type for Simplified Thermal Energy Storage operation
    ChillerHeaterSupervisory, // Scheme Type for managed control of water plants
    Num
};

enum class PlantEquipmentType
{
    Invalid = -1,
    Boiler_Simple,
    Boiler_Steam,
    Chiller_Absorption,          // older BLAST absorption chiller
    Chiller_Indirect_Absorption, // revised absorption chiller
    Chiller_CombTurbine,
    Chiller_ConstCOP,
    Chiller_DFAbsorption,
    Chiller_Electric, // basic BLAST Chiller
    Chiller_ElectricEIR,
    Chiller_ElectricReformEIR,
    Chiller_ElectricASHRAE205,
    Chiller_EngineDriven,
    CoolingTower_SingleSpd,
    CoolingTower_TwoSpd,
    CoolingTower_VarSpd,
    Generator_FCExhaust,
    HeatPumpWtrHeaterPumped,
    HPWaterEFCooling,
    HPWaterEFHeating,
    HPWaterPECooling,
    HPWaterPEHeating,
    Pipe,
    PipeSteam,
    PipeExterior,
    PipeInterior,
    PipeUnderground,
    PurchChilledWater,
    PurchHotWater,
    TS_IceDetailed,
    TS_IceSimple,
    ValveTempering,
    WtrHeaterMixed,
    WtrHeaterStratified,
    PumpVariableSpeed,
    PumpConstantSpeed,
    PumpCondensate,
    PumpBankVariableSpeed,
    PumpBankConstantSpeed,
    WaterUseConnection,
    CoilWaterCooling,             // demand side component
    CoilWaterDetailedFlatCooling, // demand side component
    CoilWaterSimpleHeating,       // demand side component
    CoilSteamAirHeating,          // demand side component
    SolarCollectorFlatPlate,      // demand side component
    PlantLoadProfile,             // demand side component
    GrndHtExchgSystem,
    GrndHtExchgSurface,
    GrndHtExchgPond,
    Generator_MicroTurbine, // newer FSEC turbine
    Generator_ICEngine,
    Generator_CTurbine, // older BLAST turbine
    Generator_MicroCHP,
    Generator_FCStackCooler,
    FluidCooler_SingleSpd,
    FluidCooler_TwoSpd,
    EvapFluidCooler_SingleSpd,
    EvapFluidCooler_TwoSpd,
    ChilledWaterTankMixed,
    ChilledWaterTankStratified,
    PVTSolarCollectorFlatPlate,
    Baseboard_Conv_Water,
    Baseboard_Rad_Conv_Steam,
    Baseboard_Rad_Conv_Water,
    LowTempRadiant_VarFlow,
    LowTempRadiant_ConstFlow,
    CooledBeamAirTerminal,
    CoilWAHPHeatingEquationFit,
    CoilWAHPCoolingEquationFit,
    CoilWAHPHeatingParamEst,
    CoilWAHPCoolingParamEst,
    RefrigSystemWaterCondenser,
    RefrigerationWaterCoolRack,
    MultiSpeedHeatPumpRecovery,
    Chiller_ExhFiredAbsorption,
    PipingSystemPipeCircuit,
    SolarCollectorICS,
    CoilVSWAHPHeatingEquationFit,
    CoilVSWAHPCoolingEquationFit,
    PlantComponentUserDefined,
    CoilUserDefined,
    ZoneHVACAirUserDefined,
    AirTerminalUserDefined,
    HeatPumpVRF,
    GrndHtExchgHorizTrench,
    FluidToFluidPlantHtExchg,
    WaterSource,
    CentralGroundSourceHeatPump,
    UnitarySysRecovery,
    PackagedTESCoolingCoil,
    CoolingTower_VarSpdMerkel,
    SwimmingPool_Indoor,
    GrndHtExchgSlinky,
    HeatPumpWtrHeaterWrapped,
    FourPipeBeamAirTerminal,
    CoolingPanel_Simple,
    HeatPumpEIRCooling,
    HeatPumpEIRHeating,
    HeatPumpFuelFiredCooling,
    HeatPumpFuelFiredHeating,
    HeatPumpAirToWaterCooling,
    HeatPumpAirToWaterHeating,
    HeatPumpAirToWater,
    PurchSteam,
    Num
};

// Parameters for component character wrt how load gets met (or not)
//  used in %HowLoadServed to facilitate load dispatch logic
enum class HowMet
{
    Invalid = -1,                         // not yet set
    NoneDemand,                           // does not meet a load, demand component
    PassiveCap,                           // Passive machine, does what conditions allow but
    ByNominalCap,                         // MaxLoad, MinLoad, OptLoad should work
    ByNominalCapLowOutLimit,              // MaxLoad, MinLoad, OptLoad but with low limit temp on outlet
    ByNominalCapHiOutLimit,               // MaxLoad, MinLoad, OptLoad but with high limit temp on outlet
    ByNominalCapFreeCoolCntrl,            // HowMet_ByNominalCap with free cool shutdown
    ByNominalCapLowOutLimitFreeCoolCntrl, // HowMet_ByNominalCapLowOutLimit with free cool shutdown
    Num
};

enum class LoadingScheme
{
    Invalid = -1,
    Optimal,              // Optimal Load Distribution Scheme
    Sequential,           // Sequential Load Distribution Scheme
    Uniform,              // Uniform Load Distribution Scheme
    UniformPLR,           // Uniform PLR Load Distribution Scheme
    SequentialUniformPLR, // Sequential Uniform PLR Load Distribution Scheme
    Num
};

enum class FlowMode
{
    Invalid = -1,
    Constant,
    NotModulated,
    LeavingSetpointModulated,
    VariableSpeedPump,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(FlowMode::Num)> FlowModeNamesUC{
    "CONSTANTFLOW", "NOTMODULATED", "LEAVINGSETPOINTMODULATED", "VARIABLESPEEDPUMPING"};

enum class CondenserFlowControl
{
    Invalid = -1,
    ConstantFlow,
    ModulatedChillerPLR,
    ModulatedLoopPLR,
    ModulatedDeltaTemperature,
    Num
};

constexpr std::array<std::string_view, static_cast<int>(CondenserFlowControl::Num)> CondenserFlowControlNamesUC{
    "CONSTANTFLOW", "MODULATEDCHILLERPLR", "MODULATEDLOOPPLR", "MODULATEDDELTATEMPERATURE"};

enum class CondenserType
{
    Invalid = -1,
    AirCooled,
    WaterCooled,
    EvapCooled,
    Num
};

static constexpr std::array<std::string_view, static_cast<int>(CondenserType::Num)> CondenserTypeNamesUC{
    "AIRCOOLED", "WATERCOOLED", "EVAPORATIVELYCOOLED"};

// SimFlagCriteriaTypes for use in performing interconnect re-sim checks
enum class CriteriaType
{
    Invalid = -1,
    MassFlowRate,
    Temperature,
    HeatTransferRate,
    Num
};

enum class FreeCoolControlMode
{
    Invalid = -1,
    WetBulb, // HeatExchanger:Hydronic model control type mode, outdoor wetbulb sensor
    DryBulb, // HeatExchanger:Hydronic model control type mode, outdoor drybulb sensor
    Loop,    // HeatExchanger:Hydronic model control type mode, loop setpoint sensor
    Num
};

enum class LoopDemandCalcScheme
{
    Invalid = -1,
    SingleSetPoint,       // Uses a single temp setpoint to calculate loop demand
    DualSetPointDeadBand, // Uses a dual temp setpoint with a deadband between the high
    Num
};

enum class CommonPipeType
{
    Invalid = -1,
    No,
    Single,
    TwoWay,
    Num
};

enum class FlowLock
{
    Invalid = -1,
    PumpQuery, // Used to ask the pumps for their min/max avail based on no constraints
    Unlocked,  // components request flow
    Locked,    // components take their inlet flow
    Num
};

enum class PressureCall
{
    Invalid = -1,
    Init,
    Calc,
    Update,
    Num
};

enum class PressSimType
{
    Invalid = -1,
    NoPressure,          // Nothing for that particular loop
    PumpPowerCorrection, // Only updating the pump power
    FlowCorrection,      // Update pump flow rate based on pump curve
    FlowSimulation,      // Full pressure network simulation
    Num
};

static constexpr std::array<std::string_view, static_cast<int>(PressSimType::Num)> PressureSimTypeNamesUC{
    "NONE",
    "PUMPPOWERCORRECTION",
    "LOOPFLOWCORRECTION",
    "PRESSURESIMULATION",
};

enum class CtrlType
{
    Invalid = -1,
    HeatingOp, // Constant for Heating Operation
    CoolingOp, // Constant for Cooling Operation
    DualOp,    // Constant for Cooling or Heating Operation
    Num
};

// branch loop type for absorption chillerheater models
enum class BrLoopType
{
    Invalid = -1,
    Chiller,
    Heater,
    Condenser,
    NoMatch,
    Num
};

static constexpr std::array<std::string_view, static_cast<int>(HowMet::Num)> HowMetTypeNamesUC = {
    "DEMANDSLOAD",
    "MEETSLOADWITHPASSIVECAPACITY",
    "MEETSLOADWITHNOMINALCAPACITY",
    "MEETSLOADWITHNOMINALCAPACITYLOWOUTLIMIT",
    "MEETSLOADWITHNOMINALCAPACITYHIOUTLIMIT",
    "MEETSLOADWITHNOMINALCAPACITYFREECOOLCONTROL",
    "MEETSLOADWITHNOMINALCAPACITYLOWOUTLIMITFREECOOLCONTROL"};

static constexpr std::array<std::string_view, static_cast<int>(LoopFlowStatus::Num)> LoopFlowStatusTypeNamesUC = {
    "NEEDSFLOWANDTURNSLOOPON", "NEEDSFLOWIFLOOPON", "RECEIVESWHATEVERFLOWAVAILABLE"};

} // namespace EnergyPlus::DataPlant

#endif
