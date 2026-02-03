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

#ifndef SurfaceGroundHeatExchanger_hh_INCLUDED
#define SurfaceGroundHeatExchanger_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SurfaceGroundHeatExchanger {

    struct SurfaceGroundHeatExchangerData : PlantComponent
    {
        ~SurfaceGroundHeatExchangerData() = default;

        std::string Name;             // name of surface GHE
        std::string ConstructionName; // name of the associated construction
        std::string InletNode;        // surface GHE inlet fluid node
        std::string OutletNode;       // surface GHE outlet fluid node
        Real64 DesignMassFlowRate;
        Real64 TubeDiameter;                     // hydronic tube inside diameter
        Real64 TubeSpacing;                      // tube spacing
        Real64 SurfaceLength;                    // active length of surface GHE
        Real64 SurfaceWidth;                     // active width of surface GHE
        Real64 TopThermAbs;                      // Thermal absortivity of top layer
        Real64 TopSolarAbs;                      // solar absortivity of top layer
        Real64 BtmThermAbs;                      // Thermal absortivity of bottom layer
        int LowerSurfCond;                       // Type of lower surf. boundary condition
        int TubeCircuits;                        // number of circuits in total
        int ConstructionNum;                     // construction index number
        int InletNodeNum;                        // inlet node number
        int OutletNodeNum;                       // oulet node number
        Material::SurfaceRoughness TopRoughness; // roughness of top layer
        Material::SurfaceRoughness BtmRoughness; // roughness of bottom layer
        int FrozenErrIndex1;                     // recurring error index
        int FrozenErrIndex2;                     // recurring error index
        int ConvErrIndex1;                       // recurring error index
        int ConvErrIndex2;                       // recurring error index
        int ConvErrIndex3;                       // recurring error index
                                                 // loop topology variables
        PlantLocation plantLoc;

        // QTF Constants
        Real64 TsrcConstCoef;
        Real64 TsrcVarCoef;
        Real64 QbtmConstCoef;
        Real64 QbtmVarCoef;
        Real64 QtopConstCoef;
        Real64 QtopVarCoef;
        // conventional CTF terms
        int NumCTFTerms; // number of terms for surface
        // could be allocated rather than hard dimensioning.
        std::array<Real64, Construction::MaxCTFTerms> CTFin{};    // surf flux in ctf - X
        std::array<Real64, Construction::MaxCTFTerms> CTFout{};   // surf flux in ctf - Z
        std::array<Real64, Construction::MaxCTFTerms> CTFcross{}; // surf flux in ctf - Y
        std::array<Real64, Construction::MaxCTFTerms> CTFflux{};  // surf flux in ctf - F
        // QTF coefficients
        std::array<Real64, Construction::MaxCTFTerms> CTFSourceIn{};   // surf flux in ctf - Wi
        std::array<Real64, Construction::MaxCTFTerms> CTFSourceOut{};  // surf flux out ctf - Wo
        std::array<Real64, Construction::MaxCTFTerms> CTFTSourceOut{}; // surf flux in qtf - x
        std::array<Real64, Construction::MaxCTFTerms> CTFTSourceIn{};  // surf flux in qtf - y
        std::array<Real64, Construction::MaxCTFTerms> CTFTSourceQ{};   // surf flux in qtf - f
        // History data
        std::array<Real64, Construction::MaxCTFTerms> TbtmHistory{};
        std::array<Real64, Construction::MaxCTFTerms> TtopHistory{};
        std::array<Real64, Construction::MaxCTFTerms> TsrcHistory{};
        std::array<Real64, Construction::MaxCTFTerms> QbtmHistory{};
        std::array<Real64, Construction::MaxCTFTerms> QtopHistory{};
        std::array<Real64, Construction::MaxCTFTerms> QsrcHistory{};
        Real64 QSrc;
        Real64 QSrcAvg;
        Real64 LastQSrc;
        Real64 LastSysTimeElapsed;
        Real64 LastTimeStepSys;

        // Report data
        Real64 InletTemp;            // water inlet temperature
        Real64 OutletTemp;           // water outlet temperature
        Real64 MassFlowRate;         // water mass flow rate
        Real64 TopSurfaceTemp;       // Top surface temperature
        Real64 BtmSurfaceTemp;       // Bottom  surface temperature
        Real64 TopSurfaceFlux;       // Top  surface heat flux
        Real64 BtmSurfaceFlux;       // Bottom  surface heat flux
        Real64 HeatTransferRate;     // total fluid heat transfer rate, Watts
        Real64 SurfHeatTransferRate; // total surface heat transfer rate, Watts
        Real64 Energy;               // cumulative energy, Joules
        Real64 SurfEnergy;           // cumulative energy, Joules
        Real64 SourceTemp;           // Source temperature

        bool MyFlag;
        bool InitQTF;
        bool MyEnvrnFlag;
        Real64 SurfaceArea; // surface GHE surface area
        bool firstTimeThrough;

        // Default Constructor
        SurfaceGroundHeatExchangerData()
            : DesignMassFlowRate(0.0), TubeDiameter(0.0), TubeSpacing(0.0), SurfaceLength(0.0), SurfaceWidth(0.0), TopThermAbs(0.0), TopSolarAbs(0.0),
              BtmThermAbs(0.0), LowerSurfCond(0), TubeCircuits(0), ConstructionNum(0), InletNodeNum(0), OutletNodeNum(0),
              TopRoughness(Material::SurfaceRoughness::Invalid), BtmRoughness(Material::SurfaceRoughness::Invalid), FrozenErrIndex1(0),
              FrozenErrIndex2(0), ConvErrIndex1(0), ConvErrIndex2(0), ConvErrIndex3(0), plantLoc{}, TsrcConstCoef(0.0), TsrcVarCoef(0.0),
              QbtmConstCoef(0.0), QbtmVarCoef(0.0), QtopConstCoef(0.0), QtopVarCoef(0.0), NumCTFTerms(0), QSrc(0.0), QSrcAvg(0.0), LastQSrc(0.0),
              LastSysTimeElapsed(0.0), LastTimeStepSys(0.0),

              InletTemp(0.0), OutletTemp(0.0), MassFlowRate(0.0), TopSurfaceTemp(0.0), BtmSurfaceTemp(0.0), TopSurfaceFlux(0.0), BtmSurfaceFlux(0.0),
              HeatTransferRate(0.0), SurfHeatTransferRate(0.0), Energy(0.0), SurfEnergy(0.0), SourceTemp(0.0),

              MyFlag(true), InitQTF(true), MyEnvrnFlag(true), SurfaceArea(0.0), firstTimeThrough(true)
        {
        }

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        static PlantComponent *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string objectName);

        void InitSurfaceGroundHeatExchanger(EnergyPlusData &state);

        //==============================================================================

        void CalcSurfaceGroundHeatExchanger(EnergyPlusData &state, bool FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
        );

        //==============================================================================

        void CalcBottomFluxCoefficents(Real64 Tbottom, // current bottom (lower) surface temperature
                                       Real64 Ttop     // current top (upper) surface temperature
        );

        //==============================================================================

        void CalcTopFluxCoefficents(Real64 Tbottom, // current bottom (lower) surface temperature
                                    Real64 Ttop     // current top (upper) surface temperature
        );

        //==============================================================================

        void CalcSourceTempCoefficents(Real64 Tbottom, // current bottom (lower) surface temperature
                                       Real64 Ttop     // current top (upper) surface temperature
        );

        //==============================================================================

        Real64 CalcSourceFlux(EnergyPlusData &state); // component number

        //==============================================================================

        void UpdateHistories(Real64 TopFlux,    // current top (top) surface flux
                             Real64 BottomFlux, // current bottom (bottom) surface flux
                             Real64 SourceFlux, // current source surface flux
                             Real64 SourceTemp  // current source temperature
        );

        //==============================================================================

        Real64 CalcHXEffectTerm(EnergyPlusData &state,
                                Real64 Temperature,  // Temperature of water entering the surface, in C
                                Real64 WaterMassFlow // Mass flow rate, in kg/s
        );

        //==============================================================================

        void CalcTopSurfTemp(Real64 FluxTop,             // top surface flux
                             Real64 &TempTop,            // top surface temperature
                             Real64 ThisDryBulb,         // dry bulb temperature
                             Real64 ThisWetBulb,         // wet bulb temperature
                             Real64 ThisSkyTemp,         // sky temperature
                             Real64 ThisBeamSolarRad,    // beam solar radiation
                             Real64 ThisDifSolarRad,     // diffuse solar radiation
                             Real64 ThisSolarDirCosVert, // vertical component of solar normal
                             Real64 ThisWindSpeed,       // wind speed
                             bool ThisIsRain,            // rain flag
                             bool ThisIsSnow             // snow flag
        );

        //==============================================================================

        void CalcBottomSurfTemp(Real64 FluxBtm,       // bottom surface flux
                                Real64 &TempBtm,      // bottom surface temperature
                                Real64 ThisDryBulb,   // dry bulb temperature
                                Real64 ThisWindSpeed, // wind speed
                                Real64 ThisGroundTemp // ground temperature
        );

        //==============================================================================

        void UpdateSurfaceGroundHeatExchngr(EnergyPlusData &state); // Index for the surface

        //==============================================================================

        void ReportSurfaceGroundHeatExchngr(EnergyPlusData &state); // Index for the surface under consideration

        //==============================================================================

        void oneTimeInit(EnergyPlusData &state) override; // Empty function for pure virtual oneTimeInitFlag function

        void oneTimeInit_new(EnergyPlusData &state) override; // Empty function for pure virtual oneTimeInitFlag function
    };

    void GetSurfaceGroundHeatExchanger(EnergyPlusData &state);

    inline std::array<Real64, Construction::MaxCTFTerms>
    eoshiftArray(std::array<Real64, Construction::MaxCTFTerms> const &a, int const shift, Real64 const initialValue)
    {
        std::array<Real64, Construction::MaxCTFTerms> o = {initialValue};
        int const b(0 + std::max(shift, 0)), e(a.size() - 1 + std::min(shift, 0));
        for (int i = b, j = std::max(0 - shift, 0); i <= e; ++i, ++j) {
            o[j] = a[i];
        }
        return o;
    }
    //==============================================================================

} // namespace SurfaceGroundHeatExchanger

