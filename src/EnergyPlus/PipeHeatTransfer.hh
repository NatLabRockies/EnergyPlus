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

#ifndef PipeHeatTransfer_hh_INCLUDED
#define PipeHeatTransfer_hh_INCLUDED

// C++ Headers
#include <memory>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array4D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PipeHeatTransfer {

    enum class EnvrnPtr
    {
        Invalid = -1,
        None,
        ZoneEnv,
        ScheduleEnv,
        OutsideAirEnv,
        GroundEnv,
        Num
    };

    enum TimeIndex
    {
        Invalid = -1,
        Previous = 1,
        Current,
        Tentative
    };

    constexpr Real64 InnerDeltaTime(60.0); // one minute time step in seconds

    struct PipeHTData : public PlantComponent
    {

        virtual ~PipeHTData() = default;

        // Members
        // Input data
        std::string Name;
        std::string Construction;                // construction object name
        std::string Environment;                 // keyword:  'Schedule', 'OutdoorAir', 'Zone'
        Sched::Schedule *envrSched = nullptr;    // temperature schedule for environmental temp
        Sched::Schedule *envrVelSched = nullptr; // temperature schedule for environmental temp
        std::string EnvrAirNode;                 // outside air node providing environmental temp
        Real64 Length;                           // total pipe length [m]
        Real64 PipeID;                           // pipe inside diameter [m]
        std::string InletNode;                   // inlet node name
        std::string OutletNode;                  // outlet node name
        int InletNodeNum;                        // inlet node number
        int OutletNodeNum;                       // outlet node number
        DataPlant::PlantEquipmentType Type;      // Type of pipe
        // derived data
        int ConstructionNum; // construction ref number
        EnvrnPtr EnvironmentPtr;
        int EnvrZonePtr;               // pointer to zone number used to set environmental temp
        int EnvrAirNodeNum;            // pointer to outside air node used to set environmental temp
        int NumSections;               // total number of nodes along pipe length
        Real64 FluidSpecHeat;          // fluid Cp [J/kg.K]
        Real64 FluidDensity;           // density [kg/m3]
        Real64 MaxFlowRate;            // max flow rate (from loop/node data)
        Real64 InsideArea;             // pipe section inside surface area [m^2]
        Real64 OutsideArea;            // pipe section outside surface area [m^2]
        Real64 SectionArea;            // cross sectional area [m^2]
        Real64 PipeHeatCapacity;       // heat capacity of pipe section [J/m.K]
        Real64 PipeOD;                 // pipe outside diameter [m]
        Real64 PipeCp;                 // pipe material Cp [J/kg.K]
        Real64 PipeDensity;            // pipe material density [kg/m3]
        Real64 PipeConductivity;       // pipe material thermal conductivity [W/m.K]
        Real64 InsulationOD;           // insulation outside diameter [m]
        Real64 InsulationCp;           // insulation  specific heat [J/kg.K]
        Real64 InsulationDensity;      // insulation density [kg/m3]
        Real64 InsulationConductivity; // insulation conductivity [W/m.K]
        Real64 InsulationThickness;    // insulation thickness [m]
        Real64 InsulationResistance;   // Insulation thermal resistance [m2.K/W]
        Real64 CurrentSimTime;         // Current simulation time [hr]
        Real64 PreviousSimTime;        // simulation time the report data was last updated
        Array1D<Real64> TentativeFluidTemp;
        Array1D<Real64> FluidTemp; // arrays for fluid and pipe temperatures at each node
        Array1D<Real64> PreviousFluidTemp;
        Array1D<Real64> TentativePipeTemp;
        Array1D<Real64> PipeTemp;
        Array1D<Real64> PreviousPipeTemp;
        int NumDepthNodes;                        // number of soil grid points in the depth direction
        int PipeNodeDepth;                        // soil depth grid point where pipe is located
        int PipeNodeWidth;                        // soil width grid point where pipe is located
        Real64 PipeDepth;                         // pipe burial depth [m]
        Real64 DomainDepth;                       // soil grid depth [m]
        Real64 dSregular;                         // grid spacing in cartesian domain [m]
        Real64 OutdoorConvCoef;                   // soil to air convection coefficient [W/m2.K]
        std::string SoilMaterial;                 // name of soil material:regular object
        int SoilMaterialNum;                      // soil material index in material data structure
        int MonthOfMinSurfTemp;                   // month of minimum ground surface temperature
        Real64 MinSurfTemp;                       // minimum annual surface temperature [C]
        Real64 SoilDensity;                       // density of soil [kg/m3]
        Real64 SoilDepth;                         // thickness of soil [m]
        Real64 SoilCp;                            // specific heat of soil [J/kg.K]
        Real64 SoilConductivity;                  // thermal conductivity of soil [W/m.K]
        Material::SurfaceRoughness SoilRoughness; // ground surface roughness
        Real64 SoilThermAbs;                      // ground surface thermal absorptivity
        Real64 SoilSolarAbs;                      // ground surface solar absorptivity
        Real64 CoefA1;                            // soil finite difference coefficient
        Real64 CoefA2;                            // soil finite difference coefficient
        Real64 FourierDS;                         // soil Fourier number based on grid spacing
        Real64 SoilDiffusivity;                   // soil thermal diffusivity [m2/s]
        Real64 SoilDiffusivityPerDay;             // soil thermal diffusivity [m2/day]
        Array4D<Real64> T;                        // soil temperature array
        bool BeginSimInit;                        // begin sim and begin environment flag
        bool BeginSimEnvrn;                       // begin sim and begin environment flag
        bool FirstHVACupdateFlag;
        bool BeginEnvrnupdateFlag;
        bool SolarExposed;       // Flag to determine if solar is included at ground surface
        Real64 SumTK;            // Sum of thickness/conductivity over all material layers
        Real64 ZoneHeatGainRate; // Lagged energy summation for zone heat gain {W}
        PlantLocation plantLoc;
        bool CheckEquipName;
        GroundTemp::BaseGroundTempsModel *groundTempModel; // non-owning pointer

        // Report data
        Real64 FluidInletTemp;          // inlet temperature [C]
        Real64 FluidOutletTemp;         // outlet temperature [C]
        Real64 MassFlowRate;            // mass flow rate [kg/s]
        Real64 FluidHeatLossRate;       // overall heat transfer rate from fluid to pipe [W]
        Real64 FluidHeatLossEnergy;     // energy transferred from fluid to pipe [J]
        Real64 PipeInletTemp;           // pipe temperature at inlet [C]
        Real64 PipeOutletTemp;          // pipe temperature at Oulet [C]
        Real64 EnvironmentHeatLossRate; // overall heat transfer rate from pipe to environment [W]
        Real64 EnvHeatLossEnergy;       // energy transferred from pipe to environment [J]
        Real64 VolumeFlowRate;

        // Default Constructor
        PipeHTData()
            : Length(0.0), PipeID(0.0), InletNodeNum(0), OutletNodeNum(0), Type(DataPlant::PlantEquipmentType::Invalid), ConstructionNum(0),
              EnvironmentPtr(EnvrnPtr::None), EnvrZonePtr(0), EnvrAirNodeNum(0), NumSections(0), FluidSpecHeat(0.0), FluidDensity(0.0),
              MaxFlowRate(0.0), InsideArea(0.0), OutsideArea(0.0), SectionArea(0.0), PipeHeatCapacity(0.0), PipeOD(0.0), PipeCp(0.0),
              PipeDensity(0.0), PipeConductivity(0.0), InsulationOD(0.0), InsulationCp(0.0), InsulationDensity(0.0), InsulationConductivity(0.0),
              InsulationThickness(0.0), InsulationResistance(0.0), CurrentSimTime(0.0), PreviousSimTime(0.0), NumDepthNodes(0), PipeNodeDepth(0),
              PipeNodeWidth(0), PipeDepth(0.0), DomainDepth(0.0), dSregular(0.0), OutdoorConvCoef(0.0), SoilMaterialNum(0), MonthOfMinSurfTemp(0),
              MinSurfTemp(0.0), SoilDensity(0.0), SoilDepth(0.0), SoilCp(0.0), SoilConductivity(0.0),
              SoilRoughness(Material::SurfaceRoughness::Invalid), SoilThermAbs(0.0), SoilSolarAbs(0.0), CoefA1(0.0), CoefA2(0.0), FourierDS(0.0),
              SoilDiffusivity(0.0), SoilDiffusivityPerDay(0.0), BeginSimInit(true), BeginSimEnvrn(true), FirstHVACupdateFlag(true),
              BeginEnvrnupdateFlag(true), SolarExposed(true), SumTK(0.0), ZoneHeatGainRate(0.0), plantLoc{}, CheckEquipName(true),
              FluidInletTemp(0.0), FluidOutletTemp(0.0), MassFlowRate(0.0), FluidHeatLossRate(0.0), FluidHeatLossEnergy(0.0), PipeInletTemp(0.0),
              PipeOutletTemp(0.0), EnvironmentHeatLossRate(0.0), EnvHeatLossEnergy(0.0), VolumeFlowRate(0.0)

        {
        }

        static PlantComponent *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string const &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void PushInnerTimeStepArrays();

        void oneTimeInit_new(EnergyPlusData &state) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void InitPipesHeatTransfer(EnergyPlusData &state, bool FirstHVACIteration);

        Real64 TBND(EnergyPlusData &state,
                    Real64 z // Current Depth
        );

        void CalcBuriedPipeSoil(EnergyPlusData &state);

        void CalcPipesHeatTransfer(EnergyPlusData &state, ObjexxFCL::Optional_int_const LengthIndex = _);

        Real64 OutsidePipeHeatTransCoef(EnergyPlusData &state);

        Real64 CalcPipeHeatTransCoef(EnergyPlusData &state,
                                     Real64 Temperature,    // Temperature of water entering the surface, in C
                                     Real64 t_MassFlowRate, // Mass flow rate, in kg/s
                                     Real64 Diameter        // Pipe diameter, m
        );

        void ReportPipesHeatTransfer(EnergyPlusData &state); // Index for the surface under consideration

        void UpdatePipesHeatTransfer(EnergyPlusData &state);

        void ValidatePipeConstruction(EnergyPlusData &state,
                                      std::string const &PipeType,         // module object of pipe (error messages)
                                      std::string const &ConstructionName, // construction name of pipe (error messages)
                                      std::string_view FieldName,          // fieldname of pipe (error messages)
                                      int t_ConstructionNum,               // pointer into construction data
                                      bool &ErrorsFound                    // set to true if errors found here
        );

        static void CalcZonePipesHeatGain(EnergyPlusData &state);
    };

    void GetPipesHeatTransfer(EnergyPlusData &state);

} // namespace PipeHeatTransfer

