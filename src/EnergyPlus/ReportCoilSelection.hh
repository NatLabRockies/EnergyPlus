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

#ifndef ReportCoilSelection_hh_INCLUDED
#define ReportCoilSelection_hh_INCLUDED

// C++ Headers
#include <memory>
#include <string>
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

class CoilSelectionData
// data object, one for each unique coil in simulation
{

public: // methods
    // Constructor
    explicit CoilSelectionData(std::string const &coilName);

public:                                  // data
    std::string coilName_;               // user-defined coil name
    std::string coilObjName;             // coil object name, e.g., Coil:Cooling:Water, Coil:Heating:DX:SingleSpeed, etc.
    bool isCooling;                      // true if this coil is for cooling
    bool isHeating;                      // true if this coil is for heating
    std::string coilLocation;            // where is the coil located?, AirLoop or Zone
    std::string desDayNameAtSensPeak;    // the name of the design day that produced the sensible Ideal Loads peak
    std::string coilSensePeakHrMin;      // time stamp of the sensible coil peak during Ideal Loads Simulation
    std::string desDayNameAtTotalPeak;   // the name of the design day that produced the total Ideal Loads peak
    std::string coilTotalPeakHrMin;      // time stamp of the sensible coil peak during Ideal Loads Simulation
    std::string desDayNameAtAirFlowPeak; // the name of the design day that produced the air flow Ideal Loads peak
    std::string airPeakHrMin;            // time stamp of the airflow peak during Ideal Loads Simulation

    int coilNum;                       //  coil array index associated with WaterCoil(), DXCoil(), or HeatingCoil() array structures
    int airloopNum;                    // Coil index to AirLoop
    int oaControllerNum;               // Coil index to OAController
    int zoneEqNum;                     //  Coil index to CurZoneEqNum, for zone coils
    int oASysNum;                      // Coil index to OASysEqSizing
    std::vector<int> zoneNum;          // list of zone indexes associated with this coil
    std::vector<std::string> zoneName; // list of zone names associated with this coil, if any
    std::string typeHVACname;          // this coil is in what sort of HVAC system, parent object types
    std::string userNameforHVACsystem; // this coil is an HVAC system named by the user.
    int zoneHVACTypeNum;               // store type num if zoneHVAC
    int zoneHVACIndex;                 // store component index for zone HVAC
    int typeof_Coil; // type of coil, e.g., PlantEquipmentType::CoilWaterSimpleHeating, PlantEquipmentType::CoilWaterDetailedFlatCooling,
                     // PlantEquipmentType::CoilWaterCooling

    DataSizing::CoilSizingConcurrence coilSizingMethodConcurrence =
        DataSizing::CoilSizingConcurrence::NA;   // non-coincident, coincident, combination, n/a
    std::string coilSizingMethodConcurrenceName; // string name of sizing method for concurrence

    int coilSizingMethodCapacity; // 8=CoolingDesignCapacity, 9=HeatingDesignCapacity, 10=CapacityPerFloorArea, 11=FractionOfAutosizedCoolingCapacity,
                                  // 12=FractionOfAutosizedHeatingCapacity
    std::string coilSizingMethodCapacityName;

    int coilSizingMethodAirFlow; // choice of how to get system design air flow rates;
                                 //  2=SupplyAirFlowRate, 3=FlowPerFloorArea, 4=FractionOfAutosizedCoolingAirflow,
                                 //  5=FractionOfAutosizedHeatingAirflow, 6=FlowPerCoolingCapacity, 7=FlowPerHeatingCapacity
    std::string coilSizingMethodAirFlowName;

    bool isCoilSizingForTotalLoad; // Type of peak to size (cooling) coils on: True is TotalCoolingLoad, False is sensible
    std::string coilPeakLoadTypeToSizeOnName;

    // Real64 coilDesCapUser; // coil capacity original input value [W]; -999 means field not applicable to this coil
    bool capIsAutosized;        // true if the coil's capacity was autosized
    std::string coilCapAutoMsg; // Yes if user cap was autosized, No if hard value entered

    bool volFlowIsAutosized;          // true if the coil's flow was autosized
    std::string coilVolFlowAutoMsg;   // Yes if user flow was autosized, no if hard value;
    Real64 coilWaterFlowUser;         // coil water flow original input value [m3/s]; -999 means field not applicable to this coil
    std::string coilWaterFlowAutoMsg; // Yes if user water flow was autosized

    bool oaPretreated;             // Was the outside air pretreated prior to inlet of coil [True or False]
    std::string coilOAPretreatMsg; // Yes if the entering air OA was pretreated before mixing, No otherwise

    bool isSupplementalHeater; // true if heating coil is a supplemental heater.
    Real64 coilTotCapFinal;
    Real64 coilSensCapFinal;
    Real64 coilRefAirVolFlowFinal;
    Real64 coilRefWaterVolFlowFinal;

    // Values from time of Ideal Load Sizing Peak
    Real64 coilTotCapAtPeak;     // coil total capacity at peak [W]
    Real64 coilSensCapAtPeak;    // coil sensible capacity at peak [W]
    Real64 coilDesMassFlow;      // coil design air mass flow rate [kg/s]
    Real64 coilDesVolFlow;       // coil design air volume flow rate [m3/s]
    Real64 coilDesEntTemp;       // coil entering dry bulb at peak [C]
    Real64 coilDesEntWetBulb;    // coil design entering wet bulb [C]
    Real64 coilDesEntHumRat;     // coil entering humidity ratio at peak [kgWater/kgDryAir]
    Real64 coilDesEntEnth;       // coil entering enthalpy [J/kg-C]
    Real64 coilDesLvgTemp;       //  coil leaving dry bulb at peak [C]
    Real64 coilDesLvgWetBulb;    // coil design leaving wet bulb [C]
    Real64 coilDesLvgHumRat;     // coil leaving humidity ratio at peak [kgWater/kgDryAir]
    Real64 coilDesLvgEnth;       // coil leaving enthalpy [J/kg-C]
    Real64 coilDesWaterMassFlow; // coil design water mass flow rate [m3/s]
    Real64 coilDesWaterEntTemp;  // coil design entering water temperature[C]
    Real64 coilDesWaterLvgTemp;  // coil design leaving water temperature [C]
    Real64 coilDesWaterTempDiff; // coil plant design delta T [ delta C]

    int pltSizNum;                         // index to PlantSizData() array if this coil attached to a plant
    int waterLoopNum;                      // Plant loop index if this (water) coil attached to a plant
    std::string plantLoopName;             // user name of the plant loop
    Real64 oaPeakTemp;                     // outside air dry bulb at ideal loads peak [C]
    Real64 oaPeakHumRat;                   // outside air humidity ratio at ideal loads peak [kgWater/kgDryAir]
    Real64 oaPeakWetBulb;                  // outside air wet bulb at peak [C]
    Real64 oaPeakVolFlow;                  // outside air volume flow rate at peak [m3/s]
    Real64 oaPeakVolFrac;                  // fraction of outside air volume flow rate at peak
    Real64 oaDoaTemp;                      // DOA unit design supply air dry bulb [C]
    Real64 oaDoaHumRat;                    // DOA unit design supply air humidity ratio [kgWater/kgDryAir]
    Real64 raPeakTemp;                     // return air dry bulb at peak [C]
    Real64 raPeakHumRat;                   // return air humidity ratio at peak [kgWater/kgDryAir]
    Real64 rmPeakTemp;                     // room air dry bulb (setpoint) at peak [C]
    Real64 rmPeakHumRat;                   // room air humidity ratio (setpoint) at peak [kgWater/kgDryAir]
    Real64 rmPeakRelHum;                   // room air relative humidity (setpoint) at peak [%]
    Real64 rmSensibleAtPeak;               // space sensible zone load at peak [W]
    Real64 rmLatentAtPeak;                 // space latent zone load at ideal loads peak [W]
    Real64 coilIdealSizCapOverSimPeakCap;  // capacity at rated conditions subtracted by capacity at simulation peak, zero if under sized [W]
    Real64 coilIdealSizCapUnderSimPeakCap; // capacity at simulation peak subtracted by capacity at rated conditions, zero if oversized [W]
    Real64 reheatLoadMult;                 // reheat coil multiplier due to over/undersizing of coil airflow
    Real64 minRatio;                       // Flow/capacity ratio too low so reducing capacity by ratio
    Real64 maxRatio;                       // Flow/capacity ratio too high so increasing capacity by ratio
    Real64 cpMoistAir;                     // specific heat capacity of moist air through coil [J/kg-C]
    Real64 cpDryAir;                       // specific heat capacity of dry air (at elevation) [J/kg-C]
    Real64 rhoStandAir;                    // density of dry air at a standard temp and at eleveation [kg/m3]
    Real64 rhoFluid;                       // density of coil plant fluid [kg/m3]
    Real64 cpFluid;                        // specific heat capacity of coil plant fluid [J/kg-K]
    Real64 coilCapFTIdealPeak; // coil capacity multiplier at Ideal Peak conditions, func of temperature only,   to the input rating point (usually
                               // for a DX coil) [W]
    Real64 coilRatedTotCap;    // rated coil total capacity [W]
    Real64 coilRatedSensCap;   // rated coil sensible capacity [W]
    Real64 ratedAirMassFlow;   // rated coil design air mass flow rate [m3/s]
    Real64 ratedCoilInDb;      // rated coil inlet air dry bulb [C]
    Real64 ratedCoilInWb;      // rated coil inlet air wet bulb [C]
    Real64 ratedCoilInHumRat;  // rated coil inlet air humidity ratio [kgWater/kgDryAir]
    Real64 ratedCoilInEnth;    // rated coil inlet air enthalpy [J/kg-C]
    Real64 ratedCoilOutDb;     // rated coil outlet air dry bulb [C]
    Real64 ratedCoilOutWb;     // rated coil outlet air wet bulb [C]
    Real64 ratedCoilOutHumRat; // rated coil outlet air humidity ratio, [kgWater/kgDryAir]
    Real64 ratedCoilOutEnth;   // rated coil outlet air enthalpy [J/kg-C]
    Real64 ratedCoilEff;       // rated coil effectiveness
    Real64 ratedCoilBpFactor;  // rated coil bypass factor
    Real64 ratedCoilAppDewPt;  // rated coil apparatus dew point [C]
    Real64 ratedCoilOadbRef;   // rated DX coil outside air dry bulb reference [C]
    Real64 ratedCoilOawbRef;   // rated DX coil outside air wet bulb reference [C]

    std::string fanAssociatedWithCoilName;      // name of fan found to be associated with this coil
    std::string fanTypeName;                    // type of fan found to be associated with this coil
    HVAC::FanType supFanType;                   // indicates which type of fan model for supply fan, legacy or new OO
    int supFanNum = 0;                          // index pointing to this fan in legacy fan data structure, 1-based struct array
    Real64 fanSizeMaxAirVolumeFlow;             // the size of the fan in terms of volume flow rate [m3/s]
    Real64 fanSizeMaxAirMassFlow;               // the size of the fan in terms of mass flow rate [kg/s]
    Real64 fanHeatGainIdealPeak;                // Fan heat gain to air during Ideal loads peak sizing [W]
    Real64 coilAndFanNetTotalCapacityIdealPeak; // coil net total capacity including fan heat gain for ideal loads peak sizing [W]

    // static plant info
    Real64 plantDesMaxMassFlowRate; // this coil's plant loop overall design flow rate [kg/s]
    Real64 plantDesRetTemp;         // this coil's plant loop design return temperature
    Real64 plantDesSupTemp;         // this coil's plant loop design supply temperature
    Real64 plantDesDeltaTemp;       // this coil's plant loop design temperature difference (delta C)
    Real64 plantDesCapacity;        // this coil's plant loop capacity [W]
    Real64 coilCapPrcntPlantCap;    // this coil's capacity as a percentage of the overall loop's capacity
    Real64 coilFlowPrcntPlantFlow;  // this coil's design flow rate as a percentage the overall loop's design flow rate
    // misc
    Real64 coilUA; // U-value times Area, coil sizing result for UA [ ]

}; // end class CoilSelectionData

