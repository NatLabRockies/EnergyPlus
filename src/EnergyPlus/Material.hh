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

#ifndef Material_hh_INCLUDED
#define Material_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>
#include <EnergyPlus/WindowModel.hh>

namespace EnergyPlus {

// Forward declarations
namespace Curve {
    struct Curve;
}

namespace Material {

    // Parameters to indicate material group type for use with the Material
    // derived type (see below):

    // Don't change these numbers because they are printed out by number in SQLite (shouldn't print out internal enums by number)
    enum class Group
    {
        Invalid = -1,
        Regular,
        AirGap,
        Shade,
        Glass,
        Gas,
        Blind,
        GasMixture,
        Screen,
        EcoRoof,
        IRTransparent,
        GlassSimple,
        ComplexShade,
        ComplexWindowGap,
        GlassEQL,
        ShadeEQL,
        DrapeEQL,
        BlindEQL,
        ScreenEQL,
        WindowGapEQL,
        GlassTCParent,
        Num
    };

    enum class GasType
    {
        Invalid = -1,
        Custom,
        Air,
        Argon,
        Krypton,
        Xenon,
        Num
    };

    enum class GapVentType
    {
        Invalid = -1,
        Sealed,
        VentedIndoor,
        VentedOutdoor,
        Num
    };

    constexpr std::array<std::string_view, (int)GapVentType::Num> gapVentTypeNamesUC = {"SEALED", "VENTEDINDOOR", "VENTEDOUTDOOR"};

    extern const std::array<std::string_view, (int)GasType::Num> gasTypeNames;
    extern const std::array<std::string_view, (int)GapVentType::Num> gapVentTypeNames;

    enum class SlatAngleType
    {
        Invalid = -1,
        FixedSlatAngle,
        MaximizeSolar,
        BlockBeamSolar,
        Num
    };

    constexpr std::array<std::string_view, (int)SlatAngleType::Num> slatAngleTypeNamesUC = {"FIXEDSLATANGLE", "MAXIMIZESOLAR", "BLOCKBEAMSOLAR"};

    // Parameter for window screens beam reflectance accounting
    enum class ScreenBeamReflectanceModel
    {
        Invalid = -1,
        DoNotModel,
        DirectBeam,
        Diffuse,
        Num
    };

    constexpr std::array<std::string_view, (int)ScreenBeamReflectanceModel::Num> screenBeamReflectanceModelNamesUC = {
        "DONOTMODEL", "MODELASDIRECTBEAM", "MODELASDIFFUSE"};

    enum class VariableAbsCtrlSignal
    {
        Invalid = -1,
        SurfaceTemperature,
        SurfaceReceivedSolarRadiation,
        SpaceHeatingCoolingMode,
        Scheduled,
        Num
    };

    constexpr std::array<std::string_view, (int)VariableAbsCtrlSignal::Num> variableAbsCtrlSignalNamesUC = {
        "SURFACETEMPERATURE", "SURFACERECEIVEDSOLARRADIATION", "SPACEHEATINGCOOLINGMODE", "SCHEDULED"};

    // Parameters to indicate surface roughness for use with the Material
    // derived type:
    enum class SurfaceRoughness
    {
        Invalid = -1,
        VeryRough,
        Rough,
        MediumRough,
        MediumSmooth,
        Smooth,
        VerySmooth,
        Num
    };

    constexpr std::array<std::string_view, (int)SurfaceRoughness::Num> surfaceRoughnessNamesUC{
        "VERYROUGH", "ROUGH", "MEDIUMROUGH", "MEDIUMSMOOTH", "SMOOTH", "VERYSMOOTH"};

    extern const std::array<std::string_view, (int)SurfaceRoughness::Num> surfaceRoughnessNames;

    // Class Hierarchy
    //
    // MaterialBase: Material, Material:AirGap, Material:NoMass, Material:InfraredTransparent
    //    |
    //    | MaterialGasMix: WindowMaterial:Gas, WindowMaterial:GasMixture
    //    |    | MaterialComplexGap: WindowMaterial:Gap (includes WindowGap:DeflectionState and WindowGap:SupportPillar)
    //    |
    //    | MaterialFen: abstract
    //    |    | MaterialGlass: WindowMaterial:Glazing, WindowMaterial:Glazing:RefractionExtinctionMethod
    //    |    |    | MaterialGlassEQL: WindowMaterial:Glazing:EquivalentLayer
    //    |    |    | MaterialGlassTCParent: WindowMaterial:GlazingGroup:Thermochromic
    //    |    |
    //    |    | MaterialShadingDevice: abstract
    //    |    |    | MaterialShade: WindowMaterial:Shade
    //    |    |    | MaterialBlind: WindowMaterial:Blind
    //    |    |    | MaterialScreen: WindowMaterial:Screen
    //    |    |    | MaterialComplexShade: WindowMaterial:ComplexShade
    //    |    |    |    | MaterialShadeEQL: WindowMaterial:Shade:EquivalentLayer
    //    |    |    |    | MaterialBlindEQL: WindowMaterial:Blind:EquivalentLayer
    //    |    |    |    | MaterialDrapeEQL: WindowMaterial:Drape:EquivalentLayer
    //    |    |    |    | MaterialScreenEQL: WindowMaterial:Screen:EquivalentLayer
    //    |
    //    | MaterialEcoRoof: Material:RoofVegeration
    //    |
    //    | MaterialEMPD: holder of EMPD MaterialProperties.
    //    |
    //    | MaterialHAMT: holder of HAMT MaterialProperties.
    //    |
    //    | MaterialPhaseChange: holder of PhaseChange MaterialProperties.