struct SurfaceGroundHeatExchangersData : BaseGlobalStruct
{

    // utility variables initialized once
    bool NoSurfaceGroundTempObjWarning = true; // This will cause a warning to be issued if no "surface" ground
    Real64 FlowRate = 0.0;                     // water mass flow rate
    Real64 TopSurfTemp = 0.0;                  // Top  surface temperature
    Real64 BtmSurfTemp = 0.0;                  // Bottom  surface temperature
    Real64 TopSurfFlux = 0.0;                  // Top  surface heat flux
    Real64 BtmSurfFlux = 0.0;                  // Bottom  surface heat flux
    Real64 SourceFlux = 0.0;                   // total heat transfer rate, Watts
    Array1D_bool CheckEquipName;

    // weather data records updated every zone time step
    Real64 PastBeamSolarRad = 0.0;    // Previous beam normal solar irradiance
    Real64 PastSolarDirCosVert = 0.0; // Previous vertical component of solar normal
    Real64 PastDifSolarRad = 0.0;     // Previous sky diffuse solar horizontal irradiance
    Real64 PastGroundTemp = 0.0;      // Previous ground temperature
    bool PastIsRain = false;          // Previous Surfaces are wet for this time interval
    bool PastIsSnow = false;          // Previous Snow on the ground for this time interval
    Real64 PastOutDryBulbTemp = 0.0;  // Previous outdoor air dry bulb temperature
    Real64 PastOutWetBulbTemp = 0.0;  // Previous outdoor air wet bulb temperature
    Real64 PastSkyTemp = 0.0;         // Previous sky temperature
    Real64 PastWindSpeed = 0.0;       // Previous outdoor air wind speed