class ReportCoilSelection

{
public: // Creation
    ReportCoilSelection() : numCoilsReported_(0)
    {
    }

public: // methods
    void finishCoilSummaryReportTable(EnergyPlusData &state);

    void setCoilFinalSizes(EnergyPlusData &state,
                           std::string const &coilName,    // user-defined name of the coil
                           std::string const &coilObjName, //  coil object name, e.g., Coil:Cooling:Water
                           Real64 const totGrossCap,       // total capacity [W]
                           Real64 const sensGrossCap,      // sensible capacity [W]
                           Real64 const airFlowRate,       // design or reference or rated air flow rate [m3/s]
                           Real64 const waterFlowRate      // design or reference or rated water flow rate [m3/s]
    );

    void setRatedCoilConditions(EnergyPlusData &state,
                                std::string const &coilName,     // ! user-defined name of the coil
                                std::string const &coilObjName,  //  coil object name, e.g., Coil:Cooling:Water
                                Real64 const RatedCoilTotCap,    // ! rated coil total capacity [W]
                                Real64 const RatedCoilSensCap,   // rated coil sensible capacity [W]
                                Real64 const RatedAirMassFlow,   // rated coil design air mass flow rate [m3/s]
                                Real64 const RatedCoilInDb,      // rated coil inlet air dry bulb at time of peak [C]
                                Real64 const RatedCoilInHumRat,  // rated coil inlet air humidity ratio [kgWater/kgDryAir]
                                Real64 const RatedCoilInWb,      // rated coil inlet air wet bulb [C]
                                Real64 const RatedCoilOutDb,     // rated coil outlet air dry bulb [C]
                                Real64 const RatedCoilOutHumRat, // rated coil outlet air humidity ratio, [kgWater/kgDryAir]
                                Real64 const RatedCoilOutWb,     // rated coil outlet air wet bulb [C]

                                Real64 const RatedCoilOadbRef,  // rated DX coil outside air dry bulb reference [C]
                                Real64 const RatedCoilOawbRef,  // rated DX coil outside air wet bulb reference [C]
                                Real64 const RatedCoilBpFactor, // rated coil bypass factor
                                Real64 const RatedCoilEff       // rated coil effectiveness
    );