    // This class is used for Material:AirGap, Material,
    // Material:NoMass, and Material:InfraredTransparent.  It is also
    // the base class for all other materials.  Material:AirGap,
    // Material:NoMass, and Material:IRT could be a much smaller
    // parent class of this, but Material:Regular is by far the most
    // common material so it makes sense to have it as the base class
    // to avoid an excess of dynamic casting.
    struct MaterialBase
    {
        // Members
        std::string Name = "";        // Name of material layer
        int Num = 0;                  // Index in material array, comes in handy sometimes
        Group group = Group::Invalid; // Material group type (see Material Parameters above.  Currently

        bool isUsed = false;

        // active: RegularMaterial, Shade, Air, WindowGlass,
        // WindowGas, WindowBlind, WindowGasMixture, Screen, EcoRoof,
        // IRTMaterial, WindowSimpleGlazing, ComplexWindowShade, ComplexWindowGap)
        SurfaceRoughness Roughness = SurfaceRoughness::Invalid; // Surface roughness index (See Surface Roughness parameters
        // above.  Current: VerySmooth, Smooth, MediumSmooth,
        // MediumRough, Rough, VeryRough)
        // Thermo-physical material properties
        Real64 Conductivity = 0.0; // Thermal conductivity of layer (W/m2K)
        Real64 Density = 0.0;      // Layer density (kg/m3)
        Real64 Resistance = 0.0;   // Layer thermal resistance (alternative to Density,
        // Conductivity, Thickness, and Specific Heat; K/W)
        bool ROnly = false; // Material defined with "R" only
        Real64 NominalR = 0.0;
        Real64 SpecHeat = 0.0;  // Layer specific heat (J/kgK)
        Real64 Thickness = 0.0; // Layer thickness (m)

        Real64 AbsorpThermal = 0.0;      // Layer thermal absorptance
        Real64 AbsorpThermalInput = 0.0; // Layer thermal absorptance input by user
        Real64 AbsorpThermalBack = 0.0;  // Infrared radiation back absorption
        Real64 AbsorpThermalFront = 0.0; // Infrared radiation front absorption

        Real64 AbsorpSolar = 0.0;      // Layer solar absorptance
        Real64 AbsorpSolarInput = 0.0; // Layer solar absorptance input by user

        Real64 AbsorpVisible = 0.0;      // Layer Visible Absorptance
        Real64 AbsorpVisibleInput = 0.0; // Layer Visible Absorptance input by user

        // Radiation parameters // Are these for windows or for opaque materials also?
        bool AbsorpSolarEMSOverrideOn = false;   // if true, then EMS calling to override value for solar absorptance
        Real64 AbsorpSolarEMSOverride = false;   // value to use when EMS calling to override value for solar absorptance
        bool AbsorpThermalEMSOverrideOn = false; // if true, then EMS calling to override value for thermal absorptance
        Real64 AbsorpThermalEMSOverride = 0.0;   // value to use when EMS calling to override value for thermal absorptance
        bool AbsorpVisibleEMSOverrideOn = false; // if true, then EMS calling to override value for visible absorptance
        Real64 AbsorpVisibleEMSOverride = 0.0;   // value to use when EMS calling to override value for visible absorptance

        // dynamic thermal and solar absorptance coating parameters
        VariableAbsCtrlSignal absorpVarCtrlSignal = VariableAbsCtrlSignal::Invalid;
        Sched::Schedule *absorpThermalVarSched = nullptr;
        Curve::Curve *absorpThermalVarCurve = nullptr;
        Sched::Schedule *absorpSolarVarSched = nullptr;
        Curve::Curve *absorpSolarVarCurve = nullptr;

        bool hasEMPD = false;
        bool hasHAMT = false;
        bool hasPCM = false;

        // Moved these into the base class for SQLite purposes
        Real64 Porosity = 0.0; // Layer porosity
        // Real64 IsoMoistCap = 0.0;   // Isothermal moisture capacity on water vapor density (m3/kg)
        // Real64 ThermGradCoef = 0.0; // Thermal-gradient coefficient for moisture capacity based on the water vapor density (kg/kgK)
        Real64 VaporDiffus = 0.0; // Layer vapor diffusivity

        bool WarnedForHighDiffusivity = false; // used to limit error messaging to just the first instance

        MaterialBase()
        {
            group = Group::AirGap;
        }
        virtual ~MaterialBase() = default;
    };

    struct MaterialFen : public MaterialBase
    {
        // Are these just for windows?
        Real64 Trans = 0.0;        // Transmittance of layer (glass, shade)
        Real64 TransVis = 0.0;     // Visible transmittance (at normal incidence)
        Real64 TransThermal = 0.0; // Infrared radiation transmittance

        Real64 ReflectSolBeamBack = 0.0;  // Solar back reflectance (beam to everything)
        Real64 ReflectSolBeamFront = 0.0; // Solar front reflectance (beam to everything)

