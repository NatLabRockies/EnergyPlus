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

#ifndef PlantTopologyLoop_hh_INCLUDED
#define PlantTopologyLoop_hh_INCLUDED

#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/LoopSide.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace DataPlant {

    // "Both" is used as a special flag and is never assigned to the loop's TypeOfLoop member
    enum class LoopType
    {
        Invalid = -1,
        Plant,
        Condenser,
        Both,
        Num
    };

    constexpr std::array<std::string_view, (int)LoopType::Num> loopTypeNames = {"PlantLoop", "CondenserLoop", "Both"};

    enum class WaterLoopType
    {
        Invalid = -1,
        HotWater,
        ChilledWater,
        None,
        Num
    };

    constexpr std::array<std::string_view, (int)LoopType::Num> waterLoopTypeNames = {"HotWater", "ChilledWater", "None"};

    static constexpr std::array<std::string_view, static_cast<int>(PressSimType::Num)> waterLoopTypeNamesUC{
        "HOTWATER",
        "CHILLEDWATER",
        "NONE",
    };
    // This needs to go, it's not helping

    struct HalfLoopContainer : std::array<HalfLoopData, static_cast<int>(DataPlant::LoopSideLocation::Num)>
    {
        HalfLoopData &operator()(LoopSideLocation ls)
        {
            return this->at(static_cast<int>(ls));
        }
    };

    constexpr std::array<DataPlant::LoopSideLocation, static_cast<int>(DataPlant::LoopSideLocation::Num)> LoopSideKeys = {
        DataPlant::LoopSideLocation::Demand, DataPlant::LoopSideLocation::Supply};

    struct PlantLoopData
    {
        // Members
        std::string Name;                      // Name of the component list
        std::string FluidName;                 // Name of the fluid specified for this loop
        DataLoopNode::NodeFluidType FluidType; // Type of fluid in the loop
        int FluidIndex = 0;

        Fluid::GlycolProps *glycol = nullptr;
        Fluid::RefrigProps *steam = nullptr;

        int MFErrIndex;  // for recurring mass flow errors
        int MFErrIndex1; // for recurring mass flow errors
        int MFErrIndex2; // for recurring mass flow errors
        // (see CheckPlantMixerSplitterConsistency)
        // Loop Operating Setpoints and Limits
        int TempSetPointNodeNum;         // Node Number for Loop Temp SP associated with SP manager
        int MaxBranch;                   // Max branches in the loop
        Real64 MinTemp;                  // Minimum temperature allowed in the loop
        Real64 MaxTemp;                  // Maximum temperature allowed in the loop
        int MinTempErrIndex;             // for recurring too cold errors
        int MaxTempErrIndex;             // for recurring too hot errors
        Real64 MinVolFlowRate;           // Minimum flow rate allowed in the loop
        Real64 MaxVolFlowRate;           // Maximum flow rate allowed in the loop
        bool MaxVolFlowRateWasAutoSized; // true if previous was set to autosized in the input
        Real64 MinMassFlowRate;          // Minimum flow rate allowed in the loop
        Real64 MaxMassFlowRate;          // Maximum flow rate allowed in the loop
        Real64 Volume;                   // Volume of the fluid in the loop
        bool VolumeWasAutoSized;         // true if Volume was set to autocalculate
        Real64 CirculationTime;          // Loop circulation time [minutes] used to autocalculate loop volume, default is 2 minutes
        Real64 Mass;                     // Mass of the fluid in the loop
        bool EMSCtrl;
        Real64 EMSValue;
        // Loop Inlet and Outlet Nodes
        HalfLoopContainer LoopSide;                           // Half loop data (Demand side or Supply Side)
        std::string OperationScheme;                          // Operation scheme name for the loop
        int NumOpSchemes;                                     // Number of items in list identified by "OpScheme"
        Array1D<OperationData> OpScheme;                      // Operation scheme data
        DataPlant::LoadingScheme LoadDistribution;            // Load distribution scheme 1 for optimal, 2 for overloading
        int PlantSizNum;                                      // index to corresponding plant sizing data array
        DataPlant::LoopDemandCalcScheme LoopDemandCalcScheme; // Load distribution scheme 1 SingleSetPoint,
        // 2 DualSetPointWithDeadBand
        DataPlant::CommonPipeType CommonPipeType;
        int EconPlantSideSensedNodeNum;
        int EconCondSideSensedNodeNum;
        int EconPlacement;
        int EconBranch;
        int EconComp;
        Real64 EconControlTempDiff;
        bool LoopHasConnectionComp;
        LoopType TypeOfLoop;
        WaterLoopType TypeOfWaterLoop;
        DataPlant::PressSimType PressureSimType;
        bool HasPressureComponents;
        Real64 PressureDrop;
        bool UsePressureForPumpCalcs;
        Real64 PressureEffectiveK;
        // report variables
        Real64 CoolingDemand;       // Plant Loop Cooling Demand, W
        Real64 HeatingDemand;       // Plant Loop Heating Demand[W]
        Real64 DemandNotDispatched; // Plant Loop Demand that was not distributed [W]
        Real64 UnmetDemand;         // Plant Loop Unmet Demand [W]
        Real64 BypassFrac;
        Real64 InletNodeFlowrate;
        Real64 InletNodeTemperature;
        Real64 OutletNodeFlowrate;
        Real64 OutletNodeTemperature;
        int LastLoopSideSimulated;

        // Default Constructor
        PlantLoopData()
            : FluidType(DataLoopNode::NodeFluidType::Blank), MFErrIndex(0), MFErrIndex1(0), MFErrIndex2(0), TempSetPointNodeNum(0), MaxBranch(0),
              MinTemp(0.0), MaxTemp(0.0), MinTempErrIndex(0), MaxTempErrIndex(0), MinVolFlowRate(0.0), MaxVolFlowRate(0.0),
              MaxVolFlowRateWasAutoSized(false), MinMassFlowRate(0.0), MaxMassFlowRate(0.0), Volume(0.0), VolumeWasAutoSized(false),
              CirculationTime(2.0), Mass(0.0), EMSCtrl(false), EMSValue(0.0), NumOpSchemes(0), LoadDistribution(DataPlant::LoadingScheme::Invalid),
              PlantSizNum(0), LoopDemandCalcScheme(DataPlant::LoopDemandCalcScheme::Invalid), CommonPipeType(DataPlant::CommonPipeType::No),
              EconPlantSideSensedNodeNum(0), EconCondSideSensedNodeNum(0), EconPlacement(0), EconBranch(0), EconComp(0), EconControlTempDiff(0.0),
              LoopHasConnectionComp(false), TypeOfLoop(LoopType::Invalid), PressureSimType(DataPlant::PressSimType::NoPressure),
              HasPressureComponents(false), PressureDrop(0.0), UsePressureForPumpCalcs(false), PressureEffectiveK(0.0), CoolingDemand(0.0),
              HeatingDemand(0.0), DemandNotDispatched(0.0), UnmetDemand(0.0), BypassFrac(0.0), InletNodeFlowrate(0.0), InletNodeTemperature(0.0),
              OutletNodeFlowrate(0.0), OutletNodeTemperature(0.0), LastLoopSideSimulated(0)
        {
        }

        void UpdateLoopSideReportVars(EnergyPlusData &state, Real64 OtherSideDemand, Real64 LocalRemLoopDemand);

        void CheckLoopExitNode(EnergyPlusData &state, bool FirstHVACIteration);

        void CalcUnmetPlantDemand(EnergyPlusData &state);
    };
} // namespace DataPlant
} // namespace EnergyPlus

#endif
