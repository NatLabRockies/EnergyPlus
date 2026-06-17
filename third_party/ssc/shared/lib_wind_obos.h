/*******************************************************************************************************
 *  Copyright 2017 Alliance for Sustainable Energy, LLC
 *
 *  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
 *  (“Alliance”) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
 *  The Government retains for itself and others acting on its behalf a nonexclusive, paid-up,
 *  irrevocable worldwide license in the software to reproduce, prepare derivative works, distribute
 *  copies to the public, perform publicly and display publicly, and to permit others to do so.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted
 *  provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, the above government
 *  rights notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, the above government
 *  rights notice, this list of conditions and the following disclaimer in the documentation and/or
 *  other materials provided with the distribution.
 *
 *  3. The entire corresponding source code of any redistribution, with or without modification, by a
 *  research entity, including but not limited to any contracting manager/operator of a United States
 *  National Laboratory, any institution of higher learning, and any non-profit organization, must be
 *  made publicly available under this license for as long as the redistribution is made available by
 *  the research entity.
 *
 *  4. Redistribution of this software, without modification, must refer to the software by the same
 *  designation. Redistribution of a modified version of this software (i) may not refer to the modified
 *  version by the same designation, or by any confusingly similar designation, and (ii) must refer to
 *  the underlying software originally provided by Alliance as “System Advisor Model” or “SAM”. Except
 *  to comply with the foregoing, the terms “System Advisor Model”, “SAM”, or any confusingly similar
 *  designation may not be used to refer to any modified version of this software or any modified
 *  version of the underlying software originally provided by Alliance without the prior written consent
 *  of Alliance.
 *
 *  5. The name of the copyright holder, contributors, the United States Government, the United States
 *  Department of Energy, or any of their employees may not be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER,
 *  CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR
 *  EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************************************/

#ifndef __wind_obos_h
#define __wind_obos_h

//#include "lib_wind_obos_defaults.h"
#include "lib_wind_obos_cable_vessel.h"
#include <vector>
#include <tuple>
#include <map>
#include <string>
#include <set>
using namespace std;

//substructure type
enum { MONOPILE, JACKET, SPAR, SEMISUBMERSIBLE };
//anchor types
enum  { DRAGEMBEDMENT, SUCTIONPILE } ;
//turbine installation methods
enum  { INDIVIDUAL, BUNNYEARS, ROTORASSEMBLED } ;
//turbine tower installation methods
enum  { ONEPIECE, TWOPIECE } ;
//installation vessel strategy
enum  { PRIMARYVESSEL, FEEDERBARGE } ;


class wobos {//WIND OFFSHORE BOS STRUCTURE TO HOLD ALL INPUTS AND OUTPUTS AND ALLOW MEMBER FUNCTIONS TO OPERATE ON THOSE VALUES
 public:
  // DEFAULTS FROM CSV FILE
//  wind_obos_defaults wobos_default;
  
  //MAIN INPUTS************************************************************************************************************
  double turbCapEx; //turbine capital cost ($/kW)
  double nTurb;//number of turbines
  double rotorD;//rotor diameter (m)
  double turbR;//turbine rating (MW)
  double hubH;//hub height (m)
  double waterD;// water depth (m)
  double distShore;//distance to shore from install site (km)
  double distPort;//distance to install site from install port (km)
  double distPtoA;//distance from install port to inshore assembly area (km) (spar only)
  double distAtoS;//distance from inshore assembly area to install site (km) (spar Only)
  int substructure; //type of substructure
  int anchor; //anchor type
  int turbInstallMethod; //turbine installation method
  int towerInstallMethod; //tower installation method
  int installStrategy; //installation vessel strategy
  bool cableOptimizer; //switch to run the cable optimizer or not
  double moorLines;//number of mooring lines for floating substructures
  double buryDepth;//array and export cable burial depth (m)
  double arrayY;//turbine array spacing between turbines on same row (rotor diameters)
  double arrayX;// turbine array spacing between turbine rows (rotor diameters)
  double substructCont;//substructure install weather contingency
  double turbCont;//turbine install weather contingency
  double elecCont;//turbine install weather contingency
  double interConVolt;//grid interconnect voltage (kV)
  double distInterCon;//distance from onshore substation to grid interconnect (miles)
  double scrapVal;//scrap value of decommissioned components ($)
  double number_install_seasons; //number of vessel mobilization/install seasons

