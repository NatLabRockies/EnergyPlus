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

#ifndef WindowComplexManager_hh_INCLUDED
#define WindowComplexManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/TARCOGGassesParams.hh>
#include <EnergyPlus/TARCOGParams.hh>

namespace EnergyPlus {

// forward declaration
struct EnergyPlusData;

namespace WindowComplexManager {

    enum class RayIdentificationType
    {
        Invalid = -1,
        Front_Incident,
        Front_Transmitted,
        Front_Reflected,
        Back_Incident,
        Back_Transmitted,
        Back_Reflected,
        Num
    };

    // Using/Aliasing
    using DataBSDFWindow::BasisElemDescr;
    using DataBSDFWindow::BasisStruct;
    using DataBSDFWindow::BSDFDaylghtPosition;
    using DataBSDFWindow::BSDFGeomDescr;
    using DataBSDFWindow::BSDFStateDescr;
    using DataBSDFWindow::BSDFWindowGeomDescr;
    using DataBSDFWindow::BSDFWindowInputStruct;
    using DataVectorTypes::Vector;

    struct WindowIndex
    {
        // Members
        int NumStates; // No States for this window
        int SurfNo;    // Surface number of window
        // Real64 Azimuth; // Window surface azimuth
        // Real64 Tilt; // Window surface tilt

        // Default Constructor
        WindowIndex() : NumStates(0)
        {
        }
    };

    struct WindowStateIndex
    {
        // Members
        int InitInc = 0;      // Flag indicating initialization needed on Incoming basis
        int IncBasisIndx = 0; // Index of basis list entry for Incoming basis
        int CopyIncState = 0; // Pointer to state from which geometry can be copied (Incident)
        int InitTrn = 0;      // Flag indicating initialization needed on Outgoing basis
        int TrnBasisIndx = 0; // Index of basis list entry for Outgoing basis
        int CopyTrnState = 0; // Pointer to state from which geometry can be copied (Outgoing)
        int Konst = 0;        // Index of state descript in Construct array
        // INTEGER  ::  ThermConst  !Index of state thermal description in Construct array
    };

    // Functions

    // void clear_state();

    void InitBSDFWindows(EnergyPlusData &state);

    void AllocateCFSStateHourlyData(EnergyPlusData &state,
                                    int const iSurf, // Surface number
                                    int const iState // Complex fenestration state number
    );

    void ExpandComplexState(EnergyPlusData &state,
                            int const iSurf, // Surface number
                            int const iConst // Construction number
    );

    void CheckCFSStates(EnergyPlusData &state, int const iSurf); // Surface number

    void InitComplexWindows(EnergyPlusData &state);

    void UpdateComplexWindows(EnergyPlusData &state);

    void CFSShadeAndBeamInitialization(EnergyPlusData &state,
                                       int const iSurf, // Window surface number
                                       int const iState // Window state number
    );

    void CalculateWindowBeamProperties(EnergyPlusData &state,
                                       int const ISurf,                   // Window surface number
                                       int const IState,                  // Window state number
                                       BSDFWindowGeomDescr const &Window, // Window Geometry
                                       BSDFGeomDescr const &Geom,         // State Geometry
                                       BSDFStateDescr &State,             // State Description
                                       int const Hour,                    // Hour number
                                       int const TS                       // Timestep number
    );

    void CalcStaticProperties(EnergyPlusData &state);

    void CalculateBasisLength(EnergyPlusData &state,
                              BSDFWindowInputStruct const &Input, // BSDF data input struct for this construction
                              int const IConst,                   // Construction number of input
                              int &NBasis                         // Calculated Basis length
    );

    void ConstructBasis(EnergyPlusData &state,
                        int const IConst, // Index for accessing Construct array
                        BasisStruct &Basis);

    void FillBasisElement(EnergyPlusData &state,
                          Real64 const Theta, // Central polar angle of element
                          Real64 const Phi,   // Central azimuthal angle of element
                          int const Elem,     // Index number of element in basis
                          BasisElemDescr &BasisElem,
                          Real64 const LowerTheta,              // Lower edge of element (polar angle)
                          Real64 const UpperTheta,              // Upper edge of element (polar angle)
                          Real64 const DPhi,                    // Width of element (azimuthal angle)
                          DataBSDFWindow::Basis const InputType // Basis type
    );

