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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/OutputReportPredefined.hh>

namespace EnergyPlus {

namespace OutputReportPredefined {

    // MODULE INFORMATION:
    //    AUTHOR         Jason Glazer of GARD Analytics, Inc.
    //    DATE WRITTEN   August 2006
    //    MODIFIED       na
    //    RE-ENGINEERED  na
    // PURPOSE OF THIS MODULE:
    //    Support the creation of predefined tabular output.
    // METHODOLOGY EMPLOYED:
    // REFERENCES:
    //    None.
    // OTHER NOTES:.
    // Using/Aliasing
    // Data
    // The following section initializes the predefined column heading variables
    // The variables get their value in AssignPredefined

    // Internal data structures to store information provided by calls

    void SetPredefinedTables(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates the structure of the predefined reports
        //   including the name and abbreviation of the report
        //   the subtables involved and the column headings.
        //   The variables defined for the columns are then
        //   used throughout the program to assign values
        //   to the subtables.

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // Climate Summary Report

        auto const &s = state.dataOutRptPredefined;

        s->pdrClim = newPreDefReport(state, "ClimaticDataSummary", "Clim", "Climatic Data Summary");

        s->pdstDesDay = newPreDefSubTable(state, s->pdrClim, "SizingPeriod:DesignDay");

        s->pdchDDmaxDB = newPreDefColumn(state, s->pdstDesDay, "Maximum Dry Bulb [C]");
        s->pdchDDrange = newPreDefColumn(state, s->pdstDesDay, "Daily Temperature Range [deltaC]");
        s->pdchDDhumid = newPreDefColumn(state, s->pdstDesDay, "Humidity Value");
        s->pdchDDhumTyp = newPreDefColumn(state, s->pdstDesDay, "Humidity Type");
        s->pdchDDwindSp = newPreDefColumn(state, s->pdstDesDay, "Wind Speed [m/s]");
        s->pdchDDwindDr = newPreDefColumn(state, s->pdstDesDay, "Wind Direction");

        s->pdstMonthlyPrec = newPreDefSubTable(state, s->pdrClim, "Monthly Precipitation Summary");

        s->pdchMonthlyTotalPrecInWeather = newPreDefColumn(state, s->pdstMonthlyPrec, "Monthly Total Precipitation from .epw [mm]");
        s->pdchMonthlyTotalHrRain = newPreDefColumn(state, s->pdstMonthlyPrec, "Monthly Total Hours of Rain from .epw");
        s->pdchMonthlyTotalPrecInSitePrec = newPreDefColumn(state, s->pdstMonthlyPrec, "Monthly Total Precipitation in Site:Precipitation [mm]");
        s->pdchMonthlyTotalIrrDep = newPreDefColumn(state, s->pdstMonthlyPrec, "Monthly Total Roof Irrigation Depth [mm]");
        s->pdchMonthlyTotalRainCol = newPreDefColumn(state, s->pdstMonthlyPrec, "Monthly Total Rain Collection Volume [m3]");

        s->pdstWthr = newPreDefSubTable(state, s->pdrClim, "Weather Statistics File");
        s->pdchWthrVal = newPreDefColumn(state, s->pdstWthr, "Value");

        // Envelope Report

        s->pdrEnvelope = newPreDefReport(state, "EnvelopeSummary", "Env", "Envelope Summary");

        s->pdstOpaque = newPreDefSubTable(state, s->pdrEnvelope, "Opaque Exterior");

        s->pdchOpCons = newPreDefColumn(state, s->pdstOpaque, "Construction");
        s->pdchOpZone = newPreDefColumn(state, s->pdstOpaque, "Zone");
        s->pdchOpSpace = newPreDefColumn(state, s->pdstOpaque, "Space");
        s->pdchOpRefl = newPreDefColumn(state, s->pdstOpaque, "Reflectance");
        s->pdchOpUfactFilm = newPreDefColumn(state, s->pdstOpaque, "U-Factor with Film [W/m2-K]");
        s->pdchOpUfactNoFilm = newPreDefColumn(state, s->pdstOpaque, "U-Factor no Film [W/m2-K]");
        s->pdchOpGrArea = newPreDefColumn(state, s->pdstOpaque, "Gross Area [m2]");
        s->pdchOpNetArea = newPreDefColumn(state, s->pdstOpaque, "Net Area [m2]");
        s->pdchOpAzimuth = newPreDefColumn(state, s->pdstOpaque, "Azimuth [deg]");
        s->pdchOpTilt = newPreDefColumn(state, s->pdstOpaque, "Tilt [deg]");
        s->pdchOpDir = newPreDefColumn(state, s->pdstOpaque, "Cardinal Direction");

        s->pdstIntOpaque = newPreDefSubTable(state, s->pdrEnvelope, "Opaque Interior");

        s->pdchIntOpCons = newPreDefColumn(state, s->pdstIntOpaque, "Construction");
        s->pdchIntOpZone = newPreDefColumn(state, s->pdstIntOpaque, "Zone");
        s->pdchIntOpSpace = newPreDefColumn(state, s->pdstIntOpaque, "Space");
        s->pdchIntOpAdjSurf = newPreDefColumn(state, s->pdstIntOpaque, "Adjacent Surface");
        s->pdchIntOpRefl = newPreDefColumn(state, s->pdstIntOpaque, "Reflectance");
        s->pdchIntOpUfactFilm = newPreDefColumn(state, s->pdstIntOpaque, "U-Factor with Film [W/m2-K]");
        s->pdchIntOpUfactNoFilm = newPreDefColumn(state, s->pdstIntOpaque, "U-Factor no Film [W/m2-K]");
        s->pdchIntOpGrArea = newPreDefColumn(state, s->pdstIntOpaque, "Gross Area [m2]");
        s->pdchIntOpNetArea = newPreDefColumn(state, s->pdstIntOpaque, "Net Area [m2]");
        s->pdchIntOpAzimuth = newPreDefColumn(state, s->pdstIntOpaque, "Azimuth [deg]");
        s->pdchIntOpTilt = newPreDefColumn(state, s->pdstIntOpaque, "Tilt [deg]");
        s->pdchIntOpDir = newPreDefColumn(state, s->pdstIntOpaque, "Cardinal Direction");

        s->pdstFen = newPreDefSubTable(state, s->pdrEnvelope, "Exterior Fenestration");

        s->pdchFenCons = newPreDefColumn(state, s->pdstFen, "Construction");
        s->pdchFenZone = newPreDefColumn(state, s->pdstFen, "Zone");
        s->pdchFenSpace = newPreDefColumn(state, s->pdstFen, "Space");
        s->pdchFenFrameDivName = newPreDefColumn(state, s->pdstFen, "Frame and Divider");
        s->pdchFenGlassAreaOf1 = newPreDefColumn(state, s->pdstFen, "Glass Area [m2]");
        s->pdchFenFrameAreaOf1 = newPreDefColumn(state, s->pdstFen, "Frame Area [m2]");
        s->pdchFenDividerAreaOf1 = newPreDefColumn(state, s->pdstFen, "Divider Area [m2]");
        s->pdchFenAreaOf1 = newPreDefColumn(state, s->pdstFen, "Area of One Opening [m2]");
        s->pdchFenArea = newPreDefColumn(state, s->pdstFen, "Area of Multiplied Openings [m2]");
        s->pdchFenUfact = newPreDefColumn(state, s->pdstFen, "Glass U-Factor [W/m2-K]");
        s->pdchFenSHGC = newPreDefColumn(state, s->pdstFen, "Glass SHGC");
        s->pdchFenVisTr = newPreDefColumn(state, s->pdstFen, "Glass Visible Transmittance");
        s->pdchFenFrameConductance = newPreDefColumn(state, s->pdstFen, "Frame Conductance [W/m2-K]");
        s->pdchFenDividerConductance = newPreDefColumn(state, s->pdstFen, "Divider Conductance [W/m2-K]");
        s->pdchFenAssemNfrcType = newPreDefColumn(state, s->pdstFen, "NFRC Product Type");
        s->pdchFenAssemUfact = newPreDefColumn(state, s->pdstFen, "Assembly U-Factor [W/m2-K]");
        s->pdchFenAssemSHGC = newPreDefColumn(state, s->pdstFen, "Assembly SHGC");
        s->pdchFenAssemVisTr = newPreDefColumn(state, s->pdstFen, "Assembly Visible Transmittance");
        s->pdchFenSwitchable = newPreDefColumn(state, s->pdstFen, "Shade Control");
        s->pdchFenParent = newPreDefColumn(state, s->pdstFen, "Parent Surface");
        s->pdchFenAzimuth = newPreDefColumn(state, s->pdstFen, "Azimuth [deg]");
        s->pdchFenTilt = newPreDefColumn(state, s->pdstFen, "Tilt [deg]");
        s->pdchFenDir = newPreDefColumn(state, s->pdstFen, "Cardinal Direction");

        s->pdstFenShd = newPreDefSubTable(state, s->pdrEnvelope, "Exterior Fenestration Shaded State");

        s->pdchFenShdFrameDiv = newPreDefColumn(state, s->pdstFenShd, "Frame and Divider");
        s->pdchFenShdUfact = newPreDefColumn(state, s->pdstFenShd, "Glass U-Factor [W/m2-K]");
        s->pdchFenShdSHGC = newPreDefColumn(state, s->pdstFenShd, "Glass SHGC");
        s->pdchFenShdVisTr = newPreDefColumn(state, s->pdstFenShd, "Glass Visible Transmittance");
        s->pdchFenShdAssemNfrcType = newPreDefColumn(state, s->pdstFenShd, "NFRC Product Type");
        s->pdchFenShdAssemUfact = newPreDefColumn(state, s->pdstFenShd, "Assembly U-Factor [W/m2-K]");
        s->pdchFenShdAssemSHGC = newPreDefColumn(state, s->pdstFenShd, "Assembly SHGC");
        s->pdchFenShdAssemVisTr = newPreDefColumn(state, s->pdstFenShd, "Assembly Visible Transmittance");

        s->pdstIntFen = newPreDefSubTable(state, s->pdrEnvelope, "Interior Fenestration");

        s->pdchIntFenCons = newPreDefColumn(state, s->pdstIntFen, "Construction");
        s->pdchIntFenZone = newPreDefColumn(state, s->pdstIntFen, "Zone");
        s->pdchIntFenSpace = newPreDefColumn(state, s->pdstIntFen, "Space");
        s->pdchIntFenAreaOf1 = newPreDefColumn(state, s->pdstIntFen, "Area of One Opening [m2]");
        s->pdchIntFenArea = newPreDefColumn(state, s->pdstIntFen, "Area of Openings [m2]");
        s->pdchIntFenUfact = newPreDefColumn(state, s->pdstIntFen, "Glass U-Factor [W/m2-K]");
        s->pdchIntFenSHGC = newPreDefColumn(state, s->pdstIntFen, "Glass SHGC");
        s->pdchIntFenVisTr = newPreDefColumn(state, s->pdstIntFen, "Glass Visible Transmittance");
        s->pdchIntFenParent = newPreDefColumn(state, s->pdstIntFen, "Parent Surface");
        // s->pdchIntFenGlassAreaOf1 =   newPreDefColumn(state, s->pdstIntFen,'Glass Area [m2]')
        // s->pdchIntFenFrameAreaOf1 =   newPreDefColumn(state, s->pdstIntFen,'Frame Area [m2]')
        // s->pdchIntFenDividerAreaOf1 =   newPreDefColumn(state, s->pdstIntFen,'Divider Area [m2]')
        // s->pdchIntFenFrameConductance =  newPreDefColumn(state, s->pdstIntFen,'Frame Conductance [W/m2-K]')
        // s->pdchIntFenDividerConductance =  newPreDefColumn(state, s->pdstIntFen,'Divider Conductance [W/m2-K]')

        s->pdstDoor = newPreDefSubTable(state, s->pdrEnvelope, "Exterior Door");
        s->pdchDrCons = newPreDefColumn(state, s->pdstDoor, "Construction");
        s->pdchDrZone = newPreDefColumn(state, s->pdstDoor, "Zone");
        s->pdchDrSpace = newPreDefColumn(state, s->pdstDoor, "Space");
        s->pdchDrUfactFilm = newPreDefColumn(state, s->pdstDoor, "U-Factor with Film [W/m2-K]");
        s->pdchDrUfactNoFilm = newPreDefColumn(state, s->pdstDoor, "U-Factor no Film [W/m2-K]");
        s->pdchDrGrArea = newPreDefColumn(state, s->pdstDoor, "Gross Area [m2]");
        s->pdchDrParent = newPreDefColumn(state, s->pdstDoor, "Parent Surface");

        s->pdstIntDoor = newPreDefSubTable(state, s->pdrEnvelope, "Interior Door");

        s->pdchIntDrCons = newPreDefColumn(state, s->pdstIntDoor, "Construction");
        s->pdchIntDrZone = newPreDefColumn(state, s->pdstIntDoor, "Zone");
        s->pdchIntDrSpace = newPreDefColumn(state, s->pdstIntDoor, "Space");
        s->pdchIntDrUfactFilm = newPreDefColumn(state, s->pdstIntDoor, "U-Factor with Film [W/m2-K]");
        s->pdchIntDrUfactNoFilm = newPreDefColumn(state, s->pdstIntDoor, "U-Factor no Film [W/m2-K]");
        s->pdchIntDrGrArea = newPreDefColumn(state, s->pdstIntDoor, "Gross Area [m2]");
        s->pdchIntDrParent = newPreDefColumn(state, s->pdstIntDoor, "Parent Surface");

        s->pdstOpqConsLayers = newPreDefSubTable(state, s->pdrEnvelope, "Opaque Construction Layers");

        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 1"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 2"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 3"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 4"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 5"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 6"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 7"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 8"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 9"));
        s->pdchOpqConsLayCol.push_back(newPreDefColumn(state, s->pdstOpqConsLayers, "Layer 10"));

        // Shading Report
        s->pdrShading = newPreDefReport(state, "ShadingSummary", "Shade", "Shading Summary");

        s->pdstSunlitFrac = newPreDefSubTable(state, s->pdrShading, "Sunlit Fraction");

        s->pdchSlfMar21_9 = newPreDefColumn(state, s->pdstSunlitFrac, "March 21 9am");
        s->pdchSlfMar21_12 = newPreDefColumn(state, s->pdstSunlitFrac, "March 21 noon");
        s->pdchSlfMar21_15 = newPreDefColumn(state, s->pdstSunlitFrac, "March 21 3pm");
        s->pdchSlfJun21_9 = newPreDefColumn(state, s->pdstSunlitFrac, "June 21 9am");
        s->pdchSlfJun21_12 = newPreDefColumn(state, s->pdstSunlitFrac, "June 21 noon");
        s->pdchSlfJun21_15 = newPreDefColumn(state, s->pdstSunlitFrac, "June 21 3pm");
        s->pdchSlfDec21_9 = newPreDefColumn(state, s->pdstSunlitFrac, "December 21 9am");
        s->pdchSlfDec21_12 = newPreDefColumn(state, s->pdstSunlitFrac, "December 21 noon");
        s->pdchSlfDec21_15 = newPreDefColumn(state, s->pdstSunlitFrac, "December 21 3pm");

        s->pdstWindowControl = newPreDefSubTable(state, s->pdrShading, "Window Control");

        s->pdchWscName = newPreDefColumn(state, s->pdstWindowControl, "Name");
        s->pdchWscShading = newPreDefColumn(state, s->pdstWindowControl, "Type");
        s->pdchWscShadCons = newPreDefColumn(state, s->pdstWindowControl, "Shaded Construction");
        s->pdchWscControl = newPreDefColumn(state, s->pdstWindowControl, "Control");
        s->pdchWscGlare = newPreDefColumn(state, s->pdstWindowControl, "Glare Control");

        // Lighting Report
        s->pdrLighting = newPreDefReport(state, "LightingSummary", "Light", "Lighting Summary");

        s->pdstInLite = newPreDefSubTable(state, s->pdrLighting, "Interior Lighting");

        s->pdchInLtZone = newPreDefColumn(state, s->pdstInLite, "Zone Name");
        s->pdchInLtSpace = newPreDefColumn(state, s->pdstInLite, "Space Name");
        s->pdchInLtSpaceType = newPreDefColumn(state, s->pdstInLite, "Space Type");
        s->pdchInLtDens = newPreDefColumn(state, s->pdstInLite, "Lighting Power Density [W/m2]");
        s->pdchInLtArea = newPreDefColumn(state, s->pdstInLite, "Space Area [m2]");
        s->pdchInLtPower = newPreDefColumn(state, s->pdstInLite, "Total Power [W]");
        s->pdchInLtEndUse = newPreDefColumn(state, s->pdstInLite, "End Use Subcategory");
        s->pdchInLtSchd = newPreDefColumn(state, s->pdstInLite, "Schedule Name");
        s->pdchInLtAvgHrSchd = newPreDefColumn(state, s->pdstInLite, "Scheduled Hours/Week [hr]");
        s->pdchInLtAvgHrOper = newPreDefColumn(state, s->pdstInLite, "Hours/Week > 1% [hr]");
        s->pdchInLtFullLoadHrs = newPreDefColumn(state, s->pdstInLite, "Full Load Hours/Week [hr]");
        s->pdchInLtRetAir = newPreDefColumn(state, s->pdstInLite, "Return Air Fraction");
        s->pdchInLtCond = newPreDefColumn(state, s->pdstInLite, "Conditioned (Y/N)");
        s->pdchInLtConsump = newPreDefColumn(state, s->pdstInLite, "Consumption [GJ]");

        s->pdstDaylight = newPreDefSubTable(state, s->pdrLighting, "Daylighting");

        s->pdchDyLtZone = newPreDefColumn(state, s->pdstDaylight, "Zone");
        s->pdchDyLtCtrlName = newPreDefColumn(state, s->pdstDaylight, "Control Name");
        s->pdchDyLtKind = newPreDefColumn(state, s->pdstDaylight, "Daylighting Method"); // detailed or DElight
        s->pdchDyLtCtrlType = newPreDefColumn(state, s->pdstDaylight, "Control Type");   // stepped or continuous
        s->pdchDyLtFrac = newPreDefColumn(state, s->pdstDaylight, "Fraction Controlled");
        s->pdchDyLtWInst = newPreDefColumn(state, s->pdstDaylight, "Lighting Installed in Zone [W]");
        s->pdchDyLtWCtrl = newPreDefColumn(state, s->pdstDaylight, "Lighting Controlled [W]");

        s->pdstExtLite = newPreDefSubTable(state, s->pdrLighting, "Exterior Lighting");

        s->pdchExLtPower = newPreDefColumn(state, s->pdstExtLite, "Total Watts");
        s->pdchExLtClock = newPreDefColumn(state, s->pdstExtLite, "Astronomical Clock/Schedule");
        s->pdchExLtSchd = newPreDefColumn(state, s->pdstExtLite, "Schedule Name");
        s->pdchExLtAvgHrSchd = newPreDefColumn(state, s->pdstExtLite, "Scheduled Hours/Week [hr]");
        s->pdchExLtAvgHrOper = newPreDefColumn(state, s->pdstExtLite, "Hours/Week > 1% [hr]");
        s->pdchExLtFullLoadHrs = newPreDefColumn(state, s->pdstExtLite, "Full Load Hours/Week [hr]");
        s->pdchExLtConsump = newPreDefColumn(state, s->pdstExtLite, "Consumption [GJ]");

        // HVAC Equipment Report

        s->pdrEquip = newPreDefReport(state, "EquipmentSummary", "Equip", "Equipment Summary");

        s->pdstMech = newPreDefSubTable(state, s->pdrEquip, "Central Plant");

        s->pdchMechType = newPreDefColumn(state, s->pdstMech, "Type");
        s->pdchMechNomCap = newPreDefColumn(state, s->pdstMech, "Reference Capacity [W]");
        s->pdchMechNomEff = newPreDefColumn(state, s->pdstMech, "Reference Efficiency [W/W]");
        s->pdchMechRatCap = newPreDefColumn(state, s->pdstMech, "Rated Capacity [W]");
        s->pdchMechRatEff = newPreDefColumn(state, s->pdstMech, "Rated Efficiency [W/W]");
        // Note: We don't want any of these to convert.
        // The Btu/W-h isn't going to convert anyways, and the W/W will convert to W/W since it has "SI" in the string as a hint
        s->pdchMechIPLVSI = newPreDefColumn(state, s->pdstMech, "IPLV in SI Units [W/W]");
        s->pdchMechIPLVIP = newPreDefColumn(state, s->pdstMech, "IPLV in IP Units [Btu/W-h]");

        // Ok Constant                        Object Name                            Module                   Example File
        // -- ------------------------------- -------------------------------------- ------------------------ -----------------
        // o  CoilDX_CoolingSingleSpeed       Coil:Cooling:DX:SingleSpeed            DXCoil                   FurnaceWithDXSystem
        // x  CoilDX_CoolingTwoSpeed          Coil:Cooling:DX:TwoSpeed               DXCoil                   5ZoneAutoDXVAV
        // o  CoilDX_CoolingTwoStageWHumControl    Coil:Cooling:DX:                 DXCoil                   SmOffPSZ-MultiModeDX
        //                                    TwoStageWithHumidityControlMode
        // o  CoilDX_MultiSpeedCooling        Coil:Cooling:DX:MultiSpeed             DXCoil                   MultispeedHeatPump
        // o  Coil_CoolingWater               Coil:Cooling:Water                     HVACWaterCoilComponent   5ZoneAirCooled
        // o  Coil_CoolingWaterDetailed       Coil:Cooling:Water:DetailedGeometry    HVACWaterCoilComponent   5zoneWaterSystems
        // o  Coil_CoolingWaterToAirHP        Coil:Cooling:WaterToAirHeatPump:       HVACWaterToAir           5ZoneWaterLoopHeatPump
        //                                      ParameterEstimation
        // o  Coil_CoolingWaterToAirHPSimple  Coil:Cooling:WaterToAirHeatPump:       HVACWaterToAir           HeatPumpWaterToAirEquationFit
        //                                      EquationFit

        // o  CoilDX_HeatingEmpirical         Coil:Heating:DX:SingleSpeed            DXCoil                   HeatPumpAuto
        // o  CoilDX_MultiSpeedHeating        Coil:Heating:DX:MultiSpeed             DXCoil                   MultispeedHeatPump
        // o  Coil_HeatingGasOrOtherFuel                 Coil:Heating:Fuel                       HVACHeatingCoils         5ZoneAutoDXVAV
        // o  Coil_HeatingElectric            Coil:Heating:Electric                  HVACHeatingCoils         PackagedTerminalAirConditioner
        // o  Coil_HeatingDesuperheater       Coil:Heating:Desuperheater             HVACHeatingCoils         SuperMarket_DesuperHeatingCoil
        // o  Coil_HeatingWater               Coil:Heating:Water                     HVACWaterCoilComponent   5ZoneAirCooled
        // o  Coil_HeatingWaterToAirHP        Coil:Heating:WaterToAirHeatPump:       HVACWaterToAir           5ZoneWaterLoopHeatPump
        //                                      ParameterEstimation
        // o  Coil_HeatingWaterToAirHPSimple  Coil:Heating:WaterToAirHeatPump:       HVACWaterToAir           HeatPumpWaterToAirEquationFit
        //                                      EquationFit
        // o  CoilDX_HeatPumpWaterHeater      Coil:WaterHeating:AirToWaterHeatPump   DXCoil                   HeatPumpWaterHeater

        // NOT INCLUDED:
        //    CoilDX_CoolingHXAssisted        CoilSystem:Cooling:DX:                 HVACHXAssistedCooolingCoil
        //                                      HeatExchangerAssisted
        //    CoilWater_CoolingHXAssisted     CoilSystem:Cooling:Water:              HVACHXAssistedCooolingCoil
        //                                      HeatExchangerAssisted

        s->pdstCoolCoil = newPreDefSubTable(state, s->pdrEquip, "Cooling Coils");

        s->pdchCoolCoilType = newPreDefColumn(state, s->pdstCoolCoil, "Type");
        s->pdchCoolCoilDesCap = newPreDefColumn(state, s->pdstCoolCoil, "Design Coil Load [W]");
        s->pdchCoolCoilTotCap = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Total Capacity [W]");
        s->pdchCoolCoilSensCap = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Sensible Capacity [W]");
        s->pdchCoolCoilLatCap = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Latent Capacity [W]");
        s->pdchCoolCoilSHR = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Sensible Heat Ratio");
        s->pdchCoolCoilNomEff = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Efficiency [W/W]");
        s->pdchCoolCoilUATotal = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Coil UA Value [W/C]");
        s->pdchCoolCoilArea = newPreDefColumn(state, s->pdstCoolCoil, "Nominal Coil Surface Area [m2]");

        s->pdstDXCoolCoil = newPreDefSubTable(state, s->pdrEquip, "DX Cooling Coil Standard Ratings 2017");
        s->pdchDXCoolCoilType = newPreDefColumn(state, s->pdstDXCoolCoil, "Cooling Coil Type (1)");
        s->pdchDXCoolCoilNetCapSI = newPreDefColumn(state, s->pdstDXCoolCoil, "Standard Rating Net Cooling Capacity [W][2]");

        s->pdchDXCoolCoilCOP = newPreDefColumn(state, s->pdstDXCoolCoil, "Standard Rating Net COP [W/W][2]");
        s->pdchDXCoolCoilEERIP = newPreDefColumn(state, s->pdstDXCoolCoil, "EER [Btu/W-h][2]");
        s->pdchDXCoolCoilSEERUserIP = newPreDefColumn(state, s->pdstDXCoolCoil, "SEER User [Btu/W-h][2,3]");
        s->pdchDXCoolCoilSEERStandardIP = newPreDefColumn(state, s->pdstDXCoolCoil, "SEER Standard [Btu/W-h][2,3]");
        s->pdchDXCoolCoilIEERIP = newPreDefColumn(state, s->pdstDXCoolCoil, "IEER [Btu/W-h][2]");

        // for DX Cooling Coil AHRI Standard 2023 Ratings | SEER2
        s->pdstDXCoolCoil_2023 = newPreDefSubTable(state, s->pdrEquip, "DX Cooling Coil Standard Ratings 2023");
        s->pdchDXCoolCoilType_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "Cooling Coil Type (1)");
        s->pdchDXCoolCoilNetCapSI_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "Standard Rating Net Cooling Capacity [W][2]");

