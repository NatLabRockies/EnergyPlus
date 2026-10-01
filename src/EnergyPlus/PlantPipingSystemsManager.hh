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

#ifndef PlantPipingSystemsManager_hh_INCLUDED
#define PlantPipingSystemsManager_hh_INCLUDED

// C++ Headers
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>

// ObjexxFCL Headers
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array3D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantPipingSystemsManager {

    // MODULE PARAMETER DEFINITIONS:
    extern std::string const ObjName_ug_GeneralDomain;
    extern std::string const ObjName_Circuit;
    extern std::string const ObjName_Segment;
    extern std::string const ObjName_HorizTrench;
    extern std::string const ObjName_ZoneCoupled_Slab;
    extern std::string const ObjName_ZoneCoupled_Basement;

    enum class SegmentFlow
    {
        Invalid = -1,
        IncreasingZ,
        DecreasingZ,
        Num
    };

    enum class MeshDistribution
    {
        Invalid = -1,
        Uniform,
        SymmetricGeometric,
        Geometric,
        Num
    };

    enum class RegionType
    {
        Invalid = -1,
        Pipe,
        BasementWall,
        BasementFloor,
        XDirection,
        YDirection,
        ZDirection,
        XSide,
        XSideWall,
        ZSide,
        ZSideWall,
        FloorInside,
        UnderFloor,
        HorizInsXSide,
        HorizInsZSide,
        VertInsLowerEdge,
        Num
    };

    enum class Direction
    {
        Invalid = -1,
        PositiveY,
        NegativeY,
        PositiveX,
        NegativeX,
        PositiveZ,
        NegativeZ,
        Num
    };

    enum class PartitionType
    {
        Invalid = -1,
        BasementWall,
        BasementFloor,
        Pipe,
        Slab,
        XSide,
        XSideWall,
        ZSide,
        ZSideWall,
        FloorInside,
        UnderFloor,
        HorizInsXSide,
        VertInsLowerEdge,
        HorizInsZSide,
        Num
    };

    enum class CellType
    {
        Invalid = -1,
        Pipe,
        GeneralField,
        GroundSurface,
        FarfieldBoundary,
        BasementWall,
        BasementFloor,
        BasementCorner,
        BasementCutaway,
        Slab,
        HorizInsulation,
        VertInsulation,
        ZoneGroundInterface,
        Num
    };

    enum class SlabPosition
    {
        Invalid = -1,
        InGrade,
        OnGrade,
        Num
    };

    enum class HorizInsulation
    {
        Invalid = -1,
        None,
        Perimeter,
        Full,
        Num
    };

    struct BaseThermalPropertySet
    {
        // Members
        Real64 Conductivity = 0.0; // W/mK
        Real64 Density = 0.0;      // kg/m3
        Real64 SpecificHeat = 0.0; // J/kgK

        // Default Constructor
        BaseThermalPropertySet() = default;

        Real64 inline diffusivity() const
        {
            return this->Conductivity / (this->Density * this->SpecificHeat);
        }
    };

    struct ExtendedFluidProperties : BaseThermalPropertySet
    {
        // Members
        Real64 Viscosity = 0.0; // kg/m-s
        Real64 Prandtl = 0.0;   // -

        // Default Constructor
        ExtendedFluidProperties() = default;
    };

    struct BaseCell
    {
        // Members
        Real64 Temperature = 0.0;               // C
        Real64 Temperature_PrevIteration = 0.0; // C
        Real64 Temperature_PrevTimeStep = 0.0;  // C
        Real64 Beta = 0.0;                      // K/W

        // Default Constructor
        BaseCell() = default;
    };

    struct RadialSizing
    {
        // Members
        Real64 InnerDia = 0.0;
        Real64 OuterDia = 0.0;

        // Default Constructor
        RadialSizing() = default;

        Real64 inline thickness() const
        {
            return (this->OuterDia - this->InnerDia) / 2.0;
        }
    };

    struct RadialCellInformation : BaseCell
    {
        // Members
        Real64 RadialCentroid = 0.0;
        Real64 InnerRadius = 0.0;
        Real64 OuterRadius = 0.0;
        BaseThermalPropertySet Properties;

        // Default Constructor
        RadialCellInformation() = default;

        // Member Constructor
        RadialCellInformation(Real64 const m_RadialCentroid, Real64 const m_MinRadius, Real64 const m_MaxRadius)
        {
            RadialCentroid = m_RadialCentroid;
            InnerRadius = m_MinRadius;
            OuterRadius = m_MaxRadius;
        }

        // Get the XY cross sectional area of the radial cell
        Real64 inline XY_CrossSectArea() const
        {
            return Constant::Pi * (pow_2(this->OuterRadius) - pow_2(this->InnerRadius));
        }
    };

    struct FluidCellInformation : BaseCell
    {
        // Members
        Real64 Volume = 0.0;
        ExtendedFluidProperties Properties;

        // Default Constructor
        FluidCellInformation() = default;

        // Member Constructor
        FluidCellInformation(Real64 const m_PipeInnerRadius, Real64 const m_CellDepth)
        {
            this->Volume = Constant::Pi * pow_2(m_PipeInnerRadius) * m_CellDepth;
        }
    };

    struct CartesianPipeCellInformation // Specialized cell information only used by cells which contain pipes
    {
        // Members
        std::vector<RadialCellInformation> Soil;
        RadialCellInformation Insulation;
        RadialCellInformation Pipe;
        FluidCellInformation Fluid;
        Real64 RadialSliceWidth = 0.0;
        Real64 InterfaceVolume = 0.0;

        // Default Constructor
        CartesianPipeCellInformation() = default;

        CartesianPipeCellInformation(Real64 GridCellWidth,
                                     EnergyPlus::PlantPipingSystemsManager::RadialSizing PipeSizes,
                                     int NumRadialNodes,
                                     Real64 CellDepth,
                                     Real64 InsulationThickness,
                                     Real64 RadialGridExtent,
                                     bool SimHasInsulation);
    };

    struct Point
    {
        // Members
        int X = 0;
        int Y = 0;

        // Default Constructor
        Point() = default;

        // Member Constructor
        Point(int const X, int const Y) : X(X), Y(Y)
        {
        }
    };

    struct PointF
    {
        // Members
        Real64 X = 0.0;
        Real64 Y = 0.0;

        // Default Constructor
        PointF() = default;

        // Member Constructor
        PointF(Real64 const X, Real64 const Y) : X(X), Y(Y)
        {
        }
    };

    struct Point3DInteger
    {
        // Members
        int X = 0;
        int Y = 0;
        int Z = 0;

        // Default Constructor
        Point3DInteger() = default;

        // Member Constructor
        Point3DInteger(int const X, int const Y, int const Z) : X(X), Y(Y), Z(Z)
        {
        }
    };

    struct Point3DReal
    {
        // Members
        Real64 X = 0.0;
        Real64 Y = 0.0;
        Real64 Z = 0.0;

        // Default Constructor
        Point3DReal() = default;

        // Member Constructor
        Point3DReal(Real64 const X, Real64 const Y, Real64 const Z) : X(X), Y(Y), Z(Z)
        {
        }
    };

    struct MeshPartition
    {
        // Members
        Real64 rDimension = 0.0;
        PartitionType partitionType = PartitionType::Pipe;
        Real64 TotalWidth = 0.0;

        // Default Constructor
        MeshPartition() = default;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
        // Member Constructor -- shows unused but it's actually implied in emplace_back calls in createPartitionCenterList
        MeshPartition(Real64 const rDimension,
                      PartitionType const partitionType, // From Enum: ParitionType
                      Real64 const TotalWidth)
            : rDimension(rDimension), partitionType(partitionType), TotalWidth(TotalWidth)
        {
        }
#pragma clang diagnostic pop

        // used to allow std::find to see if a MeshPartition matches a float (rDimension) value
        bool operator==(Real64 a)
        {
            return this->rDimension == a;
        }
    };

    struct GridRegion
    {
        // Members
        Real64 Min = 0.0;
        Real64 Max = 0.0;
        RegionType thisRegionType = RegionType::Pipe;
        std::vector<Real64> CellWidths;

        // Default Constructor
        GridRegion() = default;

        // Member Constructor
        GridRegion(Real64 Min, Real64 Max, RegionType thisRegionType, std::vector<Real64> CellWidths)
            : Min(Min), Max(Max), thisRegionType(thisRegionType), CellWidths(std::move(CellWidths))
        {
        }
    };

    struct RectangleF
    {
        // Members
        Real64 X_min = 0.0;
        Real64 Y_min = 0.0;
        Real64 Width = 0.0;
        Real64 Height = 0.0;

        // Default Constructor
        RectangleF() = default;

        // Member Constructor
        RectangleF(Real64 const X_min, Real64 const Y_min, Real64 const Width, Real64 const Height)
            : X_min(X_min), Y_min(Y_min), Width(Width), Height(Height)
        {
        }

        bool inline contains(EnergyPlus::PlantPipingSystemsManager::PointF p) const
        {
            return ((this->X_min <= p.X) && (p.X < (this->X_min + this->Width)) && (this->Y_min <= p.Y) && (p.Y < (this->Y_min + this->Height)));
        }
    };

    struct NeighborInformation
    {
        // Members
        Real64 ThisCentroidToNeighborWall = 0.0;
        Real64 ThisWallToNeighborCentroid = 0.0;
        Real64 adiabaticMultiplier = 1.0;
        Direction direction = Direction::NegativeX;

        // Default Constructor
        NeighborInformation() = default;
    };

    struct CartesianCell : BaseCell
    {
        // Members
        int X_index = 0;
        int Y_index = 0;
        int Z_index = 0;
        Real64 X_min = 0.0;
        Real64 X_max = 0.0;
        Real64 Y_min = 0.0;
        Real64 Y_max = 0.0;
        Real64 Z_min = 0.0;
        Real64 Z_max = 0.0;
        Point3DReal Centroid;
        CellType cellType = CellType::Invalid;
        std::map<Direction, NeighborInformation> NeighborInfo;
        CartesianPipeCellInformation PipeCellData;
        BaseThermalPropertySet Properties;

        // Default Constructor
        CartesianCell() = default;

        Real64 inline width() const
        {
            return this->X_max - this->X_min;
        }

        Real64 inline height() const
        {
            return this->Y_max - this->Y_min;
        }

        Real64 inline depth() const
        {
            return this->Z_max - this->Z_min;
        }

        Real64 inline XNormalArea() const
        {
            return this->depth() * this->height();
        }

        Real64 inline YNormalArea() const
        {
            return this->depth() * this->width();
        }

        Real64 inline ZNormalArea() const
        {
            return this->width() * this->height();
        }

        Real64 inline volume() const
        {
            return this->width() * this->depth() * this->height();
        }

        Real64 normalArea(Direction direction) const;

        void EvaluateNeighborCoordinates(Direction CurDirection, int &NX, int &NY, int &NZ) const;
    };

    struct MeshExtents
    {
        // Members
        Real64 xMax = 0.0;
        Real64 yMax = 0.0;
        Real64 zMax = 0.0;

        // Default Constructor
        MeshExtents() = default;

        // Member Constructor
        MeshExtents(Real64 const xMax, Real64 const yMax, Real64 const zMax) : xMax(xMax), yMax(yMax), zMax(zMax)
        {
        }
    };

    struct CellExtents : MeshExtents
    {
        // Members
        Real64 Xmin;
        Real64 Ymin;
        Real64 Zmin;

        // Member Constructor
        CellExtents(Real64 const Xmax, Real64 const Ymax, Real64 const Zmax, Real64 const Xmin, Real64 const Ymin, Real64 const Zmin)
            : MeshExtents(Xmax, Ymax, Zmax), Xmin(Xmin), Ymin(Ymin), Zmin(Zmin)
        {
        }
    };

    struct DistributionStructure
    {
        // Members
        MeshDistribution thisMeshDistribution = MeshDistribution::Uniform;
        int RegionMeshCount = 0;
        Real64 GeometricSeriesCoefficient = 0.0;

        // Default Constructor
        DistributionStructure() = default;
    };

    struct MeshProperties
    {
        // Members
        DistributionStructure X;
        DistributionStructure Y;
        DistributionStructure Z;

        // Default Constructor
        MeshProperties() = default;
    };

    struct SimulationControl
    {
        // Members
        Real64 MinimumTemperatureLimit = -1000;
        Real64 MaximumTemperatureLimit = 1000;
        Real64 Convergence_CurrentToPrevIteration = 0.0;
        int MaxIterationsPerTS = 0;

        // Default Constructor
        SimulationControl() = default;
    };

    struct BasementZoneInfo
    {
        // Members
        Real64 Depth = 0;  // m
        Real64 Width = 0;  // m
        Real64 Length = 0; // m
        bool ShiftPipesByWidth = false;
        std::string WallBoundaryOSCMName;
        int WallBoundaryOSCMIndex = 0;
        std::string FloorBoundaryOSCMName;
        int FloorBoundaryOSCMIndex = 0;
        std::vector<int> WallSurfacePointers;
        std::vector<int> FloorSurfacePointers;
        int BasementWallXIndex = -1;
        int BasementFloorYIndex = -1;

        // Default Constructor
        BasementZoneInfo() = default;
    };

    struct MeshPartitions
    {
        // Members
        std::vector<MeshPartition> X;
        std::vector<MeshPartition> Y;
        std::vector<MeshPartition> Z;

        // Default Constructor
        MeshPartitions() = default;
    };

    struct MoistureInfo
    {
        // Members
        Real64 Theta_liq = 0.3; // volumetric moisture content of the soil
        Real64 Theta_sat = 0.5; // volumetric moisture content of soil at saturation
        Real64 GroundCoverCoefficient = 0.408;
        Real64 rhoCP_soil_liq = 0.0;
        Real64 rhoCP_soil_transient = 0.0;
        Real64 rhoCP_soil_ice = 0.0;
        Real64 rhoCp_soil_liq_1 = 0.0;

        // Default Constructor
        MoistureInfo() = default;
    };

    struct CurSimConditionsInfo
    {
        // Members
        // Simulation conditions
        Real64 PrevSimTimeSeconds = -1.0;
        Real64 CurSimTimeSeconds = 0.0;
        Real64 CurSimTimeStepSize = 0.0;
        // Environmental conditions
        Real64 CurAirTemp = 10.0;
        Real64 CurWindSpeed = 2.6;
        Real64 CurIncidentSolar = 0.0;
        Real64 CurRelativeHumidity = 100.0;

        // Default Constructor
        CurSimConditionsInfo() = default;
    };

    struct Segment
    {
        // Members
        // ID
        std::string Name;
        // Misc inputs
        PointF PipeLocation;
        Point PipeCellCoordinates;
        SegmentFlow FlowDirection = SegmentFlow::IncreasingZ;
        // Reporting variables
        Real64 InletTemperature = 0.0;
        Real64 OutletTemperature = 0.0;
        Real64 FluidHeatLoss = 0.0;
        // Error handling flags
        bool PipeCellCoordinatesSet = false;
        // Other flags
        bool IsActuallyPartOfAHorizontalTrench = false;

        // Default Constructor
        Segment() = default;

        void initPipeCells(int x, int y);

        bool operator==(std::string const &a)
        {
            return this->Name == a;
        }

        static Segment *factory(EnergyPlusData &state, const std::string &segmentName);
    };

    struct Circuit : public PlantComponent
    {

        // Members
        // ID
        std::string Name;
        // Inlet and outlet information
        std::string InletNodeName;
        std::string OutletNodeName;
        int InletNodeNum = 0;
        int OutletNodeNum = 0;
        Point3DInteger CircuitInletCell;
        Point3DInteger CircuitOutletCell;
        // Names and pointers to pipe segments found in this pipe circuit
        std::vector<Segment *> pipeSegments;
        // Pointer to the domain which contains this pipe circuit
        int ParentDomainIndex = 0;
        // Misc inputs
        RadialSizing PipeSize;
        RadialSizing InsulationSize;
        Real64 RadialMeshThickness = 0.0;
        bool HasInsulation = false;
        Real64 DesignVolumeFlowRate = 0.0;
        Real64 DesignMassFlowRate = 0.0;
        Real64 Convergence_CurrentToPrevIteration = 0.0;
        int MaxIterationsPerTS = 0;
        int NumRadialCells = 0;
        BaseThermalPropertySet PipeProperties;
        BaseThermalPropertySet InsulationProperties;
        // Flags
        bool NeedToFindOnPlantLoop = true;
        bool IsActuallyPartOfAHorizontalTrench = false;
        // Location of this pipe circuit in the PlantLoop topology
        PlantLocation plantLoc{};
        ExtendedFluidProperties CurFluidPropertySet; // is_used
        // Variables used to pass information from INIT-type routines to CALC-type routines
        Real64 CurCircuitInletTemp = 23.0;
        Real64 CurCircuitFlowRate = 0.1321;
        Real64 CurCircuitConvectionCoefficient = 0.0;
        // Reporting variables
        Real64 InletTemperature = 0.0;
        Real64 OutletTemperature = 0.0;
        Real64 FluidHeatLoss = 0.0;

        // Default Constructor
        Circuit() = default;

        virtual ~Circuit() = default;

        void initInOutCells(CartesianCell const &in, CartesianCell const &out);

        static PlantComponent *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType, const std::string &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        bool operator==(std::string const &a)
        {
            return this->Name == a;
        }

        static Circuit *factory(EnergyPlusData &state, const std::string &circuit, bool &errorsFound);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    struct ZoneCoupledSurfaceData
    {
        // Members
        // ID
        std::string Name;
        // Surface data
        int IndexInSurfaceArray;
        Real64 SurfaceArea;
        Real64 Width;
        Real64 Length;
        Real64 Depth;
        Real64 Conductivity;
        Real64 Density;
        Real64 InsulationConductivity;
        Real64 InsulationDensity;
        int Zone;

        // Default Constructor
        ZoneCoupledSurfaceData()
            : IndexInSurfaceArray(0), SurfaceArea(0.0), Width(0.0), Length(0.0), Depth(0.0), Conductivity(0.0), Density(0.0),
              InsulationConductivity(0.0), InsulationDensity(0.0), Zone(0)
        {
        }
    };

    struct Domain
    {
        // Members
        // ID
        std::string Name;
        // Names and pointers to circuits found in this domain
        std::vector<Circuit *> circuits;
        int MaxIterationsPerTS;
        // Flag variables
        bool OneTimeInit;
        bool BeginSimInit;
        bool BeginSimEnvironment;
        bool DomainNeedsSimulation;
        bool DomainNeedsToBeMeshed;
        bool IsActuallyPartOfAHorizontalTrench;
        bool HasAPipeCircuit;
        bool HasZoneCoupledSlab;
        bool HasZoneCoupledBasement;
        // "Input" data structure variables
        MeshExtents Extents;
        MeshProperties Mesh;
        BaseThermalPropertySet GroundProperties;
        BaseThermalPropertySet SlabProperties;
        BaseThermalPropertySet BasementInterfaceProperties;
        BaseThermalPropertySet HorizInsProperties;
        BaseThermalPropertySet VertInsProperties;
        SimulationControl SimControls;
        GroundTemp::BaseGroundTempsModel *groundTempModel; // non-owning pointer
        BasementZoneInfo BasementZone;
        MoistureInfo Moisture;
        // "Internal" data structure variables
        MeshPartitions Partitions;
        CurSimConditionsInfo Cur;
        bool HasBasement;
        // Zone coupled variables
        std::vector<ZoneCoupledSurfaceData> ZoneCoupledSurfaces;
        int ZoneCoupledOSCMIndex;
        Real64 PerimeterOffset;
        SlabPosition slabPosition = SlabPosition::Invalid;
        int SlabMaterialNum;
        Real64 SlabArea;
        Real64 SlabWidth;
        Real64 SlabLength;
        Real64 SlabThickness;
        int XIndex;
        int YIndex;
        int ZIndex;
        int x_max_index;
        int y_max_index;
        int z_max_index;
        HorizInsulation HorizIns = HorizInsulation::Invalid;
        int HorizInsMaterialNum;
        Real64 HorizInsThickness;
        Real64 HorizInsWidth;
        Real64 HeatFlux;
        Real64 WallHeatFlux;
        Real64 FloorHeatFlux;
        Real64 AggregateHeatFlux;
        Real64 AggregateWallHeatFlux;
        Real64 AggregateFloorHeatFlux;
        int NumHeatFlux;
        bool ResetHeatFluxFlag;
        Real64 ConvectionCoefficient;
        bool VertInsPresentFlag;
        int VertInsMaterialNum;
        Real64 VertInsThickness;
        Real64 VertInsDepth;
        int XWallIndex;
        int YFloorIndex;
        int ZWallIndex;
        int InsulationXIndex;
        int InsulationYIndex;
        int InsulationZIndex;
#ifdef GET_OUT
        // Don't seem to be used anywhere
        bool SimTimeStepFlag = false;
        bool SimHourlyFlag = false;
        bool SimDailyFlag = false;
#endif // GET_OUT
        Real64 ZoneCoupledSurfaceTemp;
        Real64 BasementWallTemp;
        Real64 BasementFloorTemp;
        int NumDomainCells;
        int NumGroundSurfCells;
        int NumInsulationCells;
        int NumSlabCells;
        Array2D<Real64> WeightingFactor;
        Array2D<Real64> WeightedHeatFlux;
        Real64 TotalEnergyUniformHeatFlux = 0.0;
        Real64 TotalEnergyWeightedHeatFlux = 0.0;
        Real64 HeatFluxWeightingFactor = 0.0;
        std::vector<GridRegion> XRegions;
        std::vector<GridRegion> YRegions;
        std::vector<GridRegion> ZRegions;

        // Main 3D cells array
        Array3D<CartesianCell> Cells;

        // Dynamic indexes to available neighbor directions for a particular cell
        std::vector<Direction> NeighborFieldCells;
        std::vector<Direction> NeighborBoundaryCells;

        // Default Constructor
        Domain()
            : MaxIterationsPerTS(10), OneTimeInit(true), BeginSimInit(true), BeginSimEnvironment(true), DomainNeedsSimulation(true),
              DomainNeedsToBeMeshed(true), IsActuallyPartOfAHorizontalTrench(false), HasAPipeCircuit(true), HasZoneCoupledSlab(false),
              HasZoneCoupledBasement(false), HasBasement(false), ZoneCoupledOSCMIndex(0), PerimeterOffset(0.0), SlabMaterialNum(0), SlabArea(0.0),
              SlabWidth(0.0), SlabLength(0.0), SlabThickness(0.0), XIndex(0), YIndex(0), ZIndex(0), x_max_index(0), y_max_index(0), z_max_index(0),
              HorizInsMaterialNum(0), HorizInsThickness(0.0254), HorizInsWidth(0.0), HeatFlux(0.0), WallHeatFlux(0.0), FloorHeatFlux(0.0),
              AggregateHeatFlux(0.0), AggregateWallHeatFlux(0.0), AggregateFloorHeatFlux(0.0), NumHeatFlux(0), ResetHeatFluxFlag(true),
              ConvectionCoefficient(0.0), VertInsPresentFlag(false), VertInsMaterialNum(0), VertInsThickness(0.0254), VertInsDepth(0.0),
              XWallIndex(0), YFloorIndex(0), ZWallIndex(0), InsulationXIndex(0), InsulationYIndex(0), InsulationZIndex(0),
              ZoneCoupledSurfaceTemp(0.0), BasementWallTemp(0.0), BasementFloorTemp(0.0), NumDomainCells(0), NumGroundSurfCells(0),
              NumInsulationCells(0), NumSlabCells(0)
        {
            NeighborFieldCells.resize(6);
            NeighborBoundaryCells.resize(6);
        }

        void developMesh(EnergyPlusData &state);

        void createPartitionCenterList(EnergyPlusData &state);

        std::vector<GridRegion> createPartitionRegionList(EnergyPlusData &state,
                                                          std::vector<MeshPartition> const &ThesePartitionCenters,
                                                          bool PartitionsExist,
                                                          Real64 DirExtentMax);

        void createRegionList(std::vector<GridRegion> &Regions,
                              std::vector<GridRegion> const &ThesePartitionRegions,
                              Real64 DirExtentMax,
                              RegionType DirDirection,
                              bool PartitionsExist,
                              ObjexxFCL::Optional_int BasementWallXIndex = _,
                              ObjexxFCL::Optional_int BasementFloorYIndex = _,
                              ObjexxFCL::Optional_int t_XIndex = _,
                              ObjexxFCL::Optional_int t_XWallIndex = _,
                              ObjexxFCL::Optional_int t_InsulationXIndex = _,
                              ObjexxFCL::Optional_int t_YIndex = _,
                              ObjexxFCL::Optional_int t_YFloorIndex = _,
                              ObjexxFCL::Optional_int t_InsulationYIndex = _,
                              ObjexxFCL::Optional_int t_ZIndex = _,
                              ObjexxFCL::Optional_int t_ZWallIndex = _,
                              ObjexxFCL::Optional_int t_InsulationZIndex = _);

        void createCellArray(std::vector<Real64> const &XBoundaryPoints,
                             std::vector<Real64> const &YBoundaryPoints,
                             std::vector<Real64> const &ZBoundaryPoints);

        void setupCellNeighbors();

        void setupPipeCircuitInOutCells();

        int getCellWidthsCount(RegionType dir) const;

        void getCellWidths(GridRegion &g, RegionType direction) const;

        void addNeighborInformation(int X,
                                    int Y,
                                    int Z,
                                    Direction direction,
                                    Real64 ThisCentroidToNeighborWall,
                                    Real64 ThisWallToNeighborCentroid,
                                    Real64 ThisAdiabaticMultiplier);

        Real64 GetBasementWallHeatFlux(EnergyPlusData &state);

        Real64 GetBasementFloorHeatFlux(EnergyPlusData &state);

        void UpdateBasementSurfaceTemperatures(EnergyPlusData &state);

        Real64 GetZoneInterfaceHeatFlux(EnergyPlusData &state);

        void UpdateZoneSurfaceTemperatures(EnergyPlusData &state);

        Real64 GetAverageTempByType(EnergyPlusData &state, CellType cellType) const;

        void InitializeSoilMoistureCalcs();

        void EvaluateSoilRhoCp(Real64 CellTemp, Real64 &rhoCp) const;

        void ShiftTemperaturesForNewTimeStep();

        void ShiftTemperaturesForNewIteration();

        bool IsConverged_CurrentToPrevIteration();

        bool CheckForOutOfRangeTemps() const;

        void EvaluateNeighborCharacteristics(
            CartesianCell &ThisCell, Direction CurDirection, Real64 &NeighborTemp, Real64 &Resistance, Real64 &AdiabaticMultiplier);

        void EvaluateCellNeighborDirections(CartesianCell const &cell, int &NumFieldCells, int &NumBoundaryCells);

        void DoEndOfIterationOperations(EnergyPlusData &state, bool &Finished);

        void DoOneTimeInitializations(EnergyPlusData &state, Circuit *thisCircuit);

        void DoStartOfTimeStepInitializations(EnergyPlusData &state);

        void DoStartOfTimeStepInitializations(EnergyPlusData &state, Circuit *thisCircuit);

        Real64 GetFarfieldTemp(EnergyPlusData &state, CartesianCell const &cell);

        void PreparePipeCircuitSimulation(Circuit *thisCircuit);

        void PerformPipeCircuitSimulation(EnergyPlusData &state, Circuit *thisCircuit);

        void PerformPipeCellSimulation(Circuit *thisCircuit, CartesianCell &ThisCell, Real64 FlowRate, Real64 EnteringTemp);

        void SimulateRadialToCartesianInterface(CartesianCell &ThisCell);

        void PerformTemperatureFieldUpdate(EnergyPlusData &state);

        Real64 EvaluateFieldCellTemperature(CartesianCell &ThisCell);

        Real64 EvaluateGroundSurfaceTemperature(EnergyPlusData &state, CartesianCell &cell);

        Real64 EvaluateBasementCellTemperature(EnergyPlusData &state, CartesianCell &cell);

        Real64 EvaluateZoneInterfaceTemperature(CartesianCell &cell);

        Real64 EvaluateFarfieldBoundaryTemperature(EnergyPlusData &state, CartesianCell &cell);

        void EvaluateFarfieldCharacteristics(
            EnergyPlusData &state, CartesianCell &cell, Direction direction, Real64 &neighbortemp, Real64 &resistance, Real64 &adiabaticMultiplier);

        void PerformIterationLoop(EnergyPlusData &state);

        void PerformIterationLoop(EnergyPlusData &state, Circuit *thisCircuit);

        void InitPipingSystems(EnergyPlusData &state, Circuit *thisCircuit);

        void UpdatePipingSystems(EnergyPlusData &state, Circuit *thisCircuit);

        void SetupZoneCoupledOutputVariables(EnergyPlusData &state);
    };

    void SimulateGroundDomains(EnergyPlusData &state, bool initOnly);

    Real64 calcFluxWeightingFactor(const Real64 weightedHeatFlux, const Real64 uniformHeatFlux);

    void CheckIfAnySlabs(EnergyPlusData &state);

    void CheckIfAnyBasements(EnergyPlusData &state);

    void GetPipingSystemsAndGroundDomainsInput(EnergyPlusData &state);

    void ReadGeneralDomainInputs(EnergyPlusData &state, const int IndexStart, const int NumGeneralizedDomains, bool &ErrorsFound);

    void ReadZoneCoupledDomainInputs(EnergyPlusData &state, const int StartingDomainNumForZone, const int NumZoneCoupledDomains, bool &ErrorsFound);

    void ReadBasementInputs(EnergyPlusData &state, const int StartingDomainNumForBasement, const int NumBasements, bool &ErrorsFound);

    bool SiteGroundDomainUsingNoMassMat(EnergyPlusData &state, Real64 const MaterialThickness, int const MaterialNum);

    void SiteGroundDomainNoMassMatError(EnergyPlusData &state,
                                        std::string_view FieldName,
                                        std::string const &UserInputField,
                                        std::string const &ObjectName);

    void ReadPipeCircuitInputs(EnergyPlusData &state, bool &ErrorsFound);

    void ReadPipeSegmentInputs(EnergyPlusData &state, bool &ErrorsFound);

    void ReadHorizontalTrenchInputs(EnergyPlusData &state,
                                    const int StartingDomainNumForHorizontal,
                                    const int StartingCircuitNumForHorizontal,
                                    bool &ErrorsFound);

    void SetupPipingSystemOutputVariables(EnergyPlusData &state);

    void IssueSevereInputFieldError(EnergyPlusData &state,
                                    std::string_view const RoutineName,
                                    std::string const &ObjectName,
                                    std::string const &InstanceName,
                                    std::string_view FieldName,
                                    std::string const &FieldEntry,
                                    std::string const &Condition,
                                    bool &ErrorsFound);

    void IssueSevereInputFieldError(EnergyPlusData &state,
                                    std::string_view const RoutineName,
                                    std::string const &ObjectName,
                                    std::string const &InstanceName,
                                    std::string_view FieldName,
                                    Real64 FieldEntry,
                                    std::string const &Condition,
                                    bool &ErrorsFound);

    int GetSurfaceCountForOSCM(EnergyPlusData &state, int OSCMIndex);

    std::vector<int> GetSurfaceIndecesForOSCM(EnergyPlusData &state, int OSCMIndex);

    std::vector<ZoneCoupledSurfaceData> GetSurfaceDataForOSCM(EnergyPlusData &state, int OSCMIndex);

    bool inline IsInRangeReal(Real64 const r, Real64 const lower, Real64 const upper)
    {
        return ((r >= lower) && (r <= upper));
    }

    bool inline IsInRange_BasementModel(Real64 const r, Real64 const lower, Real64 const upper)
    {
        return ((r >= lower) && (r < upper));
    }

    void ShiftPipeTemperaturesForNewIteration(CartesianCell &ThisPipeCell);

    std::vector<Real64> CreateBoundaryList(std::vector<GridRegion> const &RegionList, Real64 DirExtentMax, RegionType DirDirection);

    void SimulateOuterMostRadialSoilSlice(Circuit *thisCircuit, CartesianCell &ThisCell);

    void SimulateAllInteriorRadialSoilSlices(CartesianCell &ThisCell);

    void SimulateInnerMostRadialSoilSlice(Circuit *thisCircuit, CartesianCell &ThisCell);

    void SimulateRadialInsulationCell(CartesianCell &ThisCell);

    void SimulateRadialPipeCell(Circuit const *thisCircuit, CartesianCell &ThisCell);

    void SimulateFluidCell(Circuit const *thisCircuit, CartesianCell &ThisCell, Real64 FlowRate, Real64 EnteringFluidTemp);

    bool IsConverged_PipeCurrentToPrevIteration(Circuit const *thisCircuit, CartesianCell const &CellToCheck);

} // namespace PlantPipingSystemsManager

struct PlantPipingSysMgrData : BaseGlobalStruct
{

    bool GetInputFlag = true;
    bool GetSegmentInputFlag = true;
    bool GetCircuitInputFlag = true;
    bool WriteEIOFlag = true;
    std::vector<PlantPipingSystemsManager::Domain> domains;
    std::vector<PlantPipingSystemsManager::Circuit> circuits;
    std::vector<PlantPipingSystemsManager::Segment> segments;
    std::unordered_map<std::string, std::string> GroundDomainUniqueNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetInputFlag = true;
        this->GetSegmentInputFlag = true;
        this->GetCircuitInputFlag = true;
        this->WriteEIOFlag = true;
        this->domains.clear();
        this->circuits.clear();
        this->segments.clear();
        this->GroundDomainUniqueNames.clear();
    }
};

} // namespace EnergyPlus

#endif
