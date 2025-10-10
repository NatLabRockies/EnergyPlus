Currently the transition program can take idf from one major version 8.x.0 (starting with version 8.2.0, prior to that major versions were 8.x.y) to the current iteration version represented by CMAKE_VERSION_MAJOR.CMAKE_VERSION_MINOR.CMAKE_VERSION_PATCH.  It is not possible to go from a minor version (iteration) to current at this time.  The capability of the transition program is naturally dependent on the transition rules being maintained throughout the iteration cycle.

Maintenance of the transition program consists of updating the rules during feature development.  (Rules should come in with feature branches if transition is required) It also requires managing some files at major release events.  Here are the steps as they are known at this time.

Task List:

* Append to the VERSIONS list in CMakeLists.txt at the root of the Transition project.
* Rename CreateNewIDFUsingRulesVX_Y_Z.in.f90 to CreateNewIDFUsingRulesVX_Y_Z.f90 and remove cmake tokens.
* Rename IDDAssignVX_Y_Z.in.f90 to IDDAssignVX_Y_Z.f90 and remove cmake tokens.
* Create a new CreateNewIDFUsingRulesVX_Y_Z.in.f90 file that is named according to the next upcoming MAJOR release.
* Create a new IDDAssignVX_Y_Z.in.f90 file that is named according to the next upcoming MAJOR release.
* Move EnergyPlusBuildSupport / release / Report Variables X1.Y1.Z1 to X2.Y2.Z2.csv to EnergyPlusBuildSupport / bin / IDFVersionUpdater / ReportVars, where the version represents the major releases.
* Create a new ReportVariables...csv files named according to the upcoming major release and prevous major release.  Place new file in EnergyPlusBuildSupport / release.
* Update install commands for previous major version IDD as well as ReportVariables...csv located in EnergyPlusTeam / CPack.cmake, so that new upcoming major version is reflected.