        MaterialFen() : MaterialBase()
        {
            group = Group::Invalid;
        }
        ~MaterialFen() = default;
        virtual bool can_instantiate() = 0;
    };

    // Abstract parent class for all WindowMaterial:X shading device materials (including Equivalent Layer).
    struct MaterialShadingDevice : public MaterialFen
    {
        // Some characteristics for blind thermal calculation
        Real64 toGlassDist = 0.0;    // Distance between window shade and adjacent glass (m)
        Real64 topOpeningMult = 0.0; // Area of air-flow opening at top of blind, expressed as a fraction
        //  of the blind-to-glass opening area at the top of the blind
        Real64 bottomOpeningMult = 0.0; // Area of air-flow opening at bottom of blind, expressed as a fraction
        //  of the blind-to-glass opening area at the bottom of the blind
        Real64 leftOpeningMult = 0.0; // Area of air-flow opening at left side of blind, expressed as a fraction
        //  of the blind-to-glass opening area at the left side of the blind
        Real64 rightOpeningMult = 0.0; // Area of air-flow opening at right side of blind, expressed as a fraction
        //  of the blind-to-glass opening area at the right side of the blind
        // Calculated blind properties

        Real64 airFlowPermeability = 0.0; // The effective area of openings in the shade itself, expressed as a
        //  fraction of the shade area
        MaterialShadingDevice() : MaterialFen()
        {
            group = Group::Invalid;
        }
        ~MaterialShadingDevice() = default;
        virtual bool can_instantiate() = 0; // Prevents this class from being instantiated
    };

    // Class for WindowMaterial:Shade
    struct MaterialShade : public MaterialShadingDevice
    {
        Real64 ReflectShade = 0.0;    // Shade or screen reflectance (interior shade only)
        Real64 ReflectShadeVis = 0.0; // Shade reflectance for visible radiation

        MaterialShade() : MaterialShadingDevice()
        {
            group = Group::Shade;
        }
        ~MaterialShade() = default;
        bool can_instantiate() override
        {
            return true;
        } // Allows this class to be instantiated
    };

    // This may seem like an overly complicated way to handle a set of
    // multi-dimensional variables, but I think that it is actually
    // cleaner than either a multi-dimensional array (and certaily
    // faster) and also better than just a long list of variables.

    // Blind-properties essentially have four dimensions: property
    // type (transmittance, reflectance, absorptance), beam or
    // diffuse, front or back, solar or visible (maybe solar or
    // visible or thermal/IR).  Rather than coming up with and
    // enforcing a consistent namming scheme for these variables,
    // arranging them into nested structres keeps the ordering (as
    // well as the naming) of the dimensions consistent, and also
    // inserts periods between the dimensions to help with
    // readability.  In this case, I chose the struct nesting to be
    // SolVis.Front.Back.BmDf.Prop.  So variables are always going to
    // be called Sol.Front.Df.Abs and Vis.Back.Df.Ref.

    // For the record, accessing variables in nested structs is as
    // fast as accessing plain scalar variables.  The reason is that
    // the compiler knows the position of every variable at every
    // level of a nested struct at compile time and can load it with a
    // single offset like it loads every other variable in an object.
    // This is another downside of references in C++.  Accessing
    // variables in structures (even nested structures) is fast.
    // Acessing variables through pointer indirection is slow.
    // W->X->Y->Z is slower than W.X.Y.Z.  References are pointers,
    // but they use the . notation rather than -> for accessing
    // fields.  So if W.X.Y.Z is implemented using nested structures
    // then it is fast, but if it is implemented using references to
    // structures then it is slow.  But without context there is no
    // way to tell which is which.

    struct BlindBmTAR
    {
        Real64 BmTra = 0.0; // Beam-beam transmittance
        Real64 DfTra = 0.0; // Beam-diff transmittance
        Real64 BmRef = 0.0; // Beam-beam reflectance
        Real64 DfRef = 0.0; // Beam-diff reflectance
        Real64 Abs = 0.0;   // Absorptance

        void interpSlatAng(BlindBmTAR const &t1, BlindBmTAR const &t2, Real64 interpFac)
        {
            BmTra = Interp(t1.BmTra, t2.BmTra, interpFac);
            DfTra = Interp(t1.DfTra, t2.DfTra, interpFac);
            BmRef = Interp(t1.BmRef, t2.BmRef, interpFac);
            DfRef = Interp(t1.DfRef, t2.DfRef, interpFac);
            Abs = Interp(t1.Abs, t2.Abs, interpFac);
        }
    };

    struct BlindDfTAR
    {
        Real64 Tra = 0.0;
        Real64 Abs = 0.0;
        Real64 Ref = 0.0;

        void interpSlatAng(BlindDfTAR const &t1, BlindDfTAR const &t2, Real64 interpFac)
        {
            Tra = Interp(t1.Tra, t2.Tra, interpFac);
            Ref = Interp(t1.Ref, t2.Ref, interpFac);
            Abs = Interp(t1.Abs, t2.Abs, interpFac);
        }
    };