  //DETAILED INPUTS************************************************************************************************************
  //General
  double projLife;//economic lifetime of the project (years)
  double inspectClear;//inspection clearance for substructure and turbine components (m)
  double plantComm; //plant commissioning cost factor
  double procurement_contingency; //contingency factor for procurement costs
  double install_contingency; //contingency factor for installation costs
  double construction_insurance; //insurance during construction factor
  double capital_cost_year_0; //capital cost spent in year 0
  double capital_cost_year_1; //capital cost spent in year 1
  double capital_cost_year_2; //capital cost spent in year 2
  double capital_cost_year_3; //capital cost spent in year 3
  double capital_cost_year_4; //capital cost spent in year 4
  double capital_cost_year_5; //capital cost spent in year 5
  double tax_rate; //effective tax_rate (federal & state)
  double interest_during_construction; //interest rate during construction

  //Substructure & Foundation
  double mpileCR;//monopile pile cost rate ($/tonne)
  double mtransCR;//monopile transition piece cost rate ($/tonne)
  double mpileD;//monopile pile diameter (m)
  double mpileL;//monopile length (m)
  double jlatticeCR;//jacket lattice cost rate ($/tonne)
  double jtransCR;//jacket transition piece cost rate ($/tonne)
  double jpileCR;//jacket pile cost rate ($/tonne)
  double jlatticeA;//jacket lattice footprint area
  double jpileL;//jacket pile length
  double jpileD;//jacket pile diameter
  double spStifColCR;//spar stiffened column cost rate ($/tonne)
  double spTapColCR;//spar tapered column cost rate ($/tonne)
  double ballCR;//ballast cost rate ($/tonne)
  double deaFixLeng;//drag embedment anchor fixed mooring line length
  double ssStifColCR;//semisubmersible stiffened column cost rate ($/tonne)
  double ssTrussCR;// semisubmersible truss cost rate ($/tonne)
  double ssHeaveCR;//semisubmersible heave plate cost rate ($/tonne)
  double sSteelCR;//secondary steel cost rate ($/tonne)
  double moorDia;//mooring line diameter
  double moorCR;//mooring line cost rate ($/m)
  double mpEmbedL;//monopile embedment length (m)
  double scourMat;
	
  //Electrical Infrastructure
  double pwrFac;//power factor to estimate losses
  double buryFac;//cable burial factor
  double arrVoltage;//array cable voltage (kV)
  double arrCab1Size;//diameter in square millimeters of array cable 1
  double arrCab1Mass;//mass of array cable 1 (kg/m)
  double cab1CurrRating;//current rating of array cable 1 (amps)
  double cab1CR;//cost rate of array cable 1 ($/m)
  double cab1TurbInterCR;//array cable size 1 turbine interface cost rate ($/interface)
  double arrCab2Size;//diameter in square millimeters of array cable 2
  double arrCab2Mass;//mass of array cable 2 (kg/m)
  double cab2CurrRating;//current rating of array cable 2 (amps)
  double cab2CR;//cost rate of array cable 2 ($/m)
  double cab2TurbInterCR;//array cable size 2 turbine interface cost rate ($/interface)
  double cab2SubsInterCR;//array cable size 2 substation interface cost rate ($/interface)
  double catLengFac;//free hanging or catenary cable length factor
  double exCabFac;// excess cable factor
  double subsTopFab;//substation topside fabrication cost ($/tonne)
  double subsTopDes;//substation topside design cost ($)
  double topAssemblyFac;//land based substation topside assembly factor
  double subsJackCR;//substation jacket substructure cost rate ($/tonne)
  double subsPileCR;//substation jacket pile cost rate ($/tonne)
  double dynCabFac;//dynamic/free hanging cable cost premium
  double shuntCR;//shunt reactor cost rate ($/MVA)
  double highVoltSG;//high voltage switchgear cost ($)
  double medVoltSG;//medium voltage switchgear cost ($)
  double backUpGen;//back up generator cost ($)
  double workSpace;//substation workshop and accommodations cost ($)
  double otherAncillary;//substation other ancillary costs ($)
  double mptCR;//main power transformer cost rate ($/MVA)
  double expVoltage;//export cable voltage (kV)
  double expCabSize;//diameter in square millimeters of the export cable
  double expCabMass;//mass of the export cable (kg/m)
  double expCabCR;//cost rate of the export cable ($/m)
  double expCurrRating;//export cable rating (amps)
  double expSubsInterCR;//cost rate of export cable substation interfaces ($/interface)
	