    void setCoilAirFlow(EnergyPlusData &state,
                        std::string const &coilName, // user-defined name of the coil
                        std::string const &coilType, // idf input object class name of coil
                        Real64 const airVdot,        // air flow rate in m3/s
                        bool const isAutoSized       // true if air flow was autosized
    );

    void setCoilWaterFlowNodeNums(EnergyPlusData &state,
                                  std::string const &coilName, // user-defined name of the coil
                                  std::string const &coilType, // idf input object class name of coil
                                  Real64 const waterVdot,      // water flow rate in m3/s
                                  bool const isAutoSized,      // true if water flow was autosized
                                  int const inletNodeNum,      // coil chw inlet node num
                                  int const outletNodeNum,     // coil chw outlet node num
                                  int const DataWaterLoopNum   // plant loop structure index
    );

    void setCoilWaterFlowPltSizNum(EnergyPlusData &state,
                                   std::string const &coilName, // user-defined name of the coil
                                   std::string const &coilType, // idf input object class name of coil
                                   Real64 const waterVdot,      // water flow rate in m3/s
                                   bool const isAutoSized,      // true if water flow was autosized
                                   int const DataPltSizNum,     // plant sizing structure index
                                   int const DataWaterLoopNum   // plant loop structure index
    );

    void setCoilEntAirTemp(EnergyPlusData &state,
                           std::string const &coilName,    // user-defined name of the coil
                           std::string const &coilType,    // idf input object class name of coil
                           Real64 const entAirDryBulbTemp, // ideal loads sizing result for air entering coil drybulb temp (C)
                           int const curSysNum,            // airloop system number index, if non zero
                           int const curZoneEqNum          // zone equipment list index, if non-zero
    );

