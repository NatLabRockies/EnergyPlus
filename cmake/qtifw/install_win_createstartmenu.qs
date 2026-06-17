// Windows commands to be performed elevated: copy and register DLLs

function Component()
{
  Component.prototype.createOperations = function()
  {
    // call default implementation
    component.createOperations();

    // ... add custom operations

    var kernel = systemInfo.kernelType;

    // On Windows (which is always true in this context)
    if( kernel == "winnt" ) {

      // Regular case: same as just using "@StartMenuDir@/link_name.lnk"
      // [QTIFW-697]: StartMenuDir returns the full path, not just the group.
      // eg: installer.value("UserStartMenuProgramsPath") + "/EnergyPlusV9-2-0"
      var startMenuDir = installer.value("StartMenuDir");

      var target_dir = startMenuDir;

      // Note: Actually there is a built-in flag AllUsers=true that will already make
      // StartMenuDir = installer.value("AllUsersStartMenuProgramsPath") + "/EnergyPlusV9-2-0"
      // So the workaround below isn't needed (keeping it just in case for now)

/*
 *      // For use with install_script.qs, for headless install via local system
 *      // admin
 *      if (installer.value("UseAllUsersStartMenu") === "true") {
 *
 *        // Extract the last part of that full group: that's your group, eg "EnergyPlusV9-2-0"
 *        var startMenuGroup = startMenuDir.match(/([^\\]*)\\*$/)[1];
 *
 *        // TODO: extra logging for debug
 *        console.log("startMenuGroup: " + startMenuGroup);
 *        console.log("startMenuDir: " + startMenuDir);
 *        console.log("Reconstructed startMenuDir using UserStartMenuProgramsPath & startMenuGroup: " + installer.value("UserStartMenuProgramsPath") + "/" + startMenuGroup);
 *
 *        target_dir = installer.value("AllUsersStartMenuProgramsPath") + "/" + startMenuGroup;
 *        console.log("UseAllUsersStartMenu was passed!")
 *      }
 */

      console.log("Target directory for Start Menu Shortcuts: " + target_dir);

      component.addOperation("CreateShortcut", "@TargetDir@/Documentation/index.html", target_dir + "/EnergyPlus Documentation.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/PreProcess/EPDraw/EPDrawGUI.exe", target_dir + "/EPDrawGUI.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/EP-Launch.exe", target_dir + "/EP-Launch.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/ExampleFiles/ExampleFiles.html", target_dir + "/Example Files Summary.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/ExampleFiles/ExampleFiles-ObjectsLink.html", target_dir + "/ExampleFiles Link to Objects.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/PreProcess/IDFEditor/IDFEditor.exe", target_dir + "/IDFEditor.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/PreProcess/IDFVersionUpdater/IDFVersionUpdater.exe", target_dir + "/IDFVersionUpdater.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/readme.html", target_dir + "/Readme Notes.lnk");
      component.addOperation("CreateShortcut", "@TargetDir@/PreProcess/WeatherConverter/Weather.exe", target_dir + "/Weather Statistics and Conversions.lnk");

    }
  }
}