    void SetupComplexWindowStateGeometry(EnergyPlusData &state,
                                         int const ISurf,             // Surface number of the complex fenestration
                                         int const IState,            // State number of the complex fenestration state
                                         int const IConst,            // Pointer to construction for this state
                                         BSDFWindowGeomDescr &Window, // Window Geometry
                                         BSDFGeomDescr &Geom,         // State Geometry
                                         BSDFStateDescr &State        // State Description
    );

    void CalcWindowStaticProperties(EnergyPlusData &state,
                                    int const ISurf,             // Surface number of the complex fenestration
                                    int const IState,            // State number of the complex fenestration state
                                    BSDFWindowGeomDescr &Window, // Window Geometry
                                    BSDFGeomDescr &Geom,         // State Geometry
                                    BSDFStateDescr &State        // State Description
    );

    Real64 SkyWeight(Vector const &DirVec); // Direction of the element to be weighted

    Real64 SkyGndWeight(Vector const &PosVec); // x,y,z(=0) of ground intersection pt

    BSDFDaylghtPosition DaylghtAltAndAzimuth(Vector const &UnitVect); // vector which needs to be converted

    Vector WorldVectFromW6(EnergyPlusData &state,
                           Real64 const Theta,                  // Polar angle in W6 Coords
                           Real64 const Phi,                    // Azimuthal angle in W6 Coords
                           const RayIdentificationType RadType, // Type of radiation: Front_Incident, etc.
                           Real64 const Gamma,                  // Surface tilt angle, radians, world coordinate system
                           Real64 const Alpha                   // Surface azimuth, radians, world coordinate system
    );

    int FindInBasis(EnergyPlusData &state,
                    Vector const &RayToFind,             // Ray vector direction in world CS
                    const RayIdentificationType RadType, // Type of radiation: Front_Incident, etc.
                    int const ISurf,                     // Window Surface number
                    int const IState,                    // Complex Fenestration state number
                    BasisStruct const &Basis,            // Complex Fenestration basis root
                    Real64 &Theta,                       // Theta value for ray
                    Real64 &Phi                          // Phi value for ray
    );

    void W6CoordsFromWorldVect(EnergyPlusData &state,
                               Vector const &RayVect,               // Ray vector direction in world CS
                               const RayIdentificationType RadType, // Type of radiation: Front_Incident, etc.
                               Real64 const Gamma,                  // Surface tilt angle, world coordinate system
                               Real64 const Alpha,                  // Surface azimuth, world coordinate system
                               Real64 &Theta,                       // Polar angle in W6 Coords
                               Real64 &Phi                          // Azimuthal angle in W6 Coords
    );

    void CalcComplexWindowThermal(EnergyPlusData &state,
                                  int const SurfNum,          // Surface number
                                  int &ConstrNum,             // Construction number
                                  Real64 const HextConvCoeff, // Outside air film conductance coefficient
                                  Real64 &SurfInsideTemp,     // Inside window surface temperature
                                  Real64 &SurfOutsideTemp,    // Outside surface temperature (C)
                                  Real64 &SurfOutsideEmiss,
                                  DataBSDFWindow::Condition const CalcCondition // Calculation condition (summer, winter or no condition)
    );

    // This function check if gas with molecular weight has already been feed into coefficients and
    // feed arrays

    void CheckGasCoefs(Real64 const currentWeight, int &indexNumber, Array1D<Real64> &wght, bool &feedData);

    int SearchAscTable(Real64 const y,            // Value to be found in the table
                       int const n,               // Number of values in the table
                       Array1S<Real64> const ytab // Table of values, monotonic, ascending order
    );

    //=================================================================================================

} // namespace WindowComplexManager

struct WindowComplexManagerData : BaseGlobalStruct
{

    Real64 const sigma; // Stefan-Boltzmann constant
    Real64 const PressureDefault;

    int const Calculate_Geometry;
    int const Copy_Geometry;

    int const TmpLen; // Length increment of temporary arrays

