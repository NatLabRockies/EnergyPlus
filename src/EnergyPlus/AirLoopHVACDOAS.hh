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

#ifndef ENERGYPLUS_AIRLOOPHVACDOAS_HH
#define ENERGYPLUS_AIRLOOPHVACDOAS_HH

// C++ Headers
#include <string>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/SimAirServingZones.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace AirLoopHVACDOAS {

    void CheckConvergence(EnergyPlusData &state);

    struct AirLoopMixer
    {
        std::string name;
        int numOfInletNodes = 0;
        int m_AirLoopMixer_Num = 0;
        int OutletNodeNum = 0;
        std::string OutletNodeName;
        std::vector<std::string> InletNodeName;
        std::vector<int> InletNodeNum;
        Real64 OutletTemp = 0.0;

        static AirLoopMixer *factory(EnergyPlusData &state, int objectNum, std::string const &objectName);
        static void getAirLoopMixer(EnergyPlusData &state);
        void CalcAirLoopMixer(EnergyPlusData &state);
    };

    struct AirLoopSplitter
    {
        std::string name;
        int numOfOutletNodes = 0;
        int m_AirLoopSplitter_Num = 0;
        std::string InletNodeName;
        std::vector<std::string> OutletNodeName;
        std::vector<int> OutletNodeNum;
        Real64 InletTemp = 0.0;
        int InletNodeNum = 0;

        static AirLoopSplitter *factory(EnergyPlusData &state, int objectNum, std::string const &objectName);
        static void getAirLoopSplitter(EnergyPlusData &state);
        void CalcAirLoopSplitter(const EnergyPlusData &state, Real64 Temp, Real64 HumRat);
    };

    struct AirLoopDOAS
    {
        // friend class AirLoopMixer and AirLoopSplitter;
        Real64 SumMassFlowRate = 0.0;
        Real64 PreheatTemp = -999.0;
        Real64 PrecoolTemp = -999.0;
        Real64 PreheatHumRat = -999.0;
        Real64 PrecoolHumRat = -999.0;
        Real64 SizingMassFlow = 0.0;
        Real64 SizingCoolOATemp = -999.0;
        Real64 SizingCoolOAHumRat = -999.0;
        Real64 HeatOutTemp = 999.0;   // outdoor air temperature for heating sizing calculation
        Real64 HeatOutHumRat = 999.0; // outdoor air humidity ratio for heating sizing calculation

        int m_AirLoopDOASNum = 0;
        int m_OASystemNum = 0;
        Sched::Schedule *m_AvailManagerSched = nullptr;
        int m_AirLoopMixerIndex = -1;
        int m_AirLoopSplitterIndex = -1;
        int NumOfAirLoops = 0;
        int m_InletNodeNum = 0;
        int m_OutletNodeNum = 0;
        int m_FanIndex = 0;
        int m_FanInletNodeNum = 0;
        int m_FanOutletNodeNum = 0;
        SimAirServingZones::CompType m_FanTypeNum = SimAirServingZones::CompType::Invalid;
        bool m_exhaustFanUsed = false;
        int m_exhaustFanIndex = -1;
        int m_exhaustFanInletNodeNum = 0;
        int m_exhaustFanOutletNodeNum = 0;
        SimAirServingZones::CompType m_exhaustFanTypeNum = SimAirServingZones::CompType::Invalid;
        int m_HeatCoilNum = 0;
        int m_CoolCoilNum = 0;
        int ConveCount = 0;
        int ConveIndex = 0;

        bool m_HeatExchangerFlag = false;
        bool SizingOnceFlag = true;
        bool FanBeforeCoolingCoilFlag = false;

        AirLoopMixer *m_CompPointerAirLoopMixer = nullptr;
        AirLoopSplitter *m_CompPointerAirLoopSplitter = nullptr;

        std::string Name;
        std::string AvailManagerSchedName;
        std::string OASystemName;
        std::string AirLoopMixerName;
        std::string AirLoopSplitterName;
        std::string FanName;

        std::vector<int> m_AirLoopNum; // array of AirLoop number
        std::vector<std::string> AirLoopName;
        std::vector<int> m_OACtrlNum; // array of OA controller number

        PlantLocation HWPlantLoc;
        int HWCtrlNodeNum = 0;
        PlantLocation CWPlantLoc;
        int CWCtrlNodeNum = 0;
        bool MyEnvrnFlag = true;

        static void getAirLoopDOASInput(EnergyPlusData &state);

        void SimAirLoopHVACDOAS(EnergyPlusData &state, bool firstHVACIteration, int &CompIndex);

        void initAirLoopDOAS(EnergyPlusData &state, bool FirstHVACIteration);

        void CalcAirLoopDOAS(EnergyPlusData &state, bool FirstHVACIteration);

        void SizingAirLoopDOAS(EnergyPlusData &state);

        void GetDesignDayConditions(EnergyPlusData &state);
    };

    int getAirLoopMixerIndex(EnergyPlusData &state, std::string const &objectName);
    int getAirLoopSplitterIndex(EnergyPlusData &state, std::string const &objectName);
    void getAirLoopHVACDOASInput(EnergyPlusData &state);

} // namespace AirLoopHVACDOAS

struct AirLoopHVACDOASData final : BaseGlobalStruct
{
    bool GetInputOnceFlag = true;
    bool getAirLoopMixerInputOnceFlag = true;
    bool getAirLoopSplitterInputOnceFlag = true;
    std::vector<AirLoopHVACDOAS::AirLoopDOAS> airloopDOAS;
    std::vector<AirLoopHVACDOAS::AirLoopMixer> airloopMixer;
    std::vector<AirLoopHVACDOAS::AirLoopSplitter> airloopSplitter;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) AirLoopHVACDOASData();
    }
};

} // namespace EnergyPlus
#endif // ENERGYPLUS_AIRLOOPHVACDOAS_HH
