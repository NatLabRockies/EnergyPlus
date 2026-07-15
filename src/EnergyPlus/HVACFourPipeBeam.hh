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

#ifndef HVACFourPipeBeam_hh_INCLUDED
#define HVACFourPipeBeam_hh_INCLUDED

#include <memory>
// ObjexxFCL Headers
#include <ObjexxFCL/Array1.hh>

// EnergyPlus Headers
#include <EnergyPlus/AirTerminalUnit.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace FourPipeBeam {

    class HVACFourPipeBeam : public AirTerminalUnit
    {

    private: // Creation
        // Default Constructor
        HVACFourPipeBeam()
            : coolingAvailable(false), heatingAvailable(false), totBeamLength(0.0), totBeamLengthWasAutosized(false), vDotNormRatedPrimAir(0.0),
              mDotNormRatedPrimAir(0.0), beamCoolingPresent(false), vDotDesignCW(0.0), vDotDesignCWWasAutosized(false), mDotDesignCW(0.0),
              qDotNormRatedCooling(0.0), deltaTempRatedCooling(0.0), vDotNormRatedCW(0.0), mDotNormRatedCW(0.0), modCoolingQdotDeltaTFuncNum(0),
              modCoolingQdotAirFlowFuncNum(0), modCoolingQdotCWFlowFuncNum(0), mDotCW(0.0), cWTempIn(0.0), cWTempOut(0.0), cWTempOutErrorCount(0),
              cWInNodeNum(0), cWOutNodeNum(0), cWplantLoc(PlantLocation(0, DataPlant::LoopSideLocation::Invalid, 0, 0)), beamHeatingPresent(false),
              vDotDesignHW(0.0), vDotDesignHWWasAutosized(false), mDotDesignHW(0.0), qDotNormRatedHeating(0.0), deltaTempRatedHeating(0.0),
              vDotNormRatedHW(0.0), mDotNormRatedHW(0.0), modHeatingQdotDeltaTFuncNum(0), modHeatingQdotAirFlowFuncNum(0),
              modHeatingQdotHWFlowFuncNum(0), mDotHW(0.0), hWTempIn(0.0), hWTempOut(0.0), hWTempOutErrorCount(0), hWInNodeNum(0), hWOutNodeNum(0),
              hWplantLoc(PlantLocation(0, DataPlant::LoopSideLocation::Invalid, 0, 0)), beamCoolingEnergy(0.0), beamCoolingRate(0.0),
              beamHeatingEnergy(0.0), beamHeatingRate(0.0), supAirCoolingEnergy(0.0), supAirCoolingRate(0.0), supAirHeatingEnergy(0.0),
              supAirHeatingRate(0.0), primAirFlow(0.0), OutdoorAirFlowRate(0.0), myEnvrnFlag(true), mySizeFlag(true), plantLoopScanFlag(true),
              zoneEquipmentListChecked(false), tDBZoneAirTemp(0.0), tDBSystemAir(0.0), mDotSystemAir(0.0), cpZoneAir(0.0), cpSystemAir(0.0),
              qDotSystemAir(0.0), qDotBeamCoolingMax(0.0), qDotBeamHeatingMax(0.0), qDotTotalDelivered(0.0), qDotBeamCooling(0.0),
              qDotBeamHeating(0.0), qDotZoneReq(0.0), qDotBeamReq(0.0), qDotZoneToHeatSetPt(0.0), qDotZoneToCoolSetPt(0.0)
        {
        }

    public:
        // Destructor
        virtual ~HVACFourPipeBeam()
        {
        }

    public: // Methods        MARK ANY THAT DON'T ALTER STATE const !!!
        ///// Note use of shared_ptr here is not a good pattern, not to be replicated without further discussion.
        static std::shared_ptr<AirTerminalUnit> fourPipeBeamFactory(EnergyPlusData &state, std::string objectName);
        void simulate(EnergyPlusData &state,
                      bool FirstHVACIteration, // TRUE if first HVAC iteration in time step
                      Real64 &NonAirSysOutput  // convective cooling by the beam system [W]
        );

        int getZoneIndex();

        int getAirLoopNum();

        Real64 getPrimAirDesignVolFlow();

        int getTermUnitSizingIndex();

        void reportTerminalUnit(EnergyPlusData &state);

    private: // Methods
        void
        init(EnergyPlusData &state,
             bool const FirstHVACIteration // TRUE if first air loop solution this HVAC step         MAYBE THIS SHOULD HAVE A DEFAULT ARG OF = false
        );

        void set_size(EnergyPlusData &state);

        void control(EnergyPlusData &state,
                     bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                     Real64 &NonAirSysOutput        // convective cooling by the beam system [W]
        );

        void calc(EnergyPlusData &state);

        void update(EnergyPlusData &state) const;

        void report(EnergyPlusData &state);

        void CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state);

    private:                                          // data
        Sched::Schedule *coolingAvailSched = nullptr; // schedule for cooling availability
        bool coolingAvailable;                        // true if beam cooling is available
        Sched::Schedule *heatingAvailSched = nullptr; // schedule for heating availability
        bool heatingAvailable;                        // true if beam heating is available

        Real64 totBeamLength;           // length of all the beams in the zone (autosizable) (m)
        bool totBeamLengthWasAutosized; // true if beam length was autosized on input
        Real64 vDotNormRatedPrimAir;    // normalized primary air volume flow rate at rating point (m3/s-m)
        Real64 mDotNormRatedPrimAir;    // normalized primary air mass flow rate at rating point (kg/s-m)
        // cooling
        bool beamCoolingPresent;          // true if chilled water system is connected to beam
        Real64 vDotDesignCW;              // Design chilled water volume flow rate (autosizable) (m3/s)
        bool vDotDesignCWWasAutosized;    // true if use input for chilled water flow was autosized
        Real64 mDotDesignCW;              // Design chilled water mass flow rate (kg/s)
        Real64 qDotNormRatedCooling;      // normalized cooling capacity at rating point (W/m)
        Real64 deltaTempRatedCooling;     // temperature difference between zone air and entering chilled water at rating point (delta C)
        Real64 vDotNormRatedCW;           // normalized chilled water volume flow rate at rating point (m3/s-m)
        Real64 mDotNormRatedCW;           // normalized chilled water mass flow rate at rating point (kg/s-m)
        int modCoolingQdotDeltaTFuncNum;  // index to curve or table modifying cooling capacity as a function of delta T ratio
        int modCoolingQdotAirFlowFuncNum; // index to curve or table modifying cooling capacity as a function of air flow ratio
        int modCoolingQdotCWFlowFuncNum;  // index to curve or table modifying cooling capacity as a function of chilled water flow ratio
        Real64 mDotCW;                    // current chilled water mass flow rate (kg/s)
        Real64 cWTempIn;                  // current inlet chilled water temperature [C]
        Real64 cWTempOut;                 // current outlet chilled water temperature [C]
        int cWTempOutErrorCount;          // counter for recurring errors in chilled water outlet temperature
        int cWInNodeNum;                  // chilled water inlet node
        int cWOutNodeNum;                 // chilled water outlet nod
        PlantLocation cWplantLoc;         // chilled water plant loop location
        // heating
        bool beamHeatingPresent;          // true if hot water system is connected to beam
        Real64 vDotDesignHW;              // Design hot water volume flow rate (autosizable) (m3/s)
        bool vDotDesignHWWasAutosized;    // true if user input for hot water flow was autosized
        Real64 mDotDesignHW;              // Design hot water mass flow rate (kg/s)
        Real64 qDotNormRatedHeating;      // normalized heating capacity at rating point (W/m)
        Real64 deltaTempRatedHeating;     // temperature difference between zone air and entering hot water at rating point (delta C)
        Real64 vDotNormRatedHW;           // normalized hot water volume flow rate at rating point (m3/s-m)
        Real64 mDotNormRatedHW;           // normalized hot water mass flow rate at rating point (kg/s-m)
        int modHeatingQdotDeltaTFuncNum;  // index to curve or table modifying heating capacity as a function of delta T ratio
        int modHeatingQdotAirFlowFuncNum; // index to curve or table modifying heating capacity as a function of air flow ratio
        int modHeatingQdotHWFlowFuncNum;  // index to curve or table modifying heating capacity as a function of chilled water flow ratio
        Real64 mDotHW;                    // current hot water mass flow rate (kg/s)
        Real64 hWTempIn;                  // current inlet hot water temperature (C)
        Real64 hWTempOut;                 // current outlet hot water temperature (C)
        int hWTempOutErrorCount;          // counter for recurring errors in hot water outlet temperature
        int hWInNodeNum;                  // hot water inlet node
        int hWOutNodeNum;                 // hot water outlet node
        PlantLocation hWplantLoc;         // hot water connection location structure

        // output variables
        Real64 beamCoolingEnergy;   // beam sensible cooling energy of all beams in the zone [J]
        Real64 beamCoolingRate;     // beam sensible cooling rate of all beams in the zone (positive convention) [W]
        Real64 beamHeatingEnergy;   // beam heating energy of all beams in the zone [J]
        Real64 beamHeatingRate;     // beam heating rate of all beams in the zone [W]
        Real64 supAirCoolingEnergy; // Total cooling energy from supply air [J]
        Real64 supAirCoolingRate;   // Total cooling rate from supply air [W]
        Real64 supAirHeatingEnergy; // Total cooling energy from supply air [J]
        Real64 supAirHeatingRate;   // Total cooling rate from supply air [W]
        Real64 primAirFlow;         // supply air flow per zone at standard elevation-adjusted density [m3/s]
        Real64 OutdoorAirFlowRate;  // zone outdoor air volume flow rate

        bool myEnvrnFlag;              // control when to re initialize for new environment period
        bool mySizeFlag;               // control when to run sizing method
        bool plantLoopScanFlag;        // control when to look up plant locations for water connections
        bool zoneEquipmentListChecked; // control when to check zone equipment list was input correctly

        Real64 tDBZoneAirTemp;      // current drybulb temperature of zone air, C
        Real64 tDBSystemAir;        // current drybulb temperature of primary supply air, C
        Real64 mDotSystemAir;       // current mass flow of primary supply air,kg/s
        Real64 cpZoneAir;           // current specific heat of zone air
        Real64 cpSystemAir;         // current specific heat of primary supply air
        Real64 qDotSystemAir;       // current heat transfer rate of primary supply air wrt zone, W
        Real64 qDotBeamCoolingMax;  // current beam cooling rate at maximum chilled water flow rate, W
        Real64 qDotBeamHeatingMax;  // current beam heating rate at maximum hot water flow rate, W
        Real64 qDotTotalDelivered;  // current combined heat transfer rate of primary supply air and beam, W
        Real64 qDotBeamCooling;     // current beam cooling rate, W
        Real64 qDotBeamHeating;     // current beam heating rate, W
        Real64 qDotZoneReq;         // current zone sensible requested load to setpoint, W
        Real64 qDotBeamReq;         // current load requested of beam, W
        Real64 qDotZoneToHeatSetPt; // current zone sensible load to heating setpoint, W
        Real64 qDotZoneToCoolSetPt; // current zone sensible load to cooling setpoint, W

    }; // HVACFourPipeBeam

} // namespace FourPipeBeam

struct FourPipeBeamData : BaseGlobalStruct
{

    ///// Note use of shared_ptr here is not a good pattern, not to be replicated without further discussion.
    Array1D<std::shared_ptr<FourPipeBeam::HVACFourPipeBeam>> FourPipeBeams; // dimension to number of machines

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        FourPipeBeams.clear();
    }
};

} // namespace EnergyPlus

#endif // HVACFourPipeBeam_hh_INCLUDED
