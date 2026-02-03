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

#ifndef WindowAC_hh_INCLUDED
#define WindowAC_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WindowAC {

    struct WindACData
    {
        // Members
        // input data
        std::string Name; // name of unit
        int UnitType;     // type of unit

        Sched::Schedule *availSched = nullptr;     // availability schedule
        Sched::Schedule *fanOpModeSched = nullptr; // fan operating mode schedule
        Sched::Schedule *fanAvailSched = nullptr;  // fan availability schedule
        Real64 MaxAirVolFlow;                      // m3/s
        Real64 MaxAirMassFlow;                     // kg/s
        Real64 OutAirVolFlow;                      // m3/s
        Real64 OutAirMassFlow;                     // kg/s
        int AirInNode;                             // inlet air node number
        int AirOutNode;                            // outlet air node number
        int OutsideAirNode;                        // outside air node number
        int AirReliefNode;                         // relief air node number
        int ReturnAirNode;                         // return air node number
        int MixedAirNode;                          // Mixed Air Node number
        std::string OAMixName;                     // name of outdoor air mixer
        std::string OAMixType;                     // type of outdoor air mixer
        int OAMixIndex;
        std::string FanName;   // name of fan
        HVAC::FanType fanType; // index to fan type
        int FanIndex;
        std::string DXCoilName; // name of cooling coil
        std::string DXCoilType; // type of cooling coil,Coil:DX:CoolingBypassFactorEmpirical or
        // 'CoilSystem:Cooling:DX:HeatExchangerAssisted'
        int DXCoilType_Num;                       // Numeric Equivalent for DXCoil Type
        int DXCoilIndex;                          // Index to DX cooling coil
        int DXCoilNumOfSpeeds;                    // number of speed levels for variable speed DX coil
        int CoilOutletNodeNum;                    // Outlet node number of DX cooling coil
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid; // mode of operation; 1=cycling fan, cycling compressor,
        // 2=continuous fan, cycling compressor
        HVAC::FanPlace fanPlace; // fan placement; 1=blow through, 2=draw through
        int MaxIterIndex1;
        int MaxIterIndex2;
        Real64 ConvergenceTol; // Convergence tolerance, fraction (ZoneLoad - Equip Output)/ZoneLoad
        // Calc data
        Real64 PartLoadFrac; // part load fraction for the unit
        bool EMSOverridePartLoadFrac;
        Real64 EMSValueForPartLoadFrac;
        // Report data
        Real64 TotCoolEnergyRate;         // total cooling output [W]
        Real64 TotCoolEnergy;             // total cooling output [J]
        Real64 SensCoolEnergyRate;        // sensible cooling output [W]
        Real64 SensCoolEnergy;            // sensible cooling output [J]
        Real64 LatCoolEnergyRate;         // sensible cooling output [W]
        Real64 LatCoolEnergy;             // sensible cooling output [J]
        Real64 ElecPower;                 // electricity consumed [W]
        Real64 ElecConsumption;           // electricity consumed [J]
        Real64 FanPartLoadRatio;          // fan part-load ratio for time step
        Real64 CompPartLoadRatio;         // compressor part-load ratio for time step
        std::string AvailManagerListName; // Name of an availability manager list object
        Avail::Status availStatus = Avail::Status::NoAction;
        int ZonePtr;         // pointer to a zone served by a Window AC unit
        int HVACSizingIndex; // index of a HVACSizing object for a Window AC unit
        bool FirstPass;      // detects first time through for resetting sizing data

        // Default Constructor
        WindACData()
            : UnitType(0), MaxAirVolFlow(0.0), MaxAirMassFlow(0.0), OutAirVolFlow(0.0), OutAirMassFlow(0.0), AirInNode(0), AirOutNode(0),
              OutsideAirNode(0), AirReliefNode(0), MixedAirNode(0), OAMixIndex(0), fanType(HVAC::FanType::Invalid), FanIndex(0), DXCoilType_Num(0),
              DXCoilIndex(0), DXCoilNumOfSpeeds(0), CoilOutletNodeNum(0), fanPlace(HVAC::FanPlace::Invalid), MaxIterIndex1(0), MaxIterIndex2(0),
              ConvergenceTol(0.0), PartLoadFrac(0.0), EMSOverridePartLoadFrac(false), EMSValueForPartLoadFrac(0.0), TotCoolEnergyRate(0.0),
              TotCoolEnergy(0.0), SensCoolEnergyRate(0.0), SensCoolEnergy(0.0), LatCoolEnergyRate(0.0), LatCoolEnergy(0.0), ElecPower(0.0),
              ElecConsumption(0.0), FanPartLoadRatio(0.0), CompPartLoadRatio(0.0), ZonePtr(0), HVACSizingIndex(0), FirstPass(true)
        {
        }
    };

    struct WindACNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        WindACNumericFieldData()
        {
        }
    };

    // Functions

    void SimWindowAC(EnergyPlusData &state,
                     std::string_view CompName,     // name of the window AC unit
                     int const ZoneNum,             // number of zone being served
                     bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                     Real64 &PowerMet,              // Sensible power supplied by window AC (W)
                     Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                     int &CompIndex                 // component index
    );

    void GetWindowAC(EnergyPlusData &state);

    void InitWindowAC(EnergyPlusData &state,
                      int const WindACNum,          // number of the current window AC unit being simulated
                      Real64 &QZnReq,               // zone load (modified as needed) (W)
                      int const ZoneNum,            // index to zone
                      bool const FirstHVACIteration // TRUE when first HVAC iteration
    );

    void SizeWindowAC(EnergyPlusData &state, int const WindACNum);

    void SimCyclingWindowAC(EnergyPlusData &state,
                            int const WindACNum,           // number of the current window AC unit being simulated
                            int const ZoneNum,             // number of zone being served !unused1208
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,              // Sensible power supplied (W)
                            Real64 const QZnReq,           // Sensible load to be met (W)
                            Real64 &LatOutputProvided      // Latent power supplied (kg/s), negative = dehumidification
    );

    void ReportWindowAC(EnergyPlusData &state, int const WindACNum); // number of the current AC unit being simulated

    void CalcWindowACOutput(EnergyPlusData &state,
                            int const WindACNum,           // Unit index in fan coil array
                            bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                            HVAC::FanOp const fanOp,       // operating mode: FanOp::Cycling | FanOp::Continuous
                            Real64 const PartLoadFrac,     // unit part load fraction
                            bool const HXUnitOn,           // Flag to toggle HX heat recovery as needed
                            Real64 &LoadMet                // load met by unit (watts)
    );

    void ControlCycWindACOutput(EnergyPlusData &state,
                                int const WindACNum,           // Unit index in fan coil array
                                bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                                HVAC::FanOp const fanOp,       // operating mode: FanOp::Cycling | FanOp::Continuous
                                Real64 const QZnReq,           // cooling output needed by zone [W]
                                Real64 &PartLoadFrac,          // unit part load fraction
                                bool &HXUnitOn                 // Used to control HX heat recovery as needed
    );

    bool getWindowACNodeNumber(EnergyPlusData &state, int const WindACNum);

    int GetWindowACZoneInletAirNode(EnergyPlusData &state, int const WindACNum);

    int GetWindowACOutAirNode(EnergyPlusData &state, int const WindACNum);

    int GetWindowACReturnAirNode(EnergyPlusData &state, int const WindACNum);

    int GetWindowACMixedAirNode(EnergyPlusData &state, int const WindACNum);

    int getWindowACIndex(EnergyPlusData &state, std::string_view CompName);

} // namespace WindowAC

