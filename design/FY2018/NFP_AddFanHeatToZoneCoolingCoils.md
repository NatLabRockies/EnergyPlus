NFP – Account for Fan Heat when sizing zone cooling coils

FSEC, R. Raustad, May 18, 2018

Initial NFP: Accepted May 30, 2018.

Final NFP updated May 30, 2018. Accepted:

Design document included in description of approach and implementation. Accepted:

#Justification for Feature Update:
Cooling coils are required to deliver sufficient capacity to meet the zone load, the outdoor air load and fan heat. Currently only the zone load and outdoor air load (based on a mixed air temperature calculation at peak load) are used to size zone cooling coils. Recently, fan heat was included in air loop cooling coil sizing and this same methodology is needed for zone cooling coils.

##Email Communication:
 - As for keeping a different variable for the fan index based on fan type, this is indeed because of the disjointed index nature of the fans, helping to ensure that developers are more cautious and it makes it more difficult to grab the wrong index. I think the idea is that as the older fan models are refactored, they will become a part of the vector and the IF blocks and multiple variables will fall away.
 - As for the implementation you laid out, it looks reasonable to me.

##Conference Call Conclusions:
NA

###Other Conference Call Topics (not in scope of current proposal):
N/A

#Overview:

#Approach:

The current method used to add fan heat to air loop cooling coils is to:

 1) Determine the fan type and index then store that information in the PrimaryAirSystem array.

SimAirServingZones::InitAirLoops (within InitAirLoopsOneTimeFlag IF block)

    if (supFanModelType == structArrayLegacyFanModels) {
        PrimaryAirSystem(AirLoopNum).SupFanNum = SupFanIndex;
        PrimaryAirSystem(AirLoopNum).supFanModelType = structArrayLegacyFanModels;
    } else if (supFanModelType == objectVectorOOFanSystemModel) {
        PrimaryAirSystem(AirLoopNum).supFanVecIndex = SupFanIndex;
        PrimaryAirSystem(AirLoopNum).supFanModelType = objectVectorOOFanSystemModel;
    }

    if (retFanModelType == structArrayLegacyFanModels) {
        PrimaryAirSystem(AirLoopNum).retFanModelType = structArrayLegacyFanModels;
        PrimaryAirSystem(AirLoopNum).RetFanNum = RetFanIndex;
    } else if (retFanModelType == objectVectorOOFanSystemModel) {
        PrimaryAirSystem(AirLoopNum).retFanModelType = objectVectorOOFanSystemModel;
        PrimaryAirSystem(AirLoopNum).retFanVecIndex = RetFanIndex;
    }

HVACUnitarySystem::SizeUnitarySystem (before sizing occurs)

    if (CurSysNum > 0 && CurOASysNum == 0 && UnitarySystem(UnitarySysNum).FanExists) {
        if (UnitarySystem(UnitarySysNum).FanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
            PrimaryAirSystem(CurSysNum).supFanVecIndex = UnitarySystem(UnitarySysNum).FanIndex;
            PrimaryAirSystem(CurSysNum).supFanModelType = DataAirSystems::objectVectorOOFanSystemModel;
        } else {
            PrimaryAirSystem(CurSysNum).SupFanNum = UnitarySystem(UnitarySysNum).FanIndex;
            PrimaryAirSystem(CurSysNum).supFanModelType = DataAirSystems::structArrayLegacyFanModels;
        }
    }