  //Assembly & Installation
  double moorTimeFac;//mooring installation timing factor (hrs/m)
  double moorLoadout;//mooring system loadout timing (hrs)
  double moorSurvey;//mooring system anchor position survey timing (hrs)
  double prepAA;//prep inshore assembly area timing (hrs)
  double prepSpar;//prep spare for tow out to assembly area timing (hrs)
  double upendSpar;//upend and ballast the spar timing (hrs)
  double prepSemi;//prep semisubmersible for turbine install timing (hrs)
  double turbFasten;//fasten turbine for transport timing (hrs)
  double boltTower;// bolt tower to substructure timing (hrs)
  double boltNacelle1;//bolt nacelle to tower timing individual components method (hrs)
  double boltNacelle2;//bolt nacelle to tower timing bunny ears method (hrs)
  double boltNacelle3;//bolt nacelle to tower timing assembled rotor method (hrs)
  double boltBlade1;//bolt blade to rotor timing individual components method (hrs)
  double boltBlade2;//bolt blade to rotor timing bunny ears method (hrs)
  double boltRotor;//bolt rotor to nacelle timing assembled rotor method (hrs)
  double vesselPosTurb;//vessel positioning timing turbine install (hrs)
  double vesselPosJack;//vessel positioning timing jacket install (hrs)
  double vesselPosMono;//vessel positioning timing monopile install (hrs)
  double subsVessPos;//vessel positioning timing offshore substation install (hrs)
  double monoFasten;//fasten monopile for transport timing (hrs)
  double jackFasten;//fasten jacket for transport timing (hrs)
  double prepGripperMono;//prepare pile gripper and upender timing monopile install (hrs)
  double prepGripperJack;//prepare pile gripper and upender timing iacket install (hrs)
  double placePiles;//lift and place jacket piles timing (hrs)
  double prepHamMono;//prepare pile hammer timing monopile install (hrs)
  double removeHamMono;//remove hammer timing monopile install (hrs)
  double prepHamJack;//prepare pile hammer timing iacket install (hrs)
  double removeHamJack;//remove hammer timing iacket install (hrs)
  double placeJack;//place  jacket timing (hrs)
  double levJack;//level jacket timing (hrs)
  double placeTemplate;//place jacket template timing (hrs)
  double hamRate;//pile hammer rate (m/hr)
  double placeMP;//place monopile pile timing (hrs)
  double instScour;//install scour protection (hrs)
  double placeTP;//place transition piece on monopile timing (hrs)
  double groutTP;//grout transition piece (hrs)
  double tpCover;//install transition piece cover timing (hrs)
  double prepTow;//prep floating substructure for towing timing (hrs)
  double spMoorCon;//connect spar to mooring system timing (hrs)
  double ssMoorCon;//connect semisubmersible to mooring system (hrs)
  double spMoorCheck;//check mooring connections to spar timing (hrs)
  double ssMoorCheck;//check mooring connections to semisubmersible timing (hrs)
  double ssBall;//ballast semisubmersible timing (hrs)
  double surfLayRate;//electrical cable surface lay rate (m/hr)
  double cabPullIn;//array cable pull in to interfaces timing (hrs)
  double cabTerm;//cable termination and testing timing (hrs)
  double cabLoadout;//array cable loadout timing (hrs)
  double buryRate;//cable bury rate (m/hr)
  double subsPullIn;//cable pull in to substation timing (hrs)
  double shorePullIn;//cable pull in to shore timing (hrs)
  double landConstruct;//land construction of required onshore electrical systems timing (days)
  double expCabLoad;//export cable loadout timing (hrs)
  double subsLoad;//substation loadout timing (hrs)
  double placeTop;//lift and place substation topside timing (hrs)
  double pileSpreadDR;//piling equipment spread day rate ($/day)
  double pileSpreadMob;//piling equipment spread mobilization/demobilization cost ($)
  double groutSpreadDR;//grouting equipment spread day rate ($/day)
  double groutSpreadMob;//grouting equipment spread mobilization/demobilization cost ($)
  double seaSpreadDR;//suction pile anchor vessel and equipment spread day rate ($/day)
  double seaSpreadMob;//suction pile anchor vessel and equipment spread mobilization/demobilization cost ($)
  double compRacks;//component racks cost ($)
  double cabSurveyCR;//cost rate of surveying and verifying electrical cable installation ($/)
  double cabDrillDist;//horizontal drilling distance for cable landfall (m)
  double cabDrillCR;//horizontal drilling cost rate ($/m)
  double mpvRentalDR;//MPV rental day rate ($/day)
  double diveTeamDR;//cable landfall dive team day rate ($/day)
  double winchDR;//Cable winch day rate
  double civilWork;//civil construction work cost ($)
  double elecWork;//electrical work cost ($)
	
