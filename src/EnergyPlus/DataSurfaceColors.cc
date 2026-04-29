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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSurfaceColors.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataSurfaceColors {

// Module containing the data dealing with the coloring of surfaces for
// various outputs (such as DXF)

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   Aug 2007
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// Contain the data for surface colors and user settings for DXF and possibly
// other surface reporting.

bool MatchAndSetColorTextString(EnergyPlusData &state,
                                std::string const &String,       // string to be matched
                                int const SetValue,              // value to be used for the color
                                std::string_view const ColorType // for now, must be DXF and probably not a string in the future
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    static constexpr std::array<std::string_view, static_cast<int>(DataSurfaceColors::ColorNo::Num)> colorkeys = {
        "TEXT",
        "WALLS",
        "WINDOWS",
        "GLASSDOORS",
        "DOORS",
        "ROOFS",
        "FLOORS",
        "DETACHEDBUILDINGSHADES",
        "DETACHEDFIXEDSHADES",
        "ATTACHEDBUILDINGSHADES",
        "PHOTOVOLTAICS",
        "TUBULARDAYLIGHTDOMES",
        "TUBULARDAYLIGHTDIFFUSERS",
        "DAYLIGHTREFERENCEPOINT1",
        "DAYLIGHTREFERENCEPOINT2",
    };

    // ColorType must be "DXF"
    if (ColorType != "DXF") {
        return false;
    }

    // try to find enum value
    int foundIdx = getEnumValue(colorkeys, Util::makeUPPER(String));
    if (foundIdx == -1) {
        return false;
    }

    // if we've made it here, we found the value
    state.dataSurfColor->DXFcolorno[foundIdx] = SetValue;
    return true;
}

void SetUpSchemeColors(EnergyPlusData &state, std::string const &SchemeName, std::string_view const ColorType)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine resets the colorno array(s) with the entered scheme name as
    // required for reporting.

    // METHODOLOGY EMPLOYED:
    // This routine first sets the color arrays to default.  Then, attempts to find the
    // scheme name in the Input File.  If found, processes that scheme and sets colors.
    // Worst case: the colors remain as default.  Note -- this allocates and deallocates
    // the alphas and numerics required to process the Report:SurfaceColorScheme object.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view CurrentModuleObject("OutputControl:SurfaceColorScheme");

    state.dataSurfColor->DXFcolorno = DataSurfaceColors::defaultcolorno;

    auto *inputProcessor = state.dataInputProcessing->inputProcessor.get();
    auto const surfaceColorSchemes = inputProcessor->epJSON.find(std::string(CurrentModuleObject));
    if (surfaceColorSchemes != inputProcessor->epJSON.end()) {
        auto matchedScheme = surfaceColorSchemes.value().end();
        for (auto it = surfaceColorSchemes.value().begin(); it != surfaceColorSchemes.value().end(); ++it) {
            if (Util::SameString(it.key(), SchemeName)) {
                matchedScheme = it;
                break;
            }
        }

        if (matchedScheme != surfaceColorSchemes.value().end()) {
            auto const &schemeFields = matchedScheme.value();
            inputProcessor->markObjectAsUsed(std::string(CurrentModuleObject), matchedScheme.key());

            for (int numargs = 1;; ++numargs) {
                auto const drawingElementKey = EnergyPlus::format("drawing_element_{}_type", numargs);
                auto const colorKey = EnergyPlus::format("color_for_drawing_element_{}", numargs);
                auto const drawingElementIt = schemeFields.find(drawingElementKey);
                auto const colorIt = schemeFields.find(colorKey);

                if (drawingElementIt == schemeFields.end() && colorIt == schemeFields.end()) {
                    break;
                }

                std::string const drawingElementFieldName = EnergyPlus::format("Drawing Element {} Type", numargs);
                std::string const colorFieldName = EnergyPlus::format("Color for Drawing Element {}", numargs);
                std::string const drawingElement = (drawingElementIt != schemeFields.end()) ? drawingElementIt->get<std::string>() : "";

                if (colorIt == schemeFields.end()) {
                    if (!drawingElement.empty()) {
                        ShowWarningError(state,
                                         EnergyPlus::format("SetUpSchemeColors: {}={}, {}={}, {} was blank.  Default color retained.",
                                                            "Name",
                                                            SchemeName,
                                                            drawingElementFieldName,
                                                            drawingElement,
                                                            colorFieldName));
                    }
                    continue;
                }

                int const numptr = colorIt->get<int>();
                if (!MatchAndSetColorTextString(state, drawingElement, numptr, ColorType)) {
                    ShowWarningError(state,
                                     EnergyPlus::format("SetUpSchemeColors: {}={}, {}={}, is invalid.  No color set.",
                                                        "Name",
                                                        SchemeName,
                                                        drawingElementFieldName,
                                                        drawingElement));
                }
            }
        } else {
            ShowWarningError(state, EnergyPlus::format("SetUpSchemeColors: Name={} not on input file. Default colors will be used.", SchemeName));
        }
    } else {
        ShowWarningError(state, EnergyPlus::format("SetUpSchemeColors: Name={} not on input file. Default colors will be used.", SchemeName));
    }
}

} // namespace EnergyPlus::DataSurfaceColors