HVACDXSystem::InitDXCoolingSystem (

        if (!DXCoolingSystem(DXSystemNum).VSCoilFanInfoSet && AirLoopNum > 0) {
            if (DXCoolingSystem(DXSystemNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {

                switch (DataAirSystems::PrimaryAirSystem(AirLoopNum).supFanModelType) {
                case DataAirSystems::structArrayLegacyFanModels: {
                    int SupFanNum = DataAirSystems::PrimaryAirSystem(AirLoopNum).SupFanNum;
                    if (SupFanNum > 0) {
                        coilSelectionReportObj->setCoilSupplyFanInfo(
                            DXCoolingSystem(DXSystemNum).CoolingCoilName, DXCoolingSystem(DXSystemNum).CoolingCoilType,
                            Fans::Fan(DataAirSystems::PrimaryAirSystem(AirLoopNum).SupFanNum).FanName, DataAirSystems::structArrayLegacyFanModels,
                            DataAirSystems::PrimaryAirSystem(AirLoopNum).SupFanNum);
                    }

                    break;
                }
                case DataAirSystems::objectVectorOOFanSystemModel: {
                    if (DataAirSystems::PrimaryAirSystem(AirLoopNum).supFanVecIndex >= 0) {
                        coilSelectionReportObj->setCoilSupplyFanInfo(
                            DXCoolingSystem(DXSystemNum).CoolingCoilName, DXCoolingSystem(DXSystemNum).CoolingCoilType,
                            HVACFan::fanObjs[DataAirSystems::PrimaryAirSystem(AirLoopNum).supFanVecIndex]->name,
                            DataAirSystems::objectVectorOOFanSystemModel, DataAirSystems::PrimaryAirSystem(AirLoopNum).supFanVecIndex);
                    }
                    break;
                }
                case DataAirSystems::fanModelTypeNotYetSet: {
                    // do nothing
                    break;
                }
                }
                DXCoolingSystem(DXSystemNum).VSCoilFanInfoSet = true;
            }
        }

where:

    int supFanVecIndex; // index in fan object vector for supply fan when model type is objectVectorOOFanSystemModel, zero-based index
    int SupFanNum;      // index of the supply fan in the Fan data structure when model type is structArrayLegacyFanModels
    fanModelTypeEnum supFanModelType; // indicates which type of fan model to call for supply fan, legacy or new OO

Note that:

 - Only main branch fans or fans in AirloopHVAC:UnitarySystem or CoilSystem:Cooling:DX set fan heat used for sizing cooling coils
 -  two different variables are used to store the PrimaryAirSystem fan index (why? simple clarity that system fan has 0 based index?)

 2) This information is used during cooling coil sizing to adjust coil size to include fan heat

ReportSizingManager::RequestSizing

    } else if (SizingType == CoolingCapacitySizing) { // for air loops

      <snip> ... air flow and inlet/outlet conditions are initialized

      SupFanNum = PrimaryAirSystem(CurSysNum).SupFanNum;
      RetFanNum = PrimaryAirSystem(CurSysNum).RetFanNum;
      switch (PrimaryAirSystem(CurSysNum).supFanModelType) {
          case DataAirSystems::structArrayLegacyFanModels: {
              FanCoolLoad = FanDesHeatGain(PrimaryAirSystem(CurSysNum).SupFanNum, DesVolFlow);
              if (coilSelectionReportObj->isCompTypeCoil(CompType) && (SupFanNum > 0)) {
                  coilSelectionReportObj->setCoilSupplyFanInfo(
                      CompName, CompType, Fans::Fan(PrimaryAirSystem(CurSysNum).SupFanNum).FanName,
                      DataAirSystems::structArrayLegacyFanModels, PrimaryAirSystem(CurSysNum).SupFanNum);
              }
              break;
          }
          case DataAirSystems::objectVectorOOFanSystemModel: {
              FanCoolLoad = HVACFan::fanObjs[PrimaryAirSystem(CurSysNum).supFanVecIndex]->getFanDesignHeatGain(DesVolFlow);
              if (coilSelectionReportObj->isCompTypeCoil(CompType) && (PrimaryAirSystem(CurSysNum).supFanVecIndex >= 0)) {
                  coilSelectionReportObj->setCoilSupplyFanInfo(
                      CompName, CompType, HVACFan::fanObjs[PrimaryAirSystem(CurSysNum).supFanVecIndex]->name,
                      DataAirSystems::objectVectorOOFanSystemModel, PrimaryAirSystem(CurSysNum).supFanVecIndex);
              }
              break;
          }
          case DataAirSystems::fanModelTypeNotYetSet: {
              // do nothing
              break;
          }
          } // end switch

      switch (PrimaryAirSystem(CurSysNum).retFanModelType) {
          case DataAirSystems::structArrayLegacyFanModels: {
              FanCoolLoad += (1.0 - OutAirFrac) * FanDesHeatGain(PrimaryAirSystem(CurSysNum).RetFanNum, DesVolFlow);
              break;
          }
          case DataAirSystems::objectVectorOOFanSystemModel: {
              FanCoolLoad += (1.0 - OutAirFrac) *
                  HVACFan::fanObjs[PrimaryAirSystem(CurSysNum).retFanVecIndex]->getFanDesignHeatGain(DesVolFlow);
              break;
          }
          case DataAirSystems::fanModelTypeNotYetSet: {
              // do nothing
              break;
          }
          } // end switch

      PrimaryAirSystem(CurSysNum).FanDesCoolLoad = FanCoolLoad;
      PeakCoilLoad = max(0.0, (rhoair * DesVolFlow * (CoilInEnth - CoilOutEnth) + FanCoolLoad));

To implement this new feature the fan index and type need to either be:

 - stored in a common location available for use during sizing

    1) ZoneEquipList(CurZoneEqNum).FanIndex = EquipmentType().FanIndex

    2) ZoneEquipList(CurZoneEqNum).FanEnumType = EquipmentType().fanEnumType

 - passed to the sizing routine via globals

    1) DataFanIndexNum = EquipmentType().FanIndex

    2) DataFanEnumType = EquipmentType().fanEnumType


It may be better to use a similar approach as is done with air loops where information is stored in a common array. If this information would be used by other functions then this method is preferred. On the other hand, this information would only be used during sizing and data globals would provide a quick implementation.

Regardless of which method is used, all zone cooling equipment models will need to be updated with, hopefully, 2 lines of code in the sizing routine.

### Implementation

    } else if (SizingType == CoolingCapacitySizing) { // for zone equipment

    <<< similar to above >>>
    FanCoolLoad = HVACFan::fanObjs[Index]->getFanDesignHeatGain(DesVolFlow);

    <snip> typical sizing calcuations finishing with:
    AutosizeDes = PeakCoilLoad;

    and then:
    AutosizeDes += FanCoolLoad;


###Testing/Validation/Data Source(s):
The new feature will be compared against exiting model.

##IO Ref (draft):
No changes.

###Proposed Report Variables:
N/A

###Proposed additions to Meters:
N/A

###EngRef (draft):
Equation changes to include fan heat in capacity calculation.

###Example File and Transition changes:
No changes.

###Other documents:
None.