    struct BlindDfTARGS
    {
        Real64 Tra = 0.0;
        Real64 TraGnd = 0.0;
        Real64 TraSky = 0.0;
        Real64 Ref = 0.0;
        Real64 RefGnd = 0.0;
        Real64 RefSky = 0.0;
        Real64 Abs = 0.0;
        Real64 AbsGnd = 0.0;
        Real64 AbsSky = 0.0;

        void interpSlatAng(BlindDfTARGS const &t1, BlindDfTARGS const &t2, Real64 interpFac)
        {
            Tra = Interp(t1.Tra, t2.Tra, interpFac);
            TraGnd = Interp(t1.TraGnd, t2.TraGnd, interpFac);
            TraSky = Interp(t1.TraSky, t2.TraSky, interpFac);
            Ref = Interp(t1.Ref, t2.Ref, interpFac);
            RefGnd = Interp(t1.RefGnd, t2.RefGnd, interpFac);
            RefSky = Interp(t1.RefSky, t2.RefSky, interpFac);
            Abs = Interp(t1.Abs, t2.Abs, interpFac);
            AbsGnd = Interp(t1.AbsGnd, t2.AbsGnd, interpFac);
            AbsSky = Interp(t1.AbsSky, t2.AbsSky, interpFac);
        }
    };

    template <int ProfAngs> struct BlindBmDf
    {
        std::array<BlindBmTAR, ProfAngs> Bm;
        BlindDfTARGS Df;

        void interpSlatAng(BlindBmDf const &t1, BlindBmDf const &t2, Real64 interpFac)
        {
            for (int i = 0; i < ProfAngs; ++i) {
                Bm[i].interpSlatAng(t1.Bm[i], t2.Bm[i], interpFac);
            }
            Df.interpSlatAng(t1.Df, t2.Df, interpFac);
        }
    };

    template <int ProfAngs> struct BlindFtBk
    {
        BlindBmDf<ProfAngs> Ft;
        BlindBmDf<ProfAngs> Bk;

        void interpSlatAng(BlindFtBk const &t1, BlindFtBk const &t2, Real64 interpFac)
        {
            Ft.interpSlatAng(t1.Ft, t2.Ft, interpFac);
            Bk.interpSlatAng(t1.Bk, t2.Bk, interpFac);
        }
    };

    struct BlindTraEmi
    {
        Real64 Tra = 0.0;
        Real64 Emi = 0.0;

        void interpSlatAng(BlindTraEmi const &t1, BlindTraEmi const &t2, Real64 interpFac)
        {
            Tra = Interp(t1.Tra, t2.Tra, interpFac);
            Emi = Interp(t1.Emi, t2.Emi, interpFac);
        }
    };

    struct BlindFtBkIR
    {
        BlindTraEmi Ft;
        BlindTraEmi Bk;

        void interpSlatAng(BlindFtBkIR const &t1, BlindFtBkIR const &t2, Real64 interpFac)
        {
            Ft.interpSlatAng(t1.Ft, t2.Ft, interpFac);
            Bk.interpSlatAng(t1.Bk, t2.Bk, interpFac);
        }
    };

    template <int ProfAngs> struct BlindTraAbsRef
    {
        BlindFtBk<ProfAngs> Sol;
        BlindFtBk<ProfAngs> Vis;
        BlindFtBkIR IR;

        void interpSlatAng(BlindTraAbsRef const &t1, BlindTraAbsRef const &t2, Real64 interpFac)
        {
            Sol.interpSlatAng(t1.Sol, t2.Sol, interpFac);
            Vis.interpSlatAng(t1.Vis, t2.Vis, interpFac);
            IR.interpSlatAng(t1.IR, t2.IR, interpFac);
        }
    };

    // Blind
    constexpr int MaxSlatAngs = 181;
    constexpr int MaxProfAngs = 37;

    constexpr Real64 dProfAng = Constant::Pi / (MaxProfAngs - 1);
    constexpr Real64 dSlatAng = Constant::Pi / (MaxSlatAngs - 1);

    // WindowMaterial:Blind class
    struct MaterialBlind : public MaterialShadingDevice
    {
        // Input properties
        DataWindowEquivalentLayer::Orientation SlatOrientation = DataWindowEquivalentLayer::Orientation::Invalid; // HORIZONTAL or VERTICAL
        DataWindowEquivalentLayer::AngleType SlatAngleType = DataWindowEquivalentLayer::AngleType::Fixed;         // FIXED or VARIABLE
        Real64 SlatWidth = 0.0;                                                                                   // Slat width (m)
        Real64 SlatSeparation = 0.0;                                                                              // Slat separation (m)
        Real64 SlatThickness = 0.0;                                                                               // Slat thickness (m)
        Real64 SlatCrown = 0.0;        // the height of the slate (length from the chord to the curve)
        Real64 SlatAngle = 0.0;        // Slat angle (deg)
        Real64 MinSlatAngle = 0.0;     // Minimum slat angle for variable-angle slats (deg) (user input)
        Real64 MaxSlatAngle = 0.0;     // Maximum slat angle for variable-angle slats (deg) (user input)
        Real64 SlatConductivity = 0.0; // Slat conductivity (W/m-K)

        BlindTraAbsRef<1> slatTAR; // Beam properties for slats are not profile-angle dependent

