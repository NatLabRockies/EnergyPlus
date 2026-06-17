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

#ifndef MixerComponent_hh_INCLUDED
#define MixerComponent_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace MixerComponent {

    struct MixerConditions
    {
        // Members
        std::string MixerName; // Name of the Mixer
        Real64 OutletTemp;
        Real64 OutletHumRat;
        Real64 OutletEnthalpy;
        Real64 OutletPressure;
        int OutletNode;
        Real64 OutletMassFlowRate;         // MassFlow through the Mixer being Simulated [kg/Sec]
        Real64 OutletMassFlowRateMaxAvail; // [kg/Sec]
        Real64 OutletMassFlowRateMinAvail; // [kg/Sec]
        bool InitFlag;
        int NumInletNodes;
        Array1D_int InletNode;
        Array1D<Real64> InletMassFlowRate;
        Array1D<Real64> InletMassFlowRateMaxAvail;
        Array1D<Real64> InletMassFlowRateMinAvail;
        Array1D<Real64> InletTemp;
        Array1D<Real64> InletHumRat;
        Array1D<Real64> InletEnthalpy;
        Array1D<Real64> InletPressure;

        // Default Constructor
        MixerConditions()
            : OutletTemp(0.0), OutletHumRat(0.0), OutletEnthalpy(0.0), OutletPressure(0.0), OutletNode(0), OutletMassFlowRate(0.0),
              OutletMassFlowRateMaxAvail(0.0), OutletMassFlowRateMinAvail(0.0), InitFlag(false), NumInletNodes(0)
        {
        }
    };

    // Functions

    void SimAirMixer(EnergyPlusData &state, std::string_view CompName, int &CompIndex);

    // Get Input Section of the Module
    //******************************************************************************

    void GetMixerInput(EnergyPlusData &state);

    // End of Get Input subroutines for the HB Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitAirMixer(EnergyPlusData &state, int MixerNum);

    // End Initialization Section of the Module
    //******************************************************************************

    // Begin Algorithm Section of the Module
    //******************************************************************************

    void CalcAirMixer(EnergyPlusData &state, int &MixerNum);

    // End Algorithm Section of the Module
    // *****************************************************************************

    // Beginning of Update subroutines for the Mixer Module
    // *****************************************************************************

    void UpdateAirMixer(EnergyPlusData &state, int const MixerNum);

    //        End of Update subroutines for the Mixer Module
    // *****************************************************************************

    // Beginning of Reporting subroutines for the Mixer Module
    // *****************************************************************************

    void ReportMixer(int MixerNum);

    //        End of Reporting subroutines for the Mixer Module
    // *****************************************************************************

    // Beginning of Utility subroutines for the Mixer Component
    // *****************************************************************************
    void GetZoneMixerIndex(
        EnergyPlusData &state, std::string const &MixerName, int &MixerIndex, bool &ErrorsFound, std::string const &ThisObjectType = std::string());

    int getZoneMixerIndexFromInletNode(EnergyPlusData &state, int InNodeNum);

    // End of Utility subroutines for the Mixer Component
    // *****************************************************************************

} // namespace MixerComponent

struct MixerComponentData : BaseGlobalStruct
{

    int NumMixers = 0;
    int LoopInletNode = 0;
    int LoopOutletNode = 0;
    bool SimAirMixerInputFlag = true;
    bool GetZoneMixerIndexInputFlag = true;
    Array1D_bool CheckEquipName;
    EPVector<MixerComponent::MixerConditions> MixerCond;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumMixers = 0;
        this->LoopInletNode = 0;
        this->LoopOutletNode = 0;
        this->GetZoneMixerIndexInputFlag = true;
        this->SimAirMixerInputFlag = true;
        this->CheckEquipName.deallocate();
        this->MixerCond.deallocate();
    }
};

} // namespace EnergyPlus

#endif