    int NumComplexWind; // Total number of complex windows

    Array1D<DataBSDFWindow::BasisStruct> BasisList;
    EPVector<WindowComplexManager::WindowIndex> WindowList;
    Array2D<WindowComplexManager::WindowStateIndex> WindowStateList;

    bool InitComplexWindowsOnce = true; // Flag for insuring things happen once
    bool InitBSDFWindowsOnce = true;
    int NumBasis = 0; // Number of unique bases (No. in BasisList)
    int MatrixNo = 0; // Index of Basis matrix

    Array1D<Real64> gap = Array1D<Real64>(TARCOGParams::maxlay, 0.0);   // Vector of gap widths [m] {maxlay}
    Array1D<Real64> thick = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Vector of glass thicknesses [m] {maxlay}
    Array1D<Real64> scon = Array1D<Real64>(TARCOGParams::maxlay, 0.0);  // Vector of conductivities of each glazing layer  [W/m.K] {maxlay}
    Array1D<Real64> tir =
        Array1D<Real64>(TARCOGParams::maxlay * 2, 0.0); // Vector of IR transmittances of each layer {2*maxlay - 2 surfaces per layer}
    Array1D<Real64> emis =
        Array1D<Real64>(TARCOGParams::maxlay * 2, 0.0);            // Vector of IR emittances of each surface {2*maxlay - 2 surfaces per layer}
    Array1D_int SupportPlr = Array1D_int(TARCOGParams::maxlay, 0); // Shows whether or not gap have support pillar
    // 0 - does not have support pillar
    // 1 - have support pillar
    Array1D<Real64> PillarSpacing = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Pillar spacing for each gap (used in case there is support pillar)
    Array1D<Real64> PillarRadius = Array1D<Real64>(TARCOGParams::maxlay, 0.0);  // Pillar radius for each gap (used in case there is support pillar)
    Array1D<Real64> asol = Array1D<Real64>(TARCOGParams::maxlay, 0.0);          // Vector of Absorbed solar energy fractions for each layer {maxlay}
    Array1D<Real64> presure = Array1D<Real64>(TARCOGParams::maxlay + 1, 0.0);   // Vector of gas pressures in gaps [N/m^2] {maxlay+1}
    Array1D<Real64> GapDefMax = Array1D<Real64>(TARCOGParams::maxlay - 1, 0.0); // Vector of gap widths in deflected state.  It will be used as input
    // if CalcDeflection = 2. In case CalcDeflection = 1 it will return recalculated
    // gap widths. [m]
    Array1D<Real64> YoungsMod = Array1D<Real64>(TARCOGParams::maxlay, 0.0);   // Vector of Young's modulus. [m]
    Array1D<Real64> PoissonsRat = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Vector of Poisson's Ratios. [m]
    Array1D<Real64> LayerDef = Array1D<Real64>(TARCOGParams::maxlay, 0.0);    // Vector of layers deflection. [m]