        s->pdchDXCoolCoilCOP_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "Standard Rating Net COP2 [W/W][2,4]");
        s->pdchDXCoolCoilEERIP_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "EER2 [Btu/W-h][2,4]");
        s->pdchDXCoolCoilSEER2UserIP_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "SEER2 User [Btu/W-h][2,3]");
        s->pdchDXCoolCoilSEER2StandardIP_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "SEER2 Standard [Btu/W-h][2,3]");
        s->pdchDXCoolCoilIEERIP_2023 = newPreDefColumn(state, s->pdstDXCoolCoil_2023, "IEER [Btu/W-h][2]");

        // for DX Cooling Coil ASHRAE 127-12 Report
        s->pdstDXCoolCoil2 = newPreDefSubTable(state, s->pdrEquip, "DX Cooling Coil ASHRAE 127 Standard Ratings Report");
        s->pdchDXCoolCoilType2 = newPreDefColumn(state, s->pdstDXCoolCoil2, "DX Cooling Coil Type");
        s->pdchDXCoolCoilNetCapSIA = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Net Cooling Capacity Test A [W]");
        s->pdchDXCoolCoilElecPowerA = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Electric Power Test A [W]");
        s->pdchDXCoolCoilNetCapSIB = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Net Cooling Capacity Test B [W]");
        s->pdchDXCoolCoilElecPowerB = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Electric Power Test B [W]");
        s->pdchDXCoolCoilNetCapSIC = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Net Cooling Capacity Test C [W]");
        s->pdchDXCoolCoilElecPowerC = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Electric Power Test C [W]");
        s->pdchDXCoolCoilNetCapSID = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Net Cooling Capacity Test D [W]");
        s->pdchDXCoolCoilElecPowerD = newPreDefColumn(state, s->pdstDXCoolCoil2, "Rated Electric Power Test D [W]");

        // Water-to-Air HP report
        s->pdstWAHP = newPreDefSubTable(state, s->pdrEquip, "Water-to-Air Heat Pumps at Rated Temperatures Report");
        s->pdchWAHPType = newPreDefColumn(state, s->pdstWAHP, "Coil Type");
        s->pdchWAHPRatedCapAtRatedCdts = newPreDefColumn(state, s->pdstWAHP, "Rated Total Capacity [W]");
        s->pdchWAHPRatedSensCapAtRatedCdts = newPreDefColumn(state, s->pdstWAHP, "Rated Sensible Capacity [W]");
        s->pdchWAHPRatedPowerAtRatedCdts = newPreDefColumn(state, s->pdstWAHP, "Rated Power[W]");
        s->pdchWAHPRatedCOPAtRatedCdts = newPreDefColumn(state, s->pdstWAHP, "Rated COP [W/W]");
        s->pdchWAHPRatedAirDBT = newPreDefColumn(state, s->pdstWAHP, "Rated Air Dry-bulb Temperature [C]");
        s->pdchWAHPRatedAirWBT = newPreDefColumn(state, s->pdstWAHP, "Rated Air Wet-bulb Temperature [C]");
        s->pdchWAHPRatedWtrT = newPreDefColumn(state, s->pdstWAHP, "Rated Water Temperature [C]");
        s->pdchWAHPDD = newPreDefColumn(state, s->pdstWAHP, "Design Day used for Sizing");

        s->pdstDXHeatCoil = newPreDefSubTable(state, s->pdrEquip, "DX Heating Coils");
        s->pdchDXHeatCoilType = newPreDefColumn(state, s->pdstDXHeatCoil, "DX Heating Coil Type");
        s->pdchDXHeatCoilHighCap = newPreDefColumn(state, s->pdstDXHeatCoil, "High Temperature Heating (net) Rating Capacity [W]");
        s->pdchDXHeatCoilLowCap = newPreDefColumn(state, s->pdstDXHeatCoil, "Low Temperature Heating (net) Rating Capacity [W]");
        s->pdchDXHeatCoilHSPFIP = newPreDefColumn(state, s->pdstDXHeatCoil, "HSPF [Btu/W-h]");
        s->pdchDXHeatCoilRegionNum = newPreDefColumn(state, s->pdstDXHeatCoil, "Region Number");
        // Std 229 Predef outputs for DX Heating Coils
        s->pdchDXHeatCoilMinOADBTforCompOp =
            newPreDefColumn(state, s->pdstDXHeatCoil, "Minimum Outdoor Dry-Bulb Temperature for Compressor Operation [C]");
        s->pdchDXHeatCoilSuppHiT = newPreDefColumn(state, s->pdstDXHeatCoil, "Supplemental Heat High Shutoff Temperature [C]");
        s->pdchDXHeatCoilAirloopName = newPreDefColumn(state, s->pdstDXHeatCoil, "Airloop Name");

        // for DX Heating Coil AHRI Standard 2023 Ratings | HSPF2
        s->pdstDXHeatCoil_2023 = newPreDefSubTable(state, s->pdrEquip, "DX Heating Coils AHRI 2023");
        s->pdchDXHeatCoilType_2023 = newPreDefColumn(state, s->pdstDXHeatCoil_2023, "DX Heating Coil Type");
        s->pdchDXHeatCoilHighCap_2023 = newPreDefColumn(state, s->pdstDXHeatCoil_2023, "High Temperature Heating (net) Rating Capacity [W]");
        s->pdchDXHeatCoilLowCap_2023 = newPreDefColumn(state, s->pdstDXHeatCoil_2023, "Low Temperature Heating (net) Rating Capacity [W]");
        s->pdchDXHeatCoilHSPF2IP_2023 = newPreDefColumn(state, s->pdstDXHeatCoil_2023, "HSPF2 [Btu/W-h]");
        s->pdchDXHeatCoilRegionNum_2023 = newPreDefColumn(state, s->pdstDXHeatCoil_2023, "Region Number");
        // Std 229 Predef outputs for DX Heating Coils AHRI 2023
        s->pdchDXHeatCoilMinOADBTforCompOp_2023 =
            newPreDefColumn(state, s->pdstDXHeatCoil_2023, "Minimum Outdoor Dry-Bulb Temperature for Compressor Operation [C]");
        s->pdchDXHeatCoilAirloopName_2023 = newPreDefColumn(state, s->pdstDXHeatCoil_2023, "Airloop Name");

        s->pdstHeatCoil = newPreDefSubTable(state, s->pdrEquip, "Heating Coils");

        s->pdchHeatCoilType = newPreDefColumn(state, s->pdstHeatCoil, "Type");
        s->pdchHeatCoilDesCap = newPreDefColumn(state, s->pdstHeatCoil, "Design Coil Load [W]");
        s->pdchHeatCoilNomCap = newPreDefColumn(state, s->pdstHeatCoil, "Nominal Total Capacity [W]");
        s->pdchHeatCoilNomEff = newPreDefColumn(state, s->pdstHeatCoil, "Nominal Efficiency [W/W]");
        // Std 229 Predef outputs for Heating Coils
        s->pdchHeatCoilUsedAsSupHeat = newPreDefColumn(state, s->pdstHeatCoil, "Used as Supplementary Heat");
        s->pdchHeatCoilAirloopName = newPreDefColumn(state, s->pdstHeatCoil, "Airloop Name");
        s->pdchHeatCoilPlantloopName = newPreDefColumn(state, s->pdstHeatCoil, "Plantloop Name");

        s->pdstFan = newPreDefSubTable(state, s->pdrEquip, "Fans");

        s->pdchFanType = newPreDefColumn(state, s->pdstFan, "Type");
        s->pdchFanTotEff = newPreDefColumn(state, s->pdstFan, "Total Efficiency [W/W]");
        s->pdchFanDeltaP = newPreDefColumn(state, s->pdstFan, "Delta Pressure [pa]");
        s->pdchFanVolFlow = newPreDefColumn(state, s->pdstFan, "Max Air Flow Rate [m3/s]");
        s->pdchFanPwr = newPreDefColumn(state, s->pdstFan, "Rated Electricity Rate [W]");
        s->pdchFanPwrPerFlow = newPreDefColumn(state, s->pdstFan, "Rated Power Per Max Air Flow Rate [W-s/m3]");
        s->pdchFanMotorIn = newPreDefColumn(state, s->pdstFan, "Motor Heat In Air Fraction");
        s->pdchFanEnergyIndex = newPreDefColumn(state, s->pdstFan, "Fan Energy Index");
        s->pdchFanEndUse = newPreDefColumn(state, s->pdstFan, "End Use Subcategory");
        s->pdchFanDesDay = newPreDefColumn(state, s->pdstFan, "Design Day Name for Fan Sizing Peak");
        s->pdchFanPkTime = newPreDefColumn(state, s->pdstFan, "Date/Time for Fan Sizing Peak");
        // Std 229 Predef outputs for Fans
        s->pdchFanPurpose = newPreDefColumn(state, s->pdstFan, "Purpose");
        s->pdchFanAutosized = newPreDefColumn(state, s->pdstFan, "Is Autosized");
        s->pdchFanMotorEff = newPreDefColumn(state, s->pdstFan, "Motor Efficiency");
        s->pdchFanMotorHeatToZoneFrac = newPreDefColumn(state, s->pdstFan, "Motor Heat to Zone Fraction");
        s->pdchFanMotorHeatZone = newPreDefColumn(state, s->pdstFan, "Motor Loss Zone Name");
        s->pdchFanAirLoopName = newPreDefColumn(state, s->pdstFan, "Airloop Name");

        s->pdstPump = newPreDefSubTable(state, s->pdrEquip, "Pumps");
        s->pdchPumpType = newPreDefColumn(state, s->pdstPump, "Type");
        s->pdchPumpControl = newPreDefColumn(state, s->pdstPump, "Control");
        s->pdchPumpHead = newPreDefColumn(state, s->pdstPump, "Head [pa]");
        s->pdchPumpFlow = newPreDefColumn(state, s->pdstPump, "Water Flow [m3/s]");
        s->pdchPumpPower = newPreDefColumn(state, s->pdstPump, "Electricity Rate [W]");
        s->pdchPumpPwrPerFlow = newPreDefColumn(state, s->pdstPump, "Power Per Water Flow Rate [W-s/m3]");
        s->pdchMotEff = newPreDefColumn(state, s->pdstPump, "Motor Efficiency [W/W]");
        s->pdchPumpEndUse = newPreDefColumn(state, s->pdstPump, "End Use Subcategory");
        // Std 229 Predef outputs for Pumps
        s->pdchPumpAutosized = newPreDefColumn(state, s->pdstPump, "Is Autosized");
        s->pdchPumpPlantloopName = newPreDefColumn(state, s->pdstPump, "Plantloop Name");
        s->pdchPumpPlantloopBranchName = newPreDefColumn(state, s->pdstPump, "Plantloop Branch Name");

        s->pdstSWH = newPreDefSubTable(state, s->pdrEquip, "Service Water Heating");
        s->pdchSWHType = newPreDefColumn(state, s->pdstSWH, "Type");
        s->pdchSWHVol = newPreDefColumn(state, s->pdstSWH, "Storage Volume [m3]");
        s->pdchSWHHeatIn = newPreDefColumn(state, s->pdstSWH, "Input [W]");
        s->pdchSWHThEff = newPreDefColumn(state, s->pdstSWH, "Thermal Efficiency [W/W]");
        s->pdchSWHRecEff = newPreDefColumn(state, s->pdstSWH, "Recovery Efficiency [W/W]");
        s->pdchSWHEnFac = newPreDefColumn(state, s->pdstSWH, "Energy Factor");

        // Std 229 Chillers in Equipment Summary
        s->pdstChiller = newPreDefSubTable(state, s->pdrEquip, "Chillers");

        s->pdchChillerType = newPreDefColumn(state, s->pdstChiller, "Type");
        s->pdchChillerRefCap = newPreDefColumn(state, s->pdstChiller, "Reference Capacity[W]");
        s->pdchChillerRefEff = newPreDefColumn(state, s->pdstChiller, "TypeReference Efficiency [W/W]");
        s->pdchChillerRatedCap = newPreDefColumn(state, s->pdstChiller, "Rated Capacity [W]");
        s->pdchChillerRatedEff = newPreDefColumn(state, s->pdstChiller, "Rated Efficiency [W/W]");
        s->pdchChillerIPLVinSI = newPreDefColumn(state, s->pdstChiller, "IPLV in SI Units [W/W]");
        s->pdchChillerIPLVinIP = newPreDefColumn(state, s->pdstChiller, "IPLV in IP Units [Btu/W-h]");
        s->pdchChillerMinPLR = newPreDefColumn(state, s->pdstChiller, "Minimum Part Load Ratio");
        s->pdchChillerFuelType = newPreDefColumn(state, s->pdstChiller, "Fuel Type");
        s->pdchChillerRatedEntCondTemp = newPreDefColumn(state, s->pdstChiller, "Rated Entering Condenser Temperature [C]");
        s->pdchChillerRatedLevEvapTemp = newPreDefColumn(state, s->pdstChiller, "Rated Leaving Evaporator Temperature [C]");
        s->pdchChillerRefEntCondTemp = newPreDefColumn(state, s->pdstChiller, "Reference Entering Condenser Temperature [C]");
        s->pdchChillerRefLevEvapTemp = newPreDefColumn(state, s->pdstChiller, "Reference Leaving Evaporator Temperature [C]");
        s->pdchChillerDesSizeRefCHWFlowRate = newPreDefColumn(state, s->pdstChiller, "Design Size Reference Chilled Water Flow Rate [kg/s]");
        s->pdchChillerDesSizeRefCondFluidFlowRate = newPreDefColumn(state, s->pdstChiller, "Design Size Reference Condenser Fluid Flow Rate [kg/s]");
        s->pdchChillerPlantloopName = newPreDefColumn(state, s->pdstChiller, "Plantloop Name");
        s->pdchChillerPlantloopBranchName = newPreDefColumn(state, s->pdstChiller, "Plantloop Branch Name");
        s->pdchChillerCondLoopName = newPreDefColumn(state, s->pdstChiller, "Condenser Loop Name");
        s->pdchChillerCondLoopBranchName = newPreDefColumn(state, s->pdstChiller, "Condenser Loop Branch Name");
        s->pdchChillerHeatRecPlantloopName = newPreDefColumn(state, s->pdstChiller, "Heat Recovery Plantloop Name");
        s->pdchChillerHeatRecPlantloopBranchName = newPreDefColumn(state, s->pdstChiller, "Heat Recovery Plantloop Branch Name");
        s->pdchChillerRecRelCapFrac = newPreDefColumn(state, s->pdstChiller, "Recovery Relative Capacity Fraction");

        // Std 229 Boiler Table in Equipment Summary
        s->pdstBoiler = newPreDefSubTable(state, s->pdrEquip, "Boilers");

        s->pdchBoilerType = newPreDefColumn(state, s->pdstBoiler, "Type");
        s->pdchBoilerRefCap = newPreDefColumn(state, s->pdstBoiler, "Reference Capacity [W]");
        s->pdchBoilerRefEff = newPreDefColumn(state, s->pdstBoiler, "Reference Efficiency[W/W]");
        s->pdchBoilerRatedCap = newPreDefColumn(state, s->pdstBoiler, "Rated Capacity [W]");
        s->pdchBoilerRatedEff = newPreDefColumn(state, s->pdstBoiler, "Rated Efficiency [W/W]");
        s->pdchBoilerMinPLR = newPreDefColumn(state, s->pdstBoiler, "Minimum Part Load Ratio");
        s->pdchBoilerFuelType = newPreDefColumn(state, s->pdstBoiler, "Fuel Type");
        s->pdchBoilerParaElecLoad = newPreDefColumn(state, s->pdstBoiler, "Parasitic Electric Load [W]");
        s->pdchBoilerPlantloopName = newPreDefColumn(state, s->pdstBoiler, "Plantloop Name");
        s->pdchBoilerPlantloopBranchName = newPreDefColumn(state, s->pdstBoiler, "Plantloop Branch Name");

        // Std 229 cooling towers and fluid coolers Table in Equipment Summary
        s->pdstCTFC = newPreDefSubTable(state, s->pdrEquip, "Cooling Towers and Fluid Coolers");

        s->pdchCTFCType = newPreDefColumn(state, s->pdstCTFC, "Type");
        s->pdchCTFCFluidType = newPreDefColumn(state, s->pdstCTFC, "Fluid Type");
        s->pdchCTFCRange = newPreDefColumn(state, s->pdstCTFC, "Range [C]");
        s->pdchCTFCApproach = newPreDefColumn(state, s->pdstCTFC, "Approach [C]");
        s->pdchCTFCDesFanPwr = newPreDefColumn(state, s->pdstCTFC, "Design Fan Power [W]");
        s->pdchCTFCDesInletAirWBT = newPreDefColumn(state, s->pdstCTFC, "Design Inlet Air Wet-Bulb Temperature [C]");
        s->pdchCTFCDesWaterFlowRate = newPreDefColumn(state, s->pdstCTFC, "Design Water Flow Rate [m3/s]");
        s->pdchCTFCLevWaterSPTemp = newPreDefColumn(state, s->pdstCTFC, "Leaving Water Setpoint Temperature [C]");
        s->pdchCTFCCondLoopName = newPreDefColumn(state, s->pdstCTFC, "Condenser Loop Name");
        s->pdchCTFCCondLoopBranchName = newPreDefColumn(state, s->pdstCTFC, "Condenser Loop Branch Name");

        // Std 229 Plantloop and CondenserLoop Table in Equipment Summary
        s->pdstPLCL = newPreDefSubTable(state, s->pdrEquip, "PlantLoop or CondenserLoop");

        s->pdchPLCLType = newPreDefColumn(state, s->pdstPLCL, "Type");
        s->pdchPLCLProvHeat = newPreDefColumn(state, s->pdstPLCL, "Provides Heating");
        s->pdchPLCLProvCool = newPreDefColumn(state, s->pdstPLCL, "Provides Cooling");
        s->pdchPLCLMaxLoopFlowRate = newPreDefColumn(state, s->pdstPLCL, "Maximum Loop Flow Rate [m3/s]");
        s->pdchPLCLMinLoopFlowRate = newPreDefColumn(state, s->pdstPLCL, "Minimum Loop Flow Rate [m3/s]");

        // Std 229 Air Terminal Table in Equipment Summary
        s->pdstAirTerm = newPreDefSubTable(state, s->pdrEquip, "Air Terminals");

        s->pdchAirTermZoneName = newPreDefColumn(state, s->pdstAirTerm, "Zone Name");
        s->pdchAirTermMinFlow = newPreDefColumn(state, s->pdstAirTerm, "Minimum Flow [m3/s]");
        s->pdchAirTermMinOutdoorFlow = newPreDefColumn(state, s->pdstAirTerm, "Minimum Outdoor Flow [m3/s]");
        s->pdchAirTermSupCoolingSP = newPreDefColumn(state, s->pdstAirTerm, "Supply Cooling Setpoint [C]");
        s->pdchAirTermSupHeatingSP = newPreDefColumn(state, s->pdstAirTerm, "Supply Heating Setpoint [C]");
        s->pdchAirTermHeatingCap = newPreDefColumn(state, s->pdstAirTerm, "Heating Capacity [W]");
        s->pdchAirTermCoolingCap = newPreDefColumn(state, s->pdstAirTerm, "Cooling Capacity [W]");

        s->pdchAirTermTypeInp = newPreDefColumn(state, s->pdstAirTerm, "Type of Input Object");
        s->pdchAirTermHeatCoilType = newPreDefColumn(state, s->pdstAirTerm, "Heat/Reheat Coil Object Type");
        s->pdchAirTermCoolCoilType = newPreDefColumn(state, s->pdstAirTerm, "Chilled Water Coil Object Type");
        s->pdchAirTermFanType = newPreDefColumn(state, s->pdstAirTerm, "Fan Object Type");
        s->pdchAirTermFanName = newPreDefColumn(state, s->pdstAirTerm, "Fan Name");
        s->pdchAirTermPrimFlow = newPreDefColumn(state, s->pdstAirTerm, "Primary Air Flow Rate [m3/s]");
        s->pdchAirTermSecdFlow = newPreDefColumn(state, s->pdstAirTerm, "Secondary Air Flow Rate [m3/s]");
        s->pdchAirTermMinFlowSch = newPreDefColumn(state, s->pdstAirTerm, "Minimum Flow Schedule Name");
        s->pdchAirTermMaxFlowReh = newPreDefColumn(state, s->pdstAirTerm, "Maximum Flow During Reheat [m3/s]");
        s->pdchAirTermMinOAflowSch = newPreDefColumn(state, s->pdstAirTerm, "Minimum Outdoor Flow Schedule Name");

        // Std 229 Air Heat Recovery
        s->pdstAirHR = newPreDefSubTable(state, s->pdrEquip, "Air Heat Recovery");

        s->pdchAirHRInputObjType = newPreDefColumn(state, s->pdstAirHR, "Type");
        s->pdchAirHRPlateOrRotary = newPreDefColumn(state, s->pdstAirHR, "Plate/Rotary");
        s->pdchAirHRSenEffAt100PerHeatAirFlow = newPreDefColumn(state, s->pdstAirHR, "Sensible Effectiveness at 100% Heating Air Flow");
        s->pdchAirHRSenEffAt100PerCoolAirFlow = newPreDefColumn(state, s->pdstAirHR, "Sensible Effectiveness at 100% Cooling Air Flow");
        s->pdchAirHRLatEffAt100PerHeatAirFlow = newPreDefColumn(state, s->pdstAirHR, "Latent Effectiveness at 100% Heating Air Flow");
        s->pdchAirHRLatEffAt100PerCoolAirFlow = newPreDefColumn(state, s->pdstAirHR, "Latent Effectiveness at 100% Cooling Air Flow");
        s->pdchAirHRSupplyAirflow = newPreDefColumn(state, s->pdstAirHR, "Supply Air Flow Rate [m3/s]");
        s->pdchAirHRExhaustAirflow = newPreDefColumn(state, s->pdstAirHR, "Exhaust Air Flow Rate [m3/s]");

        // Sizing Report

        s->pdrSizing = newPreDefReport(state, "HVACSizingSummary", "Size", "HVAC Sizing Summary");

        s->pdstSpaceClSize = newPreDefSubTable(state, s->pdrSizing, "Space Sensible Cooling");

        s->pdchSpClCalcDesLd = newPreDefColumn(state, s->pdstSpaceClSize, "Calculated Design Load [W]");
        s->pdchSpClUserDesLd = newPreDefColumn(state, s->pdstSpaceClSize, "User Design Load [W]");
        s->pdchSpClUserDesLdPerArea = newPreDefColumn(state, s->pdstSpaceClSize, "User Design Load per Area [W/m2]");
        s->pdchSpClCalcDesAirFlow = newPreDefColumn(state, s->pdstSpaceClSize, "Calculated Design Air Flow [m3/s]");
        s->pdchSpClUserDesAirFlow = newPreDefColumn(state, s->pdstSpaceClSize, "User Design Air Flow [m3/s]");
        s->pdchSpClDesDay = newPreDefColumn(state, s->pdstSpaceClSize, "Design Day Name");
        s->pdchSpClPkTime = newPreDefColumn(state, s->pdstSpaceClSize, "Date/Time Of Peak {TIMESTAMP}");
        s->pdchSpClPkTstatTemp = newPreDefColumn(state, s->pdstSpaceClSize, "Thermostat Setpoint Temperature at Peak Load [C]");
        s->pdchSpClPkIndTemp = newPreDefColumn(state, s->pdstSpaceClSize, "Indoor Temperature at Peak Load [C]");
        s->pdchSpClPkIndHum = newPreDefColumn(state, s->pdstSpaceClSize, "Indoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchSpClPkOATemp = newPreDefColumn(state, s->pdstSpaceClSize, "Outdoor Temperature at Peak Load [C]");
        s->pdchSpClPkOAHum = newPreDefColumn(state, s->pdstSpaceClSize, "Outdoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchSpClPkOAMinFlow = newPreDefColumn(state, s->pdstSpaceClSize, "Minimum Outdoor Air Flow Rate [m3/s]");
        s->pdchSpClPkDOASHeatGain = newPreDefColumn(state, s->pdstSpaceClSize, "Heat Gain Rate from DOAS [W]");
        addFootNoteSubTable(state,
                            s->pdstSpaceClSize,
                            "The Design Load is the space sensible load only. It does not include any system effects or ventilation loads.");

        s->pdstZoneClSize = newPreDefSubTable(state, s->pdrSizing, "Zone Sensible Cooling");

        s->pdchZnClCalcDesLd = newPreDefColumn(state, s->pdstZoneClSize, "Calculated Design Load [W]");
        s->pdchZnClUserDesLd = newPreDefColumn(state, s->pdstZoneClSize, "User Design Load [W]");
        s->pdchZnClUserDesLdPerArea = newPreDefColumn(state, s->pdstZoneClSize, "User Design Load per Area [W/m2]");
        s->pdchZnClCalcDesAirFlow = newPreDefColumn(state, s->pdstZoneClSize, "Calculated Design Air Flow [m3/s]");
        s->pdchZnClUserDesAirFlow = newPreDefColumn(state, s->pdstZoneClSize, "User Design Air Flow [m3/s]");
        s->pdchZnClDesDay = newPreDefColumn(state, s->pdstZoneClSize, "Design Day Name");
        s->pdchZnClPkTime = newPreDefColumn(state, s->pdstZoneClSize, "Date/Time Of Peak {TIMESTAMP}");
        s->pdchZnClPkTstatTemp = newPreDefColumn(state, s->pdstZoneClSize, "Thermostat Setpoint Temperature at Peak Load [C]");
        s->pdchZnClPkIndTemp = newPreDefColumn(state, s->pdstZoneClSize, "Indoor Temperature at Peak Load [C]");
        s->pdchZnClPkIndHum = newPreDefColumn(state, s->pdstZoneClSize, "Indoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchZnClPkOATemp = newPreDefColumn(state, s->pdstZoneClSize, "Outdoor Temperature at Peak Load [C]");
        s->pdchZnClPkOAHum = newPreDefColumn(state, s->pdstZoneClSize, "Outdoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchZnClPkOAMinFlow = newPreDefColumn(state, s->pdstZoneClSize, "Minimum Outdoor Air Flow Rate [m3/s]");
        s->pdchZnClPkDOASHeatGain = newPreDefColumn(state, s->pdstZoneClSize, "Heat Gain Rate from DOAS [W]");
        addFootNoteSubTable(
            state, s->pdstZoneClSize, "The Design Load is the zone sensible load only. It does not include any system effects or ventilation loads.");

        s->pdstSpaceHtSize = newPreDefSubTable(state, s->pdrSizing, "Space Sensible Heating");

        s->pdchSpHtCalcDesLd = newPreDefColumn(state, s->pdstSpaceHtSize, "Calculated Design Load [W]");
        s->pdchSpHtUserDesLd = newPreDefColumn(state, s->pdstSpaceHtSize, "User Design Load [W]");
        s->pdchSpHtUserDesLdPerArea = newPreDefColumn(state, s->pdstSpaceHtSize, "User Design Load per Area [W/m2]");
        s->pdchSpHtCalcDesAirFlow = newPreDefColumn(state, s->pdstSpaceHtSize, "Calculated Design Air Flow [m3/s]");
        s->pdchSpHtUserDesAirFlow = newPreDefColumn(state, s->pdstSpaceHtSize, "User Design Air Flow [m3/s]");
        s->pdchSpHtDesDay = newPreDefColumn(state, s->pdstSpaceHtSize, "Design Day Name");
        s->pdchSpHtPkTime = newPreDefColumn(state, s->pdstSpaceHtSize, "Date/Time Of Peak {TIMESTAMP}");
        s->pdchSpHtPkTstatTemp = newPreDefColumn(state, s->pdstSpaceHtSize, "Thermostat Setpoint Temperature at Peak Load [C]");
        s->pdchSpHtPkIndTemp = newPreDefColumn(state, s->pdstSpaceHtSize, "Indoor Temperature at Peak Load [C]");
        s->pdchSpHtPkIndHum = newPreDefColumn(state, s->pdstSpaceHtSize, "Indoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchSpHtPkOATemp = newPreDefColumn(state, s->pdstSpaceHtSize, "Outdoor Temperature at Peak Load [C]");
        s->pdchSpHtPkOAHum = newPreDefColumn(state, s->pdstSpaceHtSize, "Outdoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchSpHtPkOAMinFlow = newPreDefColumn(state, s->pdstSpaceHtSize, "Minimum Outdoor Air Flow Rate [m3/s]");
        s->pdchSpHtPkDOASHeatGain = newPreDefColumn(state, s->pdstSpaceHtSize, "Heat Gain Rate from DOAS [W]");
        addFootNoteSubTable(state,
                            s->pdstSpaceHtSize,
                            "The Design Load is the space sensible load only. It does not include any system effects or ventilation loads.");

        s->pdstZoneHtSize = newPreDefSubTable(state, s->pdrSizing, "Zone Sensible Heating");

        s->pdchZnHtCalcDesLd = newPreDefColumn(state, s->pdstZoneHtSize, "Calculated Design Load [W]");
        s->pdchZnHtUserDesLd = newPreDefColumn(state, s->pdstZoneHtSize, "User Design Load [W]");
        s->pdchZnHtUserDesLdPerArea = newPreDefColumn(state, s->pdstZoneHtSize, "User Design Load per Area [W/m2]");
        s->pdchZnHtCalcDesAirFlow = newPreDefColumn(state, s->pdstZoneHtSize, "Calculated Design Air Flow [m3/s]");
        s->pdchZnHtUserDesAirFlow = newPreDefColumn(state, s->pdstZoneHtSize, "User Design Air Flow [m3/s]");
        s->pdchZnHtDesDay = newPreDefColumn(state, s->pdstZoneHtSize, "Design Day Name");
        s->pdchZnHtPkTime = newPreDefColumn(state, s->pdstZoneHtSize, "Date/Time Of Peak {TIMESTAMP}");
        s->pdchZnHtPkTstatTemp = newPreDefColumn(state, s->pdstZoneHtSize, "Thermostat Setpoint Temperature at Peak Load [C]");
        s->pdchZnHtPkIndTemp = newPreDefColumn(state, s->pdstZoneHtSize, "Indoor Temperature at Peak Load [C]");
        s->pdchZnHtPkIndHum = newPreDefColumn(state, s->pdstZoneHtSize, "Indoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchZnHtPkOATemp = newPreDefColumn(state, s->pdstZoneHtSize, "Outdoor Temperature at Peak Load [C]");
        s->pdchZnHtPkOAHum = newPreDefColumn(state, s->pdstZoneHtSize, "Outdoor Humidity Ratio at Peak Load [kgWater/kgDryAir]");
        s->pdchZnHtPkOAMinFlow = newPreDefColumn(state, s->pdstZoneHtSize, "Minimum Outdoor Air Flow Rate [m3/s]");
        s->pdchZnHtPkDOASHeatGain = newPreDefColumn(state, s->pdstZoneHtSize, "Heat Gain Rate from DOAS [W]");
        addFootNoteSubTable(
            state, s->pdstZoneHtSize, "The Design Load is the zone sensible load only. It does not include any system effects or ventilation loads.");

        s->pdstSystemSize = newPreDefSubTable(state, s->pdrSizing, "System Design Air Flow Rates");

        s->pdchSysSizCalcClAir = newPreDefColumn(state, s->pdstSystemSize, "Calculated cooling [m3/s]");
        s->pdchSysSizUserClAir = newPreDefColumn(state, s->pdstSystemSize, "User cooling [m3/s]");
        s->pdchSysSizCalcHtAir = newPreDefColumn(state, s->pdstSystemSize, "Calculated heating [m3/s]");
        s->pdchSysSizUserHtAir = newPreDefColumn(state, s->pdstSystemSize, "User heating [m3/s]");
        s->pdchSysSizAdjustedClAir = newPreDefColumn(state, s->pdstSystemSize, "Adjusted cooling [m3/s]");
        s->pdchSysSizAdjustedHtAir = newPreDefColumn(state, s->pdstSystemSize, "Adjusted heating [m3/s]");
        s->pdchSysSizAdjustedMainAir = newPreDefColumn(state, s->pdstSystemSize, "Adjusted main [m3/s]");
        s->pdchSysSizCalcHeatFlowRatio = newPreDefColumn(state, s->pdstSystemSize, "Calculated Heating Air Flow Ratio []");
        s->pdchSysSizUserHeatFlowRatio = newPreDefColumn(state, s->pdstSystemSize, "User Heating Air Flow Ratio []");

        s->pdstPlantSize = newPreDefSubTable(state, s->pdrSizing, "Plant Loop Coincident Design Fluid Flow Rate Adjustments");
        //        s->pdchPlantSizPass = newPreDefColumn(state,  s->pdstPlantSize, "Sizing Pass" );
        s->pdchPlantSizPrevVdot = newPreDefColumn(state, s->pdstPlantSize, "Previous Design Volume Flow Rate [m3/s]");
        s->pdchPlantSizMeasVdot = newPreDefColumn(state, s->pdstPlantSize, "Algorithm Volume Flow Rate [m3/s]");
        s->pdchPlantSizCalcVdot = newPreDefColumn(state, s->pdstPlantSize, "Coincident Design Volume Flow Rate [m3/s]");
        s->pdchPlantSizCoincYesNo = newPreDefColumn(state, s->pdstPlantSize, "Coincident Size Adjusted");
        s->pdchPlantSizDesDay = newPreDefColumn(state, s->pdstPlantSize, "Peak Sizing Period Name");
        s->pdchPlantSizPkTimeDayOfSim = newPreDefColumn(state, s->pdstPlantSize, "Peak Day into Period {TIMESTAMP}[day]");
        s->pdchPlantSizPkTimeHour = newPreDefColumn(state, s->pdstPlantSize, "Peak Hour Of Day {TIMESTAMP}[hr]");
        s->pdchPlantSizPkTimeMin = newPreDefColumn(state, s->pdstPlantSize, "Peak Step Start Minute {TIMESTAMP}[min]");

        s->pdst2CoilSummaryCoilSelection = newPreDefSubTable(state, s->pdrSizing, "Coil Sizing Summary");
        // coil meta data information
        //    the first column will be the coil name, the unique user name from input. It has no header or column definition
        s->pdch2CoilType = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Type");
        s->pdch2CoilHVACType = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "HVAC Type");
        s->pdch2CoilHVACName = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "HVAC Name");

        // coil Final size summary, regardless of how determined (
        s->pdch2CoilFinalTotalCap = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Final Gross Total Capacity [W]");
        s->pdch2CoilFinalSensCap = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Final Gross Sensible Capacity [W]");
        s->pdch2CoilFinalAirVolFlowRate =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Final Reference Air Volume Flow Rate [m3/s]");
        s->pdch2CoilFinalPlantVolFlowRate =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Final Reference Plant Fluid Volume Flow Rate [m3/s]");
        s->pdch2CoilUA = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil U-value Times Area Value [W/K]");

        // results from regular zone and system sizing calcs, "At Ideal Loads Peak"
        s->pdch2CoilDDnameSensIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Design Day Name at Sensible Ideal Loads Peak");
        s->pdch2CoilDateTimeSensIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Date/Time at Sensible Ideal Loads Peak");
        s->pdch2CoilDDnameAirFlowIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Design Day Name at Air Flow Ideal Loads Peak");
        s->pdch2CoilDateTimeAirFlowIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Date/Time at Air Flow Ideal Loads Peak");

        s->pdch2CoilTotalCapIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Total Capacity at Ideal Loads Peak [W]");
        s->pdch2CoilSensCapIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Sensible Capacity at Ideal Loads Peak [W]");
        s->pdch2CoilAirVolumeFlowIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Air Volume Flow Rate at Ideal Loads Peak [m3/s]");
        s->pdch2CoilEntDryBulbIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Entering Air Drybulb at Ideal Loads Peak [C]");
        s->pdch2CoilEntWetBulbIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Entering Air Wetbulb at Ideal Loads Peak [C]");
        s->pdch2CoilEntHumRatIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Entering Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdch2CoilLvgDryBulbIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Leaving Air Drybulb at Ideal Loads Peak [C]");
        s->pdch2CoilLvgWetBulbIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Leaving Air Wetbulb at Ideal Loads Peak [C]");
        s->pdch2CoilLvgHumRatIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Leaving Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdch2OADryBulbIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Outdoor Air Drybulb at Ideal Loads Peak [C]");
        s->pdch2OAHumRatIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Outdoor Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdch2OAWetBulbatIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Outdoor Air Wetbulb at Ideal Loads Peak [C]");
        s->pdch2OAFlowPrcntIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Outdoor Air Flow Percentage at Ideal Loads Peak [%]");
        s->pdch2ZoneAirDryBulbIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Zone Air Drybulb at Ideal Loads Peak [C]");
        s->pdch2ZoneAirHumRatIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Zone Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdch2ZoneAirRelHumIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Zone Air Relative Humidity at Ideal Loads Peak [%]");
        s->pdch2ZoneSensibleLoadIdealPeak =
            newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Zone Sensible Heat Gain at Ideal Loads Peak [W]");
        s->pdch2ZoneLatentLoadIdealPeak = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Zone Latent Heat Gain at Ideal Loads Peak [W]");
        // results for coil at Rated Conditions
        s->pdch2CoilRatedTotalCap = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Total Capacity at Rating Conditions [W]");
        s->pdch2CoilRatedSensCap = newPreDefColumn(state, s->pdst2CoilSummaryCoilSelection, "Coil Sensible Capacity at Rating Conditions [W]");

        s->pdrCoilSizingDetailsTable = newPreDefReport(state, "CoilSizingDetails", "Coil", "Coil Sizing Details");
        s->pdstCoilSummaryCoilSelection = newPreDefSubTable(state, s->pdrCoilSizingDetailsTable, "Coils");
        // coil meta data information
        //    the first column will be the coil name, the unique user name from input. It has no header or column definition
        s->pdchCoilType = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Type");
        s->pdchCoilLocation = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Location");
        s->pdchCoilHVACType = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "HVAC Type");
        s->pdchCoilHVACName = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "HVAC Name");
        s->pdchCoilZoneName = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Zone Name(s)");

        s->pdchSysSizingMethCoinc = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "System Sizing Method Concurrence");
        s->pdchSysSizingMethCap = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "System Sizing Method Capacity");
        s->pdchSysSizingMethAir = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "System Sizing Method Air Flow");

        s->pdchCoilIsCapAutosized = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Autosized Coil Capacity?");
        s->pdchCoilIsAirFlowAutosized = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Autosized Coil Airflow?");
        s->pdchCoilIsWaterFlowAutosized = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Autosized Coil Water Flow?");
        s->pdchCoilIsOATreated = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "OA Pretreated prior to coil inlet?");

        // coil Final size summary, regardless of how determined (
        // get rid of these, this will be the same as At Rating Conditions.
        s->pdchCoilFinalTotalCap = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Final Gross Total Capacity [W]");
        s->pdchCoilFinalSensCap = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Final Gross Sensible Capacity [W]");
        s->pdchCoilFinalAirVolFlowRate = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Final Reference Air Volume Flow Rate [m3/s]");
        s->pdchCoilFinalPlantVolFlowRate =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Final Reference Plant Fluid Volume Flow Rate [m3/s]");

        // Misc Design output
        s->pdchCoilUA = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil U-value Times Area Value [W/K]");
        s->pdchReheatCoilMultiplier = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Terminal Unit Reheat Coil Multiplier");
        s->pdchFlowCapRatioLowCapIncreaseRatio =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "DX Coil Capacity Increase Ratio from Too Low Flow/Capacity Ratio");
        s->pdchFlowCapRatioHiCapDecreaseRatio =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "DX Coil Capacity Decrease Ratio from Too High Flow/Capacity Ratio");

        s->pdchMoistAirSpecificHeat =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Moist Air Heat Capacity [J/kg-K]"); // standard? for ideal sizing calcs?
        s->pdchDryAirSpecificHeat =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Dry Air Heat Capacity [J/kg-K]"); // standard? for ideal sizing calcs?
        s->pdchStandRhoAir = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Standard Air Density Adjusted for Elevation [kg/m3]");

        // Fan info for coil
        s->pdchFanAssociatedWithCoilName = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Supply Fan Name for Coil");
        s->pdchFanAssociatedWithCoilType = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Supply Fan Type for Coil");
        s->pdchFanAssociatedVdotSize = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Supply Fan Maximum Air Volume Flow Rate [m3/s]");
        s->pdchFanAssociatedMdotSize = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Supply Fan Maximum Air Mass Flow Rate [kg/s]");

        // Plant info for coil
        s->pdchCoilPlantLoopName = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Name for Coil");
        s->pdchPlantFluidSpecificHeat =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Fluid Specific Heat Capacity [J/kg-K]"); // standard/inits ?
        s->pdchPlantFluidDensity =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Fluid Density [kg/m3]"); // standard/inits ? for ideal sizing calcs?
        s->pdchPlantMassFlowMaximum = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Maximum Fluid Mass Flow Rate [kg/s]");
        s->pdchPlantRetTempDesign = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Design Fluid Return Temperature [C]");
        s->pdchPlantSupTempDesign = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Design Fluid Supply Temperature [C]");
        s->pdchPlantDeltaTempDesign = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Design Fluid Temperature Difference [deltaC]");
        s->pdchPlantCapacity = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Plant Design Capacity [W]");
        s->pdchCoilCapPrcntPlantCapacity =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Capacity Percentage of Plant Design Capacity [%]");
        s->pdchCoilFlowPrcntPlantFlow =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Fluid Flow Rate Percentage of Plant Design Flow Rate [%]");

        // results from regular zone and system sizing calcs, "At Ideal Loads Peak"
        s->pdchCoilDDnameSensIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Design Day Name at Sensible Ideal Loads Peak");
        s->pdchCoilDateTimeSensIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Date/Time at Sensible Ideal Loads Peak");
        s->pdchCoilDDnameTotIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Design Day Name at Total Ideal Loads Peak");
        s->pdchCoilDateTimeTotIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Date/Time at Total Ideal Loads Peak");
        s->pdchCoilDDnameAirFlowIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Design Day Name at Air Flow Ideal Loads Peak");
        s->pdchCoilDateTimeAirFlowIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Date/Time at Air Flow Ideal Loads Peak");
        s->pdchCoilPeakLoadTypeToSizeOn = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Peak Load Type to Size On");

        s->pdchCoilTotalCapIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Total Capacity at Ideal Loads Peak [W]");
        s->pdchCoilSensCapIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Sensible Capacity at Ideal Loads Peak [W]");
        s->pdchCoilOffRatingCapacityModifierIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Off-Rating Capacity Modifier at Ideal Loads Peak [ ]");
        s->pdchCoilAirMassFlowIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Air Mass Flow Rate at Ideal Loads Peak [kg/s]");
        s->pdchCoilAirVolumeFlowIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Air Volume Flow Rate at Ideal Loads Peak [m3/s]");
        s->pdchCoilEntDryBulbIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Drybulb at Ideal Loads Peak [C]");
        s->pdchCoilEntWetBulbIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Wetbulb at Ideal Loads Peak [C]");
        s->pdchCoilEntHumRatIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdchCoilEntEnthalpyIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Enthalpy at Ideal Loads Peak [J/kg]");
        s->pdchCoilLvgDryBulbIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Drybulb at Ideal Loads Peak [C]");
        s->pdchCoilLvgWetBulbIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Wetbulb at Ideal Loads Peak [C]");
        s->pdchCoilLvgHumRatIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdchCoilLvgEnthalpyIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Enthalpy at Ideal Loads Peak [J/kg]");
        s->pdchCoilWaterMassFlowIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Plant Fluid Mass Flow Rate at Ideal Loads Peak [kg/s]");
        s->pdchCoilEntWaterTempIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Plant Fluid Temperature at Ideal Loads Peak [C]");
        s->pdchCoilLvgWaterTempIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Plant Fluid Temperature at Ideal Loads Peak [C]");
        s->pdchCoilWaterDeltaTempIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Plant Fluid Temperature Difference at Ideal Loads Peak [deltaC]");
        s->pdchFanHeatGainIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Supply Fan Air Heat Gain at Ideal Loads Peak [W]");
        s->pdchCoilNetTotalCapacityIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil and Fan Net Total Capacity at Ideal Loads Peak [W]");
        s->pdchOADryBulbIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Outdoor Air Drybulb at Ideal Loads Peak [C]");
        s->pdchOAHumRatIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Outdoor Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdchOAWetBulbatIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Outdoor Air Wetbulb at Ideal Loads Peak [C]");
        s->pdchOAVolFlowIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Outdoor Air Volume Flow Rate at Ideal Loads Peak [m3/s]");
        s->pdchOAFlowPrcntIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Outdoor Air Flow Percentage at Ideal Loads Peak [%]");
        s->pdchAirSysRADryBulbIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "System Return Air Drybulb at Ideal Loads Peak [C]");
        s->pdchAirSysRAHumRatIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "System Return Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdchZoneAirDryBulbIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Zone Air Drybulb at Ideal Loads Peak [C]");
        s->pdchZoneAirHumRatIdealPeak =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Zone Air Humidity Ratio at Ideal Loads Peak [kgWater/kgDryAir]");
        s->pdchZoneAirRelHumIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Zone Air Relative Humidity at Ideal Loads Peak [%]");
        s->pdchZoneSensibleLoadIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Zone Sensible Heat Gain at Ideal Loads Peak [W]");
        s->pdchZoneLatentLoadIdealPeak = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Zone Latent Heat Gain at Ideal Loads Peak [W]");
        // results for coil at Rated Conditions
        s->pdchCoilRatedTotalCap = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Total Capacity at Rating Conditions [W]");
        s->pdchCoilRatedSensCap = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Sensible Capacity at Rating Conditions [W]");

        s->pdchCoilRatedAirMass = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Air Mass Flow Rate at Rating Conditions [kg/s]");
        s->pdchCoilRatedEntDryBulb = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Drybulb at Rating Conditions [C]");
        s->pdchCoilRatedEntWetBulb = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Wetbulb at Rating Conditions [C]");
        s->pdchCoilRatedEntHumRat =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Humidity Ratio at Rating Conditions [kgWater/kgDryAir]");
        s->pdchCoilRatedEntEnthalpy =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Entering Air Enthalpy at Rating Conditions [J/kg]");
        s->pdchCoilRatedLvgDryBulb = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Drybulb at Rating Conditions [C]");
        s->pdchCoilRatedLvgWetBulb = newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Wetbulb at Rating Conditions [C]");
        s->pdchCoilRatedLvgHumRat =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Humidity Ratio at Rating Conditions [kgWater/kgDryAir]");
        s->pdchCoilRatedLvgEnthalpy =
            newPreDefColumn(state, s->pdstCoilSummaryCoilSelection, "Coil Leaving Air Enthalpy at Rating Conditions [J/kg]");

        // Std 229 New Table "Coil Connections"
        s->pdstCoilConnections = newPreDefSubTable(state, s->pdrCoilSizingDetailsTable, "Coil Connections");
        // coil connections information
        s->pdchCoilName_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Coil Name");
        s->pdchCoilType_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Coil Type");
        s->pdchCoilLoc_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Coil Location");
        s->pdchCoilHVACType_CCs = newPreDefColumn(state, s->pdstCoilConnections, "HVAC Type");
        s->pdchCoilHVACName_CCs = newPreDefColumn(state, s->pdstCoilConnections, "HVAC Name");
        s->pdchCoilZoneNames_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Zone Name(s)");
        s->pdchCoilSupFanName_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Supply Fan Name for HVAC");
        s->pdchCoilSupFanType_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Supply Fan Type for HVAC");
        s->pdchCoilAirloopName_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Airloop Name");
        s->pdchCoilPlantName_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Plant Name for Coil");
        s->pdchCoilPlantloopName_CCs = newPreDefColumn(state, s->pdstCoilConnections, "Plant Loop Name");

        // System Summary Report

        s->pdrSystem = newPreDefReport(state, "SystemSummary", "Sys", "System Summary");

        s->pdstEconomizer = newPreDefSubTable(state, s->pdrSystem, "Economizer");

        s->pdchEcoKind = newPreDefColumn(state, s->pdstEconomizer, "High Limit Shutoff Control");
        s->pdchEcoMinOA = newPreDefColumn(state, s->pdstEconomizer, "Minimum Outdoor Air [m3/s]");
        s->pdchEcoMaxOA = newPreDefColumn(state, s->pdstEconomizer, "Maximum Outdoor Air [m3/s]");
        s->pdchEcoRetTemp = newPreDefColumn(state, s->pdstEconomizer, "Return Air Temp Limit");
        s->pdchEcoRetEnth = newPreDefColumn(state, s->pdstEconomizer, "Return Air Enthalpy Limit");
        s->pdchEcoOATempLim = newPreDefColumn(state, s->pdstEconomizer, "Outdoor Air Temperature Limit [C]");
        s->pdchEcoOAEnthLim = newPreDefColumn(state, s->pdstEconomizer, "Outdoor Air Enthalpy Limit [C]");

        s->pdstDemCntlVent = newPreDefSubTable(state, s->pdrSystem, "Demand Controlled Ventilation using Controller:MechanicalVentilation");
        s->pdchDCVventMechName = newPreDefColumn(state, s->pdstDemCntlVent, "Controller:MechanicalVentilation Name");
        s->pdchDCVperPerson = newPreDefColumn(state, s->pdstDemCntlVent, "Outdoor Air Per Person [m3/s-person]");
        s->pdchDCVperArea = newPreDefColumn(state, s->pdstDemCntlVent, "Outdoor Air Per Area [m3/s-m2]");
        s->pdchDCVperZone = newPreDefColumn(state, s->pdstDemCntlVent, "Outdoor Air Per Zone [m3/s]");
        s->pdchDCVperACH = newPreDefColumn(state, s->pdstDemCntlVent, "Outdoor Air ACH [ach]");
        s->pdchDCVMethod = newPreDefColumn(state, s->pdstDemCntlVent, "Outdoor Air Method");
        s->pdchDCVOASchName = newPreDefColumn(state, s->pdstDemCntlVent, "Outdoor Air Schedule Name");
        // added for new DCV
        s->pdchDCVZoneADEffCooling = newPreDefColumn(state, s->pdstDemCntlVent, "Air Distribution Effectiveness in Cooling Mode");
        s->pdchDCVZoneADEffHeating = newPreDefColumn(state, s->pdstDemCntlVent, "Air Distribution Effectiveness in Heating Mode");
        s->pdchDCVZoneADEffSchName = newPreDefColumn(state, s->pdstDemCntlVent, "Air Distribution Effectiveness Schedule Name");
        s->pdchDCVType = newPreDefColumn(state, s->pdstDemCntlVent, "Type");

        s->pdstSimpleComfort = newPreDefSubTable(state, s->pdrSystem, "Time Not Comfortable Based on Simple ASHRAE 55-2004");
        s->pdchSCwinterClothes = newPreDefColumn(state, s->pdstSimpleComfort, "Winter Clothes [hr]");
        s->pdchSCsummerClothes = newPreDefColumn(state, s->pdstSimpleComfort, "Summer Clothes [hr]");
        s->pdchSCeitherClothes = newPreDefColumn(state, s->pdstSimpleComfort, "Summer or Winter Clothes [hr]");

        s->pdstUnmetLoads = newPreDefSubTable(state, s->pdrSystem, "Time Setpoint Not Met");
        s->pdchULnotMetHeat = newPreDefColumn(state, s->pdstUnmetLoads, "During Heating [hr]");
        s->pdchULnotMetCool = newPreDefColumn(state, s->pdstUnmetLoads, "During Cooling [hr]");
        s->pdchULnotMetHeatOcc = newPreDefColumn(state, s->pdstUnmetLoads, "During Occupied Heating [hr]");
        s->pdchULnotMetCoolOcc = newPreDefColumn(state, s->pdstUnmetLoads, "During Occupied Cooling [hr]");

        s->pdstStatSchd = newPreDefSubTable(state, s->pdrSystem, "Thermostat Schedules");
        s->pdchStatName = newPreDefColumn(state, s->pdstStatSchd, "Thermostat Name 1");
        s->pdchStatCtrlTypeSchd = newPreDefColumn(state, s->pdstStatSchd, "Control Type Schedule");
        s->pdchStatSchdType1 = newPreDefColumn(state, s->pdstStatSchd, "Control Type");
        s->pdchStatSchdTypeName1 = newPreDefColumn(state, s->pdstStatSchd, "Control Type Name");
        s->pdchStatSchdHeatName = newPreDefColumn(state, s->pdstStatSchd, "Heating Schedule");
        s->pdchStatSchdCoolName = newPreDefColumn(state, s->pdstStatSchd, "Cooling Schedule");

        // HVAC Topology Report
        s->pdrTopology = newPreDefReport(state, "HVACTopology", "Top", "HVAC Topology");

        s->pdstTopAirLoop = newPreDefSubTable(state, s->pdrTopology, "Air Loop Supply Side Component Arrangement");
        s->pdchTopAirLoopName = newPreDefColumn(state, s->pdstTopAirLoop, "Airloop Name");
        s->pdchTopAirSplitName = newPreDefColumn(state, s->pdstTopAirLoop, "Splitter Name");
        s->pdchTopAirBranchName = newPreDefColumn(state, s->pdstTopAirLoop, "Supply Branch Name");
        s->pdchTopAirSupplyBranchType = newPreDefColumn(state, s->pdstTopAirLoop, "Supply Branch Type");
        s->pdchTopAirCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Component Type");
        s->pdchTopAirCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Component Name");
        s->pdchTopAirSubCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Component Type");
        s->pdchTopAirSubCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Component Name");
        s->pdchTopAirSubSubCompType = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Sub-Component Type");
        s->pdchTopAirSubSubCompName = newPreDefColumn(state, s->pdstTopAirLoop, "Sub-Sub-Component Name");
        s->pdchTopAirMixName = newPreDefColumn(state, s->pdstTopAirLoop, "Mixer Name");

        s->pdstTopAirDemand = newPreDefSubTable(state, s->pdrTopology, "Air Loop Demand Side Component Arrangement");
        s->pdchTopAirDemandName = newPreDefColumn(state, s->pdstTopAirDemand, "Airloop Name");
        s->pdchTopAirSupplyBranchName = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Branch Name");
        s->pdchTopAirSupplyDuctType = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Branch Type");
        s->pdchTopAirSupplyPCompType = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Path Component Type");
        s->pdchTopAirSupplyPCompName = newPreDefColumn(state, s->pdstTopAirDemand, "Supply Path Component Name");
        s->pdchTopAirTermUnitType = newPreDefColumn(state, s->pdstTopAirDemand, "Terminal Unit Type");
        s->pdchTopAirTermUnitName = newPreDefColumn(state, s->pdstTopAirDemand, "Terminal Unit Name");
        s->pdchTopAirZoneName = newPreDefColumn(state, s->pdstTopAirDemand, "Zone Name");
        s->pdchTopAirReturnPCompType = newPreDefColumn(state, s->pdstTopAirDemand, "Return Path Component Type");
        s->pdchTopAirReturnPCompName = newPreDefColumn(state, s->pdstTopAirDemand, "Return Path Component Name");

        s->pdstTopZnEqp = newPreDefSubTable(state, s->pdrTopology, "Zone Equipment Component Arrangement");
        s->pdchTopZnEqpName = newPreDefColumn(state, s->pdstTopZnEqp, "Zone Name");
        s->pdchTopZnEqpCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Component Type");
        s->pdchTopZnEqpCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Component Name");
        s->pdchTopZnEqpSubCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Component Type");
        s->pdchTopZnEqpSubCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Component Name");
        s->pdchTopZnEqpSubSubCompType = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Sub-Component Type");
        s->pdchTopZnEqpSubSubCompName = newPreDefColumn(state, s->pdstTopZnEqp, "Sub-Sub-Component Name");

        s->pdstTopPlantLoop2 = newPreDefSubTable(state, s->pdrTopology, "Plant Loop Component Arrangement");
        s->pdchTopPlantLoopType2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Loop Type");
        s->pdchTopPlantLoopName2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Loop Name");
        s->pdchTopPlantSide2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Side");
        s->pdchTopPlantSplitName2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Splitter Name");
        s->pdchTopPlantBranchName2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Branch Name");
        s->pdchTopPlantCompType2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Component Type");
        s->pdchTopPlantCompName2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Component Name");
        s->pdchTopPlantMixName2 = newPreDefColumn(state, s->pdstTopPlantLoop2, "Mixer Name");

        // Outdoor Air Report
        s->pdrOutsideAir = newPreDefReport(state, "OutdoorAirSummary", "OA", "Outdoor Air Summary");

        s->pdstOAavgOcc = newPreDefSubTable(state, s->pdrOutsideAir, "Average Outdoor Air During Occupied Hours");

        s->pdchOaoAvgNumOcc1 = newPreDefColumn(state, s->pdstOAavgOcc, "Average Number of Occupants");
        s->pdchOaoNomNumOcc1 = newPreDefColumn(state, s->pdstOAavgOcc, "Nominal Number of Occupants");
        s->pdchOaoZoneVol1 = newPreDefColumn(state, s->pdstOAavgOcc, "Zone Volume [m3]");
        s->pdchOaoAvgMechVent = newPreDefColumn(state, s->pdstOAavgOcc, "Mechanical Ventilation [ach]");
        s->pdchOaoAvgInfil = newPreDefColumn(state, s->pdstOAavgOcc, "Infiltration [ach]");
        s->pdchOaoAvgAFNInfil = newPreDefColumn(state, s->pdstOAavgOcc, "AFN Infiltration [ach]");
        s->pdchOaoAvgSimpVent = newPreDefColumn(state, s->pdstOAavgOcc, "Simple Ventilation [ach]");
        // s->pdchOaoAvgTotVent =   newPreDefColumn(state, s->pdstOAavgOcc,'Total Ventilation [ach]')

        addFootNoteSubTable(state, s->pdstOAavgOcc, "Values shown for a single zone without multipliers");

        s->pdstOAminOcc = newPreDefSubTable(state, s->pdrOutsideAir, "Minimum Outdoor Air During Occupied Hours");

        s->pdchOaoAvgNumOcc2 = newPreDefColumn(state, s->pdstOAminOcc, "Average Number of Occupants");
        s->pdchOaoNomNumOcc2 = newPreDefColumn(state, s->pdstOAminOcc, "Nominal Number of Occupants");
        s->pdchOaoZoneVol2 = newPreDefColumn(state, s->pdstOAminOcc, "Zone Volume [m3]");
        s->pdchOaoMinMechVent = newPreDefColumn(state, s->pdstOAminOcc, "Mechanical Ventilation [ach]");
        s->pdchOaoMinInfil = newPreDefColumn(state, s->pdstOAminOcc, "Infiltration [ach]");
        s->pdchOaoMinAFNInfil = newPreDefColumn(state, s->pdstOAminOcc, "AFN Infiltration [ach]");
        s->pdchOaoMinSimpVent = newPreDefColumn(state, s->pdstOAminOcc, "Simple Ventilation [ach]");
        // s->pdchOaoMinTotVent =   newPreDefColumn(state, s->pdstOAminOcc,'Total Ventilation [ach]')
        addFootNoteSubTable(state, s->pdstOAminOcc, "Values shown for a single zone without multipliers");

        // Outdoor Air Details Report
        s->pdrOutsideAirDetails = newPreDefReport(state, "OutdoorAirDetails", "OAD", "Outdoor Air Details");

        s->pdstOAmechVentParByZone = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Mechanical Ventilation Parameters by Zone");
        s->pdchOaMvAirLpNm = newPreDefColumn(state, s->pdstOAmechVentParByZone, "AirLoop Name");
        s->pdchOaMvAvgNumOcc = newPreDefColumn(state, s->pdstOAmechVentParByZone, "Average Number of Occupants");
        s->pdchOaMvNomNumOcc = newPreDefColumn(state, s->pdstOAmechVentParByZone, "Nominal Number of Occupants");
        s->pdchOaMvZoneVol = newPreDefColumn(state, s->pdstOAmechVentParByZone, "Zone Volume [m3]");
        s->pdchOaMvZoneArea = newPreDefColumn(state, s->pdstOAmechVentParByZone, "Zone Area [m2]");
        s->pdchOaMvDesZnOa = newPreDefColumn(state, s->pdstOAmechVentParByZone, "Design Zone Outdoor Airflow - Voz [m3/s]");
        s->pdchOaMvMinDynTrgVent = newPreDefColumn(state, s->pdstOAmechVentParByZone, "Minimum Dynamic Target Ventilation - Voz-dyn-min [m3/s]");
        addFootNoteSubTable(
            state, s->pdstOAmechVentParByZone, "Values shown for a single zone without multipliers. Total Facility includes multipliers.");

        s->pdstOAtotAirByZone = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Total Outdoor Air by Zone");
        s->pdchOaTaBzMechVent = newPreDefColumn(state, s->pdstOAtotAirByZone, "Mechanical Ventilation [m3]");
        s->pdchOaTaBzNatVent = newPreDefColumn(state, s->pdstOAtotAirByZone, "Natural Ventilation [m3]");
        s->pdchOaTaBzTotVent = newPreDefColumn(state, s->pdstOAtotAirByZone, "Total Ventilation [m3]");
        s->pdchOaTaBzInfil = newPreDefColumn(state, s->pdstOAtotAirByZone, "Infiltration [m3]");
        s->pdchOaTaBzTotVentInfil = newPreDefColumn(state, s->pdstOAtotAirByZone, "Total Ventilation and Infiltration [m3]");
        s->pdchOaTaBzDynTrgVent = newPreDefColumn(state, s->pdstOAtotAirByZone, "Dynamic Target Ventilation - Voz-dyn [m3]");
        s->pdchOaTaBzTmBelow = newPreDefColumn(state, s->pdstOAtotAirByZone, "Time Below Voz-dyn [hr]");
        s->pdchOaTaBzTmAt = newPreDefColumn(state, s->pdstOAtotAirByZone, "Time At Voz-dyn [hr]");
        s->pdchOaTaBzTmAbove = newPreDefColumn(state, s->pdstOAtotAirByZone, "Time Above Voz-dyn [hr]");
        s->pdchOaTaBzTmAboveUnocc = newPreDefColumn(state, s->pdstOAtotAirByZone, "Time Above Zero When Unoccupied [hr]");
        addFootNoteSubTable(state, s->pdstOAtotAirByZone, "Values shown for a single zone without multipliers. Total Facility includes multipliers.");

        s->pdstOAavgOccByZone = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Average Outdoor Air During Occupancy by Zone - Flow Rates");
        s->pdchOaOccBzMechVent = newPreDefColumn(state, s->pdstOAavgOccByZone, "Mechanical Ventilation [m3/s]");
        s->pdchOaOccBzNatVent = newPreDefColumn(state, s->pdstOAavgOccByZone, "Natural Ventilation [m3/s]");
        s->pdchOaOccBzTotVent = newPreDefColumn(state, s->pdstOAavgOccByZone, "Total Ventilation [m3/s]");
        s->pdchOaOccBzInfil = newPreDefColumn(state, s->pdstOAavgOccByZone, "Infiltration [m3/s]");
        s->pdchOaOccBzTotVentInfil = newPreDefColumn(state, s->pdstOAavgOccByZone, "Total Ventilation and Infiltration [m3/s]");
        s->pdchOaOccBzDynTrgVent = newPreDefColumn(state, s->pdstOAavgOccByZone, "Dynamic Target Ventilation - Voz-dyn [m3/s]");
        s->pdchOaOccBzTmBelow = newPreDefColumn(state, s->pdstOAavgOccByZone, "Time Below Voz-dyn [hr]");
        s->pdchOaOccBzTmAt = newPreDefColumn(state, s->pdstOAavgOccByZone, "Time At Voz-dyn [hr]");
        s->pdchOaOccBzTmAbove = newPreDefColumn(state, s->pdstOAavgOccByZone, "Time Above Voz-dyn [hr]");
        addFootNoteSubTable(state, s->pdstOAavgOccByZone, "Values shown for a single zone without multipliers. Total Facility includes multipliers.");

        s->pdstOAtotAirByLoop = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Total Outdoor Air by AirLoop");
        s->pdchOaTaAlMechVent = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Mechanical Ventilation [m3]");
        s->pdchOaTaAlNatVent = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Natural Ventilation [m3]");
        s->pdchOaTaAlTotVent = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Total Ventilation [m3]");
        s->pdchOaTaAlSumDynTrgVent = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Sum Zone Dynamic Target Ventilation - Voz-sum-dyn [m3]");
        s->pdchOaTaAlTmBelow = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Time Below Voz-sum-dyn [hr]");
        s->pdchOaTaAlTmAt = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Time At Voz-sum-dyn [hr]");
        s->pdchOaTaAlTmAbove = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Time Above Voz-sum-dyn [hr]");
        s->pdchOaTaAlTmAboveUnocc = newPreDefColumn(state, s->pdstOAtotAirByLoop, "Time Above Zero When Unoccupied [hr]");
        addFootNoteSubTable(state, s->pdstOAtotAirByLoop, "Values shown include multipliers.");

        s->pdstOAavgOccByLoop = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Average Outdoor Air During Occupancy by AirLoop");
        s->pdchOaOccAlMechVent = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Mechanical Ventilation [m3/s]");
        s->pdchOaOccAlNatVent = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Natural Ventilation [m3/s]");
        s->pdchOaOccAlTotVent = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Total Ventilation [m3/s]");
        s->pdchOaOccAlSumDynTrgVent = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Sum Zone Dynamic Target Ventilation - Voz-sum-dyn [m3/s]");
        s->pdchOaOccAlTmBelow = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Time Below Voz-sum-dyn [hr]");
        s->pdchOaOccAlTmAt = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Time At Voz-sum-dyn [hr]");
        s->pdchOaOccAlTmAbove = newPreDefColumn(state, s->pdstOAavgOccByLoop, "Time Above Voz-sum-dyn [hr]");
        addFootNoteSubTable(state, s->pdstOAavgOccByLoop, "Values shown include multipliers.");

        s->pdstOAtimeFactorsDurOcc = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Outdoor Air Controller Limiting Factors by AirLoop");
        s->pdchOaTmFctNoLimit = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "No Limiting Factor [hr]");        // todo
        s->pdchOaTmFctLimit = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Limits and Scheduled Limits [hr]"); // todo
        s->pdchOaTmFctEcono = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Economizer [hr]");                  // todo
        s->pdchOaTmFctExhaust = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Exhaust Flow [hr]");              // todo
        s->pdchOaTmFctMixedLimit = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Mixed Air Flow [hr]");         // todo
        s->pdchOaTmFctHiHumid = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "High Humidity [hr]");             // todo
        s->pdchOaTmFctDCV = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Demand Controlled Ventilation [hr]"); // todo
        s->pdchOaTmFctNiteVent = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Night Ventilation [hr]");        // todo
        s->pdchOaTmFctDemand = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Demand Limiting [hr]");            // todo
        s->pdchOaTmFctEMS = newPreDefColumn(state, s->pdstOAtimeFactorsDurOcc, "Energy Management System [hr]");      // todo

        s->pdstOAavgFactorsDurOcc = newPreDefSubTable(state, s->pdrOutsideAirDetails, "Average Outdoor Air for Limiting Factors During Occupancy");
        s->pdchOaAvFctNoLimit = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "No Limiting Factor [m3/s]");        // todo
        s->pdchOaAvFctLimit = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Limits and Scheduled Limits [m3/s]"); // todo
        s->pdchOaAvFctEcono = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Economizer [m3/s]");                  // todo
        s->pdchOaAvFctExhaust = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Exhaust Flow [m3/s]");              // todo
        s->pdchOaAvFctMixedLimit = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Mixed Air Flow [m3/s]");         // todo
        s->pdchOaAvFctHiHumid = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "High Humidity [m3/s]");             // todo
        s->pdchOaAvFctDCV = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Demand Controlled Ventilation [m3/s]"); // todo
        s->pdchOaAvFctNiteVent = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Night Ventilation [m3/s]");        // todo
        s->pdchOaAvFctDemand = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Demand Limiting [m3/s]");            // todo
        s->pdchOaAvFctEMS = newPreDefColumn(state, s->pdstOAavgFactorsDurOcc, "Energy Management System [m3/s]");      // todo
        addFootNoteSubTable(state, s->pdstOAavgFactorsDurOcc, "Values shown include multipliers.");

        // Object Count Report
        s->pdrObjCnt = newPreDefReport(state, "ObjectCountSummary", "Count", "Object Count Summary");

        s->pdstSurfCnt = newPreDefSubTable(state, s->pdrObjCnt, "Surfaces by Class");
        s->pdchSurfCntTot = newPreDefColumn(state, s->pdstSurfCnt, "Total");
        s->pdchSurfCntExt = newPreDefColumn(state, s->pdstSurfCnt, "Outdoors");

        s->pdstHVACcnt = newPreDefSubTable(state, s->pdrObjCnt, "HVAC");
        s->pdchHVACcntVal = newPreDefColumn(state, s->pdstHVACcnt, "Count");

        s->pdstFieldCnt = newPreDefSubTable(state, s->pdrObjCnt, "Input Fields");
        s->pdchFieldCntVal = newPreDefColumn(state, s->pdstFieldCnt, "Count");

        // Energy Meters report
        s->pdrEnergyMeters = newPreDefReport(state, "EnergyMeters", "Meters", "Energy Meters");

        // s->pdstEMvalues = newPreDefSubTable(state, s->pdrEnergyMeters,'Annual and Peak Values')
        // s->pdchEMannual = newPreDefColumn(state, s->pdstEMvalues,'Annual Value [GJ]')
        // s->pdchEMminvalue = newPreDefColumn(state, s->pdstEMvalues,'Minimum Value [J]')
        // s->pdchEMminvaluetime = newPreDefColumn(state, s->pdstEMvalues,'Timestamp of Minimum')
        // s->pdchEMmaxvalue = newPreDefColumn(state, s->pdstEMvalues,'Maximum Value [J]')
        // s->pdchEMmaxvaluetime = newPreDefColumn(state, s->pdstEMvalues,'Timestamp of Maximum')
        // Electricity Sub Table
        s->pdstEMelecvalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Electricity");
        addFootNoteSubTable(
            state, s->pdstEMelecvalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMelecannual = newPreDefColumn(state, s->pdstEMelecvalues, "Electricity Annual Value [GJ]");
        s->pdchEMelecminvalue = newPreDefColumn(state, s->pdstEMelecvalues, "Electricity Minimum Value [W]");
        s->pdchEMelecminvaluetime = newPreDefColumn(state, s->pdstEMelecvalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMelecmaxvalue = newPreDefColumn(state, s->pdstEMelecvalues, "Electricity Maximum Value [W]");
        s->pdchEMelecmaxvaluetime = newPreDefColumn(state, s->pdstEMelecvalues, "Timestamp of Maximum {TIMESTAMP}");

        // Gas Sub Table
        s->pdstEMgasvalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Natural Gas");
        addFootNoteSubTable(
            state, s->pdstEMgasvalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMgasannual = newPreDefColumn(state, s->pdstEMgasvalues, "Natural Gas Annual Value [GJ]");
        s->pdchEMgasminvalue = newPreDefColumn(state, s->pdstEMgasvalues, "Natural Gas Minimum Value [W]");
        s->pdchEMgasminvaluetime = newPreDefColumn(state, s->pdstEMgasvalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMgasmaxvalue = newPreDefColumn(state, s->pdstEMgasvalues, "Natural Gas Maximum Value [W]");
        s->pdchEMgasmaxvaluetime = newPreDefColumn(state, s->pdstEMgasvalues, "Timestamp of Maximum {TIMESTAMP}");

        // Cool SubTable
        s->pdstEMcoolvalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Cooling");
        addFootNoteSubTable(
            state, s->pdstEMcoolvalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMcoolannual = newPreDefColumn(state, s->pdstEMcoolvalues, "Cooling Annual Value [GJ]");
        s->pdchEMcoolminvalue = newPreDefColumn(state, s->pdstEMcoolvalues, "Cooling Minimum Value [W]");
        s->pdchEMcoolminvaluetime = newPreDefColumn(state, s->pdstEMcoolvalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMcoolmaxvalue = newPreDefColumn(state, s->pdstEMcoolvalues, "Cooling Maximum Value [W]");
        s->pdchEMcoolmaxvaluetime = newPreDefColumn(state, s->pdstEMcoolvalues, "Timestamp of Maximum {TIMESTAMP}");

        // Water SubTable
        s->pdstEMwatervalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Water");
        addFootNoteSubTable(
            state, s->pdstEMwatervalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMwaterannual = newPreDefColumn(state, s->pdstEMwatervalues, "Annual Value [m3]");
        s->pdchEMwaterminvalue = newPreDefColumn(state, s->pdstEMwatervalues, "Minimum Value [m3/s]");
        s->pdchEMwaterminvaluetime = newPreDefColumn(state, s->pdstEMwatervalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMwatermaxvalue = newPreDefColumn(state, s->pdstEMwatervalues, "Maximum Value [m3/s]");
        s->pdchEMwatermaxvaluetime = newPreDefColumn(state, s->pdstEMwatervalues, "Timestamp of Maximum {TIMESTAMP}");

        // Other KG SubTable
        s->pdstEMotherKGvalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Other by Weight/Mass");
        addFootNoteSubTable(
            state, s->pdstEMotherKGvalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMotherKGannual = newPreDefColumn(state, s->pdstEMotherKGvalues, "Annual Value [kg]");
        s->pdchEMotherKGminvalue = newPreDefColumn(state, s->pdstEMotherKGvalues, "Minimum Value [kg/s]");
        s->pdchEMotherKGminvaluetime = newPreDefColumn(state, s->pdstEMotherKGvalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMotherKGmaxvalue = newPreDefColumn(state, s->pdstEMotherKGvalues, "Maximum Value [kg/s]");
        s->pdchEMotherKGmaxvaluetime = newPreDefColumn(state, s->pdstEMotherKGvalues, "Timestamp of Maximum {TIMESTAMP}");

        // Other M3 SubTable
        s->pdstEMotherM3values = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Other Volumetric");
        addFootNoteSubTable(
            state, s->pdstEMotherM3values, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMotherM3annual = newPreDefColumn(state, s->pdstEMotherM3values, "Annual Value [m3]");
        s->pdchEMotherM3minvalue = newPreDefColumn(state, s->pdstEMotherM3values, "Minimum Value [m3/s]");
        s->pdchEMotherM3minvaluetime = newPreDefColumn(state, s->pdstEMotherM3values, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMotherM3maxvalue = newPreDefColumn(state, s->pdstEMotherM3values, "Maximum Value [m3/s]");
        s->pdchEMotherM3maxvaluetime = newPreDefColumn(state, s->pdstEMotherM3values, "Timestamp of Maximum {TIMESTAMP}");

        // Other M3 SubTable
        s->pdstEMotherLvalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Other Liquid/Gas");
        addFootNoteSubTable(
            state, s->pdstEMotherLvalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMotherLannual = newPreDefColumn(state, s->pdstEMotherLvalues, "Annual Value [L]");
        s->pdchEMotherLminvalue = newPreDefColumn(state, s->pdstEMotherLvalues, "Minimum Value [L]");
        s->pdchEMotherLminvaluetime = newPreDefColumn(state, s->pdstEMotherLvalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMotherLmaxvalue = newPreDefColumn(state, s->pdstEMotherLvalues, "Maximum Value [L]");
        s->pdchEMotherLmaxvaluetime = newPreDefColumn(state, s->pdstEMotherLvalues, "Timestamp of Maximum {TIMESTAMP}");

        // Other J SubTable
        s->pdstEMotherJvalues = newPreDefSubTable(state, s->pdrEnergyMeters, "Annual and Peak Values - Other");
        addFootNoteSubTable(
            state, s->pdstEMotherJvalues, "Values shown are for all completed run periods - including any simulations run during sizing periods");
        s->pdchEMotherJannual = newPreDefColumn(state, s->pdstEMotherJvalues, "Annual Value [GJ]");
        s->pdchEMotherJminvalue = newPreDefColumn(state, s->pdstEMotherJvalues, "Minimum Value [W]");
        s->pdchEMotherJminvaluetime = newPreDefColumn(state, s->pdstEMotherJvalues, "Timestamp of Minimum {TIMESTAMP}");
        s->pdchEMotherJmaxvalue = newPreDefColumn(state, s->pdstEMotherJvalues, "Maximum Value [W]");
        s->pdchEMotherJmaxvaluetime = newPreDefColumn(state, s->pdstEMotherJvalues, "Timestamp of Maximum {TIMESTAMP}");

        // Sensible Heat Gain Component Report
        s->pdrSensibleGain = newPreDefReport(state, "SensibleHeatGainSummary", "SHGS", "Sensible Heat Gain Summary");

        s->pdstSHGSannual = newPreDefSubTable(state, s->pdrSensibleGain, "Annual Building Sensible Heat Gain Components");

        s->pdchSHGSAnZoneEqHt = newPreDefColumn(state, s->pdstSHGSannual, "HVAC Zone Eq & Other Sensible Air Heating [GJ]");
        s->pdchSHGSAnZoneEqCl = newPreDefColumn(state, s->pdstSHGSannual, "HVAC Zone Eq & Other Sensible Air Cooling [GJ]");
        s->pdchSHGSAnHvacATUHt = newPreDefColumn(state, s->pdstSHGSannual, "HVAC Terminal Unit Sensible Air Heating [GJ]");
        s->pdchSHGSAnHvacATUCl = newPreDefColumn(state, s->pdstSHGSannual, "HVAC Terminal Unit Sensible Air Cooling [GJ]");
        s->pdchSHGSAnSurfHt = newPreDefColumn(state, s->pdstSHGSannual, "HVAC Input Heated Surface Heating [GJ]");
        s->pdchSHGSAnSurfCl = newPreDefColumn(state, s->pdstSHGSannual, "HVAC Input Cooled Surface Cooling [GJ]");
        s->pdchSHGSAnPeoplAdd = newPreDefColumn(state, s->pdstSHGSannual, "People Sensible Heat Addition [GJ]");
        s->pdchSHGSAnLiteAdd = newPreDefColumn(state, s->pdstSHGSannual, "Lights Sensible Heat Addition [GJ]");
        s->pdchSHGSAnEquipAdd = newPreDefColumn(state, s->pdstSHGSannual, "Equipment Sensible Heat Addition [GJ]");
        s->pdchSHGSAnWindAdd = newPreDefColumn(state, s->pdstSHGSannual, "Window Heat Addition [GJ]");
        s->pdchSHGSAnIzaAdd = newPreDefColumn(state, s->pdstSHGSannual, "Interzone Air Transfer Heat Addition [GJ]");
        s->pdchSHGSAnInfilAdd = newPreDefColumn(state, s->pdstSHGSannual, "Infiltration Heat Addition [GJ]");
        s->pdchSHGSAnOtherAdd = newPreDefColumn(state, s->pdstSHGSannual, "Opaque Surface Conduction and Other Heat Addition [GJ]");
        s->pdchSHGSAnEquipRem = newPreDefColumn(state, s->pdstSHGSannual, "Equipment Sensible Heat Removal [GJ]");
        s->pdchSHGSAnWindRem = newPreDefColumn(state, s->pdstSHGSannual, "Window Heat Removal [GJ]");
        s->pdchSHGSAnIzaRem = newPreDefColumn(state, s->pdstSHGSannual, "Interzone Air Transfer Heat Removal [GJ]");
        s->pdchSHGSAnInfilRem = newPreDefColumn(state, s->pdstSHGSannual, "Infiltration Heat Removal [GJ]");
        s->pdchSHGSAnOtherRem = newPreDefColumn(state, s->pdstSHGSannual, "Opaque Surface Conduction and Other Heat Removal [GJ]");

        s->pdstSHGSpkCl = newPreDefSubTable(state, s->pdrSensibleGain, "Peak Cooling Sensible Heat Gain Components");

        s->pdchSHGSClTimePeak = newPreDefColumn(state, s->pdstSHGSpkCl, "Time of Peak {TIMESTAMP}");
        s->pdchSHGSClHvacHt = newPreDefColumn(state, s->pdstSHGSpkCl, "HVAC Zone Eq & Other Sensible Air Heating [W]");
        s->pdchSHGSClHvacCl = newPreDefColumn(state, s->pdstSHGSpkCl, "HVAC Zone Eq & Other Sensible Air Cooling [W]");
        s->pdchSHGSClHvacATUHt = newPreDefColumn(state, s->pdstSHGSpkCl, "HVAC Terminal Unit Sensible Air Heating [W]");
        s->pdchSHGSClHvacATUCl = newPreDefColumn(state, s->pdstSHGSpkCl, "HVAC Terminal Unit Sensible Air Cooling [W]");
        s->pdchSHGSClSurfHt = newPreDefColumn(state, s->pdstSHGSpkCl, "HVAC Input Heated Surface Heating [W]");
        s->pdchSHGSClSurfCl = newPreDefColumn(state, s->pdstSHGSpkCl, "HVAC Input Cooled Surface Cooling [W]");
        s->pdchSHGSClPeoplAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "People Sensible Heat Addition [W]");
        s->pdchSHGSClLiteAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "Lights Sensible Heat Addition [W]");
        s->pdchSHGSClEquipAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "Equipment Sensible Heat Addition [W]");
        s->pdchSHGSClWindAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "Window Heat Addition [W]");
        s->pdchSHGSClIzaAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "Interzone Air Transfer Heat Addition [W]");
        s->pdchSHGSClInfilAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "Infiltration Heat Addition [W]");
        s->pdchSHGSClOtherAdd = newPreDefColumn(state, s->pdstSHGSpkCl, "Opaque Surface Conduction and Other Heat Addition [W]");
        s->pdchSHGSClEquipRem = newPreDefColumn(state, s->pdstSHGSpkCl, "Equipment Sensible Heat Removal [W]");
        s->pdchSHGSClWindRem = newPreDefColumn(state, s->pdstSHGSpkCl, "Window Heat Removal [W]");
        s->pdchSHGSClIzaRem = newPreDefColumn(state, s->pdstSHGSpkCl, "Interzone Air Transfer Heat Removal [W]");
        s->pdchSHGSClInfilRem = newPreDefColumn(state, s->pdstSHGSpkCl, "Infiltration Heat Removal [W]");
        s->pdchSHGSClOtherRem = newPreDefColumn(state, s->pdstSHGSpkCl, "Opaque Surface Conduction and Other Heat Removal [W]");

        s->pdstSHGSpkHt = newPreDefSubTable(state, s->pdrSensibleGain, "Peak Heating Sensible Heat Gain Components");

        s->pdchSHGSHtTimePeak = newPreDefColumn(state, s->pdstSHGSpkHt, "Time of Peak {TIMESTAMP}");
        s->pdchSHGSHtHvacHt = newPreDefColumn(state, s->pdstSHGSpkHt, "HVAC Zone Eq & Other Sensible Air Heating [W]");
        s->pdchSHGSHtHvacCl = newPreDefColumn(state, s->pdstSHGSpkHt, "HVAC Zone Eq & Other Sensible Air Cooling [W]");
        s->pdchSHGSHtHvacATUHt = newPreDefColumn(state, s->pdstSHGSpkHt, "HVAC Terminal Unit Sensible Air Heating [W]");
        s->pdchSHGSHtHvacATUCl = newPreDefColumn(state, s->pdstSHGSpkHt, "HVAC Terminal Unit Sensible Air Cooling [W]");
        s->pdchSHGSHtSurfHt = newPreDefColumn(state, s->pdstSHGSpkHt, "HVAC Input Heated Surface Heating [W]");
        s->pdchSHGSHtSurfCl = newPreDefColumn(state, s->pdstSHGSpkHt, "HVAC Input Cooled Surface Cooling [W]");
        s->pdchSHGSHtPeoplAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "People Sensible Heat Addition [W]");
        s->pdchSHGSHtLiteAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "Lights Sensible Heat Addition [W]");
        s->pdchSHGSHtEquipAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "Equipment Sensible Heat Addition [W]");
        s->pdchSHGSHtWindAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "Window Heat Addition [W]");
        s->pdchSHGSHtIzaAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "Interzone Air Transfer Heat Addition [W]");
        s->pdchSHGSHtInfilAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "Infiltration Heat Addition [W]");
        s->pdchSHGSHtOtherAdd = newPreDefColumn(state, s->pdstSHGSpkHt, "Opaque Surface Conduction and Other Heat Addition [W]");
        s->pdchSHGSHtEquipRem = newPreDefColumn(state, s->pdstSHGSpkHt, "Equipment Sensible Heat Removal [W]");
        s->pdchSHGSHtWindRem = newPreDefColumn(state, s->pdstSHGSpkHt, "Window Heat Removal [W]");
        s->pdchSHGSHtIzaRem = newPreDefColumn(state, s->pdstSHGSpkHt, "Interzone Air Transfer Heat Removal [W]");
        s->pdchSHGSHtInfilRem = newPreDefColumn(state, s->pdstSHGSpkHt, "Infiltration Heat Removal [W]");
        s->pdchSHGSHtOtherRem = newPreDefColumn(state, s->pdstSHGSpkHt, "Opaque Surface Conduction and Other Heat Removal [W]");

        // Standard62Report
        if (state.dataGlobal->DoZoneSizing || state.dataGlobal->DoSystemSizing) {
            s->pdrStd62 = newPreDefReport(state, "Standard62.1Summary", "Std62", "Standard 62.1 Summary");

            s->pdstS62sysVentReqCool = newPreDefSubTable(state, s->pdrStd62, "System Ventilation Requirements for Cooling");

            s->pdchS62svrClSumVpz = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Sum of Zone Primary Air Flow - Vpz-sum [m3/s]");
            s->pdchS62svrClPs = newPreDefColumn(state, s->pdstS62sysVentReqCool, "System Population - Ps");
            s->pdchS62svrClSumPz = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Sum of Zone Population - Pz-sum");
            s->pdchS62svrClD = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Occupant Diversity - D");
            s->pdchS62svrClDorg = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Origin of D");
            s->pdchS62svrClVou = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Uncorrected Outdoor Air Intake Airflow - Vou [m3/s]");
            s->pdchS62svrClVps = newPreDefColumn(state, s->pdstS62sysVentReqCool, "System Primary Airflow - Vps [m3/s]");
            s->pdchS62svrClXs = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Average Outdoor Air Fraction - Xs");
            s->pdchS62svrClEv = newPreDefColumn(state, s->pdstS62sysVentReqCool, "System Ventilation Efficiency - Ev");
            s->pdchS62svrClEvMthd = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Calculation Method for Ev");
            s->pdchS62svrClVot = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Outdoor Air Intake Flow - Vot [m3/s]");
            s->pdchS62svrClPercOA = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Percent Outdoor Air - %OA");
            s->pdchS62svrClEnvironmentOfPs = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Environment Name of Peak System Population - Ps");
            s->pdchS62svrClTimeOfPs = newPreDefColumn(state, s->pdstS62sysVentReqCool, "Date and Time of Last Peak System Population - Ps");

            s->pdstS62sysVentReqHeat = newPreDefSubTable(state, s->pdrStd62, "System Ventilation Requirements for Heating");

            s->pdchS62svrHtSumVpz = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Sum of Zone Primary Air Flow - Vpz-sum [m3/s]");
            s->pdchS62svrHtPs = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "System Population - Ps");
            s->pdchS62svrHtSumPz = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Sum of Zone Population - Pz-sum");
            s->pdchS62svrHtD = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Occupant Diversity - D");
            s->pdchS62svrHtDorg = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Origin of D");
            s->pdchS62svrHtVou = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Uncorrected Outdoor Air Intake Airflow - Vou [m3/s]");
            s->pdchS62svrHtVps = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "System Primary Airflow - Vps [m3/s]");
            s->pdchS62svrHtXs = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Average Outdoor Air Fraction - Xs");
            s->pdchS62svrHtEv = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "System Ventilation Efficiency - Ev");
            s->pdchS62svrHtEvMthd = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Calculation Method for Ev");
            s->pdchS62svrHtVot = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Outdoor Air Intake Flow Vot [m3/s]");
            s->pdchS62svrHtPercOA = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Percent Outdoor Air - %OA");
            s->pdchS62svrHtEnvironmentOfPs = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Environment Name of Peak System Population - Ps");
            s->pdchS62svrHtTimeOfPs = newPreDefColumn(state, s->pdstS62sysVentReqHeat, "Date and Time of Last Peak System Population - Ps");

            s->pdstS62znVentPar = newPreDefSubTable(state, s->pdrStd62, "Zone Ventilation Parameters");

            s->pdchS62zvpAlN = newPreDefColumn(state, s->pdstS62znVentPar, "AirLoop Name");
            s->pdchS62zvpRp = newPreDefColumn(state, s->pdstS62znVentPar, "People Outdoor Air Rate - Rp [m3/s-person]");
            s->pdchS62zvpPz = newPreDefColumn(state, s->pdstS62znVentPar, "Zone Population - Pz");
            s->pdchS62zvpRa = newPreDefColumn(state, s->pdstS62znVentPar, "Area Outdoor Air Rate - Ra [m3/s-m2]");
            s->pdchS62zvpAz = newPreDefColumn(state, s->pdstS62znVentPar, "Zone Floor Area - Az [m2]");
            s->pdchS62zvpVbz = newPreDefColumn(state, s->pdstS62znVentPar, "Breathing Zone Outdoor Airflow - Vbz [m3/s]");
            s->pdchS62zvpClEz = newPreDefColumn(state, s->pdstS62znVentPar, "Cooling Zone Air Distribution Effectiveness - Ez-clg");
            s->pdchS62zvpClVoz = newPreDefColumn(state, s->pdstS62znVentPar, "Cooling Zone Outdoor Airflow - Voz-clg [m3/s]");
            s->pdchS62zvpHtEz = newPreDefColumn(state, s->pdstS62znVentPar, "Heating Zone Air Distribution Effectiveness - Ez-htg");
            s->pdchS62zvpHtVoz = newPreDefColumn(state, s->pdstS62znVentPar, "Heating Zone Outdoor Airflow - Voz-htg [m3/s]");

            s->pdstS62sysVentPar = newPreDefSubTable(state, s->pdrStd62, "System Ventilation Parameters");

            s->pdchS62svpRp = newPreDefColumn(state, s->pdstS62sysVentPar, "People Outdoor Air Rate - Rp [m3/s-person]");
            s->pdchS62svpPz = newPreDefColumn(state, s->pdstS62sysVentPar, "Sum of Zone Population - Pz-sum");
            s->pdchS62svpRa = newPreDefColumn(state, s->pdstS62sysVentPar, "Area Outdoor Air Rate - Ra [m3/s-m2]");
            s->pdchS62svpAz = newPreDefColumn(state, s->pdstS62sysVentPar, "Sum of Zone Floor Area - Az-sum [m2]");
            s->pdchS62svpVbz = newPreDefColumn(state, s->pdstS62sysVentPar, "Breathing Zone Outdoor Airflow - Vbz [m3/s]");
            s->pdchS62svpClVoz = newPreDefColumn(state, s->pdstS62sysVentPar, "Cooling Zone Outdoor Airflow - Voz-clg [m3/s]");
            s->pdchS62svpHtVoz = newPreDefColumn(state, s->pdstS62sysVentPar, "Heating Zone Outdoor Airflow - Voz-htg [m3/s]");

            s->pdstS62znCoolDes = newPreDefSubTable(state, s->pdrStd62, "Zone Ventilation Calculations for Cooling Design");

            s->pdchS62zcdAlN = newPreDefColumn(state, s->pdstS62znCoolDes, "AirLoop Name");
            s->pdchS62zcdBox = newPreDefColumn(state, s->pdstS62znCoolDes, "Box Type");
            s->pdchS62zcdVpz = newPreDefColumn(state, s->pdstS62znCoolDes, "Zone Primary Airflow - Vpz [m3/s]");
            // s->pdchS62zcdVps =         newPreDefColumn(state, s->pdstS62znCoolDes,'System Primary Airflow - Vps [m3/s]')
            // s->pdchS62zcdVsec =        newPreDefColumn(state, s->pdstS62znCoolDes,'Secondary Fan Airflow - Vsec [m3/s]')
            s->pdchS62zcdVdz = newPreDefColumn(state, s->pdstS62znCoolDes, "Zone Discharge Airflow - Vdz [m3/s]");
            s->pdchS62zcdVpzmin = newPreDefColumn(state, s->pdstS62znCoolDes, "Minimum Zone Primary Airflow - Vpz-min [m3/s]");
            s->pdchS62zcdVpzminSPSize =
                newPreDefColumn(state, s->pdstS62znCoolDes, "Is Vpz-min calculated using the Standard 62.1 Simplified Procedure?");
            s->pdchS62zcdVozclg = newPreDefColumn(state, s->pdstS62znCoolDes, "Zone Outdoor Airflow Cooling - Voz-clg [m3/s]");
            s->pdchS62zcdZpz = newPreDefColumn(state, s->pdstS62znCoolDes, "Primary Outdoor Air Fraction - Zpz");
            s->pdchS62zcdEp = newPreDefColumn(state, s->pdstS62znCoolDes, "Primary Air Fraction - Ep");
            s->pdchS62zcdEr = newPreDefColumn(state, s->pdstS62znCoolDes, "Secondary Recirculation Fraction- Er");
            s->pdchS62zcdFa = newPreDefColumn(state, s->pdstS62znCoolDes, "Supply Air Fraction- Fa");
            s->pdchS62zcdFb = newPreDefColumn(state, s->pdstS62znCoolDes, "Mixed Air Fraction - Fb");
            s->pdchS62zcdFc = newPreDefColumn(state, s->pdstS62znCoolDes, "Outdoor Air Fraction - Fc");
            s->pdchS62zcdEvz = newPreDefColumn(state, s->pdstS62znCoolDes, "Zone Ventilation Efficiency - Evz");

            s->pdstS62sysCoolDes = newPreDefSubTable(state, s->pdrStd62, "System Ventilation Calculations for Cooling Design");

            s->pdchS62scdVpz = newPreDefColumn(state, s->pdstS62sysCoolDes, "Sum of Zone Primary Airflow - Vpz-sum [m3/s]");
            s->pdchS62scdVps = newPreDefColumn(state, s->pdstS62sysCoolDes, "System Primary Airflow - Vps [m3/s]");
            // s->pdchS62scdVsec =        newPreDefColumn(state, s->pdstS62sysCoolDes,'Secondary Fan Airflow - Vsec [m3/s]')
            s->pdchS62scdVdz = newPreDefColumn(state, s->pdstS62sysCoolDes, "Sum of Zone Discharge Airflow - Vdz-sum [m3/s]");
            s->pdchS62scdVpzmin = newPreDefColumn(state, s->pdstS62sysCoolDes, "Sum of Min Zone Primary Airflow - Vpz-min [m3/s]");
            s->pdchS62scdVozclg = newPreDefColumn(state, s->pdstS62sysCoolDes, "Zone Outdoor Airflow Cooling - Voz-clg [m3/s]");
            s->pdchS62scdEvz = newPreDefColumn(state, s->pdstS62sysCoolDes, "Zone Ventilation Efficiency - Evz-min");

            s->pdstS62znHeatDes = newPreDefSubTable(state, s->pdrStd62, "Zone Ventilation Calculations for Heating Design");

            s->pdchS62zhdAlN = newPreDefColumn(state, s->pdstS62znHeatDes, "AirLoop Name");
            s->pdchS62zhdBox = newPreDefColumn(state, s->pdstS62znHeatDes, "Box Type");
            s->pdchS62zhdVpz = newPreDefColumn(state, s->pdstS62znHeatDes, "Zone Primary Airflow - Vpz [m3/s]");
            // s->pdchS62zhdVps =         newPreDefColumn(state, s->pdstS62znHeatDes,'System Primary Airflow - Vps [m3/s]')
            // s->pdchS62zhdVsec =        newPreDefColumn(state, s->pdstS62znHeatDes,'Secondary Fan Airflow - Vsec [m3/s]')
            s->pdchS62zhdVdz = newPreDefColumn(state, s->pdstS62znHeatDes, "Zone Discharge Airflow - Vdz [m3/s]");
            s->pdchS62zhdVpzmin = newPreDefColumn(state, s->pdstS62znHeatDes, "Minimum Zone Primary Airflow - Vpz-min [m3/s]");
            s->pdchS62zhdVpzminSPSize =
                newPreDefColumn(state, s->pdstS62znHeatDes, "Is Vpz-min calculated using the Standard 62.1 Simplified Procedure?");
            s->pdchS62zhdVozhtg = newPreDefColumn(state, s->pdstS62znHeatDes, "Zone Outdoor Airflow Heating - Voz-htg [m3/s]");
            s->pdchS62zhdZpz = newPreDefColumn(state, s->pdstS62znHeatDes, "Primary Outdoor Air Fraction - Zpz");
            s->pdchS62zhdEp = newPreDefColumn(state, s->pdstS62znHeatDes, "Primary Air Fraction - Ep");
            s->pdchS62zhdEr = newPreDefColumn(state, s->pdstS62znHeatDes, "Secondary Recirculation Fraction- Er");
            s->pdchS62zhdFa = newPreDefColumn(state, s->pdstS62znHeatDes, "Supply Air Fraction- Fa");
            s->pdchS62zhdFb = newPreDefColumn(state, s->pdstS62znHeatDes, "Mixed Air Fraction - Fb");
            s->pdchS62zhdFc = newPreDefColumn(state, s->pdstS62znHeatDes, "Outdoor Air Fraction - Fc");
            s->pdchS62zhdEvz = newPreDefColumn(state, s->pdstS62znHeatDes, "Zone Ventilation Efficiency - Evz");

            s->pdstS62sysHeatDes = newPreDefSubTable(state, s->pdrStd62, "System Ventilation Calculations for Heating Design");

            s->pdchS62shdVpz = newPreDefColumn(state, s->pdstS62sysHeatDes, "Sum of Zone Primary Airflow - Vpz-sum [m3/s]");
            s->pdchS62shdVps = newPreDefColumn(state, s->pdstS62sysHeatDes, "System Primary Airflow - Vps [m3/s]");
            // s->pdchS62shdVsec =        newPreDefColumn(state, s->pdstS62sysHeatDes,'Secondary Fan Airflow - Vsec [m3/s]')
            s->pdchS62shdVdz = newPreDefColumn(state, s->pdstS62sysHeatDes, "Sum of Zone Discharge Airflow - Vdz-sum [m3/s]");
            s->pdchS62shdVpzmin = newPreDefColumn(state, s->pdstS62sysHeatDes, "Sum of Min Zone Primary Airflow - Vpz-min [m3/s]");
            s->pdchS62shdVozhtg = newPreDefColumn(state, s->pdstS62sysHeatDes, "Zone Outdoor Airflow Heating - Voz-htg [m3/s]");
            s->pdchS62shdEvz = newPreDefColumn(state, s->pdstS62sysHeatDes, "Zone Ventilation Efficiency - Evz-min");
        }

        s->pdrLeed = newPreDefReport(state, "LEEDsummary", "LEED", "LEED Summary");

        s->pdstLeedGenInfo = newPreDefSubTable(state, s->pdrLeed, "Sec1.1A-General Information");
        // single column with rows of:
        //    Principal Heating Source
        //    Weather File
        //    Climate Zone
        //    Heating Degree Days
        //    Cooling Degree Days
        //    HDD and CDD data source
        //    Total gross floor area
        s->pdchLeedGenData = newPreDefColumn(state, s->pdstLeedGenInfo, "Data");

        s->pdstLeedSpaceUsageType = newPreDefSubTable(state, s->pdrLeed, "EAp2-1. Space Usage Type");
        s->pdchLeedSutSpArea = newPreDefColumn(state, s->pdstLeedSpaceUsageType, "Space Area [m2]");
        s->pdchLeedSutOcArea = newPreDefColumn(state, s->pdstLeedSpaceUsageType, "Regularly Occupied Area [m2]");
        s->pdchLeedSutUnArea = newPreDefColumn(state, s->pdstLeedSpaceUsageType, "Unconditioned Area [m2]");
        s->pdchLeedSutHrsWeek = newPreDefColumn(state, s->pdstLeedSpaceUsageType, "Typical Hours/Week in Operation [hr/wk]");

        s->pdstLeedAdvsMsg = newPreDefSubTable(state, s->pdrLeed, "EAp2-2. Advisory Messages");
        // single column with rows of:
        //    Number of hours heating loads not met
        //    Number of hours cooling loads not met
        //    Total
        //    Difference
        //    Number of warning messages
        //    Number of error messages
        //    Number of defaults overridden
        s->pdchLeedAmData = newPreDefColumn(state, s->pdstLeedAdvsMsg, "Data");

        s->pdstLeedEneTypSum = newPreDefSubTable(state, s->pdrLeed, "EAp2-3. Energy Type Summary");
        // multiple columns with rows of
        //    Electricity
        //    Natural Gas
        //    <additional fuels>
        s->pdchLeedEtsRtNm = newPreDefColumn(state, s->pdstLeedEneTypSum, "Utility Rate");
        s->pdchLeedEtsVirt = newPreDefColumn(state, s->pdstLeedEneTypSum, "Virtual Rate [$/unit energy]");
        s->pdchLeedEtsEneUnt = newPreDefColumn(state, s->pdstLeedEneTypSum, "Units of Energy");
        s->pdchLeedEtsDemUnt = newPreDefColumn(state, s->pdstLeedEneTypSum, "Units of Demand");

        s->pdstLeedPerf = newPreDefSubTable(state, s->pdrLeed, "EAp2-4/5. Performance Rating Method Compliance");
        // Multiple colums with rows of:
        //     Interior Lighting
        //     Exterior Lighting
        //     Space Heating
        //     Space Cooling
        //     Pumps
        //     Heat Rejection
        //     Fans-Interior
        //     Fans-Parking Garage
        //     Service Water Heating
        //     Receptacle Equipment
        //     Interior Lighting (process)
        //     Refrigeration Equipment
        //     Cooking
        //     Industrial Process
        //     Elevators and Escalators
        //     Total
        s->pdchLeedPerfElEneUse = newPreDefColumn(state, s->pdstLeedPerf, "Electricity Energy Use [GJ]");
        s->pdchLeedPerfElDem = newPreDefColumn(state, s->pdstLeedPerf, "Electricity Demand [W]");
        s->pdchLeedPerfGasEneUse = newPreDefColumn(state, s->pdstLeedPerf, "Natural Gas Energy Use [GJ]");
        s->pdchLeedPerfGasDem = newPreDefColumn(state, s->pdstLeedPerf, "Natural Gas Demand [W]");
        s->pdchLeedPerfGasolineEneUse = newPreDefColumn(state, s->pdstLeedPerf, "Gasoline Use [GJ]");
        s->pdchLeedPerfGasolineDem = newPreDefColumn(state, s->pdstLeedPerf, "Gasoline Demand [W]");
        s->pdchLeedPerfDieselEneUse = newPreDefColumn(state, s->pdstLeedPerf, "Diesel Use [GJ]");
        s->pdchLeedPerfDieselDem = newPreDefColumn(state, s->pdstLeedPerf, "Diesel Demand [W]");
        s->pdchLeedPerfCoalEneUse = newPreDefColumn(state, s->pdstLeedPerf, "Coal Use [GJ]");
        s->pdchLeedPerfCoalDem = newPreDefColumn(state, s->pdstLeedPerf, "Coal Demand [W]");
        s->pdchLeedPerfFuelOil1EneUse = newPreDefColumn(state, s->pdstLeedPerf, "Fuel Oil No 1 Use [GJ]");
        s->pdchLeedPerfFuelOil1Dem = newPreDefColumn(state, s->pdstLeedPerf, "Fuel Oil No 1 Demand [W]");
        s->pdchLeedPerfFuelOil2EneUse = newPreDefColumn(state, s->pdstLeedPerf, "Fuel Oil No 2 Use [GJ]");
        s->pdchLeedPerfFuelOil2Dem = newPreDefColumn(state, s->pdstLeedPerf, "Fuel Oil No 2 Demand [W]");
        s->pdchLeedPerfPropaneEneUse = newPreDefColumn(state, s->pdstLeedPerf, "Propane Use [GJ]");
        s->pdchLeedPerfPropaneDem = newPreDefColumn(state, s->pdstLeedPerf, "Propane Demand [W]");
        s->pdchLeedPerfOtherFuel1EneUse = newPreDefColumn(state, s->pdstLeedPerf, "Other Fuel 1 Use [GJ]");
        s->pdchLeedPerfOtherFuel1Dem = newPreDefColumn(state, s->pdstLeedPerf, "Other Fuel 1 Demand [W]");
        s->pdchLeedPerfOtherFuel2EneUse = newPreDefColumn(state, s->pdstLeedPerf, "Other Fuel 2 Use [GJ]");
        s->pdchLeedPerfOtherFuel2Dem = newPreDefColumn(state, s->pdstLeedPerf, "Other Fuel 2 Demand [W]");
        s->pdchLeedPerfDisClEneUse = newPreDefColumn(state, s->pdstLeedPerf, "District Cooling Use [GJ]");
        s->pdchLeedPerfDisClDem = newPreDefColumn(state, s->pdstLeedPerf, "District Cooling Demand [W]");
        s->pdchLeedPerfDisHtWtrEneUse = newPreDefColumn(state, s->pdstLeedPerf, "District Heating Water Use [GJ]");
        s->pdchLeedPerfDisHtWtrDem = newPreDefColumn(state, s->pdstLeedPerf, "District Heating Water Demand [W]");
        s->pdchLeedPerfDisHtStEneUse = newPreDefColumn(state, s->pdstLeedPerf, "District Heating Steam Use [GJ]");
        s->pdchLeedPerfDisHtStDem = newPreDefColumn(state, s->pdstLeedPerf, "District Heating Steam Demand [W]");

        s->pdstLeedEneUseSum = newPreDefSubTable(state, s->pdrLeed, "EAp2-6. Energy Use Summary");
        // Multiple columns with rows of:
        //    Electricity
        //    Natural Gas
        //    <additional fuels>
        //    Total
        s->pdchLeedEusProc = newPreDefColumn(state, s->pdstLeedEneUseSum, "Process Subtotal [GJ]");
        s->pdchLeedEusTotal = newPreDefColumn(state, s->pdstLeedEneUseSum, "Total Energy Use [GJ]");

        s->pdstLeedEneCostSum = newPreDefSubTable(state, s->pdrLeed, "EAp2-7. Energy Cost Summary");
        // Multiple columns with rows of:
        //    Electricity
        //    Natural Gas
        //    <additional fuels>
        //    Total
        s->pdchLeedEcsProc = newPreDefColumn(state, s->pdstLeedEneCostSum, "Process Subtotal [$]");
        s->pdchLeedEcsTotal = newPreDefColumn(state, s->pdstLeedEneCostSum, "Total Energy Cost [$]");

        s->pdstLeedRenewSum = newPreDefSubTable(state, s->pdrLeed, "L-1. Renewable Energy Source Summary");
        // Multiple columns with rows of each renewable source
        s->pdchLeedRenRatCap = newPreDefColumn(state, s->pdstLeedRenewSum, "Rated Capacity [kW]");
        s->pdchLeedRenAnGen = newPreDefColumn(state, s->pdstLeedRenewSum, "Annual Energy Generated [GJ]");

        s->pdstLeedEneUseIntEl = newPreDefSubTable(state, s->pdrLeed, "EAp2-17a. Energy Use Intensity - Electricity");
        // Single column with rows of:
        //    Interior lighting
        //    Space heating
        //    Space cooling
        //    Fans-interior
        //    Service water heating
        //    Receptacle equipment
        //    Miscellaneous
        //    Subtotal
        s->pdchLeedEuiElec = newPreDefColumn(state, s->pdstLeedEneUseIntEl, "Electricty [MJ/m2]");

        s->pdstLeedEneUseIntNatG = newPreDefSubTable(state, s->pdrLeed, "EAp2-17b. Energy Use Intensity - Natural Gas");
        // Single column with rows of:
        //    Space heating
        //    Service water heating
        //    Miscellaneous
        //    Subtotal
        s->pdchLeedEuiNatG = newPreDefColumn(state, s->pdstLeedEneUseIntNatG, "Natural Gas [MJ/m2]");

        s->pdstLeedEneUseIntOthr = newPreDefSubTable(state, s->pdrLeed, "EAp2-17c. Energy Use Intensity - Additional");
        // Single column with rows of:
        //    Miscellaneous
        //    Subtotal
        s->pdchLeedEuiOthr = newPreDefColumn(state, s->pdstLeedEneUseIntOthr, "Additional [MJ/m2]");

        s->pdstLeedEneUsePerc = newPreDefSubTable(state, s->pdrLeed, "EAp2-18. End Use Percentage");
        // single column with rows of:
        //    Interior Lighting
        //    Space heating
        //    Space cooling
        //    Fans-Interior
        //    Service Water Heating
        //    Receptacle Equipment
        //    Miscellaneous
        s->pdchLeedEupPerc = newPreDefColumn(state, s->pdstLeedEneUsePerc, "Percent [%]");

        s->pdstLeedEqFlLdHrs = newPreDefSubTable(state, s->pdrLeed, "Schedules-Equivalent Full Load Hours (Schedule Type=Fraction)");
        s->pdchLeedEflhEflh = newPreDefColumn(state, s->pdstLeedEqFlLdHrs, "Equivalent Full Load Hours of Operation Per Year [hr]");
        s->pdchLeedEflhNonZerHrs = newPreDefColumn(state, s->pdstLeedEqFlLdHrs, "Hours > 1% [hr]");

        s->pdstLeedSchedSetPts = newPreDefSubTable(state, s->pdrLeed, "Schedules-SetPoints (Schedule Type=Temperature)");
        s->pdChLeedSchStPtFirstObjUsed = newPreDefColumn(state, s->pdstLeedSchedSetPts, "First Object Used");
        s->pdChLeedSchStPtMonthUsed = newPreDefColumn(state, s->pdstLeedSchedSetPts, "Month Assumed");
        s->pdchLeedSchStPt11amWednesday = newPreDefColumn(state, s->pdstLeedSchedSetPts, "11am First Wednesday [C]");
        s->pdchLeedSchStPt11amWedCnt = newPreDefColumn(state, s->pdstLeedSchedSetPts, "Days with Same 11am Value");
        s->pdchLeedSchStPt11pmWednesday = newPreDefColumn(state, s->pdstLeedSchedSetPts, "11pm First Wednesday [C]");
        s->pdchLeedSchStPt11pmWedCnt = newPreDefColumn(state, s->pdstLeedSchedSetPts, "Days with Same 11pm Value");

        s->pdrCO2Resilience = newPreDefReport(state, "CO2ResilienceSummary", "CO2R", "Annual CO2 Resilience Summary");

        s->pdstCO2Hours = newPreDefSubTable(state, s->pdrCO2Resilience, "CO2 Level Hours");
        s->pdchCO2HourSafe = newPreDefColumn(state, s->pdstCO2Hours, "Safe (<= 1000 ppm) [hr]");
        s->pdchCO2HourCaution = newPreDefColumn(state, s->pdstCO2Hours, "Caution (> 1000, <= 5000 ppm) [hr]");
        s->pdchCO2HourHazard = newPreDefColumn(state, s->pdstCO2Hours, "Hazard (> 5000 ppm) [hr]");

        s->pdstCO2OccuHours = newPreDefSubTable(state, s->pdrCO2Resilience, "CO2 Level OccupantHours");
        s->pdchCO2OccuHourSafe = newPreDefColumn(state, s->pdstCO2OccuHours, "Safe (<= 1000 ppm) [hr]");
        s->pdchCO2OccuHourCaution = newPreDefColumn(state, s->pdstCO2OccuHours, "Caution (> 1000, <= 5000 ppm) [hr]");
        s->pdchCO2OccuHourHazard = newPreDefColumn(state, s->pdstCO2OccuHours, "Hazard (> 5000 ppm) [hr]");

        s->pdstCO2OccupiedHours = newPreDefSubTable(state, s->pdrCO2Resilience, "CO2 Level OccupiedHours");
        s->pdchCO2OccupiedHourSafe = newPreDefColumn(state, s->pdstCO2OccupiedHours, "Safe (<= 1000 ppm) [hr]");
        s->pdchCO2OccupiedHourCaution = newPreDefColumn(state, s->pdstCO2OccupiedHours, "Caution (> 1000, <= 5000 ppm) [hr]");
        s->pdchCO2OccupiedHourHazard = newPreDefColumn(state, s->pdstCO2OccupiedHours, "Hazard (> 5000 ppm) [hr]");

        s->pdrVisualResilience = newPreDefReport(state, "VisualResilienceSummary", "VisualR", "Annual Visual Resilience Summary");

        s->pdstIllumHours = newPreDefSubTable(state, s->pdrVisualResilience, "Illuminance Level Hours");
        s->pdchIllumHourDark = newPreDefColumn(state, s->pdstIllumHours, "A Bit Dark (<= 100 lux) [hr]");
        s->pdchIllumHourDim = newPreDefColumn(state, s->pdstIllumHours, "Dim (> 100, <= 300 lux) [hr]");
        s->pdchIllumHourAdequate = newPreDefColumn(state, s->pdstIllumHours, "Adequate (> 300, <= 500 lux) [hr]");
        s->pdchIllumHourBright = newPreDefColumn(state, s->pdstIllumHours, "Bright (>500 lux) [hr]");

        s->pdstIllumOccuHours = newPreDefSubTable(state, s->pdrVisualResilience, "Illuminance Level OccupantHours");
        s->pdchIllumOccuHourDark = newPreDefColumn(state, s->pdstIllumOccuHours, "A Bit Dark (<= 100 lux) [hr]");
        s->pdchIllumOccuHourDim = newPreDefColumn(state, s->pdstIllumOccuHours, "Dim (> 100, <= 300 lux) [hr]");
        s->pdchIllumOccuHourAdequate = newPreDefColumn(state, s->pdstIllumOccuHours, "Adequate (> 300, <= 500 lux) [hr]");
        s->pdchIllumOccuHourBright = newPreDefColumn(state, s->pdstIllumOccuHours, "Bright (>500 lux) [hr]");

        s->pdstIllumOccupiedHours = newPreDefSubTable(state, s->pdrVisualResilience, "Illuminance Level OccupiedHours");
        s->pdchIllumOccupiedHourDark = newPreDefColumn(state, s->pdstIllumOccupiedHours, "A Bit Dark (<= 100 lux) [hr]");
        s->pdchIllumOccupiedHourDim = newPreDefColumn(state, s->pdstIllumOccupiedHours, "Dim (> 100, <= 300 lux) [hr]");
        s->pdchIllumOccupiedHourAdequate = newPreDefColumn(state, s->pdstIllumOccupiedHours, "Adequate (> 300, <= 500 lux) [hr]");
        s->pdchIllumOccupiedHourBright = newPreDefColumn(state, s->pdstIllumOccupiedHours, "Bright (>500 lux) [hr]");
    }

    void PreDefTableEntry(EnergyPlusData &state,
                          int const columnIndex,
                          std::string_view objName,
                          Real64 const tableEntryReal,
                          ObjexxFCL::Optional_int_const numSigDigits)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates an entry for predefined tables when the entry
        //   is a real variable

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        incrementTableEntry(state);
        int sigDigitCount = 2;
        // check for number of significant digits
        if (present(numSigDigits)) {
            if ((numSigDigits <= 9) && (numSigDigits >= 0)) {
                sigDigitCount = numSigDigits;
            }
        }

        if (std::abs(tableEntryReal) < 1e8) { // change from 1e10 for more robust entry writing
            // something changed in FMT 7.x and "{:#12.{}F}" now outputs 13. So changing it to 11.{}F to maintain existing functionality. Likely
            // related to https://github.com/fmtlib/fmt/issues/1893
            state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).charEntry =
                format("{:#11.{}F}", tableEntryReal, sigDigitCount);
        } else {
            // Formatting in scientific notation, zero sigDigits makes zero sense.
            // **for something greater than 1E+08**, one sigDigits is very unhelpful (you're having an accuracy of 0.5E+07 at best)
            if (sigDigitCount < 2) {
                sigDigitCount = 2;
            }
            state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).charEntry =
                format("{:12.{}Z}", tableEntryReal, sigDigitCount);
        }

        if (state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).charEntry.size() > 12) {
            state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).charEntry = "  Too Big";
        }

        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).objectName = objName;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).indexColumn = columnIndex;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).origRealEntry = tableEntryReal;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).significantDigits = sigDigitCount;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).origEntryIsReal = true;
    }

    void PreDefTableEntry(EnergyPlusData &state, int const columnIndex, std::string_view objName, std::string_view tableEntryChar)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates an entry for predefined tables when the entry
        //   is a character variable

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        incrementTableEntry(state);
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).charEntry = tableEntryChar;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).objectName = objName;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).indexColumn = columnIndex;
    }

    void PreDefTableEntry(EnergyPlusData &state, int const columnIndex, std::string_view objName, int const tableEntryInt)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates an entry for predefined tables when the entry
        //   is a integer variable

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        incrementTableEntry(state);
        // convert the integer to a string
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).charEntry = format("{:12}", tableEntryInt);
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).objectName = objName;
        state.dataOutRptPredefined->tableEntry(state.dataOutRptPredefined->numTableEntry).indexColumn = columnIndex;
    }

    std::string RetrievePreDefTableEntry(EnergyPlusData &state, int const columnIndex, std::string_view objName)
    {
        for (int iTableEntry = 1; iTableEntry <= state.dataOutRptPredefined->numTableEntry; ++iTableEntry) {
            if (state.dataOutRptPredefined->tableEntry(iTableEntry).indexColumn == columnIndex &&
                state.dataOutRptPredefined->tableEntry(iTableEntry).objectName == objName) {
                return trimmed(ljustified(state.dataOutRptPredefined->tableEntry(iTableEntry).charEntry));
            }
        }
        return "NOT FOUND";
    }

    void incrementTableEntry(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Manages the resizing of the TableEntry Array

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        if (!allocated(state.dataOutRptPredefined->tableEntry)) {
            state.dataOutRptPredefined->tableEntry.allocate(sizeIncrement);
            state.dataOutRptPredefined->sizeTableEntry = sizeIncrement;
            state.dataOutRptPredefined->numTableEntry = 1;
        } else {
            ++state.dataOutRptPredefined->numTableEntry;
            // if larger than current size grow the array
            if (state.dataOutRptPredefined->numTableEntry > state.dataOutRptPredefined->sizeTableEntry) {
                state.dataOutRptPredefined->tableEntry.redimension(
                    state.dataOutRptPredefined->sizeTableEntry *=
                    2); // Tuned Changed += sizeIncrement to *= 2 for reduced heap allocations (at some space cost)
            }
        }
    }

    void AddCompSizeTableEntry(
        EnergyPlusData &state, std::string_view FieldType, std::string_view FieldName, std::string_view FieldDescription, Real64 const FieldValue)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   July 2007
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates an entry for component size tables.

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (!allocated(state.dataOutRptPredefined->CompSizeTableEntry)) {
            state.dataOutRptPredefined->CompSizeTableEntry.allocate(sizeIncrement);
            state.dataOutRptPredefined->sizeCompSizeTableEntry = sizeIncrement;
            state.dataOutRptPredefined->numCompSizeTableEntry = 1;
        } else {
            ++state.dataOutRptPredefined->numCompSizeTableEntry;
            // if larger than current size grow the array
            if (state.dataOutRptPredefined->numCompSizeTableEntry > state.dataOutRptPredefined->sizeCompSizeTableEntry) {
                state.dataOutRptPredefined->CompSizeTableEntry.redimension(
                    state.dataOutRptPredefined->sizeCompSizeTableEntry *=
                    2); // Tuned Changed += sizeIncrement to *= 2 for reduced heap allocations (at some space cost)
            }
        }
        state.dataOutRptPredefined->CompSizeTableEntry(state.dataOutRptPredefined->numCompSizeTableEntry).typeField = FieldType;
        state.dataOutRptPredefined->CompSizeTableEntry(state.dataOutRptPredefined->numCompSizeTableEntry).nameField = FieldName;
        state.dataOutRptPredefined->CompSizeTableEntry(state.dataOutRptPredefined->numCompSizeTableEntry).description = FieldDescription;
        state.dataOutRptPredefined->CompSizeTableEntry(state.dataOutRptPredefined->numCompSizeTableEntry).valField = FieldValue;
    }

    void AddShadowRelateTableEntry(EnergyPlusData &state, int const castingField, int const receivingField, int const receivingKind)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   July 2007
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates an entry for any shadow hierarchy tables that consist
        //   of items and one or more subitems for each item.

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // CHARACTER(len=*),INTENT(IN)  :: castingField
        // CHARACTER(len=*),INTENT(IN)  :: receivingField

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (!allocated(state.dataOutRptPredefined->ShadowRelate)) {
            state.dataOutRptPredefined->ShadowRelate.allocate(sizeIncrement);
            state.dataOutRptPredefined->sizeShadowRelate = sizeIncrement;
            state.dataOutRptPredefined->numShadowRelate = 1;
        } else {
            ++state.dataOutRptPredefined->numShadowRelate;
            // if larger than current size grow the array
            if (state.dataOutRptPredefined->numShadowRelate > state.dataOutRptPredefined->sizeShadowRelate) {
                state.dataOutRptPredefined->ShadowRelate.redimension(
                    state.dataOutRptPredefined->sizeShadowRelate *=
                    2); // Tuned Changed += sizeIncrement to *= 2 for reduced heap allocations (at some space cost)
            }
        }
        state.dataOutRptPredefined->ShadowRelate(state.dataOutRptPredefined->numShadowRelate).castSurf = castingField;
        state.dataOutRptPredefined->ShadowRelate(state.dataOutRptPredefined->numShadowRelate).recSurf = receivingField;
        state.dataOutRptPredefined->ShadowRelate(state.dataOutRptPredefined->numShadowRelate).recKind = receivingKind;
    }

    int newPreDefReport(EnergyPlusData &state, std::string_view inReportName, std::string_view inReportAbrev, std::string_view inReportNamewithSpaces)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Creates a new index for the next predefined report

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Return value
        int newPreDefReport;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        if (!allocated(state.dataOutRptPredefined->reportName)) {
            state.dataOutRptPredefined->reportName.allocate(sizeIncrement);
            state.dataOutRptPredefined->sizeReportName = sizeIncrement;
            state.dataOutRptPredefined->numReportName = 1;
        } else {
            ++state.dataOutRptPredefined->numReportName;
            // if larger than current size grow the array
            if (state.dataOutRptPredefined->numReportName > state.dataOutRptPredefined->sizeReportName) {
                state.dataOutRptPredefined->reportName.redimension(
                    state.dataOutRptPredefined->sizeReportName *=
                    2); // Tuned Changed += sizeIncrement to *= 2 for reduced heap allocations (at some space cost)
            }
        }
        // initialize new record
        state.dataOutRptPredefined->reportName(state.dataOutRptPredefined->numReportName).name = inReportName;
        state.dataOutRptPredefined->reportName(state.dataOutRptPredefined->numReportName).abrev = inReportAbrev;
        state.dataOutRptPredefined->reportName(state.dataOutRptPredefined->numReportName).namewithspaces = inReportNamewithSpaces;
        state.dataOutRptPredefined->reportName(state.dataOutRptPredefined->numReportName).show = false;
        newPreDefReport = state.dataOutRptPredefined->numReportName;
        return newPreDefReport;
    }

    int newPreDefSubTable(EnergyPlusData &state, int const reportIndex, std::string_view subTableName)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Assigns the index for predefined sub-tables

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Return value

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        if (!allocated(state.dataOutRptPredefined->subTable)) {
            state.dataOutRptPredefined->subTable.allocate(sizeIncrement);
            state.dataOutRptPredefined->sizeSubTable = sizeIncrement;
            state.dataOutRptPredefined->numSubTable = 1;
        } else {
            ++state.dataOutRptPredefined->numSubTable;
            // if larger than current size then grow the array
            if (state.dataOutRptPredefined->numSubTable > state.dataOutRptPredefined->sizeSubTable) {
                state.dataOutRptPredefined->subTable.redimension(
                    state.dataOutRptPredefined->sizeSubTable *=
                    2); // Tuned Changed += sizeIncrement to *= 2 for reduced heap allocations (at some space cost)
            }
        }
        // initialize new record)
        state.dataOutRptPredefined->subTable(state.dataOutRptPredefined->numSubTable).name = subTableName;
        state.dataOutRptPredefined->subTable(state.dataOutRptPredefined->numSubTable).indexReportName = reportIndex;
        return state.dataOutRptPredefined->numSubTable;
    }

    void addFootNoteSubTable(EnergyPlusData &state, int const subTableIndex, std::string_view footnoteText)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2008
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Adds a footnote to a subtable

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        if ((subTableIndex >= 0) && (subTableIndex <= state.dataOutRptPredefined->numSubTable)) {
            state.dataOutRptPredefined->subTable(subTableIndex).footnote = footnoteText;
        }
    }

    int newPreDefColumn(EnergyPlusData &state, int const subTableIndex, std::string_view columnHeading)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   August 2006
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //   Assigns the index variables for all predefined reports

        // METHODOLOGY EMPLOYED:
        //   Simple assignments to public variables.

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Return value
        int newPreDefColumn;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        if (!allocated(state.dataOutRptPredefined->columnTag)) {
            state.dataOutRptPredefined->columnTag.allocate(sizeIncrement);
            state.dataOutRptPredefined->sizeColumnTag = sizeIncrement;
            state.dataOutRptPredefined->numColumnTag = 1;
        } else {
            ++state.dataOutRptPredefined->numColumnTag;
            // if larger than current size grow the array
            if (state.dataOutRptPredefined->numColumnTag > state.dataOutRptPredefined->sizeColumnTag) {
                state.dataOutRptPredefined->columnTag.redimension(
                    state.dataOutRptPredefined->sizeColumnTag *=
                    2); // Tuned Changed += sizeIncrement to *= 2 for reduced heap allocations (at some space cost)
            }
        }
        // initialize new record)
        state.dataOutRptPredefined->columnTag(state.dataOutRptPredefined->numColumnTag).heading = columnHeading;
        state.dataOutRptPredefined->columnTag(state.dataOutRptPredefined->numColumnTag).indexSubTable = subTableIndex;
        newPreDefColumn = state.dataOutRptPredefined->numColumnTag;
        return newPreDefColumn;
    }

} // namespace OutputReportPredefined

} // namespace EnergyPlus
