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

#include "AirflowNetwork/Linkages.hpp"

namespace EnergyPlus {

namespace AirflowNetwork {

void handle_nonrectangular_surfaces(EquivRec equivalent_rectangle_method, EPVector<DataSurfaces::SurfaceData> Surface, const std::string &surface_name, int surface_number,
    Real64 &height, Real64 &width, Real64 user_aspect_ratio, EnergyPlusData *state)
{
    if (equivalent_rectangle_method == EquivRec::Height) {
        if (Surface(surface_number).Tilt < 1.0 || Surface(surface_number).Tilt > 179.0) { // horizontal surface
            // check base surface shape
            if (Surface(Surface(surface_number).BaseSurf).Sides == 4) {
                Real64 baseratio = Surface(Surface(surface_number).BaseSurf).Width / Surface(Surface(surface_number).BaseSurf).Height;
                width = sqrt(Surface(surface_number).Area * baseratio);
                height = Surface(surface_number).Area / width;
                if (state != nullptr) {
                    // Going to lie about the source of the error
                    ShowWarningError(*state, "AirflowNetwork::Solver::get_input: AirflowNetwork:MultiZone:Surface object = " + surface_name);
                    ShowContinueError(*state,
                                        "The entered choice of Equivalent Rectangle Method is PolygonHeight. This choice is not valid for "
                                        "a horizontal surface.");
                    ShowContinueError(*state, "The BaseSurfaceAspectRatio choice is used. Simulation continues.");
                }
            } else {
                width = sqrt(Surface(surface_number).Area * user_aspect_ratio);
                height = Surface(surface_number).Area / width;
                // add warning
                if (state != nullptr) {
                    // Going to lie about the source of the error
                    ShowWarningError(*state, "AirflowNetwork::Solver::get_input: AirflowNetwork:MultiZone:Surface object = " + surface_name);
                    ShowContinueError(*state,
                                        "The entered choice of Equivalent Rectangle Method is PolygonHeight. This choice is not valid for "
                                        "a horizontal surface with a polygonal base surface.");
                    ShowContinueError(*state, "The default aspect ratio at 1 is used. Simulation continues.");
                }
            }
        } else {
            Real64 minHeight = min(Surface(surface_number).Vertex(1).z, Surface(surface_number).Vertex(2).z);
            Real64 maxHeight = max(Surface(surface_number).Vertex(1).z, Surface(surface_number).Vertex(2).z);
            for (int j = 3; j <= Surface(surface_number).Sides; ++j) {
                minHeight = min(minHeight,
                                min(Surface(surface_number).Vertex(j - 1).z, Surface(surface_number).Vertex(j).z));
                maxHeight = max(maxHeight,
                                max(Surface(surface_number).Vertex(j - 1).z, Surface(surface_number).Vertex(j).z));
            }
            if (maxHeight > minHeight) {
                height = maxHeight - minHeight;
                width = Surface(surface_number).Area / (maxHeight - minHeight);
            }
            // There's no else here, this is a fall-through. This appears to agree with the original intent, which defaults the height and width to the surface values.
        }
    } else if (equivalent_rectangle_method == EquivRec::BaseAspectRatio) {

        if (Surface(Surface(surface_number).BaseSurf).Sides == 4) {
            Real64 baseratio = Surface(Surface(surface_number).BaseSurf).Width / Surface(Surface(surface_number).BaseSurf).Height;
            width = sqrt(Surface(surface_number).Area * baseratio);
            height = Surface(surface_number).Area / width;
        } else {
            Real64 minHeight = min(Surface(surface_number).Vertex(1).z, Surface(surface_number).Vertex(2).z);
            Real64 maxHeight = max(Surface(surface_number).Vertex(1).z, Surface(surface_number).Vertex(2).z);
            for (int j = 3; j <= Surface(surface_number).Sides; ++j) {
                minHeight = min(minHeight, min(Surface(surface_number).Vertex(j - 1).z, Surface(surface_number).Vertex(j).z));
                maxHeight = max(maxHeight, max(Surface(surface_number).Vertex(j - 1).z, Surface(surface_number).Vertex(j).z));
            }
            if (maxHeight > minHeight) {
                height = maxHeight - minHeight;
                width = Surface(surface_number).Area / (maxHeight - minHeight);
                // add warning
                if (state != nullptr) {
                    // Going to lie about the source of the error
                    ShowWarningError(*state,
                                     "AirflowNetwork::Solver::get_input: AirflowNetwork:MultiZone:Surface object = " +
                                         surface_name);
                    ShowContinueError(*state,
                                      "The entered choice of Equivalent Rectangle Method is BaseSurfaceAspectRatio. This choice is not "
                                      "valid for a polygonal base surface.");
                    ShowContinueError(*state, "The PolygonHeight choice is used. Simulation continues.");
                }
            } else {
                width = sqrt(Surface(surface_number).Area * user_aspect_ratio);
                height = Surface(surface_number).Area / width;
                // add warning
                if (state != nullptr) {
                    // Going to lie about the source of the error
                    ShowWarningError(*state, "AirflowNetwork::Solver::get_input: AirflowNetwork:MultiZone:Surface object = " + surface_name);
                    ShowContinueError(*state,
                                      "The entered choice of Equivalent Rectangle Method is BaseSurfaceAspectRatio. This choice is not "
                                      "valid for a horizontal surface with a polygonal base surface.");
                    ShowContinueError(*state, "The default aspect ratio at 1 is used. Simulation continues.");
                }
            }
        }
    } else if (equivalent_rectangle_method == EquivRec::UserAspectRatio) {
        width = sqrt(Surface(surface_number).Area * user_aspect_ratio);
        height = Surface(surface_number).Area / width;
    }
    // end of handle_nonrectangular_surfaces
}

} // namespace AirflowNetwork

} // namespace EnergyPlus