    Array2D_int iprop = Array2D_int(TARCOGGassesParams::maxgas, TARCOGParams::maxlay + 1, 1); // Matrix of gas codes - see above {maxgap x maxgas}
    Array2D<Real64> frct =
        Array2D<Real64>(TARCOGGassesParams::maxgas, TARCOGParams::maxlay + 1, 0.0); // Matrix of mass percentages in gap mixtures  {maxgap x maxgas}
    Array2D<Real64> gcon = Array2D<Real64>(3, TARCOGGassesParams::maxgas, 0.0);     // Matrix of constants for gas conductivity calc
    //     (A, B, C for max of 10 gasses) {maxgas x 3}
    Array2D<Real64> gvis = Array2D<Real64>(3, TARCOGGassesParams::maxgas, 0.0); // Matrix of constants for gas dynamic viscosity calc
    //     (A, B, C for max of 10 gasses) {maxgas x 3}
    Array2D<Real64> gcp = Array2D<Real64>(3, TARCOGGassesParams::maxgas, 0.0); // Matrix of constants for gas specific heat calc at constant pressure
    //     (A, B, C for max of 10 gasses) {maxgas x 3}
    Array1D<Real64> wght = Array1D<Real64>(TARCOGGassesParams::maxgas, 0.0); // Vector of Molecular weights for gasses {maxgas}
    Array1D<Real64> gama = Array1D<Real64>(TARCOGGassesParams::maxgas, 0.0); // Vector of specific heat ration for low pressure calc {maxgas}
    Array1D_int nmix = Array1D_int(TARCOGParams::maxlay + 1, 0);             // Vector of number of gasses in gas mixture of each gap {maxlay+1}
    Array1D_int ibc = Array1D_int(2, 0);                                     // Vector of boundary condition flags (ibc(1) - outdoor, ibc(2) - indoor)
    //             0 - h to be calculated;
    //             1 - combined film coefficient (h) prescribed;
    //             2 - convective film coefficient (hc) prescribed.
    //           Also used in old algorithms for calculating h, accessible through
    //           negative values for flags:
    //             -1  - old SPC142 correlation
    //             -2  - Klems-Yazdanian correlation (applicable to outdoor only)
    //             -3  - Kimura correlation (applicable to outdoor only)
    Array1D<Real64> Atop = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Vector with areas of top openings - between SD layers and top of
    //               glazing cavity, for each layer [m^2] {maxlay} *
    Array1D<Real64> Abot = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Vector with areas of bottom openings - between SD layers
    //               and bottom of glazing cavity [m^2] {maxlay}
    Array1D<Real64> Al = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Vector with areas of left-hand side openings - between SD layers
    //               and left end of glazing cavity [m^2] {maxlay}
    Array1D<Real64> Ar = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Vector of areas of right-hand side openings - between SD layers
    //               and right end of glazing cavity [m^2] {maxlay}
    Array1D<Real64> Ah = Array1D<Real64>(TARCOGParams::maxlay, 0.0);          // Vector of total areas of holes for each SD [m^2] {maxlay}
    Array1D<Real64> SlatThick = Array1D<Real64>(TARCOGParams::maxlay, 0.0);   // Thickness of the slat material [m] {maxlay} **
    Array1D<Real64> SlatWidth = Array1D<Real64>(TARCOGParams::maxlay, 0.0);   // Slat width [m] {maxlay}
    Array1D<Real64> SlatAngle = Array1D<Real64>(TARCOGParams::maxlay, 0.0);   // Slat tilt angle [deg] {maxlay}
    Array1D<Real64> SlatCond = Array1D<Real64>(TARCOGParams::maxlay, 0.0);    // Conductivity of the slat material [W/m.K] {maxlay}
    Array1D<Real64> SlatSpacing = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Distance between slats [m] {maxlay}
    Array1D<Real64> SlatCurve = Array1D<Real64>(TARCOGParams::maxlay, 0.0);   // Curvature radius of the slat [m] {maxlay}
    Array1D<Real64> vvent = Array1D<Real64>(TARCOGParams::maxlay + 1, 0.0);   // Vector of velocities for forced ventilation, for each gap, and for
    //               outdoor and indoor environment [m/s] {maxlay+1} ***
    Array1D<Real64> tvent =
        Array1D<Real64>(TARCOGParams::maxlay + 1, 0.0); // Vector of temperatures of ventilation gas for forced ventilation, for each
    //  gap, and for outdoor and indoor environment [K] {maxlay+1}
    Array1D<TARCOGParams::TARCOGLayerType> LayerType =
        Array1D<TARCOGParams::TARCOGLayerType>(TARCOGParams::maxlay, TARCOGParams::TARCOGLayerType::SPECULAR); // Glazing layer type flag {maxlay}:
    //                 0 - Specular layer,
    //                 1 - Venetian blind (SD)
    //                 2 - Woven shade (SD) (not implemented)
    //                 3 - Diffuse shade (not implemented)
    Array1D_int nslice = Array1D_int(TARCOGParams::maxlay, 0); // Vector of numbers of slices in a laminated glazing layers
    //   (0 - monolithic layer) {maxlay}
    Array1D<Real64> LaminateA = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Left-hand side array for creating slice equations {maxlay}
    Array1D<Real64> LaminateB = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Right-hand side array for creating slice equations {maxlay}
    Array1D<Real64> sumsol = Array1D<Real64>(TARCOGParams::maxlay, 0.0);    // Array of absorbed solar energy fractions for each laminated
    //               glazing layer [W/m^2] {maxlay}
    Array1D<Real64> theta = Array1D<Real64>(TARCOGParams::maxlay * 2, 0.0); // Vector of average temperatures of glazing surfaces [K] {2*maxlay}
    Array1D<Real64> q = Array1D<Real64>(TARCOGParams::maxlay * 2 + 1, 0.0); // Vector of various heat fluxes [W/m^2] {2*maxlay+1},
    //    depending on element index:
    //    1  = qout (heat flux from outer-most glazing surface to outdoor space)
    //   2*i = qpane(i) (heat flux through i-th glazing layer)
    // 2*i-1 = qgap(i) (heat flux from i-th glazing cavity to indoor-faced
    //          surface of the adjacent glazing layer)
    // 2*nlayer+1 = qin (heat flux from indoor space to inner-most glazing
    //              surface)
    Array1D<Real64> qprim = Array1D<Real64>(TARCOGParams::maxlay1, 0.0); // Vector of heat fluxes from the outdoor-faced surfaces of glazing layers
    //    towards the adjacent glazing cavity [W/m2]
    Array1D<Real64> qv = Array1D<Real64>(TARCOGParams::maxlay1, 0.0);    // Vector of heat fluxes to each gap by ventillation [W/m^2]
    Array1D<Real64> hcgap = Array1D<Real64>(TARCOGParams::maxlay1, 0.0); // Convective part of gap effective conductivity {maxlay}
    Array1D<Real64> hrgap = Array1D<Real64>(TARCOGParams::maxlay1, 0.0); // Radiative part of gap effective conductivity (including in and out)
    Array1D<Real64> hg = Array1D<Real64>(TARCOGParams::maxlay, 0.0);     // Gas conductance of the glazing cavity
    //         [W/m^2.K] - EN673 and ISO 10292 procedure
    Array1D<Real64> hr = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Radiation conductance of the glazing cavity
    //         [W/m^2.K] - EN673 and ISO 10292 procedure
    Array1D<Real64> hs = Array1D<Real64>(TARCOGParams::maxlay, 0.0); // Thermal conductance of the glazing cavity
    //         [W/m^2.K] - EN673 and ISO 10292 procedure
    Array1D<Real64> Ra = Array1D<Real64>(TARCOGParams::maxlay + 1, 0.0);               // Vector of Rayleigh numbers, for each gap {maxlay}
    Array1D<Real64> Nu = Array1D<Real64>(TARCOGParams::maxlay + 1, 0.0);               // Vector of Nusselt numbers, for each gap {maxlay}
    Array1D<Real64> Keff = Array1D<Real64>(TARCOGParams::maxlay, 0.0);                 // Vector of keff values for gaps [W/m.K] {maxlay}
    Array1D<Real64> ShadeGapKeffConv = Array1D<Real64>(TARCOGParams::maxlay - 1, 0.0); // Vector of convective keff values for areas above/below
    // SD layers [W/m.K] {maxlay-1}
    Array1D<Real64> deltaTemp = Array1D<Real64>(100, 0.0);
    Array1D_int iMinDT = Array1D_int(1, 0);
    Array1D_int IDConst = Array1D_int(100, 0);

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumComplexWind = 0;
        this->BasisList.deallocate();
        this->WindowList.deallocate();
        this->WindowStateList.deallocate();
        this->InitComplexWindowsOnce = true;
        this->InitBSDFWindowsOnce = true;
        this->NumBasis = 0;
        this->MatrixNo = 0;
        this->LayerType = Array1D<TARCOGParams::TARCOGLayerType>(TARCOGParams::maxlay, TARCOGParams::TARCOGLayerType::SPECULAR);
    }

    // Default Constructor
    WindowComplexManagerData()
        : sigma(5.6697e-8), PressureDefault(101325.0), Calculate_Geometry(1), Copy_Geometry(2), TmpLen(20), NumComplexWind(0), NumBasis(0),
          MatrixNo(0)
    {
    }
};

} // namespace EnergyPlus

#endif
