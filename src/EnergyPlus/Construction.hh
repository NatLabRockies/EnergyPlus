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

#ifndef Construction_hh_INCLUDED
#define Construction_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/WindowManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Construction {

    int constexpr MaxLayersInConstruct(11); // Maximum number of layers allowed in a single construction
    int constexpr MaxCTFTerms(19);          // Maximum number of CTF terms allowed to still allow stability
    // Note Sync with SurfaceGroundHeatExchanger::local::MaxCTFTerms
    // ** has to be big enough to hold no matter what window model
    //    each window model should validate layers individually

    // Nested one-field structs just to keep overall structure
    // consistent with Material::BlindTAR.  See Material.hh for
    // discussion of this approach.
    struct BlindSolVis
    {
        struct
        {
            struct
            {
                Material::BlindDfTARGS Df;
            } Ft;
            struct
            {
                Material::BlindDfTAR Df;
            } Bk;
        } Sol;
        struct
        {
            struct
            {
                Material::BlindDfTAR Df;
            } Ft;
            struct
            {
                Material::BlindDfTAR Df;
            } Bk;
        } Vis;
    };

    // Nested one-field structs keep overall structure consistent with Material::BlindTAR
    struct BlindSolDfAbs
    {
        struct
        {
            struct
            {
                struct
                {
                    Real64 Abs = 0.0;
                    Real64 AbsGnd = 0.0;
                    Real64 AbsSky = 0.0;
                } Df;
            } Ft; // Front

            struct
            {
                struct
                {
                    Real64 Abs = 0.0;
                } Df;
            } Bk; // Back
        } Sol;
    };

    struct TCLayer
    {
        int constrNum;
        Real64 specTemp;
    };

    // This needs to get broken up too
    struct ConstructionProps
    {
        // Members
        std::string Name;  // Name of construction
        int TotLayers = 0; // Total number of layers for the construction; for windows
        //  this is the total of the glass, gas and shade layers
        int TotSolidLayers = 0; // Total number of solid (glass or shade) layers (windows only)
        int TotGlassLayers = 0; // Total number of glass layers (windows only)
        Array1D_int LayerPoint; // Pointer array which refers back to
        // the Material structure; LayerPoint(i)=j->Material(j)%Name,etc
        bool IsUsed = false;              // Marked true when the construction is used
        bool IsUsedCTF = false;           // Mark true when the construction is used for a surface with CTF calculations
        bool IsCondFD = false;            // Mark true when SurfaceProperty:HeatTransferAlgorithm:Construction is set to ConductionFiniteDifference
        Real64 InsideAbsorpVis = 0.0;     // Inside Layer visible absorptance of an opaque surface; not used for windows.
        Real64 OutsideAbsorpVis = 0.0;    // Outside Layer visible absorptance of an opaque surface; not used for windows.
        Real64 InsideAbsorpSolar = 0.0;   // Inside Layer solar absorptance of an opaque surface; not used for windows.
        Real64 OutsideAbsorpSolar = 0.0;  // Outside Layer solar absorptance of an opaque surface; not used for windows.
        Real64 InsideAbsorpThermal = 0.0; // Inside Layer Thermal absorptance for opaque surfaces or windows;
        // for windows, applies to innermost glass layer
        Real64 OutsideAbsorpThermal = 0.0;                                                 // Outside Layer Thermal absorptance
        Material::SurfaceRoughness OutsideRoughness = Material::SurfaceRoughness::Invalid; // Outside Surface roughness index
        int DayltPropPtr = 0;                                                              // Pointer to Daylight Construction Properties
        int W5FrameDivider = 0; // FrameDivider number for window construction from Window5 data file;
        //  zero is construction not from Window5 file or Window5 construction has no frame.
        // Conductive properties for the construction
        std::array<Real64, MaxCTFTerms> CTFCross{};     // Cross or Y terms of the CTF equation
        std::array<Real64, MaxCTFTerms> CTFFlux{};      // Flux history terms of the CTF equation
        std::array<Real64, MaxCTFTerms> CTFInside{};    // Inside or Z terms of the CTF equation
        std::array<Real64, MaxCTFTerms> CTFOutside{};   // Outside or X terms of the CTF equation
        std::array<Real64, MaxCTFTerms> CTFSourceIn{};  // Heat source/sink inside terms of CTF equation
        std::array<Real64, MaxCTFTerms> CTFSourceOut{}; // Heat source/sink outside terms of CTF equation
        Real64 CTFTimeStep = 0.0;                       // Time increment for stable simulation of construct (could be greater than TimeStep)
        // The next three series of terms are used to calculate the temperature at the location of a source/sink
        // in the QTF formulation.  This calculation is necessary to allow the proper simulation of a
        // radiant system.
        std::array<Real64, MaxCTFTerms> CTFTSourceOut{}; // Outside terms of the CTF equation for interior temp
        // calc@source location
        std::array<Real64, MaxCTFTerms> CTFTSourceIn{}; // Inside terms of the CTF equation for interior temp
        // calc@source location
        std::array<Real64, MaxCTFTerms> CTFTSourceQ{}; // Source/sink terms of the CTF equation for interior temp
        // calc@source location
        // The next three series of terms are used to calculate the temperature at a location specified by the user.
        // This location must be between two layers and is intended to allow the user to evaluate whether or not
        // condensation is a possibility between material layers.
        std::array<Real64, MaxCTFTerms> CTFTUserOut{}; // Outside terms of the CTF equation for interior temp
        // calc@user location
        std::array<Real64, MaxCTFTerms> CTFTUserIn{}; // Inside terms of the CTF equation for interior temp
        // calc@user location
        std::array<Real64, MaxCTFTerms> CTFTUserSource{}; // Source/sink terms of the CTF equation for interior temp
        // calc@user location
        int NumHistories = 0; // CTFTimeStep/TimeStepZone or the number of temp/flux history series
        // for the construction
        int NumCTFTerms = 0;        // Number of CTF terms for this construction (not including terms at current time)
        Real64 UValue = 0.0;        // Overall heat transfer coefficient for the construction
        int SolutionDimensions = 0; // Number of dimensions in the solution (1 for normal constructions,
        // 1 or 2 for constructions with sources or sinks)-->may allow 3-D later?
        int SourceAfterLayer = 0; // Source/sink is present after this layer in the construction
        int TempAfterLayer = 0;   // User is requesting a temperature calculation after this layer in the construction
        // This location is also the position of a temperature on the interior of a slab
        // that could be used to control a low temperature radiant system
        Real64 ThicknessPerpend = 0.0; // Thickness between planes of symmetry in the direction
        // perpendicular to the main direction of heat transfer
        // (same as half the distance between tubes)
        Real64 userTemperatureLocationPerpendicular = 0.0; // Location of the source perpendicular to the main direction
        // of heat transfer.  Used in conjunction with the TempAfterLayer
        // term to provide specific location of user defined temperature.
        // This value is only used when SolutionDimension = 2.
        // Moisture Transfer Functions term belong here as well
        // BLAST detailed solar model parameters
        Real64 AbsDiffIn = 0.0;  // Inner absorptance coefficient for diffuse radiation
        Real64 AbsDiffOut = 0.0; // Outer absorptance coefficient for diffuse radiation
        // Variables for window constructions
        Array1D<Real64> AbsDiff; // Diffuse solar absorptance for each glass layer,
        // bare glass or shade on

        std::array<Real64, Material::MaxSlatAngs> effShadeBlindEmi;
        std::array<Real64, Material::MaxSlatAngs> effGlassEmi;

        std::array<BlindSolVis, Material::MaxSlatAngs> blindTARs;

        // Sol diffuse absorptance per glass layer with blind on
        Array1D<std::array<BlindSolDfAbs, Material::MaxSlatAngs>> layerSlatBlindDfAbs;

        Array1D<Real64> AbsDiffBack;                                  // Diffuse back solar absorptance for each glass layer
        Real64 AbsDiffShade = 0.0;                                    // Diffuse solar absorptance for shade
        Real64 AbsDiffBackShade = 0.0;                                // Diffuse back solar absorptance for shade
        Real64 ShadeAbsorpThermal = 0.0;                              // Diffuse back thermal absorptance of shade
        Array1D<std::array<Real64, Window::maxPolyCoef>> AbsBeamCoef; // Coefficients of incidence-angle polynomial for solar
        // absorptance for each solid glazing layer
        Array1D<std::array<Real64, Window::maxPolyCoef>> AbsBeamBackCoef;                          // As for AbsBeamCoef but for back-incident solar
        std::array<Real64, Window::maxPolyCoef> AbsBeamShadeCoef = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // Shade abs inc-angle coefs
        Real64 TransDiff = 0.0;           // Diffuse solar transmittance, bare glass or shade on
        Real64 TransDiffVis;              // Diffuse visible transmittance, bare glass or shade on
        Real64 ReflectSolDiffBack = 0.0;  // Diffuse back solar reflectance, bare glass or shade on
        Real64 ReflectSolDiffFront = 0.0; // Diffuse front solar reflectance, bare glass or shade on
        Real64 ReflectVisDiffBack = 0.0;  // Diffuse back visible reflectance, bare glass or shade on
        Real64 ReflectVisDiffFront = 0.0; // Diffuse front visible reflectance, bare glass or shade on
        std::array<Real64, Window::maxPolyCoef> TransSolBeamCoef = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};     // beam sol trans inc-angle coefs
        std::array<Real64, Window::maxPolyCoef> TransVisBeamCoef = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};     // beam vis trans inc-angle coefs
        std::array<Real64, Window::maxPolyCoef> ReflSolBeamFrontCoef = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // beam sol ref front inc-angle coefs
        std::array<Real64, Window::maxPolyCoef> ReflSolBeamBackCoef = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  // beam sol ref back inc-angle coefs
        Array1D<std::array<Real64, Window::maxPolyCoef>> tBareSolCoef;  // Isolated glass solar transmittance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> tBareVisCoef;  // Isolated glass visible transmittance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> rfBareSolCoef; // Isolated glass front solar reflectance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> rfBareVisCoef; // Isolated glass front visible reflectance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> rbBareSolCoef; // Isolated glass back solar reflectance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> rbBareVisCoef; // Isolated glass back visible reflectance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> afBareSolCoef; // Isolated glass front solar absorptance coeffs of inc. angle polynomial
        Array1D<std::array<Real64, Window::maxPolyCoef>> abBareSolCoef; // Isolated glass back solar absorptance coeffs of inc. angle polynomial
        Array1D<Real64> tBareSolDiff;                                   // Isolated glass diffuse solar transmittance
        Array1D<Real64> tBareVisDiff;                                   // Isolated glass diffuse visible transmittance
        Array1D<Real64> rfBareSolDiff;                                  // Isolated glass diffuse solar front reflectance
        Array1D<Real64> rfBareVisDiff;                                  // Isolated glass diffuse visible front reflectance
        Array1D<Real64> rbBareSolDiff;                                  // Isolated glass diffuse solar back reflectance
        Array1D<Real64> rbBareVisDiff;                                  // Isolated glass diffuse visible back reflectance
        Array1D<Real64> afBareSolDiff;                                  // Isolated glass diffuse solar front absorptance
        Array1D<Real64> abBareSolDiff;                                  // Isolated glass diffuse solar back absorptance
        bool FromWindow5DataFile = false;                               // True if this is a window construction extracted from the Window5 data file
        Real64 W5FileMullionWidth = 0.0;                                // Width of mullion for construction from Window5 data file (m)
        DataWindowEquivalentLayer::Orientation W5FileMullionOrientation =
            DataWindowEquivalentLayer::Orientation::Invalid; // Orientation of mullion, if present, for Window5 data file construction,
        Real64 W5FileGlazingSysWidth = 0.0;                  // Glass width for construction from Window5 data file (m)
        Real64 W5FileGlazingSysHeight = 0.0;                 // Glass height for construction form Window5 data file (m)
        Real64 SummerSHGC = 0.0;                             // Calculated ASHRAE SHGC for summer conditions
        Real64 VisTransNorm = 0.0;                           // The normal visible transmittance
        Real64 SolTransNorm = 0.0;                           // the normal solar transmittance
        bool SourceSinkPresent = false;                      // .TRUE. if there is a source/sink within this construction
        bool TypeIsWindow = false;                           // True if a window construction, false otherwise
        bool WindowTypeBSDF = false;                         // True for complex window, false otherwise
        bool TypeIsEcoRoof = false;                          // -- true for construction with ecoRoof outside, the flag
        //-- is turned on when the outside layer is of type EcoRoof
        bool TypeIsIRT = false;          // -- true for construction with IRT material
        bool TypeIsCfactorWall = false;  // -- true for construction with Construction:CfactorUndergroundWall
        bool TypeIsFfactorFloor = false; // -- true for construction with Construction:FfactorGroundFloor

        // Added TH 12/22/2008 for thermochromic windows
        bool isTCWindow = false;
        bool isTCMaster = false;
        int TCMasterConstrNum = 0; // The master TC construction referenced by its slave constructions
        int TCMasterMatNum = 0;    // The master TC material
        int TCLayerNum = 0;        // Which material layer is the TC glazing, counting all material layers.
        int TCGlassNum = 0;        // Which glass layer is the TC glazing, counting from glass layers only.
        int numTCChildConstrs;
        Array1D<TCLayer> TCChildConstrs;
        Real64 specTemp;

        // For CFactor underground walls
        Real64 CFactor = 0.0;
        Real64 Height = 0.0;
        // For FFactor slabs-on-grade or underground floors
        Real64 FFactor = 0.0;
        Real64 Area = 0.0;
        Real64 PerimeterExposed = 0.0;
        bool ReverseConstructionNumLayersWarning = false;
        bool ReverseConstructionLayersOrderWarning = false;
        // Complex Fenestration
        DataBSDFWindow::BSDFWindowInputStruct BSDFInput; // nest structure with user input for complex fenestration
        // EquivalentLayer Window
        bool WindowTypeEQL = false;      // True for equivalent layer window, false otherwise
        int EQLConsPtr = 0;              // Pointer to equivalent Layer window construction
        Array1D<Real64> AbsDiffFrontEQL; // Diffuse layer system front absorptance for EQL window
        Array1D<Real64> AbsDiffBackEQL;  // Diffuse layer system back absorptance for EQL window
        Real64 TransDiffFrontEQL = 0.0;  // Diffuse system front transmittance for EQL window
        Real64 TransDiffBackEQL = 0.0;   // Diffuse system back transmittance for EQL window
        // Air boundary
        bool TypeIsAirBoundary = false;                    // true for Construction:AirBoundary
        bool TypeIsAirBoundaryMixing = false;              // true for Construction:AirBoundary with SimpleMixing for air exchange
        Real64 AirBoundaryACH = 0.0;                       // Air boundary simple mixing air changes per hour [1/hr]
        Sched::Schedule *airBoundaryMixingSched = nullptr; // Air boundary simple mixing schedule index

        int rcmax = 0;              // Total number of nodes in the construct (<= MaxTotNodes)
        Array2D<Real64> AExp;       // Exponential of AMat
        Array2D<Real64> AInv;       // Inverse of AMat
        Array2D<Real64> AMat;       // "A" matrix from Seem's dissertation (constant coefficients of linear system)
        Array1D<Real64> BMat;       // "B" matrix of state space method (non-zero elements)
        Array1D<Real64> CMat;       // "C" matrix of state space method (non-zero elements)
        Array1D<Real64> DMat;       // "D" matrix of state space method (non-zero elements)
        Array1D<Real64> e;          // Coefficients for the surface flux history term
        Array2D<Real64> Gamma1;     // Intermediate calculation array corresponding to a term in Seem's dissertation
        Array2D<Real64> Gamma2;     // Intermediate calculation array corresponding to a term in Seem's dissertation
        Array3D<Real64> s;          // Coefficients for the surface temperature history terms
        Array2D<Real64> s0;         // Coefficients for the current surface temperature terms
        Array2D<Real64> IdenMatrix; // Identity Matrix
        int NumOfPerpendNodes = 7;  // Number of nodes in the direction
        // perpendicular to the main direction of heat transfer.  This is only used
        // when a two-dimensional solution has been requested for a construction
        // with a heat source/sink.
        int NodeSource = 0;   // Node at which a source or sink is present
        int NodeUserTemp = 0; // Node where user wishes to calculate a temperature (for constructions with sources/sinks only)

        // Default Constructor
        ConstructionProps()
            : LayerPoint(MaxLayersInConstruct, 0), TransDiffVis(0.0), tBareSolDiff(5, 0.0), tBareVisDiff(5, 0.0), rfBareSolDiff(5, 0.0),
              rfBareVisDiff(5, 0.0), rbBareSolDiff(5, 0.0), rbBareVisDiff(5, 0.0), afBareSolDiff(5, 0.0), abBareSolDiff(5, 0.0),
              AbsDiffFrontEQL(DataWindowEquivalentLayer::CFSMAXNL, 0.0), AbsDiffBackEQL(DataWindowEquivalentLayer::CFSMAXNL, 0.0)
        {
            BMat.allocate(3);
            CMat.allocate(2);
            DMat.allocate(2);
            s0.allocate(3, 4);
        }

        void calculateTransferFunction(EnergyPlusData &state, bool &ErrorsFound, bool &DoCTFErrorReport);

        void calculateExponentialMatrix(); // Time step of the resulting CTFs

        void calculateInverseMatrix();

        void calculateGammas();

        void calculateFinalCoefficients();

        void reportTransferFunction(EnergyPlusData &state, int cCounter);

        void reportLayers(EnergyPlusData &state);

        bool isGlazingConstruction(EnergyPlusData &state) const;

        Real64 setThicknessPerpendicular(EnergyPlusData &state, Real64 userValue);

        Real64 setUserTemperatureLocationPerpendicular(EnergyPlusData &state, Real64 userValue);

        void setNodeSourceAndUserTemp(Array1D_int &Nodes);

        void setArraysBasedOnMaxSolidWinLayers(EnergyPlusData &state);
    };
} // namespace Construction

struct ConstructionData : BaseGlobalStruct
{
    Array1D<Construction::ConstructionProps> Construct;
    Array1D_int LayerPoint = Array1D<int>(Construction::MaxLayersInConstruct, 0);

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ConstructionData();
    }
};

} // namespace EnergyPlus

#endif
