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

#ifndef WindowManagerExteriorThermal_hh_INCLUDED
#define WindowManagerExteriorThermal_hh_INCLUDED

namespace EnergyPlus {

namespace DataSurfaces {
    struct SurfaceData;
}

namespace DataHeatBalance {
    struct MaterialBase;
    struct MaterialChild;
} // namespace DataHeatBalance

} // namespace EnergyPlus

namespace Tarcog {
namespace ISO15099 {
    class CBaseIGULayer;
    class CEnvironment;
    class CIGU;
    class CSingleSystem;
    class CSystem;
    class IIGUSystem;
} // namespace ISO15099
} // namespace Tarcog

namespace Gases {

class CGas;
}

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Window {

    enum class ShadePosition
    {
        Invalid = -1,
        NoShade,
        Interior,
        Exterior,
        Between,
        Num
    };

    // Routine that calculates heat transfer balance by using Windows-CalcEngine routines
    void CalcWindowHeatBalanceExternalRoutines(EnergyPlusData &state,
                                               int const SurfNum,          // Surface number
                                               Real64 const HextConvCoeff, // Outside air film conductance coefficient
                                               Real64 &SurfInsideTemp,     // Inside window surface temperature
                                               Real64 &SurfOutsideTemp     // Outside surface temperature (C)
    );

    Real64 GetIGUUValueForNFRCReport(EnergyPlusData &state, int surfNum, int constrNum, Real64 windowWidth, Real64 windowHeight);
    Real64 GetSHGCValueForNFRCReporting(EnergyPlusData &state, int surfNum, int constrNum, Real64 windowWidth, Real64 windowHeight);

    void GetWindowAssemblyNfrcForReport(EnergyPlusData &state,
                                        int const surfNum,
                                        int constrNum,
                                        Real64 windowWidth,
                                        Real64 windowHeight,
                                        EnergyPlus::DataSurfaces::NfrcVisionType vision,
                                        Real64 &uvalue,
                                        Real64 &shgc,
                                        Real64 &vt);

    // Class that is used to create layers for Windows-CalcEngine
    class CWCEHeatTransferFactory
    {
    public:
        CWCEHeatTransferFactory(EnergyPlusData &state, EnergyPlus::DataSurfaces::SurfaceData const &surface, int t_SurfNum, int t_ConstrNum);

        std::shared_ptr<Tarcog::ISO15099::CSingleSystem> getTarcogSystem(EnergyPlusData &state, Real64 const t_HextConvCoeff);

        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> getIGULayer(EnergyPlusData &state, int const t_Index);
        std::shared_ptr<Tarcog::ISO15099::CEnvironment> getIndoor(EnergyPlusData &state) const;
        std::shared_ptr<Tarcog::ISO15099::CEnvironment> getOutdoor(EnergyPlusData &state, Real64 const t_Hext) const;
        Tarcog::ISO15099::CIGU getIGU() const;
        Tarcog::ISO15099::CIGU getIGU(double width, double height, double tilt);

        static int getActiveConstructionNumber(EnergyPlusData &state, EnergyPlus::DataSurfaces::SurfaceData const &surface, int t_SurfNum);

        // for assembly windoww reporting
        std::shared_ptr<Tarcog::ISO15099::IIGUSystem> getTarcogSystemForReporting(
            EnergyPlusData &state, bool const useSummerConditions, const double width, const double height, const double tilt);

        // This special case of interior shade is necessary only because of strange calculation of heat flow on interior side
        // It probably needs to be removed since calculation is no different from any other case. It is left over from
        // old EnergyPlus code and it needs to be checked.
        bool isInteriorShade() const;
        double overallUfactorFromFilmsAndCond(double conductance, double insideFilm, double outsideFilm);

        // methods specifically for helping in NFRC assembly calculations
        std::shared_ptr<Tarcog::ISO15099::CEnvironment> getOutdoorNfrc(bool const useSummerConditions);
        std::shared_ptr<Tarcog::ISO15099::CEnvironment> getIndoorNfrc(bool const useSummerConditions);

        // Method to determine current construction shade type
        static EnergyPlus::DataSurfaces::WinShadingType getShadeType(EnergyPlusData &state, int ConstrNum);

    private:
        DataSurfaces::SurfaceData m_Surface;
        DataSurfaces::SurfaceWindowCalc m_Window;
        ShadePosition m_ShadePosition;
        int m_SurfNum;
        size_t m_SolidLayerIndex;
        int m_ConstructionNumber;
        int m_TotLay;

        // Next two booleans are to keep track of order for shading devices. Some variables
        // are kept in separate arrays for specular glazings and shading devices and it is
        // important to keep index numbering in order to extract correct results
        bool m_InteriorBSDFShade;
        bool m_ExteriorShade;

        int getNumOfLayers(EnergyPlusData &state) const;

        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer>
        getSolidLayer(EnergyPlusData &state, Material::MaterialBase const *material, int const t_Index);

        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> getGapLayer(Material::MaterialBase const *material) const;

        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> getShadeToGlassLayer(EnergyPlusData &state, int const t_Index) const;

        std::shared_ptr<Tarcog::ISO15099::CBaseIGULayer> getComplexGapLayer(EnergyPlusData &state, Material::MaterialBase const *material) const;

        Gases::CGas getGas(Material::MaterialBase const *material) const;
        static Gases::CGas getAir();
        Material::MaterialBase *getLayerMaterial(EnergyPlusData &state, int const t_Index) const;
    };
} // namespace Window

} // namespace EnergyPlus

#endif