struct WindowACData : BaseGlobalStruct
{

    // MODULE PARAMETER DEFINITIONS
    int const WindowAC_UnitType;
    std::string const cWindowAC_UnitType;
    Array1D_string const cWindowAC_UnitTypes;

    bool MyOneTimeFlag;
    bool ZoneEquipmentListChecked;

    int NumWindAC;
    int NumWindACCyc;
    Array1D_bool MySizeFlag;
    bool GetWindowACInputFlag; // First time, input is "gotten"
    bool CoolingLoad;          // defines a cooling load
    Array1D_bool CheckEquipName;
    // Object Data
    Array1D<WindowAC::WindACData> WindAC;
    Array1D<WindowAC::WindACNumericFieldData> WindACNumericFields; // holds window AC numeric input fields character field name

    Array1D_bool MyEnvrnFlag;  // one time initialization flag
    Array1D_bool MyZoneEqFlag; // used to set up zone equipment availability managers

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumWindAC = 0;
        this->NumWindACCyc = 0;
        this->GetWindowACInputFlag = true;
        this->CoolingLoad = false;
        this->MyOneTimeFlag = true;
        this->ZoneEquipmentListChecked = false;
        this->MySizeFlag.deallocate();
        this->CheckEquipName.deallocate();
        this->WindAC.deallocate();
        this->WindACNumericFields.deallocate();
        this->MyEnvrnFlag.deallocate();
        this->MyZoneEqFlag.deallocate();
    }

    // Default Constructor
    WindowACData()
        : WindowAC_UnitType(1), cWindowAC_UnitType("ZoneHVAC:WindowAirConditioner"), cWindowAC_UnitTypes(1, cWindowAC_UnitType), MyOneTimeFlag(true),
          ZoneEquipmentListChecked(false), NumWindAC(0), NumWindACCyc(0), GetWindowACInputFlag(true), CoolingLoad(false)
    {
    }
};

} // namespace EnergyPlus

#endif
