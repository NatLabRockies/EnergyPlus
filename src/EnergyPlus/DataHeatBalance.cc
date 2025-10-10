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

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataHeatBalance {

// MODULE INFORMATION:
//       AUTHOR         Rick Strand
//       DATE WRITTEN   August 1997 (rewritten)
//       MODIFIED       Aug-Oct 1997 RKS (added derived types)
//       MODIFIED       Feb 1999 (FW) Added visible radiation parameters,
//                      WindowShadingControl type and SurfaceWindowCalc type
//                      Sep 1999 (FW) Added/modified Window4 gas variables
//                      Jul 2003 (CC) Added reference temperature variable for air models
//                      Aug 2003 (FW) Added FractionReturnAirPlenTempCoeff1 and
//                      FractionReturnAirPlenTempCoeff2 to Type LightsData
//                      Nov 2003 (FW) Add FullExteriorWithRefl and FullInteriorExteriorWithRefl
//                       as SolarDistribution values
//                      Dec 2003 (PGE) Added Zone List and Zone Group; added SNLoad variables
//                      August 2006 (COP) Added variable k coefficient and PCM enthalpy.
//                      Dec 2006 (DJS-PSU) Added ecoroof material
//                      Dec 2008 TH added new properties to MaterialProperties and
//                              ConstructionData for thermochromic windows
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This module should contain the information that is needed to pass
// from the Heat Balance Module and all of the Zone Initializations
// such as ConductionTransferFunction, GlassCalculation,
// SolarShading, etc. Modules.

// Using/Aliasing
using namespace DataVectorTypes;
using DataBSDFWindow::BSDFLayerAbsorpStruct;
using DataBSDFWindow::BSDFWindowInputStruct;

// Functions

Real64 SpaceData::sumHATsurf(EnergyPlusData &state)
{
    // PURPOSE OF THIS FUNCTION:
    // This function calculates the space sum of Hc*Area*Tsurf.

    Real64 sumHATsurf = 0.0;

    for (int surfNum = this->HTSurfaceFirst; surfNum <= this->HTSurfaceLast; ++surfNum) {
        Real64 Area = state.dataSurface->Surface(surfNum).Area;

        if (state.dataSurface->Surface(surfNum).Class == DataSurfaces::SurfaceClass::Window) {
            if (state.dataSurface->SurfWinDividerArea(surfNum) > 0.0) {
                if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(surfNum))) {
                    // The area is the shade or blind area = sum of the glazing area and the divider area (which is zero if no divider)
                    Area += state.dataSurface->SurfWinDividerArea(surfNum);
                } else {
                    // Window divider contribution (only for window with divider and no interior shade or blind)
                    sumHATsurf += state.dataHeatBalSurf->SurfHConvInt(surfNum) * state.dataSurface->SurfWinDividerArea(surfNum) *
                                  (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(surfNum)) * state.dataSurface->SurfWinDividerTempIn(surfNum);
                }
            }

            if (state.dataSurface->SurfWinFrameArea(surfNum) > 0.0) {
                // Window frame contribution
                sumHATsurf += state.dataHeatBalSurf->SurfHConvInt(surfNum) * state.dataSurface->SurfWinFrameArea(surfNum) *
                              (1.0 + state.dataSurface->SurfWinProjCorrFrIn(surfNum)) * state.dataSurface->SurfWinFrameTempIn(surfNum);
            }
        }

        sumHATsurf += state.dataHeatBalSurf->SurfHConvInt(surfNum) * Area * state.dataHeatBalSurf->SurfTempInTmp(surfNum);
    }

    return sumHATsurf;
}

Real64 ZoneData::sumHATsurf(EnergyPlusData &state)
{
    Real64 sumHATsurf = 0.0;
    for (int spaceNum : this->spaceIndexes) {
        sumHATsurf += state.dataHeatBal->space(spaceNum).sumHATsurf(state);
    }
    return sumHATsurf;
}
void ZoneData::SetOutBulbTempAt(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Noel Keen (LBL)/Linda Lawrie
    //       DATE WRITTEN   August 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Routine provides facility for doing bulk Set Temperature at Height.

    if (state.dataEnvrn->SiteTempGradient == 0.0) {
        OutDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
        OutWetBulbTemp = state.dataEnvrn->OutWetBulbTemp;
    } else {
        // Base temperatures at Z = 0 (C)
        Real64 const BaseDryTemp(state.dataEnvrn->OutDryBulbTemp + state.dataEnvrn->WeatherFileTempModCoeff);
        Real64 const BaseWetTemp(state.dataEnvrn->OutWetBulbTemp + state.dataEnvrn->WeatherFileTempModCoeff);

        Real64 const Z(Centroid.z); // Centroid value
        if (Z <= 0.0) {
            OutDryBulbTemp = BaseDryTemp;
            OutWetBulbTemp = BaseWetTemp;
        } else {
            OutDryBulbTemp = BaseDryTemp - state.dataEnvrn->SiteTempGradient * DataEnvironment::EarthRadius * Z / (DataEnvironment::EarthRadius + Z);
            OutWetBulbTemp = BaseWetTemp - state.dataEnvrn->SiteTempGradient * DataEnvironment::EarthRadius * Z / (DataEnvironment::EarthRadius + Z);
        }
    }
}

