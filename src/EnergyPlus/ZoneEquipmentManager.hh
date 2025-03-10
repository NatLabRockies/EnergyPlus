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

#ifndef ZoneEquipmentManager_hh_INCLUDED
#define ZoneEquipmentManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ZoneEquipmentManager {

    struct SimulationOrder
    {
        std::string EquipTypeName = "";
        DataZoneEquipment::ZoneEquipType equipType = DataZoneEquipment::ZoneEquipType::Invalid;
        std::string EquipName = "";
        int EquipPtr = 0;
        int CoolingPriority = 0;
        int HeatingPriority = 0;
    };

    void ManageZoneEquipment(EnergyPlusData &state,
                             bool FirstHVACIteration,
                             bool &SimZone, // Set to false at the end of the routine
                             bool &SimAir   // Eventually set to true via SimZoneEquipment if AirLoop must be resimulated
    );

    void GetZoneEquipment(EnergyPlusData &state);

    void InitZoneEquipment(EnergyPlusData &state, bool FirstHVACIteration); // unused 1208

    void sizeZoneSpaceEquipmentPart1(EnergyPlusData &state,
                                     DataZoneEquipment::EquipConfiguration &ZoneEquipConfig,
                                     DataSizing::ZoneSizingData &zsCalcSizing,
                                     DataZoneEnergyDemands::ZoneSystemSensibleDemand &zsEnergyDemand,
                                     DataZoneEnergyDemands::ZoneSystemMoistureDemand &zsMoistureDemand,
                                     DataHeatBalance::ZoneData const &zoneOrSpace,
                                     int zoneNum,
                                     int spaceNum = 0);

    void sizeZoneSpaceEquipmentPart2(EnergyPlusData &state,
                                     DataZoneEquipment::EquipConfiguration &zoneEquipConfig,
                                     DataSizing::ZoneSizingData &zsCalcSizing,
                                     int zoneNum,
                                     int spaceNum = 0);

    void SizeZoneEquipment(EnergyPlusData &state);

    void SetUpZoneSizingArrays(EnergyPlusData &state);

    void calcSizingOA(EnergyPlusData &state,
                      DataSizing::ZoneSizingData &zsFinalSizing,
                      DataSizing::ZoneSizingData &zsCalcFinalSizing,
                      bool &dsoaError,
                      bool &ErrorsFound,
                      int const zoneNum,
                      int const spaceNum = 0);

    void fillZoneSizingFromInput(EnergyPlusData &state,
                                 DataSizing::ZoneSizingInputData const &zoneSizingInput,
                                 Array2D<DataSizing::ZoneSizingData> &zsSizing,
                                 Array2D<DataSizing::ZoneSizingData> &zsCalcSizing,
                                 DataSizing::ZoneSizingData &zsFinalSizing,
                                 DataSizing::ZoneSizingData &zsCalcFinalSizing,
                                 std::string_view const zoneOrSpaceName,
                                 int const zoneOrSpaceNum);

    void RezeroZoneSizingArrays(EnergyPlusData &state);

    void UpdateZoneSizing(EnergyPlusData &state, Constant::CallIndicator CallIndicator);

    void updateZoneSizingBeginDay(EnergyPlusData const &state, DataSizing::ZoneSizingData &zsCalcSizing);

    void updateZoneSizingDuringDay(DataSizing::ZoneSizingData &zsSizing,
                                   DataSizing::ZoneSizingData &zsCalcSizing,
                                   Real64 const tstatHi,
                                   Real64 const tstatLo,
                                   Real64 &sizTstatHi,
                                   Real64 &sizTstatLo,
                                   int const timeStepInDay,
                                   Real64 const fracTimeStepZone);

    void updateZoneSizingEndDayMovingAvg(DataSizing::ZoneSizingData &zsCalcSizing, int const numTimeStepsInAvg);

    void updateZoneSizingEndDay(DataSizing::ZoneSizingData &zsCalcSizing,
                                DataSizing::ZoneSizingData &zsCalcFinalSizing,
                                int const numTimeStepInDay,
                                DataSizing::DesDayWeathData const &desDayWeath,
                                Real64 const stdRhoAir);

    void updateZoneSizingEndZoneSizingCalc1(EnergyPlusData &state, int const zoneNum);

    void updateZoneSizingEndZoneSizingCalc2(EnergyPlusData &state, DataSizing::ZoneSizingData &zsCalcSizing);

    void writeZszSpsz(EnergyPlusData &state,
                      EnergyPlus::InputOutputFile &outputFile,
                      int const numSpacesOrZones,
                      EPVector<DataSizing::ZoneSizingData> const &zsCalcFinalSizing,
                      Array2D<DataSizing::ZoneSizingData> const &zsCalcSizing,
                      bool const forSpaces);

    std::string sizingPeakTimeStamp(EnergyPlusData const &state, int timeStepIndex);

    void updateZoneSizingEndZoneSizingCalc3(DataSizing::ZoneSizingData &zsCalcFinalSizing,
                                            Array2D<DataSizing::ZoneSizingData> &zsCalcSizing,
                                            bool &anyLatentLoad,
                                            int const zoneOrSpaceNum);

    void updateZoneSizingEndZoneSizingCalc4(DataSizing::ZoneSizingData &zsSizing, DataSizing::ZoneSizingData const &zsCalcSizing);

    void updateZoneSizingEndZoneSizingCalc5(DataSizing::ZoneSizingData &zsFinalSizing, DataSizing::ZoneSizingData const &zsCalcFinalSizing);

    void updateZoneSizingEndZoneSizingCalc6(DataSizing::ZoneSizingData &zsSizing,
                                            DataSizing::ZoneSizingData const &zsCalcSizing,
                                            int const numTimeStepsInDay);

    void updateZoneSizingEndZoneSizingCalc7(EnergyPlusData &state,
                                            DataSizing::ZoneSizingData &zsFinalSizing,
                                            DataSizing::ZoneSizingData &zsCalcFinalSizing,
                                            Array2D<DataSizing::ZoneSizingData> &zsSizing,
                                            Array2D<DataSizing::ZoneSizingData> &zsCalcSizing,
                                            int const zoneOrSpaceNum);

    void SimZoneEquipment(EnergyPlusData &state, bool FirstHVACIteration, bool &SimAir);

    void SetZoneEquipSimOrder(EnergyPlusData &state, int ControlledZoneNum);

    void InitSystemOutputRequired(EnergyPlusData &state, int ZoneNum, bool FirstHVACIteration, bool ResetSimOrder = false);

    void initOutputRequired(EnergyPlusData &state,
                            int const ZoneNum,
                            DataZoneEnergyDemands::ZoneSystemSensibleDemand &energy,
                            DataZoneEnergyDemands::ZoneSystemMoistureDemand &moisture,
                            bool const FirstHVACIteration,
                            bool const ResetSimOrder,
                            int spaceNum = 0);

    void DistributeSystemOutputRequired(EnergyPlusData &state, int ZoneNum, bool FirstHVACIteration);

    void distributeOutputRequired(EnergyPlusData &state,
                                  int const ZoneNum,
                                  DataZoneEnergyDemands::ZoneSystemSensibleDemand &energy,
                                  DataZoneEnergyDemands::ZoneSystemMoistureDemand &moisture);

    void updateSystemOutputRequired(EnergyPlusData &state,
                                    int const ZoneNum,
                                    Real64 const SysOutputProvided, // sensible output provided by zone equipment (W)
                                    Real64 const LatOutputProvided, // latent output provided by zone equipment (kg/s)
                                    DataZoneEnergyDemands::ZoneSystemSensibleDemand &energy,
                                    DataZoneEnergyDemands::ZoneSystemMoistureDemand &moisture,
                                    int const EquipPriorityNum = -1 // optional index in PrioritySimOrder for this update
    );

    void adjustSystemOutputRequired(Real64 const sensibleRatio, // sensible load adjustment
                                    Real64 const latentRatio,   // latent load adjustment
                                    DataZoneEnergyDemands::ZoneSystemSensibleDemand &energy,
                                    DataZoneEnergyDemands::ZoneSystemMoistureDemand &moisture,
                                    int const equipPriorityNum // index in PrioritySimOrder
    );

    void CalcZoneMassBalance(EnergyPlusData &state, bool FirstHVACIteration);

    void CalcZoneInfiltrationFlows(EnergyPlusData &state,
                                   int const ZoneNum,                      // current zone index
                                   Real64 const &ZoneReturnAirMassFlowRate // zone total zone return air mass flow rate
    );

    void CalcAirFlowSimple(EnergyPlusData &state,
                           int SysTimestepLoop = 0,                    // System time step index
                           bool AdjustZoneMixingFlowFlag = false,      // flags to adjust zone mxing mass flow rate
                           bool AdjustZoneInfiltrationFlowFlag = false // flags to djust zone infiltration air flow rate
    );

    void GetStandAloneERVNodes(EnergyPlusData &state, DataHeatBalance::ZoneAirBalanceData &thisZoneAirBalance);

    void CalcZoneMixingFlowRateOfReceivingZone(EnergyPlusData &state, int ZoneNum, Real64 &ZoneMixingAirMassFlowRate);

    void CalcZoneMixingFlowRateOfSourceZone(EnergyPlusData &state, int ZoneNum);

    void CalcZoneLeavingConditions(EnergyPlusData &state, bool FirstHVACIteration);

    void UpdateZoneEquipment(EnergyPlusData &state, bool &SimAir);

    void CalcDOASSupCondsForSizing(EnergyPlusData &state,
                                   Real64 OutDB,                        // outside air temperature [C]
                                   Real64 OutHR,                        // outside humidity ratio [kg Water / kg Dry Air]
                                   DataSizing::DOASControl DOASControl, // dedicated outside air control strategy
                                   Real64 DOASLowTemp,                  // DOAS low setpoint [C]
                                   Real64 DOASHighTemp,                 // DOAS high setpoint [C]
                                   Real64 W90H, // humidity ratio at DOAS high setpoint temperature and 90% relative humidity [kg Water / kg Dry Air]
                                   Real64 W90L, // humidity ratio at DOAS low setpoint temperature and 90% relative humidity [kg Water / kg Dry Air]
                                   Real64 &DOASSupTemp, // DOAS supply temperature [C]
                                   Real64 &DOASSupHR    // DOAS Supply Humidity ratio [kg Water / kg Dry Air]
    );

    void AutoCalcDOASControlStrategy(EnergyPlusData &state);

    void ReportInfiltrations(EnergyPlusData &state);

    void ReportZoneSizingDOASInputs(EnergyPlusData &state,
                                    std::string const &ZoneName,         // the name of the zone
                                    std::string const &DOASCtrlStrategy, // DOAS control strategy
                                    Real64 DOASLowTemp,                  // DOAS design low setpoint temperature [C]
                                    Real64 DOASHighTemp,                 // DOAS design high setpoint temperature [C]
                                    bool &headerAlreadyPrinted);

} // namespace ZoneEquipmentManager

struct ZoneEquipmentManagerData : BaseGlobalStruct
{

    Array1D<Real64> AvgData;    // scratch array for storing averaged data -- keep on state to avoid it being recreated
    int NumOfTimeStepInDay = 0; // number of zone time steps in a day
    bool GetZoneEquipmentInputFlag = true;
    bool SizeZoneEquipmentOneTimeFlag = true;

    Array1D<ZoneEquipmentManager::SimulationOrder> PrioritySimOrder;

    bool InitZoneEquipmentOneTimeFlag = true;
    bool InitZoneEquipmentEnvrnFlag = true;
    bool FirstPassZoneEquipFlag = true; // indicates first pass through zone equipment, used to reset selected ZoneEqSizing variables

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) ZoneEquipmentManagerData();
    }
};

} // namespace EnergyPlus

#endif