struct PipeHeatTransferData : BaseGlobalStruct
{

    int nsvNumOfPipeHT = 0;            // Number of Pipe Heat Transfer objects
    int nsvInletNodeNum = 0;           // module variable for inlet node number
    int nsvOutletNodeNum = 0;          // module variable for outlet node number
    Real64 nsvMassFlowRate = 0.0;      // pipe mass flow rate
    Real64 nsvVolumeFlowRate = 0.0;    // pipe volumetric flow rate
    Real64 nsvDeltaTime = 0.0;         // time change from last update
    Real64 nsvInletTemp = 0.0;         // pipe inlet temperature
    Real64 nsvOutletTemp = 0.0;        // pipe outlet temperature
    Real64 nsvEnvironmentTemp = 0.0;   // environmental temperature (surrounding pipe)
    Real64 nsvEnvHeatLossRate = 0.0;   // heat loss rate from pipe to the environment
    Real64 nsvFluidHeatLossRate = 0.0; // overall heat loss from fluid to pipe
    int nsvNumInnerTimeSteps = 0;      // the number of "inner" time steps for our model
    bool GetPipeInputFlag = true;      // First time, input is "gotten"
    bool MyEnvrnFlag = true;
    Array1D<PipeHeatTransfer::PipeHTData> PipeHT;
    std::unordered_map<std::string, std::string> PipeHTUniqueNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->nsvNumOfPipeHT = 0;
        this->nsvInletNodeNum = 0;
        this->nsvOutletNodeNum = 0;
        this->nsvMassFlowRate = 0.0;
        this->nsvVolumeFlowRate = 0.0;
        this->nsvDeltaTime = 0.0;
        this->nsvInletTemp = 0.0;
        this->nsvOutletTemp = 0.0;
        this->nsvEnvironmentTemp = 0.0;
        this->nsvEnvHeatLossRate = 0.0;
        this->nsvFluidHeatLossRate = 0.0;
        this->nsvNumInnerTimeSteps = 0;
        this->GetPipeInputFlag = true;
        this->MyEnvrnFlag = true;
        this->PipeHT.deallocate();
        this->PipeHTUniqueNames.clear();
    }
};

} // namespace EnergyPlus

#endif