  //Port & Staging
  double nCrane600;
  double nCrane1000;
  double crane600DR;//600 tonne capacity crawler crane day rate ($/day)
  double crane1000DR;//1000 tonne capacity crawler crane day rate ($/day)
  double craneMobDemob;//crane mobilization and demobilization cost ($)
  double entranceExitRate;//port entrance and exit cost ($/m^2/occurrence)
  double dockRate;//port docking cost ($/day)
  double wharfRate;//port wharf loading and unloading cost ($/tonne)
  double laydownCR;//port laydown and storage cost ($/m/day)
	
  //Engineering & Management
  double estEnMFac;//estimated engineering and management cost factor
	
  //Development
  double preFEEDStudy;//pre-fornt end engineering design (FEED) study cost ($)
  double feedStudy;// FEED study cost ($)
  double stateLease;//state leasing cost ($)
  double outConShelfLease;//outer continental shelf lease cost ($)
  double saPlan;//site assessment plan cost ($)
  double conOpPlan;//construction operations plan cost ($)
  double nepaEisMet;//national environmental protection agency (NEPA) environmental impact (EIS) meteorological (met) tower study cost ($)
  double physResStudyMet;//physical resource met tower study cost ($)
  double bioResStudyMet;//biological resource met tower study ($)
  double socEconStudyMet;//socioeconomic met tower study cost ($)
  double navStudyMet;//navigation met tower study ($)
  double nepaEisProj;// NEPA EIS project site study cost ($)
  double physResStudyProj;//physical resource project site study cost ($)
  double bioResStudyProj;//biological resource project site study cost ($)
  double socEconStudyProj;//socioeconomic project site study cost ($)
  double navStudyProj;//navigation project site study cost ($)
  double coastZoneManAct;//coastal zone management act compliance cost ($)
  double rivsnHarbsAct;//rivers & harbors act section 10 compliance cost ($)
  double cleanWatAct402;//clean water act section 402 compliance cost ($)
  double cleanWatAct404;//clean water act section 404 compliance cost ($)
  double faaPlan;//federal aviation administration (FAA) plans and mitigation cost ($)
  double endSpecAct;//endangered species act compliance cost ($)
  double marMamProtAct;//marine mammal protection act compliance cost ($)
  double migBirdAct;//migratory bird act compliance ($)
  double natHisPresAct;//national historic preservation act compliance cost ($)
  double addLocPerm;//additional local and state permissions and compliance cost ($)
  double metTowCR;//meteorological tower fabrication, design, and install cost rate ($/MW)
  double decomDiscRate;//decommissioning expense discount rate

  //VECTORS TO HOLD VARIABLES************************************************************************************************************
  //cable vectors
  vector<cableFamily> arrCables;
  vector<cableFamily> expCables;

  //vessels
  vessel turbInstVessel;
  vessel turbFeederBarge;
  vessel subInstVessel;
  vessel subFeederBarge;
  vessel scourProtVessel;
  vessel arrCabInstVessel;
  vessel expCabInstVessel;
  vessel substaInstVessel;

  // arrays of vessels
  vector<vessel> turbSupportVessels;
  vector<vessel> subSupportVessels;
  vector<vessel> elecTugs;
  vector<vessel> elecSupportVessels;
  //CABLE & VESSEL TEMPLATES*******************************************************************************************
  map<int, cableFamily> arrayTemplates;
  map<string, vessel> vesselTemplates;
  //OUTPUTS************************************************************************************************************
  // Turbine outputs
  double hubD;
  double bladeL;
  double chord;
  double nacelleW;
  double nacelleL;
  double rnaM;
  double towerD;
  double towerM;

