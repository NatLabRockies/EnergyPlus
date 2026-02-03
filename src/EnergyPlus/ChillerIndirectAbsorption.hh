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

#ifndef ChillerIndirectAbsorption_hh_INCLUDED
#define ChillerIndirectAbsorption_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ChillerIndirectAbsorption {

    struct ReportVars
    {
        // Members
        Real64 PumpingPower = 0.0;         // reporting: W - electric pumping power
        Real64 QGenerator = 0.0;           // reporting: W - steam heat transfer rate
        Real64 QEvap = 0.0;                // reporting: W - evaporator heat transfer rate
        Real64 QCond = 0.0;                // reporting: W - condenser heat transfer rate
        Real64 PumpingEnergy = 0.0;        // reporting: J - electric pumping power
        Real64 GeneratorEnergy = 0.0;      // reporting: J - steam heat transfer rate
        Real64 EvapEnergy = 0.0;           // reporting: J - evaporator heat transfer rate
        Real64 CondEnergy = 0.0;           // reporting: J - condenser heat transfer rate
        Real64 CondInletTemp = 0.0;        // reporting: C - condenser inlet temperature
        Real64 EvapInletTemp = 0.0;        // reporting: C - evaporator inlet temperature
        Real64 CondOutletTemp = 0.0;       // reporting: C - condenser outlet temperature
        Real64 EvapOutletTemp = 0.0;       // reporting: C - evaporator outlet temperature
        Real64 Evapmdot = 0.0;             // reporting: kg/ - evaporator mass flow rate
        Real64 Condmdot = 0.0;             // reporting: kg/ - condenser mass flow rate
        Real64 Genmdot = 0.0;              // reporting: generators mass flow rate when connected to plant
        Real64 SteamMdot = 0.0;            // reporting: kg/s - steam mass flow rate
        Real64 ActualCOP = 0.0;            // reporting: coefficient of performance = QEvap/QGenerator
        Real64 ChillerPartLoadRatio = 0.0; // reporting: part-load ratio
        Real64 ChillerCyclingFrac = 0.0;   // reporting: chiller on/off cycling fraction
        Real64 LoopLoss = 0.0;             // reporting: W - loop loss from absorber outlet to condensate pump inlet
    };

    struct IndirectAbsorberSpecs : PlantComponent
    {
        // Members
        std::string Name;                         // user identifier
        Real64 NomCap = 0.0;                      // W - design nominal capacity of Absorber
        bool NomCapWasAutoSized = false;          // true if Nominal capacity was autosize on input
        Real64 NomPumpPower = 0.0;                // W - design nominal capacity of Absorber
        bool NomPumpPowerWasAutoSized = false;    // true if nominal pump power was autosize on input
        Real64 EvapVolFlowRate = 0.0;             // m3/s - design nominal water volumetric flow rate through the evaporator
        bool EvapVolFlowRateWasAutoSized = false; // true if evaporator flow rate was autosize on input
        Real64 CondVolFlowRate = 0.0;             // m3/s - design nominal water volumetric flow rate through the condenser
        bool CondVolFlowRateWasAutoSized = false; // true if condenser flow rate was autosize on input
        Real64 EvapMassFlowRateMax = 0.0;         // kg/s - Max Design Evaporator Mass Flow Rate converted from Volume Flow Rate
        Real64 CondMassFlowRateMax = 0.0;         // Max Design Condenser Mass Flow Rate [kg/s]
        Real64 GenMassFlowRateMax = 0.0;          // kg/s - Max Design Generator Mass Flow Rate converted from Volume Flow Rate
        Real64 MinPartLoadRat = 0.0;              // (BLAST MIN) min allowed operating frac full load
        Real64 MaxPartLoadRat = 0.0;              // (BLAST MAX) max allowed operating frac full load
        Real64 OptPartLoadRat = 0.0;              // (BLAST BEST) optimal operating frac full load
        Real64 TempDesCondIn = 0.0;               // C - (BLAST ADJTC(1)The design secondary loop fluid
        // temperature at the Absorber condenser side inlet
        Real64 MinCondInletTemp = 0.0;                 // C - minimum condenser inlet temperature for chiller operation
        Real64 MinGeneratorInletTemp = 0.0;            // C - minimum generator inlet temperature for chiller operation
        Real64 TempLowLimitEvapOut = 0.0;              // C - low temperature shut off
        Real64 GeneratorVolFlowRate = 0.0;             // m3/s - hot water volumetric flow rate through generator
        bool GeneratorVolFlowRateWasAutoSized = false; // true if hot water flow was autosize on input
        Real64 GeneratorSubcool = 0.0;                 // C - amount of subcooling in steam generator
        Real64 LoopSubcool = 0.0;                      // C - amount of subcooling in steam generator
        Real64 GeneratorDeltaTemp = -99999.0;          // C - generator fluid temperature difference (water only)
        bool GeneratorDeltaTempWasAutoSized = true;    // true if generator delta T was autosize on input
        Real64 SizFac = 0.0;                           // Sizing factor
        int EvapInletNodeNum = 0;                      // Node number on the inlet side of the plant
        int EvapOutletNodeNum = 0;                     // Node number on the outlet side of the plant
        int CondInletNodeNum = 0;                      // Node number on the inlet side of the condenser
        int CondOutletNodeNum = 0;                     // Node number on the outlet side of the condenser
        int GeneratorInletNodeNum = 0;                 // Generator inlet node number, steam/water side
        int GeneratorOutletNodeNum = 0;                // Generator outlet node number, steam/water side
        int GeneratorInputCurvePtr = 0;                // Index to steam use curve as a function of PLR
        int PumpPowerCurvePtr = 0;                     // Index to pump power curve as a function of PLR
        int CapFCondenserTempPtr = 0;                  // Index to capacity as a function of absorber temp curve
        int CapFEvaporatorTempPtr = 0;                 // Index to capacity as a function of evaporator temp curve
        int CapFGeneratorTempPtr = 0;                  // Index to capacity as a function of generator temp curve
        int HeatInputFCondTempPtr = 0;                 // Index to generator heat input as a function of absorber temp
        int HeatInputFEvapTempPtr = 0;                 // Index to generator heat input as a function of absorber temp
        int ErrCount2 = 0;                             // error counter
        DataLoopNode::NodeFluidType GenHeatSourceType =
            DataLoopNode::NodeFluidType::Blank;                      // Generator heat source type, DataLoopNode::NodeFluidType::Steam=3 or
                                                                     // DataLoopNode::NodeFluidType::Water=2
        Fluid::RefrigProps *steam = nullptr;                         // STEAM Fluid Properties
        bool Available = false;                                      // need an array of logicals--load identifiers of available equipment
        bool ON = false;                                             // simulate the machine at it's operating part load ratio
        DataPlant::FlowMode FlowMode = DataPlant::FlowMode::Invalid; // one of 3 modes for component flow during operation
        bool ModulatedFlowSetToLoop = false;                         // True if the setpoint is missing at the outlet node
        bool ModulatedFlowErrDone = false;                           // true if setpoint warning issued
        int MinCondInletTempCtr = 0;                                 // Low condenser temp warning message counter
        int MinCondInletTempIndex = 0;                               // Low condenser temp warning message index
        int MinGenInletTempCtr = 0;                                  // Low generator temp warning message counter
        int MinGenInletTempIndex = 0;                                // Low generator temp warning message index
        PlantLocation CWPlantLoc;                                    // chilled water plant loop component index
        PlantLocation CDPlantLoc;                                    // condenser water plant loop component index
        PlantLocation GenPlantLoc;                                   // generator plant loop component index
        bool FaultyChillerSWTFlag = false;                           // True if the chiller has SWT sensor fault
        int FaultyChillerSWTIndex = 0;                               // Index of the fault object corresponding to the chiller
        Real64 FaultyChillerSWTOffset = 0.0;                         // Chiller SWT sensor offset
        bool PossibleSubcooling = false;                             // flag to indicate chiller is doing less cooling that requested
        Real64 CondMassFlowRate = 0.0;                               // Kg/s - condenser mass flow rate, water side
        Real64 EvapMassFlowRate = 0.0;                               // Kg/s - evaporator mass flow rate, water side
        Real64 GenMassFlowRate = 0.0;                                // Kg/s - steam mass flow rate, water side
        Real64 CondOutletTemp = 0.0;                                 // C - condenser outlet temperature, water side
        Real64 EvapOutletTemp = 0.0;                                 // C - evaporator outlet temperature, water side
        Real64 GenOutletTemp = 0.0;                                  // C - generator fluid outlet temperature
        Real64 SteamOutletEnthalpy = 0.0;                            // J/kg - generator fluid outlet enthalpy
        Real64 PumpingPower = 0.0;                                   // W - rate of Absorber energy use
        Real64 PumpingEnergy = 0.0;                                  // J - Absorber energy use
        Real64 QGenerator = 0.0;                                     // W - rate of Absorber steam use
        Real64 GeneratorEnergy = 0.0;                                // J - Absorber steam use
        Real64 QEvaporator = 0.0;                                    // W - rate of heat transfer to the evaporator coil
        Real64 EvaporatorEnergy = 0.0;                               // J - heat transfer to the evaporator coil
        Real64 QCondenser = 0.0;                                     // W - rate of heat transfer to the condenser coil
        Real64 CondenserEnergy = 0.0;                                // J - heat transfer to the condenser coil
        Real64 ChillerONOFFCyclingFrac = 0.0;                        // fraction of time chiller is on
        Real64 EnergyLossToEnvironment = 0.0;                        // J - piping energy loss from generator outlet to pump inlet
        bool GenInputOutputNodesUsed = false;
        bool MyOneTimeFlag = true;
        bool MyEnvrnFlag = true;
        ReportVars Report;
        DataBranchAirLoopPlant::ControlType EquipFlowCtrl = DataBranchAirLoopPlant::ControlType::Invalid;

        Fluid::GlycolProps *water = nullptr;

        static IndirectAbsorberSpecs *factory(EnergyPlusData &state, std::string const &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void getSizingFactor(Real64 &sizFac) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void initialize(EnergyPlusData &state, bool RunFlag, Real64 MyLoad);

        void setupOutputVars(EnergyPlusData &state);

        void sizeChiller(EnergyPlusData &state);

        void updateRecords(EnergyPlusData &state, Real64 MyLoad, bool RunFlag);

        void calculate(EnergyPlusData &state, Real64 MyLoad, bool RunFlag);
    };

    void GetIndirectAbsorberInput(EnergyPlusData &state);

} // namespace ChillerIndirectAbsorption

struct ChillerIndirectAbsoprtionData : BaseGlobalStruct
{
    bool GetInput = true;
    Array1D<ChillerIndirectAbsorption::IndirectAbsorberSpecs> IndirectAbsorber;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ChillerIndirectAbsoprtionData();
    }
};

} // namespace EnergyPlus

#endif
