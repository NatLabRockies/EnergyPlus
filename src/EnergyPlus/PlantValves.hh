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

#ifndef PlantValves_hh_INCLUDED
#define PlantValves_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantValves {

    struct TemperValveData : PlantComponent
    {
        // Members
        // user input data
        std::string Name;             // User identifier
        int PltInletNodeNum = 0;      // Node number on the inlet side of the plant
        int PltOutletNodeNum = 0;     // Node number on the outlet side of the plant
        int PltStream2NodeNum = 0;    // Node number on the outlet side of the second stream
        int PltSetPointNodeNum = 0;   // Node number for the setpoint node.
        int PltPumpOutletNodeNum = 0; // node number for the pump outlet (for flow rate)
        // Calculated and from elsewhere
        bool environmentInit = true;    // flag for initializationL true means do the initializations
        Real64 FlowDivFract = 0.0;      // Fraction of flow sent down diversion path
        Real64 Stream2SourceTemp = 0.0; // Temperature [C] of stream 2 being mixed
        Real64 InletTemp = 0.0;         // Temperature [C] of inlet to valve
        Real64 SetPointTemp = 0.0;      // setpoint Temperatures [C] at control node.
        Real64 MixedMassFlowRate = 0.0; // Flow rate downstream of mixer [kg/s]
        // loop topology variables
        PlantLocation plantLoc{};
        bool compDelayedInitFlag = true;

        TemperValveData() = default;

        virtual ~TemperValveData() = default;

        static PlantComponent *factory(EnergyPlusData &state, std::string objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void initialize(EnergyPlusData &state);

        void calculate(EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    void GetPlantValvesInput(EnergyPlusData &state);

} // namespace PlantValves

struct PlantValvesData : BaseGlobalStruct
{

    bool GetTemperingValves = true;
    bool OneTimeInitFlag = true;
    int NumTemperingValves = 0;
    EPVector<PlantValves::TemperValveData> TemperValve; // dimension to No. of TemperingValve objects

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        GetTemperingValves = true;
        OneTimeInitFlag = true;
        NumTemperingValves = 0;
        TemperValve.deallocate();
    }
};

} // namespace EnergyPlus

#endif