    void setCoilEntAirHumRat(EnergyPlusData &state,
                             std::string const &coilName, // user-defined name of the coil
                             std::string const &coilType, // idf input object class name of coil
                             Real64 const entAirHumRat);

    void setCoilEntWaterTemp(EnergyPlusData &state,
                             std::string const &coilName, // user-defined name of the coil
                             std::string const &coilType, // idf input object class name of coil
                             Real64 const entWaterTemp    // degree C
    );

    void setCoilLvgWaterTemp(EnergyPlusData &state,
                             std::string const &coilName, // user-defined name of the coil
                             std::string const &coilType, // idf input object class name of coil
                             Real64 const lvgWaterTemp    // degree C
    );

    void setCoilWaterDeltaT(EnergyPlusData &state,
                            std::string const &coilName, // user-defined name of the coil
                            std::string const &coilType, // idf input object class name of coil
                            Real64 const CoilWaterDeltaT // degree C temperature difference used to size coil
    );

    void setCoilLvgAirTemp(EnergyPlusData &state,
                           std::string const &coilName,   // user-defined name of the coil
                           std::string const &coilType,   // idf input object class name of coil
                           Real64 const lvgAirDryBulbTemp // air temperature leaving coil {C}
    );

    void setCoilLvgAirHumRat(EnergyPlusData &state,
                             std::string const &coilName, // user-defined name of the coil
                             std::string const &coilType, // idf input object class name of coil
                             Real64 const lvgAirHumRat    //
    );

