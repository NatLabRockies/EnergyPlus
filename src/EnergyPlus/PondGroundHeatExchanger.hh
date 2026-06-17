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

#ifndef PondGroundHeatExchanger_hh_INCLUDED
#define PondGroundHeatExchanger_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PondGroundHeatExchanger {

    struct PondGroundHeatExchangerData : PlantComponent
    {
        // Members
        std::string Name;           // name of pond GHE
        std::string InletNode;      // pond inlet fluid node
        std::string OutletNode;     // pond outlet fluid node
        Real64 DesignMassFlowRate;  // design flow rate of circulating fluid
        Real64 DesignCapacity;      // design cooling capacity of pond at
        Real64 Depth;               // depth of pond
        Real64 Area;                // area of pond
        Real64 TubeInDiameter;      // hydronic tube inside diameter
        Real64 TubeOutDiameter;     // hydronic tube outside diameter
        Real64 TubeConductivity;    // hydronic tube thermal conductivity
        Real64 GrndConductivity;    // ground thermal conductivity
        Real64 CircuitLength;       // length of each circuit
        Real64 BulkTemperature;     // current pond bulk temperature
        Real64 PastBulkTemperature; // past pond bulk temperature
        int NumCircuits;            // number of circuits in total
        int InletNodeNum;           // inlet node number
        int OutletNodeNum;          // oulet node number
        int FrozenErrIndex;         // for recurring warnings
        int ConsecutiveFrozen;      // count of time steps consecutive frozen
                                    // loop topology variables
        PlantLocation plantLoc;

        // Report data
        Real64 InletTemp;        // fluid inlet temperature
        Real64 OutletTemp;       // fluid outlet temperature
        Real64 MassFlowRate;     // fluid mass flow rate
        Real64 PondTemp;         // pond bulk temperature
        Real64 HeatTransferRate; // total fluid heat transfer rate, Watts
        Real64 Energy;           // cumulative energy, Joules

        bool OneTimeFlag;
        bool MyFlag;
        bool setupOutputVarsFlag;

        Fluid::GlycolProps *water = nullptr;

        bool firstTimeThrough;

        // Default Constructor
        PondGroundHeatExchangerData()
            : DesignMassFlowRate(0.0), DesignCapacity(0.0), Depth(0.0), Area(0.0), TubeInDiameter(0.0), TubeOutDiameter(0.0), TubeConductivity(0.0),
              GrndConductivity(0.0), CircuitLength(0.0), BulkTemperature(0.0), PastBulkTemperature(0.0), NumCircuits(0), InletNodeNum(0),
              OutletNodeNum(0), FrozenErrIndex(0), ConsecutiveFrozen(0), plantLoc{}, InletTemp(0.0), OutletTemp(0.0), MassFlowRate(0.0),
              PondTemp(0.0), HeatTransferRate(0.0), Energy(0.0), OneTimeFlag(true), MyFlag(true), setupOutputVarsFlag(true), firstTimeThrough(true)
        {
        }

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void InitPondGroundHeatExchanger(EnergyPlusData &state, bool FirstHVACIteration);

        void setupOutputVars(EnergyPlusData &state);

        void CalcPondGroundHeatExchanger(EnergyPlusData &state);

        Real64 CalcTotalFLux(EnergyPlusData &state, Real64 PondBulkTemp // pond temp for this flux calculation
        );

        Real64 CalcEffectiveness(EnergyPlusData &state,
                                 Real64 InsideTemperature, // Temperature of fluid in pipe circuit, in C
                                 Real64 PondTemperature,   // Temperature of pond water (i.e. outside the pipe), in C
                                 Real64 massFlowRate       // Mass flow rate, in kg/s
        );

        Real64 CalcSolarFlux(EnergyPlusData &state) const;

        void UpdatePondGroundHeatExchanger(EnergyPlusData &state);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;
    };

    void GetPondGroundHeatExchanger(EnergyPlusData &state);

} // namespace PondGroundHeatExchanger

struct PondGroundHeatExchangerData : BaseGlobalStruct
{

    bool GetInputFlag = true;
    int NumOfPondGHEs = 0;
    Array1D<PondGroundHeatExchanger::PondGroundHeatExchangerData> PondGHE;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetInputFlag = true;
        this->NumOfPondGHEs = 0;
        this->PondGHE.deallocate();
    }
};

} // namespace EnergyPlus

#endif