    bool GetInputFlag = true;

    // time keeping variables used for keeping track of average flux over each time step
    Array1D<Real64> QRadSysSrcAvg;      // Average source over the time step
    Array1D<Real64> LastSysTimeElapsed; // record of system time
    Array1D<Real64> LastTimeStepSys;    // previous time step size
    bool InitializeTempTop = false;

    Array1D<SurfaceGroundHeatExchanger::SurfaceGroundHeatExchangerData> SurfaceGHE;
    Real64 FluxTop; // top surface flux
    Real64 FluxBtm; // bottom surface flux
    Real64 TempBtm; // bottom surface temp
    Real64 TempTop; // top surface temp

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NoSurfaceGroundTempObjWarning = true;
        FlowRate = 0.0;
        TopSurfTemp = 0.0;
        BtmSurfTemp = 0.0;
        TopSurfFlux = 0.0;
        BtmSurfFlux = 0.0;
        SourceFlux = 0.0;
        CheckEquipName.clear();
        PastBeamSolarRad = 0.0;
        PastSolarDirCosVert = 0.0;
        PastDifSolarRad = 0.0;
        PastGroundTemp = 0.0;
        PastIsRain = false;
        PastIsSnow = false;
        PastOutDryBulbTemp = 0.0;
        PastOutWetBulbTemp = 0.0;
        PastSkyTemp = 0.0;
        PastWindSpeed = 0.0;
        GetInputFlag = true;
        QRadSysSrcAvg.clear();
        LastSysTimeElapsed.clear();
        LastTimeStepSys.clear();
        InitializeTempTop = false;
        SurfaceGHE.clear();
    }

    // Default Constructor
    SurfaceGroundHeatExchangersData() = default;
};
} // namespace EnergyPlus

#endif