    void setCoilCoolingCapacity(EnergyPlusData &state,
                                std::string const &coilName,    // user-defined name of the coil
                                std::string const &coilType,    // idf input object class name of coil
                                Real64 const totalCoolingCap,   // {W} coil cooling capacity
                                bool const isAutoSize,          // true if value was autosized
                                int const curSysNum,            // airloop system number index, if non zero
                                int const curZoneEqNum,         // zone equipment list index, if non-zero
                                int const curOASysNum,          // OA system equipment list index, if non-zero
                                Real64 const fanCoolLoad,       // {W} fan load used in ideal loads coil sizing
                                Real64 const coilCapFunTempFac, // {W} curve result for modification factor for capacity as a function of temperature
                                Real64 const DXFlowPerCapMinRatio, // non dimensional ratio, capacity adjustment ratio min
                                Real64 const DXFlowPerCapMaxRatio  // non dimensional ratio, capacity adjustment ratio max
    );

    void setCoilHeatingCapacity(EnergyPlusData &state,
                                std::string const &coilName,    // user-defined name of the coil
                                std::string const &coilType,    // idf input object class name of coil
                                Real64 const totalHeatingCap,   // {W} coil Heating capacity
                                bool const isAutoSize,          // true if value was autosized
                                int const curSysNum,            // airloop system number index, if non zero
                                int const curZoneEqNum,         // zone equipment list index, if non-zero
                                int const curOASysNum,          // OA system equipment list index, if non-zero
                                Real64 const fanHeatGain,       // {W} fan load used in ideal loads coil sizing
                                Real64 const coilCapFunTempFac, // {W} curve result for modification factor for capacity as a function of temperature
                                Real64 const DXFlowPerCapMinRatio, // non dimensional ratio, capacity adjustment ratio min
                                Real64 const DXFlowPerCapMaxRatio  // non dimensional ratio, capacity adjustment ratio max
    );

    void setCoilWaterCoolingCapacity(EnergyPlusData &state,
                                     std::string const &coilName,  // user-defined name of the coil
                                     std::string const &coilType,  // idf input object class name of coil
                                     Real64 const totalCoolingCap, // {W} coil cooling capacity
                                     bool const isAutoSize,        // true if value was autosized
                                     int const inletNodeNum,       // coil chw inlet node num
                                     int const outletNodeNum,      // coil chw outlet node num
                                     int const dataWaterLoopNum    // plant loop structure index
    );