        // Beam properties for blinds are profile angle dependent, so template must be instantiated with MaxProfAngs+1
        std::array<BlindTraAbsRef<MaxProfAngs + 1>, MaxSlatAngs> TARs;

        // Default Constructor
        MaterialBlind() : MaterialShadingDevice()
        {
            group = Group::Blind;
        }
        ~MaterialBlind() = default;
        bool can_instantiate()
        {
            return true;
        } // This function allows this class to be instantiated

        Real64 BeamBeamTrans(Real64 profAng, Real64 slatAng) const;
    };

    // WindowMaterial:ComplexShade class
    struct MaterialComplexShade : public MaterialShadingDevice
    {
        // Layer type (OtherShadingType, Venetian, Woven, Perforated)
        TARCOGParams::TARCOGLayerType LayerType = TARCOGParams::TARCOGLayerType::Invalid;
        Real64 FrontEmissivity = 0.0;  // Emissivity of front surface
        Real64 BackEmissivity = 0.0;   // Emissivity of back surface
        Real64 SlatWidth = 0.0;        // Slat width (m)
        Real64 SlatSpacing = 0.0;      // Slat spacing (m)
        Real64 SlatThickness = 0.0;    // Slat thickness (m)
        Real64 SlatAngle = 0.0;        // Slat angle (deg)
        Real64 SlatConductivity = 0.0; // Slat conductivity (W/m2K)
        Real64 SlatCurve = 0.0;        // Curvature radius of slat (if =0 then flat) (m)

        Real64 frontOpeningMult = 0.0;

        MaterialComplexShade() : MaterialShadingDevice()
        {
            group = Group::ComplexShade;
        }
        ~MaterialComplexShade() = default;
        bool can_instantiate()
        {
            return true;
        } // This function allows this class to be instantiated
    };

    int constexpr maxMixGases = 5;

    struct GasCoeffs
    {
        Real64 c0 = 0.0;
        Real64 c1 = 0.0;
        Real64 c2 = 0.0;
    };

    struct Gas
    {
        GasType type = GasType::Custom;
        GasCoeffs con = GasCoeffs();
        GasCoeffs vis = GasCoeffs();
        GasCoeffs cp = GasCoeffs();
        Real64 wght = 0.0;
        Real64 specHeatRatio = 0.0;
    };

    extern const std::array<Gas, 10> gases;

    // Class for WindowMaterial:Gas and WindowMaterial:GasMixture.  Parent class for Material:ComplexWindowGap
    struct MaterialGasMix : public MaterialBase
    {
        //  up to 5 gases in a mixture [Window gas only].  It is defined as parameter (GasCoefs)
        int numGases = 0; // Number of gases in a window gas mixture

        std::array<Real64, maxMixGases> gasFracts = {0.0};
        std::array<Gas, maxMixGases> gases = {Gas()};

        GapVentType gapVentType = GapVentType::Sealed; // Gap Ven type for equivalent Layer window model

        MaterialGasMix() : MaterialBase()
        {
            group = Group::Gas;
        }
        ~MaterialGasMix() = default;
    };

    // Class for Material:ComplexWindowGap
    struct MaterialComplexWindowGap : public MaterialGasMix
    {
        Real64 Pressure = 0.0;
        Real64 pillarSpacing = 0.0; // Spacing between centers of support pillars (m)
        Real64 pillarRadius = 0.0;  // Support pillar radius (m)
        Real64 deflectedThickness = 0.0;

        MaterialComplexWindowGap() : MaterialGasMix()
        {
            group = Group::ComplexWindowGap;
        }
        ~MaterialComplexWindowGap() = default;
    };

    struct ScreenBmTraAbsRef
    {
        struct
        {
            Real64 Tra = 0.0;
        } Bm;
        struct
        {
            Real64 Tra = 0.0;
        } Df;
        Real64 Abs = 0.0;
        Real64 Ref = 0.0;
    };

    struct ScreenBmTAR
    {
        struct
        {
            ScreenBmTraAbsRef Ft, Bk;
        } Sol;
        struct
        {
            ScreenBmTraAbsRef Ft, Bk;
        } Vis;
    };

    // Screen Beam Transmittance, Absorptance, Reflectance (TAR) properties
    struct ScreenBmTransAbsRef
    {
        Real64 BmTrans = 0.0; // Beam solar transmittance (dependent on sun angle)
        // (this value can include scattering if the user so chooses)
        Real64 BmTransBack = 0.0; // Beam solar transmittance (dependent on sun angle) from back side of screen
        Real64 BmTransVis = 0.0;  // Visible solar transmittance (dependent on sun angle)
        // (this value can include visible scattering if the user so chooses)
        Real64 DfTrans = 0.0;     // Beam solar transmitted as diffuse radiation (dependent on sun angle)
        Real64 DfTransBack = 0.0; // Beam solar transmitted as diffuse radiation (dependent on sun angle) from back side
        Real64 DfTransVis = 0.0;  // Visible solar transmitted as diffuse radiation (dependent on sun angle)