void ZoneData::SetWindSpeedAt(EnergyPlusData const &state, Real64 const fac)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   June 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Routine provides facility for doing bulk Set Windspeed at Height.

    if (state.dataEnvrn->SiteWindExp == 0.0) {
        WindSpeed = state.dataEnvrn->WindSpeed;
    } else {
        Real64 const Z(Centroid.z); // Centroid value
        if (Z <= 0.0) {
            WindSpeed = 0.0;
        } else {
            //  [Met] - at meterological Station, Height of measurement is usually 10m above ground
            //  LocalWindSpeed = Windspeed [Met] * (Wind Boundary LayerThickness [Met]/Height [Met])**Wind Exponent[Met] &
            //                     * (Height above ground / Site Wind Boundary Layer Thickness) ** Site Wind Exponent
            WindSpeed = fac * std::pow(Z, state.dataEnvrn->SiteWindExp);
        }
    }
}

void ZoneData::SetWindDirAt(Real64 const fac)
{
    WindDir = fac;
}

void AirReportVars::setUpOutputVars(EnergyPlusData &state, std::string_view prefix, std::string const &name)
{
    SetupOutputVariable(state,
                        format("{} Mean Air Temperature", prefix),
                        Constant::Units::C,
                        this->MeanAirTemp,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Wetbulb Globe Temperature", prefix),
                        Constant::Units::C,
                        this->WetbulbGlobeTemp,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Operative Temperature", prefix),
                        Constant::Units::C,
                        this->OperativeTemp,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Mean Air Dewpoint Temperature", prefix),
                        Constant::Units::C,
                        this->MeanAirDewPointTemp,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Mean Air Humidity Ratio", prefix),
                        Constant::Units::kgWater_kgDryAir,
                        this->MeanAirHumRat,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance Internal Convective Heat Gain Rate", prefix),
                        Constant::Units::W,
                        this->SumIntGains,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance Surface Convection Rate", prefix),
                        Constant::Units::W,
                        this->SumHADTsurfs,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance Interzone Air Transfer Rate", prefix),
                        Constant::Units::W,
                        this->SumMCpDTzones,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance Outdoor Air Transfer Rate", prefix),
                        Constant::Units::W,
                        this->SumMCpDtInfil,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance System Air Transfer Rate", prefix),
                        Constant::Units::W,
                        this->SumMCpDTsystem,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance System Convective Heat Gain Rate", prefix),
                        Constant::Units::W,
                        this->SumNonAirSystem,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Heat Balance Air Energy Storage Rate", prefix),
                        Constant::Units::W,
                        this->CzdTdt,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    if (state.dataGlobal->DisplayAdvancedReportVariables) {
        SetupOutputVariable(state,
                            format("{} Air Heat Balance Deviation Rate", prefix),
                            Constant::Units::W,
                            this->imBalance,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            name);
    }
}

void SetZoneOutBulbTempAt(EnergyPlusData &state)
{
    for (auto &zone : state.dataHeatBal->Zone) {
        zone.SetOutBulbTempAt(state);
    }
}

void CheckZoneOutBulbTempAt(EnergyPlusData &state)
{
    // Using/Aliasing
    using DataEnvironment::SetOutBulbTempAt_error;

    Real64 minBulb = 0.0;
    for (auto &zone : state.dataHeatBal->Zone) {
        minBulb = min(minBulb, zone.OutDryBulbTemp, zone.OutWetBulbTemp);
        if (minBulb < -100.0) {
            SetOutBulbTempAt_error(state, "Zone", zone.Centroid.z, zone.Name);
        }
    }
}

void SetZoneWindSpeedAt(EnergyPlusData &state)
{
    Real64 const fac(state.dataEnvrn->WindSpeed * state.dataEnvrn->WeatherFileWindModCoeff *
                     std::pow(state.dataEnvrn->SiteWindBLHeight, -state.dataEnvrn->SiteWindExp));
    for (auto &zone : state.dataHeatBal->Zone) {
        zone.SetWindSpeedAt(state, fac);
    }
}

void SetZoneWindDirAt(EnergyPlusData &state)
{
    // Using/Aliasing
    Real64 const fac(state.dataEnvrn->WindDir);
    for (auto &zone : state.dataHeatBal->Zone) {
        zone.SetWindDirAt(fac);
    }
}

void CheckAndSetConstructionProperties(EnergyPlusData &state,
                                       int const ConstrNum, // Construction number to be set/checked
                                       bool &ErrorsFound    // error flag that is set when certain errors have occurred
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 2006

    // This routine checks some properties of entered constructions; sets some properties; and sets
    // an error flag for certain error conditions.
    auto &s_mat = state.dataMaterial;

    auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
    int TotLayers = thisConstruct.TotLayers; // Number of layers in a construction
    if (TotLayers == 0) {
        return; // error condition, hopefully caught elsewhere
    }
    int InsideLayer = TotLayers; // Inside Layer of Construct; for window construct, layer no. of inside glass
    if (thisConstruct.LayerPoint(InsideLayer) <= 0) {
        return; // Error condition
    }

    //   window screen is not allowed on inside layer

    thisConstruct.DayltPropPtr = 0;
    int InsideMaterNum = thisConstruct.LayerPoint(InsideLayer); // Material "number" of the Inside layer
    if (InsideMaterNum != 0) {
        auto const *mat = s_mat->materials(InsideMaterNum);
        thisConstruct.InsideAbsorpVis = mat->AbsorpVisible;
        thisConstruct.InsideAbsorpSolar = mat->AbsorpSolar;

        // Following line applies only to opaque surfaces; it is recalculated later for windows.
        thisConstruct.ReflectVisDiffBack = 1.0 - mat->AbsorpVisible;
    }

    int OutsideMaterNum = thisConstruct.LayerPoint(1); // Material "number" of the Outside layer
    if (OutsideMaterNum != 0) {
        auto const *mat = s_mat->materials(OutsideMaterNum);
        thisConstruct.OutsideAbsorpVis = mat->AbsorpVisible;
        thisConstruct.OutsideAbsorpSolar = mat->AbsorpSolar;
    }

    thisConstruct.TotSolidLayers = 0;
    thisConstruct.TotGlassLayers = 0;
    thisConstruct.AbsDiffShade = 0.0;

    // Check if any layer is glass, gas, shade, screen or blind; if so it is considered a window construction for
    // purposes of error checking.

    thisConstruct.TypeIsWindow = false;
    for (int Layer = 1; Layer <= TotLayers; ++Layer) {
        int const MaterNum = thisConstruct.LayerPoint(Layer);
        if (MaterNum == 0) {
            continue; // error -- has been caught will stop program later
        }
        auto const *mat = s_mat->materials(MaterNum);
        thisConstruct.TypeIsWindow =
            (mat->group == Material::Group::Glass || mat->group == Material::Group::Gas || mat->group == Material::Group::GasMixture ||
             mat->group == Material::Group::Shade || mat->group == Material::Group::Blind || mat->group == Material::Group::Screen ||
             mat->group == Material::Group::GlassSimple || mat->group == Material::Group::ComplexShade ||
             mat->group == Material::Group::ComplexWindowGap || mat->group == Material::Group::GlassEQL || mat->group == Material::Group::ShadeEQL ||
             mat->group == Material::Group::DrapeEQL || mat->group == Material::Group::ScreenEQL || mat->group == Material::Group::BlindEQL ||
             mat->group == Material::Group::WindowGapEQL);
        bool TypeIsNotWindow =
            (mat->group == Material::Group::Invalid || mat->group == Material::Group::AirGap || mat->group == Material::Group::Regular ||
             mat->group == Material::Group::EcoRoof || mat->group == Material::Group::IRTransparent);
        if (!thisConstruct.TypeIsWindow && !TypeIsNotWindow) {
            assert(false);
        }
    }

    if (InsideMaterNum == 0) {
        return;
    }
    auto const *matInside = s_mat->materials(InsideMaterNum);
    if (OutsideMaterNum == 0) {
        return;
    }
    auto const *matOutside = s_mat->materials(OutsideMaterNum);

    if (thisConstruct.TypeIsWindow) {

        bool WrongMaterialsMix = false;
        thisConstruct.NumCTFTerms = 0;
        thisConstruct.NumHistories = 0;
        for (int Layer = 1; Layer <= TotLayers; ++Layer) {
            int const MaterNum = thisConstruct.LayerPoint(Layer);
            if (MaterNum == 0) {
                continue; // error -- has been caught will stop program later
            }
            auto const *mat = s_mat->materials(MaterNum);
            WrongMaterialsMix =
                !((mat->group == Material::Group::Glass) || (mat->group == Material::Group::Gas) || (mat->group == Material::Group::GasMixture) ||
                  (mat->group == Material::Group::Shade) || (mat->group == Material::Group::Blind) || (mat->group == Material::Group::Screen) ||
                  (mat->group == Material::Group::GlassSimple) || (mat->group == Material::Group::ComplexShade) ||
                  (mat->group == Material::Group::ComplexWindowGap) || (mat->group == Material::Group::GlassEQL) ||
                  (mat->group == Material::Group::ShadeEQL) || (mat->group == Material::Group::DrapeEQL) ||
                  (mat->group == Material::Group::ScreenEQL) || (mat->group == Material::Group::BlindEQL) ||
                  (mat->group == Material::Group::WindowGapEQL));
        }

        if (WrongMaterialsMix) { // Illegal material for a window construction
            ShowSevereError(state,
                            format("Error: Window construction={} has materials other than glass, gas, shade, screen, blind, complex shading, "
                                   "complex gap, or simple system.",
                                   thisConstruct.Name));
            ErrorsFound = true;
            // Do not check number of layers for BSDF type of window since that can be handled
        } else if ((TotLayers > 8) && (!thisConstruct.WindowTypeBSDF) &&
                   (!thisConstruct.WindowTypeEQL)) { // Too many layers for a window construction
            ShowSevereError(state,
                            format("CheckAndSetConstructionProperties: Window construction={} has too many layers (max of 8 allowed -- 4 glass + 3 "
                                   "gap + 1 shading device).",
                                   thisConstruct.Name));
            ErrorsFound = true;

        } else if (TotLayers == 1) {
            auto const *mat = s_mat->materials(thisConstruct.LayerPoint(1));
            Material::Group matGroup = mat->group;
            if ((matGroup == Material::Group::Shade) || (matGroup == Material::Group::Gas) || (matGroup == Material::Group::GasMixture) ||
                (matGroup == Material::Group::Blind) || (matGroup == Material::Group::Screen) || (matGroup == Material::Group::ComplexShade) ||
                (matGroup == Material::Group::ComplexWindowGap)) {
                ShowSevereError(state,
                                format("CheckAndSetConstructionProperties: The single-layer window construction={} has a gas, complex gap, shade, "
                                       "complex shade, screen or blind material; it should be glass of simple glazing system.",
                                       thisConstruct.Name));
                ErrorsFound = true;
            }
        }

        // Find total glass layers, total shade/blind layers and total gas layers in a window construction

        bool WrongWindowLayering = false;
        int TotGlassLayers = 0;
        int TotShadeLayers = 0; // Includes shades, blinds, and screens
        int TotGasLayers = 0;
        for (int Layer = 1; Layer <= TotLayers; ++Layer) {
            int const MaterNum = thisConstruct.LayerPoint(Layer);
            if (MaterNum == 0) {
                continue; // error -- has been caught will stop program later
            }
            auto const *mat = s_mat->materials(MaterNum);
            if (mat->group == Material::Group::Glass) {
                ++TotGlassLayers;
            }
            if (mat->group == Material::Group::GlassSimple) {
                ++TotGlassLayers;
            }
            if (mat->group == Material::Group::Shade || mat->group == Material::Group::Blind || mat->group == Material::Group::Screen ||
                mat->group == Material::Group::ComplexShade) {
                ++TotShadeLayers;
            }
            if (mat->group == Material::Group::Gas || mat->group == Material::Group::GasMixture || mat->group == Material::Group::ComplexWindowGap) {
                ++TotGasLayers;
            }
            if (Layer < TotLayers) {
                int const MaterNumNext = thisConstruct.LayerPoint(Layer + 1);
                // Adjacent layers of same type not allowed
                if (MaterNumNext == 0) {
                    continue;
                }
                if (mat->group == s_mat->materials(MaterNumNext)->group) {
                    WrongWindowLayering = true;
                }
            }
        }

        // It is not necessary to check rest of BSDF window structure since that is performed inside TARCOG90 routine.
        // That routine also allow structures which are not allowed in rest of this routine
        if (thisConstruct.WindowTypeBSDF) {
            thisConstruct.TotGlassLayers = TotGlassLayers;
            thisConstruct.TotSolidLayers = TotGlassLayers + TotShadeLayers;
            thisConstruct.InsideAbsorpThermal = matInside->AbsorpThermalBack;
            thisConstruct.OutsideAbsorpThermal = matOutside->AbsorpThermalFront;
            return;
        }

        if (thisConstruct.WindowTypeEQL) {
            thisConstruct.InsideAbsorpThermal = matInside->AbsorpThermalBack;
            thisConstruct.OutsideAbsorpThermal = matOutside->AbsorpThermalFront;
            return;
        }

        if (matOutside->group == Material::Group::Gas || matOutside->group == Material::Group::GasMixture ||
            matInside->group == Material::Group::Gas || matInside->group == Material::Group::GasMixture) {
            WrongWindowLayering = true; // Gas cannot be first or last layer
        }
        if (TotShadeLayers > 1) {
            WrongWindowLayering = true; // At most one shade, screen or blind allowed
        }

        // If there is a diffusing glass layer no shade, screen or blind is allowed
        for (int Layer = 1; Layer <= TotLayers; ++Layer) {
            int const MatNum = thisConstruct.LayerPoint(Layer);
            if (MatNum == 0) {
                continue; // error -- has been caught will stop program later
            }
            auto const *mat = s_mat->materials(MatNum);
            if (mat->group != Material::Group::Glass) {
                continue;
            }
            auto const *matGlass = dynamic_cast<Material::MaterialGlass const *>(mat);
            assert(matGlass != nullptr);
            if (matGlass->SolarDiffusing && TotShadeLayers > 0) {
                ErrorsFound = true;
                ShowSevereError(state, format("CheckAndSetConstructionProperties: Window construction={}", thisConstruct.Name));
                ShowContinueError(state, format("has diffusing glass={} and a shade, screen or blind layer.", matGlass->Name));
                break;
            }
        }

        // If there is a diffusing glass layer it must be the innermost layer
        if (TotGlassLayers > 1) {
            int GlassLayNum = 0;
            for (int Layer = 1; Layer <= TotLayers; ++Layer) {
                int const MatNum = thisConstruct.LayerPoint(Layer);
                if (MatNum == 0) {
                    continue; // error -- has been caught will stop program later
                }
                auto const *mat = s_mat->materials(MatNum);
                if (mat->group != Material::Group::Glass) {
                    continue;
                }

                auto const *matGlass = dynamic_cast<Material::MaterialGlass const *>(mat);
                assert(matGlass != nullptr);
                ++GlassLayNum;
                if (GlassLayNum < TotGlassLayers && matGlass->SolarDiffusing) {
                    ErrorsFound = true;
                    ShowSevereError(state, format("CheckAndSetConstructionProperties: Window construction={}", thisConstruct.Name));
                    ShowContinueError(state, format("has diffusing glass={} that is not the innermost glass layer.", matGlass->Name));
                }
            }
        }

        // interior window screen is not allowed. Check for invalid between-glass screen is checked below.
        if (TotShadeLayers == 1 && matInside->group == Material::Group::Screen && TotLayers != 1) {
            WrongWindowLayering = true;
        }

        // Consistency checks for a construction with a between-glass shade or blind

        if (TotShadeLayers == 1 && matOutside->group != Material::Group::Shade && matOutside->group != Material::Group::Blind &&
            matOutside->group != Material::Group::Screen && matInside->group != Material::Group::Shade &&
            matInside->group != Material::Group::Blind && matInside->group != Material::Group::ComplexShade && !WrongWindowLayering) {

            // This is a construction with a between-glass shade or blind

            if (TotGlassLayers >= 4) {
                // Quadruple pane not allowed.
                WrongWindowLayering = true;
            } else if (TotGlassLayers == 2 || TotGlassLayers == 3) {
                bool ValidBGShadeBlindConst = false;
                if (TotGlassLayers == 2) {
                    if (TotLayers != 5) {
                        WrongWindowLayering = true;
                    } else {
                        if (matOutside->group == Material::Group::Glass &&
                            (s_mat->materials(thisConstruct.LayerPoint(2))->group == Material::Group::Gas ||
                             s_mat->materials(thisConstruct.LayerPoint(2))->group == Material::Group::GasMixture) &&
                            ((s_mat->materials(thisConstruct.LayerPoint(3))->group == Material::Group::Shade ||
                              s_mat->materials(thisConstruct.LayerPoint(3))->group == Material::Group::Blind) &&
                             s_mat->materials(thisConstruct.LayerPoint(3))->group != Material::Group::Screen) &&
                            (s_mat->materials(thisConstruct.LayerPoint(4))->group == Material::Group::Gas ||
                             s_mat->materials(thisConstruct.LayerPoint(4))->group == Material::Group::GasMixture) &&
                            s_mat->materials(thisConstruct.LayerPoint(5))->group == Material::Group::Glass) {
                            ValidBGShadeBlindConst = true;
                        }
                    }
                } else { // TotGlassLayers = 3
                    if (TotLayers != 7) {
                        WrongWindowLayering = true;
                    } else {
                        if (matOutside->group == Material::Group::Glass &&
                            (s_mat->materials(thisConstruct.LayerPoint(2))->group == Material::Group::Gas ||
                             s_mat->materials(thisConstruct.LayerPoint(2))->group == Material::Group::GasMixture) &&
                            s_mat->materials(thisConstruct.LayerPoint(3))->group == Material::Group::Glass &&
                            (s_mat->materials(thisConstruct.LayerPoint(4))->group == Material::Group::Gas ||
                             s_mat->materials(thisConstruct.LayerPoint(4))->group == Material::Group::GasMixture) &&
                            ((s_mat->materials(thisConstruct.LayerPoint(5))->group == Material::Group::Shade ||
                              s_mat->materials(thisConstruct.LayerPoint(5))->group == Material::Group::Blind) &&
                             s_mat->materials(thisConstruct.LayerPoint(5))->group != Material::Group::Screen) &&
                            (s_mat->materials(thisConstruct.LayerPoint(6))->group == Material::Group::Gas ||
                             s_mat->materials(thisConstruct.LayerPoint(6))->group == Material::Group::GasMixture) &&
                            s_mat->materials(thisConstruct.LayerPoint(7))->group == Material::Group::Glass) {
                            ValidBGShadeBlindConst = true;
                        }
                    }
                } // End of check if TotGlassLayers = 2 or 3
                if (!ValidBGShadeBlindConst) {
                    WrongWindowLayering = true;
                }
                if (!WrongWindowLayering) {
                    int const LayNumSh = 2 * TotGlassLayers - 1;
                    int const MatSh = thisConstruct.LayerPoint(LayNumSh);
                    auto const *matSh = s_mat->materials(MatSh);
                    // For double pane, shade/blind must be layer #3.
                    // For triple pane, it must be layer #5 (i.e., between two inner panes).
                    if (matSh->group != Material::Group::Shade && matSh->group != Material::Group::Blind) {
                        WrongWindowLayering = true;
                    }
                    if (TotLayers != 2 * TotGlassLayers + 1) {
                        WrongWindowLayering = true;
                    }
                    if (!WrongWindowLayering) {
                        // Gas on either side of a between-glass shade/blind must be the same
                        int const MatGapL = thisConstruct.LayerPoint(LayNumSh - 1);
                        int const MatGapR = thisConstruct.LayerPoint(LayNumSh + 1);
                        auto const *matGapL = dynamic_cast<const Material::MaterialGasMix *>(s_mat->materials(MatGapL));
                        auto const *matGapR = dynamic_cast<const Material::MaterialGasMix *>(s_mat->materials(MatGapR));
                        for (int IGas = 0; IGas < Material::maxMixGases; ++IGas) {
                            if ((matGapL->gases[IGas].type != matGapR->gases[IGas].type) || (matGapL->gasFracts[IGas] != matGapR->gasFracts[IGas])) {
                                WrongWindowLayering = true;
                            }
                        }
                        // Gap width on either side of a between-glass shade/blind must be the same
                        if (std::abs(matGapL->Thickness - matGapR->Thickness) > 0.0005) {
                            WrongWindowLayering = true;
                        }
                        if (matSh->group == Material::Group::Blind) {
                            auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(matSh);
                            assert(matBlind != nullptr);
                            if ((matGapL->Thickness + matGapR->Thickness) < matBlind->SlatWidth) {
                                ErrorsFound = true;
                                ShowSevereError(state, format("CheckAndSetConstructionProperties: For window construction {}", thisConstruct.Name));
                                ShowContinueError(state, "the slat width of the between-glass blind is greater than");
                                ShowContinueError(state, "the sum of the widths of the gas layers adjacent to the blind.");
                            }
                        } // End of check if material is window blind
                    } // End of check if WrongWindowLayering
                } // End of check if WrongWindowLayering
            } // End of check on total glass layers
        } // End of check if construction has between-glass shade/blind

        // Check Simple Windows,
        if (s_mat->materials(thisConstruct.LayerPoint(1))->group == Material::Group::GlassSimple) {
            if (TotLayers > 1) {
                // check that none of the other layers are glazing or gas
                for (int Layer = 1; Layer <= TotLayers; ++Layer) {
                    int const MaterNum = thisConstruct.LayerPoint(Layer);
                    if (MaterNum == 0) {
                        continue; // error -- has been caught will stop program later
                    }
                    auto const *mat = s_mat->materials(MaterNum);
                    if (mat->group == Material::Group::Glass) {
                        ErrorsFound = true;
                        ShowSevereError(state, format("CheckAndSetConstructionProperties: Error in window construction {}--", thisConstruct.Name));
                        ShowContinueError(state, "For simple window constructions, no other glazing layers are allowed.");
                    }
                    if (mat->group == Material::Group::Gas) {
                        ErrorsFound = true;
                        ShowSevereError(state, format("CheckAndSetConstructionProperties: Error in window construction {}--", thisConstruct.Name));
                        ShowContinueError(state, "For simple window constructions, no other gas layers are allowed.");
                    }
                }
            }
        }

        if (WrongWindowLayering) {
            ShowSevereError(state, format("CheckAndSetConstructionProperties: Error in window construction {}--", thisConstruct.Name));
            ShowContinueError(state, "  For multi-layer window constructions the following rules apply:");
            ShowContinueError(state, "    --The first and last layer must be a solid layer (glass or shade/screen/blind),");
            ShowContinueError(state, "    --Adjacent glass layers must be separated by one and only one gas layer,");
            ShowContinueError(state, "    --Adjacent layers must not be of the same type,");
            ShowContinueError(state, "    --Only one shade/screen/blind layer is allowed,");
            ShowContinueError(state, "    --An exterior shade/screen/blind must be the first layer,");
            ShowContinueError(state, "    --An interior shade/blind must be the last layer,");
            ShowContinueError(state, "    --An interior screen is not allowed,");
            ShowContinueError(state, "    --For an exterior shade/screen/blind or interior shade/blind, there should not be a gas layer");
            ShowContinueError(state, "    ----between the shade/screen/blind and adjacent glass,");
            ShowContinueError(state, "    --A between-glass screen is not allowed,");
            ShowContinueError(state, "    --A between-glass shade/blind is allowed only for double and triple glazing,");
            ShowContinueError(state, "    --A between-glass shade/blind must have adjacent gas layers of the same type and width,");
            ShowContinueError(state, "    --For triple glazing the between-glass shade/blind must be between the two inner glass layers,");
            ShowContinueError(state, "    --The slat width of a between-glass blind must be less than the sum of the widths");
            ShowContinueError(state, "    ----of the gas layers adjacent to the blind.");
            ErrorsFound = true;
        }

        thisConstruct.TotGlassLayers = TotGlassLayers;
        thisConstruct.TotSolidLayers = TotGlassLayers + TotShadeLayers;

        // In following, InsideLayer is layer number of inside glass and InsideAbsorpThermal applies
        // only to inside glass; it is corrected later in InitGlassOpticalCalculations
        // if construction has inside shade or blind.
        if (matInside->group == Material::Group::Shade || matInside->group == Material::Group::Blind) {
            --InsideLayer;
        }
        if (InsideLayer > 0) {
            InsideMaterNum = thisConstruct.LayerPoint(InsideLayer);
            thisConstruct.InsideAbsorpThermal = matInside->AbsorpThermalBack;
        }
        if (InsideMaterNum != 0) {
            auto const *thisInsideMaterial = s_mat->materials(InsideMaterNum);
            thisConstruct.InsideAbsorpVis = thisInsideMaterial->AbsorpVisible;
            thisConstruct.InsideAbsorpSolar = thisInsideMaterial->AbsorpSolar;
        }

        if ((matOutside->group == Material::Group::Glass) || (matOutside->group == Material::Group::GlassSimple)) { // Glass
            thisConstruct.OutsideAbsorpThermal = matOutside->AbsorpThermalFront;
        } else { // Exterior shade, blind or screen
            thisConstruct.OutsideAbsorpThermal = matOutside->AbsorpThermal;
        }

    } else { // Opaque surface
        thisConstruct.InsideAbsorpThermal = matInside->AbsorpThermal;
        thisConstruct.OutsideAbsorpThermal = matOutside->AbsorpThermal;
    }

    thisConstruct.OutsideRoughness = matOutside->Roughness;

    if (matOutside->group == Material::Group::AirGap) {
        ShowSevereError(state, format("CheckAndSetConstructionProperties: Outside Layer is Air for construction {}", thisConstruct.Name));
        ShowContinueError(state, format("  Error in material {}", matOutside->Name));
        ErrorsFound = true;
    }
    if (InsideLayer > 0) {
        if (matInside->group == Material::Group::AirGap) {
            ShowSevereError(state, format("CheckAndSetConstructionProperties: Inside Layer is Air for construction {}", thisConstruct.Name));
            ShowContinueError(state, format("  Error in material {}", matInside->Name));
            ErrorsFound = true;
        }
    }

    if (matOutside->group == Material::Group::EcoRoof) {
        thisConstruct.TypeIsEcoRoof = true;
        // need to check EcoRoof is not non-outside layer
        for (int Layer = 2; Layer <= TotLayers; ++Layer) {
            if (s_mat->materials(thisConstruct.LayerPoint(Layer))->group == Material::Group::EcoRoof) {
                ShowSevereError(state,
                                format("CheckAndSetConstructionProperties: Interior Layer is EcoRoof for construction {}", thisConstruct.Name));
                ShowContinueError(state, format("  Error in material {}", s_mat->materials(thisConstruct.LayerPoint(Layer))->Name));
                ErrorsFound = true;
            }
        }
    }

    if (matOutside->group == Material::Group::IRTransparent) {
        thisConstruct.TypeIsIRT = true;
        if (thisConstruct.TotLayers != 1) {
            ShowSevereError(
                state,
                format("CheckAndSetConstructionProperties: Infrared Transparent (IRT) Construction is limited to 1 layer {}", thisConstruct.Name));
            ShowContinueError(state, "  Too many layers in referenced construction.");
            ErrorsFound = true;
        }
    }
}

int AssignReverseConstructionNumber(EnergyPlusData &state,
                                    int const ConstrNum, // Existing Construction number of first surface
                                    bool &ErrorsFound)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   December 2006

    // PURPOSE OF THIS FUNCTION:
    // For interzone, unentered surfaces, we need to have "reverse" constructions
    // assigned to the created surfaces.  These need to be the reverse (outside to inside layer)
    // of existing surfaces.  Plus, there may be one already in the data structure so this is looked for as well.

    // METHODOLOGY EMPLOYED:
    // Create reverse layers.  Look in current constructions to see if match.  If no match, create a new one.

    auto &s_mat = state.dataMaterial;
    // Return value
    int NewConstrNum; // Reverse Construction Number

    if (ConstrNum == 0) {
        // error caught elsewhere
        NewConstrNum = 0;
        return NewConstrNum;
    }

    auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
    thisConstruct.IsUsed = true;
    int nLayer = 0;
    state.dataConstruction->LayerPoint = 0;
    for (int Loop = thisConstruct.TotLayers; Loop >= 1; --Loop) {
        ++nLayer;
        state.dataConstruction->LayerPoint(nLayer) = thisConstruct.LayerPoint(Loop);
    }

    // now, got thru and see if there is a match already....
    NewConstrNum = 0;
    for (int Loop = 1; Loop <= state.dataHeatBal->TotConstructs; ++Loop) {
        bool Found = true;
        for (nLayer = 1; nLayer <= Construction::MaxLayersInConstruct; ++nLayer) {
            if (state.dataConstruction->Construct(Loop).LayerPoint(nLayer) != state.dataConstruction->LayerPoint(nLayer)) {
                Found = false;
                break;
            }
        }
        if (Found) {
            NewConstrNum = Loop;
            state.dataConstruction->Construct(Loop).IsUsed = true;
            break;
        }
    }

    // if need new one, bunch o stuff
    if (NewConstrNum == 0) {
        ++state.dataHeatBal->TotConstructs;
        state.dataConstruction->Construct.redimension(state.dataHeatBal->TotConstructs);
        state.dataHeatBal->NominalRforNominalUCalculation.redimension(state.dataHeatBal->TotConstructs);
        state.dataHeatBal->NominalRforNominalUCalculation(state.dataHeatBal->TotConstructs) = 0.0;
        state.dataHeatBal->NominalU.redimension(state.dataHeatBal->TotConstructs);
        state.dataHeatBal->NominalU(state.dataHeatBal->TotConstructs) = 0.0;
        state.dataHeatBal->NominalUBeforeAdjusted.redimension(state.dataHeatBal->TotConstructs);
        state.dataHeatBal->NominalUBeforeAdjusted(state.dataHeatBal->TotConstructs) = 0.0;
        state.dataHeatBal->CoeffAdjRatio.redimension(state.dataHeatBal->TotConstructs) = 1.0;
        //  Put in new attributes
        NewConstrNum = state.dataHeatBal->TotConstructs;
        state.dataConstruction->Construct(NewConstrNum).IsUsed = true;
        state.dataConstruction->Construct(state.dataHeatBal->TotConstructs) =
            state.dataConstruction->Construct(ConstrNum); // preserve some of the attributes.
        // replace others...
        state.dataConstruction->Construct(state.dataHeatBal->TotConstructs).Name = "iz-" + state.dataConstruction->Construct(ConstrNum).Name;
        state.dataConstruction->Construct(state.dataHeatBal->TotConstructs).TotLayers = state.dataConstruction->Construct(ConstrNum).TotLayers;
        for (nLayer = 1; nLayer <= Construction::MaxLayersInConstruct; ++nLayer) {
            state.dataConstruction->Construct(state.dataHeatBal->TotConstructs).LayerPoint(nLayer) = state.dataConstruction->LayerPoint(nLayer);
            if (state.dataConstruction->LayerPoint(nLayer) != 0) {
                state.dataHeatBal->NominalRforNominalUCalculation(state.dataHeatBal->TotConstructs) +=
                    s_mat->materials(state.dataConstruction->LayerPoint(nLayer))->NominalR;
            }
        }

        // no error if zero -- that will have been caught with earlier construction
        // the following line was changed to fix CR7601
        if (state.dataHeatBal->NominalRforNominalUCalculation(state.dataHeatBal->TotConstructs) != 0.0) {
            state.dataHeatBal->NominalU(state.dataHeatBal->TotConstructs) =
                1.0 / state.dataHeatBal->NominalRforNominalUCalculation(state.dataHeatBal->TotConstructs);
        }

        CheckAndSetConstructionProperties(state, state.dataHeatBal->TotConstructs, ErrorsFound);
    }

    return NewConstrNum;
}