    void setCoilWaterHeaterCapacityNodeNums(EnergyPlusData &state,
                                            std::string const &coilName,  // user-defined name of the coil
                                            std::string const &coilType,  // idf input object class name of coil
                                            Real64 const totalHeatingCap, // {W} coil Heating capacity
                                            bool const isAutoSize,        // true if value was autosized
                                            int const inletNodeNum,       // coil chw inlet node num
                                            int const outletNodeNum,      // coil chw outlet node num
                                            int const dataWaterLoopNum    // plant loop structure index
    );

    void setCoilWaterHeaterCapacityPltSizNum(EnergyPlusData &state,
                                             std::string const &coilName,  // user-defined name of the coil
                                             std::string const &coilType,  // idf input object class name of coil
                                             Real64 const totalHeatingCap, // {W} coil Heating capacity
                                             bool const isAutoSize,        // true if value was autosized
                                             int const dataPltSizNum,      // plant sizing structure index
                                             int const dataWaterLoopNum    // plant loop structure index
    );

    void setCoilUA(EnergyPlusData &state,
                   std::string const &coilName,            // user-defined name of the coil
                   std::string const &coilType,            // idf input object class name of coil
                   Real64 const UAvalue,                   // [W/k] UA value for coil,
                   Real64 const dataCapacityUsedForSizing, // [W] sizing global
                   bool const isAutoSize,                  // true if value was autosized
                   int const curSysNum,                    // airloop system number index, if non zero
                   int const curZoneEqNum                  // zone equipment list index, if non-zero
    );

    void setCoilReheatMultiplier(EnergyPlusData &state,
                                 std::string const &coilName, // user-defined name of the coil
                                 std::string const &coilType, // idf input object class name of coil
                                 Real64 const multiplierReheatLoad);

    void setCoilSupplyFanInfo(EnergyPlusData &state,
                              std::string const &coilName, // user-defined name of the coil
                              std::string const &coilType, // idf input object class name of coil
                              std::string const &fanName,
                              HVAC::FanType fanType,
                              int fanIndex);

    void setCoilEqNum(EnergyPlusData &state,
                      std::string const &coilName,
                      std::string const &coilType,
                      int const curSysNum,
                      int const curOASysNum,
                      int const curZoneEqNum);

    static std::string getTimeText(EnergyPlusData &state, int const timeStepAtPeak);

    bool isCompTypeFan(std::string const &compType // string component type, input object class name
    );

    bool isCompTypeCoil(std::string const &compType // string component type, input object class name
    );

    void setZoneLatentLoadCoolingIdealPeak(int const zoneIndex, Real64 const zoneCoolingLatentLoad);

    void setZoneLatentLoadHeatingIdealPeak(int const zoneIndex, Real64 const zoneHeatingLatentLoad);

private: // methods
    void doAirLoopSetup(EnergyPlusData &state, int const coilVecIndex);

    void doZoneEqSetup(EnergyPlusData &state, int const coilVecIndex);

    void doFinalProcessingOfCoilData(EnergyPlusData &state);

    void writeCoilSelectionOutput(EnergyPlusData &state);

    void writeCoilSelectionOutput2(EnergyPlusData &state);

    int getIndexForOrCreateDataObjFromCoilName(EnergyPlusData &state,
                                               std::string const &coilName, // user-defined name of the coil
                                               std::string const &coilType  // idf input object class name of coil
    );

    void associateZoneCoilWithParent(EnergyPlusData &state, std::unique_ptr<CoilSelectionData> &c);

public: // data
    int numCoilsReported_;
    std::vector<std::unique_ptr<CoilSelectionData>> coilSelectionDataObjs;

}; // end ReportCoilSelection class

void createCoilSelectionReportObj(EnergyPlusData &state);

struct ReportCoilSelectionData : BaseGlobalStruct
{

    std::unique_ptr<ReportCoilSelection> coilSelectionReportObj;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->coilSelectionReportObj.release();
    }
};

} // namespace EnergyPlus

#endif
