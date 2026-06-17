function Component()
{
  Component.prototype.createOperations = function()
  {
    // Call default implementation
    component.createOperations();
    let kernel = systemInfo.kernelType;
    if (kernel === "linux") {
      // For Linux, we need to set up the .desktop file
      const version = "@ProductVersion@";
      // Now get the path to EnergyPlus installation plus the path to drop the .desktop file
      const installPath = installer.value("TargetDir"); // User-selected install path
      const homeDir = installer.value("HomeDir");
      const desktopFilePath = homeDir + "/.local/share/applications/energyplus." + version + ".desktop";
      const templateFile = installPath + "/share/applications/energyplus.desktop.in";
      // Populate the template, but delay execution by using addOperation() after install is complete
      component.addOperation(
        "Execute", "/bin/sh", [
          "-c",
          "if [ -f '" + templateFile + "' ]; then " +
          "sed 's|ENERGYPLUS_INSTALL_DIR|" + installPath + "|g; s|ENERGYPLUS_VERSION|" + version + "|g' '" + templateFile + "' > '" + desktopFilePath + "' && " +
          "chmod 644 '" + desktopFilePath + "'; " +
          "else echo 'Error: .desktop template file missing'; fi"
        ],
        "UNDOEXECUTE",
        "rm", desktopFilePath
      );
    } else if( kernel === "winnt" ) {
      const target_dir = installer.value("StartMenuDir");
      component.addOperation("CreateShortcut", "@TargetDir@/windows_gui_launcher.exe", target_dir + "/EP-Launch-Python.lnk", "eplaunch");
      component.addOperation("CreateShortcut", "@TargetDir@/windows_gui_launcher.exe", target_dir + "/IDFVersionUpdater-Python.lnk", "updater");
    }
  }
}