        // The following reflectance properties are dependent on sun angle:
        Real64 RefSolFront = 0.0; // Beam solar reflected as diffuse radiation when sun is in front of screen
        Real64 RefVisFront = 0.0; // Visible solar reflected as diffuse radiation when sun is in front of screen
        Real64 RefSolBack = 0.0;  // Beam solar reflected as diffuse radiation when sun is in back of screen
        Real64 RefVisBack = 0.0;  // Visible solar reflected as diffuse radiation when sun is in back of screen
        Real64 AbsSolFront = 0.0; // Front surface solar beam absorptance
        Real64 AbsSolBack = 0.0;  // Back surface solar beam absorptance
    };

#define PRECALC_INTERP_SCREEN

    constexpr int minDegResolution = 5;

    constexpr int maxIPhi = (Constant::Pi * Constant::RadToDeg / minDegResolution) + 1;
    constexpr int maxITheta = (Constant::Pi * Constant::RadToDeg / minDegResolution) + 1;

    // Class for Material:Screen
    struct MaterialScreen : public MaterialShadingDevice
    {
        Real64 diameterToSpacingRatio = 0.0; // ratio of screen material diameter to screen material spacing

        ScreenBeamReflectanceModel bmRefModel = ScreenBeamReflectanceModel::Invalid; // user specified method of accounting for scattered solar beam

        Real64 DfTrans = 0.0;    // Back surface diffuse solar transmitted
        Real64 DfTransVis = 0.0; // Back surface diffuse visible solar transmitted
        Real64 DfRef = 0.0;      // Back reflection of solar diffuse radiation
        Real64 DfRefVis = 0.0;   // Back reflection of visible diffuse radiation
        Real64 DfAbs = 0.0;      // Absorption of diffuse radiation

        Real64 ShadeRef = 0.0;       // Screen assembly solar reflectance (user input adjusted for holes in screen)
        Real64 ShadeRefVis = 0.0;    // Screen assembly visible reflectance (user input adjusted for holes in screen)
        Real64 CylinderRef = 0.0;    // Screen material solar reflectance (user input, does not account for holes in screen)
        Real64 CylinderRefVis = 0.0; // Screen material visible reflectance (user input, does not account for holes in screen)

        int mapDegResolution = 0;                                      // Resolution of azimuth and altitude angles to print in transmittance map
        Real64 dPhi = (Real64)minDegResolution * Constant::DegToRad;   // phi increments (rad)
        Real64 dTheta = (Real64)minDegResolution * Constant::DegToRad; // theta increments (rad)

        std::array<std::array<ScreenBmTransAbsRef, maxITheta>, maxIPhi> btars;

        MaterialScreen() : MaterialShadingDevice()
        {
            group = Group::Screen;
        }
        ~MaterialScreen() = default;
        bool can_instantiate() override
        {
            return true;
        } // Allows this class to be instantiated
    };

    // Class for Material:Shade:EquivalentLayer and parent class for
    // Material:Blind:EquivalentLayer, Material:Drape:EquivalentLayer
    // and MaterialScreen:EquivalentLayer
    struct MaterialShadeEQL : public MaterialShadingDevice
    {
        // Shading properties are profile-angle independent in EQL model
        BlindTraAbsRef<1> TAR;

        MaterialShadeEQL() : MaterialShadingDevice()
        {
            group = Group::ShadeEQL;
        }
        virtual ~MaterialShadeEQL() = default;
        bool can_instantiate() override
        {
            return true;
        } // Allows this class to be instantiated
    };

    // Class for Material:Screen:EquivalentLayer
    struct MaterialScreenEQL : public MaterialShadeEQL
    {
        Real64 wireSpacing = 0.0;  // insect screen wire spacing
        Real64 wireDiameter = 0.0; // insect screen wire diameter

        MaterialScreenEQL() : MaterialShadeEQL()
        {
            group = Group::ScreenEQL;
        }
        virtual ~MaterialScreenEQL() = default;
    };

    struct MaterialDrapeEQL : public MaterialShadeEQL
    {
        bool isPleated = false;     // if pleated drape= true, if nonpleated drape = false
        Real64 pleatedWidth = 0.0;  // width of the pleated drape fabric section
        Real64 pleatedLength = 0.0; // length of the pleated drape fabric section

        MaterialDrapeEQL() : MaterialShadeEQL()
        {
            group = Group::DrapeEQL;
        } // Can be any number of 'group' types, so don't set it here
        virtual ~MaterialDrapeEQL() = default;
    };

    struct MaterialBlindEQL : public MaterialShadeEQL
    {
        Real64 SlatWidth = 0.0;                                      // slat width
        Real64 SlatSeparation = 0.0;                                 // slat separation
        Real64 SlatCrown = 0.0;                                      // slat crown
        Real64 SlatAngle = 0.0;                                      // slat angle
        SlatAngleType slatAngleType = SlatAngleType::FixedSlatAngle; // slat angle control type, 0=fixed, 1=maximize solar, 2=block beam
        DataWindowEquivalentLayer::Orientation SlatOrientation = DataWindowEquivalentLayer::Orientation::Invalid; // horizontal or vertical

        MaterialBlindEQL() : MaterialShadeEQL()
        {
            group = Group::BlindEQL;
        } // Can be any number of 'group' types, so don't set it here
        virtual ~MaterialBlindEQL() = default;
    };

    // EcoRoof
    enum EcoRoofCalcMethod
    {
        Invalid = -1,
        Simple,
        SchaapGenuchten,
        Num
    };

