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

#ifndef ThermalChimney_hh_INCLUDED
#define ThermalChimney_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2A.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ThermalChimney {

    struct ThermalChimneyData
    {
        // Members
        std::string Name;
        int RealZonePtr;
        std::string RealZoneName;
        Sched::Schedule *availSched = nullptr;
        Real64 AbsorberWallWidth;
        Real64 AirOutletCrossArea;
        Real64 DischargeCoeff;
        int TotZoneToDistrib;
        bool EMSOverrideOn;         // if true then EMS is requesting to override
        Real64 EMSAirFlowRateValue; // value EMS is setting for air flow rate
        Array1D_int ZonePtr;
        Array1D_int spacePtr;
        Array1D_string ZoneName;
        Array1D<Real64> DistanceThermChimInlet;
        Array1D<Real64> RatioThermChimAirFlow;
        Array1D<Real64> EachAirInletCrossArea;

        // Default Constructor
        ThermalChimneyData()
            : RealZonePtr(0), AbsorberWallWidth(0.0), AirOutletCrossArea(0.0), DischargeCoeff(0.0), TotZoneToDistrib(0), EMSOverrideOn(false),
              EMSAirFlowRateValue(0)
        {
        }
    };

    struct ThermChimZnReportVars
    {
        // Members
        Real64 ThermalChimneyHeatLoss; // Heat Gain {Joules} due to ThermalChimney
        Real64 ThermalChimneyHeatGain; // Heat Loss {Joules} due to ThermalChimney
        Real64 ThermalChimneyVolume;   // Volume of Air {m3} due to ThermalChimney
        Real64 ThermalChimneyMass;     // Mass of Air {kg} due to ThermalChimney

        // Default Constructor
        ThermChimZnReportVars() : ThermalChimneyHeatLoss(0.0), ThermalChimneyHeatGain(0.0), ThermalChimneyVolume(0.0), ThermalChimneyMass(0.0)
        {
        }
    };

    struct ThermChimReportVars
    {
        // Members
        Real64 OverallTCVolumeFlow;      // Volume of Air {m3/s} due to ThermalChimney
        Real64 OverallTCVolumeFlowStd;   // Volume of Air {m3/s} due to ThermalChimney at standard conditions
        Real64 OverallTCMassFlow;        // Mass of Air {kg/s} due to ThermalChimney
        Real64 OutletAirTempThermalChim; // Air Temp {C} of ThermalChimney

        // Default Constructor
        ThermChimReportVars() : OverallTCVolumeFlow(0.0), OverallTCVolumeFlowStd(0.0), OverallTCMassFlow(0.0), OutletAirTempThermalChim(0.0)
        {
        }
    };

    // Functions

    void ManageThermalChimney(EnergyPlusData &state);

    void GetThermalChimney(EnergyPlusData &state, bool &ErrorsFound); // If errors found in input

    void CalcThermalChimney(EnergyPlusData &state);

    void ReportThermalChimney(EnergyPlusData &state);

    void GaussElimination(Array2A<Real64> EquaCoef, Array1D<Real64> &EquaConst, Array1D<Real64> &ThermChimSubTemp, int const NTC);

    //        End of Module Subroutines for ThermalChimney

    //*****************************************************************************************

} // namespace ThermalChimney
struct ThermalChimneysData : BaseGlobalStruct
{

    bool ThermalChimneyGetInputFlag = true;
    int TotThermalChimney = 0; // Total ThermalChimney Statements in input

    EPVector<ThermalChimney::ThermalChimneyData> ThermalChimneySys;
    EPVector<ThermalChimney::ThermChimZnReportVars> ZnRptThermChim;
    EPVector<ThermalChimney::ThermChimReportVars> ThermalChimneyReport;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        ThermalChimneyGetInputFlag = true;
        ZnRptThermChim.deallocate();
        ThermalChimneySys.deallocate();
        ThermalChimneyReport.deallocate();
    }

    // Default Constructor
    ThermalChimneysData() = default;
};
} // namespace EnergyPlus

#endif
