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

#ifndef MoistureBalanceEMPDManager_hh_INCLUDED
#define MoistureBalanceEMPDManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Material.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace MoistureBalanceEMPDManager {

    struct MaterialEMPD : public Material::MaterialBase
    {
        // EMPD
        Real64 mu = 0.0;               // Water Vapor Diffusion Resistance Factor (dimensionless)
        Real64 moistACoeff = 0.0;      // Moisture Equation Coefficient a
        Real64 moistBCoeff = 0.0;      // Moisture Equation Coefficient b
        Real64 moistCCoeff = 0.0;      // Moisture Equation Coefficient c
        Real64 moistDCoeff = 0.0;      // Moisture Equation Coefficient d
        Real64 surfaceDepth = 0.0;     // Surface-layer penetration depth (m)
        Real64 deepDepth = 0.0;        // Deep-layer penetration depth (m)
        Real64 coatingThickness = 0.0; // Coating Layer Thickness (m)
        Real64 muCoating = 0.0;        // Coating Layer water vapor diffusion resistance factor (dimensionless)

        MaterialEMPD() : MaterialBase()
        {
            group = Material::Group::Regular;
        } // Change this to group EMPD?
        ~MaterialEMPD() = default;

        Real64 calcDepthFromPeriod(EnergyPlusData &state, Real64 period); // in seconds
    };

    struct EMPDReportVarsData
    {
        Real64 rv_surface;
        Real64 RH_surface_layer;
        Real64 RH_deep_layer;
        Real64 w_surface_layer;
        Real64 w_deep_layer;
        Real64 mass_flux_zone;
        Real64 mass_flux_deep;
        Real64 u_surface_layer;
        Real64 u_deep_layer;

        // Default constructor
        EMPDReportVarsData()
            : rv_surface(0.015), RH_surface_layer(0.0), RH_deep_layer(0.0), w_surface_layer(0.015), w_deep_layer(0.015), mass_flux_zone(0.0),
              mass_flux_deep(0.0), u_surface_layer(0.0), u_deep_layer(0.0)
        {
        }
    };

    void GetMoistureBalanceEMPDInput(EnergyPlusData &state);

    void InitMoistureBalanceEMPD(EnergyPlusData &state);

    void CalcMoistureBalanceEMPD(EnergyPlusData &state,
                                 int SurfNum,
                                 Real64 SurfTempIn, // INSIDE SURFACE TEMPERATURE at current time step
                                 Real64 TempZone,   // Zone temperature at current time step.
                                 Real64 &TempSat    // Saturated surface temperature.
    );

    void UpdateMoistureBalanceEMPD(EnergyPlusData &state, int SurfNum); // Surface number

    void ReportMoistureBalanceEMPD(EnergyPlusData &state);

} // namespace MoistureBalanceEMPDManager

struct MoistureBalanceEMPDManagerData : BaseGlobalStruct
{

    // Array of structs that hold the empd report vars data, one for each surface.
    EPVector<MoistureBalanceEMPDManager::EMPDReportVarsData> EMPDReportVars;
    bool InitEnvrnFlag = true;
    int ErrCount = 0;
    bool OneTimeFlag = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->EMPDReportVars.deallocate();
        this->InitEnvrnFlag = true;
        this->ErrCount = 0;
        this->OneTimeFlag = true;
    }
};

} // namespace EnergyPlus

#endif