    extern const std::array<std::string_view, (int)EcoRoofCalcMethod::Num> ecoRoofCalcMethodNamesUC;

    // Class for Material:RoofVegetation
    struct MaterialEcoRoof : public MaterialBase
    {
        // EcoRoof-Related properties, essentially for the plant layer,
        //    the soil layer uses the same resource as a regular material
        EcoRoofCalcMethod calcMethod = EcoRoofCalcMethod::Invalid; // 1-Simple, 2-SchaapGenuchten
        Real64 HeightOfPlants = 0.0;                               // plants' height
        Real64 LAI = 0.0;                                          // LeafAreaIndex (Dimensionless???)
        Real64 Lreflectivity = 0.0;                                // LeafReflectivity
        Real64 LEmissitivity = 0.0;                                // LeafEmissivity
        Real64 InitMoisture = 0.0;                                 // Initial soil moisture DJS
        Real64 MinMoisture = 0.0;                                  // Minimum moisture allowed DJS
        Real64 RStomata = 0.0;                                     // Minimum stomatal resistance DJS

        MaterialEcoRoof() : MaterialBase()
        {
            group = Group::EcoRoof;
        }
        ~MaterialEcoRoof() = default;
    };

    struct WindowThermalModelParams
    {
        // Members
        std::string Name;                                                                                   // Window thermal model name
        TARCOGGassesParams::Stdrd CalculationStandard = TARCOGGassesParams::Stdrd::Invalid;                 // Tarcog calculation standard
        TARCOGParams::TARCOGThermalModel ThermalModel = TARCOGParams::TARCOGThermalModel::Invalid;          // Tarcog thermal model
        Real64 SDScalar = 0.0;                                                                              // SDScalar coefficient
        TARCOGParams::DeflectionCalculation DeflectionModel = TARCOGParams::DeflectionCalculation::Invalid; // Deflection model
        Real64 VacuumPressureLimit = 0.0; // Pressure limit at which it will be considered vacuum gas state
        Real64 InitialTemperature = 0.0;  // Window(s) temperature in time of fabrication
        Real64 InitialPressure = 0.0;     // Window(s) pressure in time of fabrication
    };

    struct SpectralDataProperties
    {
        // Members
        std::string Name;           // Name of spectral data set
        int NumOfWavelengths = 0;   // Number of wavelengths in the data set
        Array1D<Real64> WaveLength; // Wavelength (microns)
        Array1D<Real64> Trans;      // Transmittance at normal incidence
        Array1D<Real64> ReflFront;  // Front reflectance at normal incidence
        Array1D<Real64> ReflBack;   // Back reflectance at normal incidence
    };

    struct MaterialGlass : public MaterialFen
    {
        // Window-related radiation parameters
        Real64 GlassTransDirtFactor = 1.0; // Multiplier on glass transmittance due to dirt
        bool SolarDiffusing = false;       // True if glass diffuses beam solar radiation
        Real64 ReflectSolDiffBack = 0.0;   // Solar back diffuse reflectance
        Real64 ReflectSolDiffFront = 0.0;  // Solar front diffuse reflectance
        Real64 ReflectVisBeamBack = 0.0;   // Visible back reflectance (beam to everything)
        Real64 ReflectVisBeamFront = 0.0;  // Visible front reflectance (beam to everything)
        Real64 ReflectVisDiffBack = 0.0;   // Visible back diffuse reflectance
        Real64 ReflectVisDiffFront = 0.0;  // Visible front diffuse reflectance
        Real64 TransSolBeam = 0.0;         // Solar transmittance (beam to everything)
        Real64 TransVisBeam = 0.0;         // Visible transmittance (beam to everything)
        // Complex fenestration parameters
        Real64 YoungModulus = 0.0;  // Young's modulus (Pa) - used in window deflection calculations
        Real64 PoissonsRatio = 0.0; // Poisson's ratio - used in window deflection calculations

        // Added 12/22/2008 for thermochromic window glazing material
        Real64 SpecTemp = 0.0;                          // Temperature corresponding to the specified material properties
        int TCParentMatNum = 0;                         // Reference to the parent object WindowMaterial:Glazing:Thermochromic
        int GlassSpectralDataPtr = 0;                   // Number of a spectral data set associated with a window glass material
        Curve::Curve *GlassSpecAngTransCurve = nullptr; // Transmittance as a function of spectral and angle associated with a glass material
        Curve::Curve *GlassSpecAngFReflCurve = nullptr; // Front reflectance as a function of spectral and angle associated with a glass material
        Curve::Curve *GlassSpecAngBReflCurve = nullptr; // Back reflectance as a function of spectral and angle associated with a glass material

        // TODO: these and others need to be moved to a child class
        // Simple Glazing System
        Real64 SimpleWindowUfactor = 0.0;       // user input for simple window U-factor with film coeffs (W/m2-k)
        Real64 SimpleWindowSHGC = 0.0;          // user input for simple window Solar Heat Gain Coefficient (non-dimensional)
        Real64 SimpleWindowVisTran = 0.0;       // (optional) user input for simple window Visual Transmittance (non-dimensional)
        bool SimpleWindowVTinputByUser = false; // false means not input, true means user provide VT input