  //Substructure & Foundation outputs
  double subTotM;
  double subTotCost;
  double moorCost;
  
  //Electrical Infrastructure outputs
  double systAngle;
  double freeCabLeng;
  double fixCabLeng;
  double nExpCab;
  double expCabLeng;
  double expCabCost;
  double nSubstation;
  double cab1Leng;
  double cab2Leng;
  double arrCab1Cost;
  double arrCab2Cost;
  double subsSubM;
  double subsPileM;
  double subsTopM;
  double totElecCost;
  
  //Assembly & Installation outputs
  double moorTime;
  double floatPrepTime;
  double turbDeckArea;
  double nTurbPerTrip;
  double turbInstTime;
  double subDeckArea;
  double nSubPerTrip;
  double subInstTime;
  double arrInstTime;
  double expInstTime;
  double subsInstTime;
  double totInstTime;
  double cabSurvey;
  double array_cable_install_cost;
  double export_cable_install_cost;
  double substation_install_cost;
  double turbine_install_cost;
  double substructure_install_cost;
  double electrical_install_cost;
  double mob_demob_cost;

  //Port & Staging outputs
  double totPnSCost;

  //Development outputs
  double totDevCost;

  // Main Cost Outputs
  double bos_capex;
  double construction_insurance_cost;
  double total_contingency_cost;
  double construction_finance_cost;
  double construction_finance_factor; //factor for construction financing
  double soft_costs;
  double totAnICost;
  double totEnMCost;
  double commissioning;
  double decomCost;
  double total_bos_cost;
	
  //SUPPORTING FUNCTIONS************************************************************************************************************
  bool isFixed() { return ((substructure == MONOPILE) || (substructure == JACKET));}
  bool isFloating() { return ((substructure == SPAR) || (substructure == SEMISUBMERSIBLE));}
  void set_vessel_defaults();
  void map2variables();
  void variables2map();
  void set_map_variable(string keyStr, string valStr);
  void set_map_variable(string keyStr, double val);
  void set_map_variable(const char* key, double val);
  double get_map_variable(const char* key);
  
  //EXECUTE FUNCTION************************************************************************************************************
  void run();

  // Constructors
  wobos();

  
 private:
  map<string, int> str2substructure { {"MONOPILE", MONOPILE}, {"JACKET", JACKET}, {"SPAR", SPAR}, {"SEMISUBMERSIBLE", SEMISUBMERSIBLE} };
  map<string, int> str2anchor { {"DRAGEMBEDMENT", DRAGEMBEDMENT}, {"SUCTIONPILE", SUCTIONPILE} };
  map<string, int> str2turbInstallMethod { {"INDIVIDUAL",INDIVIDUAL}, {"BUNNYEARS",BUNNYEARS}, {"ROTORASSEMBLED", ROTORASSEMBLED} } ;
  map<string, int> str2towerInstallMethod { {"ONEPIECE", ONEPIECE}, {"TWOPIECE", TWOPIECE} } ;
  map<string, int> str2installStrategy { {"PRIMARYVESSEL", PRIMARYVESSEL}, {"FEEDERBARGE", FEEDERBARGE} } ;

  set<string> variable_percentage {"substructCont", "turbCont", "elecCont", "plantComm", "procurement_contingency", "install_contingency",
      "construction_insurance", "capital_cost_year_0", "capital_cost_year_1", "capital_cost_year_2", "capital_cost_year_3",
      "capital_cost_year_4", "capital_cost_year_5", "tax_rate", "interest_during_construction"};
 