Real64 ComputeNominalUwithConvCoeffs(EnergyPlusData &state,
                                     int const numSurf, // index for Surface array.
                                     bool &isValid      // returns true if result is valid
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   September 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate Nominal U-value with convection/film coefficients for reporting by
    // adding on prescribed R-values for interior and exterior convection coefficients
    // as found in ASHRAE 90.1-2004, Appendix A. Used in EIO and tabular reports.
    // ASHRAE 90.1-2004 Section A9.4.1 shows the following:
    //      R-value Condition
    //      All exterior conditions                        IP: 0.17  SI: 0.0299
    //      All semi-exterior surfaces                     IP: 0.46  SI: 0.0810
    //      Interior horizontal surfaces, heat flow up     IP: 0.61  SI: 0.1074
    //      Interior horizontal surfaces, heat flow down   IP: 0.92  SI: 0.1620
    //      Interior vertical surfaces                     IP: 0.68  SI: 0.1198
    // This section shows the same value in 90.1-2010 and 90.2-2010
    // Note that this report does not use the semi-exterior surface value because
    // EnergyPlus does not have a way to specifically tell whether or not a surface
    // is connected to a semi-exterior area of the building.  Users can always use
    // the Nominal U-Value to manually calculated this.  The values calculated here
    // are simply reported to the EIO file and not used for any calculations.

    // Return value
    Real64 NominalUwithConvCoeffs; // return value

    static constexpr std::array<Real64, static_cast<int>(DataSurfaces::SurfaceClass::Num)> filmCoefs = {
        0.0,       // None
        0.1197548, // Wall
        0.1620212, // Floor
        0.1074271, // Roof
        0.0,       // IntMass
        0.0,       // Detached_B
        0.0,       // Detached_F
        0.1197548, // Window
        0.1197548, // GlassDoor
        0.1197548, // Door
        0.0,       // Shading
        0.0,       // Overhang
        0.0,       // Fin
        0.0,       // TDD_Dome
        0.0        // TDD_Diffuser
    }; // If anything added to the enum SurfaceClass, adjust this list appropriately

    Real64 insideFilm;
    Real64 outsideFilm;

    isValid = true;

    auto &thisSurface = state.dataSurface->Surface(numSurf);

    // exterior conditions
    switch (thisSurface.ExtBoundCond) {
    case DataSurfaces::ExternalEnvironment: { // ExtBoundCond = 0
        outsideFilm = 0.0299387;              // All exterior conditions
    } break;
    case DataSurfaces::OtherSideCoefCalcExt: {
        outsideFilm = state.dataSurface->OSC(thisSurface.OSCPtr).SurfFilmCoef;
    } break;
    case DataSurfaces::Ground:
    case DataSurfaces::OtherSideCoefNoCalcExt:
    case DataSurfaces::OtherSideCondModeledExt:
    case DataSurfaces::GroundFCfactorMethod:
    case DataSurfaces::KivaFoundation: { // All these cases have a negative ExtBoundCond so don't use film coefficients
        outsideFilm = 0.0;
    } break;
    default: { // Interior Surface Attached to a Zone (ExtBoundCond is a surface)
        outsideFilm = filmCoefs[static_cast<int>(state.dataSurface->Surface(thisSurface.ExtBoundCond).Class)];
    } break;
    }
    // interior conditions and calculate the return value
    if (state.dataHeatBal->NominalU(thisSurface.Construction) > 0.0) {
        insideFilm = filmCoefs[static_cast<int>(thisSurface.Class)];
        if (insideFilm == 0.0) {
            outsideFilm = 0.0;
        }
        NominalUwithConvCoeffs =
            1.0 / (insideFilm + (1.0 / state.dataHeatBal->NominalU(state.dataSurface->Surface(numSurf).Construction)) + outsideFilm);
    } else {
        isValid = false;
        NominalUwithConvCoeffs = state.dataHeatBal->NominalU(state.dataSurface->Surface(numSurf).Construction);
    }

    return NominalUwithConvCoeffs;
}