        Window::OpticalDataModel windowOpticalData = Window::OpticalDataModel::SpectralAverage;

        MaterialGlass() : MaterialFen()
        {
            group = Group::Glass;
        }
        ~MaterialGlass() = default;
        bool can_instantiate()
        {
            return true;
        }

        void SetupSimpleWindowGlazingSystem(EnergyPlusData &state);
    };

    struct MaterialGlassEQL : public MaterialFen
    {
        BlindTraAbsRef<1> TAR;
        Window::OpticalDataModel windowOpticalData = Window::OpticalDataModel::SpectralAverage;

        MaterialGlassEQL() : MaterialFen()
        {
            group = Group::GlassEQL;
        }
        ~MaterialGlassEQL() = default;
        bool can_instantiate()
        {
            return true;
        }
    };

    struct MaterialRefSpecTemp
    {
        int matNum = 0;
        Real64 specTemp = 0.0;
    };

    struct MaterialGlassTC : public MaterialBase
    {
        // Members
        int numMatRefs = 0; // Number of TC glazing materials
        Array1D<MaterialRefSpecTemp> matRefs;

        MaterialGlassTC() : MaterialBase()
        {
            group = Group::GlassTCParent;
        }
        ~MaterialGlassTC() = default;
    };

    int GetMaterialNum(EnergyPlusData const &state, std::string const &matName);
    MaterialBase *GetMaterial(EnergyPlusData &state, std::string const &matName);

    void GetMaterialData(EnergyPlusData &state, bool &errorsFound); // set to true if errors found in input
    void GetVariableAbsorptanceInput(EnergyPlusData &state, bool &errorsFound);
    void GetWindowGlassSpectralData(EnergyPlusData &state, bool &errorsFound);

    // Angles must be in radians
    void GetRelativePhiTheta(Real64 phiWin, Real64 thetaWin, Vector3<Real64> const &solcos, Real64 &phi, Real64 &theta);
    void NormalizePhiTheta(Real64 &phi, Real64 &theta);
    void GetPhiThetaIndices(Real64 phi, Real64 theta, Real64 dPhi, Real64 dTheta, int &iPhi1, int &iPhi2, int &iTheta1, int &iTheta2);

    void CalcScreenTransmittance(EnergyPlusData &state,
                                 MaterialScreen const *screen,
                                 Real64 phi,   // Sun altitude relative to surface outward normal (rad)
                                 Real64 theta, // Sun azimuth relative to surface outward normal (rad)
                                 ScreenBmTransAbsRef &tar);

    void NormalizeProfSlat(Real64 &profAng, Real64 &slatAng);
    void GetProfIndices(Real64 profAng, int &iProf1, int &iProf2);
    void GetSlatIndicesInterpFac(Real64 slatAng, int &iSlat1, int &iSlat2, Real64 &interpFac);
} // namespace Material

struct MaterialData : BaseGlobalStruct
{
    Array1D<Material::MaterialBase *> materials;
    std::map<std::string, int> materialMap;

    int NumRegulars = 0;
    int NumNoMasses = 0;
    int NumIRTs = 0;
    int NumAirGaps = 0;
    int NumW5Glazings = 0;         // Window5 Glass Materials, specified by transmittance and front and back reflectance
    int NumW5AltGlazings = 0;      // Window5 Glass Materials, specified by index of refraction and extinction coeff
    int NumW5Gases = 0;            // Window5 Single-Gas Materials
    int NumW5GasMixtures = 0;      // Window5 Gas Mixtures
    int NumW7SupportPillars = 0;   // Complex fenestration support pillars
    int NumW7DeflectionStates = 0; // Complex fenestration deflection states
    int NumW7Gaps = 0;             // Complex fenestration material gaps
    int NumBlinds = 0;             // Total number of blind materials
    int NumScreens = 0;            // Total number of exterior window screen materials
    int NumTCGlazings = 0;         // Number of TC glazing object - WindowMaterial:Glazing:Thermochromic found in the idf file
    int NumShades = 0;             // Total number of shade materials
    int NumComplexGaps = 0;        // Total number of window gaps for complex fenestrations
    int NumSimpleWindows = 0;      // number of simple window systems.
    int NumEQLGlazings = 0;        // Window5 Single-Gas Materials for Equivalent Layer window model
    int NumEQLShades = 0;          // Total number of shade materials for Equivalent Layer window model
    int NumEQLDrapes = 0;          // Total number of drape materials for Equivalent Layer window model
    int NumEQLBlinds = 0;          // Total number of blind materials for Equivalent Layer window model
    int NumEQLScreens = 0;         // Total number of exterior window screen materials for Equivalent Layer window model
    int NumEQLGaps = 0;            // Window5 Equivalent Layer Single-Gas Materials
    int NumEcoRoofs = 0;

    bool AnyVariableAbsorptance = false;
    int NumSpectralData = 0;

    Array1D<Material::WindowThermalModelParams> WindowThermalModel;
    Array1D<Material::SpectralDataProperties> SpectralData;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        for (int i = 0; i < materials.isize(); ++i) {
            delete materials[i];
        }
        materials.clear();
        materialMap.clear();
    }
};

} // namespace EnergyPlus

#endif