  map<string, double> mapVars {
    {"substructure", 0.0},
    {"anchor", 0.0},
    {"turbInstallMethod", 0.0},
    {"towerInstallMethod", 0.0},
    {"installStrategy", 0.0},
    {"cableOptimizer", 0.0},
    {"turbCapEx", 0.0},
      {"nTurb", 0.0},
  {"rotorD", 0.0},
  {"turbR", 0.0},
  {"hubH", 0.0},
  {"waterD", 0.0},
  {"distShore", 0.0},
  {"distPort", 0.0},
  {"distPtoA", 0.0},
  {"distAtoS", 0.0},
  {"moorLines", 0.0},
  {"buryDepth", 0.0},
  {"arrayY", 0.0},
  {"arrayX", 0.0},
  {"substructCont", 0.0},
  {"turbCont", 0.0},
  {"elecCont", 0.0},
  {"interConVolt", 0.0},
  {"distInterCon", 0.0},
  {"scrapVal", 0.0},
  {"number_install_seasons", 0.0},

  //General
  {"projLife", 0.0},
  {"inspectClear", 0.0},
  {"plantComm", 0.0},
  {"procurement_contingency", 0.0},
  {"install_contingency", 0.0},
  {"construction_insurance", 0.0},
  {"capital_cost_year_0", 0.0},
  {"capital_cost_year_1", 0.0},
  {"capital_cost_year_2", 0.0},
  {"capital_cost_year_3", 0.0},
  {"capital_cost_year_4", 0.0},
  {"capital_cost_year_5", 0.0},
  {"tax_rate", 0.0},
  {"interest_during_construction", 0.0},

  //Substructure & Foundation
  {"mpileCR", 0.0},
  {"mtransCR", 0.0},
  {"mpileD", 0.0},
  {"mpileL", 0.0},
  {"jlatticeCR", 0.0},
  {"jtransCR", 0.0},
  {"jpileCR", 0.0},
  {"jlatticeA", 0.0},
  {"jpileL", 0.0},
  {"jpileD", 0.0},
  {"spStifColCR", 0.0},
  {"spTapColCR", 0.0},
  {"ballCR", 0.0},
  {"deaFixLeng", 0.0},
  {"ssStifColCR", 0.0},
  {"ssTrussCR", 0.0},
  {"ssHeaveCR", 0.0},
  {"sSteelCR", 0.0},
  {"moorDia", 0.0},
  {"moorCR", 0.0},
  {"mpEmbedL", 0.0},
  {"scourMat", 0.0},
  
  //Electrical Infrastructure
  {"pwrFac", 0.0},
  {"buryFac", 0.0},
  {"arrVoltage", 0.0},
  {"arrCab1Size", 0.0},
  {"arrCab1Mass", 0.0},
  {"cab1CurrRating", 0.0},
  {"cab1CR", 0.0},
  {"cab1TurbInterCR", 0.0},
  {"arrCab2Size", 0.0},
  {"arrCab2Mass", 0.0},
  {"cab2CurrRating", 0.0},
  {"cab2CR", 0.0},
  {"cab2TurbInterCR", 0.0},
  {"cab2SubsInterCR", 0.0},
  {"catLengFac", 0.0},
  {"exCabFac", 0.0},
  {"subsTopFab", 0.0},
  {"subsTopDes", 0.0},
  {"topAssemblyFac", 0.0},
  {"subsJackCR", 0.0},
  {"subsPileCR", 0.0},
  {"dynCabFac", 0.0},
  {"shuntCR", 0.0},
  {"highVoltSG", 0.0},
  {"medVoltSG", 0.0},
  {"backUpGen", 0.0},
  {"workSpace", 0.0},
  {"otherAncillary", 0.0},
  {"mptCR", 0.0},
  {"expVoltage", 0.0},
  {"expCabSize", 0.0},
  {"expCabMass", 0.0},
  {"expCabCR", 0.0},
  {"expCurrRating", 0.0},
  {"expSubsInterCR", 0.0},
	
  //Assembly & Installation
  {"moorTimeFac", 0.0},
  {"moorLoadout", 0.0},
  {"moorSurvey", 0.0},
  {"prepAA", 0.0},
  {"prepSpar", 0.0},
  {"upendSpar", 0.0},
  {"prepSemi", 0.0},
  {"turbFasten", 0.0},
  {"boltTower", 0.0},
  {"boltNacelle1", 0.0},
  {"boltNacelle2", 0.0},
  {"boltNacelle3", 0.0},
  {"boltBlade1", 0.0},
  {"boltBlade2", 0.0},
  {"boltRotor", 0.0},
  {"vesselPosTurb", 0.0},
  {"vesselPosJack", 0.0},
  {"vesselPosMono", 0.0},
  {"subsVessPos", 0.0},
  {"monoFasten", 0.0},
  {"jackFasten", 0.0},
  {"prepGripperMono", 0.0},
  {"prepGripperJack", 0.0},
  {"placePiles", 0.0},
  {"prepHamMono", 0.0},
  {"removeHamMono", 0.0},
  {"prepHamJack", 0.0},
  {"removeHamJack", 0.0},
  {"placeJack", 0.0},
  {"levJack", 0.0},
  {"placeTemplate", 0.0},
  {"hamRate", 0.0},
  {"placeMP", 0.0},
  {"instScour", 0.0},
  {"placeTP", 0.0},
  {"groutTP", 0.0},
  {"tpCover", 0.0},
  {"prepTow", 0.0},
  {"spMoorCon", 0.0},
  {"ssMoorCon", 0.0},
  {"spMoorCheck", 0.0},
  {"ssMoorCheck", 0.0},
  {"ssBall", 0.0},
  {"surfLayRate", 0.0},
  {"cabPullIn", 0.0},
  {"cabTerm", 0.0},
  {"cabLoadout", 0.0},
  {"buryRate", 0.0},
  {"subsPullIn", 0.0},
  {"shorePullIn", 0.0},
  {"landConstruct", 0.0},
  {"expCabLoad", 0.0},
  {"subsLoad", 0.0},
  {"placeTop", 0.0},
  {"pileSpreadDR", 0.0},
  {"pileSpreadMob", 0.0},
  {"groutSpreadDR", 0.0},
  {"groutSpreadMob", 0.0},
  {"seaSpreadDR", 0.0},
  {"seaSpreadMob", 0.0},
  {"compRacks", 0.0},
  {"cabSurveyCR", 0.0},
  {"cabDrillDist", 0.0},
  {"cabDrillCR", 0.0},
  {"mpvRentalDR", 0.0},
  {"diveTeamDR", 0.0},
  {"winchDR", 0.0},
  {"civilWork", 0.0},
  {"elecWork", 0.0},
	
  //Port & Staging
  {"nCrane600", 0.0},
  {"nCrane1000", 0.0},
  {"crane600DR", 0.0},
  {"crane1000DR", 0.0},
  {"craneMobDemob", 0.0},
  {"entranceExitRate", 0.0},
  {"dockRate", 0.0},
  {"wharfRate", 0.0},
  {"laydownCR", 0.0},
	
  //Engineering & Management
  {"estEnMFac", 0.0},
	
  //Development
  {"preFEEDStudy", 0.0},
  {"feedStudy", 0.0},
  {"stateLease", 0.0},
  {"outConShelfLease", 0.0},
  {"saPlan", 0.0},
  {"conOpPlan", 0.0},
  {"nepaEisMet", 0.0},
  {"physResStudyMet", 0.0},
  {"bioResStudyMet", 0.0},
  {"socEconStudyMet", 0.0},
  {"navStudyMet", 0.0},
  {"nepaEisProj", 0.0},
  {"physResStudyProj", 0.0},
  {"bioResStudyProj", 0.0},
  {"socEconStudyProj", 0.0},
  {"navStudyProj", 0.0},
  {"coastZoneManAct", 0.0},
  {"rivsnHarbsAct", 0.0},
  {"cleanWatAct402", 0.0},
  {"cleanWatAct404", 0.0},
  {"faaPlan", 0.0},
  {"endSpecAct", 0.0},
  {"marMamProtAct", 0.0},
  {"migBirdAct", 0.0},
  {"natHisPresAct", 0.0},
  {"addLocPerm", 0.0},
  {"metTowCR", 0.0},
    {"decomDiscRate", 0.0},

    
    // INPUTS/OUTPUTS, depending on how module is called
  // Turbine outputs
  {"hubD", 0.0},
  {"bladeL", 0.0},
  {"chord", 0.0},
  {"nacelleW", 0.0},
  {"nacelleL", 0.0},
  {"rnaM", 0.0},
  {"towerD", 0.0},
  {"towerM", 0.0},

  //Substructure & Foundation outputs
  {"subTotM", 0.0},
  {"subTotCost", 0.0},
  {"moorCost", 0.0},

    
    // OUTPUTS
  //Electrical Infrastructure outputs
  {"systAngle", 0.0},
  {"freeCabLeng", 0.0},
  {"fixCabLeng", 0.0},
  {"nExpCab", 0.0},
  {"expCabLeng", 0.0},
  {"expCabCost", 0.0},
  {"nSubstation", 0.0},
  {"cab1Leng", 0.0},
  {"cab2Leng", 0.0},
  {"arrCab1Cost", 0.0},
  {"arrCab2Cost", 0.0},
  {"subsSubM", 0.0},
  {"subsPileM", 0.0},
  {"subsTopM", 0.0},
  {"totElecCost", 0.0},
  
  //Assembly & Installation outputs
  {"moorTime", 0.0},
  {"floatPrepTime", 0.0},
  {"turbDeckArea", 0.0},
  {"nTurbPerTrip", 0.0},
  {"turbInstTime", 0.0},
  {"subDeckArea", 0.0},
  {"nSubPerTrip", 0.0},
  {"subInstTime", 0.0},
  {"arrInstTime", 0.0},
  {"expInstTime", 0.0},
  {"subsInstTime", 0.0},
  {"totInstTime", 0.0},
  {"cabSurvey", 0.0},
  {"array_cable_install_cost", 0.0},
  {"export_cable_install_cost", 0.0},
  {"substation_install_cost", 0.0},
  {"turbine_install_cost", 0.0},
  {"substructure_install_cost", 0.0},
  {"electrical_install_cost", 0.0},
  {"mob_demob_cost", 0.0},

  //Port & Staging outputs
  {"totPnSCost", 0.0},

  //Development outputs
  {"totDevCost", 0.0},

  // Main Cost Outputs
  {"bos_capex", 0.0},
  {"construction_insurance_cost", 0.0},
  {"total_contingency_cost", 0.0},
  {"construction_finance_cost", 0.0},
  {"construction_finance_factor", 0.0}, //factor for construction financing
  {"soft_costs", 0.0},
  {"totAnICost", 0.0},
  {"totEnMCost", 0.0},
  {"commissioning", 0.0},
  {"decomCost", 0.0},
  {"total_bos_cost", 0.0}
  };
  