void SetFlagForWindowConstructionWithShadeOrBlindLayer(EnergyPlusData &state)
{

    // PURPOSE OF THIS SUBROUTINE:
    // check fenestrations with shading control and set a flag to true if its construction has
    // either shade or blind material layer

    // METHODOLOGY EMPLOYED:
    // Loop through Surface and register any shading controls, and loop through the construction
    // material layer

    // Using/Aliasing
    using DataSurfaces::ExternalEnvironment;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int loopSurfNum(0); // surface index
    int ConstrNum(0);   // construction index
    int NumLayers(0);   // number of material layers in a construction
    int Layer(0);       // construction material layer index
    int MaterNum(0);    // construction material index

    auto &s_mat = state.dataMaterial;

    for (loopSurfNum = 1; loopSurfNum <= state.dataSurface->TotSurfaces; ++loopSurfNum) {

        if (state.dataSurface->Surface(loopSurfNum).Class != DataSurfaces::SurfaceClass::Window) {
            continue;
        }
        if (state.dataSurface->Surface(loopSurfNum).ExtBoundCond != ExternalEnvironment) {
            continue;
        }
        if (!state.dataSurface->Surface(loopSurfNum).HasShadeControl) {
            continue;
        }
        if (state.dataSurface->Surface(loopSurfNum).activeShadedConstruction == 0) {
            continue;
        }

        ConstrNum = state.dataSurface->Surface(loopSurfNum).activeShadedConstruction;
        auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
        if (thisConstruct.TypeIsWindow) {
            NumLayers = thisConstruct.TotLayers;
            for (Layer = 1; Layer <= NumLayers; ++Layer) {
                MaterNum = thisConstruct.LayerPoint(Layer);
                if (MaterNum == 0) {
                    continue;
                }
                auto const *mat = s_mat->materials(MaterNum);
                if (mat->group == Material::Group::Shade || mat->group == Material::Group::Blind) {
                    state.dataSurface->SurfWinHasShadeOrBlindLayer(loopSurfNum) = true;
                }
            }
        }
    }
}

void AllocateIntGains(EnergyPlusData &state)
{
    state.dataHeatBal->ZoneIntGain.allocate(state.dataGlobal->NumOfZones);
    state.dataHeatBal->spaceIntGain.allocate(state.dataGlobal->numSpaces);
    state.dataHeatBal->spaceIntGainDevices.allocate(state.dataGlobal->numSpaces);
    state.dataDayltg->spacePowerReductionFactor.dimension(state.dataGlobal->numSpaces, 1.0);
}

} // namespace EnergyPlus::DataHeatBalance