  void set_templates();
  vector<cableFamily> set_cables(vector<int> cableVoltages);
  vector<vessel> set_vessels(vector<string> vesselNames);
  
  //General Module
  void set_turbine_parameters();  

  // Final summation
  void calculate_bos_cost();

  //Substructure & Foundation Module
  tuple<double, double> calculate_monopile();
  tuple<double, double> calculate_jacket();
  tuple<double, double> calculate_spar();
  tuple<double, double> calculate_ballast();
  tuple<double, double> calculate_semi();
  tuple<double, double> calculate_secondary_steel(int substructure);
  tuple<double, double> calculate_secondary_steel();
  void calculate_mooring();
  void calculate_substructure_mass_cost();

  //Electrical Infrastructure Module
  double calculate_subsea_cable_cost();
  double calculate_substation_cost();
  double calculate_onshore_transmission_cost();
  double calculate_export_cable_cost(double expCurrRating, double expVoltage, double expCabMass, double expSubsInterCR, double expCabCR);
  double calculate_array_cable_cost(double cab1CurrRating, double cab2CurrRating, double arrVoltage, double arrCab1Mass, double arrCab2Mass,
				    double cab1CR, double cab2CR, double cab1TurbInterCR, double cab2TurbInterCR, double cab2SubsInterCR);
  void calculate_electrical_infrastructure_cost();

  //Assembly & Installation Module
  void calculate_assembly_and_installation();
  double MinTurbDeckArea();
  double TurbineInstall();
  double SubstructureInstTime();
  void TurbInstCost();
  void SubInstCost();
  void ElectricalInstCost();
  void VesselMobDemobCost();
  double TotInstCost();

  //Port & Staging Module
  double calculate_entrance_exit_costs();
  void calculate_port_and_staging_costs();

  //Engineering & Management Module
  void calculate_engineering_management_cost();

  //Development Module
  void calculate_development_cost();

  double DecomissExpense();
  double PlantCommissioning();

  //cable cost optimizing functions
  void ArrayCabCostOptimizer();
  void ExportCabCostOptimizer();
};

// For SAM
//extern wobos wobos_obj;
//extern wobos *wobos_cm;

#endif
